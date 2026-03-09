# Build All Configurations for Notepad3
# Usage: .\BuildAll.ps1 [-Configuration Release|Debug] [-Clean]
#
# This script builds all platforms: Win32, x64, x64_AVX2, ARM64
# It auto-detects Visual Studio installation (Professional, Community, Enterprise)

param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    [switch]$Clean = $false,
    [switch]$WhatIf = $false
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
            Write-Host "Found: $vsEdition" -ForegroundColor Cyan
            return $msbuild
        }
    }
    
    # Fallback to PATH
    $msbuild = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($msbuild) { return $msbuild.Source }
    
    throw "MSBuild not found. Please install Visual Studio or run from Developer Command Prompt."
}

# Build function
function Build-Platform {
    param(
        [string]$Platform,
        [string]$Config,
        [string]$MSBuild,
        [string]$ExtraArgs = ""
    )
    
    $sln = Join-Path $RepoRoot "Notepad3.sln"
    $displayName = $Platform
    
    # Handle AVX2 special case - use native Release_AVX2 configuration
    $actualPlatform = $Platform
    $actualConfig = $Config
    if ($Platform -eq "x64_AVX2") {
        $actualPlatform = "x64"
        $actualConfig = "${Config}_AVX2"
        $displayName = "x64 (AVX2)"
    }
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Yellow
    Write-Host "Building: $displayName $actualConfig" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    
    $buildArgs = @(
        "`"$sln`"",
        "/m",
        "/p:Configuration=$actualConfig",
        "/p:Platform=$actualPlatform",
        "/v:minimal"
    )
    
    # ExtraArgs no longer needed for AVX2
    if ($Clean) { $buildArgs += "/t:Clean" }
    
    $cmd = "& `"$MSBuild`" $($buildArgs -join ' ')"
    
    if ($WhatIf) {
        Write-Host "[WhatIf] $cmd" -ForegroundColor Gray
        return $true
    }
    
    Write-Host $cmd -ForegroundColor Gray
    $result = & $MSBuild $buildArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build FAILED: $displayName $Config" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Build SUCCESS: $displayName $Config" -ForegroundColor Green
    return $true
}

# Main
$msbuild = Find-MSBuild
Write-Host "MSBuild: $msbuild" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Cyan
Write-Host "Repository: $RepoRoot" -ForegroundColor Cyan

# Build all platforms - AVX2 uses native Release_AVX2 configuration
# Order doesn't matter anymore since AVX2 has its own output directory
$platforms = @("Win32", "x64", "x64_AVX2", "ARM64")
$results = @{}

foreach ($platform in $platforms) {
    $success = Build-Platform -Platform $platform -Config $Configuration -MSBuild $msbuild
    $results[$platform] = $success
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "Build Summary" -ForegroundColor White
Write-Host "========================================" -ForegroundColor White

foreach ($platform in $platforms) {
    $status = if ($results[$platform]) { "SUCCESS" } else { "FAILED" }
    $color = if ($results[$platform]) { "Green" } else { "Red" }
    Write-Host "$platform $Configuration : $status" -ForegroundColor $color
}

$failed = $results.Values | Where-Object { -not $_ }
if ($failed.Count -gt 0) {
    Write-Host ""
    Write-Host "$($failed.Count) build(s) failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "All builds completed successfully!" -ForegroundColor Green
exit 0
