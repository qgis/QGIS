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

#include "pdfcolorspaces.h"
#include "pdfobject.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfutils.h"
#include "pdfpattern.h"
#include "pdfcms.h"
#include "pdfexecutionpolicy.h"

#include <QCryptographicHash>

#include "pdfdbgheap.h"

#include <execution>

namespace pdf
{

PDFColorComponentMatrix_3x3 getInverseMatrix(const PDFColorComponentMatrix_3x3& matrix)
{
    const PDFColorComponent a_11 = matrix.getValue(0, 0);
    const PDFColorComponent a_12 = matrix.getValue(0, 1);
    const PDFColorComponent a_13 = matrix.getValue(0, 2);
    const PDFColorComponent a_21 = matrix.getValue(1, 0);
    const PDFColorComponent a_22 = matrix.getValue(1, 1);
    const PDFColorComponent a_23 = matrix.getValue(1, 2);
    const PDFColorComponent a_31 = matrix.getValue(2, 0);
    const PDFColorComponent a_32 = matrix.getValue(2, 1);
    const PDFColorComponent a_33 = matrix.getValue(2, 2);

    const PDFColorComponent determinant = -a_13* a_22 * a_31 + a_12 * a_23 * a_31 + a_13 * a_21 * a_32 - a_11 * a_23 * a_32 - a_12 * a_21 * a_33 + a_11 * a_22 * a_33;
    const PDFColorComponent coefficient = !qIsNull(determinant) ? 1.0 / determinant : 0.0;

    PDFColorComponentMatrix_3x3 inversedMatrix { a_22 * a_33 - a_23 * a_32, a_13 * a_32 - a_12 * a_33, a_12 * a_23 - a_13 * a_22,
                                                 a_23 * a_31 - a_21 * a_33, a_11 * a_33 - a_13 * a_31, a_13 * a_21 - a_11 * a_23,
                                                 a_21 * a_32 - a_22 * a_31, a_12 * a_31 - a_11 * a_32, a_11 * a_22 - a_12 * a_21 };
    inversedMatrix.multiplyByFactor(coefficient);
    return inversedMatrix;
}

PDFColor PDFDeviceGrayColorSpace::getDefaultColorOriginal() const
{
    return PDFColor(0.0f);
}

QColor PDFDeviceGrayColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(cms);
    Q_ASSERT(color.size() == getColorComponentCount());
    Q_UNUSED(isRange01);

    PDFColorComponent component = clip01(color[0]);

    // If color management system handles the color transformation, then use it,
    // otherwise fall back to the generic case.
    QColor cmsColor = cms->getColorFromDeviceGray(color, intent, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    QColor result(QColor::Rgb);
    result.setRgbF(component, component, component, 1.0);
    return result;
}

size_t PDFDeviceGrayColorSpace::getColorComponentCount() const
{
    return 1;
}

void PDFDeviceGrayColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    if (!cms->fillRGBBufferFromDeviceGray(colors, intent, outputBuffer, reporter))
    {
        PDFAbstractColorSpace::fillRGBBuffer(colors, outputBuffer, intent, cms, reporter);
    }
}

PDFColor PDFDeviceRGBColorSpace::getDefaultColorOriginal() const
{
    return PDFColor(0.0f, 0.0f, 0.0f);
}

QColor PDFDeviceRGBColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(color.size() == getColorComponentCount());
    Q_UNUSED(isRange01);

    PDFColorComponent r = clip01(color[0]);
    PDFColorComponent g = clip01(color[1]);
    PDFColorComponent b = clip01(color[2]);

    PDFColor clippedColor(r, g, b);
    QColor cmsColor = cms->getColorFromDeviceRGB(clippedColor, intent, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    QColor result(QColor::Rgb);
    result.setRgbF(r, g, b, 1.0);
    return result;
}

size_t PDFDeviceRGBColorSpace::getColorComponentCount() const
{
    return 3;
}

void PDFDeviceRGBColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    if (!cms->fillRGBBufferFromDeviceRGB(colors, intent, outputBuffer, reporter))
    {
        PDFAbstractColorSpace::fillRGBBuffer(colors, outputBuffer, intent, cms, reporter);
    }
}

PDFColor PDFDeviceCMYKColorSpace::getDefaultColorOriginal() const
{
    return PDFColor(0.0f, 0.0f, 0.0f, 1.0f);
}

QColor PDFDeviceCMYKColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(color.size() == getColorComponentCount());
    Q_UNUSED(isRange01);

    PDFColorComponent c = clip01(color[0]);
    PDFColorComponent m = clip01(color[1]);
    PDFColorComponent y = clip01(color[2]);
    PDFColorComponent k = clip01(color[3]);

    PDFColor clippedColor(c, m, y, k);
    QColor cmsColor = cms->getColorFromDeviceCMYK(clippedColor, intent, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    QColor result(QColor::Cmyk);
    result.setCmykF(c, m, y, k, 1.0);
    return result;
}

size_t PDFDeviceCMYKColorSpace::getColorComponentCount() const
{
    return 4;
}

void PDFDeviceCMYKColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    if (!cms->fillRGBBufferFromDeviceCMYK(colors, intent, outputBuffer, reporter))
    {
        PDFAbstractColorSpace::fillRGBBuffer(colors, outputBuffer, intent, cms, reporter);
    }
}

bool PDFAbstractColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    return getColorSpace() == other->getColorSpace();
}

bool PDFAbstractColorSpace::isBlendColorSpace() const
{
    switch (getColorSpace())
    {
        case pdf::PDFAbstractColorSpace::ColorSpace::DeviceGray:
        case pdf::PDFAbstractColorSpace::ColorSpace::DeviceRGB:
        case pdf::PDFAbstractColorSpace::ColorSpace::DeviceCMYK:
        case pdf::PDFAbstractColorSpace::ColorSpace::CalGray:
        case pdf::PDFAbstractColorSpace::ColorSpace::CalRGB:
        case pdf::PDFAbstractColorSpace::ColorSpace::ICCBased:
            return true;

        default:
            return false;
    }
}

QColor PDFAbstractColorSpace::getDefaultColor(const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    return getColor(getDefaultColorOriginal(), cms, intent, reporter, true);
}

QImage PDFAbstractColorSpace::getImage(const PDFImageData& imageData,
                                       const PDFImageData& softMask,
                                       const PDFCMS* cms,
                                       RenderingIntent intent,
                                       PDFRenderErrorReporter* reporter,
                                       const PDFOperationControl* operationControl) const
{
    if (imageData.isValid())
    {
        switch (imageData.getMaskingType())
        {
            case PDFImageData::MaskingType::None:
            {
                QImage image(imageData.getWidth(), imageData.getHeight(), QImage::Format_RGB888);
                image.fill(QColor(Qt::white));

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != getColorComponentCount())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(getColorComponentCount()).arg(componentCount));
                }

                const std::vector<PDFReal>& decode = imageData.getDecode();
                if (!decode.empty() && decode.size() != componentCount * 2)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid size of the decode array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
                }

                const unsigned int imageWidth = imageData.getWidth();
                const unsigned int imageHeight = imageData.getHeight();

                QMutex exceptionMutex;
                std::optional<PDFException> exception;

                auto transformPixelLine = [&](unsigned int i)
                {
                    // Is operation being cancelled?
                    if (PDFOperationControl::isOperationCancelled(operationControl))
                    {
                        return;
                    }

                    try
                    {
                        PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());
                        reader.seek(i * imageData.getStride());

                        const double max = reader.max();
                        const double coefficient = 1.0 / max;
                        unsigned char* outputLine = image.scanLine(i);

                        std::vector<float> inputColors(imageWidth * componentCount, 0.0f);
                        auto itInputColor = inputColors.begin();

                        if (!decode.empty())
                        {
                            // Interpolate value
                            for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                            {
                                for (unsigned int k = 0; k < componentCount; ++k)
                                {
                                    PDFReal value = reader.read();
                                    *itInputColor++ = interpolate(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                                }
                            }
                        }
                        else
                        {
                            // Just read all values and multiply them with coefficient
                            const unsigned int pixelCount = imageData.getWidth() * componentCount;
                            for (unsigned int j = 0; j < pixelCount; ++j)
                            {
                                PDFReal value = reader.read();
                                *itInputColor++ = value * coefficient;
                            }
                        }

                        fillRGBBuffer(inputColors, outputLine, intent, cms, reporter);
                    }
                    catch (const PDFException &lineException)
                    {
                        QMutexLocker lock(&exceptionMutex);
                        if (!exception)
                        {
                            exception = lineException;
                        }
                    }
                };

                auto range = PDFIntegerRange<unsigned int>(0, imageHeight);
                PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), transformPixelLine);

                if (exception)
                {
                    throw *exception;
                }

                return image;
            }

            case PDFImageData::MaskingType::SoftMask:
            {
                const bool hasMatte = !softMask.getMatte().empty();
                QImage image(imageData.getWidth(), imageData.getHeight(), hasMatte ? QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBA8888);
                image.fill(QColor(Qt::white));

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != getColorComponentCount())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(getColorComponentCount()).arg(componentCount));
                }

                const std::vector<PDFReal>& decode = imageData.getDecode();
                if (!decode.empty() && decode.size() != componentCount * 2)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid size of the decode array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
                }

                const unsigned int imageWidth = imageData.getWidth();
                const unsigned int imageHeight = imageData.getHeight();

                QImage alphaMask = createAlphaMask(softMask);
                if (alphaMask.size() != image.size())
                {
                    // Scale the alpha mask, if it is masked
                    alphaMask = alphaMask.scaled(image.size());
                }

                QMutex exceptionMutex;
                std::optional<PDFException> exception;

                auto transformPixelLine = [&](unsigned int i)
                {
                    // Is operation being cancelled?
                    if (PDFOperationControl::isOperationCancelled(operationControl))
                    {
                        return;
                    }

                    try
                    {
                        PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());
                        reader.seek(i * imageData.getStride());

                        const double max = reader.max();
                        const double coefficient = 1.0 / max;
                        unsigned char* outputLine = image.scanLine(i);
                        unsigned char* alphaLine = alphaMask.scanLine(i);

                        std::vector<float> inputColors(imageWidth * componentCount, 0.0f);
                        std::vector<unsigned char> outputColors(imageWidth * 3, 0);

                        auto itInputColor = inputColors.begin();
                        for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                        {
                            for (unsigned int k = 0; k < componentCount; ++k)
                            {
                                PDFReal value = reader.read();

                                // Interpolate value, if it is not empty
                                if (!decode.empty())
                                {
                                    *itInputColor++ = interpolate(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                                }
                                else
                                {
                                    *itInputColor++ = value * coefficient;
                                }
                            }
                        }

                        fillRGBBuffer(inputColors, outputColors.data(), intent, cms, reporter);

                        const unsigned char* transformedLine = outputColors.data();
                        for (unsigned int ii = 0; ii < imageWidth; ++ii)
                        {
                            *outputLine++ = *transformedLine++;
                            *outputLine++ = *transformedLine++;
                            *outputLine++ = *transformedLine++;
                            *outputLine++ = *alphaLine++;
                        }
                    }
                    catch (const PDFException &lineException)
                    {
                        QMutexLocker lock(&exceptionMutex);
                        if (!exception)
                        {
                            exception = lineException;
                        }
                    }
                };

                auto range = PDFIntegerRange<unsigned int>(0, imageHeight);
                PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, range.begin(), range.end(), transformPixelLine);

                if (exception)
                {
                    throw *exception;
                }

                return image;
            }

            case PDFImageData::MaskingType::ColorKeyMasking:
            {
                QImage image(imageData.getWidth(), imageData.getHeight(), QImage::Format_RGBA8888);
                image.fill(QColor(Qt::transparent));

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != getColorComponentCount())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(getColorComponentCount()).arg(componentCount));
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

                const double max = reader.max();
                const double coefficient = 1.0 / max;
                for (unsigned int i = 0, rowCount = imageData.getHeight(); i < rowCount; ++i)
                {
                    reader.seek(i * imageData.getStride());
                    unsigned char* outputLine = image.scanLine(i);

                    for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                    {
                        // Number of masked-out colors
                        unsigned int maskedColors = 0;

                        for (unsigned int k = 0; k < componentCount; ++k)
                        {
                            PDFBitReader::Value value = reader.read();

                            // Interpolate value, if decode is not empty
                            if (!decode.empty())
                            {
                                color[k] = interpolate(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                            }
                            else
                            {
                                color[k] = value * coefficient;
                            }

                            Q_ASSERT(2 * k + 1 < colorKeyMask.size());
                            if (static_cast<std::decay<decltype(colorKeyMask)>::type::value_type>(value) >= colorKeyMask[2 * k] &&
                                static_cast<std::decay<decltype(colorKeyMask)>::type::value_type>(value) <= colorKeyMask[2 * k + 1])
                            {
                                ++maskedColors;
                            }
                        }

                        QColor transformedColor = getColor(color, cms, intent, reporter, true);
                        QRgb rgb = transformedColor.rgb();

                        *outputLine++ = qRed(rgb);
                        *outputLine++ = qGreen(rgb);
                        *outputLine++ = qBlue(rgb);
                        *outputLine++ = (maskedColors == componentCount) ? 0x00 : 0xFF;
                    }
                }

                return image;
            }

            default:
            {
                throw PDFRendererException(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image masking not implemented!"));
            }
        }
    }

    return QImage();
}

void PDFAbstractColorSpace::fillRGBBuffer(const std::vector<float>& colors,
                                          unsigned char* outputBuffer,
                                          RenderingIntent intent,
                                          const PDFCMS* cms,
                                          PDFRenderErrorReporter* reporter) const
{
    // Generic solution
    size_t colorComponentCount = getColorComponentCount();
    size_t pixels = colors.size() / colorComponentCount;

    auto it = colors.cbegin();
    for (size_t i = 0; i < pixels; ++i)
    {
        PDFColor color;
        color.resize(colorComponentCount);
        for (size_t j = 0; j < colorComponentCount; ++j)
        {
            color[j] = *it++;
        }
        QColor transformedColor = getColor(color, cms, intent, reporter, true);
        QRgb rgb = transformedColor.rgb();

        *outputBuffer++ = qRed(rgb);
        *outputBuffer++ = qGreen(rgb);
        *outputBuffer++ = qBlue(rgb);
    }
}

QColor PDFAbstractColorSpace::getCheckedColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    if (getColorComponentCount() != color.size())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid number of color components. Expected number is %1, actual number is %2.").arg(static_cast<int>(getColorComponentCount())).arg(static_cast<int>(color.size())));
    }

    return getColor(color, cms, intent, reporter, true);
}

QImage PDFAbstractColorSpace::createAlphaMask(const PDFImageData& softMask)
{
    if (softMask.getMaskingType() != PDFImageData::MaskingType::None)
    {
        throw PDFException(PDFTranslationContext::tr("Soft mask can't have masking."));
    }

    if (softMask.getWidth() < 1 || softMask.getHeight() < 1)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid size of soft mask."));
    }

    QImage image(softMask.getWidth(), softMask.getHeight(), QImage::Format_Alpha8);

    unsigned int componentCount = softMask.getComponents();
    if (componentCount != 1)
    {
        throw PDFException(PDFTranslationContext::tr("Soft mask should have only 1 color component (alpha) instead of %1.").arg(componentCount));
    }

    const std::vector<PDFReal>& decode = softMask.getDecode();
    if (!decode.empty() && decode.size() != componentCount * 2)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid size of the decode array. Expected %1, actual %2.").arg(componentCount * 2).arg(decode.size()));
    }

    PDFBitReader reader(&softMask.getData(), softMask.getBitsPerComponent());

    PDFColor color;
    color.resize(componentCount);

    const double max = reader.max();
    const double coefficient = 1.0 / max;
    for (unsigned int i = 0, rowCount = softMask.getHeight(); i < rowCount; ++i)
    {
        reader.seek(i * softMask.getStride());
        unsigned char* outputLine = image.scanLine(i);

        for (unsigned int j = 0; j < softMask.getWidth(); ++j)
        {
            PDFReal alpha = 0.0;

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

            alpha = qBound(0.0, alpha, 1.0);
            uint8_t alphaCoded = alpha * 255;
            *outputLine++ = alphaCoded;
        }
    }

    return image;
}

PDFColorSpacePointer PDFAbstractColorSpace::createColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                             const PDFDocument* document,
                                                             const PDFObject& colorSpace)
{
    std::set<QByteArray> usedNames;
    return createColorSpaceImpl(colorSpaceDictionary, document, colorSpace, COLOR_SPACE_MAX_LEVEL_OF_RECURSION, usedNames);
}

PDFColorSpacePointer PDFAbstractColorSpace::createDeviceColorSpaceByName(const PDFDictionary* colorSpaceDictionary,
                                                                         const PDFDocument* document,
                                                                         const QByteArray& name)
{
    std::set<QByteArray> usedNames;
    return createDeviceColorSpaceByNameImpl(colorSpaceDictionary, document, name, COLOR_SPACE_MAX_LEVEL_OF_RECURSION, usedNames);
}

PDFColor PDFAbstractColorSpace::convertToColor(const std::vector<PDFReal>& components)
{
    PDFColor result;

    for (PDFReal component : components)
    {
        result.push_back(component);
    }

    return result;
}

bool PDFAbstractColorSpace::isColorEqual(const PDFColor& color1, const PDFColor& color2, PDFReal tolerance)
{
    const size_t size = color1.size();
    if (size != color2.size())
    {
        return false;
    }

    for (size_t i = 0; i < size; ++i)
    {
        if (std::fabs(color1[i] - color2[i]) > tolerance)
        {
            return false;
        }
    }

    return true;
}

PDFColor PDFAbstractColorSpace::mixColors(const PDFColor& color1, const PDFColor& color2, PDFReal ratio)
{
    const size_t size = color1.size();
    Q_ASSERT(size == color2.size());

    PDFColor result;
    result.resize(size);
    for (size_t i = 0; i < size; ++i)
    {
        result[i] = color1[i] * (1.0 - ratio) + color2[i] * ratio;
    }

    return result;
}

bool PDFAbstractColorSpace::transform(const PDFAbstractColorSpace* source,
                                      const PDFAbstractColorSpace* target,
                                      const PDFCMS* cms,
                                      RenderingIntent intent,
                                      const PDFColorBuffer input,
                                      PDFColorBuffer output,
                                      PDFRenderErrorReporter* reporter)
{
    Q_ASSERT(source);
    Q_ASSERT(target);
    Q_ASSERT(cms);
    Q_ASSERT(target->isBlendColorSpace());
    Q_ASSERT(input.size() % source->getColorComponentCount() == 0);

    const std::size_t sourceColors = input.size() / source->getColorComponentCount();
    const std::size_t targetColors = output.size() / target->getColorComponentCount();

    if (sourceColors != targetColors)
    {
        // This is bad input values. Function should not be called with these parameters.
        // Assert and return false. We do not want crash.
        Q_ASSERT(false);
        return false;
    }

    const std::size_t sourceColorsRemainder = input.size() % source->getColorComponentCount();
    const std::size_t targetColorsRemainder = output.size() % target->getColorComponentCount();

    if (sourceColorsRemainder > 0 || targetColorsRemainder > 0)
    {
        // Input/output buffer size is incorrect
        Q_ASSERT(false);
        return false;
    }

    if (source->equals(target))
    {
        // Just copy input buffer to output buffer
        Q_ASSERT(input.size() == output.size());
        std::copy(input.begin(), input.end(), output.begin());
        return true;
    }

    // We will use following algorithm to transform colors:
    //    1. Determine source color space type for cms,
    //       and adjust colors to this color space type
    //    2. Prepare work output buffer of target size (can have
    //       different size than real output buffer)
    //    3. Transform colors using CMS
    //    4. Transform output color buffer to target color buffer

    PDFCMS::ColorSpaceTransformParams params;
    std::vector<PDFColorComponent> transformedInputColorsVector;
    std::vector<PDFColorComponent> transformedOutputColorsVector;
    PDFColorBuffer transformedInput = input;
    PDFColorBuffer transformedOutput = output;
    params.intent = intent;

    switch (source->getColorSpace())
    {
        case ColorSpace::DeviceGray:
            params.sourceType = PDFCMS::ColorSpaceType::DeviceGray;
            break;

        case ColorSpace::DeviceRGB:
            params.sourceType = PDFCMS::ColorSpaceType::DeviceRGB;
            break;

        case ColorSpace::DeviceCMYK:
            params.sourceType = PDFCMS::ColorSpaceType::DeviceCMYK;
            break;

        case ColorSpace::CalGray:
        {
            params.sourceType = PDFCMS::ColorSpaceType::XYZ;

            // Transform gray to XYZ
            const PDFCalGrayColorSpace* calGrayColorSpace = static_cast<const PDFCalGrayColorSpace*>(source);
            const PDFColorComponent gamma = calGrayColorSpace->getGamma();

            transformedInputColorsVector.resize(input.size() * 3, 0.0f);
            auto it = transformedInputColorsVector.begin();
            for (PDFColorComponent gray : input)
            {
                const PDFColorComponent A = clip01(gray);
                const PDFColorComponent xyzColor = std::pow(A, gamma);

                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = xyzColor;
                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = xyzColor;
                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = xyzColor;
            }
            Q_ASSERT(it == transformedInputColorsVector.end());

            transformedInput = PDFColorBuffer(transformedInputColorsVector.data(), transformedInputColorsVector.size());
            break;
        }

        case ColorSpace::CalRGB:
        {
            params.sourceType = PDFCMS::ColorSpaceType::XYZ;

            const PDFCalRGBColorSpace* calRGBColorSpace = static_cast<const PDFCalRGBColorSpace*>(source);
            const PDFColor3 gamma = calRGBColorSpace->getGamma();
            const PDFColorComponentMatrix_3x3 matrix = calRGBColorSpace->getMatrix();

            transformedInputColorsVector.resize(input.size(), 0.0f);
            auto it = transformedInputColorsVector.begin();

            for (auto sourceIt = input.cbegin(); sourceIt != input.cend(); sourceIt = std::next(sourceIt, 3))
            {
                PDFColor3 ABC = { };
                Q_ASSERT(sourceIt != input.end());
                ABC[0] = *sourceIt;
                Q_ASSERT(sourceIt + 1 != input.end());
                ABC[1] = *std::next(sourceIt, 1);
                Q_ASSERT(sourceIt + 2 != input.end());
                ABC[2] = *std::next(sourceIt, 2);
                ABC = colorPowerByFactors(ABC, gamma);

                const PDFColor3 XYZ = matrix * ABC;

                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = XYZ[0];
                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = XYZ[1];
                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = XYZ[2];
            }
            Q_ASSERT(it == transformedInputColorsVector.end());

            transformedInput = PDFColorBuffer(transformedInputColorsVector.data(), transformedInputColorsVector.size());
            break;
        }

        case ColorSpace::Lab:
        {
            params.sourceType = PDFCMS::ColorSpaceType::XYZ;

            const PDFLabColorSpace* labColorSpace = static_cast<const PDFLabColorSpace*>(source);
            const PDFColorComponent aMin = labColorSpace->getAMin();
            const PDFColorComponent aMax = labColorSpace->getAMax();
            const PDFColorComponent bMin = labColorSpace->getBMin();
            const PDFColorComponent bMax = labColorSpace->getBMax();

            transformedInputColorsVector.resize(input.size(), 0.0f);
            auto it = transformedInputColorsVector.begin();

            for (auto sourceIt = input.cbegin(); sourceIt != input.cend(); sourceIt = std::next(sourceIt, 3))
            {
                Q_ASSERT(sourceIt != input.end());
                Q_ASSERT(sourceIt + 1 != input.end());
                Q_ASSERT(sourceIt + 2 != input.end());

                PDFColorComponent LStar = qBound<PDFColorComponent>(PDFColorComponent(0.0), interpolate(*sourceIt, 0.0, 1.0, 0.0, 100.0), PDFColorComponent(100.0));
                PDFColorComponent aStar = qBound<PDFColorComponent>(aMin, interpolate(*std::next(sourceIt, 1), 0.0, 1.0, aMin, aMax), aMax);
                PDFColorComponent bStar = qBound<PDFColorComponent>(bMin, interpolate(*std::next(sourceIt, 2), 0.0, 1.0, bMin, bMax), bMax);

                const PDFColorComponent param1 = (LStar + 16.0f) / 116.0f;
                const PDFColorComponent param2 = aStar / 500.0f;
                const PDFColorComponent param3 = bStar / 200.0f;

                const PDFColorComponent L = param1 + param2;
                const PDFColorComponent M = param1;
                const PDFColorComponent N = param1 - param3;

                auto g = [](PDFColorComponent x) -> PDFColorComponent
                {
                    if (x >= 6.0f / 29.0f)
                    {
                        return x * x * x;
                    }
                    else
                    {
                        return (108.0f / 841.0f) * (x - 4.0f / 29.0f);
                    }
                };

                const PDFColorComponent gL = g(L);
                const PDFColorComponent gM = g(M);
                const PDFColorComponent gN = g(N);

                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = gL;
                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = gM;
                Q_ASSERT(it != transformedInputColorsVector.end());
                *it++ = gN;
            }
            Q_ASSERT(it == transformedInputColorsVector.end());

            transformedInput = PDFColorBuffer(transformedInputColorsVector.data(), transformedInputColorsVector.size());
            break;
        }

        case ColorSpace::ICCBased:
        {
            const PDFICCBasedColorSpace* iccBasedColorSpace = static_cast<const PDFICCBasedColorSpace*>(source);

            params.sourceType = PDFCMS::ColorSpaceType::ICC;
            params.sourceIccId = iccBasedColorSpace->getIccProfileDataChecksum();
            params.sourceIccData = iccBasedColorSpace->getIccProfileData();

            size_t colorComponentCount = iccBasedColorSpace->getColorComponentCount();
            const PDFICCBasedColorSpace::Ranges& ranges = iccBasedColorSpace->getRange();

            transformedInputColorsVector.resize(input.size(), 0.0f);
            auto outputIt = transformedInputColorsVector.begin();
            for (auto inputIt = input.cbegin(); inputIt != input.cend();)
            {
                for (size_t i = 0; i < colorComponentCount; ++i)
                {
                    const size_t imin = 2 * i + 0;
                    const size_t imax = 2 * i + 1;
                    *outputIt++ = qBound(ranges[imin], *inputIt++, ranges[imax]);
                }
            }

            transformedInput = PDFColorBuffer(transformedInputColorsVector.data(), transformedInputColorsVector.size());
            break;
        }

        case ColorSpace::Indexed:
        {
            const PDFIndexedColorSpace* indexedColorSpace = static_cast<const PDFIndexedColorSpace*>(source);

            PDFColorSpacePointer baseColorSpace = indexedColorSpace->getBaseColorSpace();
            std::vector<PDFColorComponent> transformedToBaseColorSpaceInput = indexedColorSpace->transformColorsToBaseColorSpace(input);
            PDFColorBuffer transformedInputBuffer(transformedToBaseColorSpaceInput.data(), transformedToBaseColorSpaceInput.size());
            return transform(baseColorSpace.data(), target, cms, intent, transformedInputBuffer, output, reporter);
        }

        case ColorSpace::Separation:
        {
            const PDFSeparationColorSpace* separationColorSpace = static_cast<const PDFSeparationColorSpace*>(source);

            PDFColorSpacePointer alternateColorSpace = separationColorSpace->getAlternateColorSpace();
            std::vector<PDFColorComponent> transformedToBaseColorSpaceInput = separationColorSpace->transformColorsToBaseColorSpace(input);
            PDFColorBuffer transformedInputBuffer(transformedToBaseColorSpaceInput.data(), transformedToBaseColorSpaceInput.size());
            return transform(alternateColorSpace.data(), target, cms, intent, transformedInputBuffer, output, reporter);
        }

        case ColorSpace::DeviceN:
        {
            const PDFDeviceNColorSpace* separationColorSpace = static_cast<const PDFDeviceNColorSpace*>(source);

            PDFColorSpacePointer alternateColorSpace = separationColorSpace->getAlternateColorSpace();
            std::vector<PDFColorComponent> transformedToBaseColorSpaceInput = separationColorSpace->transformColorsToBaseColorSpace(input);
            PDFColorBuffer transformedInputBuffer(transformedToBaseColorSpaceInput.data(), transformedToBaseColorSpaceInput.size());
            return transform(alternateColorSpace.data(), target, cms, intent, transformedInputBuffer, output, reporter);
        }

        case ColorSpace::Pattern:
            return false;

        default:
            Q_ASSERT(false);
            return false;
    }

    switch (target->getColorSpace())
    {
        case ColorSpace::DeviceGray:
            // Output buffer size is the same as target type
            params.targetType = PDFCMS::ColorSpaceType::DeviceGray;
            break;

        case ColorSpace::DeviceRGB:
            // Output buffer size is the same as target type
            params.targetType = PDFCMS::ColorSpaceType::DeviceRGB;
            break;

        case ColorSpace::DeviceCMYK:
            // Output buffer size is the same as target type
            params.targetType = PDFCMS::ColorSpaceType::DeviceCMYK;
            break;

        case pdf::PDFAbstractColorSpace::ColorSpace::CalGray:
        {
            params.targetType = PDFCMS::ColorSpaceType::XYZ;
            transformedOutputColorsVector.resize(output.size() * 3, 0.0f);
            transformedOutput = PDFColorBuffer(transformedOutputColorsVector.data(), transformedOutputColorsVector.size());
            break;
        }

        case pdf::PDFAbstractColorSpace::ColorSpace::CalRGB:
        {
            // Output buffer size is the same as target type
            params.targetType = PDFCMS::ColorSpaceType::XYZ;
            transformedOutputColorsVector.resize(output.size(), 0.0f);
            transformedOutput = PDFColorBuffer(transformedOutputColorsVector.data(), transformedOutputColorsVector.size());
            break;
        }

        case pdf::PDFAbstractColorSpace::ColorSpace::ICCBased:
        {
            const PDFICCBasedColorSpace* iccBasedColorSpace = static_cast<const PDFICCBasedColorSpace*>(target);

            params.targetType = PDFCMS::ColorSpaceType::ICC;
            params.targetIccId = iccBasedColorSpace->getIccProfileDataChecksum();
            params.targetIccData = iccBasedColorSpace->getIccProfileData();
            break;
        }

        default:
            Q_ASSERT(false);
            return false;
    }

    params.input = transformedInput;
    params.output = transformedOutput;

    cms->transformColorSpace(params);

    switch (target->getColorSpace())
    {
        case pdf::PDFAbstractColorSpace::ColorSpace::CalGray:
        {
            const PDFCalGrayColorSpace* calGrayColorSpace = static_cast<const PDFCalGrayColorSpace*>(source);
            const PDFColorComponent gamma = 1.0 / calGrayColorSpace->getGamma();

            auto outputIt = output.begin();
            for (auto transformedOutputIt = transformedOutput.cbegin(); transformedOutputIt != transformedOutput.cend(); transformedOutputIt = std::next(transformedOutputIt, 3))
            {
                PDFColor3 XYZ = { };
                Q_ASSERT(transformedOutputIt != transformedOutput.end());
                XYZ[0] = *transformedOutputIt;
                Q_ASSERT(transformedOutputIt + 1 != transformedOutput.end());
                XYZ[1] = *std::next(transformedOutputIt, 1);
                Q_ASSERT(transformedOutputIt + 2 != transformedOutput.end());
                XYZ[2] = *std::next(transformedOutputIt, 2);

                const PDFColorComponent gray = (XYZ[0] + XYZ[1] + XYZ[2]) * 0.333333333333333;
                const PDFColorComponent grayWithGamma = std::pow(gray, gamma);
                Q_ASSERT(outputIt != output.cend());
                *outputIt++ = grayWithGamma;
            }
            Q_ASSERT(outputIt == output.cend());
            break;
        }

        case pdf::PDFAbstractColorSpace::ColorSpace::CalRGB:
        {
            const PDFCalRGBColorSpace* calRGBColorSpace = static_cast<const PDFCalRGBColorSpace*>(source);
            const PDFColor3 gammaForward = calRGBColorSpace->getGamma();
            const PDFColorComponentMatrix_3x3 matrixForward = calRGBColorSpace->getMatrix();
            const PDFColor3 gammaInverse = PDFColor3{ 1.0f / gammaForward[0], 1.0f / gammaForward[1], 1.0f / gammaForward[2] };
            const PDFColorComponentMatrix_3x3 matrixInverse = getInverseMatrix(matrixForward);

            auto outputIt = output.begin();
            for (auto transformedOutputIt = transformedOutput.cbegin(); transformedOutputIt != transformedOutput.cend(); transformedOutputIt = std::next(transformedOutputIt, 3))
            {
                PDFColor3 XYZ = { };
                XYZ[0] = *transformedOutputIt;
                XYZ[1] = *std::next(transformedOutputIt, 1);
                XYZ[2] = *std::next(transformedOutputIt, 2);

                const PDFColor3 RGB = matrixInverse * XYZ;
                const PDFColor3 RGBwithGamma = colorPowerByFactors(RGB, gammaInverse);

                Q_ASSERT(outputIt != output.end());
                *outputIt++ = RGBwithGamma[0];
                Q_ASSERT(outputIt != output.end());
                *outputIt++ = RGBwithGamma[1];
                Q_ASSERT(outputIt != output.end());
                *outputIt++ = RGBwithGamma[2];
            }
            Q_ASSERT(outputIt == output.cend());
            break;
        }

        default:
            break;
    }

    return true;
}

PDFColorSpacePointer PDFAbstractColorSpace::createColorSpaceImpl(const PDFDictionary* colorSpaceDictionary,
                                                                 const PDFDocument* document,
                                                                 const PDFObject& colorSpace,
                                                                 int recursion,
                                                                 std::set<QByteArray>& usedNames)
{
    if (--recursion <= 0)
    {
        throw PDFException(PDFTranslationContext::tr("Can't load color space, because color space structure is too complex."));
    }

    if (colorSpace.isName())
    {
        return createDeviceColorSpaceByNameImpl(colorSpaceDictionary, document, colorSpace.getString(), recursion, usedNames);
    }
    else if (colorSpace.isArray())
    {
        // First value of the array should be identification name, second value dictionary with parameters
        const PDFArray* array = colorSpace.getArray();
        size_t count = array->getCount();

        if (count > 0)
        {
            // Name of the color space
            const PDFObject& colorSpaceIdentifier = document->getObject(array->getItem(0));
            if (colorSpaceIdentifier.isName())
            {
                QByteArray name = colorSpaceIdentifier.getString();

                const PDFDictionary* dictionary = nullptr;
                const PDFStream* stream = nullptr;
                if (count > 1)
                {
                    const PDFObject& colorSpaceSettings = document->getObject(array->getItem(1));
                    if (colorSpaceSettings.isDictionary())
                    {
                        dictionary = colorSpaceSettings.getDictionary();
                    }
                    if (colorSpaceSettings.isStream())
                    {
                        stream = colorSpaceSettings.getStream();
                    }
                }

                if (name == COLOR_SPACE_NAME_PATTERN)
                {
                    PDFColorSpacePointer uncoloredPatternColorSpace;
                    if (count == 2)
                    {
                        uncoloredPatternColorSpace = createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(array->getItem(1)), recursion, usedNames);
                    }

                    return PDFColorSpacePointer(new PDFPatternColorSpace(std::make_shared<PDFInvalidPattern>(), qMove(uncoloredPatternColorSpace), PDFColor()));
                }

                if (dictionary)
                {
                    if (name == COLOR_SPACE_NAME_CAL_GRAY)
                    {
                        return PDFCalGrayColorSpace::createCalGrayColorSpace(document, dictionary);
                    }
                    else if (name == COLOR_SPACE_NAME_CAL_RGB)
                    {
                        return PDFCalRGBColorSpace::createCalRGBColorSpace(document, dictionary);
                    }
                    else if (name == COLOR_SPACE_NAME_LAB)
                    {
                        return PDFLabColorSpace::createLabColorSpace(document, dictionary);
                    }
                }

                if (stream && name == COLOR_SPACE_NAME_ICCBASED)
                {
                    return PDFICCBasedColorSpace::createICCBasedColorSpace(colorSpaceDictionary, document, stream, recursion, usedNames);
                }

                if (name == COLOR_SPACE_NAME_INDEXED && count == 4)
                {
                    return PDFIndexedColorSpace::createIndexedColorSpace(colorSpaceDictionary, document, array, recursion, usedNames);
                }

                if (name == COLOR_SPACE_NAME_SEPARATION && count == 4)
                {
                    return PDFSeparationColorSpace::createSeparationColorSpace(colorSpaceDictionary, document, array, recursion, usedNames);
                }

                if (name == COLOR_SPACE_NAME_DEVICE_N && count >= 4)
                {
                    return PDFDeviceNColorSpace::createDeviceNColorSpace(colorSpaceDictionary, document, array, recursion, usedNames);
                }

                // Try to just load by standard way - we can have "standard" color space stored in array
                return createColorSpaceImpl(colorSpaceDictionary, document, colorSpaceIdentifier, recursion, usedNames);
            }
        }
    }

    throw PDFException(PDFTranslationContext::tr("Invalid color space."));
}

PDFColorSpacePointer PDFAbstractColorSpace::createDeviceColorSpaceByNameImpl(const PDFDictionary* colorSpaceDictionary,
                                                                             const PDFDocument* document,
                                                                             const QByteArray& name,
                                                                             int recursion,
                                                                             std::set<QByteArray>& usedNames)
{
    if (--recursion <= 0)
    {
        throw PDFException(PDFTranslationContext::tr("Can't load color space, because color space structure is too complex."));
    }

    // Jakub Melka: This flag is set  when we are already parsing the name. This can occur
    // for example by this way: we have DefaultRGB, which is ICC profile. In this ICC profile,
    // we create alternate alternate RGB color space also from DefaultRGB name. But in this time,
    // we must use generic RGB color space, not DefaultRGB from color space dictionary, because
    // it will be cyclical dependency.
    bool isNameAlreadyProcessed = usedNames.count(name);
    usedNames.insert(name);

    if (name == COLOR_SPACE_NAME_PATTERN)
    {
        return PDFColorSpacePointer(new PDFPatternColorSpace(std::make_shared<PDFInvalidPattern>(), nullptr, PDFColor()));
    }

    if (name == COLOR_SPACE_NAME_DEVICE_GRAY || name == COLOR_SPACE_NAME_ABBREVIATION_DEVICE_GRAY)
    {
        if (colorSpaceDictionary && colorSpaceDictionary->hasKey(COLOR_SPACE_NAME_DEFAULT_GRAY) && !isNameAlreadyProcessed)
        {
            return createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(colorSpaceDictionary->get(COLOR_SPACE_NAME_DEFAULT_GRAY)), recursion, usedNames);
        }
        else
        {
            return PDFColorSpacePointer(new PDFDeviceGrayColorSpace());
        }
    }
    else if (name == COLOR_SPACE_NAME_DEVICE_RGB || name == COLOR_SPACE_NAME_ABBREVIATION_DEVICE_RGB)
    {
        if (colorSpaceDictionary && colorSpaceDictionary->hasKey(COLOR_SPACE_NAME_DEFAULT_RGB) && !isNameAlreadyProcessed)
        {
            return createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(colorSpaceDictionary->get(COLOR_SPACE_NAME_DEFAULT_RGB)), recursion, usedNames);
        }
        else
        {
            return PDFColorSpacePointer(new PDFDeviceRGBColorSpace());
        }
    }
    else if (name == COLOR_SPACE_NAME_DEVICE_CMYK || name == COLOR_SPACE_NAME_ABBREVIATION_DEVICE_CMYK || name == COLOR_SPACE_NAME_ABBREVIATION_CAL_CMYK)
    {
        if (colorSpaceDictionary && colorSpaceDictionary->hasKey(COLOR_SPACE_NAME_DEFAULT_CMYK) && !isNameAlreadyProcessed)
        {
            return createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(colorSpaceDictionary->get(COLOR_SPACE_NAME_DEFAULT_CMYK)), recursion, usedNames);
        }
        else
        {
            return PDFColorSpacePointer(new PDFDeviceCMYKColorSpace());
        }
    }
    else if (colorSpaceDictionary && colorSpaceDictionary->hasKey(name))
    {
        return createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(colorSpaceDictionary->get(name)), recursion, usedNames);
    }

    throw PDFException(PDFTranslationContext::tr("Invalid color space."));
}

/// Conversion matrix from XYZ space to RGB space. Values are taken from this article:
/// https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_.28.22gamma.22.29
static constexpr const PDFColorComponentMatrix<3, 3> matrixXYZtoRGB(
     3.2406f, -1.5372f, -0.4986f,
    -0.9689f,  1.8758f,  0.0415f,
     0.0557f, -0.2040f,  1.0570f
);

PDFColor3 PDFAbstractColorSpace::convertXYZtoRGB(const PDFColor3& xyzColor)
{
    return matrixXYZtoRGB * xyzColor;
}

PDFColor PDFXYZColorSpace::getDefaultColorOriginal() const
{
    PDFColor color;
    const size_t componentCount = getColorComponentCount();
    for (size_t i = 0; i < componentCount; ++i)
    {
        color.push_back(0.0f);
    }
    return color;
}

bool PDFXYZColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFAbstractColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFXYZColorSpace*>(other));
    const PDFXYZColorSpace* typedOther = static_cast<const PDFXYZColorSpace*>(other);
    return m_whitePoint == typedOther->getWhitePoint() && m_correctionCoefficients == typedOther->getCorrectionCoefficients();
}

PDFXYZColorSpace::PDFXYZColorSpace(PDFColor3 whitePoint) :
    m_whitePoint(whitePoint),
    m_correctionCoefficients()
{
    PDFColor3 mappedWhitePoint = convertXYZtoRGB(m_whitePoint);

    m_correctionCoefficients[0] = 1.0f / mappedWhitePoint[0];
    m_correctionCoefficients[1] = 1.0f / mappedWhitePoint[1];
    m_correctionCoefficients[2] = 1.0f / mappedWhitePoint[2];
}

const PDFColor3& PDFXYZColorSpace::getCorrectionCoefficients() const
{
    return m_correctionCoefficients;
}

PDFCalGrayColorSpace::PDFCalGrayColorSpace(PDFColor3 whitePoint, PDFColor3 blackPoint, PDFColorComponent gamma) :
    PDFXYZColorSpace(whitePoint),
    m_blackPoint(blackPoint),
    m_gamma(gamma)
{

}

bool PDFCalGrayColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFXYZColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFCalGrayColorSpace*>(other));
    const PDFCalGrayColorSpace* typedOther = static_cast<const PDFCalGrayColorSpace*>(other);
    return m_blackPoint == typedOther->getBlackPoint() && m_gamma == typedOther->getGamma();
}

QColor PDFCalGrayColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(color.size() == getColorComponentCount());
    Q_UNUSED(isRange01);

    const PDFColorComponent A = clip01(color[0]);
    const PDFColorComponent xyzColor = std::pow(A, m_gamma);

    PDFColor3 xyzColorCMS = { xyzColor, xyzColor, xyzColor };
    QColor cmsColor = cms->getColorFromXYZ(m_whitePoint, xyzColorCMS, intent, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    const PDFColor3 xyzColorMultipliedByWhitePoint = colorMultiplyByFactor(m_whitePoint, xyzColor);
    const PDFColor3 rgb = convertXYZtoRGB(xyzColorMultipliedByWhitePoint);
    const PDFColor3 calibratedRGB = colorMultiplyByFactors(rgb, m_correctionCoefficients);
    return fromRGB01(calibratedRGB);
}

size_t PDFCalGrayColorSpace::getColorComponentCount() const
{
    return 1;
}

void PDFCalGrayColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    std::vector<float> xyzColors(colors.size() * 3, 0.0f);
    auto it = xyzColors.begin();
    for (float gray : colors)
    {
        const PDFColorComponent xyzColor = std::pow(clip01(gray), m_gamma);
        *it++ = xyzColor;
        *it++ = xyzColor;
        *it++ = xyzColor;
    }

    Q_ASSERT(xyzColors.size() == colors.size() * 3);
    if (!cms->fillRGBBufferFromXYZ(m_whitePoint, xyzColors, intent, outputBuffer, reporter))
    {
        PDFAbstractColorSpace::fillRGBBuffer(colors, outputBuffer, intent, cms, reporter);
    }
}

PDFColorSpacePointer PDFCalGrayColorSpace::createCalGrayColorSpace(const PDFDocument* document, const PDFDictionary* dictionary)
{
    // Standard D65 white point
    PDFColor3 whitePoint = { 0.9505f, 1.0000f, 1.0890f };
    PDFColor3 blackPoint = { 0, 0, 0 };
    PDFColorComponent gamma = 1.0f;

    PDFDocumentDataLoaderDecorator loader(document);
    loader.readNumberArrayFromDictionary(dictionary, CAL_WHITE_POINT, whitePoint.begin(), whitePoint.end());
    loader.readNumberArrayFromDictionary(dictionary, CAL_BLACK_POINT, blackPoint.begin(), blackPoint.end());
    gamma = loader.readNumberFromDictionary(dictionary, CAL_GAMMA, gamma);

    return PDFColorSpacePointer(new PDFCalGrayColorSpace(whitePoint, blackPoint, gamma));
}

PDFCalRGBColorSpace::PDFCalRGBColorSpace(PDFColor3 whitePoint, PDFColor3 blackPoint, PDFColor3 gamma, PDFColorComponentMatrix_3x3 matrix) :
    PDFXYZColorSpace(whitePoint),
    m_blackPoint(blackPoint),
    m_gamma(gamma),
    m_matrix(matrix)
{

}

bool PDFCalRGBColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFXYZColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFCalRGBColorSpace*>(other));
    const PDFCalRGBColorSpace* typedOther = static_cast<const PDFCalRGBColorSpace*>(other);
    return m_blackPoint == typedOther->getBlackPoint() && m_gamma == typedOther->getGamma() && m_matrix == typedOther->getMatrix();
}

QColor PDFCalRGBColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(color.size() == getColorComponentCount());
    Q_UNUSED(isRange01);

    const PDFColor3 ABC = clip01(PDFColor3{ color[0], color[1], color[2] });
    const PDFColor3 ABCwithGamma = colorPowerByFactors(ABC, m_gamma);
    const PDFColor3 XYZ = m_matrix * ABCwithGamma;

    QColor cmsColor = cms->getColorFromXYZ(m_whitePoint, XYZ, intent, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    const PDFColor3 rgb = convertXYZtoRGB(XYZ);
    const PDFColor3 calibratedRGB = colorMultiplyByFactors(rgb, m_correctionCoefficients);
    return fromRGB01(calibratedRGB);
}

size_t PDFCalRGBColorSpace::getColorComponentCount() const
{
    return 3;
}

void PDFCalRGBColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    std::vector<float> xyzColors(colors.size(), 0.0f);
    auto it = xyzColors.begin();
    for (size_t i = 0; i < colors.size(); i += 3)
    {
        Q_ASSERT(i + 2 < colors.size());
        const PDFColor3 ABC = clip01(PDFColor3{ colors[i + 0], colors[i + 1], colors[i + 2] });
        const PDFColor3 ABCwithGamma = colorPowerByFactors(ABC, m_gamma);
        const PDFColor3 XYZ = m_matrix * ABCwithGamma;

        *it++ = XYZ[0];
        *it++ = XYZ[1];
        *it++ = XYZ[2];
    }

    Q_ASSERT(xyzColors.size() == colors.size());
    if (!cms->fillRGBBufferFromXYZ(m_whitePoint, xyzColors, intent, outputBuffer, reporter))
    {
        PDFAbstractColorSpace::fillRGBBuffer(colors, outputBuffer, intent, cms, reporter);
    }
}

PDFColorSpacePointer PDFCalRGBColorSpace::createCalRGBColorSpace(const PDFDocument* document, const PDFDictionary* dictionary)
{
    // Standard D65 white point
    PDFColor3 whitePoint = { 0.9505f, 1.0000f, 1.0890f };
    PDFColor3 blackPoint = { 0, 0, 0 };
    PDFColor3 gamma = { 1.0f, 1.0f, 1.0f };
    PDFColorComponentMatrix_3x3 matrix( 1, 0, 0,
                                        0, 1, 0,
                                        0, 0, 1 );

    PDFDocumentDataLoaderDecorator loader(document);
    loader.readNumberArrayFromDictionary(dictionary, CAL_WHITE_POINT, whitePoint.begin(), whitePoint.end());
    loader.readNumberArrayFromDictionary(dictionary, CAL_BLACK_POINT, blackPoint.begin(), blackPoint.end());
    loader.readNumberArrayFromDictionary(dictionary, CAL_GAMMA, gamma.begin(), gamma.end());
    loader.readNumberArrayFromDictionary(dictionary, CAL_MATRIX, matrix.begin(), matrix.end());

    matrix.transpose();

    return PDFColorSpacePointer(new PDFCalRGBColorSpace(whitePoint, blackPoint, gamma, matrix));
}

PDFColor3 PDFCalRGBColorSpace::getBlackPoint() const
{
    return m_blackPoint;
}

PDFColor3 PDFCalRGBColorSpace::getGamma() const
{
    return m_gamma;
}

const PDFColorComponentMatrix_3x3& PDFCalRGBColorSpace::getMatrix() const
{
    return m_matrix;
}

PDFLabColorSpace::PDFLabColorSpace(PDFColor3 whitePoint,
                                   PDFColor3 blackPoint,
                                   PDFColorComponent aMin,
                                   PDFColorComponent aMax,
                                   PDFColorComponent bMin,
                                   PDFColorComponent bMax) :
    PDFXYZColorSpace(whitePoint),
    m_blackPoint(blackPoint),
    m_aMin(aMin),
    m_aMax(aMax),
    m_bMin(bMin),
    m_bMax(bMax)
{

}

bool PDFLabColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFXYZColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFLabColorSpace*>(other));
    const PDFLabColorSpace* typedOther = static_cast<const PDFLabColorSpace*>(other);
    return m_blackPoint == typedOther->getBlackPoint() &&
           m_aMin == typedOther->getAMin() && m_aMax == typedOther->getAMax() &&
           m_bMin == typedOther->getBMin() && m_bMax == typedOther->getBMax();
}

QColor PDFLabColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(color.size() == getColorComponentCount());

    PDFColorComponent LStar = 0;
    PDFColorComponent aStar = 0;
    PDFColorComponent bStar = 0;

    if (isRange01)
    {
        LStar = qBound<PDFColorComponent>(PDFColorComponent(0.0), interpolate(color[0], 0.0, 1.0, 0.0, 100.0), PDFColorComponent(100.0));
        aStar = qBound<PDFColorComponent>(m_aMin, interpolate(color[1], 0.0, 1.0, m_aMin, m_aMax), m_aMax);
        bStar = qBound<PDFColorComponent>(m_bMin, interpolate(color[2], 0.0, 1.0, m_bMin, m_bMax), m_bMax);
    }
    else
    {
        LStar = qBound<PDFColorComponent>(PDFColorComponent(0.0), color[0], PDFColorComponent(100.0));
        aStar = qBound<PDFColorComponent>(m_aMin, color[1], m_aMax);
        bStar = qBound<PDFColorComponent>(m_bMin, color[2], m_bMax);
    }

    const PDFColorComponent param1 = (LStar + 16.0f) / 116.0f;
    const PDFColorComponent param2 = aStar / 500.0f;
    const PDFColorComponent param3 = bStar / 200.0f;

    const PDFColorComponent L = param1 + param2;
    const PDFColorComponent M = param1;
    const PDFColorComponent N = param1 - param3;

    auto g = [](PDFColorComponent x) -> PDFColorComponent
    {
        if (x >= 6.0f / 29.0f)
        {
            return x * x * x;
        }
        else
        {
            return (108.0f / 841.0f) * (x - 4.0f / 29.0f);
        }
    };

    const PDFColorComponent gL = g(L);
    const PDFColorComponent gM = g(M);
    const PDFColorComponent gN = g(N);
    const PDFColor3 gLMN = { gL, gM, gN };

    QColor cmsColor = cms->getColorFromXYZ(m_whitePoint, gLMN, intent, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    const PDFColor3 XYZ = colorMultiplyByFactors(m_whitePoint, gLMN);
    const PDFColor3 rgb = convertXYZtoRGB(XYZ);
    const PDFColor3 calibratedRGB = colorMultiplyByFactors(rgb, m_correctionCoefficients);
    return fromRGB01(calibratedRGB);
}

size_t PDFLabColorSpace::getColorComponentCount() const
{
    return 3;
}

void PDFLabColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    auto g = [](PDFColorComponent x) -> PDFColorComponent
    {
        if (x >= 6.0f / 29.0f)
        {
            return x * x * x;
        }
        else
        {
            return (108.0f / 841.0f) * (x - 4.0f / 29.0f);
        }
    };

    std::vector<float> xyzColors(colors.size(), 0.0f);
    auto it = xyzColors.begin();
    for (size_t i = 0; i < colors.size(); i += 3)
    {
        Q_ASSERT(i + 2 < colors.size());

        const PDFColorComponent LStar = qBound(0.0, interpolate(colors[i + 0], 0.0, 1.0, 0.0, 100.0), 100.0);
        const PDFColorComponent aStar = qBound<PDFColorComponent>(m_aMin, interpolate(colors[i + 1], 0.0, 1.0, m_aMin, m_aMax), m_aMax);
        const PDFColorComponent bStar = qBound<PDFColorComponent>(m_bMin, interpolate(colors[i + 2], 0.0, 1.0, m_bMin, m_bMax), m_bMax);

        const PDFColorComponent param1 = (LStar + 16.0f) / 116.0f;
        const PDFColorComponent param2 = aStar / 500.0f;
        const PDFColorComponent param3 = bStar / 200.0f;

        const PDFColorComponent L = param1 + param2;
        const PDFColorComponent M = param1;
        const PDFColorComponent N = param1 - param3;

        *it++ = g(L);
        *it++ = g(M);
        *it++ = g(N);
    }

    Q_ASSERT(xyzColors.size() == colors.size());
    if (!cms->fillRGBBufferFromXYZ(m_whitePoint, xyzColors, intent, outputBuffer, reporter))
    {
        PDFAbstractColorSpace::fillRGBBuffer(colors, outputBuffer, intent, cms, reporter);
    }
}

PDFColorSpacePointer PDFLabColorSpace::createLabColorSpace(const PDFDocument* document, const PDFDictionary* dictionary)
{
    // Standard D65 white point
    PDFColor3 whitePoint = { 0.9505f, 1.0000f, 1.0890f };
    PDFColor3 blackPoint = { 0, 0, 0 };

    static_assert(std::numeric_limits<PDFColorComponent>::has_infinity, "Fix this code!");
    const PDFColorComponent infPos = std::numeric_limits<PDFColorComponent>::infinity();
    const PDFColorComponent infNeg = -std::numeric_limits<PDFColorComponent>::infinity();
    std::array<PDFColorComponent, 4> minMax = { infNeg, infPos, infNeg, infPos };

    PDFDocumentDataLoaderDecorator loader(document);
    loader.readNumberArrayFromDictionary(dictionary, CAL_WHITE_POINT, whitePoint.begin(), whitePoint.end());
    loader.readNumberArrayFromDictionary(dictionary, CAL_BLACK_POINT, blackPoint.begin(), blackPoint.end());
    loader.readNumberArrayFromDictionary(dictionary, CAL_RANGE, minMax.begin(), minMax.end());

    return PDFColorSpacePointer(new PDFLabColorSpace(whitePoint, blackPoint, minMax[0], minMax[1], minMax[2], minMax[3]));
}

PDFColorComponent PDFLabColorSpace::getAMin() const
{
    return m_aMin;
}

PDFColorComponent PDFLabColorSpace::getAMax() const
{
    return m_aMax;
}

PDFColorComponent PDFLabColorSpace::getBMin() const
{
    return m_bMin;
}

PDFColorComponent PDFLabColorSpace::getBMax() const
{
    return m_bMax;
}

PDFColor3 PDFLabColorSpace::getBlackPoint() const
{
    return m_blackPoint;
}

PDFICCBasedColorSpace::PDFICCBasedColorSpace(PDFColorSpacePointer alternateColorSpace, Ranges range, QByteArray iccProfileData, PDFObjectReference metadata) :
    m_alternateColorSpace(qMove(alternateColorSpace)),
    m_range(range),
    m_iccProfileData(qMove(iccProfileData)),
    m_metadata(metadata)
{
    // Compute checksum
    m_iccProfileDataChecksum = QCryptographicHash::hash(m_iccProfileData, QCryptographicHash::Md5);
}

PDFColor PDFICCBasedColorSpace::getDefaultColorOriginal() const
{
    PDFColor color;
    const size_t componentCount = getColorComponentCount();
    for (size_t i = 0; i < componentCount; ++i)
    {
        color.push_back(0.0f);
    }
    return color;
}

QColor PDFICCBasedColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_ASSERT(color.size() == getColorComponentCount());
    Q_UNUSED(isRange01);

    size_t colorComponentCount = getColorComponentCount();

    // Clip color values by range
    PDFColor clippedColor = color;
    for (size_t i = 0; i < colorComponentCount; ++i)
    {
        const size_t imin = 2 * i + 0;
        const size_t imax = 2 * i + 1;
        clippedColor[i] = qBound(m_range[imin], clippedColor[i], m_range[imax]);
    }

    QColor cmsColor = cms->getColorFromICC(clippedColor, intent, m_iccProfileDataChecksum, m_iccProfileData, reporter);
    if (cmsColor.isValid())
    {
        return cmsColor;
    }

    return m_alternateColorSpace->getColor(clippedColor, cms, intent, reporter, true);
}

size_t PDFICCBasedColorSpace::getColorComponentCount() const
{
    return m_alternateColorSpace->getColorComponentCount();
}

void PDFICCBasedColorSpace::fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const
{
    size_t colorComponentCount = getColorComponentCount();
    std::vector<float> clippedColors(colors.size(), 0.0f);
    for (size_t i = 0, colorCount = colors.size(); i < colorCount; ++i)
    {
        const size_t componentIndex = i % colorComponentCount;
        const size_t imin = 2 * componentIndex + 0;
        const size_t imax = 2 * componentIndex + 1;
        clippedColors[i] = qBound(m_range[imin], colors[i], m_range[imax]);
    }

    if (!cms->fillRGBBufferFromICC(clippedColors, intent, outputBuffer, m_iccProfileDataChecksum, m_iccProfileData, reporter))
    {
        // Try to fill buffer from alternate color space
        m_alternateColorSpace->fillRGBBuffer(clippedColors, outputBuffer, intent, cms, reporter);
    }
}

bool PDFICCBasedColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFAbstractColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFICCBasedColorSpace*>(other));
    const PDFICCBasedColorSpace* typedOther = static_cast<const PDFICCBasedColorSpace*>(other);

    const PDFAbstractColorSpace* alternateColorSpace = typedOther->getAlternateColorSpace();

    if (static_cast<bool>(m_alternateColorSpace.data()) != static_cast<bool>(alternateColorSpace))
    {
        return false;
    }

    if (m_alternateColorSpace && alternateColorSpace && !m_alternateColorSpace->equals(alternateColorSpace))
    {
        return false;
    }

    return m_range == typedOther->getRange() && m_iccProfileDataChecksum == typedOther->getIccProfileDataChecksum();
}

PDFColorSpacePointer PDFICCBasedColorSpace::createICCBasedColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                                     const PDFDocument* document,
                                                                     const PDFStream* stream,
                                                                     int recursion,
                                                                     std::set<QByteArray>& usedNames)
{
    // First, try to load alternate color space, if it is present
    const PDFDictionary* dictionary = stream->getDictionary();
    QByteArray iccProfileData = document->getDecodedStream(stream);

    PDFDocumentDataLoaderDecorator loader(document);
    PDFColorSpacePointer alternateColorSpace;
    if (dictionary->hasKey(ICCBASED_ALTERNATE))
    {
        alternateColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(dictionary->get(ICCBASED_ALTERNATE)), recursion, usedNames);
    }
    else
    {
        // Determine color space from parameter N, which determines number of components
        const PDFInteger N = loader.readIntegerFromDictionary(dictionary, ICCBASED_N, 0);

        switch (N)
        {
            case 1:
            {
                alternateColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, PDFObject::createName(COLOR_SPACE_NAME_DEVICE_GRAY), recursion, usedNames);
                break;
            }

            case 3:
            {
                alternateColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, PDFObject::createName(COLOR_SPACE_NAME_DEVICE_RGB), recursion, usedNames);
                break;
            }

            case 4:
            {
                alternateColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, PDFObject::createName(COLOR_SPACE_NAME_DEVICE_CMYK), recursion, usedNames);
                break;
            }

            default:
            {
                throw PDFException(PDFTranslationContext::tr("Can't determine alternate color space for ICC based profile. Number of components is %1.").arg(N));
                break;
            }
        }
    }

    if (!alternateColorSpace)
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine alternate color space for ICC based profile."));
    }

    Ranges ranges = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
    static_assert(ranges.size() == 8, "Fix initialization above!");

    const size_t components = alternateColorSpace->getColorComponentCount();
    const size_t rangeSize = 2 * components;

    if (rangeSize > ranges.size())
    {
        throw PDFException(PDFTranslationContext::tr("Too much color components for ICC based profile."));
    }

    auto itStart = ranges.begin();
    auto itEnd = std::next(itStart, rangeSize);
    loader.readNumberArrayFromDictionary(dictionary, ICCBASED_RANGE, itStart, itEnd);

    return PDFColorSpacePointer(new PDFICCBasedColorSpace(qMove(alternateColorSpace), ranges, qMove(iccProfileData), loader.readReferenceFromDictionary(dictionary, "Metadata")));
}

const PDFICCBasedColorSpace::Ranges& PDFICCBasedColorSpace::getRange() const
{
    return m_range;
}

const QByteArray& PDFICCBasedColorSpace::getIccProfileData() const
{
    return m_iccProfileData;
}

const QByteArray& PDFICCBasedColorSpace::getIccProfileDataChecksum() const
{
    return m_iccProfileDataChecksum;
}

const PDFAbstractColorSpace* PDFICCBasedColorSpace::getAlternateColorSpace() const
{
    return m_alternateColorSpace.data();
}

PDFIndexedColorSpace::PDFIndexedColorSpace(PDFColorSpacePointer baseColorSpace, QByteArray&& colors, int maxValue) :
    m_baseColorSpace(qMove(baseColorSpace)),
    m_colors(qMove(colors)),
    m_maxValue(maxValue)
{

}

bool PDFIndexedColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFAbstractColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFIndexedColorSpace*>(other));
    const PDFIndexedColorSpace* typedOther = static_cast<const PDFIndexedColorSpace*>(other);

    const PDFAbstractColorSpace* baseColorSpace = typedOther->getBaseColorSpace().data();

    if (static_cast<bool>(m_baseColorSpace.data()) != static_cast<bool>(baseColorSpace))
    {
        return false;
    }

    if (m_baseColorSpace && baseColorSpace && !m_baseColorSpace->equals(baseColorSpace))
    {
        return false;
    }

    return m_colors == typedOther->getColors() && m_maxValue == typedOther->getMaxValue();
}

PDFColor PDFIndexedColorSpace::getDefaultColorOriginal() const
{
    return PDFColor(0.0f);
}

QColor PDFIndexedColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    // Indexed color space value must have exactly one component!
    Q_ASSERT(color.size() == 1);
    Q_UNUSED(isRange01);

    const int colorIndex = qBound(MIN_VALUE, static_cast<int>(color[0]), m_maxValue);
    const int componentCount = static_cast<int>(m_baseColorSpace->getColorComponentCount());
    const int byteOffset = colorIndex * componentCount;

    // We must point into the array. Check first and last component.
    Q_ASSERT(byteOffset + componentCount - 1 < m_colors.size());

    PDFColor decodedColor;
    const char* bytePointer = m_colors.constData() + byteOffset;

    for (int i = 0; i < componentCount; ++i)
    {
        const unsigned char value = *bytePointer++;
        const PDFColorComponent component = static_cast<PDFColorComponent>(value) / 255.0f;
        decodedColor.push_back(component);
    }

    return m_baseColorSpace->getColor(decodedColor, cms, intent, reporter, true);
}

size_t PDFIndexedColorSpace::getColorComponentCount() const
{
    return 1;
}

QImage PDFIndexedColorSpace::getImage(const PDFImageData& imageData,
                                      const PDFImageData& softMask,
                                      const PDFCMS* cms,
                                      RenderingIntent intent,
                                      PDFRenderErrorReporter* reporter,
                                      const PDFOperationControl* operationControl) const
{
    if (imageData.isValid())
    {
        switch (imageData.getMaskingType())
        {
            case PDFImageData::MaskingType::None:
            {
                QImage image(imageData.getWidth(), imageData.getHeight(), QImage::Format_RGB888);
                image.fill(QColor(Qt::white));

                unsigned int componentCount = imageData.getComponents();
                PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());

                if (componentCount != getColorComponentCount())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for indexed color space. Color space has %1 colors. Provided color count is %4.").arg(getColorComponentCount()).arg(componentCount));
                }

                Q_ASSERT(componentCount == 1);

                PDFColor color;
                color.resize(1);

                for (unsigned int i = 0, rowCount = imageData.getHeight(); i < rowCount; ++i)
                {
                    // Is operation being cancelled?
                    if (PDFOperationControl::isOperationCancelled(operationControl))
                    {
                        throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Operation cancelled!"));
                    }

                    reader.seek(i * imageData.getStride());
                    unsigned char* outputLine = image.scanLine(i);

                    for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                    {
                        PDFBitReader::Value index = reader.read();
                        color[0] = index;

                        QColor transformedColor = getColor(color, cms, intent, reporter, false);
                        QRgb rgb = transformedColor.rgb();

                        *outputLine++ = qRed(rgb);
                        *outputLine++ = qGreen(rgb);
                        *outputLine++ = qBlue(rgb);
                    }
                }

                return image;
            }

            case PDFImageData::MaskingType::SoftMask:
            {
                const bool hasMatte = !softMask.getMatte().empty();
                QImage image(imageData.getWidth(), imageData.getHeight(), hasMatte ? QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBA8888);

                unsigned int componentCount = imageData.getComponents();
                PDFBitReader reader(&imageData.getData(), imageData.getBitsPerComponent());

                if (componentCount != getColorComponentCount())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for indexed color space. Color space has %1 colors. Provided color count is %4.").arg(getColorComponentCount()).arg(componentCount));
                }

                Q_ASSERT(componentCount == 1);

                PDFColor color;
                color.resize(1);

                QImage alphaMask = createAlphaMask(softMask);
                if (alphaMask.size() != image.size())
                {
                    // Scale the alpha mask, if it is masked
                    alphaMask = alphaMask.scaled(image.size());
                }

                for (unsigned int i = 0, rowCount = imageData.getHeight(); i < rowCount; ++i)
                {
                    // Is operation being cancelled?
                    if (PDFOperationControl::isOperationCancelled(operationControl))
                    {
                        throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Operation cancelled!"));
                    }

                    reader.seek(i * imageData.getStride());
                    unsigned char* outputLine = image.scanLine(i);
                    unsigned char* alphaLine = alphaMask.scanLine(i);

                    for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                    {
                        PDFBitReader::Value index = reader.read();
                        color[0] = index;

                        QColor transformedColor = getColor(color, cms, intent, reporter, false);
                        QRgb rgb = transformedColor.rgb();

                        *outputLine++ = qRed(rgb);
                        *outputLine++ = qGreen(rgb);
                        *outputLine++ = qBlue(rgb);
                        *outputLine++ = *alphaLine++;
                    }
                }

                return image;
            }

            case PDFImageData::MaskingType::ColorKeyMasking:
            {
                QImage image(imageData.getWidth(), imageData.getHeight(), QImage::Format_RGBA8888);
                image.fill(QColor(Qt::transparent));

                unsigned int componentCount = imageData.getComponents();
                if (componentCount != getColorComponentCount())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid colors for color space. Color space has %1 colors. Provided color count is %4.").arg(getColorComponentCount()).arg(componentCount));
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

                const PDFReal max = reader.max();
                for (unsigned int i = 0, rowCount = imageData.getHeight(); i < rowCount; ++i)
                {
                    reader.seek(i * imageData.getStride());
                    unsigned char* outputLine = image.scanLine(i);

                    for (unsigned int j = 0; j < imageData.getWidth(); ++j)
                    {
                        // Number of masked-out colors
                        unsigned int maskedColors = 0;

                        for (unsigned int k = 0; k < componentCount; ++k)
                        {
                            PDFBitReader::Value value = reader.read();

                            // Interpolate value, if decode is not empty
                            if (!decode.empty())
                            {
                                color[k] = interpolate(value, 0.0, max, decode[2 * k], decode[2 * k + 1]);
                            }
                            else
                            {
                                color[k] = value;
                            }

                            Q_ASSERT(2 * k + 1 < colorKeyMask.size());
                            if (static_cast<std::decay<decltype(colorKeyMask)>::type::value_type>(value) >= colorKeyMask[2 * k] &&
                                static_cast<std::decay<decltype(colorKeyMask)>::type::value_type>(value) <= colorKeyMask[2 * k + 1])
                            {
                                ++maskedColors;
                            }
                        }

                        QColor transformedColor = getColor(color, cms, intent, reporter, false);
                        QRgb rgb = transformedColor.rgb();

                        *outputLine++ = qRed(rgb);
                        *outputLine++ = qGreen(rgb);
                        *outputLine++ = qBlue(rgb);
                        *outputLine++ = (maskedColors == componentCount) ? 0x00 : 0xFF;
                    }
                }

                return image;
            }

            default:
                throw PDFRendererException(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Image masking not implemented!"));
        }
    }

    return QImage();
}

PDFColorSpacePointer PDFIndexedColorSpace::createIndexedColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                                   const PDFDocument* document,
                                                                   const PDFArray* array,
                                                                   int recursion,
                                                                   std::set<QByteArray>& usedNames)
{
    Q_ASSERT(array->getCount() == 4);

    // Read base color space
    PDFColorSpacePointer baseColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(array->getItem(1)), recursion, usedNames);

    if (!baseColorSpace)
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine base color space for indexed color space."));
    }

    // Read maximum value
    PDFDocumentDataLoaderDecorator loader(document);
    const int maxValue = qBound<int>(MIN_VALUE, loader.readInteger(array->getItem(2), 0), MAX_VALUE);

    // Read stream/byte string with corresponding color values
    QByteArray colors;
    const PDFObject& colorDataObject = document->getObject(array->getItem(3));

    if (colorDataObject.isString())
    {
        colors = colorDataObject.getString();
    }
    else if (colorDataObject.isStream())
    {
        colors = document->getDecodedStream(colorDataObject.getStream());
    }

    // Check, if we have enough colors
    const int colorCount = maxValue - MIN_VALUE + 1;
    const int componentCount = static_cast<int>(baseColorSpace->getColorComponentCount());
    const int byteCount = colorCount * componentCount;
    if (byteCount > colors.size())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid colors for indexed color space. Color space has %1 colors, %2 color components and must have %3 size. Provided size is %4.").arg(colorCount).arg(componentCount).arg(byteCount).arg(colors.size()));
    }

    return PDFColorSpacePointer(new PDFIndexedColorSpace(qMove(baseColorSpace), qMove(colors), maxValue));
}

PDFColorSpacePointer PDFIndexedColorSpace::getBaseColorSpace() const
{
    return m_baseColorSpace;
}

std::vector<PDFColorComponent> PDFIndexedColorSpace::transformColorsToBaseColorSpace(const PDFColorBuffer buffer) const
{
    const std::size_t colorComponentCount = m_baseColorSpace->getColorComponentCount();
    std::vector<PDFColorComponent> result(buffer.size() * colorComponentCount, 0.0f);

    auto outputIt = result.begin();
    for (PDFColorComponent input : buffer)
    {
        const int colorIndex = qBound(MIN_VALUE, static_cast<int>(input), m_maxValue);
        const int byteOffset = int(colorIndex * colorComponentCount);

        // We must point into the array. Check first and last component.
        Q_ASSERT(byteOffset + colorComponentCount - 1 < static_cast<size_t>(m_colors.size()));

        const char* bytePointer = m_colors.constData() + byteOffset;

        for (std::size_t i = 0; i < colorComponentCount; ++i)
        {
            const unsigned char value = *bytePointer++;
            const PDFColorComponent component = static_cast<PDFColorComponent>(value) / 255.0f;
            Q_ASSERT(outputIt != result.cend());
            *outputIt++ = component;
        }
    }
    Q_ASSERT(outputIt == result.cend());

    return result;
}

int PDFIndexedColorSpace::getMaxValue() const
{
    return m_maxValue;
}

const QByteArray& PDFIndexedColorSpace::getColors() const
{
    return m_colors;
}

PDFSeparationColorSpace::PDFSeparationColorSpace(QByteArray&& colorName, PDFColorSpacePointer alternateColorSpace, PDFFunctionPtr tintTransform) :
    m_colorName(qMove(colorName)),
    m_alternateColorSpace(qMove(alternateColorSpace)),
    m_tintTransform(qMove(tintTransform)),
    m_isNone(m_colorName == "None"),
    m_isAll(m_colorName == "All")
{

}

bool PDFSeparationColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFAbstractColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFSeparationColorSpace*>(other));
    const PDFSeparationColorSpace* typedOther = static_cast<const PDFSeparationColorSpace*>(other);

    const PDFAbstractColorSpace* alternateColorSpace = typedOther->getAlternateColorSpace().data();

    if (static_cast<bool>(m_alternateColorSpace.data()) != static_cast<bool>(alternateColorSpace))
    {
        return false;
    }

    if (m_alternateColorSpace && alternateColorSpace && !m_alternateColorSpace->equals(alternateColorSpace))
    {
        return false;
    }

    return m_colorName == typedOther->getColorName();
}

PDFColor PDFSeparationColorSpace::getDefaultColorOriginal() const
{
    return PDFColor(1.0f);
}

QColor PDFSeparationColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    // Separation color space value must have exactly one component!
    Q_ASSERT(color.size() == 1);
    Q_UNUSED(isRange01);

    // According to the PDF 2.0 specification, separation color space, with colorant name "None"
    // should not produce any visible output.
    if (m_isNone)
    {
        return Qt::transparent;
    }

    // Input value
    double tint = color.back();

    // Jakub Melka: According to the PDF 2.0 specification, separation color space, with colorant name "All"
    // should apply tint value to all output colorants, alternate color space and tint function should
    // be ignored, and because QColor is aditive, we must invert the tint value.
    if (m_isAll)
    {
        const double inversedTint = qBound(0.0, 1.0 - tint, 1.0);
        return QColor::fromRgbF(inversedTint, inversedTint, inversedTint);
    }

    // Output values
    std::vector<double> outputColor;
    outputColor.resize(m_alternateColorSpace->getColorComponentCount(), 0.0);
    PDFFunction::FunctionResult result = m_tintTransform->apply(&tint, &tint + 1, outputColor.data(), outputColor.data() + outputColor.size());

    if (result)
    {
        PDFColor inputColor;
        std::for_each(outputColor.cbegin(), outputColor.cend(), [&inputColor](double value) { inputColor.push_back(static_cast<float>(value)); });
        return m_alternateColorSpace->getColor(inputColor, cms, intent, reporter, false);
    }
    else
    {
        // Return invalid color
        return QColor();
    }
}

size_t PDFSeparationColorSpace::getColorComponentCount() const
{
    return 1;
}

std::vector<PDFColorComponent> PDFSeparationColorSpace::transformColorsToBaseColorSpace(const PDFColorBuffer buffer) const
{
    const std::size_t colorComponentCount = m_alternateColorSpace->getColorComponentCount();
    std::vector<PDFColorComponent> result(buffer.size() * colorComponentCount, 0.0f);

    std::vector<double> outputColor;
    outputColor.resize(colorComponentCount, 0.0);

    auto outputIt = result.begin();
    for (PDFColorComponent input : buffer)
    {
        // Input value
        double tint = input;

        Q_ASSERT(outputIt + (colorComponentCount - 1) != result.cend());

        if (m_isAll)
        {
            const double inversedTint = qBound(0.0, 1.0 - tint, 1.0);
            std::fill(outputIt, outputIt + colorComponentCount, inversedTint);
        }
        else
        {
            m_tintTransform->apply(&tint, &tint + 1, outputColor.data(), outputColor.data() + outputColor.size());
            std::copy(outputColor.cbegin(), outputColor.cend(), outputIt);
        }

        outputIt = std::next(outputIt, colorComponentCount);
    }
    Q_ASSERT(outputIt == result.cend());

    return result;
}

PDFColorSpacePointer PDFSeparationColorSpace::createSeparationColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                                         const PDFDocument* document,
                                                                         const PDFArray* array,
                                                                         int recursion,
                                                                         std::set<QByteArray>& usedNames)
{
    Q_ASSERT(array->getCount() == 4);

    // Read color name
    const PDFObject& colorNameObject = document->getObject(array->getItem(1));
    if (!colorNameObject.isName())
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine color name for separation color space."));
    }
    QByteArray colorName = colorNameObject.getString();

    // Read alternate color space
    PDFColorSpacePointer alternateColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(array->getItem(2)), recursion, usedNames);
    if (!alternateColorSpace)
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine alternate color space for separation color space."));
    }

    PDFFunctionPtr tintTransform = PDFFunction::createFunction(document, array->getItem(3));
    if (!tintTransform)
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine tint transform for separation color space."));
    }

    return PDFColorSpacePointer(new PDFSeparationColorSpace(qMove(colorName), qMove(alternateColorSpace), qMove(tintTransform)));
}

PDFColorSpacePointer PDFSeparationColorSpace::getAlternateColorSpace() const
{
    return m_alternateColorSpace;
}

const QByteArray& PDFSeparationColorSpace::getColorName() const
{
    return m_colorName;
}

const unsigned char* PDFImageData::getRow(unsigned int rowIndex) const
{
    const unsigned char* data = reinterpret_cast<const unsigned char*>(m_data.constData());

    Q_ASSERT(rowIndex < m_height);
    return data + (rowIndex * m_stride);
}

bool PDFPatternColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    // Compare pointers
    return this == other;
}

QColor PDFPatternColorSpace::getDefaultColor(const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(cms);
    Q_UNUSED(intent);
    Q_UNUSED(reporter);

    return QColor(Qt::transparent);
}

PDFColor PDFPatternColorSpace::getDefaultColorOriginal() const
{
    return PDFColor();
}

QColor PDFPatternColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_UNUSED(color);
    Q_UNUSED(cms);
    Q_UNUSED(intent);
    Q_UNUSED(reporter);
    Q_UNUSED(isRange01);

    throw PDFException(PDFTranslationContext::tr("Pattern doesn't have defined uniform color."));
}

size_t PDFPatternColorSpace::getColorComponentCount() const
{
    return 0;
}

PDFDeviceNColorSpace::PDFDeviceNColorSpace(PDFDeviceNColorSpace::Type type,
                                           PDFDeviceNColorSpace::Colorants&& colorants,
                                           PDFColorSpacePointer alternateColorSpace,
                                           PDFColorSpacePointer processColorSpace,
                                           PDFFunctionPtr tintTransform,
                                           std::vector<QByteArray>&& colorantsPrintingOrder,
                                           std::vector<QByteArray> processColorSpaceComponents) :
    m_type(type),
    m_colorants(qMove(colorants)),
    m_alternateColorSpace(qMove(alternateColorSpace)),
    m_processColorSpace(qMove(processColorSpace)),
    m_tintTransform(qMove(tintTransform)),
    m_colorantsPrintingOrder(qMove(colorantsPrintingOrder)),
    m_processColorSpaceComponents(qMove(processColorSpaceComponents)),
    m_isNone(false)
{
    m_isNone = std::all_of(m_colorants.cbegin(), m_colorants.cend(), [](const auto& colorant) { return colorant.name == "None"; });
}

bool PDFDeviceNColorSpace::equals(const PDFAbstractColorSpace* other) const
{
    if (!PDFAbstractColorSpace::equals(other))
    {
        return false;
    }

    Q_ASSERT(dynamic_cast<const PDFDeviceNColorSpace*>(other));
    const PDFDeviceNColorSpace* typedOther = static_cast<const PDFDeviceNColorSpace*>(other);

    const PDFAbstractColorSpace* alternateColorSpace = typedOther->getAlternateColorSpace().data();

    if (static_cast<bool>(m_alternateColorSpace.data()) != static_cast<bool>(alternateColorSpace))
    {
        return false;
    }

    if (m_alternateColorSpace && alternateColorSpace && !m_alternateColorSpace->equals(alternateColorSpace))
    {
        return false;
    }

    const Colorants& colorants = typedOther->getColorants();

    if (m_colorants.size() != colorants.size())
    {
        return false;
    }

    for (size_t i = 0; i < m_colorants.size(); ++i)
    {
        if (m_colorants[i].name != colorants[i].name)
        {
            return false;
        }
    }

    return true;
}

PDFColor PDFDeviceNColorSpace::getDefaultColorOriginal() const
{
    PDFColor color;
    color.resize(getColorComponentCount());

    // Jakub Melka: According to the PDF 2.0 specification, each channel should
    // be initially set to 1.0.
    for (size_t i = 0, colorComponentCount = color.size(); i < colorComponentCount; ++i)
    {
        color[i] = 1.0;
    }

    return color;
}

QColor PDFDeviceNColorSpace::getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const
{
    Q_UNUSED(isRange01);

    // According to the PDF 2.0 specification, DeviceN color space, with all colorant name "None"
    // should not produce any visible output.
    if (m_isNone)
    {
        return Qt::transparent;
    }

    // Input values
    std::vector<double> inputColor(color.size(), 0.0);
    for (size_t i = 0, count = inputColor.size(); i < count; ++i)
    {
        inputColor[i] = color[i];
    }

    // Output values
    std::vector<double> outputColor;
    outputColor.resize(m_alternateColorSpace->getColorComponentCount(), 0.0);
    PDFFunction::FunctionResult result = m_tintTransform->apply(inputColor.data(), inputColor.data() + inputColor.size(), outputColor.data(), outputColor.data() + outputColor.size());

    if (result)
    {
        PDFColor inputColor2;
        std::for_each(outputColor.cbegin(), outputColor.cend(), [&inputColor2](double value) { inputColor2.push_back(static_cast<float>(value)); });
        return m_alternateColorSpace->getColor(inputColor2, cms, intent, reporter, false);
    }
    else
    {
        // Return invalid color
        return QColor();
    }
}

size_t PDFDeviceNColorSpace::getColorComponentCount() const
{
    return m_colorants.size();
}

std::vector<PDFColorComponent> PDFDeviceNColorSpace::transformColorsToBaseColorSpace(const PDFColorBuffer buffer) const
{
    std::vector<PDFColorComponent> result;

    const std::size_t colorantCount = getColorComponentCount();
    if (colorantCount > 0)
    {
        const std::size_t inputColorCount = buffer.size() / colorantCount;
        const std::size_t alternateColorSpaceComponentCount = m_alternateColorSpace->getColorComponentCount();
        result.resize(inputColorCount * alternateColorSpaceComponentCount, 0.0f);

        std::vector<double> inputColor(colorantCount, 0.0);
        std::vector<double> outputColor(alternateColorSpaceComponentCount, 0.0);

        auto outputIt = result.begin();
        for (auto it = buffer.begin(); it != buffer.end(); it = std::next(it, colorantCount))
        {
            std::copy(it, it + colorantCount, inputColor.begin());
            m_tintTransform->apply(inputColor.data(), inputColor.data() + inputColor.size(), outputColor.data(), outputColor.data() + outputColor.size());
            std::copy(outputColor.cbegin(), outputColor.cend(), outputIt);
            outputIt = std::next(outputIt, alternateColorSpaceComponentCount);
        }
        Q_ASSERT(outputIt == result.cend());
    }

    return result;
}

PDFColorSpacePointer PDFDeviceNColorSpace::createDeviceNColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                                   const PDFDocument* document,
                                                                   const PDFArray* array,
                                                                   int recursion,
                                                                   std::set<QByteArray>& usedNames)
{
    Q_ASSERT(array->getCount() >= 4);

    PDFDocumentDataLoaderDecorator loader(document);
    std::vector<QByteArray> colorantNames = loader.readNameArray(array->getItem(1));

    if (colorantNames.empty())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid colorants for DeviceN color space."));
    }

    std::vector<ColorantInfo> colorants;
    colorants.resize(colorantNames.size());
    for (size_t i = 0; i < colorantNames.size(); ++i)
    {
        colorants[i].name = qMove(colorantNames[i]);
    }

    // Read alternate color space
    PDFColorSpacePointer alternateColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(array->getItem(2)), recursion, usedNames);
    if (!alternateColorSpace)
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine alternate color space for DeviceN color space."));
    }

    PDFFunctionPtr tintTransform = PDFFunction::createFunction(document, array->getItem(3));
    if (!tintTransform)
    {
        throw PDFException(PDFTranslationContext::tr("Can't determine tint transform for DeviceN color space."));
    }

    Type type = Type::DeviceN;
    std::vector<QByteArray> colorantsPrintingOrder;
    PDFColorSpacePointer processColorSpace;
    std::vector<QByteArray> processColorSpaceComponents;

    // Now, check, if we have attributes, and if yes, then read them
    if (array->getCount() == 5)
    {
        const PDFObject& object = document->getObject(array->getItem(4));
        if (object.isDictionary())
        {
            const PDFDictionary* attributesDictionary = object.getDictionary();
            QByteArray subtype = loader.readNameFromDictionary(attributesDictionary, "Subtype");
            if (subtype == "NChannel")
            {
                type = Type::NChannel;
            }

            const PDFObject& colorantsObject = document->getObject(attributesDictionary->get("Colorants"));
            if (colorantsObject.isDictionary())
            {
                const PDFDictionary* colorantsDictionary = colorantsObject.getDictionary();

                // Separation color spaces for each colorant
                for (ColorantInfo& colorantInfo : colorants)
                {
                    if (colorantsDictionary->hasKey(colorantInfo.name))
                    {
                        colorantInfo.separationColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, document->getObject(colorantsDictionary->get(colorantInfo.name)), recursion, usedNames);
                    }
                }
            }

            const PDFObject& mixingHints = document->getObject(attributesDictionary->get("MixingHints"));
            if (mixingHints.isDictionary())
            {
                const PDFDictionary* mixingHintsDictionary = mixingHints.getDictionary();

                // Printing order
                colorantsPrintingOrder = loader.readNameArray(mixingHintsDictionary->get("PrintingOrder"));

                // Solidities
                const PDFObject& solidityObject = document->getObject(mixingHintsDictionary->get("Solidites"));
                if (solidityObject.isDictionary())
                {
                    const PDFDictionary* solidityDictionary = solidityObject.getDictionary();
                    const PDFReal defaultSolidity = loader.readNumberFromDictionary(solidityDictionary, "Default", 0.0);
                    for (ColorantInfo& colorantInfo : colorants)
                    {
                        colorantInfo.solidity = loader.readNumberFromDictionary(solidityDictionary, colorantInfo.name, defaultSolidity);
                    }
                }

                // Dot gain
                const PDFObject& dotGainObject = document->getObject(mixingHintsDictionary->get("DotGain"));
                if (dotGainObject.isDictionary())
                {
                    const PDFDictionary* dotGainDictionary = dotGainObject.getDictionary();
                    for (ColorantInfo& colorantInfo : colorants)
                    {
                        const PDFObject& dotGainFunctionObject = document->getObject(dotGainDictionary->get(colorantInfo.name));
                        if (!dotGainFunctionObject.isNull())
                        {
                            colorantInfo.dotGain = PDFFunction::createFunction(document, dotGainFunctionObject);
                        }
                    }
                }
            }

            // Process
            const PDFObject& processObject = document->getObject(attributesDictionary->get("Process"));
            if (processObject.isDictionary())
            {
                const PDFDictionary* processDictionary = processObject.getDictionary();
                const PDFObject& processColorSpaceObject = document->getObject(processDictionary->get("ColorSpace"));
                if (!processColorSpaceObject.isNull())
                {
                    processColorSpace = PDFAbstractColorSpace::createColorSpaceImpl(colorSpaceDictionary, document, processColorSpaceObject, recursion, usedNames);
                    processColorSpaceComponents = loader.readNameArrayFromDictionary(processDictionary, "Components");
                }
            }
        }
    }

    return PDFColorSpacePointer(new PDFDeviceNColorSpace(type, qMove(colorants), qMove(alternateColorSpace), qMove(processColorSpace), qMove(tintTransform), qMove(colorantsPrintingOrder), qMove(processColorSpaceComponents)));
}

}   // namespace pdf
