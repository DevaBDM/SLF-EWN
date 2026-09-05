-- Project-local Neovim config for bare-metal MCU CMake workflow with
-- nvimdots (toggleterm). Currently wired for AVR; the keymaps themselves
-- are architecture-agnostic (they just drive the CMake targets above).
local toolchain_file = "../toolchain/avr-toolchain.cmake"
local hex_output_path = "/mnt/d/DevaBDM/Embedded Systems/Proteus/LandSlideDetectorReceiver"
local project_name = "landslideReciver_device"

local function run_cmd(cmd)
  require("toggleterm.terminal").Terminal:new({
    cmd = cmd,
    hidden = true,
    direction = "float",
    close_on_exit = false,
  }):toggle()
end

vim.keymap.set("n", "<leader>cC", function()
  run_cmd(string.format(
    "cmake -B build -DCMAKE_TOOLCHAIN_FILE=%s -DHEX_OUTPUT_PATH='%s' .",
    toolchain_file, hex_output_path
  ))
end, { desc = "Configure CMake" })

vim.keymap.set("n", "<leader>cb", function()
  run_cmd("cmake --build build")
end, { desc = "Build ELF + HEX" })

vim.keymap.set("n", "<leader>cB", function()
  run_cmd(string.format("cmake --build build --target %s", project_name))
end, { desc = "Build ELF only" })

vim.keymap.set("n", "<leader>ch", function()
  run_cmd("cmake --build build --target hex")
end, { desc = "Generate HEX file" })

vim.keymap.set("n", "<leader>cf", function()
  run_cmd("cmake --build build --target flash")
end, { desc = "Upload HEX to MCU" })

vim.keymap.set("n", "<leader>ca", function()
  run_cmd(string.format(
    "cmake -B build -DCMAKE_TOOLCHAIN_FILE=%s -DHEX_OUTPUT_PATH='%s' . && cmake --build build --target flash",
    toolchain_file, hex_output_path
  ))
end, { desc = "All-in-one: configure, build, flash" })
