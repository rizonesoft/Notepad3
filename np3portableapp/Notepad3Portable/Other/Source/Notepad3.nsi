;Copyright 2004-2016 John T. Haller of PortableApps.com
;encoding: UTF-8

;Website: https://PortableApps.com/Notepad3Portable

;This software is OSI Certified Open Source Software.
;OSI Certified is a certification mark of the Open Source Initiative.

;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either version 2
;of the License, or (at your option) any later version.

;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.

;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

!define PORTABLEAPPNAME "Notepad3, Portable Edition"
!define NamePortable "Notepad3, Portable Edition"
!define APPNAME "Notepad3"
!define NAME "Notepad3Portable"
!define AppID "Notepad3Portable"
!define VER "5.18.1108.1350"
!define WEBSITE "PortableApps.com/Notepad3Portable"
!define DEFAULTEXE "Notepad3.exe"
!define DEFAULTAPPDIR "Notepad3"
!define LAUNCHERLANGUAGE "English"

;=== Program Details
Name "${PORTABLEAPPNAME}"
OutFile "${DEFAULTEXE}"
Caption "${PORTABLEAPPNAME} | PortableApps.com"
VIProductVersion "${VER}"
VIAddVersionKey ProductName "${PORTABLEAPPNAME}"
VIAddVersionKey Comments "A notepad replacement"
VIAddVersionKey CompanyName "Rizonesoft"
VIAddVersionKey LegalCopyright "Derick Payne"
VIAddVersionKey FileDescription "Based on code from Notepad2, Florian Balmer© 1996-2011"
VIAddVersionKey FileVersion "${VER}"
VIAddVersionKey ProductVersion "${VER}"
VIAddVersionKey InternalName "${PORTABLEAPPNAME}"
VIAddVersionKey LegalTrademarks "Rizonesoft © 2008-2026"
VIAddVersionKey OriginalFilename "${DEFAULTEXE}"
;VIAddVersionKey PrivateBuild ""
;VIAddVersionKey SpecialBuild ""

;=== Runtime Switches
CRCCheck On
WindowIcon Off
SilentInstall Silent
AutoCloseWindow True
RequestExecutionLevel user
XPStyle on
Unicode true

; Best Compression
SetCompress Auto
SetCompressor /SOLID lzma
SetCompressorDictSize 32
SetDatablockOptimize On

;=== Include
;(Standard NSIS)

;=== Program Icon
Icon "..\..\App\AppInfo\appicon.ico"

;=== Icon & Stye ===
BrandingText "Rizonesoft©"

;=== Languages
LoadLanguageFile "${NSISDIR}\Contrib\Language files\${LAUNCHERLANGUAGE}.nlf"
!include PortableApps.comLauncherLANG_${LAUNCHERLANGUAGE}.nsh

Section "Main"
	MessageBox MB_OK|MB_ICONINFORMATION "Notepad3.exe is running and will close when you click OK."
SectionEnd
