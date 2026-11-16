# Trail harvest

When to read: Harvest session insights into the agent trail

It covers the whole session, not only the last task. It reports first and writes only the proposals the user approves in their own message; an agent that picked it on its own asks before starting. It never stages or commits.

## Steps

1. Go back over the whole session: files read more than once, searches outside the repository, wrong turns and what corrected them, commands that failed before one worked, trail statements that proved wrong, notes or drafts written outside the trail.
2. Keep what an agent starting a similar task would have wanted at the start: an invariant, a trap, a command, a route to a file, a reason. Drop what belongs to this one case.
3. Split each kept item into its verified and unverified parts. Re-check a verified part against the code; propose an unverified part marked `Unverified:`, with what would verify it.
4. Check the trail first: a statement to correct beats a new one, and one the trail already holds is no proposal.
5. Report `Trail harvest: none — <why>`, or one line per proposal (`<file>: <statement> — evidence: <path:line or command>`), each in the file the [index](../index.md) routes that kind of statement to; unverified items go to [Open questions](../open-questions.md). Ask which to add.
6. Write only the approved proposals, following [Propose a doc update](maintenance.md#propose-a-doc-update), then run `pwsh -NoProfile -File Build\scripts\Test-Docs.ps1`.
