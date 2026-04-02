NotepadCrypt
sources and some technical notes

NotepadCrypt is a simple text editor which can optionally encrypt the text files it edits.  NotepadCrypt was designed to fill a requirement to encrypt configuration files which had to be edited by humans but read by programs, without the hazard that would obviously exist if editing required the human editor to make a temporary unencrypted file.  I needed an editor which could read and write an encrypted file, for which the encrypted format was documented and assessable to C and Perl programs.  To my surprise, none existed.

NotepadCrypt was constructed using open source software, including a low level implementation of AES encryption and Sha256 hashing;  and Notepad2 a really nice open source replacement for Notepad.   None of this software had to be changed in any substantial way, but the file format and key management had to be designed and implemented to stitch it all together.   The requirement that a program be able to read the encrypted file led to a design which included a master key, which I think is extremely useful to anyone using NotepadCrypt to encrypt their own confidential files.

Source code:

    Sha256 hashing implementation (used to convert passphrases to encryption keys)  based on an implementation by Christoper Devine.  This file is available from many sources on the web.
    AES encryption, for which there are many open source implementations.
    Notepad2 editor, which also requires the Scintilla editing widget.
    NotepadCrypt version 4.2.25  Full Sources:  This zip includes the as-modified source code for all of the above, VC.net projects for NotepadCrypt and a simple command line tool which uses the same file format.  There are a couple of "extras" - a test program that uses tcsh and a java program that can read NotepadCrypt's file format.  Unless you are a programmer, you're probably better off getting the binary only distribution for NotepadCrypt and NotepadCryptCL

Technical Details For NotepadCrypt  

Overall Design: Sha256 hash is used to convert an ascii pass phrase to a 256 bit encryption key.  Pseudorandom data is used as an initialization vector for AES-256 encryption.  Optionally, a the encryption key (NOT the passphrase)  is encrypted using a second master key, and included in the file header.  This master key can be used as an emergency data recovery key, or as a second key to be used by programs to read encrypted files.

Overall File Format:  Consists of a preamble, the encrypted data, and some padding at the end.

Encrypted files start with an 8 byte preamble, the first 4 bytes are a "magic number" to identify the file type (currently 0x04030201) and a 4 byte sub-file type, (currently either 0x00000001 or 0x00000002 if the file has a master key). 

The next 16 bytes of the preamble are the initialization vector for the AES engine, to be used with the file key. Each file gets a unique 16 bytes of  pseudo random noise. 

Next, for master keyed files, is a 16 byte IV for the master key, followed by a 32 byte block containing the file key, encrypted with the master key, using the master key IV and CBC block chaining.

Next, is the actual file data, encrypted using the file key and the IV, and CBC block chaining.

Finally, are 1-16 bytes of padding to round out the last AES block.  Note that there are never 0 bytes of padding.

Passphrase Management: 256 bit encryption keys are generated from the ascii passphrase by passing the passphrase through a SHA256 hash. The passphrase itself is never stored anywhere except in the dynamic memory of the encrypting program.

Key management over file generations: If the file is opened using a file passphrase, the passphrase is retained and used as the default for the passphrase dialog. If the file is opened using a master passphrase, the recovered file key is used as the default encryption for new files.  This allows an editor who does not know the file passphrase to propogate a file key he could not create. If the file contains a master key, and neither the file or master passphrase is changed, then the retained, master-encrypted  file key is copied into the next file generation (It is still valid).  This allows an editor who knows only the file passphrase to propogate a master key he could not create.


