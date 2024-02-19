//    Copyright (C) 2023 Jakub Melka
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

#include "pdfcolorconvertor.h"
#include "pdfimageconversion.h"
#include "pdfutils.h"
#include "pdfdbgheap.h"

#include <cmath>

namespace pdf
{

PDFColorConvertor::PDFColorConvertor()
{
    calculateSigmoidParams();
}

bool PDFColorConvertor::isActive() const
{
    return m_mode != Mode::Normal;
}

void PDFColorConvertor::setMode(Mode mode)
{
    m_mode = mode;
}

QColor PDFColorConvertor::convert(QColor color, bool background, bool foreground) const
{
    switch (m_mode)
    {
        case Mode::Normal:
            return color;

        case Mode::InvertedColors:
            return invertColor(color);

        case Mode::Grayscale:
        {
            int gray = qGray(color.red(), color.green(), color.blue());
            return QColor(gray, gray, gray, color.alpha());
        }

        case Mode::HighContrast:
        {
            const float lightness = color.lightnessF();
            const float adjustedLightness = correctLigthnessBySigmoidFunction(lightness);
            QColor hslColor = color.toHsl();
            hslColor.setHslF(hslColor.hueF(), hslColor.saturationF(), adjustedLightness, color.alphaF());
            return hslColor.toRgb();
        }

        case Mode::Bitonal:
        {
            const int lightness = color.lightness();
            QColor bitonalColor = (lightness >= m_bitonalThreshold) ? QColor(Qt::white) : QColor(Qt::black);
            bitonalColor.setAlpha(bitonalColor.alpha());
            return bitonalColor;
        }

        case Mode::CustomColors:
        {
            if (background)
            {
                return m_backgroundColor;
            }

            if (foreground)
            {
                return m_foregroundColor;
            }

            const float lightness = color.lightnessF();
            QColor convertedColor = m_foregroundColor;
            convertedColor.setRedF(convertedColor.redF() * lightness);
            convertedColor.setGreenF(convertedColor.greenF() * lightness);
            convertedColor.setBlueF(convertedColor.blueF() * lightness);
            return convertedColor;
        }

        default:
            Q_ASSERT(false);
            break;
    }

    return color;
}

QImage PDFColorConvertor::convert(QImage image) const
{
    switch (m_mode)
    {
        case Mode::Normal:
            return image;

        case Mode::InvertedColors:
        {
            image.invertPixels(QImage::InvertRgb);
            return image;
        }

        case Mode::Grayscale:
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QImage alpha = image.convertedTo(QImage::Format_Alpha8);
            QImage grayscaleImage = image.convertedTo(QImage::Format_Grayscale8);
#else
            QImage alpha = image;
            alpha.convertTo(QImage::Format_Alpha8);
            QImage grayscaleImage = image;
            grayscaleImage.convertTo(QImage::Format_Grayscale8);
#endif
            QImage resultImage = grayscaleImage;
            resultImage.convertTo(QImage::Format_ARGB32);
            resultImage.setAlphaChannel(std::move(alpha));
            return resultImage;
        }

        case Mode::Bitonal:
        {
            PDFImageConversion imageConversion;
            imageConversion.setConversionMethod(PDFImageConversion::ConversionMethod::Automatic);
            imageConversion.setImage(image);

            if (imageConversion.convert())
            {
                return imageConversion.getConvertedImage();
            }

            return image;
        }

        case Mode::HighContrast:
        {
            for (int row = 0; row < image.height(); ++row)
            {
                for (int column = 0; column < image.width(); ++column)
                {
                    QColor color = image.pixelColor(column, row);
                    QColor adjustedColor = convert(color, false, false);
                    image.setPixelColor(column, row, adjustedColor);
                }
            }

            return image;
        }

        case Mode::CustomColors:
        {
            for (int row = 0; row < image.height(); ++row)
            {
                for (int column = 0; column < image.width(); ++column)
                {
                    QColor color = image.pixelColor(column, row);
                    const float lightness = 1.0f - color.lightnessF();
                    QColor convertedColor = m_foregroundColor;
                    convertedColor.setRedF(convertedColor.redF() * lightness);
                    convertedColor.setGreenF(convertedColor.greenF() * lightness);
                    convertedColor.setBlueF(convertedColor.blueF() * lightness);
                    image.setPixelColor(column, row, convertedColor);
                }
            }

            return image;
        }

        default:
            Q_ASSERT(false);
            break;
    }

    return image;
}

void PDFColorConvertor::setHighContrastBrightnessFactor(float factor)
{
    m_sigmoidParamC = factor;
    calculateSigmoidParams();
}

float PDFColorConvertor::correctLigthnessBySigmoidFunction(float lightness) const
{
    const float adjustedLightness = sigmoidFunction(lightness);
    const float normalizedLightness = (adjustedLightness - m_sigmoidParamC_Black) / m_sigmoidParamC_Range;
    return qBound(0.0f, normalizedLightness, 1.0f);
}

float PDFColorConvertor::sigmoidFunction(float value) const
{
    return 1.0f / (1.0f + std::exp(-m_sigmoidParamC * (value - 0.5f)));
}

float PDFColorConvertor::sigmoidParamC_Black() const
{
    return sigmoidFunction(0.0);
}

float PDFColorConvertor::sigmoidParamC_White() const
{
    return sigmoidFunction(1.0);
}

float PDFColorConvertor::sigmoidParamC_Range() const
{
    return sigmoidParamC_White() - sigmoidParamC_Black();
}

void PDFColorConvertor::calculateSigmoidParams()
{
    m_sigmoidParamC_Black = sigmoidParamC_Black();
    m_sigmoidParamC_White = sigmoidParamC_White();
    m_sigmoidParamC_Range = sigmoidParamC_Range();
}

QColor PDFColorConvertor::getForegroundColor() const
{
    return m_foregroundColor;
}

QColor PDFColorConvertor::getBackgroundColor() const
{
    return m_backgroundColor;
}

void PDFColorConvertor::setForegroundColor(const QColor& newForegroundColor)
{
    m_foregroundColor = newForegroundColor;
}

void PDFColorConvertor::setBackgroundColor(const QColor& newBackgroundColor)
{
    m_backgroundColor = newBackgroundColor;
}

int PDFColorConvertor::getBitonalThreshold() const
{
    return m_bitonalThreshold;
}

void PDFColorConvertor::setBitonalThreshold(int newBitonalThreshold)
{
    m_bitonalThreshold = newBitonalThreshold;
}

}   // namespace pdf
