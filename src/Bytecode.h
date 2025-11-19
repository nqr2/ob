#ifndef BYTECODE_H_INCLUDED
#define BYTECODE_H_INCLUDED

#include "ContextFwd.h"

#include <stddef.h>
#include <stdint.h>

typedef uint8_t Instruction;

#define INSN_GET_OPCODE(I) ((I) & 0xf)
#define INSN_GET_DATA(I) ((I) >> 4)

typedef enum {
  OP_PUSH_LITERAL = 0,  // push a literal
  OP_SEND = 1,          // send a message to a known receiver
  OP_IMPLICIT_SEND = 2, // send a message to the implicit receiver
  OP_EXTEND = 3,        // extend the payload by prepending 4 bits
  OP_RETURN = 4,        // implements ^
  OP_SELF = 5,          // push the explicit receiver
  OP_SELF_SEND = 6,     // send a message to the explicit receiver
  OP_R7 = 7,
  OP_R8 = 8,
  OP_R9 = 9,
  OP_Ra = 10,
  OP_Rb = 11,
  OP_Rc = 12,
  OP_Rd = 13,
  OP_Re = 14,
  OP_Rf = 15,
} Opcode;

void bc_run(Context ctx, size_t len, const uint8_t *code);

#endif
