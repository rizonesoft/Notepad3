# Fix ARM64 Cross-Compilation for Language Projects
# This script updates muirct.exe paths to use x64 host tools instead of target platform
#
# Problem: Post-build events use $(PlatformShortName)\muirct.exe which tries to run
#          ARM64 binaries on x64 host when cross-compiling.
# Fix: Use x64\muirct.exe always (works for all targets on x64 host)

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

# Find all language project files
$languageDirs = @(
    (Join-Path $RootPath "language"),
    (Join-Path $RootPath "minipath\language")
)

$modifiedCount = 0
$files = @()

foreach ($dir in $languageDirs) {
    if (Test-Path $dir) {
        $files += Get-ChildItem -Path $dir -Filter "*.vcxproj" -Recurse
    }
}

Write-Host "Found $($files.Count) language project files" -ForegroundColor Cyan
Write-Host ""

foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    $original = $content
    
    # Pattern: $(UCRTContentRoot)bin\$(TargetPlatformVersion)\$(PlatformShortName)\muirct.exe
    # Replace $(PlatformShortName) with x64 for muirct.exe path
    
    # This regex matches the muirct.exe path and replaces $(PlatformShortName) with x64
    $pattern = '(\$\(UCRTContentRoot\)bin\\\$\(TargetPlatformVersion\)\\)\$\(PlatformShortName\)(\\muirct\.exe)'
    $replacement = '${1}x64${2}'
    
    $content = $content -replace $pattern, $replacement
    
    if ($content -ne $original) {
        Write-Host "[UPDATE] $($file.Name)" -ForegroundColor Yellow
        
        if (-not $WhatIf) {
            Set-Content -Path $file.FullName -Value $content -NoNewline
        }
        $modifiedCount++
    }
    else {
        # Check if already fixed or uses different pattern
        if ($content -match 'x64\\muirct\.exe') {
            Write-Host "[OK] $($file.Name) - Already using x64" -ForegroundColor Green
        }
        elseif ($content -match 'muirct\.exe') {
            Write-Host "[MANUAL] $($file.Name) - Uses different pattern, check manually" -ForegroundColor Magenta
        }
        else {
            Write-Host "[SKIP] $($file.Name) - No muirct.exe usage" -ForegroundColor Gray
        }
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
    Write-Host "Changes applied! ARM64 cross-compilation should now work." -ForegroundColor Green
}
