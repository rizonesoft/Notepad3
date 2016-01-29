/**
 * rijndael-api-fst.c
 *
 * @version 2.9 (December 2000)
 *
 * Optimised ANSI C code for the Rijndael cipher (now AES)
 *
 * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Acknowledgements:
 *
 * We are deeply indebted to the following people for their bug reports,
 * fixes, and improvement suggestions to this implementation. Though we
 * tried to list all contributions, we apologise in advance for any
 * missing reference.
 *
 * Andrew Bales <Andrew.Bales@Honeywell.com>
 * Markus Friedl <markus.friedl@informatik.uni-erlangen.de>
 * John Skodon <skodonj@webquill.com>
 */
/* @doc CRYPTO

  None of the functinality has been changed, but some names and definitions
  have been tweaked for compatibility with the local environment.

*/
#include <windows.h>
//#include "helpers.h"
//#include "appreg.h"
//#include "resource.h"
#include <stdio.h>
#include "crypto.h"
#include "sha-256.h"
#include "rijndael-alg-fst.h"
#include "rijndael-api-fst.h"
/* @func
 Generate a 256 bit AES key from a passphrase, using reasonably acceptable
 procedures to obfustate the original passphrase.  The resulting key can be
 used for AES encryption by <f AES_setup>
<nl>Overview: <l Crypto Utilities>
 */
void AES_keygen(char *passphrase,	//* @parm the ascii passphrase
        BYTE key[32])		//* @parm the result key
{
  Sha256String(passphrase,key);
}
/* @func
 prepare an AES key for use.  TheKey is a string of hex digits,
 keyLen is the length of the crypto key to generate, which ought
 to be 256 in normal circumstances.  If the key is already available
 in binary form, call <f AES_bin_setup>
<nl>Overview: <l Crypto Utilities>
*/
int AES_setup
  (AES_keyInstance *key,	// @parm the <t AES_keyInstance> to be initialized
  AES_MODES direction,	// @parm either <e AES_MODES.AES_DIR_ENCRYPT> or <e AES_MODES.AES_DIR_DECRYPT>
  int keyLen,				// @parm the length of the key in bits (better be 256)
  char *TheKey)			// @parm the key itself, a hex string
{
  int i;
  char *keyMat;
  u8 cipherKey[MAXKB];


  if (TheKey != NULL) {
    //strncpy(key->TheKey, TheKey, keyLen/4);
    memcpy_s(key->TheKey, AES_MAX_KEY_SIZE, TheKey, keyLen / 4);
  }

  /* initialize key schedule: */
  keyMat = key->TheKey;
  for (i = 0; i < keyLen/8; i++) {
    int t, v;

    t = *keyMat++;
    if ((t >= '0') && (t <= '9')) v = (t - '0') << 4;
    else if ((t >= 'a') && (t <= 'f')) v = (t - 'a' + 10) << 4;
    else if ((t >= 'A') && (t <= 'F')) v = (t - 'A' + 10) << 4;
    else return BAD_KEY_MAT;

    t = *keyMat++;
    if ((t >= '0') && (t <= '9')) v ^= (t - '0');
    else if ((t >= 'a') && (t <= 'f')) v ^= (t - 'a' + 10);
    else if ((t >= 'A') && (t <= 'F')) v ^= (t - 'A' + 10);
    else return BAD_KEY_MAT;

    cipherKey[i] = (u8)v;
  }
  return(AES_bin_setup(key,direction,keyLen,cipherKey));
}
/* @func
 lower level version of <f AES_setup> where the key is already 
 converted to binary.
*/
int AES_bin_setup	
  (AES_keyInstance *key,	// @parm the <t AES_keyInstance> to be initialized
  AES_MODES direction,	// @parm either <e AES_MODES.AES_DIR_ENCRYPT> or <e AES_MODES.AES_DIR_DECRYPT>
  int keyLen,				// @parm the length of the key in bits (better be 256)
  BYTE *cipherKey)		// @parm the key itself, keyLen/8 bytes
{
  if (key == NULL) {
    return BAD_KEY_INSTANCE;
  }

  if ((direction == AES_DIR_ENCRYPT) || (direction == AES_DIR_DECRYPT)) {
    key->direction = direction;
  } else {
    return BAD_KEY_DIR;
  }

  if ((keyLen == 128) || (keyLen == 192) || (keyLen == 256)) {
    key->keyLen = keyLen;
  } else {
    return BAD_KEY_MAT;
  }

  if (direction == AES_DIR_ENCRYPT) {
    key->Nr = rijndaelKeySetupEnc(key->rk, cipherKey, keyLen);
  } else {
    key->Nr = rijndaelKeySetupDec(key->rk, cipherKey, keyLen);
  }
  rijndaelKeySetupEnc(key->ek, cipherKey, keyLen);
  return TRUE;
}

/* @func
 Prepare a cipher object for use with one encyption steam.  This
 sets the cypher mode and int initial vector.   The initial vector
 can be any 16 bytes, and need not be kept secret.
<nl>Overview: <l Crypto Utilities>
  */
int AES_bin_cipherInit
  (AES_cipherInstance *cipher,	//@parm the <t AES_cipherInstance> to be set up
  AES_MODES mode,				//@parm the <t AES_MODES> to use, <e AES_MODES.AES_MODE_CBC> is recommended
  BYTE *IV)			//@parm the IV, any 16 bytes
{
  if ((mode == AES_MODE_ECB) || (mode == AES_MODE_CBC) || (mode == AES_MODE_CFB1)) {
    cipher->mode = mode;
  } else {
    return BAD_CIPHER_MODE;
  }
  if(IV!=NULL)
  {
    memcpy(cipher->IV,IV,AES_MAX_IV_SIZE);
  } else {
    memset(cipher->IV, 0, AES_MAX_IV_SIZE);
  }
  return TRUE;
}

/* @func
 Prepare a cipher object for use with one encyption stream.  This
 sets the cypher mode and int initial vector.   The initial vector
 can be any 16 bytes, and need not be kept secret.
<nl>Overview: <l Crypto Utilities>
  */
int AES_cipherInit
  (AES_cipherInstance *cipher,	//@parm the <t AES_cipherInstance> to be set up
  AES_MODES mode,				//@parm the <t AES_MODES> to use, <e AES_MODES.AES_MODE_CBC> is recommended
  char *IV)			//@parm the IV, ascii hex to define 16 bytes
{
  if ((mode == AES_MODE_ECB) || (mode == AES_MODE_CBC) || (mode == AES_MODE_CFB1)) {
    cipher->mode = mode;
  } else {
    return BAD_CIPHER_MODE;
  }
  if (IV != NULL) {
    int i;
    for (i = 0; i < AES_MAX_IV_SIZE; i++) {
      int t, j;

      t = IV[2*i];
      if ((t >= '0') && (t <= '9')) j = (t - '0') << 4;
      else if ((t >= 'a') && (t <= 'f')) j = (t - 'a' + 10) << 4;
      else if ((t >= 'A') && (t <= 'F')) j = (t - 'A' + 10) << 4;
      else return BAD_CIPHER_INSTANCE;

      t = IV[2*i+1];
      if ((t >= '0') && (t <= '9')) j ^= (t - '0');
      else if ((t >= 'a') && (t <= 'f')) j ^= (t - 'a' + 10);
      else if ((t >= 'A') && (t <= 'F')) j ^= (t - 'A' + 10);
      else return BAD_CIPHER_INSTANCE;

      cipher->IV[i] = (u8)j;
    }
  } else {
    memset(cipher->IV, 0, AES_MAX_IV_SIZE);
  }
  return TRUE;
}
/* @func
 Encrypt a block of data, using the provided key and cipher.  The block
 should be a multiple of 16 bytes long, The trailing bytes
 mod 16 to be ignored. In CBC mode, the cipher IV is updated to be ready
 to encrypt the next block.
<nl>Overview: <l Crypto Utilities>

  @rdesc number of bytes encrypted
  */
int AES_blockEncrypt
  (AES_cipherInstance *cipher, //@parm the current <t AES_cipherInstance>
   AES_keyInstance *key,	//@parm the current <t AES_keyInstance>
  BYTE *input,	// @parm the input data
  int inputLen,	// @parm the size of the input data
  BYTE *outBuffer) //@parm a buffer to receive the encrypted data
{
  int i, k, t, numBlocks;
  u8 block[16], *iv;

  if (cipher == NULL ||
    key == NULL ||
    key->direction == AES_DIR_DECRYPT) {
    return BAD_CIPHER_STATE;
  }
  if (input == NULL || inputLen <= 0) {
    return 0; /* nothing to do */
  }

  numBlocks = inputLen/16;

  switch (cipher->mode) {
  case AES_MODE_ECB:
    for (i = numBlocks; i > 0; i--) {
      rijndaelEncrypt(key->rk, key->Nr, input, outBuffer);
      input += 16;
      outBuffer += 16;
    }
    break;

  case AES_MODE_CBC:
    iv = cipher->IV;
    for (i = numBlocks; i > 0; i--) {
      ((u32*)block)[0] = ((u32*)input)[0] ^ ((u32*)iv)[0];
      ((u32*)block)[1] = ((u32*)input)[1] ^ ((u32*)iv)[1];
      ((u32*)block)[2] = ((u32*)input)[2] ^ ((u32*)iv)[2];
      ((u32*)block)[3] = ((u32*)input)[3] ^ ((u32*)iv)[3];
      rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
      iv = outBuffer;
      input += 16;
      outBuffer += 16;
    }
    // copy the iv for proper chaining to the next block
    if (numBlocks > 0)
          memcpy(cipher->IV,outBuffer-AES_MAX_IV_SIZE,AES_MAX_IV_SIZE);
    break;

    case AES_MODE_CFB1:
    iv = cipher->IV;
        for (i = numBlocks; i > 0; i--) {
      memcpy(outBuffer, input, 16);
            for (k = 0; k < 128; k++) {
        rijndaelEncrypt(key->ek, key->Nr, iv, block);
                outBuffer[k >> 3] ^= (block[0] & 0x80U) >> (k & 7);
                for (t = 0; t < 15; t++) {
                  iv[t] = (iv[t] << 1) | (iv[t + 1] >> 7);
                }
                iv[15] = (iv[15] << 1) | ((outBuffer[k >> 3] >> (7 - (k & 7))) & 1);
            }
            outBuffer += 16;
            input += 16;
        }
        break;

  default:
    return BAD_CIPHER_STATE;
  }

  return 16*numBlocks;
}

/* @func
  Encrypt data using the current key and cipher, using RFC 2040-like padding,
  which puts the count of "pad" bytes in each pad byte.  If you are encoding
  multiple blocks, all but the last should be multiples of 16 in size and
  be encrypted using <f AES_encrypt>.  This last block will be padded to
  fill out the block, or if the original was already a multiple of 16, a
  full 16 bytes of padding will be added.   Conventional use is to always 
  provide at least one pad byte. If the original file was
  a multiple of 16, supply a block of 16 pad bytes so the decrypted data
  can be exactly the size of the encrypted data.  In CBC mode, the cipher IV is updated to be ready
 to encrypt the next block, even though there will normally not be a next block.
<nl>Overview: <l Crypto Utilities>
  @rdesc	length in octets (not bits) of the encrypted output buffer.
 */
int AES_padEncrypt
  (AES_cipherInstance *cipher, //@parm the current <t AES_cipherInstance>
   AES_keyInstance *key,	//@parm the current <t AES_keyInstance>
  BYTE *input,	// @parm the input data
  int inputOctets, // @parm the size of the input data
  BYTE *outBuffer) //@parm a buffer to receive the encrypted data
{
  int i, numBlocks, padLen;
  u8 block[16], *iv;

  if (cipher == NULL ||
    key == NULL ||
    key->direction == AES_DIR_DECRYPT) {
    return BAD_CIPHER_STATE;
  }
  if (input == NULL || inputOctets < 0) {
    return 0; /* nothing to do */
  }

  numBlocks = inputOctets/16;

  switch (cipher->mode) {
  case AES_MODE_ECB:
    for (i = numBlocks; i > 0; i--) {
      rijndaelEncrypt(key->rk, key->Nr, input, outBuffer);
      input += 16;
      outBuffer += 16;
    }
    padLen = 16 - (inputOctets - 16*numBlocks);
    if((padLen <= 0) || (padLen > 16))
      { BUG1("Padding must be 1-16, is %d",padLen); 
      }
    memcpy(block, input, 16 - padLen);
    memset(block + 16 - padLen, padLen, padLen);
    rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
    break;

  case AES_MODE_CBC:
    iv = cipher->IV;
    for (i = numBlocks; i > 0; i--) {
      ((u32*)block)[0] = ((u32*)input)[0] ^ ((u32*)iv)[0];
      ((u32*)block)[1] = ((u32*)input)[1] ^ ((u32*)iv)[1];
      ((u32*)block)[2] = ((u32*)input)[2] ^ ((u32*)iv)[2];
      ((u32*)block)[3] = ((u32*)input)[3] ^ ((u32*)iv)[3];
      rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
      iv = outBuffer;
      input += 16;
      outBuffer += 16;
    }
    padLen = 16 - (inputOctets - 16*numBlocks);
    if((padLen <= 0) || (padLen > 16))
      { BUG1("Padding must be 1-16, is %d",padLen); 
      }
    for (i = 0; i < 16 - padLen; i++) {
      block[i] = input[i] ^ iv[i];
    }
    for (i = 16 - padLen; i < 16; i++) {
      block[i] = (BYTE)padLen ^ iv[i];
    }
    rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
    // set for chaining to the next block, even though there will normally not be one
        memcpy(cipher->IV,outBuffer,AES_MAX_IV_SIZE);
    break;

  default:
    return BAD_CIPHER_STATE;
  }

  return 16*(numBlocks + 1);
}
/* @func
Decrypt a block of data using the supplied key and cipher.  The block
should be a multiple of 16; the trailing bytes mod 16 are ignored.
In CBC mode, the IV of the cypher is updated to be ready to decrypt the
next block.
<nl>Overview: <l Crypto Utilities>
@rdesc the number of bytes decrypted
  */
int AES_blockDecrypt
  (AES_cipherInstance *cipher,	//@parm the current <t AES_cipherInstance>
  AES_keyInstance *key,	//@parm the current <t AES_keyInstance>
  BYTE *input,	//@parm the input encrypted data
  int inputLen,	//@parm the size of the input
  BYTE *outBuffer) //@parm a buffer to receive the decrypted buffer
{	int lim=32;
  int i, k, t, numBlocks;
  u8 block[16], *iv;

  if (cipher == NULL ||
    key == NULL ||
    cipher->mode != AES_MODE_CFB1 && key->direction == AES_DIR_ENCRYPT) {
    return BAD_CIPHER_STATE;
  }
  if (input == NULL || inputLen <= 0) {
    return 0; /* nothing to do */
  }

  numBlocks = inputLen/16;

  switch (cipher->mode) {
  case AES_MODE_ECB:
    for (i = numBlocks; i > 0; i--) {
      rijndaelDecrypt(key->rk, key->Nr, input, outBuffer);
      input += 16;
      outBuffer += 16;
    }
    break;

  case AES_MODE_CBC:
    iv = cipher->IV;
    for (i = numBlocks; i > 0; i--) 
    {
      rijndaelDecrypt(key->rk, key->Nr, input, block);
      ((u32*)block)[0] ^= ((u32*)iv)[0];
      ((u32*)block)[1] ^= ((u32*)iv)[1];
      ((u32*)block)[2] ^= ((u32*)iv)[2];
      ((u32*)block)[3] ^= ((u32*)iv)[3];
      memcpy(cipher->IV, input, 16);
      memcpy(outBuffer, block, 16);
      input += 16;
      outBuffer += 16;
    }
    break;

    case AES_MODE_CFB1:
    iv = cipher->IV;
        for (i = numBlocks; i > 0; i--) {
      memcpy(outBuffer, input, 16);
            for (k = 0; k < 128; k++) {
        rijndaelEncrypt(key->ek, key->Nr, iv, block);
                for (t = 0; t < 15; t++) {
                  iv[t] = (iv[t] << 1) | (iv[t + 1] >> 7);
                }
                iv[15] = (iv[15] << 1) | ((input[k >> 3] >> (7 - (k & 7))) & 1);
                outBuffer[k >> 3] ^= (block[0] & 0x80U) >> (k & 7);
            }
            outBuffer += 16;
            input += 16;
        }
        break;

  default:
    return BAD_CIPHER_STATE;
  }

  return 16*numBlocks;
}
/* @func
Decrypt a block of data using the supplied key and cipher.  The block
must be a multiple of 16 bytes, and should be padded in the manner of 
<f AES_padEncrypt> the trailing bytes mod 16 are ignored.  In CBC
mode, the IV is updated to be ready to decrypt the next block, even
thought there normally will not be any more blocks.
<nl>Overview: <l Crypto Utilities>
@rdesc the number of bytes decrypted
  */

int AES_padDecrypt
  (AES_cipherInstance *cipher,	//@parm the current <t AES_cipherInstance>
  AES_keyInstance *key,	//@parm the current <t AES_keyInstance>
  BYTE *input,	//@parm the input encrypted data
  int inputOctets,	//@parm the size of the input
  BYTE *outBuffer) //@parm a buffer to receive the decrypted buffer 
{
  int i, numBlocks, padLen;
  u8 block[16];

  if (cipher == NULL ||
    key == NULL ||
    key->direction == AES_DIR_ENCRYPT) {
    return BAD_CIPHER_STATE;
  }
  if (input == NULL || inputOctets <= 0) {
    return 0; /* nothing to do */
  }
  if (inputOctets % 16 != 0) {
    return BAD_DATA;
  }

  numBlocks = inputOctets/16;

  switch (cipher->mode) {
  case AES_MODE_ECB:
    /* all blocks but last */
    for (i = numBlocks - 1; i > 0; i--) {
      rijndaelDecrypt(key->rk, key->Nr, input, outBuffer);
      input += 16;
      outBuffer += 16;
    }
    /* last block */
    rijndaelDecrypt(key->rk, key->Nr, input, block);
    padLen = block[15];
    if (padLen >= 16) {
      return BAD_DATA;
    }
    for (i = 16 - padLen; i < 16; i++) {
      if (block[i] != padLen) {
        return BAD_DATA;
      }
    }
    memcpy(outBuffer, block, 16 - padLen);
    break;

  case AES_MODE_CBC:
    /* all blocks but last */
    for (i = numBlocks - 1; i > 0; i--) {
      rijndaelDecrypt(key->rk, key->Nr, input, block);
      ((u32*)block)[0] ^= ((u32*)cipher->IV)[0];
      ((u32*)block)[1] ^= ((u32*)cipher->IV)[1];
      ((u32*)block)[2] ^= ((u32*)cipher->IV)[2];
      ((u32*)block)[3] ^= ((u32*)cipher->IV)[3];
      memcpy(cipher->IV, input, 16);
      memcpy(outBuffer, block, 16);
      input += 16;
      outBuffer += 16;
    }
    /* last block */
    rijndaelDecrypt(key->rk, key->Nr, input, block);
    ((u32*)block)[0] ^= ((u32*)cipher->IV)[0];
    ((u32*)block)[1] ^= ((u32*)cipher->IV)[1];
    ((u32*)block)[2] ^= ((u32*)cipher->IV)[2];
    ((u32*)block)[3] ^= ((u32*)cipher->IV)[3];
    memcpy(cipher->IV, input, 16);
    padLen = block[15];
    if (padLen <= 0 || padLen > 16) {
      return BAD_DATA;
    }
    for (i = 16 - padLen; i < 16; i++) {
      if (block[i] != padLen) {
        return BAD_DATA;
      }
    }
    memcpy(outBuffer, block, 16 - padLen);
    break;

  default:
    return BAD_CIPHER_STATE;
  }

  return 16*numBlocks - padLen;
}

#ifdef INTERMEDIATE_VALUE_KAT
/**
 *	cipherUpdateRounds:
 *
 *	Encrypts/Decrypts exactly one full block a specified number of rounds.
 *	Only used in the Intermediate Value Known Answer Test.
 *
 *	Returns:
 *		TRUE - on success
 *		BAD_CIPHER_STATE - cipher in bad state (e.g., not initialized)
 */
int cipherUpdateRounds(AES_cipherInstance *cipher, AES_keyInstance *key,
    BYTE *input, int inputLen, BYTE *outBuffer, int rounds) {
  u8 block[16];

  if (cipher == NULL || key == NULL) {
    return BAD_CIPHER_STATE;
  }

  memcpy(block, input, 16);

  switch (key->direction) {
  case AES_DIR_ENCRYPT:
    rijndaelEncryptRound(key->rk, key->Nr, block, rounds);
    break;

  case AES_DIR_DECRYPT:
    rijndaelDecryptRound(key->rk, key->Nr, block, rounds);
    break;

  default:
    return BAD_KEY_DIR;
  }

  memcpy(outBuffer, block, 16);

  return TRUE;
}
#endif /* INTERMEDIATE_VALUE_KAT */
