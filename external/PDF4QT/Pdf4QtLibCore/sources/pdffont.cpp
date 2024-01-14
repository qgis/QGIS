//    Copyright (C) 2019-2022 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

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
#include <QMutex>
#include <QReadWriteLock>
#include <QPainterPath>
#include <QDataStream>

#include "pdfdbgheap.h"

#if defined(Q_OS_WIN)
#include "Windows.h"
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

    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeCNS, true, "Ming" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeCNS, false, "Fangti" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeCNS, true, "MingLiU" },

    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeJapan, false, "Gothic" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeJapan, true, "Mincho" },

    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeKorea, false, "Gulim" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeKorea, false, "Dotum" },
    PDF_Default_CJK_Font{ ECjkDefaultFontType::AdobeKorea, true, "Batang" },
};

/// Storage class for system fonts
class PDFSystemFontInfoStorage
{
public:

    /// Returns instance of storage
    static const PDFSystemFontInfoStorage* getInstance();

    /// Loads font from descriptor
    /// \param descriptor Descriptor describing the font
    QByteArray loadFont(const CIDSystemInfo* cidSystemInfo,
                        const FontDescriptor* descriptor,
                        StandardFontType standardFontType,
                        PDFRenderErrorReporter* reporter) const;

private:
    explicit PDFSystemFontInfoStorage();

    /// Loads font from descriptor
    /// \param descriptor Descriptor describing the font
    QByteArray loadFontImpl(const FontDescriptor* descriptor,
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

QByteArray PDFSystemFontInfoStorage::loadFont(const CIDSystemInfo* cidSystemInfo,
                                              const FontDescriptor* descriptor,
                                              StandardFontType standardFontType,
                                              PDFRenderErrorReporter* reporter) const
{
    QString fontName;

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

    QByteArray fontData = loadFontImpl(descriptor, fontName, standardFontType, reporter);

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

    return fontData;
}

QByteArray PDFSystemFontInfoStorage::loadFontImpl(const FontDescriptor* descriptor,
                                                  QString fontName,
                                                  StandardFontType standardFontType,
                                                  PDFRenderErrorReporter* reporter) const
{
    QByteArray result;

#if defined(Q_OS_WIN)

    Q_UNUSED(standardFontType);
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
                result = getFontData(&fontInfo.logFont, hdc);

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
                    result = getFontData(&logFont, hdc);

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
                result = getFontData(&fontInfo.logFont, hdc);

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
                    result = getFontData(&logFont, hdc);

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
                result = getFontData(&logFont, hdc);

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
            f.open(QIODevice::ReadOnly);
            result = f.readAll();
            f.close();
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

    CallbackInfo callbackInfo{ this, hdc};
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

QByteArray PDFSystemFontInfoStorage::getFontData(const LOGFONT* font, HDC hdc)
{
    QByteArray byteArray;

    if (HFONT fontHandle = ::CreateFontIndirect(font))
    {
        HGDIOBJ oldFont = ::SelectObject(hdc, fontHandle);

        DWORD size = ::GetFontData(hdc, 0, 0, nullptr, 0);
        if (size != GDI_ERROR)
        {
            byteArray.resize(static_cast<int>(size));
            ::GetFontData(hdc, 0, 0, byteArray.data(), byteArray.size());
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

PDFFont::PDFFont(CIDSystemInfo CIDSystemInfo, FontDescriptor fontDescriptor) :
    m_CIDSystemInfo(qMove(CIDSystemInfo)),
    m_fontDescriptor(qMove(fontDescriptor))
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
            const encoding::EncodingTable* encoding = font->getEncoding();
            const GlyphIndices* glyphIndices = font->getGlyphIndices();

            textSequence.items.reserve(textSequence.items.size() + byteArray.size());
            for (int i = 0, count = byteArray.size(); i < count; ++i)
            {
                GID glyphIndex = (*glyphIndices)[static_cast<uint8_t>(byteArray[i])];

                if (!glyphIndex)
                {
                    // Try to obtain glyph index from unicode
                    if (m_face->charmap && m_face->charmap->encoding == FT_ENCODING_UNICODE)
                    {
                        glyphIndex = FT_Get_Char_Index(m_face, (*encoding)[static_cast<uint8_t>(byteArray[i])].unicode());
                    }
                }

                const PDFReal glyphWidth = font->getGlyphAdvance(static_cast<uint8_t>(byteArray[i]));

                if (glyphIndex)
                {
                    const Glyph& glyph = getGlyph(glyphIndex);
                    textSequence.items.emplace_back(&glyph.glyph, (*encoding)[static_cast<uint8_t>(byteArray[i])], glyph.advance);
                }
                else
                {
                    reporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Glyph for simple font character code '%1' not found.").arg(static_cast<uint8_t>(byteArray[i])));
                    if (glyphWidth > 0)
                    {
                        const QPainterPath* nullpath = nullptr;
                        textSequence.items.emplace_back(nullpath, QChar(), glyphWidth * m_pixelSize * FONT_WIDTH_MULTIPLIER);
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

            std::vector<CID> cids = cmap->interpret(byteArray);
            textSequence.items.reserve(textSequence.items.size() + cids.size());
            for (CID cid : cids)
            {
                const GID glyphIndex = CIDtoGIDmapper->map(cid);
                const PDFReal glyphWidth = font->getGlyphAdvance(cid);

                if (glyphIndex)
                {
                    QChar character = toUnicode->getToUnicode(cid);
                    const Glyph& glyph = getGlyph(glyphIndex);
                    textSequence.items.emplace_back(&glyph.glyph, character, glyph.advance);
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
                        textSequence.items.emplace_back(nullpath, QChar(), -glyphWidth);
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
            const encoding::EncodingTable* encoding = font->getEncoding();
            const GlyphIndices* glyphIndices = font->getGlyphIndices();

            for (size_t i = 0; i < encoding->size(); ++i)
            {
                QChar character = (*encoding)[i];
                GID glyphIndex = (*glyphIndices)[static_cast<uint8_t>(i)];

                if (!glyphIndex)
                {
                    // Try to obtain glyph index from unicode
                    if (m_face->charmap && m_face->charmap->encoding == FT_ENCODING_UNICODE)
                    {
                        glyphIndex = FT_Get_Char_Index(m_face, character.unicode());
                    }
                }

                if (glyphIndex)
                {
                    CharacterInfo info;
                    info.gid = glyphIndex;
                    info.character = character;
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
                    const GID gid = CIDtoGIDmapper->map(cid);

                    if (!gid)
                    {
                        continue;
                    }

                    if (!FT_Load_Glyph(m_face, gid, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING))
                    {
                        CharacterInfo info;
                        info.gid = gid;
                        info.character = toUnicode->getToUnicode(cid);
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
    if (glyphIndex)
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

    static Glyph dummy;
    return dummy;
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
            if (font->getFontType() == FontType::Type1 || font->getFontType() == FontType::MMType1)
            {
                Q_ASSERT(dynamic_cast<const PDFType1Font*>(font.get()));
                const PDFType1Font* type1Font = static_cast<const PDFType1Font*>(font.get());
                standardFontType = type1Font->getStandardFontType();
            }

            const PDFSystemFontInfoStorage* fontStorage = PDFSystemFontInfoStorage::getInstance();
            impl->m_systemFontData = fontStorage->loadFont(font->getCIDSystemInfo(), descriptor, standardFontType, reporter);

            if (impl->m_systemFontData.isEmpty())
            {
                throw PDFException(PDFTranslationContext::tr("Can't load system font '%1'.").arg(QString::fromLatin1(descriptor->fontName)));
            }

            PDFRealizedFontImpl::checkFreeTypeError(FT_Init_FreeType(&impl->m_library));
            PDFRealizedFontImpl::checkFreeTypeError(FT_New_Memory_Face(impl->m_library, reinterpret_cast<const FT_Byte*>(impl->m_systemFontData.constData()), impl->m_systemFontData.size(), 0, &impl->m_face));
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

PDFFontPointer PDFFont::createFont(const PDFObject& object, const PDFDocument* document)
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
    constexpr const std::array<std::pair<const char*, StandardFontType>, 14> standardFonts = {
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
        std::pair<const char*, StandardFontType>{ "ZapfDingbats", StandardFontType::ZapfDingbats }
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
    GlyphIndices glyphIndexArray = { };
    switch (fontType)
    {
        case FontType::Type1:
        case FontType::MMType1:
        case FontType::TrueType:
        {
            bool hasDifferences = false;
            encoding::EncodingTable differences = { };

            if (fontDictionary->hasKey("Encoding"))
            {
                constexpr const std::array<std::pair<const char*, PDFEncoding::Encoding>, 3> encodings = {
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
                        // We get encoding for the standard font. If we have invalid standard font,
                        // then we get standard encoding. So we shouldn't test it.
                        encoding = getEncodingForStandardFont(standardFont);
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

                                    QChar character = PDFNameToUnicode::getUnicodeUsingResolvedName(item.getString());
                                    differences[currentOffset] = character;

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
            else if (fontDictionary->hasKey("ToUnicode"))
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

                const bool hasToUnicode = toUnicodeCMap.isValid();
                if (hasToUnicode)
                {
                    for (size_t i = 0; i < differences.size(); ++i)
                    {
                        QChar character = toUnicodeCMap.getToUnicode(static_cast<CID>(i));

                        if (!character.isNull())
                        {
                            differences[i] = character;
                            hasDifferences = true;
                        }
                    }
                }
            }

            if (encoding == PDFEncoding::Encoding::Invalid)
            {
                // We get encoding for the standard font. If we have invalid standard font,
                // then we get standard encoding. So we shouldn't test it.
                encoding = getEncodingForStandardFont(standardFont);
            }

            if (encoding == PDFEncoding::Encoding::Invalid)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid encoding entry of the font."));
            }

            simpleFontEncodingTable = *PDFEncoding::getTableForEncoding(encoding);

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
                    }

                    // Set the encoding to custom
                    encoding = PDFEncoding::Encoding::Custom;
                }

                // Fill in missing characters from standard encoding
                const encoding::EncodingTable& standardEncoding = *PDFEncoding::getTableForEncoding(PDFEncoding::Encoding::Standard);
                for (size_t i = 0; i < standardEncoding.size(); ++i)
                {
                    if ((simpleFontEncodingTable[i].isNull() || simpleFontEncodingTable[i] == QChar(QChar::SpecialCharacter::ReplacementCharacter)) &&
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
                                                        QChar character = PDFNameToUnicode::getUnicodeForName(byteArrayBuffer);
                                                        if (character.isNull())
                                                        {
                                                            character = PDFNameToUnicode::getUnicodeForNameZapfDingbats(byteArrayBuffer);
                                                        }
                                                        if (!character.isNull())
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
                                FT_UInt glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(i));
                                if (glyphIndex > 0)
                                {
                                    glyphIndexArray[i] = glyphIndex;
                                }
                            }
                        }

                        finishFont();

                        // Fill the glyph index array from unicode, if we have unicode mapping
                        if (!FT_Select_Charmap(face, FT_ENCODING_UNICODE))
                        {
                            for (size_t i = 0; i < simpleFontEncodingTable.size(); ++i)
                            {
                                QChar character = simpleFontEncodingTable[i];
                                if (!character.isNull() && character != QChar(QChar::SpecialCharacter::ReplacementCharacter))
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

            return PDFFontPointer(new PDFType0Font(qMove(cidSystemInfo), qMove(fontDescriptor), qMove(cmap), qMove(toUnicodeCMap), qMove(cidToGidMapper), defaultWidth, qMove(advances)));
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
            return PDFFontPointer(new PDFType3Font(qMove(fontDescriptor), firstCharF3, lastCharF3, fontMatrix, qMove(characterContentStreams), qMove(widthsF3), document->getObject(fontDictionary->get("Resources")), qMove(toUnicodeCMap)));
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
            return PDFFontPointer(new PDFType1Font(fontType, qMove(cidSystemInfo), qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encoding, simpleFontEncodingTable, standardFont, glyphIndexArray));

        case FontType::TrueType:
            return PDFFontPointer(new PDFTrueTypeFont(qMove(cidSystemInfo), qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encoding, simpleFontEncodingTable, glyphIndexArray));

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return PDFFontPointer();
}

PDFSimpleFont::PDFSimpleFont(CIDSystemInfo cidSystemInfo,
                             FontDescriptor fontDescriptor,
                             QByteArray name,
                             QByteArray baseFont,
                             PDFInteger firstChar,
                             PDFInteger lastChar,
                             std::vector<PDFInteger> widths,
                             PDFEncoding::Encoding encodingType,
                             encoding::EncodingTable encoding,
                             GlyphIndices glyphIndices) :
    PDFFont(qMove(cidSystemInfo), qMove(fontDescriptor)),
    m_name(qMove(name)),
    m_baseFont(qMove(baseFont)),
    m_firstChar(firstChar),
    m_lastChar(lastChar),
    m_widths(qMove(widths)),
    m_encodingType(encodingType),
    m_encoding(encoding),
    m_glyphIndices(glyphIndices)
{

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
                           CIDSystemInfo cidSystemInfo,
                           FontDescriptor fontDescriptor,
                           QByteArray name,
                           QByteArray baseFont,
                           PDFInteger firstChar,
                           PDFInteger lastChar,
                           std::vector<PDFInteger> widths,
                           PDFEncoding::Encoding encodingType,
                           encoding::EncodingTable encoding,
                           StandardFontType standardFontType,
                           GlyphIndices glyphIndices) :
    PDFSimpleFont(qMove(cidSystemInfo), qMove(fontDescriptor), qMove(name), qMove(baseFont), firstChar, lastChar, qMove(widths), encodingType, encoding, glyphIndices),
    m_fontType(fontType),
    m_standardFontType(standardFontType)
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
        }
    }
}

PDFFontPointer PDFFontCache::getFont(const PDFObject& fontObject) const
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
            PDFFontPointer font = PDFFont::createFont(fontObject, m_document);

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
        return PDFFont::createFont(fontObject, m_document);
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

        return createFromData(data);
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

    std::sort(entries.begin(), entries.end());
    entries = optimize(entries);

    if (!additionalMappings.empty())
    {
        for (const PDFFontCMap& map : additionalMappings)
        {
            entries.insert(entries.cend(), map.m_entries.cbegin(), map.m_entries.cend());
        }
    }

    return PDFFontCMap(qMove(entries), vertical);
}

QByteArray PDFFontCMap::serialize() const
{
    QByteArray result;

    {
        QDataStream stream(&result, QIODevice::WriteOnly);
        stream << m_maxKeyLength;
        stream << m_vertical;
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
            result.push_back(cid);

            value = 0;
            scannedBytes = 0;
        }
        else if (scannedBytes == m_maxKeyLength)
        {
            // This means error occured - fill empty CID
            result.push_back(0);
            value = 0;
            scannedBytes = 0;
        }
    }

    return result;
}

QChar PDFFontCMap::getToUnicode(CID cid) const
{
    if (isValid())
    {
        auto it = std::find_if(m_entries.cbegin(), m_entries.cend(), [cid](const Entry& entry) { return entry.from <= cid && entry.to >= cid; });
        if (it != m_entries.cend())
        {
            const Entry& entry = *it;
            const CID unicodeCID = cid - entry.from + entry.cid;
            return QChar(unicodeCID);
        }
    }

    return QChar();
}

PDFFontCMap::PDFFontCMap(Entries&& entries, bool vertical) :
    m_entries(qMove(entries)),
    m_maxKeyLength(0),
    m_vertical(vertical)
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

PDFType3Font::PDFType3Font(FontDescriptor fontDescriptor,
                           int firstCharacterIndex,
                           int lastCharacterIndex,
                           QTransform fontMatrix,
                           std::map<int, QByteArray>&& characterContentStreams,
                           std::vector<double>&& widths,
                           const PDFObject& resources,
                           PDFFontCMap toUnicode) :
    PDFFont(CIDSystemInfo(), qMove(fontDescriptor)),
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
            textSequence.items.emplace_back(contentStream, character, width);
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

}   // namespace pdf
