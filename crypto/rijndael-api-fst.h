/**
 * rijndael-api-fst.h
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
*/
#ifndef __RIJNDAEL_API_FST_H
#define __RIJNDAEL_API_FST_H


#include "rijndael-alg-fst.h"

/*  @enum AES_MODES |
modes used by AES encryption
<nl>Overview: <l Crypto Utilities>
*/
typedef enum 
{
     AES_DIR_ENCRYPT   =        0, /*  @emem Are we encrpyting?  */
     AES_DIR_DECRYPT   =        1, /*  @emem Are we decrpyting?  */
     AES_MODE_ECB      =        1, /*  @emem Are we ciphering in ECB mode?   */
     AES_MODE_CBC      =        2, /*  @emem Are we ciphering in CBC mode?   */
     AES_MODE_CFB1     =        3, /*  @emem Are we ciphering in 1-bit CFB mode? */
     AES_BITSPERBLOCK  =      128 /* @emem Default number of bits in a cipher block */
} AES_MODES;

/* @enum  AES_ERROR_CODES |
error codes used by AES encryption
<nl>Overview: <l Crypto Utilities>
*/ 
typedef enum 
{
     BAD_KEY_DIR       =   -1, /* @emem Key direction is invalid, e.g., unknown value */
     BAD_KEY_MAT       =   -2, /* @emem Key material not of correct length */
     BAD_KEY_INSTANCE  =   -3, /* @emem Key passed is not valid */
     BAD_CIPHER_MODE   =   -4, /* @emem Params struct passed to cipherInit invalid */
     BAD_CIPHER_STATE  =   -5, /* @emem Cipher in wrong state (e.g., not initialized) */
     BAD_BLOCK_LENGTH  =   -6, /* @emem bad block length */
     BAD_CIPHER_INSTANCE = -7, /* @emem bad cypher instance */
     BAD_DATA            = -8, /* @emem Data contents are invalid, e.g., invalid padding */
     BAD_OTHER           = -9 /*  @emem Unknown error */
} AES_ERROR_CODES;
/* @enum  AES_CONSTANTS |
misc constants used by AES encryption
<nl>Overview: <l Crypto Utilities>
*/
typedef enum 
{
     AES_MAX_KEY_SIZE    =     64, /* @emem # of ASCII char's needed to represent a key in hex */
     AES_MAX_IV_SIZE     =     16 /* @emem # bytes needed to represent an IV  */
} AES_CONSTANTS;

/*  @struct AES_keyInstance |
The structure for AES key information.  The one key can be used to
encrypt or decrypt many files.
<nl>Overview: <l Crypto Utilities>
*/

typedef struct 
{
    AES_MODES  direction;                /* Key used for encrypting or decrypting? */
    int   keyLen;                   /* Length of the key  */
    char  TheKey[AES_MAX_KEY_SIZE+1];  /* Raw key data in ASCII, e.g., user input or KAT values */
	int   Nr;                       /* key-length-dependent number of rounds */
	u32   rk[4*(MAXNR + 1)];        /* key schedule */
	u32   ek[4*(MAXNR + 1)];        /* CFB1 key schedule (encryption only) */
} AES_keyInstance;



/* @struct  AES_cypherInstance |
The structure for AES encryption/decryption steams.  A different
instance of this must be associated with each stream being encrypted
of decrypted.
<nl>Overview: <l Crypto Utilities>
*/

typedef struct 
{                    /* changed order of the components */
    AES_MODES  mode;                     /* MODE_ECB, MODE_CBC, or MODE_CFB1 */
    BYTE  IV[AES_MAX_IV_SIZE];          /* A possible Initialization Vector for ciphering */
} AES_cipherInstance;





/*  Function prototypes  */
void AES_keygen(char *passphrase, BYTE key[32]);

int AES_setup(AES_keyInstance *key, AES_MODES direction, int keyLen, char *xkeyMaterial);

int AES_bin_setup(AES_keyInstance *key, AES_MODES direction, int keyLen, BYTE *xkeyMaterial);

int AES_cipherInit(AES_cipherInstance *cipher, AES_MODES mode, char *IV);

int AES_bin_cipherInit(AES_cipherInstance *cipher, AES_MODES mode, BYTE *IV);

int AES_blockEncrypt(AES_cipherInstance *cipher, AES_keyInstance *key,
        BYTE *input, int inputLen, BYTE *outBuffer);

int AES_padEncrypt(AES_cipherInstance *cipher, AES_keyInstance *key,
		BYTE *input, int inputOctets, BYTE *outBuffer);

int AES_blockDecrypt(AES_cipherInstance *cipher, AES_keyInstance *key,
        BYTE *input, int inputLen, BYTE *outBuffer);

int AES_padDecrypt(AES_cipherInstance *cipher, AES_keyInstance *key,
		BYTE *input, int inputOctets, BYTE *outBuffer);

#ifdef INTERMEDIATE_VALUE_KAT
int cipherUpdateRounds(AES_cipherInstance *cipher, AES_keyInstance *key,
        BYTE *input, int inputLen, BYTE *outBuffer, int Rounds);
#endif /* INTERMEDIATE_VALUE_KAT */

#endif /* __RIJNDAEL_API_FST_H */
