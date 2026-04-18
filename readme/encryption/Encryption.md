# AES-256 Rijndael Encryption

Notepad3 can transparently encrypt and decrypt text files using **AES-256** (Rijndael) in **CBC mode**. Passphrases are converted to 256-bit encryption keys via **SHA-256** hashing. An optional **master key** provides emergency data recovery or programmatic access without knowing the file passphrase.

The same encrypted file format is used by both the editor and the companion command-line tool **`np3encrypt`**, so files can be processed interchangeably.

| | |
|---|---|
| **Algorithm** | AES-256-CBC (Rijndael, 256-bit key, 128-bit blocks) |
| **Key derivation** | SHA-256 hash of passphrase |
| **Menu** | File → Set Encr**y**ption Passphrase… |
| **Shortcut** | *(none by default — accessible via the File menu)* |
| **CLI tool** | `np3encrypt.exe` (built alongside Notepad3) |

---

## Quick Start

### Encrypting a File

1. Open or create a text file in Notepad3.
2. Go to **File → Set Encryption Passphrase…** — the **Encryption** dialog opens.
3. Enter a passphrase in the top field and check **"Encrypt using Passphrase"**.
4. *(Optional)* Enter a master passphrase in the lower field and check **"Set New Master Key"** — see [Master Key](#master-key) below.
5. Click **OK**.
6. **Save** the file (`Ctrl+S`). The file is now encrypted on disk.

### Decrypting a File

1. Open an encrypted file in Notepad3 (**File → Open** or drag-and-drop).
2. Notepad3 detects the encryption preamble and displays the **Decrypt File** dialog.
3. Enter the file passphrase (or check **"decrypt using the master key"** and enter the master passphrase).
4. Click **OK** — the file is decrypted and displayed in the editor.

> **Wrong passphrase?** — Notepad3 shows *"The Passphrase is incorrect — Retry another Passphrase?"*. Click **Retry** to enter a different passphrase, or **Cancel** to open the raw encrypted bytes.

### Removing Encryption

1. Open the encrypted file (entering the correct passphrase).
2. Go to **File → Set Encryption Passphrase…**.
3. **Clear** the passphrase field and **uncheck** "Encrypt using Passphrase".
4. Click **OK**, then **Save**. The file is now saved as plain text.

---

## The Encryption Dialog

When you select **File → Set Encryption Passphrase…**, the Encryption dialog appears with these controls:

| Control | Purpose |
|---------|---------|
| **Passphrase** (top field) | The file passphrase used to encrypt/decrypt the file. |
| **Encrypt using Passphrase** | Checkbox — when checked, the file will be encrypted on save. |
| **Optional Master Key** (lower field) | A second passphrase for the master key (see [Master Key](#master-key)). |
| **Set New Master Key** | Checkbox — when checked, a new master key is generated from the master passphrase and embedded in the file. |
| **Reuse Master Key** | Checkbox — appears when the file already contains a master key. Keeps the existing master key without re-entering the master passphrase. |
| **show passphrases** | Checkbox — toggles visibility of the passphrase fields (hidden by default with dot characters). |

---

## The Decrypt Dialog

When Notepad3 opens a file that starts with the encryption preamble, the **Decrypt File** dialog prompts for the passphrase:

| Control | Purpose |
|---------|---------|
| **Passphrase** | Enter the file passphrase or the master passphrase. |
| **This file has a master key** | Label — shown only if the file contains an embedded master key block. |
| **decrypt using the master key** | Checkbox — when checked, the entered passphrase is treated as the master passphrase instead of the file passphrase. |
| **show passphrase** | Checkbox — toggles passphrase visibility. |

---

## Master Key

An encrypted file can optionally contain a copy of its own file key, encrypted with a second **master key** (derived from a master passphrase). This allows anyone who knows the master passphrase to decrypt the file — even without knowing the file passphrase.

### Use Cases

#### 1. Data Recovery

Using different passphrases for different files (and changing them over time) is good security practice, but creates the risk of forgetting a passphrase. A master key acts as an emergency recovery mechanism:

- The master passphrase is rarely used and never shared, making it unlikely to be compromised.
- A single master passphrase can recover any file that was encrypted with it, regardless of the file's individual passphrase.

> **Recommendation:** Choose a master passphrase that is very hard to guess but very hard to forget — for example, a long memorable sentence. Never write it down. Treat it as an emergency-only measure.

#### 2. Programmatic / Trapdoor Access

When an automated system needs to read encrypted files that are edited by humans:

- Humans use individual file passphrases that they can change as needed.
- The program uses the master passphrase (embedded in its configuration) to decrypt any file.
- Changing the file passphrase does not break the program's access, because the master key block is preserved.

### How the Master Key Is Preserved

The master key block is automatically propagated across file saves without requiring the master passphrase to be re-entered:

- **File opened with file passphrase, master key present:** The encrypted master key block is copied unchanged into the next saved version. An editor who knows only the file passphrase preserves a master key they could not create.
- **File opened with master passphrase:** The recovered file key is reused for the next save. An editor who knows only the master passphrase preserves a file key they could not create.
- **Master passphrase changed:** A new master key block is generated, replacing the old one.

Use the **"Reuse Master Key"** checkbox in the Encryption dialog to explicitly keep the existing master key when re-saving.

---

## Command-Line Tool: `np3encrypt`

Notepad3 ships with **`np3encrypt.exe`**, a command-line utility that encrypts and decrypts files using the same format as the editor. This is useful for batch processing, scripting, and automation.

### Usage

```
np3encrypt {command} source destination passphrase [master-passphrase]
```

### Commands

| Command | Description | Passphrases |
|---------|-------------|-------------|
| `EF` | **Encrypt** with file passphrase only | `passphrase` = file passphrase |
| `EM` | **Encrypt** with file + master passphrase | `passphrase` = file passphrase, `master-passphrase` = master passphrase |
| `DF` | **Decrypt** using the file passphrase | `passphrase` = file passphrase |
| `DM` | **Decrypt** using the master passphrase | `passphrase` = master passphrase |

### Examples

```cmd
rem Encrypt a file with a file passphrase
np3encrypt EF plaintext.txt encrypted.txt "my secret phrase"

rem Encrypt with both file and master passphrases
np3encrypt EM plaintext.txt encrypted.txt "file phrase" "master phrase"

rem Decrypt using the file passphrase
np3encrypt DF encrypted.txt decrypted.txt "file phrase"

rem Decrypt using the master passphrase
np3encrypt DM encrypted.txt decrypted.txt "master phrase"
```

> **Note:** `np3encrypt` reads from `source` and writes to `destination`. They must be different files. The source file is not modified.

---

## Passphrase Best Practices

While AES-256 provides very strong encryption, the overall security depends on the quality of your passphrase. The encryption key is only as strong as the passphrase from which it is derived.

### Recommendations

- **Use long passphrases** — a full sentence is much harder to crack than a short password. For example: *"The rain in Spain falls mainly on the plain"* is far stronger than *"P@ssw0rd"*.
- **Use different passphrases for different files** — if one passphrase is compromised, only that file is affected.
- **Change passphrases periodically** — rotate file passphrases over time, especially for sensitive data.
- **Use a master key for recovery** — this guards against data loss from forgotten passphrases.
- **Never write passphrases down** in easily discoverable locations.
- **Be aware of your environment** — the most likely avenues of compromise are social engineering, physical observation, or keyloggers, not brute-force attacks on AES-256.

### What AES-256 Does and Does Not Protect

| Scenario | Protection level |
|----------|-----------------|
| Preventing casual access to your files | ✔ Strong |
| Protecting data if a device is lost or stolen | ✔ Strong (with a good passphrase) |
| Protecting against someone who already has your passphrase | ✘ None |
| Protecting against keyloggers or physical observation | ✘ None (the passphrase itself is the weak link) |
| Brute-force attacks on a long passphrase | ✔ Computationally infeasible with AES-256 |
| Dictionary attacks on a short, word-like passphrase | ⚠ Vulnerable — use longer passphrases |

---

## Technical Reference

### Encryption Algorithm

- **Cipher:** AES (Rijndael) with 256-bit keys and 128-bit (16-byte) blocks
- **Mode:** CBC (Cipher Block Chaining) — each block is XORed with the previous ciphertext block before encryption
- **Key derivation:** The passphrase (as a byte stream) is passed through **SHA-256** to produce a 256-bit encryption key
- **Initialization vector (IV):** 16 bytes of pseudorandom data, unique per file, stored in the file header
- **Padding:** PKCS-style padding — 1 to 16 bytes appended to fill the final 16-byte block (there are always at least 1 byte of padding)

### Unicode Passphrase Handling

Passphrases entered in Notepad3's dialog are Unicode (UTF-16). They are converted to a byte stream by interleaving the low and high bytes of each character (skipping zero bytes). This produces the same byte sequence as an ASCII passphrase for pure-ASCII input, maintaining compatibility with the original NotepadCrypt format and the `np3encrypt` command-line tool.

### Encrypted File Format

An encrypted file has the following binary layout:

```
┌──────────────────────────────────────────────────────┐
│  Preamble (8 bytes)                                  │
│    ├── Magic number: 0x04030201 (4 bytes)            │
│    └── Format type (4 bytes):                        │
│          0x00000001 = file key only                   │
│          0x00000002 = file key + master key           │
├──────────────────────────────────────────────────────┤
│  File IV (16 bytes)                                  │
│    Pseudorandom initialization vector for AES-CBC    │
├──────────────────────────────────────────────────────┤
│  Master Key Block (only if format = 0x00000002)      │
│    ├── Master IV (16 bytes)                          │
│    └── Encrypted file key (32 bytes)                 │
│         (file key encrypted with master key + IV)    │
├──────────────────────────────────────────────────────┤
│  Encrypted data                                      │
│    File content encrypted with AES-256-CBC           │
│    using the file key and file IV                    │
├──────────────────────────────────────────────────────┤
│  Padding (1–16 bytes)                                │
│    Fills the final AES block                         │
└──────────────────────────────────────────────────────┘
```

#### Header Sizes

| Component | Size | Present |
|-----------|------|---------|
| Magic number | 4 bytes | Always |
| Format type | 4 bytes | Always |
| File IV | 16 bytes | Always |
| Master IV | 16 bytes | Master key format only |
| Encrypted file key | 32 bytes | Master key format only |
| **Total header (no master key)** | **24 bytes** | |
| **Total header (with master key)** | **72 bytes** | |

### Key Management Across File Generations

When a file is re-saved, the encryption keys are managed as follows:

| Scenario | Behavior |
|----------|----------|
| File opened with file passphrase, passphrase unchanged | Same passphrase → same key used for re-encryption |
| File opened with file passphrase, passphrase changed | New passphrase → new key generated |
| File opened with master passphrase | Recovered file key is reused (original file passphrase not needed) |
| Master key present, neither passphrase changed | Existing encrypted master key block is copied into new file |
| Master passphrase changed | New master key block generated from new master passphrase |

This design ensures that:
- An editor who knows only the **file passphrase** can preserve the master key (they cannot create one, but they can propagate it).
- An editor who knows only the **master passphrase** can preserve the file key (they cannot create one from a file passphrase, but they can propagate the recovered key).

### Cryptographic Components

| Component | Source | Purpose |
|-----------|--------|---------|
| AES (Rijndael) | `rijndael-alg-fst.c` / `rijndael-api-fst.c` | Block cipher (256-bit key, 128-bit blocks) |
| SHA-256 | `sha-256.c` | Key derivation from passphrase |
| CBC mode | Built into the AES API (`AES_MODE_CBC`) | Block chaining for data encryption |

### Compatibility

The encrypted file format is compatible with the original **NotepadCrypt** format (by Dave Dyer). Files encrypted by Notepad3 can be decrypted by any tool that implements the same format, and vice versa. The format is documented above and in `Build\Docs\crypto\` in the Notepad3 source tree.

---

## Related Menu Items

| Menu path | Action |
|-----------|--------|
| File → Set Encr**y**ption Passphrase… | Open the Encryption dialog to set/change/remove encryption |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "The Passphrase is incorrect" on open | You entered the wrong passphrase. Click **Retry** to try again. If the file has a master key, check **"decrypt using the master key"** and enter the master passphrase instead. |
| File opens as garbled binary data | You cancelled the passphrase dialog. The raw encrypted bytes are displayed. Close and reopen the file, entering the correct passphrase. |
| Encryption passphrase option is not visible | Make sure you are in the **File** menu — the item is "Set Encryption Passphrase…". |
| `np3encrypt` not found | The tool is built as part of the Notepad3 solution. Look for `np3encrypt.exe` in the build output directory alongside `Notepad3.exe`. |
| Forgot the file passphrase | If the file has a master key, use the master passphrase to decrypt it (in the editor or via `np3encrypt DM`). If there is no master key, the data cannot be recovered. |
