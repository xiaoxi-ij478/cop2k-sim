#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstring>

#include "dis.hpp"

int main(int argc, char **argv)
{
    COP2K::DIS dis;

    if (argc < 3 || !strcmp(argv[1], "--help")) {
        std::cerr << "usage: dis <instr.txt> <file.bin>" << std::endl;
        return EXIT_FAILURE;
    }

    FILE *instr_txt = fopen(argv[1], "r");

    if (!instr_txt)
        return EXIT_FAILURE;

    try {
        dis.opcode.load_instr_txt(instr_txt);

    } catch (const std::runtime_error &) {
        return EXIT_FAILURE;
    }

    fclose(instr_txt);
    std::ifstream ifs(argv[2]);
    std::string instr_byte;

    if (!ifs)
        return EXIT_FAILURE;

    std::copy(
        std::istream_iterator<unsigned char>(std::cin),
        std::istream_iterator<unsigned char>(),
        std::back_inserter(instr_byte)
    );
    ifs.close();
    std::cout << dis.disassemble(instr_byte) << std::endl;
}
