# ARM64 Flicker Analysis — Notepad3 Editor Pane

**Scope:** Slow, visible flicker in the Scintilla editor pane on Qualcomm ARM64 devices, strongly correlated with paste operations, aggravated by Windows 11 25H2, behaving differently across GDI vs. D2D rendering. A recent ARM64 mitigation reduced but did not eliminate the problem.

**Date:** 2026-04-24
**Repository state:** branch `dev_master`, tip `d6d5f41b5`

---

## 1. Problem statement

- Affects: Notepad3 on Windows-on-ARM (Snapdragon / Qualcomm Adreno GPUs).
- Trigger: most visible during copy/paste into the editor pane; also reported on theme / DPI / color-scheme transitions.
- OS sensitivity: worse on Windows 11 25H2 than on 23H2/24H2.
- Rendering-mode sensitivity: flicker pattern differs between GDI (`SC_TECHNOLOGY_DEFAULT`) and D2D variants.
- After the recent ARM64 fix: reduced but still present.

---

## 2. What the recent ARM64 fix actually does

Two application-level changes (no Scintilla patches were needed for the fix itself):

| Change | Location | Effect |
|---|---|---|
| Main window carries `WS_EX_COMPOSITED` on ARM64 only | `src/Notepad3.c:1914-1918` | DWM double-buffers the parent bottom-up. |
| Default `RenderingTechnology = SC_TECHNOLOGY_DIRECTWRITERETAIN` (2) on ARM64 | `src/Config/Config.cpp:1332-1338` | Scintilla uses a GDI-compatible retained `ID2D1DCRenderTarget` instead of a DXGI flip swap chain. |

Pre-existing and unchanged by that fix:

- The **status bar also has `WS_EX_COMPOSITED`** unconditionally (`src/Notepad3.c:3314-3323`). On ARM64 this means the paint path now has **nested** composited surfaces (main + status).
- `scintilla/np3_patches/upstream_558/001_undo_selection_redraw.patch` adds an extra `Redraw()` after undo-selection restore — relevant because paste is an undoable action.

---

## 3. Cause model — why paste specifically, why 25H2, why Adreno

A paste is not a single repaint; it is a **storm of paint triggers** arriving in the same few milliseconds, coinciding with a DWM state that Adreno/25H2 handles poorly:

1. `SciCall_Paste()` modifies many lines atomically → Scintilla issues at least one full-visible-range `Redraw()`.
2. `SCN_MODIFIED` fires → `EnableDocChangeNotification()` (`src/Notepad3.c:374-381`) calls `EditUpdateVisibleIndicators()` → additional margin / marker invalidates.
3. The same callback calls `UpdateStatusbar(false)`. This is deferred via a timer, but the deferred path at `src/Notepad3.c:10818-10833` does:

   ```c
   SendMessage(Globals.hwndStatus, WM_SETREDRAW, FALSE, 0);
   // ... SB_SETPARTS / SB_SETMINHEIGHT / StatusSetText ...
   SendMessage(Globals.hwndStatus, WM_SETREDRAW, TRUE, 0);
   InvalidateRect(Globals.hwndStatus, NULL, TRUE);   // note TRUE
   ```

4. `SCN_UPDATEUI` fires → caret / selection repaint + status update.
5. Auto-scroll into view if the caret moves below the visible region → another full pane invalidate.
6. If the paste lands inside an active "mark all occurrences" region, `MarkAllOccurrences` queues and fires shortly after.
7. On undo, the patched `Redraw()` from `upstream_558/001` fires again.

On Intel/AMD + recent Win11 builds, DWM collapses this into ~1 frame. On **Adreno + 25H2**, DWM appears to present the parent composited buffer and Scintilla's own D2D surface on *different ticks*, exposing the composited buffer (background colour) between Scintilla's `BeginDraw`/`EndDraw` pairs. `DIRECTWRITERETAIN` reduces this because the retained target keeps the previous frame visible across the gap — but if the parent composited buffer is invalidated by a secondary update (status bar) mid-burst, a flash is still visible.

**Why the GDI vs. D2D behaviour differs:**

| Mode | Mechanism | Interaction with `WS_EX_COMPOSITED` |
|---|---|---|
| `SC_TECHNOLOGY_DEFAULT` (0, GDI) | No swap chain; WM_PAINT-driven | Parent composition mostly wins; flicker is background-erase-driven. |
| `SC_TECHNOLOGY_DIRECTWRITE` (1) | DXGI swap chain on Scintilla's HWND | Scintilla presents independently of DWM compositor → heavy flicker on Adreno/25H2. |
| `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2) | GDI-compatible D2D; retained target | Current ARM64 default; compromise — presented via GDI on WM_PAINT. |
| `SC_TECHNOLOGY_DIRECTWRITEDC` (3) | `ID2D1DCRenderTarget` over a DC from WM_PAINT | Fully parented into GDI; `WS_EX_COMPOSITED` can buffer the result end-to-end. |

---

## 4. Remaining suspects, ranked by likelihood

### A. Status-bar invalidation with `bErase = TRUE` during paste bursts — **high suspicion**

`src/Notepad3.c:10833`:

```c
InvalidateRect(Globals.hwndStatus, NULL, TRUE);
```

The status bar is `SBT_OWNERDRAW | WS_EX_COMPOSITED`. Passing `TRUE` forces a `WM_ERASEBKGND` cycle on a window that owner-draws its own background — wasted work that also invalidates the parent composited buffer. Every paste triggers this path. Matches the paste-coupling pattern exactly.

### B. Nested `WS_EX_COMPOSITED` (main + status bar) — **medium-high suspicion**

Main window on ARM64 and the status bar (always) both carry `WS_EX_COMPOSITED`. DWM documentation warns against combining `WS_EX_COMPOSITED` with child windows that do their own buffered rendering; having **two layers** of compositor-buffering along the paint path is a known problematic combination on newer DWM builds. The status-bar flag predates the ARM64 fix and wasn't revisited when the parent flag was added.

### C. `SC_TECHNOLOGY_DIRECTWRITERETAIN` is a half-measure — **medium suspicion**

Mode 2 uses `ID2D1DCRenderTarget` + the retain-usage bit but still presents via GDI on WM_PAINT, with its own back buffer. Mode 3 (`DIRECTWRITEDC`) hands the DC from the HWND's WM_PAINT straight through, which is what `WS_EX_COMPOSITED` expects. On some 25H2 builds the retained target fails to be invalidated between paste-triggered paints, leaving a stale tile that the next paint flashes over. Would explain "helped a bit, still flickers".

### D. `MsgThemeChanged` uses `RDW_ERASE | RDW_ALLCHILDREN` — **low-medium suspicion**

`src/Notepad3.c:3478`:

```c
RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
```

Correct for a theme change, but `MsgDPIChanged` re-invokes `MsgThemeChanged`, and 25H2 has been observed to fire spurious DPI / colour-scheme change messages more eagerly (dark mode toggles, accent changes, HDR transitions). Note the contradiction with `src/DarkMode/ListViewUtil.hpp:64`, which deliberately omits `RDW_ERASE` for exactly this reason — the main-window path is the outlier. Relevant only if the user reports flicker on non-paste events too.

### E. `LimitNotifyEvents` / `RestoreNotifyEvents` is not used around paste — **medium suspicion**

`src/Notepad3.h:182` defines a macro that suppresses `SCN_*` notifications and toggles `WM_SETREDRAW` on the edit window. It is used for bulk edits (sort, encode, etc.) — but **paste uses `UndoTransActionBegin` only**, which groups undo but does nothing for notification throttling. Each inserted chunk can still trigger a full notify / repaint cycle. Especially relevant for paste of large clipboard contents.

### F. `SciCall_SetBufferedDraw` is never called — **low suspicion**

Declared at `src/SciCall.h:166-167` but never invoked. In GDI mode and `DIRECTWRITEDC` mode this flag enables Scintilla's own offscreen bitmap buffering; under D2D modes 1 and 2 Scintilla ignores it. Relevant only when experimenting with `DIRECTWRITEDC`.

---

## 5. Recommended experiments (ordered, cheapest first)

Each step isolates one hypothesis. Validate with the reporter after each.

1. **Flip the status-bar erase flag.** Change `InvalidateRect(Globals.hwndStatus, NULL, TRUE)` → `FALSE` at `src/Notepad3.c:10833`. Zero risk (owner-drawn control paints its own background). Targets suspect **A**.
2. **Remove `WS_EX_COMPOSITED` from the status bar on ARM64** (or everywhere). Targets suspect **B**. If flicker worsens on non-ARM64, revert; on ARM64 with the parent already composited, the status-bar flag is almost certainly redundant.
3. **Ask the reporter to set `RenderingTechnology=3` (`DIRECTWRITEDC`) in their INI.** Single-line INI change, no rebuild needed. If it fixes flicker, make it the ARM64 default (targets suspect **C**). If it worsens paste flicker, keep mode 2 and move on.
4. **Wrap the paste path in `LimitNotifyEvents` / `RestoreNotifyEvents`** (targets suspect **E**):

   ```c
   case IDM_EDIT_PASTE:
       if (SciCall_CanPaste()) {
           if (s_flagPasteBoard) { s_bLastCopyFromMe = true; }
           LimitNotifyEvents();
           UndoTransActionBegin();
           SciCall_Paste();
           EndUndoTransAction();
           RestoreNotifyEvents();
       }
       break;
   ```

   The macro already toggles `WM_SETREDRAW` on `Globals.hwndEdit` and invalidates once at the end. This collapses the paste burst into a single repaint — directly targeting the paste-specific complaint.

5. **Audit `_DelayUpdateStatusbar` throttle interval for ARM64.** If ≤16 ms, multiple fires occur inside a single frame; ~50 ms during paste bursts is safer.
6. **Long term:** introduce a `Settings2.ARM64FlickerWorkaround` umbrella flag bundling whichever of (1)–(4) prove necessary, so end-users on affected hardware can opt in without a rebuild. (Document per repo convention: `Build/Notepad3.ini` commented entry + prose in `readme/config/Configuration.md`.)

---

## 6. What not to do

- **Don't add `WM_ERASEBKGND` handlers.** The architecture deliberately relies on OS + Scintilla painting; intercepting becomes a maintenance rabbit hole.
- **Don't drop `WS_EX_COMPOSITED` from the main window on ARM64 without a replacement.** It is doing real work; the problem is the downstream invalidation traffic, not the flag itself.
- **Don't add `DwmFlush()` calls.** Tempting but wrong — serializes against DWM at significant cost and typically hides the symptom rather than fixing it.

---

## 7. Bottom line

The dominant remaining contributor is almost certainly a **paste-driven cascade of secondary invalidations** (status bar with `bErase=TRUE`, double-composited surfaces, un-throttled `SCN_*` notifications) interacting with an already-fragile DWM path on Adreno + 25H2. The recent ARM64 fix (`WS_EX_COMPOSITED` + `DIRECTWRITERETAIN`) addressed the *Scintilla* side; what remains is the **peripheral paint traffic around each paste** that the parent composited buffer has to keep up with.

Cheapest first move: change the `TRUE` erase flag at `src/Notepad3.c:10833` to `FALSE`, and wrap `IDM_EDIT_PASTE` in `LimitNotifyEvents` / `RestoreNotifyEvents`. Those two together are ~5 lines and target the two most load-bearing suspects.

---

## Appendix A — Key files and line references

| Concern | File | Line(s) |
|---|---|---|
| ARM64 `WS_EX_COMPOSITED` on main window | `src/Notepad3.c` | 1914-1918 |
| ARM64 default render technology | `src/Config/Config.cpp` | 1332-1338 |
| Render-tech applied to Scintilla | `src/Notepad3.c` | 2578-2579 |
| Render-tech runtime switch | `src/Notepad3.c` | 6701-6708 |
| Render-tech INI load | `src/Config/Config.cpp` | 1826 |
| `MsgThemeChanged` | `src/Notepad3.c` | 3432-3484 |
| Status-bar creation (WS_EX_COMPOSITED) | `src/Notepad3.c` | 3314-3323 |
| Status-bar update (erase TRUE) | `src/Notepad3.c` | 10818-10833 |
| `IDM_EDIT_PASTE` handler | `src/Notepad3.c` | 5222-5231 |
| `UndoTransActionBegin` / `EndUndoTransAction` | `src/Notepad3.h` | 188-189 |
| `LimitNotifyEvents` / `RestoreNotifyEvents` | `src/Notepad3.h` | 182 |
| `EnableDocChangeNotification` (post-modify callback) | `src/Notepad3.c` | 374-381 |
| DarkMode redraw without `RDW_ERASE` | `src/DarkMode/ListViewUtil.hpp` | 64, 126 |
| Undo-selection Redraw patch | `scintilla/np3_patches/upstream_558/001_undo_selection_redraw.patch` | — |

## Appendix B — Scintilla rendering technology constants

| Constant | Value | Underlying target | Swap model |
|---|---|---|---|
| `SC_TECHNOLOGY_DEFAULT` | 0 | GDI | `WM_PAINT` BitBlt |
| `SC_TECHNOLOGY_DIRECTWRITE` | 1 | DXGI flip swap chain | Independent present |
| `SC_TECHNOLOGY_DIRECTWRITERETAIN` | 2 | `ID2D1DCRenderTarget` + retain | Via GDI on `WM_PAINT` |
| `SC_TECHNOLOGY_DIRECTWRITEDC` | 3 | `ID2D1DCRenderTarget` on caller DC | Via `WM_PAINT` DC |

---

## Appendix C — `SciCall_SetBufferedDraw` deep-dive

Follow-up investigation: could suspect F (Scintilla's own buffered-draw flag) be a useful ARM64-only lever? Answer: **no, not a promising path**. Detailed reasoning below, grounded in the vendored Scintilla sources under `scintilla/`.

### C.1 What the flag does

**Default value:** `true`, set in `EditView::EditView()` at `scintilla/src/EditView.cxx:186`.

**Upstream comment** (`scintilla/src/EditView.h:60-62`):

> "In bufferedDraw mode, graphics operations are drawn to a pixmap and then copied to the screen. This avoids flashing but is about 30% slower."

**Mechanism (GDI path):** when on, Scintilla allocates two persistent pixmaps per view (`Editor.cxx:1861-1866`):

- `pixmapLine` — width of the client area, height of one line
- `pixmapSelMargin` — fixed-column width, full client height

In the paint loop, each line is drawn into `pixmapLine` and then `BitBlt`-ed onto the window DC (`EditView.cxx:2490-2614`, `SurfaceGDI.cxx:638-644`). When off, Scintilla draws directly to the window DC under an explicit `SetClip`/`PopClip` (`Editor.cxx:1910-1957`).

### C.2 The auto-override that kills this lever

Every `SCI_SETTECHNOLOGY` handler (`scintilla/win32/ScintillaWin.cxx:2280-2308`) forcibly assigns:

```cpp
view.bufferedDraw = technologyNew == Technology::Default;
```

Practical consequence:

| `SetTechnology` argument | `bufferedDraw` becomes |
|---|---|
| `SC_TECHNOLOGY_DEFAULT` (0, GDI) | `true` |
| `SC_TECHNOLOGY_DIRECTWRITE` (1) | `false` |
| `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2) — **ARM64 default** | `false` |
| `SC_TECHNOLOGY_DIRECTWRITEDC` (3) | `false` |

So any `SciCall_SetBufferedDraw(true)` we emit must come **after** `SciCall_SetTechnology`, and must be re-applied on every runtime tech switch (`Notepad3.c:6701-6708`), every `MsgThemeChanged`, and every `MsgDPIChanged` (the latter re-enters `MsgThemeChanged`).

### C.3 Benefits

- **GDI mode (0):** real benefit — per-line pixmap isolation reduces intra-line flicker. But already on by default, so Notepad3 does not need to touch it.
- **D2D modes (1, 2, 3, 4):** the flag is technically wired (`SurfaceD2D::Copy` uses `pRenderTarget->DrawBitmap`, `SurfaceD2D.cxx:989-1000`), so forcing it on would allocate per-line D2D bitmap render targets. Functionally works, but adds a stage to the pipeline rather than eliminating one.
- **None of these targets the actual ARM64 failure mode** (DWM compositor present misalignment between the parent composited buffer and Scintilla's D2D surface).

### C.4 Disadvantages

- **~30% slower paint** (upstream's own number).
- **Extra GDI handles / D2D bitmaps** — two persistent pixmaps held per view, reallocated on every resize, tech change, or `DropGraphics()`.
- **Silently clobbered** by every `SetTechnology` call — fragile to maintain.
- **Adds a third buffer stage on ARM64.** Main window already has `WS_EX_COMPOSITED` (parent composited buffer) and Scintilla has its retained D2D target. Inserting a per-line Scintilla pixmap makes three stages of buffering competing for GPU bandwidth — plausibly worse on Adreno, not better.
- **`WM_ERASEBKGND` is already suppressed** inside Scintilla's own wndproc (`ScintillaWin.cxx:2472-2473: return 1`), so the classical "avoid background flash" benefit is already paid for without this flag.

### C.5 Would ARM-gating `SciCall_SetBufferedDraw(true)` help?

Almost certainly not:

1. ARM64 default is `DIRECTWRITERETAIN`. Scintilla auto-disables `bufferedDraw` when that tech is set. Without re-applying after every `SetTechnology`, the call is a no-op.
2. The reported symptom is compositor-present misalignment, not per-line paint cadence. Buffered draw operates at the paint-cadence layer and cannot influence DWM composition ordering.
3. Adding a third buffer layer increases GPU work on a platform whose GPU/compositor pipeline is the root bottleneck.

### C.6 Verdict

Not pursued. The flag is a pre-DWM GDI-era flicker mitigation; it does not reach the layer where the ARM64 + 25H2 + Adreno symptom actually lives. Leverage points B and C remain the right next moves and are pursued in the code changes dated 2026-04-24.

### C.7 Key references

| Concern | File | Line(s) |
|---|---|---|
| `bufferedDraw` default = `true` | `scintilla/src/EditView.cxx` | 186 |
| Auto-override on `SCI_SETTECHNOLOGY` | `scintilla/win32/ScintillaWin.cxx` | 2303 |
| Pixmap allocation | `scintilla/src/Editor.cxx` | 1861-1866 |
| GDI `BitBlt` copy | `scintilla/win32/SurfaceGDI.cxx` | 638-644 |
| D2D `DrawBitmap` copy | `scintilla/win32/SurfaceD2D.cxx` | 989-1000 |
| Clip branches on flag | `scintilla/src/Editor.cxx` | 1910-1957 |
| Scintilla swallows `WM_ERASEBKGND` | `scintilla/win32/ScintillaWin.cxx` | 2472-2473 |

---

## Appendix D — Applied changes from leverage points B and E (2026-04-24)

After shipping leverage points A (status-bar erase flag) and the initial E wrap on `IDM_EDIT_PASTE`, the remaining paste sites and leverage point B were applied. Leverage point C was prepared but then reverted — see Appendix E.

**Leverage point B — un-nest `WS_EX_COMPOSITED` on ARM64** (`src/Notepad3.c:3314-3327`). The status bar drops its own `WS_EX_COMPOSITED` on ARM64 only; on that platform the main window already composites its children bottom-up, so the status bar's own flag only created a nested compositor stage. Non-ARM64 platforms retain the existing flag unchanged.

**Leverage point E extended — `LimitNotifyEvents` / `RestoreNotifyEvents` around all four `SciCall_Paste()` sites.** Same pattern as the earlier `IDM_EDIT_PASTE` change: collapse the `SCN_MODIFIED` / `SCN_UPDATEUI` burst into one end-of-paste invalidate. Scope matches the existing `UndoTransActionBegin` / `EndUndoTransAction` block exactly at each site; no wider refactoring.

| Site | File:Line | Trigger |
|---|---|---|
| `IDM_EDIT_PASTE` | `src/Notepad3.c:5233-5237` | Menu / Ctrl+V (shipped earlier) |
| `EditSwapClipboard()` | `src/Edit.c:936-954` | `IDM_EDIT_SWAP` |
| New-from-clipboard at startup | `src/Notepad3.c:2140-2148` | `/c` command-line flag |
| `PasteBoardTimerProc()` | `src/Notepad3.c:12464-12499` | `/B` pasteboard-monitor mode, each clipboard change |

All four sites retain their `UndoTransActionBegin`/`EndUndoTransAction` undo grouping unchanged; the new macros wrap around the existing undo wrap.

---

## Appendix E — Leverage point C not applied as a default change (2026-04-24)

**Decision:** the ARM64 default rendering technology stays at `SC_TECHNOLOGY_DIRECTWRITERETAIN` (2). A brief switch to `SC_TECHNOLOGY_DIRECTWRITEDC` (3) was reverted on the same day.

**Rationale:**

1. The original recommendation in section 5 was to test mode 3 **as an INI-side experiment first**, then only promote it to default if the reporter confirmed improvement. Flipping the default without that validation skipped the decision gate.
2. Project preference is to stay on D2D rendering paths. Although mode 3 still uses DirectWrite for glyph rendering, its presentation path (`ID2D1DCRenderTarget` blitting via GDI on `WM_PAINT`) is GDI-composed at the final stage. This is closer in character to the GDI mode (0) that is considered "not usable" than to modes 1 and 2, which use proper D2D render targets.
3. `DIRECTWRITERETAIN` (2) remains the best compromise among the D2D-only options: it keeps the retained back buffer that mitigates the Adreno + 25H2 compositor misalignment, without routing the final stage through GDI.

**What this means for further investigation:**

- Mode 3 is *not* promoted to default.
- It may still be tested ad-hoc by individual users via INI (`RenderingTechnology=3`) without a rebuild.
- Next ARM64 levers to try while staying on D2D: the three deferred `SciCall_Paste` wrapping sites (`EditSwapClipboard` at `src/Edit.c:943,946`, startup `/c` at `src/Notepad3.c:2144`, `PasteBoardTimerProc` at `src/Notepad3.c:12488`); and possibly raising the clamp at `Config.cpp:1329` to expose `SC_TECHNOLOGY_DIRECT_WRITE_1` (4) as an experimental option.
