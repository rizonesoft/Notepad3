# Deferred File Loading Enhancements — ILoader, ANSI Memory, Chunked Streaming

## Background: What Was Already Implemented

In a prior session, these items were analyzed and **already committed** to `Dev_Master`:

| Done | Item | File(s) |
|------|------|---------|
| Yes | **Bug fix:** `BeginWaitCursor` timing — wait cursor now activates after file size is known | `src/Edit.c` |
| Yes | **Enhancement:** `FILE_FLAG_SEQUENTIAL_SCAN` added to `CreateFileW` | `src/Edit.c:1197` |
| Yes | **Bug fix:** `Style_SetLexerFromFile` no longer overrides `SC_CACHE_DOCUMENT` for small files | `src/Styles.c:1765-1768` |
| Yes | **Enhancement:** Idle styling tiered — `SC_IDLESTYLING_AFTERVISIBLE` for files >2MB | `src/Styles.c:1785-1791` |
| Yes | **Bug fix:** UTF-16 `WideCharToMultiByte` now uses exact char counts (no off-by-one) | `src/Edit.c:1399-1412` |

**This plan covers the 3 remaining enhancements** that require moderate-to-major refactoring and should be profiled before/after to validate the benefit.

---

## File Loading Pipeline Overview

```
CreateFileW(FILE_FLAG_SEQUENTIAL_SCAN)
  → ReadFileXL() [chunked DWORD_MAX reads]
  → ReadAndDecryptFile() [optional AES-256 decrypt]
  → Encoding_DetectEncoding() [BOM check, uchardet, UTF-8 validation]
  → Encoding conversion:
      UTF-16 path:  SwabEx (if BE) → WideCharToMultiByteEx → UTF-8
      UTF-8 path:   direct (skip BOM if present)
      ANSI/MBCS:    MultiByteToWideCharEx → WideCharToMultiByteEx → UTF-8
  → EditSetNewText()
      → _PrepareDocBuffer() [clear markers, set wrap=NONE]
      → EditSetDocumentBuffer()
          → CreateNewDocument(): SciCall_CreateDocument + SciCall_ReplaceTarget
  → Style_SetLexerFromFile() [lexer, indicators, idle styling, layout cache]
  → Post-load: caret restore, EOL/indent checks, file watching
```

### Key Files (line numbers as of Feb 2025)

| File | Function | Lines | Role |
|------|----------|-------|------|
| `src/Edit.c` | `EditLoadFile()` | 1177-1487 | Disk I/O, encoding detection, conversion |
| `src/Edit.c` | `EditSetNewText()` | 419-448 | Buffer → Scintilla handoff orchestration |
| `src/Edit.c` | `_PrepareDocBuffer()` | 407-417 | Clear markers, disable wrap before load |
| `src/Config/Config.cpp` | `EditSetDocumentBuffer()` | 2946-2977 | Final Scintilla document creation |
| `src/Config/Config.cpp` | `CreateNewDocument()` | 2898-2923 | `SCI_CREATEDOCUMENT` + `SCI_REPLACETARGET` |
| `src/Helpers.c` | `ReadFileXL()` | 835-850 | Chunked disk read (DWORD_MAX chunks) |
| `src/crypto/crypto.c` | `ReadAndDecryptFile()` | ~463 | Read + optional AES decrypt |
| `src/EncodingDetection.cpp` | `Encoding_DetectEncoding()` | ~1261 | BOM/uchardet/UTF-8 analysis |
| `src/Notepad3.c` | `FileLoad()` | ~10917-11230 | Orchestration, post-load processing |
| `src/Styles.c` | `Style_SetLexerFromFile()` | ~1700-1791 | Lexer, indicators, styling setup |
| `src/SciCall.h` | `SciCall_CreateLoader()` | 221 | Wrapper: `DeclareSciCallR2(CreateLoader, CREATELOADER, sptr_t, DocPos, bytes, int, options)` |
| `scintilla/include/ILoader.h` | `ILoader` class | 16-22 | `AddData()`, `ConvertToDocument()`, `Release()` |
| `scintilla/doc/ScintillaDoc.html` | | 7587-7630 | ILoader documentation |

---

## Enhancement A: Use `SCI_CREATELOADER` (ILoader) for UTF-8 Fast Path

**Priority:** Medium — reduces peak memory for the most common encoding path
**Effort:** ~30 lines changed in `Config.cpp`
**Risk:** Moderate — new Scintilla API usage, needs profiling

### Problem

`CreateNewDocument()` in `src/Config/Config.cpp:2898-2923` currently:
1. `SciCall_CreateDocument(lenText, docOptions)` — Scintilla allocates a document buffer
2. `SciCall_SetDocPointer(pNewDocumentPtr)` — installs the empty document
3. `SciCall_TargetWholeDocument()` + `SciCall_ReplaceTarget(lenText, lpstrText)` — copies all data into it

Step 3 copies the **entire** UTF-8 buffer into Scintilla's internal buffer. During this copy, both the app's `lpData`/`lpDataUTF8` buffer and Scintilla's document buffer exist simultaneously. For a 500MB file, that's ~1GB peak memory.

### Solution: ILoader

Scintilla's `ILoader` interface (`scintilla/include/ILoader.h`) allows feeding data directly into a document under construction via `AddData()`, avoiding the double-buffer:

```cpp
#include "ILoader.h"  // scintilla/include/ILoader.h

// In CreateNewDocument(), replace the SCI_CREATEDOCUMENT + SCI_REPLACETARGET path:
sptr_t const loaderPtr = SciCall_CreateLoader((DocPos)lenText, docOptions);
if (loaderPtr) {
    Scintilla::ILoader* pLoader = reinterpret_cast<Scintilla::ILoader*>(loaderPtr);

    static constexpr size_t CHUNK_SIZE = 4 * 1024 * 1024;  // 4MB chunks
    int status = SC_STATUS_OK;
    for (size_t offset = 0; offset < lenText && status == SC_STATUS_OK; offset += CHUNK_SIZE) {
        size_t const n = min(CHUNK_SIZE, lenText - offset);
        status = pLoader->AddData(lpstrText + offset, (Sci_Position)n);
    }

    if (status == SC_STATUS_OK) {
        void* pDoc = pLoader->ConvertToDocument();  // ownership transferred
        SciCall_SetDocPointer((sptr_t)pDoc);
        SciCall_ReleaseDocument((sptr_t)pDoc);
    } else {
        pLoader->Release();
        // fall back to current approach
    }
}
```

### Key Details

- `Config.cpp` is already C++, so `reinterpret_cast` and `Scintilla::ILoader` are available
- `SciCall_CreateLoader` wrapper already exists in `SciCall.h:221`
- The `ILoader::AddData()` returns `SC_STATUS_*` codes — check for `SC_STATUS_OK`
- `ConvertToDocument()` returns a `void*` doc pointer; Scintilla owns it after `SetDocPointer`, caller must `ReleaseDocument`
- The `reload` parameter (for `SciCall_ReplaceTargetMinimal`) is NOT compatible with ILoader — ILoader always creates a fresh document. So ILoader should only be used when `!reload` (first load, not file revert). The `reload` path should keep using `ReplaceTargetMinimal`.
- Need to include path: `#include "ILoader.h"` — verify include search paths in the `Config.cpp` compilation unit

### What to Profile

- Peak memory (Task Manager / Process Explorer) loading a 200MB UTF-8 file, before vs. after
- Load time regression check — ILoader should be same speed or faster
- Verify no difference in loaded content (diff test)

---

## Enhancement B: Reduce ANSI/MBCS Triple-Allocation Peak Memory

**Priority:** Low — only affects non-UTF-8 files; complexity outweighs benefit
**Effort:** Complex
**Risk:** High — encoding edge cases

### Problem

The ANSI/MBCS conversion path in `src/Edit.c:1446-1474` performs:

```
Step 1: lpData = raw file data                        [fileSize bytes]
Step 2: lpDataWide = AllocMem(cbData * 2 + 16)        [2x fileSize]
        MultiByteToWideCharEx → fills lpDataWide
Step 3: FreeMem(lpData)
        lpData = AllocMem(cbDataWide * 3 + 16)        [up to 3x wchar count]
        WideCharToMultiByteEx → fills new lpData
Step 4: EditSetNewText(lpData) → copies into Scintilla
Step 5: FreeMem(lpDataWide), FreeMem(lpData)
```

Between steps 2-3, peak memory is `fileSize + 2*fileSize + 3*wcharCount` ≈ up to **6x fileSize** for worst-case MBCS expansion. After step 3 frees the original `lpData`, peak drops, but the intermediate spike can be significant for large ANSI files.

### Possible Optimizations

1. **Query exact sizes first:** Call `MultiByteToWideChar` and `WideCharToMultiByte` with `NULL` output to get exact sizes, then allocate tightly. Currently uses worst-case multipliers (`*2`, `*3`).

2. **Reuse buffer when safe:** For single-byte encodings (Latin-1, CP1252, CP1250, etc.), the UTF-8 expansion is at most 2x. If `cbDataWide * 2 + 16 <= SizeOfMem(original lpData)`, reuse the original buffer. But this requires knowing the encoding class.

3. **Combine with ILoader (Enhancement A):** If Enhancement A is done, the ANSI path could feed UTF-8 chunks into ILoader instead of building a full UTF-8 buffer. But this requires chunked conversion — see Enhancement C.

### Assessment

The current code is correct, clear, and handles all edge cases. The triple-allocation is inherent to Win32's two-step conversion (no direct ANSI→UTF-8 API). **Only implement if profiling shows this path is a real-world bottleneck** for specific users with large non-UTF-8 files.

---

## Enhancement C: Chunked Streaming Pipeline (Disk → Convert → ILoader)

**Priority:** High value, high effort — the "ultimate" file loading optimization
**Effort:** Major architectural change (~200+ lines, multiple files)
**Risk:** High — multi-byte boundary handling, encryption interaction

### Vision

Full streaming pipeline replacing the current "read everything → convert everything → copy to Scintilla":

```
Disk → [4MB chunks] → Encoding Conversion → ILoader::AddData() → ConvertToDocument()
```

**Benefits:**
- Peak memory: ~12MB constant (two 4MB buffers + ILoader internal) vs. current 2-6x fileSize
- Enables progress bar / percentage during load
- Enables user cancellation during long loads
- First visible content appears faster (could style first chunk while loading rest)

### Architecture

```
┌─────────────────────────────────────────────────────┐
│ EditLoadFile()                                       │
│                                                      │
│  1. CreateFileW (FILE_FLAG_SEQUENTIAL_SCAN)          │
│  2. Read first 4MB chunk                             │
│  3. Detect encoding from first chunk:                │
│     - BOM check, uchardet, UTF-8 validation          │
│     - FileVars_GetFromData (first chunk only)         │
│  4. Create ILoader: SciCall_CreateLoader(fileSize)    │
│  5. Loop:                                             │
│     a. ReadFileXL(chunk, 4MB)                         │
│     b. Convert chunk: Source → [UTF-16] → UTF-8       │
│     c. ILoader::AddData(utf8Chunk, utf8Len)           │
│     d. Report progress (optional)                     │
│     e. Check cancellation (optional)                  │
│  6. doc = ILoader::ConvertToDocument()                │
│  7. SciCall_SetDocPointer(doc)                        │
│                                                      │
│  Fallback: if ILoader fails, use current monolithic   │
│            approach                                   │
└─────────────────────────────────────────────────────┘
```

### Challenges and Solutions

#### Challenge 1: Multi-byte boundary splitting

When reading in 4MB chunks, a chunk boundary can land in the middle of a multi-byte sequence:
- **UTF-8:** 1-4 byte sequences. If the last byte has high bit set, check if it starts a multi-byte sequence. Back up to the last complete sequence; carry over the incomplete bytes to the next chunk.
- **UTF-16:** 2 or 4 bytes (surrogate pairs). Ensure chunk size is even. If last WCHAR is a high surrogate (0xD800-0xDBFF), carry it over.
- **Shift-JIS / GBK / Big5:** Lead byte followed by trail byte. If last byte is a lead byte, carry it over.
- **Generic approach:** After reading a chunk, scan backward from the end to find the last safe split point. Carry over the trailing incomplete bytes (max 3-4 bytes) to prepend to the next chunk.

```c
// Example: find safe UTF-8 split point
size_t SafeUTF8Split(const char* data, size_t len) {
    if (len == 0) return 0;
    // Scan backward up to 3 bytes to find a complete sequence
    for (size_t i = 1; i <= min(3, len); i++) {
        unsigned char c = (unsigned char)data[len - i];
        if ((c & 0x80) == 0) return len;           // ASCII — complete
        if ((c & 0xC0) == 0xC0) {                  // Start byte
            int expected = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : 2;
            if (i >= expected) return len;          // Complete sequence
            return len - i;                         // Incomplete — split before it
        }
    }
    return len;  // All continuation bytes within 3 — shouldn't happen in valid UTF-8
}
```

#### Challenge 2: Encoding detection needs first chunk

`Encoding_DetectEncoding()` needs the raw data to detect encoding. Solution: read the first chunk, run detection, then process all chunks (including the first) through the encoding conversion loop.

#### Challenge 3: Encryption

`ReadAndDecryptFile()` currently reads the entire file, then decrypts in-place. AES-256 CBC decryption is block-based (16 bytes) and could work on chunks, but the current implementation (`src/crypto/crypto.c`) is monolithic. **Fallback:** For encrypted files, use the current monolithic path.

#### Challenge 4: `FileVars_GetFromData`

`FileVars_GetFromData()` scans for Emacs/Vim file variables in the first few lines. It only needs the first chunk. Call it on the first chunk's UTF-8 output before continuing the loop.

#### Challenge 5: `EditDetectEOLMode`

Currently runs on the complete UTF-8 buffer. For chunked loading, accumulate EOL counts during the conversion loop (count `\r\n`, `\r`, `\n` as you go) and determine the mode at the end.

### Files to Modify

| File | Changes |
|------|---------|
| `src/Edit.c` | Major rewrite of `EditLoadFile()` to add chunked path alongside monolithic fallback |
| `src/Config/Config.cpp` | New function (or modify `EditSetDocumentBuffer`) to accept ILoader instead of buffer |
| `src/Edit.h` | New function declarations if needed |
| `src/Helpers.c` | Possibly add `SafeEncodingSplit()` helper |
| `src/crypto/crypto.c` | Skip chunked path for encrypted files (no change needed) |

### Implementation Strategy

1. **Phase 1:** Implement Enhancement A (ILoader for UTF-8) first — validates ILoader integration
2. **Phase 2:** Add chunked reading for UTF-8-only files (no encoding conversion needed — the simplest case)
3. **Phase 3:** Add chunked encoding conversion for ANSI/MBCS
4. **Phase 4:** Add chunked UTF-16 → UTF-8 conversion
5. **Phase 5:** Add progress reporting / cancellation UI

Each phase is independently testable and can be shipped separately.

### What to Profile

- Peak memory for 100MB, 500MB, 1GB files across encodings (UTF-8, UTF-16LE, Shift-JIS, Latin-1)
- Load time comparison: chunked vs. monolithic
- UI responsiveness during load (can the user interact / cancel?)

---

## Verification Checklist (for all enhancements)

1. **Build:** `Build\Build_x64.cmd Release` — clean compile, no warnings
2. **Functional tests:**
   - Load UTF-8 file (with/without BOM), verify content correct
   - Load UTF-16LE/BE file (with/without BOM), verify content correct
   - Load ANSI/Latin-1/Shift-JIS file, verify content correct
   - Load encrypted file, verify decryption still works
   - Load file >2GB (on x64), verify large-file path
   - File revert (`FileRevert`) preserves diff-minimal behavior (`ReplaceTargetMinimal`)
3. **Memory tests:**
   - Compare peak working set (Task Manager) for 200MB UTF-8 file, before vs. after
   - Compare for 200MB UTF-16LE file
4. **Performance tests:**
   - Load time for 100MB, 500MB files on SSD
   - Load time for 100MB file on HDD (if available — tests `FILE_FLAG_SEQUENTIAL_SCAN` interaction)
5. **Edge cases:**
   - Empty file (0 bytes)
   - 1-byte file
   - File with embedded NUL characters
   - File that is exactly a multiple of chunk size (4MB)
   - File that is chunk size + 1 byte
   - UTF-8 file where a multi-byte sequence spans the chunk boundary
