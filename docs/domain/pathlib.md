# PathLib and Long Paths

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
