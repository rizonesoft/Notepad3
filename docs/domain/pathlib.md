---
status: agent-observed
verified-at: 874b714d748a2003b7b71e7a4d07fcf6f6b978af
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

Evidence scope: code inspection of long-path prefix conditions, comparison normalization/empty inputs, and two-path wrapper limitations; the `Path_CreateDirectoryEx` result contract is additionally exercised by `test\TestCreateDirectory.ps1` (agent-run at the commit above; the sub-248-character real-file-system cases only). The wrapper table is navigation, not a separately tested API guarantee. Human review is pending.

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
- `Path_CreateDirectoryEx(hpth)` in `src\PathLib.c` converts the raw Win32 status of `SHCreateDirectoryExW` to a real `HRESULT`: `S_OK` when created; `HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)` only when the path is an existing directory; `HRESULT_FROM_WIN32(ERROR_FILE_EXISTS)` when a file occupies the path; otherwise `HRESULT_FROM_WIN32(status)`. `SHCreateDirectoryExW` reports `ERROR_ALREADY_EXISTS` (183) for an existing file too, so the wrapper checks `Path_IsExistingDirectory()` before reporting an existing directory. "Already exists" is still a failure code: accept it with an explicit `hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)` comparison next to `FAILED(hr)`, as `src\Config\Config.cpp:CreateIniFile`, `_BuildSnapshotPath` and the directory recreation in `src\Notepad3.c` do. `src\Styles.c:_EnsureThemesDirectory` deliberately ignores the result.
- `test\TestCreateDirectory.ps1` compiles the production wrapper and those three callers against the real file system (created, existing directory, conflicting file, path failures) and injects raw statuses such as access denied; see [Windows build and test](../build-and-test/windows-build-and-test.md).
- [Microsoft's `SHCreateDirectoryExW` contract](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreatedirectoryexw) documents a fully qualified path of at most 248 characters, including the terminating null. Prefixing in the wrapper is not evidence that this Shell API supports arbitrary long paths; its runtime limit has not been tested here.
