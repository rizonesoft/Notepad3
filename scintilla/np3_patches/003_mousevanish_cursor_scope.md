# Scope "Hide Pointer While Typing" to the Edit Client

## Description

Reworks the "hide mouse pointer while typing" behaviour (Windows setting
`SPI_GETMOUSEVANISH`) so it is scoped to the Scintilla **edit client** instead of
the whole application.

Previously the pointer was hidden with `::ShowCursor(FALSE)`. That API manipulates
a **per-input-queue (application-wide) counter**: because Notepad3's main window,
toolbar, statusbar, scrollbars, margins and the edit control all run on the same
thread/input-queue, typing blanked the pointer over the *entire* window. The
matching `::ShowCursor(TRUE)` only ran inside the edit control's mouse handlers, so
when the pointer re-entered over a non-text region (toolbar/margins/scrollbar) it
stayed invisible until it finally reached the text area — "the pointer is invisible
for a while after moving back in".

The patch switches to `::SetCursor(nullptr)`, which is per-window / per-message and
does **not** touch the shared show-count. The empty cursor is re-asserted in
`WM_SETCURSOR` while `cursorIsHidden` is set (so it stays hidden over the text area
during typing), and is cleared on genuine pointer movement, on `WM_MOUSELEAVE` and
on `WM_KILLFOCUS`. A hidden pointer is only revealed on a *real* `WM_MOUSEMOVE`
(position actually changed), preventing the flicker from synthetic moves that
Windows generates when a popup/statusbar repaints under a stationary pointer.

## Issues Fixed

- Pointer invisible over toolbar/margins/scrollbars "for a while" after typing with
  the mouse outside the window, then moving it back in.

## Related History

- [#4942](https://github.com/rizonesoft/Notepad3/issues/4942) - Typing causes cursor
  to flash, disappear and reappear (anti-flicker `ptMouseLast != pt` gate preserved).
- [#5369](https://github.com/rizonesoft/Notepad3/issues/5369) - Recover cursor state
  when the editor window loses focus (`WM_KILLFOCUS` recovery preserved).

## Files Modified

- `scintilla/win32/ScintillaWin.cxx`

## Changes

1. `HideCursorIfPreferred()` - hide via `::SetCursor(nullptr)` instead of
   `::ShowCursor(FALSE)` (client-scoped, not the process-wide counter).
2. `WM_SETCURSOR` (`HTCLIENT`) - while hidden, re-assert `::SetCursor(nullptr)`;
   otherwise show the context cursor as before.
3. `WM_MOUSEMOVE` - reveal only on real movement (`ptMouseLast != pt`) via
   `DisplayCursor(ContextCursor(pt))`; removed `::ShowCursor(TRUE)`.
4. `WM_MOUSELEAVE` / `WM_KILLFOCUS` - clear `cursorIsHidden` only (no counter to
   restore); removed `::ShowCursor(TRUE)`.
