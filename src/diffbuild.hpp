#pragma once

#ifdef DIFFBUILD
#define DIFFABLE_EXTERN(type, name)                                                                                    \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        extern type name;                                                                                              \
    }
#define DIFFABLE_STATIC(type, name)                                                                                    \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        extern type name;                                                                                              \
    }
#else
#define DIFFABLE_EXTERN(type, name) extern type name;
#define DIFFABLE_STATIC(type, name) type name;
#endif
