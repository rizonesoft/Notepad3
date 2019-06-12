// Scintilla source code edit control
/** @file LexTOML.cxx
** Lexer for TOML (Tom's Obvious Markup Language -> Tom's Obvious, Minimal Language
** Written by RaiKoHoff
** TOML Spec: https://github.com/toml-lang/toml
**/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>
#include <map>
#include <algorithm>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciXLexer.h"

#include "StringCopy.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharSetX.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "DefaultLexer.h"


using namespace Scintilla;

namespace {
  // Use an unnamed namespace to protect the functions and classes from name conflicts

  enum DataType {
    String,
    Integer,
    Float,
    Boolean,
    Datetime,
    Array,
    Table,
    Unknown
  };

  int GetDataTypeStyle(const int numType) {
    if (numType == DataType::Unknown) {
      return SCE_TOML_PARSINGERROR;
    }
    return SCE_TOML_VALUE;
  }

  inline bool IsFuncName(const char* str) {
    const char* identifiers[] = {
        "proc",
        "func",
        "macro",
        "method",
        "template",
        "iterator",
        "converter"
    };
    for (const char* id : identifiers) {
      if (strcmp(str, id) == 0) {
        return true;
      }
    }
    return false;
  }

  struct OptionsTOML {
    bool fold;
    bool foldCompact;

    OptionsTOML() {
      fold = true;
      foldCompact = true;
    }
  };

  static const char* const tomlWordListDesc[] = {
      "TOML",
      nullptr
  };

  struct OptionSetTOML : public OptionSet<OptionsTOML> {
    OptionSetTOML() {

      DefineProperty("fold", &OptionsTOML::fold, "FOLD COMMENT");
      DefineProperty("fold.compact", &OptionsTOML::foldCompact, "FOLDCOMPACT COMMENT");

      DefineWordListSets(tomlWordListDesc);
    }
  };

  LexicalClass lexicalClasses[] = {
    // Lexer TOML SCLEX_TOML SCE_TOML_:
    0,  "SCE_TOML_DEFAULT",        "default",              "Default",
    1,  "SCE_TOML_COMMENT",        "comment",              "Comment",
    2,  "SCE_TOML_KEY",            "key",                  "Key",
    3,  "SCE_TOML_SECTION",        "section",              "Section",
    4,  "SCE_TOML_ASSIGNMENT",     "assignment",           "Assignment",
    5,  "SCE_TOML_DEFVAL",         "default value",        "Default Value",
    6,  "SCE_TOML_VALUETYPE",      "value type",           "Value Type",
    7,  "SCE_TOML_PARSINGERROR",   "type error",           "Type Error",
  };

  } // end of namespace

class LexerTOML : public DefaultLexer {
  CharacterSet setWord;
  WordList keywords;
  OptionsTOML options;
  OptionSetTOML osTOML;

public:
  LexerTOML() 
    : DefaultLexer(lexicalClasses, ELEMENTS(lexicalClasses))
    , setWord(CharacterSet::setAlphaNum, "_", 0x80, true) 
  { }

  virtual ~LexerTOML() { }

  void SCI_METHOD Release() override {
    delete this;
  }

  int SCI_METHOD Version() const override {
    return lvRelease4;
  }

  const char* SCI_METHOD PropertyNames() override {
    return osTOML.PropertyNames();
  }

  int SCI_METHOD PropertyType(const char* name) override {
    return osTOML.PropertyType(name);
  }

  const char* SCI_METHOD DescribeProperty(const char* name) override {
    return osTOML.DescribeProperty(name);
  }


  const char* SCI_METHOD DescribeWordListSets() override {
    return osTOML.DescribeWordListSets();
  }

  void* SCI_METHOD PrivateCall(int, void*) override {
    return nullptr;
  }

  int SCI_METHOD LineEndTypesSupported() override {
    return SC_LINE_END_TYPE_UNICODE;
  }

  int SCI_METHOD PrimaryStyleFromStyle(int style) override {
    return style;
  }

  static ILexer4* LexerFactoryTOML() {
    return new LexerTOML();
  }

  // --------------------------------------------------------------------------

  Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;

  Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;

  void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

  void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

};


Sci_Position SCI_METHOD LexerTOML::PropertySet(const char* key, const char* val) {
  if (osTOML.PropertySet(&options, key, val)) {
    return 0;
  }
  return -1;
}


Sci_Position SCI_METHOD LexerTOML::WordListSet(int n, const char* wl) 
{
  WordList* wordListN = nullptr;

  switch (n) {
    case 0:
      wordListN = &keywords;
      break;
  }

  Sci_Position firstModification = -1;

  if (wordListN) {
    WordList wlNew;
    wlNew.Set(wl);

    if (*wordListN != wlNew) {
      wordListN->Set(wl);
      firstModification = 0;
    }
  }
  return firstModification;
}
// ----------------------------------------------------------------------------


constexpr bool IsCommentChar(const int ch) noexcept {
  //return (ch == '#') || (ch == ':');
  return (ch == '#');
}
// ----------------------------------------------------------------------------

constexpr bool IsAssignChar(const int ch) noexcept {
  //return (ch == '=') || (ch == ':');
  return (ch == '=');
}
// ----------------------------------------------------------------------------

inline bool IsAKeyChar(const int ch) {
  return (IsAlphaNumeric(ch) || ch == '_');
}
// ----------------------------------------------------------------------------


static int GetBracketLevel(StyleContext& sc) 
{
  Sci_Position const posCurrent = static_cast<Sci_Position>(sc.currentPos);

  bool ignore = false;
  int iBracketLevel = -1;

  Sci_Position i = 0;
  while ((--i + posCurrent) >= 0) 
  {
    if (sc.GetRelative(i) == '"') {
      ignore = !ignore; // toggle string
    }
    else if (!ignore) {
      if (IsAssignChar(sc.GetRelative(i))) {
        break; // must be within assignment
      }
      else if (sc.GetRelative(i) == ']') {
        --iBracketLevel;
      }
      else if (sc.GetRelative(i) == '[') {
        ++iBracketLevel;
      }
    }
  }
  return iBracketLevel;
}
// ----------------------------------------------------------------------------



void SCI_METHOD LexerTOML::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) 
{
  Accessor styler(pAccess, nullptr);
  StyleContext sc(startPos, length, initStyle, styler);

  bool inSectionDef = false;
  bool inMultiLnString = (sc.state == SCE_TOML_STRING);
  bool inMultiLnArrayDef = (sc.state == SCE_TOML_ARRAY);

  for (; sc.More(); sc.Forward())
  {

    // --------------------------------------------------
    // check if in the middle of a line continuation ...
    // --------------------------------------------------
    if (sc.atLineStart) {
      switch (sc.state)
      {
        case SCE_TOML_STRING:
          if (!inMultiLnString) {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
          break;
        case SCE_TOML_ARRAY:
          if (!inMultiLnArrayDef) {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
          break;
        case SCE_TOML_PARSINGERROR:
          // preserve error
          break;
        default:
          sc.SetState(SCE_TOML_DEFAULT); // reset
          break;
      }
    }

    // -------------------------
    // current state independent
    // -------------------------
    if (IsLineBreak(sc.ch)) {
      continue; // eat line breaks
    }

    if (sc.ch != SCE_TOML_PARSINGERROR) 
    {
      if (IsCommentChar(sc.ch)) {
        if (inSectionDef) {
          sc.SetState(SCE_TOML_PARSINGERROR);
        }
        else if (inMultiLnString || inMultiLnArrayDef) {
          sc.ForwardSetState(sc.state); // ignore
        }
        else {
          sc.SetState(SCE_TOML_COMMENT);
        }
      }

    } // SCE_TOML_PARSINGERROR


    // -------------------------
    // state dependent
    // -------------------------
    switch (sc.state)
    {
      case SCE_TOML_DEFAULT:
        if (IsASpaceOrTab(sc.ch)) {
          // eat
        }
        else if (IsCommentChar(sc.ch)) {
          sc.SetState(SCE_TOML_COMMENT);
        }
        else if (sc.ch == '[') {
          sc.SetState(SCE_TOML_SECTION);
          inSectionDef = true;
        }
        else if (IsAKeyChar(sc.ch)) {
          sc.SetState(SCE_TOML_KEY);
        }
        else {
          sc.SetState(SCE_TOML_PARSINGERROR);
        }
        break;

      case SCE_TOML_COMMENT:
        // eat - rest of line is comment
        break;

      case SCE_TOML_SECTION:
        if (sc.ch == ']') {
          inSectionDef = false;
        }
        else if (IsCommentChar(sc.ch)) {
          if (!inSectionDef) {
            sc.SetState(SCE_TOML_COMMENT);
          }
          else {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
        }
        break;

      case SCE_TOML_KEY:
        if (IsASpaceOrTab(sc.ch)) {
          sc.SetState(SCE_TOML_ASSIGNMENT); // end of key
        }
        else if (IsAssignChar(sc.ch)) {
          sc.SetState(SCE_TOML_ASSIGNMENT);
        }
        else if (!IsAKeyChar(sc.ch)) {
          sc.SetState(SCE_TOML_PARSINGERROR);
        }
        break;

      case SCE_TOML_ASSIGNMENT:
        if (IsAssignChar(sc.ch)) {
          sc.ForwardSetState(SCE_TOML_VALUE);
          // fall through case SCE_TOML_VALUE:
        }
        else if (IsASpaceOrTab(sc.ch)) {
          break; // OK
        }
        else {
          sc.SetState(SCE_TOML_PARSINGERROR);
          break;
        }
        // fall through

      case SCE_TOML_VALUE:
        if (sc.ch == '[') {
          sc.SetState(SCE_TOML_ARRAY);
          inMultiLnArrayDef = true;
        }
        else if (sc.ch == ']') {
          sc.SetState(SCE_TOML_PARSINGERROR);
        }
        else if (sc.ch == '"') {
          sc.SetState(SCE_TOML_STRING);
          if (sc.Match(R"(""")")) {
            inMultiLnString = true;
            sc.Forward(2);
          }
        }
        break;

      case SCE_TOML_STRING:
        if (sc.ch == '\\') {
          sc.ForwardSetState(SCE_TOML_STRING);
        }
        else if (sc.ch == '"') {
          if (!inMultiLnString) {
            sc.ForwardSetState(SCE_TOML_VALUE);
          }
          else { 
            // inMultiLnString
            if (sc.Match(R"(""")")) {
              sc.Forward(2);
              sc.ForwardSetState(SCE_TOML_VALUE);
              inMultiLnString = false;
            }
            else {
              sc.SetState(SCE_TOML_PARSINGERROR);
            }
          }
        }
        break;

      case SCE_TOML_ARRAY:
        if (sc.ch == ']') {
          int const level = GetBracketLevel(sc);
          if (level == 0) {
            sc.ForwardSetState(SCE_TOML_VALUE);
            inMultiLnArrayDef = false;
          }
          else if (level < 0) {
            sc.SetState(SCE_TOML_PARSINGERROR);
            inMultiLnArrayDef = false;
          }
        }
        break;

      case SCE_TOML_PARSINGERROR:
        // still parsing error until new line
        break;

      default:
        sc.SetState(SCE_TOML_PARSINGERROR); // unknown
        break;
    }

    //if (sc.atLineEnd) {
    //  // ---
    //}

  }
  sc.Complete();
}
// ----------------------------------------------------------------------------



void SCI_METHOD LexerTOML::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument* pAccess)
{
  if (!options.fold) {
    return;
  }

  Accessor styler(pAccess, nullptr);

  //const Sci_Position docLines = styler.GetLine(styler.Length());
  //const Sci_Position maxPos = startPos + length;
  //const Sci_Position maxLines = styler.GetLine(maxPos == styler.Length() ? maxPos : maxPos - 1);

  const Sci_PositionU endPos = startPos + length;
  int visibleChars = 0;
  Sci_Position lineCurrent = styler.GetLine(startPos);

  char chNext = styler[startPos];
  int styleNext = styler.StyleAt(startPos);
  bool headerPoint = false;
  int lev;

  for (Sci_PositionU i = startPos; i < endPos; i++) {
    const char ch = chNext;
    chNext = styler[i + 1];

    const int style = styleNext;
    styleNext = styler.StyleAt(i + 1);
    const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

    if (style == SCE_TOML_SECTION) {
      headerPoint = true;
    }

    if (atEOL) {
      lev = SC_FOLDLEVELBASE;

      if (lineCurrent > 0) {
        const int levelPrevious = styler.LevelAt(lineCurrent - 1);

        if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
          lev = SC_FOLDLEVELBASE + 1;
        }
        else {
          lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
        }
      }

      if (headerPoint) {
        lev = SC_FOLDLEVELBASE;
      }
      if (visibleChars == 0 && options.foldCompact)
        lev |= SC_FOLDLEVELWHITEFLAG;

      if (headerPoint) {
        lev |= SC_FOLDLEVELHEADERFLAG;
      }
      if (lev != styler.LevelAt(lineCurrent)) {
        styler.SetLevel(lineCurrent, lev);
      }

      lineCurrent++;
      visibleChars = 0;
      headerPoint = false;
    }
    if (!isspacechar(ch))
      visibleChars++;
  }

  if (lineCurrent > 0) {
    const int levelPrevious = styler.LevelAt(lineCurrent - 1);
    if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
      lev = SC_FOLDLEVELBASE + 1;
    }
    else {
      lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
    }
  }
  else {
    lev = SC_FOLDLEVELBASE;
  }
  int flagsNext = styler.LevelAt(lineCurrent);
  styler.SetLevel(lineCurrent, lev | (flagsNext & ~SC_FOLDLEVELNUMBERMASK));

}
// ----------------------------------------------------------------------------

LexerModule lmTOML(SCLEX_TOML, LexerTOML::LexerFactoryTOML, "toml", tomlWordListDesc);

// ----------------------------------------------------------------------------

