#include <stdlib.h>

#pragma exceptions on

#include "Exception.hpp"

namespace std {
typedef void (*new_handler)();
new_handler __new_handler;

class bad_alloc : public exception {
public:
  bad_alloc() throw() {}
  bad_alloc(const bad_alloc&) throw() {}
  virtual ~bad_alloc() throw() {}
  virtual const char* what() const throw() { return "bad_alloc"; }
};
struct nothrow_t {};
nothrow_t nothrow;
} // namespace std

__declspec(weak) void* operator new(size_t size) throw(std::bad_alloc) {
  void* p;
  if (size == 0)
    size = 4;
  for (;;) {
    p = malloc(size);
    if (p)
      break;
    std::new_handler handler = std::__new_handler;
    if (handler)
      handler();
    else
      throw std::bad_alloc();
  }
  return p;
}

__declspec(weak) void* operator new(size_t size, const std::nothrow_t&) throw() {
  try {
    return operator new(size);
  } catch (...) {
    return 0;
  }
}

__declspec(weak) void operator delete(void* ptr) throw() {
  if (ptr)
    free(ptr);
}

__declspec(weak) void* operator new[](size_t size) throw(std::bad_alloc) {
  return operator new(size);
}

__declspec(weak) void* operator new[](size_t size, const std::nothrow_t& tag) throw() {
  return operator new(size, tag);
}

__declspec(weak) void operator delete[](void* ptr) throw() { operator delete(ptr); }

__declspec(weak) void operator delete(void* ptr, const std::nothrow_t&) throw() {
  operator delete(ptr);
}

__declspec(weak) void operator delete[](void* ptr, const std::nothrow_t&) throw() {
  operator delete(ptr);
}
