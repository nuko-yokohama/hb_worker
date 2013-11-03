#ifndef __UTIL_H__
#define __UTIL_H__

#define NUMBER_COMMAND  0
#define START_COMMAND   1
#define QUIT_COMMAND    2
#define INVALID_COMMAND 3

#define GET_HITS(n)	n / 0x1000
#define GET_BLOWS(n)	n % 0x1000

int compare_numbers(char* num1, char* num2) ;
int create_random_number(char* number_buffer) ;
int check_number(char* number);

#endif
