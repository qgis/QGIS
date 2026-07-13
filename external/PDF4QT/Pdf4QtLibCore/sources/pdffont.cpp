// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdffont.h"
#include "pdfdocument.h"
#include "pdfparser.h"
#include "pdfnametounicode.h"
#include "pdfexception.h"
#include "pdfutils.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/fterrors.h>
#include <freetype/ftoutln.h>
#include <freetype/t1tables.h>

#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QMutex>
#include <QReadWriteLock>
#include <QPainterPath>
#include <QDataStream>

#include <limits>

#include "pdfdbgheap.h"

#if defined(Q_OS_WIN)
#include "Windows.h"
#include <dwrite.h>
#elif defined(Q_OS_UNIX)
#include <fontconfig/fontconfig.h>
#endif

#if defined(PDF4QT_USE_PRAGMA_LIB)
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "User32")
#endif

namespace pdf
{

enum class ECjkDefaultFontType
{
    Invalid,
    AdobeGB,
    AdobeCNS,
    AdobeJapan,
    AdobeKorea
};

struct PDF_Default_CJK_Font
{
    ECjkDefaultFontType type = ECjkDefaultFontType::Invalid;
    bool isSerif = false;
    const char* name = nullptr;
};

static constexpr std::array S_DEFAULT_CJK_FONTS =
{
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "KaiTi_GB2312" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "Song" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, false, "Heiti" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "SimFang" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "FangSong" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, false, "SimHei" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "SimSun" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "SimKai" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "KaiTi" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "SimLi" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "SimLiU" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "STSong-Light" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeGB, true, "STSong-Light,Bold" },

    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeCNS, true, "Ming" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeCNS, false, "Fangti" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeCNS, true, "MingLiU" },

    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeJapan, false, "Gothic" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeJapan, true, "Mincho" },

    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeKorea, false, "Gulim" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeKorea, false, "Dotum" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeKorea, true, "Batang" },
};

struct PDF_Font_Replacement
{
    const char* origFont;
    const char* replaceFont;
};

static constexpr std::array S_FONT_REPLACEMENTS
{
    PDF_Font_Replacement{"Futura", "Calibri"},
    PDF_Font_Replacement{"Utopia-Bold", "Georgia"},
    PDF_Font_Replacement{"Utopia-BoldItalic", "Georgia"},
    PDF_Font_Replacement{"Utopia-Italic", "Georgia"},
    PDF_Font_Replacement{"Utopia-Semibold", "Georgia"},
    PDF_Font_Replacement{"Utopia-SemiboldItalic", "Georgia"},
    PDF_Font_Replacement{"Utopia", "Georgia"}
};

static bool isMicrosoftSymbolCharmap(FT_CharMap charMap)
{
    return charMap && charMap->platform_id == 3 && charMap->encoding_id == 0;
}

static bool isAppleRomanCharmap(FT_CharMap charMap)
{
    return charMap && charMap->platform_id == 1 && charMap->encoding_id == 0;
}

static bool isUnicodeCharmap(FT_CharMap charMap)
{
    return charMap && charMap->encoding == FT_ENCODING_UNICODE;
}

template<typename Predicate>
static bool hasCharmap(FT_Face face, Predicate predicate)
{
    for (FT_Int i = 0; i < face->num_charmaps; ++i)
    {
        if (predicate(face->charmaps[i]))
        {
            return true;
        }
    }

    return false;
}

template<typename Predicate>
static FT_CharMap selectCharmap(FT_Face face, Predicate predicate)
{
    for (FT_Int i = 0; i < face->num_charmaps; ++i)
    {
        FT_CharMap charMap = face->charmaps[i];
        if (predicate(charMap) && !FT_Set_Charmap(face, charMap))
        {
            return charMap;
        }
    }

    return nullptr;
}

static bool shouldUseSymbolicTrueTypeCMap(const FontDescriptor& fontDescriptor, FT_Face face)
{
    return fontDescriptor.isSymbolic() ||
           (!fontDescriptor.isNonSymbolic() && hasCharmap(face, isMicrosoftSymbolCharmap) && !hasCharmap(face, isUnicodeCharmap));
}

static FT_CharMap selectSymbolicTrueTypeCMap(FT_Face face)
{
    if (FT_CharMap charMap = selectCharmap(face, isMicrosoftSymbolCharmap))
    {
        return charMap;
    }

    if (FT_CharMap charMap = selectCharmap(face, isAppleRomanCharmap))
    {
        return charMap;
    }

    return selectCharmap(face, [](FT_CharMap charMap) { return !isUnicodeCharmap(charMap); });
}

static FT_UInt getSymbolicTrueTypeGlyphIndex(FT_Face face, FT_CharMap charMap, FT_ULong characterCode)
{
    FT_UInt glyphIndex = FT_Get_Char_Index(face, characterCode);

    if (isMicrosoftSymbolCharmap(charMap) && !glyphIndex && characterCode <= 0xFF)
    {
        glyphIndex = FT_Get_Char_Index(face, characterCode + 0xF000);
    }

    if (isMicrosoftSymbolCharmap(charMap) && !glyphIndex && characterCode <= 0xFF)
    {
        glyphIndex = FT_Get_Char_Index(face, characterCode + 0xF100);
    }

    return glyphIndex;
}

struct SystemFontData
{
    QByteArray data;
    FT_Long faceIndex = 0;

    bool isEmpty() const { return data.isEmpty(); }
};

#if defined(Q_OS_WIN)
template<typename T>
static void releaseComObject(T*& object)
{
    if (object)
    {
        object->Release();
        object = nullptr;
    }
}

static DWRITE_FONT_STRETCH getDirectWriteFontStretch(QFont::Stretch stretch)
{
    switch (stretch)
    {
        case QFont::UltraCondensed:
            return DWRITE_FONT_STRETCH_ULTRA_CONDENSED;
        case QFont::ExtraCondensed:
            return DWRITE_FONT_STRETCH_EXTRA_CONDENSED;
        case QFont::Condensed:
            return DWRITE_FONT_STRETCH_CONDENSED;
        case QFont::SemiCondensed:
            return DWRITE_FONT_STRETCH_SEMI_CONDENSED;
        case QFont::SemiExpanded:
            return DWRITE_FONT_STRETCH_SEMI_EXPANDED;
        case QFont::Expanded:
            return DWRITE_FONT_STRETCH_EXPANDED;
        case QFont::ExtraExpanded:
            return DWRITE_FONT_STRETCH_EXTRA_EXPANDED;
        case QFont::UltraExpanded:
            return DWRITE_FONT_STRETCH_ULTRA_EXPANDED;
        case QFont::Unstretched:
        case QFont::AnyStretch:
        default:
            return DWRITE_FONT_STRETCH_NORMAL;
    }
}

static bool matchesDirectWriteFontName(const QString& fontName, const QString& candidate)
{
    QString adjustedCandidate = candidate;
    for (const char* string : { "PS", "MT", "Regular", "Bold", "Italic", "Oblique" })
    {
        adjustedCandidate.remove(QLatin1String(string), Qt::CaseInsensitive);
    }
    adjustedCandidate = adjustedCandidate.remove(QChar(' ')).remove(QChar('-')).remove(QChar(',')).trimmed();

    return candidate.compare(fontName, Qt::CaseInsensitive) == 0 ||
           adjustedCandidate.compare(fontName, Qt::CaseInsensitive) == 0;
}
#endif

/// Storage class for system fonts
class PDFSystemFontInfoStorage
{
public:

    /// Returns instance of storage
    static const PDFSystemFontInfoStorage* getInstance();

    /// Loads font from descriptor
    /// \param descriptor Descriptor describing the font
    SystemFontData loadFont(const CIDSystemInfo* cidSystemInfo,
                            const FontDescriptor* descriptor,
                            StandardFontType standardFontType,
                            PDFRenderErrorReporter* reporter) const;

private:
    explicit PDFSystemFontInfoStorage();

    /// Loads font from descriptor
    /// \param descriptor Descriptor describing the font
    SystemFontData loadFontImpl(const FontDescriptor* descriptor,
                                QString fontName,
                                StandardFontType standardFontType,
                                PDFRenderErrorReporter* reporter) const;

#ifdef Q_OS_UNIX
    static void checkFontConfigError(FcBool result);
#endif

    /// Create a postscript name for comparation purposes
    static QString getFontPostscriptName(QString fontName);

#ifdef Q_OS_WIN
    /// Callback for enumerating fonts
    static int CALLBACK enumerateFontProc(const LOGFONT* font, const TEXTMETRIC* textMetrics, DWORD fontType, LPARAM lParam);

    /// Retrieves font data for desired font
    static QByteArray getFontData(const LOGFONT* font, HDC hdc);

    /// Retrieves font data for desired font using DirectWrite
    static SystemFontData getDirectWriteFontData(const FontDescriptor* descriptor, const QString& fontName);

    struct FontInfo
    {
        QString faceName;
        QString faceNameAdjusted;
        LOGFONT logFont;
        TEXTMETRIC textMetric;
    };

    struct CallbackInfo
    {
        PDFSystemFontInfoStorage* storage = nullptr;
        HDC hdc = nullptr;
        std::set<QString> usedFonts;
    };

    std::vector<FontInfo> m_fontInfos;
#endif
};

const PDFSystemFontInfoStorage* PDFSystemFontInfoStorage::getInstance()
{
    static PDFSystemFontInfoStorage instance;
    return &instance;
}

SystemFontData PDFSystemFontInfoStorage::loadFont(const CIDSystemInfo* cidSystemInfo,
                                                  const FontDescriptor* descriptor,
                                                  StandardFontType standardFontType,
                                                  PDFRenderErrorReporter* reporter) const
{
    QString fontName;
    QString standardFontSubstituteFileName;

    // Standard font substitute
    switch (standardFontType)
    {
        case StandardFontType::TimesRoman:
            standardFontSubstituteFileName = "LiberationSerif-Regular.ttf";
            break;

        case StandardFontType::TimesRomanBold:
            standardFontSubstituteFileName = "LiberationSerif-Bold.ttf";
            break;

        case StandardFontType::TimesRomanItalics:
            standardFontSubstituteFileName = "LiberationSerif-Italic.ttf";
            break;

        case StandardFontType::TimesRomanBoldItalics:
            standardFontSubstituteFileName = "LiberationSerif-BoldItalic.ttf";
            break;

        case StandardFontType::Helvetica:
            standardFontSubstituteFileName = "LiberationSans-Regular.ttf";
            break;

        case StandardFontType::HelveticaBold:
            standardFontSubstituteFileName = "LiberationSans-Bold.ttf";
            break;

        case StandardFontType::HelveticaOblique:
            standardFontSubstituteFileName = "LiberationSans-Italic.ttf";
            break;

        case StandardFontType::HelveticaBoldOblique:
            standardFontSubstituteFileName = "LiberationSans-BoldItalic.ttf";
            break;

        case StandardFontType::Courier:
            standardFontSubstituteFileName = "LiberationMono-Regular.ttf";
            break;

        case StandardFontType::CourierBold:
            standardFontSubstituteFileName = "LiberationMono-Bold.ttf";
            break;

        case StandardFontType::CourierOblique:
            standardFontSubstituteFileName = "LiberationMono-Italic.ttf";
            break;

        case StandardFontType::CourierBoldOblique:
            standardFontSubstituteFileName = "LiberationMono-BoldItalic.ttf";
            break;

        case StandardFontType::Symbol:
        case StandardFontType::ZapfDingbats:
            break;

        default:
            break;
    }

    if (!standardFontSubstituteFileName.isEmpty())
    {
        QFile file(QString(":/fonts/liberation-fonts-ttf/%1").arg(standardFontSubstituteFileName));
        if (file.open(QFile::ReadOnly))
        {
            QByteArray data = file.readAll();
            file.close();

            if (!data.isEmpty())
            {
                return SystemFontData{ data, 0 };
            }
        }
    }

    // Exact match font face name
    switch (standardFontType)
    {
        case StandardFontType::TimesRoman:
        case StandardFontType::TimesRomanBold:
        case StandardFontType::TimesRomanItalics:
        case StandardFontType::TimesRomanBoldItalics:
        {
            fontName = "TimesNewRoman";
            break;
        }

        case StandardFontType::Helvetica:
        case StandardFontType::HelveticaBold:
        case StandardFontType::HelveticaOblique:
        case StandardFontType::HelveticaBoldOblique:
        {
            fontName = "Arial";
            break;
        }

        case StandardFontType::Courier:
        case StandardFontType::CourierBold:
        case StandardFontType::CourierOblique:
        case StandardFontType::CourierBoldOblique:
        {
            fontName = "CourierNew";
            break;
        }

        case StandardFontType::Symbol:
        case StandardFontType::ZapfDingbats:
        {
            fontName = "Symbol";
            break;
        }

        default:
        {
            fontName = getFontPostscriptName(descriptor->fontName);
            break;
        }
    }

    SystemFontData fontData = loadFontImpl(descriptor, fontName, standardFontType, reporter);

    if (fontData.isEmpty() && cidSystemInfo->registry == "Adobe")
    {
        // Try to load CJK font
        ECjkDefaultFontType cjkDefaultFontType = ECjkDefaultFontType::Invalid;

        if (cidSystemInfo->ordering == "GB1")
        {
            cjkDefaultFontType = ECjkDefaultFontType::AdobeGB;
        }
        else if (cidSystemInfo->ordering == "CNS1")
        {
            cjkDefaultFontType = ECjkDefaultFontType::AdobeCNS;
        }
        else if (cidSystemInfo->ordering == "Japan1")
        {
            cjkDefaultFontType = ECjkDefaultFontType::AdobeJapan;
        }
        else if (cidSystemInfo->ordering == "Korea1")
        {
            cjkDefaultFontType = ECjkDefaultFontType::AdobeKorea;
        }

        if (cjkDefaultFontType != ECjkDefaultFontType::Invalid)
        {
            for (const PDF_Default_CJK_Font& defaultCjkFont : S_DEFAULT_CJK_FONTS)
            {
                if (defaultCjkFont.type == cjkDefaultFontType &&
                    defaultCjkFont.isSerif == descriptor->isSerif())
                {
                    fontData = loadFontImpl(descriptor, defaultCjkFont.name, StandardFontType::Invalid, reporter);

                    if (!fontData.isEmpty())
                    {
                        return fontData;
                    }
                }
            }
        }
    }

    if (fontData.isEmpty())
    {
        for (const PDF_Font_Replacement& fontReplacement : S_FONT_REPLACEMENTS)
        {
            if (fontName.contains(QLatin1String(fontReplacement.origFont)))
            {
                fontData = loadFontImpl(descriptor, QString(fontReplacement.replaceFont), StandardFontType::Invalid, reporter);

                if (!fontData.isEmpty())
                {
                    return fontData;
                }
            }
        }
    }

    return fontData;
}

SystemFontData PDFSystemFontInfoStorage::loadFontImpl(const FontDescriptor* descriptor,
                                                      QString fontName,
                                                      StandardFontType standardFontType,
                                                      PDFRenderErrorReporter* reporter) const
{
    SystemFontData result;

#if defined(Q_OS_WIN)

    Q_UNUSED(standardFontType);
    result = getDirectWriteFontData(descriptor, fontName);
    if (!result.isEmpty())
    {
        return result;
    }

    HDC hdc = GetDC(NULL);
    const BYTE lfItalic = (descriptor->italicAngle != 0.0 ? TRUE : FALSE);
    if (!fontName.isEmpty())
    {
        for (const FontInfo& fontInfo : m_fontInfos)
        {
            if (fontInfo.faceNameAdjusted == fontName &&
                fontInfo.logFont.lfWeight == descriptor->fontWeight &&
                fontInfo.logFont.lfItalic == lfItalic)
            {
                result = SystemFontData{ getFontData(&fontInfo.logFont, hdc), 0 };

                if (!result.isEmpty())
                {
                    break;
                }
            }
        }

        // Match for font family
        if (result.isEmpty())
        {
            for (const FontInfo& fontInfo : m_fontInfos)
            {
                if (fontInfo.faceNameAdjusted == fontName)
                {
                    LOGFONT logFont = fontInfo.logFont;
                    logFont.lfWeight = descriptor->fontWeight;
                    logFont.lfItalic = lfItalic;
                    result = SystemFontData{ getFontData(&logFont, hdc), 0 };

                    if (!result.isEmpty())
                    {
                        break;
                    }
                }
            }
        }
    }

    // Exact match for font, if font can't be exact matched, then match font family
    // and try to set weight
    QString fontFamily = QString::fromLatin1(descriptor->fontFamily);

    if (!fontFamily.isEmpty() && result.isEmpty())
    {
        for (const FontInfo& fontInfo : m_fontInfos)
        {
            if (fontInfo.faceName.contains(fontFamily) &&
                fontInfo.logFont.lfWeight == descriptor->fontWeight &&
                fontInfo.logFont.lfItalic == lfItalic)
            {
                result = SystemFontData{ getFontData(&fontInfo.logFont, hdc), 0 };

                if (!result.isEmpty())
                {
                    reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Inexact font substitution: font %1 replaced by %2 using font family %3.").arg(fontName, fontInfo.faceNameAdjusted, fontFamily));
                    break;
                }
            }
        }

        // Match for font family
        if (result.isEmpty())
        {
            for (const FontInfo& fontInfo : m_fontInfos)
            {
                if (fontInfo.faceName.contains(fontFamily))
                {
                    LOGFONT logFont = fontInfo.logFont;
                    logFont.lfWeight = descriptor->fontWeight;
                    logFont.lfItalic = lfItalic;
                    result = SystemFontData{ getFontData(&logFont, hdc), 0 };

                    if (!result.isEmpty())
                    {
                        reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Inexact font substitution: font %1 replaced by %2 using font family %3.").arg(fontName, fontInfo.faceNameAdjusted, fontFamily));
                        break;
                    }
                }
            }
        }
    }

    // Try to inexact match for font name - find similar font
    if (!fontName.isEmpty() && result.isEmpty())
    {
        for (const FontInfo& fontInfo : m_fontInfos)
        {
            if (fontInfo.faceNameAdjusted.contains(fontName))
            {
                LOGFONT logFont = fontInfo.logFont;
                logFont.lfWeight = descriptor->fontWeight;
                logFont.lfItalic = lfItalic;
                result = SystemFontData{ getFontData(&logFont, hdc), 0 };

                if (!result.isEmpty())
                {
                    reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Inexact font substitution: font %1 replaced by %2.").arg(fontName, fontInfo.faceNameAdjusted));
                    break;
                }
            }
        }
    }

    ReleaseDC(NULL, hdc);
    return result;
#elif defined(Q_OS_UNIX)
    FcPattern* p = FcPatternBuild(nullptr, FC_FAMILY, FcTypeString, fontName.constData(), nullptr);
    if (!p)
    {
        throw PDFException(PDFTranslationContext::tr("FontConfig error building pattern for font %1").arg(fontName));
    }

    constexpr const std::array<std::pair<PDFReal, int>, 9> weights{
            std::pair<PDFReal, int>{100, FC_WEIGHT_EXTRALIGHT},
            std::pair<PDFReal, int>{200, FC_WEIGHT_LIGHT},
            std::pair<PDFReal, int>{300, FC_WEIGHT_BOOK},
            std::pair<PDFReal, int>{400, FC_WEIGHT_NORMAL},
            std::pair<PDFReal, int>{500, FC_WEIGHT_MEDIUM},
            std::pair<PDFReal, int>{600, FC_WEIGHT_DEMIBOLD},
            std::pair<PDFReal, int>{700, FC_WEIGHT_BOLD},
            std::pair<PDFReal, int>{800, FC_WEIGHT_EXTRABOLD},
            std::pair<PDFReal, int>{900, FC_WEIGHT_EXTRABOLD}};
    auto wit = std::lower_bound(weights.cbegin(), weights.cend(), descriptor->fontWeight, [](const std::pair<PDFReal, int>& data, PDFReal key) { return data.first < key; });
    if (wit != weights.cend())
    {
        checkFontConfigError(FcPatternAddInteger(p, FC_WEIGHT, wit->second));
    }

    constexpr const std::array<std::pair<QFont::Stretch, int>, 9> stretches{
        std::pair<QFont::Stretch, int>{QFont::UltraCondensed, FC_WIDTH_ULTRACONDENSED},
        std::pair<QFont::Stretch, int>{QFont::ExtraCondensed, FC_WIDTH_EXTRACONDENSED},
        std::pair<QFont::Stretch, int>{QFont::Condensed, FC_WIDTH_CONDENSED},
        std::pair<QFont::Stretch, int>{QFont::SemiCondensed, FC_WIDTH_SEMICONDENSED},
        std::pair<QFont::Stretch, int>{QFont::Unstretched, FC_WIDTH_NORMAL},
        std::pair<QFont::Stretch, int>{QFont::SemiExpanded, FC_WIDTH_SEMIEXPANDED},
        std::pair<QFont::Stretch, int>{QFont::Expanded, FC_WIDTH_EXPANDED},
        std::pair<QFont::Stretch, int>{QFont::ExtraExpanded, FC_WIDTH_EXTRAEXPANDED},
        std::pair<QFont::Stretch, int>{QFont::UltraExpanded, FC_WIDTH_ULTRAEXPANDED}};

    auto sit = std::find_if(stretches.cbegin(), stretches.cend(), [&](const std::pair<QFont::Stretch, int>& item) { return item.first == descriptor->fontStretch; });
    if (sit != stretches.cend())
    {
        checkFontConfigError(FcPatternAddInteger(p, FC_WIDTH, sit->second));
    }

    checkFontConfigError(FcConfigSubstitute(nullptr, p, FcMatchPattern));
    FcDefaultSubstitute(p);
    FcResult res = FcResultNoMatch;
    FcPattern* match = FcFontMatch(nullptr, p, &res);
    if (match)
    {
        FcChar8* s = nullptr;
        if (FcPatternGetString(match, FC_FILE, 0, &s) == FcResultMatch)
        {
            QFile f(QString::fromUtf8(reinterpret_cast<char*>(s)));
            if ( f.open(QIODevice::ReadOnly) )
            {
                result.data = f.readAll();
                f.close();
            }
        }
    }

    if (result.isEmpty() && standardFontType == StandardFontType::Invalid)
    {
        reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Inexact font substitution: font %1 replaced by standard font Times New Roman.").arg(fontName));
        result = loadFontImpl(descriptor, fontName, StandardFontType::TimesRoman, reporter);
    }

    return result;
#endif
}

PDFSystemFontInfoStorage::PDFSystemFontInfoStorage()
{
#ifdef Q_OS_WIN
    LOGFONT logfont;
    std::memset(&logfont, 0, sizeof(logfont));
    logfont.lfCharSet = DEFAULT_CHARSET;
    logfont.lfFaceName[0] = 0;
    logfont.lfPitchAndFamily = 0;

    HDC hdc = GetDC(NULL);

    CallbackInfo callbackInfo{ this, hdc, std::set<QString>() };
    EnumFontFamiliesEx(hdc, &logfont, &PDFSystemFontInfoStorage::enumerateFontProc, reinterpret_cast<LPARAM>(&callbackInfo), 0);

    ReleaseDC(NULL, hdc);
#endif
}

#ifdef Q_OS_WIN
int PDFSystemFontInfoStorage::enumerateFontProc(const LOGFONT* font, const TEXTMETRIC* textMetrics, DWORD fontType, LPARAM lParam)
{
    if ((fontType & TRUETYPE_FONTTYPE))
    {
        CallbackInfo* callbackInfo = reinterpret_cast<CallbackInfo*>(lParam);

        FontInfo fontInfo;
        fontInfo.logFont = *font;
        fontInfo.textMetric = *textMetrics;
        fontInfo.faceName = QString::fromWCharArray(font->lfFaceName);
        fontInfo.faceNameAdjusted = getFontPostscriptName(fontInfo.faceName);

        if (callbackInfo->usedFonts.count(fontInfo.faceName))
        {
            return TRUE;
        }
        else
        {
            callbackInfo->usedFonts.insert(fontInfo.faceName);
        }

        callbackInfo->storage->m_fontInfos.push_back(qMove(fontInfo));

        // For debug purposes only!
#if 0
        QByteArray byteArray = getFontData(font, callbackInfo->hdc);
        qDebug() << "Font: " << QString::fromWCharArray(font->lfFaceName) << ", italic = " << font->lfItalic << ", weight = " << font->lfWeight << ", data size = " << byteArray.size();
#endif
    }

    return TRUE;
}

SystemFontData PDFSystemFontInfoStorage::getDirectWriteFontData(const FontDescriptor* descriptor, const QString& fontName)
{
    SystemFontData result;

    const QString descriptorFontFamily = QString::fromLatin1(descriptor->fontFamily);
    if (fontName.isEmpty() && descriptorFontFamily.isEmpty())
    {
        return result;
    }

    using DWriteCreateFactoryFunction = HRESULT(WINAPI*)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);

    HMODULE dwriteModule = ::LoadLibraryW(L"dwrite.dll");
    if (!dwriteModule)
    {
        return result;
    }

    DWriteCreateFactoryFunction createFactory = reinterpret_cast<DWriteCreateFactoryFunction>(::GetProcAddress(dwriteModule, "DWriteCreateFactory"));
    if (!createFactory)
    {
        ::FreeLibrary(dwriteModule);
        return result;
    }

    IDWriteFactory* factory = nullptr;
    IDWriteFontCollection* fontCollection = nullptr;
    IDWriteFontFamily* matchedFamily = nullptr;
    IDWriteFont* matchedFont = nullptr;
    IDWriteFontFace* fontFace = nullptr;
    IDWriteFontFile* fontFile = nullptr;
    IDWriteFontFileLoader* fontFileLoader = nullptr;
    IDWriteLocalFontFileLoader* localFontFileLoader = nullptr;

    auto cleanup = [&]()
    {
        releaseComObject(localFontFileLoader);
        releaseComObject(fontFileLoader);
        releaseComObject(fontFile);
        releaseComObject(fontFace);
        releaseComObject(matchedFont);
        releaseComObject(matchedFamily);
        releaseComObject(fontCollection);
        releaseComObject(factory);
        ::FreeLibrary(dwriteModule);
    };

    HRESULT hr = createFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory));
    if (FAILED(hr) || !factory)
    {
        cleanup();
        return result;
    }

    hr = factory->GetSystemFontCollection(&fontCollection, FALSE);
    if (FAILED(hr) || !fontCollection)
    {
        cleanup();
        return result;
    }

    const UINT32 familyCount = fontCollection->GetFontFamilyCount();
    for (UINT32 familyIndex = 0; familyIndex < familyCount && !matchedFamily; ++familyIndex)
    {
        IDWriteFontFamily* family = nullptr;
        IDWriteLocalizedStrings* familyNames = nullptr;

        if (SUCCEEDED(fontCollection->GetFontFamily(familyIndex, &family)) &&
            family &&
            SUCCEEDED(family->GetFamilyNames(&familyNames)) &&
            familyNames)
        {
            const UINT32 nameCount = familyNames->GetCount();
            for (UINT32 nameIndex = 0; nameIndex < nameCount; ++nameIndex)
            {
                UINT32 nameLength = 0;
                if (FAILED(familyNames->GetStringLength(nameIndex, &nameLength)))
                {
                    continue;
                }

                std::vector<wchar_t> nameBuffer(nameLength + 1, wchar_t());
                if (SUCCEEDED(familyNames->GetString(nameIndex, nameBuffer.data(), static_cast<UINT32>(nameBuffer.size()))))
                {
                    const QString candidateName = QString::fromWCharArray(nameBuffer.data());
                    if (matchesDirectWriteFontName(fontName, candidateName) ||
                        (!descriptorFontFamily.isEmpty() && matchesDirectWriteFontName(descriptorFontFamily, candidateName)))
                    {
                        matchedFamily = family;
                        family = nullptr;
                        break;
                    }
                }
            }
        }

        releaseComObject(familyNames);
        releaseComObject(family);
    }

    if (!matchedFamily)
    {
        cleanup();
        return result;
    }

    const int requestedWeight = qBound(1, qRound(descriptor->fontWeight), 999);
    const DWRITE_FONT_WEIGHT fontWeight = static_cast<DWRITE_FONT_WEIGHT>(requestedWeight);
    const DWRITE_FONT_STRETCH fontStretch = getDirectWriteFontStretch(descriptor->fontStretch);
    const DWRITE_FONT_STYLE fontStyle = descriptor->italicAngle != 0.0 ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

    hr = matchedFamily->GetFirstMatchingFont(fontWeight, fontStretch, fontStyle, &matchedFont);
    if (FAILED(hr) || !matchedFont)
    {
        cleanup();
        return result;
    }

    hr = matchedFont->CreateFontFace(&fontFace);
    if (FAILED(hr) || !fontFace)
    {
        cleanup();
        return result;
    }

    UINT32 fileCount = 0;
    hr = fontFace->GetFiles(&fileCount, nullptr);
    if (FAILED(hr) || fileCount == 0)
    {
        cleanup();
        return result;
    }

    std::vector<IDWriteFontFile*> fontFiles(fileCount, nullptr);
    hr = fontFace->GetFiles(&fileCount, fontFiles.data());
    if (FAILED(hr) || fileCount == 0)
    {
        for (IDWriteFontFile* file : fontFiles)
        {
            if (file)
            {
                file->Release();
            }
        }
        cleanup();
        return result;
    }

    fontFile = fontFiles.front();
    fontFiles.front() = nullptr;
    for (IDWriteFontFile* file : fontFiles)
    {
        if (file)
        {
            file->Release();
        }
    }

    const void* referenceKey = nullptr;
    UINT32 referenceKeySize = 0;
    hr = fontFile->GetReferenceKey(&referenceKey, &referenceKeySize);
    if (FAILED(hr))
    {
        cleanup();
        return result;
    }

    hr = fontFile->GetLoader(&fontFileLoader);
    if (FAILED(hr) || !fontFileLoader)
    {
        cleanup();
        return result;
    }

    hr = fontFileLoader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), reinterpret_cast<void**>(&localFontFileLoader));
    if (FAILED(hr) || !localFontFileLoader)
    {
        cleanup();
        return result;
    }

    UINT32 pathLength = 0;
    hr = localFontFileLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &pathLength);
    if (FAILED(hr))
    {
        cleanup();
        return result;
    }

    std::vector<wchar_t> pathBuffer(pathLength + 1, wchar_t());
    hr = localFontFileLoader->GetFilePathFromKey(referenceKey, referenceKeySize, pathBuffer.data(), static_cast<UINT32>(pathBuffer.size()));
    if (FAILED(hr))
    {
        cleanup();
        return result;
    }

    QFile file(QString::fromWCharArray(pathBuffer.data()));
    if (file.open(QFile::ReadOnly))
    {
        result.data = file.readAll();
        result.faceIndex = static_cast<FT_Long>(fontFace->GetIndex());
        file.close();
    }

    cleanup();
    return result;
}

QByteArray PDFSystemFontInfoStorage::getFontData(const LOGFONT* font, HDC hdc)
{
    QByteArray byteArray;

    if (HFONT fontHandle = ::CreateFontIndirect(font))
    {
        HGDIOBJ oldFont = ::SelectObject(hdc, fontHandle);

        DWORD size = ::GetFontData(hdc, 0, 0, nullptr, 0);
        if (size != GDI_ERROR && size <= DWORD((std::numeric_limits<int>::max)()))
        {
            byteArray.resize(static_cast<int>(size));

            const DWORD readSize = ::GetFontData(hdc, 0, 0, byteArray.data(), size);
            if (readSize == GDI_ERROR || readSize != size)
            {
                byteArray.clear();
            }
        }

        ::SelectObject(hdc, oldFont);
        ::DeleteObject(fontHandle);
    }

    return byteArray;
}
#endif

#ifdef Q_OS_UNIX
void PDFSystemFontInfoStorage::checkFontConfigError(FcBool result)
{
    if (!result)
    {
        throw PDFException(PDFTranslationContext::tr("Fontconfig error"));
    }
}
#endif

QString PDFSystemFontInfoStorage::getFontPostscriptName(QString fontName)
{
    for (const char* string : { "PS", "MT", "Regular", "Bold", "Italic", "Oblique" })
    {
        fontName.remove(QLatin1String(string), Qt::CaseInsensitive);
    }

    return fontName.remove(QChar(' ')).remove(QChar('-')).remove(QChar(',')).trimmed();
}

PDFFont::PDFFont(CIDSystemInfo CIDSystemInfo, QByteArray fontId, FontDescriptor fontDescriptor) :
    m_CIDSystemInfo(qMove(CIDSystemInfo)),
    m_fontDescriptor(qMove(fontDescriptor)),
    m_fontId(qMove(fontId))
{

}

class IRealizedFontImpl
{
public:
    explicit IRealizedFontImpl() = default;
    virtual ~IRealizedFontImpl() = default;

    /// Fills the text sequence by interpreting byte array according font data and
    /// produces glyphs for the font.
    /// \param byteArray Array of bytes to be interpreted
    /// \param textSequence Text sequence to be filled
    virtual void fillTextSequence(const QByteArray& byteArray, TextSequence& textSequence, PDFRenderErrorReporter* reporter) = 0;

    /// Returns true, if font has horizontal writing system
    virtual bool isHorizontalWritingSystem() const = 0;

    /// Dumps information about the font
    virtual void dumpFontToTreeItem(ITreeFactory* treeFactory) const { Q_UNUSED(treeFactory); }

    /// Returns postscript name of the font
    virtual QString getPostScriptName() const { return QString(); }

    /// Returns character info
    virtual CharacterInfos getCharacterInfos() const = 0;
};

/// Implementation of the PDFRealizedFont class using PIMPL pattern for Type 3 fonts
class PDFRealizedType3FontImpl : public IRealizedFontImpl
{
public:
    explicit PDFRealizedType3FontImpl(PDFFontPointer parentFont, PDFReal pixelSize) : m_pixelSize(pixelSize), m_parentFont(parentFont) { }
    virtual ~PDFRealizedType3FontImpl() override = default;

    PDFReal getPixelSize() const { return m_pixelSize; }

    virtual void fillTextSequence(const QByteArray& byteArray, TextSequence& textSequence, PDFRenderErrorReporter* reporter) override;
    virtual bool isHorizontalWritingSystem() const override;
    virtual CharacterInfos getCharacterInfos() const override;

private:
    /// Pixel size of the font
    PDFReal m_pixelSize = 0.0;

    /// Parent font
    PDFFontPointer m_parentFont;
};

/// Implementation of the PDFRealizedFont class using PIMPL pattern
class PDFRealizedFontImpl : public IRealizedFontImpl
{
public:
    explicit PDFRealizedFontImpl();
    virtual ~PDFRealizedFontImpl();

    virtual void fillTextSequence(const QByteArray& byteArray, TextSequence& textSequence, PDFRenderErrorReporter* reporter) override;
    virtual bool isHorizontalWritingSystem() const override { return !m_isVertical; }
    virtual void dumpFontToTreeItem(ITreeFactory* treeFactory) const override;
    virtual QString getPostScriptName() const override { return m_postScriptName; }
    virtual CharacterInfos getCharacterInfos() const override;

    static constexpr const PDFReal PIXEL_SIZE_MULTIPLIER = 100.0;

private:
    friend class PDFRealizedFont;

    static constexpr const PDFReal FONT_WIDTH_MULTIPLIER = 1.0 / 1000.0;
    static constexpr const PDFReal FORMAT_26_6_MULTIPLIER = 1 / 64.0;
    static constexpr const PDFReal FONT_MULTIPLIER = FORMAT_26_6_MULTIPLIER / PIXEL_SIZE_MULTIPLIER;

    struct Glyph
    {
        QPainterPath glyph;
        PDFReal advance = 0.0;
    };

    static int outlineMoveTo(const FT_Vector* to, void* user);
    static int outlineLineTo(const FT_Vector* to, void* user);
    static int outlineConicTo(const FT_Vector* control, const FT_Vector* to, void* user);
    static int outlineCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user);

    /// Get glyph for glyph index
    const Glyph& getGlyph(unsigned int glyphIndex);

    /// Returns true if glyph index can be rendered. Glyph index 0 is usually
    /// .notdef, but some embedded PDF fonts use it as a real glyph.
    bool canRenderGlyphIndex(GID glyphIndex, QChar character) const;

    /// Function checks, if error occured, and if yes, then exception is thrown
    static void checkFreeTypeError(FT_Error error);

    /// Read/write lock for accessing the glyph data
    QReadWriteLock m_readWriteLock;

    /// Glyph cache, must be protected by the mutex above
    std::unordered_map<unsigned int, Glyph> m_glyphCache;

    /// For embedded fonts, this byte array contains embedded font data
    QByteArray m_embeddedFontData;

    /// For system fonts, this byte array contains system font data
    QByteArray m_systemFontData;

    /// Face index for system font collections
    FT_Long m_systemFontFaceIndex;

    /// Instance of FreeType library assigned to this font
    FT_Library m_library;

    /// Face of the font
    FT_Face m_face;

    /// Pixel size of the font
    PDFReal m_pixelSize;

    /// Parent font
    PDFFontPointer m_parentFont;

    /// True, if font is embedded
    bool m_isEmbedded;

    /// True, if font has vertical writing system
    bool m_isVertical;

    /// Postscript name of the font
    QString m_postScriptName;
};

PDFRealizedFontImpl::PDFRealizedFontImpl() :
    m_library(nullptr),
    m_face(nullptr),
    m_systemFontFaceIndex(0),
    m_pixelSize(0.0),
    m_parentFont(nullptr),
    m_isEmbedded(false),
    m_isVertical(false)
{

}

PDFRealizedFontImpl::~PDFRealizedFontImpl()
{
    if (m_face)
    {
        FT_Done_Face(m_face);
        m_face = nullptr;
    }

    if (m_library)
    {
        FT_Done_FreeType(m_library);
        m_library = nullptr;
    }
}

void PDFRealizedFontImpl::fillTextSequence(const QByteArray& byteArray, TextSequence& textSequence, PDFRenderErrorReporter* reporter)
{
    switch (m_parentFont->getFontType())
    {
        case FontType::Type1:
        case FontType::TrueType:
        case FontType::MMType1:
        {
            // We can use encoding
            Q_ASSERT(dynamic_cast<PDFSimpleFont*>(m_parentFont.get()));
            const PDFSimpleFont* font = static_cast<PDFSimpleFont*>(m_parentFont.get());
            const GlyphIndices* glyphIndices = font->getGlyphIndices();

            textSequence.items.reserve(textSequence.items.size() + byteArray.size());
            for (int i = 0, count = byteArray.size(); i < count; ++i)
            {
                const CID cid = static_cast<uint8_t>(byteArray[i]);
                GID glyphIndex = (*glyphIndices)[cid];

                if (!glyphIndex)
                {
                    // Try to obtain glyph index from unicode
                    if (m_face->charmap && m_face->charmap->encoding == FT_ENCODING_UNICODE)
                    {
                        glyphIndex = FT_Get_Char_Index(m_face, font->getUnicode(cid).unicode());
                    }
                }

                const PDFReal glyphWidth = font->getGlyphAdvance(cid);

                if (glyphIndex)
                {
                    const Glyph& glyph = getGlyph(glyphIndex);
                    textSequence.items.emplace_back(&glyph.glyph, font->getUnicode(cid), glyph.advance, cid);
                }
                else
                {
                    reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Glyph for simple font character code '%1' not found.").arg(cid));
                    if (glyphWidth > 0)
                    {
                        const QPainterPath* nullpath = nullptr;
                        textSequence.items.emplace_back(nullpath, QChar(), glyphWidth * m_pixelSize * FONT_WIDTH_MULTIPLIER, cid);
                    }
                }
            }
            break;
        }

        case FontType::Type0:
        {
            Q_ASSERT(dynamic_cast<PDFType0Font*>(m_parentFont.get()));
            const PDFType0Font* font = static_cast<PDFType0Font*>(m_parentFont.get());

            const PDFFontCMap* cmap = font->getCMap();
            const PDFFontCMap* toUnicode = font->getToUnicode();
            const PDFCIDtoGIDMapper* CIDtoGIDmapper = font->getCIDtoGIDMapper();

            std::vector<PDFFontCMap::MappedCode> mappedCodes = cmap->interpretWithCode(byteArray);
            textSequence.items.reserve(textSequence.items.size() + mappedCodes.size());
            for (const PDFFontCMap::MappedCode& mappedCode : mappedCodes)
            {
                const CID cid = mappedCode.cid;
                QChar character = toUnicode->getToUnicode(mappedCode.code, mappedCode.byteCount);
                if (character.isNull() && !m_isEmbedded)
                {
                    character = cmap->getUnicodeFromCode(mappedCode.code);
                }

                std::optional<GID> glyphIndex;
                if (!m_isEmbedded && !character.isNull() && m_face->charmap && m_face->charmap->encoding == FT_ENCODING_UNICODE)
                {
                    const GID unicodeGlyphIndex = FT_Get_Char_Index(m_face, character.unicode());
                    if (unicodeGlyphIndex)
                    {
                        glyphIndex = unicodeGlyphIndex;
                    }
                }
                if (!glyphIndex)
                {
                    const std::optional<GID> mappedGlyphIndex = CIDtoGIDmapper->tryMap(cid);
                    if (mappedGlyphIndex && canRenderGlyphIndex(*mappedGlyphIndex, character))
                    {
                        glyphIndex = mappedGlyphIndex;
                    }
                }

                const PDFReal glyphWidth = font->getGlyphAdvance(cid);

                if (glyphIndex)
                {
                    const Glyph& glyph = getGlyph(*glyphIndex);
                    textSequence.items.emplace_back(&glyph.glyph, character, glyph.advance, cid);
                }
                else
                {
                    if (cid > 0)
                    {
                        // Character with CID == 0 is treated as default whitespace, it hasn't glyph
                        reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Glyph for composite font character with cid '%1' not found.").arg(cid));
                    }

                    if (glyphWidth > 0)
                    {
                        // We do not multiply advance with font size and FONT_WIDTH_MULTIPLIER, because in the code,
                        // "advance" is treated as in font space.
                        const QPainterPath* nullpath = nullptr;
                        textSequence.items.emplace_back(nullpath, QChar(), -glyphWidth, cid);
                    }
                }
            }

            break;
        }

        default:
        {
            // Unhandled font type
            Q_ASSERT(false);
            break;
        }
    }
}

CharacterInfos PDFRealizedFontImpl::getCharacterInfos() const
{
    CharacterInfos result;

    switch (m_parentFont->getFontType())
    {
        case FontType::Type1:
        case FontType::TrueType:
        case FontType::MMType1:
        {
            // We can use encoding
            Q_ASSERT(dynamic_cast<PDFSimpleFont*>(m_parentFont.get()));
            const PDFSimpleFont* font = static_cast<PDFSimpleFont*>(m_parentFont.get());
            const GlyphIndices* glyphIndices = font->getGlyphIndices();

            for (size_t i = 0; i < glyphIndices->size(); ++i)
            {
                GID glyphIndex = (*glyphIndices)[static_cast<uint8_t>(i)];

                if (!glyphIndex)
                {
                    // Try to obtain glyph index from unicode
                    if (m_face->charmap && m_face->charmap->encoding == FT_ENCODING_UNICODE)
                    {
                        glyphIndex = FT_Get_Char_Index(m_face, font->getUnicode(static_cast<CID>(i)).unicode());
                    }
                }

                if (glyphIndex)
                {
                    CharacterInfo info;
                    info.gid = glyphIndex;
                    info.character = font->getUnicode(static_cast<CID>(i));
                    result.emplace_back(qMove(info));
                }
            }

            break;
        }

        case FontType::Type0:
        {
            Q_ASSERT(dynamic_cast<PDFType0Font*>(m_parentFont.get()));
            const PDFType0Font* font = static_cast<PDFType0Font*>(m_parentFont.get());

            const PDFFontCMap* toUnicode = font->getToUnicode();
            const PDFCIDtoGIDMapper* CIDtoGIDmapper = font->getCIDtoGIDMapper();

            FT_UInt index = 0;
            FT_ULong character = FT_Get_First_Char(m_face, &index);
            while (index != 0)
            {
                const GID gid = index;
                const CID cid = CIDtoGIDmapper->unmap(gid);

                CharacterInfo info;
                info.gid = gid;
                info.character = toUnicode->getToUnicode(cid);
                result.emplace_back(qMove(info));

                character = FT_Get_Next_Char(m_face, character, &index);
            }

            if (result.empty())
            {
                // We will try all reasonable high CIDs
                for (CID cid = 0; cid < QChar::LastValidCodePoint; ++cid)
                {
                    const std::optional<GID> gid = CIDtoGIDmapper->tryMap(cid);

                    if (!gid)
                    {
                        continue;
                    }

                    const QChar unicodeCharacter = toUnicode->getToUnicode(cid);
                    if (!canRenderGlyphIndex(*gid, unicodeCharacter))
                    {
                        continue;
                    }

                    if (!FT_Load_Glyph(m_face, *gid, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING))
                    {
                        CharacterInfo info;
                        info.gid = *gid;
                        info.character = unicodeCharacter;
                        result.emplace_back(qMove(info));
                    }
                }
            }

            break;
        }

        default:
        {
            // Unhandled font type
            Q_ASSERT(false);
            break;
        }
    }

    return result;
}

void PDFRealizedFontImpl::dumpFontToTreeItem(ITreeFactory* treeFactory) const
{
    treeFactory->pushItem({ PDFTranslationContext::tr("Details") });

    if (m_face->family_name)
    {
        treeFactory->addItem({ PDFTranslationContext::tr("Font"), QString::fromLatin1(m_face->family_name) });
    }
    if (m_face->style_name)
    {
        treeFactory->addItem({ PDFTranslationContext::tr("Style"), QString::fromLatin1(m_face->style_name) });
    }

    QString yesString = PDFTranslationContext::tr("Yes");
    QString noString = PDFTranslationContext::tr("No");

    treeFactory->addItem( { PDFTranslationContext::tr("Glyph count"), QString::number(m_face->num_glyphs) });
    treeFactory->addItem( { PDFTranslationContext::tr("Is CID keyed"), (m_face->face_flags & FT_FACE_FLAG_CID_KEYED) ? yesString : noString });
    treeFactory->addItem( { PDFTranslationContext::tr("Is bold"), (m_face->style_flags & FT_STYLE_FLAG_BOLD) ? yesString : noString });
    treeFactory->addItem( { PDFTranslationContext::tr("Is italics"), (m_face->style_flags & FT_STYLE_FLAG_ITALIC) ? yesString : noString });
    treeFactory->addItem( { PDFTranslationContext::tr("Has vertical writing system"), (m_face->face_flags & FT_FACE_FLAG_VERTICAL) ? yesString : noString });
    treeFactory->addItem( { PDFTranslationContext::tr("Has SFNT storage scheme"), (m_face->face_flags & FT_FACE_FLAG_SFNT) ? yesString : noString });
    treeFactory->addItem( { PDFTranslationContext::tr("Has glyph names"), (m_face->face_flags & FT_FACE_FLAG_GLYPH_NAMES) ? yesString : noString });

    if (m_face->num_charmaps > 0)
    {
        treeFactory->pushItem({ PDFTranslationContext::tr("Encoding") });
        for (FT_Int i = 0; i < m_face->num_charmaps; ++i)
        {
            FT_CharMap charMap = m_face->charmaps[i];

            const FT_Encoding encoding = charMap->encoding;
            QString encodingName;
            switch (encoding)
            {
                case FT_ENCODING_NONE:
                    encodingName = PDFTranslationContext::tr("None");
                    break;

                case FT_ENCODING_UNICODE:
                    encodingName = PDFTranslationContext::tr("Unicode");
                    break;

                case FT_ENCODING_MS_SYMBOL:
                    encodingName = PDFTranslationContext::tr("MS Symbol");
                    break;

                case FT_ENCODING_SJIS:
                    encodingName = PDFTranslationContext::tr("Japanese Shift JIS");
                    break;

                case FT_ENCODING_PRC:
                    encodingName = PDFTranslationContext::tr("PRC - Simplified Chinese");
                    break;

                case FT_ENCODING_BIG5:
                    encodingName = PDFTranslationContext::tr("Traditional Chinese");
                    break;

                case FT_ENCODING_WANSUNG:
                    encodingName = PDFTranslationContext::tr("Korean Extended Wansung");
                    break;

                case FT_ENCODING_JOHAB:
                    encodingName = PDFTranslationContext::tr("Korean Standard");
                    break;

                case FT_ENCODING_ADOBE_STANDARD:
                    encodingName = PDFTranslationContext::tr("Adobe Standard");
                    break;

                case FT_ENCODING_ADOBE_EXPERT:
                    encodingName = PDFTranslationContext::tr("Adobe Expert");
                    break;
                case FT_ENCODING_ADOBE_CUSTOM:
                    encodingName = PDFTranslationContext::tr("Adobe Custom");
                    break;

                case FT_ENCODING_ADOBE_LATIN_1:
                    encodingName = PDFTranslationContext::tr("Adobe Latin 1");
                    break;

                case FT_ENCODING_OLD_LATIN_2:
                    encodingName = PDFTranslationContext::tr("Old Latin 1");
                    break;

                case FT_ENCODING_APPLE_ROMAN:
                    encodingName = PDFTranslationContext::tr("Apple Roman");
                    break;

                default:
                    encodingName = PDFTranslationContext::tr("Unknown");
                    break;
            }

            QString encodingString = PDFTranslationContext::tr("Platform/Encoding = %1 %2").arg(charMap->platform_id).arg(charMap->encoding_id);
            treeFactory->addItem({ encodingName, encodingString });
        }
        treeFactory->popItem();
    }

    treeFactory->popItem();
}

int PDFRealizedFontImpl::outlineMoveTo(const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->glyph.moveTo(to->x * FONT_MULTIPLIER, to->y * FONT_MULTIPLIER);
    return 0;
}

int PDFRealizedFontImpl::outlineLineTo(const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->glyph.lineTo(to->x * FONT_MULTIPLIER, to->y * FONT_MULTIPLIER);
    return 0;
}

int PDFRealizedFontImpl::outlineConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->glyph.quadTo(control->x * FONT_MULTIPLIER, control->y * FONT_MULTIPLIER, to->x * FONT_MULTIPLIER, to->y * FONT_MULTIPLIER);
    return 0;
}

int PDFRealizedFontImpl::outlineCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
    Glyph* glyph = reinterpret_cast<Glyph*>(user);
    glyph->glyph.cubicTo(control1->x * FONT_MULTIPLIER, control1->y * FONT_MULTIPLIER, control2->x * FONT_MULTIPLIER, control2->y * FONT_MULTIPLIER, to->x * FONT_MULTIPLIER, to->y * FONT_MULTIPLIER);
    return 0;
}

const PDFRealizedFontImpl::Glyph& PDFRealizedFontImpl::getGlyph(unsigned int glyphIndex)
{
    {
        QReadLocker readLock(&m_readWriteLock);

        // First look into cache
        auto it = m_glyphCache.find(glyphIndex);
        if (it != m_glyphCache.cend())
        {
            return it->second;
        }
    }

    QWriteLocker writeLock(&m_readWriteLock);
    Glyph glyph;

    FT_Outline_Funcs glyphOutlineInterface;
    glyphOutlineInterface.delta = 0;
    glyphOutlineInterface.shift = 0;
    glyphOutlineInterface.move_to = PDFRealizedFontImpl::outlineMoveTo;
    glyphOutlineInterface.line_to = PDFRealizedFontImpl::outlineLineTo;
    glyphOutlineInterface.conic_to = PDFRealizedFontImpl::outlineConicTo;
    glyphOutlineInterface.cubic_to = PDFRealizedFontImpl::outlineCubicTo;

    checkFreeTypeError(FT_Load_Glyph(m_face, glyphIndex, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING));
    checkFreeTypeError(FT_Outline_Decompose(&m_face->glyph->outline, &glyphOutlineInterface, &glyph));
    glyph.glyph.closeSubpath();
    glyph.advance = !m_isVertical ? m_face->glyph->advance.x : m_face->glyph->advance.y;
    glyph.advance *= FONT_MULTIPLIER;

    auto it = m_glyphCache.find(glyphIndex);
    if (it == m_glyphCache.cend())
    {
        it = m_glyphCache.insert(std::make_pair(glyphIndex, qMove(glyph))).first;
    }
    return it->second;
}

bool PDFRealizedFontImpl::canRenderGlyphIndex(GID glyphIndex, QChar character) const
{
    if (glyphIndex != 0)
    {
        return true;
    }

    if (character.isNull())
    {
        return false;
    }

    if (m_face && FT_Has_PS_Glyph_Names(m_face))
    {
        char glyphName[128] = { };
        if (!FT_Get_Glyph_Name(m_face, glyphIndex, glyphName, static_cast<FT_ULong>(std::size(glyphName))))
        {
            return qstrcmp(glyphName, ".notdef") != 0;
        }
    }

    return m_isEmbedded;
}

void PDFRealizedFontImpl::checkFreeTypeError(FT_Error error)
{
    if (error)
    {
        QString message;
        if (const char* errorString = FT_Error_String(error))
        {
            message = QString::fromLatin1(errorString);
        }

        throw PDFException(PDFTranslationContext::tr("FreeType error code %1: %2").arg(error).arg(message));
    }
}

PDFRealizedFont::~PDFRealizedFont()
{
    delete m_impl;
}

void PDFRealizedFont::fillTextSequence(const QByteArray& byteArray, TextSequence& textSequence, PDFRenderErrorReporter* reporter)
{
    m_impl->fillTextSequence(byteArray, textSequence, reporter);
}

bool PDFRealizedFont::isHorizontalWritingSystem() const
{
    return m_impl->isHorizontalWritingSystem();
}

void PDFRealizedFont::dumpFontToTreeItem(ITreeFactory* treeFactory) const
{
    m_impl->dumpFontToTreeItem(treeFactory);
}

QString PDFRealizedFont::getPostScriptName() const
{
    return m_impl->getPostScriptName();
}

CharacterInfos PDFRealizedFont::getCharacterInfos() const
{
    return m_impl->getCharacterInfos();
}

PDFRealizedFontPointer PDFRealizedFont::createRealizedFont(PDFFontPointer font, PDFReal pixelSize, PDFRenderErrorReporter* reporter)
{
    PDFRealizedFontPointer result;

    if (pixelSize < 0.0)
    {
        pixelSize = qAbs(pixelSize);
    }

    if (font->getFontType() == FontType::Type3)
    {
        result.reset(new PDFRealizedFont(new PDFRealizedType3FontImpl(font, pixelSize)));
    }
    else
    {
        std::unique_ptr<PDFRealizedFontImpl> implPtr(new PDFRealizedFontImpl());

        PDFRealizedFontImpl* impl = implPtr.get();
        impl->m_parentFont = font;
        impl->m_pixelSize = pixelSize;

        const PDFFontCMap* cmap = font->getCMap();
        const FontDescriptor* descriptor = font->getFontDescriptor();
        if (descriptor->isEmbedded())
        {
            PDFRealizedFontImpl::checkFreeTypeError(FT_Init_FreeType(&impl->m_library));
            const QByteArray* embeddedFontData = descriptor->getEmbeddedFontData();
            Q_ASSERT(embeddedFontData);
            impl->m_embeddedFontData = *embeddedFontData;

            // At this time, embedded font data should not be empty!
            Q_ASSERT(!impl->m_embeddedFontData.isEmpty());

            PDFRealizedFontImpl::checkFreeTypeError(FT_New_Memory_Face(impl->m_library, reinterpret_cast<const FT_Byte*>(impl->m_embeddedFontData.constData()), impl->m_embeddedFontData.size(), 0, &impl->m_face));
            FT_Select_Charmap(impl->m_face, FT_ENCODING_UNICODE); // We try to select unicode encoding, but if it fails, we don't do anything (use glyph indices instead)
            PDFRealizedFontImpl::checkFreeTypeError(FT_Set_Pixel_Sizes(impl->m_face, 0, qRound(pixelSize * PDFRealizedFontImpl::PIXEL_SIZE_MULTIPLIER)));
            impl->m_isVertical = cmap ? cmap->isVertical() : false;
            impl->m_isEmbedded = true;
            result.reset(new PDFRealizedFont(implPtr.release()));
        }
        else
        {
            StandardFontType standardFontType = StandardFontType::Invalid;
            if (const PDFSimpleFont* simpleFont = dynamic_cast<const PDFSimpleFont*>(font.get()))
            {
                standardFontType = simpleFont->getStandardFontType();
            }

            const PDFSystemFontInfoStorage* fontStorage = PDFSystemFontInfoStorage::getInstance();
            const SystemFontData systemFontData = fontStorage->loadFont(font->getCIDSystemInfo(), descriptor, standardFontType, reporter);
            impl->m_systemFontData = systemFontData.data;
            impl->m_systemFontFaceIndex = systemFontData.faceIndex;

            if (impl->m_systemFontData.isEmpty())
            {
                throw PDFException(PDFTranslationContext::tr("Can't load system font '%1'.").arg(QString::fromLatin1(descriptor->fontName)));
            }

            PDFRealizedFontImpl::checkFreeTypeError(FT_Init_FreeType(&impl->m_library));
            if (impl->m_systemFontData.size() > (std::numeric_limits<FT_Long>::max)())
            {
                throw PDFException(PDFTranslationContext::tr("System font '%1' is too large to be loaded by FreeType.").arg(QString::fromLatin1(descriptor->fontName)));
            }
            PDFRealizedFontImpl::checkFreeTypeError(FT_New_Memory_Face(impl->m_library, reinterpret_cast<const FT_Byte*>(impl->m_systemFontData.constData()), static_cast<FT_Long>(impl->m_systemFontData.size()), impl->m_systemFontFaceIndex, &impl->m_face));
            FT_Select_Charmap(impl->m_face, FT_ENCODING_UNICODE); // We try to select unicode encoding, but if it fails, we don't do anything (use glyph indices instead)
            PDFRealizedFontImpl::checkFreeTypeError(FT_Set_Pixel_Sizes(impl->m_face, 0, qRound(pixelSize * PDFRealizedFontImpl::PIXEL_SIZE_MULTIPLIER)));
            impl->m_isVertical = cmap ? cmap->isVertical() : false;
            impl->m_isEmbedded = false;
            if (const char* postScriptName = FT_Get_Postscript_Name(impl->m_face))
            {
                impl->m_postScriptName = QString::fromLatin1(postScriptName);
            }
            result.reset(new PDFRealizedFont(implPtr.release()));
        }
    }

    return result;
}

FontDescriptor PDFFont::readFontDescriptor(const PDFObject& fontDescriptorObject, const PDFDocument* document)
{
    FontDescriptor fontDescriptor;
    PDFDocumentDataLoaderDecorator fontLoader(document);
    if (fontDescriptorObject.isDictionary())
    {
        const PDFDictionary* fontDescriptorDictionary = fontDescriptorObject.getDictionary();
        fontDescriptor.fontName = fontLoader.readNameFromDictionary(fontDescriptorDictionary, "FontName");
        fontDescriptor.fontFamily = fontLoader.readStringFromDictionary(fontDescriptorDictionary, "FontFamily");

        constexpr const std::array<std::pair<const char*, QFont::Stretch>, 9> stretches = {
            std::pair<const char*, QFont::Stretch>{ "UltraCondensed", QFont::UltraCondensed },
            std::pair<const char*, QFont::Stretch>{ "ExtraCondensed", QFont::ExtraCondensed },
            std::pair<const char*, QFont::Stretch>{ "Condensed", QFont::Condensed },
            std::pair<const char*, QFont::Stretch>{ "SemiCondensed", QFont::SemiCondensed },
            std::pair<const char*, QFont::Stretch>{ "Normal", QFont::Unstretched },
            std::pair<const char*, QFont::Stretch>{ "SemiExpanded", QFont::SemiExpanded },
            std::pair<const char*, QFont::Stretch>{ "Expanded", QFont::Expanded },
            std::pair<const char*, QFont::Stretch>{ "ExtraExpanded", QFont::ExtraExpanded },
            std::pair<const char*, QFont::Stretch>{ "UltraExpanded", QFont::UltraExpanded }
        };
        fontDescriptor.fontStretch = fontLoader.readEnumByName(fontDescriptorDictionary->get("FontStretch"), stretches.cbegin(), stretches.cend(), QFont::Unstretched);
        fontDescriptor.fontWeight = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "FontWeight", 500);
        fontDescriptor.italicAngle = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "ItalicAngle", 0.0);
        fontDescriptor.ascent = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "Ascent", 0.0);
        fontDescriptor.descent = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "Descent", 0.0);
        fontDescriptor.leading = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "Leading", 0.0);
        fontDescriptor.capHeight = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "CapHeight", 0.0);
        fontDescriptor.xHeight = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "XHeight", 0.0);
        fontDescriptor.stemV = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "StemV", 0.0);
        fontDescriptor.stemH = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "StemH", 0.0);
        fontDescriptor.avgWidth = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "AvgWidth", 0.0);
        fontDescriptor.maxWidth = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "MaxWidth", 0.0);
        fontDescriptor.missingWidth = fontLoader.readNumberFromDictionary(fontDescriptorDictionary, "MissingWidth", 0.0);
        fontDescriptor.flags = fontLoader.readIntegerFromDictionary(fontDescriptorDictionary, "Flags", 0);
        fontDescriptor.boundingBox = fontLoader.readRectangle(fontDescriptorDictionary->get("FontBBox"), QRectF());
        fontDescriptor.charset = fontLoader.readStringFromDictionary(fontDescriptorDictionary, "Charset");

        auto loadStream = [fontDescriptorDictionary, document](QByteArray& byteArray, const char* name)
        {
            if (fontDescriptorDictionary->hasKey(name))
            {
                const PDFObject& streamObject = document->getObject(fontDescriptorDictionary->get(name));
                if (streamObject.isStream())
                {
                    byteArray = document->getDecodedStream(streamObject.getStream());
                }
            }
        };
        loadStream(fontDescriptor.fontFile, "FontFile");
        loadStream(fontDescriptor.fontFile2, "FontFile2");
        loadStream(fontDescriptor.fontFile3, "FontFile3");
    }

    return fontDescriptor;
}

CIDSystemInfo PDFFont::readCIDSystemInfo(const PDFObject& cidSystemInfoObject, const PDFDocument* document)
{
    CIDSystemInfo cidSystemInfo;

    if (const PDFDictionary* cidSystemInfoDictionary = document->getDictionaryFromObject(cidSystemInfoObject))
    {
        PDFDocumentDataLoaderDecorator cidSystemInfoLoader(document);
        cidSystemInfo.registry = cidSystemInfoLoader.readStringFromDictionary(cidSystemInfoDictionary, "Registry");
        cidSystemInfo.ordering = cidSystemInfoLoader.readStringFromDictionary(cidSystemInfoDictionary, "Ordering");
        cidSystemInfo.supplement = cidSystemInfoLoader.readIntegerFromDictionary(cidSystemInfoDictionary, "Supplement", 0);
    }

    return cidSystemInfo;
}

QByteArray PDFFont::getFontId() const
{
    return m_fontId;
}

PDFEncodedText PDFFont::encodeText(const QString& text) const
{
    PDFEncodedText result;
    result.isValid = true;

    for (qsizetype i = 0, size = text.size(); i < size; ++i)
    {
        const QChar character = text[i];
        char32_t codePoint = character.unicode();
        QString sourceCharacter = character;

        if (character.isHighSurrogate() && i + 1 < size && text[i + 1].isLowSurrogate())
        {
            codePoint = QChar::surrogateToUcs4(character, text[i + 1]);
            sourceCharacter += text[i + 1];
            ++i;
        }

        QByteArray encoded = encodeCharacter(codePoint);
        if (!encoded.isEmpty())
        {
            result.encodedText.append(encoded);
            result.errorString += "_";
        }
        else
        {
            result.isValid = false;
            result.errorString += sourceCharacter;
        }
    }

    return result;
}

PDFFontPointer PDFFont::createFont(const PDFObject& object, QByteArray fontId, const PDFDocument* document)
{
    const PDFObject& dereferencedFontDictionary = document->getObject(object);
    if (!dereferencedFontDictionary.isDictionary())
    {
        throw PDFException(PDFTranslationContext::tr("Font object must be a dictionary."));
    }

    const PDFDictionary* fontDictionary = dereferencedFontDictionary.getDictionary();
    PDFDocumentDataLoaderDecorator fontLoader(document);

    // First, determine the font subtype
    constexpr const std::array fontTypes = {
        std::pair<const char*, FontType>{ "Type0", FontType::Type0 },
        std::pair<const char*, FontType>{ "Type1", FontType::Type1 },
        std::pair<const char*, FontType>{ "TrueType", FontType::TrueType },
        std::pair<const char*, FontType>{ "Type3", FontType::Type3},
        std::pair<const char*, FontType>{ "MMType1", FontType::MMType1 }
    };

    const FontType fontType = fontLoader.readEnumByName(fontDictionary->get("Subtype"), fontTypes.cbegin(), fontTypes.cend(), FontType::Invalid);
    if (fontType == FontType::Invalid)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid font type."));
    }

    QByteArray name = fontLoader.readNameFromDictionary(fontDictionary, "Name");
    QByteArray baseFont = fontLoader.readNameFromDictionary(fontDictionary, "BaseFont");
    const PDFInteger firstChar = fontLoader.readIntegerFromDictionary(fontDictionary, "FirstChar", 0);
    const PDFInteger lastChar = fontLoader.readIntegerFromDictionary(fontDictionary, "LastChar", 255);
    std::vector<PDFInteger> widths = fontLoader.readIntegerArrayFromDictionary(fontDictionary, "Widths");

    // Read standard font
    static constexpr std::array standardFonts = {
        std::pair<const char*, StandardFontType>{ "Times-Roman", StandardFontType::TimesRoman },
        std::pair<const char*, StandardFontType>{ "Times-Bold", StandardFontType::TimesRomanBold },
        std::pair<const char*, StandardFontType>{ "Times-Italic", StandardFontType::TimesRomanItalics },
        std::pair<const char*, StandardFontType>{ "Times-BoldItalic", StandardFontType::TimesRomanBoldItalics },
        std::pair<const char*, StandardFontType>{ "Helvetica", StandardFontType::Helvetica },
        std::pair<const char*, StandardFontType>{ "Helvetica-Bold", StandardFontType::HelveticaBold },
        std::pair<const char*, StandardFontType>{ "Helvetica-Oblique", StandardFontType::HelveticaOblique },
        std::pair<const char*, StandardFontType>{ "Helvetica-BoldOblique", StandardFontType::HelveticaBoldOblique },
        std::pair<const char*, StandardFontType>{ "Courier", StandardFontType::Courier },
        std::pair<const char*, StandardFontType>{ "Courier-Bold", StandardFontType::CourierBold },
        std::pair<const char*, StandardFontType>{ "Courier-Oblique", StandardFontType::CourierOblique },
        std::pair<const char*, StandardFontType>{ "Courier-BoldOblique", StandardFontType::CourierBoldOblique },
        std::pair<const char*, StandardFontType>{ "Symbol", StandardFontType::Symbol },
        std::pair<const char*, StandardFontType>{ "ZapfDingbats", StandardFontType::ZapfDingbats },

        std::pair<const char*, StandardFontType>{ "TimesNewRomanPSMT", StandardFontType::TimesRoman },
        std::pair<const char*, StandardFontType>{ "TimesNewRomanPS-BoldMT", StandardFontType::TimesRomanBold },
        std::pair<const char*, StandardFontType>{ "TimesNewRomanPS-ItalicMT", StandardFontType::TimesRomanItalics },
        std::pair<const char*, StandardFontType>{ "TimesNewRomanPS-BoldItalicMT", StandardFontType::TimesRomanBoldItalics },
        std::pair<const char*, StandardFontType>{ "ArialMT", StandardFontType::Helvetica },
        std::pair<const char*, StandardFontType>{ "Arial-BoldMT", StandardFontType::HelveticaBold },
        std::pair<const char*, StandardFontType>{ "Arial-ItalicMT", StandardFontType::HelveticaOblique },
        std::pair<const char*, StandardFontType>{ "Arial-BoldItalicMT", StandardFontType::HelveticaBoldOblique },
        std::pair<const char*, StandardFontType>{ "CourierNewPSMT", StandardFontType::Courier },
        std::pair<const char*, StandardFontType>{ "CourierNewPS-BoldMT", StandardFontType::CourierBold },
        std::pair<const char*, StandardFontType>{ "CourierNewPS-ItalicMT", StandardFontType::CourierOblique },
        std::pair<const char*, StandardFontType>{ "CourierNewPS-BoldItalicMT", StandardFontType::CourierBoldOblique },
    };
    const StandardFontType standardFont = fontLoader.readEnumByName(fontDictionary->get("BaseFont"), standardFonts.cbegin(), standardFonts.cend(), StandardFontType::Invalid);

    // Read Font Descriptor
    const PDFObject& fontDescriptorObject = document->getObject(fontDictionary->get("FontDescriptor"));
    FontDescriptor fontDescriptor = readFontDescriptor(fontDescriptorObject, document);

    // Read CID System Info
    const PDFObject& cidSystemInfoObject = document->getObject(fontDictionary->get("CIDSystemInfo"));
    CIDSystemInfo cidSystemInfo = readCIDSystemInfo(cidSystemInfoObject, document);

    // Read Font Encoding
    // The font encoding for the simple font is determined by this algorithm:
    //      1) Try to use Encoding dictionary to determine base encoding
    //         (it can be MacRomanEncoding, MacExpertEncoding, WinAnsiEncoding or StandardEncoding)
    //      2) If it is not present, then try to obtain built-in encoding from the font file (usually, this is not possible)
    //      3) Use default encoding for the font depending on the font type
    //          - one of the 14 base fonts - use builtin encoding for the font type
    //          - TrueType - use WinAnsiEncoding
    //          - all others - use StandardEncoding
    //      4) Merge with Differences, if present
    //      5) Fill missing characters from StandardEncoding
    // After the encoding is obtained, try to extract glyph indices for embedded font.

    PDFEncoding::Encoding encoding = PDFEncoding::Encoding::Invalid;
    encoding::EncodingTable simpleFontEncodingTable = { };
    encoding::EncodingTable simpleFontToUnicodeTable = { };
    bool hasToUnicode = false;
    GlyphIndices glyphIndexArray = { };
    GlyphNames glyphNameArray = { };
    switch (fontType)
    {
        case FontType::Type1:
        case FontType::MMType1:
        case FontType::TrueType:
        {
            bool hasDifferences = false;
            encoding::EncodingTable differences = { };
            GlyphNames differenceGlyphNames = { };
            bool useEmbeddedBuiltInEncoding = false;

            if (fontDictionary->hasKey("Encoding"))
            {
                constexpr const std::array<std::pair<const char*, PDFEncoding::Encoding>, 4> encodings = {
                    std::pair<const char*, PDFEncoding::Encoding>{ "StandardEncoding", PDFEncoding::Encoding::Standard },
                    std::pair<const char*, PDFEncoding::Encoding>{ "MacRomanEncoding", PDFEncoding::Encoding::MacRoman },
                    std::pair<const char*, PDFEncoding::Encoding>{ "MacExpertEncoding", PDFEncoding::Encoding::MacExpert },
                    std::pair<const char*, PDFEncoding::Encoding>{ "WinAnsiEncoding", PDFEncoding::Encoding::WinAnsi }
                };

                const PDFObject& encodingObject = document->getObject(fontDictionary->get("Encoding"));
                if (encodingObject.isName())
                {
                    // Decode name of the encoding
                    encoding = fontLoader.readEnumByName(encodingObject, encodings.cbegin(), encodings.cend(), PDFEncoding::Encoding::Invalid);
                }
                else if (encodingObject.isDictionary())
                {
                    // Dictionary with base encoding and differences (all optional)
                    const PDFDictionary* encodingDictionary = encodingObject.getDictionary();
                    if (encodingDictionary->hasKey("BaseEncoding"))
                    {
                        encoding = fontLoader.readEnumByName(encodingDictionary->get("BaseEncoding"), encodings.cbegin(), encodings.cend(), PDFEncoding::Encoding::Invalid);
                    }
                    else
                    {
                        if (fontDescriptor.isEmbedded() && (fontType == FontType::Type1 || fontType == FontType::MMType1))
                        {
                            useEmbeddedBuiltInEncoding = true;
                        }
                        else
                        {
                            // Without an embedded built-in encoding, use the best available substitute base.
                            encoding = getEncodingForStandardFont(standardFont);
                        }
                    }

                    if (encodingDictionary->hasKey("Differences"))
                    {
                        const PDFObject& differencesArray = document->getObject(encodingDictionary->get("Differences"));
                        if (differencesArray.isArray())
                        {
                            hasDifferences = true;
                            const PDFArray* array = differencesArray.getArray();
                            size_t currentOffset = 0;
                            for (size_t i = 0, count = array->getCount(); i < count; ++i)
                            {
                                const PDFObject& item = document->getObject(array->getItem(i));
                                if (item.isInt())
                                {
                                    currentOffset = static_cast<size_t>(item.getInteger());
                                }
                                else if (item.isName())
                                {
                                    if (currentOffset >= differences.size())
                                    {
                                        throw PDFException(PDFTranslationContext::tr("Invalid differences in encoding entry of the font."));
                                    }

                                    const QByteArray glyphName = item.getString();
                                    QChar character = PDFNameToUnicode::getUnicodeUsingResolvedName(glyphName);
                                    differences[currentOffset] = character;
                                    differenceGlyphNames[currentOffset] = glyphName;

                                    ++currentOffset;
                                }
                                else
                                {
                                    throw PDFException(PDFTranslationContext::tr("Invalid differences in encoding entry of the font."));
                                }
                            }
                        }
                        else
                        {
                            throw PDFException(PDFTranslationContext::tr("Invalid differences in encoding entry of the font."));
                        }
                    }
                }
                else
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid encoding entry of the font."));
                }
            }
            else if (fontDescriptor.isEmbedded() && (fontType == FontType::Type1 || fontType == FontType::MMType1))
            {
                useEmbeddedBuiltInEncoding = true;
            }
            if (fontDictionary->hasKey("ToUnicode"))
            {
                PDFFontCMap toUnicodeCMap;
                const PDFObject& toUnicode = document->getObject(fontDictionary->get("ToUnicode"));
                if (toUnicode.isName())
                {
                    toUnicodeCMap = PDFFontCMap::createFromName(toUnicode.getString());
                }
                else if (toUnicode.isStream())
                {
                    const PDFStream* stream = toUnicode.getStream();
                    QByteArray decodedStream = document->getDecodedStream(stream);
                    toUnicodeCMap = PDFFontCMap::createFromData(decodedStream);
                }

                hasToUnicode = toUnicodeCMap.isValid();
                if (hasToUnicode)
                {
                    for (size_t i = 0; i < simpleFontToUnicodeTable.size(); ++i)
                    {
                        QChar character = toUnicodeCMap.getToUnicode(static_cast<CID>(i));

                        if (!character.isNull())
                        {
                            simpleFontToUnicodeTable[i] = character;
                        }
                    }
                }
            }

            if (encoding == PDFEncoding::Encoding::Invalid && !useEmbeddedBuiltInEncoding)
            {
                // We get encoding for the standard font. If we have invalid standard font,
                // then we get standard encoding. So we shouldn't test it.
                encoding = getEncodingForStandardFont(standardFont);
            }

            if (encoding == PDFEncoding::Encoding::Invalid && !useEmbeddedBuiltInEncoding)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid encoding entry of the font."));
            }

            if (encoding != PDFEncoding::Encoding::Invalid)
            {
                simpleFontEncodingTable = *PDFEncoding::getTableForEncoding(encoding);
                for (size_t i = 0; i < simpleFontEncodingTable.size(); ++i)
                {
                    QChar character = simpleFontEncodingTable[i];
                    if (!character.isNull() && character != QChar(QChar::SpecialCharacter::ReplacementCharacter))
                    {
                        glyphNameArray[i] = PDFNameToUnicode::getNameForUnicode(character);
                        if (glyphNameArray[i].isEmpty())
                        {
                            glyphNameArray[i] = PDFNameToUnicode::getNameForUnicodeZapfDingbats(character);
                        }
                    }
                }
            }

            auto finishFont = [&]
            {
                // Fill in differences
                if (hasDifferences)
                {
                    for (size_t i = 0; i < differences.size(); ++i)
                    {
                        if (!differences[i].isNull())
                        {
                            simpleFontEncodingTable[i] = differences[i];
                        }
                        if (!differenceGlyphNames[i].isEmpty())
                        {
                            glyphNameArray[i] = differenceGlyphNames[i];
                        }
                    }

                    // Set the encoding to custom
                    encoding = PDFEncoding::Encoding::Custom;
                }

                // Fill in missing characters from standard encoding
                const encoding::EncodingTable& standardEncoding = *PDFEncoding::getTableForEncoding(PDFEncoding::Encoding::Standard);
                for (size_t i = 0; i < standardEncoding.size(); ++i)
                {
                    if (differenceGlyphNames[i].isEmpty() &&
                            (simpleFontEncodingTable[i].isNull() || simpleFontEncodingTable[i] == QChar(QChar::SpecialCharacter::ReplacementCharacter)) &&
                            (!standardEncoding[i].isNull() && standardEncoding[i] != QChar(QChar::SpecialCharacter::ReplacementCharacter)))
                    {
                        simpleFontEncodingTable[i] = standardEncoding[i];
                    }
                }
            };

            if (fontDescriptor.isEmbedded())
            {
                // Return encoding from the embedded font
                const QByteArray* embeddedFontData = fontDescriptor.getEmbeddedFontData();
                Q_ASSERT(embeddedFontData);

                FT_Library library;
                if (!FT_Init_FreeType(&library))
                {
                    FT_Face face;
                    if (!FT_New_Memory_Face(library, reinterpret_cast<const FT_Byte*>(embeddedFontData->constData()), embeddedFontData->size(), 0, &face))
                    {
                        if (FT_Has_PS_Glyph_Names(face))
                        {
                            for (FT_Int i = 0; i < face->num_charmaps; ++i)
                            {
                                FT_CharMap charMap = face->charmaps[i];
                                switch (charMap->encoding)
                                {
                                    case FT_ENCODING_ADOBE_STANDARD:
                                    case FT_ENCODING_ADOBE_LATIN_1:
                                    case FT_ENCODING_ADOBE_CUSTOM:
                                    case FT_ENCODING_ADOBE_EXPERT:
                                    {
                                        // Try to load data from the encoding
                                        if (!FT_Set_Charmap(face, charMap))
                                        {
                                            for (size_t iTable = 0; iTable < simpleFontEncodingTable.size(); ++iTable)
                                            {
                                                FT_UInt glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(iTable));

                                                if (glyphIndex == 0)
                                                {
                                                    glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(iTable + 0xF000));
                                                }

                                                if (glyphIndex == 0)
                                                {
                                                    glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(iTable + 0xF100));
                                                }

                                                if (glyphIndex > 0)
                                                {
                                                    // Fill the glyph index array
                                                    glyphIndexArray[iTable] = glyphIndex;

                                                    // Set mapping to unicode
                                                    char buffer[128] = { };
                                                    if (!FT_Get_Glyph_Name(face, glyphIndex, buffer, static_cast<FT_ULong>(std::size(buffer))))
                                                    {
                                                        QByteArray byteArrayBuffer(buffer);
                                                        if (useEmbeddedBuiltInEncoding)
                                                        {
                                                            glyphNameArray[iTable] = byteArrayBuffer;
                                                        }
                                                        QChar character = PDFNameToUnicode::getUnicodeForName(byteArrayBuffer);
                                                        if (character.isNull())
                                                        {
                                                            character = PDFNameToUnicode::getUnicodeForNameZapfDingbats(byteArrayBuffer);
                                                        }
                                                        if (useEmbeddedBuiltInEncoding && !character.isNull())
                                                        {
                                                            encoding = PDFEncoding::Encoding::Custom;
                                                            simpleFontEncodingTable[iTable] = character;
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        break;
                                    }

                                    default:
                                        break;
                                }
                            }
                        }
                        else if (!FT_Select_Charmap(face, FT_ENCODING_UNICODE))
                        {
                            // if we have unicode mapping (3, 1), then we want to skip
                            // Mac Roman Encoding, according to PDF Specification 2.0.
                            // We will load encoding using unicode character map below.
                        }
                        else if (!FT_Select_Charmap(face, FT_ENCODING_APPLE_ROMAN))
                        {
                            // We have (1, 0) Mac Roman Encoding, which is slightly different, than Mac Roman Encoding defined
                            // in PDF (for 15 characters).
                            simpleFontEncodingTable = *PDFEncoding::getTableForEncoding(PDFEncoding::Encoding::MacOsRoman);
                            encoding = PDFEncoding::Encoding::Custom;

                            for (size_t i = 0; i < simpleFontEncodingTable.size(); ++i)
                            {
                                QChar character = simpleFontEncodingTable[i];
                                if (!character.isNull() && character != QChar(QChar::SpecialCharacter::ReplacementCharacter))
                                {
                                    glyphNameArray[i] = PDFNameToUnicode::getNameForUnicode(character);
                                    if (glyphNameArray[i].isEmpty())
                                    {
                                        glyphNameArray[i] = PDFNameToUnicode::getNameForUnicodeZapfDingbats(character);
                                    }
                                }

                                FT_UInt glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(i));
                                if (glyphIndex > 0)
                                {
                                    glyphIndexArray[i] = glyphIndex;
                                }
                            }
                        }

                        finishFont();

                        if ((fontType == FontType::Type1 || fontType == FontType::MMType1) && FT_Has_PS_Glyph_Names(face))
                        {
                            const FT_UInt notdefGlyphIndex = FT_Get_Name_Index(face, ".notdef");
                            for (size_t i = 0; i < glyphNameArray.size(); ++i)
                            {
                                if (!glyphNameArray[i].isEmpty())
                                {
                                    FT_UInt glyphIndex = FT_Get_Name_Index(face, glyphNameArray[i].constData());
                                    if (glyphIndex)
                                    {
                                        glyphIndexArray[i] = glyphIndex;
                                    }
                                    else if (!differenceGlyphNames[i].isEmpty())
                                    {
                                        glyphIndexArray[i] = notdefGlyphIndex;
                                    }
                                }
                            }
                        }

                        bool hasSymbolicTrueTypeCMap = false;
                        if (fontType == FontType::TrueType && shouldUseSymbolicTrueTypeCMap(fontDescriptor, face))
                        {
                            if (FT_CharMap charMap = selectSymbolicTrueTypeCMap(face))
                            {
                                hasSymbolicTrueTypeCMap = true;
                                for (size_t i = 0; i < glyphIndexArray.size(); ++i)
                                {
                                    const FT_UInt glyphIndex = getSymbolicTrueTypeGlyphIndex(face, charMap, static_cast<FT_ULong>(i));
                                    if (glyphIndex > 0)
                                    {
                                        glyphIndexArray[i] = glyphIndex;
                                    }
                                }
                            }
                        }

                        // Fill the glyph index array from unicode, if we have unicode mapping.
                        // Symbolic TrueType fonts use the font program's own cmap directly.
                        if (!hasSymbolicTrueTypeCMap && !FT_Select_Charmap(face, FT_ENCODING_UNICODE))
                        {
                            for (size_t i = 0; i < simpleFontEncodingTable.size(); ++i)
                            {
                                QChar character = simpleFontEncodingTable[i];
                                if ((fontType == FontType::TrueType || (glyphIndexArray[i] == 0 && differenceGlyphNames[i].isEmpty())) &&
                                    !character.isNull() && character != QChar(QChar::SpecialCharacter::ReplacementCharacter))
                                {
                                    const FT_UInt glyphIndex = FT_Get_Char_Index(face, character.unicode());
                                    if (glyphIndex > 0)
                                    {
                                        glyphIndexArray[i] = glyphIndex;
                                    }
                                }
                            }
                        }

                        FT_Done_Face(face);
                    }

                    FT_Done_FreeType(library);
                }
            }
            else
            {
                // Finish font - fill differences
                finishFont();
            }

            break;
        }

        case FontType::Type0:
        {
            // This is composite font (CID keyed font)

            // Load CMAP
            PDFFontCMap cmap;
            const PDFObject& cmapObject = document->getObject(fontDictionary->get("Encoding"));
            if (cmapObject.isName())
            {
                cmap = PDFFontCMap::createFromName(cmapObject.getString());
            }
            else if (cmapObject.isStream())
            {
                const PDFStream* stream = cmapObject.getStream();
                QByteArray decodedStream = document->getDecodedStream(stream);
                cmap = PDFFontCMap::createFromData(decodedStream);
            }

            if (!cmap.isValid())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid CMAP in CID-keyed font."));
            }

            const PDFObject& descendantFonts = document->getObject(fontDictionary->get("DescendantFonts"));
            if (!descendantFonts.isArray())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid descendant font in CID-keyed font."));
            }

            const PDFArray* descendantFontsArray = descendantFonts.getArray();
            if (descendantFontsArray->getCount() != 1)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid number (%1) of descendant fonts in CID-keyed font - exactly one is required.").arg(descendantFontsArray->getCount()));
            }

            const PDFObject& descendantFont = document->getObject(descendantFontsArray->getItem(0));
            if (!descendantFont.isDictionary())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid descendant font in CID-keyed font."));
            }

            const PDFDictionary* descendantFontDictionary = descendantFont.getDictionary();

            const PDFObject& fontDescriptorObjectForCompositeFont = document->getObject(descendantFontDictionary->get("FontDescriptor"));
            fontDescriptor = readFontDescriptor(fontDescriptorObjectForCompositeFont, document);

            const PDFObject& cidSystemInfoObjectForCompositeFont = document->getObject(descendantFontDictionary->get("CIDSystemInfo"));
            cidSystemInfo = readCIDSystemInfo(cidSystemInfoObjectForCompositeFont, document);

            QByteArray cidToGidMapping;
            const PDFObject& cidToGidMappingObject = document->getObject(descendantFontDictionary->get("CIDToGIDMap"));
            if (cidToGidMappingObject.isStream())
            {
                const PDFStream* cidToGidMappingStream = cidToGidMappingObject.getStream();
                cidToGidMapping = document->getDecodedStream(cidToGidMappingStream);
            }
            PDFCIDtoGIDMapper cidToGidMapper(qMove(cidToGidMapping));

            baseFont = fontLoader.readNameFromDictionary(descendantFontDictionary, "BaseFont");

            // Read default advance
            PDFReal dw = fontLoader.readNumberFromDictionary(descendantFontDictionary, "DW", 1000.0);
            std::array<PDFReal, 2> dw2 = { };
            fontLoader.readNumberArrayFromDictionary(descendantFontDictionary, "DW2", dw2.begin(), dw2.end());
            PDFReal defaultWidth = descendantFontDictionary->hasKey("DW") ? dw : dw2.back();

            // Read horizontal advances
            std::unordered_map<CID, PDFReal> advances;
            if (descendantFontDictionary->hasKey("W"))
            {
                 const PDFObject& wArrayObject = document->getObject(descendantFontDictionary->get("W"));
                 if (wArrayObject.isArray())
                 {
                     const PDFArray* wArray = wArrayObject.getArray();
                     const size_t size = wArray->getCount();

                     for (size_t i = 0; i < size;)
                     {
                         CID startCID = fontLoader.readInteger(wArray->getItem(i++), 0);
                         const PDFObject& arrayOrCID = document->getObject(wArray->getItem(i++));

                         if (arrayOrCID.isInt())
                         {
                             CID endCID = arrayOrCID.getInteger();
                             PDFReal width = fontLoader.readInteger(wArray->getItem(i++), 0);
                             for (CID currentCID = startCID; currentCID <= endCID; ++currentCID)
                             {
                                 advances[currentCID] = width;
                             }
                         }
                         else if (arrayOrCID.isArray())
                         {
                             const PDFArray* widthArray = arrayOrCID.getArray();
                             const size_t widthArraySize = widthArray->getCount();
                             for (size_t widthArrayIndex = 0; widthArrayIndex < widthArraySize; ++widthArrayIndex)
                             {
                                 PDFReal width = fontLoader.readNumber(widthArray->getItem(widthArrayIndex), 0);
                                 advances[startCID + static_cast<CID>(widthArrayIndex)] = width;
                             }
                         }
                     }
                 }
            }

            PDFFontCMap toUnicodeCMap;
            const PDFObject& toUnicode = document->getObject(fontDictionary->get("ToUnicode"));
            if (toUnicode.isName())
            {
                toUnicodeCMap = PDFFontCMap::createFromName(toUnicode.getString());
            }
            else if (toUnicode.isStream())
            {
                const PDFStream* stream = toUnicode.getStream();
                QByteArray decodedStream = document->getDecodedStream(stream);
                toUnicodeCMap = PDFFontCMap::createFromData(decodedStream);
            }

            return PDFFontPointer(new PDFType0Font(qMove(cidSystemInfo), qMove(fontId), qMove(fontDescriptor), qMove(cmap), qMove(toUnicodeCMap), qMove(cidToGidMapper), defaultWidth, qMove(advances)));
        }

        case FontType::Type3:
        {
            // Read the font matrix
            std::vector<PDFReal> fontMatrixValues = fontLoader.readNumberArrayFromDictionary(fontDictionary, "FontMatrix");

            if (fontMatrixValues.size() != 6)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid Type 3 font matrix."));
            }
            QTransform fontMatrix(fontMatrixValues[0], fontMatrixValues[1], fontMatrixValues[2], fontMatrixValues[3], fontMatrixValues[4], fontMatrixValues[5]);

            PDFObject charProcs = document->getObject(fontDictionary->get("CharProcs"));
            if (!charProcs.isDictionary())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid Type 3 font character content streams."));
            }
            const PDFDictionary* charProcsDictionary = charProcs.getDictionary();

            const PDFInteger firstCharF3 = fontLoader.readIntegerFromDictionary(fontDictionary, "FirstChar", -1);
            const PDFInteger lastCharF3 = fontLoader.readIntegerFromDictionary(fontDictionary, "LastChar", -1);

            if (firstCharF3 < 0 || lastCharF3 > 255 || firstCharF3 > lastCharF3)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid Type 3 font character range (from %1 to %2).").arg(firstCharF3).arg(lastCharF3));
            }

            const PDFObject& encodingF3 = document->getObject(fontDictionary->get("Encoding"));
            if (!encodingF3.isDictionary())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid Type 3 font encoding."));
            }

            const PDFDictionary* encodingDictionary = encodingF3.getDictionary();
            const PDFObject& differences = document->getObject(encodingDictionary->get("Differences"));
            if (!differences.isArray())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid Type 3 font encoding."));
            }

            std::map<int, QByteArray> characterContentStreams;

            const PDFArray* differencesArray = differences.getArray();
            size_t currentOffset = 0;
            for (size_t i = 0, count = differencesArray->getCount(); i < count; ++i)
            {
                const PDFObject& item = document->getObject(differencesArray->getItem(i));
                if (item.isInt())
                {
                    currentOffset = static_cast<size_t>(item.getInteger());
                }
                else if (item.isName())
                {
                    if (currentOffset > 255)
                    {
                        throw PDFException(PDFTranslationContext::tr("Invalid differences in encoding entry of type 3 font."));
                    }

                    QByteArray characterName = item.getString();
                    const PDFObject& characterContentStreamObject = document->getObject(charProcsDictionary->get(characterName));
                    if (characterContentStreamObject.isStream())
                    {
                        QByteArray contentStream = document->getDecodedStream(characterContentStreamObject.getStream());
                        characterContentStreams[static_cast<int>(currentOffset)] = qMove(contentStream);
                    }

                    ++currentOffset;
                }
                else
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid differences in encoding entry of type 3 font."));
                }
            }

            PDFFontCMap toUnicodeCMap;
            const PDFObject& toUnicode = document->getObject(fontDictionary->get("ToUnicode"));
            if (toUnicode.isName())
            {
                toUnicodeCMap = PDFFontCMap::createFromName(toUnicode.getString());
            }
            else if (toUnicode.isStream())
            {
                const PDFStream* stream = toUnicode.getStream();
                QByteArray decodedStream = document->getDecodedStream(stream);
                toUnicodeCMap = PDFFontCMap::createFromData(decodedStream);
            }

            std::vector<PDFReal> widthsF3 = fontLoader.readNumberArrayFromDictionary(fontDictionary, "Widths");
            return PDFFontPointer(new PDFType3Font(qMove(fontDescriptor), qMove(fontId), firstCharF3, lastCharF3, fontMatrix, qMove(characterContentStreams), qMove(widthsF3), document->getObject(fontDictionary->get("Resources")), qMove(toUnicodeCMap)));
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    switch (fontType)
    {
        case FontType::Type1:
        case FontType::MMType1:
            return PDFFontPointer(new PDFType1Font(fontType, qMove(fontId), qMove(cidSystemInfo), qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encoding, simpleFontEncodingTable, simpleFontToUnicodeTable, hasToUnicode, standardFont, glyphIndexArray, qMove(glyphNameArray)));

        case FontType::TrueType:
            return PDFFontPointer(new PDFTrueTypeFont(qMove(cidSystemInfo), qMove(fontId), qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encoding, simpleFontEncodingTable, simpleFontToUnicodeTable, hasToUnicode, standardFont, glyphIndexArray, qMove(glyphNameArray)));

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return PDFFontPointer();
}

PDFSimpleFont::PDFSimpleFont(CIDSystemInfo cidSystemInfo,
                             QByteArray fontId,
                             FontDescriptor fontDescriptor,
                             QByteArray name,
                             QByteArray baseFont,
                             PDFInteger firstChar,
                             PDFInteger lastChar,
                             std::vector<PDFInteger> widths,
                             PDFEncoding::Encoding encodingType,
                             encoding::EncodingTable encoding,
                             encoding::EncodingTable toUnicode,
                             bool hasToUnicode,
                             StandardFontType standardFontType,
                             GlyphIndices glyphIndices,
                             GlyphNames glyphNames) :
    PDFFont(qMove(cidSystemInfo), qMove(fontId), qMove(fontDescriptor)),
    m_name(qMove(name)),
    m_baseFont(qMove(baseFont)),
    m_firstChar(firstChar),
    m_lastChar(lastChar),
    m_widths(qMove(widths)),
    m_encodingType(encodingType),
    m_encoding(encoding),
    m_toUnicode(toUnicode),
    m_hasToUnicode(hasToUnicode),
    m_glyphIndices(glyphIndices),
    m_glyphNames(qMove(glyphNames)),
    m_standardFontType(standardFontType)
{

}

QChar PDFSimpleFont::getUnicode(CID cid) const
{
    if (cid < m_toUnicode.size() && m_hasToUnicode && !m_toUnicode[cid].isNull())
    {
        return m_toUnicode[cid];
    }
    if (cid < m_encoding.size())
    {
        return m_encoding[cid];
    }

    return QChar();
}

PDFInteger PDFSimpleFont::getGlyphAdvance(size_t index) const
{
    const size_t min = m_firstChar;
    const size_t max = m_lastChar;

    if (index >= min && index <= max)
    {
        const size_t adjustedIndex = index - min;
        if (adjustedIndex < m_widths.size())
        {
            return m_widths[adjustedIndex];
        }
    }

    return 0;
}

QByteArray PDFSimpleFont::encodeCharacter(char32_t codePoint) const
{
    if (codePoint == 0 || codePoint > 0xFFFF)
    {
        // Simple fonts can produce BMP characters only during decoding
        return QByteArray();
    }

    const QChar character(static_cast<char16_t>(codePoint));

    // We compare against getUnicode(), because the forward pass (fillTextSequence)
    // emits getUnicode(cid) as the decoded character. First, prefer character codes
    // having a direct glyph index. Then accept any character code with matching
    // unicode - during rendering, the glyph is resolved by the same unicode fallback
    // (FT_Get_Char_Index) as in the forward pass.
    for (int i = 0; i < 256; ++i)
    {
        if (getUnicode(i) == character && m_glyphIndices[i] != GID())
        {
            return QByteArray(1, static_cast<char>(i));
        }
    }

    for (int i = 0; i < 256; ++i)
    {
        if (getUnicode(i) == character)
        {
            return QByteArray(1, static_cast<char>(i));
        }
    }

    return QByteArray();
}

void PDFSimpleFont::dumpFontToTreeItem(ITreeFactory* treeFactory) const
{
    BaseClass::dumpFontToTreeItem(treeFactory);

    QString encodingTypeString;
    switch (m_encodingType)
    {
       case PDFEncoding::Encoding::Standard:
            encodingTypeString = PDFTranslationContext::tr("Standard");
            break;

       case PDFEncoding::Encoding::MacRoman:
            encodingTypeString = PDFTranslationContext::tr("Mac Roman");
            break;

       case PDFEncoding::Encoding::WinAnsi:
            encodingTypeString = PDFTranslationContext::tr("Win Ansi");
            break;

       case PDFEncoding::Encoding::PDFDoc:
            encodingTypeString = PDFTranslationContext::tr("PDF Doc");
            break;

       case PDFEncoding::Encoding::MacExpert:
            encodingTypeString = PDFTranslationContext::tr("Mac Expert");
            break;

       case PDFEncoding::Encoding::Symbol:
            encodingTypeString = PDFTranslationContext::tr("Symbol");
            break;

       case PDFEncoding::Encoding::ZapfDingbats:
            encodingTypeString = PDFTranslationContext::tr("Zapf Dingbats");
            break;

       case PDFEncoding::Encoding::MacOsRoman:
            encodingTypeString = PDFTranslationContext::tr("Mac OS Roman");
            break;

       case PDFEncoding::Encoding::Custom:
            encodingTypeString = PDFTranslationContext::tr("Custom");
            break;

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    treeFactory->addItem({ PDFTranslationContext::tr("Encoding"), encodingTypeString });
}

PDFType1Font::PDFType1Font(FontType fontType,
                           QByteArray fontId,
                           CIDSystemInfo cidSystemInfo,
                           FontDescriptor fontDescriptor,
                           QByteArray name,
                           QByteArray baseFont,
                           PDFInteger firstChar,
                           PDFInteger lastChar,
                           std::vector<PDFInteger> widths,
                           PDFEncoding::Encoding encodingType,
                           encoding::EncodingTable encoding,
                           encoding::EncodingTable toUnicode,
                           bool hasToUnicode,
                           StandardFontType standardFontType,
                           GlyphIndices glyphIndices,
                           GlyphNames glyphNames) :
    PDFSimpleFont(qMove(cidSystemInfo), qMove(fontId), qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encodingType, encoding, toUnicode, hasToUnicode, standardFontType, glyphIndices, qMove(glyphNames)),
    m_fontType(fontType)
{

}

FontType PDFType1Font::getFontType() const
{
    return m_fontType;
}

void PDFType1Font::dumpFontToTreeItem(ITreeFactory* treeFactory) const
{
    BaseClass::dumpFontToTreeItem(treeFactory);

    if (m_standardFontType != StandardFontType::Invalid)
    {
        QString standardFontTypeString;
        switch (m_standardFontType)
        {
            case StandardFontType::TimesRoman:
            case StandardFontType::TimesRomanBold:
            case StandardFontType::TimesRomanItalics:
            case StandardFontType::TimesRomanBoldItalics:
                standardFontTypeString = PDFTranslationContext::tr("Times Roman");
                break;

            case StandardFontType::Helvetica:
            case StandardFontType::HelveticaBold:
            case StandardFontType::HelveticaOblique:
            case StandardFontType::HelveticaBoldOblique:
                standardFontTypeString = PDFTranslationContext::tr("Helvetica");
                break;

            case StandardFontType::Courier:
            case StandardFontType::CourierBold:
            case StandardFontType::CourierOblique:
            case StandardFontType::CourierBoldOblique:
                standardFontTypeString = PDFTranslationContext::tr("Courier");
                break;

            case StandardFontType::Symbol:
                standardFontTypeString = PDFTranslationContext::tr("Symbol");
                break;

            case StandardFontType::ZapfDingbats:
                standardFontTypeString = PDFTranslationContext::tr("Zapf Dingbats");
                break;

            default:
                Q_ASSERT(false);
                break;
        }

        treeFactory->addItem({ PDFTranslationContext::tr("Standard font"), standardFontTypeString });
    }
}

FontType PDFTrueTypeFont::getFontType() const
{
    return FontType::TrueType;
}

PDFFontCache::~PDFFontCache()
{
    clearTextDrawingApplicationFonts();
}

void PDFFontCache::clearTextDrawingApplicationFonts()
{
    for (int applicationFontId : m_textDrawingApplicationFontIds)
    {
        QFontDatabase::removeApplicationFont(applicationFontId);
    }
    m_textDrawingApplicationFontIds.clear();
}

void PDFFontCache::setDocument(const PDFModifiedDocument& document)
{
    QMutexLocker lock(&m_mutex);
    if (m_document != document)
    {
        m_document = document;

        // Jakub Melka: If document has not reset flag, then fonts of the
        // document remains the same. So it is not needed to clear font cache.
        if (document.hasReset() || document.hasPageContentsChanged())
        {
            m_fontCache.clear();
            m_realizedFontCache.clear();
            m_textDrawingFontCache.clear();
            clearTextDrawingApplicationFonts();
        }
    }
}

const PDFFontCache::TextDrawingFontInfo* PDFFontCache::getFontForTextDrawing(const PDFFontPointer& font, PDFRenderErrorReporter* reporter) const
{
    Q_ASSERT(font);

    QMutexLocker lock(&m_mutex);
    auto it = m_textDrawingFontCache.find(font);
    if (it == m_textDrawingFontCache.cend())
    {
        TextDrawingFontInfo info;

        if (font->getFontType() != FontType::Type3)
        {
            const FontDescriptor* descriptor = font->getFontDescriptor();
            bool hasExactFont = false;

            // Prefer registering the embedded font program directly, so glyph shapes
            // match the embedded/subset font exactly.
            const QByteArray& embeddedTrueTypeOrOpenType = !descriptor->fontFile2.isEmpty() ? descriptor->fontFile2 : descriptor->fontFile3;
            if (!embeddedTrueTypeOrOpenType.isEmpty())
            {
                const int applicationFontId = QFontDatabase::addApplicationFontFromData(embeddedTrueTypeOrOpenType);
                if (applicationFontId != -1)
                {
                    const QStringList families = QFontDatabase::applicationFontFamilies(applicationFontId);
                    if (!families.isEmpty())
                    {
                        info.font = QFont(families.front());
                        info.isUsable = true;
                        hasExactFont = true;
                        m_textDrawingApplicationFontIds.push_back(applicationFontId);
                    }
                    else
                    {
                        QFontDatabase::removeApplicationFont(applicationFontId);
                    }
                }
            }

            if (!hasExactFont)
            {
                // Embedded font program is missing (or is a Type 1 program, which is not
                // reliably registrable via QFontDatabase on all platforms) - substitute
                // a system font using the descriptor hints.
                QFont substituteFont(QString::fromLatin1(descriptor->fontFamily));
                substituteFont.setWeight(QFont::Weight(qBound(1, int(descriptor->fontWeight), 1000)));
                substituteFont.setStretch(descriptor->fontStretch);
                substituteFont.setItalic(descriptor->isItalic());
                substituteFont.setStyleHint(descriptor->isFixedPitch() ? QFont::Monospace :
                                             descriptor->isSerif() ? QFont::Serif : QFont::SansSerif);
                info.font = substituteFont;
                info.isUsable = true;

                if (reporter)
                {
                    reporter->reportRenderErrorOnce(RenderErrorType::Warning,
                                                     PDFTranslationContext::tr("Font '%1' is not embedded, using substitute font for real text drawing.").arg(QString::fromLatin1(descriptor->fontName)));
                }
            }
        }

        it = m_textDrawingFontCache.insert(std::make_pair(font, info)).first;
    }

    return &it->second;
}

PDFFontPointer PDFFontCache::getFont(const PDFObject& fontObject, const QByteArray& fontId) const
{
    if (fontObject.isReference())
    {
        // Font is object reference. Look in the cache, if we have it, then return it.

        QMutexLocker lock(&m_mutex);
        PDFObjectReference reference = fontObject.getReference();

        auto it = m_fontCache.find(reference);
        if (it == m_fontCache.cend())
        {
            // We must create the font
            PDFFontPointer font = PDFFont::createFont(fontObject, fontId, m_document);

            if (m_fontCacheShrinkDisabledObjects.empty() && m_fontCache.size() >= m_fontCacheLimit)
            {
                // We have exceeded the cache limit. Clear the cache.
                m_fontCache.clear();
            }

            it = m_fontCache.insert(std::make_pair(reference, qMove(font))).first;
        }
        return it->second;
    }
    else
    {
        // Object is not a reference. Create font directly and return it.
        return PDFFont::createFont(fontObject, fontId, m_document);
    }
}

PDFRealizedFontPointer PDFFontCache::getRealizedFont(const PDFFontPointer& font, PDFReal size, PDFRenderErrorReporter* reporter) const
{
    Q_ASSERT(font);

    QMutexLocker lock(&m_mutex);
    auto it = m_realizedFontCache.find(std::make_pair(font, size));
    if (it == m_realizedFontCache.cend())
    {
        // We must create the realized font
        PDFRealizedFontPointer realizedFont = PDFRealizedFont::createRealizedFont(font, size, reporter);

        if (m_fontCacheShrinkDisabledObjects.empty() && m_realizedFontCache.size() >= m_realizedFontCacheLimit)
        {
            m_realizedFontCache.clear();
        }

        it = m_realizedFontCache.insert(std::make_pair(std::make_pair(font, size), qMove(realizedFont))).first;
    }

    return it->second;
}

void PDFFontCache::setCacheShrinkEnabled(const void* source, bool enabled)
{
    QMutexLocker lock(&m_mutex);
    if (enabled)
    {
        m_fontCacheShrinkDisabledObjects.erase(source);
        lock.unlock();
        shrink();
    }
    else
    {
        m_fontCacheShrinkDisabledObjects.insert(source);
    }
}

void PDFFontCache::setCacheLimits(std::size_t fontCacheLimit, std::size_t instancedFontCacheLimit)
{
    if (m_fontCacheLimit != fontCacheLimit || m_realizedFontCacheLimit != instancedFontCacheLimit)
    {
        m_fontCacheLimit = fontCacheLimit;
        m_realizedFontCacheLimit = instancedFontCacheLimit;
        shrink();
    }
}

void PDFFontCache::shrink()
{
    QMutexLocker lock(&m_mutex);
    if (m_fontCacheShrinkDisabledObjects.empty())
    {
        if (m_fontCache.size() >= m_fontCacheLimit)
        {
            m_fontCache.clear();
        }
        if (m_realizedFontCache.size() >= m_realizedFontCacheLimit)
        {
            m_realizedFontCache.clear();
        }
    }
}

const QByteArray* FontDescriptor::getEmbeddedFontData() const
{
    if (!fontFile.isEmpty())
    {
        return &fontFile;
    }
    else if (!fontFile2.isEmpty())
    {
        return &fontFile2;
    }
    else if (!fontFile3.isEmpty())
    {
        return &fontFile3;
    }

    return nullptr;
}

PDFFontCMap PDFFontCMap::createFromName(const QByteArray& name)
{
    QFile file(QString(":/cmaps/%1").arg(QString::fromLatin1(name)));
    if (file.exists())
    {
        QByteArray data;
        if (file.open(QFile::ReadOnly))
        {
            data = file.readAll();
            file.close();
        }

        PDFFontCMap result = createFromData(data);
        result.m_unicodeEncoded = name.contains("-UCS2-");
        return result;
    }

    throw PDFException(PDFTranslationContext::tr("Can't load CID font mapping named '%1'.").arg(QString::fromLatin1(name)));
}

PDFFontCMap PDFFontCMap::createFromData(const QByteArray& data)
{
    Entries entries;
    entries.reserve(1024); // Arbitrary number, we have enough memory, better than perform reallocation each time

    std::vector<PDFFontCMap> additionalMappings;
    PDFLexicalAnalyzer parser(data.constBegin(), data.constEnd());

    bool vertical = false;
    PDFLexicalAnalyzer::Token previousToken;
    while (!parser.isAtEnd())
    {
        PDFLexicalAnalyzer::Token token = parser.fetch();

        if (token.type == PDFLexicalAnalyzer::TokenType::Name && token.data.toByteArray() == "WMode")
        {
            PDFLexicalAnalyzer::Token valueToken = parser.fetch();
            vertical = valueToken.type == PDFLexicalAnalyzer::TokenType::Integer && valueToken.data.value<PDFInteger>() == 1;
            continue;
        }

        auto fetchCode = [] (const PDFLexicalAnalyzer::Token& currentToken) -> std::pair<unsigned int, unsigned int>
        {
            if (currentToken.type == PDFLexicalAnalyzer::TokenType::String)
            {
                QByteArray byteArray = currentToken.data.toByteArray();

                unsigned int codeValue = 0;
                for (int i = 0; i < byteArray.size(); ++i)
                {
                    codeValue = (codeValue << 8) + static_cast<unsigned char>(byteArray[i]);
                }

                return std::make_pair(codeValue, byteArray.size());
            }

            throw PDFException(PDFTranslationContext::tr("Can't fetch code from CMap definition."));
        };

        auto fetchCID = [] (const PDFLexicalAnalyzer::Token& currentToken) -> CID
        {
            if (currentToken.type == PDFLexicalAnalyzer::TokenType::Integer)
            {
                return currentToken.data.value<PDFInteger>();
            }

            throw PDFException(PDFTranslationContext::tr("Can't fetch CID from CMap definition."));
        };

        auto fetchUnicode = [](const PDFLexicalAnalyzer::Token& currentToken) -> CID
        {
            if (currentToken.type == PDFLexicalAnalyzer::TokenType::String)
            {
                QByteArray byteArray = currentToken.data.toByteArray();

                if (byteArray.size() == 2)
                {
                    CID unicodeValue = 0;
                    for (int i = 0; i < byteArray.size(); ++i)
                    {
                        unicodeValue = (unicodeValue << 8) + static_cast<unsigned char>(byteArray[i]);
                    }
                    return unicodeValue;
                }
            }

            return 0;
        };

        if (token.type == PDFLexicalAnalyzer::TokenType::Command)
        {
            QByteArray command = token.data.toByteArray();
            if (command == "usecmap")
            {
                if (previousToken.type == PDFLexicalAnalyzer::TokenType::Name)
                {
                    additionalMappings.emplace_back(createFromName(previousToken.data.toByteArray()));
                }
                else
                {
                    throw PDFException(PDFTranslationContext::tr("Can't use cmap inside cmap file."));
                }
            }
            else if (command == "beginbfrange")
            {
                while (true)
                {
                    PDFLexicalAnalyzer::Token token1 = parser.fetch();

                    if (token1.type == PDFLexicalAnalyzer::TokenType::Command &&
                            token1.data.toByteArray() == "endbfrange")
                    {
                        break;
                    }

                    PDFLexicalAnalyzer::Token token2 = parser.fetch();
                    PDFLexicalAnalyzer::Token token3 = parser.fetch();

                    std::pair<unsigned int, unsigned int> from = fetchCode(token1);
                    std::pair<unsigned int, unsigned int> to = fetchCode(token2);

                    if (token3.type == PDFLexicalAnalyzer::TokenType::ArrayStart)
                    {
                        unsigned int current = from.first;

                        while (true)
                        {
                            PDFLexicalAnalyzer::Token arrayToken = parser.fetch();

                            // Do we have end of array?
                            if (arrayToken.type == PDFLexicalAnalyzer::TokenType::ArrayEnd)
                            {
                                break;
                            }

                            CID cid = fetchUnicode(arrayToken);
                            entries.emplace_back(current, current, qMax(from.second, to.second), cid);
                            ++current;
                        }
                    }
                    else
                    {
                        CID cid = fetchUnicode(token3);
                        entries.emplace_back(from.first, to.first, qMax(from.second, to.second), cid);
                    }
                }
            }
            else if (command == "begincidrange")
            {
                while (true)
                {
                    PDFLexicalAnalyzer::Token token1 = parser.fetch();

                    if (token1.type == PDFLexicalAnalyzer::TokenType::Command &&
                        token1.data.toByteArray() == "endcidrange")
                    {
                        break;
                    }

                    PDFLexicalAnalyzer::Token token2 = parser.fetch();
                    PDFLexicalAnalyzer::Token token3 = parser.fetch();

                    std::pair<unsigned int, unsigned int> from = fetchCode(token1);
                    std::pair<unsigned int, unsigned int> to = fetchCode(token2);
                    CID cid = fetchCID(token3);

                    entries.emplace_back(from.first, to.first, qMax(from.second, to.second), cid);
                }
            }
            else if (command == "begincidchar")
            {
                while (true)
                {
                    PDFLexicalAnalyzer::Token token1 = parser.fetch();

                    if (token1.type == PDFLexicalAnalyzer::TokenType::Command &&
                        token1.data.toByteArray() == "endcidchar")
                    {
                        break;
                    }

                    PDFLexicalAnalyzer::Token token2 = parser.fetch();

                    std::pair<unsigned int, unsigned int> code = fetchCode(token1);
                    CID cid = fetchCID(token2);

                    entries.emplace_back(code.first, code.first, code.second, cid);
                }
            }
            else if (command == "beginbfchar")
            {
                while (true)
                {
                    PDFLexicalAnalyzer::Token token1 = parser.fetch();

                    if (token1.type == PDFLexicalAnalyzer::TokenType::Command &&
                        token1.data.toByteArray() == "endbfchar")
                    {
                        break;
                    }

                    PDFLexicalAnalyzer::Token token2 = parser.fetch();

                    std::pair<unsigned int, unsigned int> code = fetchCode(token1);
                    CID cid = fetchUnicode(token2);

                    entries.emplace_back(code.first, code.first, code.second, cid);
                }
            }
        }

        previousToken = token;
    }

    if (!additionalMappings.empty())
    {
        const Entries overridingEntries = entries;
        for (const PDFFontCMap& map : additionalMappings)
        {
            for (const Entry& inheritedEntry : map.m_entries)
            {
                std::vector<std::pair<unsigned int, unsigned int>> ranges{ { inheritedEntry.from, inheritedEntry.to } };

                for (const Entry& overridingEntry : overridingEntries)
                {
                    if (overridingEntry.byteCount != inheritedEntry.byteCount)
                    {
                        continue;
                    }

                    std::vector<std::pair<unsigned int, unsigned int>> updatedRanges;
                    for (const auto& range : ranges)
                    {
                        const unsigned int rangeFrom = range.first;
                        const unsigned int rangeTo = range.second;

                        if (overridingEntry.to < rangeFrom || overridingEntry.from > rangeTo)
                        {
                            updatedRanges.emplace_back(range);
                            continue;
                        }

                        if (overridingEntry.from > rangeFrom)
                        {
                            updatedRanges.emplace_back(rangeFrom, overridingEntry.from - 1);
                        }
                        if (overridingEntry.to < rangeTo)
                        {
                            updatedRanges.emplace_back(overridingEntry.to + 1, rangeTo);
                        }
                    }
                    ranges = qMove(updatedRanges);

                    if (ranges.empty())
                    {
                        break;
                    }
                }

                for (const auto& range : ranges)
                {
                    entries.emplace_back(range.first, range.second, inheritedEntry.byteCount, inheritedEntry.cid + range.first - inheritedEntry.from);
                }
            }
        }
    }

    std::sort(entries.begin(), entries.end());
    entries = optimize(entries);

    return PDFFontCMap(qMove(entries), vertical, false);
}

QByteArray PDFFontCMap::serialize() const
{
    QByteArray result;

    {
        QDataStream stream(&result, QIODevice::WriteOnly);
        stream << m_maxKeyLength;
        stream << m_vertical;
        stream << m_unicodeEncoded;
        stream << m_entries.size();
        for (const Entry& entry : m_entries)
        {
            stream << entry.from;
            stream << entry.to;
            stream << entry.byteCount;
            stream << entry.cid;
        }
    }

    return qCompress(result, 9);
}

PDFFontCMap PDFFontCMap::deserialize(const QByteArray& byteArray)
{
    PDFFontCMap result;
    QByteArray decompressed = qUncompress(byteArray);
    QDataStream stream(&decompressed, QIODevice::ReadOnly);
    stream >> result.m_maxKeyLength;
    stream >> result.m_vertical;
    stream >> result.m_unicodeEncoded;

    Entries::size_type size = 0;
    stream >> size;
    result.m_entries.reserve(size);
    for (Entries::size_type i = 0; i < size; ++i)
    {
        Entry entry;
        stream >> entry.from;
        stream >> entry.to;
        stream >> entry.byteCount;
        stream >> entry.cid;
        result.m_entries.push_back(entry);
    }

    return result;
}

std::vector<CID> PDFFontCMap::interpret(const QByteArray& byteArray) const
{
    std::vector<CID> result;
    const std::vector<MappedCode> mappedCodes = interpretWithCode(byteArray);
    result.reserve(mappedCodes.size());

    for (const MappedCode& mappedCode : mappedCodes)
    {
        result.push_back(mappedCode.cid);
    }

    return result;
}

std::vector<PDFFontCMap::MappedCode> PDFFontCMap::interpretWithCode(const QByteArray& byteArray) const
{
    std::vector<MappedCode> result;
    result.reserve(byteArray.size() / m_maxKeyLength);

    unsigned int value = 0;
    unsigned int scannedBytes = 0;

    for (int i = 0, size = byteArray.size(); i < size; ++i)
    {
        value = (value << 8) + static_cast<unsigned char>(byteArray[i]);
        ++scannedBytes;

        // Find suitable mapping
        auto it = std::find_if(m_entries.cbegin(), m_entries.cend(), [value, scannedBytes](const Entry& entry) { return entry.from <= value && entry.to >= value && entry.byteCount == scannedBytes; });
        if (it != m_entries.cend())
        {
            const Entry& entry = *it;
            const CID cid = value - entry.from + entry.cid;
            result.push_back(MappedCode{ cid, value, scannedBytes });

            value = 0;
            scannedBytes = 0;
        }
        else if (scannedBytes == m_maxKeyLength)
        {
            // This means error occured - fill empty CID
            result.push_back(MappedCode{ 0, value, scannedBytes });
            value = 0;
            scannedBytes = 0;
        }
    }

    return result;
}

QByteArray PDFFontCMap::encode(CID cid) const
{
    QByteArray byteArray;

    for (const auto& entry : m_entries)
    {
        // Entry maps codes [from, to] to CIDs [cid, cid + (to - from)]
        unsigned int minPossibleValue = entry.cid;
        unsigned int maxPossibleValue = entry.cid + (entry.to - entry.from);

        if (cid >= minPossibleValue && cid <= maxPossibleValue)
        {
            // Calculate the original value from cid
            unsigned int value = cid - entry.cid + entry.from;

            byteArray.reserve(entry.byteCount);

            // Construct byte array for this value based on the entry's byteCount
            for (int i = entry.byteCount - 1; i >= 0; --i)
            {
                byteArray.append(static_cast<char>((value >> (8 * i)) & 0xFF));
            }

            break;
        }
    }

    return byteArray;
}

QChar PDFFontCMap::getToUnicode(CID cid) const
{
    return getToUnicode(cid, 0);
}

QChar PDFFontCMap::getToUnicode(CID cid, unsigned int byteCount) const
{
    if (isValid())
    {
        auto it = std::find_if(m_entries.cbegin(), m_entries.cend(), [cid, byteCount](const Entry& entry) {
            const bool byteCountMatches = byteCount == 0 || entry.byteCount == byteCount;
            return byteCountMatches && entry.from <= cid && entry.to >= cid;
        });
        if (it != m_entries.cend())
        {
            const Entry& entry = *it;
            const CID unicodeCID = cid - entry.from + entry.cid;
            return QChar(unicodeCID);
        }
    }

    return QChar();
}

CID PDFFontCMap::getFromUnicode(QChar character) const
{
    if (!character.isNull())
    {
        char16_t ucs4 = character.unicode();
        const CID unicodeCID = ucs4;

        for (const Entry& entry : m_entries)
        {
            const CID minUnicodeCID = entry.cid;
            const CID maxUnicodeCID = (entry.to - entry.from) + entry.cid;

            if (unicodeCID >= minUnicodeCID && unicodeCID <= maxUnicodeCID)
            {
                const CID cid = unicodeCID + entry.from - entry.cid;
                return cid;
            }
        }
    }

    return CID();
}

QChar PDFFontCMap::getUnicodeFromCode(unsigned int code) const
{
    return m_unicodeEncoded ? QChar(code) : QChar();
}

void PDFFontCMap::enumerate(const std::function<void(unsigned int, unsigned int, CID)>& callback) const
{
    // Clamp enumeration of a single entry, so pathological CMaps with huge
    // code ranges (e.g. 4-byte ranges) do not lock up the enumeration.
    constexpr unsigned int maxEnumeratedCodesPerEntry = 65536;

    for (const Entry& entry : m_entries)
    {
        const unsigned int rangeSize = entry.to - entry.from;
        const unsigned int lastCode = (rangeSize < maxEnumeratedCodesPerEntry) ? entry.to : entry.from + maxEnumeratedCodesPerEntry - 1;

        // 64-bit loop counter to avoid overflow when lastCode == UINT_MAX
        for (quint64 code = entry.from; code <= lastCode; ++code)
        {
            const unsigned int codeValue = static_cast<unsigned int>(code);
            callback(codeValue, entry.byteCount, CID(codeValue - entry.from + entry.cid));
        }
    }
}

bool PDFFontCMap::containsCode(unsigned int code, unsigned int byteCount) const
{
    return std::any_of(m_entries.cbegin(), m_entries.cend(), [code, byteCount](const Entry& entry) { return entry.from <= code && entry.to >= code && entry.byteCount == byteCount; });
}

PDFFontCMap::PDFFontCMap(Entries&& entries, bool vertical, bool unicodeEncoded) :
    m_entries(qMove(entries)),
    m_maxKeyLength(0),
    m_vertical(vertical),
    m_unicodeEncoded(unicodeEncoded)
{
    m_maxKeyLength = std::accumulate(m_entries.cbegin(), m_entries.cend(), 0, [](unsigned int a, const Entry& b) { return qMax(a, b.byteCount); });
}

PDFFontCMap::Entries PDFFontCMap::optimize(const PDFFontCMap::Entries& entries)
{
    Entries result;
    result.reserve(entries.size());

    if (!entries.empty())
    {
        Entry current = entries.front();
        for (size_t i = 1, count = entries.size(); i < count; ++i)
        {
            Entry toMerge = entries[i];

            if (current.canMerge(toMerge))
            {
                current = current.merge(toMerge);
            }
            else
            {
                result.emplace_back(current);
                current = toMerge;
            }
        }
        result.emplace_back(current);
    }

    result.shrink_to_fit();
    return result;
}

PDFFontCMapRepository* PDFFontCMapRepository::getInstance()
{
    static PDFFontCMapRepository repository;
    return &repository;
}

void PDFFontCMapRepository::saveToFile(const QString& fileName) const
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        size_t size = m_cmaps.size();

        {
            QDataStream stream(&file);
            stream << size;
            for (const auto& item : m_cmaps)
            {
                stream << item.first;
                stream << item.second;
            }
        }

        file.close();
    }
}

bool PDFFontCMapRepository::loadFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
        {
            QDataStream stream(&file);
            size_t size = 0;
            stream >> size;
            for (size_t i = 0; i < size; ++i)
            {
                QByteArray key;
                QByteArray value;
                stream >> key;
                stream >> value;
                m_cmaps[qMove(key)] = qMove(value);
            }
        }

        file.close();
        return true;
    }

    return false;
}

PDFFontCMapRepository::PDFFontCMapRepository()
{

}

PDFReal PDFType0Font::getGlyphAdvance(CID cid) const
{
    auto it = m_advances.find(cid);
    if (it != m_advances.cend())
    {
        return it->second;
    }

    return m_defaultAdvance;
}

QByteArray PDFType0Font::encodeCharacter(char32_t codePoint) const
{
    std::call_once(m_encodeMapFlag, &PDFType0Font::buildEncodeMap, this);

    auto it = m_encodeMap.find(codePoint);
    if (it != m_encodeMap.cend())
    {
        return it->second;
    }

    return QByteArray();
}

void PDFType0Font::buildEncodeMap() const
{
    auto serializeCode = [](unsigned int code, unsigned int byteCount)
    {
        QByteArray bytes;
        bytes.reserve(byteCount);
        for (int i = static_cast<int>(byteCount) - 1; i >= 0; --i)
        {
            bytes.append(static_cast<char>((code >> (8 * i)) & 0xFF));
        }
        return bytes;
    };

    // Pass 1: enumerate the ToUnicode CMap. Character code is accepted, if the encoding
    // CMap decodes it, or if it has maximal code length - in that case, interpretWithCode
    // consumes it as a whole code unit even without a matching entry (error path), and
    // the ToUnicode character is still produced by the forward pass.
    const unsigned int maxKeyLength = m_cmap.getMaxKeyLength();
    m_toUnicode.enumerate([&, this](unsigned int code, unsigned int byteCount, CID unicodeValue)
    {
        if (unicodeValue == 0)
        {
            return;
        }

        const char32_t codePoint = unicodeValue;
        if (m_encodeMap.count(codePoint))
        {
            return;
        }

        if (!m_cmap.containsCode(code, byteCount) && byteCount != maxKeyLength)
        {
            return;
        }

        // Overlapping ToUnicode entries: the forward pass uses the first matching
        // entry, so verify that decoding this code really produces our character.
        if (m_toUnicode.getToUnicode(code, byteCount) != QChar(static_cast<char16_t>(unicodeValue)))
        {
            return;
        }

        m_encodeMap.emplace(codePoint, serializeCode(code, byteCount));
    });

    // Pass 2: for unicode encoded predefined CMaps of non-embedded fonts, the forward
    // pass falls back to interpreting the character code directly as unicode, but only
    // when the ToUnicode CMap gave no character for the code.
    if (!m_fontDescriptor.isEmbedded() && m_cmap.isUnicodeEncoded())
    {
        m_cmap.enumerate([&, this](unsigned int code, unsigned int byteCount, CID)
        {
            if (code == 0 || code > 0xFFFF)
            {
                return;
            }

            const char32_t codePoint = code;
            if (m_encodeMap.count(codePoint))
            {
                return;
            }

            if (!m_toUnicode.getToUnicode(code, byteCount).isNull())
            {
                return;
            }

            m_encodeMap.emplace(codePoint, serializeCode(code, byteCount));
        });
    }
}

PDFType3Font::PDFType3Font(FontDescriptor fontDescriptor,
                           QByteArray fontId,
                           int firstCharacterIndex,
                           int lastCharacterIndex,
                           QTransform fontMatrix,
                           std::map<int, QByteArray>&& characterContentStreams,
                           std::vector<double>&& widths,
                           const PDFObject& resources,
                           PDFFontCMap toUnicode) :
    PDFFont(CIDSystemInfo(), qMove(fontId), qMove(fontDescriptor)),
    m_firstCharacterIndex(firstCharacterIndex),
    m_lastCharacterIndex(lastCharacterIndex),
    m_fontMatrix(fontMatrix),
    m_characterContentStreams(qMove(characterContentStreams)),
    m_widths(qMove(widths)),
    m_resources(resources),
    m_toUnicode(qMove(toUnicode))
{

}

FontType PDFType3Font::getFontType() const
{
    return FontType::Type3;
}

void PDFType3Font::dumpFontToTreeItem(ITreeFactory* treeFactory) const
{
    treeFactory->addItem({ PDFTranslationContext::tr("Character count"), QString::number(m_characterContentStreams.size()) });
}

double PDFType3Font::getWidth(int characterIndex) const
{
    if (characterIndex >= m_firstCharacterIndex && characterIndex <= m_lastCharacterIndex)
    {
        size_t index = characterIndex - m_firstCharacterIndex;
        if (index < m_widths.size())
        {
            return m_widths[index];
        }
    }

    return 0.0;
}

const QByteArray* PDFType3Font::getContentStream(int characterIndex) const
{
    auto it = m_characterContentStreams.find(characterIndex);
    if (it != m_characterContentStreams.cend())
    {
        return &it->second;
    }

    return nullptr;
}

QByteArray PDFType3Font::encodeCharacter(char32_t codePoint) const
{
    if (codePoint == 0 || codePoint > 0xFFFF)
    {
        return QByteArray();
    }

    const QChar character(static_cast<char16_t>(codePoint));

    // Prefer character codes having a content stream
    for (const auto& item : m_characterContentStreams)
    {
        if (item.first >= 0 && item.first <= 255 && getUnicode(item.first) == character)
        {
            return QByteArray(1, static_cast<char>(item.first));
        }
    }

    for (int i = 1; i < 256; ++i)
    {
        if (getUnicode(i) == character)
        {
            return QByteArray(1, static_cast<char>(i));
        }
    }

    return QByteArray();
}

void PDFRealizedType3FontImpl::fillTextSequence(const QByteArray& byteArray, TextSequence& textSequence, PDFRenderErrorReporter* reporter)
{
    Q_ASSERT(dynamic_cast<const PDFType3Font*>(m_parentFont.get()));
    const PDFType3Font* parentFont = static_cast<const PDFType3Font*>(m_parentFont.get());

    textSequence.items.reserve(byteArray.size());
    for (int i = 0, characterCount = byteArray.size(); i < characterCount; ++i)
    {
        int index = static_cast<uint8_t>(byteArray[i]);
        const QByteArray* contentStream = parentFont->getContentStream(index);
        QChar character = parentFont->getUnicode(index);
        const double width = parentFont->getWidth(index);

        if (contentStream)
        {
            textSequence.items.emplace_back(contentStream, character, width, index);
        }
        else
        {
            // Report error, and add advance, if we have it
            reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Content stream for type 3 font character code '%1' not found.").arg(index));

            if (width > 0.0)
            {
                textSequence.items.emplace_back(width);
            }
        }
    }
}

bool PDFRealizedType3FontImpl::isHorizontalWritingSystem() const
{
    return true;
}

CharacterInfos PDFRealizedType3FontImpl::getCharacterInfos() const
{
    CharacterInfos result;

    Q_ASSERT(dynamic_cast<const PDFType3Font*>(m_parentFont.get()));
    const PDFType3Font* parentFont = static_cast<const PDFType3Font*>(m_parentFont.get());

    for (const auto& contentStreamItem : parentFont->getContentStreams())
    {
        CharacterInfo info;
        info.gid = contentStreamItem.first;
        info.character = parentFont->getUnicode(contentStreamItem.first);
        result.emplace_back(qMove(info));
    }

    return result;
}

QByteArray PDFSystemFont::getFontData(const QByteArray& fontName)
{
    const PDFSystemFontInfoStorage* storage = PDFSystemFontInfoStorage::getInstance();

    CIDSystemInfo systemInfo;
    PDFRenderErrorReporterDummy reporter;
    FontDescriptor descriptor;
    descriptor.fontName = fontName;

    return storage->loadFont(&systemInfo, &descriptor, StandardFontType::Invalid, &reporter).data;
}

}   // namespace pdf
