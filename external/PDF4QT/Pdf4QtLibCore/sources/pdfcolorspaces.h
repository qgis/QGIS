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

#ifndef PDFCOLORSPACES_H
#define PDFCOLORSPACES_H

#include "pdfflatarray.h"
#include "pdffunction.h"
#include "pdfutils.h"
#include "pdfoperationcontrol.h"

#include <QColor>
#include <QImage>
#include <QSharedPointer>

#include <set>

namespace pdf
{
class PDFCMS;
class PDFArray;
class PDFObject;
class PDFStream;
class PDFPattern;
class PDFDocument;
class PDFDictionary;
class PDFAbstractColorSpace;
class PDFPatternColorSpace;
class PDFRenderErrorReporter;

using PDFColor = PDFFlatArray<PDFColorComponent, 4>;
using PDFColorSpacePointer = QSharedPointer<PDFAbstractColorSpace>;
using PDFColorBuffer = PDFBuffer<PDFColorComponent>;
using PDFConstColorBuffer = PDFBuffer<const PDFColorComponent>;

static constexpr const int COLOR_SPACE_MAX_LEVEL_OF_RECURSION = 12;

static constexpr const char* COLOR_SPACE_DICTIONARY = "ColorSpace";

static constexpr const char* COLOR_SPACE_NAME_DEVICE_GRAY = "DeviceGray";
static constexpr const char* COLOR_SPACE_NAME_DEVICE_RGB = "DeviceRGB";
static constexpr const char* COLOR_SPACE_NAME_DEVICE_CMYK = "DeviceCMYK";

static constexpr const char* COLOR_SPACE_NAME_ABBREVIATION_DEVICE_GRAY = "G";
static constexpr const char* COLOR_SPACE_NAME_ABBREVIATION_DEVICE_RGB = "RGB";
static constexpr const char* COLOR_SPACE_NAME_ABBREVIATION_DEVICE_CMYK = "CMYK";
static constexpr const char* COLOR_SPACE_NAME_ABBREVIATION_CAL_CMYK = "CalCMYK";

static constexpr const char* COLOR_SPACE_NAME_DEFAULT_GRAY = "DefaultGray";
static constexpr const char* COLOR_SPACE_NAME_DEFAULT_RGB = "DefaultRGB";
static constexpr const char* COLOR_SPACE_NAME_DEFAULT_CMYK = "DefaultCMYK";

static constexpr const char* COLOR_SPACE_NAME_CAL_GRAY = "CalGray";
static constexpr const char* COLOR_SPACE_NAME_CAL_RGB = "CalRGB";
static constexpr const char* COLOR_SPACE_NAME_LAB = "Lab";
static constexpr const char* COLOR_SPACE_NAME_ICCBASED = "ICCBased";
static constexpr const char* COLOR_SPACE_NAME_INDEXED = "Indexed";
static constexpr const char* COLOR_SPACE_NAME_SEPARATION = "Separation";
static constexpr const char* COLOR_SPACE_NAME_DEVICE_N = "DeviceN";
static constexpr const char* COLOR_SPACE_NAME_PATTERN = "Pattern";

static constexpr const char* CAL_WHITE_POINT = "WhitePoint";
static constexpr const char* CAL_BLACK_POINT = "BlackPoint";
static constexpr const char* CAL_GAMMA = "Gamma";
static constexpr const char* CAL_MATRIX = "Matrix";
static constexpr const char* CAL_RANGE = "Range";

static constexpr const char* ICCBASED_ALTERNATE = "Alternate";
static constexpr const char* ICCBASED_N = "N";
static constexpr const char* ICCBASED_RANGE = "Range";

enum class BlackPointCompensationMode
{
    Default,
    ON,
    OFF
};

/// Image raw data - containing data for image. Image data are row-ordered, and by components.
/// So the row can be for 3-components RGB like 'RGBRGBRGB...RGB', where size of row in bytes is 3 * width of image.
class PDFImageData
{
public:

    enum class MaskingType
    {
        None,
        ColorKeyMasking,    ///< Masking by color key
        ImageMask,          ///< Masking by 1-bit image (see "ImageMask" entry in image's dictionary), current color from the graphic state is used to paint an image
        SoftMask,           ///< Image is masked by soft mask
    };

    explicit PDFImageData() :
        m_components(0),
        m_bitsPerComponent(0),
        m_width(0),
        m_height(0),
        m_stride(0),
        m_maskingType(MaskingType::None)
    {

    }

    explicit inline PDFImageData(unsigned int components,
                                 unsigned int bitsPerComponent,
                                 unsigned int width,
                                 unsigned int height,
                                 unsigned int stride,
                                 MaskingType maskingType,
                                 QByteArray data,
                                 std::vector<PDFInteger>&& colorKeyMask,
                                 std::vector<PDFReal>&& decode,
                                 std::vector<PDFReal>&& matte) :
        m_components(components),
        m_bitsPerComponent(bitsPerComponent),
        m_width(width),
        m_height(height),
        m_stride(stride),
        m_maskingType(maskingType),
        m_data(qMove(data)),
        m_colorKeyMask(qMove(colorKeyMask)),
        m_decode(qMove(decode)),
        m_matte(qMove(matte))
    {

    }

    unsigned int getComponents() const { return m_components; }
    unsigned int getBitsPerComponent() const { return m_bitsPerComponent; }
    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }
    unsigned int getStride() const { return m_stride; }
    MaskingType getMaskingType() const { return m_maskingType; }
    const QByteArray& getData() const { return m_data; }
    const std::vector<PDFInteger>& getColorKeyMask() const { return m_colorKeyMask; }
    const std::vector<PDFReal>& getDecode() const { return m_decode; }
    const std::vector<PDFReal>& getMatte() const { return m_matte; }

    void setMaskingType(MaskingType maskingType) { m_maskingType = maskingType; }
    void setDecode(std::vector<PDFReal> decode) { m_decode = qMove(decode); }

    /// Returns number of color channels
    unsigned int getColorChannels() const { return m_components; }

    bool isValid() const { return m_width && m_height && m_components && m_bitsPerComponent; }

    const unsigned char* getRow(unsigned int rowIndex) const;

private:
    unsigned int m_components;
    unsigned int m_bitsPerComponent;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
    MaskingType m_maskingType;

    QByteArray m_data;

    /// Mask entry of the image. If it is empty, no color key masking is induced.
    /// If it is not empty, then it should contain 2 x number of color components,
    /// consisting of [ min_0, max_0, min_1, max_1, ... , min_n, max_n ].
    std::vector<PDFInteger> m_colorKeyMask;

    /// Decode array. If it is empty, then no decoding is performed. If it is nonempty,
    /// then contains n pairs of numbers, where n is number of color components. If ImageMask
    /// in the image dictionary is true, then decode array should be [0 1] or [1 0].
    std::vector<PDFReal> m_decode;

    /// Matte color (color, agains which is image preblended, when using soft masking
    /// image (defined for soft masks).
    std::vector<PDFReal> m_matte;
};

using PDFColor3 = std::array<PDFColorComponent, 3>;

/// Matrix for color component multiplication (for example, conversion between some color spaces)
template<size_t Rows, size_t Cols>
class PDFColorComponentMatrix
{
public:
    explicit constexpr inline PDFColorComponentMatrix() : m_values() { }

    template<typename... Components>
    explicit constexpr inline PDFColorComponentMatrix(Components... components) : m_values({ static_cast<PDFColorComponent>(components)... }) { }

    bool operator==(const PDFColorComponentMatrix&) const = default;
    bool operator!=(const PDFColorComponentMatrix&) const = default;

    std::array<PDFColorComponent, Rows> operator*(const std::array<PDFColorComponent, Cols>& color) const
    {
        std::array<PDFColorComponent, Rows> result = { };

        for (size_t row = 0; row < Rows; ++row)
        {
            for (size_t column = 0; column < Cols; ++column)
            {
                result[row] += m_values[row * Cols + column] * color[column];
            }
        }

        return result;
    }

    inline PDFColorComponent getValue(size_t row, size_t column) const
    {
        return m_values[row * Cols + column];
    }
    inline void setValue(size_t row, size_t column, PDFColorComponent value)
    {
        m_values[row * Cols + column] = value;
    }

    void transpose()
    {
        Q_ASSERT(Rows == Cols);

        for (size_t row = 0; row < Rows; ++row)
        {
            for (size_t column = row; column < Cols; ++column)
            {
                const size_t index1 = row * Cols + column;
                const size_t index2 = column * Cols + row;
                std::swap(m_values[index1], m_values[index2]);
            }
        }
    }

    void makeIdentity()
    {
        Q_ASSERT(Rows == Cols);

        m_values = { };

        for (size_t row = 0; row < Rows; ++row)
        {
            const size_t index = row * Cols + row;
            m_values[index] = PDFColorComponent(1.0f);
        }
    }

    void makeDiagonal(auto diagonalItems)
    {
        Q_ASSERT(Rows == Cols);
        Q_ASSERT(diagonalItems.size() == Rows);

        m_values = { };

        for (size_t row = 0; row < Rows; ++row)
        {
            const size_t index = row * Cols + row;
            m_values[index] = diagonalItems[row];
        }
    }

    void multiplyByFactor(PDFColorComponent factor)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            *it *= factor;
        }
    }

    inline typename std::array<PDFColorComponent, Rows * Cols>::iterator begin() { return m_values.begin(); }
    inline typename std::array<PDFColorComponent, Rows * Cols>::iterator end() { return m_values.end(); }

private:
    std::array<PDFColorComponent, Rows * Cols> m_values;
};

template<size_t i, size_t k, size_t j>
static inline PDFColorComponentMatrix<i, j> operator*(const PDFColorComponentMatrix<i, k>& left, const PDFColorComponentMatrix<k, j>& right)
{
    PDFColorComponentMatrix<i, j> result;

    for (size_t ci = 0; ci < i; ++ci)
    {
        for (size_t cj = 0; cj < j; ++cj)
        {
            PDFColorComponent value = 0.0f;

            for (size_t ck = 0; ck < k; ++ck)
            {
                value += left.getValue(ci, ck) * right.getValue(ck, cj);
            }

            result.setValue(ci, cj, value);
        }
    }

    return result;
}

using PDFColorComponentMatrix_3x3 = PDFColorComponentMatrix<3, 3>;

/// Represents PDF's color space (abstract class). Contains functions for parsing
/// color spaces.
class PDF4QTLIBCORESHARED_EXPORT PDFAbstractColorSpace
{
public:
    explicit PDFAbstractColorSpace() = default;
    virtual ~PDFAbstractColorSpace() = default;

    enum class ColorSpace
    {
        DeviceGray,
        DeviceRGB,
        DeviceCMYK,
        CalGray,
        CalRGB,
        Lab,
        ICCBased,
        Indexed,
        Separation,
        DeviceN,
        Pattern,
        Invalid
    };

    /// Returns color space identification
    virtual ColorSpace getColorSpace() const = 0;

    /// Returns true, if two color spaces are equal
    virtual bool equals(const PDFAbstractColorSpace* other) const;

    /// Returns true, if this color space can be used for blending
    bool isBlendColorSpace() const;

    /// Returns default color for the color space
    virtual QColor getDefaultColor(const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const;

    /// Returns default color in original color space (not transformed to QColor)
    virtual PDFColor getDefaultColorOriginal() const = 0;

    /// Returns transformed color for given input color. Color is transformed using color
    /// management system (cms), if color management system fails, and returns invalid color,
    /// then generic solution for color transformation is used (which is often not valid).
    /// Caller can also specify rendering intent and error reporter, where color management
    /// system can write errors during color transformation.
    /// \param color Input color
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param reporter Error reporter
    /// \param isRange01 Is range [0, 1], or native color component range (for Lab spaces)?
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const = 0;

    /// Returns color component count
    virtual size_t getColorComponentCount() const = 0;

    /// Transforms image data to result image.
    /// \param imageData Image data
    /// \param softMask Soft mask (for alpha channel)
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param reporter Error reporter
    /// \param operationControl Operation control
    virtual QImage getImage(const PDFImageData& imageData,
                            const PDFImageData& softMask,
                            const PDFCMS* cms,
                            RenderingIntent intent,
                            PDFRenderErrorReporter* reporter,
                            const PDFOperationControl* operationControl) const;

    /// Fills RGB buffer using colors from \p colors. Colors are transformed
    /// by this color space (or color management system is used). Buffer
    /// must be big enough to contain all 8-bit RGB data.
    /// \param Colors Input color buffer
    /// \param intent Rendering intent
    /// \param outputBuffer 8-bit RGB output buffer
    /// \param cms Color management system
    /// \param reporter Render error reporter
    virtual void fillRGBBuffer(const std::vector<float>& colors,
                               unsigned char* outputBuffer,
                               RenderingIntent intent,
                               const PDFCMS* cms,
                               PDFRenderErrorReporter* reporter) const;

    /// If this class is pattern space, returns this, otherwise returns nullptr.
    virtual const PDFPatternColorSpace* asPatternColorSpace() const { return nullptr; }

    /// Checks, if number of color components is OK, and if yes, converts them to the QColor value.
    /// If they are not OK, exception is thrown. \sa getColor
    /// \param color Input color
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param reporter Error reporter
    QColor getCheckedColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const;

    /// Creates alpha mask from soft image data. Exception is thrown, if something fails.
    /// \param softMask Soft mask
    static QImage createAlphaMask(const PDFImageData& softMask);

    /// Parses the desired color space. If desired color space is not found, then exception is thrown.
    /// If everything is OK, then shared pointer to the new color space is returned.
    /// \param colorSpaceDictionary Dictionary containing color spaces of the page
    /// \param document Document (for loading objects)
    /// \param colorSpace Identification of color space (either name or array), must be a direct object
    static PDFColorSpacePointer createColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                 const PDFDocument* document,
                                                 const PDFObject& colorSpace);

    /// Creates device color space by name. Color space can be created by this function only, if
    /// it is simple - one of the basic device color spaces (gray, RGB or CMYK).
    /// \param colorSpaceDictionary Dictionary containing color spaces of the page
    /// \param document Document (for loading objects)
    /// \param name Name of the color space
    static PDFColorSpacePointer createDeviceColorSpaceByName(const PDFDictionary* colorSpaceDictionary,
                                                             const PDFDocument* document,
                                                             const QByteArray& name);

    /// Converts a vector of real numbers to the PDFColor
    static PDFColor convertToColor(const std::vector<PDFReal>& components);

    /// Returns true, if two colors are equal (considering the tolerance). So, if one
    /// of the color components differs more than \p tolerance from the another, then
    /// false is returned. If colors have different number of components, false is returned.
    /// \param color1 First color
    /// \param color2 Second color
    /// \param tolerance Color tolerance
    static bool isColorEqual(const PDFColor& color1, const PDFColor& color2, PDFReal tolerance);

    /// Mix colors according the given ratio.
    /// \param color1 First color
    /// \param color2 Second color
    /// \param ratio Mixing ratio
    static PDFColor mixColors(const PDFColor& color1, const PDFColor& color2, PDFReal ratio);

    /// Transforms color from source color space to target color space. Target color space
    /// must be blend color space.
    /// \param source Source color space
    /// \param target Target color space (must be blend color space)
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param input Input color buffer
    /// \param output Output color buffer, must match size of input  color buffer
    /// \param reporter Error reporter
    static bool transform(const PDFAbstractColorSpace* source,
                          const PDFAbstractColorSpace* target,
                          const PDFCMS* cms,
                          RenderingIntent intent,
                          const PDFColorBuffer input,
                          PDFColorBuffer output,
                          PDFRenderErrorReporter* reporter);

protected:
    /// Clips the color component to range [0, 1]
    static constexpr PDFColorComponent clip01(PDFColorComponent component) { return qBound<PDFColorComponent>(PDFColorComponent(0.0), component, PDFColorComponent(1.0)); }

    /// Clips the color to range [0 1] in all components
    static constexpr PDFColor3 clip01(const PDFColor3& color)
    {
        PDFColor3 result = color;

        for (PDFColorComponent& component : result)
        {
            component = clip01(component);
        }

        return result;
    }

    /// Parses the desired color space. If desired color space is not found, then exception is thrown.
    /// If everything is OK, then shared pointer to the new color space is returned.
    /// \param colorSpaceDictionary Dictionary containing color spaces of the page
    /// \param document Document (for loading objects)
    /// \param colorSpace Identification of color space (either name or array), must be a direct object
    /// \param recursion Recursion guard
    /// \param usedNames Names, which were already parsed
    static PDFColorSpacePointer createColorSpaceImpl(const PDFDictionary* colorSpaceDictionary,
                                                     const PDFDocument* document,
                                                     const PDFObject& colorSpace,
                                                     int recursion,
                                                     std::set<QByteArray>& usedNames);

    /// Creates device color space by name. Color space can be created by this function only, if
    /// it is simple - one of the basic device color spaces (gray, RGB or CMYK).
    /// \param colorSpaceDictionary Dictionary containing color spaces of the page
    /// \param document Document (for loading objects)
    /// \param name Name of the color space
    /// \param usedNames Names, which were already parsed
    static PDFColorSpacePointer createDeviceColorSpaceByNameImpl(const PDFDictionary* colorSpaceDictionary,
                                                                 const PDFDocument* document,
                                                                 const QByteArray& name,
                                                                 int recursion,
                                                                 std::set<QByteArray>& usedNames);

    /// Converts XYZ value to the standard RGB value (linear). No gamma correction is applied.
    /// Default transformation matrix is applied.
    /// \param xyzColor Color in XYZ space
    static PDFColor3 convertXYZtoRGB(const PDFColor3& xyzColor);

    /// Multiplies color by factor
    /// \param color Color to be multiplied
    /// \param factor Multiplication factor
    static constexpr PDFColor3 colorMultiplyByFactor(const PDFColor3& color, PDFColorComponent factor)
    {
        PDFColor3 result = color;
        for (PDFColorComponent& component : result)
        {
            component *= factor;
        }
        return result;
    }

    /// Multiplies color by factors (stored in components of the color)
    /// \param color Color to be multiplied
    /// \param factor Multiplication factors
    static constexpr PDFColor3 colorMultiplyByFactors(const PDFColor3& color, const PDFColor3& factors)
    {
        PDFColor3 result = { };
        for (size_t i = 0; i < color.size(); ++i)
        {
            result[i] = color[i] * factors[i];
        }
        return result;
    }

    /// Powers color by factors (stored in components of the color)
    /// \param color Color to be multiplied
    /// \param factor Power factors
    static constexpr PDFColor3 colorPowerByFactors(const PDFColor3& color, const PDFColor3& factors)
    {
        PDFColor3 result = { };
        for (size_t i = 0; i < color.size(); ++i)
        {
            result[i] = std::pow(color[i], factors[i]);
        }
        return result;
    }

    /// Converts RGB values of range [0.0, 1.0] to standard QColor
    /// \param color Color to be converted
    static inline QColor fromRGB01(const PDFColor3& color)
    {
        PDFColorComponent r = clip01(color[0]);
        PDFColorComponent g = clip01(color[1]);
        PDFColorComponent b = clip01(color[2]);

        QColor result(QColor::Rgb);
        result.setRgbF(r, g, b, 1.0);
        return result;
    }
};

class PDFDeviceGrayColorSpace : public PDFAbstractColorSpace
{
public:
    explicit PDFDeviceGrayColorSpace() = default;
    virtual ~PDFDeviceGrayColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::DeviceGray; }
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors,unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;
};

class PDFDeviceRGBColorSpace : public PDFAbstractColorSpace
{
public:
    explicit PDFDeviceRGBColorSpace() = default;
    virtual ~PDFDeviceRGBColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::DeviceRGB; }
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors,unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;
};

class PDFDeviceCMYKColorSpace : public PDFAbstractColorSpace
{
public:
    explicit PDFDeviceCMYKColorSpace() = default;
    virtual ~PDFDeviceCMYKColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::DeviceCMYK; }
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors,unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;
};

class PDFXYZColorSpace : public PDFAbstractColorSpace
{
public:
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual bool equals(const PDFAbstractColorSpace* other) const override;

    const PDFColor3& getWhitePoint() const { return m_whitePoint; }
    const PDFColor3& getCorrectionCoefficients() const;

protected:
    explicit PDFXYZColorSpace(PDFColor3 whitePoint);
    virtual ~PDFXYZColorSpace() = default;

    PDFColor3 m_whitePoint;

    /// What are these coefficients? We want to map white point from XYZ space to white point
    /// of RGB space. These coefficients are reciprocal values to the point converted from XYZ white
    /// point. So, if we call getColor(m_whitePoint), then we should get vector (1.0, 1.0, 1.0)
    /// after multiplication by these coefficients.
    PDFColor3 m_correctionCoefficients;
};

class PDFCalGrayColorSpace : public PDFXYZColorSpace
{
public:
    explicit PDFCalGrayColorSpace(PDFColor3 whitePoint, PDFColor3 blackPoint, PDFColorComponent gamma);
    virtual ~PDFCalGrayColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::CalGray; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors,unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;

    PDFColorComponent getGamma() const { return m_gamma; }
    const PDFColor3& getBlackPoint() const  { return m_blackPoint; }

    /// Creates CalGray color space from provided values.
    /// \param document Document
    /// \param dictionary Dictionary
    static PDFColorSpacePointer createCalGrayColorSpace(const PDFDocument* document, const PDFDictionary* dictionary);

private:
    PDFColor3 m_blackPoint;
    PDFColorComponent m_gamma;
};

class PDFCalRGBColorSpace : public PDFXYZColorSpace
{
public:
    explicit PDFCalRGBColorSpace(PDFColor3 whitePoint, PDFColor3 blackPoint, PDFColor3 gamma, PDFColorComponentMatrix_3x3 matrix);
    virtual ~PDFCalRGBColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::CalRGB; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors,unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;

    /// Creates CalRGB color space from provided values.
    /// \param document Document
    /// \param dictionary Dictionary
    static PDFColorSpacePointer createCalRGBColorSpace(const PDFDocument* document, const PDFDictionary* dictionary);

    PDFColor3 getBlackPoint() const;
    PDFColor3 getGamma() const;
    const PDFColorComponentMatrix_3x3& getMatrix() const;

private:
    PDFColor3 m_blackPoint;
    PDFColor3 m_gamma;
    PDFColorComponentMatrix_3x3 m_matrix;
};

class PDFLabColorSpace : public PDFXYZColorSpace
{
public:
    explicit PDFLabColorSpace(PDFColor3 whitePoint, PDFColor3 blackPoint, PDFColorComponent aMin, PDFColorComponent aMax, PDFColorComponent bMin, PDFColorComponent bMax);
    virtual ~PDFLabColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::Lab; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors,unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;

    /// Creates Lab color space from provided values.
    /// \param document Document
    /// \param dictionary Dictionary
    static PDFColorSpacePointer createLabColorSpace(const PDFDocument* document, const PDFDictionary* dictionary);

    PDFColorComponent getAMin() const;
    PDFColorComponent getAMax() const;
    PDFColorComponent getBMin() const;
    PDFColorComponent getBMax() const;

    PDFColor3 getBlackPoint() const;

private:
    PDFColor3 m_blackPoint;
    PDFColorComponent m_aMin;
    PDFColorComponent m_aMax;
    PDFColorComponent m_bMin;
    PDFColorComponent m_bMax;
};

class PDFICCBasedColorSpace : public PDFAbstractColorSpace
{
public:
    static constexpr const size_t MAX_COLOR_COMPONENTS = 4;
    using Ranges = std::array<PDFColorComponent, MAX_COLOR_COMPONENTS * 2>;

    explicit PDFICCBasedColorSpace(PDFColorSpacePointer alternateColorSpace, Ranges range, QByteArray iccProfileData, PDFObjectReference metadata);
    virtual ~PDFICCBasedColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::ICCBased; }
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual void fillRGBBuffer(const std::vector<float>& colors, unsigned char* outputBuffer, RenderingIntent intent, const PDFCMS* cms, PDFRenderErrorReporter* reporter) const override;
    virtual bool equals(const PDFAbstractColorSpace* other) const override;

    PDFObjectReference getMetadata() const { return m_metadata; }

    /// Creates ICC based color space from provided values.
    /// \param colorSpaceDictionary Color space dictionary
    /// \param document Document
    /// \param stream Stream with ICC profile
    /// \param recursion Recursion guard
    /// \param usedNames Names, which were already parsed
    static PDFColorSpacePointer createICCBasedColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                         const PDFDocument* document,
                                                         const PDFStream* stream,
                                                         int recursion,
                                                         std::set<QByteArray>& usedNames);

    const Ranges& getRange() const;
    const QByteArray& getIccProfileData() const;
    const QByteArray& getIccProfileDataChecksum() const;
    const PDFAbstractColorSpace* getAlternateColorSpace() const;

private:
    PDFColorSpacePointer m_alternateColorSpace;
    Ranges m_range;
    QByteArray m_iccProfileData;
    QByteArray m_iccProfileDataChecksum;
    PDFObjectReference m_metadata;
};

class PDFIndexedColorSpace : public PDFAbstractColorSpace
{
public:
    explicit PDFIndexedColorSpace(PDFColorSpacePointer baseColorSpace, QByteArray&& colors, int maxValue);
    virtual ~PDFIndexedColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::Indexed; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual QImage getImage(const PDFImageData& imageData,
                            const PDFImageData& softMask,
                            const PDFCMS* cms,
                            RenderingIntent intent,
                            PDFRenderErrorReporter* reporter,
                            const PDFOperationControl* operationControl) const override;

    /// Creates indexed color space from provided values.
    /// \param colorSpaceDictionary Color space dictionary
    /// \param document Document
    /// \param array Array with indexed color space definition
    /// \param recursion Recursion guard
    /// \param usedNames Names, which were already parsed
    static PDFColorSpacePointer createIndexedColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                        const PDFDocument* document,
                                                        const PDFArray* array,
                                                        int recursion,
                                                        std::set<QByteArray>& usedNames);

    PDFColorSpacePointer getBaseColorSpace() const;
    std::vector<PDFColorComponent> transformColorsToBaseColorSpace(const PDFColorBuffer buffer) const;

    int getMaxValue() const;
    const QByteArray& getColors() const;

private:
    static constexpr const int MIN_VALUE = 0;
    static constexpr const int MAX_VALUE = 255;

    PDFColorSpacePointer m_baseColorSpace;
    QByteArray m_colors;
    int m_maxValue;
};

class PDFSeparationColorSpace : public PDFAbstractColorSpace
{
public:
    explicit PDFSeparationColorSpace(QByteArray&& colorName, PDFColorSpacePointer alternateColorSpace, PDFFunctionPtr tintTransform);
    virtual ~PDFSeparationColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::Separation; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;

    bool isNone() const { return m_isNone; }
    bool isAll() const { return m_isAll; }

    std::vector<PDFColorComponent> transformColorsToBaseColorSpace(const PDFColorBuffer buffer) const;

    /// Creates separation color space from provided values.
    /// \param colorSpaceDictionary Color space dictionary
    /// \param document Document
    /// \param array Array with separation color space definition
    /// \param recursion Recursion guard
    /// \param usedNames Names, which were already parsed
    static PDFColorSpacePointer createSeparationColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                           const PDFDocument* document,
                                                           const PDFArray* array,
                                                           int recursion,
                                                           std::set<QByteArray>& usedNames);

    PDFColorSpacePointer getAlternateColorSpace() const;
    const QByteArray& getColorName() const;

private:
    QByteArray m_colorName;
    PDFColorSpacePointer m_alternateColorSpace;
    PDFFunctionPtr m_tintTransform;
    bool m_isNone;
    bool m_isAll;
};

class PDFDeviceNColorSpace : public PDFAbstractColorSpace
{
public:

    enum class Type
    {
        DeviceN,
        NChannel
    };

    struct ColorantInfo
    {
        QByteArray name;
        PDFColorSpacePointer separationColorSpace;
        PDFReal solidity = 0.0;
        PDFFunctionPtr dotGain;
    };

    using Colorants = std::vector<ColorantInfo>;

    explicit PDFDeviceNColorSpace(Type type,
                                  Colorants&& colorants,
                                  PDFColorSpacePointer alternateColorSpace,
                                  PDFColorSpacePointer processColorSpace,
                                  PDFFunctionPtr tintTransform,
                                  std::vector<QByteArray>&& colorantsPrintingOrder,
                                  std::vector<QByteArray> processColorSpaceComponents);
    virtual ~PDFDeviceNColorSpace() = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::DeviceN; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;

    /// Returns type of DeviceN color space
    Type getType() const { return m_type; }

    const Colorants& getColorants() const { return m_colorants; }
    const PDFColorSpacePointer& getAlternateColorSpace() const { return m_alternateColorSpace; }
    const PDFColorSpacePointer& getProcessColorSpace() const { return m_processColorSpace; }
    const PDFFunctionPtr& getTintTransform() const { return m_tintTransform; }
    const std::vector<QByteArray>& getPrintingOrder() const { return m_colorantsPrintingOrder; }
    const std::vector<QByteArray>& getProcessColorSpaceComponents() const { return m_processColorSpaceComponents; }
    bool isNone() const { return m_isNone; }

    std::vector<PDFColorComponent> transformColorsToBaseColorSpace(const PDFColorBuffer buffer) const;

    /// Creates DeviceN color space from provided values.
    /// \param colorSpaceDictionary Color space dictionary
    /// \param document Document
    /// \param array Array with DeviceN color space definition
    /// \param recursion Recursion guard
    /// \param usedNames Names, which were already parsed
    static PDFColorSpacePointer createDeviceNColorSpace(const PDFDictionary* colorSpaceDictionary,
                                                        const PDFDocument* document,
                                                        const PDFArray* array,
                                                        int recursion,
                                                        std::set<QByteArray>& usedNames);

private:
    Type m_type;
    Colorants m_colorants;
    PDFColorSpacePointer m_alternateColorSpace;
    PDFColorSpacePointer m_processColorSpace;
    PDFFunctionPtr m_tintTransform;
    std::vector<QByteArray> m_colorantsPrintingOrder;
    std::vector<QByteArray> m_processColorSpaceComponents;
    bool m_isNone;
};

class PDFPatternColorSpace : public PDFAbstractColorSpace
{
public:
    explicit PDFPatternColorSpace(std::shared_ptr<PDFPattern>&& pattern, PDFColorSpacePointer&& uncoloredPatternColorSpace, PDFColor uncoloredPatternColor) :
        m_pattern(qMove(pattern)),
        m_uncoloredPatternColorSpace(qMove(uncoloredPatternColorSpace)),
        m_uncoloredPatternColor(qMove(uncoloredPatternColor))
    {

    }

    virtual ~PDFPatternColorSpace() override = default;

    virtual ColorSpace getColorSpace() const override { return ColorSpace::Pattern; }
    virtual bool equals(const PDFAbstractColorSpace* other) const override;
    virtual QColor getDefaultColor(const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual PDFColor getDefaultColorOriginal() const override;
    virtual QColor getColor(const PDFColor& color, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool isRange01) const override;
    virtual size_t getColorComponentCount() const override;
    virtual const PDFPatternColorSpace* asPatternColorSpace() const override { return this; }

    const PDFPattern* getPattern() const { return m_pattern.get(); }
    PDFColorSpacePointer getUncoloredPatternColorSpace() const { return m_uncoloredPatternColorSpace; }
    PDFColor getUncoloredPatternColor() const { return m_uncoloredPatternColor; }

private:
     std::shared_ptr<PDFPattern> m_pattern;
     PDFColorSpacePointer m_uncoloredPatternColorSpace;
     PDFColor m_uncoloredPatternColor;
};

}   // namespace pdf

#endif // PDFCOLORSPACES_H
