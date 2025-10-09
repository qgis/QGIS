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
