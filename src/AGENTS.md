# First-party source guidance

Before changing unfamiliar C/C++ behavior, read [Codebase and conventions](../docs/architecture/codebase-and-conventions.md) for wrapper, buffer-lifetime, undo, and dialog-result constraints. Read only the relevant topic below; these links are not eager imports.

| Area | Guidance |
|---|---|
| File load/save, Open/MRU dialogs | [File I/O](../docs/domain/file-io.md) |
| Path handles and Win32 file APIs | [PathLib](../docs/domain/pathlib.md) |
| Encoding selection and diagnostics | [Detection](../docs/domain/encoding-detection.md), [tests](../docs/domain/encoding-diagnostics-and-tests.md) |
| Scintilla message replacements | [API traps](../docs/architecture/scintilla-api-usage.md) |
| Lexer registration, styles, themes | [Lexer development](../docs/domain/lexer-development.md) |
| INI persistence | [Configuration](../docs/domain/configuration-and-ini.md); additional local guidance in `Config\AGENTS.md` |

Other topics and build/test procedures remain in the [topic index](../docs/README.md). Local vendored-library edits require explicit scope approval; do not treat a library upgrade as permission to discard Notepad3 patches.
