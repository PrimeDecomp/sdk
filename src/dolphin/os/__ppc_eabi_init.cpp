#ifdef PRIME1
#include "dolphin/__ppc_eabi_init.h"
#include "dolphin/base/PPCArch.h"
#else
#include "GameVersions.h"
#include <dolphin/__ppc_eabi_init.h>
#include <dolphin/PPCArch.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
void __OSPSInit();
#ifndef PRIME1
void __OSFPRInit();
#endif
void __OSCacheInit();

asm void __init_hardware(void) {
  // clang-format off
  nofralloc
  mfmsr r0
  ori r0, r0, 0x2000
  mtmsr r0

  mflr r31
  bl __OSPSInit
#ifndef PRIME1
  bl __OSFPRInit
#endif
  bl __OSCacheInit
  mtlr r31
  blr
  // clang-format on
}

asm void __flush_cache(register void* address, register unsigned int size) {
  // clang-format off
  nofralloc
  lis r5,  ~0
  ori r5, r5, ~14
  and r5, r5, r3
  subf r3, r5, r3
  add r4, r4, r3

loop:    
  dcbst r0, r5
  sync
  icbi r0, r5
  addic r5, r5, 8
  subic. r4, r4, 8    
  bge loop
  isync
  blr
  // clang-format on
}

#ifdef PRIME1
void __init_user() { __init_cpp(); }
#else
__declspec(weak) void InitMetroTRK_BBA() { return; }

#if VERSION >= VERSION_R32J01
asm void __init_user() {
  // clang-format off
  nofralloc
  stwu r1, -0x10(r1)
  mflr r0
  stw r0, 0x14(r1)
  bl __init_cpp
  lwz r0, 0x14(r1)
  mtlr r0
  addi r1, r1, 0x10
  blr
  // clang-format on
}
#else
void __init_user() { __init_cpp(); }
#endif
#endif

typedef void (*voidfunctionptr)(void); // pointer to function returning void
__declspec(section ".init") extern voidfunctionptr _ctors[];
__declspec(section ".init") extern voidfunctionptr _dtors[];

void __init_cpp(void) {

  /*
   *	call static initializers
   */
  for (voidfunctionptr* constructor = _ctors; *constructor; constructor++) {
    (*constructor)();
  }
}

#ifdef PRIME1
void _ExitProcess(void) { PPCHalt(); }
#else
#if VERSION < VERSION_R32J01
void _ExitProcess(void) { PPCHalt(); }
#else
void exit(int status) {
  for (voidfunctionptr* destructor = _dtors; *destructor; destructor++) {
    (*destructor)();
  }
  PPCHalt();
}
#endif
#endif

#ifdef __cplusplus
}
#endif
