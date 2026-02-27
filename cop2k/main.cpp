#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <vector>

#include "cop2k.hpp"

namespace COP2K
{
    class CLI;

    struct CLICommand {
        virtual unsigned min_arg_len() const = 0;
        virtual unsigned max_arg_len() const = 0;
        virtual const char *help_string() const = 0;
        virtual void func(CLI &, const std::vector<std::string> &) = 0;
    };

    struct CLI {
        CLI() : request_quit(false) {}

        void cli_get_cmd()
        {
            CLICommand *cmd_func = nullptr;
            std::string line;
            std::vector<std::string> args;
            std::cout << "COP2K> ";
            std::getline(std::cin, line);

            if (line.empty())
                return;

            split_str(line, ' ', args);
            std::string cmd = args.front();
            args.erase(args.cbegin());

            try {
                cmd_func = &commands.at(cmd);

            } catch (const std::out_of_range &) {
                std::cerr << "error: command '" << cmd << "' does not exist." << std::endl;
                return;
            }

            if (!args.empty() && args.at(0) == "--help") {
                std::cerr << "usage: " << cmd_func->help_string() << std::endl;
                return;
            }

            if (
                args.size() < cmd_func->min_arg_len() ||
                args.size() > cmd_func->max_arg_len()
            ) {
                std::cerr <<
                          "error: wrong argument number. expected " <<
                          cmd_func->min_arg_len() << "~" << cmd_func->max_arg_len() <<
                          ", got " << args.size() << std::endl;
                return;
            }

            cmd_func->func(*this, args);
        }

        static std::vector<std::string> split_str(
            const std::string &str,
            char delim,
            std::vector<std::string> &dest
        )
        {
            std::istringstream iss(str);
            std::string tmp;
            dest.clear();

            while (std::getline(iss, tmp, delim))
                dest.emplace_back(tmp);

            return dest;
        }

        bool request_quit;
        const static std::unordered_map<std::string, CLICommand &> commands;
        COP2K machine;
    };

#define BEGIN_CLI_COMMAND(name, min_arg_len_val, max_arg_len_val, help_str) \
    struct CLI##name : CLICommand { \
        static CLI##name instance; \
        unsigned min_arg_len() const override { return min_arg_len_val; } \
        unsigned max_arg_len() const override { return max_arg_len_val; } \
        const char *help_string() const { return help_str; } \
        void func(CLI &cli, [[maybe_unused]] const std::vector<std::string> &args) override

#define END_CLI_COMMAND(name) \
private: \
    CLI##name() {} \
}; \
CLI##name CLI##name::instance = CLI##name();

    BEGIN_CLI_COMMAND(Help, 0, 1, "help [command name]")
    {
        CLICommand *cmd_func = nullptr;

        if (args.size() == 1) {
            try {
                cmd_func = &cli.commands.at(args.at(0));

            } catch (const std::out_of_range &) {
                std::cerr << "error: command '" << args.at(0) << "' does not exist." << std::endl;
                return;
            }

            std::cout << "'" << args.at(0) << "' usage: " << cmd_func->help_string() << std::endl;
            return;
        }

        for (const auto &i : cli.commands)
            std::cout << "'" << i.first << "' usage: " << i.second.help_string() << std::endl;
    }
    END_CLI_COMMAND(Help)

    BEGIN_CLI_COMMAND(Quit, 0, 0, "quit")
    {
        cli.request_quit = true;
    }
    END_CLI_COMMAND(Quit)

    BEGIN_CLI_COMMAND(Exit, 0, 0, "exit")
    {
        cli.request_quit = true;
    }
    END_CLI_COMMAND(Exit)

    BEGIN_CLI_COMMAND(SetFlag, 2, 2, "setflag <flag> {true|false}")
    {
        if (args.at(1) != "true" && args.at(1) != "false") {
            std::cerr <<
                      "error: argument 2 != true && != false." << std::endl <<
                      help_string() << std::endl;
            return;
        }

#define SET_FLAG(flag) \
    if (args.at(0) == #flag) \
        cli.machine.flag.set(args.at(1) == "true" ? true : false)
        SET_FLAG(emwr);
        else
            SET_FLAG(emrd);

        else
            SET_FLAG(pcoe);

        else
            SET_FLAG(emen);

        else
            SET_FLAG(iren);

        else
            SET_FLAG(eint);

        else
            SET_FLAG(elp);

        else
            SET_FLAG(maren);

        else
            SET_FLAG(maroe);

        else
            SET_FLAG(outen);

        else
            SET_FLAG(sten);

        else
            SET_FLAG(rrd);

        else
            SET_FLAG(rwr);

        else
            SET_FLAG(x2);

        else
            SET_FLAG(x1);

        else
            SET_FLAG(x0);

        else
            SET_FLAG(wen);

        else
            SET_FLAG(aen);

        else
            SET_FLAG(s2);

        else
            SET_FLAG(s1);

        else
            SET_FLAG(s0);

        else
            SET_FLAG(sa);

        else
            SET_FLAG(sb);

        else
            SET_FLAG(ireq);

        else
            SET_FLAG(iack);

        else
            SET_FLAG(halt);

        else
            std::cerr << "error: no such flag: '" << args.at(0) << '\'' << std::endl;

#undef SET_FLAG
    }
    END_CLI_COMMAND(SetFlag)

    BEGIN_CLI_COMMAND(GetFlag, 0, 1, "getflag [flag]")
    {
        if (args.empty()) {
#define GET_FLAG(flag) \
    #flag ": " << cli.machine.flag.get() << std::endl
            std::cout <<
                      GET_FLAG(emwr) <<
                      GET_FLAG(emrd) <<
                      GET_FLAG(pcoe) <<
                      GET_FLAG(emen) <<
                      GET_FLAG(iren) <<
                      GET_FLAG(eint) <<
                      GET_FLAG(elp) <<
                      GET_FLAG(maren) <<
                      GET_FLAG(maroe) <<
                      GET_FLAG(outen) <<
                      GET_FLAG(sten) <<
                      GET_FLAG(rrd) <<
                      GET_FLAG(rwr) <<
                      GET_FLAG(x2) <<
                      GET_FLAG(x1) <<
                      GET_FLAG(x0) <<
                      GET_FLAG(wen) <<
                      GET_FLAG(aen) <<
                      GET_FLAG(s2) <<
                      GET_FLAG(s1) <<
                      GET_FLAG(s0) <<
                      GET_FLAG(sa) <<
                      GET_FLAG(sb) <<
                      GET_FLAG(ireq) <<
                      GET_FLAG(iack) <<
                      GET_FLAG(halt) <<
                      GET_FLAG(manual_dbus) <<
                      GET_FLAG(running_manually);
#undef GET_FLAG
            return;
        }

#define GET_FLAG(flag) \
    if (args.at(0) == #flag) \
        std::cout << #flag ": " << cli.machine.flag.get() << std::endl
        GET_FLAG(emwr);
        else
            GET_FLAG(emrd);

        else
            GET_FLAG(pcoe);

        else
            GET_FLAG(emen);

        else
            GET_FLAG(iren);

        else
            GET_FLAG(eint);

        else
            GET_FLAG(elp);

        else
            GET_FLAG(maren);

        else
            GET_FLAG(maroe);

        else
            GET_FLAG(outen);

        else
            GET_FLAG(sten);

        else
            GET_FLAG(rrd);

        else
            GET_FLAG(rwr);

        else
            GET_FLAG(x2);

        else
            GET_FLAG(x1);

        else
            GET_FLAG(x0);

        else
            GET_FLAG(wen);

        else
            GET_FLAG(aen);

        else
            GET_FLAG(s2);

        else
            GET_FLAG(s1);

        else
            GET_FLAG(s0);

        else
            GET_FLAG(sa);

        else
            GET_FLAG(sb);

        else
            GET_FLAG(ireq);

        else
            GET_FLAG(iack);

        else
            GET_FLAG(halt);

        else
            GET_FLAG(manual_dbus);

        else
            GET_FLAG(running_manually);

        else
            std::cerr << "error: no such flag: '" << args.at(0) << '\'' << std::endl;

#undef GET_FLAG
    }
    END_CLI_COMMAND(GetFlag)

    BEGIN_CLI_COMMAND(Clock, 0, 1, "clock [count]")
    {
        unsigned clock_count = args.empty() ? 1 : std::stoi(args.at(0));

        while (clock_count--)
            cli.machine.run_clock();
    }
    END_CLI_COMMAND(Clock)

    BEGIN_CLI_COMMAND(GetReg, 0, 1, "getreg [reg]")
    {
        if (args.empty()) {
#define GET_REG(reg) \
    #reg ": " << static_cast<unsigned>(cli.machine.reg.get()) << std::endl
            std::cout <<
                      GET_REG(manual_dbus_input) <<
                      GET_REG(upc) <<
                      GET_REG(pc) <<
                      GET_REG(mar) <<
                      GET_REG(ia) <<
                      GET_REG(st) <<
                      GET_REG(in) <<
                      GET_REG(out) <<
                      GET_REG(ir) <<
                      GET_REG(l) <<
                      GET_REG(d) <<
                      GET_REG(r) <<
                      GET_REG(a) <<
                      GET_REG(w) <<
                      GET_REG(r0) <<
                      GET_REG(r1) <<
                      GET_REG(r2) <<
                      GET_REG(r3);
#undef GET_REG
            return;
        }

#define GET_REG(reg) \
    if (args.at(0) == #reg) \
        std::cout << #reg ": " << static_cast<unsigned>(cli.machine.reg.get()) << std::endl
        GET_REG(manual_dbus_input);
        else
            GET_REG(upc);

        else
            GET_REG(pc);

        else
            GET_REG(mar);

        else
            GET_REG(ia);

        else
            GET_REG(st);

        else
            GET_REG(in);

        else
            GET_REG(out);

        else
            GET_REG(ir);

        else
            GET_REG(l);

        else
            GET_REG(d);

        else
            GET_REG(r);

        else
            GET_REG(a);

        else
            GET_REG(w);

        else
            GET_REG(r0);

        else
            GET_REG(r1);

        else
            GET_REG(r2);

        else
            GET_REG(r3);

        else
            std::cerr << "error: no such register: '" << args.at(0) << '\'' << std::endl;

#undef GET_REG
    }
    END_CLI_COMMAND(GetReg)

    BEGIN_CLI_COMMAND(SetReg, 2, 2, "setreg <reg> <val>")
    {
        unsigned val = std::stoi(args.at(1));

        if (val > 255) {
            std::cerr << "error: val > 255." << std::endl;
            return;
        }

#define SET_REG(reg) \
    if (args.at(0) == #reg) \
        cli.machine.reg.set(val)
        SET_REG(manual_dbus_input);
        else
            SET_REG(upc);

        else
            SET_REG(pc);

        else
            SET_REG(mar);

        else
            SET_REG(ia);

        else
            SET_REG(st);

        else
            SET_REG(in);

        else
            SET_REG(out);

        else
            SET_REG(ir);

        else
            SET_REG(l);

        else
            SET_REG(d);

        else
            SET_REG(r);

        else
            SET_REG(a);

        else
            SET_REG(w);

        else
            SET_REG(r0);

        else
            SET_REG(r1);

        else
            SET_REG(r2);

        else
            SET_REG(r3);

        else
            std::cerr << "error: no such register: '" << args.at(0) << '\'' << std::endl;

#undef SET_REG
    }
    END_CLI_COMMAND(SetReg)

    BEGIN_CLI_COMMAND(WriteMem, 2, 2, "writemem <addr> <val>")
    {
        unsigned addr = std::stoi(args.at(0)), val = std::stoi(args.at(1));

        if (addr > 255) {
            std::cerr << "error: addr > 255." << std::endl;
            return;
        }

        if (val > 255) {
            std::cerr << "error: val > 255." << std::endl;
            return;
        }

        cli.machine.em.set_data_at(addr, val);
    }
    END_CLI_COMMAND(WriteMem)

    BEGIN_CLI_COMMAND(ReadMem, 0, 1, "readmem [addr]")
    {
        if (args.empty()) {
            for (unsigned i = 0; i < 16; i++) {
                std::cout << (i << 4) << ": " << std::endl;

                for (unsigned j = 0; j < 16; j++)
                    std::cout << static_cast<unsigned>(cli.machine.em.get_data_at(i << 4 | j)) << ' ';

                std::cout << std::endl;
            }

            return;
        }

        unsigned addr = std::stoi(args.at(0));
        if (addr > 255) {
            std::cerr << "error: addr > 255." << std::endl;
            return;
        }

        std::cout << addr << ": " << static_cast<unsigned>(cli.machine.em.get_data_at(addr)) << std::endl;
    }
    END_CLI_COMMAND(ReadMem)

#undef BEGIN_CLI_COMMAND
#undef END_CLI_COMMAND

#define COMMAND(cmd_str, name) { #cmd_str, CLI##name::instance }
    const std::unordered_map<std::string, CLICommand &> CLI::commands = {
        COMMAND(setflag, SetFlag),
        COMMAND(getflag, GetFlag),
        COMMAND(getreg, GetReg),
        COMMAND(setreg, SetReg),
        COMMAND(clock, Clock),
        COMMAND(writemem, WriteMem),
        COMMAND(readmem, ReadMem),
        COMMAND(quit, Quit),
        COMMAND(exit, Exit),
        COMMAND(help, Help)
    };
#undef COMMAND
}

int main()
{
    std::cout << std::boolalpha << std::hex << std::showbase;
    COP2K::CLI cli;

    while (!cli.request_quit)
        cli.cli_get_cmd();
}
