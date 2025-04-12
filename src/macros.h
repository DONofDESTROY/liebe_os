#ifndef MACROS_H
#define MACROS_H
#include "stdint.h"

#define ISERR(val) ((int)(uintptr_t)val < 0)
#define ISERR_I(val) (val < 0)
#define ERROR(val) (void *)(val)
#define ERROR_I(val) (uintptr_t)(val)

#endif // !MACROS_H
