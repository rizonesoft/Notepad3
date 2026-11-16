param(
    [string]$Notepad3Source
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if (!$Notepad3Source) { $Notepad3Source = Join-Path $root 'src\Notepad3.c' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-webtemplates-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content $Notepad3Source -Raw
    $helpers = foreach ($signature in @('static bool _IsFileWebTemplate(', 'static bool _CanRunWebTemplate(', 'static void _UpdateWebTemplateMenu(', 'static HRESULT _GetDefaultBrowserClass(', 'static bool _PrepareWebTemplate(')) {
        $start = $source.IndexOf($signature, [StringComparison]::Ordinal)
        if ($start -lt 0) { continue }
        $body = $source.IndexOf('{', $start)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing end of $signature" }
        $source.Substring($start, $body + $end.Index + 1 - $start)
    }
    Set-Content (Join-Path $temp 'WebTemplateHelpers.inc') ($helpers -join "`n") -Encoding utf8

    $start = $source.IndexOf('    case CMD_WEBACTION1:', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing WebTemplate command cases' }
    $commands = $source.Substring($start)
    $end = [regex]::Match($commands, '(?m)^    break;')
    if (!$end.Success) { throw 'Missing WebTemplate command end' }
    Set-Content (Join-Path $temp 'WebTemplateCommands.inc') $commands.Substring(0, $end.Index + $end.Length) -Encoding utf8
    $menu = [regex]::Matches($source, '(?m)^    _UpdateWebTemplateMenu\(hmenu,[^\r\n]*')
    if ($menu.Count -ne 1) { throw 'Missing shared WebTemplate menu update' }
    Set-Content (Join-Path $temp 'WebTemplateMenu.inc') ($menu.Value -join "`n") -Encoding utf8
    $config = Get-Content (Join-Path $root 'src\Config\Config.cpp') -Raw
    $start = $config.IndexOf('    IniSectionGetStringNoQuotes(IniSecSettings2, L"WebTemplate1"', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing WebTemplate settings block' }
    $end = $config.IndexOf('    Settings2.LexerSQLNumberSignAsComment', $start, [StringComparison]::Ordinal)
    if ($end -lt 0) { throw 'Missing WebTemplate settings block end' }
    Set-Content (Join-Path $temp 'WebTemplateSettings.inc') $config.Substring($start, $end - $start) -Encoding utf8
    $resource = Get-Content (Join-Path $root 'src\Notepad3.rc') -Raw
    $accelerators = [regex]::Match($resource, '(?ms)^IDR_MAINWND ACCELERATORS\r?\nBEGIN.*?^END')
    if (!$accelerators.Success) { throw 'Missing main accelerator table' }
    $menuResource = Join-Path $root 'language\np3_en_us\menu_en_us.rc'
    Set-Content (Join-Path $temp 'WebTemplateResources.rc') @(
        '#include "resource.h"'
        "#include `"$($menuResource.Replace('\', '\\'))`""
        $accelerators.Value
    ) -Encoding utf8

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    $test = Join-Path $PSScriptRoot 'TestWebTemplates.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && rc /nologo /I`"$root\src`" /fo WebTemplateResources.res WebTemplateResources.rc && cl /nologo /c /utf-8 /W4 /WX /DUNICODE /D_UNICODE /I`"$root\scintilla\include`" `"$root\src\DynStrg.c`" /Fo:DynStrg.obj && cl /nologo /EHsc /std:c++20 /utf-8 /W4 /WX /DUNICODE /D_UNICODE /I`"$temp`" /I`"$root\scintilla\include`" `"$test`" DynStrg.obj WebTemplateResources.res /Fe:TestWebTemplates.exe /link shlwapi.lib user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'WebTemplate test build failed' }
    & (Join-Path $temp 'TestWebTemplates.exe')
    if ($LASTEXITCODE -ne 0) { throw 'WebTemplate tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
