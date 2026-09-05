# AVR Project Setup Script

This repository contains a single file: `setup.sh`.  
It is a **bootstrap script** for creating new AVR microcontroller projects with a standardized build and editor workflow.

---

## 📌 What the Script Does

When you run `setup.sh` inside an empty project directory, it will:

1. **Prompt for inputs**
   - Project name (defaults to current folder name)
   - MCU type (e.g. `atmega32`)
   - CPU frequency (Hz, e.g. `1000000`)
   - Programmer (e.g. `usbasp`)
   - HEX output directory (absolute path)
   - Path to AVR toolchain CMake file

2. **Validate inputs**
   - Ensures project name is valid
   - Ensures F_CPU is an integer
   - Ensures HEX output path is absolute

3. **Generate files**
   - `CMakeLists.txt` → build configuration with ELF, HEX, and flash targets
   - `main.c` → bare skeleton with `main()` and infinite loop
   - `.nvim.lua` → project-local Neovim config with keybindings for build/flash
   - `.gitignore` → ignores build artifacts and temporary files

4. **Initialize Git repository**
   - Runs `git init` if needed
   - Stages generated files
   - Commits them with message:
     ```
     Initial AVR project setup for <project_name>
     ```

---

## ⚙️ Usage

1. Copy `setup.sh` into an empty directory.
2. Make it executable:

   ```bash
   chmod +x setup.sh
   ```

3. Run it:
   ```bash
   ./setup.sh
   ```
4. Answer the prompts.
5. The script will generate the project files and commit them to a new Git repo.

---

## 🖥️ Neovim Integration

The generated `.nvim.lua` provides keybindings (using toggleterm in nvimdots):

| Keybinding   | Action                                |
| ------------ | ------------------------------------- |
| `<leader>cC` | Configure CMake                       |
| `<leader>cb` | Build ELF + HEX                       |
| `<leader>cB` | Build ELF only                        |
| `<leader>ch` | Generate HEX file                     |
| `<leader>cf` | Flash HEX to MCU                      |
| `<leader>ca` | All-in-one: configure → build → flash |

---

## 🚀 Typical Workflow

1. Run `setup.sh` to initialize the project.
2. Open Neovim in the project directory.
3. Use the keybindings to configure, build, and flash.
4. Start coding in `main.c`.

---

## 📝 Notes

- The script is idempotent: it warns before overwriting existing files.
- If Git user.name/email are not configured, the commit will fail with instructions.
- The HEX file is tracked as a build artifact in CMake — if deleted, it regenerates automatically.
- This setup is generic: replace `main.c` and adjust MCU/programmer for other AVR projects.

---

## 🎯 Purpose

This script is designed to **automate project setup** so you don’t have to manually create boilerplate files.
It ensures every new AVR project starts with:

- A reproducible build system
- A bare source file ready to compile
- Editor integration for fast iteration
- Version control from the very first commit
