#include "NMWException.h"

DestructorChain* __global_destructor_chain;

#ifdef PRIME1
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

#ifdef PRIME1
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
