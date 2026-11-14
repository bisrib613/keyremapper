# Key Remapper — native clean-room rebuild

Key Remapper is a native Win32 x86 keyboard/mouse remapper. It is intentionally small, per-user, and does not bundle .NET, a service, a driver, or a helper process.

## Current build — v1.7.1

Runtime behavior:
- Closing the dashboard hides it to the system tray.
- Tray menu: Open, Enable Remapping, Preferences > Run at startup / Reset Defaults, Exit.
- Run at startup launches the app hidden to the tray.
- Exit releases active synthetic input, removes both low-level hooks, removes the tray icon, and terminates the single process.
- If Explorer restarts, the tray icon is registered again.
- If a tray icon cannot be created, the app does not hide itself into an inaccessible state.

Remapping:
- Keyboard key to keyboard key.
- Keyboard key to mouse button or wheel action.
- Mouse button or wheel action to keyboard or mouse action.
- Source and target modifier combinations.
- Modifier keys can themselves be mapping sources.
- Capture supports ordinary keys, mouse input, Escape, and modifier-only input.
- Block a source input.
- Pause/resume without leaving synthetic keys held.
- Symbol choices are resolved from the active Windows keyboard layout rather than a hard-coded US layout.
- Input injection failures are surfaced in the runtime status.

Persistence:
- Legacy migration now closes its probe handle before atomic replacement on Windows, preventing false "recovery failed" status after upgrading from v1.6.x.
- Mappings live in `%APPDATA%\KeyRemapperClean\mappings.txt`.
- v1.7 uses a validated versioned config format and migrates legacy v1.x mapping files automatically.
- Writes are temp-file + replace, with a validated last-known-good backup in `mappings.bak`.
- A corrupt primary config is recovered from a valid backup instead of silently becoming an empty mapping set.

Desktop behavior:
- Single process only; no service, driver, daemon, or scheduled remapping task.
- Per-Monitor V2 DPI-aware layout.
- Run at startup is stored per-user under HKCU and removed on uninstall.
- The installer uses Inno Setup and does not require administrator elevation.

## Build and test

GitHub Actions builds two executables from the same tagged source:
- `KeyRemapper.exe`: production binary.
- `KeyRemapperTests.exe`: isolated regression build.

The test build uses its own temporary config directory and a separate `KeyRemapper.Test` startup registry value. Production does not expose the old `--self-test` path.

The regression suite covers symbol mappings, upsert/modify, persistence recovery, dropdown wheel behavior, startup state, keyboard/mouse hooks, modifier-only and Escape capture, modifier-source mappings, safe pause while a target is held, and key-up behavior after remapping is re-enabled.

Releases are tag-driven. `VERSION`, installer metadata, PE version resources, and the application manifest must agree before CI will publish an immutable release asset.

## Known platform limitation

The app runs as the current user with `asInvoker`. Windows may block `SendInput` into a higher-integrity/elevated application because of UIPI. The app reports an injection failure instead of silently claiming success.

Release binaries are currently not Authenticode-signed. Code signing requires a trusted signing certificate and is intentionally not faked or bypassed.

## Reference implementation note

The supplied reference installer uses Inno Setup and contains a native x86 payload. This repository is a clean-room implementation of the observed remapping behavior; it does not copy proprietary source code, licensing, or protection logic.
