---
status: agent-observed
verified-at: cf8b520fc97bf039b68497041e61bbed2b7a88de
owner: "@RaiKoHoff"
anchors:
  - path: src/PathLib.c
    symbol: PrependLongPathPrefix
  - path: src/PathLib.c
    symbol: Path_StrgComparePath
  - path: src/PathLib.c
    symbol: Path_CreateDirectoryEx
---

# PathLib and Long Paths

Evidence scope: code inspection of long-path prefix conditions, comparison normalization, and the raw directory-status contract at the commit above. The wrapper table is navigation, not a separately tested API guarantee. Human review is pending.

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

- Compare paths with `Path_StrgComparePath()` (`src\PathLib.c`): it normalizes only when `bNormalize` is true, then uses locale-independent, case-insensitive `CompareStringOrdinal`. For raw wide strings use `CompareStringOrdinal(s1, -1, s2, -1, TRUE)`. Never use `_wcsicmp` / `_wcsnicmp` for path identity.
- `Path_CreateDirectoryEx(hpth)` in `src\PathLib.c` currently returns the raw `SHCreateDirectoryExW` status despite its `HRESULT` declaration. Check `ERROR_SUCCESS` or `ERROR_ALREADY_EXISTS` explicitly; do not use `SUCCEEDED` / `FAILED` or compare with `HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)`. The existing HRESULT-style check in `src\Config\Config.cpp:CreateIniFile` is not a safe template: positive Win32 error codes would pass it.
