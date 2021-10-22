================================================================================
=                                                                              =
=            Notepad3 - light-weight Scintilla-based text editor for Windows   =
=                                                                              =
=                                                   (c) Rizonesoft 2008-2021   =
=                                                 https://www.rizonesoft.com   =
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
- State of the art Regular Expression search engine (Onigmu)
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
Windows 7, 8, 8.1 and 10 both 32-bit and 64-bit

--------------------------------------------------------------------------------
Development:
------------
- Florian 'Flo' Balmer (Notepad2)       https://www.flos-freeware.ch
- RaiKoHoff                             https://github.com/RaiKoHoff

--------------------------------------------------------------------------------
Contributors:
-------------
Derick Payne (© Rizonesoft)             https://www.rizonesoft.com
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
N.Hodgson (Scintilla)           https://www.scintilla.org
K.Kosako (Oniguruma Regex)      https://github.com/kkos/oniguruma
Stefan Küng (grepWin)           https://github.com/stefankueng/grepWin
D.Dyer (NotepadCrypt)           https://www.real-me.net/ddyer/notepad/NotepadCrypt.html
T.D.Hanson (uthash)             https://troydhanson.github.io/uthash
Carbo Kuo (Mozilla's uchardet)  https://www.freedesktop.org/wiki/Software/uchardet
Lewis Van Winkle (TinyExpr)     https://github.com/codeplea/tinyexpr

--------------------------------------------------------------------------------
Acknowledgments:
----------------
alex-ilin               https://github.com/alex-ilin
alexantr                https://github.com/alexantr
ashish12phnx            https://github.com/ashish12phnx
bravo-hero              https://github.com/bravo-hero
craigo-                 https://github.com/craigo-/
Crane70                 https://github.com/Crane70
danfong                 https://github.com/danfong
engelhro                https://github.com/engelhro
geogeo-gr               https://github.com/geogeo-gr
Hexaae                  https://github.com/Hexaae
hpwamr                  https://github.com/hpwamr
igorruckert             https://github.com/igorruckert
jupester                https://github.com/jupester
kofifus                 https://github.com/kofifus
Lacn0755                https://github.com/Lacn0755
leeoniya                https://github.com/leeoniya
lhmouse                 https://github.com/lhmouse
ltGuillaume             https://github.com/ltGuillaume
maboroshin              https://github.com/maboroshin
MadDogVachon            https://github.com/MadDogVachon
MelchiorGaspar          https://github.com/MelchiorGaspar
Mitezuss                https://github.com/Mitezuss
RaffaeleBianc0          https://github.com/RaffaeleBianc0
RaiKoHoff               https://github.com/RaiKoHoff
rizonesoft              https://github.com/rizonesoft
Rudolfin                https://github.com/Rudolfin
Stephan-P               https://github.com/Stephan-P
ThreeLightsBeyond       https://github.com/ThreeLightsBeyond
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
Oniguruma Regex                 https://github.com/kkos/oniguruma/blob/master/COPYING
grepWin                         https://github.com/stefankueng/grepWin/blob/master/LICENSE
uthash                          https://troydhanson.github.io/uthash/license.html
uchardet                        https://www.freedesktop.org/wiki/Software/uchardet/#license
TinyExpr                        https://github.com/codeplea/tinyexpr/blob/master/LICENSE
Notepad3 icon by Vexels.com     https://www.vexels.com
  -  designed by                mailto:smanashova@gmail.com

--------------------------------------------------------------------------------
Notepad3 Licence:
-----------------
Notepad3 and MiniPath Copyright © 2008-2021 Rizonesoft, All rights reserved.
https://www.rizonesoft.com

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
