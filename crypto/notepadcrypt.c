/*
*
* Distributed under the terms of the GNU General Public License,
* see License.txt for details.
*
* Author: Dave Dyer  ddyer@real-me.net  Oct/2005
*
* bug fix 5/2013 by Nayuki Minase <nayuki@eigenstate.org>
*   encrypted file was corrupted for source files of 16-31 bytes.
*   also http://nayuki.eigenstate.org/page/notepadcrypt-format-decryptor-java
*/
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "rijndael-api-fst.h"
#include "crypto.h"

#define BLOCKSIZE  (64 * 1024) // the optimal buffer size for sequential I/O on Windows NT/2k/XP
typedef struct AES_file
{	FILE *file;			
  AES_cipherInstance cipher;
  AES_keyInstance key;
  BOOL encrypted;
  BYTE buffer[BLOCKSIZE];
  long bytesleft;
  long buffer_index;
  long buffer_end;
} AES_file;


void gen_iv(unsigned char *buf, int size)
{
    while(--size >= 0) buf[size] = size;//+= CM_random();
}
/* @func
open a file, possibly encrypted using notepad2 format, for reading and decryption.  
@rdesc 0 for success
*/
long ROpen_AES
  (char * name,	//@parm the file to open
   AES_file * fp, //@parm the <t AES_file> object to keep track of the open file
   char *filekey, //@parm the file's passphrase, or an empty string, or NULL
   char *masterkey//@parm the file's master passphrase, or  an empty string, or NULL
   )
{
  FILE *file = NULL;
  if (fopen_s(&file, name, "rb") != 0) { printf("File %s can't be opened\n", name); return(1); }
  fp->file=file;
  fp->buffer_index=0;
  fp->buffer_end=0;
  fp->bytesleft=0;
  fp->encrypted=FALSE;
  // get the file length
  fseek(file,0,SEEK_END); 
  fp->bytesleft=ftell(file); 
  fseek(file,0,SEEK_SET); 

  // read the maximum preable size, so we will have an even number of encrypted blocks
  // left over if this is an encrypted file.
  fp->buffer_end = (long)fread(fp->buffer,1,MASTER_KEY_OFFSET,fp->file);
  fp->bytesleft -= fp->buffer_end;
  if(fp->buffer_end>=MASTER_KEY_OFFSET)
  {	unsigned long *lbuf = (unsigned long *)&fp->buffer;
    BYTE binFileKey[KEY_BYTES];
    BOOL hasFileKey=FALSE;

    //possibly encrypted
    if(lbuf[0]==PREAMBLE)
    {
    switch(lbuf[1])
    {
    default: 
      printf("File %s is encrypted with an unsupported format: %d",name,lbuf[1]);
      fclose(file);
      return(1);
    case MASTERKEY_FORMAT:
      // read the masterkey block
      if(fread(fp->buffer+fp->buffer_end,1,KEY_BYTES+AES_MAX_IV_SIZE,fp->file)
              !=(KEY_BYTES+AES_MAX_IV_SIZE))
      { fclose(fp->file);
        return(2);	// short file
      }
      fp->buffer_index = fp->buffer_end;
      fp->bytesleft -= (KEY_BYTES+AES_MAX_IV_SIZE);

      if(masterkey && *masterkey)
      {
      BYTE binMasterKey[KEY_BYTES];
      AES_keygen(masterkey,binMasterKey);
      AES_bin_setup(&fp->key,AES_DIR_DECRYPT,KEY_BYTES*8,binMasterKey);
      AES_bin_cipherInit(&fp->cipher,AES_MODE_CBC,&fp->buffer[MASTER_KEY_OFFSET]);
      AES_blockDecrypt(&fp->cipher,&fp->key,&fp->buffer[MASTER_KEY_OFFSET+AES_MAX_IV_SIZE],sizeof(binFileKey),binFileKey);
      hasFileKey=TRUE;
      }
      else
      if(filekey && *filekey)
      {
      AES_keygen(filekey,binFileKey);
      fp->buffer_index=fp->buffer_end;
      hasFileKey=TRUE;
      }
      break;
    case FILEKEY_FORMAT:
      if(filekey && *filekey)
      {
      AES_keygen(filekey,binFileKey);
      fp->buffer_index=fp->buffer_end;
      hasFileKey=TRUE;
      }
      break;
    }
    if(hasFileKey)
    {	fp->encrypted=TRUE;
      AES_bin_setup(&fp->key,AES_DIR_DECRYPT,KEY_BYTES*8,binFileKey);
      AES_bin_cipherInit(&fp->cipher,AES_MODE_CBC,&fp->buffer[PREAMBLE_SIZE]);
      return(0);
    }
    printf("File %s is encrypted, but no suitable passphrase is available",
        name);
    fclose(file);
    return(3);
    }
  }
  return(0);	// file is too short to be encrypted
}
/* @func
 encrypt infile to outfile, using filephrase to generate the key,
 and optionally using masterphrase as the master key
*/
int encrypt(char *infile,char *outfile,char *filephrase,char *masterphrase)
{	int err=0;
FILE *in = NULL;
if (fopen_s(&in, infile, "rb")  != 0) { printf("input file %s can't be opened\1", infile); err++; }
  else
  {
    FILE *out = NULL;
    if (fopen_s(&out, outfile, "wb") != 0) { printf("output file %s can't be opened\n", outfile); err++; }
  else
  {	BYTE buffer[BLOCKSIZE];
    unsigned long preamble[] = { PREAMBLE, FILEKEY_FORMAT};
    BYTE iv[AES_MAX_IV_SIZE];
    BYTE filekey[KEY_BYTES];
    BOOL masterformat = masterphrase && *masterphrase;
    AES_cipherInstance cipher;
    AES_keyInstance key;

    if(masterformat) { preamble[1]=MASTERKEY_FORMAT; }

    gen_iv(iv,sizeof(iv));				// generate a random iv
    AES_keygen(filephrase,filekey);		// make key file passphrase
    fwrite(preamble,1,sizeof(preamble),out);	// write the preamble
    fwrite(iv,1,sizeof(iv),out);				// and the iv

    AES_bin_setup(&key,AES_DIR_ENCRYPT,KEY_BYTES*8,filekey);	// prepare the encryption
    AES_bin_cipherInit(&cipher,AES_MODE_CBC,iv);

    if(masterformat)
    {	// encrypt the file key with the masterkey and write it.
      BYTE masteriv[AES_MAX_IV_SIZE];
      BYTE masterkey[KEY_BYTES];
      BYTE encfilekey[KEY_BYTES];
      AES_cipherInstance mastercipher;
      AES_keyInstance mkey;
      
      AES_keygen(masterphrase,masterkey);			// generate the master key
      gen_iv(masteriv,sizeof(masteriv));			// and an iv for it
      AES_bin_setup(&mkey,AES_DIR_ENCRYPT,KEY_BYTES*8,masterkey);
      AES_bin_cipherInit(&mastercipher,AES_MODE_CBC,masteriv);
      // encrypt the file key using the master key
      AES_blockEncrypt(&mastercipher,&mkey,filekey,sizeof(filekey),encfilekey);
      fwrite(masteriv,1,sizeof(masteriv),out);
      fwrite(encfilekey,1,sizeof(encfilekey),out);
    }

    // now encrypt and output the actual data
    { long bytesread=0;
      long bytesencrypted=0;
    do {
      bytesread = (long)fread(buffer,1,sizeof(buffer),in);
      bytesencrypted=0;
      if(bytesread>0)
      {	bytesencrypted = AES_blockEncrypt(&cipher,&key,buffer,bytesread,buffer);
        fwrite(buffer,1,bytesencrypted,out);
      }
    } while((bytesread>0)&&(bytesencrypted==bytesread));
    // pad the last block
    bytesencrypted = AES_padEncrypt(&cipher,&key,buffer+bytesencrypted,(bytesread-bytesencrypted),buffer);
    fwrite(buffer,1,bytesencrypted,out);
    fclose(out);
    }
    }
  fclose(in);
  }
  return(err);
}
/* @func
 decrypt a file using filephrase or masterphrase.  If the file has a master key
 and masterphrase is supplied, masterphrase is used.  Otherwise filephrase.
 */
int decrypt(char *infile,char *outfile,char *filephrase,char *masterphrase)
{	AES_file in;
  int err=0;
  if(0==ROpen_AES(infile,&in,filephrase,masterphrase))
  {
    FILE *out = NULL;
    if (fopen_s(&out, outfile, "wb") == 0)
    {	while(in.bytesleft > 0)
      {	if(in.buffer_index<in.buffer_end) 
        { //write the data already available 
         fwrite(in.buffer+in.buffer_index,1,in.buffer_end-in.buffer_index,out);
        }
      // read and decrypt some more data
      {long sizeread = (long)fread(in.buffer,1,sizeof(in.buffer),in.file);
      if(sizeread<=0) 
        { printf("ran out of input data\n");
          in.bytesleft = 0;
          err++;
        }
       AES_blockDecrypt(&in.cipher,&in.key,in.buffer,sizeread,in.buffer);
       in.bytesleft -= sizeread;
       in.buffer_index = 0;
       in.buffer_end = sizeread;
       }
      }
      // now we just have one buffer containing some padding
      in.buffer_end -= in.buffer[in.buffer_end-1];
      fwrite(in.buffer+in.buffer_index,1,in.buffer_end-in.buffer_index,out);
      fclose(out);

    }
    fclose(in.file);
  }
  return(err);
}

int main(int argc, char *argv[])
{	int err=0;
    if(argc >= 4 )
    {	long idx=1;
        char *op = argv[idx++];
    char *infile = argv[idx++];
    char *outfile = argv[idx++];
    char *pass1 = argv[idx++];
    char *pass2 = (idx<argc) ? argv[idx++] : "";

    if(_stricmp(op,"EF")==0)
    {	// encrypt with file passphrase only
      encrypt(infile,outfile,pass1,"");
    }
    else if(_stricmp(op,"DF")==0)
    {	// decrypt using the file passphrase
      decrypt(infile,outfile,pass1,"");
    }
    else if((_stricmp(op,"EM")==0) && (*pass2!=(char)0))
    {	// encrypt using file and master passphrases
      encrypt(infile,outfile,pass1,pass2);
    }
    else if(_stricmp(op,"DM")==0)
    {	// decrypt using the master passphrase
      decrypt(infile,outfile,"",pass1);
    }
    else { err++; }
    }
  else
  {	err++;
  }
    if(err)
  {printf("notepadcrypt - command line file encrypt/decrypt compatible with notepad2\n"
  "Usage: notepadcrypt {ef em df dm} source destination {passphrase} {passphrase}\n\n");
  }

    return err;
}

