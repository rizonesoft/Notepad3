#define NOMINMAX
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <strsafe.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using cpi_enc_t = int;
using HPATHL = const wchar_t*;
#include "EncodingDetectionTypes.inc"

template<class A, class B> size_t min_s(A a, B b) { return (std::min)((size_t)a, (size_t)b); }
template<class A, class B> size_t max_s(A a, B b) { return (std::max)((size_t)a, (size_t)b); }
#define COUNTOF(a) (sizeof(a) / sizeof((a)[0]))
constexpr float MIN_CONFIDENCE_ANSI_NOT_UTF8 = 0.995f;
struct {
    bool LoadASCIIasUTF8 = true;
    int DefaultEncoding = CPI_UTF8;
    bool UseDefaultForFileEncoding = false;
    bool LoadNFOasOEM = false;
    int AnalyzeReliableConfidenceLevel = 60;
} Settings;
struct { float LocaleAnsiCodePageAnalysisBonus = 0.20f; } Settings2;
struct { bool bDevDebugMode = false; } Flags;
constexpr unsigned FV_ENCODING = 64;
struct FileVars { unsigned mask = 0; };
struct { FileVars fvCurFile; int DOSEncoding = CPI_OEM; } Globals;
static int forcedEncoding = CPI_NONE;
static int weakEncoding = CPI_NONE;
static int fileVarEncoding = CPI_NONE;
static const wchar_t* extension = L".txt";
static int legacyEncoding = CPI_ANSI_DEFAULT;
constexpr int CPI_TEST_CP1251 = 8;
constexpr int CPI_TEST_CP1258 = 9;
static int analysisCalls = 0;
static size_t analysisLength = 0;
static bool failAllocation = false;
static float analysisConfidence = 0.5f;
bool Encoding_IsValid(int enc) { return enc >= 0 && enc < 10; }
bool Encoding_IsUNICODE(int enc) { return enc >= 2 && enc <= 7; }
bool Encoding_IsUNICODE_REVERSE(int enc) { return enc == CPI_UNICODEBE || enc == CPI_UNICODEBEBOM; }
bool Encoding_IsEXTERNAL_8BIT(int enc) { return enc == CPI_ANSI_DEFAULT || enc >= 8; }
int Encoding_SrcWeak(int) { return weakEncoding; }
int Encoding_Forced(int) { return forcedEncoding; }
bool FileVars_IsValidEncoding(FileVars*) { return Encoding_IsValid(fileVarEncoding); }
int FileVars_GetEncoding(FileVars*) { return fileVarEncoding; }
void FileVars_GetFromData(const char*, size_t, FileVars*) {}
const wchar_t* Path_FindExtension(HPATHL) { return extension; }
int StringCchCompareXI(const wchar_t* a, const wchar_t* b) { return _wcsicmp(a, b); }
static WCHAR wchEncodingInfo[1024] = L"";
UINT Encoding_GetCodePage(int enc) {
    return enc == CPI_ANSI_DEFAULT ? 1252 : enc == CPI_TEST_CP1251 ? 1251 : enc == CPI_TEST_CP1258 ? 1258 : CP_UTF8;
}
void* AllocMem(size_t size, DWORD flags) { return failAllocation ? nullptr : HeapAlloc(GetProcessHeap(), flags, size); }
void FreeMem(void* p) { HeapFree(GetProcessHeap(), 0, p); }
bool IsUTF8Signature(const char* p) { return memcmp(p, "\xEF\xBB\xBF", 3) == 0; }
bool Has_UTF32_LE_BOM(const char* p, size_t n) { return n >= 4 && memcmp(p, "\xFF\xFE\0\0", 4) == 0; }
bool Has_UTF32_BE_BOM(const char* p, size_t n) { return n >= 4 && memcmp(p, "\0\0\xFE\xFF", 4) == 0; }
bool Has_UTF32_BOM(const char* p, size_t n) { return Has_UTF32_LE_BOM(p, n) || Has_UTF32_BE_BOM(p, n); }
bool Has_UTF16_LE_BOM(const char* p, size_t n) { return n >= 2 && memcmp(p, "\xFF\xFE", 2) == 0 && !Has_UTF32_LE_BOM(p, n); }
bool Has_UTF16_BE_BOM(const char* p, size_t n) { return n >= 2 && memcmp(p, "\xFE\xFF", 2) == 0; }
bool IsValidUTF8(const char*, size_t, bool*, bool*, bool*);
cpi_enc_t AnalyzeText_UCHARDET(const char* text, size_t length, float* confidence, char* name, int count)
{
    ++analysisCalls;
    analysisLength = length;
    // Deterministic reproduction of the reliable, wrong legacy-encoding result.
    bool nulls = false;
    bool const utf8 = IsValidUTF8(text, length, nullptr, nullptr, &nulls) || nulls;
    StringCchCopyA(name, count, utf8 ? "UTF-8" : legacyEncoding == CPI_TEST_CP1258 ? "WINDOWS-1258" : "WINDOWS-1252");
    *confidence = analysisConfidence;
    return utf8 ? CPI_UTF8 : legacyEncoding;
}
#include "EncodingDetectionFunctions.inc"

static void Check(bool condition, const char* name)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", name);
        std::exit(1);
    }
}

static std::string Encode16(const std::wstring& text, bool bigEndian)
{
    std::string bytes;
    for (wchar_t ch : text) {
        bytes.push_back((char)(bigEndian ? ch >> 8 : ch & 0xFF));
        bytes.push_back((char)(bigEndian ? ch & 0xFF : ch >> 8));
    }
    return bytes;
}

static ENC_DET_T Detect(const std::string& bytes, bool force = false, bool skip = false)
{
    return Encoding_DetectEncoding(L"test.txt", bytes.data(), bytes.size(), CPI_ANSI_DEFAULT, skip, false, force);
}

static void CheckModes(const std::string& bytes, int expected)
{
    for (bool debug : {false, true}) {
        Flags.bDevDebugMode = debug;
        for (bool force : {false, true}) {
            int const before = analysisCalls;
            ENC_DET_T const result = Detect(bytes, force);
            Check(result.Encoding == expected, "UTF-16 selection in normal/debug/forced detection");
            Check(result.bIsReverse == Encoding_IsUNICODE_REVERSE(expected), "UTF-16 byte order");
            Check((analysisCalls > before) == (debug || force), "diagnostics may analyze without changing selection");
            if (debug) {
                Check(wcsstr(wchEncodingInfo, expected == CPI_UTF8SIGN ? L"ENC=UTF-8+BOM" : L"ENC=UTF-16") == wchEncodingInfo,
                      "title begins with final selection");
                Check(wcsstr(wchEncodingInfo, expected == CPI_UTF8SIGN ?
                      L"UCD(50.0%)=UTF-8" : L"UCD(60.0%)=WINDOWS-1252") != nullptr,
                      "title shows UCHARDET candidate with adjusted confidence");
                if (expected != CPI_UTF8SIGN) {
                    Check(wcsstr(wchEncodingInfo, L"U16=UTF-16") != nullptr, "title identifies UTF-16 evidence separately");
                }
            }
        }
    }
    Flags.bDevDebugMode = false;
}

int main(int argc, char** argv)
{
    {
        for (const auto& sample : {std::string("legacy \xE9"), std::string("UTF-8 \xE2\x82\xAC")}) {
            bool const legacy = sample[0] == 'l';
            for (int hint : {CPI_NONE, CPI_ANSI_DEFAULT, CPI_UTF8}) {
                ENC_DET_T result = INIT_ENC_DET_T;
                Flags.bDevDebugMode = true;
                Encoding_AnalyzeText(sample.data(), sample.size(), &result, hint);
                float const expected = legacy || hint == CPI_UTF8 ? 0.60f : 0.50f;
                Check(std::fabs(result.confidence - expected) < 0.00001f, "single preference bonus for hint/locale/both/neither");
            }
        }
        for (float weight : {0.0f, 0.25f, 1.0f}) {
            Settings2.LocaleAnsiCodePageAnalysisBonus = weight;
            ENC_DET_T result = INIT_ENC_DET_T;
            Encoding_AnalyzeText("legacy \xE9", 8, &result, CPI_ANSI_DEFAULT);
            Check(std::fabs(result.confidence - (0.5f + 0.5f * weight)) < 0.00001f,
                  "configured preference weight is applied once");
        }
        Settings2.LocaleAnsiCodePageAnalysisBonus = 0.20f;
        Flags.bDevDebugMode = false;
    }
    {
        std::string const legacy = "legacy \xE9";
        Check(Detect(legacy).bIsAnalysisReliable, "20% preference bonus lifts raw 50% to the 60% threshold");
        Settings2.LocaleAnsiCodePageAnalysisBonus = 0.0f;
        for (float confidence : {0.599f, 0.60f, 0.80f}) {
            analysisConfidence = confidence;
            Check(Detect(legacy).bIsAnalysisReliable == (confidence >= 0.60f),
                  "60% reliability boundary is inclusive");
        }
        analysisConfidence = 0.5f;
        Settings2.LocaleAnsiCodePageAnalysisBonus = 0.20f;
        Settings.AnalyzeReliableConfidenceLevel = 50;
        Check(Detect(legacy).bIsAnalysisReliable, "explicit confidence threshold remains effective");
        Settings.AnalyzeReliableConfidenceLevel = 60;
    }
    std::wstring text;
    {
        std::string large(500000, 'x');
        size_t length = 0;
        char* sample = _BuildAnalysisSample(large.data(), large.size(), &length);
        Check(sample && length == 200000, "no-line-break sample respects exact byte budget");
        FreeMem(sample);
        for (size_t pos = 80; pos < large.size(); pos += 81) { large[pos] = '\n'; }
        sample = _BuildAnalysisSample(large.data(), large.size(), &length);
        Check(sample && length <= 200000 && sample[length - 1] == large.back(),
              "line-aligned sampling stays bounded and includes tail");
        FreeMem(sample);
        failAllocation = true;
        Flags.bDevDebugMode = true;
        Check(Detect(large).Encoding == CPI_UTF8 && analysisLength == 200000, "sample allocation failure falls back to bounded prefix");
        Flags.bDevDebugMode = false;
        failAllocation = false;
        forcedEncoding = CPI_UTF8SIGN;
        for (const char* shortText : {"", "x", "xy"}) {
            Check(!Detect(shortText).bIsUTF8Sig, "short unsigned input cannot acquire a requested UTF-8 signature");
        }
        forcedEncoding = CPI_NONE;
    }
    for (int i = 0; i < 20; ++i) { text += L"UTF-16 text with CRLF\r\n"; }
    text += L"\xD83D\xDE00 \xD80C\xDC00\r\n";
    for (bool big : {false, true}) {
        int const expected = big ? CPI_UNICODEBE : CPI_UNICODE;
        std::string bytes = Encode16(text, big);
        CheckModes(bytes, expected);
        CheckModes(Encode16(L"\xFEFF" + text, big), big ? CPI_UNICODEBEBOM : CPI_UNICODEBOM);
        CheckModes(bytes + 'x', expected);
        Check(Detect(bytes, false, true).Encoding == CPI_ANSI_DEFAULT, "skip Unicode detection");
        forcedEncoding = CPI_UTF8;
        Check(Detect(bytes, true).Encoding == CPI_UTF8, "explicit encoding survives forced detection");
        forcedEncoding = big ? CPI_UNICODEBEBOM : CPI_UNICODEBOM;
        Check(!Detect(bytes).bHasBOM, "requested save BOM does not invent an input BOM");
        Check(Detect(Encode16(L"\xFEFF" + text, big)).bHasBOM, "matching physical UTF-16 BOM is consumed");
        std::string const oppositeBOM = big ? std::string("\xFF\xFE", 2) : std::string("\xFE\xFF", 2);
        ENC_DET_T const oppositeResult = Detect(oppositeBOM + bytes, true);
        Check(!oppositeResult.bHasBOM && oppositeResult.bIsReverse == big, "opposite BOM cannot override forced byte order or strip bytes");
        forcedEncoding = CPI_UTF8;
        Flags.bDevDebugMode = true;
        Detect(bytes, true);
        Check(wcsstr(wchEncodingInfo, L"ENC=UTF-8 <- OVR") == wchEncodingInfo,
              "title preserves explicit override reason");
        Check(wcsstr(wchEncodingInfo, L"UCD(60.0%)=WINDOWS-1252") != nullptr,
              "override preserves statistical candidate");
        Flags.bDevDebugMode = false;
        forcedEncoding = big ? CPI_UNICODE : CPI_UNICODEBE;
        ENC_DET_T const forced = Detect(bytes, true);
        Check(forced.Encoding == forcedEncoding && forced.bIsReverse == !big, "explicit byte order survives analysis");
        forcedEncoding = CPI_NONE;
        std::string invalid = bytes + Encode16(L"\xD800", big);
        bool strong = false;
        Check(DetectUTF16ByNullDistribution(invalid.data(), invalid.size(), &strong) == CPI_NONE,
              "unpaired surrogate rejected by structural detector");
        Check(DetectUTF16ByWindows(invalid.data(), invalid.size()) == CPI_NONE,
              "unpaired surrogate rejected by Windows fallback");
        Check(Detect(invalid).Encoding == CPI_ANSI_DEFAULT, "invalid surrogate data is not selected as UTF-16");
        invalid = bytes + std::string(5000, '\0') + Encode16(L"\xDC00", big);
        Check(!IsValidUTF16(invalid.data(), invalid.size(), big), "validation extends beyond first 4096 bytes");
        for (const std::wstring& bad : {std::wstring(L"\xDC00"), std::wstring(L"\xD800\xD800"), std::wstring(L"\xD800x")}) {
            invalid = bytes + Encode16(bad, big);
            Check(!IsValidUTF16(invalid.data(), invalid.size(), big), "malformed surrogate sequences rejected");
        }
        // Sparse ASCII NULs defeat the distribution ratio but leave Windows control evidence.
        std::wstring sparse = L"\r\n";
        for (int i = 0; i < 50; ++i) { sparse += L"\x4E00\x4E81"; }
        sparse += L"\r\n";
        bytes = Encode16(sparse, big);
        Check(DetectUTF16ByWindows(bytes.data(), bytes.size()) == expected, "Windows fallback byte order");
        CheckModes(bytes, expected);
        ENC_DET_T const windowsResult = Detect(bytes);
        Check(wcscmp(windowsResult.selectionReason, L"UTF-16(WIN)") == 0, "Windows selection source recorded");
        std::string const utf32 = big ? std::string("\0\0\0A\0\0\0B", 8) : std::string("A\0\0\0B\0\0\0", 8);
        ENC_DET_T const result32 = Detect(utf32);
        Check(result32.bIsUTF32 && result32.bIsReverse == big, "UTF-32 still precedes UTF-16 heuristics");
        Flags.bDevDebugMode = true;
        Detect(utf32);
        Check(wcsstr(wchEncodingInfo, L"U8=valid+NUL") != nullptr, "BOM-less UTF-32 retains completed UTF-8 scan diagnostics");
        Flags.bDevDebugMode = false;
        forcedEncoding = CPI_ANSI_DEFAULT;
        Check(!Detect(utf32, true).bIsUTF32, "forced analysis cannot bypass an explicit encoding via UTF-32");
        std::string const bom32 = big ? std::string("\0\0\xFE\xFF", 4) : std::string("\xFF\xFE\0\0", 4);
        Check(!Detect(bom32 + utf32).bIsUTF32, "explicit encoding overrides UTF-32 BOM import");
        forcedEncoding = CPI_NONE;
    }
    std::string const utf8 = std::string("Hello\0 Unicode \xE2\x82\xAC\r\n", 20);
    forcedEncoding = CPI_UTF8SIGN;
    Check(!Detect(utf8).bIsUTF8Sig, "UTF-8 save signature does not strip unsigned input bytes");
    forcedEncoding = CPI_NONE;
    CheckModes(std::string("\xEF\xBB\xBF") + utf8, CPI_UTF8SIGN);
    for (bool debug : {false, true}) {
        Flags.bDevDebugMode = debug;
        Check(Detect(utf8).Encoding == CPI_UTF8, "UTF-8 with stray NUL preserved");
    }
    Flags.bDevDebugMode = false;
    Settings.UseDefaultForFileEncoding = true;
    Check(Detect(utf8).Encoding == CPI_UTF8, "configured UTF-8 default preserves valid UTF-8 with NULs");
    Settings.UseDefaultForFileEncoding = false;

    {
        std::string const legacyText("\xE0\xE1\xE2");
        legacyEncoding = CPI_TEST_CP1258;
        weakEncoding = CPI_TEST_CP1251;
        ENC_DET_T const hint = Detect(legacyText);
        Check(hint.Encoding == CPI_TEST_CP1251 && hint.analyzedEncoding == CPI_TEST_CP1258 &&
              wcscmp(hint.selectionReason, L"HINT") == 0, "unreliable nonlocale UCD loses to remembered encoding");
        analysisConfidence = 0.60f;
        Check(Detect(legacyText).Encoding == CPI_TEST_CP1258, "reliable UCD beats remembered encoding");
        analysisConfidence = 0.50f;
        Globals.fvCurFile.mask = FV_ENCODING;
        fileVarEncoding = CPI_TEST_CP1258;
        ENC_DET_T const tag = Detect(legacyText, true);
        Check(tag.Encoding == CPI_TEST_CP1258 && wcscmp(tag.selectionReason, L"F-TAG") == 0,
              "file tag survives forced statistical analysis");
        forcedEncoding = CPI_TEST_CP1251;
        Check(Detect(legacyText, true).Encoding == CPI_TEST_CP1251, "explicit override beats file tag");
        forcedEncoding = CPI_NONE;
        Settings.LoadNFOasOEM = true;
        extension = L".nfo";
        ENC_DET_T const nfo = Detect(legacyText, true);
        Check(nfo.Encoding == CPI_OEM && wcscmp(nfo.selectionReason, L"NFO/OEM") == 0, "NFO/OEM preference beats file tag");
        Settings.LoadNFOasOEM = false;
        extension = L".txt";
        Globals.fvCurFile.mask = 0;
        fileVarEncoding = weakEncoding = CPI_NONE;
        legacyEncoding = CPI_ANSI_DEFAULT;
    }
    {
        ENC_DET_T result = INIT_ENC_DET_T;
        result.Encoding = CPI_UTF8;
        result.selectionReason = L"UTF-8(valid)";
        result.bUTF8WithNulls = true;
        result.analysisSkipReason = L"disabled";
        _SetEncodingTitleInfo(&result);
        Check(wcsstr(wchEncodingInfo, L"U8=valid+NUL") != nullptr, "title distinguishes UTF-8 with NULs");
        Check(wcsstr(wchEncodingInfo, L"UCD=skipped(disabled)") != nullptr, "title explains skipped analysis");
        result.bIsUTF32 = true;
        result.bHasBOM = true;
        result.selectionReason = L"BOM(U32LE->U8)";
        _SetEncodingTitleInfo(&result);
        Check(wcsstr(wchEncodingInfo, L"U8=not checked") != nullptr, "UTF-32 fast path does not claim validation");
        result.bIsUTF32 = false;
        result.bHasBOM = false;
        result.bUCDRan = true;
        result.analyzedEncoding = CPI_ANSI_DEFAULT;
        result.Encoding = CPI_ANSI_DEFAULT;
        result.selectionReason = L"UCD(FORCE)";
        result.forcedEncoding = CPI_ANSI_DEFAULT;
        _SetEncodingTitleInfo(&result);
        Check(wcsstr(wchEncodingInfo, L"<- UCD(FORCE)") != nullptr, "forced auto-detection uses UCD abbreviation");
        result.selectionReason = L"UCD";
        result.forcedEncoding = CPI_NONE;
        _SetEncodingTitleInfo(&result);
        Check(wcsstr(wchEncodingInfo, L"ENC=CP1252 <- UCD |") == wchEncodingInfo, "accepted UCHARDET uses UCD abbreviation");
        Check(wcsstr(wchEncodingInfo, L"accepted") == nullptr && wcsstr(wchEncodingInfo, L"rejected") == nullptr &&
              wcsstr(wchEncodingInfo, L"ignored") == nullptr, "title omits redundant outcome labels");
    }
    if (argc > 1) {
        for (const char* endian : {"le", "be"}) {
            std::string path = std::string(argv[1]) + "\\test_utf16_" + endian + "_bom_no_bom.txt";
            std::ifstream file(path, std::ios::binary);
            Check(file.is_open(), path.c_str());
            std::string bytes((std::istreambuf_iterator<char>(file)), {});
            CheckModes(bytes, strcmp(endian, "be") == 0 ? CPI_UNICODEBE : CPI_UNICODE);
            std::printf("Fixture passed: %s\n", path.c_str());
        }
    }
    std::puts("Encoding detection regression tests passed.");
}
