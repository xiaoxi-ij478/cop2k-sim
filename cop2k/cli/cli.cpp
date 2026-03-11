#include "cli.hpp"

int main()
{
    std::cout << std::boolalpha << std::hex << std::showbase;
    COP2K::CLI cli;

    while (!cli.request_quit)
        cli.cli_get_cmd();
}
