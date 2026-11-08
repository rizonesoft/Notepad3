# Clipboard Monitoring (Pasteboard Mode)

Use Notepad3 as a clipboard scratchpad: capture text copied from other applications, edit the collected snippets, and save them when needed. This guide covers the workflow extended for [rizonesoft/Notepad3#5425](https://github.com/rizonesoft/Notepad3/issues/5425).

**Monitoring is off by default and its active state is not persisted to the INI.** Start it with `/b` or **Edit -> Toggle Clipboard Monitoring**. Stop it with the same menu command, or double-click **CBS** in the status bar's INS/OVR field. Stopping leaves the document open and restores normal save-prompt behavior.

## Capture once, or keep collecting

```bat
Notepad3.exe /c
Notepad3.exe /b
Notepad3.exe /b /i
```

| Launch | Behavior |
|---|---|
| `/c` | Open a new window containing the current clipboard text once; no ongoing monitoring. |
| `/b` | Start monitoring. Also paste the current clipboard once if the document is empty and Untitled after startup loading. |
| `/b /i` | Start monitoring, briefly show the window, then minimize if the conditions below allow it. |

The startup paste does not replace an opened file or content already loaded into the document. **Turning monitoring on through the menu does not paste the current clipboard immediately**; it waits for a new clipboard update.

Clipboard text is pasted **at the current caret/selection**, not forced to the end of the document. Leave the caret at the end with no selection when collecting a running list. Automatic pasting temporarily disables auto-indent; an entry and its separator/timestamp form one undo transaction.

This is not a clipboard-history manager. Clipboard updates are debounced, so several rapid copies can result in only the latest text being captured. Repeated notifications for an unchanged clipboard sequence are ignored, and Notepad3's own copy/cut commands are excluded from automatic recapture.

## Briefly show, then minimize

With `/b /i`, the default delay is **1500 ms**. Edit the active INI with **Ctrl+F7**, put the setting under `[Settings2]`, and restart:

```ini
[Settings2]
PasteBoardInitialShowMs=1000
```

Allowed range: **500-5000 ms**. The delay applies only to the `/b /i` startup combination, not to a later paste or the menu toggle. Minimization uses the configured tray/taskbar preference.

When the timer fires, Notepad3 stays visible if:

- **Ctrl is held down** at that instant.
- The clipboard has **nothing pasteable**.

The second check concerns the clipboard, not whether the document is empty. The delay is not a guarantee that a paste succeeded. Monitoring remains active whether the window minimizes or stays visible.

## Format collected entries

The default separator is one line break using the document's current line ending. To add blank lines and local-time timestamps:

```ini
[Settings2]
PasteBoardSeparator=\r\n\r\n
PasteBoardAddTimestamp=1
PasteBoardDebounceMs=200
```

This example uses CRLF; use `\n\n` for an LF document. Timestamps have the form `[HH:MM:SS] `.

**An absent separator setting and an empty setting are different:** omit `PasteBoardSeparator` to use the document's line ending; use `PasteBoardSeparator=` to concatenate entries without an added separator. Custom separators include only the characters you specify: for a dashed line, use `\r\n---\r\n`. The separator is skipped on the first paste after activation and whenever the caret is already at a line start.

The debounce setting waits for a quiet interval after clipboard updates (default **200 ms**, range **0-5000 ms**). A larger value can coalesce more rapid copies; it does not preserve all intermediate clipboard contents.

See the [configuration reference](Configuration.md#clipboard-monitoring) for the full setting syntax.

## Optional disposable scratchpad

Normally, captured text marks the document as modified and closing asks whether to save it. To make the prompt suppressible for an **Untitled document while monitoring is active**:

```ini
[Settings2]
DiscardOnClosingUntitledPasteBoard=1
```

This setting **does not immediately discard anything or remove the first prompt**. Restart, launch with `/b`, and close a modified Untitled document. To opt out, check **Don't display this message again** and choose **No** (discard). If the active INI is writable, that answer is saved as `MsgDiscardUntitled` under `[Suppressed Messages]`; later matching prompts replay it silently. Choosing Yes remembers the save answer instead; Cancel is never saved.

**Warning:** after opting out with No, even manually edited text in an Untitled Pasteboard can be discarded without a save prompt. There is no special exemption for edits made after automatic pasting. Save important content explicitly.

The opt-out does not apply when monitoring is off or the document has an associated file path. To restore the prompt, remove `MsgDiscardUntitled` from `[Suppressed Messages]` while Notepad3 is closed, or set `DiscardOnClosingUntitledPasteBoard=0` and restart (this also clears the remembered answer).

See [DiscardOnClosingUntitledPasteBoard](Configuration.md#discardonclosinguntitledpasteboard0) for the setting reference.

## Interaction with Monitoring Log

Clipboard Monitoring and **Monitoring Log** (tailing a file) are mutually exclusive. Turn the active mode off before enabling the other. If `/b` is supplied while `MonitoringLog=true` is saved in the INI, clipboard monitoring wins for that launch without changing the saved log-monitoring preference.

## Source references

These descriptions were checked against source, not an interactive Windows GUI test; human review is pending.

- [`src/Notepad3.c:1019`](../../src/Notepad3.c#L1019): `PasteBoard_Start` and `PasteBoard_Stop` manage activation; `InitInstance` at line 2451 implements startup capture, and `WM_CLIPBOARDUPDATE` at line 2669 schedules subsequent pastes.
- [`src/Notepad3.c:13542`](../../src/Notepad3.c#L13542): `PasteBoardTimerProc` handles caret insertion, separators, timestamps, debouncing, and duplicate detection.
- [`src/Notepad3.c:2112`](../../src/Notepad3.c#L2112): `_DeferMinimizeTimerProc` checks Ctrl and clipboard pasteability.
- [`src/Notepad3.c:12735`](../../src/Notepad3.c#L12735): `FileSave` gates the optional `MsgDiscardUntitled` prompt on Untitled + active Pasteboard + the setting.
- [`src/Config/Config.cpp:1425`](../../src/Config/Config.cpp#L1425) and [line 1523](../../src/Config/Config.cpp#L1523): `LoadSettings` loads the five Pasteboard settings and clears the remembered discard answer when its gate is disabled.
- [`src/Dialogs.c:467`](../../src/Dialogs.c#L467) and [line 514](../../src/Dialogs.c#L514): `_InfoBoxLngDlgProc` and `InfoBoxLng` handle saved answers and the opt-out checkbox.

For other startup switches, see the [command-line reference](../cmdln/CmdLnOptions.md#pasteboard-clipboard-collector-b).
