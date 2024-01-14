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
