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

#include "pdfblendfunction.h"
#include "pdfdbgheap.h"

#include <algorithm>

namespace pdf
{

constexpr const std::pair<const char*, BlendMode> BLEND_MODE_INFOS[] =
{
    { "Normal", BlendMode::Normal },
    { "Multiply", BlendMode::Multiply },
    { "Screen", BlendMode::Screen },
    { "Overlay", BlendMode::Overlay },
    { "Darken", BlendMode::Darken },
    { "Lighten", BlendMode::Lighten },
    { "ColorDodge", BlendMode::ColorDodge },
    { "ColorBurn", BlendMode::ColorBurn },
    { "HardLight", BlendMode::HardLight },
    { "SoftLight", BlendMode::SoftLight },
    { "Difference", BlendMode::Difference },
    { "Exclusion", BlendMode::Exclusion },
    { "Hue", BlendMode::Hue },
    { "Saturation", BlendMode::Saturation },
    { "Color", BlendMode::Color },
    { "Luminosity", BlendMode::Luminosity },
    { "Compatible", BlendMode::Compatible }
};

BlendMode PDFBlendModeInfo::getBlendMode(const QByteArray& name)
{
    for (const std::pair<const char*, BlendMode>& info : BLEND_MODE_INFOS)
    {
        if (info.first == name)
        {
            return info.second;
        }
    }

    return BlendMode::Invalid;
}

bool PDFBlendModeInfo::isSupportedByQt(BlendMode mode)
{
    switch (mode)
    {
       case BlendMode::Normal:
       case BlendMode::Multiply:
       case BlendMode::Screen:
       case BlendMode::Overlay:
       case BlendMode::Darken:
       case BlendMode::Lighten:
       case BlendMode::ColorDodge:
       case BlendMode::ColorBurn:
       case BlendMode::HardLight:
       case BlendMode::SoftLight:
       case BlendMode::Difference:
       case BlendMode::Exclusion:
       case BlendMode::Compatible:
            return true;

        default:
            return false;
    }
}

bool PDFBlendModeInfo::isSeparable(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Normal:
        case BlendMode::Multiply:
        case BlendMode::Screen:
        case BlendMode::Overlay:
        case BlendMode::Darken:
        case BlendMode::Lighten:
        case BlendMode::ColorDodge:
        case BlendMode::ColorBurn:
        case BlendMode::HardLight:
        case BlendMode::SoftLight:
        case BlendMode::Difference:
        case BlendMode::Exclusion:
        case BlendMode::Compatible:
        case BlendMode::Overprint_SelectBackdrop:
        case BlendMode::Overprint_SelectNonZeroSourceOrBackdrop:
        case BlendMode::Overprint_SelectNonOneSourceOrBackdrop:
            return true;

        case BlendMode::Hue:
        case BlendMode::Saturation:
        case BlendMode::Color:
        case BlendMode::Luminosity:
            return false;

        default:
            Q_ASSERT(false);
            return false;
    }
}

bool PDFBlendModeInfo::isWhitePreserving(BlendMode mode)
{
    if (!isSeparable(mode))
    {
        return false;
    }

    if (mode == BlendMode::Difference || mode == BlendMode::Exclusion)
    {
        return false;
    }

    return true;
}

QPainter::CompositionMode PDFBlendModeInfo::getCompositionModeFromBlendMode(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Normal:
            return QPainter::CompositionMode_SourceOver;
        case BlendMode::Multiply:
            return QPainter::CompositionMode_Multiply;
        case BlendMode::Screen:
            return QPainter::CompositionMode_Screen;
        case BlendMode::Overlay:
            return QPainter::CompositionMode_Overlay;
        case BlendMode::Darken:
            return QPainter::CompositionMode_Darken;
        case BlendMode::Lighten:
            return QPainter::CompositionMode_Lighten;
        case BlendMode::ColorDodge:
            return QPainter::CompositionMode_ColorDodge;
        case BlendMode::ColorBurn:
            return QPainter::CompositionMode_ColorBurn;
        case BlendMode::HardLight:
            return QPainter::CompositionMode_HardLight;
        case BlendMode::SoftLight:
            return QPainter::CompositionMode_SoftLight;
        case BlendMode::Difference:
            return QPainter::CompositionMode_Difference;
        case BlendMode::Exclusion:
            return QPainter::CompositionMode_Exclusion;
        case BlendMode::Compatible:
            return QPainter::CompositionMode_SourceOver;

        default:
            break;
    }

    return QPainter::CompositionMode_SourceOver;
}

QString PDFBlendModeInfo::getBlendModeName(BlendMode mode)
{
    for (const std::pair<const char*, BlendMode>& info : BLEND_MODE_INFOS)
    {
        if (info.second == mode)
        {
            return QString::fromLatin1(info.first);
        }
    }

    return "Unknown";
}

QString PDFBlendModeInfo::getBlendModeTranslatedName(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Normal:
        case BlendMode::Compatible:
            return PDFTranslationContext::tr("Normal");
        case BlendMode::Multiply:
            return PDFTranslationContext::tr("Multiply");
        case BlendMode::Screen:
            return PDFTranslationContext::tr("Screen");
        case BlendMode::Overlay:
            return PDFTranslationContext::tr("Overlay");
        case BlendMode::Darken:
            return PDFTranslationContext::tr("Darken");
        case BlendMode::Lighten:
            return PDFTranslationContext::tr("Lighten");
        case BlendMode::ColorDodge:
            return PDFTranslationContext::tr("ColorDodge");
        case BlendMode::ColorBurn:
            return PDFTranslationContext::tr("ColorBurn");
        case BlendMode::HardLight:
            return PDFTranslationContext::tr("HardLight");
        case BlendMode::SoftLight:
            return PDFTranslationContext::tr("SoftLight");
        case BlendMode::Difference:
            return PDFTranslationContext::tr("Difference");
        case BlendMode::Exclusion:
            return PDFTranslationContext::tr("Exclusion");
        case BlendMode::Hue:
            return PDFTranslationContext::tr("Hue");
        case BlendMode::Saturation:
            return PDFTranslationContext::tr("Saturation");
        case BlendMode::Color:
            return PDFTranslationContext::tr("Color");
        case BlendMode::Luminosity:
            return PDFTranslationContext::tr("Luminosity");

        default:
            break;
    }

    return PDFTranslationContext::tr("Unknown");
}

std::vector<BlendMode> PDFBlendModeInfo::getBlendModes()
{
    return
    {
        BlendMode::Normal,
        BlendMode::Multiply,
        BlendMode::Screen,
        BlendMode::Overlay,
        BlendMode::Darken,
        BlendMode::Lighten,
        BlendMode::ColorDodge,
        BlendMode::ColorBurn,
        BlendMode::HardLight,
        BlendMode::SoftLight,
        BlendMode::Difference,
        BlendMode::Exclusion,
        BlendMode::Hue,
        BlendMode::Saturation,
        BlendMode::Color,
        BlendMode::Luminosity
    };
}

PDFColorComponent PDFBlendFunction::blend(BlendMode mode, PDFColorComponent Cb, PDFColorComponent Cs)
{
    switch (mode)
    {
        case BlendMode::Normal:
        case BlendMode::Compatible:
            return Cs;

        case BlendMode::Multiply:
            return Cb * Cs;

        case BlendMode::Screen:
            return Cb + Cs - Cb * Cs;

        case BlendMode::Overlay:
            return blend(BlendMode::HardLight, Cs, Cb);

        case BlendMode::Darken:
            return qMin(Cb, Cs);

        case BlendMode::Lighten:
            return qMax(Cb, Cs);

        case BlendMode::ColorDodge:
        {
            if (qFuzzyIsNull(Cb))
            {
                return 0.0f;
            }

            const PDFColorComponent CsInverted = 1.0f - Cs;
            if (Cb >= CsInverted)
            {
                return 1.0f;
            }

            return Cb / CsInverted;
        }

        case BlendMode::ColorBurn:
        {
            const PDFColorComponent CbInverted = 1.0f - Cb;
            if (qFuzzyIsNull(CbInverted))
            {
                return 1.0f;
            }

            if (CbInverted >= Cs)
            {
                return 0.0f;
            }

            return 1.0f - CbInverted / Cs;
        }

        case BlendMode::HardLight:
        {
            if (Cs <= 0.5f)
            {
                return blend(BlendMode::Multiply, Cb, 2.0f * Cs);
            }
            else
            {
                return blend(BlendMode::Screen, Cb, 2.0f * Cs - 1.0f);
            }
        }

        case BlendMode::SoftLight:
        {
            if (Cs <= 0.5f)
            {
                return Cb - (1.0f - 2.0f * Cs) * Cb * (1.0f - Cb);
            }
            else
            {
                PDFColorComponent D = 0.0f;
                if (Cb <= 0.25)
                {
                    D = ((16.0f * Cb - 12.0f) * Cb + 4.0f) * Cb;
                }
                else
                {
                    D = std::sqrt(Cb);
                }
                return Cb + (2.0f * Cs - 1.0f) * (D - Cb);
            }
        }

        case BlendMode::Difference:
            return qAbs(Cb - Cs);

        case BlendMode::Exclusion:
            return Cb + Cs - 2.0f * Cb * Cs;

        case BlendMode::Overprint_SelectBackdrop:
            return Cb;

        case BlendMode::Overprint_SelectNonZeroSourceOrBackdrop:
        {
            if (qFuzzyIsNull(Cs))
            {
                return Cb;
            }

            return Cs;
        }

        case BlendMode::Overprint_SelectNonOneSourceOrBackdrop:
        {
            if (qFuzzyIsNull(1.0f - Cs))
            {
                return Cb;
            }

            return Cs;
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return Cs;
}

PDFRGB PDFBlendFunction::blend_Hue(PDFRGB Cb, PDFRGB Cs)
{
    return nonseparable_SetLum(nonseparable_SetSat(Cs, nonseparable_Sat(Cb)), nonseparable_Lum(Cb));
}

PDFRGB PDFBlendFunction::blend_Saturation(PDFRGB Cb, PDFRGB Cs)
{
    return nonseparable_SetLum(nonseparable_SetSat(Cb, nonseparable_Sat(Cs)), nonseparable_Lum(Cb));
}

PDFRGB PDFBlendFunction::blend_Color(PDFRGB Cb, PDFRGB Cs)
{
    return nonseparable_SetLum(Cs, nonseparable_Lum(Cb));
}

PDFRGB PDFBlendFunction::blend_Luminosity(PDFRGB Cb, PDFRGB Cs)
{
    return nonseparable_SetLum(Cb, nonseparable_Lum(Cs));
}

PDFGray PDFBlendFunction::blend_Hue(PDFGray Cb, PDFGray Cs)
{
    return nonseparable_rgb2gray(blend_Hue(nonseparable_gray2rgb(Cb), nonseparable_gray2rgb(Cs)));
}

PDFGray PDFBlendFunction::blend_Saturation(PDFGray Cb, PDFGray Cs)
{
    return nonseparable_rgb2gray(blend_Saturation(nonseparable_gray2rgb(Cb), nonseparable_gray2rgb(Cs)));
}

PDFGray PDFBlendFunction::blend_Color(PDFGray Cb, PDFGray Cs)
{
    return nonseparable_rgb2gray(blend_Color(nonseparable_gray2rgb(Cb), nonseparable_gray2rgb(Cs)));
}

PDFGray PDFBlendFunction::blend_Luminosity(PDFGray Cb, PDFGray Cs)
{
    return nonseparable_rgb2gray(blend_Luminosity(nonseparable_gray2rgb(Cb), nonseparable_gray2rgb(Cs)));
}

PDFCMYK PDFBlendFunction::blend_Hue(PDFCMYK Cb, PDFCMYK Cs)
{
    return nonseparable_rgb2cmyk(blend_Hue(nonseparable_cmyk2rgb(Cb), nonseparable_cmyk2rgb(Cs)), Cb[3]);
}

PDFCMYK PDFBlendFunction::blend_Saturation(PDFCMYK Cb, PDFCMYK Cs)
{
    return nonseparable_rgb2cmyk(blend_Saturation(nonseparable_cmyk2rgb(Cb), nonseparable_cmyk2rgb(Cs)), Cb[3]);
}

PDFCMYK PDFBlendFunction::blend_Color(PDFCMYK Cb, PDFCMYK Cs)
{
    return nonseparable_rgb2cmyk(blend_Color(nonseparable_cmyk2rgb(Cb), nonseparable_cmyk2rgb(Cs)), Cb[3]);
}

PDFCMYK PDFBlendFunction::blend_Luminosity(PDFCMYK Cb, PDFCMYK Cs)
{
    return nonseparable_rgb2cmyk(blend_Luminosity(nonseparable_cmyk2rgb(Cb), nonseparable_cmyk2rgb(Cs)), Cs[3]);
}

PDFGray PDFBlendFunction::blend_Nonseparable(BlendMode mode, PDFGray Cb, PDFGray Cs)
{
    switch (mode)
    {
        case BlendMode::Hue:
            return blend_Hue(Cb, Cs);

        case BlendMode::Saturation:
            return blend_Saturation(Cb, Cs);

        case BlendMode::Color:
            return blend_Color(Cb, Cs);

        case BlendMode::Luminosity:
            return blend_Luminosity(Cb, Cs);

        default:
            Q_ASSERT(false);
            break;
    }

    return Cs;
}

PDFRGB PDFBlendFunction::blend_Nonseparable(BlendMode mode, PDFRGB Cb, PDFRGB Cs)
{
    switch (mode)
    {
        case BlendMode::Hue:
            return blend_Hue(Cb, Cs);

        case BlendMode::Saturation:
            return blend_Saturation(Cb, Cs);

        case BlendMode::Color:
            return blend_Color(Cb, Cs);

        case BlendMode::Luminosity:
            return blend_Luminosity(Cb, Cs);

        default:
            Q_ASSERT(false);
            break;
    }

    return Cs;
}

PDFCMYK PDFBlendFunction::blend_Nonseparable(BlendMode mode, PDFCMYK Cb, PDFCMYK Cs)
{
    switch (mode)
    {
        case BlendMode::Hue:
            return blend_Hue(Cb, Cs);

        case BlendMode::Saturation:
            return blend_Saturation(Cb, Cs);

        case BlendMode::Color:
            return blend_Color(Cb, Cs);

        case BlendMode::Luminosity:
            return blend_Luminosity(Cb, Cs);

        default:
            Q_ASSERT(false);
            break;
    }

    return Cs;
}

PDFColorComponent PDFBlendFunction::getLuminosity(PDFGray gray)
{
    return nonseparable_Lum(nonseparable_gray2rgb(gray));
}

PDFColorComponent PDFBlendFunction::getLuminosity(PDFRGB rgb)
{
    return nonseparable_Lum(rgb);
}

PDFColorComponent PDFBlendFunction::getLuminosity(PDFCMYK cmyk)
{
    // This is according to chapter 11.5.3, deriving soft mask from a group luminosity
    return 1.0f - qMin(1.0f, 0.30f * cmyk[0] + 0.59f * cmyk[1] + 0.11f * cmyk[2] + cmyk[3]);
}

PDFRGB PDFBlendFunction::nonseparable_gray2rgb(PDFGray gray)
{
    return nonseparable_SetLum(PDFRGB{ 0.0f, 0.0f, 0.0f }, gray);
}

PDFGray PDFBlendFunction::nonseparable_rgb2gray(PDFRGB rgb)
{
    // Just convert to luminosity
    return nonseparable_Lum(rgb);
}

PDFRGB PDFBlendFunction::nonseparable_cmyk2rgb(PDFCMYK cmyk)
{
    return PDFRGB{ 1.0f - cmyk[0], 1.0f - cmyk[1], 1.0f - cmyk[2] };
}

PDFCMYK PDFBlendFunction::nonseparable_rgb2cmyk(PDFRGB rgb, PDFColorComponent K)
{
    return PDFCMYK{ 1.0f - rgb[0], 1.0f - rgb[1], 1.0f - rgb[2], K };
}

PDFColorComponent PDFBlendFunction::nonseparable_Lum(PDFRGB rgb)
{
    return 0.30 * rgb[0] + 0.59 * rgb[1] + 0.11 * rgb[2];
}

PDFColorComponent PDFBlendFunction::nonseparable_Sat(PDFRGB rgb)
{
    const PDFColorComponent min = *std::min_element(rgb.cbegin(), rgb.cend());
    const PDFColorComponent max = *std::max_element(rgb.cbegin(), rgb.cend());
    return max - min;
}

PDFRGB PDFBlendFunction::nonseparable_SetLum(PDFRGB C, PDFColorComponent l)
{
    const PDFColorComponent d = l - nonseparable_Lum(C);
    PDFRGB result = C;
    result[0] += d;
    result[1] += d;
    result[2] += d;
    return nonseparable_ClipColor(result);
}

PDFRGB PDFBlendFunction::nonseparable_SetSat(PDFRGB C, PDFColorComponent s)
{
    auto it_min = std::min_element(C.begin(), C.end());
    auto it_max = std::max_element(C.begin(), C.end());
    auto it_mid = C.end();
    for (auto it = C.begin(); it != C.end(); ++it)
    {
        if (it != it_min && it != it_max)
        {
            it_mid = it;
            break;
        }
    }
    Q_ASSERT(it_mid != C.end());

    PDFRGB result = C;
    if (*it_max > *it_min)
    {
        *it_mid = (*it_mid - *it_min) * s / (*it_max - *it_min);
        *it_max = s;
        result = C;
    }
    else
    {
        std::fill(result.begin(), result.end(), 0.0f);
    }

    return result;
}

PDFRGB PDFBlendFunction::nonseparable_ClipColor(PDFRGB C)
{
    PDFRGB result = C;
    const PDFColorComponent l = nonseparable_Lum(C);
    const PDFColorComponent n = *std::min_element(C.cbegin(), C.cend());
    const PDFColorComponent x = *std::max_element(C.cbegin(), C.cend());

    if (n < 0.0f)
    {
        const PDFColorComponent factor = 1.0f / (l - n);
        result[0] = l + (result[0] - l) * l * factor;
        result[1] = l + (result[1] - l) * l * factor;
        result[2] = l + (result[2] - l) * l * factor;
    }

    if (x > 1.0f)
    {
        const PDFColorComponent factor = 1.0f / (x - l);
        result[0] = l + (result[0] - l) * (1.0f - l) * factor;
        result[1] = l + (result[1] - l) * (1.0f - l) * factor;
        result[2] = l + (result[2] - l) * (1.0f - l) * factor;
    }

    return result;
}

}   // namespace pdf
