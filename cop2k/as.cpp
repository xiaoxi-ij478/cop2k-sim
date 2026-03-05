#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cctype>
#include <utility>
#include <algorithm>
#include <regex>
#include <iterator>

#include "cop2k.hpp"

enum class ParseStatus {
    MNEMONIC,
    SRC,
    COMMA,
    DST,
    AT,
    ADDR,
    COLON,
    SIGNALS
};
enum class Operand : unsigned char {
    NONE, A, REG, REGADDR, IMMED, ADDR
};
enum class AsmParseStatus {
    LABEL,

};

namespace std
{
    template<typename T>
    regex_token_iterator<T> begin(regex_token_iterator<T> &c)
    {
        return c;
    }

    template<typename T>
    regex_token_iterator<T> end(regex_token_iterator<T> &)
    {
        return regex_token_iterator<T>();
    }

    template<typename T>
    regex_iterator<T> begin(regex_iterator<T> &c)
    {
        return c;
    }

    template<typename T>
    regex_iterator<T> end(regex_iterator<T> &)
    {
        return regex_iterator<T>();
    }
}
struct Instruction {
    bool exist = false;
    unsigned char byte;
    std::string name;
    std::string desc;
    Operand src, dst;
} instr[256 >> 2];

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "usage: as <instr.ins> <file.asm> [-o <out.bin>]" << std::endl;
        return EXIT_FAILURE;
    }

    COP2K::MicroProgramMemory um;
    std::ifstream instr_ifs(argv[1]), asm_ifs(argv[2]);
    std::string instr_str, asm_str;
    std::copy(
        std::istreambuf_iterator<char>(instr_ifs.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::back_inserter(instr_str)
    );
    std::copy(
        std::istreambuf_iterator<char>(asm_ifs.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::back_inserter(asm_str)
    );
    std::ofstream out_ofs;

    if (argc == 5) {
        if (!strcmp(argv[3], "-o")) {
            std::cerr << "usage: as <instr.ins> <file.asm> [-o <out.bin>]" << std::endl;
            return EXIT_FAILURE;
        }

        out_ofs.open(argv[4]);
        std::cout.rdbuf(out_ofs.rdbuf());
    }

    std::string word;
    std::string temp_menmonic;
    Operand temp_src, temp_dst;
    int temp_addr;
    ParseStatus status;
    std::regex instr_regex(
        "("
        "[a-z]+|" // mnemonic
        ",|"
        "@|"
        ":|"
        ";|"
        "[1-9][0-9]*|"
        "0x[0-9a-f]+|"
        "0[0-7]+|"
        "A|R?|@R?|#II|MM|" // src, dst
        "!?(?:s0|s1|s2|aen|wen|x0|x1|x2|fen|cn|rwr|rrd|sten|outen|maroe|maren|elp|eint|iren|emen|pcoe|emrd|emwr)|" // signals
        "\\s+|"
        "[^\\s]*" // anything else
        ")",
        std::regex_constants::ECMAScript | std::regex_constants::icase
    );
    std::sregex_iterator instr_regex_iterator(
        instr_str.cbegin(),
        instr_str.cend(),
        instr_regex
    );

    for (const auto &i : instr_regex_iterator) {
        std::cout << std::boolalpha<<i.empty()<<' '<<i.str() << std::endl;
        /*switch (status) {
            case ParseStatus::MNEMONIC:
                std::transform(word.begin(), word.end(), word.begin(), toupper);

                if (word.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
                    std::cerr << "error: menmonic has characters other than alphabets: '" << word <<
                              "'." << std::endl;
                    return EXIT_FAILURE;
                }

                temp_menmonic = std::move(word);
                status = ParseStatus::SRC;
                break;

            case ParseStatus::SRC:
                if (word == "@")
                    // no src, no dst
                    temp_src = temp_dst = Operand::NONE;

                else if (word == "A")
                    temp_src = Operand::A;

                else if (word == "R?")
                    temp_src = Operand::REG;

                else if (word == "@R?")
                    temp_src = Operand::REGADDR;

                else if (word == "#II")
                    temp_src = Operand::IMMED;

                else if (word == "MM")
                    temp_src = Operand::ADDR;

                else {
                    std::cerr << "error: src invalid: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                status = ParseStatus::COMMA;
                break;

            case ParseStatus::COMMA:
                if (word != ",") {
                    std::cerr << "error: invalid token: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                status = ParseStatus::DST;
                break;

            case ParseStatus::DST:
                if (word == "A")
                    temp_dst = Operand::A;

                else if (word == "R?")
                    temp_dst = Operand::REG;

                else if (word == "@R?")
                    temp_dst = Operand::REGADDR;

                else if (word == "#II")
                    temp_dst = Operand::IMMED;

                else if (word == "MM")
                    temp_dst = Operand::ADDR;

                else {
                    std::cerr << "error: dst invalid: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                status = ParseStatus::AT;
                break;

            case ParseStatus::AT:
                if (word != "@") {
                    std::cerr << "error: invalid token: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                status = ParseStatus::ADDR;
                break;

            case ParseStatus::ADDR:
                temp_addr = std::stoi(word);

                if (temp_addr < 0) {
                    std::cerr << "error: addr < 0: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                if (temp_addr > 255) {
                    std::cerr << "error: addr > 255: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                if (temp_addr & 3) {
                    std::cerr << "error: addr not aligned to 4 bits: '" << word << "'." <<
                              std::endl;
                    return EXIT_FAILURE;
                }

                status = ParseStatus::COLON;
                break;

            case ParseStatus::COLON:
                if (word != ":") {
                    std::cerr << "error: invalid token: '" << word << "'." << std::endl;
                    return EXIT_FAILURE;
                }

                status = ParseStatus::SIGNALS;
                break;

            case ParseStatus::SIGNALS:
        #define SET_BIT(pos, name) \
        if (word == #name) \
        um.set_data_at(temp_addr, pos, true); \
        else if (word == "!" #name) \
        um.set_data_at(temp_addr, pos, false)
                SET_BIT(0, s0);

                else
                    SET_BIT(1, s1);

                else
                    SET_BIT(2, s2);

                else
                    SET_BIT(3, aen);

                else
                    SET_BIT(4, wen);

                else
                    SET_BIT(5, x0);

                else
                    SET_BIT(6, x1);

                else
                    SET_BIT(7, x2);

                else
                    SET_BIT(8, fen);

                else
                    SET_BIT(9, cn);

                else
                    SET_BIT(10, rwr);

                else
                    SET_BIT(11, rrd);

                else
                    SET_BIT(12, sten);

                else
                    SET_BIT(13, outen);

                else
                    SET_BIT(14, maroe);

                else
                    SET_BIT(15, maren);

                else
                    SET_BIT(16, elp);

                else
                    SET_BIT(17, eint);

                else
                    SET_BIT(18, iren);

                else
                    SET_BIT(19, emen);

                else
                    SET_BIT(20, pcoe);

                else
                    SET_BIT(21, emrd);

                else
                    SET_BIT(22, emwr);

                else if (word == ";")
                    status = ParseStatus::MNEMONIC;

                else
                    std::cerr << "error: unknown item " << word << '.' << std::endl;

                break;
        }*/
    }
}
