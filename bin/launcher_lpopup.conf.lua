local lpopup = require("lpopup")

function lpopup_execute(argv)
  local const = lpopup.apps.launcher_const
  lpopup.apps.launcher(argv, {
    system = {
      show = { 
        const.SHOW_MOUSE, 
        0, 
        0
      },
      icon = true,
    },

    rootdir = lfs.currentdir() .. [[\data]]
  })
end
