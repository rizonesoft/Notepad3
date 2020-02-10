# ------------------------------------------------------------
# PowerShell script to perform Version patching for Notepad3 
# ToDo:
# - adapt $Build number in case of local machine builds
# ------------------------------------------------------------
param(
	[switch]$AppVeyorEnv = $false,
	[string]$VerPatch = ""
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
  $AppName = "Notepad3"
	$Major = 5
	$Minor = [int]$(Get-Date -format yy)
	$Revis = [int]$(Get-Date -format MMdd)
	if ($AppVeyorEnv) {
		$CommitID = ([string]($env:appveyor_repo_commit)).substring(0,7)
		$Build = [int]($env:appveyor_build_number)
	}
	else {
		$CommitID = [string]($env:computername)
		$Build = [int](Get-Content "Versions\build.txt") + 1
	}
	if (!$CommitID) { $CommitID = "---" }
	if (!$Build) { $Build = 0 }
	$SciVer = [string](Get-Content "scintilla\version.txt")
	if (!$SciVer) { $SciVer = 0 }
	$OnigVer = [string](Get-Content "oniguruma\version.txt")
	if (!$OnigVer) { $OnigVer = "0.0.0" }
	$UChardetVer = [string](Get-Content "uchardet\version.txt")
	if (!$UChardetVer) { $UChardetVer = "0.0.0" }
	$TinyExprVer = [string](Get-Content "tinyexpr\version.txt")
	if (!$TinyExprVer) { $TinyExprVer = "0.0.0" }
	$UtHashVer = [string](Get-Content "uthash\version.txt")
	if (!$UtHashVer) { $UtHashVer = "0.0.0" }
	
	$CompleteVer = "$Major.$Minor.$Revis.$Build"
	DebugOutput("Version number: 'v$CompleteVer $VerPatch'")

#~if ($VerPatch) { $VerPatch = " $VerPatch" }  # ensure space in front of string

	Copy-Item -LiteralPath "Versions\VersionEx.h.tpl" -Destination "src\VersionEx.h" -Force
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$APPNAME\$', "$AppName" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MAJOR\$', "$Major" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MINOR\$', "$Minor" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MAINT\$', "$Revis" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$BUILD\$', "$Build" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$SCIVER\$', "$SciVer" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$ONIGURUMAVER\$', "$OnigVer" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$UCHARDETVER\$', "$UChardetVer" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$TINYEXPRVER\$', "$TinyExprVer" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$UTHASHVER\$', "$UtHashVer" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$VERPATCH\$', "$VerPatch" } | Set-Content "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$COMMITID\$', "$CommitID" } | Set-Content "src\VersionEx.h"
	
	Copy-Item -LiteralPath "Versions\Notepad3.exe.manifest.tpl" -Destination "res\Notepad3.exe.manifest.conf" -Force
	(Get-Content "res\Notepad3.exe.manifest.conf") | ForEach-Object { $_ -replace '\$APPNAME\$', "$AppName" } | Set-Content "res\Notepad3.exe.manifest.conf"
	(Get-Content "res\Notepad3.exe.manifest.conf") | ForEach-Object { $_ -replace '\$VERPATCH\$', "$VerPatch" } | Set-Content "res\Notepad3.exe.manifest.conf"
	(Get-Content "res\Notepad3.exe.manifest.conf") | ForEach-Object { $_ -replace '\$VERSION\$', "$CompleteVer" } | Set-Content "res\Notepad3.exe.manifest.conf"

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

