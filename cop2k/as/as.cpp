#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>
#include <iomanip>
#include <cstdio>

#include "libcop2k.hpp"
#include "libopcode.hpp"
#include "as.hpp"

int main(int argc, char **argv)
{
    COP2K::AS as;

    if (argc < 3 || !strcmp(argv[1], "--help")) {
        std::cerr << "usage: as <instr.txt> <file.asm> [-o <out.bin>]" << std::endl;
        return EXIT_FAILURE;
    }

    FILE *instr_file = fopen(argv[1], "r");
    FILE *asm_file = fopen(argv[2], "r");
    FILE *out_file = stdout;

    if (argc == 5) {
        if (strcmp(argv[3], "-o")) {
            std::cerr << "usage: as <instr.txt> <file.asm> [-o <out.bin>]" << std::endl;
            return EXIT_FAILURE;
        }

        out_file = fopen(argv[4], "w");
    }

    if (!instr_file || !asm_file || !out_file)
        return EXIT_FAILURE;

    try {
        as.opcode.load_instr_txt(instr_file);

    } catch (const std::runtime_error &) {
        return EXIT_FAILURE;
    }

    fclose(instr_file);

    try {
        as.assemble_file(asm_file);

    } catch (const std::runtime_error &) {
        return EXIT_FAILURE;
    }

    std::string memory = as.em.dump_content();
    fwrite(memory.c_str(), 1, memory.size(), out_file);
}
