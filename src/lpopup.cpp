#define LP_PUBLIC 1
#include "lp_constants.h"
#include "lp_platform.h"
#include "lp_lua.h"

#ifdef LP_OS_WIN
int _main(int argc, char **argv) { // {{{
#else
int main(int argc, char **argv) { // {{{
#endif
  setlocale(LC_ALL, LP_LOCALE);
  lp::platform::init(argc, argv);
  lp::lua::init();

  lua_getglobal(lp::L, "lpopup_execute");
  lua_newtable(lp::L);
  const int top = lua_gettop(lp::L);
  for(int i = 1; i <= argc; i++) {
    lua_pushinteger(lp::L, i);
    lua_pushstring(lp::L, argv[i]);
    lua_settable(lp::L, top);
  }
  if(lua_pcall(lp::L, 1, 1, 0)) {
    lp::platform::message("Error", lua_tostring(lp::L, lua_gettop(lp::L)), lp::MESSAGE_ERROR);
    return 1;
  }

  return lp::platform::run(argc, argv);
} // }}}

