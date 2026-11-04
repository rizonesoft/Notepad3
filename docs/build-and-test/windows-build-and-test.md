# Windows Build and Test

Run commands from the repository root. Discover installed tools before assuming they are on PATH.

## Focused feedback

```powershell
pwsh -NoProfile -File test\test_files\encoding\TestEncodingDetection.ps1
pwsh -NoProfile -File test\TestOpenDialogDirectory.ps1
pwsh -NoProfile -File Build\scripts\Test-Docs.ps1
```

The encoding runner compiles current detector functions in a temporary native harness and removes its generated files. It requires Visual C++ build tools and stubs application state and UCHARDET; it does not test the full editor load path. See [Encoding diagnostics and tests](../domain/encoding-diagnostics-and-tests.md#regression-coverage).

The Open-directory runner also requires Visual C++ build tools. It extracts `_GetParentDir`, `_CanonicalizeInitialDir`, and `_ResolveOpenInitialDir` from `src\Dialogs.c` into a temporary harness with in-memory path/MRU stubs. Its 14 cases cover precedence, missing recent files/directories, drive roots, quoted relative history, and the Save/history distinction. It does not exercise real PathLib/filesystem behavior, environment expansion, `OpenFileDlg` aliasing, or the Windows dialog UI.

For compiler-only feedback on the main project, locate MSBuild:

```powershell
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
if (!$msbuild) { throw 'MSBuild not found' }
& $msbuild src\Notepad3.vcxproj /t:ClCompile /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $LASTEXITCODE" }
```

`ClCompile` is not a linked application build: it does not validate resource DLLs or integration with rebuilt dependencies. To isolate compiler outputs, add `/p:BuildProjectReferences=false` and override `IntDir` / `OutDir` with separate directories outside the checkout. Do not clean existing user build outputs.

## Linked builds and prerequisites

- `Build\Build_x64.cmd [Release|Debug]` invokes the full solution for one platform; variants are `_Win32`, `_ARM64`, and `_x64_AVX2`. Default is Release; x64_AVX2 maps to platform x64 and configuration `Release_AVX2` / `Debug_AVX2` in `Build\scripts\Build.ps1`.
- For missing packages or a first build, use a current NuGet CLI: `nuget restore Notepad3.sln`. CI provisions it with `NuGet/setup-nuget`. The bundled `.nuget\NuGet.exe` is legacy (2.7); its restore compatibility has not been verified.
- `pwsh -NoProfile -File Version.ps1` regenerates `src\VersionEx.h` and `res\Notepad3.exe.conf.manifest` and updates version bookkeeping in `Versions\`. It changes local build state; do not run it solely for a documentation check.
- `Build\BuildAll.cmd` builds all four platforms; `Build\Clean.cmd` removes build outputs. Neither is required for a docs-only change.

Equivalent Release/x64 solution invocation after discovering MSBuild above:

```powershell
& $msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
```

Use the default solution target for integration, as CI does. If a narrower target fails, record its exact command and project context rather than assuming it is universally unsupported. Exit status is required; a fresh `Bin\Release_x64_v145\Notepad3.exe` timestamp alone does not establish a successful build.

## Tests and CI

- Product CI is `.github\workflows\build.yml`: `windows-2025-vs2026`, Release on Win32, x64, x64_AVX2, and ARM64. Version/GUI tests run on Win32 and x64.
- Docs CI is `.github\workflows\docs.yml`; it runs the existing docs checker independently, including Markdown-only changes that skip the product build.
- `cmd /d /c "call test\TestFileVersion.cmd"` checks existing x86/x64 binaries against today's date and `Versions\build.txt`. Old local binaries can fail without a source regression; compare the reported expected/actual versions.
- The GUI smoke test requires AutoHotkey v2 and an available desktop: `cmd /d /c "cd /d test && call TestAhkNotepad3.cmd"`. It creates `test\_TESTDIR`, copies an executable/config, and writes `test\test.log`; do not run it in a read-only review or on a shared desktop without approval.
- In the shared CI `shell: cmd` test step, preserve `|| exit /b 1` after the version test so the GUI command cannot mask its failure.
- The About-box smoke test uses `PostMessage(WM_COMMAND, ...)` rather than `ControlClick`, which was unreliable on the runner desktop. Query dates/versions with PowerShell, not `wmic`.

## Formatting and analysis

`src\.clang-format` and `.editorconfig` are the formatting sources of truth; do not reformat unrelated legacy code. clang-format, clang-tidy, and clangd may be installed under Visual Studio's `VC\Tools\Llvm\x64\bin` even when absent from PATH.

No working repository-wide clang-tidy, compilation-database, or sanitizer recipe is established by this guide. Tool availability alone is not a passing analysis run; report such checks as unrun unless configured and exercised for the affected code.

## Python environment

For scripting tasks, activate the git-ignored `.venv\` when present and inspect its version rather than assuming one:

```powershell
. .\.venv\Scripts\Activate.ps1
python --version
```

Activate and run Python in the same shell call; activation does not persist across separate agent shell calls. If the venv is absent, check `py` / `python`. Install dependencies only when required by the task, not as routine setup.

## Windows platform support

Supported platforms: Win32 (x86), x64, x64_AVX2, and ARM64. ARM 32-bit is not supported; `Release|ARM` maps to Win32.

- Both ARM64 and x64 define `_WIN64`; use `_M_ARM64` or `NP3_BUILD_ARM64` in `TypeDefs.h` to distinguish them.
- ARM64 build settings include `CETCompat=false` (CET is x86/x64 only), `TargetMachine=MachineARM64`, and `_WIN64`. Related scripts: `Build\scripts\FixARM64{CETCompat,CrossCompile,OutDir}.ps1`.
