#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>
#include <iomanip>
#include <unordered_map>

#include "cop2k.hpp"
#include "as.hpp"

Instruction instructions[256 >> 2];
unsigned instruction_numbers = 0;
COP2K::Memory em;
unsigned em_pos = 0;
std::unordered_map<std::string, unsigned char> consts;

void clear_instruction_set(void)
{
    for (Instruction &i : instructions) {
        i.byte = 0;
        i.mnemonic.clear();
        i.src = i.dst = Operand::NONE;
        std::memset(&i.microprogram, 1, sizeof(i.microprogram));
    }

    instruction_numbers = 0;
}

void add_to_instruction_set(const struct InstructionYacc &instruction)
{
    instructions[instruction_numbers++] = instruction;
}

void add_instruction(
    const char *mnemonic,
    const struct AsmInstructionOperandYacc &operand
)
{
}

unsigned char get_const(const char *name)
{
    return consts.at(name);
}

void set_const(const char *name, unsigned char val)
{
    consts.emplace(name, val);
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
    //for (unsigned i = 0; i < instruction_numbers; i++) {
    //}
}
