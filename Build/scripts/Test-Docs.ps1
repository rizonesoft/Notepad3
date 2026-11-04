<#
.SYNOPSIS
    Validates the docs\ tree: index completeness, relative links, anchors, page size.
.DESCRIPTION
    Errors (exit 1): page missing from docs\README.md, unresolved relative link or anchor.
    Metadata is optional; declared source anchors and verification commits are checked.
    Warnings: page size, possibly stale evidence, unavailable shallow history.
    Also checks links into docs from AGENTS.md, CLAUDE.md and readme**.md, and AGENTS.md code-span page paths.
    Fenced code blocks and inline code are ignored for links and headings.
#>
param(
    [string]$DocsDir = (Join-Path $PSScriptRoot '..\..\docs'),
    [int]$MaxPageBytes = 8192
)
$ErrorActionPreference = 'Stop'
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

$pages = Get-ChildItem -LiteralPath $docs -Recurse -Filter *.md
$readme = Join-Path $docs 'README.md'
$readmeText = (Read-Prose $readme) -join "`n"

foreach ($p in $pages) {
    $rel = $p.FullName.Substring($docs.Length + 1).Replace('\', '/')
    Test-Evidence $p.FullName $rel
    if ($p.FullName -ne $readme -and $readmeText -notmatch [regex]::Escape("]($rel)")) {
        Write-Host "ERROR  $rel is not linked from docs/README.md"; $errors++
    }
    if ($p.Length -gt $MaxPageBytes) {
        Write-Host "WARN   $rel is $($p.Length) bytes (> $MaxPageBytes); consider splitting"
    }
    $text = (Read-Prose $p.FullName) -join "`n" -replace '`[^`]*`', ''
    foreach ($m in [regex]::Matches($text, '\]\(([^)\s]+)\)')) {
        $target = $m.Groups[1].Value
        if ($target -match '^[a-z]+://|^mailto:') { continue }
        $path, $frag = $target, $null
        if ($target.Contains('#')) { $path, $frag = $target.Split('#', 2) }
        $dest = if ($path) { Join-Path $p.DirectoryName $path } else { $p.FullName }
        if (-not (Test-Path -LiteralPath $dest)) {
            Write-Host "ERROR  $rel -> $target (file not found)"; $errors++; continue
        }
        if ($frag -and $dest -like '*.md' -and ((Get-Anchors $dest) -notcontains $frag)) {
            Write-Host "ERROR  $rel -> $target (anchor not found)"; $errors++
        }
    }
}

# Inbound references: files outside docs\ that point into docs\.
# Markdown links resolving inside docs\ must exist (with anchors); AGENTS.md code spans
# such as `docs/domain/x.md` must name an existing page.
$inbound = @()
foreach ($n in 'AGENTS.md', 'CLAUDE.md') {
    if (Test-Path -LiteralPath (Join-Path $root $n)) { $inbound += Get-Item -LiteralPath (Join-Path $root $n) }
}
if (Test-Path -LiteralPath (Join-Path $root 'readme')) {
    $inbound += Get-ChildItem -LiteralPath (Join-Path $root 'readme') -Recurse -Filter *.md
}
$instructionPaths = @(& git -C $root ls-files -- ':(glob)**/AGENTS.md' ':(glob)**/CLAUDE.md' '.github/copilot-instructions.md' '.github/instructions/*.instructions.md' 'lexilla/np3_patches/README.md' 'scintilla/np3_patches/upstream_558/README.md' 2>$null)
foreach ($path in $instructionPaths) {
    if (Test-Path -LiteralPath (Join-Path $root $path)) { $inbound += Get-Item -LiteralPath (Join-Path $root $path) }
}
$inbound = @($inbound | Sort-Object FullName -Unique)
foreach ($f in $inbound) {
    $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
    $raw = (Read-Prose $f.FullName) -join "`n"
    foreach ($m in [regex]::Matches(($raw -replace '`[^`]*`', ''), '\]\(([^)\s]+)\)')) {
        $target = $m.Groups[1].Value
        if ($target -match '^[a-z]+://|^mailto:|^#') { continue }
        $path, $frag = $target, $null
        if ($target.Contains('#')) { $path, $frag = $target.Split('#', 2) }
        $dest = [System.IO.Path]::GetFullPath((Join-Path $f.DirectoryName $path))
        $isInstruction = $f.Name -in 'AGENTS.md', 'CLAUDE.md', 'copilot-instructions.md' -or $f.Name -like '*.instructions.md'
        if (!$isInstruction -and -not $dest.StartsWith($docs + '\', [StringComparison]::OrdinalIgnoreCase)) { continue }
        if (-not (Test-Path -LiteralPath $dest)) {
            Write-Host "ERROR  $rel -> $target (file not found)"; $errors++; continue
        }
        if ($frag -and $dest -like '*.md' -and ((Get-Anchors $dest) -notcontains $frag)) {
            Write-Host "ERROR  $rel -> $target (anchor not found)"; $errors++
        }
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
    if ($f.Name -eq 'AGENTS.md') {
        foreach ($m in [regex]::Matches($raw, '`(docs/[^`\s]+\.md)`')) {
            if (-not (Test-Path -LiteralPath (Join-Path $root $m.Groups[1].Value))) {
                Write-Host "ERROR  $rel cites missing page $($m.Groups[1].Value)"; $errors++
            }
        }
    }
}
if ($errors) { Write-Host "$errors error(s)"; exit 1 }
Write-Host "docs OK ($($pages.Count) pages)"
