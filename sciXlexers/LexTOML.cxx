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

  struct OptionsTOML {
    bool fold;
    bool foldCompact;

    OptionsTOML() {
      fold = true;
      foldCompact = true;
    }
  };

  static const char* const tomlWordLists[] = {
      "Keyword",
      nullptr
  };

  struct OptionSetTOML : public OptionSet<OptionsTOML> {
    OptionSetTOML() {

      DefineProperty("fold", &OptionsTOML::fold, "FOLD COMMENT");
      DefineProperty("fold.compact", &OptionsTOML::foldCompact, "FOLDCOMPACT COMMENT");

      DefineWordListSets(tomlWordLists);
    }
  };

  LexicalClass lexicalClasses[] = {
    // Lexer TOML SCLEX_TOML SCE_TOML_:
    0,  "SCE_TOML_DEFAULT",        "default",              "Default",
    1,  "SCE_TOML_KEYWORD",        "keyword",              "Keyword",
    2,  "SCE_TOML_COMMENT",        "comment",              "Comment",
    3,  "SCE_TOML_SECTION",        "section",              "Section",
    4,  "SCE_TOML_KEY",            "key",                  "Key",
    5,  "SCE_TOML_ASSIGNMENT",     "assignment",           "Assignment",
    6,  "SCE_TOML_VALUE",          "value",                "Value",
    7,  "SCE_TOML_NUMBER",         "number",               "Number",
    8,  "SCE_TOML_STR_BASIC",      "string_basic",         "Basic String",
    9,  "SCE_TOML_STR_LITERAL",    "string_basic",         "Literal String",
   10,  "SCE_TOML_PARSINGERROR",   "type_error",           "Type Error",
  };

  } // end of namespace

class LexerTOML : public DefaultLexer {
  
  CharacterSet validKey;
  CharacterSet validKeyWord;
  CharacterSet validNumberEnd;
  CharacterSet chDateTime;
  
  WordList keywords;

  OptionsTOML options;
  OptionSetTOML osTOML;

public:
  LexerTOML() 
    : DefaultLexer(lexicalClasses, ELEMENTS(lexicalClasses))
    , validKey(CharacterSet::setAlphaNum, R"(-_.)", 0x80, false)
    , validKeyWord(CharacterSet::setAlphaNum, "_+-", 0x80, false)
    , validNumberEnd(CharacterSet::setNone, " \t\n\v\f\r#,)}]", 0x80, false)
    , chDateTime(CharacterSet::setNone, "-:TZ", 0x80, false)
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

inline bool IsAIdentifierChar(const int ch) {
  return (IsAlphaNumeric(ch) || ch == '_' || ch == '.');
}
// ----------------------------------------------------------------------------

inline bool IsAKeywordChar(const int ch) {
  return (IsAIdentifierChar(ch) || ch == '+' || ch == '-');
}
// ----------------------------------------------------------------------------


static int GetBracketLevel(StyleContext& sc, const bool stopAtLnBreak = false) 
{
  Sci_Position const posCurrent = static_cast<Sci_Position>(sc.currentPos);

  int iBracketLevel = -1;
  int inInlTbl = 0;

  Sci_Position i = 0;
  while (((--i + posCurrent) >= 0))
  {
    int const ch = sc.GetRelative(i);

    if (stopAtLnBreak && IsLineBreak(ch)) {
      break;
    }

    if (ch == '}') {
      ++inInlTbl;
    }
    else if (ch == '{') {
      --inInlTbl;
    }

    if (IsAssignChar(ch) && (inInlTbl == 0)) {
      break; // must be the assignment begin
    }
    else if (ch == ']') {
      --iBracketLevel;
    }
    else if (ch == '[') {
      ++iBracketLevel;
    }
  }
  return iBracketLevel;
}
// ----------------------------------------------------------------------------

static bool IsDateTimeStr(StyleContext& sc, const CharacterSet& validCh, const CharacterSet& valEnd)
{
  Sci_Position const posCurrent = static_cast<Sci_Position>(sc.currentPos);
  Sci_Position const posEnd = static_cast<Sci_Position>(sc.lineStartNext);

  bool bDateTimeFlag = false;
  
  Sci_Position i = 0;
  while ((++i + posCurrent) < posEnd)
  {
    int const ch = sc.GetRelative(i);

    if (!Scintilla::IsADigit(ch) && !validCh.Contains(ch) && (ch != '.')) {
      if (valEnd.Contains(ch)) {
        return bDateTimeFlag;
      }
      else {
        return false;
      }
    }
    if (validCh.Contains(ch)) {
      bDateTimeFlag = true;
    }
  }
  return bDateTimeFlag;
}
// ----------------------------------------------------------------------------


static bool IsLookAheadLineEmpty(StyleContext& sc)
{
  Sci_Position const posCurrent = static_cast<Sci_Position>(sc.currentPos);
  Sci_Position const posEnd = static_cast<Sci_Position>(sc.lineStartNext);

  bool bLHLineEmpty = true;

  Sci_Position i = 0;
  while ((++i + posCurrent) < posEnd)
  {
    int const ch = sc.GetRelative(i);

    if (!Scintilla::IsASpace(ch)) {
      if (IsCommentChar(ch)) {
        break; // ignore rest of line
      }
      bLHLineEmpty = false;
      break;
    }
  }
  return bLHLineEmpty;
}
// ----------------------------------------------------------------------------

static bool IsLookAheadInList(StyleContext& sc, const CharacterSet& validCh, const WordList& keywords)
{
  Sci_Position const posCurrent = static_cast<Sci_Position>(sc.currentPos);
  Sci_Position const posEnd = static_cast<Sci_Position>(sc.lineStartNext);

  static char identifier[1024] = { '\0' };

  int j = 0;
  Sci_Position i = -1;
  while (((++i + posCurrent) < posEnd) && (j < 1023))
  {
    int const ch = sc.GetRelative(i);

    if (IsABlankOrTabX(ch)) {
      if (j == 0) { continue; }
    }
    if (validCh.Contains(ch)) {
      identifier[j++] = static_cast<char>(ch);
      continue;
    }
    identifier[j] = '\0';
    break;
  }

  if (identifier[0] != '\0') {
    TrimIdentifier(identifier, identifier);
    if (keywords.InList(identifier)) {
      return true;
    }
  }
  return false;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------

void SCI_METHOD LexerTOML::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) 
{
  Accessor styler(pAccess, nullptr);
  StyleContext sc(startPos, length, initStyle, styler);

  bool inSQuotedKey = false;
  bool inDQuotedKey = false;
  bool inInnerQKey = false;

  bool inSectionDef = false;
  
  bool inMultiLnString = (sc.state == SCE_TOML_STR_BASIC) || (sc.state == SCE_TOML_STR_LITERAL);
  bool inMultiLnArrayDef = false;
  
  bool inHex = false;
  bool inBin = false;
  bool inOct = false;

  bool bPossibleKeyword = true;

  for (; sc.More(); sc.Forward())
  {

    // --------------------------------------------------
    // check if in the middle of a line continuation ...
    // --------------------------------------------------
    // reset context infos
    if (sc.atLineStart) {
      inMultiLnArrayDef = (GetBracketLevel(sc) >= 0);
      inSQuotedKey = inDQuotedKey = inInnerQKey = false;
      bPossibleKeyword = true;

      switch (sc.state)
      {
        case SCE_TOML_STR_BASIC:
        case SCE_TOML_STR_LITERAL:
          if (!inMultiLnString) {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
          break;
        case SCE_TOML_KEY:
        case SCE_TOML_ASSIGNMENT:
          sc.SetState(SCE_TOML_PARSINGERROR);
          break;
        case SCE_TOML_PARSINGERROR:
          if (!inMultiLnArrayDef) {
            sc.SetState(SCE_TOML_DEFAULT);
          }
          break;
        default:
          if (!inMultiLnArrayDef) {
            sc.SetState(SCE_TOML_DEFAULT); // reset
          }
          break;
      }
    }

    // -------------------------
    // current state independent
    // -------------------------
    if (IsLineBreak(sc.ch)) {
      continue; // eat line breaks
    }

    if (sc.state != SCE_TOML_PARSINGERROR) 
    {
      if (IsCommentChar(sc.ch)) {
        if (inSectionDef) {
          sc.SetState(SCE_TOML_PARSINGERROR);
        }
        else if ((sc.state == SCE_TOML_STR_BASIC) || 
                 (sc.state == SCE_TOML_STR_LITERAL) || 
                 inMultiLnArrayDef)
        {
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
        else if (validKey.Contains(sc.ch)) {
          sc.SetState(SCE_TOML_KEY);
        }
        else { // not valid - maybe quoted
          if (sc.ch == '"') {
            inDQuotedKey = true;
            sc.SetState(SCE_TOML_KEY);
          }
          else if (sc.ch == '\'') {
            inSQuotedKey = true;
            sc.SetState(SCE_TOML_KEY);
          }
          else {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
        }
        break;


      case SCE_TOML_COMMENT:
        // eat - rest of line is comment
        break;


      case SCE_TOML_SECTION:
        if (sc.ch == ']') {
          if (GetBracketLevel(sc, true) == 0) {
            inSectionDef = false;
          }
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
        if (sc.atLineEnd) {
          sc.SetState(SCE_TOML_PARSINGERROR);
          break;
        }
        else if ((sc.ch == '"') && inDQuotedKey) {
          if (inInnerQKey) {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
          else {
            sc.ForwardSetState(SCE_TOML_ASSIGNMENT); // end of key
          }
          break;
        }
        else if ((sc.ch == '\'') && inSQuotedKey) {
          if (inInnerQKey) {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
          else {
            sc.ForwardSetState(SCE_TOML_ASSIGNMENT); // end of key
          }
          break;
        }
        else if (IsASpaceOrTab(sc.ch)) {
          if (!(inSQuotedKey || inDQuotedKey || inInnerQKey)) {
            sc.SetState(SCE_TOML_ASSIGNMENT); // end of key
          }
          break; // else eat
        }
        else if (IsAssignChar(sc.ch)) {
          if ((inSQuotedKey || inDQuotedKey || inInnerQKey)) {
            break;
          }
          sc.SetState(SCE_TOML_ASSIGNMENT); // end of key
          // === fall through ===  case SCE_TOML_ASSIGNMENT:
        }
        else if (validKey.Contains(sc.ch)) {
          break; //  eat
        }
        else {
          if ((sc.ch == '"') && inSQuotedKey) {
            inInnerQKey = !inInnerQKey; //toggle
          }
          else if ((sc.ch == '\'') && inDQuotedKey) {
            inInnerQKey = !inInnerQKey; //toggle
          }
          else if (!(inSQuotedKey || inDQuotedKey || inInnerQKey)) {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
          break; // else eat
        }
        // === fall through ===

      case SCE_TOML_ASSIGNMENT:
        if (sc.atLineEnd) {
          sc.SetState(SCE_TOML_PARSINGERROR);
          break;
        }
        else if (IsAssignChar(sc.ch)) {
          if (IsLookAheadLineEmpty(sc)) {
            sc.ForwardSetState(SCE_TOML_PARSINGERROR);
            break;
          }
          else {
            sc.ForwardSetState(SCE_TOML_VALUE);
            // === fall through ===  case SCE_TOML_VALUE:
          }
        }
        else if (IsASpace(sc.ch)) {
          break; // OK
        }
        else {
          sc.SetState(SCE_TOML_PARSINGERROR);
          break;
        }
        // === fall through ===

      case SCE_TOML_VALUE:
        if (bPossibleKeyword && IsLookAheadInList(sc, validKeyWord, keywords)) {
          sc.SetState(SCE_TOML_KEYWORD);
          break;
        }
        else {
          bPossibleKeyword = false;
        }
        if (sc.ch == '[') {
          inMultiLnArrayDef = true;
        }
        else if (sc.ch == ']') {
          int const level = GetBracketLevel(sc);
          if (level == 0) {
            inMultiLnArrayDef = false;
          }
          else if (level < 0) {
            sc.SetState(SCE_TOML_PARSINGERROR);
            inMultiLnArrayDef = false;
          }
        }
        else if (IsNumber(sc)) {
          if (IsDateTimeStr(sc, chDateTime, validNumberEnd)) {
            sc.SetState(SCE_TOML_DATETIME);
          }
          else {
            sc.SetState(SCE_TOML_NUMBER);
            if ((sc.ch == '+') || (sc.ch == '-')) {
              sc.Forward();
            }
            inHex = IsNumHex(sc);
            inBin = IsNumBinary(sc);
            inOct = IsNumOctal(sc);
            if (inHex || inBin || inOct) {
              sc.Forward(2);
            }
            if (IsNumExponent(sc)) {
              sc.Forward(2);
              if ((sc.ch == '+') || (sc.ch == '-')) {
                sc.Forward();
              }
            }
          }
        }
        else if (sc.ch == '"') {
          sc.SetState(SCE_TOML_STR_BASIC);
          if (sc.Match(R"(""")")) {
            inMultiLnString = true;
            sc.Forward(2);
          }
        }
        else if (sc.ch == '\'') {
          sc.SetState(SCE_TOML_STR_LITERAL);
          if (sc.Match(R"(''')")) {
            inMultiLnString = true;
            sc.Forward(2);
          }
        }
        break;


      case SCE_TOML_KEYWORD:
        if (!(IsASpaceX(sc.ch) || validKeyWord.Contains(sc.ch))) {
          sc.SetState(SCE_TOML_VALUE);
        }
        break;


      case SCE_TOML_NUMBER:
        if (sc.ch == '_') {
          // eat // TODO: only once
        }
        else if (inHex || inBin || inOct) {
          if (validNumberEnd.Contains(sc.ch)) {
            sc.SetState(SCE_TOML_VALUE);
            inHex = false;
            inBin = false;
            inOct = false;
          }
          else {
            if ((inHex && !IsADigit(sc.ch, 16)) ||
                (inBin && !IsADigit(sc.ch,  2)) ||
                (inOct && !IsADigit(sc.ch,  8)))
            {
              sc.SetState(SCE_TOML_PARSINGERROR);
            }
          }
        }
        else if (IsNumExponent(sc)) {
          sc.Forward();
          if ((sc.chNext == '+') || (sc.chNext == '-')) {
            sc.Forward();
          }
        }
        else if (sc.ch == '.') {
          // eat // TODO: only once
        }
        else if (IsADigit(sc.ch)) {
          // eat
        }
        else {
          if (validNumberEnd.Contains(sc.ch)) {
            sc.SetState(SCE_TOML_VALUE);
            inHex = false;
            inBin = false;
            inOct = false;
          }
          else {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
        }
        break;


      case SCE_TOML_DATETIME:
        if (!IsADigit(sc.ch) && !chDateTime.Contains(sc.ch) && (sc.ch != '.')) {
          if (validNumberEnd.Contains(sc.ch)) {
            sc.SetState(SCE_TOML_VALUE);
          }
          else {
            sc.SetState(SCE_TOML_PARSINGERROR);
          }
        }
        break;


      case SCE_TOML_STR_BASIC:
      case SCE_TOML_STR_LITERAL:
        if (sc.ch == '"') {
          if (sc.state == SCE_TOML_STR_BASIC) {
            if (sc.chPrev != '\\') {
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
              }
            }
          }
        }
        else if (sc.ch == '\'') {
          if (sc.state == SCE_TOML_STR_LITERAL) {
            if (!inMultiLnString) {
              sc.ForwardSetState(SCE_TOML_VALUE);
            }
            else {
              // inMultiLnString
              if (sc.Match(R"(''')")) {
                sc.Forward(2);
                sc.ForwardSetState(SCE_TOML_VALUE);
                inMultiLnString = false;
              }
            }
          }
        }
        break;


      case SCE_TOML_PARSINGERROR:
        // keep parsing error until new line
        break;


      default:
        sc.SetState(SCE_TOML_PARSINGERROR); // unknown
        break;
    }

    //~if (sc.atLineEnd) {
    //~  if (!inMultiLnArrayDef && !inMultiLnString) {
    //~    if (sc.state == SCE_TOML_VALUE) {
    //~      sc.ForwardSetState(SCE_TOML_DEFAULT);
    //~    }
    //~  }
    //~}

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

LexerModule lmTOML(SCLEX_TOML, LexerTOML::LexerFactoryTOML, "toml", tomlWordLists);

// ----------------------------------------------------------------------------

