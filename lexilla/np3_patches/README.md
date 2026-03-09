# Notepad3 Lexilla Patches

This directory contains documentation of all modifications made to the upstream Lexilla library for Notepad3 compatibility.

## Patch Registry

| ID | Patch File | Target File | Description | Required |
|----|------------|-------------|-------------|----------|
| 001 | `001_StyleContext_MatchNext.patch` | `lexlib/StyleContext.h` | Adds `MatchNext()` and `Match(char,char,char)` overloads | ✅ Yes |
| 002 | `002_Lexilla_CustomCatalogue.patch` | `src/Lexilla.cxx` | Custom lexer catalogue (NP3 subset only) | ✅ Yes |

---

## Patch Details

### 001: StyleContext MatchNext Extensions

**File:** `lexlib/StyleContext.h`  
**Status:** Required for NP3 lexers

Adds the following methods to the `StyleContext` class:

```cpp
bool Match(char ch0, char ch1, char ch2) const noexcept;
bool MatchNext() const noexcept;
bool MatchNext(char ch0, char ch1) const noexcept;
bool MatchNext(char ch0, char ch1, char ch2) const noexcept;
```

**Used by:**
- `lexers_x/LexKotlin.cxx`
- `lexers_x/orig/zufuliu/LexKotlin.cxx`
- `lexers_x/orig/zufuliu/LexDart.cxx`

**Marker:** `// >>>>>>>>>>>>>>> BEG NON STD SCI PATCH >>>>>>>>>>>>>>>`

---

### 002: Custom Lexer Catalogue

**File:** `src/Lexilla.cxx`  
**Status:** Required - DO NOT UPGRADE

NP3 compiles only a subset of Lexilla's lexers (~47 vs 124 upstream). The `Lexilla.cxx` file contains the catalogue of registered lexers. Using the upstream version causes 80+ unresolved symbol errors.

**NP3 Lexer Sources:**
- `lexilla/lexers/` - 41 curated upstream lexers
- `lexilla/lexers_x/` - 6 NP3-specific lexers (AHK, CSV, JSON, Kotlin, Verilog)

---

## Upgrade Procedure

When upgrading Lexilla to a new version:

1. **DO NOT** blindly copy `lexlib/StyleContext.h` - preserve patch 001
2. **DO NOT** copy `src/Lexilla.cxx` - keep NP3's custom catalogue
3. **Safe to copy:** `include/`, `lexlib/CharacterCategory.cxx`, individual lexers from `lexers/`
4. After upgrade, verify patches are intact:
   ```powershell
   git diff --no-index upstream/StyleContext.h lexilla/lexlib/StyleContext.h
   ```

---

## Directory Structure

```
lexilla/np3_patches/
├── README.md                           # This file
├── 001_StyleContext_MatchNext.patch    # StyleContext extensions
└── 002_Lexilla_CustomCatalogue.patch   # Custom lexer catalogue diff
```

---

## Version History

| Lexilla Version | Upgrade Date | Notes |
|-----------------|--------------|-------|
| 5.4.5 → 5.4.6 | 2026-01-18 | Incremental upgrade, patches preserved |
