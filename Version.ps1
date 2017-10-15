# ------------------------------------------------------------
# PowerShell script to perform Version patching for Notepad3 
# ToDo:
# - adapt $Build number in case of local machine builds
# ------------------------------------------------------------
param(
	[switch]$AppVeyorEnv = $false
)
# ------------------------------------------------------------
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"  ## Write-Error should throw ...
# ------------------------------------------------------------
$ScriptDir = Split-Path -Path "$(if ($PSVersionTable.PSVersion.Major -ge 3) { $PSCommandPath } else { & { $MyInvocation.ScriptName } })" -Parent
# ------------------------------------------------------------
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$LastExitCode = 0
# ------------------------------------------------------------

# ============================================================
function DebugOutput($msg)
# ------------------------------------------------------------
{
	#~return ## disabled debug output
	if ($msg -ne $null) { 
		Write-Host "$msg"
	}
}
# ------------------------------------------------------------

# ============================================================

try 
{
	$Major = 2
	$Minor = [int]$(Get-Date -format yy)
	$Revis = [int]$(Get-Date -format Mdd)
	if ($AppVeyorEnv) {
		$Build = [int]($env:appveyor_build_number)
	}
	else {
		$Build = [int](Get-Content "Versions\build.txt") + 1
	}
	if (!$Build) { $Build = 0 }
	$SciVer = [int](Get-Content "scintilla\version.txt")
	if (!$SciVer) { $SciVer = 0 }
	
	$CompleteVer = "$Major.$Minor.$Revis.$Build"
	DebugOutput("Version number: '$CompleteVer'")

	Copy-Item -LiteralPath "Versions\VersionEx.h.tpl" -Destination "src\VersionEx.h" -Force
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MAJOR\$', "$Major" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MINOR\$', "$Minor" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MAINT\$', "$Revis" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$BUILD\$', "$Build" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$SCIVER\$', "$SciVer" } | Set-Content "src\VersionEx.h"
	Copy-Item -LiteralPath "Versions\Notepad3.exe.manifest.tpl" -Destination "res\Notepad3.exe.manifest.conf" -Force
	(Get-Content "res\Notepad3.exe.manifest.conf") | ForEach-Object { $_ -replace '\$VERSION\$', $CompleteVer } | Set-Content "res\Notepad3.exe.manifest.conf"
	if ($AppVeyorEnv) {
		Update-AppveyorBuild -Version $CompleteVer
  }
}
catch 
{
	if ($LastExitCode -eq 0) { $LastExitCode = 99 }
	$errorMessage = $_.Exception.Message
	Write-Warning "Exception caught: `"$errorMessage`"!"
	throw $_
}
finally
{
	$Build | Set-Content "Versions\build.txt"
	[Environment]::SetEnvironmentVariable("LASTEXITCODE", $LastExitCode, "User")
	$host.SetShouldExit($LastExitCode)
	Write-Host "VersionPatching: Done! Elapsed time: $($stopwatch.Elapsed)."
	Exit $LastExitCode
}

