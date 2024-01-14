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

#ifndef PDFBLENDFUNCTION_H
#define PDFBLENDFUNCTION_H

#include "pdfglobal.h"

#include <QPainter>

namespace pdf
{

enum class BlendMode
{
    // Separable blending modes
    Normal,     ///< Select source color, backdrop is ignored: B(source, dest) = source
    Multiply,   ///< Backdrop and source colors are multiplied: B(source, dest) = source * dest
    Screen,     ///< B(source, dest) = 1.0 - (1.0 - source) * (1.0 - dest) + source + dest - source * dest
    Overlay,    ///< B(source, dest) = HardLight(dest, source)
    Darken,     ///< Selects the darker of the colors. B(source, dest) = qMin(source, dest)
    Lighten,    ///< Selects the lighter of the colors. B(source, dest) = qMax(source, dest)
    ColorDodge, ///< Brightens the backdrop color reflecting the source color. B(source, dest) = qMin(1.0, dest / (1.0 - source)), or 1.0, if source is 1.0
    ColorBurn,  ///< Darkens the backdrop color reflecting the source color. B(source, dest) = 1.0 - qMin(1.0, (1.0 - dest) / source), or 0.0, if source is 0.0
    HardLight,  ///< Multiply or screen the color, depending on the source value.
    SoftLight,  ///< Darkens or lightens the color, depending on the source value.
    Difference, ///< Subtract the darker of colors from the lighter color. B(source, dest) = qAbs(source - dest)
    Exclusion,  ///< B(source, dest) = source + dest - 2 * source * dest

    // Non-separable blending modes
    Hue,
    Saturation,
    Color,
    Luminosity,

    // For compatibility - older PDF specification specifies Compatible mode, which is equal
    // to normal. It should be recognized for sake of compatibility.
    Compatible, ///< Equals to normal

    // Special blend modes for handling overprint. Used only internally.
    Overprint_SelectBackdrop,
    Overprint_SelectNonZeroSourceOrBackdrop,
    Overprint_SelectNonOneSourceOrBackdrop,

    // Invalid blending mode - for internal purposes only
    Invalid
};

class PDFBlendModeInfo
{
public:
    PDFBlendModeInfo() = delete;

    /// Returns blend mode from the specified string. If string is invalid,
    /// then \p Invalid blend mode is returned.
    /// \param name Name of the blend mode
    static BlendMode getBlendMode(const QByteArray& name);

    /// Returns true, if blend mode is supported by Qt drawing subsystem
    /// \param mode Blend mode
    static bool isSupportedByQt(BlendMode mode);

    /// Returns true, if blend mode is separable
    static bool isSeparable(BlendMode mode);

    /// Returns true, if blend mode is white-preserving (i.e. B(1.0, 1.0) == 1.0)
    static bool isWhitePreserving(BlendMode mode);

    /// Returns composition mode for Qt drawing subsystem from blend mode defined
    /// in PDF standard. If blend mode is not supported by Qt drawing subsystem, then default
    /// composition mode is returned.
    /// \param mode Blend mode
    static QPainter::CompositionMode getCompositionModeFromBlendMode(BlendMode mode);

    /// Returns blend mode name
    /// \param mode Blend mode
    static QString getBlendModeName(BlendMode mode);

    /// Returns blend mode translated name
    /// \param mode Blend mode
    static QString getBlendModeTranslatedName(BlendMode mode);

    /// Returns vector of all blend modes, excluding duplicate ones (for example,
    /// Compatible mode is equal to Normal blend mode)
    static std::vector<BlendMode> getBlendModes();
};

/// Class grouping together blend functions. Can also blend non-separable blend modes,
/// such as Color, Hue, Saturation and Luminosity, according 11.3.5.3 of PDF 2.0 specification.
class PDFBlendFunction
{
public:
    PDFBlendFunction() = delete;

    /// Blend function used to blend separable blend modes
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFColorComponent blend(BlendMode mode, PDFColorComponent Cb, PDFColorComponent Cs);

    /// Blend non-separable hue function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFRGB blend_Hue(PDFRGB Cb, PDFRGB Cs);

    /// Blend non-separable saturation function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFRGB blend_Saturation(PDFRGB Cb, PDFRGB Cs);

    /// Blend non-separable color function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFRGB blend_Color(PDFRGB Cb, PDFRGB Cs);

    /// Blend non-separable luminosity function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFRGB blend_Luminosity(PDFRGB Cb, PDFRGB Cs);

    /// Blend non-separable hue function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFGray blend_Hue(PDFGray Cb, PDFGray Cs);

    /// Blend non-separable saturation function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFGray blend_Saturation(PDFGray Cb, PDFGray Cs);

    /// Blend non-separable color function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFGray blend_Color(PDFGray Cb, PDFGray Cs);

    /// Blend non-separable luminosity function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFGray blend_Luminosity(PDFGray Cb, PDFGray Cs);

    /// Blend non-separable hue function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFCMYK blend_Hue(PDFCMYK Cb, PDFCMYK Cs);

    /// Blend non-separable saturation function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFCMYK blend_Saturation(PDFCMYK Cb, PDFCMYK Cs);

    /// Blend non-separable color function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFCMYK blend_Color(PDFCMYK Cb, PDFCMYK Cs);

    /// Blend non-separable luminosity function
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFCMYK blend_Luminosity(PDFCMYK Cb, PDFCMYK Cs);

    /// Blend non-separabe. It is incorrect to call this function
    /// with blend mode, which is separable.
    /// \param mode Non-separable blend mode
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFGray blend_Nonseparable(BlendMode mode, PDFGray Cb, PDFGray Cs);

    /// Blend non-separabe. It is incorrect to call this function
    /// with blend mode, which is separable.
    /// \param mode Non-separable blend mode
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFRGB blend_Nonseparable(BlendMode mode, PDFRGB Cb, PDFRGB Cs);

    /// Blend non-separabe. It is incorrect to call this function
    /// with blend mode, which is separable.
    /// \param mode Non-separable blend mode
    /// \param Cb Backdrop color
    /// \param Cs Source color
    static PDFCMYK blend_Nonseparable(BlendMode mode, PDFCMYK Cb, PDFCMYK Cs);

    /// Get luminosity from color value
    /// \param gray Color value
    static PDFColorComponent getLuminosity(PDFGray gray);

    /// Get luminosity from color value
    /// \param rgb Color value
    static PDFColorComponent getLuminosity(PDFRGB rgb);

    /// Get luminosity from color value
    /// \param cmyk Color value
    static PDFColorComponent getLuminosity(PDFCMYK cmyk);

    /// Union function
    static constexpr PDFColorComponent blend_Union(PDFColorComponent b, PDFColorComponent s) { return b + s - b * s; }

private:
    static PDFRGB nonseparable_gray2rgb(PDFGray gray);
    static PDFGray nonseparable_rgb2gray(PDFRGB rgb);
    static PDFRGB nonseparable_cmyk2rgb(PDFCMYK cmyk);
    static PDFCMYK nonseparable_rgb2cmyk(PDFRGB rgb, PDFColorComponent K);
    static PDFColorComponent nonseparable_Lum(PDFRGB rgb);
    static PDFColorComponent nonseparable_Sat(PDFRGB rgb);
    static PDFRGB nonseparable_SetLum(PDFRGB C, PDFColorComponent l);
    static PDFRGB nonseparable_SetSat(PDFRGB C, PDFColorComponent s);
    static PDFRGB nonseparable_ClipColor(PDFRGB C);
};

}   // namespace pdf

#endif // PDFBLENDFUNCTION_H
