#ifndef __LP_LUA_H__
#define __LP_LUA_H__

#include "lp_constants.h"
#include "lp_utils.h"

namespace lp {
  LP_EXPORT lua_State *L;
  namespace lua {
    void init();
    void alert_stack(lua_State *L);

    static int menu_id_seq = 0;
    class Menu : private lp::NonCopyable<Menu> {
      public:

        Menu() {
          id_ = lp::lua::menu_id_seq++;
        }

        int getId() const { return id_; }

      protected:
        int id_;
    };
  }

  namespace luamodule { // {{{
    int create(lua_State *L);
    // cmd:string, options{workdir:string,async:bool,bool:hide} -> {code:int, stdout:string, stderr:string, errmsg:string}
    int call(lua_State *L); 
    // cmd:string, options{workdir:string} -> void
    int shell_execute(lua_State *L); 
    // src:string -> new:string
    int utf82local(lua_State *L);
    // src:string -> new:string
    int local2utf8(lua_State *L);
  } // }}}
}

#endif
