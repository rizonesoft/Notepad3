# Scintilla & Lexilla Upgrade Plan

## Current vs Latest Versions

| Library | Current | Latest | Gap |
|---------|---------|--------|-----|
| Scintilla | 5.5.7 | **5.5.8** | 1 minor |
| Lexilla | 5.4.5 | **5.4.6** | 1 minor |

## Customization Analysis

### Lexilla: ✅ No Modifications
- Clean upstream code
- Safe to replace directly

### Scintilla: 🟡 Has Customizations

| Customization | Files | Impact |
|---------------|-------|--------|
| **Oniguruma Regex Engine** | `oniguruma/scintilla/OnigurumaRegExEngine.cxx` (~900 lines) | Must preserve |
| **Oniguruma Library** | `oniguruma/src/` (40+ files) | Must preserve |
| **Custom VS Projects** | `Scintilla.vcxproj`, `ScintillaDLL.vcxproj` | May need merge |
| **NP3 Compiler Defines** | `SCI_OWNREGEX`, `NP3`, `ONIG_STATIC`, `NO_CXX11_REGEX` | Must keep |

---

## Upgrade Steps

### Phase 1: Lexilla (Easy)

```
[ ] 1. Download Lexilla 5.4.6 from scintilla.org
[ ] 2. Backup current lexilla/ folder
[ ] 3. Replace lexilla/src/ contents
[ ] 4. Replace lexilla/include/ contents
[ ] 5. Keep existing .vcxproj files (just update if needed)
[ ] 6. Rebuild and test
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
