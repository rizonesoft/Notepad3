$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-lexer-sorting-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content (Join-Path $root 'src\Styles.c') -Raw
    $functions = foreach ($name in @('Style_GetLexerDisplayName', '_GetLexerIndex', '_CompareLexersByDisplayName', '_FindLexerTreeItem', '_GetListViewLexer')) {
        $start = [regex]::Match($source, "(?m)^(?:static )?[^\r\n]+ \b$name\(")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    Set-Content (Join-Path $temp 'LexerDialogSorting.inc') ($functions -join "`n") -Encoding ascii
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    $test = Join-Path $PSScriptRoot 'TestLexerDialogSorting.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /EHsc /std:c++20 /W4 /WX /DUNICODE /D_UNICODE /I`"$temp`" `"$test`" /Fe:TestLexerDialogSorting.exe /link comctl32.lib shlwapi.lib user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Lexer-sorting test build failed' }
    & (Join-Path $temp 'TestLexerDialogSorting.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Lexer-sorting tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
