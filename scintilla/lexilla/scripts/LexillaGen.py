#!/usr/bin/env python3
# LexillaGen.py - implemented 2019 by Neil Hodgson neilh@scintilla.org
# Released to the public domain.

# Regenerate the Lexilla source files that list all the lexers.
# Should be run whenever a new lexer is added or removed.
# Requires Python 3.6 or later
# Files are regenerated in place with templates stored in comments.
# The format of generation comments is documented in FileGenerator.py.

import os, sys

sys.path.append(os.path.join("..", "..", "scripts"))

from FileGenerator import Regenerate, UpdateLineInFile, \
    ReplaceREInFile, UpdateLineInPlistFile, ReadFileAsList, UpdateFileFromLines, \
    FindSectionInList
import ScintillaData

def RegenerateAll(root):

    scintillaBase = os.path.abspath(root)

    sci = ScintillaData.ScintillaData(root + os.sep)
    
    src = os.path.join(root, "lexilla", "src")

    Regenerate(os.path.join(src, "Lexilla.cxx"), "//", sci.lexerModules)
    Regenerate(os.path.join(src, "lexilla.mak"), "#", sci.lexFiles)

    #~ startDir = os.getcwd()
    #~ os.chdir(os.path.join(scintillaBase, "win32"))
    #~ win32.DepGen.Generate()
    #~ os.chdir(os.path.join(scintillaBase, "gtk"))
    #~ gtk.DepGen.Generate()
    #~ os.chdir(startDir)

if __name__=="__main__":
    RegenerateAll(os.path.join("..", ".."))
