#ifndef OB_CORE_BYTECODE_H_INCLUDED
#define OB_CORE_BYTECODE_H_INCLUDED

#include "Array.h"
#include "ContextFwd.h"

#include <stddef.h>
#include <stdint.h>

typedef uint8_t ob_Instruction;

#define OBBC_GET_OPCODE(I) ((I) & 0xf)
#define OBBC_GET_DATA(I) ((I) >> 4)

#define OBBC_MAKE(I, D) ((I) | ((D) << 4))

typedef enum {
  OBBC_PUSH_LITERAL = 0,  // push a literal
  OBBC_SEND = 1,          // send a message to a known receiver
  OBBC_IMPLICIT_SEND = 2, // send a message to the implicit receiver
  OBBC_EXTEND = 3,        // extend the payload by prepending 4 bits
  OBBC_RETURN = 4,        // implements ^
  OBBC_SELF = 5,          // push the explicit receiver
  OBBC_ARRAY = 6,         // construct an Array from <index> items in the stack.
  OP_R7 = 7,
  OP_R8 = 8,
  OP_R9 = 9,
  OP_Ra = 10,
  OP_Rb = 11,
  OP_Rc = 12,
  OP_Rd = 13,
  OP_Re = 14,
  OP_Rf = 15,
} ob_Opcode;

void obbc_run(ob_Context ctx, size_t len, const uint8_t *code);

void obbc_append_insn(ob_Array *out, ob_Instruction insn);
uint8_t obbc_append_index(ob_Array *out, uint64_t index);

#endif
