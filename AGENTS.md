# Notepad3 Agent Instructions

Notepad3 is a Windows-only Win32 C/C++ editor built on Scintilla/Lexilla. Start with [`docs/README.md`](docs/README.md) for the topic index, then read the relevant pages before changing unfamiliar behavior.

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
- During each session, capture verified, durable Notepad3 insights found during implementation, debugging, or repository investigation in the relevant `docs\` page. If an investigation uncovers a useful topic not covered by the index, create a focused page in the appropriate subdirectory and add it to the index.
- Update or remove guidance when code changes make it inaccurate. Do not add transient observations, unverified hypotheses, or duplicate material; link to the canonical page instead.
- Write docs for fast agent retrieval: concise actionable guidance, scannable headings, and exact source/test paths or identifiers; prioritize task-relevant constraints and decision points over background narrative.
- Keep this file as the agent’s operating guidance and navigation entry point; use `docs\` for detailed reference material.
