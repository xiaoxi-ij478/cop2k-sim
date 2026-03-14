#ifndef AS_HPP_INCLUDED
#define AS_HPP_INCLUDED

#include <algorithm>
#include <iterator>
#include <cstdio>
#include <stdexcept>
#include <stack>
#include <unordered_map>
#include <utility>

#include "libcop2k.hpp"

// we can't use fancy classes in Yacc's structs
struct InstructionOperand {
    COP2K::Operand src_type, dst_type;
    /*
     * for R?, R<arg>;
     * for @R? == R?;
     * for #II, arg == II;
     * for MM, arg == MM;
     * for A, arg is ignored
     */
    unsigned char src, dst;
};

namespace COP2K
{
    class AS;
    void assemble(FILE *in, AS *as, bool no_eval);

    class AS
    {
        public:
            AS() : overflow(false) {}

            constexpr void assemble_file(FILE *in)
            {
                // we use two-pass mode to scan for any label / constants
                consts.clear();
                em.clear();
                assemble(in, this, true);
                em.clear();
                rewind(in);
                assemble(in, this, false);
            }

            constexpr void clear()
            {
                consts.clear();
                em.clear();
                opcode.clear();
                overflow=false;
            }

            constexpr void add_instruction(
                const std::string &mnemonic,
                const std::string &label,
                const struct InstructionOperand &operand,
                unsigned lineno
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
                    mnemonic == "end" ||
                    mnemonic == "if" ||
                    mnemonic == "else" ||
                    mnemonic == "endif"
                )
                    return;

                else if (mnemonic == "db")
                    PUT_BYTE(operand.src);

                else if (mnemonic == "org") {
                    if (operand.src == 255)
                        overflow = true;

                    em.set_addr(operand.src);

                } else if (mnemonic == "00const")
                    // we use "construct if not exist, return if exists" feature of map
                    consts[label] = std::make_pair(operand.src, lineno);

                else if (mnemonic == "00label")
                    labels[label] = std::make_pair(em.get_addr(), lineno);

                else {
                    const Opcode::Instruction &ins = opcode.get_from_mnemonic(
                                                         mnemonic,
                                                         operand.src_type,
                                                         operand.dst_type
                                                     );

                    if (
                        operand.src_type == Operand::REG ||
                        operand.src_type == Operand::REGADDR
                    )
                        PUT_BYTE(ins.byte | operand.src);

                    else if (
                        operand.dst_type == Operand::REG ||
                        operand.dst_type == Operand::REGADDR
                    )
                        PUT_BYTE(ins.byte | operand.dst);

                    else
                        PUT_BYTE(ins.byte);

                    switch (operand.src_type) {
                        case Operand::NONE:
                        case Operand::REG_A:
                        case Operand::REG:
                        case Operand::REGADDR:
                            break;

                        case Operand::IMMED:
                        case Operand::MEMADDR:
                            PUT_BYTE(operand.src);
                            break;
                    }

                    switch (operand.dst_type) {
                        case Operand::NONE:
                        case Operand::REG_A:
                        case Operand::REG:
                        case Operand::REGADDR:
                            break;

                        case Operand::IMMED:
                        case Operand::MEMADDR:
                            PUT_BYTE(operand.dst);
                            break;
                    }
                }

#undef PUT_BYTE
            }

            Opcode opcode;
            Memory em;
            // *INDENT-OFF*
            std::unordered_map<
                std::string,
                std::pair<
                    unsigned char, // value
                    unsigned // line no
                >
            > consts;
            std::unordered_map<
                std::string,
                std::pair<
                    unsigned char, // value
                    unsigned // line no
                >
            > labels;
            // *INDENT-ON*
            bool overflow;
    };
}

#endif // AS_HPP_INCLUDED
