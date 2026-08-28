// MIT License
//
// Copyright (c) 2018-2026 Jakub Melka and Contributors
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

#include "pdfeditorfallbackfont.h"
#include "pdfpagecontenteditorcontentstreambuilder.h"
#include "pdfexception.h"

#include <QFont>
#include <QTextStream>

#include "pdfdbgheap.h"

namespace pdf
{

std::vector<PDFEditorFallbackFontManager::Run> PDFEditorFallbackFontManager::encode(const std::u32string& codePoints,
                                                                                    const PDFFontPointer& similarToFont,
                                                                                    PDFDictionary& fontDictionary,
                                                                                    const std::function<void(const QString&)>& errorCallback)
{
    std::vector<Run> runs;

    if (codePoints.empty())
    {
        return runs;
    }

    SourceFontFallback& fallback = getSourceFontFallback(similarToFont);
    std::set<size_t> updatedFontIndices;

    for (const char32_t codePoint : codePoints)
    {
        std::pair<size_t, int> assignment;

        auto it = fallback.codeMap.find(codePoint);
        if (it != fallback.codeMap.cend())
        {
            assignment = it->second;
        }
        else
        {
            assignment = assignCode(fallback, codePoint, fontDictionary, errorCallback);
            updatedFontIndices.insert(assignment.first);
        }

        const FallbackFont& font = fallback.fonts[assignment.first];
        if (!runs.empty() && runs.back().fontResourceKey == font.resourceKey)
        {
            runs.back().encodedBytes.append(static_cast<char>(assignment.second));
        }
        else
        {
            runs.push_back(Run{ font.resourceKey, QByteArray(1, static_cast<char>(assignment.second)) });
        }
    }

    for (const size_t fontIndex : updatedFontIndices)
    {
        const FallbackFont& font = fallback.fonts[fontIndex];
        fontDictionary.setEntry(PDFInplaceOrMemoryString(font.resourceKey), buildFontDictionaryObject(font));
    }

    return runs;
}

PDFEditorFallbackFontManager::SourceFontFallback& PDFEditorFallbackFontManager::getSourceFontFallback(const PDFFontPointer& similarToFont)
{
    const QByteArray sourceFontId = similarToFont ? similarToFont->getFontId() : QByteArray();

    auto it = m_fallbacks.find(sourceFontId);
    if (it == m_fallbacks.cend())
    {
        SourceFontFallback fallbackData;

        QFont substituteFont;
        if (similarToFont)
        {
            const FontDescriptor* descriptor = similarToFont->getFontDescriptor();

            QByteArray fontFamily = descriptor->fontFamily;
            if (fontFamily.isEmpty())
            {
                fontFamily = descriptor->fontName;

                // Strip the subset font prefix ("ABCDEF+")
                if (fontFamily.size() > 7 && fontFamily[6] == '+')
                {
                    fontFamily = fontFamily.mid(7);
                }
            }

            substituteFont = QFont(QString::fromLatin1(fontFamily));
            substituteFont.setWeight(QFont::Weight(qBound(1, int(descriptor->fontWeight), 1000)));
            substituteFont.setStretch(descriptor->fontStretch);
            substituteFont.setItalic(descriptor->isItalic());
            substituteFont.setStyleHint(descriptor->isFixedPitch() ? QFont::Monospace :
                                        descriptor->isSerif() ? QFont::Serif : QFont::SansSerif);
        }

        substituteFont.setPixelSize(1000);
        fallbackData.substituteFont = QRawFont::fromFont(substituteFont);

        it = m_fallbacks.emplace(sourceFontId, std::move(fallbackData)).first;
    }

    return it->second;
}

std::pair<size_t, int> PDFEditorFallbackFontManager::assignCode(SourceFontFallback& fallback,
                                                                char32_t codePoint,
                                                                PDFDictionary& fontDictionary,
                                                                const std::function<void(const QString&)>& errorCallback)
{
    if (fallback.fonts.empty() || fallback.fonts.back().nextCode > LAST_CHARACTER_CODE)
    {
        createFallbackFont(fallback, fontDictionary);
    }

    const size_t fontIndex = fallback.fonts.size() - 1;
    FallbackFont& font = fallback.fonts[fontIndex];

    GlyphRecord glyphRecord = createGlyphRecord(fallback.substituteFont, codePoint);
    if (!glyphRecord.hasGlyph && errorCallback)
    {
        errorCallback(PDFTranslationContext::tr("Character '%1' (U+%2) is not available in any substitute font, an empty glyph is used.")
                          .arg(QString::fromUcs4(&codePoint, 1))
                          .arg(uint(codePoint), 4, 16, QChar('0')));
    }

    const int code = font.nextCode++;
    font.fontBBox = font.fontBBox.united(glyphRecord.path.boundingRect());
    font.glyphs[code] = std::move(glyphRecord);
    fallback.codeMap[codePoint] = std::make_pair(fontIndex, code);

    return std::make_pair(fontIndex, code);
}

PDFEditorFallbackFontManager::FallbackFont& PDFEditorFallbackFontManager::createFallbackFont(SourceFontFallback& fallback, PDFDictionary& fontDictionary)
{
    FallbackFont font;

    int i = 0;
    while (true)
    {
        QByteArray currentKey = QString("PDF4QT_Fb%1").arg(++i).toLatin1();
        if (!fontDictionary.hasKey(currentKey))
        {
            font.resourceKey = currentKey;

            // Reserve the key immediately, so a second fallback font created
            // during the same call does not get the same key. The placeholder
            // is replaced by the real font dictionary object in encode().
            fontDictionary.setEntry(PDFInplaceOrMemoryString(currentKey), PDFObject());
            break;
        }
    }

    fallback.fonts.push_back(std::move(font));
    return fallback.fonts.back();
}

PDFEditorFallbackFontManager::GlyphRecord PDFEditorFallbackFontManager::createGlyphRecord(const QRawFont& substituteFont, char32_t codePoint) const
{
    GlyphRecord record;
    record.codePoint = codePoint;

    if (codePoint <= 0xFFFF)
    {
        record.glyphName = QString("uni%1").arg(uint(codePoint), 4, 16, QChar('0')).toUpper().toLatin1();
    }
    else
    {
        record.glyphName = ("u" + QString("%1").arg(uint(codePoint), 0, 16).toUpper()).toLatin1();
    }

    auto tryExtractGlyph = [&record, codePoint](const QRawFont& rawFont) -> bool
    {
        if (!rawFont.isValid())
        {
            return false;
        }

        const QString text = QString::fromUcs4(&codePoint, 1);
        const QList<quint32> glyphIndices = rawFont.glyphIndexesForString(text);
        if (glyphIndices.size() != 1 || glyphIndices.front() == 0)
        {
            return false;
        }

        const quint32 glyphIndex = glyphIndices.front();
        QPainterPath glyphPath = rawFont.pathForGlyph(glyphIndex);

        // Qt glyph paths have the y-axis pointing down, PDF glyph space
        // has the y-axis pointing up.
        glyphPath = QTransform(1, 0, 0, -1, 0, 0).map(glyphPath);

        const QList<QPointF> advances = rawFont.advancesForGlyphIndexes({ glyphIndex });

        record.path = std::move(glyphPath);
        record.width = advances.size() == 1 ? advances.front().x() : rawFont.averageCharWidth();
        record.hasGlyph = true;
        return true;
    };

    if (tryExtractGlyph(substituteFont))
    {
        return record;
    }

    for (const QRawFont& cascadeFont : getCascadeFonts())
    {
        if (tryExtractGlyph(cascadeFont))
        {
            return record;
        }
    }

    // No substitute font contains the glyph - use an empty glyph
    // with a reasonable advance, the character still round-trips
    // through the ToUnicode mapping.
    record.width = 500.0;
    record.hasGlyph = false;
    return record;
}

const std::vector<QRawFont>& PDFEditorFallbackFontManager::getCascadeFonts() const
{
    if (m_cascadeFonts.empty())
    {
        auto addCascadeFont = [this](QFont font)
        {
            font.setPixelSize(1000);
            QRawFont rawFont = QRawFont::fromFont(font);
            if (rawFont.isValid())
            {
                m_cascadeFonts.push_back(std::move(rawFont));
            }
        };

        addCascadeFont(QFont());
        addCascadeFont(QFont("Segoe UI Symbol"));
        addCascadeFont(QFont("Segoe UI Emoji"));
    }

    return m_cascadeFonts;
}

PDFObject PDFEditorFallbackFontManager::buildFontDictionaryObject(const FallbackFont& font) const
{
    Q_ASSERT(!font.glyphs.empty());

    const int firstChar = font.glyphs.begin()->first;
    const int lastChar = font.glyphs.rbegin()->first;

    QRectF fontBBox = font.fontBBox;
    if (fontBBox.isNull())
    {
        fontBBox = QRectF(0, 0, 1000, 1000);
    }

    PDFArray fontBBoxArray;
    fontBBoxArray.appendItem(PDFObject::createReal(fontBBox.left()));
    fontBBoxArray.appendItem(PDFObject::createReal(fontBBox.top()));
    fontBBoxArray.appendItem(PDFObject::createReal(fontBBox.right()));
    fontBBoxArray.appendItem(PDFObject::createReal(fontBBox.bottom()));

    PDFArray fontMatrixArray;
    fontMatrixArray.appendItem(PDFObject::createReal(0.001));
    fontMatrixArray.appendItem(PDFObject::createReal(0.0));
    fontMatrixArray.appendItem(PDFObject::createReal(0.0));
    fontMatrixArray.appendItem(PDFObject::createReal(0.001));
    fontMatrixArray.appendItem(PDFObject::createReal(0.0));
    fontMatrixArray.appendItem(PDFObject::createReal(0.0));

    PDFArray widthsArray;
    PDFArray differencesArray;
    PDFDictionary charProcsDictionary;

    differencesArray.appendItem(PDFObject::createInteger(firstChar));

    for (int code = firstChar; code <= lastChar; ++code)
    {
        auto it = font.glyphs.find(code);
        if (it == font.glyphs.cend())
        {
            // Character codes are assigned contiguously, this is a safety net only
            widthsArray.appendItem(PDFObject::createReal(0.0));
            differencesArray.appendItem(PDFObject::createName(QString(".notdef%1").arg(code).toLatin1()));
            continue;
        }

        const GlyphRecord& glyph = it->second;
        widthsArray.appendItem(PDFObject::createReal(glyph.width));
        differencesArray.appendItem(PDFObject::createName(glyph.glyphName));

        QByteArray charProcContent;
        {
            QTextStream stream(&charProcContent, QDataStream::WriteOnly);
            stream << glyph.width << " 0 d0" << Qt::endl;
            PDFPageContentEditorContentStreamBuilder::writePathGeometry(stream, glyph.path);
            if (!glyph.path.isEmpty())
            {
                stream << "f" << Qt::endl;
            }
        }

        PDFDictionary charProcStreamDictionary;
        charProcStreamDictionary.setEntry(PDFInplaceOrMemoryString("Length"), PDFObject::createInteger(charProcContent.size()));
        charProcsDictionary.setEntry(PDFInplaceOrMemoryString(glyph.glyphName),
                                     PDFObject::createStream(std::make_shared<PDFStream>(qMove(charProcStreamDictionary), qMove(charProcContent))));
    }

    PDFDictionary encodingDictionary;
    encodingDictionary.setEntry(PDFInplaceOrMemoryString("Type"), PDFObject::createName("Encoding"));
    encodingDictionary.setEntry(PDFInplaceOrMemoryString("Differences"), PDFObject::createArray(std::make_shared<PDFArray>(qMove(differencesArray))));

    QByteArray toUnicodeData = buildToUnicodeStreamData(font);
    PDFDictionary toUnicodeStreamDictionary;
    toUnicodeStreamDictionary.setEntry(PDFInplaceOrMemoryString("Length"), PDFObject::createInteger(toUnicodeData.size()));
    PDFObject toUnicodeObject = PDFObject::createStream(std::make_shared<PDFStream>(qMove(toUnicodeStreamDictionary), qMove(toUnicodeData)));

    PDFDictionary fontDictionaryObject;
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("Type"), PDFObject::createName("Font"));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("Subtype"), PDFObject::createName("Type3"));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("FontBBox"), PDFObject::createArray(std::make_shared<PDFArray>(qMove(fontBBoxArray))));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("FontMatrix"), PDFObject::createArray(std::make_shared<PDFArray>(qMove(fontMatrixArray))));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("FirstChar"), PDFObject::createInteger(firstChar));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("LastChar"), PDFObject::createInteger(lastChar));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("Widths"), PDFObject::createArray(std::make_shared<PDFArray>(qMove(widthsArray))));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("Encoding"), PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(encodingDictionary))));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("CharProcs"), PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(charProcsDictionary))));
    fontDictionaryObject.setEntry(PDFInplaceOrMemoryString("ToUnicode"), std::move(toUnicodeObject));

    return PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(fontDictionaryObject)));
}

QByteArray PDFEditorFallbackFontManager::buildToUnicodeStreamData(const FallbackFont& font) const
{
    QByteArray data;
    QTextStream stream(&data, QDataStream::WriteOnly);

    stream << "/CIDInit /ProcSet findresource begin" << Qt::endl;
    stream << "12 dict begin" << Qt::endl;
    stream << "begincmap" << Qt::endl;
    stream << "/CIDSystemInfo << /Registry (Adobe) /Ordering (UCS) /Supplement 0 >> def" << Qt::endl;
    stream << "/CMapName /Adobe-Identity-UCS def" << Qt::endl;
    stream << "/CMapType 2 def" << Qt::endl;
    stream << "1 begincodespacerange" << Qt::endl;
    stream << "<00> <FF>" << Qt::endl;
    stream << "endcodespacerange" << Qt::endl;

    // At most 100 mappings per beginbfchar section
    constexpr int maxMappingsPerSection = 100;
    auto it = font.glyphs.cbegin();
    int remaining = int(font.glyphs.size());
    while (remaining > 0)
    {
        const int sectionSize = qMin(remaining, maxMappingsPerSection);
        stream << sectionSize << " beginbfchar" << Qt::endl;

        for (int i = 0; i < sectionSize; ++i, ++it)
        {
            const int code = it->first;
            const char32_t codePoint = it->second.codePoint;

            QByteArray unicodeBytes;
            if (codePoint <= 0xFFFF)
            {
                unicodeBytes.append(static_cast<char>((codePoint >> 8) & 0xFF));
                unicodeBytes.append(static_cast<char>(codePoint & 0xFF));
            }
            else
            {
                // Encode as UTF-16BE surrogate pair
                const char32_t value = codePoint - 0x10000;
                const char16_t highSurrogate = static_cast<char16_t>(0xD800 + (value >> 10));
                const char16_t lowSurrogate = static_cast<char16_t>(0xDC00 + (value & 0x3FF));
                unicodeBytes.append(static_cast<char>((highSurrogate >> 8) & 0xFF));
                unicodeBytes.append(static_cast<char>(highSurrogate & 0xFF));
                unicodeBytes.append(static_cast<char>((lowSurrogate >> 8) & 0xFF));
                unicodeBytes.append(static_cast<char>(lowSurrogate & 0xFF));
            }

            stream << "<" << QString("%1").arg(code, 2, 16, QChar('0')).toUpper() << "> <" << unicodeBytes.toHex().toUpper() << ">" << Qt::endl;
        }

        stream << "endbfchar" << Qt::endl;
        remaining -= sectionSize;
    }

    stream << "endcmap" << Qt::endl;
    stream << "CMapName currentdict /CMap defineresource pop" << Qt::endl;
    stream << "end" << Qt::endl;
    stream << "end" << Qt::endl;

    stream.flush();
    return data;
}

}   // namespace pdf
