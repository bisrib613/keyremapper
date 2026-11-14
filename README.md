# Key Remapper — native clean-room rebuild

This repository contains a native Win32 x86 implementation. The previous .NET self-contained prototype was removed because it produced an unnecessarily large installer and did not match the reference application's architecture.

## Audit of the reference installer

The supplied reference installer is Inno Setup 5.5.7. Its main payload is a native x86 `keyremapper.exe`, not a .NET application. The payload executable is packed/protected, so this project does not copy proprietary source code, licensing, or protection logic. It is a clean-room implementation of the remapping behavior.

## Current native build

- Close button hides the dashboard to the system tray
- Tray menu: Open, Enable Remapping, Preferences > Run at startup / Reset Defaults, Exit
- Windows startup launches hidden to tray
- Exit unloads hooks, removes the tray icon, and terminates the single process

- Keyboard key to keyboard key
- Keyboard key to mouse button or wheel action
- Mouse button or wheel action to keyboard or mouse action
- Block a source input
- Capture source/target directly from keyboard or mouse
- Pause/resume
- Persistent mappings in `%APPDATA%\KeyRemapperClean\mappings.txt`
- Low-level keyboard and mouse hooks
- Injected-output suppression to prevent remap loops
- CI smoke test checks that both low-level hooks can be installed

## Installer

GitHub Actions builds `KeyRemapper_Setup.exe` with MSVC x86 and Inno Setup, then publishes it to GitHub Releases. It is an installer, not a portable build, and it does not bundle the .NET runtime.

## Current scope

Version 1.6.0 is focused on a stable native remapping core plus tray-first desktop behavior. It is not yet full feature parity with the reference application. Profiles, per-application rules, modifier combinations, double-press timing, folders, and tray behavior will be added after the core keyboard/mouse hook path is validated on real Windows hardware.
