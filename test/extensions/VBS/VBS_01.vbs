Option Explicit

Rem -- If Execute per error, then exit immediatly !!!
wscript.quit

Rem -- Debugging
Dim context
Set context = CreateObject("Scripting.Dictionary")
wscript.echo DateYYYYMMDD()
wscript.echo FileCounter()

Rem -- MACRO REGION BEGINS

Rem -- Keep track of if it is the first time a macro runs.
Rem -- Macros may run multiple times when the GUI is shown.
Dim firstTime
firstTime = True

Rem -- Return the current date in YYYYMMDD format
Function DateYYYYMMDD()
    Dim retv, d

    d = Now
    retv = Right("0000" & Year(d), 4) & Right("00" & Month(d), 2) & Right("00" & Day(d), 2)
    DateYYYYMMDD = retv
End Function

Rem -- Example of document counter
Function FileCounter()
    Dim filename, objShell, procEnv, fso, f, filecontent, cnt
    Const ForReading = 1, ForWriting = 2

    Rem -- Create the COM objects needed for this function
    Set objShell = CreateObject("WScript.Shell")
    Set procEnv = objShell.Environment("Process")
    Set fso = CreateObject("Scripting.FileSystemobject")

    Rem -- Get file name of counter file
    Rem -- Each user will have their own counter file in this example
    filename = procEnv("APPDATA") & "\pdf-counter.txt"
    
    If fso.FileExists(filename) Then
        Rem -- Load Counter
        Set f = fso.OpenTextFile(filename, ForReading)
        filecontent = f.ReadAll
        f.Close
        cnt = CLng(filecontent)
    Else
        cnt = 0
    End If
    cnt = cnt + 1

    Rem -- Save Counter if it is the first time we increment it
    If firstTime Then
        Set f = fso.OpenTextFile(filename, ForWriting, True)
        f.Write CStr(cnt)
        f.Close
    End If

    Rem -- Clean up
    Set procEnv = Nothing
    Set objShell = Nothing
    Set fso = Nothing

    firstTime = false

    FileCounter = CStr(cnt)
End Function
