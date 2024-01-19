//    Copyright (C) 2020-2022 Jakub Melka
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

#include "pdftransparencyrenderer.h"
#include "pdfdocument.h"
#include "pdfcms.h"
#include "pdfexecutionpolicy.h"
#include "pdfimage.h"
#include "pdfpattern.h"
#include "pdfdbgheap.h"

#include <QtMath>
#include <iterator>

namespace pdf
{

PDFFloatBitmap::PDFFloatBitmap() :
    m_width(0),
    m_height(0),
    m_pixelSize(0)
{

}

PDFFloatBitmap::PDFFloatBitmap(size_t width, size_t height, PDFPixelFormat format) :
    m_format(format),
    m_width(width),
    m_height(height),
    m_pixelSize(format.getChannelCount())
{
    Q_ASSERT(format.isValid());

    m_data.resize(format.calculateBitmapDataLength(width, height), static_cast<PDFColorComponent>(0.0f));

    if (m_format.hasActiveColorMask())
    {
        m_activeColorMask.resize(width * height, 0);
    }
}

PDFColorBuffer PDFFloatBitmap::getPixel(size_t x, size_t y)
{
    Q_ASSERT(x < m_width);
    Q_ASSERT(y < m_height);

    const size_t index = getPixelIndex(x, y);
    return PDFColorBuffer(m_data.data() + index, m_pixelSize);
}

PDFConstColorBuffer PDFFloatBitmap::getPixel(size_t x, size_t y) const
{
    Q_ASSERT(x < m_width);
    Q_ASSERT(y < m_height);

    const size_t index = getPixelIndex(x, y);
    return PDFConstColorBuffer(m_data.data() + index, m_pixelSize);
}

PDFColorBuffer PDFFloatBitmap::getPixels()
{
    return PDFColorBuffer(m_data.data(), m_data.size());
}

PDFColorComponent PDFFloatBitmap::getPixelInkCoverage(size_t x, size_t y) const
{
    PDFConstColorBuffer buffer = getPixel(x, y);

    const uint8_t colorChannelIndexStart = m_format.getColorChannelIndexStart();
    const uint8_t colorChannelIndexEnd = m_format.getColorChannelIndexEnd();

    PDFColorComponent inkCoverage = 0.0;
    for (uint8_t i = colorChannelIndexStart; i < colorChannelIndexEnd; ++i)
    {
        inkCoverage += buffer[i];
    }

    return inkCoverage;
}

PDFFloatBitmap PDFFloatBitmap::getInkCoverageBitmap() const
{
    PDFFloatBitmap result(getWidth(), getHeight(), PDFPixelFormat::createFormat(1, 0, false, true, false));

    for (size_t y = 0; y < getHeight(); ++y)
    {
        for (size_t x = 0; x < getWidth(); ++x)
        {
            PDFColorComponent coverage = getPixelInkCoverage(x, y);
            PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);
            targetProcessColorBuffer[0] = coverage;
        }
    }

    return result;
}

const PDFColorComponent* PDFFloatBitmap::begin() const
{
    return m_data.data();
}

const PDFColorComponent* PDFFloatBitmap::end() const
{
    return m_data.data() + m_data.size();
}

PDFColorComponent* PDFFloatBitmap::begin()
{
    return m_data.data();
}

PDFColorComponent* PDFFloatBitmap::end()
{
    return m_data.data() + m_data.size();
}

void PDFFloatBitmap::makeTransparent()
{
    if (m_format.hasShapeChannel())
    {
        fillChannel(m_format.getShapeChannelIndex(), 0.0f);
    }

    if (m_format.hasOpacityChannel())
    {
        fillChannel(m_format.getOpacityChannelIndex(), 0.0f);
    }
}

void PDFFloatBitmap::makeOpaque()
{
    if (m_format.hasShapeChannel())
    {
        fillChannel(m_format.getShapeChannelIndex(), 1.0f);
    }

    if (m_format.hasOpacityChannel())
    {
        fillChannel(m_format.getOpacityChannelIndex(), 1.0f);
    }
}

void PDFFloatBitmap::makeColorBlack()
{
    fillProcessColorChannels(m_format.hasProcessColorsSubtractive() ? 1.0 : 0.0);
}

void PDFFloatBitmap::makeColorWhite()
{
    fillProcessColorChannels(m_format.hasProcessColorsSubtractive() ? 0.0 : 1.0);
}

size_t PDFFloatBitmap::getPixelIndex(size_t x, size_t y) const
{
    return (y * m_width + x) * m_pixelSize;
}

uint32_t PDFFloatBitmap::getPixelActiveColorMask(size_t x, size_t y) const
{
    Q_ASSERT(hasActiveColorMask());
    return m_activeColorMask[y * m_width + x];
}

void PDFFloatBitmap::markPixelActiveColorMask(size_t x, size_t y, uint32_t activeColorMask)
{
    Q_ASSERT(hasActiveColorMask());
    m_activeColorMask[y * m_width + x] |= activeColorMask;
}

void PDFFloatBitmap::setPixelActiveColorMask(size_t x, size_t y, uint32_t activeColorMask)
{
    Q_ASSERT(hasActiveColorMask());
    m_activeColorMask[y * m_width + x] = activeColorMask;
}

void PDFFloatBitmap::setAllColorActive()
{
    std::fill(m_activeColorMask.begin(), m_activeColorMask.end(), PDFPixelFormat::getAllColorsMask());
}

void PDFFloatBitmap::setAllColorInactive()
{
    std::fill(m_activeColorMask.begin(), m_activeColorMask.end(), 0);
}

void PDFFloatBitmap::setColorActivity(uint32_t mask)
{
    std::fill(m_activeColorMask.begin(), m_activeColorMask.end(), mask);
}

QImage PDFFloatBitmap::getChannelImage(uint8_t channelIndex) const
{
    if (channelIndex >= getPixelSize())
    {
        return QImage();
    }

    QImage image(int(getWidth()), int(getHeight()), QImage::Format_Grayscale8);

    for (int y = 0; y < image.height(); ++y)
    {
        uchar* line = image.scanLine(y);
        for (int x = 0; x < image.width(); ++x)
        {
            PDFConstColorBuffer buffer = getPixel(x, y);
            line[x] = qRound(buffer[channelIndex] * 255);
        }
    }

    return image;
}

PDFFloatBitmap PDFFloatBitmap::extractProcessColors() const
{
    PDFPixelFormat format = PDFPixelFormat::createFormat(m_format.getProcessColorChannelCount(), 0, false, m_format.hasProcessColorsSubtractive(), false);
    PDFFloatBitmap result(getWidth(), getHeight(), format);

    for (size_t x = 0; x < getWidth(); ++x)
    {
        for (size_t y = 0; y < getHeight(); ++y)
        {
            PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
            PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

            Q_ASSERT(sourceProcessColorBuffer.size() >= targetProcessColorBuffer.size());
            std::copy(sourceProcessColorBuffer.cbegin(), std::next(sourceProcessColorBuffer.cbegin(), targetProcessColorBuffer.size()), targetProcessColorBuffer.begin());
        }
    }

    return result;
}

PDFFloatBitmap PDFFloatBitmap::extractSpotChannel(uint8_t channel) const
{
    PDFPixelFormat format = PDFPixelFormat::createFormat(0, 1, false, m_format.hasProcessColorsSubtractive(), false);
    PDFFloatBitmap result(getWidth(), getHeight(), format);

    Q_ASSERT(m_format.hasSpotColors());
    Q_ASSERT(m_format.getSpotColorChannelIndexStart() <= channel);
    Q_ASSERT(m_format.getSpotColorChannelIndexEnd() > channel);

    for (size_t x = 0; x < getWidth(); ++x)
    {
        for (size_t y = 0; y < getHeight(); ++y)
        {
            PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
            PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

            targetProcessColorBuffer[0] = sourceProcessColorBuffer[channel];
        }
    }

    return result;
}

PDFFloatBitmap PDFFloatBitmap::extractOpacityChannel() const
{
    PDFFloatBitmap result(getWidth(), getHeight(), PDFPixelFormat::createOpacityMask());

    if (m_format.hasOpacityChannel())
    {
        const uint8_t opacityChannel = m_format.getOpacityChannelIndex();
        for (size_t x = 0; x < getWidth(); ++x)
        {
            for (size_t y = 0; y < getHeight(); ++y)
            {
                PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
                PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

                targetProcessColorBuffer[0] = sourceProcessColorBuffer[opacityChannel];
            }
        }
    }
    else
    {
        result.makeOpaque();
    }

    return result;
}

PDFFloatBitmap PDFFloatBitmap::extractLuminosityChannel() const
{
    PDFFloatBitmap result(getWidth(), getHeight(), PDFPixelFormat::createOpacityMask());

    const uint8_t sourceChannelIndex = m_format.getProcessColorChannelIndexStart();
    switch (m_format.getProcessColorChannelCount())
    {
        case 1:
        {
            for (size_t x = 0; x < getWidth(); ++x)
            {
                for (size_t y = 0; y < getHeight(); ++y)
                {
                    PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
                    PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

                    targetProcessColorBuffer[0] = PDFBlendFunction::getLuminosity(PDFGray(sourceProcessColorBuffer[sourceChannelIndex]));
                }
            }
            break;
        }

        case 3:
        {
            for (size_t x = 0; x < getWidth(); ++x)
            {
                for (size_t y = 0; y < getHeight(); ++y)
                {
                    PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
                    PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

                    PDFColorComponent r = sourceProcessColorBuffer[sourceChannelIndex + 0];
                    PDFColorComponent g = sourceProcessColorBuffer[sourceChannelIndex + 1];
                    PDFColorComponent b = sourceProcessColorBuffer[sourceChannelIndex + 2];
                    targetProcessColorBuffer[0] = PDFBlendFunction::getLuminosity(PDFRGB{ r, g, b });
                }
            }
            break;
        }

        case 4:
        {
            for (size_t x = 0; x < getWidth(); ++x)
            {
                for (size_t y = 0; y < getHeight(); ++y)
                {
                    PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
                    PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

                    PDFColorComponent _c = sourceProcessColorBuffer[sourceChannelIndex + 0];
                    PDFColorComponent _m = sourceProcessColorBuffer[sourceChannelIndex + 1];
                    PDFColorComponent _y = sourceProcessColorBuffer[sourceChannelIndex + 2];
                    PDFColorComponent _k = sourceProcessColorBuffer[sourceChannelIndex + 3];
                    targetProcessColorBuffer[0] = PDFBlendFunction::getLuminosity(PDFCMYK{ _c, _m, _y, _k });
                }
            }
            break;
        }

        default:
        {
            result.makeOpaque();
            break;
        }
    }

    if (m_format.hasOpacityChannel())
    {
        const uint8_t opacityChannel = m_format.getOpacityChannelIndex();
        for (size_t x = 0; x < getWidth(); ++x)
        {
            for (size_t y = 0; y < getHeight(); ++y)
            {
                PDFConstColorBuffer sourceProcessColorBuffer = getPixel(x, y);
                PDFColorBuffer targetProcessColorBuffer = result.getPixel(x, y);

                targetProcessColorBuffer[0] = sourceProcessColorBuffer[opacityChannel];
            }
        }
    }
    else
    {
        result.makeOpaque();
    }

    return result;
}

void PDFFloatBitmap::copyChannel(const PDFFloatBitmap& sourceBitmap, uint8_t channelFrom, uint8_t channelTo)
{
    Q_ASSERT(getWidth() == sourceBitmap.getWidth());
    Q_ASSERT(getHeight() == sourceBitmap.getHeight());

    for (size_t x = 0; x < getWidth(); ++x)
    {
        for (size_t y = 0; y < getHeight(); ++y)
        {
            PDFConstColorBuffer sourceProcessColorBuffer = sourceBitmap.getPixel(x, y);
            PDFColorBuffer targetProcessColorBuffer = getPixel(x, y);

            targetProcessColorBuffer[channelTo] = sourceProcessColorBuffer[channelFrom];
        }
    }
}

PDFFloatBitmap PDFFloatBitmap::resize(size_t width, size_t height, Qt::TransformationMode mode) const
{
    if (width == 0 || height == 0)
    {
        return PDFFloatBitmap();
    }

    PDFFloatBitmap bitmap(width, height, getPixelFormat());

    const qreal pixelRatioH = qreal(getWidth()) / qreal(width);
    const qreal pixelRatioV = qreal(getHeight()) / qreal(height);

    switch (mode)
    {
        case Qt::FastTransformation:
        {
            for (size_t yDest = 0; yDest < height; ++yDest)
            {
                for (size_t xDest = 0; xDest < width; ++xDest)
                {
                    size_t xSrc = qFloor(pixelRatioH * xDest);
                    size_t ySrc = qFloor(pixelRatioV * yDest);

                    PDFConstColorBuffer srcBuffer = getPixel(xSrc, ySrc);
                    PDFColorBuffer destBuffer = bitmap.getPixel(xDest, yDest);

                    Q_ASSERT(srcBuffer.size() == destBuffer.size());

                    // Just copy the color
                    std::copy(srcBuffer.cbegin(), srcBuffer.cend(), destBuffer.begin());
                }
            }

            break;
        }

        case Qt::SmoothTransformation:
        {
            const size_t pixelCount = getPixelSize();
            std::vector<PDFColorComponent> buffer(pixelCount, 0.0f);

            for (size_t yDest = 0; yDest < height; ++yDest)
            {
                for (size_t xDest = 0; xDest < width; ++xDest)
                {
                    const qreal xOrdinateStart = pixelRatioH * xDest;
                    const qreal xOrdinateEnd = xOrdinateStart + pixelRatioH;
                    const qreal yOrdinateStart = pixelRatioV * yDest;
                    const qreal yOrdinateEnd = yOrdinateStart + pixelRatioV;

                    size_t xSrcStart = qFloor(xOrdinateStart);
                    size_t xSrcEnd = qMin<qreal>(qCeil(xOrdinateEnd), getWidth());
                    size_t ySrcStart = qFloor(yOrdinateStart);
                    size_t ySrcEnd = qMin<qreal>(qCeil(yOrdinateEnd), getHeight());

                    std::fill(buffer.begin(), buffer.end(), 0.0f);

                    qreal sumPortion = 0.0;

                    for (size_t i = xSrcStart; i < xSrcEnd; ++i)
                    {
                        const qreal xSubpixelStart = qMax(qreal(i), xOrdinateStart);
                        const qreal xSubpixelEnd = qMin(qreal(i + 1), xOrdinateEnd);
                        const qreal xPortion = xSubpixelEnd - xSubpixelStart;

                        for (size_t j = ySrcStart; j < ySrcEnd; ++j)
                        {
                            const qreal ySubpixelStart = qMax(qreal(j), yOrdinateStart);
                            const qreal ySubpixelEnd = qMin(qreal(j + 1), yOrdinateEnd);
                            const qreal yPortion = ySubpixelEnd - ySubpixelStart;
                            const qreal pixelPortion = xPortion * yPortion;

                            PDFConstColorBuffer srcBuffer = getPixel(i, j);

                            for (size_t k = 0; k < pixelCount; ++k)
                            {
                                buffer[k] += srcBuffer[k] * pixelPortion;
                            }

                            sumPortion += pixelPortion;
                        }
                    }

                    // Compute weighed sum of pixels
                    const qreal coefficient = qFuzzyIsNull(sumPortion) ? 0.0 : 1.0 / sumPortion;
                    for (size_t k = 0; k < pixelCount; ++k)
                    {
                        buffer[k] *= coefficient;
                    }

                    PDFColorBuffer destBuffer = bitmap.getPixel(xDest, yDest);
                    Q_ASSERT(buffer.size() == destBuffer.size());
                    std::copy(buffer.cbegin(), buffer.cend(), destBuffer.begin());
                }
            }

            break;
        }

        default:
            Q_ASSERT(false);
            break;
    }

    return bitmap;
}

void PDFFloatBitmap::blend(const PDFFloatBitmap& source,
                           PDFFloatBitmap& target,
                           const PDFFloatBitmap& backdrop,
                           const PDFFloatBitmap& initialBackdrop,
                           const PDFFloatBitmap& blendSoftMask,
                           bool alphaIsShape,
                           PDFColorComponent constantAlpha,
                           BlendMode mode,
                           bool knockoutGroup,
                           OverprintMode overprintMode,
                           QRect blendRegion)
{
    Q_ASSERT(source.getWidth() == target.getWidth());
    Q_ASSERT(source.getHeight() == target.getHeight());
    Q_ASSERT(source.getPixelFormat() == target.getPixelFormat());
    Q_ASSERT(source.getWidth() == blendSoftMask.getWidth());
    Q_ASSERT(source.getHeight() == blendSoftMask.getHeight());
    Q_ASSERT(blendSoftMask.getPixelFormat() == PDFPixelFormat::createOpacityMask());

    Q_ASSERT(blendRegion.left() >= 0);
    Q_ASSERT(blendRegion.top() >= 0);
    Q_ASSERT(static_cast<std::size_t>( blendRegion.right() ) < source.getWidth());
    Q_ASSERT(static_cast< std::size_t >( blendRegion.bottom() ) < source.getHeight());

    const PDFPixelFormat pixelFormat = source.getPixelFormat();
    const uint8_t shapeChannel = pixelFormat.getShapeChannelIndex();
    const uint8_t opacityChannel = pixelFormat.getOpacityChannelIndex();
    const uint8_t colorChannelStart = pixelFormat.getColorChannelIndexStart();
    const uint8_t colorChannelEnd = pixelFormat.getColorChannelIndexEnd();
    const uint8_t processColorChannelStart = pixelFormat.getProcessColorChannelIndexStart();
    const uint8_t processColorChannelEnd = pixelFormat.getProcessColorChannelIndexEnd();
    const uint8_t spotColorChannelStart = pixelFormat.getSpotColorChannelIndexStart();
    const uint8_t spotColorChannelEnd = pixelFormat.getSpotColorChannelIndexEnd();
    std::vector<PDFColorComponent> B_i(source.getPixelSize(), 0.0f);
    std::vector<BlendMode> channelBlendModes(source.getPixelSize(), mode);

    // For blending spot colors, only white preserving blend modes are possible.
    // If this is not the case, revert spot color blend mode to normal blending.
    // See 11.7.4.2 of PDF 2.0 specification.
    if (pixelFormat.hasSpotColors() && !PDFBlendModeInfo::isWhitePreserving(mode))
    {
        auto itBegin = std::next(channelBlendModes.begin(), spotColorChannelStart);
        auto itEnd = std::next(channelBlendModes.begin(), spotColorChannelEnd);
        std::fill(itBegin, itEnd, BlendMode::Normal);
    }

    // Handle overprint mode for normal blend mode. We do not support
    // oveprinting for other blend modes, than normal.

    auto getBlendModeForPixel = [&source, &channelBlendModes, pixelFormat, overprintMode, mode](size_t x, size_t y, uint8_t channel)
    {
        switch (overprintMode)
        {
            case OverprintMode::NoOveprint:
                break;

            case OverprintMode::Overprint_Mode_0:
            {
                // Select source color, if channel is active,
                // otherwise select backdrop color.

                const uint32_t activeColorChannels = source.hasActiveColorMask() ? source.getPixelActiveColorMask(x, y) : PDFPixelFormat::getAllColorsMask();
                uint32_t flag = (static_cast<uint32_t>(1)) << channel;
                if (channelBlendModes[channel] == BlendMode::Normal && !(activeColorChannels & flag))
                {
                    // Color channel is inactive
                    return BlendMode::Overprint_SelectBackdrop;
                }

                break;
            }

            case OverprintMode::Overprint_Mode_1:
            {
                // For process colors, select source color, if it is nonzero,
                // otherwise select backdrop. If process color channel is inactive,
                // select backdrop.

                const uint32_t activeColorChannels = source.hasActiveColorMask() ? source.getPixelActiveColorMask(x, y) : PDFPixelFormat::getAllColorsMask();

                if (pixelFormat.hasProcessColors() && mode == BlendMode::Normal &&
                    channel >= pixelFormat.getProcessColorChannelIndexStart() && channel < pixelFormat.getProcessColorChannelIndexEnd())
                {
                    uint32_t flag = (static_cast<uint32_t>(1)) << channel;
                    if (!(activeColorChannels & flag))
                    {
                        // Color channel is inactive
                        return BlendMode::Overprint_SelectBackdrop;
                    }
                    else
                    {
                        // Color channel is active, but select source color only, if it is nonzero
                        return pixelFormat.hasProcessColorsSubtractive() ? BlendMode::Overprint_SelectNonOneSourceOrBackdrop
                                                                         : BlendMode::Overprint_SelectNonZeroSourceOrBackdrop;
                    }
                }

                if (pixelFormat.hasSpotColors() && channel >= pixelFormat.getSpotColorChannelIndexStart() && channel < pixelFormat.getSpotColorChannelIndexEnd())
                {
                    // For spot colors, select backdrop, if channel is inactive,
                    // otherwise select source color.

                    uint32_t flag = (static_cast<uint32_t>(1)) << channel;
                    if (channelBlendModes[channel] == BlendMode::Normal && !(activeColorChannels & flag))
                    {
                        // Color channel is inactive
                        return BlendMode::Overprint_SelectBackdrop;
                    }
                }

                break;
            }

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }

        return channelBlendModes[channel];
    };

    for (int x = blendRegion.left(); x <= blendRegion.right(); ++x)
    {
        for (int y = blendRegion.top(); y <= blendRegion.bottom(); ++y)
        {
            PDFConstColorBuffer sourceColor = source.getPixel(x, y);
            PDFColorBuffer targetColor = target.getPixel(x, y);
            PDFConstColorBuffer backdropColor = backdrop.getPixel(x, y);
            PDFConstColorBuffer initialBackdropColor = initialBackdrop.getPixel(x, y);
            PDFConstColorBuffer alphaColorBuffer = blendSoftMask.getPixel(x, y);

            const PDFColorComponent softMaskValue = alphaColorBuffer[0];
            const PDFColorComponent f_j_i = sourceColor[shapeChannel];
            const PDFColorComponent f_m_i = alphaIsShape ? softMaskValue : 1.0f;
            const PDFColorComponent f_k_i = alphaIsShape ? constantAlpha : 1.0f;
            const PDFColorComponent q_m_i = !alphaIsShape ? softMaskValue : 1.0f;
            const PDFColorComponent q_k_i = !alphaIsShape ? constantAlpha : 1.0f;
            const PDFColorComponent f_s_i = f_j_i * f_m_i * f_k_i;
            const PDFColorComponent alpha_j_i = sourceColor[opacityChannel];
            const PDFColorComponent alpha_s_i = alpha_j_i * (f_m_i * q_m_i) * (f_k_i * q_k_i);

            // Old alpha (alpha_g_i_1) is stored in target (immediate) buffer
            const PDFColorComponent alpha_g_i_1 = targetColor[opacityChannel];

            // alpha_g_0 == 0.0f according to the specification, otherwise select alpha_g_i_1 from target color
            const PDFColorComponent alpha_g_b = knockoutGroup ? 0.0f : alpha_g_i_1;

            // alpha_0 is taken from initial backdrop color buffer
            const PDFColorComponent alpha_0 = initialBackdropColor[opacityChannel];

            // f_g_i_1 is stored in target (immediate) buffer
            const PDFColorComponent f_g_i_1 = targetColor[shapeChannel];

            // Formulas taken from
            const PDFColorComponent f_g_i = PDFBlendFunction::blend_Union(f_g_i_1, f_s_i);
            const PDFColorComponent alpha_g_i = (1.0f - f_s_i) * alpha_g_i_1 + (f_s_i - alpha_s_i) * alpha_g_b + alpha_s_i;
            const PDFColorComponent alpha_i_1 = PDFBlendFunction::blend_Union(alpha_0, alpha_g_i_1);
            const PDFColorComponent alpha_i = PDFBlendFunction::blend_Union(alpha_0, alpha_g_i);

            // alpha_b is either alpha_0 (for knockout group) or alpha_i_1
            const PDFColorComponent alpha_b = knockoutGroup ? alpha_0 : alpha_i_1;

            if (qFuzzyIsNull(alpha_g_i))
            {
                // If alpha_i is zero, then color is undefined, just fill shape/opacity
                targetColor[shapeChannel] = f_g_i;
                targetColor[opacityChannel] = alpha_g_i;
                continue;
            }

            if (target.hasActiveColorMask())
            {
                const uint32_t activeColorChannels = source.hasActiveColorMask() ? source.getPixelActiveColorMask(x, y) : PDFPixelFormat::getAllColorsMask();
                target.markPixelActiveColorMask(x, y, activeColorChannels);
            }

            std::fill(B_i.begin(), B_i.end(), 0.0f);

            // Calculate blended pixel
            if (PDFBlendModeInfo::isSeparable(mode))
            {
                // Separable blend mode - process each color separately
                const bool isProcessColorSubtractive = pixelFormat.hasProcessColorsSubtractive();
                const bool isSpotColorSubtractive = pixelFormat.hasSpotColorsSubtractive();

                if (pixelFormat.hasProcessColors())
                {
                    if (!isProcessColorSubtractive)
                    {
                        for (uint8_t i = processColorChannelStart; i < processColorChannelEnd; ++i)
                        {
                            const BlendMode pixelBlendMode = getBlendModeForPixel(x, y, i);
                            B_i[i] = PDFBlendFunction::blend(pixelBlendMode, backdropColor[i], sourceColor[i]);
                        }
                    }
                    else
                    {
                        for (uint8_t i = processColorChannelStart; i < processColorChannelEnd; ++i)
                        {
                            const BlendMode pixelBlendMode = getBlendModeForPixel(x, y, i);
                            B_i[i] = 1.0f - PDFBlendFunction::blend(pixelBlendMode, 1.0f - backdropColor[i], 1.0f - sourceColor[i]);
                        }
                    }
                }

                if (pixelFormat.hasSpotColors())
                {

                    if (!isSpotColorSubtractive)
                    {
                        for (uint8_t i = spotColorChannelStart; i < spotColorChannelEnd; ++i)
                        {
                            const BlendMode pixelBlendMode = getBlendModeForPixel(x, y, i);
                            B_i[i] = PDFBlendFunction::blend(pixelBlendMode, backdropColor[i], sourceColor[i]);
                        }
                    }
                    else
                    {
                        for (uint8_t i = spotColorChannelStart; i < spotColorChannelEnd; ++i)
                        {
                            const BlendMode pixelBlendMode = getBlendModeForPixel(x, y, i);
                            B_i[i] = 1.0f - PDFBlendFunction::blend(pixelBlendMode, 1.0f - backdropColor[i], 1.0f - sourceColor[i]);
                        }
                    }
                }
            }
            else
            {
                // Nonseparable blend mode - process colors together
                if (pixelFormat.hasProcessColors())
                {
                    switch (pixelFormat.getProcessColorChannelCount())
                    {
                        case 1:
                        {
                            // Gray
                            const PDFGray Cb = backdropColor[processColorChannelStart];
                            const PDFGray Cs = sourceColor[processColorChannelStart];
                            const PDFGray blended = PDFBlendFunction::blend_Nonseparable(mode, Cb, Cs);
                            B_i[pixelFormat.getProcessColorChannelIndexStart()] = blended;
                            break;
                        }

                        case 3:
                        {
                            // RGB
                            const PDFRGB Cb = { backdropColor[processColorChannelStart + 0],
                                                backdropColor[processColorChannelStart + 1],
                                                backdropColor[processColorChannelStart + 2] };
                            const PDFRGB Cs = { sourceColor[processColorChannelStart + 0],
                                                sourceColor[processColorChannelStart + 1],
                                                sourceColor[processColorChannelStart + 2] };
                            const PDFRGB blended = PDFBlendFunction::blend_Nonseparable(mode, Cb, Cs);
                            B_i[processColorChannelStart + 0] = blended[0];
                            B_i[processColorChannelStart + 1] = blended[1];
                            B_i[processColorChannelStart + 2] = blended[2];
                            break;
                        }

                        case 4:
                        {
                            // CMYK
                            const PDFCMYK Cb = { backdropColor[processColorChannelStart + 0],
                                                 backdropColor[processColorChannelStart + 1],
                                                 backdropColor[processColorChannelStart + 2],
                                                 backdropColor[processColorChannelStart + 3] };
                            const PDFCMYK Cs = { sourceColor[processColorChannelStart + 0],
                                                 sourceColor[processColorChannelStart + 1],
                                                 sourceColor[processColorChannelStart + 2],
                                                 sourceColor[processColorChannelStart + 3] };
                            const PDFCMYK blended = PDFBlendFunction::blend_Nonseparable(mode, Cb, Cs);
                            B_i[processColorChannelStart + 0] = blended[0];
                            B_i[processColorChannelStart + 1] = blended[1];
                            B_i[processColorChannelStart + 2] = blended[2];
                            B_i[processColorChannelStart + 3] = blended[3];
                            break;
                        }

                        default:
                        {
                            // This is a serious error. Blended buffer remains unchanged (zero)
                            Q_ASSERT(false);
                            break;
                        }
                    }
                }

                if (pixelFormat.hasSpotColors())
                {
                    const bool isSpotColorSubtractive = pixelFormat.hasSpotColorsSubtractive();
                    if (!isSpotColorSubtractive)
                    {
                        for (uint8_t i = spotColorChannelStart; i < spotColorChannelEnd; ++i)
                        {
                            const BlendMode pixelBlendMode = getBlendModeForPixel(x, y, i);
                            B_i[i] = PDFBlendFunction::blend(pixelBlendMode, backdropColor[i], sourceColor[i]);
                        }
                    }
                    else
                    {
                        for (uint8_t i = spotColorChannelStart; i < spotColorChannelEnd; ++i)
                        {
                            const BlendMode pixelBlendMode = getBlendModeForPixel(x, y, i);
                            B_i[i] = 1.0f - PDFBlendFunction::blend(pixelBlendMode, 1.0f - backdropColor[i], 1.0f - sourceColor[i]);
                        }
                    }
                }
            }

            for (uint8_t i = colorChannelStart; i < colorChannelEnd; ++i)
            {
                const PDFColorComponent C_s_i = sourceColor[i];
                const PDFColorComponent C_b = backdropColor[i];
                const PDFColorComponent C_i_1 = targetColor[i];

                PDFColorComponent C_t = (f_s_i - alpha_s_i) * alpha_b * C_b + alpha_s_i * ((1.0f - alpha_b) * C_s_i + alpha_b * B_i[i]);
                PDFColorComponent C_i = ((1.0f - f_s_i) * alpha_i_1 * C_i_1 + C_t) / alpha_i;

                targetColor[i] = C_i;
            }

            targetColor[shapeChannel] = f_g_i;
            targetColor[opacityChannel] = alpha_g_i;
        }
    }
}

void PDFFloatBitmap::blendConvertedSpots(const PDFFloatBitmap& convertedSpotColors)
{
    Q_ASSERT(convertedSpotColors.getPixelFormat().getProcessColorChannelCount() == m_format.getProcessColorChannelCount());

    const uint8_t processColorChannelStart = m_format.getProcessColorChannelIndexStart();
    const uint8_t processColorChannelEnd = m_format.getProcessColorChannelIndexEnd();

    const PDFColorComponent* sourcePixel = convertedSpotColors.begin();
    for (PDFColorComponent* targetPixel = begin(); targetPixel != end(); targetPixel += m_pixelSize, sourcePixel+= convertedSpotColors.getPixelSize())
    {
        for (uint8_t i = processColorChannelStart; i < processColorChannelEnd; ++i)
        {
            if (m_format.hasProcessColorsSubtractive())
            {
                targetPixel[i] = PDFBlendFunction::blend_Union(targetPixel[i], sourcePixel[i]);
            }
            else
            {
                targetPixel[i] = targetPixel[i] * sourcePixel[i];
            }
        }
    }
}

void PDFFloatBitmap::fillProcessColorChannels(PDFColorComponent value)
{
    if (!m_format.hasProcessColors())
    {
        // No process colors
        return;
    }

    const uint8_t channelStart = m_format.getProcessColorChannelIndexStart();
    const uint8_t channelEnd = m_format.getProcessColorChannelIndexEnd();

    for (PDFColorComponent* pixel = begin(); pixel != end(); pixel += m_pixelSize)
    {
        std::fill(pixel + channelStart, pixel + channelEnd, value);
    }
}

void PDFFloatBitmap::fillChannel(size_t channel, PDFColorComponent value)
{
    // Do we have just one channel?
    if (m_format.getChannelCount() == 1)
    {
        Q_ASSERT(channel == 0);
        std::fill(m_data.begin(), m_data.end(), value);
        return;
    }

    for (PDFColorComponent* pixel = begin(); pixel != end(); pixel += m_pixelSize)
    {
        pixel[channel] = value;
    }
}

PDFFloatBitmap PDFFloatBitmap::createOpaqueSoftMask(size_t width, size_t height)
{
    PDFFloatBitmap result(width, height, PDFPixelFormat::createOpacityMask());
    result.makeOpaque();
    return result;
}

PDFFloatBitmapWithColorSpace::PDFFloatBitmapWithColorSpace()
{

}

PDFFloatBitmapWithColorSpace::PDFFloatBitmapWithColorSpace(size_t width, size_t height, PDFPixelFormat format) :
    PDFFloatBitmap(width, height, format)
{

}

PDFFloatBitmapWithColorSpace::PDFFloatBitmapWithColorSpace(size_t width, size_t height, PDFPixelFormat format, PDFColorSpacePointer blendColorSpace) :
    PDFFloatBitmap(width, height, format),
    m_colorSpace(blendColorSpace)
{

}

PDFColorSpacePointer PDFFloatBitmapWithColorSpace::getColorSpace() const
{
    return m_colorSpace;
}

void PDFFloatBitmapWithColorSpace::setColorSpace(const PDFColorSpacePointer& colorSpace)
{
    m_colorSpace = colorSpace;
}

void PDFFloatBitmapWithColorSpace::convertToColorSpace(const PDFCMS* cms,
                                                       RenderingIntent intent,
                                                       const PDFColorSpacePointer& targetColorSpace,
                                                       PDFRenderErrorReporter* reporter)
{
    Q_ASSERT(m_colorSpace);
    if (m_colorSpace->equals(targetColorSpace.get()))
    {
        return;
    }

    const uint8_t targetDeviceColors = static_cast<uint8_t>(targetColorSpace->getColorComponentCount());
    PDFPixelFormat newFormat = getPixelFormat();
    newFormat.setProcessColors(targetDeviceColors);
    newFormat.setProcessColorsSubtractive(targetDeviceColors == 4);

    PDFFloatBitmap sourceProcessColors = extractProcessColors();
    PDFFloatBitmap targetProcessColors(sourceProcessColors.getWidth(), sourceProcessColors.getHeight(), PDFPixelFormat::createFormat(targetDeviceColors, 0, false, newFormat.hasProcessColorsSubtractive(), newFormat.hasActiveColorMask()));

    if (!PDFAbstractColorSpace::transform(m_colorSpace.data(), targetColorSpace.data(), cms, intent, sourceProcessColors.getPixels(), targetProcessColors.getPixels(), reporter))
    {
        reporter->reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Transformation between blending color space failed."));
    }

    PDFFloatBitmapWithColorSpace temporary(getWidth(), getHeight(), newFormat, targetColorSpace);
    for (size_t x = 0; x < getWidth(); ++x)
    {
        for (size_t y = 0; y < getHeight(); ++y)
        {
            PDFColorBuffer sourceProcessColorBuffer = targetProcessColors.getPixel(x, y);
            PDFColorBuffer sourceSpotColorAndOpacityBuffer = getPixel(x, y);
            PDFColorBuffer targetBuffer = temporary.getPixel(x, y);

            Q_ASSERT(sourceProcessColorBuffer.size() <= targetBuffer.size());

            // Copy process colors
            PDFColorComponent* targetIt = targetBuffer.begin();
            targetIt = std::copy(sourceProcessColorBuffer.cbegin(), sourceProcessColorBuffer.cend(), targetIt);

            Q_ASSERT(std::distance(targetIt, targetBuffer.end()) == temporary.getPixelFormat().getSpotColorChannelCount() + temporary.getPixelFormat().getAuxiliaryChannelCount());

            const PDFColorComponent* sourceIt = std::next(sourceSpotColorAndOpacityBuffer.cbegin(), getPixelFormat().getProcessColorChannelCount());
            targetIt = std::copy(sourceIt, sourceSpotColorAndOpacityBuffer.cend(), targetIt);

            Q_ASSERT(targetIt == targetBuffer.cend());
        }
    }

    // Simplification - set all color channels active
    temporary.setAllColorActive();
    *this = qMove(temporary);
}

PDFTransparencyRenderer::PDFTransparencyRenderer(const PDFPage* page,
                                                 const PDFDocument* document,
                                                 const PDFFontCache* fontCache,
                                                 const PDFCMS* cms,
                                                 const PDFOptionalContentActivity* optionalContentActivity,
                                                 const PDFInkMapper* inkMapper,
                                                 PDFTransparencyRendererSettings settings,
                                                 QTransform pagePointToDevicePointMatrix) :
    BaseClass(page, document, fontCache, cms, optionalContentActivity, pagePointToDevicePointMatrix, PDFMeshQualitySettings()),
    m_inkMapper(inkMapper),
    m_active(false),
    m_settings(settings)
{
    m_deviceColorSpace.reset(new PDFDeviceRGBColorSpace());
    m_processColorSpace.reset(new PDFDeviceCMYKColorSpace());
}

void PDFTransparencyRenderer::setDeviceColorSpace(PDFColorSpacePointer colorSpace)
{
    if (!colorSpace || colorSpace->isBlendColorSpace())
    {
        // Set device color space only, when it is a blend color space
        m_deviceColorSpace = colorSpace;
    }
}

void PDFTransparencyRenderer::setProcessColorSpace(PDFColorSpacePointer colorSpace)
{
    if (!colorSpace || colorSpace->isBlendColorSpace())
    {
        // Set process color space only, when it is a blend color space
        m_processColorSpace = colorSpace;
    }
}

void PDFTransparencyRenderer::beginPaint(QSize pixelSize)
{
    Q_ASSERT(!m_active);
    m_active = true;

    Q_ASSERT(pixelSize.isValid());
    Q_ASSERT(m_deviceColorSpace);
    Q_ASSERT(m_processColorSpace);

    m_originalProcessBitmap = PDFFloatBitmapWithColorSpace();
    m_transparencyGroupDataStack.clear();
    m_painterStateStack.push(PDFTransparencyPainterState());

    // Initialize initial opaque soft mask
    PDFFloatBitmap initialSoftMaskBitmap;
    createOpaqueSoftMask(initialSoftMaskBitmap, pixelSize.width(), pixelSize.height());
    m_painterStateStack.top().softMask = PDFTransparencySoftMask(true, qMove(initialSoftMaskBitmap));

    PDFPixelFormat pixelFormat = PDFPixelFormat::createFormat(uint8_t(m_deviceColorSpace->getColorComponentCount()),
                                                              uint8_t(m_inkMapper->getActiveSpotColorCount()),
                                                              true, m_deviceColorSpace->getColorComponentCount() == 4,
                                                              true);

    PDFFloatBitmapWithColorSpace paper = PDFFloatBitmapWithColorSpace(pixelSize.width(), pixelSize.height(), pixelFormat, m_deviceColorSpace);
    paper.makeColorWhite();

    PDFTransparencyGroupPainterData deviceGroup;
    deviceGroup.alphaIsShape = getGraphicState()->getAlphaIsShape();
    deviceGroup.alphaStroke = getGraphicState()->getAlphaStroking();
    deviceGroup.alphaFill = getGraphicState()->getAlphaFilling();
    deviceGroup.blendMode = getGraphicState()->getBlendMode();
    deviceGroup.blackPointCompensationMode = getGraphicState()->getBlackPointCompensationMode();
    deviceGroup.renderingIntent = RenderingIntent::RelativeColorimetric;
    deviceGroup.initialBackdrop = qMove(paper);
    deviceGroup.immediateBackdrop = deviceGroup.initialBackdrop;
    deviceGroup.blendColorSpace = m_deviceColorSpace;

    m_transparencyGroupDataStack.emplace_back(qMove(deviceGroup));

    // Create page transparency group
    PDFObject pageTransparencyGroupObject = getPage()->getTransparencyGroup(&getDocument()->getStorage());
    PDFTransparencyGroup transparencyGroup = parseTransparencyGroup(pageTransparencyGroupObject);
    transparencyGroup.isolated = true;

    if (!transparencyGroup.colorSpacePointer)
    {
        transparencyGroup.colorSpacePointer = m_processColorSpace;
    }

    m_pageTransparencyGroupGuard.reset(new PDFTransparencyGroupGuard(this, qMove(transparencyGroup)));
    m_transparencyGroupDataStack.back().filterColorsUsingMask = (m_settings.flags.testFlag(PDFTransparencyRendererSettings::ActiveColorMask) &&
                                                                 m_settings.activeColorMask != PDFPixelFormat::getAllColorsMask());
    m_transparencyGroupDataStack.back().activeColorMask = m_settings.activeColorMask;
    m_transparencyGroupDataStack.back().transformSpotsToDevice = m_settings.flags.testFlag(PDFTransparencyRendererSettings::SeparationSimulation);
    m_transparencyGroupDataStack.back().saveOriginalImage = m_settings.flags.testFlag(PDFTransparencyRendererSettings::SaveOriginalProcessImage);
}

const PDFFloatBitmap& PDFTransparencyRenderer::endPaint()
{
    Q_ASSERT(m_active);
    m_textTransparencyGroupGuard.reset(); // Just safeguard - ET operator may not be present
    m_pageTransparencyGroupGuard.reset();
    m_active = false;
    m_painterStateStack.pop();

    return *getImmediateBackdrop();
}

QImage PDFTransparencyRenderer::toImageImpl(const PDFFloatBitmapWithColorSpace& floatImage, bool use16Bit) const
{
    QImage image;

    Q_ASSERT(floatImage.getPixelFormat().getProcessColorChannelCount() == 3);

    if (use16Bit)
    {
        image = QImage(int(floatImage.getWidth()), int(floatImage.getHeight()), QImage::Format_RGBA64);

        const PDFPixelFormat pixelFormat = floatImage.getPixelFormat();
        const int height = image.height();
        const int width = image.width();
        const float scale = std::numeric_limits<quint16>::max();
        const uint8_t channelStart = pixelFormat.getProcessColorChannelIndexStart();
        const uint8_t channelEnd = pixelFormat.getProcessColorChannelIndexEnd();
        const uint8_t opacityChannel = pixelFormat.getOpacityChannelIndex();

        for (int y = 0; y < height; ++y)
        {
            quint16* pixels = reinterpret_cast<quint16*>(image.bits() + y * image.bytesPerLine());

            for (int x = 0; x < width; ++x)
            {
                PDFConstColorBuffer colorBuffer = floatImage.getPixel(x, y);

                for (uint8_t channel = channelStart; channel < channelEnd; ++channel)
                {
                    *pixels++ = quint16(colorBuffer[channel] * scale);
                }

                *pixels++ = quint16(colorBuffer[opacityChannel] * scale);
            }
        }
    }
    else
    {
        image = QImage(int(floatImage.getWidth()), int(floatImage.getHeight()), QImage::Format_RGBA8888);

        const PDFPixelFormat pixelFormat = floatImage.getPixelFormat();
        const int height = image.height();
        const int width = image.width();
        const float scale = std::numeric_limits<quint8>::max();
        const uint8_t channelStart = pixelFormat.getProcessColorChannelIndexStart();
        const uint8_t channelEnd = pixelFormat.getProcessColorChannelIndexEnd();
        const uint8_t opacityChannel = pixelFormat.getOpacityChannelIndex();

        for (int y = 0; y < height; ++y)
        {
            quint8* pixels = reinterpret_cast<quint8*>(image.bits() + y * image.bytesPerLine());

            for (int x = 0; x < width; ++x)
            {
                PDFConstColorBuffer colorBuffer = floatImage.getPixel(x, y);

                for (uint8_t channel = channelStart; channel < channelEnd; ++channel)
                {
                    *pixels++ = quint8(colorBuffer[channel] * scale);
                }

                *pixels++ = quint8(colorBuffer[opacityChannel] * scale);
            }
        }
    }

    return image;
}

QImage PDFTransparencyRenderer::toImage(bool use16Bit, bool usePaper, const PDFRGB& paperColor) const
{
    QImage image;

    if (m_transparencyGroupDataStack.size() == 1 && // We have finished the painting
        getImmediateBackdrop()->getPixelFormat().getProcessColorChannelCount() == 3) // We have exactly three process colors (RGB)
    {
        const PDFFloatBitmapWithColorSpace& floatImage = *getImmediateBackdrop();
        Q_ASSERT(floatImage.getPixelFormat().hasOpacityChannel());

        if (!usePaper)
        {
            return toImageImpl(floatImage, use16Bit);
        }

        PDFFloatBitmapWithColorSpace paperImage(floatImage.getWidth(), floatImage.getHeight(), floatImage.getPixelFormat(), floatImage.getColorSpace());
        createPaperBitmap(paperImage, paperColor);

        PDFFloatBitmap imageSoftMask;
        createOpaqueSoftMask(imageSoftMask, paperImage.getWidth(), paperImage.getHeight());

        QRect blendRegion(0, 0, int(floatImage.getWidth()), int(floatImage.getHeight()));
        PDFFloatBitmapWithColorSpace::blend(floatImage, paperImage, paperImage, paperImage, imageSoftMask, false, 1.0f, BlendMode::Normal, false, PDFFloatBitmap::OverprintMode::NoOveprint, blendRegion);

        return toImageImpl(paperImage, use16Bit);
    }

    return image;
}

void PDFTransparencyRenderer::clearColor(const PDFColor& color)
{
    PDFFloatBitmapWithColorSpace* backdrop = getImmediateBackdrop();
    const PDFPixelFormat pixelFormat = backdrop->getPixelFormat();

    const uint8_t processColorChannelStart = pixelFormat.getProcessColorChannelIndexStart();
    const uint8_t processColorChannelEnd = pixelFormat.getProcessColorChannelIndexEnd();

    for (uint8_t i = processColorChannelStart; i < processColorChannelEnd; ++i)
    {
        if (i >= color.size())
        {
            reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Invalid clear color - process color %1 was not found in clear color.").arg(i));
            return;
        }

        backdrop->fillChannel(i, color[i]);
    }

    if (color.size() > pixelFormat.getProcessColorChannelCount())
    {
        reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("More colors in clear color (%1) than process color channel count (%2).").arg(color.size()).arg(pixelFormat.getProcessColorChannelCount()));
    }
}

bool PDFTransparencyRenderer::isContentKindSuppressed(ContentKind kind) const
{
    switch (kind)
    {
        case ContentKind::Shapes:
            if (!m_settings.flags.testFlag(PDFTransparencyRendererSettings::DisplayVectorGraphics))
            {
                return true;
            }
            break;

        case ContentKind::Text:
            if (!m_settings.flags.testFlag(PDFTransparencyRendererSettings::DisplayText))
            {
                return true;
            }
            break;

        case ContentKind::Images:
            if (!m_settings.flags.testFlag(PDFTransparencyRendererSettings::DisplayImages))
            {
                return true;
            }
            break;

        case ContentKind::Shading:
            if (!m_settings.flags.testFlag(PDFTransparencyRendererSettings::DisplayShadings))
            {
                return true;
            }
            break;

        case ContentKind::Tiling:
            if (!m_settings.flags.testFlag(PDFTransparencyRendererSettings::DisplayTilingPatterns))
            {
                return true;
            }
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    return BaseClass::isContentKindSuppressed(kind);
}

void PDFTransparencyRenderer::performPixelSampling(const PDFReal shape,
                                                   const PDFReal opacity,
                                                   const uint8_t shapeChannel,
                                                   const uint8_t opacityChannel,
                                                   const uint8_t colorChannelStart,
                                                   const uint8_t colorChannelEnd,
                                                   int x,
                                                   int y,
                                                   const PDFMappedColor& fillColor,
                                                   const PDFPainterPathSampler& clipSampler,
                                                   const PDFPainterPathSampler& pathSampler)
{
    const PDFColorComponent clipValue = clipSampler.sample(QPoint(x, y));
    const PDFColorComponent objectShapeValue = pathSampler.sample(QPoint(x, y));
    const PDFColorComponent shapeValue = objectShapeValue * clipValue * shape;

    if (shapeValue > 0.0f)
    {
        // We consider old object shape - we use Union function to
        // set shape channel value.

        PDFColorBuffer pixel = m_drawBuffer.getPixel(x, y);
        pixel[shapeChannel] = PDFBlendFunction::blend_Union(shapeValue, pixel[shapeChannel]);
        pixel[opacityChannel] = pixel[shapeChannel]  * opacity;

        // Copy color
        for (uint8_t colorChannelIndex = colorChannelStart; colorChannelIndex < colorChannelEnd; ++colorChannelIndex)
        {
            pixel[colorChannelIndex] = fillColor.mappedColor[colorChannelIndex];
        }

        m_drawBuffer.markPixelActiveColorMask(x, y, fillColor.activeChannels);
    }
}

void PDFTransparencyRenderer::performFillFragmentFromTexture(const PDFReal shape,
                                                             const PDFReal opacity,
                                                             const uint8_t shapeChannel,
                                                             const uint8_t opacityChannel,
                                                             const uint8_t colorChannelStart,
                                                             const uint8_t colorChannelEnd,
                                                             int x,
                                                             int y,
                                                             const QTransform& worldToTextureMatrix,
                                                             const PDFFloatBitmap& texture,
                                                             const PDFPainterPathSampler& clipSampler)
{
    // Get pixel buffer from texture
    QPointF sourcePoint(x, y);
    QPointF texturePoint = sourcePoint * worldToTextureMatrix;

    if (texturePoint.x() < 0.0 ||
        texturePoint.x() >= texture.getWidth() ||
        texturePoint.y() < 0.0 ||
        texturePoint.y() >= texture.getHeight())
    {
        // Fragment is outside of the texture
        return;
    }

    const size_t texelCoordinateX = qFloor(texturePoint.x());
    const size_t texelCoordinateY = qFloor(texturePoint.y());

    PDFConstColorBuffer texel = texture.getPixel(texelCoordinateX, texelCoordinateY);

    const PDFColorComponent clipValue = clipSampler.sample(QPoint(x, y));
    const PDFColorComponent objectShapeValue = texel[shapeChannel];
    const PDFColorComponent objectOpacityValue = texel[opacityChannel];
    const PDFColorComponent shapeValue = objectShapeValue * clipValue * shape;
    const PDFColorComponent opacityValue = objectOpacityValue * clipValue * shape * opacity;

    if (shapeValue > 0.0f)
    {
        // We consider old object shape - we use Union function to
        // set shape channel value.

        PDFColorBuffer pixel = m_drawBuffer.getPixel(x, y);
        pixel[shapeChannel] = PDFBlendFunction::blend_Union(shapeValue, pixel[shapeChannel]);
        pixel[opacityChannel] = opacityValue;

        // Copy color
        for (uint8_t colorChannelIndex = colorChannelStart; colorChannelIndex < colorChannelEnd; ++colorChannelIndex)
        {
            pixel[colorChannelIndex] = texel[colorChannelIndex];
        }

        m_drawBuffer.markPixelActiveColorMask(x, y, texture.getPixelActiveColorMask(texelCoordinateX, texelCoordinateY));
    }
}

void PDFTransparencyRenderer::collapseSpotColorsToDeviceColors(PDFFloatBitmapWithColorSpace& bitmap)
{
    PDFPixelFormat pixelFormat = bitmap.getPixelFormat();

    if (!pixelFormat.hasSpotColors())
    {
        return;
    }

    const uint8_t spotColorIndexStart = pixelFormat.getSpotColorChannelIndexStart();
    const uint8_t spotColorIndexEnd = pixelFormat.getSpotColorChannelIndexEnd();

    for (uint8_t i = spotColorIndexStart; i < spotColorIndexEnd; ++i)
    {
        // Collapse spot color
        const PDFInkMapper::ColorInfo* spotColor = m_inkMapper->getActiveSpotColor(i - spotColorIndexStart);
        Q_ASSERT(spotColor);

        switch (spotColor->colorSpace->getColorSpace())
        {
            case PDFAbstractColorSpace::ColorSpace::Separation:
            {
                PDFFloatBitmap spotColorBitmap = bitmap.extractSpotChannel(i);
                PDFFloatBitmap processColorBitmap(spotColorBitmap.getWidth(), spotColorBitmap.getHeight(), PDFPixelFormat::createFormat(pixelFormat.getProcessColorChannelCount(), 0, false, pixelFormat.hasProcessColorsSubtractive(), false));
                if (!PDFAbstractColorSpace::transform(spotColor->colorSpace.data(), bitmap.getColorSpace().data(), getCMS(), getGraphicState()->getRenderingIntent(), spotColorBitmap.getPixels(), processColorBitmap.getPixels(), this))
                {
                    reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Transformation of spot color to blend color space failed."));
                }

                bitmap.blendConvertedSpots(processColorBitmap);
                break;
            }

            case PDFAbstractColorSpace::ColorSpace::DeviceN:
            {
                PDFFloatBitmap deviceNBitmap(bitmap.getWidth(), bitmap.getHeight(), PDFPixelFormat::createFormat(uint8_t(spotColor->colorSpace->getColorComponentCount()), 0, false, true, false));
                PDFFloatBitmap processColorBitmap(bitmap.getWidth(), bitmap.getHeight(), PDFPixelFormat::createFormat(pixelFormat.getProcessColorChannelCount(), 0, false, pixelFormat.hasProcessColorsSubtractive(), false));

                deviceNBitmap.copyChannel(bitmap, i, spotColor->colorSpaceIndex);

                if (!PDFAbstractColorSpace::transform(spotColor->colorSpace.data(), bitmap.getColorSpace().data(), getCMS(), getGraphicState()->getRenderingIntent(), deviceNBitmap.getPixels(), processColorBitmap.getPixels(), this))
                {
                    reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Transformation of spot color to blend color space failed."));
                }

                bitmap.blendConvertedSpots(processColorBitmap);
                break;
            }

            default:
                reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Transformation of spot color to blend color space failed."));
                break;
        }

    }
}

PDFFloatBitmapWithColorSpace PDFTransparencyRenderer::getImage(const PDFImage& sourceImage)
{
    PDFFloatBitmapWithColorSpace bitmap;

    const PDFImageData& imageData = sourceImage.getImageData();

    if (imageData.isValid())
    {
        const bool isImageMask = imageData.getMaskingType() == PDFImageData::MaskingType::ImageMask;
        if (sourceImage.getColorSpace() && !isImageMask)
        {
            bitmap = getColoredImage(sourceImage);
        }
        else if (isImageMask)
        {
            if (imageData.getBitsPerComponent() != 1)
            {
                throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid number bits of image mask (should be 1 bit instead of %1 bits).").arg(imageData.getBitsPerComponent()));
            }

            if (imageData.getWidth() == 0 || imageData.getHeight() == 0)
            {
                throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid size of image (%1x%2)").arg(imageData.getWidth()).arg(imageData.getHeight()));
            }

            bitmap = PDFFloatBitmapWithColorSpace(imageData.getWidth(), imageData.getHeight(), m_drawBuffer.getPixelFormat(), getBlendColorSpace());
            const bool flip01 = !imageData.getDecode().empty() && qFuzzyCompare(imageData.getDecode().front(), 1.0);
            PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());

            PDFPixelFormat pixelFormat = bitmap.getPixelFormat();
            const PDFMappedColor& fillColor = getMappedFillColor();
            Q_ASSERT(fillColor.mappedColor.size() == pixelFormat.getColorChannelCount());

            for (size_t i = pixelFormat.getColorChannelIndexStart(); i < pixelFormat.getColorChannelIndexEnd(); ++i)
            {
                bitmap.fillChannel(i, fillColor.mappedColor[i]);
            }

            Q_ASSERT(pixelFormat.hasShapeChannel());
            Q_ASSERT(pixelFormat.hasOpacityChannel());

            const uint8_t shapeChannelIndex = pixelFormat.getShapeChannelIndex();
            const uint8_t opacityChannelIndex = pixelFormat.getOpacityChannelIndex();

            const bool alphaIsShape = getGraphicState()->getAlphaIsShape();

            for (unsigned int i = 0, rowCount = imageData.getHeight(); i < rowCount; ++i)
            {
                reader.seek(i * imageData.getStride());

                for (unsigned int j = 0, colCount = imageData.getWidth(); j < colCount; ++j)
                {
                    PDFColorBuffer buffer = bitmap.getPixel(j, i);

                    const bool transparent = flip01 != static_cast<bool>(reader.read());

                    if (alphaIsShape)
                    {
                        const PDFColorComponent shapeValue = transparent ? 0.0f : 1.0f;
                        const PDFColorComponent opacityValue = shapeValue;
                        buffer[shapeChannelIndex] = shapeValue;
                        buffer[opacityChannelIndex] = opacityValue;
                    }
                    else
                    {
                        const PDFColorComponent shapeValue = 1.0f;
                        const PDFColorComponent opacityValue = transparent ? 0.0f : 1.0f;
                        buffer[shapeChannelIndex] = shapeValue;
                        buffer[opacityChannelIndex] = opacityValue;
                    }
                }
            }

            bitmap.setColorActivity(fillColor.activeChannels);
        }
    }

    return bitmap;
}

PDFFloatBitmapWithColorSpace PDFTransparencyRenderer::convertImageToBlendSpace(const PDFFloatBitmapWithColorSpace& image)
{
    PDFFloatBitmapWithColorSpace convertedImage(image.getWidth(), image.getHeight(), m_drawBuffer.getPixelFormat(), getBlendColorSpace());
    auto imageColorSpace = image.getColorSpace();
    Q_ASSERT(imageColorSpace);
    const PDFFloatBitmapWithColorSpace* sourceImage = &image;
    PDFFloatBitmapWithColorSpace temporaryImage;

    PDFInkMapping inkMapping = m_inkMapper->createMapping(imageColorSpace.data(), getBlendColorSpace().data(), m_drawBuffer.getPixelFormat());
    if (!inkMapping.isValid())
    {
        temporaryImage = image;
        temporaryImage.convertToColorSpace(getCMS(), getGraphicState()->getRenderingIntent(), getBlendColorSpace(), this);
        inkMapping = m_inkMapper->createMapping(getBlendColorSpace().data(), getBlendColorSpace().data(), m_drawBuffer.getPixelFormat());
        sourceImage = &temporaryImage;
    }

    Q_ASSERT(inkMapping.isValid());

    const uint8_t sourceBufferShapeChannelIndex = sourceImage->getPixelFormat().getShapeChannelIndex();
    const uint8_t sourceBufferOpacityChannelIndex = sourceImage->getPixelFormat().getOpacityChannelIndex();
    const uint8_t targetBufferShapeChannelIndex = convertedImage.getPixelFormat().getShapeChannelIndex();
    const uint8_t targetBufferOpacityChannelIndex = convertedImage.getPixelFormat().getOpacityChannelIndex();

    for (size_t y = 0; y < sourceImage->getHeight(); ++y)
    {
        for (size_t x = 0; x < sourceImage->getWidth(); ++x)
        {
            PDFConstColorBuffer sourceBuffer = sourceImage->getPixel(x, y);
            PDFColorBuffer targetBuffer = convertedImage.getPixel(x,  y);

            for (const PDFInkMapping::Mapping& ink : inkMapping.mapping)
            {
                switch (ink.type)
                {
                    case pdf::PDFInkMapping::Pass:
                        targetBuffer[ink.target] = sourceBuffer[ink.source];
                        break;

                    default:
                        Q_ASSERT(false);
                        break;
                }
            }

            targetBuffer[targetBufferShapeChannelIndex] = sourceBuffer[sourceBufferShapeChannelIndex];
            targetBuffer[targetBufferOpacityChannelIndex] = sourceBuffer[sourceBufferOpacityChannelIndex];
        }
    }

    convertedImage.setColorActivity(inkMapping.activeChannels);
    return convertedImage;
}

PDFFloatBitmapWithColorSpace PDFTransparencyRenderer::getColoredImage(const PDFImage& sourceImage)
{
    PDFFloatBitmapWithColorSpace result;

    const PDFImageData& imageData = sourceImage.getImageData();
    const PDFImageData& imageSoftMask = sourceImage.getSoftMaskData();

    PDFColorSpacePointer imageColorSpace = sourceImage.getColorSpace();
    size_t colorComponentCount = imageColorSpace->getColorComponentCount();
    bool isCMYK = colorComponentCount == 4;
    const bool useSmoothImageTransformation = m_settings.flags.testFlag(PDFTransparencyRendererSettings::SmoothImageTransformation) && sourceImage.isInterpolated();

    if (!imageColorSpace)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid image color space."));
    }

    auto setColorSpaceAndMakeOpaque = [&](auto imageColorSpace)
    {
        result.setColorSpace(imageColorSpace);
        result.makeOpaque();
    };

    Q_ASSERT(imageData.isValid());
    if (imageColorSpace->getColorSpace() == PDFAbstractColorSpace::ColorSpace::Indexed)
    {
        const PDFIndexedColorSpace* indexedColorSpace = dynamic_cast<const PDFIndexedColorSpace*>(imageColorSpace.data());
        imageColorSpace = indexedColorSpace->getBaseColorSpace();

        if (!imageColorSpace)
        {
            throw PDFException(PDFTranslationContext::tr("Invalid base color space of indexed color space."));
        }

        unsigned int componentCount = imageData.getComponents();
        PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());

        if (componentCount != colorComponentCount)
        {
            throw PDFException(PDFTranslationContext::tr("Invalid colors for indexed color space. Color space has %1 colors. Provided color count is %4.").arg(colorComponentCount).arg(componentCount));
        }

        const unsigned int imageWidth = imageData.getWidth();
        const unsigned int imageHeight = imageData.getHeight();
        Q_ASSERT(componentCount == 1);

        std::vector<PDFColorComponent> colorIndices;
        colorIndices.reserve(imageData.getWidth() * imageData.getHeight());

        for (unsigned int i = 0; i < imageHeight; ++i)
        {
            reader.seek(i * imageData.getStride());

            for (unsigned int j = 0; j < imageWidth; ++j)
            {
                PDFBitReader::Value index = reader.read();
                colorIndices.push_back(index);
            }
        }

        PDFColorBuffer indicesBuffer(colorIndices.data(), colorIndices.size());
        std::vector<PDFColorComponent> transformedColors = indexedColorSpace->transformColorsToBaseColorSpace(indicesBuffer);
        colorComponentCount = imageColorSpace->getColorComponentCount();
        isCMYK = colorComponentCount == 4;

        if (transformedColors.size() != colorComponentCount * imageWidth * imageHeight)
        {
            throw PDFException(PDFTranslationContext::tr("Conversion of indexed image to base color space failed."));
        }

        result = PDFFloatBitmapWithColorSpace(imageWidth, imageHeight, PDFPixelFormat::createFormat(uint8_t(colorComponentCount), 0, true, isCMYK, false));
        setColorSpaceAndMakeOpaque(imageColorSpace);

        for (unsigned int i = 0; i < imageHeight; ++i)
        {
            for (unsigned int j = 0; j < imageWidth; ++j)
            {
                PDFColorBuffer buffer = result.getPixel(j, i);

                size_t pixelIndex = (i * imageWidth + j) * colorComponentCount;
                for (size_t k = 0; k < colorComponentCount; ++k)
                {
                    Q_ASSERT(pixelIndex + k < transformedColors.size());
                    buffer[k] = transformedColors[pixelIndex + k];
                }
            }
        }

        switch (imageData.getMaskingType())
        {
            case PDFImageData::MaskingType::None:
                break;

            case PDFImageData::MaskingType::SoftMask:
            {
                PDFFloatBitmap alphaMask = getAlphaMaskFromSoftMask(imageSoftMask);
                if (alphaMask.getWidth() != result.getWidth() || alphaMask.getHeight() != result.getHeight())
                {
                    // Scale the alpha mask, if it is masked
                    alphaMask = alphaMask.resize(result.getWidth(), result.getHeight(), useSmoothImageTransformation ? Qt::SmoothTransformation : Qt::FastTransformation);
                }
                Q_ASSERT(alphaMask.getPixelFormat().getChannelCount() == 2);
                Q_ASSERT(alphaMask.getPixelFormat().hasOpacityChannel());
                Q_ASSERT(alphaMask.getPixelFormat().hasShapeChannel());

                Q_ASSERT(result.getPixelFormat().hasShapeChannel());
                Q_ASSERT(result.getPixelFormat().hasOpacityChannel());

                const uint8_t sourceShapeChannelIndex = alphaMask.getPixelFormat().getShapeChannelIndex();
                const uint8_t sourceOpacityChannelIndex = alphaMask.getPixelFormat().getOpacityChannelIndex();
                const uint8_t targetShapeChannelIndex = result.getPixelFormat().getShapeChannelIndex();
                const uint8_t targetOpacityChannelIndex = result.getPixelFormat().getOpacityChannelIndex();

                for (size_t i = 0; i < imageHeight; ++i)
                {
                    for (unsigned int j = 0; j < imageWidth; ++j)
                    {
                        PDFColorBuffer targetBuffer = result.getPixel(j, i);
                        PDFColorBuffer alphaBuffer = alphaMask.getPixel(j, i);

                        targetBuffer[targetShapeChannelIndex] = alphaBuffer[sourceShapeChannelIndex];
                        targetBuffer[targetOpacityChannelIndex] = alphaBuffer[sourceOpacityChannelIndex];
                    }
                }

                break;
            }

            default:
                throw PDFRendererException(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image masking not implemented!"));
        }
    }
    else
    {
        switch (imageData.getMaskingType())
        {
            case PDFImageData::MaskingType::None:
            {
                result = PDFFloatBitmapWithColorSpace(imageData.getWidth(), imageData.getHeight(), PDFPixelFormat::createFormat(uint8_t(colorComponentCount), 0, true, isCMYK, false));
                setColorSpaceAndMakeOpaque(imageColorSpace);

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != colorComponentCount)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(colorComponentCount).arg(componentCount));
                }

                const std::vector<PDFReal>& decode = imageData.getDecode();
                if (!decode.empty() && decode.size() != componentCount * 2)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid size of the decode array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
                }

                const unsigned int imageWidth = imageData.getWidth();
                const unsigned int imageHeight = imageData.getHeight();

                PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());
                const PDFColorComponent max = reader.max();
                const PDFColorComponent coefficient = 1.0 / max;

                for (size_t i = 0; i < imageHeight; ++i)
                {
                    reader.seek(i * imageData.getStride());

                    for (size_t j = 0; j < imageWidth; ++j)
                    {
                        PDFColorBuffer buffer = result.getPixel(j, i);

                        for (size_t k = 0; k < componentCount; ++k)
                        {
                            PDFReal value = reader.read();

                            // Interpolate value, if it is not empty
                            if (!decode.empty())
                            {
                                buffer[k] = interpolateColors(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                            }
                            else
                            {
                                buffer[k] = value * coefficient;
                            }
                        }
                    }
                }

                break;
            }

            case PDFImageData::MaskingType::SoftMask:
            {
                result = PDFFloatBitmapWithColorSpace(imageData.getWidth(), imageData.getHeight(), PDFPixelFormat::createFormat(uint8_t(colorComponentCount), 0, true, isCMYK, false));
                result.setColorSpace(imageColorSpace);

                const bool hasMatte = !imageSoftMask.getMatte().empty();
                std::vector<PDFReal> matte = imageSoftMask.getMatte();

                if (hasMatte && matte.size() != colorComponentCount)
                {
                    reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Invalid matte color."));
                }

                matte.resize(colorComponentCount, 0.0f);

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != colorComponentCount)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(colorComponentCount).arg(componentCount));
                }

                const std::vector<PDFReal>& decode = imageData.getDecode();
                if (!decode.empty() && decode.size() != componentCount * 2)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid size of the decode array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
                }

                const unsigned int imageWidth = imageData.getWidth();
                const unsigned int imageHeight = imageData.getHeight();

                PDFFloatBitmap alphaMask = getAlphaMaskFromSoftMask(imageSoftMask);
                if (alphaMask.getWidth() != result.getWidth() || alphaMask.getHeight() != result.getHeight())
                {
                    // Scale the alpha mask, if it is masked
                    alphaMask = alphaMask.resize(result.getWidth(), result.getHeight(), useSmoothImageTransformation ? Qt::SmoothTransformation : Qt::FastTransformation);
                }
                Q_ASSERT(alphaMask.getPixelFormat().getChannelCount() == 2);
                Q_ASSERT(alphaMask.getPixelFormat().hasOpacityChannel());
                Q_ASSERT(alphaMask.getPixelFormat().hasShapeChannel());

                PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());
                const PDFColorComponent max = reader.max();
                const PDFColorComponent coefficient = 1.0 / max;

                Q_ASSERT(result.getPixelFormat().hasShapeChannel());
                Q_ASSERT(result.getPixelFormat().hasOpacityChannel());

                const uint8_t sourceShapeChannelIndex = alphaMask.getPixelFormat().getShapeChannelIndex();
                const uint8_t sourceOpacityChannelIndex = alphaMask.getPixelFormat().getOpacityChannelIndex();
                const uint8_t targetShapeChannelIndex = result.getPixelFormat().getShapeChannelIndex();
                const uint8_t targetOpacityChannelIndex = result.getPixelFormat().getOpacityChannelIndex();

                for (size_t i = 0; i < imageHeight; ++i)
                {
                    reader.seek(i * imageData.getStride());

                    for (unsigned int j = 0; j < imageWidth; ++j)
                    {
                        PDFColorBuffer targetBuffer = result.getPixel(j, i);
                        PDFColorBuffer alphaBuffer = alphaMask.getPixel(j, i);

                        for (unsigned int k = 0; k < componentCount; ++k)
                        {
                            PDFReal value = reader.read();

                            // Interpolate value, if it is not empty
                            if (!decode.empty())
                            {
                                targetBuffer[k] = interpolateColors(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                            }
                            else
                            {
                                targetBuffer[k] = value * coefficient;
                            }
                        }

                        targetBuffer[targetShapeChannelIndex] = alphaBuffer[sourceShapeChannelIndex];
                        targetBuffer[targetOpacityChannelIndex] = alphaBuffer[sourceOpacityChannelIndex];
                        const PDFColorComponent alpha = targetBuffer[targetOpacityChannelIndex];

                        // Un-premultiply with matte color, according to chapter 11.6.5.2 in PDF
                        // 2.0 specification, we use inversion of following formula:
                        //
                        //      c' = m + alpha * (c - m)
                        //
                        //  So, inversion is:
                        //
                        //      c = m + (c' - m) / alpha
                        //
                        if (hasMatte && !qFuzzyIsNull(alpha))
                        {
                            for (unsigned int k = 0; k < componentCount; ++k)
                            {
                                const PDFColorComponent m = matte[k];
                                targetBuffer[k] = qBound(0.0f, m + (targetBuffer[k] - m) / alpha, 1.0f);
                            }
                        }
                    }
                }

                break;
            }

            case PDFImageData::MaskingType::ColorKeyMasking:
            {
                result = PDFFloatBitmapWithColorSpace(imageData.getWidth(), imageData.getHeight(), PDFPixelFormat::createFormat(uint8_t(colorComponentCount), 0, true, isCMYK, false));
                setColorSpaceAndMakeOpaque(imageColorSpace);

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != colorComponentCount)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(colorComponentCount).arg(componentCount));
                }

                Q_ASSERT(componentCount > 0);
                const std::vector<PDFInteger>& colorKeyMask = imageData.getColorKeyMask();
                if (colorKeyMask.size() / 2 != componentCount)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid number of color components in color key mask. Expected %1, provided %2.").arg(2 * componentCount).arg(colorKeyMask.size()));
                }

                const std::vector<PDFReal>& decode = imageData.getDecode();
                if (!decode.empty() && decode.size() != componentCount * 2)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid size of the decoded array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
                }

                PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());

                PDFColor color;
                color.resize(componentCount);

                const PDFColorComponent max = reader.max();
                const PDFColorComponent coefficient = 1.0 / max;
                const bool alphaIsShape = getGraphicState()->getAlphaIsShape();

                const uint8_t targetShapeChannelIndex = result.getPixelFormat().getShapeChannelIndex();
                const uint8_t targetOpacityChannelIndex = result.getPixelFormat().getOpacityChannelIndex();

                for (unsigned int i = 0, rowCount = imageData.getHeight(); i < rowCount; ++i)
                {
                    reader.seek(i * imageData.getStride());

                    for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                    {
                        // Number of masked-out colors
                        unsigned int maskedColors = 0;

                        PDFColorBuffer targetBuffer = result.getPixel(j, i);

                        for (unsigned int k = 0; k < componentCount; ++k)
                        {
                            PDFBitReader::Value value = reader.read();

                            // Interpolate value, if decode is not empty
                            if (!decode.empty())
                            {
                                targetBuffer[k] = interpolateColors(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                            }
                            else
                            {
                                targetBuffer[k] = value * coefficient;
                            }

                            Q_ASSERT(2 * k + 1 < colorKeyMask.size());
                            if (static_cast<std::decay<decltype(colorKeyMask)>::type::value_type>(value) >= colorKeyMask[2 * k] &&
                                    static_cast<std::decay<decltype(colorKeyMask)>::type::value_type>(value) <= colorKeyMask[2 * k + 1])
                            {
                                ++maskedColors;
                            }
                        }

                        const PDFColorComponent alpha = (maskedColors == componentCount) ? 0.0f : 1.0f;
                        const PDFColorComponent shape = alphaIsShape ? alpha : 1.0f;

                        targetBuffer[targetShapeChannelIndex] = shape;
                        targetBuffer[targetOpacityChannelIndex] = alpha;
                    }
                }

                break;
            }

            default:
            {
                throw PDFRendererException(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image masking not implemented!"));
            }
        }
    }

    // Jakub Melka: We are mapping into draw buffer, so we must use draw buffer pixel format
    return convertImageToBlendSpace(result);
}

PDFFloatBitmap PDFTransparencyRenderer::getAlphaMaskFromSoftMask(const PDFImageData& imageSoftMask)
{
    if (imageSoftMask.getMaskingType() != PDFImageData::MaskingType::None)
    {
        throw PDFException(PDFTranslationContext::tr("Soft mask can't have masking."));
    }

    if (imageSoftMask.getWidth() < 1 || imageSoftMask.getHeight() < 1)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid size of soft mask."));
    }

    PDFFloatBitmap result(imageSoftMask.getWidth(), imageSoftMask.getHeight(), PDFPixelFormat::createFormat(0, 0, true, false, false));

    unsigned int componentCount = imageSoftMask.getComponents();
    if (componentCount != 1)
    {
        throw PDFException(PDFTranslationContext::tr("Soft mask should have only 1 color component (alpha) instead of %1.").arg(componentCount));
    }

    const std::vector<PDFReal>& decode = imageSoftMask.getDecode();
    if (!decode.empty() && decode.size() != componentCount * 2)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid size of the decode array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
    }

    PDFBitReader reader(&imageSoftMask.getData(), imageSoftMask.getBitsPerComponent());

    PDFColor color;
    color.resize(componentCount);

    const PDFColorComponent max = reader.max();
    const PDFColorComponent coefficient = 1.0 / max;
    const uint8_t targetShapeChannelIndex = result.getPixelFormat().getShapeChannelIndex();
    const uint8_t targetOpacityChannelIndex = result.getPixelFormat().getOpacityChannelIndex();
    const bool alphaIsShape = getGraphicState()->getAlphaIsShape();

    for (unsigned int i = 0, rowCount = imageSoftMask.getHeight(); i < rowCount; ++i)
    {
        reader.seek(i * imageSoftMask.getStride());

        for (unsigned int j = 0, colCount = imageSoftMask.getWidth(); j < colCount; ++j)
        {
            PDFColorComponent alpha = 0.0;
            PDFReal value = reader.read();

            // Interpolate value, if it is not empty
            if (!decode.empty())
            {
                alpha = interpolate(value, 0.0, max, decode[0], decode[1]);
            }
            else
            {
                alpha = value * coefficient;
            }

            alpha = qBound(0.0f, alpha, 1.0f);
            const PDFColorComponent shape = alphaIsShape ? alpha : 1.0f;

            PDFColorBuffer targetBuffer = result.getPixel(j, i);
            targetBuffer[targetShapeChannelIndex] = shape;
            targetBuffer[targetOpacityChannelIndex] = alpha;
        }
    }

    return result;
}

void PDFTransparencyRenderer::processSoftMask(const PDFDictionary* softMask)
{
    if (m_painterStateStack.empty())
    {
        // Jakub Melka: This occurs only in initialization phase.
        // Just quit, opaque soft mask is initialized when beginPaint is called.
        return;
    }

    if (!softMask)
    {
        // Make soft mask opaque
        getPainterState()->softMask.makeOpaque();
    }
    else
    {
        PDFSoftMaskDefinition softMaskDefinition = PDFSoftMaskDefinition::parse(softMask, this);

        if (!softMaskDefinition.getFormStream())
        {
            reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Invalind soft mask."));
            getPainterState()->softMask.makeOpaque();
            return;
        }

        // Jakub Melka: Define blend color space
        PDFColorSpacePointer blendColorSpace = softMaskDefinition.getTransparencyGroup().colorSpacePointer;
        if (!blendColorSpace)
        {
            blendColorSpace.reset(new PDFDeviceRGBColorSpace());
        }
        if (!blendColorSpace->isBlendColorSpace())
        {
            reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Invalid blend color space of soft mask definition."));
            getPainterState()->softMask.makeOpaque();
            return;
        }

        PDFInkMapper inkMapper(nullptr, getDocument());
        PDFTransparencyRenderer softMaskRenderer(getPage(), getDocument(), getFontCache(), getCMS(), getOptionalContentActivity(), &inkMapper, m_settings, getPagePointToDevicePointMatrix());
        softMaskRenderer.initializeProcessor();

        PDFPageContentProcessorState graphicState = *getGraphicState();
        graphicState.setSoftMask(nullptr);

        softMaskRenderer.setDeviceColorSpace(blendColorSpace);
        softMaskRenderer.setProcessColorSpace(blendColorSpace);

        softMaskRenderer.beginPaint(QSize(int(m_drawBuffer.getWidth()), int(m_drawBuffer.getHeight())));
        softMaskRenderer.clearColor(softMaskDefinition.getBackdropColor());
        softMaskRenderer.setGraphicsState(graphicState);
        softMaskRenderer.processForm(softMaskDefinition.getFormStream());
        const PDFFloatBitmap& renderedSoftMask = softMaskRenderer.endPaint();
        PDFFloatBitmap createdSoftMask;

        switch (softMaskDefinition.getType())
        {
            case pdf::PDFPageContentProcessor::PDFSoftMaskDefinition::Type::Alpha:
                createdSoftMask = renderedSoftMask.extractOpacityChannel();
                break;

            case pdf::PDFPageContentProcessor::PDFSoftMaskDefinition::Type::Luminosity:
                createdSoftMask = renderedSoftMask.extractLuminosityChannel();
                break;

            default:
            case pdf::PDFPageContentProcessor::PDFSoftMaskDefinition::Type::Invalid:
                reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Invalid soft mask type."));
                createdSoftMask = renderedSoftMask.extractOpacityChannel();
                break;
        }

        if (const PDFFunction* function = softMaskDefinition.getTransferFunction())
        {
            const size_t width = createdSoftMask.getWidth();
            const size_t height = createdSoftMask.getHeight();

            for (size_t y = 0; y < height; ++y)
            {
                for (size_t x = 0; x < width; ++x)
                {
                    PDFColorBuffer pixel = createdSoftMask.getPixel(x, y);
                    PDFReal sourceValue = pixel[0];
                    PDFReal targetValue = sourceValue;

                    PDFFunction::FunctionResult result = function->apply(&sourceValue, &sourceValue + 1, &targetValue, &targetValue + 1);

                    if (!result)
                    {
                        reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Evaulation of soft mask transfer function failed."));
                    }

                    pixel[0] = targetValue;
                }
            }
        }

        getPainterState()->softMask = PDFTransparencySoftMask(false, qMove(createdSoftMask));
    }
}

void PDFTransparencyRenderer::createOpaqueBitmap(PDFFloatBitmap& bitmap)
{
    bitmap.makeOpaque();
}

void PDFTransparencyRenderer::createPaperBitmap(PDFFloatBitmap& bitmap, const PDFRGB& paperColor)
{
    bitmap.makeOpaque();
    bitmap.fillChannel(0, paperColor[0]);
    bitmap.fillChannel(1, paperColor[1]);
    bitmap.fillChannel(2, paperColor[2]);
}

void PDFTransparencyRenderer::performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule)
{
    Q_UNUSED(text);
    Q_UNUSED(fillRule);

    QTransform worldMatrix = getCurrentWorldMatrix();

    const PDFReal shapeStroking = getShapeStroking();
    const PDFReal opacityStroking = getOpacityStroking();
    const PDFReal shapeFilling = getShapeFilling();
    const PDFReal opacityFilling = getOpacityFilling();

    PDFPixelFormat format = m_drawBuffer.getPixelFormat();
    Q_ASSERT(format.hasShapeChannel());
    Q_ASSERT(format.hasOpacityChannel());

    const uint8_t shapeChannel = format.getShapeChannelIndex();
    const uint8_t opacityChannel = format.getOpacityChannelIndex();
    const uint8_t colorChannelStart = format.getColorChannelIndexStart();
    const uint8_t colorChannelEnd = format.getColorChannelIndexEnd();

    if (fill)
    {
        QPainterPath worldPath = worldMatrix.map(path);
        QRect fillRect = getActualFillRect(worldPath.controlPointRect());

        // Fill rect may be, or may not be valid. It depends on the painter path
        // and world matrix. Path can be translated outside of the paint area.
        if (fillRect.isValid())
        {
            PDFPainterPathSampler clipSampler(m_painterStateStack.top().clipPath, m_settings.samplesCount, 1.0f, fillRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));
            PDFPainterPathSampler pathSampler(worldPath, m_settings.samplesCount, 0.0f, fillRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));
            const PDFMappedColor& fillColor = getMappedFillColor();

            if (isMultithreadedPathSamplingUsed(fillRect))
            {
                if (fillRect.width() > fillRect.height())
                {
                    // Columns
                    PDFIntegerRange<int> range(fillRect.left(), fillRect.right() + 1);
                    auto processEntry = [&, this](int x)
                    {
                        for (int y = fillRect.top(); y <= fillRect.bottom(); ++y)
                        {
                            performPixelSampling(shapeFilling, opacityFilling, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, fillColor, clipSampler, pathSampler);
                        }
                    };
                    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
                }
                else
                {
                    // Rows
                    PDFIntegerRange<int> range(fillRect.top(), fillRect.bottom() + 1);
                    auto processEntry = [&, this](int y)
                    {
                        for (int x = fillRect.left(); x <= fillRect.right(); ++x)
                        {
                            performPixelSampling(shapeFilling, opacityFilling, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, fillColor, clipSampler, pathSampler);
                        }
                    };
                    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
                }
            }
            else
            {
                for (int x = fillRect.left(); x <= fillRect.right(); ++x)
                {
                    for (int y = fillRect.top(); y <= fillRect.bottom(); ++y)
                    {
                        performPixelSampling(shapeFilling, opacityFilling, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, fillColor, clipSampler, pathSampler);
                    }
                }
            }

            m_drawBuffer.modify(fillRect, true, false);
        }
    }

    if (stroke)
    {
        // We must stroke the path.
        QPainterPathStroker stroker;
        stroker.setCapStyle(getGraphicState()->getLineCapStyle());
        stroker.setWidth(getGraphicState()->getLineWidth());
        stroker.setMiterLimit(getGraphicState()->getMitterLimit());
        stroker.setJoinStyle(getGraphicState()->getLineJoinStyle());

        const PDFLineDashPattern& lineDashPattern = getGraphicState()->getLineDashPattern();
        if (!lineDashPattern.isSolid())
        {
            stroker.setDashPattern(lineDashPattern.createForQPen(getGraphicState()->getLineWidth()));
            stroker.setDashOffset(lineDashPattern.getDashOffset());
        }
        QPainterPath strokedPath = stroker.createStroke(path);

        QPainterPath worldPath = worldMatrix.map(strokedPath);
        QRect strokeRect = getActualFillRect(worldPath.controlPointRect());

        // Fill rect may be, or may not be valid. It depends on the painter path
        // and world matrix. Path can be translated outside of the paint area.
        if (strokeRect.isValid())
        {
            PDFPainterPathSampler clipSampler(m_painterStateStack.top().clipPath, m_settings.samplesCount, 1.0f, strokeRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));
            PDFPainterPathSampler pathSampler(worldPath, m_settings.samplesCount, 0.0f, strokeRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));
            const PDFMappedColor& strokeColor = getMappedStrokeColor();

            if (isMultithreadedPathSamplingUsed(strokeRect))
            {
                if (strokeRect.width() > strokeRect.height())
                {
                    // Columns
                    PDFIntegerRange<int> range(strokeRect.left(), strokeRect.right() + 1);
                    auto processEntry = [&, this](int x)
                    {
                        for (int y = strokeRect.top(); y <= strokeRect.bottom(); ++y)
                        {
                            performPixelSampling(shapeStroking, opacityStroking, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, strokeColor, clipSampler, pathSampler);
                        }
                    };
                    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
                }
                else
                {
                    // Rows
                    PDFIntegerRange<int> range(strokeRect.top(), strokeRect.bottom() + 1);
                    auto processEntry = [&, this](int y)
                    {
                        for (int x = strokeRect.left(); x <= strokeRect.right(); ++x)
                        {
                            performPixelSampling(shapeStroking, opacityStroking, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, strokeColor, clipSampler, pathSampler);
                        }
                    };
                    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
                }
            }
            else
            {
                for (int x = strokeRect.left(); x <= strokeRect.right(); ++x)
                {
                    for (int y = strokeRect.top(); y <= strokeRect.bottom(); ++y)
                    {
                        performPixelSampling(shapeStroking, opacityStroking, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, strokeColor, clipSampler, pathSampler);
                    }
                }
            }

            m_drawBuffer.modify(strokeRect, false, true);
        }
    }

    flushDrawBuffer();
}

bool PDFTransparencyRenderer::performPathPaintingUsingShading(const QPainterPath& path, bool stroke, bool fill, const PDFShadingPattern* shadingPattern)
{
    if (path.isEmpty())
    {
        // Path is empty
        return true;
    }

    // Exactly one of stroke/fill must be true and other must be false
    Q_ASSERT(stroke != fill);

    QTransform worldMatrix = getCurrentWorldMatrix();
    QPainterPath worldPath = worldMatrix.map(path);
    QRect fillRect = getActualFillRect(worldPath.controlPointRect());

    if (fillRect.isEmpty())
    {
        // Jakub Melka: nothing to draw, rectangle is empty
        return true;
    }

    std::unique_ptr<PDFShadingSampler> sampler(shadingPattern->createSampler(getPatternBaseMatrix()));
    if (!sampler)
    {
        // Can't create sampler - this is error
        reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Cannot create shading sampler."));
        return true;
    }

    // Now, we have a sampler, so we create a texture, which we will later use
    // as color source.
    const PDFAbstractColorSpace* colorSpace = shadingPattern->getColorSpace();
    const size_t shadingColorComponentCount = colorSpace->getColorComponentCount();
    PDFFloatBitmapWithColorSpace texture(fillRect.width() + 1, fillRect.height() + 1, PDFPixelFormat::createFormat(uint8_t(shadingColorComponentCount), 0, true, shadingColorComponentCount == 4, false), shadingPattern->getColorSpacePtr());
    QPointF offset = fillRect.topLeft();

    PDFPixelFormat texturePixelFormat = texture.getPixelFormat();
    uint8_t textureShapeChannel = texturePixelFormat.getShapeChannelIndex();
    uint8_t textureOpacityChannel = texturePixelFormat.getOpacityChannelIndex();

    if (fillRect.width() > fillRect.height())
    {
        // Columns
        PDFIntegerRange<int> range(fillRect.left(), fillRect.right() + 1);
        auto processEntry = [&, this](int x)
        {
            for (int y = fillRect.top(); y <= fillRect.bottom(); ++y)
            {
                const int texelCoordinateX = x - fillRect.left();
                const int texelCoordinateY = y - fillRect.top();

                PDFColorBuffer buffer = texture.getPixel(texelCoordinateX, texelCoordinateY);
                bool isSampled = sampler->sample(QPointF(x, y) + offset, buffer.resized(shadingColorComponentCount), m_settings.shadingAlgorithmLimit);
                const PDFColorComponent textureSampleShape = isSampled ? 1.0f : 0.0f;
                buffer[textureShapeChannel] = textureSampleShape;
                buffer[textureOpacityChannel] = textureSampleShape;
            }
        };
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
    }
    else
    {
        // Rows
        PDFIntegerRange<int> range(fillRect.top(), fillRect.bottom() + 1);
        auto processEntry = [&, this](int y)
        {
            for (int x = fillRect.left(); x <= fillRect.right(); ++x)
            {
                const int texelCoordinateX = x - fillRect.left();
                const int texelCoordinateY = y - fillRect.top();

                PDFColorBuffer buffer = texture.getPixel(texelCoordinateX, texelCoordinateY);
                bool isSampled = sampler->sample(QPointF(x, y) + offset, buffer.resized(shadingColorComponentCount), m_settings.shadingAlgorithmLimit);
                const PDFColorComponent textureSampleShape = isSampled ? 1.0f : 0.0f;
                buffer[textureShapeChannel] = textureSampleShape;
                buffer[textureOpacityChannel] = textureSampleShape;
            }
        };
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
    }

    // Convert image to a blend color space
    texture = convertImageToBlendSpace(texture);
    texturePixelFormat = texture.getPixelFormat();
    textureShapeChannel = texturePixelFormat.getShapeChannelIndex();
    textureOpacityChannel = texturePixelFormat.getOpacityChannelIndex();

    PDFPainterPathSampler clipSampler(m_painterStateStack.top().clipPath, m_settings.samplesCount, 1.0f, fillRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));
    PDFPainterPathSampler pathSampler(worldPath, m_settings.samplesCount, 0.0f, fillRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));

    const PDFReal constantShape = stroke ? getShapeStroking() : getShapeFilling();
    const PDFReal constantOpacity = stroke ? getOpacityStroking() : getOpacityFilling();

    Q_ASSERT(m_drawBuffer.getPixelFormat() == texture.getPixelFormat());

    const PDFPixelFormat drawBufferPixelFormat = m_drawBuffer.getPixelFormat();
    const uint8_t drawBufferShapeChannel = drawBufferPixelFormat.getShapeChannelIndex();
    const uint8_t drawBufferOpacityChannel = drawBufferPixelFormat.getOpacityChannelIndex();
    const uint32_t colorChannelStart = drawBufferPixelFormat.getColorChannelIndexStart();
    const uint32_t colorChannelEnd = drawBufferPixelFormat.getColorChannelIndexEnd();

    for (int x = fillRect.left(); x <= fillRect.right(); ++x)
    {
        for (int y = fillRect.top(); y <= fillRect.bottom(); ++y)
        {
            const int texelCoordinateX = x - fillRect.left();
            const int texelCoordinateY = y - fillRect.top();
            PDFColorBuffer texel = texture.getPixel(texelCoordinateX, texelCoordinateY);

            const PDFColorComponent textureShape = texel[drawBufferShapeChannel];
            const PDFColorComponent textureOpacity = texel[drawBufferOpacityChannel];
            const PDFColorComponent clipValue = clipSampler.sample(QPoint(x, y));
            const PDFColorComponent objectShapeValue = pathSampler.sample(QPoint(x, y));
            const PDFColorComponent shapeValue = objectShapeValue * clipValue * constantShape * textureShape;
            const PDFColorComponent opacityValue = shapeValue * constantOpacity * textureOpacity;

            if (shapeValue > 0.0f)
            {
                // We consider old object shape - we use Union function to
                // set shape channel value.

                PDFColorBuffer pixel = m_drawBuffer.getPixel(x, y);
                pixel[drawBufferShapeChannel] = PDFBlendFunction::blend_Union(shapeValue, pixel[drawBufferShapeChannel]);
                pixel[drawBufferOpacityChannel] = opacityValue;

                // Copy color
                for (uint8_t colorChannelIndex = colorChannelStart; colorChannelIndex < colorChannelEnd; ++colorChannelIndex)
                {
                    pixel[colorChannelIndex] = texel[colorChannelIndex];
                }

                m_drawBuffer.markPixelActiveColorMask(x, y, texture.getPixelActiveColorMask(texelCoordinateX, texelCoordinateY));
            }
        }
    }

    m_drawBuffer.modify(fillRect, fill, stroke);
    return true;
}

void PDFTransparencyRenderer::performFinishPathPainting()
{
    flushDrawBuffer();
}

void PDFTransparencyRenderer::performClipping(const QPainterPath& path, Qt::FillRule fillRule)
{
    Q_UNUSED(fillRule);

    PDFTransparencyPainterState* painterState = getPainterState();

    if (!painterState->clipPath.isEmpty())
    {
        painterState->clipPath = painterState->clipPath.intersected(getCurrentWorldMatrix().map(path));
    }
    else
    {
        painterState->clipPath = getCurrentWorldMatrix().map(path);
    }
}

void PDFTransparencyRenderer::performUpdateGraphicsState(const PDFPageContentProcessorState& state)
{
    PDFPageContentProcessorState::StateFlags stateFlags = state.getStateFlags();

    const bool colorTransformAffected = stateFlags.testFlag(PDFPageContentProcessorState::StateRenderingIntent) ||
                                        stateFlags.testFlag(PDFPageContentProcessorState::StateBlackPointCompensation);

    if (colorTransformAffected ||
        stateFlags.testFlag(PDFPageContentProcessorState::StateStrokeColor) ||
        stateFlags.testFlag(PDFPageContentProcessorState::StateStrokeColorSpace))
    {
        m_mappedStrokeColor.dirty();
    }

    if (colorTransformAffected ||
        stateFlags.testFlag(PDFPageContentProcessorState::StateFillColor) ||
        stateFlags.testFlag(PDFPageContentProcessorState::StateFillColorSpace))
    {
        m_mappedFillColor.dirty();
    }

    BaseClass::performUpdateGraphicsState(state);

    if (stateFlags.testFlag(PDFPageContentProcessorState::StateSoftMask))
    {
        processSoftMask(state.getSoftMask());
    }
}

void PDFTransparencyRenderer::performSaveGraphicState(ProcessOrder order)
{
    if (order == ProcessOrder::AfterOperation)
    {
        m_painterStateStack.push(m_painterStateStack.top());
    }
}

void PDFTransparencyRenderer::performRestoreGraphicState(ProcessOrder order)
{
    if (order == ProcessOrder::BeforeOperation)
    {
        m_painterStateStack.pop();
    }
    if (order == ProcessOrder::AfterOperation)
    {
        invalidateCachedItems();
    }
}

void PDFTransparencyRenderer::performBeginTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup)
{
    if (order == ProcessOrder::BeforeOperation)
    {
        PDFTransparencyGroupPainterData data;
        data.group = transparencyGroup;
        data.alphaIsShape = getGraphicState()->getAlphaIsShape();
        data.alphaFill = getGraphicState()->getAlphaFilling();
        data.alphaStroke = getGraphicState()->getAlphaStroking();
        data.blendMode = getGraphicState()->getBlendMode();
        data.blackPointCompensationMode = getGraphicState()->getBlackPointCompensationMode();
        data.renderingIntent = getGraphicState()->getRenderingIntent();
        data.blendColorSpace = transparencyGroup.colorSpacePointer;

        if (!data.blendColorSpace)
        {
            data.blendColorSpace = getBlendColorSpace();
        }

        // Create initial backdrop, according to 11.4.8 of PDF 2.0 specification.
        // If group is knockout, use initial backdrop.
        data.initialBackdrop = *getBackdrop();

        if (isTransparencyGroupIsolated())
        {
            // Make initial backdrop transparent
            data.makeInitialBackdropTransparent();
        }
        else if (!isTransparencyGroupKnockout())
        {
            // We have stored alpha_g_i in immediate buffer. We must mix it with alpha_0 to get alpha_i
            const PDFFloatBitmapWithColorSpace* initialBackdrop = getInitialBackdrop();
            const uint8_t opacityChannelIndex = initialBackdrop->getPixelFormat().getOpacityChannelIndex();
            const size_t width = data.initialBackdrop.getWidth();
            const size_t height = data.initialBackdrop.getHeight();

            for (size_t x = 0; x < width; ++x)
            {
                for (size_t y = 0; y < height; ++y)
                {
                    PDFConstColorBuffer oldPixel = initialBackdrop->getPixel(x, y);
                    PDFColorBuffer newPixel = data.initialBackdrop.getPixel(x, y);
                    newPixel[opacityChannelIndex] = PDFBlendFunction::blend_Union(oldPixel[opacityChannelIndex], newPixel[opacityChannelIndex]);
                }
            }
        }

        // Prepare soft mask
        data.softMask = getPainterState()->softMask;

        data.initialBackdrop.convertToColorSpace(getCMS(), data.renderingIntent, data.blendColorSpace, this);
        data.immediateBackdrop = data.initialBackdrop;

        // Jakub Melka: According to 11.4.8 of PDF 2.0 specification, we must
        // initialize f_g_0 and alpha_g_0 to zero. We store f_g_0 and alpha_g_0
        // in the immediate backdrop, so we will make it transparent.
        data.makeImmediateBackdropTransparent();

        // Create draw buffer
        m_drawBuffer = PDFDrawBuffer(data.immediateBackdrop.getWidth(), data.immediateBackdrop.getHeight(), data.immediateBackdrop.getPixelFormat());

        m_transparencyGroupDataStack.emplace_back(qMove(data));
        invalidateCachedItems();
    }
}

void PDFTransparencyRenderer::performEndTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup)
{
    Q_UNUSED(transparencyGroup);

    if (order == ProcessOrder::AfterOperation)
    {
        // "Unblend" the initial backdrop from immediate backdrop, according to 11.4.8
        removeInitialBackdrop();

        PDFTransparencyGroupPainterData sourceData = qMove(m_transparencyGroupDataStack.back());
        m_transparencyGroupDataStack.pop_back();

        // Filter inactive colors - clear all colors in immediate mask,
        // which are set to inactive.
        if (sourceData.filterColorsUsingMask)
        {
            const PDFPixelFormat pixelFormat = sourceData.immediateBackdrop.getPixelFormat();
            const uint32_t colorChannelStart = pixelFormat.getColorChannelIndexStart();
            const uint32_t colorChannelEnd = pixelFormat.getColorChannelIndexEnd();
            const uint32_t processColorChannelEnd = pixelFormat.getProcessColorChannelIndexEnd();

            for (uint32_t colorChannelIndex = colorChannelStart; colorChannelIndex < colorChannelEnd; ++colorChannelIndex)
            {
                const uint32_t flag = 1 << colorChannelIndex;
                if (!(sourceData.activeColorMask & flag))
                {
                    const bool isProcessColor = colorChannelIndex < processColorChannelEnd;
                    const bool isSubtractive = isProcessColor ? pixelFormat.hasProcessColorsSubtractive() : pixelFormat.hasSpotColorsSubtractive();

                    sourceData.immediateBackdrop.fillChannel(colorChannelIndex, isSubtractive ? 0.0f : 1.0f);
                }
            }
        }

        if (sourceData.saveOriginalImage)
        {
            m_originalProcessBitmap = sourceData.immediateBackdrop;
        }

        // Collapse spot colors
        if (sourceData.transformSpotsToDevice)
        {
            collapseSpotColorsToDeviceColors(sourceData.immediateBackdrop);
        }

        PDFTransparencyGroupPainterData& targetData = m_transparencyGroupDataStack.back();
        sourceData.immediateBackdrop.convertToColorSpace(getCMS(), targetData.renderingIntent, targetData.blendColorSpace, this);

        PDFOverprintMode overprintMode = getGraphicState()->getOverprintMode();
        const bool useOverprint = overprintMode.overprintFilling || overprintMode.overprintStroking;

        PDFFloatBitmap::OverprintMode selectedOverprintMode = PDFFloatBitmap::OverprintMode::NoOveprint;
        if (useOverprint)
        {
            selectedOverprintMode = overprintMode.overprintMode == 0 ? PDFFloatBitmap::OverprintMode::Overprint_Mode_0
                                                                     : PDFFloatBitmap::OverprintMode::Overprint_Mode_1;
        }

        PDFFloatBitmap::blend(sourceData.immediateBackdrop, targetData.immediateBackdrop, *getBackdrop(), *getInitialBackdrop(), *sourceData.softMask.getSoftMask(),
                              sourceData.alphaIsShape, sourceData.alphaFill, sourceData.blendMode, sourceData.group.knockout, selectedOverprintMode, getPaintRect());

        // Create draw buffer
        PDFFloatBitmapWithColorSpace* backdrop = getImmediateBackdrop();
        m_drawBuffer = PDFDrawBuffer(backdrop->getWidth(), backdrop->getHeight(), backdrop->getPixelFormat());

        invalidateCachedItems();
    }
}

void PDFTransparencyRenderer::performTextBegin(ProcessOrder order)
{
    if (order == ProcessOrder::AfterOperation && getGraphicState()->getTextKnockout())
    {
        // In a case of text knockout, use text transparency group of type knockout
        PDFTransparencyGroup transparencyGroup;
        transparencyGroup.knockout = true;
        m_textTransparencyGroupGuard.reset(new PDFTransparencyGroupGuard(this, qMove(transparencyGroup)));
    }
}

void PDFTransparencyRenderer::performTextEnd(ProcessOrder order)
{
    if (order == ProcessOrder::BeforeOperation)
    {
        m_textTransparencyGroupGuard.reset();
    }
}

bool PDFTransparencyRenderer::performOriginalImagePainting(const PDFImage& image)
{
    PDFFloatBitmap texture = getImage(image);

    if (m_settings.flags.testFlag(PDFTransparencyRendererSettings::SmoothImageTransformation) && image.isInterpolated())
    {
        // Test, if we can use smooth images. We can use them under following conditions:
        //  1) Transformed rectangle is not skewed or deformed (so vectors (0, 1) and (1, 0) are orthogonal)
        //  2) We are shrinking the image
        //  3) Aspect ratio of the image is the same

        QTransform matrix = getCurrentWorldMatrix();
        QLineF mappedWidthVector = matrix.map(QLineF(0, 0, texture.getWidth(), 0));
        QLineF mappedHeightVector = matrix.map(QLineF(0, 0, 0, texture.getHeight()));
        qreal angle = mappedWidthVector.angleTo(mappedHeightVector);
        if (qFuzzyCompare(angle, 90.0))
        {
            // Image is not skewed, so we if we are shrinking the image
            const qreal originalWidth = texture.getWidth();
            const qreal originalHeight = texture.getHeight();
            const qreal originalRatio = originalWidth / originalHeight;
            const qreal transformedWidth = mappedWidthVector.length();
            const qreal transformedHeight = mappedHeightVector.length();
            const qreal transformedRatio = transformedWidth / transformedHeight;

            if (qFuzzyCompare(originalRatio, transformedRatio) && originalWidth > transformedWidth && originalHeight > transformedHeight)
            {
                uint32_t activeColorMask = texture.getPixelActiveColorMask(0, 0);
                texture = texture.resize(qCeil(transformedWidth), qCeil(transformedHeight), Qt::SmoothTransformation);
                texture.setColorActivity(activeColorMask);
            }
        }
    }

    QTransform imageTransform(1.0 / qreal(texture.getWidth()), 0, 0, 1.0 / qreal(texture.getHeight()), 0, 0);
    QTransform worldMatrix = imageTransform * getCurrentWorldMatrix();

    // Because Qt uses opposite axis direction than PDF, then we must transform the y-axis
    // to the opposite (so the image is then unchanged)
    worldMatrix.translate(0.0, texture.getHeight());
    worldMatrix.scale(1, -1);

    QPolygonF imagePolygon;
    imagePolygon << QPointF(0.0, 0.0);
    imagePolygon << QPointF(0.0, texture.getHeight());
    imagePolygon << QPointF(texture.getWidth(), texture.getHeight());
    imagePolygon << QPointF(texture.getWidth(), 0.0);

    QTransform worldToTextureMatrix = worldMatrix.inverted();
    QRectF boundingRectangle = worldMatrix.map(imagePolygon).boundingRect();
    QRect fillRect = getActualFillRect(boundingRectangle);

    const PDFReal shape = getShapeFilling();
    const PDFReal opacity = getOpacityFilling();

    PDFPixelFormat format = m_drawBuffer.getPixelFormat();
    Q_ASSERT(format.hasShapeChannel());
    Q_ASSERT(format.hasOpacityChannel());
    Q_ASSERT(format == texture.getPixelFormat());

    const uint8_t shapeChannel = format.getShapeChannelIndex();
    const uint8_t opacityChannel = format.getOpacityChannelIndex();
    const uint8_t colorChannelStart = format.getColorChannelIndexStart();
    const uint8_t colorChannelEnd = format.getColorChannelIndexEnd();

    // Fill rect may be, or may not be valid. It depends on the painter path
    // and world matrix. Path can be translated outside of the paint area.
    if (fillRect.isValid())
    {
        PDFPainterPathSampler clipSampler(m_painterStateStack.top().clipPath, m_settings.samplesCount, 1.0f, fillRect, m_settings.flags.testFlag(PDFTransparencyRendererSettings::PrecisePathSampler));

        if (isMultithreadedPathSamplingUsed(fillRect))
        {
            if (fillRect.width() > fillRect.height())
            {
                // Columns
                PDFIntegerRange<int> range(fillRect.left(), fillRect.right() + 1);
                auto processEntry = [&, this](int x)
                {
                    for (int y = fillRect.top(); y <= fillRect.bottom(); ++y)
                    {
                        performFillFragmentFromTexture(shape, opacity, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, worldToTextureMatrix, texture, clipSampler);
                    }
                };
                PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
            }
            else
            {
                // Rows
                PDFIntegerRange<int> range(fillRect.top(), fillRect.bottom() + 1);
                auto processEntry = [&, this](int y)
                {
                    for (int x = fillRect.left(); x <= fillRect.right(); ++x)
                    {
                        performFillFragmentFromTexture(shape, opacity, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, worldToTextureMatrix, texture, clipSampler);
                    }
                };
                PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), processEntry);
            }
        }
        else
        {
            for (int x = fillRect.left(); x <= fillRect.right(); ++x)
            {
                for (int y = fillRect.top(); y <= fillRect.bottom(); ++y)
                {
                    performFillFragmentFromTexture(shape, opacity, shapeChannel, opacityChannel, colorChannelStart, colorChannelEnd, x, y, worldToTextureMatrix, texture, clipSampler);
                }
            }
        }

        m_drawBuffer.modify(fillRect, true, false);
        flushDrawBuffer();
    }

    return true;
}

void PDFTransparencyRenderer::performImagePainting(const QImage& image)
{
    Q_UNUSED(image);

    reportRenderErrorOnce(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image painting not implemented."));
}

void PDFTransparencyRenderer::performMeshPainting(const PDFMesh& mesh)
{
    Q_UNUSED(mesh);

    reportRenderErrorOnce(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Mesh painting not implemented."));
}

PDFReal PDFTransparencyRenderer::getShapeStroking() const
{
    return getGraphicState()->getAlphaIsShape() ? getGraphicState()->getAlphaStroking() : 1.0;
}

PDFReal PDFTransparencyRenderer::getOpacityStroking() const
{
    return !getGraphicState()->getAlphaIsShape() ? getGraphicState()->getAlphaStroking() : 1.0;
}

PDFReal PDFTransparencyRenderer::getShapeFilling() const
{
    return getGraphicState()->getAlphaIsShape() ? getGraphicState()->getAlphaFilling() : 1.0;
}

PDFReal PDFTransparencyRenderer::getOpacityFilling() const
{
    return !getGraphicState()->getAlphaIsShape() ? getGraphicState()->getAlphaFilling() : 1.0;
}

void PDFTransparencyRenderer::invalidateCachedItems()
{
    m_mappedStrokeColor.dirty();
    m_mappedFillColor.dirty();
}

void PDFTransparencyRenderer::removeInitialBackdrop()
{
    PDFFloatBitmapWithColorSpace* immediateBackdrop = getImmediateBackdrop();
    PDFFloatBitmapWithColorSpace* initialBackdrop = getInitialBackdrop();
    PDFPixelFormat pixelFormat = immediateBackdrop->getPixelFormat();

    const uint8_t alphaChannelIndex = pixelFormat.getOpacityChannelIndex();
    const uint8_t colorChannelIndexStart = pixelFormat.getColorChannelIndexStart();
    const uint8_t colorChannelIndexEnd= pixelFormat.getColorChannelIndexEnd();

    Q_ASSERT(alphaChannelIndex != PDFPixelFormat::INVALID_CHANNEL_INDEX);
    Q_ASSERT(colorChannelIndexStart != PDFPixelFormat::INVALID_CHANNEL_INDEX);
    Q_ASSERT(colorChannelIndexEnd != PDFPixelFormat::INVALID_CHANNEL_INDEX);

    for (size_t x = 0; x < immediateBackdrop->getWidth(); ++x)
    {
        for (size_t y = 0; y < immediateBackdrop->getHeight(); ++y)
        {
            PDFColorBuffer initialBackdropColorBuffer = initialBackdrop->getPixel(x, y);
            PDFColorBuffer immediateBackdropColorBuffer = immediateBackdrop->getPixel(x, y);

            const PDFColorComponent alpha_0 = initialBackdropColorBuffer[alphaChannelIndex];
            const PDFColorComponent alpha_g_n = immediateBackdropColorBuffer[alphaChannelIndex];

            if (!qFuzzyIsNull(alpha_g_n))
            {
                for (uint8_t i = colorChannelIndexStart; i < colorChannelIndexEnd; ++i)
                {
                    const PDFColorComponent C_0 = initialBackdropColorBuffer[i];
                    const PDFColorComponent C_n = immediateBackdropColorBuffer[i];
                    const PDFColorComponent C = C_n + (C_n - C_0) * alpha_0 * (1.0f / alpha_g_n - 1.0f);
                    const PDFColorComponent C_clipped = qBound(0.0f, C, 1.0f);
                    immediateBackdropColorBuffer[i] = C_clipped;
                }
            }
        }
    }
}

PDFFloatBitmapWithColorSpace* PDFTransparencyRenderer::getInitialBackdrop()
{
    return &m_transparencyGroupDataStack.back().initialBackdrop;
}

PDFFloatBitmapWithColorSpace* PDFTransparencyRenderer::getImmediateBackdrop()
{
    return &m_transparencyGroupDataStack.back().immediateBackdrop;
}

PDFFloatBitmapWithColorSpace* PDFTransparencyRenderer::getBackdrop()
{
    if (isTransparencyGroupKnockout())
    {
        return getInitialBackdrop();
    }
    else
    {
        return getImmediateBackdrop();
    }
}

const PDFFloatBitmapWithColorSpace* PDFTransparencyRenderer::getInitialBackdrop() const
{
    return &m_transparencyGroupDataStack.back().initialBackdrop;
}

const PDFFloatBitmapWithColorSpace* PDFTransparencyRenderer::getImmediateBackdrop() const
{
    return &m_transparencyGroupDataStack.back().immediateBackdrop;
}

const PDFFloatBitmapWithColorSpace* PDFTransparencyRenderer::getBackdrop() const
{
    if (isTransparencyGroupKnockout())
    {
        return getInitialBackdrop();
    }
    else
    {
        return getImmediateBackdrop();
    }
}

const PDFColorSpacePointer& PDFTransparencyRenderer::getBlendColorSpace() const
{
    return m_transparencyGroupDataStack.back().blendColorSpace;
}

bool PDFTransparencyRenderer::isTransparencyGroupIsolated() const
{
    return m_transparencyGroupDataStack.back().group.isolated;
}

bool PDFTransparencyRenderer::isTransparencyGroupKnockout() const
{
    return m_transparencyGroupDataStack.back().group.knockout;
}

const PDFTransparencyRenderer::PDFMappedColor& PDFTransparencyRenderer::getMappedStrokeColor()
{
    return m_mappedStrokeColor.get(this, &PDFTransparencyRenderer::getMappedStrokeColorImpl);
}

const PDFTransparencyRenderer::PDFMappedColor& PDFTransparencyRenderer::getMappedFillColor()
{
    return m_mappedFillColor.get(this, &PDFTransparencyRenderer::getMappedFillColorImpl);
}

void PDFTransparencyRenderer::fillMappedColorUsingMapping(const PDFPixelFormat pixelFormat,
                                                          PDFMappedColor& result,
                                                          const PDFInkMapping& inkMapping,
                                                          const PDFColor& sourceColor)
{
    result.mappedColor.resize(pixelFormat.getColorChannelCount());

    // Zero the color
    for (size_t i = 0; i < pixelFormat.getColorChannelCount(); ++i)
    {
        result.mappedColor[i] = 0.0f;
    }

    for (const PDFInkMapping::Mapping& ink : inkMapping.mapping)
    {
        // Sanity check of source color
        if (ink.source >= sourceColor.size())
        {
            reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Invalid source ink index %1.").arg(ink.source));
            continue;
        }

        // Sanity check of target color
        if (ink.target >= result.mappedColor.size())
        {
            reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Invalid target ink index %1.").arg(ink.target));
            continue;
        }

        switch (ink.type)
        {
            case pdf::PDFInkMapping::Pass:
                result.mappedColor[ink.target] = sourceColor[ink.source];
                break;

            default:
                Q_ASSERT(false);
                break;
        }
    }

    result.activeChannels = inkMapping.activeChannels;
}

PDFTransparencyRenderer::PDFMappedColor PDFTransparencyRenderer::createMappedColor(const PDFColor& sourceColor, const PDFAbstractColorSpace* sourceColorSpace)
{
    PDFMappedColor result;

    const PDFAbstractColorSpace* targetColorSpace = getBlendColorSpace().data();
    const PDFPixelFormat pixelFormat = getImmediateBackdrop()->getPixelFormat();

    Q_ASSERT(targetColorSpace->equals(getImmediateBackdrop()->getColorSpace().data()));

    PDFInkMapping inkMapping = m_inkMapper->createMapping(sourceColorSpace, targetColorSpace, pixelFormat);
    if (inkMapping.isValid())
    {
        fillMappedColorUsingMapping(pixelFormat, result, inkMapping, sourceColor);
    }
    else
    {
        // Jakub Melka: We must convert color form source color space to target color space
        std::vector<PDFColorComponent> sourceColorVector(sourceColor.size(), 0.0f);
        for (size_t i = 0; i < sourceColorVector.size(); ++i)
        {
            sourceColorVector[i] = sourceColor[i];
        }
        PDFColorBuffer sourceColorBuffer(sourceColorVector.data(), sourceColorVector.size());

        std::vector<PDFColorComponent> targetColorVector(pixelFormat.getProcessColorChannelCount(), 0.0f);
        PDFColorBuffer targetColorBuffer(targetColorVector.data(), targetColorVector.size());

        if (!PDFAbstractColorSpace::transform(sourceColorSpace, targetColorSpace, getCMS(), getGraphicState()->getRenderingIntent(), sourceColorBuffer, targetColorBuffer, this))
        {
            reportRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Transformation from source color space to target blending color space failed."));
        }

        PDFColor adjustedSourceColor;
        adjustedSourceColor.resize(targetColorBuffer.size());

        for (size_t i = 0; i < targetColorBuffer.size(); ++i)
        {
            adjustedSourceColor[i] = targetColorBuffer[i];
        }

        inkMapping = m_inkMapper->createMapping(targetColorSpace, targetColorSpace, pixelFormat);

        Q_ASSERT(inkMapping.isValid());
        fillMappedColorUsingMapping(pixelFormat, result, inkMapping, adjustedSourceColor);
    }

    return result;
}

PDFTransparencyRenderer::PDFMappedColor PDFTransparencyRenderer::getMappedStrokeColorImpl()
{
    const PDFAbstractColorSpace* sourceColorSpace = getGraphicState()->getStrokeColorSpace();
    const PDFColor& sourceColor = getGraphicState()->getStrokeColorOriginal();

    return createMappedColor(sourceColor, sourceColorSpace);
}

PDFTransparencyRenderer::PDFMappedColor PDFTransparencyRenderer::getMappedFillColorImpl()
{
    const PDFAbstractColorSpace* sourceColorSpace = getGraphicState()->getFillColorSpace();
    const PDFColor& sourceColor = getGraphicState()->getFillColorOriginal();

    return createMappedColor(sourceColor, sourceColorSpace);
}

QRect PDFTransparencyRenderer::getPaintRect() const
{
    return QRect(0, 0, int(getBackdrop()->getWidth()), int(getBackdrop()->getHeight()));
}

QRect PDFTransparencyRenderer::getActualFillRect(const QRectF& fillRect) const
{
    int xLeft = qFloor(fillRect.left()) - 1;
    int xRight = qCeil(fillRect.right()) + 1;
    int yTop = qFloor(fillRect.top()) - 1;
    int yBottom = qCeil(fillRect.bottom()) + 1;

    QRect drawRect(xLeft, yTop, xRight - xLeft, yBottom - yTop);
    return getPaintRect().intersected(drawRect);
}

void PDFTransparencyRenderer::flushDrawBuffer()
{
    if (m_drawBuffer.isModified())
    {
        PDFOverprintMode overprintMode = getGraphicState()->getOverprintMode();
        const bool useOverprint = (overprintMode.overprintFilling && m_drawBuffer.isContainsFilling()) ||
                                  (overprintMode.overprintStroking && m_drawBuffer.isContainsStroking());

        PDFFloatBitmap::OverprintMode selectedOverprintMode = PDFFloatBitmap::OverprintMode::NoOveprint;
        if (useOverprint)
        {
            selectedOverprintMode = overprintMode.overprintMode == 0 ? PDFFloatBitmap::OverprintMode::Overprint_Mode_0
                                                                     : PDFFloatBitmap::OverprintMode::Overprint_Mode_1;
        }

        PDFFloatBitmap::blend(m_drawBuffer, *getImmediateBackdrop(), *getBackdrop(), *getInitialBackdrop(), *getPainterState()->softMask.getSoftMask(),
                              getGraphicState()->getAlphaIsShape(), 1.0f, getGraphicState()->getBlendMode(), isTransparencyGroupKnockout(),
                              selectedOverprintMode, m_drawBuffer.getModifiedRect());


        m_drawBuffer.clear();
    }
}

bool PDFTransparencyRenderer::isMultithreadedPathSamplingUsed(QRect fillRect) const
{
    if (!m_settings.flags.testFlag(PDFTransparencyRendererSettings::MultithreadedPathSampler))
    {
        return false;
    }

    return fillRect.width() * fillRect.height() > m_settings.multithreadingPathSampleThreshold && fillRect.width() > 1;
}

PDFInkMapper::PDFInkMapper(const PDFCMSManager* manager, const PDFDocument* document) :
    m_cmsManager(manager),
    m_document(document)
{
    // Initialize device separations
    std::vector<ColorInfo> graySeparations = getSeparations(1, false);
    std::vector<ColorInfo> rgbSeparations = getSeparations(3, false);
    std::vector<ColorInfo> cmykSeparations = getSeparations(4, false);

    m_deviceColors.insert(m_deviceColors.end(), std::make_move_iterator(graySeparations.begin()), std::make_move_iterator(graySeparations.end()));
    m_deviceColors.insert(m_deviceColors.end(), std::make_move_iterator(rgbSeparations.begin()), std::make_move_iterator(rgbSeparations.end()));
    m_deviceColors.insert(m_deviceColors.end(), std::make_move_iterator(cmykSeparations.begin()), std::make_move_iterator(cmykSeparations.end()));
}

std::vector<PDFInkMapper::ColorInfo> PDFInkMapper::getSeparations(uint32_t processColorCount, bool withSpots) const
{
    std::vector<ColorInfo> result;
    result.reserve(getActiveSpotColorCount() + processColorCount);

    PDFRenderErrorReporterDummy renderErrorReporter;
    PDFCMSPointer cms = m_cmsManager ? m_cmsManager->getCurrentCMS() : nullptr;

    switch (processColorCount)
    {
        case 1:
        {
            PDFDeviceGrayColorSpace grayColorSpace;

            ColorInfo gray;
            gray.name = "Gray";
            gray.textName = PDFTranslationContext::tr("Gray");
            gray.canBeActive = true;
            gray.active = true;
            gray.isSpot = false;
            gray.spotColorIndex = 0;
            gray.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceGray;
            gray.color = cms ? grayColorSpace.getColor(PDFColor(1.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(gray));
            break;
        }

        case 3:
        {
            PDFDeviceRGBColorSpace rgbColorSpace;

            ColorInfo red;
            red.name = "Red";
            red.textName = PDFTranslationContext::tr("Red");
            red.canBeActive = true;
            red.active = true;
            red.isSpot = false;
            red.spotColorIndex = 0;
            red.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceRGB;
            red.color = cms ? rgbColorSpace.getColor(PDFColor(1.0f, 0.0f, 0.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(red));

            ColorInfo green;
            green.name = "Green";
            green.textName = PDFTranslationContext::tr("Green");
            green.canBeActive = true;
            green.active = true;
            green.isSpot = false;
            green.spotColorIndex = 1;
            green.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceRGB;
            green.color = cms ? rgbColorSpace.getColor(PDFColor(0.0f, 1.0f, 0.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(green));

            ColorInfo blue;
            blue.name = "Blue";
            blue.textName = PDFTranslationContext::tr("Blue");
            blue.canBeActive = true;
            blue.active = true;
            blue.isSpot = false;
            blue.spotColorIndex = 2;
            blue.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceRGB;
            blue.color = cms ? rgbColorSpace.getColor(PDFColor(0.0f, 0.0f, 1.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(blue));
            break;
        }

        case 4:
        {
            PDFDeviceCMYKColorSpace cmykColorSpace;

            ColorInfo cyan;
            cyan.name = "Cyan";
            cyan.textName = PDFTranslationContext::tr("Cyan");
            cyan.canBeActive = true;
            cyan.active = true;
            cyan.isSpot = false;
            cyan.spotColorIndex = 0;
            cyan.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceCMYK;
            cyan.color = cms ? cmykColorSpace.getColor(PDFColor(1.0f, 0.0f, 0.0f, 0.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(cyan));

            ColorInfo magenta;
            magenta.name = "Magenta";
            magenta.textName = PDFTranslationContext::tr("Magenta");
            magenta.canBeActive = true;
            magenta.active = true;
            magenta.isSpot = false;
            magenta.spotColorIndex = 1;
            magenta.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceCMYK;
            magenta.color = cms ? cmykColorSpace.getColor(PDFColor(0.0f, 1.0f, 0.0f, 0.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(magenta));

            ColorInfo yellow;
            yellow.name = "Yellow";
            yellow.textName = PDFTranslationContext::tr("Yellow");
            yellow.canBeActive = true;
            yellow.active = true;
            yellow.isSpot = false;
            yellow.spotColorIndex = 2;
            yellow.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceCMYK;
            yellow.color = cms ? cmykColorSpace.getColor(PDFColor(0.0f, 0.0f, 1.0f, 0.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(yellow));

            ColorInfo black;
            black.name = "Black";
            black.textName = PDFTranslationContext::tr("Black");
            black.canBeActive = true;
            black.active = true;
            black.isSpot = false;
            black.spotColorIndex = 3;
            black.colorSpaceType = PDFAbstractColorSpace::ColorSpace::DeviceCMYK;
            black.color = cms ? cmykColorSpace.getColor(PDFColor(0.0f, 0.0f, 0.0f, 1.0f), cms.get(), RenderingIntent::Perceptual, &renderErrorReporter, true) : QColor();
            result.emplace_back(qMove(black));
            break;
        }

        default:
        {
            for (uint32_t i = 0; i < processColorCount; ++i)
            {
                ColorInfo generic;
                generic.textName = PDFTranslationContext::tr("Process Generic%1").arg(i + 1);
                generic.name = generic.textName.toLatin1();
                generic.canBeActive = true;
                generic.active = true;
                generic.isSpot = false;
                generic.spotColorIndex = i;
                result.emplace_back(qMove(generic));
            }
        }
    }

    if (withSpots)
    {
        for (const auto& spotColor : m_spotColors)
        {
            if (!spotColor.active)
            {
                // Skip inactive spot colors
                continue;
            }

            result.emplace_back(spotColor);
        }
    }

    return result;
}

void PDFInkMapper::createSpotColors(bool activate)
{
    m_spotColors.clear();
    m_activeSpotColors = 0;

    PDFRenderErrorReporterDummy renderErrorReporter;
    PDFCMSPointer cms = m_cmsManager ? m_cmsManager->getCurrentCMS() : nullptr;

    const PDFCatalog* catalog = m_document->getCatalog();
    const size_t pageCount = catalog->getPageCount();
    for (size_t i = 0; i < pageCount; ++i)
    {
        const PDFPage* page = catalog->getPage(i);
        PDFObject resources = m_document->getObject(page->getResources());

        if (resources.isDictionary() && resources.getDictionary()->hasKey("ColorSpace"))
        {
            const PDFDictionary* colorSpaceDictionary = m_document->getDictionaryFromObject(resources.getDictionary()->get("ColorSpace"));
            if (colorSpaceDictionary)
            {
                std::size_t colorSpaces = colorSpaceDictionary->getCount();
                for (size_t csIndex = 0; csIndex < colorSpaces; ++ csIndex)
                {
                    PDFColorSpacePointer colorSpacePointer;
                    try
                    {
                        colorSpacePointer = PDFAbstractColorSpace::createColorSpace(colorSpaceDictionary, m_document, m_document->getObject(colorSpaceDictionary->getValue(csIndex)));
                    }
                    catch (const PDFException&)
                    {
                        // Ignore invalid color spaces
                        continue;
                    }

                    if (!colorSpacePointer)
                    {
                        continue;
                    }

                    switch (colorSpacePointer->getColorSpace())
                    {
                        case PDFAbstractColorSpace::ColorSpace::Separation:
                        {
                            const PDFSeparationColorSpace* separationColorSpace = dynamic_cast<const PDFSeparationColorSpace*>(colorSpacePointer.data());

                            if (!separationColorSpace->isNone() && !separationColorSpace->isAll() && !separationColorSpace->getColorName().isEmpty())
                            {
                                // Try to add spot color
                                const QByteArray& colorName = separationColorSpace->getColorName();
                                if (!containsSpotColor(colorName) && !containsProcessColor(colorName))
                                {
                                    ColorInfo info;
                                    info.name = colorName;
                                    info.textName = PDFEncoding::convertTextString(info.name);
                                    info.colorSpace = colorSpacePointer;
                                    info.spotColorIndex = uint32_t(m_spotColors.size());
                                    info.color = cms ? separationColorSpace->getColor(pdf::PDFColor(1.0f), cms.get(), pdf::RenderingIntent::Perceptual, &renderErrorReporter, true) : nullptr;
                                    m_spotColors.emplace_back(qMove(info));
                                }
                            }

                            break;
                        }

                        case PDFAbstractColorSpace::ColorSpace::DeviceN:
                        {
                            const PDFDeviceNColorSpace* deviceNColorSpace = dynamic_cast<const PDFDeviceNColorSpace*>(colorSpacePointer.data());

                            if (!deviceNColorSpace->isNone())
                            {
                                const PDFDeviceNColorSpace::Colorants& colorants = deviceNColorSpace->getColorants();
                                for (size_t ii = 0; ii < colorants.size(); ++ii)
                                {
                                    const PDFDeviceNColorSpace::ColorantInfo& colorantInfo = colorants[ii];
                                    if (!containsSpotColor(colorantInfo.name) && !containsProcessColor(colorantInfo.name))
                                    {
                                        PDFColor color;
                                        color.resize(deviceNColorSpace->getColorComponentCount());
                                        color[ii] = 1.0f;

                                        ColorInfo info;
                                        info.name = colorantInfo.name;
                                        info.textName = PDFEncoding::convertTextString(info.name);
                                        info.colorSpaceIndex = uint32_t(ii);
                                        info.colorSpace = colorSpacePointer;
                                        info.spotColorIndex = uint32_t(m_spotColors.size());
                                        info.color = cms ? deviceNColorSpace->getColor(color, cms.get(), pdf::RenderingIntent::Perceptual, &renderErrorReporter, true) : nullptr;
                                        m_spotColors.emplace_back(qMove(info));
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
        }
    }

    size_t minIndex = qMin<uint32_t>(uint32_t(m_spotColors.size()), MAX_SPOT_COLOR_COMPONENTS);
    for (size_t i = 0; i < minIndex; ++i)
    {
        m_spotColors[i].canBeActive = true;
    }

    setSpotColorsActive(activate);
}

bool PDFInkMapper::containsSpotColor(const QByteArray& colorName) const
{
    return getSpotColor(colorName) != nullptr;
}

bool PDFInkMapper::containsProcessColor(const QByteArray& colorName) const
{
    return getProcessColor(colorName) != nullptr;
}

const PDFInkMapper::ColorInfo* PDFInkMapper::getSpotColor(const QByteArray& colorName) const
{
    auto it = std::find_if(m_spotColors.cbegin(), m_spotColors.cend(), [&colorName](const auto& info) { return info.name == colorName; });
    if (it != m_spotColors.cend())
    {
        return &*it;
    }

    return nullptr;
}

const PDFInkMapper::ColorInfo* PDFInkMapper::getProcessColor(const QByteArray& colorName) const
{
    auto it = std::find_if(m_deviceColors.cbegin(), m_deviceColors.cend(), [&colorName](const auto& info) { return info.name == colorName; });
    if (it != m_deviceColors.cend())
    {
        return &*it;
    }

    return nullptr;
}

const PDFInkMapper::ColorInfo* PDFInkMapper::getActiveProcessColor(const QByteArray& colorName, PDFAbstractColorSpace::ColorSpace colorSpace) const
{
    auto it = std::find_if(m_deviceColors.cbegin(), m_deviceColors.cend(), [&colorName, colorSpace](const auto& info) { return info.name == colorName && info.active && info.colorSpaceType == colorSpace; });
    if (it != m_deviceColors.cend())
    {
        return &*it;
    }

    return nullptr;
}

const PDFInkMapper::ColorInfo* PDFInkMapper::getActiveSpotColor(size_t index) const
{
    for (const ColorInfo& info : m_spotColors)
    {
        if (info.active)
        {
            if (index == 0)
            {
                return &info;
            }

            --index;
        }
    }

    return nullptr;
}

void PDFInkMapper::setSpotColorsActive(bool active)
{
    m_activeSpotColors = 0;

    if (active)
    {
        for (auto& spotColor : m_spotColors)
        {
            if (spotColor.canBeActive)
            {
                spotColor.active = true;
                ++m_activeSpotColors;
            }
        }
    }
    else
    {
        for (auto& spotColor : m_spotColors)
        {
            spotColor.active = false;
        }
    }
}

PDFInkMapping PDFInkMapper::createMapping(const PDFAbstractColorSpace* sourceColorSpace,
                                          const PDFAbstractColorSpace* targetColorSpace,
                                          PDFPixelFormat targetPixelFormat) const
{
    PDFInkMapping mapping;

    const PDFAbstractColorSpace::ColorSpace colorSpaceType = targetColorSpace->getColorSpace();
    Q_ASSERT(targetColorSpace->getColorComponentCount() == targetPixelFormat.getProcessColorChannelCount());

    if (sourceColorSpace->equals(targetColorSpace))
    {
        Q_ASSERT(sourceColorSpace->getColorComponentCount() == targetColorSpace->getColorComponentCount());

        uint8_t colorCount = uint8_t(targetColorSpace->getColorComponentCount());
        mapping.mapping.reserve(colorCount);

        for (uint8_t i = 0; i < colorCount; ++i)
        {
            mapping.map(i, i);
        }
    }
    else
    {
        switch (sourceColorSpace->getColorSpace())
        {
            case PDFAbstractColorSpace::ColorSpace::Separation:
            {
                const PDFSeparationColorSpace* separationColorSpace = dynamic_cast<const PDFSeparationColorSpace*>(sourceColorSpace);

                if (separationColorSpace->isAll())
                {
                    // Map this color to all device colors
                    uint32_t colorCount = static_cast<uint32_t>(targetColorSpace->getColorComponentCount());
                    mapping.mapping.reserve(colorCount);

                    for (size_t i = 0; i < colorCount; ++i)
                    {
                        mapping.map(0, uint8_t(i));
                    }
                }
                else if (!separationColorSpace->isNone() && !separationColorSpace->getColorName().isEmpty())
                {
                    const QByteArray& colorName = separationColorSpace->getColorName();

                    // First try to map it as process color, if we do not succeed, then map it as spot color
                    const ColorInfo* processColor = getActiveProcessColor(colorName, colorSpaceType);
                    if (processColor)
                    {
                        if (targetPixelFormat.hasProcessColors() && processColor->spotColorIndex < targetPixelFormat.getProcessColorChannelCount())
                        {
                            mapping.map(0, uint8_t(targetPixelFormat.getProcessColorChannelIndexStart() + processColor->spotColorIndex));
                        }
                    }
                    else
                    {
                        const ColorInfo* spotColor = getSpotColor(colorName);
                        if (spotColor && spotColor->active && targetPixelFormat.hasSpotColors() && spotColor->spotColorIndex < targetPixelFormat.getSpotColorChannelCount())
                        {
                            mapping.map(0, uint8_t(targetPixelFormat.getSpotColorChannelIndexStart() + spotColor->spotColorIndex));
                        }
                    }
                }

                break;
            }

            case PDFAbstractColorSpace::ColorSpace::DeviceN:
            {
                const PDFDeviceNColorSpace* deviceNColorSpace = dynamic_cast<const PDFDeviceNColorSpace*>(sourceColorSpace);

                if (!deviceNColorSpace->isNone())
                {
                    const PDFDeviceNColorSpace::Colorants& colorants = deviceNColorSpace->getColorants();
                    for (size_t i = 0; i < colorants.size(); ++i)
                    {
                        const PDFDeviceNColorSpace::ColorantInfo& colorantInfo = colorants[i];

                        // First try to map it as process color, if we do not succeed, then map it as spot color
                        const ColorInfo* processColor = getActiveProcessColor(colorantInfo.name, colorSpaceType);
                        if (processColor)
                        {
                            if (targetPixelFormat.hasProcessColors() && processColor->spotColorIndex < targetPixelFormat.getProcessColorChannelCount())
                            {
                                mapping.map(uint8_t(i), uint8_t(targetPixelFormat.getProcessColorChannelIndexStart() + processColor->spotColorIndex));
                            }
                        }
                        else
                        {
                            const ColorInfo* info = getSpotColor(colorantInfo.name);

                            if (info && info->active && targetPixelFormat.hasSpotColors() && info->spotColorIndex < targetPixelFormat.getSpotColorChannelCount())
                            {
                                mapping.map(uint8_t(i), uint8_t(targetColorSpace->getColorComponentCount() + info->spotColorIndex));
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

    return mapping;
}

PDFPainterPathSampler::PDFPainterPathSampler(QPainterPath path, int samplesCount, PDFColorComponent defaultShape, QRect fillRect, bool precise) :
    m_defaultShape(defaultShape),
    m_samplesCount(qMax(samplesCount, 1)),
    m_path(qMove(path)),
    m_fillRect(fillRect),
    m_precise(precise)
{
    if (!precise)
    {
        m_fillPolygon = m_path.toFillPolygon();
        prepareScanLines();
    }
}

PDFColorComponent PDFPainterPathSampler::sample(QPoint point) const
{
    if (m_path.isEmpty() || !m_fillRect.contains(point))
    {
        return m_defaultShape;
    }

    if (!m_scanLineInfo.empty())
    {
        return sampleByScanLine(point);
    }

    const qreal coordX1 = point.x();
    const qreal coordX2 = coordX1 + 1.0;
    const qreal coordY1 = point.y();
    const qreal coordY2 = coordY1 + 1.0;

    const qreal centerX = (coordX1 + coordX2) * 0.5;
    const qreal centerY = (coordY1 + coordY2) * 0.5;

    const QPointF topLeft(coordX1, coordY1);
    const QPointF topRight(coordX2, coordY1);
    const QPointF bottomLeft(coordX1, coordY2);
    const QPointF bottomRight(coordX2, coordY2);

    if (m_samplesCount <= 1)
    {
        // Jakub Melka: Just one sample
        if (m_precise)
        {
            return m_path.contains(QPointF(centerX, centerY)) ? 1.0f : 0.0f;
        }
        else
        {
            return m_fillPolygon.contains(QPointF(centerX, centerY)) ? 1.0f : 0.0f;
        }
    }

    int cornerHits = 0;
    Qt::FillRule fillRule = m_path.fillRule();

    if (m_precise)
    {
        cornerHits += m_path.contains(topLeft) ? 1 : 0;
        cornerHits += m_path.contains(topRight) ? 1 : 0;
        cornerHits += m_path.contains(bottomLeft) ? 1 : 0;
        cornerHits += m_path.contains(bottomRight) ? 1 : 0;
    }
    else
    {
        cornerHits += m_fillPolygon.containsPoint(topLeft, fillRule) ? 1 : 0;
        cornerHits += m_fillPolygon.containsPoint(topRight, fillRule) ? 1 : 0;
        cornerHits += m_fillPolygon.containsPoint(bottomLeft, fillRule) ? 1 : 0;
        cornerHits += m_fillPolygon.containsPoint(bottomRight, fillRule) ? 1 : 0;
    }

    if (cornerHits == 4)
    {
        // Completely inside
        return 1.0;
    }

    if (cornerHits == 0)
    {
        // Completely outside
        return 0.0;
    }

    // Otherwise we must use regular sample grid
    const qreal offset = 1.0f / PDFColorComponent(m_samplesCount + 1);
    PDFColorComponent sampleValue = 0.0f;
    const PDFColorComponent sampleGain = 1.0f / PDFColorComponent(m_samplesCount * m_samplesCount);
    for (int ix = 0; ix < m_samplesCount; ++ix)
    {
        const qreal x = offset * (ix + 1) + coordX1;

        for (int iy = 0; iy < m_samplesCount; ++iy)
        {
            const qreal y = offset * (iy + 1) + coordY1;

            if (m_precise)
            {
                if (m_path.contains(QPointF(x, y)))
                {
                    sampleValue += sampleGain;
                }
            }
            else
            {
                if (m_fillPolygon.containsPoint(QPointF(x, y), fillRule))
                {
                    sampleValue += sampleGain;
                }
            }
        }
    }

    return sampleValue;
}

PDFColorComponent PDFPainterPathSampler::sampleByScanLine(QPoint point) const
{
    int scanLinePosition = point.y() - m_fillRect.y();

    size_t scanLineCountPerPixel = getScanLineCountPerPixel();
    size_t scanLineTopRow = scanLinePosition * scanLineCountPerPixel;
    size_t scanLineBottomRow = scanLineTopRow + scanLineCountPerPixel - 1;
    size_t scanLineGridRowTop = scanLineTopRow + 1;

    Qt::FillRule fillRule = m_path.fillRule();

    auto performSampling = [&](size_t scanLineIndex, PDFReal firstOrdinate, int sampleCount, PDFReal step, PDFReal gain)
    {
        ScanLineInfo info = m_scanLineInfo[scanLineIndex];
        auto it = std::next(m_scanLineSamples.cbegin(), info.indexStart);

        PDFReal ordinate = firstOrdinate;
        PDFColorComponent value = 0.0;
        auto ordinateIt = it;
        for (int i = 0; i < sampleCount; ++i)
        {
            while (std::next(ordinateIt)->x < ordinate)
            {
                ++ordinateIt;
            }

            int windingNumber = ordinateIt->windingNumber;

            const bool inside = (fillRule == Qt::WindingFill) ? windingNumber != 0 : windingNumber % 2 != 0;
            if (inside)
            {
                value += gain;
            }

            ordinate += step;
        }

        return value;
    };

    const qreal coordX1 = point.x();
    const PDFReal cornerValue = performSampling(scanLineTopRow, coordX1, 2, 1.0, 1.0) + performSampling(scanLineBottomRow, coordX1, 2, 1.0, 1.0);

    if (qFuzzyIsNull(4.0 - cornerValue))
    {
        // Whole inside
        return 1.0;
    }

    if (qFuzzyIsNull(cornerValue))
    {
        // Whole outside
        return 0.0;
    }

    const qreal offset = 1.0f / PDFColorComponent(m_samplesCount + 1);
    PDFColorComponent sampleValue = 0.0f;
    const PDFColorComponent sampleGain = 1.0f / PDFColorComponent(m_samplesCount * m_samplesCount);

    for (int i = 0; i < m_samplesCount; ++i)
    {
        sampleValue += performSampling(scanLineGridRowTop++, coordX1 + offset, m_samplesCount, offset, sampleGain);
    }

    return sampleValue;
}

size_t PDFPainterPathSampler::getScanLineCountPerPixel() const
{
    return m_samplesCount + 2;
}

void PDFPainterPathSampler::prepareScanLines()
{
    if (m_fillPolygon.isEmpty())
    {
        return;
    }

    for (int yOffset = m_fillRect.top(); yOffset <= m_fillRect.bottom(); ++yOffset)
    {
        const qreal coordY1 = yOffset;
        const qreal coordY2 = coordY1 + 1.0;

        // Top pixel line
        if (m_scanLineInfo.empty())
        {
            m_scanLineInfo.emplace_back(createScanLine(coordY1));
        }
        else
        {
            m_scanLineInfo.emplace_back(m_scanLineInfo.back());
        }

        // Sample grid
        const qreal offset = 1.0f / PDFColorComponent(m_samplesCount + 1);
        for (int iy = 0; iy < m_samplesCount; ++iy)
        {
            const qreal y = offset * (iy + 1) + coordY1;
            m_scanLineInfo.emplace_back(createScanLine(y));
        }

        // Bottom pixel line
        m_scanLineInfo.emplace_back(createScanLine(coordY2));
    }
}

PDFPainterPathSampler::ScanLineInfo PDFPainterPathSampler::createScanLine(qreal y)
{
    ScanLineInfo result;
    result.indexStart = m_scanLineSamples.size();

    // Add start item
    m_scanLineSamples.emplace_back(-std::numeric_limits<PDFReal>::infinity(), 0);

    // Traverse polygon, add sample for each polygon line, we must
    // also implicitly close last edge (if polygon is not closed)
    for (int i = 1; i < m_fillPolygon.size(); ++i)
    {
        createScanLineSample(m_fillPolygon[i - 1], m_fillPolygon[i], y);
    }

    if (m_fillPolygon.front() != m_fillPolygon.back())
    {
        createScanLineSample(m_fillPolygon.back(), m_fillPolygon.front(), y);
    }

    // Add end item
    m_scanLineSamples.emplace_back(+std::numeric_limits<PDFReal>::infinity(), 0);

    result.indexEnd = m_scanLineSamples.size();

    auto it = std::next(m_scanLineSamples.begin(), result.indexStart);
    auto itEnd = std::next(m_scanLineSamples.begin(), result.indexEnd);

    // Jakub Melka: now, sort the line samples and compute properly the winding number
    std::sort(it, itEnd);

    int currentWindingNumber = 0;
    for (; it != itEnd; ++it)
    {
        currentWindingNumber += it->windingNumber;
        it->windingNumber = currentWindingNumber;
    }

    return result;
}

void PDFPainterPathSampler::createScanLineSample(const QPointF& p1, const QPointF& p2, qreal y)
{
    PDFReal y1 = p1.y();
    PDFReal y2 = p2.y();

    if (qFuzzyIsNull(y2 - y1))
    {
        // Ignore horizontal lines
        return;
    }

    PDFReal x1 = p1.x();
    PDFReal x2 = p2.x();

    int windingNumber = 1;
    if (y2 < y1)
    {
        std::swap(y1, y2);
        std::swap(x1, x2);
        windingNumber = -1;
    }

    // Do we have intercept?
    if (y1 <= y && y < y2)
    {
        const PDFReal x = interpolate(y, y1, y2, x1, x2);
        m_scanLineSamples.emplace_back(x, windingNumber);
    }
}

void PDFDrawBuffer::clear()
{
    if (!m_modifiedRect.isValid())
    {
        return;
    }

    for (int x = m_modifiedRect.left(); x <= m_modifiedRect.right(); ++x)
    {
        for (int y = m_modifiedRect.top(); y <= m_modifiedRect.bottom(); ++y)
        {
            PDFColorBuffer buffer = getPixel(x, y);
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            setPixelActiveColorMask(x, y, 0);
        }
    }

    m_containsFilling = false;
    m_containsStroking = false;
    m_modifiedRect = QRect();
}

void PDFDrawBuffer::modify(QRect rect, bool containsFilling, bool containsStroking)
{
    m_modifiedRect = m_modifiedRect.united(rect);
    m_containsFilling |= containsFilling;
    m_containsStroking |= containsStroking;
}

void PDFTransparencyRenderer::PDFTransparencyGroupPainterData::makeInitialBackdropTransparent()
{
    initialBackdrop.makeTransparent();
}

void PDFTransparencyRenderer::PDFTransparencyGroupPainterData::makeImmediateBackdropTransparent()
{
    immediateBackdrop.makeTransparent();
    immediateBackdrop.setAllColorInactive();
}

void PDFTransparencyRenderer::PDFTransparencySoftMask::makeOpaque()
{
    if (!isOpaque())
    {
        m_data->isOpaque = true;
        m_data->softMask.makeOpaque();
    }
}

PDFInkCoverageCalculator::PDFInkCoverageCalculator(const PDFDocument* document,
                                                   const PDFFontCache* fontCache,
                                                   const PDFCMSManager* cmsManager,
                                                   const PDFOptionalContentActivity* optionalContentActivity,
                                                   const PDFInkMapper* inkMapper,
                                                   PDFProgress* progress,
                                                   PDFTransparencyRendererSettings settings) :
    m_document(document),
    m_fontCache(fontCache),
    m_cmsManager(cmsManager),
    m_optionalContentActivity(optionalContentActivity),
    m_inkMapper(inkMapper),
    m_progress(progress),
    m_settings(settings)
{
    Q_ASSERT(m_document);
    Q_ASSERT(m_fontCache);
    Q_ASSERT(m_cmsManager);
    Q_ASSERT(m_optionalContentActivity);
    Q_ASSERT(m_inkMapper);
}

void PDFInkCoverageCalculator::perform(QSize size, const std::vector<PDFInteger>& pages)
{
    if (pages.empty())
    {
        // Nothing to do
        return;
    }

    if (m_progress)
    {
        m_progress->start(pages.size(), ProgressStartupInfo());
    }

    auto calculatePageCoverage = [this, size](PDFInteger pageIndex)
    {
        if (pageIndex >= PDFInteger(m_document->getCatalog()->getPageCount()))
        {
            return;
        }

        const PDFPage* page = m_document->getCatalog()->getPage(pageIndex);
        if (!page)
        {
            return;
        }

        QRectF pageRect = page->getRotatedMediaBox();
        QSizeF pageSize = pageRect.size();
        pageSize.scale(size.width(), size.height(), Qt::KeepAspectRatio);
        QSize imageSize = pageSize.toSize();

        if (!imageSize.isValid())
        {
            return;
        }

        pdf::PDFTransparencyRendererSettings settings;
        settings.flags.setFlag(PDFTransparencyRendererSettings::SaveOriginalProcessImage, true);

        // Jakub Melka: debug is very slow, use multithreading
#ifdef QT_DEBUG
        settings.flags.setFlag(PDFTransparencyRendererSettings::MultithreadedPathSampler, true);
#endif

        settings.flags.setFlag(PDFTransparencyRendererSettings::ActiveColorMask, false);
        settings.flags.setFlag(PDFTransparencyRendererSettings::SeparationSimulation, true);
        settings.activeColorMask = PDFPixelFormat::getAllColorsMask();

        QTransform pagePointToDevicePoint = pdf::PDFRenderer::createPagePointToDevicePointMatrix(page, QRect(QPoint(0, 0), imageSize));
        pdf::PDFCMSPointer cms = m_cmsManager->getCurrentCMS();
        pdf::PDFTransparencyRenderer renderer(page, m_document, m_fontCache, cms.data(), m_optionalContentActivity,
                                              m_inkMapper, settings, pagePointToDevicePoint);

        renderer.beginPaint(imageSize);
        renderer.processContents();
        renderer.endPaint();

        PDFFloatBitmapWithColorSpace originalProcessImage = renderer.getOriginalProcessBitmap();
        QSizeF pageSizeMM = page->getRotatedMediaBoxMM().size();

        pdf::PDFPixelFormat pixelFormat = originalProcessImage.getPixelFormat();
        pdf::PDFColorComponent totalArea = pageSizeMM.width() * pageSizeMM.height();
        pdf::PDFColorComponent pixelArea = totalArea / pdf::PDFColorComponent(originalProcessImage.getWidth() * originalProcessImage.getHeight());

        std::vector<PDFColorComponent> pageCoverage;
        const uint8_t colorChannelCount = pixelFormat.getColorChannelCount();
        pageCoverage.resize(colorChannelCount, 0.0f);

        for (size_t y = 0; y < originalProcessImage.getHeight(); ++y)
        {
            for (size_t x = 0; x < originalProcessImage.getWidth(); ++x)
            {
                const pdf::PDFColorBuffer buffer = originalProcessImage.getPixel(x, y);
                const pdf::PDFColorComponent alpha = pixelFormat.hasOpacityChannel() ? buffer[pixelFormat.getOpacityChannelIndex()] : 1.0f;

                for (uint8_t i = 0; i < colorChannelCount; ++i)
                {
                    pageCoverage[i] += buffer[i] * alpha;
                }
            }
        }

        std::vector<PDFColorComponent> pageRatioCoverage = pageCoverage;
        for (uint8_t i = 0; i < colorChannelCount; ++i)
        {
            pageCoverage[i] *= pixelArea;
            pageRatioCoverage[i] *= pixelArea / totalArea;
        }

        std::vector<PDFInkMapper::ColorInfo> separations = m_inkMapper->getSeparations(pixelFormat.getProcessColorChannelCount());
        Q_ASSERT(pixelFormat.getColorChannelCount() == separations.size());

        std::vector<InkCoverageChannelInfo> results;
        results.reserve(separations.size());

        for (size_t i = 0; i < separations.size(); ++i)
        {
            const PDFInkMapper::ColorInfo& colorInfo = separations[i];

            InkCoverageChannelInfo info;
            info.color = colorInfo.color;
            info.name = colorInfo.name;
            info.textName = colorInfo.textName;
            info.isSpot = colorInfo.isSpot;
            info.coveredArea = pageCoverage[i];
            info.ratio = pageRatioCoverage[i];
            results.emplace_back(qMove(info));
        }

        if (m_progress)
        {
            m_progress->step();
        }

        QMutexLocker lock(&m_mutex);
        m_inkCoverageResults[pageIndex] = qMove(results);
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, pages.begin(), pages.end(), calculatePageCoverage);

    if (m_progress)
    {
        m_progress->finish();
    }
}

void PDFInkCoverageCalculator::clear()
{
    QMutexLocker lock(&m_mutex);
    m_inkCoverageResults.clear();
}

const std::vector<PDFInkCoverageCalculator::InkCoverageChannelInfo>* PDFInkCoverageCalculator::getInkCoverage(PDFInteger pageIndex) const
{
    auto it = m_inkCoverageResults.find(pageIndex);
    if (it != m_inkCoverageResults.end())
    {
        return &it->second;
    }

    static const std::vector<PDFInkCoverageCalculator::InkCoverageChannelInfo> dummy;
    return &dummy;
}

const PDFInkCoverageCalculator::InkCoverageChannelInfo* PDFInkCoverageCalculator::findCoverageInfoByName(const std::vector<PDFInkCoverageCalculator::InkCoverageChannelInfo>& infos,
                                                                                                         const QByteArray& name)
{
    auto it = std::find_if(infos.cbegin(), infos.cend(), [&name](const auto& info) { return info.name == name; });
    if (it != infos.cend())
    {
        return &*it;
    }

    return nullptr;
}

PDFInkCoverageCalculator::InkCoverageChannelInfo* PDFInkCoverageCalculator::findCoverageInfoByName(std::vector<PDFInkCoverageCalculator::InkCoverageChannelInfo>& infos, const QByteArray& name)
{
    auto it = std::find_if(infos.begin(), infos.end(), [&name](const auto& info) { return info.name == name; });
    if (it != infos.cend())
    {
        return &*it;
    }

    return nullptr;
}

}   // namespace pdf
