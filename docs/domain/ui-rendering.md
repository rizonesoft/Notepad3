# Dark Mode and Rendering

- `src\DarkMode\` implements Windows 10/11 dark mode using IAT hooks on uxtheme/user32; stub DLLs are included.
- ARM64 defaults to `SC_TECHNOLOGY_DIRECTWRITERETAIN` in `src\Config\Config.cpp:LoadSettings`; its source comment records an Adreno/Windows 11 25H2 flicker workaround, not a general guarantee of flicker-free rendering. The ARM64 main window uses `WS_EX_COMPOSITED` in `src\Notepad3.c`. Users can override via `RenderingTechnology` / the View menu; assessing the workaround's effect requires testing on the affected hardware.
- `MsgThemeChanged()` wraps bar recreation, lexer reset, and restyling in `WM_SETREDRAW FALSE/TRUE`, then calls `RedrawWindow()` once. DarkMode's `RedrawWindow()` in `ListViewUtil.hpp` omits `RDW_ERASE`.
