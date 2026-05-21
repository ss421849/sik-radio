#ifndef ZADANIE1_BUFFER_H
#define ZADANIE1_BUFFER_H

#include <cstdint>
#include <string>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <utility>
#include <set>
#include "err.h"

// Odwracanie bajtów w liczbach pomiędzy kolejnością sieciową i lokalną
#define ntohll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

using byte_t = uint8_t;

class Buffer {
public:
    // Dane z linii komend
    size_t BSIZE;

    // Dane wyciągane z paczek
    byte_t* buffer;
    uint64_t session_id;
    uint64_t BYTE0;
    size_t PSIZE;

    // Indeksy potrzebne do nawigowania w obrębie buforu
    uint64_t buffer_begin;  // Numer pierwszego bajtu do wypisania
    uint64_t buffer_end;    // Numer bajtu za ostatnim, który otrzymano (koniec bufora)

    ssize_t last_received_pack_num;
    std::set<size_t> unreceived_packs;


    Buffer(size_t bsize);

    void new_session(uint64_t new_session_id, uint64_t new_byte0, size_t new_psize);

    // Sprawdza, czy paczka o zadanym numerze first_byte_num jeszcze nie została otrzymana
    // Aktualizuje unreceived_packs, tzn.:
    //   * Jeśli paczka ma większy numer od dotychczas otrzymanych,
    //     to dodaje nowe numery mniejsze od numeru paczki.
    //   * Jeśli oczekiwano na paczkę o danym numerze to usuwa ten numer z unreceived_packs
    // Zwraca false, jeśli paczka o zadanym numerze już nie jest potrzebna
    // (bo została otrzymana, lub jest za stara) i true w p.p.
    bool check_unreceived_packs(uint64_t first_byte_num);

    // Usuwa ze zbioru numerów nieotrzymanych paczek te, które są już tak stare, że nie ma na nie miejsca w buforze
    void clear_old_unreceived_packs();

    bool is_ready_to_cout() const {
        return BSIZE * 3 / 4 <= buffer_end - BYTE0 && buffer_begin < buffer_end;
    }

    // Analizuje paczkę zapisaną w audio_pack
    //    void parse_pack(void* audio_pack) {
    void parse_pack(const void* audio_pack, size_t audio_pack_size);

    // Wypisuje na stdout jedną paczkę
    // i aktualizuje buffer_begin
    void print_pack();

    // Pomija ewentualne dziury na początku bufora
    // i przesuwa odpowiednio buffer_begin tak,
    // aby wskazywał na indeks pierwszego bajtu zapisanego w buforze gotowego do wypisania,
    // lub na koniec bufora
    void skip_holes();

    ~Buffer();
};


#endif //ZADANIE1_BUFFER_H
