// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* AccelKeys.c                                                                 *
*   Load Accelerator Keys at Runtime                                          *
*                                                                             *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*******************************************************************************/

// https://docs.microsoft.com/en-us/windows/desktop/menurc/using-keyboard-accelerators
// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadacceleratorsw

// ============================================================================

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
#include <windows.h>

#include "resource.h"

#include "AccelKeys.h"

// ============================================================================

// typedef struct tagACCEL {
//   BYTE   fVirt;   // FVIRTKEY (else ASCII code) | FNOINVERT | FCONTROL | FALT | FSHIFT
//   WORD   key;     // ASCII character code or a virtual-key code
//   WORD   cmd;
// } ACCEL, *LPACCEL;

typedef struct _CMDNAMEIDMAP {
    char* pCmdName;
    int   ridStrgTbl;
    ACCEL accel;
}
CMD_NAME_ID_MAP, *LP_CMD_NAME_ID_MAP;


// ============================================================================

CMD_NAME_ID_MAP NP3_ACCEL_KEY_MAP[] = {
    { "CMD_DUPLINEORSEL", 0, { (FVIRTKEY | FNOINVERT), 0, IDM_EDIT_DUPLINEORSELECTION } }
    ,{ "CMD_TRANSPLINES",  0, { (FVIRTKEY | FNOINVERT), 0, IDM_EDIT_LINETRANSPOSE } }
};





// ============================================================================
// VKey Lookup Support
// ----------------------------------------------------------------------------

#define MAXKEYS 25

typedef struct _VKEYS {
    char *pKeyName;
    char *pKeyString;
} VKEYS;

VKEYS vkeys[MAXKEYS] = {
    "BkSp",     "Back Space",
    "PgUp",     "Page Up",
    "PgDn",     "Page Down",
    "End",      "End",
    "Home",     "Home",
    "Lft",      "Left",
    "Up",       "Up",
    "Rgt",      "Right",
    "Dn",       "Down",
    "Ins",      "Insert",
    "Del",      "Delete",
    "Mult",     "Multiply",
    "Add",      "Add",
    "Sub",      "Subtract",
    "DecPt",    "Decimal Point",
    "Div",      "Divide",
    "F2",       "F2",
    "F3",       "F3",
    "F5",       "F5",
    "F6",       "F6",
    "F7",       "F7",
    "F8",       "F8",
    "F9",       "F9",
    "F11",      "F11",
    "F12",      "F12"
};
// ============================================================================


// ============================================================================
// ============================================================================
// Constant                 value           Description
// ---------------------------------------------- -
// VK_LBUTTON               0x01            Left mouse button
// VK_RBUTTON               0x02            Right mouse button
// VK_CANCEL                0x03            Control - break processing
// VK_MBUTTON               0x04            Middle mouse button(three - button mouse)
// VK_XBUTTON1              0x05            X1 mouse button
// VK_XBUTTON2              0x06            X2 mouse button
// -                        0x07            Undefined
// VK_BACK                  0x08            BACKSPACE key
// VK_TAB                   0x09            TAB key
// -                        0x0A-0B         Reserved
// VK_CLEAR                 0x0C            CLEAR key
// VK_RETURN                0x0D            ENTER key
// -                        0x0E - 0F       Undefined
// VK_SHIFT                 0x10            SHIFT key
// VK_CONTROL               0x11            CTRL key
// VK_MENU                  0x12            ALT key
// VK_PAUSE                 0x13            PAUSE key
// VK_CAPITAL               0x14            CAPS LOCK key
// VK_KANA                  0x15            IME Kana mode
// VK_HANGUEL               0x15            IME Hanguel mode(maintained for compatibility; use VK_HANGUL)
// VK_HANGUL                0x15            IME Hangul mode
// -                        0x16            Undefined
// VK_JUNJA                 0x17            IME Junja mode
// VK_FINAL                 0x18            IME final mode
// VK_HANJA                 0x19            IME Hanja mode
// VK_KANJI                 0x19            IME Kanji mode
// -                        0x1A            Undefined
// VK_ESCAPE                0x1B            ESC key
// VK_CONVERT               0x1C            IME convert
// VK_NONCONVERT            0x1D            IME nonconvert
// VK_ACCEPT                0x1E            IME accept
// VK_MODECHANGE            0x1F            IME mode change request
// VK_SPACE                 0x20            SPACEBAR
// VK_PRIOR                 0x21            PAGE UP key
// VK_NEXT                  0x22            PAGE DOWN key
// VK_END                   0x23            END key
// VK_HOME                  0x24            HOME key
// VK_LEFT                  0x25            LEFT ARROW key
// VK_UP                    0x26            UP ARROW key
// VK_RIGHT                 0x27            RIGHT ARROW key
// VK_DOWN                  0x28            DOWN ARROW key
// VK_SELECT                0x29            SELECT key
// VK_PRINT                 0x2A            PRINT key
// VK_EXECUTE               0x2B            EXECUTE key
// VK_SNAPSHOT              0x2C            PRINT SCREEN key
// VK_INSERT                0x2D            INS key
// VK_DELETE                0x2E            DEL key
// VK_HELP                  0x2F            HELP key
// -                        0x30            Undefined
// -                        0x30 - 39       0 - 9 key
// -                        0x3A - 40       Undefined
// -                        0x41 - 5A       A - Z key
// VK_LWIN                  0x5B            Left Windows key(Natural keyboard)
// VK_RWIN                  0x5C            Right Windows key(Natural keyboard)
// VK_APPS                  0x5D            Applications key(Natural keyboard)
// -                        0x5E            Reserved
// VK_SLEEP                 0x5F            Computer Sleep key
// VK_NUMPAD0               0x60            Numeric keypad 0 key
// VK_NUMPAD1               0x61            Numeric keypad 1 key
// VK_NUMPAD2               0x62            Numeric keypad 2 key
// VK_NUMPAD3               0x63            Numeric keypad 3 key
// VK_NUMPAD4               0x64            Numeric keypad 4 key
// VK_NUMPAD5               0x65            Numeric keypad 5 key
// VK_NUMPAD6               0x66            Numeric keypad 6 key
// VK_NUMPAD7               0x67            Numeric keypad 7 key
// VK_NUMPAD8               0x68            Numeric keypad 8 key
// VK_NUMPAD9               0x69            Numeric keypad 9 key
// VK_MULTIPLY              0x6A            Multiply key
// VK_ADD                   0x6B            Add key
// VK_SEPARATOR             0x6C            Separator key
// VK_SUBTRACT              0x6D            Subtract key
// VK_DECIMAL               0x6E            Decimal key
// VK_DIVIDE                0x6F            Divide key
// VK_F1                    0x70            F1 key
// VK_F2                    0x71            F2 key
// VK_F3                    0x72            F3 key
// VK_F4                    0x73            F4 key
// VK_F5                    0x74            F5 key
// VK_F6                    0x75            F6 key
// VK_F7                    0x76            F7 key
// VK_F8                    0x77            F8 key
// VK_F9                    0x78            F9 key
// VK_F10                   0x79            F10 key
// VK_F11                   0x7A            F11 key
// VK_F12                   0x7B            F12 key
// VK_F13                   0x7C            F13 key
// VK_F14                   0x7D            F14 key
// VK_F15                   0x7E            F15 key
// VK_F16                   0x7F            F16 key
// VK_F17                   0x80            F17 key
// VK_F18                   0x81            F18 key
// VK_F19                   0x82            F19 key
// VK_F20                   0x83            F20 key
// VK_F21                   0x84            F21 key
// VK_F22                   0x85            F22 key
// VK_F23                   0x86            F23 key
// VK_F24                   0x87            F24 key
// -                        0x88 - 8F       Unassigned
// VK_NUMLOCK               0x90            NUM LOCK key
// VK_SCROLL                0x91            SCROLL LOCK key
// -                        0x92 - 96       OEM specific
// -                        0x97 - 9F       Unassigned
// VK_LSHIFT                0xA0            Left SHIFT key
// VK_RSHIFT                0xA1            Right SHIFT key
// VK_LCONTROL              0xA2            Left CONTROL key
// VK_RCONTROL              0xA3            Right CONTROL key
// VK_LMENU                 0xA4            Left MENU key
// VK_RMENU                 0xA5            Right MENU key
// VK_BROWSER_BACK          0xA6            Browser Back key
// VK_BROWSER_FORWARD       0xA7            Browser Forward key
// VK_BROWSER_REFRESH       0xA8            Browser Refresh key
// VK_BROWSER_STOP          0xA9            Browser Stop key
// VK_BROWSER_SEARCH        0xAA            Browser Search key
// VK_BROWSER_FAVORITES     0xAB            Browser Favorites key
// VK_BROWSER_HOME          0xAC            Browser Start and Home key
// VK_VOLUME_MUTE           0xAD            Volume Mute key
// VK_VOLUME_DOWN           0xAE            Volume Down key
// VK_VOLUME_UP             0xAF            Volume Up key
// VK_MEDIA_NEXT_TRACK      0xB0            Next Track key
// VK_MEDIA_PREV_TRACK      0xB1            Previous Track key
// VK_MEDIA_STOP            0xB2            Stop Media key
// VK_MEDIA_PLAY_PAUSE      0xB3            Play / Pause Media key
// VK_LAUNCH_MAIL           0xB4            Start Mail key
// VK_LAUNCH_MEDIA_SELECT   0xB5            Select Media key
// VK_LAUNCH_APP1           0xB6            Start Application 1 key
// VK_LAUNCH_APP2           0xB7            Start Application 2 key
// -                        0xB8 - B9       Reserved
// VK_OEM_1                 0xBA            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the ';:' key
// VK_OEM_PLUS              0xBB            For any country / region, the '+' key
// VK_OEM_COMMA             0xBC            For any country / region, the ',' key
// VK_OEM_MINUS             0xBD            For any country / region, the '-' key
// VK_OEM_PERIOD            0xBE            For any country / region, the '.' key
// VK_OEM_2                 0xBF            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the '/?' key
// VK_OEM_3                 0xC0            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the '`~' key
// -                        0xC1 - D7       Reserved
// -                        0xD8 - DA       Unassigned
// VK_OEM_4                 0xDB            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the '[{' key
// VK_OEM_5                 0xDC            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the '\|' key
// VK_OEM_6                 0xDD            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the ']}' key
// VK_OEM_7                 0xDE            Used for miscellaneous characters; it can vary by keyboard.For the US standard keyboard, the 'single-quote/double-quote' key
// VK_OEM_8                 0xDF            Used for miscellaneous characters; it can vary by keyboard.
// -                        0xE0            Reserved
// -                        0xE1            OEM specific
// VK_OEM_102               0xE2            Either the angle bracket key or the backslash key on the RT 102 - key keyboard
// -                        0xE3 - E4       OEM specific
// -                        0xE4            OEM specific
// VK_PROCESSKEY            0xE5            IME PROCESS key
// -                        0xE6            OEM specific
// VK_PACKET                0xE7            Used to pass Unicode characters as if they were keystrokes.The VK_PACKET key is the low word of a 32 - bit Virtual Key value used for non - keyboard input methods.For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
// -                        0xE8            Unassigned
// -                        0xE9 - F5       OEM specific
// VK_ATTN                  0xF6            Attn key
// VK_CRSEL                 0xF7            CrSel key
// VK_EXSEL                 0xF8            ExSel key
// VK_EREOF                 0xF9            Erase EOF key
// VK_PLAY                  0xFA            Play key
// VK_ZOOM                  0xFB            Zoom key
// VK_NONAME                0xFC            Reserved
// VK_PA1                   0xFD            PA1 key
// VK_OEM_CLEAR             0xFE            Clear key
// ============================================================================

// Sytem-Wide Accelerators
// ---------------------------
// ALT + ESC	         Switches to the next application.
// ALT + F4	           Closes an application or a window.
// ALT + HYPHEN	       Opens the Window menu for a document window.
// ALT + PRINT SCREEN	 Copies an image in the active window onto the clipboard.
// ALT + SPACEBAR	     Opens the Window menu for the application's main window.
// ALT + TAB	         Switches to the next application.
// CTRL + ESC	         Switches to the Start menu.
// CTRL + F4	         Closes the active group or document window.
// F1	                 Starts the application's help file, if one exists.
// PRINT SCREEN	       Copies an image on the screen onto the clipboard.
// SHIFT + ALT + TAB	 Switches to the previous application.The user must press and hold down ALT + SHIFT while pressing TAB.

