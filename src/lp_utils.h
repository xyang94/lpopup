#ifndef __LP_UTILS_H__
#define __LP_UTILS_H__

#include "lp_constants.h"

namespace lp {
  class Error;
  namespace utils {
    void exit_application(const int code = 0);
  }

  // Utility classes {{{
  template <class T>
  class NonCopyable { // {{{
    protected:
      NonCopyable () {}
      virtual ~NonCopyable () {} 
    private: 
      NonCopyable (const NonCopyable &);
      NonCopyable& operator=(const NonCopyable &);
  }; // }}}

  class Error : private NonCopyable<Error> { // {{{
    protected:
      unsigned int code_;
      std::string  msg_;

    public:
      Error() : code_(0) {}
      Error(unsigned int code, const char *msg) : code_(code), msg_(std::string(msg)) {};
      Error(unsigned int code, const std::string &msg) : code_(code), msg_(msg) {}
    
      const unsigned int& getCode() const {return code_;}
      void setCode(const int value) { code_ = value;}
      const std::string& getMessage() const {return msg_;}
      void setMessage(const char *value) { msg_ = value; }
      void setMessage(const std::string &value) { msg_ = value; }
  }; // }}}

  class Scoped : private NonCopyable<Scoped> {
    public:
      explicit Scoped(lp::scoped_func func) : func_(func) {}
      ~Scoped() {func_();}

    protected:
      lp::scoped_func func_;
  };

}

#endif
