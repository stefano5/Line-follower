# Line-follower with Arduino due

Arduino code: https://create.arduino.cc/projecthub/stefano_maugeri/autonomous-line-follower-with-seeed-shield-bot-1-2-and-arte-4faf68?ref=search&ref_id=line%20follower&offset=14

This was an universitary course project.
The aim was to control a little rover via serial, but also to allow it following an unknown path. In order to do this, I worked on periodic tasks, shared variables, motor control (rover uses H-bridges).

Two softwares has been developed, the first one is a real time application linux based, the second one is an arduino scketch.


## PC firmware
This software takes care of the communication between the pc and the rover via serial communication.
It also has a GUI where the following information are displayed:
  direction of travel of the vehicle
  estimation of the vehicle trajectory
  controlling of the forward velocity
  choose of the operation mode (manual control or autonomous line follower)

This software uses the pthread library to handle concurrent tasks.

There is a summary report: [report (ITA)](Report_ita.pdf).


## Arduino firmware
This firware takes care of the low level motor control and the serial communication.

[Real time ARte](http://arte.retis.santannapisa.it/) was used to create and manage tasks on arduino due. Three tasks has been developed on arduino.


## Arduino installation
Follow [this](http://arte.retis.santannapisa.it/getting_started.html) to install the required library. ARTe is an extension to the Arduino framework that supports multitasking and real-time preemptive scheduling.

## PC installation
allegro4 is required, install it:

```
$ cd ./[my path]/Line-follower
```
```
$ sudo chmod +x install_allegro.sh install.sh
```
```
$ ./install.sh
```

This script will install all the required dependencies and it will build the pc application. 
When the procedure ends, use this command to run the application:

```
$ sudo ./main /dev/ttyACM0
```

You may need to change the port!

_sudo_ is nedeed because the application will use a different scheduler than the default linux scheduler.




