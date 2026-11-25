#ifndef _RUNTIME_EXCEPTION_HPP
#define _RUNTIME_EXCEPTION_HPP

namespace std {
class exception {
public:
  exception() throw() {}
  exception(const exception&) throw() {}
  virtual ~exception() throw() {}
  virtual const char* what() const throw() { return "exception"; }
};
} // namespace std

#endif // _RUNTIME_EXCEPTION_HPP
