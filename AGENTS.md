# Notepad3 Agent Instructions

Notepad3 is a Windows-only Win32 C/C++ editor built on Scintilla/Lexilla. Start with [`docs/README.md`](docs/README.md) for the topic index, then read the relevant pages before changing unfamiliar behavior.

## Working in this repository

- Before implementing a feature, inspect the closest existing implementation and its tests. Reuse the established pattern and implement at the common point where relevant call paths converge.
- Make the smallest change that fixes the root cause. Keep unrelated refactoring, formatting, and cleanup out of the diff.
- Add or update focused tests for changed behavior, then run the narrowest relevant test or build. Check existing test failures against current behavior before treating them as regressions.
- For C/C++ changes, read [Codebase and conventions](docs/architecture/codebase-and-conventions.md) for wrapper, buffer-lifetime, undo, and dialog-result constraints.
- Preserve user changes in the worktree. Do not revert unrelated edits.
- If intended behavior is unclear or the change requires unrelated refactoring, dependency changes, or altering a vendored patch, stop and ask with a recommended approach.
- Done means the requested behavior is addressed, focused checks pass, and the diff contains no unrelated changes. State baseline failures, unrun checks, and remaining uncertainty rather than claiming success.

## Focused checks

- Encoding regression: `pwsh -NoProfile -File test\test_files\encoding\TestEncodingDetection.ps1` (requires Visual C++ build tools).
- Docs: `pwsh -NoProfile -File Build\scripts\Test-Docs.ps1`.
- Build, other tests, and tool prerequisites: [Windows build and test](docs/build-and-test/windows-build-and-test.md). Compile-only feedback is not a linked application build.

## Documentation

- Docs may be stale. Verify relevant source paths and symbols before relying on a claim; when changing cited behavior, check the affected guidance too.
- Propose doc changes only for a durable invariant, rationale, or trap that prevents a repeated mistake, or to correct/remove inaccurate guidance. Include exact source/test anchors and verification evidence in the diff or PR description; a human reviews the proposal. Do not claim human verification without that review.
- Keep uncertain observations separate in [`docs/open-questions.md`](docs/open-questions.md), with what would verify them. Do not add speculative claims to topic pages.
- No doc update is required just because a session investigated code or a page is short. Avoid code paraphrases and duplicate guidance; link to the canonical page. Index any approved new topic page.
- After editing docs, run the docs check above.
