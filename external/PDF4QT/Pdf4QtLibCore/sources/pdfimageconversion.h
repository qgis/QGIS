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

#ifndef PDFIMAGECONVERSION_H
#define PDFIMAGECONVERSION_H

#include "pdfglobal.h"

#include <QImage>

namespace pdf
{

/// This class facilitates various image conversions,
/// including transforming colored images into monochromatic (or bitonal) formats.
class PDF4QTLIBCORESHARED_EXPORT PDFImageConversion
{
public:
    PDFImageConversion();

    enum class ConversionMethod
    {
        Automatic,  ///< The threshold is determined automatically using an algorithm.
        Manual      ///< The threshold is manually provided by the user.
    };

    /// Sets the image to be converted using the specified conversion method.
    /// This operation resets any previously converted image and the automatic threshold,
    /// thereby erasing all prior image data.
    /// \param image The image to be set for conversion.
    void setImage(QImage image);

    /// Sets the method for image conversion. Multiple methods are available
    /// for selection. If the manual method is chosen, an appropriate threshold
    /// must also be set by the user.
    /// \param method The conversion method to be used.
    void setConversionMethod(ConversionMethod method);

    /// Sets the manual threshold value. When a non-manual (e.g., automatic) conversion
    /// method is in use, this function will retain the manual threshold settings,
    /// but the conversion will utilize an automatically calculated threshold for the image.
    /// The manually set threshold is preserved and not overwritten. Therefore, if the
    /// manual conversion method is later selected, the previously established manual
    /// threshold will be applied.
    /// \param threshold The manual threshold value to be set.
    void setThreshold(int threshold);

    /// This method converts the image into a bitonal (monochromatic) format. If
    /// the automatic threshold calculation is enabled, it executes Otsu's 1D algorithm
    /// to determine the threshold. When the manual conversion method is selected,
    /// the automatic threshold calculation is bypassed, and the predefined manual threshold
    /// value is utilized instead. This method returns true if the conversion is
    /// successful, and false otherwise.
    bool convert();

    /// Returns the threshold used in image conversion. If the automatic conversion method is
    /// selected, this function should be called only after executing the convert() method;
    /// otherwise, it may return invalid data. The automatic threshold calculation is
    /// performed within the convert() method.
    /// \returns The threshold value used in image conversion.
    int getThreshold() const;

    /// Returns the converted image. This method should only be called after
    /// the convert() method has been executed, and additionally, only if the
    /// convert() method returns true. If these conditions are not met, the result
    /// is undefined.
    QImage getConvertedImage() const;

private:
    int calculateOtsu1DThreshold() const;

    static constexpr int DEFAULT_THRESHOLD = 128;

    QImage m_image;
    QImage m_convertedImage;
    ConversionMethod m_conversionMethod = ConversionMethod::Automatic;
    int m_automaticThreshold = DEFAULT_THRESHOLD;
    int m_manualThreshold = DEFAULT_THRESHOLD;
};

}   // namespace pdf

#endif // PDFIMAGECONVERSION_H
