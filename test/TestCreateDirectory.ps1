param(
    # Alternate PathLib.c, e.g. a pre-fix copy, to show the regression cases failing.
    [string]$PathLibSource
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if (!$PathLibSource) { $PathLibSource = Join-Path $root 'src\PathLib.c' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-create-directory-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)

function Get-Function([string]$path, [string]$signature) {
    $source = Get-Content $path -Raw
    $start = $source.IndexOf($signature, [StringComparison]::Ordinal)
    if ($start -lt 0) { throw "Missing $signature in $path" }
    $body = $source.IndexOf('{', $start)
    $end = [regex]::Match($source.Substring($body), '(?m)^}')
    if (!$end.Success) { throw "Missing end of $signature" }
    $source.Substring($start, $body + $end.Index + 1 - $start)
}

try {
    Set-Content (Join-Path $temp 'PathLibCreateDirectory.inc') (Get-Function $PathLibSource 'HRESULT PTHAPI Path_CreateDirectoryEx(') -Encoding ascii
    Set-Content (Join-Path $temp 'CreateIniFile.inc') (Get-Function (Join-Path $root 'src\Config\Config.cpp') 'extern "C" bool CreateIniFile(') -Encoding ascii
    $notepad3 = Join-Path $root 'src\Notepad3.c'
    Set-Content (Join-Path $temp 'BuildSnapshotPath.inc') (Get-Function $notepad3 'static HPATHL _BuildSnapshotPath(') -Encoding ascii
    $recreate = [regex]::Match((Get-Content $notepad3 -Raw), 'HRESULT const hr = Path_CreateDirectoryEx\(hdir_path\);\s+bDirCreated = [^;]+;')
    if (!$recreate.Success) { throw 'Missing directory-recreation caller in Notepad3.c' }
    Set-Content (Join-Path $temp 'RecreateDirectory.inc') $recreate.Value -Encoding ascii

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    $test = Join-Path $PSScriptRoot 'TestCreateDirectory.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /EHsc /std:c++20 /W4 /WX /I`"$temp`" `"$test`" /Fe:TestCreateDirectory.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Create-directory test build failed' }
    & (Join-Path $temp 'TestCreateDirectory.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Create-directory tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
