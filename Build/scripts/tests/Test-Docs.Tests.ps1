$ErrorActionPreference = 'Stop'
$checker = (Resolve-Path (Join-Path $PSScriptRoot '..\Test-Docs.ps1')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-docs-tests-' + [guid]::NewGuid())
$passed = 0
function Write-Fixture([string]$path, [string]$text) {
    $dest = Join-Path $temp $path
    [void][IO.Directory]::CreateDirectory((Split-Path $dest))
    [IO.File]::WriteAllText($dest, $text)
}
function Git-Fixture {
    & git -C $temp @args 1>$null
    if ($LASTEXITCODE -ne 0) { throw "Fixture git failed: $args" }
}
function Check([string]$name, [string]$page, [int]$exitCode, [string]$message) {
    Write-Fixture 'agent-trail\topic.md' $page
    $output = & pwsh -NoProfile -File $checker -DocsDir (Join-Path $temp 'agent-trail') 2>&1
    if ($LASTEXITCODE -ne $exitCode -or ($message -and ($output -join "`n") -notmatch [regex]::Escape($message))) {
        throw "${name}: expected exit $exitCode / '$message'; got $LASTEXITCODE`n$($output -join "`n")"
    }
    $script:passed++
}
try {
    [void][IO.Directory]::CreateDirectory($temp)
    Git-Fixture init --quiet
    Git-Fixture config user.name 'Docs checker fixture'
    Git-Fixture config user.email 'fixture@example.invalid'
    $index = @'
- Topic folders: `architecture`, `playbooks`.
- Instruction files: `AGENTS.md`, `src/AGENTS.md`.
- Forbidden adapters: `CLAUDE.md`, `.claude/CLAUDE.md`, `CLAUDE.local.md`.
- Skill roots: `.claude/skills`, `.github/skills`, `.agents/skills`.

| If you are changing... | Read |
|---|---|
| Topic | [Topic](topic.md) |
'@
    $legacy = "# Legacy page`n`nWhen to read: Topic"
    Write-Fixture 'agent-trail\index.md' $index
    Write-Fixture 'agent-trail\topic.md' $legacy
    Write-Fixture 'AGENTS.md' '[Index](agent-trail/index.md)'
    Write-Fixture 'src\sample.c' 'void Example(void) {}'
    Write-Fixture 'src\AGENTS.md' '[Topic](../agent-trail/topic.md)'
    Git-Fixture add -- .
    Git-Fixture -c core.hooksPath=NUL commit --quiet -m 'Initial fixture' -m 'Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>'
    $commit = & git -C $temp rev-parse HEAD
    $page = @"
---
status: agent-observed
verified-at: $commit
owner: "@RaiKoHoff"
anchors:
  - path: src/sample.c
    symbol: Example
---
# Topic

When to read: Topic
"@
    Check 'legacy metadata optional' $legacy 0 'docs OK'
    Check 'valid metadata and nested scope' $page 0 'docs OK'
    Check 'missing symbol' ($page.Replace('symbol: Example', 'symbol: Missing')) 1 'symbol not found'
    Check 'symbol token boundary' ($page.Replace('symbol: Example', 'symbol: Exam')) 1 'symbol not found'
    Check 'untracked anchor' ($page.Replace('src/sample.c', 'src/local.c')) 1 'anchor is not tracked'
    Write-Fixture 'src\local.c' 'void Example(void) {}'
    Check 'existing untracked anchor' ($page.Replace('src/sample.c', 'src/local.c')) 1 'anchor is not tracked'
    Check 'path escape' ($page.Replace('src/sample.c', '../outside.c')) 1 'anchor must stay inside'
    Check 'invalid commit' ($page.Replace($commit, 'HEAD')) 1 'verified-at must be a full source commit'
    Check 'unresolved commit' ($page.Replace($commit, ('0' * 40))) 1 'unresolved verification commit'
    Check 'missing path-only reason' ($page.Replace('    symbol: Example', '')) 1 'path-only anchor needs a reason'
    Check 'path-only reason accepted' ($page.Replace('symbol: Example', 'reason: source layout')) 0 'docs OK'
    Check 'malformed metadata' ($page.Replace('status: agent-observed', 'status: guessed')) 1 'invalid or missing status'
    Check 'unterminated metadata' ($page -replace '---\r?\n# Topic', '# Topic') 1 'unterminated metadata'
    Remove-Item -LiteralPath (Join-Path $temp 'src\sample.c')
    Check 'missing tracked file' $page 1 'anchor file not found'
    Write-Fixture 'src\sample.c' "void Example(void) { /* changed */ }"
    Check 'dirty source warning' $page 0 'possibly stale (dirty source)'
    Git-Fixture add -- src/sample.c
    Git-Fixture -c core.hooksPath=NUL commit --quiet -m 'Change source fixture' -m 'Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>'
    Check 'changed source warning' $page 0 'possibly stale (source changed)'
    $tree = & git -C $temp rev-parse 'HEAD^{tree}'
    $unrelated = & git -C $temp -c core.hooksPath=NUL commit-tree $tree -m 'Unrelated fixture root' -m 'Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>'
    if ($LASTEXITCODE -ne 0) { throw 'Cannot create non-ancestor fixture' }
    Check 'non-ancestor commit' ($page.Replace($commit, $unrelated)) 1 'not an ancestor'
    Write-Fixture 'src\AGENTS.md' '@missing.md'
    Check 'nested import missing' $page 1 'unresolved or external instruction import'
    Write-Fixture 'src\AGENTS.md' '[Missing](../agent-trail/missing.md)'
    Check 'nested docs link missing' $page 1 'file not found'
    Write-Fixture 'src\AGENTS.md' '[Topic](../agent-trail/topic.md)'
    Check 'trigger missing' '# Topic' 1 'needs exactly one When to read line'
    Check 'trigger mismatch' ($legacy.Replace('Topic', 'Other')) 1 'does not match its index row'
    Check 'duplicate trigger' ($legacy + "`nWhen to read: Topic") 1 'needs exactly one When to read line'
    Write-Fixture 'agent-trail\index.md' ($index.Replace('Topic folders:', 'Folders:'))
    Check 'layout declaration missing' $legacy 1 'must declare Topic folders exactly once'
    Write-Fixture 'agent-trail\index.md' ($index.Replace('`architecture`', '`../outside`'))
    Check 'layout path escape' $legacy 1 'invalid layout path'
    Write-Fixture 'agent-trail\index.md' ($index + "`n| Topic | [Nested](other/topic.md) |")
    Write-Fixture 'agent-trail\other\topic.md' $legacy
    Check 'undeclared folder' $legacy 1 'undeclared topic folder'
    Remove-Item -LiteralPath (Join-Path $temp 'agent-trail\other\topic.md')
    Write-Fixture 'agent-trail\index.md' $index
    Check 'repository link escape' ($legacy + "`n[Outside](../../)") 1 'link must stay inside repository'
    Check 'rooted code path missing' ($legacy + "`n" + '`src/missing.c`') 1 'missing repository path'
    Check 'relative code path exists' ($legacy + "`n" + '`../src/sample.c`') 0 'docs OK'
    Check 'Windows code path exists' ($legacy + "`n" + '`src\sample.c`') 0 'docs OK'
    Check 'absolute external code path warns' ($legacy + "`n``${temp}-outside\missing.c``") 0 'unverified path outside known repository roots'
    Check 'link anchor missing' ($legacy + "`n[Missing](#missing)") 1 'anchor not found'
    Check 'link anchor exists' ($legacy + "`n[Heading](#legacy-page)") 0 'docs OK'
    Check 'unknown root warning' ($legacy + "`n" + '`other-repository/missing.c`') 0 'unverified path outside known repository roots'
    Check 'schematic paths skipped' ($legacy + "`n" + '`src/styleLex<name>.c` `src/*.c`') 0 'docs OK'
    Check 'fenced examples ignored' ($legacy + "`n``````text`n[Missing](missing.md) ``src/missing.c```n``````") 0 'docs OK'
    foreach ($adapter in 'CLAUDE.md', 'src\CLAUDE.md', 'src\.claude\CLAUDE.md', 'src\CLAUDE.local.md') {
        Write-Fixture $adapter '@AGENTS.md'
        Check "forbidden adapter $adapter" $legacy 1 'forbidden adapter'
        Remove-Item -LiteralPath (Join-Path $temp $adapter)
    }
    Write-Fixture '.github\copilot-instructions.md' '[Wrong](../src/AGENTS.md)'
    Git-Fixture add -- .github/copilot-instructions.md
    Check 'adapter target' $legacy 1 'must link the canonical root AGENTS.md'
    Write-Fixture '.github\copilot-instructions.md' '[Rules](../AGENTS.md)'
    Check 'adapter pointer accepted' $legacy 0 'docs OK'
    Write-Fixture 'agent-trail\playbooks\review.md' $legacy
    Write-Fixture 'agent-trail\index.md' ($index + "`n| Topic | [Review](playbooks/review.md) |")
    $skillPath = '.claude\skills\trail-review\SKILL.md'
    Write-Fixture $skillPath '[Review](../../../agent-trail/playbooks/review.md)'
    Check 'untracked wrapper discovered' $legacy 0 'docs OK'
    Write-Fixture $skillPath '[Wrong](../../../AGENTS.md)'
    Check 'wrapper needs playbook' $legacy 1 'must link a canonical playbook'
    Write-Fixture $skillPath '[Missing](../../../agent-trail/playbooks/missing.md)'
    Check 'wrapper target missing' $legacy 1 'file not found'
    Write-Fixture $skillPath '[External](https://example.invalid/review.md)'
    Check 'external link not a playbook' $legacy 1 'must link a canonical playbook'
    Remove-Item -LiteralPath (Join-Path $temp $skillPath)
    Remove-Item -LiteralPath (Join-Path $temp 'agent-trail\playbooks\review.md')
    Write-Fixture 'agent-trail\index.md' $index
    Git-Fixture clone --quiet --depth=1 --no-local $temp (Join-Path $temp 'shallow')
    # The committed legacy page is valid; install metadata pointing to excluded history.
    [IO.File]::WriteAllText((Join-Path $temp 'shallow\agent-trail\topic.md'), $page)
    $output = & pwsh -NoProfile -File $checker -DocsDir (Join-Path $temp 'shallow\agent-trail') 2>&1
    if ($LASTEXITCODE -ne 0 -or ($output -join "`n") -notmatch 'verification unavailable \(shallow history\)') {
        throw "Shallow history case failed: $($output -join "`n")"
    }
    $passed++
    Write-Fixture 'unexpected\AGENTS.md' '# Unlisted scope'
    Check 'unlisted instruction scope' $legacy 1 'instruction file is not declared: unexpected/AGENTS.md'
    Write-Host "All $passed docs checker cases passed."
} finally {
    if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force }
}
