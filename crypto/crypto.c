// encoding: UTF-8
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
#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>
#include <intsafe.h>
#include <shellapi.h>
#include <time.h>
#include "..\src\Dialogs.h"
#include "..\src\Helpers.h"
#include "..\src\resource.h"
#include "rijndael-api-fst.h"
#include "crypto.h"

#define WKEY_LEN 256
#define KEY_LEN  512

static bool  useFileKey = false;			// file should be encrypted
static char fileKey[KEY_LEN] = { 0 };		// ascii passphrase for the file key
static WCHAR unicodeFileKey[WKEY_LEN] = { 0 };	// unicode file passphrase
static bool  useMasterKey = false;		    // file should have a master key
static char masterKey[KEY_LEN] = { 0 };		// ascii passphrase for the master key
static WCHAR unicodeMasterKey[WKEY_LEN] = { 0 };	// unicode master passphrase
static BYTE  binFileKey[KEY_BYTES];			// the encryption key in for the file
static bool  hasBinFileKey = false;
static BYTE  masterFileKey[KEY_BYTES];		// file key encrypted with the master key
static BYTE  masterFileIV[AES_MAX_IV_SIZE];	// the iv for the master key
static bool  hasMasterFileKey = false;
static bool  masterKeyAvailable = false;		// information for the passphrase dialog box


bool IsEncryptionRequired()
{
  return (useFileKey || hasMasterFileKey);
}


void ResetEncryption()
{
    masterKeyAvailable = false;
    hasMasterFileKey = false;
    hasBinFileKey = false;
    useMasterKey = false;
    useFileKey = false;
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
    while ((src[sidx] != 0) && (didx < destLim)) {
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
    PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwndControl, true);
}


//=============================================================================
// helper function for the "set passphrase" dialog before output
// the complication in this version is that the incoming text is unicode. We deal
// with it by converting the unicode to an ascii compatible byte stream, so the
// caller (and hence the rest of the encryption) doesn't know unicode was involved.
INT_PTR CALLBACK SetKeysDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    UNUSED(lParam);
    const WCHAR wDot = (WCHAR)0x25CF;

    switch (umsg) {

    case WM_INITDIALOG:
    {
        SetDialogIconNP3(hDlg);
        SetDlgItemText(hDlg, IDC_PWD_EDIT1, unicodeFileKey);
        SetDlgItemText(hDlg, IDC_PWD_EDIT2, unicodeMasterKey);
        ShowWindow(GetDlgItem(hDlg, IDC_PWD_CHECK3), hasMasterFileKey);
        CheckDlgButton(hDlg, IDC_PWD_CHECK3, SetBtn(hasMasterFileKey));
        CheckDlgButton(hDlg, IDC_PWD_CHECK2, SetBtn(hasBinFileKey || useFileKey));
        CheckDlgButton(hDlg, IDC_PWD_CHECK1, SetBtn(useMasterKey));
        CenterDlgInParent(hDlg, NULL);
        // Don't use: SetFocus( GetDlgItem( hDlg, IDC_PWD_EDIT1 ) );
        SetDialogFocus(hDlg, GetDlgItem(hDlg, IDC_PWD_EDIT1));
    }
    return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hDlg, (RECT*)lParam, NULL);
      return !0;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case IDC_PWD_CHECK4:
          {
            if (IsButtonChecked(hDlg, IDC_PWD_CHECK4)) {
              SendDlgItemMessage(hDlg, IDC_PWD_EDIT1, EM_SETPASSWORDCHAR, 0, 0);
              SendDlgItemMessage(hDlg, IDC_PWD_EDIT2, EM_SETPASSWORDCHAR, 0, 0);
            }
            else {
              SendDlgItemMessage(hDlg, IDC_PWD_EDIT1, EM_SETPASSWORDCHAR, (WPARAM)wDot, 0);
              SendDlgItemMessage(hDlg, IDC_PWD_EDIT2, EM_SETPASSWORDCHAR, (WPARAM)wDot, 0);
            }
            InvalidateRect(hDlg, NULL, TRUE);
          }
          return !0;
        break;

        case IDOK:
        {
            bool const useMas = IsButtonChecked(hDlg, IDC_PWD_CHECK1);
            bool const useFil = IsButtonChecked(hDlg, IDC_PWD_CHECK2);
            bool const reuseMas = IsButtonChecked(hDlg, IDC_PWD_CHECK3);
            WCHAR newFileKey[WKEY_LEN] = { 0 };
            WCHAR newMasKey[WKEY_LEN] = { 0 };
            hasMasterFileKey &= reuseMas;
            GetDlgItemText(hDlg, IDC_PWD_EDIT1, newFileKey, COUNTOF(newFileKey));
            GetDlgItemText(hDlg, IDC_PWD_EDIT2, newMasKey, COUNTOF(newMasKey));
            useFileKey = !((newFileKey[0] <= ' ') || !useFil);
            useMasterKey = !((newMasKey[0] <= ' ') || !useMas);
            memcpy(unicodeFileKey, newFileKey, sizeof(unicodeFileKey));
            memcpy(unicodeMasterKey, newMasKey, sizeof(unicodeMasterKey));
            unicodeStringCpy(fileKey, unicodeFileKey, sizeof(fileKey));
            unicodeStringCpy(masterKey, unicodeMasterKey, sizeof(masterKey));
            EndDialog(hDlg, IDOK);
            return true;
        }

        break;

        case IDC_PWD_EDIT1:
        {
            WCHAR newFileKey[WKEY_LEN] = { 0 };
            GetDlgItemText(hDlg, IDC_PWD_EDIT1, newFileKey, COUNTOF(newFileKey));
            CheckDlgButton(hDlg, IDC_PWD_CHECK2, SetBtn(newFileKey[0] > ' '));
        }

        break;

        case IDC_PWD_EDIT2:
        {
            WCHAR newMasKey[WKEY_LEN] = { 0 };
            GetDlgItemText(hDlg, IDC_PWD_EDIT2, newMasKey, COUNTOF(newMasKey));
            {
                bool newuse = (newMasKey[0] > ' ');	// no leading whitespace or empty passwords
                CheckDlgButton(hDlg, IDC_PWD_CHECK1, SetBtn(newuse));

                if (newuse) { CheckDlgButton(hDlg, IDC_PWD_CHECK3, BST_UNCHECKED); }
            }
        }

        break;

        case IDC_PWD_CHECK3:  // check reuse, uncheck set new and inverse
        {
            bool const reuseMas = IsButtonChecked(hDlg, IDC_PWD_CHECK3);
            if (reuseMas) { CheckDlgButton(hDlg, IDC_PWD_CHECK1, SetBtn(!reuseMas)); }
        }

        break;

        case IDC_PWD_CHECK1:
        {
            bool const useMas = IsButtonChecked(hDlg, IDC_PWD_CHECK1);
            if (useMas) { CheckDlgButton(hDlg, IDC_PWD_CHECK3, SetBtn(!useMas)); }
        }

        break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;

        }

        break;

    }
    return 0;
}

//
// helper for setting password when reading a file
// the complication in this version is that the incoming text is unicode. We deal
// with it by converting the unicode to an ascii compatible byte stream, so the
// caller (and hence the rest of the encryption) doesn't know unicode was involved.
//
INT_PTR CALLBACK GetKeysDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  const WCHAR wDot = (WCHAR)0x25CF;

  switch (umsg) {

  case WM_INITDIALOG:
    {
      SetDialogIconNP3(hDlg);
      int vis = masterKeyAvailable ? SW_SHOW : SW_HIDE;
      ShowWindow(GetDlgItem(hDlg, IDC_PWD_STATMPW), vis);
      ShowWindow(GetDlgItem(hDlg, IDC_PWD_CHECK3), vis);
      //~SetDlgItemText( hDlg, IDC_PWD_EDIT3, fileKey );
      SetDlgItemText(hDlg, IDC_PWD_EDIT3, unicodeFileKey);
      CheckDlgButton(hDlg, IDC_PWD_CHECK3, BST_UNCHECKED);
      CenterDlgInParent(hDlg, NULL);
      // Don't use: SetFocus( GetDlgItem( hDlg, IDC_PWD_EDIT3 ) );
      SetDialogFocus(hDlg, GetDlgItem(hDlg, IDC_PWD_EDIT3));
    }
    return !0;

  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hDlg, (RECT*)lParam, NULL);
    return !0;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {
    case IDC_PWD_CHECK4:
      {
        if (IsButtonChecked(hDlg, IDC_PWD_CHECK4)) {
          SendDlgItemMessage(hDlg, IDC_PWD_EDIT3, EM_SETPASSWORDCHAR, 0, 0);
        }
        else {
          SendDlgItemMessage(hDlg, IDC_PWD_EDIT3, EM_SETPASSWORDCHAR, (WPARAM)wDot, 0);
        }
        InvalidateRect(hDlg, NULL, TRUE);
      }
      return !0;

    case IDOK:
      {
        bool const useMas = IsButtonChecked(hDlg, IDC_PWD_CHECK3);
        WCHAR newKey[WKEY_LEN] = { L'\0' };
        GetDlgItemText(hDlg, IDC_PWD_EDIT3, newKey, COUNTOF(newKey));

        if (useMas) {
          memcpy(unicodeMasterKey, newKey, sizeof(unicodeMasterKey));
          unicodeStringCpy(masterKey, unicodeMasterKey, sizeof(masterKey));
          useFileKey = false;
          useMasterKey = true;
        }
        else {
          memcpy(unicodeFileKey, newKey, sizeof(unicodeFileKey));
          unicodeStringCpy(fileKey, unicodeFileKey, sizeof(fileKey));
          useFileKey = true;
          useMasterKey = false;
        }
        EndDialog(hDlg, IDOK);
      }
      return !0;

    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;
    }
    break;
  }
  return 0;
}


// set passphrases for output
bool GetFileKey(HWND hwnd)
{
  return (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_PASSWORDS),
                                       GetParent(hwnd), SetKeysDlgProc, (LPARAM)hwnd));
}

// set passphrases for file being input
bool ReadFileKey(HWND hwnd, bool master)
{
    masterKeyAvailable = master;
    return (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_READPW),
                                         GetParent(hwnd), GetKeysDlgProc, (LPARAM)hwnd));
}


// ////////////////////////////////////////////////////////////////////////////
//
// read the file data, decrypt if necessary, 
// return the result as a new allocation
//
int ReadAndDecryptFile(HWND hwnd, HANDLE hFile, size_t fileSize, void** result, size_t *resultlen)
{
  HANDLE rawhandle = *result;
  BYTE* rawdata = (BYTE*)GlobalLock(rawhandle);

  int returnFlag = DECRYPT_SUCCESS;
  bool usedEncryption = false;
  bool bRetryPassPhrase = true;
  size_t bytesRead = 0ULL;

  while (bRetryPassPhrase) {

    SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);
    returnFlag = DECRYPT_SUCCESS;
    usedEncryption = false;
    bRetryPassPhrase = false;

    bool const bReadOk = ReadFileXL(hFile, (char* const)rawdata, fileSize, &bytesRead);

    returnFlag = bReadOk ? DECRYPT_SUCCESS : DECRYPT_FREAD_FAILED;

    // we read the file, check if it looks like our encryption format

    if (bReadOk && (bytesRead > (PREAMBLE_SIZE + AES_MAX_IV_SIZE))) {

      long *ldata = (long*)rawdata;

      if (ldata && (ldata[0] == PREAMBLE)) {
        long scheme = ldata[1];
        unsigned long code_offset = PREAMBLE_SIZE + AES_MAX_IV_SIZE;

        switch (scheme) {

        case MASTERKEY_FORMAT:
          code_offset += sizeof(masterFileKey) + sizeof(masterFileIV);
          // save the encrypted file key and IV.  They can be reused if the
          // passphrases are not changed.
          memcpy(masterFileIV, &rawdata[MASTER_KEY_OFFSET], sizeof(masterFileIV));
          memcpy(masterFileKey, &rawdata[MASTER_KEY_OFFSET + sizeof(masterFileIV)], sizeof(masterFileKey));
          hasMasterFileKey = true;

          // fall through
        case FILEKEY_FORMAT:
          {
            bool haveFileKey = ReadFileKey(hwnd, scheme == MASTERKEY_FORMAT);

            if (useFileKey) {
              // use the file key to decode
              /*@@@
                char ansiKey[KEY_LEN+1];
                ptrdiff_t len = WideCharToMultiByteEx( CP_ACP, WC_NO_BEST_FIT_CHARS, fileKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                ansiKey[len] = '\0';
                AES_keygen( ansiKey, binFileKey );		// generate the encryption key from the passphrase
                */
              AES_keygen(fileKey, binFileKey);		// generate the encryption key from the passphrase
              hasBinFileKey = true;
            }
            else if ((scheme == MASTERKEY_FORMAT) && useMasterKey) {	// use the master key to recover the file key
              BYTE binMasterKey[KEY_BYTES];
              AES_keyInstance masterdecode;
              AES_cipherInstance mastercypher;
              /*@@@
                char ansiKey[KEY_LEN+1];
                int ptrdiff_t = WideCharToMultiByteEx( CP_ACP, WC_NO_BEST_FIT_CHARS, masterKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                AES_keygen( ansiKey, binMasterKey );
                */
              AES_keygen(masterKey, binMasterKey);
              AES_bin_setup(&masterdecode, AES_DIR_DECRYPT, KEY_BYTES * 8, binMasterKey);
              AES_bin_cipherInit(&mastercypher, AES_MODE_CBC, masterFileIV);
              AES_blockDecrypt(&mastercypher, &masterdecode, masterFileKey, sizeof(binFileKey), binFileKey);
              hasBinFileKey = true;
              haveFileKey = true;
              useMasterKey = false;
            }

            if (haveFileKey) {
              usedEncryption = true;
              AES_keyInstance fileDecode;
              AES_cipherInstance fileCypher;
              AES_bin_setup(&fileDecode, AES_DIR_DECRYPT, KEY_BYTES * 8, binFileKey);
              AES_bin_cipherInit(&fileCypher, AES_MODE_CBC, &rawdata[PREAMBLE_SIZE]);	// IV is next
              { // finally, decrypt the actual data
                ptrdiff_t nbb = BAD_CIPHER_STATE;
                ptrdiff_t nbp = BAD_CIPHER_STATE;
                if ((bytesRead - code_offset) >= PAD_SLOP) {
                  nbb = AES_blockDecrypt(&fileCypher, &fileDecode, &rawdata[code_offset], bytesRead - code_offset - PAD_SLOP, rawdata);
                }
                if (nbb >= 0) {
                  nbp = AES_padDecrypt(&fileCypher, &fileDecode, &rawdata[code_offset + nbb], bytesRead - code_offset - nbb, rawdata + nbb);
                }
                if (nbp >= 0) {
                  size_t const nb = nbb + nbp;
                  rawdata[nb] = '\0';
                  rawdata[nb + 1] = '\0';	// two zeros in case it's multi-byte
                  *resultlen = nb;
                }
                else {
                  bRetryPassPhrase = (InfoBoxLng(MB_RETRYCANCEL | MB_ICONWARNING, NULL, IDS_MUI_PASS_FAILURE) == IDRETRY);
                  if (!bRetryPassPhrase) {
                    // enable raw encryption read
                    *resultlen = bytesRead;
                    returnFlag |= DECRYPT_WRONG_PASS;
                  }
                }
              }
            }
            else {
              // enable raw encryption read
              returnFlag |= DECRYPT_CANCELED_NO_PASS;
            }
          }
          break;

        default:
          BUG1("format %d not understood", scheme);
          returnFlag |= DECRYPT_FATAL_ERROR;
          break;
        }
      }
    }
  }  // while bRetryPassPhrase

  if (!usedEncryption) { // here, the file is believed to be a straight text file
    ResetEncryption();
    *resultlen = bytesRead;
    returnFlag |= DECRYPT_NO_ENCRYPTION;
  }

  GlobalUnlock(rawhandle);

  return returnFlag;
}

bool EncryptAndWriteFile(HWND hwnd, HANDLE hFile, BYTE *data, size_t size, size_t*written)
{
    UNUSED(hwnd);

    if (IsEncryptionRequired()) {
        AES_keyInstance fileEncode;		// encryption key for the file
        AES_cipherInstance fileCypher;	// cypher for the file, including the IV
        DWORD PREAMBLE_written = 0;
        BYTE precodedata[AES_MAX_IV_SIZE * 2 + KEY_BYTES * 2 + PREAMBLE_SIZE];
        long precode_size = AES_MAX_IV_SIZE + PREAMBLE_SIZE; //precode in standard file format
        long *PREAMBLE_data = (long *)precodedata;
        PREAMBLE_data[0] = PREAMBLE;
        PREAMBLE_data[1] = FILEKEY_FORMAT;

        static int sequence = 1;	// sequence counter so each time is unique
        srand(sequence++ ^ (unsigned int)time(NULL));
        {
            for (int i = 0; i < AES_MAX_IV_SIZE; i++) {
                precodedata[PREAMBLE_SIZE + i] = 0;//rand();
            }
        }

        {
            if (useFileKey) {
                // generate the encryption key from the passphrase
                /* @@@
                        char ansiKey[KEY_LEN+1];
                        int ptrdiff_t = WideCharToMultiByteEx( CP_ACP, WC_NO_BEST_FIT_CHARS, fileKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                        ansiKey[len] = '\0';
                        AES_keygen( ansiKey, binFileKey );
                        */
                AES_keygen(fileKey, binFileKey);
                hasBinFileKey = true;
            };

            AES_bin_setup(&fileEncode, AES_DIR_ENCRYPT, KEY_BYTES * 8, binFileKey);

            AES_bin_cipherInit(&fileCypher, AES_MODE_CBC, &precodedata[PREAMBLE_SIZE]);

            if (useMasterKey && *masterKey) {	//setup with the master key and encrypt the file key.
              //append the encrypted file key to the end of the PREAMBLE block
                BYTE binMasterKey[KEY_BYTES];
                AES_keyInstance masterencode;
                AES_cipherInstance mastercypher;
                /* @@@
                        char ansiKey[KEY_LEN+1];
                        ptrdiff_t len = WideCharToMultiByteEx( CP_ACP, WC_NO_BEST_FIT_CHARS, masterKey, -1, ansiKey, KEY_LEN, NULL, NULL );
                        ansiKey[len] = '\0';
                        AES_keygen( ansiKey, binMasterKey );
                        */
                AES_keygen(masterKey, binMasterKey);
                AES_bin_setup(&masterencode, AES_DIR_ENCRYPT, KEY_BYTES * 8, binMasterKey);
                {// generate another IV for the master key

                    for (int i = 0; i < sizeof(masterFileIV); i++) { masterFileIV[i] = (BYTE)(rand() & BYTE_MAX); }
                }

                AES_bin_cipherInit(&mastercypher, AES_MODE_CBC, masterFileIV);

                AES_blockEncrypt(&mastercypher, &masterencode, binFileKey, sizeof(binFileKey), masterFileKey);
                hasMasterFileKey = true;
            }

            if (hasMasterFileKey) {// copy the encrypted (new or recycled) into the output
                memcpy(&precodedata[precode_size], masterFileIV, sizeof(masterFileIV));
                memcpy(&precodedata[precode_size + sizeof(masterFileIV)], masterFileKey, sizeof(masterFileKey));
                precode_size += sizeof(masterFileKey) + sizeof(masterFileIV);
                PREAMBLE_data[1] = MASTERKEY_FORMAT;
            }

            // write the PREAMBLE, punt if that failed
            if (!WriteFile(hFile, precodedata, precode_size, &PREAMBLE_written, NULL)) {
                *written = PREAMBLE_written;
                return false;
            }
        }
        // now encrypt the main file
        {
            bool bWriteRes = false;
          
            BYTE* encdata = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + PAD_SLOP);  // add slop to the end for padding
            if (!encdata) {
              return bWriteRes;
            }

            size_t enclen = 0;
            if (size > PAD_SLOP) { enclen += AES_blockEncrypt(&fileCypher, &fileEncode, data, size - PAD_SLOP, encdata); }

            enclen += AES_padEncrypt(&fileCypher, &fileEncode, data + enclen, size - enclen, encdata + enclen);

            size_t enclen_written = 0;
            bWriteRes = WriteFileXL(hFile, (const char* const)encdata, enclen, &enclen_written);

            HeapFree(GetProcessHeap(), 0, encdata);             // clean-up

            *written = (size_t)PREAMBLE_written + enclen_written;		// return the file size written
            return bWriteRes;									// and the file ok status
        }
    }
    else {
        // not an encrypted file, write normally
        return WriteFileXL(hFile, (const char* const)data, size, written);
    }
}

