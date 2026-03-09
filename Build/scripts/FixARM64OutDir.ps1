# Fix ARM64 Release OutDir/IntDir for all projects
# Many projects are missing these settings after VS Configuration Manager created ARM64 configs

param(
    [string]$RootPath = $null,
    [switch]$WhatIf = $false
)

$ErrorActionPreference = "Stop"

if (-not $RootPath) {
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $RootPath = Split-Path -Parent (Split-Path -Parent $ScriptDir)
}

Write-Host "Repository: $RootPath" -ForegroundColor Cyan
Write-Host "Mode: $(if ($WhatIf) { 'WhatIf (no changes)' } else { 'Apply Changes' })" -ForegroundColor Cyan
Write-Host ""

# Find all vcxproj files
$files = Get-ChildItem -Path $RootPath -Filter "*.vcxproj" -Recurse | 
Where-Object { $_.FullName -notmatch "_upgrade_staging" }

Write-Host "Scanning $($files.Count) project files..." -ForegroundColor Cyan
Write-Host ""

$modifiedCount = 0

foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    $original = $content
    
    # Check if this project has ARM64 configurations
    if ($content -notmatch "Release\|ARM64") {
        continue
    }
    
    # Check if Release|ARM64 PropertyGroup already has OutDir
    # Pattern: PropertyGroup with Release|ARM64 condition that contains OutDir
    if ($content -match "<PropertyGroup[^>]*Release\|ARM64[^>]*>[\s\S]*?<OutDir>") {
        Write-Host "[OK] $($file.Name) - Already has Release|ARM64 OutDir" -ForegroundColor Green
        continue
    }
    
    # Find the Release|x64 OutDir pattern to copy
    $x64Match = [regex]::Match($content, "<PropertyGroup Condition=`"'\`$\(Configuration\)\|\`$\(Platform\)'=='Release\|x64'`">\s*(?:<[^>]+>[^<]*</[^>]+>\s*)*<OutDir>([^<]+)</OutDir>\s*<IntDir>([^<]+)</IntDir>")
    
    if (-not $x64Match.Success) {
        # Try alternative pattern - some projects have different ordering
        $x64Match = [regex]::Match($content, "<PropertyGroup Condition=`"'\`$\(Configuration\)\|\`$\(Platform\)'=='Release\|x64'`">\s*[\s\S]*?<OutDir>([^<]+)</OutDir>[\s\S]*?<IntDir>([^<]+)</IntDir>")
    }
    
    if (-not $x64Match.Success) {
        Write-Host "[SKIP] $($file.Name) - No Release|x64 OutDir to copy from" -ForegroundColor Gray
        continue
    }
    
    $outDir = $x64Match.Groups[1].Value
    $intDir = $x64Match.Groups[2].Value
    
    # Find Release|ARM64 PropertyGroup and add OutDir/IntDir if missing
    # Pattern: PropertyGroup with Release|ARM64 condition, ending with </PropertyGroup>
    $arm64Pattern = "(<PropertyGroup Condition=`"'\`$\(Configuration\)\|\`$\(Platform\)'=='Release\|ARM64'`">)([\s\S]*?)(</PropertyGroup>)"
    
    $content = [regex]::Replace($content, $arm64Pattern, {
            param($match)
            $start = $match.Groups[1].Value
            $middle = $match.Groups[2].Value
            $end = $match.Groups[3].Value
        
            # Check if OutDir already exists in this group
            if ($middle -match "<OutDir>") {
                return $match.Value
            }
        
            # Add OutDir and IntDir before closing tag
            $newContent = $start + $middle + "    <OutDir>$outDir</OutDir>`r`n    <IntDir>$intDir</IntDir>`r`n  " + $end
            return $newContent
        })
    
    if ($content -ne $original) {
        Write-Host "[UPDATE] $($file.Name)" -ForegroundColor Yellow
        
        if (-not $WhatIf) {
            Set-Content -Path $file.FullName -Value $content -NoNewline
        }
        $modifiedCount++
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "Summary" -ForegroundColor White  
Write-Host "========================================" -ForegroundColor White
Write-Host "Files scanned: $($files.Count)"
Write-Host "Files modified: $modifiedCount"

if ($WhatIf -and $modifiedCount -gt 0) {
    Write-Host ""
    Write-Host "Run without -WhatIf to apply changes" -ForegroundColor Yellow
}
elseif ($modifiedCount -gt 0) {
    Write-Host ""
    Write-Host "Changes applied! ARM64 OutDir settings fixed." -ForegroundColor Green
}
