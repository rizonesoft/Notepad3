# 
# Thanks to "jooseng" for this little macro "Custom_Code_Post_Install".
#
!macro CustomCodePostInstall
    ${If} ${FileExists} "$INSTDIR\Other\Source\Notepad3Portable.cmd"
        CopyFiles /SILENT "$INSTDIR\Other\Source\Notepad3Portable.cmd" "$INSTDIR\Notepad3Portable.cmd"
    ${EndIf}
!macroend

