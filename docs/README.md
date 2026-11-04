# Notepad3 Documentation

This directory holds repository-specific guidance for contributors and coding agents. [`AGENTS.md`](../AGENTS.md) is the session entry point; use this index to find relevant details without scanning the repository.

## Writing a page

- Follow the [root documentation policy](../AGENTS.md#documentation) for proposals, evidence, uncertainty, and validation.
- Keep one coherent topic per page, with actionable constraints before background and exact source/test paths or symbols.
- Link canonical guidance instead of copying it; add any new page to the routing table below.
- The docs checker warns above 8 KB. Split only when it improves retrieval; short pages do not need expansion.

## Where to look

| If you are changing... | Read |
|---|---|
| Menus, commands, source layout, coding conventions, buffers | [Codebase and conventions](architecture/codebase-and-conventions.md) |
| Replacing home-grown `Edit.c` code with Scintilla messages | [Scintilla API usage](architecture/scintilla-api-usage.md) |
| Build, MSBuild, CI, test scripts, Python scripting | [Windows build and test](build-and-test/windows-build-and-test.md) |
| `Edit.c` load/save, atomic save, file watching, Open/MRU dialogs | [File I/O](domain/file-io.md) |
| Any Win32 file/path call on `HPATHL` | [PathLib](domain/pathlib.md) |
| Encoding selection or UTF-16/32 handling | [Encoding detection](domain/encoding-detection.md), then [diagnostics and tests](domain/encoding-diagnostics-and-tests.md) |
| Lexers, styles, themes | [Lexer development](domain/lexer-development.md) |
| UI strings, locale resources | [Localization](domain/localization.md) |
| Search/replace, PCRE2, URL hotspots | [Regular expressions](domain/regular-expressions.md) |
| INI settings, persistence | [Configuration and INI](domain/configuration-and-ini.md) |
| Dark mode, rendering, theme-change redraw | [Dark mode and rendering](domain/ui-rendering.md) |
| Startup, activation, AppID, new-window reuse | [Startup and window lifecycle](domain/startup-and-window-lifecycle.md) |
| Expression evaluation (`=?`, Go To, line numbering), `tinyexpr_cif` | [TinyExpr++ wrapper](domain/tinyexpr-wrapper.md) |
| Unverified findings / what would verify them | [Open questions](open-questions.md) |
