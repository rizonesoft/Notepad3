# Patch 003: LexMarkdown — AtTermStart accepts opening punctuation

**File:** `lexilla/lexers/LexMarkdown.cxx`
**Status:** NP3 local fix (candidate for upstream submission)
**First applied:** 2026-05-21

## Problem

The stock Lexilla Markdown lexer fails to recognise inline spans that
open immediately after common opening punctuation. The user-reported
case is inline code:

```
(`Hello`)        <- not highlighted
( `Hello`)       <- highlighted (extra space)
```

The same defect rejects:

```
[`x`]   {`x`}   <`x`>   "`x`"   '`x`'
(**x**) [**x**] (~~x~~) (*x*)   (_x_)
```

VS Code and other CommonMark-compliant renderers accept all of these.
Per CommonMark:

- **Code spans** have no left-flank restriction whatsoever.
- **Emphasis / strong** opening uses left-flanking-delimiter-run rules;
  a delimiter preceded by Unicode punctuation and followed by a
  non-whitespace non-punctuation char is a valid left flank.

## Root cause

`AtTermStart` (helper used by `IsCompleteStyleRegion` and the multi-
backtick code-span entry point) only returns `true` when `chPrev` is
whitespace or start-of-file:

```cpp
bool AtTermStart(const StyleContext &sc) noexcept {
    return sc.currentPos == 0 || sc.chPrev == 0 || isspacechar(sc.chPrev);
}
```

With `chPrev = '('`, the guard rejects the opening backtick and the
span never enters `SCE_MARKDOWN_CODE`/`SCE_MARKDOWN_CODE2`. Same for
`SCE_MARKDOWN_STRONG{1,2}`, `SCE_MARKDOWN_EM{1,2}`, and
`SCE_MARKDOWN_STRIKEOUT`.

## Fix

Extend `AtTermStart` to additionally accept `(`, `[`, `{`, `<`, `"`,
`'` as valid left-edge characters. This covers all bracketed and
quoted forms users typically write, without loosening behaviour for
mid-word delimiters (`foo*bar*` remains unchanged — `chPrev = 'o'` is
still rejected).

```cpp
bool AtTermStart(const StyleContext &sc) noexcept {
    if (sc.currentPos == 0 || sc.chPrev == 0 || isspacechar(sc.chPrev))
        return true;
    switch (sc.chPrev) {
    case '(': case '[': case '{': case '<':
    case '"': case '\'':
        return true;
    default:
        return false;
    }
}
```

## Scope

Affects all inline tokens gated by `IsCompleteStyleRegion`:

- `` ` ``  (single-backtick code)
- `` `` `` `` (multi-backtick code, via direct `AtTermStart(sc)` call)
- `**` / `__` (strong)
- `*` / `_` (emphasis)
- `~~` (strikeout)

Block-level constructs (headers, lists, code blocks, blockquote,
hrule, links) are untouched.

## Visual verification

Open `test/test_files/StyleLexers/styleLexMARKDOWN/README.md` in
Notepad3 after build. The section appended for this patch demonstrates
each adjacency variant — inline code/strong/em/strikeout following
every opener should colour correctly.

## Upstream

This is an upstream Lexilla defect. The fix is small and self-
contained; a PR against ScintillaOrg/lexilla would be welcome. Until
then, keep this patch.

## Upgrade procedure

After a Lexilla upgrade:

1. Diff `lexilla/lexers/LexMarkdown.cxx` against the upstream copy.
2. Reapply this patch (`003_LexMarkdown_AtTermStart.patch`) if the
   upstream still ships the whitespace-only `AtTermStart`.
3. If upstream has integrated the fix, retire this patch and remove
   its row from `README.md`.
