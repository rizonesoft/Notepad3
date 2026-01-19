# Build Single Platform for Notepad3
# Usage: .\Build.ps1 -Platform x64 [-Configuration Release|Debug] [-Clean]
#
# Platforms: Win32, x64, x64_AVX2, ARM64

param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Win32", "x64", "x64_AVX2", "ARM64")]
    [string]$Platform,
    
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    
    [switch]$Clean = $false
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)

# Find MSBuild via vswhere (prefers VS 2022, falls back to latest)
function Find-MSBuild {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        # Try VS 2022 first (version 17.x)
        $vsPath = & $vswhere -version "[17.0,18.0)" -property installationPath 2>$null
        if (-not $vsPath) {
            # Fallback to latest
            $vsPath = & $vswhere -latest -property installationPath
        }
        $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
        if (Test-Path $msbuild) {
            $vsEdition = & $vswhere -path $vsPath -property catalog_productDisplayName 2>$null
            if ([string]::IsNullOrWhiteSpace($vsEdition)) {
                $vsVersion = & $vswhere -path $vsPath -property installationVersion 2>$null
                $vsEdition = "Visual Studio $vsVersion"
            }
            Write-Host "Visual Studio: $vsEdition" -ForegroundColor Cyan
            return $msbuild
        }
    }
    
    # Fallback to PATH
    $msbuild = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($msbuild) { return $msbuild.Source }
    
    throw "MSBuild not found. Install Visual Studio or run from Developer Command Prompt."
}

$msbuild = Find-MSBuild
$sln = Join-Path $RepoRoot "Notepad3.sln"

Write-Host "MSBuild: $msbuild" -ForegroundColor Cyan
Write-Host "Solution: $sln" -ForegroundColor Cyan
Write-Host "Platform: $Platform | Configuration: $Configuration" -ForegroundColor Cyan
Write-Host ""

# Handle platform and configuration
$actualPlatform = $Platform
$actualConfig = $Configuration

if ($Platform -eq "x64_AVX2") {
    # Use native Release_AVX2 configuration
    $actualPlatform = "x64"
    $actualConfig = "${Configuration}_AVX2"
    Write-Host "AVX2 Mode: Using $actualConfig configuration" -ForegroundColor Yellow
}

# Build arguments
$buildArgs = @(
    "`"$sln`"",
    "/m",
    "/p:Configuration=$actualConfig",
    "/p:Platform=$actualPlatform",
    "/v:minimal"
)

if ($Clean) {
    $buildArgs += "/t:Clean"
    Write-Host "Mode: Clean" -ForegroundColor Yellow
}

Write-Host "Building..." -ForegroundColor Yellow
& $msbuild $buildArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
exit 0

