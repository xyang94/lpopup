local lpopup = require("lpopup")

function lpopup_execute(argv)
  local const = lpopup.apps.ext_const
  lpopup.apps.ext(argv, {
    system = {
      show = { 
        const.SHOW_MOUSE, 
        0, 
        0
      },
      icon = true,
      order = {"ext", "type", "all"}
    },
    items = {
      [".*"] = {
        {"--------------"},
        {"context", const.NATIVE_CONTEXT_MENU, {icon=const.DIRICON}},
        {argv[1], function()
          lpopup.shell_execute([["]]..argv[1]..[["]])
        end, {icon=argv[1]}}
      },
      [".directory"] = {
        {"explorer", [[C:\Windows\explorer.exe "${1}"]]},
      },
      ["txt|bat"] = {
        {"notepad", [[c:\windows\system32\notepad.exe "${1}"]]},
        {"|"},
        {"notepads", {
            {"notepad1", [[c:\windows\system32\notepad.exe "${1}"]]},
            {"notepad2", [[c:\windows\system32\notepad.exe "${1}"]]},
        }}
      }
    }
  })
end
