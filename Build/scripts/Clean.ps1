# Clean All Build Outputs
# Usage: .\Clean.ps1

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir
$BinDir = Join-Path $RepoRoot "Bin"

Write-Host "Cleaning build outputs..." -ForegroundColor Yellow
Write-Host "Bin directory: $BinDir" -ForegroundColor Cyan

if (Test-Path $BinDir) {
    $dirs = Get-ChildItem -Path $BinDir -Directory
    foreach ($dir in $dirs) {
        Write-Host "  Removing: $($dir.Name)" -ForegroundColor Gray
        Remove-Item -Path $dir.FullName -Recurse -Force -ErrorAction SilentlyContinue
    }
    Write-Host "Clean completed!" -ForegroundColor Green
}
else {
    Write-Host "Bin directory does not exist - nothing to clean." -ForegroundColor Yellow
}
