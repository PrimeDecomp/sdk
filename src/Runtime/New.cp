#include <stdlib.h>

#pragma exceptions on

namespace std {
class exception {
public:
  exception() throw() {}
  virtual ~exception() throw();
  virtual const char* what() const throw();
};
#ifdef RVL_PROTO_RUNTIME
typedef void (*new_handler)();

class bad_alloc : public exception {
public:
  bad_alloc() throw() {}
  virtual ~bad_alloc() throw();
  virtual const char* what() const throw();
};
#endif
} // namespace std

#ifdef RVL_PROTO_RUNTIME
// Guessed name: the global new-handler slot, zero-initialised.
std::new_handler __new_handler;
#endif

const char* std::exception::what() const throw() { return "exception"; }

#ifdef RVL_PROTO_RUNTIME
const char* std::bad_alloc::what() const throw() { return "bad_alloc"; }

void* operator new(size_t size) {
  void* p;
  if (size == 0)
    size = 4;
  for (;;) {
    p = malloc(size);
    if (p)
      break;
    std::new_handler handler = __new_handler;
    if (handler)
      handler();
    else
      throw std::bad_alloc();
  }
  return p;
}
#endif

void operator delete(void* ptr) throw() {
  if (ptr)
    free(ptr);
}

std::exception::~exception() throw() {}

#ifdef RVL_PROTO_RUNTIME
std::bad_alloc::~bad_alloc() throw() {}
#endif
