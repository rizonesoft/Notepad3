/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Edit.h                                                                      *
*   Text File Editing Helper Stuff                                            *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/


// extern "C" declarations of Scintilla functions
BOOL Scintilla_RegisterClasses(void*);
BOOL Scintilla_ReleaseResources();


typedef struct _editfindreplace
{
  char szFind[512];
  char szReplace[512];
  char szFindUTF8[3*512];
  char szReplaceUTF8[3*512];
  UINT fuFlags;
  BOOL bTransformBS;
  BOOL bObsolete /* was bFindUp */;
  BOOL bFindClose;
  BOOL bReplaceClose;
  BOOL bNoFindWrap;
  HWND hwnd;
#ifdef BOOKMARK_EDITION
  BOOL bWildcardSearch;
#endif
  //HANDLE hMRUFind;
  //HANDLE hMRUReplace;

} EDITFINDREPLACE, *LPEDITFINDREPLACE, *LPCEDITFINDREPLACE;


#define IDMSG_SWITCHTOFIND    300
#define IDMSG_SWITCHTOREPLACE 301


#define ALIGN_LEFT       0
#define ALIGN_RIGHT      1
#define ALIGN_CENTER     2
#define ALIGN_JUSTIFY    3
#define ALIGN_JUSTIFY_EX 4


#define SORT_ASCENDING  0
#define SORT_DESCENDING 1
#define SORT_SHUFFLE    2
#define SORT_MERGEDUP   4
#define SORT_UNIQDUP    8
#define SORT_UNIQUNIQ  16
#define SORT_NOCASE    32
#define SORT_LOGICAL   64
#define SORT_COLUMN   128


HWND  EditCreate(HWND);
void  EditSetNewText(HWND,char*,DWORD);
BOOL  EditConvertText(HWND,UINT,UINT,BOOL);
BOOL  EditSetNewEncoding(HWND,int,int,BOOL,BOOL);
char* EditGetClipboardText(HWND);
BOOL  EditCopyAppend(HWND);
int   EditDetectEOLMode(HWND,char*,DWORD);
BOOL  EditLoadFile(HWND,LPCWSTR,BOOL,int*,int*,BOOL*,BOOL*);
BOOL  EditSaveFile(HWND,LPCWSTR,int,BOOL*,BOOL);

void  EditInvertCase(HWND);
void  EditTitleCase(HWND);
void  EditSentenceCase(HWND);

void  EditURLEncode(HWND);
void  EditURLDecode(HWND);
void  EditEscapeCChars(HWND);
void  EditUnescapeCChars(HWND);
void  EditChar2Hex(HWND);
void  EditHex2Char(HWND);
void  EditModifyNumber(HWND,BOOL);

void  EditTabsToSpaces(HWND,int,BOOL);
void  EditSpacesToTabs(HWND,int,BOOL);

void  EditMoveUp(HWND);
void  EditMoveDown(HWND);
void  EditModifyLines(HWND,LPCWSTR,LPCWSTR);
void  EditAlignText(HWND,int);
void  EditEncloseSelection(HWND,LPCWSTR,LPCWSTR);
void  EditToggleLineComments(HWND,LPCWSTR,BOOL);
void  EditPadWithSpaces(HWND,BOOL,BOOL);
void  EditStripFirstCharacter(HWND);
void  EditStripLastCharacter(HWND);
void  EditStripTrailingBlanks(HWND,BOOL);
void  EditCompressSpaces(HWND);
void  EditRemoveBlankLines(HWND,BOOL);
void  EditWrapToColumn(HWND,int);
void  EditJoinLinesEx(HWND);
void  EditSortLines(HWND,int);

void  EditJumpTo(HWND,int,int);
void  EditSelectEx(HWND,int,int);
void  EditFixPositions(HWND);
void  EditEnsureSelectionVisible(HWND);
void  EditGetExcerpt(HWND,LPWSTR,DWORD);

HWND  EditFindReplaceDlg(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditFindNext(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditFindPrev(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditReplace(HWND,LPCEDITFINDREPLACE);
BOOL  EditReplaceAll(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditReplaceAllInSelection(HWND,LPCEDITFINDREPLACE,BOOL);
BOOL  EditLinenumDlg(HWND);
BOOL  EditModifyLinesDlg(HWND,LPWSTR,LPWSTR);
BOOL  EditEncloseSelectionDlg(HWND,LPWSTR,LPWSTR);
BOOL  EditInsertTagDlg(HWND,LPWSTR,LPWSTR);
BOOL  EditSortDlg(HWND,int*);
BOOL  EditAlignDlg(HWND,int*);
BOOL  EditPrint(HWND,LPCWSTR,LPCWSTR);
void  EditPrintSetup(HWND);
void  EditPrintInit();
void  EditMarkAll(HWND,int,BOOL,BOOL);
void  CompleteWord(HWND, BOOL);

extern int g_DOSEncoding;

#define NCP_DEFAULT            1
#define NCP_UTF8               2
#define NCP_UTF8_SIGN          4
#define NCP_UNICODE            8
#define NCP_UNICODE_REVERSE   16
#define NCP_UNICODE_BOM       32
#define NCP_8BIT              64
#define NCP_INTERNAL          (NCP_DEFAULT|NCP_UTF8|NCP_UTF8_SIGN|NCP_UNICODE|NCP_UNICODE_REVERSE|NCP_UNICODE_BOM)
#define NCP_RECODE           128
#define CPI_NONE              -1
#define CPI_DEFAULT            0
#define CPI_OEM                1
#define CPI_UNICODEBOM         2
#define CPI_UNICODEBEBOM       3
#define CPI_UNICODE            4
#define CPI_UNICODEBE          5
#define CPI_UTF8               6
#define CPI_UTF8SIGN           7
#define CPI_UTF7               8

#define IDS_ENCODINGNAME0  61000
#define IDS_EOLMODENAME0   62000

typedef struct _np2encoding {
  UINT    uFlags;
  UINT    uCodePage;
  char*   pszParseNames;
  int     idsName;
  WCHAR   wchLabel[32];
} NP2ENCODING;

void Encoding_InitDefaults();
int  Encoding_MapIniSetting(BOOL,int);
void Encoding_GetLabel(int);
int  Encoding_MatchW(LPCWSTR);
int  Encoding_MatchA(char*);
BOOL Encoding_IsValid(int);
void Encoding_AddToListView(HWND,int,BOOL);
BOOL Encoding_GetFromListView(HWND,int *);
void Encoding_AddToComboboxEx(HWND,int,BOOL);
BOOL Encoding_GetFromComboboxEx(HWND,int *);

BOOL IsUnicode(const char*,int,LPBOOL,LPBOOL);
BOOL IsUTF8(const char*,int);
BOOL IsUTF7(const char*,int);


//void SciInitThemes(HWND);
//LRESULT CALLBACK SciThemedWndProc(HWND,UINT,WPARAM,LPARAM);


#define FV_TABWIDTH        1
#define FV_INDENTWIDTH     2
#define FV_TABSASSPACES    4
#define FV_TABINDENTS      8
#define FV_WORDWRAP       16
#define FV_LONGLINESLIMIT 32
#define FV_ENCODING       64
#define FV_MODE          128

typedef struct _filevars {

  int mask;
  int iTabWidth;
  int iIndentWidth;
  BOOL bTabsAsSpaces;
  BOOL bTabIndents;
  BOOL fWordWrap;
  int iLongLinesLimit;
  char tchEncoding[32];
  int  iEncoding;
  char tchMode[32];

} FILEVARS, *LPFILEVARS;

BOOL FileVars_Init(char*,DWORD,LPFILEVARS);
BOOL FileVars_Apply(HWND,LPFILEVARS);
BOOL FileVars_ParseInt(char*,char*,int*);
BOOL FileVars_ParseStr(char*,char*,char*,int);
BOOL FileVars_IsUTF8(LPFILEVARS);
BOOL FileVars_IsNonUTF8(LPFILEVARS);
BOOL FileVars_IsValidEncoding(LPFILEVARS);
int  FileVars_GetEncoding(LPFILEVARS);



///   End of Edit.h   \\\
