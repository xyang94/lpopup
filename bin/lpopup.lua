local lpopup = {
  SHOW_CENTER = 0,
  SHOW_COORD  = 1,
  SHOW_MOUSE  = 2,
  SHOW_WINDOW = 3,

  OS = "windows",
  PATHSEP = "\\",
  apps = {},
  _root_menu = nil
}

local function getornil(v, name) -- {{{
  if type(v) ~= "table" then
    return nil
  else
    return v[name]
  end
end -- }}}

local function extendtable(dest, source, ...) -- {{{
  local options = lpopup.options({include_functions=false}, {...})
  for k, v in pairs(source) do
    if not options.include_functions and type(v) ~= "function" then
      dest[k] = v
    end
  end
  return dest
end -- }}}

local utf8skipdata = { -- {{{
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
} -- }}}

lpopup.root_menu = function() -- {{{
  if lpopup._root_menu == nil then
    lpopup._root_menu = lpopup.Menu.new()
  end
  return lpopup._root_menu
end -- }}}

lpopup.options = function(defaults,_opts) -- {{{
  local options = #_opts>0 and _ops or {}
  for k, v in pairs(options) do
    defaults[k] = v
  end
  return defaults
end -- }}}

lpopup.utf8iter = function(str) -- {{{
  local length = #str
  local pos    = 1
  return function()
    if pos <= length then
      local size = utf8skipdata[string.byte(str, i)+1]
      local ret = string.sub(str, pos, pos+size-1)
      pos = pos + size
      return ret
    end
  end
end -- }}}

lpopup.tokenize = function(text) -- {{{
  local tokens = {}
  local state = 0
  local buf = {}
  local pos = 2
  local utf8list = {""}
  for v in lpopup.utf8iter(text) do table.insert(utf8list, v) end

  while pos < #utf8list+2 do
    local char = utf8list[pos]
    local prev = utf8list[pos-1]
    if state == 0 then
      if char == " " then
      elseif char == [["]] then
        buf = {}
        state = 1
      else
        buf = {char}
        state = 2
      end
    elseif state == 1 then
      if prev == "\\" then
        if char == [["]] then
          table.insert(buf, [["]])
        else
          table.insert(buf, "\\" .. char)
        end
      else
        if char == [["]] then
          table.insert(tokens, table.concat(buf, ""))
          state = 0
        elseif char == "\\" then
        else
          table.insert(buf, char)
        end
      end
    elseif state == 2 then
      if char == " " then
        table.insert(tokens, table.concat(buf, ""))
        state = 0
      else
        table.insert(buf, char)
      end
    end
    pos = pos + 1
  end
  return tokens
end -- }}}

lpopup.nop = function(...) -- {{{
end -- }}}

lpopup.template = function(tpl, map) -- {{{
  local kmap = {}
  for k, v in pairs(map) do
    kmap[tostring(k)] = v
  end
  local ret = string.gsub(tpl, [[%${(%w+)}]], function(s)
      return kmap[s]
  end)
  return ret
end -- }}}

lpopup.launch = function(cmd, ...) -- {{{
  return lpopup.call(cmd, lpopup.options({
    hide = false,
    async = true,
  },{...}))
end -- }}}

lpopup.capture = function(cmd, ...) -- {{{
  local ret = lpopup.call(cmd, lpopup.options({
    hide = true,
    async = false,
  }, {...}))
  ret.stdout = string.gsub(ret.stdout, "\r\n", "\n")
  ret.stderr = string.gsub(ret.stderr, "\r\n", "\n")
  return ret
end -- }}}

lpopup.parse_shortcut_file = function(path) -- {{{
  local fp = io.open(path, "rb")
  local link = fp:read("*a")
  local at = function(pos) return string.byte(string.sub(link, pos, pos)) end
  local function bytes2short(off)
    return bit.bor(bit.lshift(bit.band(at(off+1), 0xff), 8), bit.band(at(off), 0xff))
  end
  local function null_delimited_string(offset)
    local i = offset
    while string.sub(link, i, i) ~= string.char(0) do
      i = i+1
    end
    return string.sub(link, offset, i-1)
  end

  fp:close()
  local is_directory = bit.band(at(0x19), 0x10) > 0
  local shell_len = 0
  if bit.band(at(0x15), 0x1) > 0 then
    shell_len = bytes2short(0x4d) + 2
  end

  local file_start = 0x4d + shell_len
  local file_location_info_flag = at(file_start + 0x08)
  local is_local = bit.band(file_location_info_flag, 2) == 0
  local finalname_offset = at(file_start + 0x18) + file_start
  local finalname = null_delimited_string(finalname_offset)
  if is_local then
    basename_offset = at(file_start + 0x10) + file_start
    basename = null_delimited_string(basename_offset)
    return basename .. finalname
  else
    local networkvolumetable_offset = at(file_start + 0x14) + file_start
    local sharename_offset = at(networkvolumetable_offset + 0x08) + networkvolumetable_offset 
    local sharename = null_delimited_string(sharename_offset)
    return  sharename .. "\\" .. finalname
  end
end -- }}}

lpopup._shortcutkeys = {}
lpopup.add_shortcutkey = function(menu, str) -- {{{
  if lpopup._shortcutkeys[menu:id()] == nil then
    lpopup._shortcutkeys[menu:id()] = {}
  end
  local keys = lpopup._shortcutkeys[menu:id()]
  local m = string.match(str, [[.*&([a-zA-Z])]])
  if m then
    keys[string.upper(m)] = 1
    return str
  end

  local ok = false
  for c in lpopup.utf8iter(str) do
    local upc = string.upper(c)
    if not ok and #c == 1 and string.byte(upc) >= 65 and string.byte(upc) < 90 and keys[upc] == nil then
      ok = upc
      keys[upc] = 1
      break
    end
  end
  return str .. (ok and " (&"..ok..")" or "")
end -- }}}

lpopup.apps.ext_const = extendtable({ -- {{{
  NATIVE_CONTEXT_MENU = 1,
  BASENAME = 1,
  DIRICON = [[c:\windows]]
}, lpopup) -- }}}

lpopup.apps.ext = function(argv, cnf) -- {{{
  local const = lpopup.apps.ext_const
  local enableicon = cnf.system.icon
  if #argv == 0 then
    os.exit(0)
  end

  local function build_menu(menu, menu_data)
    for i, v in ipairs(menu_data) do
      local vicon = getornil(v[3], "icon")
      if string.match(v[1], [[^%-%-%-.*]]) then
        if menu:item_count() > 0  then
          menu:add_hseparator()
        end
      elseif string.match(v[1], [[^%|.*]]) then
        if menu:item_count() > 0  then
          menu:add_vseparator()
        end
      elseif type(v[2]) == "table" then
        local submenu = lpopup.Menu.new()
        menu:add_submenu(lpopup.add_shortcutkey(menu, v[1]), submenu, {icon=enableicon and (vicon or const.DIRICON) or nil})
        build_menu(submenu, v[2])
      else
        if v[1] == const.BASENAME then
          v[1] = string.match(argv[1], ".*"..lpopup.PATHSEP.."(.*)")
        end
        if v[2] == const.NATIVE_CONTEXT_MENU then
          menu:add_file_context(lpopup.add_shortcutkey(menu, v[1]), argv[1], {icon=enableicon and (vicon or "auto") or nil})
        elseif type(v[2]) == "function" then
          menu:add_item(lpopup.add_shortcutkey(menu, v[1]), v[2],{icon=enableicon and vicon or nil})
        else
          local expanded = lpopup.template(v[2], argv)
          local firstitem = "" 
          if enableicon and v[3] == nil then
            firstitem = lpopup.tokenize(expanded)[1]
            -- if string.find(firstitem, lpopup.PATHSEP) == nil then
            --   local where = lpopup.capture([[where "]] .. firstitem .. [["]])
            --   if where.code == 0 then
            --     firstitem = string.match(lpopup.local2utf8(where.stdout), "([^\n]+)\n?")
            --   end
            -- end
          end
          menu:add_item(lpopup.add_shortcutkey(menu, v[1]), function()
            lpopup.shell_execute(expanded, {workdir=getornil(v[3], "workdir")})
          end, {icon=enableicon and (vicon or firstitem) or nil})
        end
      end
    end
  end

  local root = lpopup.root_menu()

  local function menu_all() 
    if cnf.items[".*"] ~= nil then
      build_menu(root, cnf.items[".*"])
    end
  end

  local function menu_type()
    local path  = string.gsub(argv[1], [[^(.*)%]]..lpopup.PATHSEP..[[$]], "%1")
    local fso   = lfs.attributes(path)

    if fso ~= nil then 
      if fso.mode == "directory" then
        menu_data = cnf.items[".directory"]
        if menu_data ~= nil then
          build_menu(root, menu_data)
        end
      elseif fso.mode == "file" then
        menu_data = cnf.items[".file"]
        if menu_data ~= nil then
          build_menu(root, menu_data)
        end
      end
    end
  end

  local function menu_ext() 
    local found = string.match(argv[1], [[^.*%.(%w+)$]])
    if found then
      local ext = found
      local menu_data = nil
      for k, v in pairs(cnf.items) do
        if string.match(k, ext.."%|") or string.match(k, "%|"..ext) or k == ext then
          menu_data = v
          break
        end
      end
      if menu_data ~= nil then
        build_menu(root, menu_data)
      end
    end
  end

  for i, v in ipairs(cnf.system.order) do
    if v == "all" then
      menu_all()
    elseif v == "type" then
      menu_type()
    elseif v == "ext" then
      menu_ext()
    end
  end

  root:show(unpack(cnf.system.show))
end -- }}}

lpopup.apps.launcher_const = extendtable({ -- {{{
  DIRICON = [[c:\windows]]
}, lpopup) -- }}}

lpopup.apps.launcher = function(argv, cnf) -- {{{
  local const = lpopup.apps.launcher_const
  local enableicon = cnf.system.icon
  local function walkdir(menu, dir)
    for file in lfs.dir(dir) do
      local utf8file = lpopup.local2utf8(file)
      if utf8file ~= "." and utf8file ~= ".." and string.sub(utf8file, 1,1) ~= "_" then
        local fullpath = dir .. const.PATHSEP .. file
        local label = utf8file
        if string.match(label, "%d+_(.*)") then
          label = string.gsub(label, "%d+_(.*)", "%1")
        end
        if string.match(label, "(.*)%.lnk") then
          label = string.gsub(label, "(.*)%.lnk", "%1")
        end

        label = lpopup.add_shortcutkey(menu, label)

        if lfs.attributes(fullpath).mode == "directory" then
          local submenu = lpopup.Menu.new()
          menu:add_submenu(label, submenu, {icon=enableicon and const.DIRICON or nil})
          walkdir(submenu, fullpath)
        elseif string.match(utf8file, ".*%.lnk") then
          local realpath = lpopup.parse_shortcut_file(fullpath)
          menu:add_item(label, function()
            lpopup.shell_execute([["]] .. lpopup.local2utf8(fullpath) .. [["]])
          end, {icon=enableicon and realpath or nil})
        end
      end
    end
  end

  local rootdir = cnf.rootdir
  local root = lpopup.root_menu()
  walkdir(root, rootdir)
  root:show(unpack(cnf.system.show))

end -- }}}

--　
if type(_lp_module) == "function" then
  for k,v in pairs(_lp_module()) do lpopup[k] = v end
end
return lpopup
