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

#ifndef PDFCOLORCONVERTOR_H
#define PDFCOLORCONVERTOR_H

#include "pdfglobal.h"

#include <QPen>
#include <QBrush>
#include <QColor>
#include <QImage>

namespace pdf
{

/// \class PDFColorConvertor
/// \brief Performs color conversions in the RGB color space.
/// This class supports multiple modes of operation, making it
/// useful for accessibility purposes, particularly for visually impaired users.
class PDF4QTLIBCORESHARED_EXPORT PDFColorConvertor
{
public:
    PDFColorConvertor();

    /// Enumeration for different modes of color conversion
    enum class Mode
    {
        Normal,             ///< No color conversion is performed.
        InvertedColors,     ///< Inverts colors in the RGB space.
        Grayscale,          ///< Converts colors to grayscale.
        HighContrast,       ///< Adjusts contrast using a sigmoid function.
        Bitonal,            ///< Creates a monochromatic (two-color) image.
        CustomColors,       ///< Applies custom foreground and background colors.
   };

    bool operator==(const PDFColorConvertor&) const = default;
    bool operator!=(const PDFColorConvertor&) const = default;

    /// Checks if color conversion is active.
    /// \return `true` if color conversion is currently active; `false` otherwise.
    /// When `false`, no color conversions are performed, and the `convert` function
    /// operates as an identity function.
    bool isActive() const;

    /// Sets the mode for color conversion.
    /// \param mode Specifies the new mode to be used for color conversion.
    /// This mode determines how colors are transformed during the conversion process.
    void setMode(Mode mode);

    /// Converts the given color based on the current mode
    /// \param color The QColor to be converted
    /// \param background Use background color
    /// \param foreground Use foreground color
    /// \return The converted QColor
    QColor convert(QColor color, bool background, bool foreground) const;

    /// Converts the given image based on the current mode
    /// \param image The image to be converted
    /// \return The converted image
    QImage convert(QImage image) const;

    /// Sets the correction factor for enhancing contrast in high contrast mode.
    /// This factor determines the level of contrast enhancement:
    /// - For subtle enhancement, set the factor between 5 and 10.
    /// - For moderate enhancement, use values between 10 and 20.
    /// - For strong enhancement, choose values of 20 or higher.
    /// \param factor The correction factor used for adjusting contrast.
    void setHighContrastBrightnessFactor(float factor);

    /// Retrieves the current bitonal threshold value used for determining lightness levels.
    /// This threshold helps in differentiating between light and dark areas in an image processing context.
    /// \returns The current bitonal threshold value as an integer. This value is used to classify pixels
    /// as either light or dark based on their lightness.
    int getBitonalThreshold() const;

    /// Sets a new bitonal threshold value to be used in image processing.
    /// This threshold determines how lightness levels are classified into binary categories (light or dark).
    /// \param newBitonalThreshold The new threshold value as an integer. It should be chosen carefully
    /// to ensure accurate differentiation between light and dark areas in images.
    void setBitonalThreshold(int newBitonalThreshold);

    /// Set background color
    /// \param newBackgroundColor Background color
    void setBackgroundColor(const QColor& newBackgroundColor);

    /// Set foreground color
    /// \param newForegroundColor Foreground color
    void setForegroundColor(const QColor& newForegroundColor);

    QColor getBackgroundColor() const;
    QColor getForegroundColor() const;

    QPen convert(const QPen& pen, bool background = false, bool foreground = true) const;
    QBrush convert(const QBrush& brush, bool background = false, bool foreground = true) const;

private:
    /// Correct lightness using sigmoid function
    /// \return Adjusted lightness normalized in range [0.0, 1.0]
    /// \param lightness Lightness in range [0.0, 1.0]
    float correctLigthnessBySigmoidFunction(float lightness) const;

    /// Calculates the value of the sigmoid function based on the given input.
    /// The returned value is unscaled and should be scaled to the range [0.0, 1.0]
    /// using other functions.
    /// \param value The input value for the sigmoid function.
    /// \return The unscaled result of the sigmoid function.
    float sigmoidFunction(float value) const;

    /// Determines the sigmoid function value for a completely black color
    /// (i.e., when lightness is set to zero).
    /// \return The sigmoid function value corresponding to black color.
    float sigmoidParamC_Black() const;

    /// Determines the sigmoid function value for a completely white color
    /// (i.e., when lightness is set to one).
    /// \return The sigmoid function value corresponding to white color.
    float sigmoidParamC_White() const;

    /// Determines the range of the sigmoid function's output,
    /// useful for scaling the range to the normalized interval [0.0, 1.0].
    /// \return The range of the sigmoid function's output.
    float sigmoidParamC_Range() const;

    /// Calculates parameters for the sigmoid function,
    /// which are used to scale color values to the interval [0.0, 1.0].
    void calculateSigmoidParams();

    Mode m_mode = Mode::Normal;
    float m_sigmoidParamC = 10.0f;
    float m_sigmoidParamC_Black = 0.0f;
    float m_sigmoidParamC_White = 1.0f;
    float m_sigmoidParamC_Range = 1.0f;
    int m_bitonalThreshold = 128;
    QColor m_backgroundColor = Qt::black;
    QColor m_foregroundColor = Qt::green;
};

}   // namespace pdf

#endif // PDFCOLORCONVERTOR_H
