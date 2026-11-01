#include <stddef.h>
#include <stdlib.h>

#pragma exceptions on

#define CTORARG_TYPE int
#define CTORARG_COMPLETE (1)

#define CTORCALL_COMPLETE(ctor, objptr)                                                            \
  (((void (*)(void*, CTORARG_TYPE))ctor)(objptr, CTORARG_COMPLETE))

#define DTORARG_TYPE int

#define DTORCALL_COMPLETE(dtor, objptr) (((void (*)(void*, DTORARG_TYPE))dtor)(objptr, -1))

extern "C" {
char __throw_catch_compare(const char* throwtype, const char* catchtype, long* offset_result);
}

void operator delete(void* ptr) throw();

namespace std {
typedef void (*terminate_handler)();
typedef void (*unexpected_handler)();
typedef void (*new_handler)();

void terminate();
void unexpected();

class exception {
public:
  exception() throw() {}
  virtual ~exception() throw();
  virtual const char* what() const throw();
};

class bad_alloc : public exception {
public:
  bad_alloc() throw() {}
  virtual ~bad_alloc() throw();
  virtual const char* what() const throw();
};

static void dthandler();
static void duhandler();

static terminate_handler thandler = dthandler;
static unexpected_handler uhandler = duhandler;
} // namespace std

// Guessed name: the global new-handler slot, zero-initialised.
std::new_handler __new_handler;

class __partial_array_destructor {
private:
  void* p;
  size_t size;
  size_t n;
  void* dtor;

public:
  size_t i;

  __partial_array_destructor(void* array, size_t elementsize, size_t nelements, void* destructor) {
    p = array;
    size = elementsize;
    n = nelements;
    dtor = destructor;
    i = n;
  }

  ~__partial_array_destructor();
};

extern "C" char __throw_catch_compare(const char* throwtype, const char* catchtype,
                                      long* offset_result) {
  const char *cptr1, *cptr2;

  *offset_result = 0;
  if ((cptr2 = catchtype) == 0) {
    return true;
  }

  cptr1 = throwtype;
  if (*cptr2 == 'P') {
    cptr2++;
    if (*cptr2 == 'C')
      cptr2++;
    if (*cptr2 == 'V')
      cptr2++;
    if (*cptr2 == 'v') {
      if (*cptr1 == 'P' || *cptr1 == '*') {
        return true;
      }
    }
    cptr2 = catchtype;
  }

  switch (*cptr1) {
  case '*':
  case '!':
    if (*cptr1++ != *cptr2++)
      return false;
    for (;;) {
      if (*cptr1 == *cptr2++) {
        if (*cptr1++ == '!') {
          long offset;

          for (offset = 0; *cptr1 != '!';) {
            offset = offset * 10 + *cptr1++ - '0';
          }
          *offset_result = offset;
          return true;
        }
      } else {
        while (*cptr1++ != '!') {
        }
        while (*cptr1++ != '!') {
        }
        if (*cptr1 == 0)
          return false;

        cptr2 = catchtype + 1;
      }
    }
    return false;
  }

  while ((*cptr1 == 'P' || *cptr1 == 'R') && *cptr1 == *cptr2) {
    cptr1++;
    cptr2++;

    if (*cptr2 == 'C') {
      if (*cptr1 == 'C')
        cptr1++;
      cptr2++;
    }
    if (*cptr1 == 'C')
      return false;

    if (*cptr2 == 'V') {
      if (*cptr1 == 'V')
        cptr1++;
      cptr2++;
    }
    if (*cptr1 == 'V')
      return false;
  }

  for (; *cptr1 == *cptr2; cptr1++, cptr2++) {
    if (*cptr1 == 0)
      return true;
  }

  return false;
}

__partial_array_destructor::~__partial_array_destructor() {
  char* ptr;

  if (i < n && dtor) {
    for (ptr = (char*)p + size * i; i > 0; i--) {
      ptr -= size;
      DTORCALL_COMPLETE(dtor, ptr);
    }
  }
}

extern "C" void __construct_array(void* ptr, void* ctor, void* dtor, size_t size, size_t n) {
  __partial_array_destructor pad(ptr, size, n, dtor);
  char* p;

  for (pad.i = 0, p = (char*)ptr; pad.i < n; pad.i++, p += size)
    CTORCALL_COMPLETE(ctor, p);
}

extern "C" void __destroy_arr(void* block, void* dtor, size_t size, size_t n) {
  char* p;

  for (p = (char*)block + size * n; n > 0; n--) {
    p -= size;
    DTORCALL_COMPLETE(dtor, p);
  }
}

const char* std::bad_alloc::what() const throw() { return "bad_alloc"; }

const char* std::exception::what() const throw() { return "exception"; }

void* operator new(size_t size) {
  void* p;

  if (size == 0)
    size = 4;
  for (;;) {
    p = malloc(size);
    if (p)
      return p;
    if (!__new_handler)
      throw std::bad_alloc();
    __new_handler();
  }
}

void operator delete(void* ptr) throw() {
  if (ptr)
    free(ptr);
}

std::exception::~exception() throw() {}

std::bad_alloc::~bad_alloc() throw() {}

namespace std {
void unexpected() { uhandler(); }

void terminate() { thandler(); }

static void duhandler() { terminate(); }

static void dthandler() { abort(); }
} // namespace std
