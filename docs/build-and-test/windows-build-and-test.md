# Windows Build and Test

## Build

- `Build\Build_x64.cmd [Release|Debug]` builds one platform; variants are `_Win32`, `_ARM64`, and `_x64_AVX2`.
- `Build\BuildAll.cmd` builds all four supported platforms. `Build\Clean.cmd` cleans outputs.
- Run `nuget restore Notepad3.sln` once before the first build.
- Run `Version.ps1` before building to regenerate `src\VersionEx.h` (`Major.YY.Mdd.Build`, with the build number in `Versions\build.txt`).
- Default configuration is Release. CI is in `.github\workflows\build.yml` (`windows-2025-vs2026`, Release × all four platforms).
- Verify builds using the default target on the full solution. `/t:Notepad3` fails with MSB4057 because not all subprojects define that target. Check that `Bin\Release_x64_v145\Notepad3.exe` has a fresh timestamp.

### APDL x64 artifact build

`pwsh Build/scripts/Build-APDL.ps1 -BuildNumber 1` builds the full solution in
Release/x64 using the installed Visual Studio toolset. It requires Windows,
PowerShell 7, NuGet on PATH, and Visual Studio C++ tools. It generates version
headers/manifests from the checked-in templates, verifies the executable's fresh
version and AMD64 PE header, and packages the binary, language DLLs, themes,
licenses, and APDL fixtures under `Bin/APDL-artifacts` with SHA-256 hashes and
source-commit metadata. These checks do not replace Windows GUI acceptance tests.

`.github/workflows/apdl-windows.yml` runs this script on `windows-latest` for
`codex/apdl-*` branch pushes or manual dispatch, then uploads `Notepad3-APDL-x64`.

### Invoking MSBuild from PowerShell

`msbuild` is not on the PowerShell PATH. Locate it with `vswhere.exe`:

```powershell
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
& $msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
Write-Host "Exit code: $LASTEXITCODE"
```

Check `$LASTEXITCODE` and scan output for `-> Notepad3.exe` or `error C` entries. Avoid reading verbose output in full; filter or tail it.

## Tests and CI

Tests include `test\TestFileVersion.cmd` and `test\TestAhkNotepad3.cmd` (AutoHotkey).

- `test\test_files\encoding\TestEncodingDetection.ps1` builds a temporary native regression harness using Visual C++ and the current encoding detector functions. See [Encoding diagnostics and tests](../domain/encoding-diagnostics-and-tests.md#regression-coverage) for coverage and stub boundaries.
- The CI runner has no `wmic`; query dates and file versions with PowerShell.
- Both tests share a `shell: cmd` step, which reports only the last command's exit code; append `|| exit /b 1` to every earlier test command.
- Drive dialogs in the AHK test should be driven with `PostMessage(WM_COMMAND, ...)`, not `ControlClick`; synthesized clicks fail intermittently on the runner desktop.

## Python environment

For scripting tasks, `.venv\` may provide Python 3.14 and is git-ignored. If absent, check for `py` or `python` before relying on Python availability.

```powershell
.venv\Scripts\python.exe <script.py>
.venv\Scripts\pip.exe install <package>
```

## Windows platform support

Supported platforms: Win32 (x86), x64, x64_AVX2, and ARM64. ARM 32-bit is not supported; `Release|ARM` maps to Win32.

- Both ARM64 and x64 define `_WIN64`; use `_M_ARM64` or `NP3_BUILD_ARM64` in `TypeDefs.h` to distinguish them.
- ARM64 build settings include `CETCompat=false` (CET is x86/x64 only), `TargetMachine=MachineARM64`, and `_WIN64`. Related scripts: `Build\scripts\FixARM64{CETCompat,CrossCompile,OutDir}.ps1`.
