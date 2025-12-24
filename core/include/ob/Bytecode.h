#ifndef OB_CORE_BYTECODE_H_INCLUDED
#define OB_CORE_BYTECODE_H_INCLUDED

/** @file
 * @brief Bytecode definitions, and the interpreter.
 */

#include "Array.h"
#include "ContextFwd.h"

#include <stddef.h>
#include <stdint.h>

typedef uint8_t ob_Instruction;

#define OBBC_GET_OPCODE(I) ((I) & 0xf)
#define OBBC_GET_DATA(I) ((I) >> 4)

#define OBBC_MAKE(I, D) ((I) | ((D) << 4))

typedef enum {
  /// Push a literal
  OBBC_PUSH_LITERAL = 0,

  /// Send a message to a known receiver
  OBBC_SEND = 1,

  /// Send a message to the implicit receiver
  OBBC_IMPLICIT_SEND = 2,

  /// Extend the payload by prepending 4 bits
  OBBC_EXTEND = 3,

  /// Implements @c ^
  OBBC_RETURN = 4,

  /// Push the explicit receiver
  OBBC_SELF = 5,

  /// Construct an @c Array from @c index items in the stack.
  OBBC_ARRAY = 6,

  OBBC_RESERVED_7 = 7,
  OBBC_RESERVED_8 = 8,
  OBBC_RESERVED_9 = 9,
  OBBC_RESERVED_a = 10,
  OBBC_RESERVED_b = 11,
  OBBC_RESERVED_c = 12,
  OBBC_RESERVED_d = 13,
  OBBC_RESERVED_e = 14,
  OBBC_RESERVED_f = 15,
} ob_Opcode;

void obbc_run(ob_Context ctx, size_t len, const uint8_t *code);

void obbc_append_insn(ob_Array *out, ob_Instruction insn);
uint8_t obbc_append_index(ob_Array *out, uint64_t index);

#endif
