#include <stdlib.h>

#pragma exceptions on

namespace std {
class exception {
public:
  exception() throw() {}
  virtual ~exception() throw();
  virtual const char* what() const throw();
};
typedef void (*new_handler)();

class bad_alloc : public exception {
public:
  bad_alloc() throw() {}
  virtual ~bad_alloc() throw();
  virtual const char* what() const throw();
};
} // namespace std

// Guessed name: the global new-handler slot, zero-initialised.
std::new_handler __new_handler;

const char* std::exception::what() const throw() { return "exception"; }

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

void operator delete(void* ptr) throw() {
  if (ptr)
    free(ptr);
}

std::exception::~exception() throw() {}

std::bad_alloc::~bad_alloc() throw() {}
