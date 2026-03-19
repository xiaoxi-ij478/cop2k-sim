#ifndef SIGNAL_EXPLAIN_HPP_INCLUDED
#define SIGNAL_EXPLAIN_HPP_INCLUDED

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <utility>

#include "libopcode_yacc.hpp" // struct Signals

namespace COP2K
{
    class SignalExplain
    {
        public:
            std::string explain(const struct Signals &signals) const
            {
                std::string abus_status;
                std::string ibus_status;
                std::string dbus_status;
                std::string alu_status;
                std::string other_status;
                std::string ret;

                if (!signals.emrd)
                    ibus_status.append("    Memory will write to instruction bus\n");

                if (!signals.pcoe)
                    abus_status.append("    Program counter will write to address bus\n");

                if (!signals.emen) {
                    if (!signals.emwr)
                        dbus_status.append("    Memory will read from data bus\n");

                    if (!signals.emrd)
                        dbus_status.append("    Memory will write to data bus\n");
                }

                if (!signals.iren)
                    ibus_status.append("    Instruction register and micro program counter will read from instruction bus\n");

                else
                    other_status.append("    Micro program counter will increase by 1\n");

                if (!signals.eint)
                    other_status.append("    Interrupt status will be cleared\n");

                if (!signals.elp)
                    dbus_status.append("    Program counter will read from data bus\n");

                else
                    other_status.append("    Program counter will increase by 1\n");

                if (!signals.maren)
                    dbus_status.append("    Memory address register will read from data bus\n");

                if (!signals.maroe)
                    dbus_status.append("    Memory address register will write to address bus\n");

                if (!signals.outen)
                    dbus_status.append("    OUT register will read from data bus\n");

                if (!signals.sten)
                    dbus_status.append("    Stack pointer will read from data bus\n");

                if (!signals.rrd)
                    dbus_status.append("    Memory address register will write to data bus\n");

                if (!signals.rwr)
                    dbus_status.append("    Memory address register will read from data bus\n");

                if (!signals.wen)
                    dbus_status.append("    'W' register will read from data bus\n");

                if (!signals.aen)
                    dbus_status.append("    'A' register will read from data bus\n");

                switch (signals.x2 << 2 | signals.x1 << 1 | signals.x0) {
                    case 0:
                        dbus_status.append("    IN register will write to data bus\n");
                        break;

                    case 1:
                        dbus_status.append("    Interrupt vector register will write to data bus\n");
                        break;

                    case 2:
                        dbus_status.append("    Stack pointer will write to data bus\n");
                        break;

                    case 3:
                        dbus_status.append("    Program counter will write to data bus\n");
                        break;

                    case 4:
                        dbus_status.append("    'D' register will write to data bus\n");
                        break;

                    case 5:
                        dbus_status.append("    'R' register will write to data bus\n");
                        break;

                    case 6:
                        dbus_status.append("    'L' register will write to data bus\n");
                        break;

                    case 7:
                        break;
                }

                switch (signals.s2 << 2 | signals.s1 << 1 | signals.s0) {
                    case 0:
                        alu_status.append("    ALU will perform add (A + W)\n");
                        break;

                    case 1:
                        alu_status.append("    ALU will perform sub (A - W)\n");
                        break;

                    case 2:
                        alu_status.append("    ALU will perform or (A | W)\n");
                        break;

                    case 3:
                        alu_status.append("    ALU will perform and (A & W)\n");
                        break;

                    case 4:
                        alu_status.append("    ALU will perform add with carry (A + W + C)\n");
                        break;

                    case 5:
                        alu_status.append("    ALU will perform sub with carry (A - W - C)\n");
                        break;

                    case 6:
                        alu_status.append("    ALU will perform not (~A)\n");
                        break;

                    case 7:
                        alu_status.append("    ALU will direct A (A)\n");
                        break;
                }

                ret.append("Data bus:\n");
                ret.append(std::move(dbus_status));
                ret.append("Address bus:\n");
                ret.append(std::move(abus_status));
                ret.append("Instruction bus:\n");
                ret.append(std::move(ibus_status));
                ret.append("ALU:\n");
                ret.append(std::move(alu_status));
                ret.append("Other:\n");
                ret.append(std::move(other_status));
                return ret;
            }
    };
}

#endif // SIGNAL_EXPLAIN_HPP_INCLUDED
