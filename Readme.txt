===============================================================================
=                                                                              =
=            Notepad3 - Windows 下基于 Scintilla 的轻量级文本编辑器   =
=                                                                              =
=                                                   (c) Rizonesoft 2008-2026   =
=                                                     https://rizonesoft.com   =
=                                                                              =
===============================================================================
Rizonesoft 记事本3 --- 说明文件 ---
===============================================================================

输入：--------------------------------------------------------------------------------
描述:
输入： ------------
记事本类似的文本编辑器基于Scintilla源代码。
Notepad3基于Notepad2的代码，并且MiniPath基于metapath的代码。

输入：--------------------------------------------------------------------------------
与Flo的官方Notepad2（由Notepad2-mod制作）相比的更改：
-------------------------------------------------------------------
- 代码折叠
- 支持书签
- 选项标记一个词的所有出现位置
- 更新了 Scintilla 组件
- Word自动完成
- 对 AutoHotkey (AHK)、AutoIt3、AviSynth、Bash 的语法高亮支持
  CMake, CoffeeScript, Inno Setup, LaTeX, Lua, Markdown, NSIS, Ruby, Tcl,
  YAML 和 VHDL 脚本。
- 改进了对NFO ANSI艺术的支持
- 其他各种小的更改和调整

输入：--------------------------------------------------------------------------------
与Notepad2-mod分支相比的更改：
------------------------------------------
- 为 Awk、D、golang、MATLAB 提供额外的语法高亮支持
- 最先进的正则表达式搜索引擎（Oniguruma）
- 新的工具栏图标基于Yusuke Kamiyama的Fugue Icons
  （由Rizonesoft购买）
- 超链接热点突出显示
  （单击在浏览器中打开 (Ctrl) / 在编辑器中加载 (Alt)）
- 新的程序图标和其他小的外观更改
- 应用内支持使用AES-256 Rijndael加密/解密文件。
  （包括用于批量处理的外部命令行工具）
- 虚拟空间矩形选择框（按住Alt键）
- 高DPI感知，包括高定义工具栏图标
- 撤销/重做保留选择
- 文件历史记录保存插入点位置（可选）
  并记住文件的编码
- 加速词导航
- 保持文件历史记录中项目的光标位置
- 统计选定的文本或单词出现的次数
- 统计并标记匹配的搜索/查找表达式出现的次数
- Visual Studio 风格的复制/粘贴当前行（不选择任何内容）
- 插入GUIDs
- 停止支持 Windows XP 版本
- 其他各种小的更改、调整和错误修复

输入：--------------------------------------------------------------------------------
支持的操作系统：
输入： ----------------------------
Windows 10 和 11（32位，64位和ARM64）

输入：--------------------------------------------------------------------------------
开发：
输入： ------------
- Florian 'Flo' Balmer (Notepad2)       https://www.flos-freeware.ch
- RaiKoHoff                             https://github.com/RaiKoHoff

输入：--------------------------------------------------------------------------------
贡献者：
输入：-------------
德里克·佩恩 (© Rizonesoft)             https://rizonesoft.com
XhmikosR (Notepad2 修改版)                 https://xhmikosr.github.io/notepad2-mod
刘凯 (CodeFolding)                   https://code.kliu.org/misc/notepad2
RL Vision (书签)                   https://www.rlvision.com/notepad2/about.php
亚历山大·莱科夫 (MarkOcc./AutoCompl.)
布鲁诺·巴贝里
马修·埃利斯（最小化到托盘）
伊格尔·塔巴尼科 (RelaunchElevated)       https://github.com/hmemcpy
等等 @ GitHub                        https://github.com/XhmikosR/notepad2-mod/graphs/contributors
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
chuanliwen              https://github.com/chuanliwen
craigo-                 https://github.com/craigo-
Crane70                 https://github.com/Crane70
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
Oniguruma Regex                 https://github.com/kkos/oniguruma/blob/master/COPYING
grepWin                         https://github.com/stefankueng/grepWin/blob/master/LICENSE
uthash                          https://troydhanson.github.io/uthash/license.html
uchardet                        https://www.freedesktop.org/wiki/Software/uchardet/#license
TinyExpr                        https://github.com/codeplea/tinyexpr/blob/master/LICENSE
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
