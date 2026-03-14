#include <fstream>
#include <iostream>
#include "dis.hpp"

int main(int argc, char **argv)
{
    COP2K::DIS dis;

    if (argc < 3) {
        std::cerr << "usage: dis <instr.txt> <file.bin>" << std::endl;
        return EXIT_FAILURE;
    }

    FILE *instr_txt = fopen(argv[1],"r");

    if (!instr_txt)
        return EXIT_FAILURE;

    dis.opcode.load_instr_txt(instr_txt);
    fclose(instr_txt);
    std::ifstream ifs(argv[2]);

    if (!ifs)
        return EXIT_FAILURE;

    std::cout << dis.disassemble(ifs) << std::endl;
}
