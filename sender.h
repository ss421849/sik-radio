

#ifndef ZADANIE1_SENDER_H
#define ZADANIE1_SENDER_H

#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <utility>
#include "err.h"

// Odwracanie bajtów w liczbach pomiędzy kolejnością sieciową i lokalną
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

class Sender {
public:
    std::string DEST_ADDR;
    uint16_t DATA_PORT;
    size_t PSIZE;
    std::string NAZWA;

    size_t AUDIO_PACK_SIZE;
    char* audio_pack;
    uint64_t session_id;
    uint64_t first_byte_num;
    char* audio_data;

    struct sockaddr_in send_address;
    int socket_fd;

    Sender(std::string receiver_address,
           uint16_t receiver_port,
           size_t psize,
           std::string senderName);

    void get_send_address();

    void bind_socket();

    void connect();

    // Wczytuje ze standardowego wejścia kolejną paczkę
    // Zwraca 0 w przypadku powodzenia i 1 w p.p.
    int get_audio_pack();

    void send_pack();

    void send_file();

    virtual ~Sender();
};

#endif //ZADANIE1_SENDER_H
