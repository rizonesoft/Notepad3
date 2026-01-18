# EditorConfig Integration Research

## Overview

**EditorConfig** maintains consistent coding styles across editors and IDEs by reading `.editorconfig` files from the project directory tree.

- Website: https://editorconfig.org/
- Spec: https://spec.editorconfig.org/
- C Library: https://github.com/editorconfig/editorconfig-core-c

## Supported Properties

| Property | Values | NP3 Scintilla API |
|----------|--------|-------------------|
| `indent_style` | `tab`, `space` | `SCI_SETUSETABS` |
| `indent_size` | integer | `SCI_SETINDENT` |
| `tab_width` | integer | `SCI_SETTABWIDTH` |
| `end_of_line` | `lf`, `cr`, `crlf` | `SCI_SETEOLMODE` |
| `charset` | `utf-8`, `latin1`, etc. | Encoding system |
| `trim_trailing_whitespace` | `true`, `false` | On-save hook |
| `insert_final_newline` | `true`, `false` | On-save hook |

## Implementation Approach: editorconfig-core-c

### Dependencies

```
editorconfig-core-c (BSD-2-Clause)
├── PCRE2 (for glob matching) - Already in NP3 via Oniguruma? Check compatibility
└── CMake (build only)
```

### Integration Points

1. **On File Open** (`Notepad3.c` → `FileLoad()`)
   - Walk up directory tree to find `.editorconfig` files
   - Call `editorconfig_parse()` with full file path
   - Apply settings via `Style_*` and `SciCall_*` functions

2. **Settings Application** (`Styles.c` or new `EditorConfig.c`)
   ```c
   void ApplyEditorConfig(LPCWSTR filePath) {
       editorconfig_handle eh = editorconfig_handle_init();
       int err = editorconfig_parse(filePath, eh);
       if (!err) {
           int count = editorconfig_handle_get_name_value_count(eh);
           for (int i = 0; i < count; i++) {
               const char *name, *value;
               editorconfig_handle_get_name_value(eh, i, &name, &value);
               // Map to NP3 settings...
           }
       }
       editorconfig_handle_destroy(eh);
   }
   ```

3. **User Toggle** (Settings2 INI option)
   - `EditorConfigSupport = 1` (default enabled)
   - Menu: View → Apply EditorConfig (toggle)

### Build Integration

- Add `editorconfig-core-c` as Git submodule or NuGet package
- Modify `scintilla.vcxproj` or create `editorconfig.vcxproj`
- Link statically to avoid DLL dependency

## Effort Estimate

| Phase | Days | Description |
|-------|------|-------------|
| Library integration | 3-5 | Build editorconfig-core-c with MSVC, link to NP3 |
| Core parsing | 2-3 | Hook file-open, call parser, get properties |
| Settings mapping | 2-3 | Map EC properties to Scintilla/NP3 APIs |
| On-save hooks | 1-2 | trim_trailing_whitespace, insert_final_newline |
| UI/Settings | 1 | Toggle option, status bar indicator |
| Testing | 2-3 | Various project structures, edge cases |
| **Total** | **11-17** | ~2-4 weeks |

## Current State

- ✅ `.editorconfig` files syntax highlighted (`styleLexPROPS.c`)
- ❌ Settings not applied when opening files
- 📋 GitHub feature request: Consider filing issue

## References

- Notepad++ EditorConfig plugin: https://github.com/editorconfig/editorconfig-notepad-plus-plus
- VS Code native support (built-in)
- Sublime Text: EditorConfig package
