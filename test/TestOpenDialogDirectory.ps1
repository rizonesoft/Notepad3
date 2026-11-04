$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-open-directory-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content (Join-Path $root 'src\Dialogs.c') -Raw
    $functions = foreach ($name in @('_GetParentDir', '_CanonicalizeInitialDir', '_ResolveOpenInitialDir')) {
        $start = $source.IndexOf("static void $name(", [StringComparison]::Ordinal)
        if ($start -lt 0) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start, $body + $end.Index + 1 - $start)
    }
    Set-Content (Join-Path $temp 'OpenDialogDirectory.inc') ($functions -join "`n") -Encoding ascii
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    $test = Join-Path $PSScriptRoot 'TestOpenDialogDirectory.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /EHsc /std:c++20 /W4 /WX /I`"$temp`" `"$test`" /Fe:TestOpenDialogDirectory.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Open-directory test build failed' }
    & (Join-Path $temp 'TestOpenDialogDirectory.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Open-directory tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
