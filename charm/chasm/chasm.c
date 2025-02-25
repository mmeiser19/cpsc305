#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "dict.h"
#include "fsms.h"
#include "chasm_types.h"

/* enums, structs, and array ins[]
 * chasm types are found in the file chasm_types.h. The types are the following.
 * toks_t - defines enumeration values for tokens
 * tokv_t - defines struct to hold token values of a specific token
 * toki_t - defines struct to hold token information for each line 
 *        - has line unique info (e.g. linenum)
 *        - has individual token info
 *    enums toks_t has a corresponding char *toks_t_str[]. Each element of toks_t_str
 *    is the string of the enum value. For example, toks_t enum data is "data"
 *    C allocates integers to enum value beginning at 0. For example,
 *    enum toks_t { data, text, ...}, data is 0, text is 1, and so on
 *    You can use an enum tag as an index into the array to access the string.
 *    For example, printf("%s\n", toks_t_str[label]); prints label
 * inst_c - defines enumeration values for Charm instruction catetories
 *    ldrstr, arilog, movcmp, branch, miscos
 *
 * inst_t - defines enumeration values for Charm instructions
 * inst_info - defines struct to hold instruction infomation
 *    struct inst_info ins[] is an array with information for each instruction.
 *    ins is indexed with an inst_t enum value. 
 *    For example ins[bgt] selects instn info for the bgt instruction
 *
 * charm_tools generates types inst_c, inst_t, inst_info and the array ins[]
 * charm_insts.txt contains the input to charm_tools
 * charm_tools output is to stdout, which can be redirected to file if desired
 * After running charm_tools, you do two things.
 * 1. replace inst_c, inst_t, and inst_info in the file chasm_types.h with those 
 *    generated by charm_tools
 * 2. replace the array ins[] immediately following this comment with the one
 *    generated by charm_tools
 */

struct inst_info ins[] = {
    {0x72646c, "ldr", ldr, ldrstr, 0x00000010}, {0x62646c, "ldb", ldb, ldrstr, 0x00000020}, 
    {0x727473, "str", str, ldrstr, 0x00000030}, {0x627473, "stb", stb, ldrstr, 0x00000040}, 
    {0x646461, "add", add, arilog, 0x00000050}, {0x627573, "sub", sub, arilog, 0x00000051}, 
    {0x6c756d, "mul", mul, arilog, 0x00000052}, {0x766964, "div", dIv, arilog, 0x00000053}, 
    {0x646e61, "and", and, arilog, 0x00000054}, {0x72726f, "orr", orr, arilog, 0x00000055}, 
    {0x726f65, "eor", eor, arilog, 0x00000056}, {0x636461, "adc", adc, arilog, 0x00000057}, 
    {0x636273, "sbc", sbc, arilog, 0x00000058}, {0x666461, "adf", adf, arilog, 0x00000059}, 
    {0x666273, "sbf", sbf, arilog, 0x0000005a}, {0x66756d, "muf", muf, arilog, 0x0000005b}, 
    {0x666964, "dif", dif, arilog, 0x0000005c}, {0x696461, "adi", adi, arilog, 0x0000005d}, 
    {0x696273, "sbi", sbi, arilog, 0x0000005e}, {0x766f6d, "mov", mov, movcmp, 0x00000000}, 
    {0x61766d, "mva", mva, movcmp, 0x00000001}, {0x706d63, "cmp", cmp, movcmp, 0x00000002}, 
    {0x747374, "tst", tst, movcmp, 0x00000003}, {0x716574, "teq", teq, movcmp, 0x00000004}, 
    {0x666873, "shf", shf, movcmp, 0x00000005}, {0x616873, "sha", sha, movcmp, 0x00000006}, 
    {0x746f72, "rot", rot, movcmp, 0x00000007}, {0x6c6162, "bal", bal, branch, 0x00000000}, 
    {0x716562, "beq", beq, branch, 0x00000001}, {0x656e62, "bne", bne, branch, 0x00000002}, 
    {0x746c62, "blt", blt, branch, 0x00000003}, {0x656c62, "ble", ble, branch, 0x00000004}, 
    {0x746762, "bgt", bgt, branch, 0x00000005}, {0x656762, "bge", bge, branch, 0x00000006}, 
    {0x726c62, "blr", blr, branch, 0x00000007}, {0x72656b, "ker", ker, miscos, 0x000000b0},
    {0x677273, "srg", srg, miscos, 0x000000b1}, {0x696f69, "ioi", ioi, miscos, 0x000000b2},
};

char *toks_t_str[] = { "data", "text", "label", "string", "inst", "comment",
                        "reg", "comma", "number", "leftbrack", "rightbrack", "exclam", "ident", "err", "endd" };

#define MAX_LINE 100        // maximum length of asm prog line
#define MAX_TOKENS 15       // maximum number of tokens on asm program line
#define MAX_PROG_LINES 2000 // maximum number of lines in an asm program

static char buf[MAX_LINE];              // lines from .chasm file read into buf
static char tokbuf[MAX_LINE*2];         // buf for line with whitespace around , [ and ]
static int  linenum = 1;                // counts lines in file

struct toki_t tok;                      // used to tokenize a line
struct toki_t toks[MAX_PROG_LINES];     // all tokenized lines are stored in toks

/*
 toksvalue - determine token type and value of the string t and put value in tok.toks[n].toktype, .tokv
 struct toki_t tok is built for each line in assembly file
 tok.toks[] is array of tokens on the current line
 The function tokenize() is filling in tok.toks[], which calls toksvalue for each string on line
 inputs
  char *t - address of string on line that is a token
   *t can point to any string on a line.
   Sample ldr r0, [r13, #5] - *t can point to "r0", ",", "[", "r13", ",", "#5", and "]"
  n - the token number in toki_t tok to compute value. for ldr r0, [r13, #5], r0 has an n value of 1,
      [ has an n value of 2, r13 has an n value of 3, and so on.
   n is used as an index into tok.toks[]
  struct inst_info ins[] - array of charm instructions - used to see if *t is type inst
 output
  struct toki_t tok.toks[n].toktype - the string in *t can be and of the enum toks_t values
   examples: ldr is inst, .data is data, r0 is reg, 0x5 is number, gusty is ident, [ is leftbrack
  struct toki_t tok.toks[n].tokv - the value of the token for reg and number tokens
   example: r8 has a value of 8 and 0xf has a value of 15
  struct toki)t tok.insttype, .instcate, and .instopcd - values when the token is inst
 */
void toksvalue(char *t, int n) {
    tok.toks[n].tokv = 0;
    for (int i = 0; i < sizeof(ins) / sizeof(struct inst_info); i++) // check for insts
        //if (strcmp(t, ins[i].inst_str) == 0) {
        if (*(int*)t == ins[i].inst_int) { // inst_int is integer of 3 char instruction
            tok.toks[n].toktype = inst;
            tok.insttype = ins[i].inst_t;
            tok.instcate = ins[i].inst_c;
            tok.instopcd = ins[i].opcode;
            return;
        }
    int base = 0;
    if (t[0] == '/' && t[1] == '/')
        tok.toks[n].toktype = comment;
    else if (strcmp(t, ".data") == 0)
        tok.toks[n].toktype = data;
    else if (strcmp(t, ".stack") == 0)
        tok.toks[n].toktype = data;
    else if (strcmp(t, ".text") == 0)
        tok.toks[n].toktype = text;
    else if (strcmp(t, ".label") == 0)
        tok.toks[n].toktype = label;
    else if (strcmp(t, ".string") == 0)
        tok.toks[n].toktype = string;
    else if (t[0] == ',')
        tok.toks[n].toktype = comma;
    else if (t[0] == '[')
        tok.toks[n].toktype = leftbrack;
    else if (t[0] == ']')
        tok.toks[n].toktype = rightbrack;
    else if (t[0] == '!')
        tok.toks[n].toktype = exclam;
    else if (isreg(t)) {  // could update isreg() to look for pc, lr, and sp
        tok.toks[n].toktype = reg;
        tok.toks[n].tokv = atoi(t+1);
    }
    else if (t[0] == 'p' && t[1] == 'c') { // && t[2] == '\0' I think this is needed
        tok.toks[n].toktype = reg;
        tok.toks[n].tokv = 15;
    }
    else if (t[0] == 'l' && t[1] == 'r') {
        tok.toks[n].toktype = reg;
        tok.toks[n].tokv = 14;
    }
    else if (t[0] == 's' && t[1] == 'p') {
        tok.toks[n].toktype = reg;
        tok.toks[n].tokv = 13;
    }
    else if ((base = isnumber(t))) {
        tok.toks[n].toktype = number;
        int digitpos = 0;
        if (t[0] == '#')
            digitpos = 1;
        if (base == 16)
            digitpos += 2;
        tok.toks[n].tokv = (int)strtol(t+digitpos, NULL, base);
    }
    else if (isid(t))
        tok.toks[n].toktype = ident;
    else  {
        tok.toks[n].toktype = err;
        tok.linetype = err; // any err token causes line to be err
    }
}

/*
 insertwhitespace - insert whitespace around chasm separators
 input
  static char buf[] - has a line read from an assembly file, read by main
  static char separators[] - static array with chasm separators
 output
  static char tokbuf[] - buf with whitespace inserted around separators
 sample
   ldr r0,[r13,#5] becomes ldr r0 , [ r13 , #5 ] - there is a trailing space
 returns
  length of tokbuf
 */
static char separators[] = ",[]!";      // chasm separators ldr r0, [r5, #3]!
int insertwhitespace() { // build tokbuf[]. insert whitespace around separators
    int j = 0, comment = 0;
    for (int i = 0; i < strlen(buf); i++) {
        if (strchr(separators, buf[i])) {
            tokbuf[j++] = ' ';
            tokbuf[j++] = buf[i];
            tokbuf[j++] = ' ';
        }
        else {
            if (buf[i] == '/' && buf[i+1] == '/')
                comment = 1;
            if (!comment && buf[i] >= 'A' && buf[i] <= 'Z')
                buf[i] |= 0x20; // convert upper case to lower
            tokbuf[j++] = buf[i];
        }
    }
    tokbuf[j++] = ' '; // put space at end in front of \0
    tokbuf[j++] = 0;
    return j;
}

/*
 tokenize - tokenize a line into a struct toki_t tok
 Each line of the assembly file is tokenized into tok
 inputs
  char buf[] - has a line read from an assembly file
  char tokbuf[] - buf with whitespace around separators
  char whitespace[] - array with whitespace chars
 output
  struct toki_t tok - a tokenized line
 */
static char whitespace[] = " \t\r\n\v"; // definition of whitespace - tokenize skips whitespace
void tokenize() {
    insertwhitespace();
    char *s = tokbuf;
    char *end_buf = tokbuf + strlen(tokbuf);
    int numtoks = 0;
    int toknum = 0;
    tok.linetype = comment;                            // assume a blank line
    while (1) {
        toknum = numtoks;
        while (s < end_buf && strchr(whitespace, *s))  // skip whitespace
            s++;
        if (*s == 0)                                   // eol - done
            break;
        char *t = s;                                   // t is address of start of token string
        while (s < end_buf && !strchr(whitespace, *s)) // consume non-whitespace, adv to end of token string
            s++;
        numtoks++;                                     // count tokens on line
        *s = 0;                                        // null terminate token string
        tok.toks[toknum].tok_str = strdup(t);          // dup token string 
        toksvalue(t, toknum);                          // determine token and its value
        if (toknum == 0)
            tok.linetype = tok.toks[0].toktype;
        if (tok.toks[toknum].toktype == comment) {     // stop calling toksvalue() when comment found
            *s = ' ';
            tok.toks[toknum].tok_str = strdup(t);      // dup remainder of line
            break;
        }
    }
    tok.toks[numtoks++].toktype = endd;                // Place endd token on toks
    tok.numtoks = numtoks;                             // number of tokens on this line
}

/*
 printtok - print a struct toki_t to stdout
 input
  struct toki_t t - the tokenized value of a line in the assembly file
 output
  tokenized line displayed on stdout
 */
void printtok(struct toki_t t) {
    printf("linenum: %d, line type: %s, numtoks: %d, address: %d\n", t.linenum, toks_t_str[t.linetype], t.numtoks, t.address);
    if (t.linetype == inst) {
        printf("  inst type: %s, ", ins[t.insttype].inst_str);
        printf("  inst cate: %d, ", t.instcate);
        printf("  inst opcd: 0x%08x\n", t.instopcd);
    }
    for (int j = 0; j < t.numtoks; j++) {
        printf("t.toks[%d].tok_str: %7s | ", j, t.toks[j].tok_str);
        printf("t.toks[%d].toktype: %10s | ", j, toks_t_str[t.toks[j].toktype]);
        printf("t.toks[%d].tokv   : %7d | ", j, t.toks[j].tokv);
        printf("\n");
    }
    printf("\n");
}

/*
 addrsymopcode - generate symbol table and put address in each line's toks[i].address member
 inputs
  struct toki_t toks[] - array of all lines in assembly file - tokenized
 output
  toks[i].address - lines that are number or inst have an address
  toks[i].instopcd - lines that are inst have an instruction opcode
  symbol table - lines that are labels are added to symbol table - key is label, value is address
  NOTE: Symbole table maintained via dictput function
  struct toki_t toks[].address - address of all labels, numbers, and instructions filled in
  struct toki_t toks[].instopcd - opcodes of instructions filled in
 */
void addrsymopcode() {
    int address = 0;  // assume code/data is at address 0
    for (int i = 0; i < linenum-1; i++) {
        // .text 0x200 and .data 0x300 directives control addresses of code/data
        if ((toks[i].linetype == data || toks[i].linetype == text) && toks[i].numtoks == 3)
            address = toks[i].toks[1].tokv; // update address based on .data and .text directives
        else if (toks[i].linetype == label) {
            toks[i].address = address;
            if (dictput(toks[i].toks[1].tok_str, toks[i].address)) // add to symbol table
                toks[i].linetype = err;     // duplicate symbole
        }
        else if (toks[i].linetype == number || toks[i].linetype == inst) {
            toks[i].address = address;
            address+= 4;
            if (toks[i].linetype == inst) {
                toks[i].instopcd = isinst(toks[i]);
                if (toks[i].instopcd < 0)
                    toks[i].linetype = err;
                if (toks[i].instopcd >= 0x50 && toks[i].instopcd <= 0x5c && toks[i].toks[5].toktype != reg)
                    toks[i].linetype = err;
            }
        }
        else if (toks[i].linetype == string && toks[i].toks[1].toktype == comment) {
            toks[i].address = address;
            //char *str = strdup(toks[i].toks[1].tok_str + 2);
            int l = strlen(toks[i].toks[1].tok_str + 2) - 1; // skip comment chars and subtract extra space
            if (l % 4 == 0)
                l++;                             // add one to null terminate the string
            //int l4 = l + 3 & ~0x3;             // make l a multiple of 4
            address = address + (l + 3 & ~0x3);  // make l a multiple of 4
        }
    }
}

/*
 identifiers - lookup identifiers in symbol table and assign the value to toks[i].toks[j].tokv
 inputs
  struct toki_t toks[] - array of all lines in assembly file - tokenized with addresses
 output
  struct toki_t toks[].toks[j].tokv - For all tokens that are identifiers, the function identifiers places their value
  in the tokv member. The value is their addres. The function addrsymopcode filled in the values of indentifiers
  NOTE: Symbol table is used to lookup identifier and return its value (address)
 */
void identifiers() {
    for (int i = 0; i < linenum-1; i++)
        if (toks[i].linetype == inst)
            for (int j = 0; j < toks[i].numtoks-1; j++) // numtoks-1 to ignore endd token
                if (toks[i].toks[j].toktype == ident) {
                    toks[i].toks[j].tokv = dictget(toks[i].toks[j].tok_str);
                    if (toks[i].toks[j].tokv == -1000001)
                        toks[i].linetype = err;
                }
}

/*
 errors - generate error messages if any
 inputs
  struct toki_t toks[] - array of all lines in assembly file - tokenized with errors (if any)
 output
  error messages to stdout
 return 
  0 - no errors in assembly input
  1 - errors in assembly input
 */
int errors() {
    int errorfound = 0;
    for (int i = 0; i < linenum-1; i++) {
        if (toks[i].linetype > comment && toks[i].linetype != number) {
            fprintf(stderr, "+++ Error +++ Line number: %d\n", toks[i].linenum);
            for (int j = 0; j < toks[i].numtoks-1; j++) // numtoks-1 to ignore endd token
                fprintf(stderr, "%s ", toks[i].toks[j].tok_str);
            fprintf(stderr, "\n");
            errorfound = 1;
        // Check for other errors, e.g. add r, r5, r5   .label without following num or inst
        }
    }
    return errorfound;
}

int verbose = 0;  // non-zero turns on verbose output - debug information
int symbols = 1;  // zero suppresses symbols from output - useful for OS

/*
 generatecode - generate code to stdout
 inputs
  struct toki_t toks[] - array of all lines in assembly file
  A toki_t struct has all needed to generate a 32-bit hex value for the code
 output
  code is displayed on stdout
 */
void generatecode() {
    int instr, opcode, rd, rm, rn, imm;
    for (int i = 0; i < linenum-1; i++) {
        if (toks[i].linetype == inst) {
          opcode = toks[i].instopcd;
          switch (toks[i].instcate) {
            case ldrstr:
              rd = toks[i].toks[1].tokv;
              switch (opcode & 0xf) {
                case 0: // ldr rd, addr
                    imm = toks[i].toks[3].tokv & 0xfffff;
                    instr = opcode << 24 | rd << 20 | imm;
                    break;
                case 1: // ldr rd, [rm]
                    rm = toks[i].toks[4].tokv;
                    instr = opcode << 24 | rd << 20 | rm << 16;
                    break;
                case 2: case 4: // ldr rd, [rm, #3] and ldr rd, [rm, #3]!
                    rm = toks[i].toks[4].tokv;
                    imm = toks[i].toks[6].tokv & 0xffff;
                    instr = opcode << 24 | rd << 20 | rm << 16 | imm;
                    break;
                case 3: case 5: // ldr rd, [rm, rn] and ldr rd, [rm, rn]!
                    rm = toks[i].toks[4].tokv;
                    rn = toks[i].toks[6].tokv;
                    instr = opcode << 24 | rd << 20 | rm << 16 | rn << 12;
                    break;
                case 6: // ldr rd, [rm], #3
                    rm = toks[i].toks[4].tokv;
                    imm = toks[i].toks[7].tokv & 0xffff;
                    instr = opcode << 24 | rd << 20 | rm << 16 | imm;
                    break;
                case 7: // ldr rd, [rm], rn
                    rm = toks[i].toks[4].tokv;
                    rn = toks[i].toks[7].tokv;
                    instr = opcode << 24 | rd << 20 | rm << 16 | rn << 12;
                    break;
                default:
                    instr = -1;
                    break;
              }
              if (verbose)
                  printf("ldrstr: 0x%08x\n", instr);
              else
                  printf("0x%08x\n", instr);
                
              break;
            case arilog:;
              rd = toks[i].toks[1].tokv;
              rm = toks[i].toks[3].tokv;
              rn = toks[i].toks[5].tokv;
              if (opcode >= 0x50 && opcode <= 0x5c)
                  instr = opcode << 24 | rd << 20 | rm << 16 | rn << 12;
              else {
                  imm = toks[i].toks[5].tokv & 0xffff;
                  instr = opcode << 24 | rd << 20 | rm << 16 | imm;
              }
              if (verbose)
                  printf("arilog: 0x%08x\n", instr);
              else
                  printf("0x%08x\n", instr);
              break;
            case movcmp:
              if (opcode >> 4 == 6) { // mov r,r
                  rd = toks[i].toks[1].tokv;
                  rm = toks[i].toks[3].tokv;
                  instr = opcode << 24 | rd << 20 | rm << 16;
              }
              else {                  // mov r,#n
                  rd = toks[i].toks[1].tokv;
                  imm = toks[i].toks[3].tokv & 0xfffff;
                  instr = opcode << 24 | rd << 20 | imm;
              }
              if (verbose)
                  printf("movcmp: 0x%08x\n", instr);
              else
                  printf("0x%08x\n", instr);
              break;
            case branch:
              if (opcode >> 4 == 9) { // bal [r]
                  rd = toks[i].toks[2].tokv;
                  instr = opcode << 24 | rd << 20;
              }
              else if (opcode >> 4 == 8) { // bal addr
                  imm = toks[i].toks[1].tokv & 0xfffff;
                  instr = opcode << 24 |  imm;
              }
              else { // bal !addr - label is in toks[2], ! is in toks[1]
                  int offset = toks[i].toks[2].tokv - toks[i].address;
                  imm = offset & 0xfffff;
                  instr = opcode << 24 |  imm;
              }
              if (verbose)
                  printf("branch: 0x%08x\n", instr);
              else
                  printf("0x%08x\n", instr);
              break;
            case miscos:
              imm = toks[i].toks[1].tokv & 0xfffff; // ker #num, srg #num
              instr = opcode << 24 | imm;
              if (verbose)
                  printf("miscos: 0x%08x\n", instr);
              else
                  printf("0x%08x\n", instr);
              break;
            default:
              printf("default\n");
              break;
          }
        }
        else if (toks[i].linetype == number) {
            if (verbose)
                printf("number: 0x%08x\n", toks[i].toks[0].tokv);
            else
                printf("0x%08x\n", toks[i].toks[0].tokv);
        }
        else if (toks[i].linetype == string && toks[i].toks[1].toktype == comment) {
            int l = strlen(toks[i].toks[1].tok_str + 2) - 1; // skip comment chars and subtract extra space
            char strbuf[80];
            for (int i = 0; i < 80; i++)
                strbuf[i] = 0;
            strncpy(strbuf, toks[i].toks[1].tok_str + 2, l);
            if (l % 4 == 0)
                l++;                 // add one to null terminate the string
            int l4 = l + 3 & ~0x3;   // make l a multiple of 4
            if (verbose)
                printf(".string %s, len: %lu, len-1: %d, mul4: %d\n", strbuf, strlen(strbuf), l, l4);
            for (int i = 0; i < l4; i+=4) {
                int v = strbuf[i]<<24 | strbuf[i+1]<<16 | strbuf[i+2]<<8 | strbuf[i+3];
                if (verbose)
                    printf("string: 0x%08x\n", v);
                else
                    printf("0x%08x\n", v);
            }
        }
        else if (toks[i].linetype == text || toks[i].linetype == data) {
            printf("%s", toks[i].toks[0].tok_str);
            if (toks[i].numtoks >= 3) // .text 100 - 3 accomodates endd token
                printf(" %d", toks[i].toks[1].tokv);
            printf("\n");
        }
     }
}

static char *listing = NULL;

void generatelisting() {
    FILE *f = fopen(listing, "w");
    for (int i = 0; i < linenum-1; i++) {
        fprintf(f, "0x%08x  ", toks[i].address);
        for (int j = 0; j < toks[i].numtoks-1; j++) {
            fprintf(f, "%s ", toks[i].toks[j].tok_str);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

/*
 get_opts - called to process command line options
 See 16CPCS405 > Labs > FileProject > utilities > hexdump.c for code sample with opts and value
 input
  count - copy of argc
  args  - copy of argv
 output
  listing - -lfile sets listing to "file"
  return of 0 is error
  return of non 0 is index in argv of assembly file name
 */
int get_opts(int count, char *args[]) {
    int opt, good = 1;
    while (good && (opt = getopt(count, args, "l:vy")) != -1) {
        printf("opt: %c\n", opt);
        switch (opt) {
            case 'l':
                listing = strdup(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'y':
                symbols = 0;
                break;
            case ':':
                fprintf(stderr, "option missing value\n");
                break;
            case '?':
               if (optopt == 'l')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                //if (isprint(optopt))
                //    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                   fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                good = 0;
                break;
        }
    }
    if(good && optind > count-1) {
        fprintf(stderr, "Number of arguments. %d\n", optind);
        good = 0;
    }
    else if (good)
        good = optind;
    return good;

}
/*
 main - entry point for chasm.
 Options
 -l file : generate a listing in file
 -y : turn off symbols in the output file
 -v : turn on verbose more
 Sample Invocations
 ./chasm file.s
 ./chasm file.s > file.o
 ./chasm -l listfile.txt file.s - listing file is not implmented
 ./chams -y file.s
 ./chams -y file.s > file.o
 ./chasm -v file.s
 */

int main(int argc, char **argv) {
    int optind = get_opts(argc, argv);
    if (!optind) {
        fprintf(stderr, "Bad chasm command - probably missing file for -l\n");
        return -1;
    }

    // An open-do-close pattern to process the assembly file
    FILE *fp;
    if ((fp = fopen(argv[optind], "r")) == NULL) {
        fprintf(stderr, "File %s not found!\n", argv[optind]);
        return -1;
    }
    // Loop - reads line, tokenizes line in tok, saves tok in array toks
    while (fgets(buf, MAX_LINE, fp)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (verbose)
            printf("%s\n", buf);
        tokenize();              // convert buf to a struct toki_t tok
        tok.linenum = linenum;
        toks[linenum-1] = tok;   // save token in array of tokens
        linenum++;
        if (linenum >= MAX_PROG_LINES)
            fprintf(stderr, "Too many lines, Max: %d\n", MAX_PROG_LINES);
    }
    fclose(fp);                  // end open-do-close on assembly file
    addrsymopcode();             // create symbol table and fill in toks[].address
    identifiers();
    if (!errors()) {
        generatecode();
        if (symbols) {
            printf(".ymbl\n");
            dictprint(0); // symbol table
        }
        if (listing) {
            generatelisting();
        }
    }
    if (verbose) {
        printf("*******************************\n");
        dictprint(verbose);
        for (int i = 0; i < linenum-1; i++)
            printtok(toks[i]);
    }

    /*
    dictput("Gusty", 22);
    printf("dictget: %d\n", dictget("Gusty"));
    printf("dictget: %d\n", dictget("gusty"));
    dictprint();

    printf("isreg(r1): %d\n", isreg("r1"));
    printf("isreg(r1): %d\n", isreg("r9"));
    printf("isreg(r15): %d\n", isreg("r15"));
    printf("isreg(r15): %d\n", isreg("r16"));
    printf("isreg(r1x): %d\n", isreg("r1x"));
    printf("goodnumber(ab12, hexdigits): %d\n", goodnumber("ab12", hexdigits));
    printf("goodnumber(abx12, hexdigits): %d\n", goodnumber("abx12", hexdigits));
    printf("goodnumber(123, decdigits): %d\n", goodnumber("123", decdigits));
    printf("goodnumber(12x, decdigits): %d\n", goodnumber("12x", decdigits));
    printf("isnumber(123): %d\n", isnumber("123"));
    printf("isnumber(0xab12): %d\n", isnumber("0xab12"));
    printf("isnumber(#123): %d\n", isnumber("#123"));
    printf("isnumber(#0xab12): %d\n", isnumber("#0xab12"));
    printf("stol(-123): %d\n", (int)strtol("-123", NULL, 10));

    printf("isinst(): %x\n", isinst(toks[0]));
    printf("isinst(): %x\n", isinst(toks[1]));
    printf("isinst(): %x\n", isinst(toks[2]));
    printf("isinst(): %x\n", isinst(toks[3]));
    printf("isinst(): %x\n", isinst(toks[4]));
    printf("isinst(): %x\n", isinst(toks[5]));
    printf("isinst(): %x\n", isinst(toks[6]));
    printf("isinst(): %x\n", isinst(toks[7]));
    printf("isinst(): %x\n", isinst(toks[8]));
    printf("isinst(): %x\n", isinst(toks[9]));
    printf("isinst(): %x\n", isinst(toks[10]));
    printf("isinst(): %x\n", isinst(toks[11]));
    printf("isinst(): %x\n", isinst(toks[12]));
    */

}
