#ifndef OB_CORE_SERIAL_H_INCLUDED
#define OB_CORE_SERIAL_H_INCLUDED

/** @file
 *
 * @brief (de)Serializing objects.
 */

#include <ob/Core.h>

#include <ql/Table.h>

#define OB_SERIAL_HEADER "\x0bOB"

typedef struct {
  ob_Ctx ctx;
  ql_Array buffer;
  ql_Table identifiers;
} ob_Serial;

void obsrl_init(ob_Serial *srl, ob_Ctx ctx);
void obsrl_free(ob_Serial *srl);

void obsrl_write(ob_Serial *srl, ob_Obj object);
ob_Obj obsrl_read(ob_Serial *srl);

void obsrl_store(ob_Serial const *srl, size_t len, uint8_t *data);
void obsrl_load(ob_Serial *srl, size_t len, uint8_t const *data);

#endif
