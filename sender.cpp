#include "sender.h"

Sender::Sender(std::string receiver_address,
           uint16_t receiver_port,
           size_t psize,
           std::string senderName)
           : DEST_ADDR(std::move(receiver_address)),
           DATA_PORT(receiver_port),
           PSIZE(psize),
           NAZWA(std::move(senderName)) {
    AUDIO_PACK_SIZE = 2 * sizeof(uint64_t) + PSIZE;
    audio_pack = (char*) malloc(AUDIO_PACK_SIZE);
    session_id = 1;
    first_byte_num = 0;
    audio_data = audio_pack + 16;
}

void Sender::get_send_address() {
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;

    int error_number;
    if ((error_number = getaddrinfo(DEST_ADDR.c_str(), NULL, &hints, &address_result))) {
        throw std::runtime_error("getaddrinfo returned error with code " + std::to_string(error_number));
    }

    send_address.sin_family = AF_INET; // IPv4
    send_address.sin_addr.s_addr =
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address

    send_address.sin_port = htons(DATA_PORT); // port from the command line

    freeaddrinfo(address_result);
}

void Sender::bind_socket() {
    socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0 && errno != 0) {
        throw std::runtime_error("socket ended with error with code: " + std::to_string(errno));
    }
}

void Sender::connect() {
    get_send_address();
    bind_socket();
}

int Sender::get_audio_pack() {
    // Pierwsze 8 bajtów dla session_id
    ((uint64_t*) audio_pack)[0] = htonll(session_id);
    // Następne 8 bajtów na numer pierwszego bajtu
    ((uint64_t*) audio_pack)[1] = htonll(first_byte_num);
    // Potem PSIZE bajtów ze standardowego wejścia
    std::cin.read(audio_data, PSIZE);

    first_byte_num += PSIZE;

    return (std::cin.rdstate() & std::istream::eofbit);
}

void Sender::send_pack() {
    int send_flags = 0;
    socklen_t address_length = (socklen_t) sizeof(send_address);
    errno = 0;

    ssize_t sent_length = sendto(socket_fd, audio_pack, AUDIO_PACK_SIZE, send_flags,
                                 (struct sockaddr *) &send_address, address_length);

    if (sent_length < 0) {
        PRINT_ERRNO();
    }
    ENSURE(sent_length == (ssize_t) AUDIO_PACK_SIZE);
}

void Sender::send_file() {
    connect();
    while (!get_audio_pack()) {
        send_pack();
    }
    close(socket_fd);
}

Sender::~Sender() {
    free(audio_pack);
}