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

## Recent files dialog (`FileMRUDlgProc`, `Dialogs.c`)

- OK on a missing entry only offers removal (`IDS_MUI_ERR_MRUDLG`). Recovery actions live in the list's context menu (right-click / Shift+F10, `_FileMRU_ContextMenu()`): open containing folder, browse for renamed/moved file, copy full path / directory / file name (`IDC_MRU_*`, `IDS_MUI_MRU_*`).
- Folder actions walk up to the nearest existing parent with `_FindExistingDirectory()`. "Browse" opens `OpenFileDlg()` there, deletes the old MRU entry (unless it is the current file) and returns the chosen path through the dialog's `DWLP_USER` `HPATHL` (`EndDialog(IDOK)`), so `IDM_FILE_RECENT` in `Notepad3.c` loads it.
- Use a local `HPATHL` in dialog actions; the dialog's `hFilePath` is shared with the `FileMRUIconThread` worker.
- Shell APIs need the `\\?\` prefix stripped: use `ShellRevealInExplorer()` / `_PathForShell()` in `Dialogs.c`.
