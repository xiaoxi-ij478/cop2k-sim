#ifndef DIS_HPP_INCLUDED
#define DIS_HPP_INCLUDED

#include <iostream>
#include <vector>
#include <utility>
#include <sstream>
#include <map>

#include "libopcode.hpp"

namespace COP2K
{
    class DIS
    {
        // NOTE: this decompiler will aggressively treat everything as code
        // rather than as data
        public:
            std::string disassemble(std::istream &in) const
            {
                struct Line {
                    std::string mnemonic;
                    std::string operand;
                    std::string label;
                    std::string byte;
                    bool src_is_memaddr;
                    unsigned char src_memaddr;
                    bool dst_is_memaddr;
                    unsigned char dst_memaddr;
                };
                std::ostringstream oss;
                std::map<unsigned char, struct Line> lines;
                unsigned label_count = 0;
                unsigned cur_pos;



                while ((cur_pos = in.tellg()) < 256) {
                    bool src_is_memaddr = false, dst_is_memaddr = false;
                    unsigned char src_memaddr = 0, dst_memaddr = 0;
                    const Opcode::Instruction *ins = nullptr;
                    unsigned char byte = in.get();
                    unsigned char tmpbyte = 0;
                    std::string ins_operand, b(std::format("{:02X}H", byte));

                    if (!byte) // do not put so many _FATCH_es
                        continue;

                    try {
                        ins = &opcode.get_from_byte(byte & ~0x3);

                        if (ins.mnemonic == "_FATCH_")
                            throw std::out_of_range();

                    } catch (const std::out_of_range &) {
                        lines.emplace(
                            cur_pos,
                            Line(
                                std::string("DB"),
                                std::format("{:02X}H", byte),
                                std::string(),
                                std::move(b),
                                false,
                                0,
                                false,
                                0
                            )
                        );
                        continue;
                    }

                    switch (ins->src) {
                        case Operand::NONE:
                            break;

                        case Operand::REG_A:
                            ins_operand.append("A");
                            break;

                        case Operand::IMMED:
                            ins_operand.append(std::format("#{:02X}H", (tmpbyte = in.get())));
                            b.append(std::format("{:02X}H", tmpbyte));
                            break;

                        case Operand::MEMADDR:
                            src_is_memaddr = true;
                            src_memaddr = in.get();
                            b.append(std::format("{:02X}H", src_memaddr));
                            break;

                        case Operand::REG:
                            ins_operand.append(std::format("R{}", byte & 0x3));
                            break;

                        case Operand::REGADDR:
                            ins_operand.append(std::format("@R{}", byte & 0x3));
                            break;
                    }

                    switch (ins->dst) {
                        case Operand::NONE:
                            break;

                        case Operand::REG_A:
                            ins_operand.append(", A");
                            break;

                        case Operand::IMMED:
                            ins_operand.append(std::format(", #{:02X}H", (tmpbyte = in.get())));
                            b.append(std::format("{:02X}H", tmpbyte));
                            break;

                        case Operand::MEMADDR:
                            dst_is_memaddr = true;
                            dst_memaddr = in.get();
                            ins_operand.append(", ");
                            b.append(std::format("{:02X}H", dst_memaddr));
                            break;

                        case Operand::REG:
                            ins_operand.append(std::format(", R{}", byte & 0x3));
                            break;

                        case Operand::REGADDR:
                            ins_operand.append(std::format(", @R{}", byte & 0x3));
                            break;
                    }

                    lines.emplace(
                        cur_pos,
                        Line(
                            ins->mnemonic,
                            std::move(ins_operand),
                            std::string(),
                            src_is_memaddr,
                            src_memaddr,
                            dst_is_memaddr,
                            dst_memaddr
                        )
                    );
                }

                for (std::pair<const unsigned char, struct Line> &i : lines) {
                    if (i.second.src_is_memaddr) {
                        try {
                            std::string &label = lines.at(i.second.src_memaddr).label;

                            if (label.empty())
                                label = std::format("L{}", label_count++);

                            i.second.operand.insert(0, label);

                        } catch (const std::out_of_range &) {
                            i.second.operand.insert(0, std::format("{:02X}", i.second.src_memaddr));
                        }
                    }

                    if (i.second.dst_is_memaddr) {
                        try {
                            std::string &label = lines.at(i.second.dst_memaddr).label;

                            if (label.empty())
                                label = std::format("L{}", label_count++);

                            i.second.operand.append(label);

                        } catch (const std::out_of_range &) {
                            i.second.operand.append(std::format("{:02X}", i.second.dst_memaddr));
                        }
                    }
                }

                for (const std::pair<const unsigned char, struct Line> &i : lines) {
                    if (!i.second.label.empty())
                        oss << i.second.label << ':' << std::endl;

                    oss << "  " << i.second.mnemonic << ' ' << i.second.operand << std::endl;
                }

                return oss.str();
            }

            Opcode opcode;
    };
}

#endif // DIS_HPP_INCLUDED
