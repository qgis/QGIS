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

#include "pdftextlayoutgenerator.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFTextLayout PDFTextLayoutGenerator::createTextLayout()
{
    m_textLayout.perform();
    m_textLayout.optimize();
    return qMove(m_textLayout);
}

bool PDFTextLayoutGenerator::isContentSuppressedByOC(PDFObjectReference ocgOrOcmd)
{
    if (m_features.testFlag(PDFRenderer::IgnoreOptionalContent))
    {
        return false;
    }

    return PDFPageContentProcessor::isContentSuppressedByOC(ocgOrOcmd);
}

bool PDFTextLayoutGenerator::isContentKindSuppressed(ContentKind kind) const
{
    switch (kind)
    {
        case ContentKind::Shapes:
        case ContentKind::Text:
        case ContentKind::Images:
        case ContentKind::Shading:
            return true;

        case ContentKind::Tiling:
            return false; // Tiling can have text

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return false;
}

void PDFTextLayoutGenerator::performOutputCharacter(const PDFTextCharacterInfo& info)
{
    if (!isContentSuppressed() && !info.character.isSpace())
    {
        m_textLayout.addCharacter(info);
    }
}

}   // namespace pdf
