#include "lp_utils.h"
#include "lp_lua.h"
#include "lp_platform.h"

void lp::utils::exit_application(const int code) { // {{{
  lp::platform::exit(code);
  if(lp::L) {
    lua_close(lp::L);
  }
  if(code == 0) { 
  }
  exit(code);
} // }}}

