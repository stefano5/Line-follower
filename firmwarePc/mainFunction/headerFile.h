#ifndef HEADERFILE_H
#define HEADERFILE_H

//*******************************************************************************************
//                                                                                                  CHOOSE SERIAL PORT
//*******************************************************************************************
#define     SERIAL_PORT "/dev/ttyACM0"



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

//*******************************************************************************************
//         Comandi da inviare al rover via seriale
//*******************************************************************************************
//modalità arduino:
#define AUTOMATIC           "1"
#define MANUAL              "2"
#define STOPPED             "3"

//comandi
#define CHANGEMODALITY      "c"
#define CMD_STOP            "5"
#define CMD_RESTART         "r"         /*Questo comando ha senso solo se siamo in modalità 'stopped', ovvero eravamo in modalità automatica e abbiamo dato stop. Se mandiamo questo vuol dire che vogliamo ripartire*/

#define CMD_FORWARD         "8"
#define CMD_BACKWARDS       "2"
#define CMD_LEFT            "4"
#define CMD_RIGHT           "6"
#define CMD_MOTORUP         "7"
#define CMD_MOTORDOWN       "1" 

//*******************************************************************************************
//         Costanti generali
//*******************************************************************************************
#define AUTOMATIC_MODE      1
#define MANUAL_MODE         0
#define STOPPED_MODE        2
#define PI                  3.14159265
#define SHUTDOWN            1
#define REBOOT              3
#define CMD_STOP_INT        atoi(CMD_STOP)
#define CMD_FORWARD_INT     atoi(CMD_FORWARD)
#define CMD_BACKWARDS_INT   atoi(CMD_BACKWARDS)
#define CMD_LEFT_INT        atoi(CMD_LEFT)
#define CMD_RIGHT_INT       atoi(CMD_RIGHT)


#define USEFULL_COMMAND -1          /*Usiamo questo define per differenziare i comandi tra "utili" all'interno di una determinata funzione e quelli non utili in quella funzione*/



//*******************************************************************************************
//         Variabili globali 
//*******************************************************************************************
int fd;                 //Descrittore porta seriale
int run;                //Finchè è TRUE il programma si ripete permettendo l'alternanza delle schermate. Viene modificato da un solo thread
int mode;               //Abbiamo due modalità, qui salviamo quella che stiamo utilizzando
int closeSw;            //Mettendo a true questa variabile forziamo lo spegnimento di questo software. Scritto da T2, letto da T1
int motorPower;         //Qui scriviamo la potenza del motore. di default parte da 140
int oldMotorPower;      //Qui vecchio valre. Cosi potremo effettuare dei controlli. Scritta e lette dallo stesso thread

struct task_par tp1;
struct task_par tp2; 
struct task_par tp3;



pthread_mutex_t mutex_screen        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_mode          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_closeSw       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_motorPower    = PTHREAD_MUTEX_INITIALIZER;


//*******************************************************************************************
//          Struttura per createButton e gestione dei processi (associati ai listener) che la funzione genera
//*******************************************************************************************

pthread_t tid[256];
int n_tid;


/*
 * Coordinate su cui opera il primo thread
*/
struct {
    int xq, yq;      //coordinate scritta "quit"
    int xc, yc;      //coordinate ellissi centro
    int rx, ry;      //liunghezza semiassi
    int xt, yt;      //coordinate scritta ellissi
} coordinatesT1;


/*


 * Coordinate su cui opera il secondo thread
*/
struct {
    int xc, yc;         //coordinate centro scritte
    int xf, yf;         //coordinate tasto "forward"
    int xr, yr;         //coordinate tasto "rigth"
    int xl, yl;         //coordinate tasto "left"
    int xb, yb;         //coordinate tasto "backwards"
} coordinatesT2;

//*******************************************************************************************
//         Prototipi
//*******************************************************************************************

//drawFunction.c
void drawEllipsisButton();
void drawHelpCommand();
void draw_gui();
void drawGraphicsMotor(int cmd, int xp, int yp);
void drawTrajectory(int xg, int yg, int *xg_old, int *yg_old, int x1, int y1, int x2, int y2);
void drawQuitKey();
void drawButtonDirection();
void drawIndicator();
void writeModality(int mode);

//managementAndReactionEvent.c
void analyzeKey(char key);
void managementIndicator();
int pressedEllipsis(int mouse_x, int mouse_y);
int pressedQuitKey(int x, int y);
int pressedForwardKey(int x, int y);
int pressedLeftKey(int x, int y);
int pressedRightKey(int x, int y);
int pressedBackwardsKey(int x, int y);
int getClearCommand(char buf);
void managementEvent(int mx, int my);
void stopAllThread();
void signalRoutine(int sig);
void getAngle(double *theta, int *oldCommand, int command);
void initAllVariable_construeGraphics(int *xcmain, int *ycmain, int *x1, int *y1, int *x2, int *y2, int *xp, int *yp);
void calculateCoordinates(double theta, float *x, float *y);

//operationMatrix.c
void wheelMatRight(int mat[][300], int rowmax, int colmax);
void initStructure_motor(int rowmax, int colmax);
void initStructure_main(int rowmax, int colmax);
void scrollMatrixMotorLeft(int rowmax, int colmax);
void scrollMatrixMotorRight(int rowmax, int colmax);
void managementMotrLeft(int cmd);
void managementMotrRight(int cmd);
void printCurveMotorLeft(int x, int y);
void printCurveMotorRight(int x, int y);

//main.c

void init_all();
void *readUserCommand(void *value);
void *managementT1(void *param);
void *construeGraphics(void *param);
void startThread(void*(*func)(void*), struct task_par *tp, int priority);
void primary_thread();
void secondary_thread();
void createScreen();
void selectMode(int mode);
#endif
