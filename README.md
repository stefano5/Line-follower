# Line-follower con Arduino due

Vedi anche: https://create.arduino.cc/projecthub/stefano_maugeri/autonomous-line-follower-with-seeed-shield-bot-1-2-and-arte-4faf68?ref=search&ref_id=line%20follower&offset=14

Questo progetto è stato richiesto per un corso universitario. 
Lo scopo era quello di controllare un rover tramite seriale, gestendo task periodici real time e variabili condivise.

Si ha quindi del codice che deve essere eseguito da un pc linux based, e un altro programma scritto ad hoc per un arduino due.

Sono quindi presenti due cartelle che contengono i relativi firmware.

## Firmware PC
Qui sono presenti i sorgenti da compilare.
Questo software gestisce la comunicazione seriale con il rover. Utilizza una GUI (scelta di progetto) su cui sono mostrate le seguenti informazioni:
  Comportamento dei motori (direzione di rotazione)
  Stima della traiettoria eseguita (real time)
  Scelta della velocità di avanzamento
  Scelta della modalità (modalità: inseguimento di percorso o manuale da tastierino numerico)
  Tasti vari
  
Una descrizione esaustiva sul codice è riportata sulla [relazione condivisa](Report_ita.pdf).

## Firmware arduino
Sono presenti due file, uno per la gestione dei motori e l'altro per la gestione dei task.

Si è usato il sistema operativo [real time ARte](http://arte.retis.santannapisa.it/) scritto per arduino due.
Per caricare il firmware su arduino è possibile seguire [questa guida](http://arte.retis.santannapisa.it/getting_started.html).

La gestione dell'inseguimento della traiettoria è affidata ad uno specifico task. Il controllo utilizza dei sensori infrarossi per orientarsi.
Tuttavia il controllo qui sviluppato non è performante e per niente smooth. Lo scopo del progetto non era infatti la sua perfetta messa a punto. È però perfettamente in grado di inseguire un percorso, ed è stato scritto per essere intrinsecamente _fault tollerant_. 

Il rover utilizzato integra un ponte H per il controllo dei motori DC, il firmware di controllo dei motori si interfaccia quindi a lui.

Ulteriori informazioni sono disponibili [qui](Line-follower/Report_ita.pdf).

## Installazione arduino
Per caricare su arduino due il software va prima scaricato il SO, utilizzare [questa guida](http://arte.retis.santannapisa.it/getting_started.html). 

## Installazione PC
È richiesta la libreria allegro4, verrà installata automaticamente.

```
$ cd ./[mio path]/Line-follower
```
```
$ sudo chmod +x install_allegro.sh  install.sh
```
```
$ ./install.sh
```

Se la libreria allegro è presente questo codice richiederà pochi istanti, altrimenti potrebbe richiedere qualche tempo per l'installazione.

Quando l'installer avrà completato viene suggerito l'output da dare per eseguire il programma, ovvero:

```
$ sudo ./main /dev/ttyACM0
```

Il parametro è opzionale, se non gli è dato nessun parametro il parametro di default è sempre _/dev/ttyACM0_. È possibile modificare la scelta di default dall'[header file](firmwarePc/mainFunction/headerFile.h)

Sono richiesti i permessi di SuperUser perchè è utilizzato uno scheduler diverso da quello di default.




