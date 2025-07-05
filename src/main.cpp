#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "include/init.hpp"
#include "include/cat-file.hpp"

enum class Command { // Use CamelCase
    Init,
    CatFile,
    Unknown
};

Command parse_command(const std::string& cmd) {
    if (cmd == "init") return Command::Init;
    if (cmd == "cat-file") return Command::CatFile;
    return Command::Unknown;
}

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!\n";
    
    if (argc < 2) {
        std::cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }
    
    std::string command = argv[1];

    switch(parse_command(command)) {
        case Command::Init:
            return git_init();
        case Command::CatFile:
            return cat_file(argc, argv);
        case Command::Unknown:
            std::cout << "Unknown command " << command << '\n';
            return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
