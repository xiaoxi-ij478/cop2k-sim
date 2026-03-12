#ifndef LIBOPCODE_HPP_INCLUDED
#define LIBOPCODE_HPP_INCLUDED

#include <algorithm>
#include <stdexcept>
#include <format>
#include <cstring>
#include <bitset>
#include <array>
#include <sstream>

namespace COP2K
{
    enum class Operand : unsigned char {
        NONE,
        REG_A,
        REG,
        REGADDR,
        IMMED,
        MEMADDR
    };

    class Opcode;
    void parse_instruction_file(FILE *in, Opcode *opcode);

    class Opcode
    {
            struct Instruction {
                bool exist;
                unsigned char byte;
                std::string mnemonic;
                std::string desc;
                Operand src, dst;
                unsigned char signal_count;
                std::array<std::bitset<24>, 4> microprogram;

                constexpr Instruction() :
                    exist(false),
                    byte(0),
                    src(Operand::NONE),
                    dst(Operand::NONE),
                    signal_count(0)
                {
                    for (std::bitset<24> &i : microprogram)
                        i.set();
                }

                constexpr void clear()
                {
                    exist = false;
                    byte = 0;
                    mnemonic.clear();
                    desc.clear();
                    src = dst = Operand::NONE;
                    signal_count = 0;

                    for (std::bitset<24> &i : microprogram)
                        i.set();
                }
            };

        public:
            constexpr Opcode()
            {
                clear();
            }

            // WARNING:
            // this function won't do sanity check on the arguments,
            // so make sure all arguments are correct!
            constexpr void add(
                unsigned char byte,
                const char *mnemonic,
                const char *desc,
                Operand src,
                Operand dst,
                unsigned char signal_count,
                const std::array<std::bitset<24>, 4> &microprogram
            )
            {
                if (instructions.at(byte >> 2).exist)
                    throw std::logic_error(
                        std::format("instruction at 0x{:02X} already defined", byte)
                    );

                instructions.at(byte >> 2).exist = true;
                instructions.at(byte >> 2).byte = byte;
                instructions.at(byte >> 2).mnemonic = mnemonic;
                instructions.at(byte >> 2).desc = desc;
                instructions.at(byte >> 2).src = src;
                instructions.at(byte >> 2).dst = dst;
                instructions.at(byte >> 2).signal_count = signal_count;
                instructions.at(byte >> 2).microprogram = microprogram;
            }

            constexpr void load_instr_txt(FILE *in)
            {
                clear();
                parse_instruction_file(in, this);

                if (!instructions.at(0x0).exist)
                    throw std::logic_error(
                        "instruction @ 0x0 MUST be _FATCH_ with no operands"
                    );

                if (!instructions.at(0xB8).exist)
                    throw std::logic_error(
                        "instruction @ 0xB8 MUST be _INT_ with no operands"
                    );
            }

            constexpr void clear()
            {
                for (Instruction &i : instructions)
                    i.clear();
            }

            constexpr const Instruction &get_from_mnemonic(
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

            constexpr const Instruction &get_from_byte(unsigned char byte) const
            {
                for (const Instruction &i : instructions)
                    if (i.exist && i.byte == byte)
                        return i;

                throw std::out_of_range(
                    std::format("instruction 0x{:02X} undefined", byte)
                );
            }

            constexpr void patch_um(uint8_t addr, const std::bitset<24> &val) {
                Instruction &ins = instructions.at(addr >> 2);

                if (!ins.exist)
                    throw std::out_of_range(
                        std::format("micro program address 0x{:02X} not bound to any instructions", addr)
                    );
                
                ins.microprogram.at(addr & 3) = val;

                if (addr & 3 == ins.signal_count - 1)
                    if (val.all())
                        ins.signal_count--;
                    else
                        ins.signal_count++;
            }

            constexpr void patch_um(uint8_t addr, unsigned bit_pos, bool val) {
                Instruction &ins = instructions.at(addr >> 2);
                if (!ins.exist)
                    throw std::out_of_range(
                        std::format("micro program address 0x{:02X} not bound to any instructions", addr)
                    );
                
                insmicroprogram.at(addr & 3).set(bit_pos, val);
                
                if (addr & 3 == ins.signal_count - 1)
                    if (ins.microprogram.at(addr & 3).all())
                        ins.signal_count--;
                    else
                        ins.signal_count++;
            }

            constexpr const std::array<Instruction>::const_iterator *begin() const
            {
                return instructions.cbegin();
            }

            constexpr const std::array<Instruction>::const_iterator *end() const
            {
                return instructions.cend();
            }

            constexpr std::string dump() const
            {
                std::ostringstream oss;

                for (const Instruction &i : instructions) {
                    if (!i.exist)
                        continue;

                    oss << i.mnemonic << ' ';

                    switch (i.src) {
                        case Operand::NONE:
                            break;

                        case Operand::REG_A:
                            oss << "A";
                            break;

                        case Operand::REG:
                            oss << "R?";
                            break;

                        case Operand::REGADDR:
                            oss << "@R?";
                            break;

                        case Operand::IMMED:
                            oss << "#II";
                            break;

                        case Operand::MEMADDR:
                            oss << "MM";
                            break;
                    }

                    switch (i.dst) {
                        case Operand::NONE:
                            break;

                        case Operand::REG_A:
                            oss << ", A";
                            break;

                        case Operand::REG:
                            oss << ", R?";
                            break;

                        case Operand::REGADDR:
                            oss << ", @R?";
                            break;

                        case Operand::IMMED:
                            oss << ", #II";
                            break;

                        case Operand::MEMADDR:
                            oss << ", MM";
                            break;
                    }

                    oss << std::format(" @ 0x{:02X}: ", i.byte);

                    if (!i.desc.empty())
                        oss << "// " << i.desc;

                    oss << std::endl;

                    unsigned a=0;
                    for (const std::bitset<24> &j : i.microprogram) {
                        std::ostringstream oss;
#define GET_BIT(pos, name) \
    if (!j.test(23 - pos)) oss << "!" #name " "
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

                        if (!oss.str().empty())
                            oss << "    " << a++ << ": "
                                      << oss.str()
                                      << std::endl;
                    }

                    oss << ';' << std::endl;
                }

                return oss.str();
            }

        private:
            std::array<Instruction, 256 >> 2> instructions;
    };
}

#endif // LIBOPCODE_HPP_INCLUDED
