# ------------------------------------------------------------
# PowerShell script to perform Version patching for Notepad3 
# ToDo:
# - adapt $Build number in case of local machine builds
# ------------------------------------------------------------
param(
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
		Write-Host ""
		Write-Host "$msg"
	}
}
# ------------------------------------------------------------

# ============================================================

try 
{
	$AppName = "Notepad3"
	$Major = 6
	$Minor = [int]$(Get-Date -format yy)
	$Revis = [int]$(Get-Date -format Mdd)
	
	$BuildPath = "Versions\build.txt"
	if (!(Test-Path $BuildPath)) {
		New-Item -Path $BuildPath -ItemType "file" -Value "101"
	}
	$Build = [int](Get-Content $BuildPath)
	if (!$Build) { $Build = 0 }
	
	$DayPath = "Versions\day.txt"
	if (!(Test-Path $DayPath)) {
		New-Item -Path $DayPath -ItemType "file" -Value "0"
	}
	$LastBuildDay = [string](Get-Content $DayPath)
	if (!$LastBuildDay) { $LastBuildDay = 0 }

	if ($LastBuildDay -ne "$Revis") {
		$Revis | Set-Content -Path $DayPath
		$Build = 0  # reset (local build)
	}
	# locally: increase build number and persist it
	$Build = $Build + 1
	# locally: read commit ID from .git\refs\heads\<first file>
	$HeadDir = ".git\refs\heads"
	$HeadMaster = Get-ChildItem -Path $HeadDir -Force -Recurse -File | Select-Object -First 1
	$CommitID = [string](Get-Content "$HeadDir\$HeadMaster" -TotalCount 8)
	if (!$CommitID) {
		$length = ([string]($env:computername)).length
		$CommitID = ([string]($env:computername)).substring(0,[math]::min($length,8)).ToLower()
	}
	$CommitID = $CommitID -replace '"', ''
	$CommitID = $CommitID -replace "'", ''
	$length = $CommitID.length
	$CommitID = $CommitID.substring(0,[math]::min($length,8))
	if (!$CommitID) { $CommitID = "---" }
	$Build | Set-Content -Path $BuildPath

	$CompleteVer = "$Major.$Minor.$Revis.$Build"
	DebugOutput("Notepad3 version number: 'v$CompleteVer $VerPatch'")
	DebugOutput("Notepad3 commit ID: '$CommitID'")
	
	$SciVer = [string](Get-Content "scintilla\version.txt")
	if (!$SciVer) { $SciVer = 0 }
	$LxiVer = [string](Get-Content "lexilla\version.txt")
	if (!$LxiVer) { $LxiVer = 0 }
	$OnigVer = [string](Get-Content "scintilla\oniguruma\version.txt")
	if (!$OnigVer) { $OnigVer = "0.0.0" }
	$UChardetVer = [string](Get-Content "src\uchardet\version.txt")
	if (!$UChardetVer) { $UChardetVer = "0.0.0" }
	$TinyExprVer = [string](Get-Content "src\tinyexpr\version.txt")
	if (!$TinyExprVer) { $TinyExprVer = "0.0.0" }
	$UtHashVer = [string](Get-Content "src\uthash\version.txt")
	if (!$UtHashVer) { $UtHashVer = "0.0.0" }

#~if ($VerPatch) { $VerPatch = " $VerPatch" }  # ensure space in front of string

	Copy-Item -LiteralPath "Versions\VersionEx.h.tpl" -Destination "src\VersionEx.h" -Force
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$APPNAME\$', "$AppName" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MAJOR\$', "$Major" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MINOR\$', "$Minor" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$MAINT\$', "$Revis" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$BUILD\$', "$Build" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$SCIVER\$', "$SciVer" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$LXIVER\$', "$LxiVer" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$ONIGURUMAVER\$', "$OnigVer" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$UCHARDETVER\$', "$UChardetVer" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$TINYEXPRVER\$', "$TinyExprVer" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$UTHASHVER\$', "$UtHashVer" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$VERPATCH\$', "$VerPatch" } | Set-Content -Path "src\VersionEx.h"
	(Get-Content "src\VersionEx.h") | ForEach-Object { $_ -replace '\$COMMITID\$', "$CommitID" } | Set-Content -Path "src\VersionEx.h"
	
	$ConfManifest = "res\Notepad3.exe.conf.manifest"
	Copy-Item -LiteralPath "Versions\Notepad3.exe.manifest.tpl" -Destination $ConfManifest -Force
	(Get-Content $ConfManifest) | ForEach-Object { $_ -replace '\$APPNAME\$', "$AppName" } | Set-Content -Path $ConfManifest
	(Get-Content $ConfManifest) | ForEach-Object { $_ -replace '\$VERPATCH\$', "$VerPatch" } | Set-Content -Path $ConfManifest
	(Get-Content $ConfManifest) | ForEach-Object { $_ -replace '\$VERSION\$', "$CompleteVer" } | Set-Content -Path $ConfManifest
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
	[Environment]::SetEnvironmentVariable("LASTEXITCODE", $LastExitCode, "User")
	$host.SetShouldExit($LastExitCode)
	Write-Host ""
	Write-Host "VersionPatching: Done! Elapsed time: $($stopwatch.Elapsed)."
	Exit $LastExitCode
}

