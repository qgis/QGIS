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

#ifndef PDFEDITORFALLBACKFONT_H
#define PDFEDITORFALLBACKFONT_H

#include "pdffont.h"
#include "pdfobject.h"

#include <QRawFont>
#include <QPainterPath>

#include <map>
#include <vector>
#include <functional>

namespace pdf
{

/// Builds Type 3 fallback fonts on the fly for characters, which cannot be
/// encoded into the currently selected font. Glyph outlines are extracted
/// from a similar system font, so no font program embedding is needed.
/// One instance of this manager lives inside the content stream builder
/// for the duration of one content stream build, fallback fonts are
/// regenerated deterministically on each build.
class PDF4QTLIBCORESHARED_EXPORT PDFEditorFallbackFontManager
{
public:
    PDFEditorFallbackFontManager() = default;

    struct Run
    {
        QByteArray fontResourceKey; ///< Key of the fallback font in the page font dictionary
        QByteArray encodedBytes;    ///< Encoded character codes for a Tj operator
    };

    /// Encodes unicode code points (already known to be not encodable in the
    /// current font). Extends or creates fallback fonts as needed, refreshes
    /// their dictionary objects in the font dictionary, and returns runs
    /// for the Tj operator (multiple runs, when more than one fallback font
    /// is needed). Characters missing even in the substitute fonts are
    /// rendered as empty glyphs and reported via the error callback.
    /// \param codePoints Unicode code points to be encoded
    /// \param similarToFont Font, which the fallback font should resemble (can be nullptr)
    /// \param fontDictionary Page font dictionary, where fallback fonts are registered
    /// \param errorCallback Callback for error reporting
    std::vector<Run> encode(const std::u32string& codePoints,
                            const PDFFontPointer& similarToFont,
                            PDFDictionary& fontDictionary,
                            const std::function<void(const QString&)>& errorCallback);

private:
    struct GlyphRecord
    {
        char32_t codePoint = 0;
        QByteArray glyphName;
        QPainterPath path;      ///< Glyph outline in the glyph space (1000 units per em, y-axis up)
        double width = 0.0;     ///< Glyph advance in the glyph space
        bool hasGlyph = false;  ///< False, if no substitute font contains the glyph (empty outline is used)
    };

    struct FallbackFont
    {
        QByteArray resourceKey;
        std::map<int, GlyphRecord> glyphs; ///< Character code to glyph
        int nextCode = FIRST_CHARACTER_CODE;
        QRectF fontBBox;
    };

    struct SourceFontFallback
    {
        QRawFont substituteFont;
        std::vector<FallbackFont> fonts;
        std::map<char32_t, std::pair<size_t, int>> codeMap; ///< Code point to (font index, character code)
    };

    /// First character code assigned in a fallback font. Code 0 is never
    /// assigned (it is treated as a placeholder during decoding) and
    /// code 32 is never assigned (word spacing would be applied to it).
    static constexpr int FIRST_CHARACTER_CODE = 33;
    static constexpr int LAST_CHARACTER_CODE = 255;

    /// Returns fallback data for the source font, creating the substitute
    /// font from the descriptor, if it does not exist yet.
    SourceFontFallback& getSourceFontFallback(const PDFFontPointer& similarToFont);

    /// Assigns a character code for the code point, extracting the glyph
    /// outline from the substitute fonts. Returns (font index, character code).
    std::pair<size_t, int> assignCode(SourceFontFallback& fallback,
                                      char32_t codePoint,
                                      PDFDictionary& fontDictionary,
                                      const std::function<void(const QString&)>& errorCallback);

    /// Creates a new fallback font with a unique resource key
    FallbackFont& createFallbackFont(SourceFontFallback& fallback, PDFDictionary& fontDictionary);

    /// Extracts the glyph outline and advance for the code point from the
    /// substitute font, or from the cascade of generic fallback fonts.
    GlyphRecord createGlyphRecord(const QRawFont& substituteFont, char32_t codePoint) const;

    /// Builds the Type 3 font dictionary object (including character content
    /// streams and the ToUnicode stream) for the fallback font.
    PDFObject buildFontDictionaryObject(const FallbackFont& font) const;

    /// Builds the ToUnicode CMap stream data for the fallback font
    QByteArray buildToUnicodeStreamData(const FallbackFont& font) const;

    /// Returns the cascade of generic fonts used when the substitute
    /// font does not contain the glyph.
    const std::vector<QRawFont>& getCascadeFonts() const;

    std::map<QByteArray, SourceFontFallback> m_fallbacks; ///< Keyed by the source font id
    mutable std::vector<QRawFont> m_cascadeFonts;
};

}   // namespace pdf

#endif // PDFEDITORFALLBACKFONT_H
