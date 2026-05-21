#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include "sender.h"

#ifndef STUDENT_ID
#define STUDENT_ID 421849
#endif

#define DEFAULT_PORT (STUDENT_ID % 10000)

namespace po = boost::program_options;


boost::optional<Sender> create_sender(int argc, const char *argv[]) {
    std::string receiver_address;
    uint16_t receiver_port;
    size_t PSIZE;
    std::string sender_name;

    po::options_description desc{"Options"};
    desc.add_options()
    ("help,h", "Help screen")
    (",a", po::value<std::string>(&receiver_address)->default_value(""), "adres odbiornika")
    (",P", po::value<uint16_t>(&receiver_port)->default_value(DEFAULT_PORT), "port UDP używany do przesyłania danych")
    (",p", po::value<size_t>(&PSIZE)->default_value(512), "rozmiar w bajtach pola audio_data paczki")
    (",n", po::value<std::string>(&sender_name)->default_value("Nienazwany Nadajnik"), "nazwa nadajnika")
    ;

    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);


    if (vm.count("help")) {
        std::cout << desc << '\n';
        return {};
    }
    else if (receiver_address.empty()) {
        throw std::runtime_error("Wymagany adres IP");
    }
    else {
        return Sender(receiver_address, receiver_port, PSIZE, sender_name);
    }
}

int main(int argc, const char *argv[])
{
    try
    {
        auto sender = create_sender(argc, argv);
        if (sender)
            sender->send_file();
    }
    catch (const po::error &ex) {
        std::cerr << ex.what() << '\n';
        exit(1);
    }
}

