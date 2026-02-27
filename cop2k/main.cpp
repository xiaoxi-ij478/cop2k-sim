#include <functional>
#include <iomanip>
#include <iostream>
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

    class CLI
    {
            friend struct CLIQuit;
            friend struct CLISetFlag;
            friend struct CLIGetFlag;
            friend struct CLIHelp;

        public:
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

            bool request_quit;

        private:
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

            const static std::unordered_map<std::string, CLICommand &> commands;
            COP2K machine;
    };

#define BEGIN_CLI_COMMAND(name) \
    struct CLI##name : CLICommand { \
        static CLI##name instance;

#define SET_CLI_MIN_ARG_LEN(len) \
    unsigned min_arg_len() const override { return len; }

#define SET_CLI_MAX_ARG_LEN(len) \
    unsigned max_arg_len() const override { return len; }

#define SET_CLI_REQUIRED_ARG_LEN(len) \
    SET_CLI_MIN_ARG_LEN(len) \
    SET_CLI_MAX_ARG_LEN(len)

#define SET_CLI_HELP_STRING(str) \
    const char *help_string() const { return str; }

#define DEFINE_CLI_FUNC() \
    void func(CLI &cli, [[maybe_unused]] const std::vector<std::string> &args) override

#define END_CLI_COMMAND(name) \
private: \
    CLI##name() {} \
}; \
CLI##name CLI##name::instance = CLI##name();

    BEGIN_CLI_COMMAND(Quit)
    SET_CLI_REQUIRED_ARG_LEN(0)
    SET_CLI_HELP_STRING("quit")

    DEFINE_CLI_FUNC()
    {
        cli.request_quit = true;
    }
    END_CLI_COMMAND(Quit)

    BEGIN_CLI_COMMAND(SetFlag)
    SET_CLI_REQUIRED_ARG_LEN(2)
    SET_CLI_HELP_STRING("set_flag <flag> {true|false}")

    DEFINE_CLI_FUNC()
    {
        if (args.at(1) != "true" && args.at(1) != "false") {
            std::cerr <<
                      "error: argument 2 != true && != false." << std::endl <<
                      help_string() << std::endl;
            return;
        }

#define SET_FLAG(flag) \
    if (args.at(0) == #flag) \
        cli.machine.set_##flag(args.at(1) == "true" ? true : false)
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

    BEGIN_CLI_COMMAND(GetFlag)
    SET_CLI_MIN_ARG_LEN(0)
    SET_CLI_MAX_ARG_LEN(1)
    SET_CLI_HELP_STRING("get_flag [flag]")

    DEFINE_CLI_FUNC()
    {
        if (args.empty()) {
#define GET_FLAG(flag) \
    #flag ": " << cli.machine.get_##flag() << std::endl
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
                      GET_FLAG(halt);
#undef GET_FLAG
            return;
        }

#define GET_FLAG(flag) \
    if (args.at(0) == #flag) \
        cli.machine.set_##flag(args.at(1) == "true" ? true : false)
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
            std::cerr << "error: no such flag: '" << args.at(0) << '\'' << std::endl;

#undef GET_FLAG
    }
    END_CLI_COMMAND(GetFlag)

    BEGIN_CLI_COMMAND(Help)
    SET_CLI_MIN_ARG_LEN(0)
    SET_CLI_MAX_ARG_LEN(1)
    SET_CLI_HELP_STRING("help [command name]")
    DEFINE_CLI_FUNC()
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

#define COMMAND(cmd_str, name) { #cmd_str, CLI##name::instance }
    const std::unordered_map<std::string, CLICommand &> CLI::commands = {
        COMMAND(set_flag, SetFlag),
        COMMAND(get_flag, GetFlag),
        COMMAND(quit, Quit),
        COMMAND(help, Help)
    };

#undef BEGIN_CLI_COMMAND
#undef SET_CLI_MIN_ARG_LEN
#undef SET_CLI_MAX_ARG_LEN
#undef SET_CLI_REQUIRED_ARG_LEN
#undef SET_CLI_HELP_STRING
#undef DEFINE_CLI_FUNC
#undef END_CLI_COMMAND
#undef COMMAND
}

int main()
{
    std::cout << std::boolalpha;
    COP2K::CLI cli;

    while (!cli.request_quit)
        cli.cli_get_cmd();
}
