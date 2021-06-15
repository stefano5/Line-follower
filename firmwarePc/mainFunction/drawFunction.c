/*
 * In questo file sono contenute tutte le funzione che disegnano su allegro.
*/
#ifndef DRAWFUNCTION_C
#define DRAWFUNCTION_C

#ifndef ALLEGRO_H 
    #error "<allegro.h> missed"
#endif

#ifndef HEADERFILE_H
    #error "\"headerFile.h\" missed"
#endif

/*
 * Disegniamo ellissi
*/      //  Chiamata da main quando main è unica istanza
void drawEllipsisButton() {
    coordinatesT1.xc = WIDTH/2;                 //Coordinate centro ellissi
    coordinatesT1.yc = 75;                      //^^^^^^^^^^^^^^^^^^
    coordinatesT1.rx = 70;
    coordinatesT1.ry = 50;
    coordinatesT1.xt = coordinatesT1.xc;         //Scriviamo al centro dell'ellissi
    coordinatesT1.yt = coordinatesT1.yc;
    
    ellipsefill(screen, coordinatesT1.xc, coordinatesT1.yc, coordinatesT1.rx, coordinatesT1.ry, RED);
    textout_centre_ex(screen, font, "STOPPA ROVER", coordinatesT1.xt, coordinatesT1.yt, BLACK, WHITE);
}


/*
 * Disegniamo i comandi
*/     //  Chiamata da main quando main è unica istanza
void drawHelpCommand() {
    int     xi, yi;      //coordinate rettangolo (punto iniziale alto a sx)
    int     xe, ye;      //coordinate rettangolo (punto finale basso a dx)
    int     xh, yh;      //coordinate scritta "help"
    int     xs;          //coordinata da cui iniziamo a scrivere le scritte di aiuto
    xi = (WIDTH/1.6) - 100;
    yi = 150;
    xe = (WIDTH/1.6) + 100;
    ye = 350;
    xh = WIDTH/1.6;
    yh = 160;
    xs = (WIDTH/1.6) - 75;

    rect(screen, xi, yi, xe, ye, BLUE);
    textout_centre_ex(screen, font, "HELP", xh, yh, BLACK, WHITE);
    textout_ex(screen, font, "q: quit", xs, yh + 20, BLACK, WHITE);                 //Qui abbiamo offset su y per incolonnare le scritte
    textout_ex(screen, font, "+: increase speed", xs, yh + 40, BLACK, WHITE);
    textout_ex(screen, font, "-: decrease speed", xs, yh + 60, BLACK, WHITE);
    textout_ex(screen, font, "5: stop rover", xs, yh + 80 , RED, WHITE);
    textout_ex(screen, font, "c: change modality", xs, yh + 100 , BLACK, WHITE);

    if (mode == MANUAL_MODE) {
        textout_ex(screen, font, "8: go forward", xs, yh + 120 , BLACK, WHITE);
        textout_ex(screen, font, "6: go left", xs, yh + 140 , BLACK, WHITE);
        textout_ex(screen, font, "4: go right", xs, yh + 160 , BLACK, WHITE);
        textout_ex(screen, font, "2: go backwards", xs, yh + 180 , BLACK, WHITE);
    } else {
        textout_ex(screen, font, "r: restart rover", xs, yh + 120 , BLUE, WHITE);
    }
}

/*
 * Disegna la base della GUI, ovvero sfondo e cornice
*/     //  Chiamata da main quando main è unica istanza
void draw_gui() {
    rectfill(screen, 0, 0, WIDTH, HEIGHT, WHITE);                   //crea sfondo
    rect(screen, 10, 10, WIDTH - 10, HEIGHT - 10, RED);             //crea cornice
}

/*
 * Funzione che si occupa di stampare il grafico motori aggiornato.
 * Si occupa lei stessa di aggiornare il grafico traslando l'array in cui salviamo le coordinate
*/    //  Chiamata da T3 (chiamata periodica)
void drawGraphicsMotor(int cmd, int xp, int yp) {
    int     x_motorLeft, y_motorLeft;                           //coordinate da cui iniziamo la stampa motore left
    int     x_motorRight, y_motorRight;                         //coordinate da cui iniziamo la stampa motore rigth
    x_motorLeft = xp;
    y_motorLeft = yp - 90;
    x_motorRight = xp;
    y_motorRight = y_motorLeft - 100;

    //Disegniamo riquadro motore sinistro
    pthread_mutex_lock(&mutex_screen);
    textout_ex(screen, font, "Motor left", x_motorLeft, y_motorLeft-10, BLACK, WHITE);
    textout_ex(screen, font, "F", x_motorLeft-10, y_motorLeft+8, RED, WHITE);
    textout_ex(screen, font, "S", x_motorLeft-10, y_motorLeft+20, RED, WHITE);
    textout_ex(screen, font, "B", x_motorLeft-10, y_motorLeft+36, RED, WHITE);
    rect(screen, x_motorLeft-2, y_motorLeft-2, x_motorLeft+200+2, y_motorLeft+50+2, BLUE);     //Disegniamo la cornice

    //Disegniamo riquadro motore destro
    textout_ex(screen, font, "Motor right", x_motorRight, y_motorRight-10, BLACK, WHITE);     //aggiorniamo schermata
    textout_ex(screen, font, "F", x_motorRight-10, y_motorRight+8, RED, WHITE);
    textout_ex(screen, font, "S", x_motorRight-10, y_motorRight+20, RED, WHITE);
    textout_ex(screen, font, "B", x_motorRight-10, y_motorRight+36, RED, WHITE);
    rect(screen, x_motorRight-2, y_motorRight-2, x_motorRight+200+2, y_motorRight+50+2, BLUE);                   //Disegniamo la cornice
    pthread_mutex_unlock(&mutex_screen);

    managementMotrLeft(cmd);
    scrollMatrixMotorLeft(50, 200);
    scrollMatrixMotorLeft(50, 200);
    printCurveMotorLeft(x_motorLeft, y_motorLeft);      //screen protetta 

    managementMotrRight(cmd);
    scrollMatrixMotorRight(50, 200);
    scrollMatrixMotorRight(50, 200);
    printCurveMotorRight(x_motorRight, y_motorRight);   //screen protetta
}


/*
   Riquadro traiettoria:
                            (x2, y2)
   °===========================°
   |                           |
   |                           |
   |                           |    ° coda (fine) della traiettoria
   |                           |    * testa  (inizio) della traiettoria
   |                           |
   |            °        *     |        Coordinate di * sono: (xg, yg)
   |            |       /      |        Tracciamo la traiettoria partendo dalle vecchie coordinate di * fino alle nuove
   |             \     /       |
   |              \___/        |
   |                           | 
   °===========================°
   (x1, y1)

 * Disegniamo la traiettoria. Per farlo ci servono le coordinate del riquadro (ultimi 4 parametri formali) 
 *      Inoltre ci servono le coordinate dei 4 punti per tracciare il segmento
 *      Ahimè, usiamo quindi 8 parametri formali

 * Prima di tracciare la traiettoria dobbiamo soddisfare le seguenti condizioni:
 1) consistenza delle coordinate (xg_old, yg_old). 0 è infatti il loro valore a cui sono inizializzate, e non avrebbe senso tracciare il segmento
 2) Che il segmento venga tracciato all'interno del suo riquadro

 * Inoltre, dovremo aggiornare le coordinate (xg_old, yg_old) a prescindere da queste condizioni.
 * L'implementazione di ciò è sottoriportata
 */     //Chiamata da T3 (chiamata periodica)
void drawTrajectory(int xg, int yg, int *xg_old, int *yg_old, int x1, int y1, int x2, int y2) {
    if (*xg_old != 0 && *yg_old != 0) {
        if (xg > x1 && xg < x2) { 
            if (yg > y2 && yg < y1) {
                pthread_mutex_lock(&mutex_screen);
                line(screen, *xg_old, *yg_old, xg, yg, BLACK);
                pthread_mutex_unlock(&mutex_screen);
            }
        }
    }

    if (*xg_old != xg)
        *xg_old = xg;
    if (*yg_old != yg)
        *yg_old = yg;
}


/*
 * Scriviamo in alto a destra la scritta "quit". Quando vorremo uscire dal programma lo potremo fare premendo su di essa
 */     //  Chiamata da main quando main è unica istanza
void drawQuitKey() {
    coordinatesT1.xq = WIDTH - 90;
    coordinatesT1.yq = 30;
    textout_ex(screen, font, "QUIT", coordinatesT1.xq, coordinatesT1.yq, BLACK, WHITE);
}


/*              forward
 *      left    (center)    rigth
 *              backwards
 *  
 *  Calcoliamo solo le coordinate di center, le altre le prendiamo relative a quel punto
 *  Le coordinate di center sono funzione di WIDTH e HEIGHT
 */     //  Chiamata da main quando abbiamo in esecuzione: main, T1, T2 
void drawButtonDirection() {
    int     xc, yc;         //coordinate centro
    int     xf, yf;         //coordinate tasto "forward"
    int     xr, yr;         //coordinate tasto "rigth"
    int     xl, yl;         //coordinate tasto "left"
    int     xb, yb;         //coordinate tasto "backwards"

    coordinatesT2.xc = xc = (int)(WIDTH/1.2);           //Le scriviamo sulla struttura per poter gestire i click nel thread T1

    coordinatesT2.yc = yc = (int)(HEIGHT/2.5);          //Usiamo inoltre delle variabili locali per praticità

    coordinatesT2.xf = xf = xc;
    coordinatesT2.yf = yf = yc - 50;

    coordinatesT2.xb = xb = xc;
    coordinatesT2.yb = yb = yc + 50;

    coordinatesT2.xl = xl = xc - 50;
    coordinatesT2.yl = yl = yc;

    coordinatesT2.xr = xr = xc + 50 + 30;      //Ulteriore offset di 30 per allineare meglio il bottore
    coordinatesT2.yr = yr = yc;

    pthread_mutex_lock(&mutex_screen);
    rectfill(screen, xl-15, yf-15, xr+15, yb+15, WHITE);
    rect(screen, xl-10, yf-10, xr+10+35, yb+10, BLUE);

    textout_ex(screen, font, "Forward", xf, yf, BLACK, WHITE);
    textout_ex(screen, font, "Backwards", xb, yb, BLACK, WHITE);
    textout_ex(screen, font, "Left", xl, yl, BLACK, WHITE);
    textout_ex(screen, font, "Right", xr, yr, BLACK, WHITE);
    pthread_mutex_unlock(&mutex_screen);
}


/*
 * Scriviamo l'unica cosa statica dell'indicatore. Il resto delle cose verranno disegnate dal relativo thread di gestione dell'indicatore
 */     //Chiamata da main quando è l'unica istanza
void drawIndicator() {
    int     x, y;
    x = (WIDTH/1.5);
    y = (HEIGHT - 120);

    textout_centre_ex(screen, font, "Power motor", x, y, BLACK, WHITE);
    oldMotorPower = -1;
}


/*
 * Scrivi la modalità in cui siamo

 * Non serve mutex su mode, la variabile è protetta dalla funzione che chiama questa
 */     //Chiamata da main quando abbiamo in esecuzione: main, T1, T2
void writeModality(int mode) {
    int     x, y;           //coordinate scritta
    x = (int)(WIDTH/1.3);
    y = (HEIGHT)/8;

    if (mode == AUTOMATIC_MODE) {
        pthread_mutex_lock(&mutex_mode);
        textout_centre_ex(screen, font, "Automatic mode", x, y, BLACK, WHITE);
        pthread_mutex_unlock(&mutex_mode);
    } else {
        pthread_mutex_lock(&mutex_mode);
        textout_centre_ex(screen, font, "Manual mode", x, y, BLACK, WHITE);
        pthread_mutex_unlock(&mutex_mode);
    }
}

#endif //DRAWFUNCTION_C
