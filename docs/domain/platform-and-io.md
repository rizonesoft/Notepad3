# Platform and I/O

## PathLib and long paths

Do not call Win32 file APIs directly with `Path_Get(hpth)`. Use PathLib wrappers, which conditionally apply the `\\?\` prefix when long paths are not enabled and the path is at least 260 characters.

| Wrapper | Win32 API |
|---|---|
| `Path_CreateFile` | `CreateFileW` |
| `Path_DeleteFile` | `DeleteFileW` |
| `Path_GetFileAttributes` / `Path_GetFileAttributesEx` | `GetFileAttributes[Ex]W` |
| `Path_SetFileAttributes` | `SetFileAttributesW` |
| `Path_ReplaceFile` | `ReplaceFileW` |
| `Path_MoveFileEx` | `MoveFileExW` |
| `Path_FindFirstFile` | `FindFirstFileW` |
| `Path_CreateDirectoryEx` | `SHCreateDirectoryExW` |
| `Path_IsExistingFile` / `Path_IsExistingDirectory` | `GetFileAttributesW` plus a type check |

- Compare paths with `Path_StrgComparePath()` (normalization and locale-independent, case-insensitive `CompareStringOrdinal`). For raw wide strings use `CompareStringOrdinal(s1, -1, s2, -1, TRUE)`. Never use `_wcsicmp` / `_wcsnicmp`.
- Use `Path_CreateDirectoryEx(hpth)` to create directories. Success is `SUCCEEDED(hr)` or `hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)`; see `CreateIniFile()` in `Config.cpp`.

## Dark mode and rendering

- `src\DarkMode\` implements Windows 10/11 dark mode using IAT hooks on uxtheme/user32; stub DLLs are included.
- ARM64 defaults to `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2), preserving the Direct2D back buffer and avoiding flicker on Qualcomm Adreno with Windows 11 25H2 DWM. The main window uses `WS_EX_COMPOSITED`. Users can override via `RenderingTechnology` / the View menu.
- `MsgThemeChanged()` wraps bar recreation, lexer reset, and restyling in `WM_SETREDRAW FALSE/TRUE`, then calls `RedrawWindow()` once. DarkMode's `RedrawWindow()` in `ListViewUtil.hpp` omits `RDW_ERASE`.

## Python environment

For scripting tasks, `.venv\` may provide Python 3.14 and is git-ignored. If absent, check for `py` or `python` before relying on Python availability.

```powershell
.venv\Scripts\python.exe <script.py>
.venv\Scripts\pip.exe install <package>
```
