# Dark Mode and Rendering

- `src\DarkMode\` implements Windows 10/11 dark mode using IAT hooks on uxtheme/user32; stub DLLs are included.
- ARM64 defaults to `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2), preserving the Direct2D back buffer and avoiding flicker on Qualcomm Adreno with Windows 11 25H2 DWM. The main window uses `WS_EX_COMPOSITED`. Users can override via `RenderingTechnology` / the View menu.
- `MsgThemeChanged()` wraps bar recreation, lexer reset, and restyling in `WM_SETREDRAW FALSE/TRUE`, then calls `RedrawWindow()` once. DarkMode's `RedrawWindow()` in `ListViewUtil.hpp` omits `RDW_ERASE`.
