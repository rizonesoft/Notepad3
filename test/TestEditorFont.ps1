# Exercise the real font dialog and read the editor's effective Scintilla fonts.
param([Parameter(Mandatory = $true)][string]$ExePath)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if (-not $IsWindows) { throw 'This GUI regression test requires Windows.' }
$ExePath = (Resolve-Path $ExePath).Path
Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
public static class FontProbe {
    public delegate bool EnumProc(IntPtr hwnd, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumChildWindows(IntPtr parent, EnumProc callback, IntPtr data);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint pid);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr hwnd, StringBuilder name, int count);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hwnd);
    [DllImport("user32.dll")] static extern bool IsWindow(IntPtr hwnd);
    [DllImport("user32.dll")] static extern IntPtr GetDlgItem(IntPtr hwnd, int id);
    [DllImport("user32.dll", CharSet=CharSet.Unicode, EntryPoint="SendMessageW")] static extern IntPtr SendText(IntPtr hwnd, uint msg, IntPtr w, string text);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int count);
    [DllImport("user32.dll")] static extern bool PostMessage(IntPtr hwnd, uint msg, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] static extern IntPtr SendMessage(IntPtr hwnd, uint msg, IntPtr w, IntPtr l);
    [DllImport("kernel32.dll", SetLastError=true)] static extern IntPtr OpenProcess(uint rights, bool inherit, int pid);
    [DllImport("kernel32.dll", SetLastError=true)] static extern IntPtr VirtualAllocEx(IntPtr p, IntPtr address, UIntPtr size, uint flags, uint protect);
    [DllImport("kernel32.dll")] static extern bool VirtualFreeEx(IntPtr p, IntPtr address, UIntPtr size, uint flags);
    [DllImport("kernel32.dll")] static extern bool ReadProcessMemory(IntPtr p, IntPtr address, byte[] bytes, UIntPtr count, out UIntPtr done);
    [DllImport("kernel32.dll")] static extern bool WriteProcessMemory(IntPtr p, IntPtr address, byte[] bytes, UIntPtr count, out UIntPtr done);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
    [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)] public struct LogFont {
        public int height, width, escapement, orientation, weight;
        public byte italic, underline, strikeout, charset, outPrecision, clipPrecision, quality, pitchAndFamily;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=32)] public string face;
    }
    static string ClassName(IntPtr h) { var s=new StringBuilder(128); GetClassName(h,s,s.Capacity); return s.ToString(); }
    public static IntPtr FindWindow(int processId, string name) {
        IntPtr result=IntPtr.Zero;
        EnumWindows((h,d)=>{uint pid;GetWindowThreadProcessId(h,out pid);if(pid==processId && ClassName(h)==name && IsWindowVisible(h)){result=h;return false;}return true;},IntPtr.Zero);
        return result;
    }
    public static IntPtr FindEditor(IntPtr parent) {
        IntPtr result=IntPtr.Zero;
        EnumChildWindows(parent,(h,d)=>{if(ClassName(h)=="Scintilla"){result=h;return false;}return true;},IntPtr.Zero);
        return result;
    }
    public static void Command(IntPtr window,int id) { if(!PostMessage(window,0x111,(IntPtr)id,IntPtr.Zero))throw new Exception("PostMessage failed"); }
    public static long Query(IntPtr window,uint msg,int arg) { return SendMessage(window,msg,(IntPtr)arg,IntPtr.Zero).ToInt64(); }
    public static string ReadFont(int pid,IntPtr editor,int style) {
        IntPtr p=OpenProcess(0x438,false,pid);if(p==IntPtr.Zero)throw new Exception("OpenProcess failed");
        IntPtr remote=VirtualAllocEx(p,IntPtr.Zero,(UIntPtr)256,0x3000,4);
        try { if(remote==IntPtr.Zero)throw new Exception("VirtualAllocEx failed"); SendMessage(editor,2486,(IntPtr)style,remote);byte[] bytes=new byte[256];UIntPtr done;if(!ReadProcessMemory(p,remote,bytes,(UIntPtr)bytes.Length,out done))throw new Exception("ReadProcessMemory failed");int end=Array.IndexOf(bytes,(byte)0);return Encoding.UTF8.GetString(bytes,0,end<0?bytes.Length:end); }
        finally { if(remote!=IntPtr.Zero)VirtualFreeEx(p,remote,UIntPtr.Zero,0x8000);CloseHandle(p); }
    }
    public static string SelectFont(int pid,IntPtr dialog,string family) {
        IntPtr combo=GetDlgItem(dialog,0x470);
        if(combo==IntPtr.Zero)throw new Exception("Not a font dialog: "+WindowText(dialog));
        long index=SendText(combo,0x158,(IntPtr)(-1),family).ToInt64(); // CB_FINDSTRINGEXACT
        if(index<0)throw new Exception("Font is not in the dialog list: "+family);
        SendMessage(combo,0x14e,(IntPtr)index,IntPtr.Zero); // CB_SETCURSEL
        SendMessage(dialog,0x111,(IntPtr)(0x10000|0x470),combo); // CBN_SELCHANGE
        int size=Marshal.SizeOf<LogFont>();
        IntPtr p=OpenProcess(0x438,false,pid);if(p==IntPtr.Zero)throw new Exception("OpenProcess failed");
        IntPtr remote=VirtualAllocEx(p,IntPtr.Zero,(UIntPtr)size,0x3000,4);
        string selected;
        try {
            if(remote==IntPtr.Zero)throw new Exception("Allocating test LOGFONT failed");
            SendMessage(dialog,0x401,IntPtr.Zero,remote); // WM_CHOOSEFONT_GETLOGFONT
            byte[] bytes=new byte[size];UIntPtr done;
            if(!ReadProcessMemory(p,remote,bytes,(UIntPtr)size,out done))throw new Exception("Reading dialog LOGFONT failed");
            selected=Encoding.Unicode.GetString(bytes,28,64).TrimEnd('\0');
        } finally {if(remote!=IntPtr.Zero)VirtualFreeEx(p,remote,UIntPtr.Zero,0x8000);CloseHandle(p);}
        string report="title="+WindowText(dialog)+" combo="+WindowText(combo)+" LOGFONT="+selected;
        if(!String.Equals(selected,family,StringComparison.OrdinalIgnoreCase))throw new Exception("Font selection failed: "+report);
        Command(dialog,1);
        return report;
    }
    static string WindowText(IntPtr h) {var text=new StringBuilder(256);GetWindowText(h,text,text.Capacity);return text.ToString();}
    public static bool Exists(IntPtr hwnd){return IsWindow(hwnd);}
}
'@

function Wait-For([scriptblock]$Condition, [string]$Description) {
    $deadline = [DateTime]::UtcNow.AddSeconds(15)
    do {
        $result = & $Condition
        if ($result -is [IntPtr]) {
            if ($result -ne [IntPtr]::Zero) { return $result }
        } elseif ($result) { return $result }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Timed out: $Description"
}

$testRoot = Join-Path $env:RUNNER_TEMP ('notepad3-font-' + [Guid]::NewGuid().ToString('N'))
New-Item $testRoot -ItemType Directory | Out-Null
$results = [Collections.Generic.List[object]]::new()
$fontIssues = [Collections.Generic.List[string]]::new()
foreach ($extension in @('bdf', 'mac', 'txt')) {
    $ini = Join-Path $testRoot ($extension + '.ini')
    $sample = Join-Path $testRoot ('sample.' + $extension)
    Set-Content $sample -Value 'GRID,1,,0.,0.,0.' -Encoding utf8
    Set-Content $ini -Encoding utf8 -Value @'
[Settings]
SaveSettings=1
[Settings2]
PreferredLanguageLocaleName=en-US
[Common Base]
Default Style=font:Consolas; size:11
'@
    $app = Start-Process $ExePath -ArgumentList "/n /f `"$ini`" `"$sample`"" -PassThru
    try {
        $main = Wait-For { $app.Refresh(); if ($app.HasExited) { throw "Editor exited: $($app.ExitCode)" }; $app.MainWindowHandle } 'main editor window'
        $editor = Wait-For { [FontProbe]::FindEditor($main) } 'Scintilla editor'
        $before = [FontProbe]::ReadFont($app.Id, $editor, 32)
        $lexer = [FontProbe]::Query($editor, 4002, 0)
        [FontProbe]::Command($main, 41004)
        $dialog = Wait-For { [FontProbe]::FindWindow($app.Id, '#32770') } 'global font dialog'
        Write-Host ([FontProbe]::SelectFont($app.Id, $dialog, 'Arial'))
        Wait-For { -not [FontProbe]::Exists($dialog) } 'font dialog OK' | Out-Null
        Start-Sleep -Milliseconds 250
        $after = [FontProbe]::ReadFont($app.Id, $editor, 32)
        $token = [FontProbe]::ReadFont($app.Id, $editor, 0)
        [FontProbe]::Command($main, 42035)
        Start-Sleep -Milliseconds 250
        Write-Host "FONT RESULT $extension lexer=$lexer default=$before -> $after token0=$token"
        Write-Host "INI AFTER $extension`n$(Get-Content $ini -Raw)"
        $results.Add([pscustomobject]@{ extension=$extension; lexer=$lexer; before=$before; after=$after; token0=$token })
        # Text Files deliberately has its own $Text0 override. APDL/NASTRAN inherit Common Base.
        if ($extension -ne 'txt' -and ($after -ne 'Arial' -or $token -ne 'Arial')) {
            $fontIssues.Add("Global font was not applied to $extension")
        }
        [FontProbe]::Command($main, 41044)
        $dialog = Wait-For { [FontProbe]::FindWindow($app.Id, '#32770') } 'current scheme font dialog'
        Write-Host ([FontProbe]::SelectFont($app.Id, $dialog, 'Courier New'))
        Wait-For { -not [FontProbe]::Exists($dialog) } 'scheme font dialog OK' | Out-Null
        Start-Sleep -Milliseconds 250
        $scheme = [FontProbe]::ReadFont($app.Id, $editor, 32)
        Write-Host "SCHEME FONT RESULT $extension -> $scheme"
        if ($scheme -ne 'Courier New') { $fontIssues.Add("Scheme font was not applied to $extension") }
    } finally {
        if (-not $app.HasExited) { [FontProbe]::Command($app.MainWindowHandle, 40025); if (-not $app.WaitForExit(5000)) { $app.Kill() } }
    }
}
$results | ConvertTo-Json | Write-Host
if ($fontIssues.Count) { throw ($fontIssues -join '; ') }
Write-Host 'Editor font GUI regression passed.'
