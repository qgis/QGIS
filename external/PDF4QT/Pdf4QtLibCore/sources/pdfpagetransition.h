//    Copyright (C) 2019-2021 Jakub Melka
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

#ifndef PDFPAGETRANSITION_H
#define PDFPAGETRANSITION_H

#include "pdfobject.h"

namespace pdf
{
class PDFObjectStorage;

/// Page transition during presentation settings.
class PDFPageTransition
{
public:

    enum class Style
    {
        Split,
        Blinds,
        Box,
        Wipe,
        Dissolve,
        Glitter,
        R,
        Fly,
        Push,
        Cover,
        Uncover,
        Fade
    };

    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    enum class Direction
    {
        Inward,
        Outward
    };

    static PDFPageTransition parse(const PDFObjectStorage* storage, PDFObject object);

    Style getStyle() const { return m_style; }
    PDFReal getDuration() const { return m_duration; }
    Orientation getOrientation() const { return m_orientation; }
    Direction getDirection() const { return m_direction; }
    PDFReal getAngle() const { return m_angle; }
    PDFReal getScale() const { return m_scale; }
    bool getRectangular() const { return m_rectangular; }

private:
    Style m_style = Style::R;
    PDFReal m_duration = 1.0;
    Orientation m_orientation = Orientation::Horizontal;
    Direction m_direction = Direction::Inward;
    PDFReal m_angle = 0.0;
    PDFReal m_scale = 1.0;
    bool m_rectangular = false;
};

}   // namespace pdf

#endif // PDFPAGETRANSITION_H
