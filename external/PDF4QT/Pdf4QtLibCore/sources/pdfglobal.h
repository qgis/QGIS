//    Copyright (C) 2018-2021 Jakub Melka
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

#ifndef PDFGLOBAL_H
#define PDFGLOBAL_H

#include <QCoreApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtSystemDetection>
#include <QtCompilerDetection>
#endif

#include <limits>
#include <tuple>
#include <array>
#include <cmath>

#include <pdf4qtlibcore_export.h>

#if !defined(PDF4QTLIBCORESHARED_EXPORT)
#if defined(PDF4QTLIBCORE_LIBRARY)
#  define PDF4QTLIBCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define PDF4QTLIBCORESHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

// Compiler detection
#if defined(_MSC_VER)
#define PDF4QT_COMPILER_MSVC 1
#endif

#if defined(__clang__)
#define PDF4QT_COMPILER_CLANG 1
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#define PDF4QT_COMPILER_MINGW 1
#endif

#if defined(__GNUC__)
#define PDF4QT_COMPILER_GCC 1
#endif

#if defined(Q_OS_WIN) && (defined(PDF4QT_COMPILER_MSVC) || defined(PDF4QT_COMPILER_CLANG))
#define PDF4QT_USE_PRAGMA_LIB 1
#endif

#if defined(Q_OS_WIN) && defined(PDF4QT_COMPILER_MSVC) && !defined(NDEBUG)
#define PDF4QT_USE_DBG_HEAP
#endif

namespace pdf
{

using PDFInteger = int64_t;
using PDFReal = double;
using PDFColorComponent = float;
using PDFGray = PDFColorComponent;
using PDFRGB = std::array<PDFColorComponent, 3>;
using PDFCMYK = std::array<PDFColorComponent, 4>;

// These constants define minimum/maximum integer and are defined in such a way,
// that even 100 times bigger integers are representable.

constexpr PDFInteger PDF_INTEGER_MIN = std::numeric_limits<int64_t>::min() / 100;
constexpr PDFInteger PDF_INTEGER_MAX = std::numeric_limits<int64_t>::max() / 100;
constexpr PDFReal PDF_EPSILON = 0.000001;

static constexpr bool isValidInteger(PDFInteger integer)
{
    return integer >= PDF_INTEGER_MIN && integer <= PDF_INTEGER_MAX;
}

static inline bool isZero(PDFReal value)
{
    return std::fabs(value) < PDF_EPSILON;
}

/// This structure represents a reference to the object - consisting of the
/// object number, and generation number.
struct PDFObjectReference
{
    constexpr inline PDFObjectReference() :
        objectNumber(0),
        generation(0)
    {

    }

    constexpr inline PDFObjectReference(PDFInteger objectNumber, PDFInteger generation) :
        objectNumber(objectNumber),
        generation(generation)
    {

    }

    PDFInteger objectNumber;
    PDFInteger generation;

    constexpr bool operator==(const PDFObjectReference& other) const
    {
        return objectNumber == other.objectNumber && generation == other.generation;
    }

    constexpr bool operator!=(const PDFObjectReference& other) const { return !(*this == other); }

    constexpr bool operator<(const PDFObjectReference& other) const
    {
        return std::tie(objectNumber, generation) < std::tie(other.objectNumber, other.generation);
    }

    constexpr bool isValid() const { return objectNumber > 0; }
};

/// Represents version identification
struct PDFVersion
{
    constexpr explicit PDFVersion() = default;
    constexpr explicit PDFVersion(uint16_t major, uint16_t minor) :
        major(major),
        minor(minor)
    {

    }

    uint16_t major = 0;
    uint16_t minor = 0;

    bool isValid() const { return major > 0; }
};

struct PDFTranslationContext
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFTranslationContext)
};

constexpr PDFReal PDF_POINT_TO_INCH = 1.0 / 72.0;
constexpr PDFReal PDF_INCH_TO_MM = 25.4; // [mm / inch]
constexpr PDFReal PDF_POINT_TO_MM = PDF_POINT_TO_INCH * PDF_INCH_TO_MM;
constexpr PDFReal PDF_MM_TO_POINT = 1.0 / PDF_POINT_TO_MM;

/// This is default "DPI", but in milimeters, so the name is DPMM (device pixel per milimeter)
constexpr PDFReal PDF_DEFAULT_DPMM = 96.0 / PDF_INCH_TO_MM;

constexpr PDFReal convertPDFPointToMM(PDFReal point)
{
    return point * PDF_POINT_TO_MM;
}

constexpr PDFReal convertMMToPDFPoint(PDFReal point)
{
    return point * PDF_MM_TO_POINT;
}

class PDFBoolGuard final
{
public:
    inline explicit PDFBoolGuard(bool& value) :
        m_value(value),
        m_oldValue(value)
    {
        m_value = true;
    }

    inline ~PDFBoolGuard()
    {
        m_value = m_oldValue;
    }

private:
    bool& m_value;
    bool m_oldValue;
};

enum class RendererEngine
{
    Software,
    OpenGL
};

enum class RenderingIntent
{
    Auto,                   ///< Rendering intent is automatically selected
    Perceptual,
    AbsoluteColorimetric,
    RelativeColorimetric,
    Saturation,
    Unknown
};

}   // namespace pdf

#endif // PDFGLOBAL_H
