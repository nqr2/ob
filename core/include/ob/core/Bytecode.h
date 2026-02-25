#ifndef OB_CORE_BYTECODE_H_INCLUDED
#define OB_CORE_BYTECODE_H_INCLUDED

/** @file
 *
 * @brief Bytecode definitions, and the interpreter.
 */

#include <ob/Core.h>

#include <ob/base/Array.h>

#include <stddef.h>
#include <stdint.h>

typedef uint8_t ob_Instruction;

#define OBBC_GET_OPCODE(I) ((I) & 0xf)
#define OBBC_GET_DATA(I) ((I) >> 4)

#define OBBC_MAKE(I, D) ((I) | ((D) << 4))

typedef enum {
  /// Push a literal
  OB_OP_PUSH = 0,

  /// Send a message to a known receiver
  OB_OP_SEND = 1,

  /// Send a message to the implicit receiver
  OB_OP_IMPLICIT = 2,

  /// Extend the payload by prepending 4 bits
  OB_OP_EXTEND = 3,

  /** @brief Runs an instruction taking 0 arguments, "indexed" by the payload.
   *
   * The list of instructions is:
   * 0. @c OP_RETURN    (The implementation of `^`)
   * 1. @c OP_DUPLICATE (Duplicate top of stack)
   * 2. @c OP_POP       (Pop top of stack)
   */
  OB_OP_EXTRA = 4,

  /// Construct an @c Array from @c index items in the stack.
  OB_OP_ARRAY = 5,

  /** Sets the debug information data.
   *
   * The format is somewhat complicated: `DEBUG length` is an opcode followed
   * by a "column delta", and a "line delta" if `length` is not 0. Both "deltas"
   * are encoded in LEB128.
   *
   * The interpreter starts by setting the currently executing line and column
   * to 0, and always adds the column and line deltas from this instruction.
   *
   * If the length == 0, it is interpreted as just having a column delta.
   *
   * When length is > 0, it is interpreted as the length of a sequence of text
   * immediately after this opcode, which contains the contents of the currently
   * executing line (so if there is no changes in the line, there is no line
   * "movement", thus the line delta is not encoded), and the column is set to 0
   * before handling the column delta.
   */
  OB_OP_DEBUG = 6,

  /** Sets the filename of the current program's debug information. The data is
   * interpreted as the length of text immediately following this opcode. Also
   * sets the line and column to 0.
   */
  OB_OP_FILENAME = 7,

  OB_OP_RESERVED8 = 8,
  OB_OP_RESERVED9 = 9,
  OB_OP_RESERVEDa = 10,
  OB_OP_RESERVEDb = 11,
  OB_OP_RESERVEDc = 12,
  OB_OP_RESERVEDd = 13,
  OB_OP_RESERVEDe = 14,
  OB_OP_RESERVEDf = 15,
} ob_Opcode;

void obbc_run(ob_Ctx ctx, size_t len, uint8_t const *code);

void obbc_append_insn(ql_Array *out, ob_Instruction insn);
uint8_t obbc_append_index(ql_Array *out, uint64_t index);

uint8_t const *obbc_read_insn(uint8_t const *source, ob_Opcode *opcode,
                              size_t *data);

#endif
