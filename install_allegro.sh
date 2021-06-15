echo "Aggiorno i repo, aspetta qualche minuto..."
sudo apt update
echo 
echo "Installo allegro 4..."
sudo apt install liballegro4*
echo 
echo "Ok, fatto"
echo "Adesso provo a compilare un helloworld che utilizza la libreria allegro. Se la compilazione avra' successo allegro e' stato installato correttamente"
echo "Altrimenti c'è stato qualche problema"
echo "Premi invio per eseguirlo"
echo ">"
read

gcc -Wall -Werror install_allegro/helloAllegro.c -g -lpthread -lm -lrt -lalleg  -o install_allegro/helloAllegro
if [ "$?" == 0 ] 
then
    echo 
    echo "La compilazione ha avuto successo! Allegro installato con successo. Per eseguire il file appena compilato usa il seguente comando: ./install_allegro/helloAllegro"
    return 0
else 
    echo
    echo
    echo "La compilazione e' fallita :(. Le cause possono essere:"
    echo "1) Le libreria non sono state installate correttamente"
    echo "2) Hai modificato qualche file"
    echo "3) hai un problema con apt"
    echo ""
    echo ""
    echo "Soluzioni 1)"
    echo "      Sei collegato ad internet? Premi invio per effettuare un test della connessione"
    echo ">"
    read
    ping -c 4 www.google.it
    echo
    echo "      Se era un problema di connessione risolvilo e riprova ri-eseguendo questo script"
    echo "      Se invece internet e' ok premi invio ora e guarda il seguente output"
    echo ">"
    read
    sudo apt update
    echo 
    echo "Se nell'output sopra riportato vedi errori allora premi ctrl + C e risolvili (source path repo: /etc/apt/sources.list), altrimenti premi invio ora"
    read
    echo "Se non si e' ancora capito qual e' il problema guarda quest'ultimo output, premi invio ora"
    echo 
    read
    sudo apt install liballegro4*
    echo
    echo "Il problema dovrebbe essere sopra riportato."
    echo "Se i repo si sono corrotti vanno ripristinati (possono essere scaricati da internet, purche' si stia attenti alla versione del proprio sistema operativo)"
    echo "Se la libreria liballegro4* non viene proprio trovata allora e' un problema di repository"
    echo ""
    echo ""
    echo "Soluzioni 2)"
    echo "      Ripristina i file ri-scaricando i file originali. Premi invio per continuare, ctrl + C per chiudere, per continuare premi invio"
    read
    echo ""
    echo ""
    echo "Soluzioni 3)"
    echo "      Premi invio adesso per provare una serie di soluzioni, e' richiesta una connessione ad internet e un po' di pazienza"
    read
    sudo rm /var/lib/dpkg/lock
    sudo apt install -f
    sudo apt clean
    sudo apt autoremove
    sudo apt upgrade
    sudo apt install 
    sudo apt update && sudo apt install liballegro4*
    echo "Riprovare adesso a rieseguire questo script"
    return 1
fi
