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
    ./install_allegro.sh && gcc -Wall -Werror firmwarePc/main.c -lpthread -lm -lrt -lalleg -o firmwarePc/main && ln -s $PWD/firmwarePc/main $PWD/
fi
