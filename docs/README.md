# Notepad3 Documentation

This directory holds repository-specific guidance for contributors and coding agents. [`AGENTS.md`](../AGENTS.md) is the session entry point; use this index to find relevant details without scanning the repository.

## Documentation rules

- **Optimize for retrieval.** Put one coherent topic in each file, use descriptive filenames and scannable headings, and place the task-specific guidance before background context.
- **Be concise and actionable.** Record verified behavior, constraints, decision points, failure modes, and required implementation/test touchpoints. Prefer a precise bullet, short procedure, or small lookup table over narrative.
- **Link to evidence.** Include exact repository paths and relevant identifiers so an agent can jump to implementation, tests, or user-facing configuration. Do not copy large code excerpts or repeat information already maintained elsewhere.
- **Maintain the source of truth.** In each session, capture verified, durable repo-specific insights found during implementation, debugging, or investigation. Create and index a focused page when a useful topic is missing; update or remove guidance when code changes make it inaccurate.
- **Keep pages lean.** Exclude transient notes, unverified hypotheses (see below), generic programming advice, and details an agent can safely derive from a nearby definition. Consolidate overlap and link to the canonical page.
- **Keep navigation complete.** Add every page to this index with a short description; use links between related pages instead of duplicating their content.
- **Respect the size guideline.** Split a page when it exceeds about 8 KB or covers two distinct tasks; `Build\scripts\Test-Docs.ps1` warns above 8 KB.
- **Park unverified findings.** Record tentative observations in [`open-questions.md`](open-questions.md) with how to verify them, not in topic pages. Promote or delete them once checked.
- **Validate after editing docs.** Run `pwsh Build\scripts\Test-Docs.ps1` (index completeness, relative links, anchors, page size).
- Keep `AGENTS.md` concise and limited to startup guidance and navigation.

## Where to look

| If you are changing... | Read |
|---|---|
| Menus, commands, source layout, coding conventions, buffers | [Codebase and conventions](architecture/codebase-and-conventions.md) |
| Build, MSBuild, CI, test scripts, Python scripting | [Windows build and test](build-and-test/windows-build-and-test.md) |
| `Edit.c` load/save, atomic save, file watching | [File I/O](domain/file-io.md) |
| Open dialog start directory, Recent files dialog | [File I/O](domain/file-io.md) |
| Any Win32 file/path call on `HPATHL` | [PathLib](domain/pathlib.md) |
| Encoding selection or UTF-16/32 handling | [Encoding detection](domain/encoding-detection.md), then [diagnostics and tests](domain/encoding-diagnostics-and-tests.md) |
| Lexers, styles, themes | [Lexer development](domain/lexer-development.md) |
| UI strings, locale resources | [Localization](domain/localization.md) |
| Search/replace, PCRE2, URL hotspots | [Regular expressions](domain/regular-expressions.md) |
| INI settings, persistence | [Configuration and INI](domain/configuration-and-ini.md) |
| Dark mode, rendering, theme-change redraw | [Dark mode and rendering](domain/ui-rendering.md) |
| Startup, activation, AppID, new-window reuse | [Startup and window lifecycle](domain/startup-and-window-lifecycle.md) |

## Contents

| Topic | Description | Pages |
|---|---|---|
| Architecture | Source map, command dispatch, vendored libraries, coding conventions, and implementation patterns. | [Codebase and conventions](architecture/codebase-and-conventions.md) |
| Build and test | Windows build targets, MSBuild invocation, CI and test gotchas, Python environment. | [Windows build and test](build-and-test/windows-build-and-test.md) |
| Domain: configuration | Portable INI lifecycle, persistence rules, and save/load behavior. | [Configuration and INI](domain/configuration-and-ini.md) |
| Domain: localization | Locale resource invariants, encoding/line-ending requirements, and adding UI strings. | [Localization](domain/localization.md) |
| Domain: lexer development | Lexer registration, property/comment/theme/localization touchpoints, and visual fixtures. | [Lexer development](domain/lexer-development.md) |
| Domain: encoding | Detection precedence, UTF validation and heuristics, UCHARDET sampling, and UTF-32 load conversion. | [Encoding detection](domain/encoding-detection.md) |
| Domain: encoding diagnostics | Developer title diagnostics, abbreviations, and the native regression harness. | [Encoding diagnostics and tests](domain/encoding-diagnostics-and-tests.md) |
| Domain: regular expressions | PCRE2 integration, search/replacement syntax, regex translations, and URL hotspots. | [Regular expressions](domain/regular-expressions.md) |
| Domain: startup and lifecycle | Pasteboard activation, taskbar Shift+Click handoff, AppID grouping, and new-window reuse behavior. | [Startup and window lifecycle](domain/startup-and-window-lifecycle.md) |
| Domain: file I/O | Load/save flow, atomic saves, error handling, file watching, Open-dialog initial directory, and Recent files dialog. | [File I/O](domain/file-io.md) |
| Domain: paths | PathLib wrappers, long-path handling, and path comparison rules. | [PathLib](domain/pathlib.md) |
| Domain: UI rendering | Dark mode, ARM64 rendering technology, and theme-change redraw. | [Dark mode and rendering](domain/ui-rendering.md) |
| Docs maintenance | Unverified findings and pages queued for coverage review. | [Open questions and coverage review](open-questions.md) |
