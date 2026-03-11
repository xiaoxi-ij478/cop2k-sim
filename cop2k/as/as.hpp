#ifndef AS_HPP_INCLUDED
#define AS_HPP_INCLUDED

#include <cstdio>
#include <stdexcept>
#include <stack>

#include "libopcode.hpp"

namespace COP2K
{
    class AS{
public:

private:
    Memory em;
    };
}

// we can't use fancy classes in Yacc's structs
struct AsmInstructionOperandYacc {
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


#endif // AS_HPP_INCLUDED
