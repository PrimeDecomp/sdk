#pragma force_active on

#include "ppc/Export/targsupp.h"

// clang-format off
asm u32 TRKAccessFile(u32, u32, u32*, u8*) {
    nofralloc
    opword 0x0FE00000
    blr
}

asm u32 TRKOpenFile(u32, u32, u32*, u8*) {
    nofralloc
    opword 0x0FE00000
    blr
}

asm u32 TRKCloseFile(u32, u32) {
    nofralloc
    opword 0x0FE00000
    blr
}

asm u32 TRKPositionFile(u32, u32, u32*, u8*) {
    nofralloc
    opword 0x0FE00000
    blr
}
// clang-format on
