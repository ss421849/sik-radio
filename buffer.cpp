#include <unistd.h>
#include "buffer.h"


Buffer::Buffer(size_t bsize) : BSIZE(bsize) {
    buffer = (byte_t*) malloc(BSIZE);
    unreceived_packs = {};
    new_session(0, 0, 0);
}

void Buffer::new_session(uint64_t new_session_id, uint64_t new_byte0, size_t new_psize) {
    session_id = new_session_id;
    BYTE0 = new_byte0;
    PSIZE = new_psize;

    buffer_begin = new_byte0;
    buffer_end = new_byte0;
    last_received_pack_num = -1;

    unreceived_packs.clear();
}

bool Buffer::check_unreceived_packs(uint64_t first_byte_num) {
    if (buffer_begin > first_byte_num) {
        return false;
    }
    else {
        auto new_pack_num = (ssize_t) ((first_byte_num - BYTE0) / PSIZE);

        // Sprawdzenie, czy powstają nowe dziury w buforze
        // (w przypadku otrzymania paczki o numerze większym niż do tej pory)
        if (new_pack_num > last_received_pack_num) {
            for (ssize_t i = last_received_pack_num + 1; i < new_pack_num; ++i) {
                unreceived_packs.insert(unreceived_packs.end(), i);
            }
            last_received_pack_num = new_pack_num;
        }
        else if (unreceived_packs.find(new_pack_num) == unreceived_packs.end()) {
            return false;
        }
        else {
            unreceived_packs.erase(new_pack_num);
        }

        // Wypisywanie na standardowe wyjście diagnostyczne numerów brakujących paczek
        for (auto itr = unreceived_packs.begin(); itr != unreceived_packs.end() && *itr < new_pack_num; itr++) {
            std::cerr << "MISSING: BEFORE " << new_pack_num << " EXPECTED " << *itr << "\n";
        }
        return true;
    }
}

void Buffer::clear_old_unreceived_packs() {
    size_t new_first_pack_num = (buffer_begin - BYTE0) / PSIZE;

    auto it = unreceived_packs.lower_bound(new_first_pack_num);
    unreceived_packs.erase(unreceived_packs.begin(),it);
}

void Buffer::parse_pack(const void* audio_pack, size_t audio_pack_size) {
    // Pierwsze 8 bajtów dla session_id
    uint64_t new_session_id = ntohll(((uint64_t*) audio_pack)[0]);
    // Następne 8 bajtów na numer pierwszego bajtu
    uint64_t first_byte_num = ntohll(((uint64_t*) audio_pack)[1]);

    if (new_session_id >= session_id) {
        if (new_session_id > session_id) {
            new_session(new_session_id, first_byte_num, audio_pack_size - 2 * sizeof(uint64_t));
        }

        if (check_unreceived_packs(first_byte_num)) {
            std::memcpy(&buffer[(first_byte_num - BYTE0) % BSIZE], &((uint64_t*) audio_pack)[2], PSIZE);
            // Aktualizacja pierwszego bajtu, na jaki jeszcze nie zarezerwowano miejsca w buforze
            if (buffer_end <= first_byte_num) {
                buffer_end = first_byte_num + PSIZE;
            }
            // Sprawdzanie, czy nadpisano niewypisane paczki
            if (buffer_begin + BSIZE <= first_byte_num) {
                // Zarezerwowanie miejsca dla tylu paczek, ile tylko się da
                buffer_begin = buffer_end - BSIZE;
                clear_old_unreceived_packs();
            }
        }
    }
}

void Buffer::print_pack() {
    write(STDOUT_FILENO, (char*) &buffer[(buffer_begin - BYTE0) % BSIZE], PSIZE);
    buffer_begin += PSIZE;

    skip_holes();
}

void Buffer::skip_holes() {
    auto next_pack_num = (ssize_t) ((buffer_begin - BYTE0) / PSIZE);
    auto it = unreceived_packs.find(next_pack_num);

    while (buffer_begin < buffer_end
        && it != unreceived_packs.end()) {
        unreceived_packs.erase(it);

        buffer_begin += PSIZE;
        next_pack_num++;
        it = unreceived_packs.find(next_pack_num);
    }
}

Buffer::~Buffer() {
    free(buffer);
    unreceived_packs.clear();
}