<#
.SYNOPSIS
    Validates the docs\ tree: index completeness, relative links, anchors, page size.
.DESCRIPTION
    Errors (exit 1): page missing from docs\README.md, unresolved relative link or anchor.
    Warnings: page larger than -MaxPageBytes (split candidate).
    Also checks links into docs from AGENTS.md, CLAUDE.md and readme**.md, and AGENTS.md code-span page paths.
    Fenced code blocks and inline code are ignored for links and headings.
#>
param(
    [string]$DocsDir = (Join-Path $PSScriptRoot '..\..\docs'),
    [int]$MaxPageBytes = 8192
)
$ErrorActionPreference = 'Stop'
$docs = (Resolve-Path $DocsDir).Path
$errors = 0

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
$root = (Resolve-Path (Join-Path $docs '..')).Path
$inbound = @()
foreach ($n in 'AGENTS.md', 'CLAUDE.md') {
    if (Test-Path -LiteralPath (Join-Path $root $n)) { $inbound += Get-Item -LiteralPath (Join-Path $root $n) }
}
if (Test-Path -LiteralPath (Join-Path $root 'readme')) {
    $inbound += Get-ChildItem -LiteralPath (Join-Path $root 'readme') -Recurse -Filter *.md
}
foreach ($f in $inbound) {
    $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
    $raw = (Read-Prose $f.FullName) -join "`n"
    foreach ($m in [regex]::Matches(($raw -replace '`[^`]*`', ''), '\]\(([^)\s]+)\)')) {
        $target = $m.Groups[1].Value
        if ($target -match '^[a-z]+://|^mailto:|^#') { continue }
        $path, $frag = $target, $null
        if ($target.Contains('#')) { $path, $frag = $target.Split('#', 2) }
        $dest = [System.IO.Path]::GetFullPath((Join-Path $f.DirectoryName $path))
        if (-not $dest.StartsWith($docs + '\', [StringComparison]::OrdinalIgnoreCase)) { continue }
        if (-not (Test-Path -LiteralPath $dest)) {
            Write-Host "ERROR  $rel -> $target (file not found)"; $errors++; continue
        }
        if ($frag -and $dest -like '*.md' -and ((Get-Anchors $dest) -notcontains $frag)) {
            Write-Host "ERROR  $rel -> $target (anchor not found)"; $errors++
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
