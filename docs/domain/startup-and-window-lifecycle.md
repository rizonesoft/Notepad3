# Startup and Window Lifecycle

## Clipboard monitoring (Pasteboard Mode)

The runtime toggle is `IDM_EDIT_TOGGLE_PASTEBOARD`; `/B` enables Pasteboard Mode at startup. It is not persisted. `IsPasteBoardActive()` in `Notepad3.h` is the cross-translation-unit query.

- Pasteboard Mode and Tail (`FileWatching.MonitoringLog`) are mutually exclusive. Both must disable the other in `MsgInitMenu`, the tail toolbar button, and the "Monitoring Log" checkbox in `ChangeNotifyDlgProc`.
- If `/B` and persisted `MonitoringLog=true` are both present at startup, Pasteboard Mode wins for the session. The persisted INI setting is left unchanged.
- `/B` pastes the current clipboard once only on an empty untitled buffer: no file argument, no content from `/c`, and no auto-loaded MRU. Runtime toggling never auto-pastes; preserve this distinction.
- `Settings2.PasteBoardSeparator`: `\x01` means one document EOL, `\0` means none, otherwise the value is literal. The separator is suppressed on the first paste after activation and when the caret is at line start.

## Taskbar Shift+Click and new windows

- `Helpers.c` sets the window's AppUserModel properties, including `PKEY_AppUserModel_RelaunchCommand`. Explorer creates an implicit shortcut from these properties and does not refresh an existing shortcut when they change.
- A Shift+Click launch can be identified from `STARTUPINFO` (`STARTF_TITLEISLINKNAME` and the shortcut path in `lpTitle`). `Start-Process <that .lnk>` emulates this launch for testing.
- The launched process reads the shortcut's AppID and finds the topmost enabled window in that AppID group. If that window acknowledges `WM_NEWWINDOWREQUEST` within the timeout, the launching process exits; an older or hung instance may not acknowledge it.
- If there is no acknowledging target, the launched process adopts the AppID and opens a new window.
- The handoff to `DialogNewWindow` passes `-n` semantics, so a taskbar new-window request bypasses `ReuseWindow`.
- Every AppUserModel property set through the window property store must be cleared in `ClearWindowAppUserModelID()` during `WM_DESTROY`.

## Startup with a directory argument

- A directory passed as the startup path opens the file dialog rooted in that directory (`Notepad3.c` startup path handling). `OpenFileDlg()` may receive the same `HPATHL` as both output and initial-directory input; it must copy the initial directory before clearing the output path (`Dialogs.c`).
