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

#ifndef PDFGLOBAL_H
#define PDFGLOBAL_H

#include <QtSystemDetection>
#include <QtCompilerDetection>
#include <QCoreApplication>

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
    Blend2D_MultiThread,
    Blend2D_SingleThread,
    QPainter,
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
