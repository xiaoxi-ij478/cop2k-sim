#include <fstream>
#include <iostream>
#include <sstream>

#include <endian.h>

#include "libopcode.hpp"

int main(int argc, char **argv)
{
    COP2K::Opcode opcode;
    std::array<COP2K::Opcode::Instruction, 64> ins;
    std::array<std::array<std::bitset<24>, 4>, 64> um;

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

    for (COP2K::Opcode::Instruction &i : ins) {
        if (!(i.exist = ifs.get()))
            continue;

        i.mnemonic.resize(ifs.get());
        ifs.read(i.mnemonic.data(), i.mnemonic.size());
        i.byte = ifs.get();
        i.src = static_cast<COP2K::Operand>(ifs.get());
        i.dst = static_cast<COP2K::Operand>(ifs.get());
        i.desc.resize(ifs.get());
        ifs.read(i.desc.data(), i.desc.size());
    }

    for (std::array<std::bitset<24>, 4> &u : um)
        for (std::bitset<24> &i : u) {
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

    for (unsigned i = 0; i < 64; i++)
        if (ins.at(i).exist)
            opcode.add(
                ins.at(i).byte,
                ins.at(i).mnemonic.c_str(),
                ins.at(i).desc.c_str(),
                ins.at(i).src,
                ins.at(i).dst,
                um.at(i)
            );

    std::cout << opcode.to_string();
}
