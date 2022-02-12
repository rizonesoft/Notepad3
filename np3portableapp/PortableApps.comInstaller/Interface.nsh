/*

NSIS Modern User Interface
Interface code for all pages
d:\PortableApps\PortableApps.comInstaller\App\nsis\Contrib\Modern UI 2\Interface.nsh
*/

;--------------------------------
;Variables

Var mui.Header.Text
Var mui.Header.Text.Font
Var mui.Header.SubText
Var mui.Header.Background
Var mui.Header.Image

Var mui.Branding.Text
Var mui.Branding.Background

Var mui.Line.Standard
Var mui.Line.FullWindow

Var mui.Button.Next
Var mui.Button.Cancel
Var mui.Button.Back


;--------------------------------
;General interface settings

!macro MUI_INTERFACE

  !ifndef MUI_INTERFACE

    !define MUI_INTERFACE

    ;These values are set after the interface settings in the script,
    ;so the script itself can override all values.

    ;Default interface settings in nsisconf.nsh
    !ifdef MUI_INSERT_NSISCONF
      !insertmacro MUI_NSISCONF
    !endif

    ;Default interface settings
    !insertmacro MUI_DEFAULT MUI_UI "${NSISDIR}\Contrib\UIs\modern.exe"
    !insertmacro MUI_DEFAULT MUI_UI_HEADERIMAGE "${NSISDIR}\Contrib\UIs\modern_headerbmp.exe"
    !insertmacro MUI_DEFAULT MUI_UI_HEADERIMAGE_RIGHT "${NSISDIR}\Contrib\UIs\modern_headerbmpr.exe"
    !insertmacro MUI_DEFAULT MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
    !insertmacro MUI_DEFAULT MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
    !insertmacro MUI_DEFAULT MUI_BGCOLOR "FFFFFF"
    !insertmacro MUI_DEFAULT MUI_TEXTCOLOR "000000"

    ;Map *_NOSTRETCH legacy define to the correct *_STRETCH value
    !verbose push 2
    !insertmacro MUI_LEGACY_MAP_NOSTRETCH MUI_HEADERIMAGE_ ""
    !insertmacro MUI_LEGACY_MAP_NOSTRETCH MUI_WELCOMEFINISHPAGE_ ""
    !insertmacro MUI_LEGACY_MAP_NOSTRETCH MUI_UNWELCOMEFINISHPAGE_ ""
    !verbose pop

    ;Default header images
    !ifdef MUI_HEADERIMAGE

      !insertmacro MUI_DEFAULT MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
      !insertmacro MUI_DEFAULT MUI_HEADERIMAGE_BITMAP_STRETCH "FitControl"
      !insertmacro MUI_DEFAULT MUI_HEADERIMAGE_BITMAP_RTL_STRETCH ${MUI_HEADERIMAGE_BITMAP_STRETCH}

      !ifndef MUI_HEADERIMAGE_UNBITMAP
        !define MUI_HEADERIMAGE_UNBITMAP "${MUI_HEADERIMAGE_BITMAP}"
        !insertmacro MUI_SET MUI_HEADERIMAGE_UNBITMAP_STRETCH ${MUI_HEADERIMAGE_BITMAP_STRETCH}
      !endif

      !if "${MUI_HEADERIMAGE_BITMAP}" == ""
        !error "Invalid MUI_HEADERIMAGE_BITMAP"
      !endif
      !if "${MUI_HEADERIMAGE_UNBITMAP}" == ""
        !error "Invalid MUI_HEADERIMAGE_UNBITMAP"
      !endif

      !ifdef MUI_HEADERIMAGE_BITMAP_RTL
        !ifndef MUI_HEADERIMAGE_UNBITMAP_RTL
          !define MUI_HEADERIMAGE_UNBITMAP_RTL "${MUI_HEADERIMAGE_BITMAP_RTL}"
          !insertmacro MUI_SET MUI_HEADERIMAGE_UNBITMAP_RTL_STRETCH ${MUI_HEADERIMAGE_BITMAP_RTL_STRETCH}
        !endif

        !if "${MUI_HEADERIMAGE_BITMAP_RTL}" == ""
          !error "Invalid MUI_HEADERIMAGE_BITMAP_RTL"
        !endif
        !if "${MUI_HEADERIMAGE_UNBITMAP_RTL}" == ""
          !error "Invalid MUI_HEADERIMAGE_UNBITMAP_RTL"
        !endif
      !endif

      !insertmacro MUI_DEFAULT MUI_HEADERIMAGE_UNBITMAP_STRETCH ${MUI_HEADERIMAGE_BITMAP_STRETCH}
      !insertmacro MUI_DEFAULT MUI_HEADERIMAGE_UNBITMAP_RTL_STRETCH ${MUI_HEADERIMAGE_BITMAP_RTL_STRETCH}

    !endif

    ;Default texts
    !insertmacro MUI_DEFAULT MUI_ABORTWARNING_TEXT "$(MUI_TEXT_ABORTWARNING)"
    !insertmacro MUI_DEFAULT MUI_UNABORTWARNING_TEXT "$(MUI_UNTEXT_ABORTWARNING)"  

    ;Apply settings

    XPStyle On ;XP style setting in manifest resource

    ;Dialog resources
    ChangeUI all "${MUI_UI}" 
    !ifdef MUI_HEADERIMAGE
      !ifndef MUI_HEADERIMAGE_RIGHT
        ChangeUI IDD_INST "${MUI_UI_HEADERIMAGE}"
      !else
        ChangeUI IDD_INST "${MUI_UI_HEADERIMAGE_RIGHT}"
      !endif
    !endif

    ;Icons
    Icon "${MUI_ICON}"
    UninstallIcon "${MUI_UNICON}"

  !endif

!macroend


;--------------------------------
;Abort warning message box

!macro MUI_ABORTWARNING

  !ifdef MUI_ABORTWARNING_CANCEL_DEFAULT
    MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2 "${MUI_ABORTWARNING_TEXT}" IDYES mui.Quit
  !else
    MessageBox MB_YESNO|MB_ICONEXCLAMATION "${MUI_ABORTWARNING_TEXT}" IDYES mui.Quit
  !endif
  
  Abort
  mui.Quit:

!macroend

!macro MUI_UNABORTWARNING

  !ifdef MUI_UNABORTWARNING_CANCEL_DEFAULT
    MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2 "${MUI_UNABORTWARNING_TEXT}" IDYES mui.Quit
  !else
    MessageBox MB_YESNO|MB_ICONEXCLAMATION "${MUI_UNABORTWARNING_TEXT}" IDYES mui.Quit
  !endif

  Abort
  mui.Quit:

!macroend


;--------------------------------
;Initialization of GUI

!macro MUI_HEADERIMAGE_INITHELPER_LOADIMAGEWITHMACRO MACRO

  !ifdef MUI_HEADERIMAGE_RIGHT
    !ifndef MUI_OPTIMIZE_ALWAYSLTR ; Undocumented
      ${if} $(^RTL) == 1
        !insertmacro ${MACRO} $mui.Header.Image "${PATH}" Left Leak
      ${Else}
        !insertmacro ${MACRO} $mui.Header.Image "${PATH}" Right Leak
      ${EndIf}
    !else
      !insertmacro ${MACRO} $mui.Header.Image "${PATH}" Right Leak
    !endif
  !else
    !insertmacro ${MACRO} $mui.Header.Image "${PATH}" Auto Leak
  !endif

!macroend
!macro MUI_HEADERIMAGE_INITHELPER_LOADIMAGE UN RTL IMGRESID PATH

  GetDlgItem $mui.Header.Image $HWNDPARENT ${IMGRESID} ; This variable is not used by every mode but we have to reference it to avoid a compiler warning.

  !if "${MUI_HEADERIMAGE_${UN}BITMAP${RTL}_STRETCH}" == "NoStretchNoCropNoAlign"

    SetBrandingImage /IMGID=${IMGRESID} "${PATH}"

  !else if "${MUI_HEADERIMAGE_${UN}BITMAP${RTL}_STRETCH}" == "NoStretchNoCrop"

    !insertmacro MUI_HEADERIMAGE_INITHELPER_LOADIMAGEWITHMACRO \
      MUI_LOADANDXALIGNIMAGE

  !else if "${MUI_HEADERIMAGE_${UN}BITMAP${RTL}_STRETCH}" == "AspectFitHeight"

    !insertmacro MUI_HEADERIMAGE_INITHELPER_LOADIMAGEWITHMACRO \
      MUI_LOADANDASPECTSTRETCHIMAGETOCONTROLHEIGHT

  !else

    !if "${MUI_HEADERIMAGE_${UN}BITMAP${RTL}_STRETCH}" != "FitControl"
      !warning 'MUI_HEADERIMAGE_${UN}BITMAP${RTL}_STRETCH set to unknown value, defaulting to FitControl'
    !endif
    SetBrandingImage /IMGID=${IMGRESID} /RESIZETOFIT "${PATH}"

  !endif

!macroend

!macro MUI_HEADERIMAGE_INIT UN IMGRESID

  ;Load and display header image

  !ifdef MUI_HEADERIMAGE

    InitPluginsDir

    !ifdef MUI_HEADERIMAGE_${UN}BITMAP_RTL
      ${if} $(^RTL) == 1

        File "/oname=$PLUGINSDIR\modern-header-rtl.bmp" "${MUI_HEADERIMAGE_${UN}BITMAP_RTL}"
        !pragma verifyloadimage "${MUI_HEADERIMAGE_${UN}BITMAP_RTL}"
        !insertmacro MUI_HEADERIMAGE_INITHELPER_LOADIMAGE "${UN}" "_RTL" ${IMGRESID} "$PLUGINSDIR\modern-header-rtl.bmp"

      ${else}
    !endif

        File "/oname=$PLUGINSDIR\modern-header.bmp" "${MUI_HEADERIMAGE_${UN}BITMAP}"
        !pragma verifyloadimage "${MUI_HEADERIMAGE_${UN}BITMAP}"
        !insertmacro MUI_HEADERIMAGE_INITHELPER_LOADIMAGE "${UN}" "" ${IMGRESID} "$PLUGINSDIR\modern-header.bmp"

    !ifdef MUI_HEADERIMAGE_${UN}BITMAP_RTL
      ${endif}
    !endif

  !endif

!macroend

!macro MUI_GUIINIT_OUTERDIALOG UNINSTALLER

  ;Initialize outer dialog (fonts & colors)

  ;Header
  GetDlgItem $mui.Header.Text $HWNDPARENT 1037
  CreateFont $mui.Header.Text.Font "$(^Font)" "$(^FontSize)" "700"
  SendMessage $mui.Header.Text ${WM_SETFONT} $mui.Header.Text.Font 0

  GetDlgItem $mui.Header.SubText $HWNDPARENT 1038

  !ifndef MUI_HEADER_TRANSPARENT_TEXT
    SetCtlColors $mui.Header.Text "${MUI_TEXTCOLOR}" "${MUI_BGCOLOR}"
    SetCtlColors $mui.Header.SubText "${MUI_TEXTCOLOR}" "${MUI_BGCOLOR}"
  !else
    SetCtlColors $mui.Header.Text "${MUI_TEXTCOLOR}" "transparent"
    SetCtlColors $mui.Header.SubText "${MUI_TEXTCOLOR}" "transparent"
  !endif

  ;Header image
  !insertmacro MUI_HEADERIMAGE_INIT "${UNINSTALLER}" 1046

  ;Header background
  GetDlgItem $mui.Header.Background $HWNDPARENT 1034
  SetCtlColors $mui.Header.Background "" "${MUI_BGCOLOR}"

  ;Header icon image background
  !ifndef MUI_HEADERIMAGE
    GetDlgItem $mui.Header.Image $HWNDPARENT 1039
    SetCtlColors $mui.Header.Image "" "${MUI_BGCOLOR}"
  !endif

  ;Branding text
  GetDlgItem $mui.Branding.Background $HWNDPARENT 1028
  SetCtlColors $mui.Branding.Background /BRANDING
  GetDlgItem $mui.Branding.Text $HWNDPARENT 1256
  SetCtlColors $mui.Branding.Text /BRANDING
  SendMessage $mui.Branding.Text ${WM_SETTEXT} 0 "STR:$(^Branding) "

  ;Lines
  GetDlgItem $mui.Line.Standard $HWNDPARENT 1035
  GetDlgItem $mui.Line.FullWindow $HWNDPARENT 1045

  ;Buttons
  GetDlgItem $mui.Button.Next $HWNDPARENT 1
  GetDlgItem $mui.Button.Cancel $HWNDPARENT 2
  GetDlgItem $mui.Button.Back $HWNDPARENT 3

!macroend


;--------------------------------
;Interface functions

!macro MUI_FUNCTION_GUIINIT

  Function .onGUIInit

    !insertmacro MUI_GUIINIT_OUTERDIALOG ""

    !ifdef MUI_PAGE_FUNCTION_GUIINIT
      Call "${MUI_PAGE_FUNCTION_GUIINIT}"
    !endif  

    !ifdef MUI_CUSTOMFUNCTION_GUIINIT
      Call "${MUI_CUSTOMFUNCTION_GUIINIT}"
    !endif

  FunctionEnd

!macroend

!macro MUI_UNFUNCTION_GUIINIT

  Function un.onGUIInit  

    !insertmacro MUI_GUIINIT_OUTERDIALOG UN
    
    !ifdef MUI_UNPAGE_FUNCTION_GUIINIT
      Call "${MUI_UNPAGE_FUNCTION_GUIINIT}"
    !endif    

    !ifdef MUI_CUSTOMFUNCTION_UNGUIINIT
      Call "${MUI_CUSTOMFUNCTION_UNGUIINIT}"
    !endif

  FunctionEnd

!macroend

!macro MUI_FUNCTION_ABORTWARNING

  Function .onUserAbort
  
    !ifdef MUI_PAGE_FUNCTION_ABORTWARNING
      Call ${MUI_PAGE_FUNCTION_ABORTWARNING}
    !endif
  
    !ifdef MUI_ABORTWARNING
      !insertmacro MUI_ABORTWARNING
    !endif
    
    !ifdef MUI_CUSTOMFUNCTION_ABORT
      Call "${MUI_CUSTOMFUNCTION_ABORT}"
    !endif
    
  FunctionEnd

!macroend

!macro MUI_FUNCTION_UNABORTWARNING

  Function un.onUserAbort
  
    !ifdef MUI_UNPAGE_FUNCTION_ABORTWARNING
      Call ${MUI_UNPAGE_FUNCTION_ABORTWARNING}
    !endif  
  
    !ifdef MUI_UNABORTWARNING
      !insertmacro MUI_UNABORTWARNING
    !endif
    
    !ifdef MUI_CUSTOMFUNCTION_UNABORT
      Call "${MUI_CUSTOMFUNCTION_UNABORT}"
    !endif
    
  FunctionEnd

!macroend
