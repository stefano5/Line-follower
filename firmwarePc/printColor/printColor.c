#include <stdio.h>

#ifndef PRINTF_COLOR
#define PRINTF_COLOR

#define PRINT_RESET       0
#define PRINT_BRIGHT      1
#define PRINT_DIM         2
#define PRINT_UNDERLINE   3
#define PRINT_BLINK       4
#define PRINT_REVERSE     7
#define PRINT_HIDDEN      8

#define PRINT_BLACK       0
#define PRINT_RED         1
#define PRINT_GREEN       2
#define PRINT_YELLOW      3
#define PRINT_BLUE        4
#define PRINT_MAGENTA     5
#define PRINT_CYAN        6
#define PRINT_WHITE       7

void textColor(int attr, int fg, int bg) {
    char    command[13];
    sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
    printf("%s", command);
}

/* Esempio di utilizzo:
int main(void){   
    textcolor(BRIGHT, RED, BLACK);  
    printf("Questo testo sara' colorato di rosso\n");
    textcolor(RESET, WHITE, BLACK); 
    return 0;
}
*/


#endif
