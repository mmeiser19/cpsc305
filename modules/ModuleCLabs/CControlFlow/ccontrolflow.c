#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main() {
    double a[1024], *p = a;
    int aelems = 0;
    while (scanf("%lf", p++) != EOF) // fill a with values from stdin
        aelems++; // count elements in a
    // At this point a is filled with aelems
    printf("Original array:\n");
    for (int i = 0; i < aelems; i++)
        printf("a[%d]: %8.2lf\n", i, a[i]);
    printf("\n");

    /* 1.
    Place your code described in 1 after this comment
    */
    // Sorting
    for (int i = 0; i < aelems; i++) {
        for (int j = 0; j < (aelems - i - 1); j++) {
            if (a[j] > a[j + 1]) {
                double temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }

    printf("Sorted array:\n");
    for (int i = 0; i < aelems; i++) {
        printf("a[%d]: %8.2lf\n", i, a[i]);
    }
    printf("\n");

    double smallest = a[0];
    printf("Smallest number: %0.2f\n", smallest);

    double largest = a[aelems - 1];
    printf("Largest number: %0.2f\n", largest);

    double sum = 0;
    for (int i = 0; i < aelems; i++) {
        double num = a[i];
        sum = sum + num;
    }
    double average = sum / aelems;
    printf("Average: %0.2f\n", average);
    printf("\n");

    int pairs;
    int pairsOf5;
    for (int i = 0; i < aelems; i++) {
        for (int j = i + 1; j < aelems; j++) {
            pairs++;
            if (a[j] - a[i] <= 5) { // If the difference between the pairs is <= 5, then they're a pair
                // within 5 of each other
                pairsOf5++;
            }
        }
    }
    printf("Total pairs: %d\n", pairs);
    printf("Pairs within 5: %d\n", pairsOf5);
    printf("\n");

    /*
    Data structures used by 2, 3, and 4.
    */
    char *cp = malloc(sizeof("C programming is fun!"));
    strcpy(cp, "C programming is fun!");
    char ca[] = "C programming is fun!";

    /* 2a.
    What does While Loop 1 compute?
    Explain While Loop 1.
    */
    int c1 = 0;
    while (ca[c1] != 0)
        c1++;
    printf("while loop 1: %d\n", c1);

    /* 2b.
    What does While Loop 2 compute?
    Explain While Loop 2.
    */
    // While Loop 2 iterates through the array ca, and the pointer points to the index marked by c2. As the
    // program iterates through the array, the pointer references the next char in the array, until it reaches
    // the null pointer at the end of the char array, which is when it terminates
    int c2 = 0;
    char *cap = ca;
    while (*cap++ !=0)
        c2++;
    printf("while loop 2: %d\n", c2);

    /* 3a.
    What does For Loop 1 compute?
    Explain For Loop 1.
    */
    for (c1 = 0; ca[c1] != 0; c1++);
    printf("for loop 1: %d\n", c1);

    /* 3b.
    What does For Loop 2 compute?
    Explain For Loop 2.
    */
    c2 = 0;
    for (cap = ca; *cap != 0; cap++)
        c2++;
    printf("for loop 2: %d\n", c2);

    /* 4. Analyze and explain the following code
    4a. What does the while loop and the immediately following if statement compute?
     // it computes a -1 due to y being one less ASCII value than z
    4b. What causes the while loop to terminate?
     // the break statement in the if statement causes the loop to terminate
    4c. Explain how the while loop and the immediately following if statement work.
    4c. What is goto_loop:?
     // goto_loop goes back to the while loop that occurred before the if statements were originally examined
    4d. Explain how the code after goto_loop: is executed 3 times.
     // goto_loop is executed 3 times since it is executed the first time when it is the next segment of code
     // that the program runs into, then is executed 2 more times when goto_loop is called again after
     // changing the char arrays g1 and g2
    */
    char *g1 = "Gusty", g2[] = "Gustz", g;
    goto_loop:
    g = 0;
    while (g1[g]) {
        if (g1[g] != g2[g])
            break;
        g++;
    }
    if (g1[g] - g2[g] < 0)
        printf("%s, %s: Negative\n", g1, g2);
    else if (g1[g] - g2[g] == 0)
        printf("%s, %s: Zero\n", g1, g2);
    else
        printf("%s, %s: Postive\n", g1, g2);
    if (g2[4] == 'z') {
        g2[4] = 'y';
        goto goto_loop;
    }
    else if (g2[4] == 'y') {
        g2[4] = 'a';
        goto goto_loop;
    }
}
