# Fix CETCompat for ARM64 configurations
# CET (Control-flow Enforcement Technology) is x86/x64 only - not compatible with ARM64
#
# This script finds all <CETCompat>true</CETCompat> entries in ARM64 ItemDefinitionGroups
# and changes them to false

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
    if ($content -notmatch "ARM64") {
        continue
    }
    
    # Find CETCompat within ARM64 ItemDefinitionGroups and change to false
    # Pattern: ItemDefinitionGroup with ARM64, then CETCompat>true
    $modified = $false
    
    # Match ARM64 ItemDefinitionGroups and fix CETCompat within them
    $pattern = "(<ItemDefinitionGroup[^>]*ARM64[^>]*>)([\s\S]*?)(</ItemDefinitionGroup>)"
    
    $content = [regex]::Replace($content, $pattern, {
            param($match)
            $start = $match.Groups[1].Value
            $middle = $match.Groups[2].Value
            $end = $match.Groups[3].Value
        
            # Replace CETCompat>true with CETCompat>false within this group
            if ($middle -match "<CETCompat>true</CETCompat>") {
                $middle = $middle -replace "<CETCompat>true</CETCompat>", "<CETCompat>false</CETCompat>"
                $script:modified = $true
            }
        
            return $start + $middle + $end
        })
    
    if ($content -ne $original) {
        Write-Host "[UPDATE] $($file.Name)" -ForegroundColor Yellow
        
        if (-not $WhatIf) {
            Set-Content -Path $file.FullName -Value $content -NoNewline
        }
        $modifiedCount++
    }
    elseif ($content -match "ARM64" -and $content -match "<CETCompat>false</CETCompat>") {
        Write-Host "[OK] $($file.Name) - CETCompat already false for ARM64" -ForegroundColor Green
    }
    elseif ($content -match "ARM64" -and $content -notmatch "CETCompat") {
        Write-Host "[SKIP] $($file.Name) - No CETCompat setting" -ForegroundColor Gray
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
    Write-Host "Changes applied! ARM64 CETCompat disabled." -ForegroundColor Green
}
