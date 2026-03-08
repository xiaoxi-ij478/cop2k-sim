#ifndef AS_HPP_INCLUDED
#define AS_HPP_INCLUDED

#include <cstdio>
#include <stdexcept>

class ParseFailure : public std::runtime_error
{
        using std::runtime_error::runtime_error;
};

enum class Operand : unsigned char {
    NONE,
    REG_A,
    REG,
    REGADDR,
    IMMED,
    MEMADDR
};

struct Signals {
    bool
    emwr, emrd, pcoe, emen, iren, eint, elp, maren, maroe,
          outen, sten, rrd, rwr, cn, fen, x2, x1, x0, wen, aen, s2, s1, s0;
};

struct MicroProgram {
    Signals signal[4];
};

struct Operands {
    Operand src, dst;
};

struct Signal {
    char *name;
    bool val;
};

struct MicroProgramSignal {
    unsigned index;
    struct Signals signal;
};

// we can't use fancy classes in Yacc's structs
struct InstructionYacc {
    unsigned char byte;
    char *mnemonic;
    Operand src, dst;
    MicroProgram microprogram;
};

struct Instruction {
    unsigned char byte;
    std::string mnemonic;
    Operand src, dst;
    MicroProgram microprogram;

    Instruction() = default;

    Instruction(const struct InstructionYacc &other) :
        byte(other.byte),
        mnemonic(other.mnemonic),
        src(other.src),
        dst(other.dst),
        microprogram(other.microprogram)
    {}

    Instruction& operator=(const struct InstructionYacc &other)
    {
        byte = other.byte;
        mnemonic = other.mnemonic;
        src = other.src;
        dst = other.dst;
        microprogram = other.microprogram;
        return *this;
    }
};

extern struct Instruction instructions[256 >> 2];
extern unsigned instruction_numbers;

void clear_instruction_set(void);
void add_to_instruction_set(const struct InstructionYacc &instruction);
void parse_instruction_file(FILE *in);

#endif // AS_HPP_INCLUDED
