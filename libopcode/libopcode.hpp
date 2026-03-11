#ifndef LIBOPCODE_HPP_INCLUDED
#define LIBOPCODE_HPP_INCLUDED

#include <algorithm>
#include <stdexcept>
#include <format>
#include <cstring>
#include <bitset>
#include <array>

namespace COP2K
{
    void parse_instruction_file(FILE *in, Opcode &opcode);

    enum class Operand : unsigned char {
        NONE,
        REG_A,
        REG,
        REGADDR,
        IMMED,
        MEMADDR
    };

    class Opcode
    {
            struct Instruction {
                bool exist;
                unsigned char byte;
                std::string mnemonic;
                Operand src, dst;
                std::array<std::bitset<24>, 4> microprogram;

                Instruction() :
                    exist(false),
                    byte(0),
                    src(Operand::NONE),
                    dst(Operand::NONE)
                {
                    for (std::bitset<24> &i : microprogram)
                        i.set();
                }

                void clear()
                {
                    exist = false;
                    byte = 0;
                    mnemonic.clear();
                    src = dst = Operand::NONE;

                    for (std::bitset<24> &i : microprogram)
                        i.set();
                }
            };

        public:
            Opcode() : instruction_numbers(0)
            {
                clear();
            }

            // WARNING:
            // this function won't do sanity check on the arguments,
            // so make sure all arguments are correct!
            void add(
                unsigned char byte,
                const char *mnemonic,
                Operand src,
                Operand dst,
                const std::array<std::bitset<24>, 4> &microprogram
            )
            {
                if (instructions[byte >> 2].exist)
                    throw std::logic_error(
                        std::format("instruction at 0x{:02X} already defined", byte)
                    );

                instructions[byte >> 2].exist = true;
                instructions[byte >> 2].byte = byte;
                instructions[byte >> 2].mnemonic = mnemonic;
                instructions[byte >> 2].src = src;
                instructions[byte >> 2].dst = dst;
                instructions[byte >> 2].microprogram = microprogram;
            }

            void load_instr_txt(FILE *in)
            {
                clear();
                parse_instruction_file(in, *this);
            }

            void clear()
            {
                for (Instruction &i : instructions)
                    i.clear();
            }

            const Instruction &get_from_mnemonic(
                const char *mnemonic,
                Operand src,
                Operand dst
            ) const
            {
                for (const Instruction &i : instructions)
                    if (i.exist && i.mnemonic == mnemonic && i.src == src && i.dst == dst)
                        return i;

                throw std::out_of_range(
                    std::format("instruction {} undefined", mnemonic)
                );
            }

            const Instruction &get_from_byte(unsigned char byte) const
            {
                for (const Instruction &i : instructions)
                    if (i.exist && i.byte == byte)
                        return i;

                throw std::out_of_range(
                    std::format("instruction 0x{:02X} undefined", byte)
                );
            }

            const Instruction *begin() const
            {
                return instructions;
            }

            const Instruction *end() const
            {
                return instructions + (256 << 2);
            }

        private:
            OpcodeUtil::Instruction instructions[256 >> 2];
    };

}

#endif // LIBOPCODE_HPP_INCLUDED
