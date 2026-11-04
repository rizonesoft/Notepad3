# Notepad3 Agent Instructions

Notepad3 is a Windows-only Win32 C/C++ editor built on Scintilla/Lexilla. Start with [`docs/README.md`](docs/README.md) for the topic index, then read the relevant pages before changing unfamiliar behavior.

Quick routing (details in `docs\`): file load/save, open/MRU dialogs → `docs/domain/file-io.md`; Win32 path calls → `docs/domain/pathlib.md`; encodings → `docs/domain/encoding-detection.md`; lexers → `docs/domain/lexer-development.md`; UI strings → `docs/domain/localization.md`; build/CI/tests → `docs/build-and-test/windows-build-and-test.md`; everything else (regex, INI, dark mode, startup, conventions) → the routing table in `docs/README.md`.

## Working in this repository

- Before implementing a feature, inspect the closest existing implementation and its tests. Reuse the established pattern and implement at the common point where relevant call paths converge.
- Make the smallest change that fixes the root cause. Keep unrelated refactoring, formatting, and cleanup out of the diff.
- Add or update focused tests for changed behavior, then run the narrowest relevant test or build. Check existing test failures against current behavior before treating them as regressions.
- Follow the repository’s C/C++ formatting, types, and helper conventions. In particular, use `SciCall.h` wrappers for Scintilla calls, PathLib wrappers for file APIs on `HPATHL`, existing global state structs rather than new globals, and undo-action macros for grouped document edits.
- Do not retain pointers from `Path_WriteAccessBuf()` / `StrgWriteAccessBuf()` across operations that can reallocate or swap their buffers. Reacquire them after such operations.
- Handle `InfoBoxLng()` results with the provided predicates or extraction macros when suppression keys are involved; the raw return may include a mode in its high word.
- Preserve user changes in the worktree. Do not revert unrelated edits.

## Documentation

- Consult the index in [`docs/README.md`](docs/README.md) to find repository-specific guidance without scanning the repository.
- Docs may be stale. Verify relevant source paths and symbols before relying on a claim; when changing cited behavior, check the affected guidance too.
- Propose doc changes only for a durable invariant, rationale, or trap that prevents a repeated mistake, or to correct/remove inaccurate guidance. Include exact source/test anchors and verification evidence in the diff or PR description; a human reviews the proposal. Do not claim human verification without that review.
- Keep uncertain observations separate in [`docs/open-questions.md`](docs/open-questions.md), with what would verify them. Do not add speculative claims to topic pages.
- No doc update is required just because a session investigated code or a page is short. Avoid code paraphrases and duplicate guidance; link to the canonical page. Index any approved new topic page.
- After editing docs, run `pwsh Build\scripts\Test-Docs.ps1`.
- Keep this file as the agent’s operating guidance and navigation entry point; use `docs\` for detailed reference material.
