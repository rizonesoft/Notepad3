# Lexer Development

Each language has an `EDITLEXER` definition in `src\StyleLexers\styleLexXXX.c`. Register new lexers in the `Styles.c` lexer array and add localization string IDs.

## Implementation touchpoints

- **Scintilla properties:** if a lexer exposes properties (for example, `lexer.foo.allow.X`), add its `case SCLEX_FOO:` to the property dispatch in `EditLexer.c` and call `SciCall_SetProperty(...)`. Otherwise constructor defaults silently remain in effect.
- **Shared lexer IDs:** `SCLEX_CPP` is used for C, C#, Java, JS, and RC; `SCLEX_D` for D and Go; `SCLEX_PYTHON` for Python and Awk. Changes to a shared lexer or its property dispatch can affect every language wired to that ID.
- **Comment toggling:** lexers with comments need `SCLEX_FOO` cases in both `Lexer_GetStreamCommentStrgs` and `Lexer_GetLineCommentStrg` in `EditLexer.c`.
- **Theme sections:** INI sections use the `pszName` field (the fourth `EDITLEXER` field), not the lexer name string. Add sections for new lexers to applicable theme INIs: `Build\Notepad3.ini`, `Build\Themes\*.ini`, `res\StdDarkModeScheme.ini`, and locale variants `Build\Notepad3_<locale>.ini`. Renaming `pszName` can orphan existing user style customizations.
- **New style rows:** add a matching style entry to the applicable theme overrides. Only `res\StdDarkModeScheme.ini` and `Build\Themes\{Dark,Obsidian,Sombra}.ini` have per-style overrides; the main and locale INIs often have empty lexer sections where inline defaults apply. For foreground/background pairs, target WCAG contrast of at least 4.5:1. `lexilla\wscite\*.properties` can provide color references.
- **Style localization:** each new `EDITSTYLE` label needs an `IDS_LEX_STR_*` definition in `language\common_res.h` and an English placeholder in all 26 `language\np3_*\lexer_*.rc` files. Check ID gaps when choosing an ID. For bulk insertion, anchor on an existing identifier (not translated text), run once, and verify one occurrence per file.

## Lexilla sources

Homebrew lexer sources under `lexilla\lexers_x\` include AutoHotkey (`LexAHK` and `LexAHK2`), CSV, JSON5, Kotlin, and Verilog. Their `SCE_*_*` enums are in `lexilla\lexers_x\SciXLexer.h`, not `lexilla\include\SciLexer.h`. `LexHTML.cxx` is under `lexilla\lexers\` in this repository; `SCLEX_HTML` and `SCLEX_XML` use that source.

## Fixtures and verification

`test\test_files\StyleLexers\styleLex<NAME>\` contains lexer fixtures. Any tokenization change (style, state-machine branch, keyword list, escape handling, fold rule, etc.) must extend or add a fixture. Open fixtures in Notepad3 for visual verification; automated lexer assertions are not in scope. Folder names follow the EDITLEXER source file; the historical AHK folder is `styleLexAHKL\`.

Style-label resources are part of lexer work; general locale resource invariants and UI strings are documented in [Localization](localization.md).
