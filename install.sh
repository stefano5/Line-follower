#!/bin/bash

if [ -f /usr/include/allegro.h ]; then
    cd firmwarePc

    gcc -Wall -Werror main.c -lpthread -lm -lrt -lalleg -o main
    
    cd ..

    if [ -h main ]; then
        unlink main
    fi

    ln -s $PWD/firmwarePc/main $PWD/

    echo "To run: sudo ./main"
else 
    echo "Allegro not found, please try: \"sudo apt install liballegr4*\""
fi
