/**
 * util.c
 *         Hit & Blow Utility module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <ctype.h>

#include "util.h"

int create_random_number(char* number_buffer) {
    int n[4]; 
    int x;
    int i;
    for (i=0; i<4; i++) {
        n[i] = -1;
    }

    for (i=0; i<4; i++) {
        while (1) {
            x = rand() % 10;
            if ( x != n[0] && x != n[1] && x != n[2] && x != n[3] ) {
                // unique number, break
                break;
            }
        }
        n[i] = x;
    }
    // set number
    for (i=0; i<4; i++) {
       number_buffer[i] = (char) (n[i] + 0x30);
    }
    return 1000 * n[0] + 100 * n[1] + 10 * n[2] + n[3];
}

int
compare_numbers(char* num1, char* num2) {
    int hits = 0;
    int blows = 0;
    int i, j;
    
    // count hits
    for (i=0; i < 4; i++) {
        if (num1[i] == num2[i]) {
            hits++;
        }
    }
    // count blows
    for (i=0; i < 4; i++) {
        for (j=0; j<4; j++) {
            if (num1[i] == num2[j]) {
                blows++;
            }
        }
    }
    // collect blows
    blows -= hits;

    return hits * 0x1000 + blows;
}

/**
 * @returns 0:number, 1:start, 2:quit, -1:invalid
 */
int check_number(char* number) {
    int i;
    if (tolower(number[0]) == 's') {
        return START_COMMAND;
    }

    // length check
    if (strlen(number) != 4) {
        // length error.
        return INVALID_COMMAND;
    }

    // digit check
    for (i=0; i<4; i++) {
        if (isdigit(number[i]) == 0) {
            // not digit error.
            return INVALID_COMMAND;
        }
    }

    // duplicate number check
    if ( (number[0] == number[1] || number[0] == number[2] || number[0] == number[3]) ||
         (number[1] == number[2] || number[1] == number[3]) ||
         (number[2] == number[3]) ) {
        return INVALID_COMMAND;
    }

    return NUMBER_COMMAND;
}


#ifdef UNIT_TEST
void test_compare_numbers(void) {
    char num1[5];
    char num2[5];
    int i,j;
    int cmp;

    srand((unsigned)time(NULL));
    for (i=0; i<10; i++) {
        memset(num1, 0x00, 5);
        create_random_number(num1);
    
        for (j=0; j<10; j++) {
            memset(num2, 0x00, 5);
            create_random_number(num2);

            cmp = compare_numbers(num1, num2);
            printf("num1=%s, num2=%s, cmp=%d, hits=%d, blows=%d\n",
                num1, num2, cmp, cmp / 0x1000, cmp % 0x1000);
        }
        cmp = compare_numbers(num1, num1);
        printf("num1=%s, num1=%s, cmp=%d, hits=%d, blows=%d\n",
            num1, num1, cmp, GET_HITS(cmp) , GET_BLOWS(cmp));
    }
}

void test_create_random_number(void) {
    int i;
    int r;
    char buf[5];
    srand((unsigned)time(NULL));
    for (i=0; i<10;i++) {
        memset(buf, 0x00, 5);
        r = create_random_number(buf);
        printf("r=%d, buf=%s\n", r, buf);
    }
}

test_check_number(void) {
    char buf[1024];
    int cmd;

    for(;;) {
        printf(">");
        gets(buf);
        cmd = check_number(buf);

        printf("cmd=%d, buf=[%s]\n", cmd, buf);
        
    }    
}

main(int argc, char** argv) {
    // test_create_random_number();
    test_compare_numbers();
    // test_check_number();
}
#endif // UNIT_TEST
