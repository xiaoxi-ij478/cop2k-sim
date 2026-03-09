#ifndef AS_HPP_INCLUDED
#define AS_HPP_INCLUDED

#include <cstdio>
#include <stdexcept>
#include <stack>

class ParseFailure : public std::logic_error
{
        using std::logic_error::logic_error;
};

class ConstNotFound : public std::logic_error
{
        using std::logic_error::logic_error;
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

// we can't use fancy classes in Yacc's structs
struct InstructionYacc {
    unsigned char byte;
    char *mnemonic;
    Operand src, dst;
    MicroProgram microprogram;
};

struct AsmInstructionOperandYacc {
    Operand src_type, dst_type;
    /*
     * for R?, R<arg>;
     * for @R? == R?;
     * for #II, arg == II;
     * for MM, arg == MM;
     * for A, arg is ignored
     */
    unsigned src, dst;
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

    Instruction &operator=(const struct InstructionYacc &other)
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
void add_instruction(
    const char *mnemonic,
    const struct AsmInstructionOperandYacc &operand
);
int get_const(const char *name);
void set_const(const char *name, int val);
void parse_instruction_file(FILE *in);

#endif // AS_HPP_INCLUDED
