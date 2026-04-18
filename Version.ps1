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
	$Major = 7
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
	$PCRE2Ver = [string](Get-Content "scintilla\pcre2\version.txt")
	if (!$PCRE2Ver) { $PCRE2Ver = "0.0" }
	$UChardetVer = [string](Get-Content "src\uchardet\version.txt")
	if (!$UChardetVer) { $UChardetVer = "0.0.0" }
	$TinyExprVer = [string](Get-Content "src\tinyexprcpp\version.txt")
	if (!$TinyExprVer) { $TinyExprVer = "0.0.0" }
	$UtHashVer = [string](Get-Content "src\uthash\version.txt")
	if (!$UtHashVer) { $UtHashVer = "0.0.0" }

#~if ($VerPatch) { $VerPatch = " $VerPatch" }  # ensure space in front of string

	$VersionContent = Get-Content "Versions\VersionEx.h.tpl" -Raw
	$VersionContent = $VersionContent -replace '\$APPNAME\$', "$AppName"
	$VersionContent = $VersionContent -replace '\$MAJOR\$', "$Major"
	$VersionContent = $VersionContent -replace '\$MINOR\$', "$Minor"
	$VersionContent = $VersionContent -replace '\$MAINT\$', "$Revis"
	$VersionContent = $VersionContent -replace '\$BUILD\$', "$Build"
	$VersionContent = $VersionContent -replace '\$SCIVER\$', "$SciVer"
	$VersionContent = $VersionContent -replace '\$LXIVER\$', "$LxiVer"
	$VersionContent = $VersionContent -replace '\$PCRE2VER\$', "$PCRE2Ver"
	$VersionContent = $VersionContent -replace '\$UCHARDETVER\$', "$UChardetVer"
	$VersionContent = $VersionContent -replace '\$TINYEXPRVER\$', "$TinyExprVer"
	$VersionContent = $VersionContent -replace '\$UTHASHVER\$', "$UtHashVer"
	$VersionContent = $VersionContent -replace '\$VERPATCH\$', "$VerPatch"
	$VersionContent = $VersionContent -replace '\$COMMITID\$', "$CommitID"
	Set-Content -Path "src\VersionEx.h" -Value $VersionContent -NoNewline

	$ConfManifest = "res\Notepad3.exe.conf.manifest"
	$ManifestContent = Get-Content "Versions\Notepad3.exe.manifest.tpl" -Raw
	$ManifestContent = $ManifestContent -replace '\$APPNAME\$', "$AppName"
	$ManifestContent = $ManifestContent -replace '\$VERPATCH\$', "$VerPatch"
	$ManifestContent = $ManifestContent -replace '\$VERSION\$', "$CompleteVer"
	Set-Content -Path $ConfManifest -Value $ManifestContent -NoNewline
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

