# Scintilla & Lexilla Upgrade Plan

## Current vs Latest Versions

| Library | Current | Latest | Gap |
|---------|---------|--------|-----|
| Scintilla | 5.5.7 | **5.5.8** | 1 minor |
| Lexilla | 5.4.5 | **5.4.6** | 1 minor |

## Customization Analysis

### Lexilla: 🔴 HAS CUSTOMIZATIONS (lexers_x/)

The `lexilla/lexers_x/` folder contains **24 custom files** that MUST be preserved:

| File | Type | Notes |
|------|------|-------|
| `LexAHK.cxx` | Custom Lexer | AutoHotkey |
| `LexCSV.cxx` | Custom Lexer | CSV/TSV |
| `LexJSON.cxx` | **Modified** | Enhanced version (orig in `orig/`) |
| `LexKotlin.cxx` | **Modified** | Enhanced version (orig from zufuliu) |
| `LexVerilog.cxx` | **Modified** | Enhanced version (orig in `orig/`) |
| `LexerUtils.cxx/h` | Helper | Shared lexer utilities |
| `CharSetX.h` | Helper | Extended character sets |
| `StringUtils.h` | Helper | String utilities |
| `SciX.iface` | Extension | Extended Scintilla interface |
| `SciXLexer.h` | Extension | Extended lexer header |
| `homebrew/LexAHK*.cxx` | Homebrew | Alternative AHK implementations |
| `orig/*.cxx` | Backups | Original upstream versions |

**CRITICAL**: The `lexers/` folder (41 files) may also have modifications - these are Lexilla's standard lexers.

### Scintilla: 🔴 HAS CUSTOMIZATIONS (oniguruma/)

| Customization | Files | Impact |
|---------------|-------|--------|
| **Oniguruma Regex Engine** | `oniguruma/scintilla/OnigurumaRegExEngine.cxx` (~904 lines) | Must preserve |
| **Oniguruma Library** | `oniguruma/src/` (40+ files) | Must preserve |
| **Custom VS Projects** | `Scintilla.vcxproj`, `ScintillaDLL.vcxproj` | May need merge |
| **NP3 Compiler Defines** | `SCI_OWNREGEX`, `NP3`, `ONIG_STATIC`, `NO_CXX11_REGEX` | Must keep |

---

## Upgrade Steps

### Phase 1: Lexilla (CAREFUL - Has Customizations)

```
[ ] 1. Download Lexilla 5.4.6 from scintilla.org
[ ] 2. Backup ENTIRE lexilla/ folder
[ ] 3. DO NOT REPLACE these (must preserve):
      - lexilla/lexers_x/ (entire folder - 24 custom files!)
      - lexilla/*.vcxproj (project files reference lexers_x)
      - lexilla/*.vcxproj.filters
[ ] 4. CAREFULLY update these folders:
      - lexilla/src/ (core library)
      - lexilla/include/ (headers)
      - lexilla/lexlib/ (lexer utilities)
[ ] 5. DIFF CHECK lexilla/lexers/ against upstream:
      - Some lexers may have NP3-specific patches
      - Compare with orig/ backups in lexers_x/
[ ] 6. Rebuild and test all syntax highlighting
```

### Phase 2: Scintilla (Moderate)

```
[ ] 1. Download Scintilla 5.5.8 from scintilla.org
[ ] 2. Backup current scintilla/ folder
[ ] 3. DO NOT replace these (preserve NP3 customizations):
      - scintilla/oniguruma/ (entire folder)
      - scintilla/*.vcxproj (project files)
      - scintilla/*.vcxproj.filters
[ ] 4. Replace these folders:
      - scintilla/src/
      - scintilla/include/
      - scintilla/win32/
      - scintilla/doc/
      - scintilla/scripts/
[ ] 5. Check for API changes in:
      - include/Scintilla.h
      - include/ILexer.h
      - src/Document.h
[ ] 6. Update OnigurumaRegExEngine.cxx if needed (API changes)
[ ] 7. Rebuild and test
```

### Phase 3: Testing

```
[ ] 1. Build Debug x64
[ ] 2. Build Release x64
[ ] 3. Build Release Win32
[ ] 4. Test basic editing
[ ] 5. Test regex search/replace
[ ] 6. Test syntax highlighting (multiple languages)
[ ] 7. Test find in files (grepWinNP3)
[ ] 8. Run automated tests
```

---

## Risk Assessment

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Regex API changes | Low | Check Document.h for RegexSearchBase changes |
| Lexer API changes | Low | Check ILexer.h interface |
| Build errors | Medium | Keep backups, incremental updates |
| Runtime bugs | Low | Thorough testing before release |

---

## Resources

- Scintilla: https://scintilla.org/
- Lexilla: https://scintilla.org/Lexilla.html
- Release History: https://scintilla.org/ScintillaHistory.html
- Oniguruma: https://github.com/kkos/oniguruma

---

## Notes

The main complexity is preserving the **Oniguruma regex integration**:
- Author: RaiKoHoff
- Location: `scintilla/oniguruma/scintilla/OnigurumaRegExEngine.cxx`
- Purpose: Unicode-aware regex with extended syntax
- Activated via `SCI_OWNREGEX` preprocessor define
