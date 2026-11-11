<#
.SYNOPSIS
    Validates the agent-trail\ tree: index completeness, relative links, anchors, page size.
.DESCRIPTION
    Errors (exit 1): page missing from agent-trail\index.md, unresolved relative link or anchor.
    Metadata is optional; declared source anchors and verification commits are checked.
    Warnings: page size, possibly stale evidence, unavailable shallow history.
    Checks the index-declared layout, loading exclusions, routing triggers and skill targets.
    Also checks inbound links and recognizable repository paths in prose code spans.
    Fenced code blocks and inline code are ignored for links and headings.
#>
param(
    [string]$DocsDir,
    [int]$MaxPageBytes = 8192
)
$ErrorActionPreference = 'Stop'
if (!$DocsDir) {
    $repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
    $entry = Get-Content -LiteralPath (Join-Path $repo 'AGENTS.md') -Raw -Encoding UTF8
    if ($entry -notmatch '\]\(([^)#]+/index\.md)\)') { throw 'AGENTS.md must link the topic index' }
    $index = [IO.Path]::GetFullPath($Matches[1], $repo)
    if (!$index.StartsWith($repo + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Topic index must stay inside the repository'
    }
    $DocsDir = Split-Path $index
}
$docs = (Resolve-Path $DocsDir).Path
$root = (Resolve-Path (Join-Path $docs '..')).Path
$errors = 0

function Evidence-Error([string]$message) {
    Write-Host "ERROR  $message"
    $script:errors++
}

# Deliberately limited to the documented scalar/list schema; no YAML dependency.
function Test-Evidence([string]$file, [string]$rel) {
    $lines = @(Get-Content -LiteralPath $file -Encoding UTF8)
    if (!$lines.Count -or $lines[0] -ne '---') { return }
    $meta = @{}
    $anchors = [System.Collections.Generic.List[hashtable]]::new()
    $anchor = $null
    $inAnchors = $false
    $closed = $false
    foreach ($line in $lines | Select-Object -Skip 1) {
        if ($line -eq '---') { $closed = $true; break }
        if (!$line.Trim()) { continue }
        if ($line -eq 'anchors:' -and !$meta.ContainsKey('anchors')) {
            $meta['anchors'] = $true
            $inAnchors = $true
            continue
        }
        if ($inAnchors -and $line -match '^  - path: (.+)$') {
            $anchor = @{ path = $Matches[1].Trim().Trim('"').Trim("'") }
            $anchors.Add($anchor)
            continue
        }
        if ($inAnchors -and $anchor -and $line -match '^    (symbol|reason): (.+)$') {
            $key, $value = $Matches[1], $Matches[2].Trim().Trim('"').Trim("'")
            if ($anchor.ContainsKey($key)) { Evidence-Error "$rel duplicate anchor field: $key"; continue }
            $anchor[$key] = $value
            continue
        }
        if (!$inAnchors -and $line -match '^(status|verified-at|owner): (.+)$') {
            $key, $value = $Matches[1], $Matches[2].Trim().Trim('"').Trim("'")
            if ($meta.ContainsKey($key)) { Evidence-Error "$rel duplicate metadata field: $key"; continue }
            $meta[$key] = $value
            continue
        }
        Evidence-Error "$rel unsupported metadata line: $line"
    }
    if (!$closed) { Evidence-Error "$rel unterminated metadata"; return }
    if ($meta.status -notin 'agent-observed', 'human-verified') {
        Evidence-Error "$rel invalid or missing status"
    }
    if (!$meta.owner) { Evidence-Error "$rel missing owner" }
    elseif ($meta.owner -eq 'unassigned') { Write-Host "WARN   $rel owner is unassigned" }
    if (!$anchors.Count) { Evidence-Error "$rel missing anchors" }
    $commit = $meta['verified-at']
    if ($commit -notmatch '^[0-9a-fA-F]{40}$') {
        Evidence-Error "$rel verified-at must be a full source commit"
        return
    }
    $gitRoot = & git -C $root rev-parse --show-toplevel 2>$null
    if ($LASTEXITCODE -ne 0 -or [IO.Path]::GetFullPath($gitRoot) -ne $root) {
        Evidence-Error "$rel metadata requires the docs parent to be a Git root"
        return
    }
    & git -C $root cat-file -e "$commit^{commit}" 2>$null
    $historyAvailable = $LASTEXITCODE -eq 0
    if (!$historyAvailable) {
        if ((& git -C $root rev-parse --is-shallow-repository) -eq 'true') {
            Write-Host "WARN   $rel verification unavailable (shallow history): $commit"
        } else { Evidence-Error "$rel unresolved verification commit: $commit" }
    } else {
        & git -C $root merge-base --is-ancestor $commit HEAD 2>$null
        if ($LASTEXITCODE -ne 0) { Evidence-Error "$rel verification commit is not an ancestor of HEAD"; return }
    }
    foreach ($a in $anchors) {
        $path = $a.path.Replace('\', '/')
        $dest = [IO.Path]::GetFullPath((Join-Path $root $path))
        if ($path -match '(^/|:|(^|/)\.\.(/|$)|[*?\[])' -or
            !$dest.StartsWith($root + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
            Evidence-Error "$rel anchor must stay inside the repository: $path"; continue
        }
        & git -C $root --literal-pathspecs ls-files --error-unmatch -- $path 1>$null 2>$null
        if ($LASTEXITCODE -ne 0) { Evidence-Error "$rel anchor is not tracked: $path"; continue }
        if (!(Test-Path -LiteralPath $dest -PathType Leaf)) {
            Evidence-Error "$rel anchor file not found: $path"; continue
        }
        if ($a.symbol) {
            $pattern = '(?<!\w)' + [regex]::Escape($a.symbol) + '(?!\w)'
            if ((Get-Content -LiteralPath $dest -Raw -Encoding UTF8) -cnotmatch $pattern) {
                Evidence-Error "$rel symbol not found: ${path}:$($a.symbol)"
            }
        } elseif (!$a.reason) { Evidence-Error "$rel path-only anchor needs a reason: $path" }
        if ($historyAvailable) {
            & git -C $root --literal-pathspecs diff --quiet $commit HEAD -- $path
            if ($LASTEXITCODE -eq 1) { Write-Host "WARN   $rel possibly stale (source changed): $path" }
            elseif ($LASTEXITCODE -ne 0) { Evidence-Error "$rel cannot compare source history: $path" }
        }
        $dirty = & git -C $root --literal-pathspecs status --porcelain -- $path
        if ($LASTEXITCODE -ne 0) { Evidence-Error "$rel cannot inspect source worktree: $path" }
        elseif ($dirty) { Write-Host "WARN   $rel possibly stale (dirty source): $path" }
    }
}

# Returns the file text without fenced code blocks.
function Read-Prose([string]$file) {
    $inFence = $false
    $out = foreach ($l in (Get-Content -LiteralPath $file -Encoding UTF8)) {
        if ($l -match '^\s*(```|~~~)') { $inFence = -not $inFence; continue }
        if (-not $inFence) { $l }
    }
    $out
}

# GitHub-style heading anchors, including -1/-2 suffixes for duplicates.
function Get-Anchors([string]$file) {
    $seen = @{}
    foreach ($l in (Read-Prose $file)) {
        if ($l -match '^#{1,6}\s+(.+?)\s*$') {
            $a = $Matches[1].ToLowerInvariant() -replace '`', '' -replace '[^\p{L}\p{N}\s_-]', '' -replace '\s', '-'
            if ($seen.ContainsKey($a)) { $seen[$a]++; "$a-$($seen[$a])" } else { $seen[$a] = 0; $a }
        }
    }
}

function Test-Contained([string]$path) {
    $path.StartsWith($root + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)
}

function Test-Links([IO.FileInfo]$file, [bool]$inboundOnly = $false) {
    $rel = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = (Read-Prose $file.FullName) -join "`n" -replace '`[^`]*`', ''
    foreach ($m in [regex]::Matches($text, '\]\(([^)\s]+)\)')) {
        $target = $m.Groups[1].Value
        if ($target -match '^[a-z][a-z0-9+.-]*:') { continue }
        $path, $frag = $target, $null
        if ($target.Contains('#')) { $path, $frag = $target.Split('#', 2) }
        $dest = if ($path) { [IO.Path]::GetFullPath($path, $file.DirectoryName) } else { $file.FullName }
        if ($inboundOnly -and !$dest.StartsWith($docs + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { continue }
        if (!(Test-Contained $dest)) {
            Evidence-Error "$rel -> $target (link must stay inside repository)"; continue
        }
        if (!(Test-Path -LiteralPath $dest)) {
            Evidence-Error "$rel -> $target (file not found)"; continue
        }
        if ($frag -and $dest -like '*.md' -and ((Get-Anchors $dest) -notcontains $frag)) {
            Evidence-Error "$rel -> $target (anchor not found)"
        }
    }
}

function Test-CodePaths([IO.FileInfo]$file) {
    $rel = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
    $text = ((Read-Prose $file.FullName) | Where-Object { $_ -notmatch '^- Forbidden adapters:' }) -join "`n"
    foreach ($m in [regex]::Matches($text, '`([^`\r\n]+)`')) {
        $path = $m.Groups[1].Value.Replace('\', '/')
        if ($path -notmatch '/' -or $path -match '://|[\s<>*?{}()|=%]' -or $path -notmatch '\.[a-zA-Z0-9]+/?$|/$') { continue }
        $first = $path.Split('/')[0]
        if (!$first) { continue }
        $rooted = $first -notin '.', '..' -and ![IO.Path]::IsPathRooted($path) -and (Test-Path -LiteralPath (Join-Path $root $first))
        $dest = [IO.Path]::GetFullPath($path, $(if ($rooted) { $root } else { $file.DirectoryName }))
        if (Test-Path -LiteralPath $dest) { continue }
        if ($rooted -and (Test-Contained $dest)) { Evidence-Error "$rel cites missing repository path $path" }
        else { Write-Host "WARN   $rel unverified path outside known repository roots: $path" }
    }
}

$pages = Get-ChildItem -LiteralPath $docs -Recurse -Filter *.md
$readme = Join-Path $docs 'index.md'
$readmeText = (Read-Prose $readme) -join "`n"
$indexRel = $readme.Substring($root.Length + 1).Replace('\', '/')
$layout = @{}
foreach ($key in 'Topic folders', 'Instruction files', 'Forbidden adapters', 'Skill roots') {
    $declarations = @([regex]::Matches($readmeText, '(?m)^- ' + [regex]::Escape($key) + ': (.+)$'))
    if ($declarations.Count -ne 1) { Evidence-Error "$indexRel must declare $key exactly once"; $layout[$key] = @(); continue }
    $layout[$key] = @([regex]::Matches($declarations[0].Groups[1].Value, '`([^`]+)`') | ForEach-Object { $_.Groups[1].Value })
    if (!$layout[$key].Count) { Evidence-Error "$indexRel empty declaration: $key" }
    foreach ($path in $layout[$key]) {
        if ($path -match '(^[/\\]|:|(^|[/\\])\.\.([/\\]|$)|[*?\[])') {
            Evidence-Error "$indexRel invalid layout path: $path"
        }
    }
}
if ($errors) { exit 1 }

foreach ($p in $pages) {
    $rel = $p.FullName.Substring($docs.Length + 1).Replace('\', '/')
    Test-Evidence $p.FullName $rel
    if ($p.FullName -ne $readme -and $readmeText -notmatch [regex]::Escape("]($rel)")) {
        Write-Host "ERROR  $rel is not linked from $indexRel"; $errors++
    }
    if ($p.FullName -ne $readme) {
        if ($rel.Contains('/') -and $rel.Split('/')[0] -notin $layout['Topic folders']) {
            Evidence-Error "$rel is in an undeclared topic folder"
        }
        $triggers = @([regex]::Matches(((Read-Prose $p.FullName) -join "`n"), '(?m)^When to read: (.+?)\r?$'))
        if ($triggers.Count -ne 1) { Evidence-Error "$rel needs exactly one When to read line" }
        else {
            $trigger = $triggers[0].Groups[1].Value
            $rows = @($readmeText -split "`n" | Where-Object { $_.Contains("]($rel)") })
            if (!($rows | Where-Object { $_.StartsWith("| $trigger |", [StringComparison]::Ordinal) })) {
                Evidence-Error "$rel when-to-read trigger does not match its index row"
            }
        }
    }
    if ($p.Length -gt $MaxPageBytes) {
        Write-Host "WARN   $rel is $($p.Length) bytes (> $MaxPageBytes); consider splitting"
    }
    Test-Links $p
    Test-CodePaths $p
}

$inbound = @()
$declaredInstructionPaths = @($layout['Instruction files'] | ForEach-Object { $_.Replace('\', '/') })
$actualInstructionPaths = @(& git -C $root ls-files --cached -- ':(glob)**/AGENTS.md' 2>$null)
if ($LASTEXITCODE -ne 0) { Evidence-Error 'cannot enumerate tracked instruction files' }
$actualInstructionPaths += @(& git -C $root ls-files --others --exclude-standard -- ':(glob)**/AGENTS.md' 2>$null)
if ($LASTEXITCODE -ne 0) { Evidence-Error 'cannot enumerate untracked instruction files' }
$actualInstructionPaths += @(& git -C $root ls-files --others --ignored --exclude-standard -- ':(glob)**/AGENTS.md' 2>$null)
if ($LASTEXITCODE -ne 0) { Evidence-Error 'cannot enumerate ignored instruction files' }
foreach ($path in @($actualInstructionPaths | Sort-Object -Unique)) {
    if ($path -notin $declaredInstructionPaths) {
        Evidence-Error "instruction file is not declared: $path"
    }
}
foreach ($path in $layout['Instruction files']) {
    $dest = Join-Path $root $path
    if (!(Test-Path -LiteralPath $dest -PathType Leaf)) { Evidence-Error "missing instruction file: $path"; continue }
    $inbound += Get-Item -LiteralPath $dest
    foreach ($adapter in $layout['Forbidden adapters']) {
        $forbidden = Join-Path (Split-Path $dest) $adapter
        if (Test-Path -LiteralPath $forbidden) { Evidence-Error "forbidden adapter: $forbidden" }
    }
}
if (Test-Path -LiteralPath (Join-Path $root 'readme')) {
    $inbound += Get-ChildItem -LiteralPath (Join-Path $root 'readme') -Recurse -Filter *.md
}
$instructionPaths = @(& git -C $root ls-files -- '*.md' 2>$null)
foreach ($path in $instructionPaths) {
    $dest = Join-Path $root $path
    if ((Test-Path -LiteralPath $dest -PathType Leaf) -and
        !([IO.Path]::GetFullPath($dest).StartsWith($docs + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase))) {
        $inbound += Get-Item -LiteralPath $dest
    }
}
foreach ($skillRoot in $layout['Skill roots']) {
    $dest = Join-Path $root $skillRoot
    if (!(Test-Path -LiteralPath $dest -PathType Container)) { continue }
    foreach ($skill in Get-ChildItem -LiteralPath $dest -Recurse -Filter SKILL.md) {
        $raw = (Read-Prose $skill.FullName) -join "`n"
        $targets = @([regex]::Matches($raw, '\]\(([^)\s]+)\)') | Where-Object {
            $link = $_.Groups[1].Value
            if ($link -notmatch '^[a-z][a-z0-9+.-]*:') {
                $target = [IO.Path]::GetFullPath((Join-Path $skill.DirectoryName ($link.Split('#', 2)[0])))
                $target.StartsWith((Join-Path $docs 'playbooks') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -and
                    (Test-Path -LiteralPath $target -PathType Leaf)
            }
        })
        if (!$targets.Count) { Evidence-Error "$($skill.FullName) must link a canonical playbook" }
        $inbound += $skill
    }
}
$inbound = @($inbound | Sort-Object FullName -Unique)
foreach ($f in $inbound) {
    $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
    $raw = (Read-Prose $f.FullName) -join "`n"
    $isInstruction = $f.Name -in 'AGENTS.md', 'CLAUDE.md', 'copilot-instructions.md', 'SKILL.md' -or $f.Name -like '*.instructions.md'
    Test-Links $f (!$isInstruction)
    if ($isInstruction) { Test-CodePaths $f }
    if ($f.Name -eq 'copilot-instructions.md' -and $raw -notmatch '\]\(\.\./AGENTS\.md\)') {
        Evidence-Error "$rel must link the canonical root AGENTS.md"
    }
    if ($f.Name -in 'AGENTS.md', 'CLAUDE.md', 'copilot-instructions.md') {
        foreach ($line in Read-Prose $f.FullName) {
            if ($line -match '^@([^\s]+)\s*$') {
                $dest = [IO.Path]::GetFullPath((Join-Path $f.DirectoryName $Matches[1]))
                if (!$dest.StartsWith($root + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase) -or
                    !(Test-Path -LiteralPath $dest -PathType Leaf)) {
                    Evidence-Error "$rel unresolved or external instruction import: $line"
                }
            }
        }
    }
}
if ($errors) { Write-Host "$errors error(s)"; exit 1 }
Write-Host "docs OK ($($pages.Count) pages)"
