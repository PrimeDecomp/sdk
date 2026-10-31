#include <stdlib.h>

#pragma exceptions on

namespace std {
class exception {
public:
  exception() throw() {}
  virtual ~exception() throw();
  virtual const char* what() const throw();
};
} // namespace std

const char* std::exception::what() const throw() { return "exception"; }

void operator delete(void* ptr) throw() {
  if (ptr)
    free(ptr);
}

std::exception::~exception() throw() {}
