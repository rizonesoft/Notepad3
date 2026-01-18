# Autosave / Backup Feature Research

## Overview

Multiple users have requested autosave/backup functionality to prevent data loss during crashes, hangs, or accidental closure.

## Related GitHub Issues

- [#1665](https://github.com/rizonesoft/Notepad3/issues/1665) - Autosave option during Windows crash/BSoD
- [#370](https://github.com/rizonesoft/Notepad3/issues/370) - Intelligent backup with optional versioning
- [#512](https://github.com/rizonesoft/Notepad3/issues/512) - Periodic backup of unsaved documents (timer-based)

## Feature Requirements

### Core Autosave
- Timer-based periodic save (configurable interval: 1-30 minutes)
- Save to temporary location (not overwrite original)
- Restore unsaved documents on next startup
- Handle "Untitled" documents (not yet saved to disk)

### Backup/Versioning (From #370)
- Create `.bak` file before overwriting original
- Optional versioning: `file.ext.bak`, `file.ext.bak_1`, `file.ext.bak_2`, ...
- Configurable backup location (same dir, temp dir, custom dir)
- Max backup count option

## Implementation Approaches

### Approach A: Simple Autosave (Minimal)
Store document state periodically to `%APPDATA%\Notepad3\recovery\`

```
%APPDATA%\Notepad3\recovery\
├── session.json          # Maps recovery files to original paths
├── doc_001.txt.recovery  # Content snapshot
├── doc_002_untitled.recovery
└── ...
```

**Effort: ~1-2 weeks**

### Approach B: Full Backup System (Complete)
Autosave + versioned backups on every save

**Effort: ~3-4 weeks**

## Implementation Tasks

| Task | Days | Description |
|------|------|-------------|
| Timer infrastructure | 1-2 | Windows timer for periodic saves |
| Recovery file format | 1 | JSON metadata + content storage |
| Recovery on startup | 1-2 | Detect and offer restore of unsaved docs |
| Backup on save | 1-2 | Create .bak before overwriting |
| Versioned backups | 1-2 | .bak_1, .bak_2, rotation/cleanup |
| UI/Settings | 1 | Enable/disable, interval, backup location |
| Testing | 2 | Crash simulation, file locking, edge cases |

## Settings (Proposed)

```ini
[Settings2]
AutosaveEnabled=1
AutosaveInterval=300        ; seconds (5 minutes default)
AutosaveRecoveryDir=        ; empty = %APPDATA%\Notepad3\recovery

CreateBackupOnSave=0
BackupVersioningEnabled=0
BackupMaxVersions=5
BackupDirectory=            ; empty = same as original file
```

## Scintilla APIs Needed

- `SCI_GETMODIFY` - Check if document modified
- `SCI_GETTEXT` / `SCI_GETCHARACTERPOINTER` - Get content for backup
- File metadata (encoding, line endings) from NP3 state

## References

- Notepad++ session recovery: `%APPDATA%\Notepad++\backup\`
- VSCode: Untitled files in `~\.vscode\Backups\`
- Sublime Text: Hot exit feature
