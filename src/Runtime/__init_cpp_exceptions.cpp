#include "NMWException.h"
#include "__ppc_eabi_linker.h"

#ifdef PRIME1
static int fragmentID = -2;
#else
#if __MWERKS__
#pragma exceptions off
#pragma internal on
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void __init_cpp_exceptions(void);
extern void __fini_cpp_exceptions(void);
extern void suspend(void);

#ifdef __cplusplus
}
#endif

#ifdef PRIME1
/* clang-format off */
static asm char* GetR2() {
  nofralloc;
  mr r3, r2
  blr
#else
static int fragmentID = -2;

void __exception_info_constants(void** info, char** R2) {
  register char* temp;
  /* clang-format off */
	asm {
		mr      temp,r2
	}
  /* clang-format on */
  *R2 = temp;
  *info = _eti_init_info;
#endif
}
/* clang-format on */

#ifdef PRIME1
extern void __init_cpp_exceptions(void) {
#else
// UNUSED: __find_exception_addresses__FPvPcPPvPPv

void __init_cpp_exceptions(void) {
#endif
  char* R2;
#ifndef PRIME1
  void* info;
#endif
  if (fragmentID == -2) {
#ifdef PRIME1

    R2 = GetR2();

    /* HACK: TODO: _eti_init_info should be _eti_init_info, we can't use the appropriate name yet due to the
     * linker not being able to generate it*/
    fragmentID = __register_fragment(_eti_init_info, R2);
#else
    __exception_info_constants(&info, &R2);
    fragmentID = __register_fragment((struct __eti_init_info*)info, R2);
#endif
  }
}

#ifdef PRIME1
extern void __fini_cpp_exceptions(void) {
#else
void __fini_cpp_exceptions(void) {
#endif
  if (fragmentID != -2) {
    __unregister_fragment(fragmentID);
    fragmentID = -2;
  }
}

#ifdef PRIME1
__declspec(section
           ".ctors") static void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;
__declspec(section
           ".dtors") static void* const __destroy_global_chain_reference = __destroy_global_chain;
__declspec(section
           ".dtors") static void* const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
#else
#pragma push
// HACK: When DTK sets all objects to force_active, the linker decides to strip
// these, for some reason. Set them to force_active as well.
#pragma force_active on
#endif

#ifndef PRIME1
/* clang-format off */
#pragma section ".ctors$10"
__declspec(section ".ctors$10")
extern void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;
#pragma section ".dtors$10"
__declspec(section ".dtors$10")
extern void* const __destroy_global_chain_reference = __destroy_global_chain;
#pragma section ".dtors$15"
__declspec(section ".dtors$15")
extern void* const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
/* clang-format on */

#pragma pop

#endif