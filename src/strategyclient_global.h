#ifndef STRATEGYCLIENT_GLOBAL_H
#define STRATEGYCLIENT_GLOBAL_H

#include "common/WGlobal.h"
#if defined(STRATEGYCLIENT_LIBRARY)
#  define STRATEGYCLIENTSHARED_EXPORT W_DECL_EXPORT
#else
#  define STRATEGYCLIENTSHARED_EXPORT W_DECL_IMPORT
#endif

#endif // STRATEGYCLIENT_GLOBAL_H
