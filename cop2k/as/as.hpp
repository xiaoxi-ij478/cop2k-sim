#ifndef AS_HPP_INCLUDED
#define AS_HPP_INCLUDED

#include <cstdio>
#include <stdexcept>
#include <stack>
#include <unordered_map>

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
    void assemble(FILE *in, AS *as);

    class AS
    {
        public:
            AS() : stop(false), overflow(false) {}

            constexpr void assemble_file(FILE *in)
            {
                em.clear();
                assemble(in, this);
            }

            constexpr void add_instruction(
                const char *mnemonic,
                const char *label,
                const struct InstructionOperand &operand
            )
            {
                if (stop)
                    return;

#define PUT_BYTE(b) \
    do { \
        if (overflow) \
            throw std::out_of_range("em overflow");\
        em.set_data(b); \
        if (em.get_addr() == 255) \
            overflow = true; \
        em.set_addr(em.get_addr() + 1); \
    } while (0)

                if (!strcasecmp(mnemonic, "end"))
                    stop = true;

                else if (
                    !strcasecmp(mnemonic, "if") ||
                    !strcasecmp(mnemonic, "else") ||
                    !strcasecmp(mnemonic, "endif")
                )
                    return;

                if (!strcasecmp(mnemonic, "db"))
                    PUT_BYTE(operand.src);

                else if (!strcasecmp(mnemonic, "org"))
                    em.set_addr(operand.src);

                else if (
                    !strcasecmp(mnemonic, "00const") ||
                    !strcasecmp(mnemonic, "00label")
                )
                    consts.emplace(label, operand.src);

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
            bool stop;
            bool overflow;
            std::unordered_map<std::string, unsigned char> consts;
    };
}

#endif // AS_HPP_INCLUDED
