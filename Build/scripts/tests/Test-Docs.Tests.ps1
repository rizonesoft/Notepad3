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
    Write-Fixture 'docs\topic.md' $page
    $output = & pwsh -NoProfile -File $checker -DocsDir (Join-Path $temp 'docs') 2>&1
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
    Write-Fixture 'docs\README.md' '[Topic](topic.md)'
    Write-Fixture 'docs\topic.md' '# Legacy page'
    Write-Fixture 'src\sample.c' 'void Example(void) {}'
    Write-Fixture 'src\CLAUDE.md' '@AGENTS.md'
    Write-Fixture 'src\AGENTS.md' '[Topic](../docs/topic.md)'
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
"@
    Check 'legacy accepted' '# Legacy page' 0 'docs OK'
    Check 'valid metadata and nested adapter' $page 0 'docs OK'
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
    Write-Fixture 'src\CLAUDE.md' '@missing.md'
    Check 'nested import missing' $page 1 'unresolved or external instruction import'
    Write-Fixture 'src\CLAUDE.md' '@AGENTS.md'
    Write-Fixture 'src\AGENTS.md' '[Missing](../docs/missing.md)'
    Check 'nested docs link missing' $page 1 'file not found'
    Write-Fixture 'src\AGENTS.md' '[Topic](../docs/topic.md)'
    Git-Fixture clone --quiet --depth=1 --no-local $temp (Join-Path $temp 'shallow')
    # The committed legacy page is valid; install metadata pointing to excluded history.
    [IO.File]::WriteAllText((Join-Path $temp 'shallow\docs\topic.md'), $page)
    $output = & pwsh -NoProfile -File $checker -DocsDir (Join-Path $temp 'shallow\docs') 2>&1
    if ($LASTEXITCODE -ne 0 -or ($output -join "`n") -notmatch 'verification unavailable \(shallow history\)') {
        throw "Shallow history case failed: $($output -join "`n")"
    }
    $passed++
    Write-Host "All $passed docs checker cases passed."
} finally {
    if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force }
}
