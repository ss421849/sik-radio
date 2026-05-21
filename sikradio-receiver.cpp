#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include "receiver.h"

#ifndef STUDENT_ID
#define STUDENT_ID 421849
#endif
#define DEFAULT_PORT (STUDENT_ID % 10000)

namespace po = boost::program_options;

boost::optional<Receiver> create_receiver(int argc, const char *argv[]) {
    std::string receiver_address;
    uint16_t receiver_port;
    int BSIZE;

    po::options_description desc{"Options"};
    desc.add_options()
    ("help,h", "Help screen")
    (",a", po::value<std::string>(&receiver_address)->default_value(""), "adres odbiornika")
    (",P", po::value<uint16_t>(&receiver_port)->default_value(DEFAULT_PORT), "port UDP używany do przesyłania danych")
    (",b", po::value<int>(&BSIZE)->default_value(65536), "rozmiar w bajtach bufora audio_data paczki")
    ;

    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);


    if (vm.count("help")) {
        std::cout << desc << '\n';
        return {};
    }
    else {
        return Receiver(receiver_address, receiver_port, BSIZE);
    }

}

int main(int argc, const char *argv[]) {
    // Wczytywanie argumentów z linii komend
    try {
        auto receiver = create_receiver(argc, argv);

        if (receiver)
            receiver->start_listening();
    }
    catch (const po::error &ex) {
        std::cerr << ex.what() << '\n';
        exit(1);
    }
    catch (const std::runtime_error &ex) {
        std::cerr << ex.what() << '\n';
        exit(1);
    }
}

