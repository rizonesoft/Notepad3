; Inno Download Plugin
; (c)2013-2014 Mitrich Software
; http://mitrichsoftware.wordpress.com/
; https://code.google.com/p/inno-download-plugin/

#define IDPROOT ExtractFilePath(__PATHFILENAME__)

#ifdef UNICODE
    #pragma include __INCLUDE__ + ";" + IDPROOT + "\unicode"
#else
    #pragma include __INCLUDE__ + ";" + IDPROOT + "\ansi"
#endif

; If IDP_DEBUG is defined before including idp.iss, script will use debug version of idp.dll (not included, you need to build it yourself).
; Debug dll messages can be viewed with SysInternals DebugView (http://technet.microsoft.com/en-us/sysinternals/bb896647.aspx)
#ifdef IDP_DEBUG
    #define DBGSUFFIX " debug"
#else
    #define DBGSUFFIX
#endif

#ifdef UNICODE
    #define IDPDLLDIR IDPROOT + "\unicode" + DBGSUFFIX
#else
    #define IDPDLLDIR IDPROOT + "\ansi" + DBGSUFFIX
#endif

#define IDP_VER_MAJOR         
#define IDP_VER_MINOR
#define IDP_VER_REV
#define IDP_VER_BUILD

#expr GetVersionComponents(IDPDLLDIR + "\idp.dll", IDP_VER_MAJOR, IDP_VER_MINOR, IDP_VER_REV, IDP_VER_BUILD)
#define IDP_VER EncodeVer(IDP_VER_MAJOR, IDP_VER_MINOR, IDP_VER_REV, IDP_VER_BUILD)

#define IDP_VER_STR GetVersionNumbersString(IDPDLLDIR + "\idp.dll")

[Files]
Source: "{#IDPDLLDIR}\idp.dll"; Flags: dontcopy;

[Code]
procedure idpAddFile(url, filename: String);                     external 'idpAddFile@files:idp.dll cdecl';
procedure idpAddFileComp(url, filename, components: String);     external 'idpAddFileComp@files:idp.dll cdecl';
procedure idpAddMirror(url, mirror: String);                     external 'idpAddMirror@files:idp.dll cdecl';
procedure idpAddFtpDir(url, mask, destdir: String; recursive: Boolean); external 'idpAddFtpDir@files:idp.dll cdecl';
procedure idpAddFtpDirComp(url, mask, destdir: String; recursive: Boolean; components: String); external 'idpAddFtpDirComp@files:idp.dll cdecl';
procedure idpClearFiles;                                         external 'idpClearFiles@files:idp.dll cdecl';
function  idpFilesCount: Integer;                                external 'idpFilesCount@files:idp.dll cdecl';
function  idpFtpDirsCount: Integer;                              external 'idpFtpDirsCount@files:idp.dll cdecl';
function  idpFileDownloaded(url: String): Boolean;               external 'idpFileDownloaded@files:idp.dll cdecl';
function  idpFilesDownloaded: Boolean;                           external 'idpFilesDownloaded@files:idp.dll cdecl';
function  idpDownloadFile(url, filename: String): Boolean;       external 'idpDownloadFile@files:idp.dll cdecl';
function  idpDownloadFiles: Boolean;                             external 'idpDownloadFiles@files:idp.dll cdecl';
function  idpDownloadFilesComp: Boolean;                         external 'idpDownloadFilesComp@files:idp.dll cdecl';
function  idpDownloadFilesCompUi: Boolean;                       external 'idpDownloadFilesCompUi@files:idp.dll cdecl';
procedure idpStartDownload;                                      external 'idpStartDownload@files:idp.dll cdecl';
procedure idpStopDownload;                                       external 'idpStopDownload@files:idp.dll cdecl';
procedure idpSetLogin(login, password: String);                  external 'idpSetLogin@files:idp.dll cdecl';
procedure idpSetProxyMode(mode: String);                         external 'idpSetProxyMode@files:idp.dll cdecl';
procedure idpSetProxyName(name: String);                         external 'idpSetProxyName@files:idp.dll cdecl';
procedure idpSetProxyLogin(login, password: String);             external 'idpSetProxyLogin@files:idp.dll cdecl';
procedure idpConnectControl(name: String; Handle: HWND);         external 'idpConnectControl@files:idp.dll cdecl';
procedure idpAddMessage(name, message: String);                  external 'idpAddMessage@files:idp.dll cdecl';
procedure idpSetInternalOption(name, value: String);             external 'idpSetInternalOption@files:idp.dll cdecl';
procedure idpSetDetailedMode(mode: Boolean);                     external 'idpSetDetailedMode@files:idp.dll cdecl';
procedure idpSetComponents(components: String);                  external 'idpSetComponents@files:idp.dll cdecl';
procedure idpReportError;                                        external 'idpReportError@files:idp.dll cdecl';
procedure idpTrace(text: String);                                external 'idpTrace@files:idp.dll cdecl';

#if defined(UNICODE) && (Ver >= 0x05050300)
procedure idpAddFileSize(url, filename: String; size: Int64);    external 'idpAddFileSize@files:idp.dll cdecl';
procedure idpAddFileSizeComp(url, filename: String; size: Int64; components: String); external 'idpAddFileSize@files:idp.dll cdecl';
function  idpGetFileSize(url: String; var size: Int64): Boolean; external 'idpGetFileSize@files:idp.dll cdecl';
function  idpGetFilesSize(var size: Int64): Boolean;             external 'idpGetFilesSize@files:idp.dll cdecl';
#else
procedure idpAddFileSize(url, filename: String; size: Dword);    external 'idpAddFileSize32@files:idp.dll cdecl';
procedure idpAddFileSizeComp(url, filename: String; size: Dword; components: String); external 'idpAddFileSize32@files:idp.dll cdecl';
function  idpGetFileSize(url: String; var size: Dword): Boolean; external 'idpGetFileSize32@files:idp.dll cdecl';
function  idpGetFilesSize(var size: Dword): Boolean;             external 'idpGetFilesSize32@files:idp.dll cdecl';
#endif

type TIdpForm = record
        Page              : TWizardPage;
        TotalProgressBar  : TNewProgressBar;
        FileProgressBar   : TNewProgressBar;
        TotalProgressLabel: TNewStaticText;
        CurrentFileLabel  : TNewStaticText;
        TotalDownloaded   : TNewStaticText; 
        FileDownloaded    : TNewStaticText;
        FileNameLabel     : TNewStaticText;
        SpeedLabel        : TNewStaticText;
        StatusLabel       : TNewStaticText;
        ElapsedTimeLabel  : TNewStaticText;
        RemainingTimeLabel: TNewStaticText;
        FileName          : TNewStaticText;
        Speed             : TNewStaticText;
        Status            : TNewStaticText;
        ElapsedTime       : TNewStaticText;
        RemainingTime     : TNewStaticText;
        DetailsButton     : TNewButton;
        GIDetailsButton   : HWND; //Graphical Installer
        DetailsVisible    : Boolean;
        InvisibleButton   : TNewButton;
    end;

    TIdpOptions = record
        DetailedMode   : Boolean;
        NoDetailsButton: Boolean;
        NoRetryButton  : Boolean;
        NoSkinnedButton: Boolean; //Graphical Installer
    end;

var IDPForm   : TIdpForm;
    IDPOptions: TIdpOptions;

function StrToBool(value: String): Boolean;
var s: String;
begin
    s := LowerCase(value);

    if      s = 'true'  then result := true
    else if s = 't'     then result := true
    else if s = 'yes'   then result := true
    else if s = 'y'     then result := true
    else if s = 'false' then result := false
    else if s = 'f'     then result := false
    else if s = 'no'    then result := false
    else if s = 'n'     then result := false
    else                     result := StrToInt(value) > 0;
end;

function WizardVerySilent: Boolean;
var i: Integer;
begin
    for i := 1 to ParamCount do
    begin
        if UpperCase(ParamStr(i)) = '/VERYSILENT' then
        begin
            result := true;
            exit;
        end;
    end;
    
    result := false;
end;

function WizardSupressMsgBoxes: Boolean;
var i: Integer;
begin
    for i := 1 to ParamCount do
    begin
        if UpperCase(ParamStr(i)) = '/SUPPRESSMSGBOXES' then
        begin
            result := true;
            exit;
        end;
    end;
    
    result := false;
end;

procedure idpSetOption(name, value: String);
var key: String;
begin
    key := LowerCase(name);

    if      key = 'detailedmode'    then IDPOptions.DetailedMode    := StrToBool(value)
    else if key = 'detailsvisible'  then IDPOptions.DetailedMode    := StrToBool(value) //alias
    else if key = 'detailsbutton'   then IDPOptions.NoDetailsButton := not StrToBool(value)
    else if key = 'skinnedbutton'   then IDPOptions.NoSkinnedButton := not StrToBool(value)
    else if key = 'retrybutton'     then 
    begin
        IDPOptions.NoRetryButton := StrToInt(value) = 0;
        idpSetInternalOption('RetryButton', value);
    end
    else
        idpSetInternalOption(name, value);
end;

procedure idpShowDetails(show: Boolean);
begin
    IDPForm.FileProgressBar.Visible    := show; 
    IDPForm.CurrentFileLabel.Visible   := show;  
    IDPForm.FileDownloaded.Visible     := show;    
    IDPForm.FileNameLabel.Visible      := show;     
    IDPForm.SpeedLabel.Visible         := show;        
    IDPForm.StatusLabel.Visible        := show;       
    IDPForm.ElapsedTimeLabel.Visible   := show;  
    IDPForm.RemainingTimeLabel.Visible := show;
    IDPForm.FileName.Visible           := show;          
    IDPForm.Speed.Visible              := show;             
    IDPForm.Status.Visible             := show;            
    IDPForm.ElapsedTime.Visible        := show;       
    IDPForm.RemainingTime.Visible      := show;
    
    IDPForm.DetailsVisible := show;
    
    if IDPForm.DetailsVisible then
    begin
        IDPForm.DetailsButton.Caption := ExpandConstant('{cm:IDP_HideButton}');
        IDPForm.DetailsButton.Top := ScaleY(184);
    end
    else
    begin
        IDPForm.DetailsButton.Caption := ExpandConstant('{cm:IDP_DetailsButton}');
        IDPForm.DetailsButton.Top := ScaleY(44);
    end;

    idpSetDetailedMode(show);
end;

procedure idpDetailsButtonClick(Sender: TObject);
begin
    idpShowDetails(not IDPForm.DetailsVisible);
end;

#ifdef GRAPHICAL_INSTALLER_PROJECT
procedure idpGIDetailsButtonClick(hButton: HWND);
begin
    idpShowDetails(not IDPForm.DetailsVisible);
  
    if IDPForm.DetailsVisible then
    begin
        ButtonSetText(IDPForm.GIDetailsButton, PAnsiChar(ExpandConstant('{cm:IDP_HideButton}')));
        ButtonSetPosition(IDPForm.GIDetailsButton, IDPForm.DetailsButton.Left-ScaleX(5), ScaleY(184), ButtonWidth, ButtonHeight);
    end
    else
    begin
        ButtonSetText(IDPForm.GIDetailsButton, PAnsiChar(ExpandConstant('{cm:IDP_DetailsButton}')));
        ButtonSetPosition(IDPForm.GIDetailsButton, IDPForm.DetailsButton.Left-ScaleX(5), ScaleY(44), ButtonWidth, ButtonHeight);
    end;
     
    ButtonRefresh(hButton);
end;

procedure idpCreateGIDetailsButton;
var swButtonNormalColor  : TColor;
    swButtonFocusedColor : TColor;
    swButtonPressedColor : TColor;
    swButtonDisabledColor: TColor;
begin
    swButtonNormalColor   := SwitchColorFormat(ExpandConstant('{#ButtonNormalColor}'));
    swButtonFocusedColor  := SwitchColorFormat(ExpandConstant('{#ButtonFocusedColor}'));
    swButtonPressedColor  := SwitchColorFormat(ExpandConstant('{#ButtonPressedColor}'));
    swButtonDisabledColor := SwitchColorFormat(ExpandConstant('{#ButtonDisabledColor}'));

    with IDPForm.DetailsButton do 
    begin
        IDPForm.GIDetailsButton := ButtonCreate(IDPForm.Page.Surface.Handle, Left-ScaleX(5), Top, ButtonWidth, ButtonHeight, 
                                   ExpandConstant('{tmp}\{#ButtonPicture}'), coButtonShadow, False);

        ButtonSetEvent(IDPForm.GIDetailsButton, ButtonClickEventID, WrapButtonCallback(@idpGIDetailsButtonClick, 1));
        ButtonSetFont(IDPForm.GIDetailsButton, ButtonFont.Handle);
        ButtonSetFontColor(IDPForm.GIDetailsButton, swButtonNormalColor, swButtonFocusedColor, swButtonPressedColor, swButtonDisabledColor);
        ButtonSetText(IDPForm.GIDetailsButton, PAnsiChar(Caption));
        ButtonSetVisibility(IDPForm.GIDetailsButton, true);
        ButtonSetEnabled(IDPForm.GIDetailsButton, true);
    end;
end;
#endif

procedure idpFormActivate(Page: TWizardPage);
begin
    if WizardSilent then
        idpSetOption('RetryButton', '0');
        
    if WizardSupressMsgBoxes then
        idpSetInternalOption('ErrorDialog', 'none');

    if not IDPOptions.NoRetryButton then
        WizardForm.BackButton.Caption := ExpandConstant('{cm:IDP_RetryButton}');
         
    idpShowDetails(IDPOptions.DetailedMode);
    IDPForm.DetailsButton.Visible := not IDPOptions.NoDetailsButton;

#ifdef GRAPHICAL_INSTALLER_PROJECT
    idpSetInternalOption('RedrawBackground', '1');
    idpConnectControl('GIBackButton', hBackButton);
    idpConnectControl('GINextButton', hNextButton);

    if not IDPOptions.NoSkinnedButton then
    begin
        IDPForm.DetailsButton.Visible := false;
        if IDPForm.GIDetailsButton = 0 then
            idpCreateGIDetailsButton;
    end;

    if IDPOptions.NoRetryButton then
        WizardForm.BackButton.Enabled := false
    else
        WizardForm.BackButton.Visible := false;

    WizardForm.NextButton.Enabled := false;
#endif
    idpSetComponents(WizardSelectedComponents(false));
    
    if WizardVerySilent then
        idpDownloadFilesComp
    else if WizardSilent then
    begin
        WizardForm.Show;
        WizardForm.Repaint;
        idpDownloadFilesCompUi;
        WizardForm.Hide;
    end
    else
        idpStartDownload;
end;

function idpShouldSkipPage(Page: TWizardPage): Boolean;
begin
    idpSetComponents(WizardSelectedComponents(false));
    Result := ((idpFilesCount = 0) and (idpFtpDirsCount = 0)) or idpFilesDownloaded;
end;

function idpBackButtonClick(Page: TWizardPage): Boolean;
begin
    if not IDPOptions.NoRetryButton then // Retry button clicked
    begin
        idpStartDownload; 
        Result := False;
    end
    else
        Result := true;
end;

function idpNextButtonClick(Page: TWizardPage): Boolean;
begin
    Result := True;
end;

procedure idpCancelButtonClick(Page: TWizardPage; var Cancel, Confirm: Boolean);
begin
    if ExitSetupMsgBox then
    begin
        IDPForm.Status.Caption := ExpandConstant('{cm:IDP_CancellingDownload}');
        WizardForm.Repaint;
        idpStopDownload;
        Cancel  := true;
        Confirm := false;
    end
    else
        Cancel := false;
end;

procedure idpReportErrorHelper(Sender: TObject);
begin
    idpReportError; //calling idpReportError in main thread for compatibility with VCL Styles for IS
end;

function idpCreateDownloadForm(PreviousPageId: Integer): Integer;
begin
    IDPForm.Page := CreateCustomPage(PreviousPageId, ExpandConstant('{cm:IDP_FormCaption}'), ExpandConstant('{cm:IDP_FormDescription}'));

    IDPForm.TotalProgressBar := TNewProgressBar.Create(IDPForm.Page);
    with IDPForm.TotalProgressBar do
    begin
        Parent := IDPForm.Page.Surface;
        Left := ScaleX(0);
        Top := ScaleY(16);
        Width := ScaleX(410);
        Height := ScaleY(20);
        Min := 0;
        Max := 100;
    end;

    IDPForm.TotalProgressLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.TotalProgressLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_TotalProgress}');
        Left := ScaleX(0);
        Top := ScaleY(0);
        Width := ScaleX(200);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 1;
    end;

    IDPForm.CurrentFileLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.CurrentFileLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_CurrentFile}');
        Left := ScaleX(0);
        Top := ScaleY(48);
        Width := ScaleX(200);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 2;
    end;

    IDPForm.FileProgressBar := TNewProgressBar.Create(IDPForm.Page);
    with IDPForm.FileProgressBar do
    begin
        Parent := IDPForm.Page.Surface;
        Left := ScaleX(0);
        Top := ScaleY(64);
        Width := ScaleX(410);
        Height := ScaleY(20);
        Min := 0;
        Max := 100;
    end;

    IDPForm.TotalDownloaded := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.TotalDownloaded do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(290);
        Top := ScaleY(0);
        Width := ScaleX(120);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 4;
    end;

    IDPForm.FileDownloaded := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.FileDownloaded do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(290);
        Top := ScaleY(48);
        Width := ScaleX(120);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 5;
    end;

    IDPForm.FileNameLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.FileNameLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_File}');
        Left := ScaleX(0);
        Top := ScaleY(100);
        Width := ScaleX(116);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 6;
    end;

    IDPForm.SpeedLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.SpeedLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_Speed}');
        Left := ScaleX(0);
        Top := ScaleY(116);
        Width := ScaleX(116);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 7;
    end;

    IDPForm.StatusLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.StatusLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_Status}');
        Left := ScaleX(0);
        Top := ScaleY(132);
        Width := ScaleX(116);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 8;
    end;

    IDPForm.ElapsedTimeLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.ElapsedTimeLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_ElapsedTime}');
        Left := ScaleX(0);
        Top := ScaleY(148);
        Width := ScaleX(116);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 9;
    end;

    IDPForm.RemainingTimeLabel := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.RemainingTimeLabel do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_RemainingTime}');
        Left := ScaleX(0);
        Top := ScaleY(164);
        Width := ScaleX(116);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 10;
    end;

    IDPForm.FileName := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.FileName do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(120);
        Top := ScaleY(100);
        Width := ScaleX(280);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 11;
    end;

    IDPForm.Speed := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.Speed do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(120);
        Top := ScaleY(116);
        Width := ScaleX(280);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 12;
    end;

    IDPForm.Status := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.Status do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(120);
        Top := ScaleY(132);
        Width := ScaleX(280);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 13;
    end;

    IDPForm.ElapsedTime := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.ElapsedTime do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(120);
        Top := ScaleY(148);
        Width := ScaleX(280);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 14;
    end;

    IDPForm.RemainingTime := TNewStaticText.Create(IDPForm.Page);
    with IDPForm.RemainingTime do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := '';
        Left := ScaleX(120);
        Top := ScaleY(164);
        Width := ScaleX(280);
        Height := ScaleY(14);
        AutoSize := False;
        TabOrder := 15;
    end;

    IDPForm.DetailsButton := TNewButton.Create(IDPForm.Page);
    with IDPForm.DetailsButton do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('{cm:IDP_DetailsButton}');
        Left := ScaleX(336);
        Top := ScaleY(184);
        Width := ScaleX(75);
        Height := ScaleY(23);
        TabOrder := 16;
        OnClick := @idpDetailsButtonClick;
    end;
    
    IDPForm.InvisibleButton := TNewButton.Create(IDPForm.Page);
    with IDPForm.InvisibleButton do
    begin
        Parent := IDPForm.Page.Surface;
        Caption := ExpandConstant('You must not see this button');
        Left := ScaleX(0);
        Top := ScaleY(0);
        Width := ScaleX(10);
        Height := ScaleY(10);
        TabOrder := 17;
        Visible := False;
        OnClick := @idpReportErrorHelper;
    end;
  
    with IDPForm.Page do
    begin
        OnActivate          := @idpFormActivate;
        OnShouldSkipPage    := @idpShouldSkipPage;
        OnBackButtonClick   := @idpBackButtonClick;
        OnNextButtonClick   := @idpNextButtonClick;
        OnCancelButtonClick := @idpCancelButtonClick;
    end;
  
    Result := IDPForm.Page.ID;
end;

procedure idpConnectControls;
begin
    idpConnectControl('TotalProgressLabel', IDPForm.TotalProgressLabel.Handle);
    idpConnectControl('TotalProgressBar',   IDPForm.TotalProgressBar.Handle);
    idpConnectControl('FileProgressBar',    IDPForm.FileProgressBar.Handle);
    idpConnectControl('TotalDownloaded',    IDPForm.TotalDownloaded.Handle);
    idpConnectControl('FileDownloaded',     IDPForm.FileDownloaded.Handle);
    idpConnectControl('FileName',           IDPForm.FileName.Handle);
    idpConnectControl('Speed',              IDPForm.Speed.Handle);
    idpConnectControl('Status',             IDPForm.Status.Handle);
    idpConnectControl('ElapsedTime',        IDPForm.ElapsedTime.Handle);
    idpConnectControl('RemainingTime',      IDPForm.RemainingTime.Handle);
    idpConnectControl('InvisibleButton',    IDPForm.InvisibleButton.Handle);
    idpConnectControl('WizardPage',         IDPForm.Page.Surface.Handle);
    idpConnectControl('WizardForm',         WizardForm.Handle);
    idpConnectControl('BackButton',         WizardForm.BackButton.Handle);
    idpConnectControl('NextButton',         WizardForm.NextButton.Handle);
    idpConnectControl('LabelFont',          IDPForm.TotalDownloaded.Font.Handle);
end;

procedure idpInitMessages;
begin
    idpAddMessage('Total progress',              ExpandConstant('{cm:IDP_TotalProgress}'));
    idpAddMessage('KB/s',                        ExpandConstant('{cm:IDP_KBs}'));
    idpAddMessage('MB/s',                        ExpandConstant('{cm:IDP_MBs}'));
    idpAddMessage('%.2f of %.2f',                ExpandConstant('{cm:IDP_X_of_X}'));
    idpAddMessage('KB',                          ExpandConstant('{cm:IDP_KB}'));
    idpAddMessage('MB',                          ExpandConstant('{cm:IDP_MB}'));
    idpAddMessage('GB',                          ExpandConstant('{cm:IDP_GB}'));
    idpAddMessage('Initializing...',             ExpandConstant('{cm:IDP_Initializing}'));
    idpAddMessage('Getting file information...', ExpandConstant('{cm:IDP_GettingFileInformation}'));
    idpAddMessage('Starting download...',        ExpandConstant('{cm:IDP_StartingDownload}'));
    idpAddMessage('Connecting...',               ExpandConstant('{cm:IDP_Connecting}'));
    idpAddMessage('Downloading...',              ExpandConstant('{cm:IDP_Downloading}'));
    idpAddMessage('Download complete',           ExpandConstant('{cm:IDP_DownloadComplete}'));
    idpAddMessage('Download failed',             ExpandConstant('{cm:IDP_DownloadFailed}'));
    idpAddMessage('Cannot connect',              ExpandConstant('{cm:IDP_CannotConnect}'));
    idpAddMessage('Unknown',                     ExpandConstant('{cm:IDP_Unknown}'));
    idpAddMessage('Download cancelled',          ExpandConstant('{cm:IDP_DownloadCancelled}'));
    idpAddMessage('HTTP error %d',               ExpandConstant('{cm:IDP_HTTPError_X}'));
    idpAddMessage('400',                         ExpandConstant('{cm:IDP_400}'));
    idpAddMessage('401',                         ExpandConstant('{cm:IDP_401}'));
    idpAddMessage('404',                         ExpandConstant('{cm:IDP_404}'));
    idpAddMessage('407',                         ExpandConstant('{cm:IDP_407}'));
    idpAddMessage('500',                         ExpandConstant('{cm:IDP_500}'));
    idpAddMessage('502',                         ExpandConstant('{cm:IDP_502}'));
    idpAddMessage('503',                         ExpandConstant('{cm:IDP_503}'));
    idpAddMessage('Retry',                       ExpandConstant('{cm:IDP_RetryButton}'));
    idpAddMessage('Ignore',                      ExpandConstant('{cm:IDP_IgnoreButton}'));
    idpAddMessage('Cancel',                      SetupMessage(msgButtonCancel));
    idpAddMessage('The following files were not downloaded:', ExpandConstant('{cm:IDP_FilesNotDownloaded}'));
    idpAddMessage('Check your connection and click ''Retry'' to try downloading the files again, or click ''Next'' to continue installing anyway.', ExpandConstant('{cm:IDP_RetryNext}'));
    idpAddMessage('Check your connection and click ''Retry'' to try downloading the files again, or click ''Cancel'' to terminate setup.', ExpandConstant('{cm:IDP_RetryCancel}'));
end;

procedure idpDownloadAfter(PageAfterId: Integer);
begin
    idpCreateDownloadForm(PageAfterId);
    idpConnectControls;
    idpInitMessages;
end;

#include <idplang\default.iss>
