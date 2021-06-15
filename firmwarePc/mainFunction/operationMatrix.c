/*
 * Le funzioni dentro questo file fanno solo operazioni su matrici, più precisamente rotazioni e traslazioni
 
 *  Sono state scritte e sono funzionanti, però alle fine non sono state usate. L'unica funzione utilizzata è la funzione scrollMatrixMotorLeft, che data una matrice trasla tutti i suoi elemti a sinistra
*/

#ifndef OPERATIONMATRIC_C
#define OPERATIONMATRIC_C

/*
 * In questa struttura abbiamo dei vettori. Ogni vettore può valere solo 0 o 1.
 
 * I vettori rappresentano un piano in cui, sulla base del valore contenuto, disegniamo delle linee in base al movimento dei motori
*/
struct {
    //char plan[300][300]       //Non più usata, ma le funzioni sotto hanno lavorato su di essa
    char motor_r[50][200];
    char motor_l[50][200];
} plan;

/*
 * Trasla tutti gli elementi di una matrice verso sopra                 NON USATA
*/
void scrollMatLeft(int mat[][300], int rowmax, int colmax) {
    int     i, j;
    for (i=0; i<rowmax; i++) {
        for (j=0; j<colmax; j++) {
            if (j+1 == colmax-1)
                mat[i][j] = 0;
            else 
                mat[i][j] = mat[i][j+1];
        }
    }
}

/*
 * Trasla tutti gli elementi di una matrice verso destra                NON USATA
*/
void scrollMatRight(int mat[][300], int rowmax, int colmax){ 
    int i, j;

    for (i=0; i<rowmax; i++) {
        for (j=colmax - 1; j>=0; j--) {
            if (j -1 == -1) 
                mat[i][j] = 0;
            else 
                mat[i][j] = mat[i][j-1];;
        }
    }
}

/*
 * Trasla tutti gli elementi di una matrice verso sotto                 NON USATA
*/
void scrollMatDown(int mat[][300], int rowmax, int colmax){
    int     i, j;
    for (i=rowmax-1; i>=-1; i--) {
        for (j=0; j<colmax; j++) {
            if (i-1 < 0)
                mat[i][j] = 0;
            else 
                mat[i][j] = mat[i-1][j];
        }
    }
}

/*
 * Trasla tutti gli elementi di una matrice verso sopra                 NON USATA
*/
void scrollMatUp(int mat[][300], int rowmax, int colmax){
    int     i, j;
    for (i=0; i<rowmax; i++) {
        for (j=0; j<colmax; j++) {
            if (i + 1 > colmax - 1)
                mat[i][j] = 0;
            else 
                mat[i][j] = mat[i+1][j];
        }
    }
}

/*
 * Ruota tutti gli elementi di una matrice verso destra                NON USATA
*/
void wheelMatRight(int mat[][300], int rowmax, int colmax) {
    int     i, j, k;
    char    temp[300][300];
    for (i=0; i<rowmax; i++) {
        for (j=0; j<colmax; j++) {
            temp[i][j] = mat[i][j];
        }
    }

    for (j=colmax-1, k=0; j>=0; j--, k++) {
        for (i=0; i<rowmax; i++) {
            mat[i][j] = temp[k][i]; 
        }
    }
}

/*
 * Ruota tutti gli elementi di una matrice verso sinistra               NON USATA
*/
void wheelMatLeft(int mat[][300], int rowmax, int colmax) {
    int     i, j;
    char    temp[300][300];
    for (i=0;i<rowmax;i++) {
        for (j=0;j<colmax;j++) {
            temp[i][j] = mat[i][j];
        }
    }

    for (i=0;i<rowmax;i++) {
        for (j=0;j<colmax;j++) {
            mat[i][j]=temp[j][(rowmax-1)-i];
        }
    }
}

/*
 * Inizializza matrici.
 * Essendo variabili globali sappiamo già che il loro valore sarà zero, però ricordando che i thread vengono stoppati e fatti ripartire ogni qual volta che si cambia la modalità di esecuzione con questa funzione
 * sovrascriviamo i vecchi punti
*/
void initStructure_motor(int rowmax, int colmax) {
    int     i, j;
    for (i=0; i<rowmax; i++) {
        for (j=0; j<colmax; j++) {
            plan.motor_l[i][j] = 0;
        }
    }

    for (i=0; i<rowmax; i++) {
        for (j=0; j<colmax; j++) {
            plan.motor_r[i][j] = 0;
        }
    }
}

/*
void initStructure_main(int rowmax, int colmax) {
    int     i, j;
    for (i=0; i<rowmax; i++) {
        for (j=0; j<colmax; j++) {
            plan.main[i][j] = 0;
        }
    }
}
*/

/*
 * Trasla verso sinistra una specifica matrice                      USATA
*/
void scrollMatrixMotorLeft(int rowmax, int colmax) {
    int     i, j;

    for (i=0; i<rowmax; i++) {
        for (j=colmax - 1; j>=0; j--) {
            if (j - 1 == -1) 
                plan.motor_l[i][j] = 0;
            else 
                plan.motor_l[i][j] = plan.motor_l[i][j-1];
        }
    }
}

/*
 * Trasla verso destra una specifica matrice                        NON USATA
*/
void scrollMatrixMotorRight(int rowmax, int colmax) {
    int     i, j;
    for (i=0; i<rowmax; i++) {
        for (j=colmax - 1; j>=0; j--) {
            if (j - 1 == -1) 
                plan.motor_r[i][j] = 0;
            else 
                plan.motor_r[i][j] = plan.motor_r[i][j-1];
        }
    }
}


/*
   (x1,y1)
   °=============================================°       
   | (x, 35)                                     |       I numeri rapprensentano gli indici della matrice in cui salviamo il grafico. Questi indici ci danno anche le coordinate di stampa sapendo che
   |      ____                                   |           la coordinata che vogliamo stampare ha valore nell'array  pari ad 1. Accederemo alla corretta coordinata con una somma: (x1 + j, y1 + j)
   |*____|   |    ...........=>                  |           La stampa risulta quindi essere molto semplice
   (x, 25)   |                                   |       In figura:      x rappresenta un offset da x1. La funzione sotto non conosce questo valore, e sinceramente neanche io. Quel valore è calcolato dalla funzione
   |       (x, 15)                               |                       scrollMatrixMotorLeft che si occupa di shiftare l'intero grafico verso destra di un pixel all'arrivo di un nuovo comando. 
   |                                             |                       Quello che sappiamo di x è che sarà sempre compreso tra 0 e 200. Ma per questa funzione x sarà sempre uguale a zero
   °=============================================°       Quindi questa funzione non disegna nessun grafico, disegna solo le variazioni dall'ultimo comando. Il grafico è "creato" dalla traslazione. 

   Questa funzione disegna quindi prima una retta orizzontale dalla vecchia retta al valore nuovo (che dipende dal corrente comando) e poi una retta orizzontale.

   In realtà le coordinate in figura sono poi specchiate rispetto una retta orizzontale passante per il punto 25. Qui sotto però non consideriamo completamente questo fattore e lasciamo questo compito alla funzione di stampa

 */
int oldCommandLeft = 5;       //Questa variabile è usata solo per questa funzione, è globale. Dichiarata qui mi ricorda che è usata solo per questa funzione
void managementMotrLeft(int cmd) {
    int     i;
    int     length = 1;

    switch (cmd) {
        case 8:
        case 6:
            if (oldCommandLeft == 5 || oldCommandLeft == 4) {
                for (i=25; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_l[i][0] = 1;             //Abbiamo salvato:   |

            } else if (oldCommandLeft == 2) {
                for (i=15; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_l[i][0] = 1;             //Abbiamo salvato:   |
                //                                                           | 
            }

            for (i=0; i<length; i++)                    //Disegniamo la barra orizzontale
                plan.motor_l[35][i] = 1;                //Abbiamo salvato:   __

            break;
        case 5:
        case 4:
            if (oldCommandLeft == 8 || oldCommandLeft == 6) {
                for (i=25; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_l[i][0] = 1;              //Abbiamo salvato:   |

            } else if (oldCommandLeft == 2) {
                for (i=15; i<25; i++)                   //Disegniamo la barra verticale
                    plan.motor_l[i][0] = 1;             //Abbiamo salvato:   |
                //                                                           | 
            } 
            for (i=0; i<length; i++)                    //Disegniamo la barra orizzontale
                plan.motor_l[25][i] = 1;                //Abbiamo salvato:   __

            break;
        case 2:
            if (oldCommandLeft == 8 || oldCommandLeft == 6) {
                for (i=15; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_l[i][0] = 1;             //Abbiamo salvato:   |
                //                                                           |

            } else if (oldCommandLeft == 5 || oldCommandLeft == 4) {
                for (i=15; i<25; i++)                   //Disegniamo la barra verticale
                    plan.motor_l[i][0] = 1;             //Abbiamo salvato:   |

            }

            for (i=0; i<length; i++)                    //Disegniamo la barra orizzontale
                plan.motor_l[15][i] = 1;                //Abbiamo salvato:   __

            break;
    }
    oldCommandLeft = cmd;
}

/*
   (x1,y1)
   °=============================================°    Quello che si fa qui è la stessa cosa che si fa nella managementMotrLeft
   | (35, 0)                                     |       
   |      ____                                   |      
   |*____|   |    ...........=>                  |     
   (25,y1)      |                                |
   |       (15, 0)                               |
   |                                             |
   °=============================================°

 */
int oldCommandRight = 5;    //Questa variabile è usata solo per questa funzione, è globale. Dichiarata qui mi ricorda che è usata solo per questa funzione
void managementMotrRight(int cmd) {
    int     i;
    int     length = 1;

    switch (cmd) {
        case 8:
        case 4:
            if (oldCommandRight == 5 || oldCommandRight == 6) {
                for (i=25; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_r[i][0] = 1;             //Abbiamo salvato:   |

            } else if (oldCommandRight == 2) {
                for (i=15; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_r[i][0] = 1;             //Abbiamo salvato:   |
                //                                                           | 
            }

            for (i=0; i<length; i++)                    //Disegniamo la barra orizzontale
                plan.motor_r[35][i] = 1;                //Abbiamo salvato:   __

            break;
        case 5:
        case 6:
            if (oldCommandRight == 8 || oldCommandRight == 4) {
                for (i=25; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_r[i][0] = 1;              //Abbiamo salvato:   |

            } else if (oldCommandRight == 2) {        
                for (i=15; i<25; i++)                   //Disegniamo la barra verticale
                    plan.motor_r[i][0] = 1;             //Abbiamo salvato:   |
                //                                                           | 
            } 
            for (i=0; i<length; i++)                    //Disegniamo la barra orizzontale
                plan.motor_r[25][i] = 1;                //Abbiamo salvato:   __

            break;
        case 2:
            if (oldCommandRight == 8 || oldCommandRight == 4) {
                for (i=15; i<35; i++)                   //Disegniamo la barra verticale
                    plan.motor_r[i][0] = 1;             //Abbiamo salvato:   |
                //                                                           |

            } else if (oldCommandRight == 5 || oldCommandRight == 6) {
                for (i=15; i<25; i++)                   //Disegniamo la barra verticale
                    plan.motor_r[i][0] = 1;             //Abbiamo salvato:   |

            }

            for (i=0; i<length; i++)                    //Disegniamo la barra orizzontale
                plan.motor_r[15][i] = 1;                //Abbiamo salvato:   __

            break;
    }
    oldCommandRight = cmd;
}

/*
 *  Stampiamo sulla GUI il grafico che è stato costrutio sul vettore
 * */
void printCurveMotorLeft(int x, int y) {
    int     i, j;
    pthread_mutex_lock(&mutex_screen);
    rectfill(screen, x, y, x+200, y+50, WHITE);
    pthread_mutex_unlock(&mutex_screen);
    for (i=0; i<50; i++) {
        for (j=0; j<200; j++) {
            if (plan.motor_l[i][j] == 1) {
                pthread_mutex_lock(&mutex_screen);
                putpixel(screen, x+j, y+50-i, BLACK);       //stampiamo partendo dal basso
                pthread_mutex_unlock(&mutex_screen);
            }                                               //In questo modo, per come sono salvati gli elementi nella matrice, se il motore va avanti il grafico è stampato verso l'alto, e se va in basso il grafico va verso il basso
        }
    }
}

/*
 *  Stampiamo sulla GUI il grafico che è stato costrutio sul vettore
 * */
void printCurveMotorRight(int x, int y) {
    int     i, j;
    pthread_mutex_lock(&mutex_screen);
    rectfill(screen, x, y, x+200, y+50, WHITE);
    pthread_mutex_unlock(&mutex_screen);
    for (i=0; i<50; i++) {
        for (j=0; j<200; j++) {
            if (plan.motor_r[i][j] == 1) {
                pthread_mutex_lock(&mutex_screen);
                putpixel(screen, x+j, y+50-i, BLACK);       //stampiamo partendo dal basso
                pthread_mutex_unlock(&mutex_screen);
            }                                               //In questo modo, per come sono salvati gli elementi nella matrice, se il motore va avanti il grafico è stampato verso l'alto, e se va in basso il grafico va verso il basso
        }
    }
} 
#endif
