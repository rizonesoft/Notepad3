// encoding: UTF-8
// Scintilla source code edit control
/** @file LexCSV.cxx
** Rainbow clouring for CSV files
** Written by RaiKoHoff
**/

#include <string>
#include <assert.h>
#include <map>
//
#include "ILexer.h"
#include "Scintilla.h"
#include "StringCopy.h"
#include "PropSetSimple.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "LexerModule.h"
#include "DefaultLexer.h"
#include "OptionSet.h"
#include "WordList.h"
//
#include "CharSetX.h"
#include "SciXLexer.h"


using namespace Scintilla;

namespace
{
// Use an unnamed namespace to protect the functions and classes from name conflicts

enum delim : unsigned int { eComma = 0, eSemic, eTab, ePipe, eMax };
static int const DelimList[eMax] = { ',',  ';',  '\t',  '|' };

// =================================================================================

struct OptionsCSV
{
    bool fold;
    bool foldCompact;
    OptionsCSV() 
        : fold(true)
        , foldCompact(true)
    { }
};

static const char* const csvWordLists[] =
{
    nullptr
};

struct OptionSetCSV : public OptionSet<OptionsCSV>
{
    OptionSetCSV()
    {
        DefineProperty("fold", &OptionsCSV::fold);
        DefineProperty("fold.compact", &OptionsCSV::foldCompact);

        DefineWordListSets(csvWordLists);
    }
};

LexicalClass lexicalClasses[] =
{
    // Lexer CSV SCLEX_CSV SCE_CSV_:
    0,  "SCE_CSV_DEFAULT",        "default",       "Default",
    1,  "SCE_CSV_COLUMN_0",       "col_0",         "Column 0",
    2,  "SCE_CSV_COLUMN_1",       "col_1",         "Column 1",
    3,  "SCE_CSV_COLUMN_2",       "col_2",         "Column 2",
    4,  "SCE_CSV_COLUMN_3",       "col_3",         "Column 3",
    5,  "SCE_CSV_COLUMN_4",       "col_4",         "Column 4",
    6,  "SCE_CSV_COLUMN_5",       "col_5",         "Column 5",
    7,  "SCE_CSV_COLUMN_6",       "col_6",         "Column 6",
    8,  "SCE_CSV_COLUMN_7",       "col_7",         "Column 7",
    9,  "SCE_CSV_COLUMN_8",       "col_8",         "Column 8",
    10, "SCE_CSV_COLUMN_9",       "col_9",         "Column 9",
};


} // end of namespace

class LexerCSV : public DefaultLexer
{

    WordList keywords;

    OptionsCSV options;
    OptionSetCSV osCSV;

public:
    LexerCSV()
        : DefaultLexer("CSV", SCLEX_CSV, lexicalClasses, ELEMENTS(lexicalClasses))
    { }

    virtual ~LexerCSV() { }

    void SCI_METHOD Release() override
    {
        delete this;
    }

    int SCI_METHOD Version() const override
    {
        return lvRelease5;
    }

    const char* SCI_METHOD PropertyNames() override
    {
        return osCSV.PropertyNames();
    }

    int SCI_METHOD PropertyType(const char* name) override
    {
        return osCSV.PropertyType(name);
    }

    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osCSV.PropertyGet(name);
    }

    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osCSV.DescribeProperty(name);
    }


    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osCSV.DescribeWordListSets();
    }

    void* SCI_METHOD PrivateCall(int, void*) override
    {
        return nullptr;
    }

    int SCI_METHOD LineEndTypesSupported() override
    {
        return SC_LINE_END_TYPE_DEFAULT;
    }

    int SCI_METHOD PrimaryStyleFromStyle(int style) override
    {
        return style;
    }

    static ILexer5* LexerFactoryCSV()
    {
        return new LexerCSV();
    }

    // --------------------------------------------------------------------------

    Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;
    Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;
    void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
    void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;

};


Sci_Position SCI_METHOD LexerCSV::PropertySet(const char* key, const char* val)
{
    if (osCSV.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerCSV::WordListSet(int n, const char* wl)
{
    WordList* wordListN = nullptr;

    switch (n)
    {
    case 0:
        wordListN = &keywords;
        break;
    }

    Sci_Position firstModification = -1;

    if (wordListN)
    {
        WordList wlNew;
        wlNew.Set(wl);
        if (*wordListN != wlNew)
        {
            wordListN->Set(wl);
            firstModification = 0;
        }
    }
    return firstModification;
}
// ----------------------------------------------------------------------------

constexpr bool IsSingleQuoteChar(const int ch) noexcept
{
    return (ch == '\'');
}
// ----------------------------------------------------------------------------

constexpr bool IsDoubleQuoteChar(const int ch) noexcept
{
    return (ch == '"');
}
// ----------------------------------------------------------------------------

constexpr unsigned int IsDelimiter(const int ch) noexcept
{
    for (unsigned int i = 0; i < eMax; ++i)
    {
        if (DelimList[i] == ch)
        {
            return i;
        }
    }
    return eMax;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

constexpr int GetStateByColumn(const int col) noexcept
{
    switch (col % 10)
    {
    case 0:
        return SCE_CSV_COLUMN_0;
    case 1:
        return SCE_CSV_COLUMN_1;
    case 2:
        return SCE_CSV_COLUMN_2;
    case 3:
        return SCE_CSV_COLUMN_3;
    case 4:
        return SCE_CSV_COLUMN_4;
    case 5:
        return SCE_CSV_COLUMN_5;
    case 6:
        return SCE_CSV_COLUMN_6;
    case 7:
        return SCE_CSV_COLUMN_7;
    case 8:
        return SCE_CSV_COLUMN_8;
    case 9:
        return SCE_CSV_COLUMN_9;
    default:
        return SCE_CSV_COLUMN_0;
    }
    return SCE_CSV_COLUMN_0;
}
// ----------------------------------------------------------------------------


void SCI_METHOD LexerCSV::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess)
{
    PropSetSimple props;
    props.SetMultiple(osCSV.PropertyNames());

    Accessor styler(pAccess, &props);

    // 2 passes:  1st pass: smart delimiter detection,   2nd pass: do styling

    Sci_PositionU delimCount[eMax] = { 0 };
    Sci_PositionU countPerPrevLine[eMax] = { 0 };

    //Sci_PositionU totalCount[eMax] = { 0 };
    //Sci_PositionU lineCount[eMax] = { 0 };

    Sci_PositionU smartDelimVote[eMax] = { 0 };
    Sci_PositionU columnAvg = 0;

    // 1st PASS:

    bool isInSQString = false;
    bool isInDQString = false;

    StyleContext cnt(startPos, length, initStyle, styler);
    for (; cnt.More(); cnt.Forward())
    {
        // reset column infos
        if (cnt.atLineStart)
        {
            isInSQString = false;
            isInDQString = false;

            for (unsigned int i = 0; i < eMax; ++i)
            {
                Sci_PositionU const dlm = delimCount[i];
                if (dlm > 0)
                {
                    smartDelimVote[i] += 1;

                    if ((dlm == countPerPrevLine[i]))
                    {
                        smartDelimVote[i] += dlm; // bonus for column number
                    }

                    // e.g. delim=TAB, all columns decimal numbers with comma(,) as decimal-point => comma wins over TAB
                    if (dlm == columnAvg)
                    {
                        smartDelimVote[i] += dlm; // correction for  #delimiter = (#columns - 1);
                    }
                    columnAvg = (columnAvg == 0) ? dlm : (columnAvg + dlm - 1) >> 1;

                }
                countPerPrevLine[i] = dlm;
                delimCount[i] = 0;

                //totalCount[i] += dlm;
                //++lineCount[i];
            }
        } // cnt.atLineStart

        if (IsSingleQuoteChar(cnt.ch))
        {
            if (!isInDQString)
            {
                isInSQString = !isInSQString; // toggle
            }
        }
        else if (IsDoubleQuoteChar(cnt.ch))
        {
            if (!isInSQString)
            {
                isInDQString = !isInDQString; // toggle
            }
        }
        else if (!isInSQString && !isInDQString)
        {
            unsigned int i = IsDelimiter(cnt.ch);
            if (i < eMax)
            {
                ++delimCount[i];
            }
        }
    }
    cnt.Complete();

    // --------------------------
    // smar delimiter selection
    // --------------------------
    int delim = DelimList[0];
    Sci_PositionU maxVote = smartDelimVote[0];
    for (unsigned int i = 1; i < eMax; ++i)
    {
        if (maxVote < smartDelimVote[i])
        {
            delim = DelimList[i];
            maxVote = smartDelimVote[i];
        }
    }
    // --------------------------

    int const delimiter = delim;

    // ------------------------------------------------------------------------------
    // 2nd PASS
    // ------------------------------------------------------------------------------

    int csvColumn = 0;
    isInSQString = false;
    isInDQString = false;

    StyleContext sc(startPos, length, initStyle, styler);
    for (; sc.More(); sc.Forward())
    {
        // reset context infos
        if (sc.atLineStart)
        {
            csvColumn = 0;
            isInSQString = false;
            isInDQString = false;
            sc.SetState(GetStateByColumn(csvColumn));
        }

        if (IsSingleQuoteChar(sc.ch))
        {
            if (!isInDQString)
            {
                isInSQString = !isInSQString; // toggle
            }
        }
        else if (IsDoubleQuoteChar(sc.ch))
        {
            if (!isInSQString)
            {
                isInDQString = !isInDQString; // toggle
            }
        }
        else if (delimiter == sc.ch)
        {
            if (!isInSQString && !isInDQString)
            {
                sc.SetState(GetStateByColumn(++csvColumn));
            }
        }

    }
    sc.Complete();
}
// ----------------------------------------------------------------------------



void SCI_METHOD LexerCSV::Fold(Sci_PositionU startPos, Sci_Position length, int, IDocument* pAccess)
{
    return;
}
// ----------------------------------------------------------------------------

LexerModule lmCSV(SCLEX_CSV, LexerCSV::LexerFactoryCSV, "CSV", csvWordLists);

// ----------------------------------------------------------------------------

