#include <SDKVersion.h>
#include "NMWException.h"

DestructorChain* __global_destructor_chain;

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
extern void* __register_global_object(void* object, void* destructor, void* regmem) {
#else
void* __register_global_object(void* object, void* destructor, void* regmem) {
#endif
  ((DestructorChain*)regmem)->next = __global_destructor_chain;
  ((DestructorChain*)regmem)->destructor = destructor;
  ((DestructorChain*)regmem)->object = object;
  __global_destructor_chain = (DestructorChain*)regmem;
  return object;
}

void __destroy_global_chain(void) {
  DestructorChain* iter;

  while ((iter = __global_destructor_chain) != 0) {
    __global_destructor_chain = iter->next;
    DTORCALL_COMPLETE(iter->destructor, iter->object);
  }
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
__declspec(section
           ".dtors") static void* const __destroy_global_chain_reference = __destroy_global_chain;
#else
/* clang-format off */
#pragma section ".dtors$10"
__declspec(section ".dtors$10")
__declspec(weak)
extern void* const __destroy_global_chain_reference = __destroy_global_chain;
/* clang-format on */
#endif
