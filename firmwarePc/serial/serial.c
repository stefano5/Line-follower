/*
 * Scopo di questo file è raggruppare le funzioni relative alla comunicazione seriale.
*/

#ifndef SERIAL_C
#define SERIAL_C

#include <errno.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>


/*
 * Con questa funzione inizializziamo la comunicazione seriale.
 * Chiamando questa funzione decidiamo di usare come velocità 89600 baud/s, di non usare bit di parità e di non settare il 'blocking'
 *      Se serve cambiare queste impostazioni possiamo chiamare le funzioni sotto commentate
*/
int init_serial(char *portname);


/*
 * Leggi i comandi che arrivano dalla seriale
 * Questi sappiamo essere di questo tipo:
 
 *  ['5']['\n']
 
 * Perciò ci interessa leggere solo il primo carattere
*/
char getCommandFromSerial(int fd);


/*
 * Stampiamo sulla console quello che arriva dalla seriale. Utile per il debug
*/
void printFromSerial(int fd);



/*
 * Con la set_interface_attribs settiamo il baud rate e il bit di parità
*/
int set_interface_attribs(int fd, int speed, int parity); 



/* 
 * Decidiamo il comportamento della funzione 'read'.
 *      Se should_block=1 la read aspetta di leggere un certo numero di byte, altrimenti legge quello che arriva fino alla fine del buffer
*/
void set_blocking(int fd, int should_block);



int set_interface_attribs (int fd, int speed, int parity) {
    struct termios  tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0){
        printf ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo,
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;                            // read doesn't block
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
    tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
        printf ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block){
    struct termios  tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        printf ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf ("error %d setting term attributes\n", errno);
}


void printFromSerial(int fd) {
    char    buf[256];                               //baffer seriale
    int     i;                                      //indice cicli
    int     n;                                      //numero elementi letti
    while (1) {
        for (i=0; i < 256; i++) buf[i]='\0';
        n = read (fd, buf, sizeof(buf));
        if (n <= 0) break;
        printf("%s", buf);
   }
}

/*
 * Leggiamo dalla seriale 'dimText' comandi
 * */
char getCommandFromSerial(int fd) {
    char    buf[256];                               //baffer seriale molto più grande della capacità che effettivasmente ci serve
    int     i;                                      //indice cicli
    int     n;                                      //numero elementi letti

    while (1) {
        for (i=0; i < 256; i++) buf[i]='\0';
        n = read (fd, buf, sizeof buf);
        if (n > 0) {
            if (buf[0] != '\n' && buf[0] != ' ') {
                return buf[0];
            } 
        } 
    }
}

/*
 * Equvale alla Serial.print() che usiamo su arduino. Con questa funzione scriviamo sulla seriale
 */
void sendToSerial(int fd, char *cmd) {
    write (fd, cmd, sizeof(char) * strlen(cmd));        //chiaramente sizeof(char) = 1
}


int init_serial(char *portname) {
    char    strError[256];
    int     i;
    int     fd;                         // File Descriptor

    for (i=0; i<256; i++) strError[i] = '\0';

    fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);

    if (fd < 0) {
        sprintf (strError, "error %d opening %s: %s\nConnect the wire and try: sudo chmod 777 %s so run again, shold be worked, if the port is correct", errno, portname, strerror (errno), portname);
#ifdef ALLEGRO_H
        allegro_message("%s", strError);
#else
        printf("%s", strError);
#endif
        return -1;
    }

    set_interface_attribs (fd, B9600, 0);              // set speed to 96000 bps, 8n1 (no parity)
    set_blocking (fd, 0);                              // set NO blocking
    //printf("[%s] Initialization successful\n", __FILE__);
    sleep(1);
    return fd;

}

void close_serial(int fd) {
    close(fd);
}
#endif
