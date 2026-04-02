# Build PortableApps Package for Notepad3
# Usage: .\BuildPortableApp.ps1 [-SkipBuild] [-SkipVersionPatch]
#          [-SkipLauncherGenerator] [-PortableAppsDir <path>]
#
# Steps:
#   1. Run Version.ps1 to generate VersionEx.h
#   2. Build x64 and Win32 Release
#   3. Copy binaries to PortableApp structure
#   4. Process appinfo_template.ini with version info
#   5. Generate Notepad3Portable.exe launcher
#   6. Package with PortableApps.comInstaller.exe

[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [switch]$SkipVersionPatch,
    [switch]$SkipLauncherGenerator,
    [string]$PortableAppsDir = "D:\PortableApps"
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)

$InstallerPath = Join-Path $PortableAppsDir "PortableApps.comInstaller\PortableApps.comInstaller.exe"

$PortableAppRoot = Join-Path $RepoRoot "np3portableapp"
$PortableAppDir = Join-Path $PortableAppRoot "Notepad3Portable"
$AppInfoDir = Join-Path $PortableAppDir "App\AppInfo"
$Notepad3AppDir = Join-Path $PortableAppDir "App\Notepad3"

# ============================================================
# Helper: colored step banner
# ============================================================
function Write-Step {
    param([int]$Number, [string]$Text)
    Write-Host ""
    Write-Host "--- Step $Number : $Text ---" -ForegroundColor Cyan
}

# ============================================================
# Helper: detect PlatformToolset from installed Visual Studio
# ============================================================
function Get-PlatformToolset {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found. Install Visual Studio."
    }
    $vsVersion = & $vswhere -latest -property installationVersion 2>$null
    if (-not $vsVersion) {
        throw "No Visual Studio installation found via vswhere."
    }
    $major = ([version]$vsVersion).Major
    $toolsetMap = @{ 15 = 'v141'; 16 = 'v142'; 17 = 'v143'; 18 = 'v145' }
    $toolset = $toolsetMap[$major]
    if (-not $toolset) {
        throw "Unknown Visual Studio major version $major - cannot determine PlatformToolset."
    }
    Write-Host "Visual Studio version: $vsVersion  ->  PlatformToolset: $toolset" -ForegroundColor Cyan
    return $toolset
}

# ============================================================
# Helper: parse version defines from VersionEx.h
# ============================================================
function Get-VersionFromHeader {
    $headerPath = Join-Path $RepoRoot "src\VersionEx.h"
    if (-not (Test-Path $headerPath)) {
        throw "src\VersionEx.h not found. Run Version.ps1 first."
    }
    $content = Get-Content $headerPath -Raw
    $defines = @{}
    foreach ($name in @('VERSION_MAJOR', 'VERSION_MINOR', 'VERSION_REV', 'VERSION_BUILD')) {
        if ($content -match "#define\s+$name\s+(\d+)") {
            $defines[$name] = $Matches[1]
        }
        else {
            throw "Could not parse $name from VersionEx.h"
        }
    }
    $version = "$($defines['VERSION_MAJOR']).$($defines['VERSION_MINOR']).$($defines['VERSION_REV']).$($defines['VERSION_BUILD'])"
    return $version
}

# ============================================================
# Helper: copy platform binaries to portable app target
# ============================================================
function Copy-PlatformBinaries {
    param(
        [string]$SourceDir,
        [string]$TargetDir
    )

    if (-not (Test-Path $SourceDir)) {
        throw "Build output not found: $SourceDir"
    }

    # Executables
    foreach ($exe in @('Notepad3.exe', 'minipath.exe')) {
        $src = Join-Path $SourceDir $exe
        $dst = Join-Path $TargetDir $exe
        if (-not (Test-Path $src)) {
            throw "Missing binary: $src"
        }
        Copy-Item $src $dst -Force
        Write-Host "  Copied $exe" -ForegroundColor Gray
    }

    # Language DLLs (np3lng.dll, mplng.dll)
    $srcLng = Join-Path $SourceDir "lng"
    $dstLng = Join-Path $TargetDir "lng"
    foreach ($dll in @('np3lng.dll', 'mplng.dll')) {
        $src = Join-Path $srcLng $dll
        $dst = Join-Path $dstLng $dll
        if (-not (Test-Path $src)) {
            Write-Warning "Missing language DLL: $src (skipping)"
            continue
        }
        Copy-Item $src $dst -Force
        Write-Host "  Copied lng\$dll" -ForegroundColor Gray
    }

    # Locale MUI files - copy each locale subdirectory that exists in the target
    $localeCount = 0
    Get-ChildItem $dstLng -Directory | Where-Object { $_.Name -match '^\w{2}-\w{2}$' } | ForEach-Object {
        $locale = $_.Name
        $srcLocale = Join-Path $srcLng $locale
        if (Test-Path $srcLocale) {
            Copy-Item "$srcLocale\*" $_.FullName -Force -Recurse
            $localeCount++
        }
        else {
            Write-Warning "  Locale $locale not found in build output (skipping)"
        }
    }
    Write-Host "  Copied $localeCount locale directories" -ForegroundColor Gray
}

# ============================================================
# Main
# ============================================================
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
Write-Host "=== Notepad3 PortableApps Package Builder ===" -ForegroundColor Green
Write-Host "Repository: $RepoRoot" -ForegroundColor Cyan

# ----------------------------------------------------------
# Step 1: Version patching
# ----------------------------------------------------------
Write-Step 1 "Version Patching"
if ($SkipVersionPatch) {
    Write-Host "Skipped (VersionEx.h assumed up-to-date)" -ForegroundColor Yellow
}
else {
    Push-Location $RepoRoot
    try {
        & (Join-Path $RepoRoot "Version.ps1")
        if ($LASTEXITCODE -ne 0) {
            throw "Version.ps1 failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

# ----------------------------------------------------------
# Step 2: Parse version info
# ----------------------------------------------------------
Write-Step 2 "Reading Version Info"
$Version = Get-VersionFromHeader
Write-Host "Version: $Version" -ForegroundColor Green

# Read dev name from _buildname.txt
$buildNamePath = Join-Path $PortableAppRoot "_buildname.txt"
if (Test-Path $buildNamePath) {
    $DevName = (Get-Content $buildNamePath -First 1).Trim().Trim('"').Trim("'")
}
else {
    $DevName = ""
}
Write-Host "DevName: '$DevName'" -ForegroundColor Green

# ----------------------------------------------------------
# Step 3: Detect toolset and build
# ----------------------------------------------------------
Write-Step 3 "Detecting Visual Studio Toolset"
$Toolset = Get-PlatformToolset

$BinX64 = Join-Path $RepoRoot "Bin\Release_x64_$Toolset"
$BinX86 = Join-Path $RepoRoot "Bin\Release_x86_$Toolset"

Write-Step 4 "Building x64 and Win32 Release"
if ($SkipBuild) {
    Write-Host "Skipped (using existing binaries)" -ForegroundColor Yellow
    if (-not (Test-Path $BinX64)) { throw "x64 build output not found: $BinX64" }
    if (-not (Test-Path $BinX86)) { throw "x86 build output not found: $BinX86" }
}
else {
    $buildScript = Join-Path $ScriptDir "Build.ps1"

    Write-Host "Building x64 Release..." -ForegroundColor Yellow
    & $buildScript -Platform x64 -Configuration Release
    if ($LASTEXITCODE -ne 0) {
        throw "x64 build failed with exit code $LASTEXITCODE"
    }

    Write-Host ""
    Write-Host "Building Win32 Release..." -ForegroundColor Yellow
    & $buildScript -Platform Win32 -Configuration Release
    if ($LASTEXITCODE -ne 0) {
        throw "Win32 build failed with exit code $LASTEXITCODE"
    }
}

# ----------------------------------------------------------
# Step 5: Copy binaries to portable app structure
# ----------------------------------------------------------
Write-Step 5 "Copying Binaries to Portable App"

$targetX64 = Join-Path $Notepad3AppDir "x64"
$targetX86 = Join-Path $Notepad3AppDir "x86"

Write-Host "x64: $BinX64 -> $targetX64" -ForegroundColor Yellow
Copy-PlatformBinaries -SourceDir $BinX64 -TargetDir $targetX64

Write-Host "x86: $BinX86 -> $targetX86" -ForegroundColor Yellow
Copy-PlatformBinaries -SourceDir $BinX86 -TargetDir $targetX86

# ----------------------------------------------------------
# Step 6: Process appinfo_template.ini
# ----------------------------------------------------------
Write-Step 6 "Generating appinfo.ini from Template"

$templatePath = Join-Path $AppInfoDir "appinfo_template.ini"
$outputPath = Join-Path $AppInfoDir "appinfo.ini"

if (-not (Test-Path $templatePath)) {
    throw "Template not found: $templatePath"
}

$content = Get-Content $templatePath -Raw
$content = $content -replace 'xxxVERSIONxxx', $Version
$content = $content -replace 'xxxDEVNAMExxx', $DevName
Set-Content -Path $outputPath -Value $content -NoNewline

Write-Host "Generated: $outputPath" -ForegroundColor Green
Write-Host "  PackageVersion = $Version" -ForegroundColor Gray
Write-Host "  DisplayVersion = $Version$DevName" -ForegroundColor Gray
Write-Host "  Name           = Notepad3Portable$DevName" -ForegroundColor Gray

# ----------------------------------------------------------
# Step 7: Generate PortableApps Launcher
# ----------------------------------------------------------
Write-Step 7 "Generating Notepad3Portable.exe Launcher"

if ($SkipLauncherGenerator) {
    Write-Host "Skipped (using existing Notepad3Portable.exe)" -ForegroundColor Yellow
}
else {
    $LauncherGeneratorPath = Join-Path $PortableAppsDir "PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe"
    if (-not (Test-Path $LauncherGeneratorPath)) {
        throw "PortableApps.comLauncherGenerator.exe not found: $LauncherGeneratorPath"
    }

    Write-Host "Generator: $LauncherGeneratorPath" -ForegroundColor Cyan
    Write-Host "App Dir:   $PortableAppDir" -ForegroundColor Cyan

    # Passing the directory as a command-line argument triggers automatic compile mode.
    $process = Start-Process -FilePath $LauncherGeneratorPath -ArgumentList "`"$PortableAppDir`"" `
        -Wait -PassThru
    if ($process.ExitCode -ne 0) {
        throw "PortableApps.comLauncherGenerator failed with exit code $($process.ExitCode)"
    }

    $launcherExe = Join-Path $PortableAppDir "Notepad3Portable.exe"
    if (-not (Test-Path $launcherExe)) {
        throw "Launcher was not generated: $launcherExe"
    }
    Write-Host "Generated: $launcherExe" -ForegroundColor Green
}

# ----------------------------------------------------------
# Step 8: Build PortableApps package
# ----------------------------------------------------------
Write-Step 8 "Building PortableApps Package"

if (-not (Test-Path $InstallerPath)) {
    throw "PortableApps.comInstaller.exe not found: $InstallerPath"
}

Write-Host "Installer: $InstallerPath" -ForegroundColor Cyan
Write-Host "App Dir:   $PortableAppDir" -ForegroundColor Cyan

# Passing the directory as a command-line argument triggers automatic compile mode:
# welcome and options pages are skipped, finish page auto-closes on success.
$process = Start-Process -FilePath $InstallerPath -ArgumentList "`"$PortableAppDir`"" `
    -Wait -PassThru
if ($process.ExitCode -ne 0) {
    throw "PortableApps.comInstaller failed with exit code $($process.ExitCode)"
}

# Find the generated .paf.exe
$pafPattern = "Notepad3Portable_*.paf.exe"
$pafFiles = Get-ChildItem $PortableAppRoot -Filter $pafPattern | Sort-Object LastWriteTime -Descending
if ($pafFiles.Count -eq 0) {
    throw "No .paf.exe output found matching $pafPattern in $PortableAppRoot"
}
$pafFile = $pafFiles[0]
$pafSize = [math]::Round($pafFile.Length / 1MB, 2)

# ----------------------------------------------------------
# Done
# ----------------------------------------------------------
Write-Host ""
Write-Host "=== PortableApps Package Built Successfully! ===" -ForegroundColor Green
Write-Host "  Output: $($pafFile.FullName)" -ForegroundColor Green
Write-Host "  Size:   $pafSize MB" -ForegroundColor Green
Write-Host "  Time:   $($stopwatch.Elapsed)" -ForegroundColor Cyan
exit 0
