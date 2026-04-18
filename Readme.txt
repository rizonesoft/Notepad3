================================================================================
=                                                                              =
=            Notepad3 - light-weight Scintilla-based text editor for Windows   =
=                                                                              =
=                                                   (c) Rizonesoft 2008-2026   =
=                                                     https://rizonesoft.com   =
=                                                                              =
================================================================================
Rizonesoft Notepad3 --- README ---
================================================================================

--------------------------------------------------------------------------------
Description:
------------
Notepad like text editor is based on the Scintilla source code. 
Notepad3 is based on code from Notepad2 and MiniPath on code from metapath.

--------------------------------------------------------------------------------
Changes compared to Flo's official Notepad2 (made in Notepad2-mod):
-------------------------------------------------------------------
- Code folding
- Support for bookmarks
- Option to mark all occurrences of a word
- Updated Scintilla component
- Word auto-completion
- Syntax highlighting support for AutoHotkey (AHK), AutoIt3, AviSynth, Bash, 
  CMake, CoffeeScript, Inno Setup, LaTeX, Lua, Markdown, NSIS, Ruby, Tcl, 
  YAML and VHDL scripts.
- Improved support for NFO ANSI art
- Other various minor changes and tweaks

--------------------------------------------------------------------------------
Changes compared to the Notepad2-mod fork:
------------------------------------------
- Additional syntax highlighting support for Awk, D, golang, MATLAB
- State of the art Regular Expression search engine (Oniguruma)
- New toolbar icons based on Yusuke Kamiyaman's Fugue Icons 
  (Purchased by Rizonesoft)
- Hyperlink Hotspot highlighting 
  (single click Open in Browser (Ctrl) / Load in Editor (Alt))
- New program icon and other small cosmetic changes
- In-App support for AES-256 Rijndael encryption/decryption of files. 
  (incl. external commandline tool for batch processing)
- Virtual Space rectangular selection box (Alt-Key down)
- High-DPI awareness, including high definition toolbar icons
- Undo/Redo preserves selection
- File History preserves Caret position (optional) 
  and remembers encoding of file
- Accelerated word navigation
- Preserve caret position of items in file history
- Count occurrences of a marked selection or word
- Count and Mark occurrences of matching search/find expression
- Visual Studio style copy/paste current line (no selection)
- Insert GUIDs
- Dropped support for Windows XP version
- Other various minor changes, tweaks and bugfixes

--------------------------------------------------------------------------------
Supported Operating Systems:
----------------------------
Windows 10 and 11 (32-bit, 64-bit and ARM64)

--------------------------------------------------------------------------------
Development:
------------
- Florian 'Flo' Balmer (Notepad2)       https://www.flos-freeware.ch
- RaiKoHoff                             https://github.com/RaiKoHoff

--------------------------------------------------------------------------------
Contributors:
-------------
Derick Payne (© Rizonesoft)             https://rizonesoft.com
Pairi Daiza (MUI language support)      https://github.com/hpwamr
XhmikosR (Notepad2-mod)                 https://xhmikosr.github.io/notepad2-mod
Kai Liu (CodeFolding)                   https://code.kliu.org/misc/notepad2
RL Vision (Bookmarks)                   https://www.rlvision.com/notepad2/about.php
Aleksandar Lekov (MarkOcc./AutoCompl.)
Bruno Barbieri
Matthew Ellis (MinimizeToTray)
Igal Tabachnik (RelaunchElevated)       https://github.com/hmemcpy
Et alii @ GitHub                        https://github.com/XhmikosR/notepad2-mod/graphs/contributors
Some Icons by Pixel perfect (Flaticon)  https://www.flaticon.com

--------------------------------------------------------------------------------
Open Source / Libraries:
------------------------
Neil Hodgson (Scintilla)            https://www.scintilla.org
Neil Hodgson (Lexilla)              https://www.scintilla.org/Lexilla.html
Philip Hazel et al. (PCRE2 Regex)   https://pcre2project.github.io/pcre2
Stefan Küng (grepWin)               https://github.com/stefankueng/grepWin
D.Dyer (NotepadCrypt)               https://www.real-me.net/ddyer/notepad/NotepadCrypt.html
Brodie Thiesfield (SimpleIni)       https://github.com/brofield/simpleini
T.D.Hanson (uthash)                 https://troydhanson.github.io/uthash
Carbo Kuo (Mozilla's uchardet)      https://www.freedesktop.org/wiki/Software/uchardet
Blake Madden (TinyExpr++)           https://github.com/Blake-Madden/tinyexpr-plusplus
Carlo Pallini (Resample BMP)        https://www.codeproject.com/Articles/22271/Plain-C-Resampling-DLL

--------------------------------------------------------------------------------
Acknowledgments:
----------------
alex-ilin               https://github.com/alex-ilin
alexantr                https://github.com/alexantr
ashish12phnx            https://github.com/ashish12phnx
bovirus                 https://github.com/bovirus
bravo-hero              https://github.com/bravo-hero
chuanliwen              https://github.com/chuanliwen
craigo-                 https://github.com/craigo-
Crane70                 https://github.com/Crane70
en2sv                   https://github.com/en2sv
engelhro                https://github.com/engelhro
Hexaae                  https://github.com/Hexaae
hpwamr                  https://github.com/hpwamr
igorruckert             https://github.com/igorruckert
jupester                https://github.com/jupester
kayazeren               https://github.com/kayazeren
kidzgy                  https://github.com/kidzgy
kofifus                 https://github.com/kofifus
kun7752                 https://github.com/kun7752
Lacn0755                https://github.com/Lacn0755
leeoniya                https://github.com/leeoniya
lhmouse                 https://github.com/lhmouse
Lidgeu                  https://github.com/Lidgeu
ltGuillaume             https://github.com/ltGuillaume
maboroshin              https://github.com/maboroshin
MadDogVachon            https://github.com/MadDogVachon
Matteo-Nigro            https://github.com/Matteo-Nigro
MelchiorGaspar          https://github.com/MelchiorGaspar
Mitezuss                https://github.com/Mitezuss
nickreserved            https://github.com/nickreserved
np3fan                  https://github.com/np3fan
p0k33m0n                https://github.com/p0k33m0n
quadratz                https://github.com/quadratz
RaiKoHoff               https://github.com/RaiKoHoff
rizonesoft              https://github.com/rizonesoft
Rudolfin                https://github.com/Rudolfin
Stephan-P               https://github.com/Stephan-P
ThreeLightsBeyond       https://github.com/ThreeLightsBeyond
tormento                https://github.com/tormento
Veikko-M                https://github.com/Veikko-M
VenusGirl               https://github.com/VenusGirl
xsak                    https://github.com/xsak
zufuliu                 https://github.com/zufuliu

--------------------------------------------------------------------------------
Resources:
----------
NotepadCrypt (Version Java)     https://www.nayuki.io/page/notepadcrypt-format-decryptor-java
Data Sharing Service            https://workupload.com

--------------------------------------------------------------------------------
Licences:
---------
Scintilla                       https://www.scintilla.org/License.txt
PCRE2 Regex                     https://github.com/PCRE2Project/pcre2/blob/main/LICENCE.md
grepWin                         https://github.com/stefankueng/grepWin/blob/master/LICENSE
uthash                          https://troydhanson.github.io/uthash/license.html
uchardet                        https://www.freedesktop.org/wiki/Software/uchardet/#license
TinyExpr++                      https://github.com/Blake-Madden/tinyexpr-plusplus/blob/master/LICENSE
Notepad3 icon by Vexels.com     https://www.vexels.com
  -  designed by                smanashova@gmail.com

--------------------------------------------------------------------------------
Notepad3 Licence:
-----------------
Notepad3 and MiniPath Copyright © 2008-2026 Rizonesoft, All rights reserved.
https://rizonesoft.com

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of Florian Balmer nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
