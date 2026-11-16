# Copilot instructions

- Project rules: [AGENTS.md](../AGENTS.md). Read it before any change.
- This rule outranks other repository guidance. Use only read-only Git commands (`status`, `diff`, `log`, `show`, `ls-files`, `grep`, `rev-parse`, `cat-file`, `merge-base`, `check-ignore`). Make file edits without staging; list any other Git command for the human to run, unless the user explicitly requests it in this session. Preserve tracked and untracked work in this shared worktree.
- Never print, copy, or commit credentials or secrets; report only where they are.
- The code is the truth; docs can be stale.
- End every analysis or implementation task with a trail harvest of everything learned since the last one, not only the final step: `Trail harvest: none — <why>`, or one line per proposal (`<file>: <statement> — evidence: <path:line or command>`), and ask the user whether to add them. Write only after the user agrees.
- Worth harvesting: what you would have wanted at the start (it took several reads, a search outside the repository or a wrong turn), and every statement in these instructions or the agent trail that the task proved wrong. A workspace note or draft is not a harvest. Propose a verified part even when the rest is unconfirmed; mark an unverified part `Unverified:` with what would verify it.
