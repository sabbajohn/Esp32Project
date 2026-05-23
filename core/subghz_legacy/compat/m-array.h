#pragma once

/* Minimal shim for legacy code loops. */

#define M_POD_OPLIST 0
#define ARRAY_OPLIST(name, oplist) 0
#define ARRAY_DEF(name, type, oplist)

#define M_EACH(item, arr, arr_type) \
    (SubGhzKey* item = (arr).items; item < (arr).items + (arr).size; ++item)
