$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-zoom-position-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content (Join-Path $root 'src\Notepad3.c') -Raw
    $types = Get-Content (Join-Path $root 'src\TypeDefs.h') -Raw
    $enum = [regex]::Match($types, '(?ms)^typedef enum ZOOM_TOOLTIP_POSITION \{.*?^\} ZOOM_TOOLTIP_POSITION;')
    $cycle = [regex]::Match($source, '(?m)^#define _MQ_TIMER_CYCLE [^\r\n]+')
    if (!$enum.Success -or !$cycle.Success) { throw 'Missing zoom position enum or timer cycle' }
    Set-Content (Join-Path $temp 'ZoomTooltipPositionTypes.inc') ($enum.Value + "`n" + $cycle.Value) -Encoding ascii
    $functions = foreach ($name in @('_GetZoomCallTipViewport', '_GetZoomCallTipCaret', '_FindZoomCallTip', '_PositionZoomCallTip', 'ShowZoomCallTip')) {
        $start = [regex]::Match($source, "(?m)^(?:static )?\w+(?: CALLBACK)? $name\([^\r\n;]*\)\r?$")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $config = Get-Content (Join-Path $root 'src\Config\Config.cpp') -Raw
    $load = [regex]::Match($config, '(?ms)^    Settings2\.ZoomTooltipPosition =.*?^    \}')
    if (!$load.Success) { throw 'Missing zoom position settings load' }
    $functions += "static void LoadZoomTooltipPosition()`n{`n" + $load.Value + "`n}"
    Set-Content (Join-Path $temp 'ZoomTooltipPosition.inc') ($functions -join "`n") -Encoding ascii
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    $test = Join-Path $PSScriptRoot 'TestZoomTooltipPosition.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /EHsc /std:c++20 /W4 /WX /DUNICODE /D_UNICODE /DNOMINMAX /I`"$temp`" `"$test`" /Fe:TestZoomTooltipPosition.exe /link user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Zoom-position test build failed' }
    & (Join-Path $temp 'TestZoomTooltipPosition.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Zoom-position tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
