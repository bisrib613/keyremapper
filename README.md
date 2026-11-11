# Key Remapper

Windows keyboard remapper (C# WinForms, .NET 8). Uses Windows low-level keyboard hooks and SendInput. No drivers, telemetry, license checks, or bundled software.

## Download
The downloadable **installer** is available in [GitHub Releases](https://github.com/bisrib613/keyremapper/releases), named `KeyRemapper_Setup.exe`. This is an Inno Setup installer, **not** a portable executable.

## Features
- Add/remove keyboard remappings.
- Enable/disable remapping.
- Automatically persist mappings in `%APPDATA%\KeyRemapper\settings.json`.
- JSON import/export and restore defaults.
- Installer creates uninstall entry and Start Menu shortcut.

## Build
Pushes to `main` build the Windows x64 application and installer using GitHub Actions. Successful builds publish or update the `v1.0.0` Release.

## Limitations
- No kernel driver; secure desktop and higher-integrity application windows can be inaccessible.
- Repeated keys, modifier combinations, non-US layouts, and physical keyboards need practical testing before treating this as production-ready.
- Unsigned executables may trigger SmartScreen warnings.