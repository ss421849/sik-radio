#include "receiver.h"
#include <poll.h>
#include <unistd.h>

#define STDOUT 0
#define UDPIN 1

#define TIMEOUT -1      // Oczekiwanie w poll może trwać dowolnie długo
#define CONNECTIONS 2

Receiver::Receiver(std::string destAddr,
                   uint16_t dataPort,
                   int bsize)
                   : DEST_ADDR(std::move(destAddr)),
                   DATA_PORT(dataPort),
                   buffer(bsize) {
    // Alokowanie miejsca na największą paczkę, jaką potencjalnie można przyjąć.
    // (Wiadomo, że musi jednocześnie mieścić się w jednym datagramie UDP i w buforze)
    AUDIO_PACK_SIZE = std::min((ssize_t) MAX_UDP_SIZE, (ssize_t) (bsize + 2 * sizeof(uint64_t)));
    audio_pack = (byte_t*) malloc(AUDIO_PACK_SIZE);
    poll_descriptors = (struct pollfd*) malloc(2 * sizeof(struct pollfd));

}

void Receiver::bind_sockets() {
    poll_descriptors[STDOUT].events = 0;  // Dopóki 3/4 bufora nie jest zapełnione, to nic nie wypisuj
    poll_descriptors[STDOUT].revents = 0;
    poll_descriptors[STDOUT].fd = STDOUT_FILENO;

    poll_descriptors[UDPIN].events = POLLIN;
    poll_descriptors[UDPIN].revents = 0;
    poll_descriptors[UDPIN].fd = socket(AF_INET, SOCK_DGRAM, 0);
    ENSURE(poll_descriptors[UDPIN].fd >= 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(DATA_PORT);

    CHECK_ERRNO(bind(poll_descriptors[UDPIN].fd, (struct sockaddr *) &server_address,
            (socklen_t) sizeof(server_address)));
}

ssize_t Receiver::receive_pack() {
    auto address_length = (socklen_t) sizeof(client_address);
    int flags = 0;
    errno = 0;

    auto len = recvfrom(poll_descriptors[UDPIN].fd, audio_pack, AUDIO_PACK_SIZE, flags,
                                      (struct sockaddr *) &client_address, &address_length);

    if (len < 0) {
        PRINT_ERRNO();
    }

    return len;
}

// Obsługa niepożądanych wydarzeń w poll
void handle_error_revents(short revents, const std::string& source) {
    if (revents & POLLERR) {
        std::cerr << "Error in " << source << '\n';
        PRINT_ERRNO();
    }
    if (revents & POLLHUP) {
        std::cerr << "Disconnection with " << source << '\n';
        exit(EXIT_FAILURE);
    }
    if (revents & POLLNVAL) {
        std::cerr << "Wrong descriptor for " << source << '\n';
        exit(EXIT_FAILURE);
    }
}

void Receiver::start_listening() {
    bind_sockets();

    while (true) {
        poll_descriptors[UDPIN].revents = 0;
        poll_descriptors[STDOUT].revents = 0;

        int poll_status = poll(poll_descriptors, CONNECTIONS, TIMEOUT);

        if (poll_status == -1) {
            if (errno == EINTR)
                fprintf(stderr, "Interrupted system call\n");
            else
                PRINT_ERRNO();
        }
        else if (poll_status > 0) {
            // Jeśli jest wolne stdout i oczekiwano na to,
            // to wypisuje kolejną paczkę
            if (poll_descriptors[STDOUT].revents & POLLOUT) {
                buffer.print_pack();
            }
            handle_error_revents(poll_descriptors[STDOUT].revents, "stdout");

            // Jeśli otrzyma nową paczkę, to zapisuje ją
            // i jeśli teraz buffer jest gotowy do wypisania kolejnej paczki,
            // to ustawia oczekiwanie na wolne stdout
            if (poll_descriptors[UDPIN].revents & POLLIN) {
                auto len = receive_pack();
                buffer.parse_pack(audio_pack, len);
            }
            handle_error_revents(poll_descriptors[UDPIN].revents, "network connection");

            // Sprawdzenie, czy nadal oczekuje na wypisanie kolejnej paczki
            poll_descriptors[STDOUT].events = buffer.is_ready_to_cout() ? POLLOUT : 0;
        }
    }
}

Receiver::~Receiver() {
    close(poll_descriptors[UDPIN].fd);
    free(poll_descriptors);
    free(audio_pack);
}