param([string]$FixtureDirectory = (Join-Path $PSScriptRoot 'UTF-16'))

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-encoding-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content (Join-Path $root 'src\uchardet\EncodingDetection.cpp') -Raw
    $header = Get-Content (Join-Path $root 'src\Encoding.h') -Raw
    $utf8 = Get-Content (Join-Path $root 'src\Encoding.c') -Raw
    function Get-Function([string]$text, [string]$signature) {
        $start = $text.IndexOf($signature, [StringComparison]::Ordinal)
        if ($start -lt 0) { throw "Missing function: $signature" }
        # These top-level functions end with an unindented brace. Counting braces
        # would also count character literals such as the HZ marker '{'.
        $body = $text.IndexOf('{', $start)
        $end = [regex]::Match($text.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing function end: $signature" }
        return $text.Substring($start, $body + $end.Index + 1 - $start)
    }
    $start = $header.IndexOf('#define NCP_DEFAULT')
    $end = $header.IndexOf('typedef struct _np2encoding')
    $types = $header.Substring($start, $end - $start)
    $start = $header.IndexOf('typedef struct _enc_det_t')
    $end = $header.IndexOf('ENC_DET_T Encoding_DetectEncoding', $start)
    $types += $header.Substring($start, $end - $start)
    Set-Content (Join-Path $temp 'EncodingDetectionTypes.inc') $types -Encoding ascii

    $functions = @(
        (Get-Function $utf8 'bool IsValidUTF8(')
        (Get-Function $source 'static bool IsValidUTF16(')
        (Get-Function $source 'static cpi_enc_t DetectUTF16ByNullDistribution(')
        (Get-Function $source 'static cpi_enc_t DetectUTF16ByWindows(')
        (Get-Function $source 'static bool DetectUTF32ByRange(')
        (Get-Function $source '__forceinline static cpi_enc_t _SetAnsiDefaultEncoding(')
        (Get-Function $source 'static void _SetResultingEncoding(')
        (Get-Function $source 'static char* _BuildAnalysisSample(')
        (Get-Function $source 'static bool _MayBeStateful7Bit(')
        (Get-Function $source 'void Encoding_AnalyzeText(')
        (Get-Function $source 'static const WCHAR* _EncodingDebugName(')
        (Get-Function $source 'static void _SetEncodingTitleInfo(const ENC_DET_T* result)')
        (Get-Function $source 'extern "C" ENC_DET_T Encoding_DetectEncoding(')
    )
    Set-Content (Join-Path $temp 'EncodingDetectionFunctions.inc') ($functions -join "`n") -Encoding ascii
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    $test = Join-Path $PSScriptRoot 'TestEncodingDetection.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /EHsc /std:c++20 /W4 /WX /I`"$temp`" `"$test`" /Fe:TestEncodingDetection.exe /link advapi32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Encoding detection test build failed' }
    & (Join-Path $temp 'TestEncodingDetection.exe') $FixtureDirectory
    if ($LASTEXITCODE -ne 0) { throw 'Encoding detection tests failed' }
} finally {
    # Only this invocation's generated files are removed.
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
