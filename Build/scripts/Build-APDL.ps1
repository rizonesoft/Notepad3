# Build and package the APDL-enabled Windows x64 editor with the installed MSVC toolset.
param([int]$BuildNumber = 1)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if (-not $IsWindows) { throw 'This build requires Windows and Visual Studio C++ tools.' }
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
Push-Location $repo
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' |
        Select-Object -First 1
    if (-not $msbuild) { throw 'Visual Studio MSBuild was not found.' }
    $nuget = (Get-Command nuget.exe -ErrorAction Stop).Source
    & $nuget restore Notepad3.sln -NonInteractive
    if ($LASTEXITCODE) { throw "NuGet restore failed: $LASTEXITCODE" }

    $now = Get-Date
    $version = "7.$($now.ToString('yy')).$($now.ToString('Mdd')).$BuildNumber"
    $commit = (& git rev-parse HEAD).Trim()
    if ($LASTEXITCODE) { throw 'Could not identify the source commit.' }
    $substitutions = @{
        '$APPNAME$' = 'Notepad3'; '$MAJOR$' = '7'; '$MINOR$' = $now.ToString('yy')
        '$MAINT$' = $now.ToString('Mdd'); '$BUILD$' = "$BuildNumber"
        '$SCIVER$' = (Get-Content scintilla/version.txt -Raw).Trim()
        '$LXIVER$' = (Get-Content lexilla/version.txt -Raw).Trim()
        '$PCRE2VER$' = (Get-Content scintilla/pcre2/version.txt -Raw).Trim()
        '$UCHARDETVER$' = (Get-Content src/uchardet/version.txt -Raw).Trim()
        '$TINYEXPRVER$' = (Get-Content src/tinyexprcpp/version.txt -Raw).Trim()
        '$UTHASHVER$' = (Get-Content src/uthash/version.txt -Raw).Trim()
        '$VERPATCH$' = ''; '$COMMITID$' = $commit.Substring(0, 8); '$VERSION$' = $version
    }
    foreach ($pair in @(
        @('Versions/VersionEx.h.tpl', 'src/VersionEx.h'),
        @('Versions/Notepad3.exe.manifest.tpl', 'res/Notepad3.exe.conf.manifest')
    )) {
        $content = Get-Content $pair[0] -Raw
        foreach ($key in $substitutions.Keys) { $content = $content.Replace($key, $substitutions[$key]) }
        if ($content -match '\$[A-Z0-9_]+\$') { throw "Unresolved template placeholder in $($pair[0])" }
        Set-Content $pair[1] -Value $content -Encoding utf8 -NoNewline
    }
    $env:GITHUB_ACTIONS_BUILD = 'true'
    $started = [DateTime]::UtcNow
    & $msbuild Notepad3.sln /m:2 /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
    if ($LASTEXITCODE) { throw "MSBuild failed: $LASTEXITCODE" }

    $outputs = @(Get-ChildItem Bin -Directory -Filter 'Release_x64_*' |
        Where-Object { Test-Path (Join-Path $_.FullName 'Notepad3.exe') })
    if ($outputs.Count -ne 1) { throw 'Expected exactly one x64 Release output directory.' }
    $exe = Get-Item (Join-Path $outputs[0].FullName 'Notepad3.exe')
    if ($exe.LastWriteTimeUtc -lt $started.AddSeconds(-2)) { throw 'The executable is stale.' }
    if ($exe.VersionInfo.FileVersionRaw.ToString() -ne $version) { throw 'Executable version mismatch.' }
    $bytes = [IO.File]::ReadAllBytes($exe.FullName)
    $pe = [BitConverter]::ToInt32($bytes, 60)
    if ($bytes[0] -ne 0x4d -or $bytes[1] -ne 0x5a -or
        [BitConverter]::ToUInt32($bytes, $pe) -ne 0x4550 -or
        [BitConverter]::ToUInt16($bytes, $pe + 4) -ne 0x8664) { throw 'Expected an AMD64 PE executable.' }

    $dist = Join-Path $repo 'Bin/APDL-artifacts'
    New-Item $dist -ItemType Directory -Force | Out-Null
    $stage = Join-Path $dist "Notepad3-APDL-$version-x64"
    if (Test-Path $stage) { throw "Package staging directory already exists: $stage" }
    New-Item $stage -ItemType Directory | Out-Null
    Get-ChildItem $outputs[0].FullName -Recurse -File |
        Where-Object { $_.Extension -in @('.exe', '.dll', '.mui') -and $_.FullName -notmatch '[\\/]obj[\\/]' } |
        ForEach-Object {
            $destination = Join-Path $stage ([IO.Path]::GetRelativePath($outputs[0].FullName, $_.FullName))
            New-Item (Split-Path $destination) -ItemType Directory -Force | Out-Null
            Copy-Item $_.FullName $destination
        }
    Copy-Item Build/Notepad3.ini $stage
    Copy-Item Build/minipath.ini $stage
    Copy-Item Build/Docs/License.txt $stage
    Copy-Item Build/Themes $stage -Recurse
    Copy-Item test/test_files/StyleLexers/styleLexAPDL (Join-Path $stage 'APDL-examples') -Recurse
    Get-ChildItem lexilla/License.txt, scintilla/License.txt | ForEach-Object {
        Copy-Item $_.FullName (Join-Path $stage ($_.Directory.Name + '-License.txt'))
    }
    $metadata = [ordered]@{
        version = $version; architecture = 'x64'; commit = $commit
        built_at_utc = [DateTime]::UtcNow.ToString('o'); toolset_output = $outputs[0].Name
        signed = $false
    }
    $metadata | ConvertTo-Json | Set-Content (Join-Path $stage 'build-info.json') -Encoding utf8
    $zip = Join-Path $dist "Notepad3-APDL-$version-x64.zip"
    Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $zip
    Copy-Item $exe.FullName (Join-Path $dist 'Notepad3.exe')
    $metadata | ConvertTo-Json | Set-Content (Join-Path $dist 'build-info.json') -Encoding utf8
    @($zip, (Join-Path $dist 'Notepad3.exe')) | ForEach-Object {
        $hash = Get-FileHash $_ -Algorithm SHA256
        "$($hash.Hash.ToLowerInvariant())  $([IO.Path]::GetFileName($_))"
    } | Set-Content (Join-Path $dist 'SHA256SUMS.txt') -Encoding ascii
    Write-Host "Built and packaged $zip"
} finally { Pop-Location }
