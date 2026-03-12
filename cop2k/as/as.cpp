#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>
#include <iomanip>
#include <unordered_map>

#include "libcop2k.hpp"
#include "libopcode.hpp"
#include "as.hpp"

COP2K::Opcode opcode;
COP2K::Memory em;
std::unordered_map<std::string, unsigned char> consts;
std::unordered_map<std::string, unsigned char> labels;

static unsigned char get_const(const char *name)
{
    return consts.at(name);
}

static void set_const(const char *name, unsigned char val)
{
    consts.emplace(name, val);
}

static unsigned char get_label(const char *name)
{
    return labels.at(name);
}

static void set_label(const char *name, unsigned char addr)
{
    labels.emplace(name, addr);
}

bool overflow = false;
void add_instruction(
    const char *mnemonic,
    const char *label,
    const struct AsmInstructionOperandYacc &operand
)
{
#define PUT_BYTE(b) \
    do { \
        if (overflow) \
            throw std::out_of_range("em overflow");\
        em.set_data(b); \
        if (em.get_addr() == 255) \
            overflow = true; \
        em.set_addr(em.get_addr() + 1); \
    } while (0)

    if (
        !strcasecmp(mnemonic, "end") ||
        !strcasecmp(mnemonic, "if") ||
        !strcasecmp(mnemonic, "else") ||
        !strcasecmp(mnemonic, "endif")
    )
        return;

    if (!strcasecmp(mnemonic, "db"))
        PUT_BYTE(operand.src);

    else if (!strcasecmp(mnemonic, "org"))
        em.set_addr(operand.src);

    else if (!strcasecmp(mnemonic, "00const"))
        set_const(label, operand.src);

    else if (!strcasecmp(mnemonic, "00label"))
        set_label(label, operand.src);

    else {
        const Instruction &ins = opcode.get_from_mnemonic(mnemonic, operand.src_type, operand.dst_type);

        switch (operand.src_type) {
            case COP2K::Operand::NONE:
            case COP2K::Operand::REG_A:
            case COP2K::Operand::IMMED:
            case COP2K::Operand::MEMADDR:
                break;

            case COP2K::Operand::REG:
            case COP2K::Operand::REGADDR:
                PUT_BYTE(ins.byte | operand.src);
                break;
        }

        switch (operand.dst_type) {
            case COP2K::Operand::NONE:
            case COP2K::Operand::REG_A:
            case COP2K::Operand::IMMED:
            case COP2K::Operand::MEMADDR:
                break;

            case COP2K::Operand::REG:
            case COP2K::Operand::REGADDR:
                PUT_BYTE(ins.byte | operand.dst);
                break;
        }

        switch (operand.src_type) {
            case COP2K::Operand::NONE:
            case COP2K::Operand::REG_A:
            case COP2K::Operand::REG:
            case COP2K::Operand::REGADDR:
                break;

            case COP2K::Operand::IMMED:
            case COP2K::Operand::MEMADDR:
                PUT_BYTE(operand.src);
                break;
        }

        switch (operand.dst_type) {
            case COP2K::Operand::NONE:
            case COP2K::Operand::REG_A:
            case COP2K::Operand::REG:
            case COP2K::Operand::REGADDR:
                break;

            case COP2K::Operand::IMMED:
            case COP2K::Operand::MEMADDR:
                PUT_BYTE(operand.dst);
                break;
        }
    }
#undef PUT_BYTE
}

int main(int argc, char **argv)
{
    em.set_addr(0);
    if (argc < 3) {
        std::cerr << "usage: as <instr.ins> <file.asm> [-o <out.bin>]" << std::endl;
        return EXIT_FAILURE;
    }

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

    opcode.load_instr_txt(instr_file);
    //for (unsigned i = 0; i < instruction_numbers; i++) {
    //}
}
