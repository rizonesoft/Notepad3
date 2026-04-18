# Focused View

**Focused View** is a filter-like display mode that shows only the lines containing occurrences of the currently marked word or selection. Non-matching lines are hidden using Scintilla's folding engine, letting you concentrate on exactly the code or text that matters.

| | |
|---|---|
| **Shortcut** | `Ctrl+Alt+V` |
| **Menu** | View → Display → Focused View |
| **Toolbar** | Focused View button (eye icon) |
| **INI setting** | `FocusViewMarkerMode` in `[Settings]` |

---

## How It Works

1. **Select or place the caret on a word** — Notepad3's *Mark Occurrences* feature automatically highlights every occurrence of that word throughout the document.
2. **Activate Focused View** (`Ctrl+Alt+V`) — all lines that do **not** contain a marked occurrence are folded away, leaving only the matching lines visible.
3. **Browse the filtered results** — scroll, copy, or inspect the visible lines.
4. **Deactivate** (`Ctrl+Alt+V` again) — all lines are restored and the editor returns to normal editing mode.

> **Tip:** Focused View also works from the **Find & Replace** dialog. Check **"Mark All Occurrences"** in the dialog, then click the **Toggle Visibility** button to enter Focused View for the current search term.

### Prerequisites

Focused View requires two conditions to be met:

| Condition | Menu path | Why |
|-----------|-----------|-----|
| **Mark Occurrences** must be enabled | View → Mark Occurrences → Active | The feature needs occurrence markers to know which lines to keep visible. |
| **"Match Visible Only"** must be disabled | View → Mark Occurrences → Match Visible Only *(unchecked)* | When matching is limited to visible lines only, the full-document markers required for folding are not available. |

If either condition is not met, the Focused View menu item and toolbar button are grayed out.

---

## View Modes

Focused View supports five display modes, selectable from **View → Display → View Mode**:

| Mode | Description | Lines hidden? | Visual indicator |
|------|-------------|:---:|---|
| **Fold** *(default)* | Non-matching lines are collapsed via code folding. | ✔ | Fold margin only |
| **Bookmark** | Matching lines are marked with a bookmark icon in the margin. | ✘ | Bookmark icon in margin |
| **Highlight** | Matching lines receive a colored background highlight. | ✘ | Line background color |
| **Bookmark & Fold** | Lines are folded *and* matching lines show bookmark icons. | ✔ | Bookmark icon + fold |
| **Highlight & Fold** | Lines are folded *and* matching lines show colored backgrounds. | ✔ | Background color + fold |

### Choosing a Mode

- **Fold** (or any combined mode with Fold) is useful when you want to see *only* the matching lines, hiding everything else. The editor becomes **read-only** while folding is active to prevent accidental edits to hidden content.
- **Bookmark** and **Highlight** (without Fold) keep all lines visible and simply add visual markers. The editor remains fully editable. These modes are useful for visually scanning for matches without losing document context.

The active mode is stored in the INI setting `FocusViewMarkerMode` and persists across sessions.

### Mode Values (INI Reference)

| Value | Mode |
|:---:|---|
| `4` | Fold *(default)* |
| `1` | Bookmark |
| `2` | Highlight |
| `5` | Bookmark & Fold |
| `6` | Highlight & Fold |

---

## Read-Only Behavior

When a Fold-based mode is active (Fold, Bookmark & Fold, or Highlight & Fold), the editor is set to **read-only** to protect hidden content from accidental modification.

- **Attempting to type** while Focused View is active automatically **deactivates** Focused View — all lines are restored and the editor returns to normal edit mode.
- Non-fold modes (Bookmark only, Highlight only) do **not** set the editor to read-only.

---

## Find & Replace Dialog Integration

The Find & Replace dialog (`Ctrl+H`) provides direct control over Focused View:

1. **Mark All Occurrences** checkbox — When checked, every match of the current search pattern is marked throughout the document (same markers used by Focused View).
2. **Toggle Visibility** button — Toggles Focused View on/off using the markers from step 1. This button is only enabled when "Mark All Occurrences" is checked.

This allows you to:
- Type a search pattern in Find & Replace
- Enable "Mark All Occurrences" to see all matches
- Click "Toggle Visibility" to hide non-matching lines
- Refine your search — the visible lines update as the pattern changes

When the Find & Replace dialog is closed, Focused View is automatically deactivated if it was started from the dialog.

---

## Interaction with Other Features

### Code Folding
While Focused View (fold mode) is active, normal code folding is temporarily overridden. Focused View disables the lexer's folding rules and takes full control of fold levels to group lines by match status. When Focused View is deactivated, the lexer's original folding behavior is restored.

The **Toggle Folds** toolbar button is disabled while Focused View is active.

### Mark Occurrences
Focused View depends on the Mark Occurrences feature. If you disable Mark Occurrences while Focused View is active, Focused View is automatically deactivated first.

The Mark Occurrences settings that affect Focused View behavior:
- **Match Case** — occurrence matching respects case
- **Whole Words / Current Word** — controls what counts as a match
- **Match Visible Only** — must be *off* for Focused View to be available

### File Operations
- **Loading a new file** or **reloading** automatically deactivates Focused View.
- **Switching lexer** (syntax highlighting) temporarily deactivates Focused View, applies the new lexer, then reactivates it.
- **Theme changes** deactivate Focused View during the transition and restore it afterward.

### Bookmarks
Focused View's bookmark markers (MARKER\_NP3\_1 through MARKER\_NP3\_8) are separate from the standard user bookmark (MARKER\_NP3\_BOOKMARK). Up to 8 simultaneous occurrence-bookmark sets are supported. If all 8 are in use, a warning is displayed.

---

## Activation Flow (Technical Summary)

```
User presses Ctrl+Alt+V (or clicks toolbar button)
  │
  ▼
IsFocusedViewAllowed()?  ──No──▶  Button/menu grayed out
  │
  Yes
  ▼
EditToggleView()
  │
  ├── Mode includes FVMM_FOLD?
  │     │
  │     ├── Activating ──▶ EditFoldMarkedLineRange(hide=true)
  │     │     ├── Override lexer folding (Lexer_SetFoldingFocusedView)
  │     │     ├── Set fold levels: matched lines visible, others hidden
  │     │     ├── FoldAll(FOLD) to collapse non-matching lines
  │     │     ├── Add secondary bookmarks/highlights if combined mode
  │     │     └── Set editor read-only
  │     │
  │     └── Deactivating ──▶ EditFoldMarkedLineRange(hide=false)
  │           ├── FoldAll(EXPAND) to show all lines
  │           ├── Restore lexer folding properties
  │           ├── Re-mark all occurrences
  │           └── Restore editor to read-write
  │
  └── Mode is Bookmark or Highlight only?
        └── EditBookMarkLineRange() — add visual markers, no folding
```

---

## Related Settings

| INI Key | Section | Default | Description |
|---------|---------|---------|-------------|
| `FocusViewMarkerMode` | `[Settings]` | `4` (Fold) | View mode — see [Mode Values](#mode-values-ini-reference) above |
| `MarkOccurrences` | `[Settings]` | `1` (on) | Enable/disable Mark Occurrences (required for Focused View) |
| `MarkOccurrencesMatchVisible` | `[Settings]` | `0` (off) | Match visible lines only — must be `0` for Focused View |
| `MarkOccurrencesMatchCase` | `[Settings]` | `0` (off) | Case-sensitive occurrence matching |
| `MarkOccurrencesMatchWholeWords` | `[Settings]` | `0` (off) | Match whole words only |

---

## Keyboard Shortcut Summary

| Shortcut | Action |
|----------|--------|
| `Ctrl+Alt+V` | Toggle Focused View on/off |

View mode is changed through the menu only: **View → Display → View Mode**.
