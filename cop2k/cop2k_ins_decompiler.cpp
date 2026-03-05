#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <endian.h>

enum class Operand : unsigned char {
    NONE, A, REG, REGADDR, IMMED, ADDR
};
struct Instruction {
    bool exist;
    unsigned char byte;
    std::string name;
    std::string desc;
    Operand src, dst;
} instr[256 >> 2];

std::bitset<24> um[256];

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "usage: cop2k_ins_decompiler <file.ins>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream ifs(argv[1]);

    if (!ifs)
        return EXIT_FAILURE;

    const char *magic_str = "INSTURCTION & uM file for COP2000\x1a";
    std::string magic;
    magic.resize(strlen(magic_str));
    ifs.read(magic.data(), magic.size());

    if (magic != magic_str)
        return EXIT_FAILURE;

    for (Instruction &i : instr) {
        if (!(i.exist = ifs.get()))
            continue;

        i.name.resize(ifs.get());
        ifs.read(i.name.data(), i.name.size());
        i.byte = ifs.get();
        i.src = static_cast<Operand>(ifs.get());
        i.dst = static_cast<Operand>(ifs.get());
        i.desc.resize(ifs.get());
        ifs.read(i.desc.data(), i.desc.size());
    }

    for (std::bitset<24> &i : um) {
        std::stringstream ss;
        unsigned a;
        ifs.read(reinterpret_cast<char *>(&a), 3);
        a = le32toh(a);

        while (a) {
            ss << ((a & 1) ? "1" : "0");
            a >>= 1;
        }

        ss >> i;
        ifs.get();
    }

    for (unsigned i = 0; i < 256 >> 2; i++) {
        if (!instr[i].exist)
            continue;

        for (unsigned j = i << 2; j < (i << 2) + 4; j++) {
            std::cout << instr[i].name << ' ';

            switch (instr[i].src) {
                case Operand::NONE:
                    break;

                case Operand::A:
                    std::cout << "A";
                    break;

                case Operand::REG:
                    std::cout << "R?";
                    break;

                case Operand::REGADDR:
                    std::cout << "@R?";
                    break;

                case Operand::IMMED:
                    std::cout << "#II";
                    break;

                case Operand::ADDR:
                    std::cout << "MM";
                    break;
            }

            switch (instr[i].dst) {
                case Operand::NONE:
                    break;

                case Operand::A:
                    std::cout << ", A";
                    break;

                case Operand::REG:
                    std::cout << ", R?";
                    break;

                case Operand::REGADDR:
                    std::cout << ", @R?";
                    break;

                case Operand::IMMED:
                    std::cout << ", #II";
                    break;

                case Operand::ADDR:
                    std::cout << ", MM";
                    break;
            }

            std::cout << " @ " <<
                      std::showbase << std::hex << std::setw(2) <<
                      j <<
                      std::dec << std::noshowbase << std::setw(0) <<
                      ": ";
#define GET_BIT(pos, name) \
    if(!um[j].test(23-pos))std::cout<<"!"#name" "
            GET_BIT(22, emwr);
            GET_BIT(21, emrd);
            GET_BIT(20, pcoe);
            GET_BIT(19, emen);
            GET_BIT(18, iren);
            GET_BIT(17, eint);
            GET_BIT(16, elp);
            GET_BIT(15, maren);
            GET_BIT(14, maroe);
            GET_BIT(13, outen);
            GET_BIT(12, sten);
            GET_BIT(11, rrd);
            GET_BIT(10, rwr);
            GET_BIT(9, cn);
            GET_BIT(8, fen);
            GET_BIT(7, x2);
            GET_BIT(6, x1);
            GET_BIT(5, x0);
            GET_BIT(4, wen);
            GET_BIT(3, aen);
            GET_BIT(2, s2);
            GET_BIT(1, s1);
            GET_BIT(0, s0);
#undef GET_BIT
            std::cout << ';' << std::endl;
        }
    }
}
