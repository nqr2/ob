#ifndef OB_CORE_PARSE_H_INCLUDED
#define OB_CORE_PARSE_H_INCLUDED

/** @file
 *
 * @brief The lexer, parser, and bytecode compiler, called the "Reader".
 */

#include <ob/Core.h>

[[deprecated("replace ob_Reader* for ob_Rdr")]]
typedef struct ob_Reader ob_Reader;
typedef struct ob_Reader *ob_Rdr;

ob_Rdr obrdr_create(ob_Ctx ctx);
void obrdr_free(ob_Rdr rdr);

void obrdr_load(ob_Rdr rdr, char const *path, size_t length, char const *data);

ob_Obj obrdr_get_method(ob_Rdr rdr);

#endif
