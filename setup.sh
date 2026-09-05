#!/usr/bin/env bash
# Bare-metal MCU project setup script (currently targeting AVR)
# Creates CMakeLists.txt, .nvim.lua, a bare main.c, .gitignore, an optional
# starter toolchain file, initializes git, and commits.
set -euo pipefail

echo "=== Bare-Metal MCU Project Setup (AVR) ==="

# ---- Prompt + validate inputs (press Enter to accept the default) -----

DEFAULT_PROJECT_NAME="$(basename "$PWD")"
DEFAULT_MCU="atmega328p"
DEFAULT_F_CPU="1000000"
DEFAULT_PROGRAMMER="usbasp"
DEFAULT_HEX_OUTPUT_PATH="${PWD}/build/hex"
DEFAULT_TOOLCHAIN_PATH="../toolchain/avr-toolchain.cmake"

read -rp "Project name [${DEFAULT_PROJECT_NAME}]: " PROJECT_NAME
PROJECT_NAME="${PROJECT_NAME:-$DEFAULT_PROJECT_NAME}"
if [[ -z "$PROJECT_NAME" || ! "$PROJECT_NAME" =~ ^[A-Za-z0-9_-]+$ ]]; then
	echo "Error: project name must be non-empty and contain only letters, digits, '_' or '-'." >&2
	exit 1
fi

read -rp "MCU type [${DEFAULT_MCU}]: " MCU
MCU="${MCU:-$DEFAULT_MCU}"

read -rp "CPU frequency in Hz, no suffix [${DEFAULT_F_CPU}]: " F_CPU
F_CPU="${F_CPU:-$DEFAULT_F_CPU}"
if [[ ! "$F_CPU" =~ ^[0-9]+$ ]]; then
	echo "Error: CPU frequency must be a plain integer (e.g. 1000000). The UL suffix is added automatically." >&2
	exit 1
fi

read -rp "Programmer [${DEFAULT_PROGRAMMER}]: " PROGRAMMER
PROGRAMMER="${PROGRAMMER:-$DEFAULT_PROGRAMMER}"

read -rp "HEX output directory, absolute path [${DEFAULT_HEX_OUTPUT_PATH}]: " HEX_OUTPUT_PATH
HEX_OUTPUT_PATH="${HEX_OUTPUT_PATH:-$DEFAULT_HEX_OUTPUT_PATH}"
if [[ "$HEX_OUTPUT_PATH" != /* ]]; then
	echo "Error: HEX output directory must be an absolute path (starting with '/')." >&2
	exit 1
fi

read -rp "Path to toolchain CMake file [${DEFAULT_TOOLCHAIN_PATH}]: " TOOLCHAIN_PATH
TOOLCHAIN_PATH="${TOOLCHAIN_PATH:-$DEFAULT_TOOLCHAIN_PATH}"

GENERATE_TOOLCHAIN=0
if [[ ! -f "$TOOLCHAIN_PATH" ]]; then
	echo "'$TOOLCHAIN_PATH' does not exist yet."
	read -rp "Generate a starter bare-metal toolchain file there now? [Y/n] " GEN_CONFIRM
	if [[ -z "$GEN_CONFIRM" || "$GEN_CONFIRM" =~ ^[Yy]$ ]]; then
		GENERATE_TOOLCHAIN=1
	else
		echo "Warning: proceeding without a toolchain file. The .nvim.lua keybinds will" >&2
		echo "         reference '$TOOLCHAIN_PATH' anyway, but 'Configure CMake' will fail until it exists." >&2
	fi
fi

# ---- Idempotency: warn before clobbering existing project files -------

EXISTING=()
for f in CMakeLists.txt main.c .nvim.lua .gitignore; do
	[[ -e "$f" ]] && EXISTING+=("$f")
done
if [[ ${#EXISTING[@]} -gt 0 ]]; then
	echo "The following files already exist and will be overwritten:"
	printf '  %s\n' "${EXISTING[@]}"
	read -rp "Continue? [y/N] " CONFIRM
	if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
		echo "Aborted."
		exit 1
	fi
fi

# ---- Create CMakeLists.txt --------------------------------------------
#
# Structure: generic bare-metal settings first (language standard, warning
# levels), then AVR-specific bits (MCU flags, HEX/flash targets) clearly
# marked so they're easy to swap out when targeting a different bare-metal
# architecture (e.g. ARM Cortex-M) later.

cat >CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.15)
project(${PROJECT_NAME} C)

# ---- Language standard: strict ISO C, no compiler-specific extensions -----
# Extensions off = no GNU-isms creeping in (statement expressions, typeof,
# non-standard attributes, etc.) that would silently break portability to a
# different compiler or architecture down the line.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(MCU "${MCU}")
set(F_CPU "${F_CPU}UL")
set(PROGRAMMER "${PROGRAMMER}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(HEX_OUTPUT_PATH "${HEX_OUTPUT_PATH}"
    CACHE PATH "Directory to place generated HEX file")
set(HEX_FILE "\${HEX_OUTPUT_PATH}/\${PROJECT_NAME}.hex")

# ---- Build type: Debug by default -------------------------------------
# Switch to Release by hand when you're done developing — either edit the
# line below to 'Release', or reconfigure with -DCMAKE_BUILD_TYPE=Release.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug or Release" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# ---- Warnings: maximum, treated as errors ----------------------------------
# NOTE: -Wconversion / -Wsign-conversion can get noisy on register-heavy MCU
# code, where 8-bit values get implicitly promoted to int. If they're more
# noise than signal for your code, comment them out — everything else here
# is worth leaving on.
add_compile_options(
  -mmcu=\${MCU}
  -Wall
  -Wextra
  -Wpedantic
  -Werror
  -Wshadow
  -Wdouble-promotion
  -Wformat=2
  -Wundef
  -Wconversion
  -Wsign-conversion
  -Wcast-align
  -Wcast-qual
  -Wlogical-op
  -Wredundant-decls
  -Wmissing-declarations
  -Wmissing-prototypes
  -Wstrict-prototypes
  -Wold-style-definition
  -Wnested-externs
  -Wpointer-arith
  -Wwrite-strings
  -ffunction-sections
  -fdata-sections
)

# ---- Debug vs Release: optimization level, debug symbols, assert control --
# -Og keeps optimizations debugger-friendly; -Os in Release favors the small
# code size that matters on flash-constrained MCUs. DEBUG/NDEBUG gate any
# assert()/debug-only code in your sources.
add_compile_options(
  "\$<\$<CONFIG:Debug>:-Og>"
  "\$<\$<CONFIG:Debug>:-g3>"
  "\$<\$<CONFIG:Release>:-Os>"
)
add_compile_definitions(
  "\$<\$<CONFIG:Debug>:DEBUG>"
  "\$<\$<CONFIG:Release>:NDEBUG>"
)

add_definitions(-DF_CPU=\${F_CPU})
add_link_options(-mmcu=\${MCU} -Wl,--gc-sections)

add_executable(\${PROJECT_NAME} main.c)

# ---- AVR-specific: HEX generation + flashing -------------------------------
# Swap this section when retargeting to another bare-metal architecture —
# other families use different objcopy output formats and flashing tools
# (e.g. arm-none-eabi-objcopy + openocd/st-flash for ARM Cortex-M).
add_custom_command(
  OUTPUT \${HEX_FILE}
  COMMAND \${CMAKE_COMMAND} -E make_directory \${HEX_OUTPUT_PATH}
  COMMAND avr-objcopy -O ihex -j .text -j .data \$<TARGET_FILE:\${PROJECT_NAME}> \${HEX_FILE}
  DEPENDS \${PROJECT_NAME}
  COMMENT "Generating HEX file at \${HEX_FILE}"
)
add_custom_target(hex ALL DEPENDS \${HEX_FILE})
add_custom_target(
  flash
  COMMAND avrdude -c \${PROGRAMMER} -p \${MCU} -U flash:w:\${HEX_FILE}:i
  DEPENDS hex
  COMMENT "Flashing HEX file to \${MCU}..."
)
EOF

# ---- Create bare main.c skeleton ---------------------------------------

cat >main.c <<'EOF'
#include <avr/io.h>

int main(void) {
    // TODO: add your application code here
    while (1) {
        // main loop
    }
}
EOF

# ---- Create project-local Neovim config --------------------------------

cat >.nvim.lua <<EOF
-- Project-local Neovim config for bare-metal MCU CMake workflow with
-- nvimdots (toggleterm). Currently wired for AVR; the keymaps themselves
-- are architecture-agnostic (they just drive the CMake targets above).
local toolchain_file = "${TOOLCHAIN_PATH}"
local hex_output_path = "${HEX_OUTPUT_PATH}"
local project_name = "${PROJECT_NAME}"

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
EOF

# ---- Create .gitignore ---------------------------------------------------

cat >.gitignore <<'EOF'
build/
*.hex
*.elf
*.o
compile_commands.json
EOF

# ---- Optionally generate a starter toolchain file ------------------------
#
# Generic bare-metal settings (CMAKE_SYSTEM_NAME Generic, the
# CMAKE_TRY_COMPILE_TARGET_TYPE workaround, CMAKE_FIND_ROOT_PATH_MODE_*)
# apply to any freestanding target. Only CMAKE_SYSTEM_PROCESSOR and the
# compiler names are AVR-specific — update those two spots when retargeting
# to e.g. ARM Cortex-M (arm-none-eabi-gcc) later.

if [[ "$GENERATE_TOOLCHAIN" -eq 1 ]]; then
	TOOLCHAIN_DIR="$(dirname "$TOOLCHAIN_PATH")"
	mkdir -p "$TOOLCHAIN_DIR"
	cat >"$TOOLCHAIN_PATH" <<'EOF'
# ---------------------------------------------------------------------------
# Bare-metal cross-compilation toolchain file.
#
# Currently targets AVR (avr-gcc). To retarget for a different bare-metal
# architecture later (e.g. ARM Cortex-M), update CMAKE_SYSTEM_PROCESSOR and
# the three compiler lines below. Everything else here is architecture-
# agnostic and can stay as-is.
# ---------------------------------------------------------------------------

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

# Cross compilers
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER avr-gcc)

# Bare-metal targets have no OS to run a test executable on, so make CMake's
# compiler sanity checks link a static library instead of an executable —
# otherwise every try_compile() check (including the initial compiler test)
# fails for lack of a runnable host.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Only look inside the toolchain's own sysroot for libraries/headers/
# packages, never the host system's — prevents accidentally linking against
# host (x86/ARM Linux) libraries instead of the target's.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
EOF
	echo "Generated starter toolchain file at '$TOOLCHAIN_PATH'."
fi

# ---- Initialize git repo and commit --------------------------------------

if [[ ! -d .git ]]; then
	git init
fi

git add CMakeLists.txt main.c .nvim.lua .gitignore

if git diff --cached --quiet; then
	echo "Nothing to commit (files unchanged from last commit)."
else
	if ! git commit -m "Initial AVR project setup for ${PROJECT_NAME}"; then
		echo "Error: git commit failed. Do you have user.name/user.email configured?" >&2
		echo "  git config --global user.name  'Your Name'" >&2
		echo "  git config --global user.email 'you@example.com'" >&2
		exit 1
	fi
fi

echo "✅ Project '${PROJECT_NAME}' setup complete!"
echo "Generated CMakeLists.txt, main.c skeleton, .nvim.lua, .gitignore, and initialized git repository."
