#ifndef POLYKEYS_H
#define POLYKEYS_H

#include "defs.h"

#ifdef _MSC_VER
# define U64_POLY(u) (u##ui64)
#else
#define U64_POLY(u) (u##ULL)
#endif

extern const U64 Random64_POLY[781];




#endif // !POLYKEYS_H
