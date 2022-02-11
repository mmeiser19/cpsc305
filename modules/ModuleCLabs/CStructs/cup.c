#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cup.h"

char *contents_to_str[] = {"TEA", "COFFEE", "WATER", "MILK", "MIXED"};
char *material_to_str[] = {"PAPER", "GLASS", "PLASTIC"};

void print_cup(cup *c) {
    printf("%s %d %d %s %s\n", c->color, c->max_oz, c->cur_oz, 
           contents_to_str[c->contents], material_to_str[c->material]);
}

cup *new_cup(int max, int cur, contents contents, enum material m, char *color) {
    cup *p = malloc(sizeof(cup));
    p->max_oz = max;
    p->cur_oz = cur;
    p->contents = contents;
    p->material = m;
    strcpy(p->color, color);
    return p;
}

int fill(cup *c, contents v, int oz) {
    c->cur_oz += oz;
    if (c->contents != v)
        c->contents = MIXED;
    if (c->cur_oz > c->max_oz) {
        c->cur_oz = c->max_oz;
        return -1;
    }
    return c->cur_oz;
}

cup *find_fullest(cup **ca, int num_eles) {
    double fullest = (double) ca[0]->cur_oz / (double) ca[0]->max_oz;
    cup *fullestCup = ca[0];
    for (int i = 0; i < num_eles; i++) {
        double ratio = (double) ca[i]->cur_oz / (double) ca[i]->max_oz;
        if (ratio > fullest) {
            fullest = ratio;
            fullestCup = ca[i];
        }
        else if (ratio == fullest) {
            if (ca[i]->cur_oz > fullestCup->cur_oz) {
                fullestCup = ca[i];
            }
        }
    }
    return fullestCup;
}

//test with 3 cups in main for sorting
void sort_cups(cup **ca, int num_eles) {
    for (int i = 0; i < num_eles; i++) {
        for (int j = 0; j < num_eles - 1 - i; j++) {
            double ratio1 = (double) ca[j]->cur_oz / ca[j]->max_oz;
            double ratio2 = (double) ca[j + 1]->cur_oz / ca[j + 1]->max_oz;
            if (ratio1 > ratio2) {
                cup *temp = ca[j];
                ca[j] = ca[j + 1];
                ca[j + 1] = temp;
            }
            else if (ratio1 == ratio2) {
                if (ca[j]->cur_oz > ca[j + 1]->cur_oz) {
                    cup *temp = ca[j];
                    ca[j] = ca[j + 1];
                    ca[j + 1] = temp;
                }
            }
        }
    }
}

void print_cups(cup **ca, int num_eles) {
    for (int i = 0; i < num_eles; i++) {
        printf("cup[%d]: ", i);
        print_cup(ca[i]);
    }
}
