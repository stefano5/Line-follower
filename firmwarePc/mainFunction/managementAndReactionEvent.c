/*
 * Raggruppiamo in questo file le funzioni di gestione thread e di reazione agli eventi
 * Le funzioni scritte per i task sono quindi qui sotto definite
*/
#ifndef MANAGEMENTANDREACTIONEVENT_H
#define MANAGEMENTANDREACTIONEVENT_H

/*
 * Analizziamo il comando e, se noto, eseguiamo il comando.
 * L'espressione all'interno dello switch serve a rendere non case sensitive i comandi
        (key > 64 && key < 91 ? key + 32 : key)
 * Sapendo infatti che i caratteri ascii compresi tra 65 e 90 sono le lettere maiuscole e sapendo che la differenza tra le lettere maiuscole e minuscole è di 32 possiamo risalire alle lettere minuscole da quelle maiuscole con una 
 *      semplice addizione. Se invece non stiamo inserendo una lettera maiuscola, ma o una lettera già in minuscolo o un numero  o i tasti '+' e '-' allora non facciamo nessuna somma

 * Mutex: Anche se la funzione risulta essere lunga comunque computazionalmente non ha un gran peso. Considerando quindi che la variabile 'mode' deve essere sia letta che scritta proteggiamo l'intero switch.
 * Inoltre, onde eviare di inserire sezioni critiche dentro altre sezioni critiche
 *
   Usiamo la variabile locale changeVariableCloseSw per aggiornare, se è stato richiesto, la variabile globale closeSw
*/      //chiamata da T2
void analyzeKey(char key) {
    int     changeVariableCloseSw = 0;
    pthread_mutex_lock(&mutex_mode);
    switch(key > 64 && key < 91 ? key + 32 : key) {
        case 'q':
            changeVariableCloseSw = SHUTDOWN;
            break;
        case '5':
            sendToSerial(fd, CMD_STOP);
            if (mode == AUTOMATIC_MODE) mode = STOPPED_MODE;
            break;
        case '+':
            if (mode != STOPPED_MODE) {
                sendToSerial(fd, CMD_MOTORUP);
                pthread_mutex_lock(&mutex_motorPower);
                motorPower += 1;
                if (motorPower > 13) motorPower = 13;              //Con 13 siamo a piena potenza
                pthread_mutex_unlock(&mutex_motorPower);
            } else allegro_message("Rover must be in motion"); 
            break;
        case '-':
            sendToSerial(fd, CMD_MOTORDOWN);
            if (mode != STOPPED_MODE) {
                pthread_mutex_lock(&mutex_motorPower);
                motorPower -= 1;
                if (motorPower < 5) motorPower = 5;                //non permettiamo alla potenza dei motori di raggiungere valori bassi perchè altrimenti il rover non riesce a camminare
                pthread_mutex_unlock(&mutex_motorPower);
            } else allegro_message("Rover must be in motion"); 
            break;
        case '8':
            if (mode == MANUAL_MODE) {
                sendToSerial(fd, CMD_FORWARD);
            }
            break;
        case '4':
            if (mode == MANUAL_MODE) {
                sendToSerial(fd, CMD_LEFT);
            }
            break;
        case '6':
            if (mode == MANUAL_MODE) {
                sendToSerial(fd, CMD_RIGHT);
            }
            break;
        case '2':
            if (mode == MANUAL_MODE) {
                sendToSerial(fd, CMD_BACKWARDS);
            }
            break;
        case 'r':
            if (mode == STOPPED_MODE) {
                sendToSerial(fd, CMD_RESTART);
                mode = AUTOMATIC_MODE;
            }
            break;
        case 'c':
            if (mode == STOPPED_MODE) mode = AUTOMATIC_MODE;
            mode = (mode == AUTOMATIC_MODE) ? MANUAL_MODE : AUTOMATIC_MODE;
            changeVariableCloseSw = REBOOT;
            break;
            //Qui il default non lo mettiamo, se premiamo un tasto a cui non corrisponde nessun comando non vogliamo far nulla
    }
    pthread_mutex_unlock(&mutex_mode);

    if (changeVariableCloseSw != 0) {
        pthread_mutex_lock(&mutex_closeSw);
        closeSw = changeVariableCloseSw;
        pthread_mutex_unlock(&mutex_closeSw);
    }
}



/* 
 * Con questo thread vogliamo creare un indicatore della velocità del tipo:

 Motor Power
 [#] [#] [#] [#] [#] [#] [#] [] [] [] [] [] [] 
 ^                                          ^
 motorPower=0                               motorPower=13
 pwm=0                                      pmw=255        pwm è il valore che arriverà alla analogWrite nel firmware del controllo motori


 * Vogliamo:        3 colori (blu = bassa potenza, grigio = media potenza, rosso = alta potenza)
 *                  I rettangoli devo essere visualizzati in base al valore attuale di motorPower
 * 
 */     //Chiamato da T1 
void managementIndicator() {
    int     xc, yc;                 //Coordinate da cui partiamo a disegnare i rettangoli
    int     xe, ye;                 //Coordinate da cui 
    int     i;
    int     motorPower_temp;

    pthread_mutex_lock(&mutex_motorPower);
    motorPower_temp = motorPower;
    pthread_mutex_unlock(&mutex_motorPower);


    if (oldMotorPower != motorPower_temp) {
        oldMotorPower = motorPower_temp;
        xc = (WIDTH/1.5) - 100;
        yc = (HEIGHT - 100);
        ye = yc + 20;

        pthread_mutex_lock(&mutex_screen);
        for (i=0; i<motorPower_temp; i++) { //disegniamo i rettangoli. Ognuno indica un livello. 
            xe = xc + 10;
            if (i < 5) {
                rectfill(screen, xc, yc, xe, ye, L_BLUE);
            } else if (i < 9) {
                rectfill(screen, xc, yc, xe, ye, GRAY);
            } else {
                rectfill(screen, xc, yc, xe, ye, RED);
            }
            xc += 10 + 5;              //5 è lo spazio tra un rettangolo e l'altro
        }

        for (i = motorPower_temp; i<13; i++) { //sappiamo che abbiamo 13 livelli, quindi se motorPower<13 copriamo i precedenti rettangoli disegnati (se succede è perchè stiamo abbassando la potenza del motore)
            xe = xc + 10;
            rectfill(screen, xc, yc, xe, ye, WHITE);
            xc += 10 + 5;               //5 è lo spazio tra un rettangolo e l'altro
        }
        pthread_mutex_unlock(&mutex_screen);
    }
}


/* Il goal è: quando premo il tasto rosso (di forma ellittica) stoppare il rover
 * Come discriminiamo i click fatti sull'ellissi dagli altri? Usiamo la matematica
 *
 * L'equazione dell'ellissi di nostro interesse e' stata studiata con Geogebra ed è
 *  40000.13 x^2 + 77407.97 y^2 - 40970044.68 x + 11641195.8 y = -10732769501.36
 * La matematica ci dice che i valori (x,y) che soddisfano la disequazione:
 *                   40000.13 x^2 + 77407.97 y^2 - 40970044.68 x + 11641195.8 y < -10732769501.36
 * appartengono all'ellissi. Possiamo così discriminare i click fatti all'interno dal tasto di forma ellittica dagli altri
 */     //Chiamata da T1
int pressedEllipsis(int mouse_x, int mouse_y) {
    float x2 = 40000.13;                //coefficiente di x^2
    float y2 = 77407.97;                //coefficiente di y^2
    float x  = -40970044.68;            //coefficiente di x
    float y  = 11611195.8;              //coefficiente di y
    float c  = -10732769501.36;         //termine noto
    float leftSide = 0.0;               //somma del lato sinistro dell'equazione. La condizione che l'eq deve soddisfare è:     leftSide < c

    leftSide = (x2 * pow(mouse_x, 2)) + (y2 * pow(mouse_y, 2)) + (x * mouse_x) + (y * (-mouse_y));        //Il meno su mouse_y è dovuto al fatto che in questo caso l'asse y positivo è verso il basso
    //                                                                                ^^^ Questo meno è dovuto al fatto che in informatica l'asse y è ribaltato
    return leftSide < c;
}


/*
 * ritorna vero se le coordinate appartengono al rettangolo sotto descritto, altrimenti falso
 *
 *    (coordinatesT1.xc, coordinatesT1.yx)                   (coordinatesT1.xc + 50, coordinatesT1.yx) 
 *          °=====================================================================°
 *          |                                                                     |
 *          |                                                                     |
 *          |       QUIT                                                          |
 *          |                                                                     |
 *          |                                                                     |
 *          °=====================================================================°
 *    (coordinatesT1.xc, coordinatesT1.yx + 30)                 (coordinatesT1.xc + 50, coordinatesT1.yx + 30) 
 *
 */
int pressedQuitKey(int x, int y) {
    return x > coordinatesT1.xq && x < coordinatesT1.xq + 50 && y > coordinatesT1.yq && y < coordinatesT1.yq + 20;
}

/*
 * L'idea è la stessa del rettangolo sopra
 */     //Chiamata da T1
int pressedForwardKey(int x, int y) {
    return x > coordinatesT2.xf && x < coordinatesT2.xf + 50 && y > coordinatesT2.yf && y < coordinatesT2.yf + 20;
}

/*
 * L'idea è la stessa del rettangolo sopra
 */     //Chiamata da T1
int pressedLeftKey(int x, int y) {
    return x > coordinatesT2.xl && x < coordinatesT2.xl + 50 && y > coordinatesT2.yl && y < coordinatesT2.yl + 20;
}

/*
 * L'idea è la stessa del rettangolo sopra
 */     //Chiamata da T1
int pressedRightKey(int x, int y) {
    return x > coordinatesT2.xr && x < coordinatesT2.xr + 50 && y > coordinatesT2.yr && y < coordinatesT2.yr + 20;
}


/*
 * L'idea è la stessa del rettangolo sopra
 */     //Chiamata da T1
int pressedBackwardsKey(int x, int y) {
    return x > coordinatesT2.xb && x < coordinatesT2.xb + 50 && y > coordinatesT2.yb && y < coordinatesT2.yb + 20;
}


/*
 * Routine di gestione di alcuni eventi fatti dall'utente sull'interfaccia grafica
 *  Gli eventi sono:
 Pressione tasto ellittico
 pressione tasto "avanti", "indietro" ecc
 * Il tasto "quit" è gestito direttamente dal task
 */     //Chiamata da T1
void managementEvent(int mx, int my) {
    if (pressedEllipsis(mx, my)) {
        sendToSerial(fd, CMD_STOP);

    } else if (pressedForwardKey(mx, my)) {
        pthread_mutex_lock(&mutex_mode);
        if (mode == MANUAL_MODE) {
            sendToSerial(fd, CMD_FORWARD);
        }
        pthread_mutex_unlock(&mutex_mode);

    } else if (pressedLeftKey(mx, my)) {
        pthread_mutex_lock(&mutex_mode);
        if (mode == MANUAL_MODE) {
            sendToSerial(fd, CMD_LEFT);
        }
        pthread_mutex_unlock(&mutex_mode);

    } else if (pressedRightKey(mx, my)) {
        pthread_mutex_lock(&mutex_mode);
        if (mode == MANUAL_MODE) {
            sendToSerial(fd, CMD_RIGHT);
        }
        pthread_mutex_unlock(&mutex_mode);

    } else if (pressedBackwardsKey(mx, my)) {
        pthread_mutex_lock(&mutex_mode);
        if (mode == MANUAL_MODE) {
            sendToSerial(fd, CMD_BACKWARDS);
        }
        pthread_mutex_unlock(&mutex_mode);

    }
}

     //Chiamata da main e routine signal
void stopAllThread() {
    int     i;
    textColor(PRINT_BRIGHT, PRINT_BLUE, PRINT_BLACK);   //trasformiamo le scritte sull'output in blu
    for (i=n_tid-1; i>=0; i--) {                          //Andiamo al contrario, evitiamo cosi di sbloccare la join nel main fino all'ultimo
        pthread_cancel(tid[i]);
        printf("Waiting tid n°: %d/%d..", i, n_tid);
        pthread_join(tid[i], NULL);
        printf("!\n");
    }
    printf("\n");
    n_tid = 0;
    textColor(PRINT_RESET, PRINT_WHITE, PRINT_BLACK);   //ripristiniamo le scritte bianche
}

/*
 * Vogliamo gestire l'uscita forzata, chiamata quindi quando da shell si preme ctrl + c
 */
void signalRoutine(int sig) {
    sendToSerial(fd, CMD_STOP);
    textColor(PRINT_BRIGHT, PRINT_BLUE, PRINT_BLACK);   //trasformiamo le scritte sull'output in blu
    printf("\nSIGINT DETECTED. Closed \n");
    stopAllThread();
    allegro_exit();
    textColor(PRINT_RESET, PRINT_WHITE, PRINT_BLACK);   //ripristiniamo le scritte bianche
    exit(EXIT_SUCCESS);                                 //Forziamo lla chiusura. Se premiamo ctrl+c è perchè siamo in debug e magari c'è stato un problema per cui vogliamo proprio forzare la chiusura
}//                                                     Questa è l'unica eccezione, a parte questa si ha sempre e solo un'unica "via" di uscita



/*
 * Vogliamo una funzione che dato in ingresso un carattere ritorna solo e soltanto i comandi noti. 
 * Dalla seriale, in fase di sviluppo, possono arrivare anche stringhe ad uso di debug. Quindi se arriva un carattere che non riconosciamo ritorniamo semplicemente il numero -1
 * 
 * La funzione poteva essere scritta in modo più compatta sapendo che la differenza di caratteri aschii tra i numeri e le lettere è fissa a 48, ma così il codice risulta essere più leggibile
 * Potevamo infatti, ad esempio, scrivere semplicemente:

 int getClearCommand(char buf) {
 return (buf - 48) < 10 ? (buf - 48) : -1;
 }

 * Ma, in questo caso, non avremmo filtrato i comandi '1' e '7' che sono i comandi di modifica della velocità. Comando che non interessano a chi usa questa funzione

*/      //chiamata da T3
int getClearCommand(char buf) {
    switch (buf) {
        case '8':   return CMD_FORWARD_INT;
        case '2':   return CMD_BACKWARDS_INT;
        case '4':   return CMD_LEFT_INT;
        case '6':   return CMD_RIGHT_INT;
        case '5':   return CMD_STOP_INT;
        default:
                    return USEFULL_COMMAND;     //Attenzione
                    //Chi chiama questa funzione farà un controllo del tipo:
                    //      if (getClearCommand(.) != USEFULL_COMMAND) {  <= Ecco perchè la costante che ritorna ha questo nome
                    //          //action
                    //      }
    }
}

/*    per DISEGNO TRAIETTORIA
 * Aggiorniamo l'angolo in base al comando attuale e al precedente comando
 * Ci interessa il precedente comando perchè, se stiamo andando indietro, e poi decidiamo di girare, il rover invertirà la sua direzione di marcia andando avanti
 */     //chiamata da T3

void getAngle(double *theta, int *oldCommand, int command) {
    if (*oldCommand != command) {
        if (command == CMD_BACKWARDS_INT) {
            *theta += 180.0;
        } else {
            if (*oldCommand == CMD_BACKWARDS_INT) {
                if (command == CMD_FORWARD_INT) {
                    *theta += 180.0;
                } else if (command == CMD_LEFT_INT || command == CMD_RIGHT_INT) {
                    *theta += 180.0;
                }
            }
        }
        *oldCommand = command;
    }

    if (command == CMD_RIGHT_INT)       *theta -= 5.0;
    else if (command == CMD_LEFT_INT)   *theta += 5.0;

    *theta = (double) ((int)*theta % 361);    //Settiamo il valore di theta affinchè sia       0 < theta < 360
    //                ^^^^^^^^^^            1° operazione, adesso theta è int
    //                ^^^^^^^^^^^^^^^^      2° operazione, adesso theta è sicuramente compreso tra 0 e 360
    //      ^^^^^^^^^^^^^^^^^^^^^^^^^^      3° operazione, adesso theta è di nuovo un double
    //Nell'operazione non si è persa nessuna informazione, theta è dichiarata double solo perchè le funzioni cos() e sin() lo richiedono
}

/*     per  DISEGNO TRAIETTORIA
 * Inizializzazione variabili task 'construeGraphics'
 */     //chiamata da T3
void initAllVariable_construeGraphics(int *xcmain, int *ycmain, int *x1, int *y1, int *x2, int *y2, int *xp, int *yp) {
    //Variabili coordinate centro del riquadro in cui disegniamo la traiettoria
    *xcmain = (int)WIDTH/3.5;
    *ycmain = (int)HEIGHT/1.5;

    //Variabili coordinate angolo basso a sinistra e alto a destra riquadro traiettoria (vedi figura sotto)
    *x1 = *xcmain - 150;
    *y1 = *ycmain + 150;
    *x2 = *xcmain + 150;
    *y2 = *ycmain - 150;

    //Variabili coordinate angolo alto a sinistra. 
    *xp = *x1;
    *yp = *y2;
}

/*    per  DISEGNO TRAIETTORIA
 * Calcoliamo le coordinate del punto verso cui abbiamo mandato il rover

 * Con i cast espliciti su vx e vy facciamo in modo di evitare cast impliciti da double a float
*/     //chiamata da T3
void calculateCoordinates(double theta, float *x, float *y) {
    float   vx, vy;
    int     speed;

    pthread_mutex_lock(&mutex_motorPower);
    speed = motorPower - 2;
    pthread_mutex_unlock(&mutex_motorPower);

    vx = (float) (speed * (float) cos (PI/180.0 * theta));
    vy = (float) (speed * (float) sin (PI/180.0 * theta)) * -1;
    *x = *x + vx * 0.1; 
    *y = *y + vy * 0.1;
}
#endif
