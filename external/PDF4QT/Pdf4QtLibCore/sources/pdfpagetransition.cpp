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

#include "pdfpagetransition.h"
#include "pdfdocument.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFPageTransition PDFPageTransition::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFPageTransition result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array<std::pair<const char*, Style>, 12> styles = {
            std::pair<const char*, Style>{ "Split", Style::Split },
            std::pair<const char*, Style>{ "Blinds", Style::Blinds },
            std::pair<const char*, Style>{ "Box", Style::Box },
            std::pair<const char*, Style>{ "Wipe", Style::Wipe },
            std::pair<const char*, Style>{ "Dissolve", Style::Dissolve },
            std::pair<const char*, Style>{ "Glitter", Style::Glitter },
            std::pair<const char*, Style>{ "R", Style::R },
            std::pair<const char*, Style>{ "Fly", Style::Fly },
            std::pair<const char*, Style>{ "Push", Style::Push },
            std::pair<const char*, Style>{ "Cover", Style::Cover },
            std::pair<const char*, Style>{ "Uncover", Style::Uncover },
            std::pair<const char*, Style>{ "Fade", Style::Fade }
        };

        constexpr const std::array<std::pair<const char*, Orientation>, 2> orientations = {
            std::pair<const char*, Orientation>{ "H", Orientation::Horizontal },
            std::pair<const char*, Orientation>{ "V", Orientation::Vertical }
        };

        constexpr const std::array<std::pair<const char*, Direction>, 2> directions = {
            std::pair<const char*, Direction>{ "I", Direction::Inward },
            std::pair<const char*, Direction>{ "O", Direction::Outward }
        };

        result.m_style = loader.readEnumByName(dictionary->get("S"), styles.cbegin(), styles.cend(), Style::R);
        result.m_duration = loader.readNumberFromDictionary(dictionary, "D", 1.0);
        result.m_orientation = loader.readEnumByName(dictionary->get("Dm"), orientations.cbegin(), orientations.cend(), Orientation::Horizontal);
        result.m_direction = loader.readEnumByName(dictionary->get("M"), directions.cbegin(), directions.cend(), Direction::Inward);
        result.m_angle = loader.readNumberFromDictionary(dictionary, "Di", 0.0);
        result.m_scale = loader.readNumberFromDictionary(dictionary, "SS", 1.0);
        result.m_rectangular = loader.readBooleanFromDictionary(dictionary, "B", false);
    }

    return result;
}

}   // namespace pdf
