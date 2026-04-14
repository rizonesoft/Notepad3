# Notepad3.c Refactoring — Extract Utilities & Split MsgCommand()

## Context

`src/Notepad3.c` was the largest module in the project at **12,985 lines**. It contained ~55 static functions and ~60 static variables spanning unrelated concerns (auto-scroll, file observation, bitmap loading, text input helpers, TinyExpr evaluation, etc.) plus a monolithic `MsgCommand()` dispatcher (2,994 lines, 360+ case statements). This refactoring improves navigability and separation of concerns without changing any behavior.

### Motivation

- **Navigability**: Finding code in a 13K-line file is slow; logically grouped helpers belong in their own modules
- **Maintainability**: The `MsgCommand()` monolith made it hard to reason about individual command groups
- **Encapsulation**: Static variables for unrelated subsystems (auto-scroll, file observation, TinyExpr) were all in global file scope, obscuring their true scope
- **Consistency**: Other modules (`Edit.c`, `Styles.c`, `Dialogs.c`) already follow clean `.c/.h` pair boundaries

### What NOT to extract

These items are intentionally kept in `Notepad3.c`:
- **`_InitGlobals()` / `_CleanUpResources()`** — core app lifecycle, touches everything
- **Message queue helpers** (`_MQ_AppendCmd`, `MQ_ExecuteNext`) — tightly integrated with timer system and UI updates
- **UI update helpers** (`_UpdateStatusbarDelayed`, `_UpdateToolbarDelayed`, `_UpdateTitlebarDelayed`) — depend on message queue + complex global state
- **`_EditSubclassProc()`** — Scintilla subclass glue, belongs near `MainWndProc`
- **`ParseCmdLnOption()`** — command-line parsing, belongs with app startup
- **`MsgInitMenu()`** — reads 20+ state variables to enable/disable menu items; inherently whole-application state

---

## Completed Work

### Part 2: MsgCommand() Split (DONE)

`MsgCommand()` refactored from a **2,994-line** switch into a **73-line thin dispatcher** that delegates to **10 static handler functions**, all remaining in `Notepad3.c`:

| Handler | Cases | Purpose |
|---------|-------|---------|
| `_HandleFileCommands` | ~27 | `IDM_FILE_*` — open/save/print/favorites/grepWin |
| `_HandleEncodingCommands` | ~10 | `IDM_ENCODING_*`, `IDM_LINEENDINGS_*` |
| `_HandleEditBasicCommands` | ~30 | `IDM_EDIT_UNDO`..`CMD_VK_INSERT` — undo/redo/cut/copy/paste/indent |
| `_HandleEditLineManipulation` | ~42 | `IDM_EDIT_ENCLOSESELECTION`..`IDM_EDIT_INSERT_GUID` — line modify/sort/join/case |
| `_HandleEditTextTransform` | ~45 | `IDM_EDIT_LINECOMMENT`..`IDM_EDIT_HEX2CHAR` — comments/encode/escape/hex |
| `_HandleEditFind` | ~21 | `IDM_EDIT_FINDMATCHINGBRACE`..`IDM_EDIT_GOTOLINE` — find/replace/bookmarks |
| `_HandleViewAndSettingsCommands` | ~99 | `IDM_VIEW_*`, `IDM_SET_*` — view/settings/rendering |
| `_HandleHelpCommands` | ~5 | `IDM_HELP_*`, `IDM_SETPASS` |
| `_HandleCmdCommands` | ~90 | `CMD_*` — keyboard shortcuts/navigation/window positioning |
| `_HandleToolbarCommands` | ~30 | `IDT_*` — toolbar dispatch via `s_ToolbarDispatch[]` lookup table |

**Dispatcher pattern:**
```c
LRESULT MsgCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    // Language/theme menu range checks (inline)
    // Timer/notification cases (inline, return immediately)
    switch(iLoWParam) { case SCEN_CHANGE: ... return FALSE; ... }

    // Handler dispatch chain
    if (_HandleFileCommands(hwnd, umsg, wParam, lParam)) { return FALSE; }
    if (_HandleEncodingCommands(...)) { return FALSE; }
    ...
    return DefWindowProc(hwnd, umsg, wParam, lParam);
}
```

Each handler returns `true` if handled, `false` to try the next. The 40 repetitive IDT_* toolbar cases were replaced with a `s_ToolbarDispatch[]` lookup table (29 standard entries + 2 special cases: `IDT_EDIT_COPY` falls back to COPYALL, `IDT_EDIT_CLEAR` falls back to `SciCall_ClearAll`).

### Part 1, Phases 1-3: Notepad3Util.c/.h (DONE)

New files created: `src/Notepad3Util.c` (349 lines), `src/Notepad3Util.h` (50 lines).

**Phase 1 — Bitmap/Image Loading** (LOW risk, ~100 LOC):
- `NP3Util_LoadBitmapFile()` — loads toolbar bitmap, validates dimensions
- `NP3Util_CreateScaledImageListFromBitmap()` — creates DPI-scaled image list
- `NP3Util_XXX_CreateScaledImageListFromBitmap()` — legacy variant using fixed `NUMTOOLBITMAPS`
- `NUMTOOLBITMAPS` macro moved from Notepad3.c to Notepad3Util.h

**Phase 2 — Word-Wrap Configuration** (LOW risk, ~100 LOC):
- `NP3Util_SetWrapStartIndent()` — sets wrap start indent based on `Settings.WordWrapIndent`
- `NP3Util_SetWrapIndentMode()` — sets wrap indent mode (same/indent/deep/fixed)
- `NP3Util_SetWrapVisualFlags(HWND)` — sets wrap visual flag symbols

**Phase 3 — Auto-Scroll** (LOW-MED risk, ~200 LOC):
- 6 static variables moved: `s_bAutoScrollMode`, `s_bAutoScrollHeld`, `s_dwAutoScrollStartTick`, `s_ptAutoScrollOrigin`, `s_ptAutoScrollMouse`, `s_dAutoScrollAccumY`
- 4 `AUTOSCROLL_*` constants moved to header
- `NP3Util_AutoScrollStart/Stop()`, `NP3Util_AutoScrollTimerProc()` — core scroll logic
- `NP3Util_IsAutoScrollMode()`, `NP3Util_IsAutoScrollHeld()`, `NP3Util_GetAutoScrollStartTick()`, `NP3Util_SetAutoScrollHeld()`, `NP3Util_AutoScrollUpdateMouse()` — state accessors for `_EditSubclassProc`

**Net result:** `Notepad3.c` reduced from 12,985 to 12,713 lines.

### Files modified
- `src/Notepad3.c` — extracted code removed, call sites updated, `#include "Notepad3Util.h"` added
- `src/Notepad3Util.c` — new implementation file
- `src/Notepad3Util.h` — new header file
- `src/Notepad3.vcxproj` — `<ClCompile>` and `<ClInclude>` entries added
- `src/Notepad3.vcxproj.filters` — filter entries added (Source Files / Header Files)
- `CLAUDE.md` — Core Modules table updated, Menu/Command Architecture section added
- `.github/copilot-instructions.md` — Core modules list updated

---

## Remaining Work — Phases 4-6

### Phase 4 — TinyExpr Evaluation (~130 LOC, MEDIUM risk)

**Static variables to move** (Notepad3.c line 186-187):
- `s_dExpression` (double) — last evaluated expression result
- `s_iExprError` (te_int_t) — last expression error code

**Functions to move:**
| Current | New | Line | LOC |
|---------|-----|------|-----|
| `_EvalTinyExpr(bool qmark)` | `NP3Util_EvalTinyExpr(bool)` | 2655 | ~150 |
| `_InterpMultiSelectionTinyExpr(te_int_t*)` | `NP3Util_InterpMultiSelectionTinyExpr(te_int_t*)` | 10086 | ~50 |

**New accessor functions needed:**
- `NP3Util_GetLastExpression()` — returns `s_dExpression` (read by `_UpdateStatusbarDelayed`)
- `NP3Util_GetLastExprError()` — returns `s_iExprError` (read by `_UpdateStatusbarDelayed`)

**Call sites to update (3):**
- Line 6839: `_EvalTinyExpr(false)` — in `_HandleCmdCommands`, case `CMD_ENTER_RETURN`
- Line 8921: `_EvalTinyExpr(true)` — in `_MsgNotifyFromEdit`, on `?` char insert
- Line 10399: `s_dExpression = _InterpMultiSelectionTinyExpr(&s_iExprError)` — in `_UpdateStatusbarDelayed`

**Dependencies:** `Settings.EvalTinyExprOnSelection`, `Encoding_SciCP`, `SciCall_*` (all via headers), `te_interp()` (needs `#include "tinyexpr/tinyexpr.h"` in Notepad3Util.c), `AllocMem`/`FreeMem` (via `Helpers.h`).

**Risk:** The statusbar code currently reads `s_dExpression`/`s_iExprError` directly — must switch to getters. The `te_interp()` call works on raw char buffers from Scintilla — encoding-sensitive but mechanically straightforward.

---

### Phase 5 — Text Input Helpers (~300 LOC, MEDIUM risk)

**Static variable to move** (line 189):
- `s_SelectionBuffer` (char*) — dynamically allocated buffer for auto-close bracket/quote tracking

**Functions to move:**
| Current | New | Line | LOC |
|---------|-----|------|-----|
| `_HandleAutoIndent(int)` | `NP3Util_HandleAutoIndent(int)` | 8291 | ~45 |
| `_HandleAutoCloseTags()` | `NP3Util_HandleAutoCloseTags()` | 8338 | ~58 |
| `_SaveSelectionToBuffer()` | `NP3Util_SaveSelectionToBuffer()` | 8398 | ~16 |
| `_EncloseSelectionBuffer(char,char)` | `NP3Util_EncloseSelectionBuffer(char,char)` | 8416 | ~17 |
| `_HandleInsertCheck(SCNotification*)` | `NP3Util_HandleInsertCheck(...)` | 8435 | ~89 |
| `_HandleDeleteCheck(SCNotification*)` | `NP3Util_HandleDeleteCheck(...)` | 8526 | ~60 |

Skip `_IsIMEOpenInNoNativeMode()` (line 8588) — dead code (`#if 0`).

**Lifecycle functions needed:**
- `NP3Util_TextInputInit()` — allocates `s_SelectionBuffer`; called from `MsgCreate`
- `NP3Util_TextInputCleanup()` — frees `s_SelectionBuffer`; called from `_CleanUpResources`

**Call sites to update (5, all in `_MsgNotifyFromEdit`):**
- Line 8635: `_HandleInsertCheck(scn)`
- Line 8642: `_SaveSelectionToBuffer()`
- Line 8658: `_HandleDeleteCheck(scn)`
- Line 8912: `_HandleAutoIndent(ich)`
- Line 8917: `_HandleAutoCloseTags()`

**Dependencies:** `Settings.AutoIndent`, `Settings.AutoCloseQuotes`, `Settings.AutoCloseBrackets`, `Settings.AutoCloseTags` (globals), `SciCall_*`/`Sci_*` (headers), `EditReplaceSelection()` (via `Edit.h`), `AllocMem`/`FreeMem`/`ReAllocMem`/`SizeOfMem` (via `Helpers.h`).

**Risk:** These functions run on the Scintilla notification hot path (`SCN_MODIFIED`, `SCN_CHARADDED`), but are called once per keystroke (not per character in bulk), so function-call overhead is negligible. Main risk is ensuring `s_SelectionBuffer` lifecycle remains correct.

---

### Phase 6 — File Observation (~450 LOC, HIGH risk — do last)

**Static variable to move** (line 483):
- `s_FileChgObsvrData` (`FCOBSRVDATA_T`) — contains event handles (`hEventFileChanged`, `hEventFileDeleted`), file metadata (`fdCurFile`), generation counter (`iObservationGeneration`, uses `InterlockedCompareExchange`/`InterlockedIncrement` seqlock pattern), background worker handle. **48 non-comment references** across Notepad3.c.

**Functions to move:**
| Current | New | Line | LOC |
|---------|-----|------|-----|
| `IsFileReadOnly()` | `NP3Util_IsFileReadOnly()` | 471 | ~15 |
| `IsFileChangedFlagSet()` | `NP3Util_IsFileChangedFlagSet()` | 487 | ~4 |
| `IsFileDeletedFlagSet()` | `NP3Util_IsFileDeletedFlagSet()` | 492 | ~4 |
| `RaiseFlagIfCurrentFileChanged()` | `NP3Util_RaiseFlagIfCurrentFileChanged()` | 497 | ~50 |
| `ResetFileObservationData(bool)` | `NP3Util_ResetFileObservationData(bool)` | 548 | ~20 |
| `IsFileVarLogFile()` | `NP3Util_IsFileVarLogFile()` | 10861 | ~10 |
| `_ResetFileWatchingMode()` | `NP3Util_ResetFileWatchingMode()` | 10871 | ~10 |
| `NotifyIfFileHasChanged()` | `NP3Util_NotifyIfFileHasChanged()` | 12364 | ~20 |
| `WatchTimerProc(...)` | `NP3Util_WatchTimerProc(...)` | 12385 | ~15 |
| `LogRotateTimerProc(...)` | `NP3Util_LogRotateTimerProc(...)` | 12402 | ~25 |
| `AtomicSaveTimerProc(...)` | `NP3Util_AtomicSaveTimerProc(...)` | 12426 | ~50 |

**Lifecycle functions needed:**
- `NP3Util_FileObservationInit()` — creates event handles; replaces code in `InitInstance()` (~lines 1843-1852)
- `NP3Util_FileObservationCleanup()` — destroys worker + event handles; replaces code in `_CleanUpResources()` (~lines 824-833)
- `NP3Util_GetFileObservationData()` — returns `PFCOBSRVDATA_T` pointer for `InstallFileWatching()` to access the struct

**Circular dependency:**
Timer callbacks call back into Notepad3.c:
- `AtomicSaveTimerProc` → `InstallFileWatching(false)`, `FileSave(FSF_SaveAlways)`
- `LogRotateTimerProc` → `PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL)`, `InstallFileWatching(true)`
- `_ResetFileWatchingMode` → `CheckCmd(GetMenu(...))`

**Resolution:** `Notepad3Util.c` already `#include`s `Notepad3.h` which declares these functions. The linker resolves cross-module calls — same pattern as `Edit.c` calling `FileLoad()`.

**Major call sites to update (~48 references):**
- `InitInstance()` — event creation → `NP3Util_FileObservationInit()`
- `_CleanUpResources()` — cleanup → `NP3Util_FileObservationCleanup()`
- `InstallFileWatching()` — direct struct field access → `NP3Util_GetFileObservationData()->`
- `MsgFileChangeNotify()` — reads flags, resets observation data
- `_UpdateTitlebarDelayed()` — calls `IsFileChangedFlagSet()`/`IsFileDeletedFlagSet()`
- `MsgInitMenu()` — calls `IsFileReadOnly()`
- `_HandleViewAndSettingsCommands`, `_HandleCmdCommands` — various flag checks

**Threading concern:** The generation counter uses `InterlockedCompareExchange`/`InterlockedIncrement` for a seqlock pattern (background worker vs. UI thread). Moving the struct doesn't change thread safety, but `NP3Util_GetFileObservationData()` returns a raw pointer — callers must not cache it across calls that could reallocate.

**Risk: HIGH** — 48 reference sites (most mechanical renames), but `InstallFileWatching()` directly manipulates struct fields (worker start/cancel, event wait). Timer proc function pointers in `SetTimer()` calls must be updated. Threading correctness is critical.

---

## Verification Strategy

After each phase:

1. **Build:** `Build\Build_x64.cmd Debug` (minimum) — no compile or link errors
2. **Diff audit:** `git diff` — confirm purely mechanical moves, no logic changes
3. **Smoke test per group:**
   - Phase 4 (TinyExpr): Select `1+2` → press `?` → verify result inserted; check statusbar expression display with column selection
   - Phase 5 (Text Input): Type `"` → verify auto-close quote; type `{` → verify auto-close bracket; press Enter after `if (...) {` → verify auto-indent; type `<div>` → verify auto-close `</div>`; press Backspace on `""` → verify pair deletion
   - Phase 6 (File Observation): Edit file in another editor → Notepad3 must prompt for reload; enable log tail mode (Ctrl+Shift+L) → verify auto-refresh; test atomic save (Settings2.AtomicFileSave=1); test file deletion detection; open/close files rapidly → verify no timer leaks
4. **Full build** after all phases: `Build\BuildAll.cmd Release` (all 4 platforms)

---

## Estimated Final Result

| Metric | Before | After (all phases) |
|--------|--------|---------------------|
| `Notepad3.c` lines | 12,985 | ~11,700 |
| `Notepad3Util.c` lines | 0 | ~1,250 |
| `MsgCommand()` lines | 2,994 | ~73 (dispatcher) |
| Toolbar switch cases | 40 repetitive | dispatch table |
| Static helpers in Notepad3.c | ~55 | ~35 |
