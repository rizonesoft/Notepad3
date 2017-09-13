;
; Active Window Info
;

#NoEnv
#NoTrayIcon
#SingleInstance Ignore
SetWorkingDir, %A_ScriptDir%
SetBatchLines, -1
CoordMode, Pixel, Screen

IfExist, ..\toolicon.icl ; Seems useful enough to support standalone operation.
	Menu, Tray, Icon, ..\toolicon.icl, 9

try
    Hotkey #a, FreezeDisplay
catch
    Hotkey #vk41, FreezeDisplay

isUpd := true
txtNotFrozen := "(Win+A to freeze display)"
txtFrozen := "(Win+A to unfreeze display)"

Gui, New, hwndhGui AlwaysOnTop Resize MinSize
Gui, Add, Text,, Window Title, Class and Process:
Gui, Add, Edit, w320 r3 ReadOnly -Wrap vCtrl_Title
Gui, Add, Text,, Mouse Position:
Gui, Add, Edit, w320 r3 ReadOnly vCtrl_MousePos
Gui, Add, Text,, Control Under Mouse Position:
Gui, Add, Edit, w320 r5 ReadOnly vCtrl_MouseCur
Gui, Add, Text,, Active Window Position:
Gui, Add, Edit, w320 r2 ReadOnly vCtrl_Pos
Gui, Add, Text,, Status Bar Text:
Gui, Add, Edit, w320 r2 ReadOnly vCtrl_SBText
Gui, Add, Checkbox, vCtrl_IsSlow, Slow TitleMatchMode
Gui, Add, Text,, Visible Text:
Gui, Add, Edit, w320 r2 ReadOnly vCtrl_VisText
Gui, Add, Text,, All Text:
Gui, Add, Edit, w320 r2 ReadOnly vCtrl_AllText
Gui, Add, Text, w320 r1 vCtrl_Freeze, % txtNotFrozen
Gui, Show,, Active Window Info
GetClientSize(hGui, temp)
horzMargin := temp*96//A_ScreenDPI - 320
SetTimer, Update, 250
return

GuiSize:
Gui %hGui%:Default
if !horzMargin
	return
ctrlW := A_GuiWidth - horzMargin
list = Title,MousePos,MouseCur,Pos,SBText,VisText,AllText,Freeze
Loop, Parse, list, `,
	GuiControl, Move, Ctrl_%A_LoopField%, w%ctrlW%
return

Update:
Gui %hGui%:Default
curWin := WinExist("A")
if (curWin = hGui)
	return
WinGetTitle, t1
WinGetClass, t2
WinGet, t3, ProcessName
GuiControl,, Ctrl_Title, % t1 "`nahk_class " t2 "`nahk_exe " t3
CoordMode, Mouse, Screen
MouseGetPos, msX, msY, msWin, msCtrlHwnd, 2
CoordMode, Mouse, Relative
MouseGetPos, mrX, mrY,, msCtrl
CoordMode, Mouse, Client
MouseGetPos, mcX, mcY
GuiControl,, Ctrl_MousePos, % "Absolute:`t" msX ", " msY " (less often used)`nRelative:`t" mrX ", " mrY " (default)`nClient:`t" mcX ", " mcY " (recommended)"
PixelGetColor, mClr, %msX%, %msY%, RGB
mClr := SubStr(mClr, 3)
mText := "`nColor:`t" mClr " (Red=" SubStr(mClr, 1, 2) " Green=" SubStr(mClr, 3, 2) " Blue=" SubStr(mClr, 5) ")"
if (curWin = msWin)
{
	ControlGetText, ctrlTxt, %msCtrl%
	mText := "ClassNN:`t" msCtrl "`nText:`t" textMangle(ctrlTxt) mText
    ControlGetPos cX, cY, cW, cH, %msCtrl%
    mText .= "`n`tx: " cX "`ty: " cY "`tw: " cW "`th: " cH
    WinToClient(msWin, cX, cY)
    GetClientSize(msCtrlHwnd, cW, cH)
    mText .= "`nClient:`tx: " cX "`ty: " cY "`tw: " cW "`th: " cH
} else mText := "`n" mText
GuiControl,, Ctrl_MouseCur, % mText
WinGetPos, wX, wY, wW, wH
GetClientSize(curWin, wcW, wcH)
GuiControl,, Ctrl_Pos, % "`tx: " wX "`ty: " wY "`tw: " wW "`th: " wH "`nClient:`t`t`tw: " wcW "`th: " wcH
sbTxt := ""
Loop
{
	StatusBarGetText, ovi, %A_Index%
	if ovi =
		break
	sbTxt .= "(" A_Index "):`t" textMangle(ovi) "`n"
}
StringTrimRight, sbTxt, sbTxt, 1
GuiControl,, Ctrl_SBText, % sbTxt
GuiControlGet, bSlow,, Ctrl_IsSlow
if bSlow
{
	DetectHiddenText, Off
	WinGetText, ovVisText
	DetectHiddenText, On
	WinGetText, ovAllText
}
else
{
	ovVisText := WinGetTextFast(false)
	ovAllText := WinGetTextFast(true)
}
GuiControl,, Ctrl_VisText, % ovVisText
GuiControl,, Ctrl_AllText, % ovAllText
return

GuiClose:
ExitApp

WinGetTextFast(detect_hidden)
{
	; WinGetText ALWAYS uses the "fast" mode - TitleMatchMode only affects
	; WinText/ExcludeText parameters.  In Slow mode, GetWindowText() is used
	; to retrieve the text of each control.
	WinGet controls, ControlListHwnd
	static WINDOW_TEXT_SIZE := 32767 ; Defined in AutoHotkey source.
	VarSetCapacity(buf, WINDOW_TEXT_SIZE * (A_IsUnicode ? 2 : 1))
	text := ""
	Loop Parse, controls, `n
	{
		if !detect_hidden && !DllCall("IsWindowVisible", "ptr", A_LoopField)
			continue
		if !DllCall("GetWindowText", "ptr", A_LoopField, "str", buf, "int", WINDOW_TEXT_SIZE)
			continue
		text .= buf "`r`n"
	}
	return text
}

GetClientSize(hWnd, ByRef w := "", ByRef h := "")
{
	VarSetCapacity(rect, 16)
	DllCall("GetClientRect", "ptr", hWnd, "ptr", &rect)
	w := NumGet(rect, 8, "int")
	h := NumGet(rect, 12, "int")
}

WinToClient(hWnd, ByRef x, ByRef y)
{
    WinGetPos wX, wY,,, ahk_id %hWnd%
    x += wX, y += wY
    VarSetCapacity(pt, 8), NumPut(y, NumPut(x, pt, "int"), "int")
    if !DllCall("ScreenToClient", "ptr", hWnd, "ptr", &pt)
        return false
    x := NumGet(pt, 0, "int"), y := NumGet(pt, 4, "int")
    return true
}

textMangle(x)
{
	if pos := InStr(x, "`n")
		x := SubStr(x, 1, pos-1), elli := true
	if StrLen(x) > 40
	{
		StringLeft, x, x, 40
		elli := true
	}
	if elli
		x .= " (...)"
	return x
}

FreezeDisplay:
Gui %hGui%:Default
isUpd := !isUpd
SetTimer, Update, % isUpd ? "On" : "Off"
GuiControl,, Ctrl_Freeze, % isUpd ? txtNotFrozen : txtFrozen
return
