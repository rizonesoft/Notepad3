# First-party source guidance

Before changing unfamiliar C/C++ behavior, read [Codebase and conventions](../agent-trail/architecture/codebase-and-conventions.md) for wrapper, buffer-lifetime, undo, and dialog-result constraints. Read only the relevant topic below; these links are not eager imports.

| Area | Guidance |
|---|---|
| File load/save, Open/MRU dialogs | [File I/O](../agent-trail/domain/file-io.md) |
| Path handles and Win32 file APIs | [PathLib](../agent-trail/domain/pathlib.md) |
| Encoding selection and diagnostics | [Detection](../agent-trail/domain/encoding-detection.md), [tests](../agent-trail/domain/encoding-diagnostics-and-tests.md) |
| Scintilla message replacements | [API traps](../agent-trail/architecture/scintilla-api-usage.md) |
| Lexer registration, styles, themes | [Lexer development](../agent-trail/domain/lexer-development.md) |
| INI persistence | [Configuration](../agent-trail/domain/configuration-and-ini.md); additional local guidance in `Config\AGENTS.md` |

Other topics and build/test procedures remain in the [topic index](../agent-trail/index.md). Local vendored-library edits require explicit scope approval; do not treat a library upgrade as permission to discard Notepad3 patches.
