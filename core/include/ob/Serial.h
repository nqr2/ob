#ifndef OB_CORE_SERIAL_H_INCLUDED
#define OB_CORE_SERIAL_H_INCLUDED

/** @file
 *
 * @brief (de)Serializing objects.
 */

#include "Object.h"

#define OB_SERIAL_HEADER "obS"

typedef struct {
  ob_Context ctx;
  ob_Array buffer;
  ob_Table identifiers;
} ob_Serial;

void obsrl_init(ob_Serial *srl, ob_Context ctx);
void obsrl_free(ob_Serial *srl);

void obsrl_write(ob_Serial *srl, ob_Obj object);
ob_Obj obsrl_read(ob_Serial *srl);

void obsrl_store(const ob_Serial *srl, size_t len, uint8_t *data);
void obsrl_load(ob_Serial *srl, size_t len, const uint8_t *data);

#endif
