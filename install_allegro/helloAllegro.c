#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>


//*******************************************************************************************
//          Costanti per allegro
//*******************************************************************************************
//NOTA BENE: le dimensioni della finestra devono essere SEMPRE multipli di 2!!
#define WIDTH               1024
#define HEIGHT              600
#define CENTER_WIDTH        WIDTH/2
#define CENTER_HEIGHT       HEIGHT/2

#define BLACK               0
#define BLUE                1
#define L_BLUE              9
#define GREEN               2
#define L_GREEN             10
#define RED                 4 
#define L_RED               12
#define YELLOW              14 
#define WHITE               15
#define GRAY                8
#define L_GRAY              4 

void init_all() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(8);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIDTH, HEIGHT, 0, 0); 
    enable_hardware_cursor();
    show_mouse(screen);
    clear_to_color(screen, WHITE);
}

int main(void) {
    init_all();
    textout_centre_ex(screen, font, "Hello world", CENTER_WIDTH, CENTER_HEIGHT, BLACK, WHITE);
    sleep(5);
    return 0;
}
