# Small maintenance tasks

When to read: Bug fix, small feature, characterization test, focused check, or doc proposal

Read only the procedure needed for the task. These are checklists, not permission to expand scope. Start at the repository root, consult the [topic index](../index.md), and verify relevant claims against code.

## Bug fix

1. Define the symptom and a bounded acceptance condition; find the closest implementation and test.
2. Reproduce safely. Add a failing regression case, or characterize existing behavior before changing it. Separate baseline failures from regressions.
3. Trace the root cause and shared callers; make the smallest fix, without unrelated cleanup.
4. Run the focused check below, then a linked build or regression subset only when affected boundaries require it.
5. Propose a doc update only for a durable missing invariant, reason, trap, or directly affected correction.

Stop and ask with a recommendation if intended behavior is ambiguous, a vendored patch needs alteration, reproduction is unsafe, or the fix requires broader refactoring.

Output: cause, focused change, exact evidence, remaining uncertainty, and the [root policy's doc-impact line](../../AGENTS.md#documentation). Ask before writing proposed docs. Do not equate stub-harness success with full application behavior.

## Small feature

1. Agree on behavior and exclusions; compare the nearest existing extension point.
2. Check only applicable touchpoints: settings persistence, UI resources, lexer registration, shared callers.
3. Add focused behavior cases, implement minimally, and use the relevant build/test procedure below.
4. For new UI text, follow [Localization](../domain/localization.md). For settings, follow [Configuration and INI](../domain/configuration-and-ini.md) and its user-reference touchpoints.
5. Propose docs only when the change adds or invalidates useful non-derivable knowledge.

Stop if this becomes an architecture change, dependency addition, ABI/format migration, or broad cleanup; recommend a separate task.

Output: implemented behavior, extension points, evidence, and explicit exclusions.

## Characterization tests

1. Choose the smallest stable behavior boundary before modifying legacy code.
2. Follow an existing harness instead of introducing a framework. The [encoding harness](../domain/encoding-diagnostics-and-tests.md#regression-coverage) demonstrates extraction with explicit stubs.
3. Capture current behavior and meaningful edge cases. Keep a suspected existing bug separate from the desired fix expectation.
4. Run against unchanged production source first; record stub/integration boundaries.
5. Change the expectation only as part of the requested behavior change.

Stop if isolation requires product refactoring or desktop interaction could disturb another user. Recommend a manual reproduction record or separately approved test seam.

Output: characterized boundary, cases, stubs, baseline outcome, and uncovered integration.

## Focused build and test

Discover prerequisites using [Windows build and test](../build-and-test/windows-build-and-test.md), then select the affected check:

```powershell
# Encoding detector (Visual C++ build tools required).
pwsh -NoProfile -File test\test_files\encoding\TestEncodingDetection.ps1
# Open-directory resolver (in-memory path/MRU stubs, not dialog UI).
pwsh -NoProfile -File test\TestOpenDialogDirectory.ps1
# Documentation links and declared evidence.
pwsh -NoProfile -File Build\scripts\Test-Docs.ps1
```

Run only the relevant commands, checking each exit code. For source/compiler feedback use the guide's `ClCompile` recipe; for resources/dependencies/integration use its linked-build procedure. Compile-only success is not a linked application build.

For checker changes also run:

```powershell
pwsh -NoProfile -File Build\scripts\tests\Test-Docs.Tests.ps1
```

Stop on missing prerequisites with the exact error and a recommendation. Do not clean existing outputs, restore dependencies, change versions, or install tools merely to make unrelated checks green. GUI tests require a safe desktop; date-sensitive version failures against old binaries are not automatically source regressions.

Output: commands, configuration, results, and boundaries; report unrun checks plainly.

## Propose a doc update

1. Ask whether the entry prevents a repeated mistake and cannot safely be derived from nearby code.
2. Correct/remove false guidance, or add one atomic invariant, rationale, or trap. Link the canonical page instead of copying it.
3. Include source path/symbol, inspected source commit, and verification method. On metadata-pilot pages, respect the stated evidence scope; do not silently certify unchecked claims.
4. Keep uncertainty in [Open questions](../open-questions.md) with a concrete verification step. Only @RaiKoHoff may approve `human-verified` status.
5. Ask for approval before writing the proposal; then present a reviewable diff within that scope, index any new page, and run the docs checker.

Stop if evidence is inference, old narrative, or an unrun command; recommend omission or an explicitly unverified observation.

Output:

```text
File:
Type: add | correct | remove
Entry:
Anchor: repository path + symbol (or explain a path-only anchor)
Verified-at: inspected source commit
Evidence: command/result or inspected code range
Status: agent-observed | unverified-agent-observation
Requested review:
```
