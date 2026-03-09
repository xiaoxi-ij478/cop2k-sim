#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>
#include <iomanip>

#include "cop2k.hpp"
#include "as.hpp"

struct Instruction instructions[256 >> 2];
unsigned instruction_numbers = 0;

void clear_instruction_set(void)
{
    for (unsigned i = 0; i < instruction_numbers; i++) {
        instructions[i].byte = 0;
        instructions[i].mnemonic.clear();
        instructions[i].src = instructions[i].dst = Operand::NONE;
        std::memset(
            &instructions[i].microprogram,
            1,
            sizeof(instructions[i].microprogram)
        );
    }

    instruction_numbers = 0;
}

void add_to_instruction_set(const struct InstructionYacc &instruction)
{
    instructions[instruction_numbers++] = instruction;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "usage: as <instr.ins> <file.asm> [-o <out.bin>]" << std::endl;
        return EXIT_FAILURE;
    }

    clear_instruction_set();
    COP2K::MicroProgramMemory um;
    std::ofstream out_ofs;

    if (argc == 5) {
        if (!strcmp(argv[3], "-o")) {
            std::cerr << "usage: as <instr.ins> <file.asm> [-o <out.bin>]" << std::endl;
            return EXIT_FAILURE;
        }

        out_ofs.open(argv[4]);
        std::cout.rdbuf(out_ofs.rdbuf());
    }

    FILE *instr_file = fopen(argv[1], "r");

    if (!instr_file)
        return 1;

    parse_instruction_file(instr_file);

    for (unsigned i = 0; i < instruction_numbers; i++) {
    }
}
