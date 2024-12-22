/* 
 * Notepad3 format decrypter
 * 
 * 
 * This standalone program decrypts files that were encrypted in Notepad3's simple format.
 * 
 * The intent of this program is to provide an independent, robust implementation for handling the file format,
 * in case Notepad3 (or that author's own small standalone decrypter) is inaccessible or has errors.
 * 
 * Prerequisites: compiled class  (javac DecryptNotepad3.java => DecryptNotepad3.class)
 * 
 * Usage: java DecryptNotepad3 InputFile [-m] Passphrase
 * Options:
 *     -m: Use master key (only applicable for files with master key)
 * Examples:
 *     java DecryptNotepad3 myencryptedfile.bin password123
 *     java DecryptNotepad3 myencryptedfile.bin -m masterPass456
 *     (Prints to standard output)
 * 
 * 
 * Copyright (c) 2017 Project Nayuki
 * All rights reserved. Contact Nayuki for licensing.
 * https://www.nayuki.io/page/notepadcrypt-format-decryptor-java
 * Old: http://nayuki.eigenstate.org/page/notepadcrypt-format-decryptor-java
 * 
 * Adaption to Notepad3 by RaiKoHoff
 *
 * NotepadCrypt resources:
 * - http://www.andromeda.com/people/ddyer/notepad/NotepadCrypt.html
 * - http://www.andromeda.com/people/ddyer/notepad/NotepadCrypt-technotes.html
 *
 * Notepad3 resources:
 * - https://rizonesoft.com/downloads/notepad3
 * - https://rizonesoft.com/documents/notepad3
 * - https://github.com/rizonesoft/Notepad3
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import static java.lang.Integer.rotateRight;

public final class DecryptNotepad3 {
	
	/* Main functions */
	
	public static void main(String[] args) throws IOException {
		// Get arguments
		File inputFile;
		String passphrase;
		boolean useMasterKey;
		if (args.length == 2) {
			useMasterKey = false;
			passphrase = args[1];
		} else if (args.length == 3 && args[1].equals("-m")) {
			useMasterKey = true;
			passphrase = args[2];
		} else {
			System.err.println("Usage: java DecryptNotepad3 InputFile [-m] Passphrase");
			System.err.println("    -m: Use master key (only applicable for files with master key)");
			System.exit(1);
			return;
		}
		inputFile = new File(args[0]);
		
		// Read file
		byte[] data = new byte[(int)inputFile.length()];
		InputStream in = new FileInputStream(inputFile);
		try {
			if (data.length > 0 && in.read(data) < data.length)
				throw new IOException("Unexpected file size");
			if (in.read(data) != -1)
				throw new IOException("Unexpected file size");
		} finally {
			in.close();
		}
		
		// Decrypt and write
		byte[] plaintext;
		try {
			plaintext = decryptFileData(data, passphrase.getBytes("US-ASCII"), useMasterKey);
		} catch (IllegalArgumentException e) {
			System.err.println("Error: " + e.getMessage());
			System.exit(1);
			return;
		}
		System.out.write(plaintext);
	}
	
	
	static byte[] decryptFileData(byte[] fileData, byte[] passphrase, boolean useMasterKey) {
		if (fileData.length == 0)
			return fileData;  // Notepad3 produces an empty file when trying to encrypt an empty text file
		
		// Parse file format
		boolean hasMasterKey;
		if (toInt32(fileData, 0) != 0x04030201)
			throw new IllegalArgumentException("Unsupported file format");
		switch (toInt32(fileData, 4)) {
			case 0x01000000:
				hasMasterKey = false;
				break;
			case 0x02000000:
				hasMasterKey = true;
				break;
			default:
				throw new IllegalArgumentException("Unsupported encryption format");
		}
		
		// Decrypt text
		byte[] cipherKey = getSha256Hash(passphrase);
		byte[] initVec = Arrays.copyOfRange(fileData, 8, 24);
		byte[] ciphertext = Arrays.copyOfRange(fileData, hasMasterKey ? 72 : 24, fileData.length);
		if (useMasterKey) {
			if (!hasMasterKey)
				throw new IllegalArgumentException("Master key mode requested on file data with no master key");
			byte[] iv = Arrays.copyOfRange(fileData, 24, 40);
			byte[] fileKey = Arrays.copyOfRange(fileData, 40, 72);
			Aes.decryptCbcMode(fileKey, cipherKey, iv);
			cipherKey = fileKey;
		}
		return decryptWithPadding(ciphertext, cipherKey, initVec);
	}
	
	
	/* Utility functions */
	
	private static byte[] decryptWithPadding(byte[] ciphertext, byte[] key, byte[] initVec) {
		// Decrypt message
		if (ciphertext.length % 16 != 0)
			throw new IllegalArgumentException("Invalid file length");
		byte[] plaintext = ciphertext.clone();
		Aes.decryptCbcMode(plaintext, key, initVec);
		
		// Check padding (rejections are always correct, but false acceptance has 1/255 chance)
		int padding = plaintext[plaintext.length - 1];
		if (padding < 1 || padding > 16)
			throw new IllegalArgumentException("Incorrect key or corrupt data");
		for (int i = 1; i <= padding; i++) {
			if (plaintext[plaintext.length - i] != padding)
				throw new IllegalArgumentException("Incorrect key or corrupt data");
		}
		
		// Strip padding
		return Arrays.copyOfRange(plaintext, 0, plaintext.length - padding);
	}
	
	
	static int toInt32(byte[] b, int off) {  // Big endian
		return (b[off + 0] & 0xFF) << 24 |
		       (b[off + 1] & 0xFF) << 16 |
		       (b[off + 2] & 0xFF) <<  8 |
		       (b[off + 3] & 0xFF) <<  0;
	}
	
	
	/* Cryptography library functions */
	
	private static byte[] getSha256Hash(byte[] msg) {
		if (msg.length > Integer.MAX_VALUE / 8)
			throw new IllegalArgumentException("Message too large for this implementation");
		
		// Add 1 byte for termination, 8 bytes for length, then round up to multiple of block size (64)
		byte[] padded = Arrays.copyOf(msg, (msg.length + 1 + 8 + 63) / 64 * 64);
		padded[msg.length] = (byte)0x80;
		for (int i = 0; i < 4; i++)
			padded[padded.length - 1 - i] = (byte)((msg.length * 8) >>> (i * 8));
		
		// Table of round constants
		final int[] K = {
			0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
			0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
			0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
			0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
			0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
			0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
			0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
			0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
		};
		
		// Compress each block
		int[] state = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};
		for (int off = 0; off < padded.length; off += 64) {
			int[] schedule = new int[64];
			for (int i = 0; i < 16; i++)
				schedule[i] = toInt32(padded, off + i * 4);
			for (int i = 16; i < 64; i++) {
				int x = schedule[i - 15];
				int y = schedule[i -  2];
				schedule[i] = schedule[i - 16] + schedule[i - 7] +
				              (rotateRight(x,  7) ^ rotateRight(x, 18) ^ (x >>>  3)) +
				              (rotateRight(y, 17) ^ rotateRight(y, 19) ^ (y >>> 10));
			}
			
			int a = state[0], b = state[1], c = state[2], d = state[3];
			int e = state[4], f = state[5], g = state[6], h = state[7];
			for (int i = 0; i < 64; i++) {
				int t1 = h + (rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25)) + ((e & f) ^ (~e & g)) + K[i] + schedule[i];
				int t2 = (rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
				h = g;
				g = f;
				f = e;
				e = d + t1;
				d = c;
				c = b;
				b = a;
				a = t1 + t2;
			}
			state[0] += a; state[1] += b; state[2] += c; state[3] += d;
			state[4] += e; state[5] += f; state[6] += g; state[7] += h;
		}
		
		// Serialize state as result
		byte[] hash = new byte[state.length * 4];
		for (int i = 0; i < hash.length; i++)
			hash[i] = (byte)(state[i / 4] >>> ((3 - i % 4) * 8));
		return hash;
	}
	
}



final class Aes {
	
	private static final int BLOCK_LEN = 16;  // Do not modify
	
	
	public static void decryptCbcMode(byte[] msg, byte[] key, byte[] initVec) {
		if (msg.length % BLOCK_LEN != 0 || initVec.length != BLOCK_LEN)
			throw new IllegalArgumentException("Message is not a multiple of block length");
		
		Aes cipher = new Aes(key);
		byte[] prevCiphertextBlock = initVec;
		for (int off = 0; off < msg.length; off += BLOCK_LEN) {
			byte[] curCiphertextBlock = Arrays.copyOfRange(msg, off, off + BLOCK_LEN);
			cipher.decryptBlock(msg, off);
			for (int i = 0; i < BLOCK_LEN; i++)
				msg[off + i] ^= prevCiphertextBlock[i];
			prevCiphertextBlock = curCiphertextBlock;
		}
	}
	
	
	private byte[][] keySchedule;
	
	
	public Aes(byte[] key) {
		if (key.length < 4 || key.length % 4 != 0)
			throw new IllegalArgumentException("Invalid key length");
		
		// Expand key into key schedule
		int nk = key.length / 4;
		int rounds = Math.max(nk, 4) + 6;
		int[] w = new int[(rounds + 1) * 4];  // Key schedule
		for (int i = 0; i < nk; i++)
			w[i] = DecryptNotepad3.toInt32(key, i * 4);
		byte rcon = 1;
		for (int i = nk; i < w.length; i++) {  // rcon = 2^(i/nk) mod 0x11B
			int tp = w[i - 1];
			if (i % nk == 0) {
				tp = subInt32Bytes(rotateRight(tp, 24)) ^ (rcon << 24);
				rcon = multiply(rcon, (byte)0x02);
			} else if (nk > 6 && i % nk == 4)
				tp = subInt32Bytes(tp);
			w[i] = w[i - nk] ^ tp;
		}
		
		keySchedule = new byte[w.length / 4][BLOCK_LEN];
		for (int i = 0; i < keySchedule.length; i++) {
			for (int j = 0; j < keySchedule[i].length; j++)
				keySchedule[i][j] = (byte)(w[i * 4 + j / 4] >>> ((3 - j % 4) * 8));
		}
	}
	
	
	public void decryptBlock(byte[] msg, int off) {
		// Initial round
		byte[] temp0 = Arrays.copyOfRange(msg, off, off + BLOCK_LEN);
		addRoundKey(temp0, keySchedule[keySchedule.length - 1]);
		byte[] temp1 = new byte[BLOCK_LEN];
		for (int i = 0; i < 4; i++) {  // Shift rows inverse and sub bytes inverse
			for (int j = 0; j < 4; j++)
				temp1[i + j * 4] = SBOX_INVERSE[temp0[i + (j - i + 4) % 4 * 4] & 0xFF];
		}
		
		// Middle rounds
		for (int k = keySchedule.length - 2; k >= 1; k--) {
			addRoundKey(temp1, keySchedule[k]);
			for (int i = 0; i < BLOCK_LEN; i += 4) {  // Mix columns inverse
				for (int j = 0; j < 4; j++) {
					temp0[i + j] = (byte)(
					        multiply(temp1[i + (j + 0) % 4], (byte)0x0E) ^
					        multiply(temp1[i + (j + 1) % 4], (byte)0x0B) ^
					        multiply(temp1[i + (j + 2) % 4], (byte)0x0D) ^
					        multiply(temp1[i + (j + 3) % 4], (byte)0x09));
				}
			}
			for (int i = 0; i < 4; i++) {  // Shift rows inverse and sub bytes inverse
				for (int j = 0; j < 4; j++)
					temp1[i + j * 4] = SBOX_INVERSE[temp0[i + (j - i + 4) % 4 * 4] & 0xFF];
			}
		}
		
		// Final round
		addRoundKey(temp1, keySchedule[0]);
		
		System.arraycopy(temp1, 0, msg, off, temp1.length);
	}
	
	
	private static void addRoundKey(byte[] block, byte[] key) {
		for (int i = 0; i < BLOCK_LEN; i++)
			block[i] ^= key[i];
	}
	
	
	/* Utilities */
	
	private static byte[] SBOX = new byte[256];
	private static byte[] SBOX_INVERSE = new byte[256];
	
	// Initialize the S-box and inverse
	static {
		for (int i = 0; i < 256; i++) {
			byte tp = reciprocal((byte)i);
			byte s = (byte)(tp ^ rotateByteLeft(tp, 1) ^ rotateByteLeft(tp, 2) ^ rotateByteLeft(tp, 3) ^ rotateByteLeft(tp, 4) ^ 0x63);
			SBOX[i] = s;
			SBOX_INVERSE[s & 0xFF] = (byte)i;
		}
	}
	
	
	private static byte multiply(byte x, byte y) {
		// Russian peasant multiplication
		byte z = 0;
		for (int i = 0; i < 8; i++) {
			z ^= x * ((y >>> i) & 1);
			x = (byte)((x << 1) ^ (((x >>> 7) & 1) * 0x11B));
		}
		return z;
	}
	
	
	private static byte reciprocal(byte x) {
		if (x == 0)
			return 0;
		else {
			for (byte y = 1; y != 0; y++) {
				if (multiply(x, y) == 1)
					return y;
			}
			throw new AssertionError();
		}
	}
	
	
	private static byte rotateByteLeft(byte x, int y) {
		if (y < 0 || y >= 8)
			throw new IllegalArgumentException("Input out of range");
		return (byte)((x << y) | ((x & 0xFF) >>> (8 - y)));
	}
	
	
	private static int subInt32Bytes(int x) {
		return (SBOX[x >>> 24 & 0xFF] & 0xFF) << 24 |
		       (SBOX[x >>> 16 & 0xFF] & 0xFF) << 16 |
		       (SBOX[x >>>  8 & 0xFF] & 0xFF) <<  8 |
		       (SBOX[x >>>  0 & 0xFF] & 0xFF) <<  0;
	}
	
}
