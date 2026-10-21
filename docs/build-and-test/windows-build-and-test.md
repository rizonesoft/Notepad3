# Windows Build and Test

## Build

- `Build\Build_x64.cmd [Release|Debug]` builds one platform; variants are `_Win32`, `_ARM64`, and `_x64_AVX2`.
- `Build\BuildAll.cmd` builds all four supported platforms. `Build\Clean.cmd` cleans outputs.
- Run `nuget restore Notepad3.sln` once before the first build.
- Run `Version.ps1` before building to regenerate `src\VersionEx.h` (`Major.YY.Mdd.Build`, with the build number in `Versions\build.txt`).
- Default configuration is Release. CI is in `.github\workflows\build.yml` (`windows-2025-vs2026`, Release × all four platforms).
- Verify builds using the default target on the full solution. `/t:Notepad3` fails with MSB4057 because not all subprojects define that target. Check that `Bin\Release_x64_v145\Notepad3.exe` has a fresh timestamp.

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

- The CI runner has no `wmic`; query dates and file versions with PowerShell.
- Both tests share a `shell: cmd` step, which reports only the last command's exit code; append `|| exit /b 1` to every earlier test command.
- Drive dialogs in the AHK test should be driven with `PostMessage(WM_COMMAND, ...)`, not `ControlClick`; synthesized clicks fail intermittently on the runner desktop.

## Windows platform support

Supported platforms: Win32 (x86), x64, x64_AVX2, and ARM64. ARM 32-bit is not supported; `Release|ARM` maps to Win32.

- Both ARM64 and x64 define `_WIN64`; use `_M_ARM64` or `NP3_BUILD_ARM64` in `TypeDefs.h` to distinguish them.
- ARM64 build settings include `CETCompat=false` (CET is x86/x64 only), `TargetMachine=MachineARM64`, and `_WIN64`. Related scripts: `Build\scripts\FixARM64{CETCompat,CrossCompile,OutDir}.ps1`.
