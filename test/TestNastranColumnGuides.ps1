# Exercise guide presets through real Windows menus and syntax selection dialogs.
param(
    [Parameter(Mandatory = $true)][string]$ExePath,
    [string[]]$FontFiles = @()
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if (-not $IsWindows) { throw 'This integration test requires Windows.' }
$ExePath = (Resolve-Path $ExePath).Path
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class GuideProbe {
    [DllImport("gdi32.dll", CharSet=CharSet.Unicode)] public static extern int AddFontResourceEx(string path,uint flags,IntPtr reserved);
    [DllImport("gdi32.dll", CharSet=CharSet.Unicode)] public static extern bool RemoveFontResourceEx(string path,uint flags,IntPtr reserved);
    delegate bool EnumProc(IntPtr h, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumChildWindows(IntPtr parent, EnumProc callback, IntPtr data);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr h, StringBuilder name, int count);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr h);
    [DllImport("user32.dll")] static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] static extern IntPtr SendMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] static extern IntPtr GetDlgItem(IntPtr h, int id);
    [DllImport("user32.dll")] static extern IntPtr GetMenu(IntPtr h);
    [DllImport("user32.dll")] static extern IntPtr GetSubMenu(IntPtr h, int index);
    [DllImport("user32.dll")] static extern int GetMenuItemCount(IntPtr h);
    [DllImport("user32.dll")] static extern uint GetMenuItemID(IntPtr h, int index);
    [DllImport("user32.dll")] static extern uint GetMenuState(IntPtr h, uint id, uint flags);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetMenuString(IntPtr h, uint id, StringBuilder text, int count, uint flags);
    [DllImport("kernel32.dll", SetLastError=true)] static extern IntPtr OpenProcess(uint access, bool inherit, int pid);
    [DllImport("kernel32.dll", SetLastError=true)] static extern IntPtr VirtualAllocEx(IntPtr p, IntPtr address, UIntPtr size, uint flags, uint protection);
    [DllImport("kernel32.dll")] static extern bool VirtualFreeEx(IntPtr p, IntPtr address, UIntPtr size, uint flags);
    [DllImport("kernel32.dll")] static extern bool WriteProcessMemory(IntPtr p, IntPtr address, byte[] bytes, UIntPtr count, out UIntPtr done);
    [DllImport("kernel32.dll")] static extern bool ReadProcessMemory(IntPtr p, IntPtr address, byte[] bytes, UIntPtr count, out UIntPtr done);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr h);
    sealed class Remote : IDisposable {
        readonly IntPtr process;
        public readonly IntPtr Address;
        public Remote(int pid, int size) {
            process=OpenProcess(0x438,false,pid);
            if(process==IntPtr.Zero)throw new Exception("OpenProcess failed");
            Address=VirtualAllocEx(process,IntPtr.Zero,(UIntPtr)size,0x3000,4);
            if(Address==IntPtr.Zero){CloseHandle(process);throw new Exception("VirtualAllocEx failed");}
        }
        public void Write(byte[] bytes){UIntPtr done;if(!WriteProcessMemory(process,Address,bytes,(UIntPtr)bytes.Length,out done)||done.ToUInt64()!=(ulong)bytes.Length)throw new Exception("WriteProcessMemory failed");}
        public byte[] Read(int size,int offset=0){byte[] bytes=new byte[size];UIntPtr done;if(!ReadProcessMemory(process,IntPtr.Add(Address,offset),bytes,(UIntPtr)size,out done)||done.ToUInt64()!=(ulong)size)throw new Exception("ReadProcessMemory failed");return bytes;}
        public void Dispose(){VirtualFreeEx(process,Address,UIntPtr.Zero,0x8000);CloseHandle(process);}
    }
    static string ClassName(IntPtr h){var text=new StringBuilder(128);GetClassName(h,text,text.Capacity);return text.ToString();}
    public static IntPtr Editor(IntPtr parent){IntPtr found=IntPtr.Zero;EnumChildWindows(parent,(h,d)=>{if(ClassName(h)=="Scintilla"){found=h;return false;}return true;},IntPtr.Zero);return found;}
    public static IntPtr SchemeDialog(int processId){IntPtr found=IntPtr.Zero;EnumWindows((h,d)=>{uint pid;GetWindowThreadProcessId(h,out pid);if(pid==processId&&ClassName(h)=="#32770"&&IsWindowVisible(h)&&GetDlgItem(h,18053)!=IntPtr.Zero){found=h;return false;}return true;},IntPtr.Zero);return found;}
    public static void Command(IntPtr h,int id){if(!PostMessage(h,0x111,(IntPtr)id,IntPtr.Zero))throw new Exception("PostMessage failed");}
    public static long Query(IntPtr h,uint msg,int w=0){return SendMessage(h,msg,(IntPtr)w,IntPtr.Zero).ToInt64();}
    public static string Edges(IntPtr editor){var result=new System.Collections.Generic.List<long>();for(int i=0;i<256;i++){long column=Query(editor,2749,i);if(column<0)break;result.Add(column);}return String.Join(" ",result);}
    public static string Font(int pid,IntPtr editor){using(var r=new Remote(pid,256)){SendMessage(editor,2486,(IntPtr)32,r.Address);return Encoding.UTF8.GetString(r.Read(256)).Split('\0')[0];}}
    public static byte[] Text(int pid,IntPtr editor){int size=checked((int)Query(editor,2006));using(var r=new Remote(pid,size+1)){SendMessage(editor,2182,(IntPtr)(size+1),r.Address);return r.Read(size);}}
    static IntPtr MenuContaining(IntPtr menu,int id){for(int i=0;i<GetMenuItemCount(menu);i++){if(GetMenuItemID(menu,i)==id)return menu;IntPtr sub=GetSubMenu(menu,i);if(sub!=IntPtr.Zero){IntPtr found=MenuContaining(sub,id);if(found!=IntPtr.Zero)return found;}}return IntPtr.Zero;}
    public static uint MenuState(IntPtr main,int id){IntPtr menu=GetMenu(main);SendMessage(main,0x116,menu,IntPtr.Zero);IntPtr parent=MenuContaining(menu,id);if(parent==IntPtr.Zero)throw new Exception("Missing menu command "+id);return GetMenuState(parent,(uint)id,0);}
    public static string MenuText(IntPtr main,int id){IntPtr parent=MenuContaining(GetMenu(main),id);var text=new StringBuilder(128);GetMenuString(parent,(uint)id,text,text.Capacity,0);return text.ToString();}
    public static void SelectScheme(int pid,IntPtr dialog,string name){
        IntPtr list=GetDlgItem(dialog,18053);
        using(var r=new Remote(pid,1024)){
            byte[] item=new byte[88]; // LVITEMW, x64
            Array.Copy(BitConverter.GetBytes(r.Address.ToInt64()+128),0,item,24,8);
            Array.Copy(BitConverter.GetBytes(256),0,item,32,4);
            int count=(int)Query(list,0x1004), selected=-1;
            for(int i=0;i<count;i++){
                r.Write(item);SendMessage(list,0x1073,(IntPtr)i,r.Address); // LVM_GETITEMTEXTW
                string text=Encoding.Unicode.GetString(r.Read(512,128)).Split('\0')[0];
                if(String.Equals(text,name,StringComparison.OrdinalIgnoreCase)){selected=i;break;}
            }
            if(selected<0)throw new Exception("Scheme not found: "+name);
            item=new byte[88];Array.Copy(BitConverter.GetBytes(3),0,item,16,4);r.Write(item);
            SendMessage(list,0x102b,(IntPtr)(-1),r.Address); // clear selected/focused
            Array.Copy(BitConverter.GetBytes(3),0,item,12,4);r.Write(item);
            SendMessage(list,0x102b,(IntPtr)selected,r.Address);
        }
        Command(dialog,1);
    }
}
'@
function Wait-For([scriptblock]$Condition, [string]$Description) {
    $deadline = [DateTime]::UtcNow.AddSeconds(15)
    do {
        $result = & $Condition
        if ($result -is [IntPtr]) { if ($result -ne [IntPtr]::Zero) { return $result } }
        elseif ($result) { return $result }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Timed out: $Description"
}
function Assert-Guides([string]$Expected, [int]$Mode = 3) {
    Wait-For { [GuideProbe]::Query($script:editor,2362) -eq $Mode -and [GuideProbe]::Edges($script:editor) -eq $Expected } "guides: mode=$Mode columns=$Expected" | Out-Null
    Write-Host "GUIDES mode=$Mode columns=$Expected"
}
function Select-Scheme([string]$Name, [int]$Lexer) {
    [GuideProbe]::Command($script:main,41001)
    $dialog = Wait-For { [GuideProbe]::SchemeDialog($script:app.Id) } 'syntax selection dialog'
    Start-Sleep -Milliseconds 200
    [GuideProbe]::SelectScheme($script:app.Id,$dialog,$Name)
    Wait-For { -not [GuideProbe]::IsWindow($dialog) -and [GuideProbe]::Query($script:editor,4002) -eq $Lexer } "scheme $Name" | Out-Null
}
function Hash-Bytes([byte[]]$Bytes) { [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($Bytes)) }
$testRoot = Join-Path $env:RUNNER_TEMP ('nastran-guides-' + [Guid]::NewGuid().ToString('N'))
New-Item $testRoot -ItemType Directory | Out-Null
$sample = Join-Path $testRoot 'columns.bdf'
Copy-Item (Join-Path $PSScriptRoot 'test_files/StyleLexers/styleLexNASTRAN/columns.bdf') $sample
$sourceHash = Hash-Bytes ([IO.File]::ReadAllBytes($sample))
$fontFamilies = @('Consolas')
$registeredFonts = [Collections.Generic.List[string]]::new()
try {
foreach ($fontFile in $FontFiles) {
    $path = (Resolve-Path $fontFile).Path
    if (-not [GuideProbe]::AddFontResourceEx($path,0,[IntPtr]::Zero)) { throw "Could not register test font: $path" }
    $registeredFonts.Add($path)
}
if ($registeredFonts.Count) { $fontFamilies += 'Sarasa Term SC' }
foreach ($fontFamily in $fontFamilies) {
foreach ($locale in @('en-US','zh-CN')) {
    $ini = Join-Path $testRoot ($fontFamily.Replace(' ','-') + '-' + $locale + '.ini')
    $colours = if ($locale -eq 'zh-CN') { 'fore:#D8D8D8; back:#202020' } else { 'fore:#202020; back:#FFFFFF' }
    @"
[Settings]
SaveSettings=1
MarkLongLines=1
LongLineMode=3
LongLinesLimit=111
MultiEdgeLines=40 111
WordWrap=0
[Settings2]
PreferredLanguageLocaleName=$locale
[Common Base]
Default Style=font:$fontFamily; size:11; $colours
[2nd Common Base]
2nd Default Style=font:Arial; size:11; $colours
"@ | Set-Content $ini -Encoding utf8
    $app = Start-Process $ExePath -ArgumentList "/n /f `"$ini`" `"$sample`"" -PassThru
    $main = [IntPtr]::Zero
    try {
        $main = Wait-For { $app.Refresh(); if ($app.HasExited) { throw "Editor exited: $($app.ExitCode)" }; $app.MainWindowHandle } 'editor window'
        $editor = Wait-For { [GuideProbe]::Editor($main) } 'Scintilla window'
        Wait-For { [GuideProbe]::Query($editor,4002) -eq 206 } 'NASTRAN recognition' | Out-Null
        if ([GuideProbe]::Font($app.Id,$editor) -ne $fontFamily) { throw "Test font was not applied: $fontFamily" }
        Assert-Guides '8 16 24 32 40 48 56 64 72 80'
        if ([GuideProbe]::MenuState($main,41067) -band 3) { throw 'NASTRAN preset is disabled.' }
        if (-not ([GuideProbe]::MenuState($main,41067) -band 8)) { throw 'Default preset is not checked.' }
        if ($locale -eq 'zh-CN' -and [GuideProbe]::MenuText($main,41067) -notmatch '字符字段') { throw 'Chinese language resources were not loaded.' }
        [GuideProbe]::Command($main,41068)
        Assert-Guides '8 24 40 56 72 80'
        [GuideProbe]::Command($main,41006)
        Assert-Guides '' 0
        [GuideProbe]::Command($main,41006)
        Assert-Guides '8 24 40 56 72 80'
        [GuideProbe]::Command($main,41066)
        Assert-Guides '' 0
        [GuideProbe]::Command($main,41068)
        Assert-Guides '8 24 40 56 72 80'
        # Changing renderers and reapplying the base scheme must retain the preset.
        foreach ($command in @(42000,42001)) {
            [GuideProbe]::Command($main,$command)
            Start-Sleep -Milliseconds 150
            Assert-Guides '8 24 40 56 72 80'
        }
        # Changing to a proportional font must suppress guides without losing
        # the selected preset; changing back must restore it in the same window.
        [GuideProbe]::Command($main,41002)
        Wait-For { [GuideProbe]::Font($app.Id,$editor) -eq 'Arial' } 'proportional font applied' | Out-Null
        Assert-Guides '' 0
        foreach ($command in @(41067,41068,41006)) {
            if (-not ([GuideProbe]::MenuState($main,$command) -band 3)) { throw "Proportional-font command $command is enabled." }
            [GuideProbe]::Command($main,$command)
        }
        Start-Sleep -Milliseconds 150
        Assert-Guides '' 0
        [GuideProbe]::Command($main,41002)
        Wait-For { [GuideProbe]::Font($app.Id,$editor) -eq $fontFamily } 'monospace font restored' | Out-Null
        Assert-Guides '8 24 40 56 72 80'
        foreach ($command in @(41018,41019)) {
            [GuideProbe]::Command($main,$command)
            Start-Sleep -Milliseconds 150
            Assert-Guides '8 24 40 56 72 80'
        }
        Select-Scheme 'ANSYS APDL' 61
        Assert-Guides '40 111'
        if (-not ([GuideProbe]::MenuState($main,41067) -band 3)) { throw 'NASTRAN preset is enabled outside NASTRAN.' }
        # Posted commands must obey the same restriction as disabled menu items.
        [GuideProbe]::Command($main,41067)
        Start-Sleep -Milliseconds 150
        Assert-Guides '40 111'
        Select-Scheme 'NASTRAN' 206
        Assert-Guides '8 24 40 56 72 80'
        if ((Hash-Bytes ([GuideProbe]::Text($app.Id,$editor))) -ne $sourceHash -or [GuideProbe]::Query($editor,2159) -ne 0) { throw 'Guide operations changed document content.' }
        [GuideProbe]::Command($main,42035)
        Wait-For { (Get-Content $ini -Raw) -match '(?m)^NastranColumnGuides=2\s*$' } 'saving the guide preference' | Out-Null
        $saved = Get-Content $ini -Raw
        if ($saved -notmatch '(?m)^MarkLongLines=(true|1)\s*$') { throw 'Ordinary marker visibility changed.' }
        foreach ($line in @('LongLinesLimit=111','MultiEdgeLines=40 111')) {
            if ($saved -notmatch ('(?m)^' + [Regex]::Escape($line) + '\s*$')) { throw "Ordinary marker preference changed: $line" }
        }
        Write-Host "PASS $fontFamily / ${locale}: presets, toggle, proportional/monospace fonts, zoom, renderer, scheme restoration, INI and document bytes"
    } finally {
        if (-not $app.HasExited) { if ($main -ne [IntPtr]::Zero) { [GuideProbe]::Command($main,40025) }; if (-not $app.WaitForExit(5000)) { $app.Kill() } }
    }
    # A fresh process must load the saved large-field preset.
    $app = Start-Process $ExePath -ArgumentList "/n /f `"$ini`" `"$sample`"" -PassThru
    $main = [IntPtr]::Zero
    try {
        $main = Wait-For { $app.Refresh(); if ($app.HasExited) { throw 'Editor exited on restart.' }; $app.MainWindowHandle } 'restarted editor'
        $editor = Wait-For { [GuideProbe]::Editor($main) } 'restarted Scintilla'
        Assert-Guides '8 24 40 56 72 80'
        Write-Host "PASS $fontFamily / ${locale}: saved preset restored after restart"
    } finally {
        if (-not $app.HasExited) { if ($main -ne [IntPtr]::Zero) { [GuideProbe]::Command($main,40025) }; if (-not $app.WaitForExit(5000)) { $app.Kill() } }
    }
}
}
$ini = Join-Path $testRoot 'mixed-fonts.ini'
@'
[Settings]
WordWrap=0
[Settings2]
PreferredLanguageLocaleName=en-US
[Common Base]
Default Style=font:Consolas; size:11
[NASTRAN]
Card=font:Arial
'@ | Set-Content $ini -Encoding utf8
$app = Start-Process $ExePath -ArgumentList "/n /f `"$ini`" `"$sample`"" -PassThru
$main = [IntPtr]::Zero
try {
    $main = Wait-For { $app.Refresh(); $app.MainWindowHandle } 'mixed-font editor'
    $editor = Wait-For { [GuideProbe]::Editor($main) } 'mixed-font Scintilla'
    Wait-For { [GuideProbe]::Query($editor,4002) -eq 206 } 'mixed-font NASTRAN recognition' | Out-Null
    Assert-Guides '' 0
    if (-not ([GuideProbe]::MenuState($main,41067) -band 3)) { throw 'Incompatible token font was accepted.' }
    Write-Host 'PASS incompatible per-token font is detected'
} finally {
    if (-not $app.HasExited) { if ($main -ne [IntPtr]::Zero) { [GuideProbe]::Command($main,40025) }; if (-not $app.WaitForExit(5000)) { $app.Kill() } }
}
if ((Hash-Bytes ([IO.File]::ReadAllBytes($sample))) -ne $sourceHash) { throw 'Source file changed on disk.' }
Write-Host 'NASTRAN column guide integration tests passed.'
} finally {
    foreach ($path in $registeredFonts) { [GuideProbe]::RemoveFontResourceEx($path,0,[IntPtr]::Zero) | Out-Null }
}
