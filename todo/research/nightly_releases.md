# Nightly Release Workflow for Notepad3

## Goal
Automate nightly builds that package all necessary files for a complete Notepad3 release and publish them as GitHub Releases.

## Release Package Contents

Based on the existing release structure, each nightly release needs:

### Executables (from `bin/Release_<platform>_v143/`)
- `Notepad3.exe`
- `minipath.exe`
- `grepWinNP3.exe`
- `np3encrypt.exe`

### Language Files (from `bin/Release_<platform>_v143/lng/`)
- `np3lng.dll`, `mplng.dll`
- All locale folders (af-ZA, de-DE, en-US, etc.)

### Configuration Files
- `Build/Notepad3.ini`
- `Build/minipath.ini`

### Documentation
- `Build/Changes.txt`
- `Build/Docs/Notepad3.txt`
- `Build/Docs/KeyboardShortcuts.txt`
- `Build/Docs/Oniguruma_RE.txt`

### Themes
- `Build/Themes/Dark.ini`
- `Build/Themes/Obsidian.ini`
- `Build/Themes/Sombra.ini`

---

## Proposed Workflow

### Triggers
- **Schedule**: Every night at 2 AM UTC
- **Manual**: `workflow_dispatch` for on-demand builds

### Build Matrix
- Win32 Release
- x64 Release

### Package Structure
```
Notepad3_Nightly_x64/
├── Notepad3.exe
├── minipath.exe
├── grepWinNP3.exe
├── np3encrypt.exe
├── Notepad3.ini
├── minipath.ini
├── Changes.txt
├── lng/
│   ├── np3lng.dll
│   ├── mplng.dll
│   └── [all locale folders]
├── Docs/
│   ├── Notepad3.txt
│   ├── KeyboardShortcuts.txt
│   └── Oniguruma_RE.txt
└── Themes/
    ├── Dark.ini
    ├── Obsidian.ini
    └── Sombra.ini
```

### Release Strategy
- Tag: `nightly-YYYY-MM-DD`
- Retention: Keep last 7 nightly releases
- Assets: ZIP files for each platform

---

## Questions to Resolve
1. Schedule timing (2 AM UTC?)
2. Retention policy (7 days?)
3. Additional files needed?
4. Naming format preference?

---

## Implementation Steps
1. Create `.github/workflows/nightly.yml`
2. Configure build matrix (reuse from `build.yml`)
3. Add packaging step to create ZIP with all files
4. Add release step to publish to GitHub Releases
5. Add cleanup step to remove old nightlies
