# Notepad3 Documentation

This directory holds repository-specific guidance for contributors and coding agents. [`AGENTS.md`](../AGENTS.md) is the session entry point; use this index to find relevant details without scanning the repository.

Scoped signposts live in [`src\AGENTS.md`](../src/AGENTS.md) and [`src\Config\AGENTS.md`](../src/Config/AGENTS.md), with one-line CLAUDE import adapters. They link to these canonical topics rather than importing them all. For a tool-version/settings check, confirm the applicable files in Claude `/context all` or Copilot `/instructions`; filesystem access alone does not establish that instructions loaded.

## Writing a page

- Follow the [root documentation policy](../AGENTS.md#documentation) for proposals, evidence, uncertainty, and validation.
- Keep one coherent topic per page, with actionable constraints before background and exact source/test paths or symbols.
- Link canonical guidance instead of copying it; add any new page to the routing table below.
- The docs checker warns above 8 KB. Split only when it improves retrieval; short pages do not need expansion.

### Verification metadata pilot

Only [PathLib](domain/pathlib.md) and [configuration](domain/configuration-and-ini.md) currently pilot metadata. Existing pages without it remain accepted. Headers use `status`, a full `verified-at` source commit, `owner`, and `anchors` entries with repository-relative `path` plus a literal `symbol`. A path-only anchor must instead include a `reason`. Keep the YAML subset simple: scalar fields and the anchor list, no multiline values or aliases.

Use `agent-observed` for code-backed agent findings; only @RaiKoHoff may approve `human-verified` status. Ownership routes review; it does not certify correctness. Each pilot states its evidence scope so the commit does not imply every sentence was verified. Uncertain observations stay in [Open questions](open-questions.md). Do not bulk-stamp old pages with the latest commit.

The checker rejects malformed declared metadata, missing/untracked anchor files, absent literal symbols, and invalid/non-ancestor verification commits. Source changes or dirty anchored files warn "possibly stale", not "wrong"; shallow missing history warns "verification unavailable". Symbol matching is a case-sensitive token search, not semantic proof. Run checker regression cases with `pwsh -NoProfile -File Build\scripts\tests\Test-Docs.Tests.ps1` when changing the checker.

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
