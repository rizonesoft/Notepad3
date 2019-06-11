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
#include "CharacterSet.h"
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
      return SCE_TOML_TYPEERROR;
    }
    return SCE_TOML_DATATYPE;
  }

  inline bool IsLetter(const int ch) {
    // 97 to 122 || 65 to 90
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
  }

  inline bool IsAWordChar(const int ch) {
    return (ch < 0x80) && (isalnum(ch) || ch == '_' || ch == '.');
  }

  inline int IsNumHex(const StyleContext& sc) {
    return (sc.chNext == 'x') || (sc.chNext == 'X');
  }

  inline int IsNumBinary(const StyleContext& sc) {
    return (sc.chNext == 'b') || (sc.chNext == 'B');
  }

  inline int IsNumOctal(const StyleContext& sc) {
    return IsADigit(sc.chNext) || sc.chNext == 'o';
  }

  constexpr bool IsNewline(const int ch) noexcept {
    return (ch == '\n' || ch == '\r');
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

  //constexpr bool IsTripleLiteral(const int style) noexcept {
  //    return style == SCE_TOML_TRIPLE || style == SCE_TOML_TRIPLEDOUBLE;
  //}
  //
  //constexpr bool IsLineComment(const int style) noexcept {
  //    return style == SCE_TOML_COMMENTLINE || style == SCE_TOML_COMMENTLINEDOC;
  //}
  //
  //constexpr bool IsStreamComment(const int style) noexcept {
  //    return style == SCE_TOML_COMMENT || style == SCE_TOML_COMMENTDOC;
  //}


  constexpr bool IsAssignChar(unsigned char ch) {
    return (ch == '=') || (ch == ':');
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
      "Keywords",
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
    0,  "SCE_TOML_DEFAULT",        "default",              "White space",
    1,  "SCE_TOML_COMMENT",        "comment block",        "Block comment",
    2,  "SCE_TOML_KEY",            "keyc",                 "Keyword",
    3,  "SCE_TOML_SECTION",        "section",              "Section",
    4,  "SCE_TOML_ASSIGNMENT",     "assignment",           "Assignment",
    5,  "SCE_TOML_DEFVAL",         "default value",        "Default Value",
    6,  "SCE_TOML_DATATYPE",       "datatype",             "Datatype",
    7,  "SCE_TOML_TYPEERROR",      "type error",           "Type Error",
  };

  } // end of namespace

class LexerTOML : public DefaultLexer {
  CharacterSet setWord;
  WordList keywords;
  OptionsTOML options;
  OptionSetTOML osTOML;

public:
  LexerTOML() :
    DefaultLexer(lexicalClasses, ELEMENTS(lexicalClasses)),
    setWord(CharacterSet::setAlphaNum, "_", 0x80, true) { }

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

  Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;

  const char* SCI_METHOD DescribeWordListSets() override {
    return osTOML.DescribeWordListSets();
  }

  Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;

  void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
  void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

  void* SCI_METHOD PrivateCall(int, void*) override {
    return 0;
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
};

Sci_Position SCI_METHOD LexerTOML::PropertySet(const char* key, const char* val) {
  if (osTOML.PropertySet(&options, key, val)) {
    return 0;
  }
  return -1;
}


Sci_Position SCI_METHOD LexerTOML::WordListSet(int n, const char* wl) {
  WordList* wordListN = 0;

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




static inline bool AtEOL(Accessor& styler, Sci_PositionU i)
{
  return (styler[i] == '\n') || ((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}

void SCI_METHOD LexerTOML::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) 
{

  Accessor styler(pAccess, nullptr);
  StyleContext sc(startPos, length, initStyle, styler);

  for (; sc.More(); sc.Forward())
  {
    if (sc.atLineStart) {
      sc.SetState(SCE_TOML_DEFAULT);
    }
  
    switch (sc.state) 
    {
      case SCE_TOML_DEFAULT:
      {
        if (sc.ch == '#' || sc.ch == '!' || sc.ch == ';') {
          sc.SetState(SCE_TOML_COMMENT);
        }
        else if (sc.ch == '[') {
          sc.SetState(SCE_TOML_SECTION);
        }
        else if (sc.ch == '@') {
          sc.SetState(SCE_TOML_DEFVAL);
        }
        else if (IsAssignChar(sc.ch)) {
          sc.SetState(SCE_TOML_ASSIGNMENT);
        }
        else if (IsLetter(sc.ch)) {
          sc.SetState(SCE_TOML_KEY);
        }
      }
      break;

      case SCE_TOML_COMMENT:
        break;

      case SCE_TOML_KEY:
        if (!IsLetter(sc.ch)) {
          if (IsAssignChar(sc.ch)) {
            sc.SetState(SCE_TOML_ASSIGNMENT);
          }
          else {
            sc.SetState(SCE_TOML_DEFAULT);
          }
        }
        break;

      case SCE_TOML_SECTION:
        if (sc.ch == ']') {
          sc.Forward();
          sc.SetState(SCE_TOML_DEFAULT);
        }
      break;

      case SCE_TOML_ASSIGNMENT:
        if (!IsAssignChar(sc.ch)) {
          sc.SetState(SCE_TOML_DEFVAL);
        }
        break;

      case SCE_TOML_DEFVAL:
        break;

      case SCE_TOML_DATATYPE:
        break;

      case SCE_TOML_TYPEERROR:
        break;

      default:
        break;
    }

    if (sc.atLineEnd) {
      sc.SetState(SCE_TOML_DEFAULT);
    }

  }
  sc.Complete();
}




void SCI_METHOD LexerTOML::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument* pAccess)
{
  if (!options.fold) {
    return;
  }

  Accessor styler(pAccess, NULL);

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

LexerModule lmTOML(SCLEX_TOML, LexerTOML::LexerFactoryTOML, "toml", tomlWordListDesc);
