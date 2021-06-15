#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>
#include <math.h>
#define  _REENTRANT
#include <pthread.h>
#include <signal.h>

#include "serial/serial.c"
#include "printColor/printColor.c"
#include "task/task.c"
#include "mainFunction/headerFile.h"
#include "mainFunction/operationMatrix.c"
#include "mainFunction/drawFunction.c"
#include "mainFunction/managementAndReactionEvent.c"


void init_all() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(8);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIDTH, HEIGHT, 0, 0); 
    enable_hardware_cursor();
    show_mouse(screen);
    motorPower = 7;
    run = TRUE;
}

//**************************************************************************************************************
//                  Task
//**************************************************************************************************************

/*
 * Gestione comandi dati dall'utente
*/  //T2
void *readUserCommand(void *value) {
    char            key;           //Salviamo qui il carattere inserito dall'utente
    struct task_par *tp;
    tp = (struct task_par*)value;
    set_period(tp);

    while (1) {
        key = readkey() & 0xFF;
        analyzeKey(key);

        if (deadline_miss(tp)) printf("!");     //soft real time
        wait_for_period(tp);
    }
}

/*
 * Gestione di:
 Tasto ellittico
 Tasto uscita
 Indicatore potenza motore

 È il thread principale, nel senso che è l'unico thead su cui viene chiamata la pthread_join. Nel while sono infatti presenti delle condizioni di uscita
 */  //T1
void *managementT1(void *param) {
    int             mutex = FALSE;
    struct task_par *tp;
    int             closeSw_temp;

    tp = (struct task_par*)param;
    tp->arg = TRUE;                                         //Sblocchiamo main

    set_period(tp);
    while (1) {
        if (mouse_b & 1) {
            if (mutex == FALSE) {
                mutex = TRUE;
                if (pressedQuitKey(mouse_x, mouse_y)) {
                    sendToSerial(fd, CMD_STOP);             //Se stiamo chiudendo il programma stoppiamo prima il rover
                    run = FALSE;                            //Facciamo poi terminare il loop del main
                    break;
                } else {
                    managementEvent(mouse_x, mouse_y);
                }

            } else mutex = FALSE;
        }

        pthread_mutex_lock(&mutex_closeSw);
        closeSw_temp = closeSw;
        pthread_mutex_unlock(&mutex_closeSw);

        if (closeSw_temp == SHUTDOWN) {                 //Questa variabile è messa a vero dal thread che legge i comandi dati dall'utente
            sendToSerial(fd, CMD_STOP);                 //Se siamo qui e' quindi perchè l'utente ah premuto il tasto 'q' 
            run = FALSE;
            break;
        }

        if (closeSw_temp == REBOOT) {
            sendToSerial(fd, CHANGEMODALITY);
            break;
        }

        managementIndicator();

        if (deadline_miss(tp)) printf("!");     //soft real time
        wait_for_period(tp);
    }
    pthread_exit(NULL);
}


/*
   Riquadro traiettoria:
   (xp, yp)                    (x2, y2)
   °===========================°
   |                           |
   |                           |
   |                           |
   |                           |
   |                           |
   |            ° <==========  | ====== (xcmain, ycmain)
   |                           |
   |                           |
   |                           |
   |                           |
   |                           | 
   °===========================°
   (x1, y1)

   Questo thread disegna la traiettoria del rover e grafica l'azione dei motori
   */     //T3
void *construeGraphics(void *param) {
    struct task_par     *tp;
    int                 command;                        //Comando ricevuto dalla seriale, potrà contenere solo comandi di direzione
    int                 xcmain, ycmain;                 //x_center_main, y_center_main. Centro da cui discegniamo il riquadro delle traiettorie
    int                 x1, y1, x2, y2, xp, yp;         //      xp: x_print  Da questa coordinata iniziamo la stampa della traiettoria
    double              theta;                           //Le funzioni cos e sin richiedono un double e ritornano un double
    int                 oldCommand;                     //Aggiorniamo la traiettoria quando cmabiamo direzione 
    float               x, y;
    int                 xg, yg;
    int                 xg_old, yg_old;
    tp = (struct task_par*)param;
    initAllVariable_construeGraphics(&xcmain, &ycmain, &x1, &y1, &x2, &y2, &xp, &yp);
    x=y=xg_old=yg_old=0;                                //Queste variabili le inizializziamo qui per non aumentare i parametri della funzione sopra, che ha già fin troppi parametri
    initStructure_motor(50, 200);

    rect(screen, x1-5, y1+5, x2+5, y2-5, BLUE);         //Disegniamo cornice per grafico motori 
    rectfill(screen,x1,y1,x2,y2, WHITE);
    rect(screen, x1-5, y1+5, x2+5, y2-5, BLUE);
    rectfill(screen,x1,y1,x2,y2, WHITE);

    oldCommand = CMD_FORWARD_INT;                       //Cosi supponiamo che il rover parta con una direzione ortogonale al monitor del pc e che vada oltre lo schermo piuttosto che verso l'utente
    theta = 90.0;
    getCommandFromSerial(fd);                           //Svuotiamo la coda seriale

    set_period(tp);

    while (1) {
        command = getClearCommand(getCommandFromSerial(fd));        //bloccante, il tempo lo scandisce arduino

        if (command != USEFULL_COMMAND) {
            drawGraphicsMotor(command, xp, yp);
            if (command != CMD_STOP_INT) {
                getAngle(&theta, &oldCommand, command);
                calculateCoordinates(theta, &x, &y);

                xg = (int)x + xcmain;                   //xc_main è già un int
                yg = (int)y + ycmain;

                drawTrajectory(xg, yg, &xg_old, &yg_old, x1, y1, x2, y2);
            }
        } 

        if (deadline_miss(tp))  printf("!");    //Soft realtime
        wait_for_period(tp);
    }
}

//**************************************************************************************************************
//                  Funzione per start thread
//**************************************************************************************************************

/*
 * Facciamo partire un nuovo thread
 *  Il primo parametro è la funzione
 *  Il secondo è la struttura da riepire (dedicata per il thread)
 *  Il terzo è la priorità che vogliamo dare al thread
 */
void startThread(void*(*func)(void*), struct task_par *tp, int priority) {
    int                 res;
    struct sched_param  mypar;
    pthread_attr_t      att;
    tp->period = 100;                           //Gli altri parametri, oltre la priorità, li lasciamo di default
    tp->deadline = 80;
    tp->priority = priority;
    tp->dmiss = 0;

    pthread_attr_init(&att);
    pthread_attr_setinheritsched(&att, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&att, SCHED_FIFO);
    mypar.sched_priority = tp->priority;
    pthread_attr_setschedparam(&att, &mypar);

    res = pthread_create(&tid[n_tid++], &att, func, (void*)tp);
    if (res != 0) {
        n_tid--;
        printf("[FATAL ERROR] Thread %d not running. Do you have the permissions?\n", n_tid);
        allegro_message("[FATAL ERROR] You must be run me whit root privileges");
    }
}

//**************************************************************************************************************
//                  Funzioni di inizializzazione e lancio thread
//**************************************************************************************************************

/*
 * Questa funzione lancia due thread. 
 *  Il primo si occupa della gestione del tasto di emergenza, del tasto di uscita, l'indicatore della potenza motore e, se siamo in mod. manuale, controlla i bottoni di direzione
 *  Il secondo si occupa di gestire i comandi dati dall'utente
 */

void primary_thread() {

    // Disegniamo il resto della pagina
    drawQuitKey();
    drawEllipsisButton(); 
    drawHelpCommand();
    drawIndicator();
    
    // Adesso facciamo partire i due thread
    tp1.arg = FALSE;
    startThread(managementT1, &tp1, 60);
    while (!tp1.arg) ;                                 //Vogliamo avere la certezza che i thread meno importanti partano dopo la partenza del thread più importante

    startThread(readUserCommand, &tp2, 40);
}

/*
 * Questa funzione lancia un thread
 *  Il thread disegna i grafici dei motori e la traiettoria
 */
void secondary_thread() {
    int     mode_temp;

    pthread_mutex_lock(&mutex_mode);
    mode_temp = mode;
    pthread_mutex_unlock(&mutex_mode);

    writeModality(mode_temp);

    if (mode_temp == MANUAL_MODE) {
        drawButtonDirection();
    }
    startThread(construeGraphics, &tp3, 90);
}



/*
 * Questa è la funzione che fa partire il tutto. 
 * La prima cosa che fa è stoppare i precedenti thread avviati, se ce ne sono. Dopo questo setta la variabile globale 'closeSw'. Quando lo fa è l'unica istanza ad essere in esecuzione
 * Poi cancella l'intera schermata sulla GUI e la ricrea.
 * Infine fa ripartire i task
 */
void createScreen() { 

    stopAllThread();
    closeSw = 0;

    clear_to_color(screen, WHITE);
    draw_gui(); 

    primary_thread(); 
    secondary_thread(mode);
}

/*
 * Scegliamo la modalità alla partenza
 * */
void selectMode(int mode) {
    if (mode == AUTOMATIC_MODE) {
        mode = AUTOMATIC_MODE;
        sendToSerial(fd, AUTOMATIC);
    } else {
        mode = MANUAL_MODE;
        sendToSerial(fd, MANUAL);
    }
}

int main(int argc, char **argv) {
    textColor(PRINT_BRIGHT, PRINT_RED, PRINT_BLACK);        //settiamo in rosso le stampe da ora in avanti
    init_all();
    signal(SIGINT, signalRoutine);                          //Gestiamo il caso di uscita forzata da shell

    fd = init_serial(argc == 2 ? argv[1] : SERIAL_PORT);

    if (fd != -1) {
        selectMode(MANUAL_MODE);
        //selectMode(AUTOMATIC_MODE);                       //Si può scegliere indifferentemente la modalità da cui partire
        
        while (run) {                                       //Questa variabile viene modificata dalla routine di uscita
            createScreen();
            pthread_join(tid[0], NULL);                     //Aspettiamo che il thread 0, relativo al bottone 'quit', venga arrestato
        }

        stopAllThread();

        close_serial(fd);
        clear_to_color(screen, 0);
    }

    allegro_exit();

    textColor(PRINT_RESET, PRINT_WHITE, PRINT_BLACK);       //Resettiamo i colori delle stampe sulla console
    exit(EXIT_SUCCESS);
}
