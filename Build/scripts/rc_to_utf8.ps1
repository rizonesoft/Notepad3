# Convert UTF-8-BOM encoded language RC files to pure UTF-8 (no BOM)
# Usage: .\rc_to_utf8.ps1

$ErrorActionPreference = "Stop"
$ScriptDir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot   = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$LangDir    = Join-Path $RepoRoot "language"

Write-Host "Stripping UTF-8 BOM from RC files in: $LangDir" -ForegroundColor Yellow

$bom       = [byte[]](0xEF, 0xBB, 0xBF)
$converted = 0
$skipped   = 0

Get-ChildItem -Path $LangDir -Filter "*.rc" -Recurse | ForEach-Object {
    $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
    if ($bytes.Length -ge 3 -and $bytes[0] -eq $bom[0] -and $bytes[1] -eq $bom[1] -and $bytes[2] -eq $bom[2]) {
        $stripped = $bytes[3..($bytes.Length - 1)]
        [System.IO.File]::WriteAllBytes($_.FullName, $stripped)
        Write-Host "  Converted: $($_.FullName.Substring($LangDir.Length + 1))" -ForegroundColor Cyan
        $converted++
    }
    else {
        $skipped++
    }
}

Write-Host ""
if ($converted -gt 0) {
    Write-Host "Done! Converted $converted file(s), skipped $skipped file(s) (no BOM)." -ForegroundColor Green
}
else {
    Write-Host "Done! No files needed conversion ($skipped file(s) already BOM-free)." -ForegroundColor Green
}
