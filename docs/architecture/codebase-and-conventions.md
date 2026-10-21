# Codebase and Conventions

## Project

Notepad3 is a Windows-only Win32 C/C++ text editor built on Scintilla/Lexilla, distributed under BSD-3. It includes MiniPath (`Ctrl+M`) and integrates the pre-built portable grepWin tool (`Ctrl+Shift+F`). The solution is `Notepad3.sln`; builds use Visual Studio 2026 and toolset v145.

## Source map

| File | Purpose |
|---|---|
| `src\Notepad3.c/h` | `wWinMain`, `MainWndProc`, global state structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`), and `MsgCommand()` |
| `src\Notepad3Util.c/h` | Bitmap/toolbar helpers, word-wrap configuration, middle-click auto-scroll |
| `src\Edit.c/h` | Find/replace (PCRE2), encoding, clipboard, indent, sort, bookmarks, folding, autocomplete |
| `src\Styles.c/h` | Scintilla styling, lexer selection, themes, margins |
| `src\Dialogs.c/h` | Dialogs and DPI-aware UI |
| `src\Config\Config.cpp/h` | INI load/save and MRU |
| `src\Encoding.c/h` | Character encoding detection/conversion |
| `src\SciCall.h` | Type-safe wrappers for Scintilla direct calls |
| `src\DynStrg.c/h` | `HSTRINGW` dynamic wide-string handle |
| `src\PathLib.c/h` | `HPATHL` path handle and long-path-aware Win32 wrappers |
| `src\TypeDefs.h` | `DocPos`, `DocLn`, `cpi_enc_t`, OS targeting, compiler macros |
| `src\MuiLanguage.c/h` | MUI language DLL loading |
| `src\StyleLexers\styleLexXXX.c` | Per-language `EDITLEXER` definitions |

## Menu and command dispatch

- `MsgInitMenu()` (`WM_INITMENU`) controls menu enable/check state.
- `MsgCommand()` (`WM_COMMAND`) delegates to static sub-handlers that return `true` when they handle a command:

| Handler | Scope |
|---|---|
| `_HandleFileCommands` | `IDM_FILE_*` |
| `_HandleEncodingCommands` | `IDM_ENCODING_*`, `IDM_LINEENDINGS_*` |
| `_HandleEditBasicCommands` | `IDM_EDIT_UNDO`..`CMD_VK_INSERT` |
| `_HandleEditLineManipulation` | `IDM_EDIT_ENCLOSESELECTION`..`IDM_EDIT_INSERT_GUID` |
| `_HandleEditTextTransform` | `IDM_EDIT_LINECOMMENT`..`IDM_EDIT_HEX2CHAR` |
| `_HandleEditFind` | `IDM_EDIT_FINDMATCHINGBRACE`..`IDM_EDIT_GOTOLINE` |
| `_HandleViewAndSettingsCommands` | `IDM_VIEW_*`, `IDM_SET_*` |
| `_HandleHelpCommands` | `IDM_HELP_*` |
| `_HandleCmdCommands` | `CMD_*` |
| `_HandleToolbarCommands` | `IDT_*` through `s_ToolbarDispatch[]` |

## Libraries and external tools

- `scintilla\` (5.5.8) and `lexilla\` (5.4.6), with NP3 patches under `np3_patches\` and offline documentation under `doc\`.
- `scintilla\pcre2\` (10.47), `src\uchardet\`, `src\tinyexpr\`, `src\tinyexprcpp\`, `src\uthash\`, and `src\crypto\` (Rijndael/SHA-256 for AES-256).
- grepWin is external and is not built from source. Pre-built executables are in `grepWin\portables\`; language files are in `grepWin\translations\`.
  - Runtime lookup in `src\Dialogs.c`: `Settings2.GrepWinPath` → `<ModuleDir>\grepWin\grepWin-x{64,86}_portable.exe` → `%APPDATA%\Rizonesoft\Notepad3\grepWin\`.
  - `grepWinLangResName[]` in `MuiLanguage.c` maps Notepad3 locales to `.lang` filenames, written to `grepwin.ini` before launch.
  - `Build\make_portable_*.cmd` packages both portable executables, LICENSE, and all `.lang` files.
  - ARM64 uses the x64 executable via emulation (`#if defined(_M_ARM64)` in `Notepad3.c`).

## Coding conventions

- Follow `.clang-format` in `src\`: four-space indent, Stroustrup braces, left-aligned pointers, no column limit, no include sorting. `.editorconfig` requires UTF-8/CRLF; preserve tabs in upstream Lexilla code.
- Use `strsafe.h`; deprecated string functions are disabled.
- Use `DocPos` / `DocPosU` / `DocLn` for document positions/lines, `cpi_enc_t` for encodings, and `HSTRINGW` / `HPATHL` instead of raw wide-string buffers. `NOMINMAX` is global; use typed equivalents or `min()` / `max()`.
- Use existing global state structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`); do not add globals.
- Always use `SciCall.h` wrappers for Scintilla calls; add a missing wrapper there rather than using `SendMessage`. Naming: `DeclareSciCall{V|R}{0|01|1|2}` (`V`=void, `R`=return; `01`=lParam-only). The message argument is the suffix after `SCI_`.

```c
DeclareSciCallV0(Undo, UNDO);                                            // SciCall_Undo()
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);         // SciCall_SetTechnology(int)
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);                  // DocPos SciCall_GetTextLength()
DeclareSciCallR1(SupportsFeature, SUPPORTSFEATURE, bool, int, feature);  // bool SciCall_SupportsFeature(int)
```

- Group undoable edits with `_BEGIN_UNDO_ACTION_` / `_END_UNDO_ACTION_` (`Notepad3.h`); they also throttle notifications during bulk edits.
- `InfoBoxLng()` returns `MAKELONG(button, mode)`. When a suppression key is non-NULL, a saved-answer replay sets the high word; do not compare the raw result with `IDNO`. Use `IsYesOkay()`, `IsRetryContinue()`, `IsNoCancelClose()`, or extract `INFOBOX_ANSW(r)` / `INFOBOX_MODE(r)`.
- For new Scintilla/Lexilla APIs, consult the vendored versions in `scintilla\version.txt` and `lexilla\version.txt` and the offline docs under `scintilla\doc\` and `lexilla\doc\`.

### Find/Replace dialog text limits

- `CB_LIMITTEXT` (`FNDRPL_BUFFER`) does not apply to `WM_SETTEXT`; bound text before calling `ComboBox_SetText*`, or an oversized value freezes the single-line combo edit (see the `bFindReplCopySelOrClip` block in `EditFindReplaceDlgProc`, `src\Edit.c`).
- `ComboBox_SetText*` fires `CBN_EDITCHANGE` synchronously; reset one-shot flags before calling it.
- `_FindInTarget` has an 8192-byte `chFind` buffer; a longer pattern makes `WideCharToMultiByte` fail (`len` = -1) and Scintilla reports "not found".

### Dynamic string/path buffer lifetime

Never use a pointer returned by `Path_WriteAccessBuf()` / `StrgWriteAccessBuf()` after an operation that may reallocate or swap the same handle's buffer. Examples include `Path_CanonicalizeEx`, `Path_Swap` / `StrgSwap`, `Path_ExpandEnvStrings` / `ExpandEnvironmentStrgs`, `Path_Append` / `Path_Reset`, `StrgCat` / `StrgInsert` / `StrgFormat` / `StrgReset`, and `Path_NormalizeEx` / `Path_AbsoluteFromApp` / `Path_RelativeToApp`.

- For read-only access afterward, use `Path_Get(h)` / `StrgGet(h)`.
- For read-write access, reacquire with `Path_WriteAccessBuf(h, 0)` / `StrgWriteAccessBuf(h, 0)`; size 0 does not resize.
