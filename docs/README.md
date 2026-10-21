# Notepad3 Documentation

This directory holds repository-specific guidance for contributors and coding agents. [`AGENTS.md`](../AGENTS.md) is the session entry point; use this index to find relevant details without scanning the repository.

## Documentation rules

- **Optimize for retrieval.** Put one coherent topic in each file, use descriptive filenames and scannable headings, and place the task-specific guidance before background context.
- **Be concise and actionable.** Record verified behavior, constraints, decision points, failure modes, and required implementation/test touchpoints. Prefer a precise bullet, short procedure, or small lookup table over narrative.
- **Link to evidence.** Include exact repository paths and relevant identifiers so an agent can jump to implementation, tests, or user-facing configuration. Do not copy large code excerpts or repeat information already maintained elsewhere.
- **Maintain the source of truth.** In each session, capture verified, durable repo-specific insights found during implementation, debugging, or investigation. Create and index a focused page when a useful topic is missing; update or remove guidance when code changes make it inaccurate.
- **Keep pages lean.** Exclude transient notes, unverified hypotheses, generic programming advice, and details an agent can safely derive from a nearby definition. Consolidate overlap and link to the canonical page.
- **Keep navigation complete.** Add every page to this index with a short description; use links between related pages instead of duplicating their content.
- Keep `AGENTS.md` concise and limited to startup guidance and navigation.

## Contents

| Topic | Description | Pages |
|---|---|---|
| Architecture | Source map, command dispatch, vendored libraries, coding conventions, and implementation patterns. | [Codebase and conventions](architecture/codebase-and-conventions.md) |
| Build and test | Windows build targets, MSBuild invocation, CI and test gotchas. | [Windows build and test](build-and-test/windows-build-and-test.md) |
| Domain: configuration | Portable INI lifecycle, persistence rules, and save/load behavior. | [Configuration and INI](domain/configuration-and-ini.md) |
| Domain: localization | Locale resource invariants, encoding/line-ending requirements, and adding UI strings. | [Localization](domain/localization.md) |
| Domain: lexer development | Lexer registration, property/comment/theme/localization touchpoints, and visual fixtures. | [Lexer development](domain/lexer-development.md) |
| Domain: text features | Navigation among encoding, lexer, localization, and regular-expression references. | [Text features](domain/text-features.md) |
| Domain: encoding | Detection precedence, UTF validation and heuristics, UCHARDET sampling, and UTF-32 load conversion. | [Encoding detection](domain/encoding-detection.md) |
| Domain: regular expressions | PCRE2 integration, search/replacement syntax, regex translations, and URL hotspots. | [Regular expressions](domain/regular-expressions.md) |
| Domain: startup and lifecycle | Pasteboard activation, taskbar Shift+Click handoff, AppID grouping, and new-window reuse behavior. | [Startup and window lifecycle](domain/startup-and-window-lifecycle.md) |
| Domain: file I/O | Load/save flow, atomic saves, error handling, and file watching. | [File I/O](domain/file-io.md) |
| Domain: platform behavior | PathLib and long paths, dark mode, rendering, and scripting environment. | [Platform and I/O](domain/platform-and-io.md) |
