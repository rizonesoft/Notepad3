
/**
*
* Distributed under the terms of the GNU General Public License,
* see License.txt for details.
*
* Author: Dave Dyer  ddyer@real-me.net  Oct/2005
*
adds the option to encrypt and decrypt files, using stong AES encryption,
optionally with master key

see ecryption-doc.txt for details

*/
#include <windows.h>
#include <time.h>
#include "..\src\Dialogs.h"
#include "..\src\Helpers.h"
#include "..\src\resource.h"
#include "rijndael-api-fst.h"
#include "crypto.h"

#define WKEY_LEN 256
#define KEY_LEN  512
#define PAD_SLOP 16

BOOL  useFileKey = FALSE;			// file should be encrypted
char fileKey[KEY_LEN] = { 0 };		// ascii passphrase for the file key
WCHAR unicodeFileKey[WKEY_LEN] = { 0 };	// unicode file passphrase
BOOL  useMasterKey = FALSE;		    // file should have a master key
char masterKey[KEY_LEN] = { 0 };		// ascii passphrase for the master key
WCHAR unicodeMasterKey[WKEY_LEN] = { 0 };	// unicode master passphrase
BYTE  binFileKey[KEY_BYTES];			// the encryption key in for the file
BOOL  hasBinFileKey = FALSE;
BYTE  masterFileKey[KEY_BYTES];		// file key encrypted with the master key
BYTE  masterFileIV[AES_MAX_IV_SIZE];	// the iv for the master key
BOOL  hasMasterFileKey = FALSE;
BOOL  masterKeyAvailable = FALSE;		// information for the passphrase dialog box

void ResetEncryption()
{
  masterKeyAvailable = FALSE;
  hasMasterFileKey = FALSE;
  hasBinFileKey = FALSE;
  useMasterKey = FALSE;
  useFileKey = FALSE;
  memset(fileKey, 0, sizeof(fileKey));
  memset(masterKey, 0, sizeof(masterKey));
  memset(binFileKey, 0, sizeof(binFileKey));
  memset(unicodeFileKey, 0, sizeof(unicodeFileKey));
  memset(unicodeMasterKey, 0, sizeof(unicodeMasterKey));
  memset(masterFileKey, 0, sizeof(masterFileKey));
  memset(masterFileIV, 0, sizeof(masterFileIV));
}
//=============================================================================

//
// copy a unicode string to a regular string, but keep the same 
// result string for simple, non-unicode characters.
// this is used to convert a unicode password to a byte stream compatible with an ascii password
//
void unicodeStringCpy(char *dest, WCHAR *src, int destSize)
{
  int sidx = 0;
  int didx = 0;
  int destLim = destSize - 1;
  while ((src[sidx] != 0) && (didx < destLim))
  {
    WCHAR c = src[sidx++];
    char clow = (char)(c & 0xff);
    if (clow != 0) { dest[didx++] = clow; }		// ignore zeros in the low order part
    if (((c & 0xff00) != 0) && (didx < destLim))		// ignore zeros in the high order part
    {
      dest[didx++] = (char)((c >> 8) & 0xff);
    }
  }
  dest[didx++] = (char)0;
}
//=============================================================================

// helper function for set focus to editbox
void SetDialogFocus(HWND hDlg, HWND hwndControl)
{
  PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwndControl, TRUE);
}


//=============================================================================
// helper function for the "set passphrase" dialog before output
// the complication in this version is that the incoming text is unicode. We deal
// with it by converting the unicode to an ascii compatible byte stream, so the
// caller (and hence the rest of the encryption) doesn't know unicode was involved.
INT_PTR CALLBACK SetKeysDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {

  case WM_INITDIALOG:
  {
    SetDlgItemText(hDlg, IDC_EDIT1, unicodeFileKey);
    SetDlgItemText(hDlg, IDC_EDIT2, unicodeMasterKey);
    ShowWindow(GetDlgItem(hDlg, IDC_CHECK3), hasMasterFileKey);
    CheckDlgButton(hDlg, IDC_CHECK3, hasMasterFileKey ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK2, hasBinFileKey | useFileKey ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK1, useMasterKey ? BST_CHECKED : BST_UNCHECKED);
    CenterDlgInParent(hDlg);
    // Don't use: SetFocus( GetDlgItem( hDlg, IDC_EDIT1 ) );
    SetDialogFocus(hDlg, GetDlgItem(hDlg, IDC_EDIT1));
  }

  return TRUE;
  break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {

    case IDOK:
    {
      BOOL useMas = IsDlgButtonChecked(hDlg, IDC_CHECK1) == BST_CHECKED;
      BOOL useFil = IsDlgButtonChecked(hDlg, IDC_CHECK2) == BST_CHECKED;
      BOOL reuseMas = IsDlgButtonChecked(hDlg, IDC_CHECK3) == BST_CHECKED;
      WCHAR newFileKey[WKEY_LEN] = { 0 };
      WCHAR newMasKey[WKEY_LEN] = { 0 };
      hasMasterFileKey &= reuseMas;
      GetDlgItemText(hDlg, IDC_EDIT1, newFileKey, sizeof(newFileKey));
      GetDlgItemText(hDlg, IDC_EDIT2, newMasKey, sizeof(newMasKey));
      useFileKey = !((newFileKey[0] <= ' ') || !useFil);
      useMasterKey = !((newMasKey[0] <= ' ') || !useMas);
      //@@@lstrcpyn(fileKey, newFileKey, WKEY_LEN);
      //@@@lstrcpyn(masterKey, newMasKey, WKEY_LEN);
      memcpy(unicodeFileKey, newFileKey, sizeof(unicodeFileKey));
      memcpy(unicodeMasterKey, newMasKey, sizeof(unicodeMasterKey));
      unicodeStringCpy(fileKey, unicodeFileKey, sizeof(fileKey));
      unicodeStringCpy(masterKey, unicodeMasterKey, sizeof(masterKey));
      EndDialog(hDlg, IDOK);
      return(TRUE);
    }

    break;

    case IDC_EDIT1:
    {
      WCHAR newFileKey[WKEY_LEN] = { 0 };
      GetDlgItemText(hDlg, IDC_EDIT1, newFileKey, sizeof(newFileKey));
      CheckDlgButton(hDlg, IDC_CHECK2, (newFileKey[0] <= ' ') ? BST_UNCHECKED : BST_CHECKED);
    }

    break;

    case IDC_EDIT2:
    {
      WCHAR newMasKey[WKEY_LEN] = { 0 };
      GetDlgItemText(hDlg, IDC_EDIT2, newMasKey, sizeof(newMasKey));
      {
        BOOL newuse = (newMasKey[0] > ' ');	// no leading whitespace or empty passwords
        CheckDlgButton(hDlg, IDC_CHECK1, newuse ? BST_CHECKED : BST_UNCHECKED);

        if (newuse) { CheckDlgButton(hDlg, IDC_CHECK3, BST_UNCHECKED); }
      }
    }

    break;

    case IDC_CHECK3:  // check reuse, uncheck set new and inverse
    {
      BOOL reuseMas = IsDlgButtonChecked(hDlg, IDC_CHECK3) == BST_CHECKED;

      if (reuseMas) { CheckDlgButton(hDlg, IDC_CHECK1, reuseMas ? BST_UNCHECKED : BST_CHECKED); }
    }

    break;

    case IDC_CHECK1:
    {
      BOOL useMas = IsDlgButtonChecked(hDlg, IDC_CHECK1) == BST_CHECKED;

      if (useMas) { CheckDlgButton(hDlg, IDC_CHECK3, useMas ? BST_UNCHECKED : BST_CHECKED); }
    }

    break;

    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;

    }

    break;

  }

  return FALSE;

}
//
// helper for setting password when reading a file
// the complication in this version is that the incoming text is unicode. We deal
// with it by converting the unicode to an ascii compatible byte stream, so the
// caller (and hence the rest of the encryption) doesn't know unicode was involved.
//
INT_PTR CALLBACK GetKeysDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  switch (umsg)
  {

  case WM_INITDIALOG:
  {
    int vis = masterKeyAvailable ? SW_SHOW : SW_HIDE;
    ShowWindow(GetDlgItem(hDlg, IDC_STATICPW), vis);
    ShowWindow(GetDlgItem(hDlg, IDC_CHECK3), vis);
    //@@@SetDlgItemText( hDlg, IDC_EDIT3, fileKey );
    SetDlgItemText(hDlg, IDC_EDIT3, unicodeFileKey);
    CheckDlgButton(hDlg, IDC_CHECK3, BST_UNCHECKED);
    CenterDlgInParent(hDlg);
    // Don't use: SetFocus( GetDlgItem( hDlg, IDC_EDIT3 ) );
    SetDialogFocus(hDlg, GetDlgItem(hDlg, IDC_EDIT3));
  }

  return TRUE;
  break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {

    case IDOK:
    {

      BOOL useMas = (IsDlgButtonChecked(hDlg, IDC_CHECK3) == BST_CHECKED);
      WCHAR newKey[WKEY_LEN] = L"\0";
      GetDlgItemText(hDlg, IDC_EDIT3, newKey, sizeof(newKey));

      if (useMas)
      {
        //@@@lstrcpyn( masterKey, newKey, WKEY_LEN );
        memcpy(unicodeMasterKey, newKey, sizeof(unicodeMasterKey));
        unicodeStringCpy(masterKey, unicodeMasterKey, sizeof(masterKey));
        useFileKey = FALSE;
        useMasterKey = TRUE;
      }
      else
      {
        //lstrcpyn( fileKey, newKey, WKEY_LEN );
        memcpy(unicodeFileKey, newKey, sizeof(unicodeFileKey));
        unicodeStringCpy(fileKey, unicodeFileKey, sizeof(fileKey));
        useFileKey = TRUE;
        useMasterKey = FALSE;
      }

      EndDialog(hDlg, IDOK);

      return(TRUE);
      break;
    }

    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;

    }

    break;
  }

  return FALSE;

}


// set passphrases for output
BOOL GetFileKey(HWND hwnd)
{
  return (IDOK == DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PASSWORDS),
    GetParent(hwnd), SetKeysDlgProc, (LPARAM)hwnd));
}

// set passphrases for file being input
BOOL ReadFileKey(HWND hwnd, BOOL master)
{
  masterKeyAvailable = master;
  return (IDOK == DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_READPW),
    GetParent(hwnd), GetKeysDlgProc, (LPARAM)hwnd));
}



// read the file data, decrypt if necessary, return the result as a new allocation
BOOL ReadAndDecryptFile(HWND hwnd, HANDLE hFile, DWORD size, void** result, DWORD *resultlen)
{
  BOOL usedEncryption = FALSE;
  HANDLE rawhandle = *result; // GlobalAlloc(GPTR, size);
  char* rawdata = GlobalLock(rawhandle);
  unsigned long readsize = 0;
  BOOL bReadSuccess = ReadFile(hFile, rawdata, size, &readsize, NULL);

  // we read the file, check if it looks like our encryption format

  if (bReadSuccess && (readsize > (PREAMBLE_SIZE + AES_MAX_IV_SIZE)))
  {
    long *ldata = (long*)rawdata;

    if (ldata[0] == PREAMBLE)
    {
      long scheme = ldata[1];
      unsigned long code_offset = PREAMBLE_SIZE + AES_MAX_IV_SIZE;

      switch (scheme)
      {
      case MASTERKEY_FORMAT:
        code_offset += sizeof(masterFileKey) + sizeof(masterFileIV);
        // save the encrypted file key and IV.  They can be reused if the
        // passphrases are not changed.
        memcpy(masterFileIV, &rawdata[MASTER_KEY_OFFSET], sizeof(masterFileIV));
        memcpy(masterFileKey, &rawdata[MASTER_KEY_OFFSET + sizeof(masterFileIV)], sizeof(masterFileKey));
        hasMasterFileKey = TRUE;

        // fall through
      case FILEKEY_FORMAT:
      {
        BOOL haveFileKey = ReadFileKey(hwnd, scheme == MASTERKEY_FORMAT);

        if (useFileKey)
        {
          // use the file key to decode
          /*@@@
                      char ansiKey[KEY_LEN+1];
                      int len = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, fileKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                      ansiKey[len] = '\0';
                      AES_keygen( ansiKey, binFileKey );		// generate the encryption key from the passphrase
                      */
          AES_keygen(fileKey, binFileKey);		// generate the encryption key from the passphrase
          hasBinFileKey = TRUE;
        }
        else if ((scheme == MASTERKEY_FORMAT) && useMasterKey)
        {	// use the master key to recover the file key
          BYTE binMasterKey[KEY_BYTES];
          AES_keyInstance masterdecode;
          AES_cipherInstance mastercypher;
          /*@@@
                      char ansiKey[KEY_LEN+1];
                      int len = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, masterKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                      AES_keygen( ansiKey, binMasterKey );
                      */
          AES_keygen(masterKey, binMasterKey);
          AES_bin_setup(&masterdecode, AES_DIR_DECRYPT, KEY_BYTES * 8, binMasterKey);
          AES_bin_cipherInit(&mastercypher, AES_MODE_CBC, masterFileIV);
          AES_blockDecrypt(&mastercypher, &masterdecode, masterFileKey, sizeof(binFileKey), binFileKey);
          hasBinFileKey = TRUE;
          haveFileKey = TRUE;
          useMasterKey = FALSE;
        }

        if (haveFileKey)
        {
          AES_keyInstance fileDecode;
          AES_cipherInstance fileCypher;
          AES_bin_setup(&fileDecode, AES_DIR_DECRYPT, KEY_BYTES * 8, binFileKey);
          AES_bin_cipherInit(&fileCypher, AES_MODE_CBC, &rawdata[PREAMBLE_SIZE]);	// IV is next
          { // finally, decrypt the actual data
            unsigned long nb = 0;
            if ((readsize - code_offset) > PAD_SLOP) {
              nb += AES_blockDecrypt(&fileCypher, &fileDecode, &rawdata[code_offset], readsize - code_offset - PAD_SLOP, rawdata);
            }
            nb += AES_padDecrypt(&fileCypher, &fileDecode, &rawdata[code_offset + nb], readsize - code_offset - nb, rawdata + nb);
            if (nb > 0) {
              rawdata[nb] = (char)0;
              rawdata[nb + 1] = (char)0;	// two zeros in case it's multibyte
              *resultlen = (DWORD)nb;
              bReadSuccess = 1;
              usedEncryption = TRUE;
            }
            else {
              *resultlen = 0;
              MsgBox(MBWARN, IDS_PASS_FAILURE);
              bReadSuccess = -1;
              usedEncryption = FALSE;
            }
          }
        }
        else
        {
          // simulate read failure
          MsgBox(MBWARN, IDS_NOPASS);
          *resultlen = 0;
          bReadSuccess = -1;
          usedEncryption = FALSE;
        }
      }

      break;

      default: BUG1("format %d not understood", scheme);
      }
    }
  }

  if (!usedEncryption)
  { // here, the file is believed to be a straight text file
    ResetEncryption();
    *resultlen = readsize;
  }

  GlobalUnlock(rawhandle);

  //if ( !bReadSuccess )
  //{
  //	GlobalFree( rawhandle );
  //}

  return(bReadSuccess);
}

BOOL EncryptAndWriteFile(HWND hwnd, HANDLE hFile, BYTE *data, DWORD size, DWORD *written)
{
  static int sequence = 1;	// sequence counter so each time is unique

  if (useFileKey || hasMasterFileKey)
  {
    AES_keyInstance fileEncode;		// encryption key for the file
    AES_cipherInstance fileCypher;	// cypher for the file, including the IV
    DWORD PREAMBLE_written = 0;
    BYTE precodedata[AES_MAX_IV_SIZE * 2 + KEY_BYTES * 2 + PREAMBLE_SIZE];
    long precode_size = AES_MAX_IV_SIZE + PREAMBLE_SIZE; //precode in standard file format
    long *PREAMBLE_data = (long *)precodedata;
    PREAMBLE_data[0] = PREAMBLE;
    PREAMBLE_data[1] = FILEKEY_FORMAT;

    srand(sequence++ ^ (unsigned int)time(NULL));
    {
      int i; for (i = 0; i < AES_MAX_IV_SIZE; i++)
      {
        precodedata[PREAMBLE_SIZE + i] = 0;//rand();
      }
    }

    {
      if (useFileKey) {
        // generate the encryption key from the passphrase
        /* @@@
                char ansiKey[KEY_LEN+1];
                int len = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, fileKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                ansiKey[len] = '\0';
                AES_keygen( ansiKey, binFileKey );
                */
        AES_keygen(fileKey, binFileKey);
        hasBinFileKey = TRUE;
      };

      AES_bin_setup(&fileEncode, AES_DIR_ENCRYPT, KEY_BYTES * 8, binFileKey);

      AES_bin_cipherInit(&fileCypher, AES_MODE_CBC, &precodedata[PREAMBLE_SIZE]);

      if (useMasterKey && *masterKey)
      {	//setup with the master key and encrypt the file key.
        //append the encrypted file key to the end of the PREAMBLE block
        BYTE binMasterKey[KEY_BYTES];
        AES_keyInstance masterencode;
        AES_cipherInstance mastercypher;
        /* @@@
                char ansiKey[KEY_LEN+1];
                int len = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, masterKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                ansiKey[len] = '\0';
                AES_keygen( ansiKey, binMasterKey );
                */
        AES_keygen(masterKey, binMasterKey);
        AES_bin_setup(&masterencode, AES_DIR_ENCRYPT, KEY_BYTES * 8, binMasterKey);
        {// generate another IV for the master key

          int i; for (i = 0; i < sizeof(masterFileIV); i++) { masterFileIV[i] = rand(); }
        }

        AES_bin_cipherInit(&mastercypher, AES_MODE_CBC, masterFileIV);

        AES_blockEncrypt(&mastercypher, &masterencode, binFileKey, sizeof(binFileKey), masterFileKey);
        hasMasterFileKey = TRUE;
      }

      if (hasMasterFileKey)
      {// copy the encrypted (new or recycled) into the output
        memcpy(&precodedata[precode_size], masterFileIV, sizeof(masterFileIV));
        memcpy(&precodedata[precode_size + sizeof(masterFileIV)], masterFileKey, sizeof(masterFileKey));
        precode_size += sizeof(masterFileKey) + sizeof(masterFileIV);
        PREAMBLE_data[1] = MASTERKEY_FORMAT;
      }

      // write the PREAMBLE, punt if that failed
      if (!WriteFile(hFile, precodedata, precode_size, &PREAMBLE_written, NULL))
      {
        *written = PREAMBLE_written;
        return(FALSE);
      }
    }

    // now encrypt the main file
    {
      HANDLE enchandle = GlobalAlloc(GPTR, size + PAD_SLOP);	// add slop to the end for padding
      BYTE *encdata = GlobalLock(enchandle);
      BOOL writeOK = FALSE;
      DWORD enclen_written = 0;
      DWORD enclen = 0;

      if (size > PAD_SLOP) { enclen += AES_blockEncrypt(&fileCypher, &fileEncode, data, size - PAD_SLOP, encdata); }

      enclen += AES_padEncrypt(&fileCypher, &fileEncode, data + enclen, size - enclen, encdata + enclen);

      writeOK = WriteFile(hFile, encdata, enclen, &enclen_written, NULL);

      GlobalUnlock(enchandle);							// clean up
      GlobalFree(enchandle);
      *written = PREAMBLE_written + enclen_written;		// return the file size written
      return(writeOK);									// and the file ok status
    }
  }
  else
  {
    // not an encrypted file, write normally
    BOOL bWriteSuccess = WriteFile(hFile, data, size, written, NULL);
    return(bWriteSuccess);
  }
}

