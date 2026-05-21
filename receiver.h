#ifndef ZADANIE1_RECEIVER_H
#define ZADANIE1_RECEIVER_H

#include "buffer.h"

#define MAX_UDP_SIZE 65507
using byte_t = uint8_t;

class Receiver {
public:
    Buffer buffer;

    // Argumenty z linii komend
    std::string DEST_ADDR;
    uint16_t DATA_PORT;

    size_t AUDIO_PACK_SIZE;
    byte_t* audio_pack;

    // Struktura poll wykorzystywana do jednoczesnego nasłuchiwania,
    // czy przyszedł nowy pakiet od sendera
    // i czy można wypisać nową paczkę na stdout
    struct pollfd *poll_descriptors;

    struct sockaddr_in client_address;

    Receiver(std::string destAddr,
             uint16_t dataPort,
             int bsize);

    // Pobiera kolejną paczkę audio z sieci
    ssize_t receive_pack();

    // Tworzy gniazdo nasłuchowe na odpowiednim porcie i drugie dla stdout
    void bind_sockets();

    // Jednocześnie nasłuchuje, czy sender nie wysłał kolejnego pakietu UDP
    // i czy można wypisać kolejną paczkę na stdout
    void start_listening();

    virtual ~Receiver();
};


#endif //ZADANIE1_RECEIVER_H
