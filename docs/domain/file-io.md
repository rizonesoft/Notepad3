# File I/O

## Load and save path

`FileSave()` / `FileLoad()` in `Notepad3.c` call `FileIO()`, which dispatches to `EditSaveFile()` / `EditLoadFile()` in `Edit.c`.

## Saving

- Atomic save uses a temporary file and `ReplaceFileW`, controlled by `Settings2.AtomicFileSave`.
- Atomic save is skipped for emergency session-end saves (`FSF_EndSession`).
- If temp-file setup fails, saving falls back to writing the target directly. After a successful temp-file write, replacement tries `ReplaceFileW` for an existing target, then `MoveFileExW` with replace/write-through flags; a replacement failure makes the save fail.
- `Globals.dwLastError` holds the Win32 error. In `FileSave()`, check `ERROR_ACCESS_DENIED` and `ERROR_PATH_NOT_FOUND` before the generic fallback.

## File watching

`InstallFileWatching()` uses `FindFirstChangeNotificationW` on the parent directory. Stop watching with `InstallFileWatching(false)` before saving, then reinstall it with `InstallFileWatching(true)` afterward.
