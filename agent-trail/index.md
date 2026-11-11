# Notepad3 Agent Trail

This directory holds repository-specific guidance for contributors and coding agents. [`AGENTS.md`](../AGENTS.md) is the session entry point; use this index to find relevant details without scanning the repository.

Scoped signposts live in [`src\AGENTS.md`](../src/AGENTS.md) and [`src\Config\AGENTS.md`](../src/Config/AGENTS.md). They link to canonical topics rather than importing them all. This tree is agent reference knowledge, separate from human guides in `readme`.

## Layout and loading contract

The checker reads these declarations; change them with the approved layout, not a second folder list in the script.

- Topic folders: `architecture`, `build-and-test`, `domain`, `playbooks`.
- Instruction files: `AGENTS.md`, `src/AGENTS.md`, `src/Config/AGENTS.md`.
- Forbidden adapters: `CLAUDE.md`, `.claude/CLAUDE.md`, `CLAUDE.local.md`.
- Skill roots: `.claude/skills`, `.github/skills`, `.agents/skills`.

Forbidden adapters are checked in each declared instruction scope. The only IDE adapter is [Copilot instructions](../.github/copilot-instructions.md), a pointer to the canonical root rather than an import or copy. No project `CLAUDE.md` adapters are used.

Claude Code must support native `AGENTS.md` loading (v2.1.277 or later, with support enabled). Parent or personal project `CLAUDE.md`/`CLAUDE.local.md` files can suppress the default fallback; check user/managed settings rather than adding project adapters. See the [official loading contract](https://code.claude.com/docs/en/memory.md#agents-md).

After loading-related changes, verify in fresh sessions: Copilot `/instructions`, Claude `/context all` for startup files, then sequential reads in `src` and `src/Config` for on-demand scopes. Check transcripts for retained root rules and duplicate content. Verify `/docs-review` discovery separately; one wrapper lives under `.claude/skills`, with no duplicate wrapper for another client. VS Code skill discovery in that location remains unverified; use the playbook link if unavailable. File existence or this already-loaded session cannot prove runtime loading.

## Writing a page

- Follow the [root documentation policy](../AGENTS.md#documentation) for proposals, evidence, uncertainty, and validation.
- Keep one coherent topic per page, with actionable constraints before background and exact source/test paths or symbols.
- Link canonical guidance instead of copying it; add any new page to the routing table below.
- Put a plain `When to read: <trigger>` line on each page; repeat its trigger verbatim in that page's routing row.
- Mark schematic paths with `<placeholder>` segments or globs so the checker does not mistake examples for literal files.
- The docs checker warns above 8 KB. Split only when it improves retrieval; short pages do not need expansion.

### Verification metadata pilot

Only [PathLib](domain/pathlib.md) and [configuration](domain/configuration-and-ini.md) currently pilot metadata. Existing pages without it remain accepted. Headers use `status`, a full `verified-at` source commit, `owner`, and `anchors` entries with repository-relative `path` plus a literal `symbol`. A path-only anchor must instead include a `reason`. Keep the YAML subset simple: scalar fields and the anchor list, no multiline values or aliases.

Use `agent-observed` for code-backed agent findings; only @RaiKoHoff may approve `human-verified` status. Ownership routes review; it does not certify correctness. Each pilot states its evidence scope so the commit does not imply every sentence was verified. Uncertain observations stay in [Open questions](open-questions.md). Do not bulk-stamp old pages with the latest commit.

The checker rejects broken/escaping links, undeclared topic folders, mismatched routing triggers, forbidden adapters, broken skill targets, malformed declared metadata, missing/untracked anchor files, absent literal symbols, and invalid/non-ancestor verification commits. Recognizable repository paths in prose code spans must exist; missing paths with an unknown root warn instead. Source changes or dirty anchored files warn "possibly stale", not "wrong"; shallow missing history warns "verification unavailable". These are structural checks, not semantic or runtime-loading proof. Run checker regression cases with `pwsh -NoProfile -File Build\scripts\tests\Test-Docs.Tests.ps1` when changing the checker.

### Maintenance and pruning

- @RaiKoHoff is the human documentation approver. No CODEOWNERS-based review routing is configured by this policy.
- When a task changes documented behavior, review the affected guidance, not the entire tree. Correct/remove directly affected false claims; propose new knowledge only when useful.
- Quarterly, the approver reviews possibly stale evidence, unresolved observations, and redundant prose. This is a lightweight human review, not an automated rewrite or expansion job.
- Remove demonstrably wrong or duplicated entries. Do not delete a page solely because it is old, short, cold, or absent from a small task sample; rare high-risk invariants can still matter.
- Keep historical plans in place with status labels and off the default task route. Promote observations only with evidence; delete disproven ones. Archival/deletion beyond an approved change needs owner review.
- Benchmark agent runs and advanced code-intelligence/sanitizer setup are deferred and require separate scope/budget approval. No access telemetry or scheduled agent work is introduced.

## Where to look

| If you are changing... | Read |
|---|---|
| Bug fix, small feature, characterization test, focused check, or doc proposal | [Maintenance playbooks](playbooks/maintenance.md) (read only the relevant procedure) |
| Instructions, routing, adapters, skills, or the docs checker | [Review the agent docs](playbooks/docs-review.md) |
| Menus, commands, source layout, coding conventions, buffers | [Codebase and conventions](architecture/codebase-and-conventions.md) |
| Replacing home-grown `Edit.c` code with Scintilla messages | [Scintilla API usage](architecture/scintilla-api-usage.md) |
| Build, MSBuild, CI, test scripts, Python scripting | [Windows build and test](build-and-test/windows-build-and-test.md) |
| `Edit.c` load/save, atomic save, file watching, Open/MRU dialogs | [File I/O](domain/file-io.md) |
| Any Win32 file/path call on `HPATHL` | [PathLib](domain/pathlib.md) |
| Encoding selection or UTF-16/32 handling | [Encoding detection](domain/encoding-detection.md) |
| Encoding diagnostics, detector regression tests, or coverage boundaries | [Encoding diagnostics and tests](domain/encoding-diagnostics-and-tests.md) |
| Lexers, styles, themes | [Lexer development](domain/lexer-development.md) |
| UI strings, locale resources | [Localization](domain/localization.md) |
| Search/replace, PCRE2, URL hotspots | [Regular expressions](domain/regular-expressions.md) |
| INI settings, persistence | [Configuration and INI](domain/configuration-and-ini.md) |
| Dark mode, rendering, theme-change redraw | [Dark mode and rendering](domain/ui-rendering.md) |
| Startup, activation, AppID, new-window reuse | [Startup and window lifecycle](domain/startup-and-window-lifecycle.md) |
| Expression evaluation (`=?`, Go To, line numbering), `tinyexpr_cif` | [TinyExpr++ wrapper](domain/tinyexpr-wrapper.md) |
| Unverified findings / what would verify them | [Open questions](open-questions.md) |
