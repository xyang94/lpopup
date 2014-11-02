#include "lp_lua.h"
#include "lp_platform.h"
extern "C" {
  LUALIB_API int luaopen_bit(lua_State *L);
}


/* compatibility stuff {{{ */
#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
static void luaL_setfuncs (lua_State *l, const luaL_Reg *reg, int nup)
{
    int i;
    luaL_checkstack(l, nup, "too many upvalues");
    for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(l, -nup);
        lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);  /* remove upvalues */
}
#endif
/* }}} */

// class Menu {{{

static lp::lua::Menu* to_menu (lua_State *L, int index) {
  lp::lua::Menu **menu_ptr = (lp::lua::Menu**)lua_touserdata(L, index);
  if (menu_ptr == 0) luaL_typerror(L, index, "lpopup.Menu");
  return *menu_ptr;
}

static lp::lua::Menu* check_menu (lua_State *L, int index) {
  lp::lua::Menu **menu_ptr;
  luaL_checktype(L, index, LUA_TUSERDATA);
  menu_ptr = (lp::lua::Menu**)luaL_checkudata(L, index, "lpopup.Menu");
  if (menu_ptr == 0) luaL_typerror(L, index, "lpopup.Menu");
  if (*menu_ptr == 0) luaL_error(L, "null lpopup.Menu");
  return *menu_ptr;
}

static lp::lua::Menu* push_menu (lua_State *L, lp::lua::Menu *value) {
  lp::lua::Menu **menu_ptr = (lp::lua::Menu**)lua_newuserdata(L, sizeof(lp::lua::Menu));
  *menu_ptr = value;
  luaL_getmetatable(L, "lpopup.Menu");
  lua_setmetatable(L, -2);
  return *menu_ptr;
}

static int Menu_new (lua_State *L) {
  lp::lua::Menu *value = new lp::lua::Menu();
  lp::platform::new_menu(value->getId());
  push_menu(L, value);
  return 1;
}

static int Menu_id(lua_State *L) {
  lp::lua::Menu *value = check_menu(L, 1);
  lua_pushinteger(L, value->getId());
  return 1;
}

static int Menu_item_count(lua_State *L) {
  lp::lua::Menu *value = check_menu(L, 1);
  lua_pushinteger(L, lp::platform::menu_item_count(value->getId()));
  return 1;
}

static int g_callback_seq = 0;
struct lp_callback_data {
  lua_State *L;
  int       seq;
};
static int Menu_add_item(lua_State *L) {
  int num = lua_gettop(L);
  const char *icon = 0;
  lp::lua::Menu *value = check_menu(L, 1);
  const char *text = luaL_checkstring(L, 2);
  if(!lua_isfunction(L, 3)){
    luaL_typerror(L, 3, "function");
  }
  if(num == 4) {
    lua_pushstring(L, "icon");
    lua_gettable(L, -2);
    if(lua_isstring(L, -1)) {
      icon = luaL_checkstring(L, -1);
    }
    lua_pop(L, 2);
  }

  g_callback_seq++;
  lua_getglobal(L, "_lp_callbacks");
  lua_pushinteger(L, g_callback_seq);
  lua_pushvalue(L, 3);
  lua_settable(L, 4);
  struct lp_callback_data *data = new struct lp_callback_data;
  data->L = L;
  data->seq = g_callback_seq;
  lp::platform::add_item(value->getId(), text, icon, [](void *data) -> void {
    struct lp_callback_data *d = (struct lp_callback_data *)data;
    lua_State *L = d->L;
    int seq = d->seq;
    int num = lua_gettop(L);
    lua_getglobal(L, "_lp_callbacks");
    lua_pushinteger(L, seq);
    lua_gettable(L, -2);
    if(lua_pcall(L, 0, 0, 0)) {
      lp::platform::message("Error", lua_tostring(L, lua_gettop(L)), lp::MESSAGE_ERROR);
    }
    lua_pop(L, num);
  }, data);
  return 0;
}

static int Menu_show(lua_State *L) {
  //lp::lua::Menu *value = check_menu(L, 1);

  int num = lua_gettop(L);
  int type = luaL_checkint(L, 2);
  if(num > 2) {
    int x = luaL_checkint(L, 3);
    int y = luaL_checkint(L, 4);
    lp::platform::show_menu(type, x, y);
  }else {
    lp::platform::show_menu(type);
  }
  return 0;
}

static int Menu_add_submenu(lua_State *L) {
  int num = lua_gettop(L);
  const char *icon = 0;

  lp::lua::Menu *self = check_menu(L, 1);
  const char *text = luaL_checkstring(L, 2);
  lp::lua::Menu *other = check_menu(L, 3);
  if(num == 4) {
    lua_pushstring(L, "icon");
    lua_gettable(L, -2);
    if(lua_isstring(L, -1)) {
      icon = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
  }

  lp::platform::add_submenu(self->getId(), other->getId(), text, icon);
  return 0;
}

static int Menu_add_hseparator(lua_State *L) {
  lp::lua::Menu *self = check_menu(L, 1);
  lp::platform::add_hseparator(self->getId());
  return 0;
}

static int Menu_add_vseparator(lua_State *L) {
  lp::lua::Menu *self = check_menu(L, 1);
  lp::platform::add_vseparator(self->getId());
  return 0;
}

static int Menu_add_file_context(lua_State *L) {
  int num = lua_gettop(L);
  const char *icon = 0;

  lp::lua::Menu *self = check_menu(L, 1);
  const char *text = luaL_checkstring(L, 2);
  const char *path = luaL_checkstring(L, 3);

  if(num == 4) {
    lua_pushstring(L, "icon");
    lua_gettable(L, -2);
    if(lua_isstring(L, -1)) {
      icon = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
    if(icon != 0 && strcmp(icon, "auto") == 0) {
      icon = path;
    }
  }

  lp::platform::add_file_context(self->getId(), text, path, icon);
  return 0;
}

static const luaL_reg Menu_methods[] = {
  {"new",            Menu_new},
  // void -> int
  {"id",             Menu_id},
  // label:text, callback:function -> void, options{icon:string}
  {"add_item",       Menu_add_item},
  // label:text, submenu:menu, options{icon:string} -> void
  {"add_submenu",    Menu_add_submenu},
  // void -> void
  {"add_hseparator",    Menu_add_hseparator},
  // void -> void
  {"add_vseparator",    Menu_add_vseparator},
  // label:string, path:string, options{icon:string} -> void
  {"add_file_context",    Menu_add_file_context},
  // void -> int
  {"item_count",    Menu_item_count},
  // type:int, x:int, y:int
  {"show",           Menu_show},
  {0,0}
};

static int Menu_gc (lua_State *L){
  lp::lua::Menu *value = to_menu(L, 1);
  if (value != 0) { delete value; }
  return 0;
}

static int Menu_tostring (lua_State *L) {
  lp::lua::Menu *value = to_menu(L, 1);
  lua_pushfstring(L, "<Menu id=%d>", value->getId());
  return 1;
}

static const luaL_reg Menu_meta[] = {
  {"__gc",       Menu_gc},
  {"__tostring", Menu_tostring},
  {0, 0}
};


int Menu_register (lua_State *L){
  int num = lua_gettop(L);
  lua_pushstring(L, "Menu");
  lua_newtable(L);
  lua_settable(L, -3);
  lua_pushstring(L, "Menu");
  lua_gettable(L, -2);
  luaL_setfuncs(L,  Menu_methods, 0); 

  luaL_newmetatable(L, "lpopup.Menu");
  luaL_register(L, 0, Menu_meta);
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -3);
  lua_rawset(L, -3);
  lua_pushliteral(L, "__metatable");
  lua_pushvalue(L, -3);
  lua_rawset(L, -3);

  lua_pop(L, lua_gettop(L)-num);
  return 0;
}

// }}}

void lp::lua::alert_stack(lua_State *L) { // {{{
  int num, i, type;
  
  num = lua_gettop(L);
  if (num == 0) {
    lp::platform::message("info", "empty stack.");
    return;
  }

  std::string str;
  char tmpstr[128];

  for(i = num; i >= 1; i--) {
    snprintf(tmpstr, 127, "%03d(%04d): ", i, -num + i - 1);
    str += tmpstr;

    type = lua_type(L, i);
    switch(type) {
    case LUA_TNIL:
      str += "NIL\n";
      break;
    case LUA_TBOOLEAN:
      str += "BOOLEAN ";
      str += (lua_toboolean(L, i) ? "true\n" : "false\n");
      break;
    case LUA_TLIGHTUSERDATA:
      str += "LIGHTUSERDATA\n";
      break;
    case LUA_TNUMBER:
      snprintf(tmpstr, 127, "NUMBER %lf\n", lua_tonumber(L, i));
      str += tmpstr;
      break;
    case LUA_TSTRING:
      str += "STRING ";
      str += lua_tostring(L, i);
      str += "\n";
      break;
    case LUA_TTABLE:
      str += "TABLE\n";
      break;
    case LUA_TFUNCTION:
      str += "FUNCTION\n";
      break;
    case LUA_TUSERDATA:
      str += "USERDATA\n";
      break;
    case LUA_TTHREAD:
      str += "TTHREAD\n";
      break;
    }
  }
  lp::platform::message("info", str.c_str());
} // }}}

void lp::lua::init(){ // {{{
  lp::L = luaL_newstate();
  luaL_openlibs(L);
  luaopen_lfs(L);
  luaopen_bit(L);
  lua_register(L, "_lp_module", &lp::luamodule::create);
  lua_newtable(L);
  lua_setglobal(L, "_lp_callbacks");

  lp::oschar osconfig_path[LP_MAX_PATH];
  lp::platform::config_path(osconfig_path, LP_MAX_PATH);
  char config_path[LP_MAX_PATH_BYTE];
  lp::platform::oschar2local_b(config_path, LP_MAX_PATH_BYTE, osconfig_path);

  if(luaL_dofile(L, config_path)){
    lp::platform::message("Error", lua_tostring(L, lua_gettop(L)), lp::MESSAGE_ERROR);
    lp::utils::exit_application(1);
  }
} // }}}

// lua module {{{
int lp::luamodule::create(lua_State *L){ // {{{

  lua_newtable(L);
  Menu_register(L);

#define REGISTER_FUNCTION(name) \
  lua_pushcfunction(L, &lp::luamodule::name); \
  lua_setfield(L, 1, #name);

  REGISTER_FUNCTION(call);
  REGISTER_FUNCTION(shell_execute);
  REGISTER_FUNCTION(utf82local);
  REGISTER_FUNCTION(local2utf8);

#undef REGISTER_FUNCTION
  return 1;
} // }}}

int lp::luamodule::call(lua_State *L) { // {{{
  const int top = lua_gettop(L);
  const char *cmd = luaL_checkstring(L, 1);
  std::string workdir = "";
  bool        async   = false;
  bool        hide    = false;

  if(top > 1) {
    if(!lua_istable(L, 2)) {
      luaL_typerror(L, 2, "table");
      return 0;
    }

    lua_pushstring(L, "workdir");
    lua_gettable(L, -2);
    if(lua_isstring(L, -1)) {
      workdir += luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);

    lua_pushstring(L, "hide");
    lua_gettable(L, -2);
    if(lua_isboolean(L, -1)) {
      hide = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_pushstring(L, "async");
    lua_gettable(L, -2);
    if(lua_isboolean(L, -1)) {
      async = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
  }

  std::string sstdout;
  std::string sstderr;
  lp::Error error;
  int ret = lp::platform::call(cmd, workdir.c_str(), hide, async, sstdout, sstderr, error);
  lua_newtable(L);
  lua_pushstring(L, "code"); lua_pushinteger(L, ret); lua_settable(L, -3);
  lua_pushstring(L, "stdout"); lua_pushstring(L, sstdout.c_str()); lua_settable(L, -3);
  lua_pushstring(L, "stderr"); lua_pushstring(L, sstderr.c_str()); lua_settable(L, -3);
  if(ret != 0) {
    lua_pushstring(L, "errmsg"); lua_pushstring(L, error.getMessage().c_str()); lua_settable(L, -3);
  }
  return 1;
} // }}}

int lp::luamodule::shell_execute(lua_State *L) { // {{{
  const int top = lua_gettop(L);
  const char *cmd = luaL_checkstring(L, 1);
  std::string workdir = "";
  if(top > 1) {
    if(!lua_istable(L, 2)) {
      luaL_typerror(L, 2, "table");
      return 0;
    }

    lua_pushstring(L, "workdir");
    lua_gettable(L, -2);
    if(lua_isstring(L, -1)) {
      workdir += luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);
  }

  lp::Error error;
  lp::platform::shell_execute(cmd, workdir.c_str(), error);
  return 0;
} // }}}

int lp::luamodule::utf82local(lua_State *L) { // {{{
  const char *text= luaL_checkstring(L, 1);
  lp::unique_char_ptr ret(lp::platform::utf82local(text));
  lua_pushstring(L, ret.get());
  return 1;
} // }}}

int lp::luamodule::local2utf8(lua_State *L) { // {{{
  const char *text= luaL_checkstring(L, 1);
  lp::unique_char_ptr ret(lp::platform::local2utf8(text));
  lua_pushstring(L, ret.get());
  return 1;
} // }}}

// }}}
