---
status: agent-observed
verified-at: b56c3e65dcc0eb6f783a1ef7e5a2289e3ec71adc
owner: "@RaiKoHoff"
anchors:
  - path: src/PathLib.c
    symbol: PrependLongPathPrefix
  - path: src/PathLib.c
    symbol: Path_StrgComparePath
  - path: src/PathLib.c
    symbol: Path_CreateDirectoryEx
  - path: src/PathLib.c
    symbol: Path_ReplaceFile
  - path: src/PathLib.c
    symbol: Path_MoveFileEx
  - path: src/Config/Config.cpp
    symbol: CreateIniFile
  - path: src/Notepad3.c
    symbol: Path_CreateDirectoryEx
---

# PathLib and Long Paths

Evidence scope: code inspection of long-path prefix conditions, comparison normalization/empty inputs, two-path wrapper limitations, and the raw directory-status contract and unsafe callers at the commit above. The wrapper table is navigation, not a separately tested API guarantee. Human review is pending.

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

- `Path_ReplaceFile()` prefixes only the replaced path; its replacement string is passed through. `Path_MoveFileEx()` prefixes only the destination; its source string is passed through. These wrappers do not make both arguments long-path safe.
- Compare nonempty paths with `Path_StrgComparePath()` (`src\PathLib.c`): it normalizes only when `bNormalize` is true, then uses locale-independent, case-insensitive `CompareStringOrdinal`. Empty inputs are special: an empty first path returns `-1`, otherwise an empty second path returns `+1`; two empty paths do not compare equal. For raw wide strings, prefer `CompareStringOrdinal(s1, -1, s2, -1, TRUE)` to retain ordinal rather than locale-dependent comparison.
- `Path_CreateDirectoryEx(hpth)` in `src\PathLib.c` currently returns the raw `SHCreateDirectoryExW` status despite its `HRESULT` declaration. Check `ERROR_SUCCESS` explicitly; if an existing directory is acceptable, handle `ERROR_ALREADY_EXISTS` and `ERROR_FILE_EXISTS` as documented existing-directory statuses. Do not use `SUCCEEDED` / `FAILED` or compare with `HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)`. The existing HRESULT-style checks in `src\Config\Config.cpp:CreateIniFile` and `src\Notepad3.c` (snapshot creation around line 3997 and directory recreation around line 12872) are not safe templates: positive Win32 error codes pass them.
- [Microsoft's `SHCreateDirectoryExW` contract](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreatedirectoryexw) documents a fully qualified path of at most 248 characters, including the terminating null. Prefixing in the wrapper is not evidence that this Shell API supports arbitrary long paths; its runtime limit has not been tested here.
