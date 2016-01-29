#ifndef _SHA256_H
#define _SHA256_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif
/*
@doc CRYPTO


*/

/* @struct sha256_context |
container for intermediate sha256 results
<nl>Overview: <l Crypto Utilities>
*/
typedef struct
{
    uint32 total[2];
    uint32 state[8];
    uint8 buffer[64];
}
sha256_context;

void sha256_starts( sha256_context *ctx );
void sha256_update( sha256_context *ctx, uint8 *input, uint32 length );
void sha256_finish( sha256_context *ctx, uint8 digest[32] );

void Sha256String(char *str,unsigned char output[32]);
void Sha256HexString(char *str,char output[65]);

#endif /* sha256.h */
