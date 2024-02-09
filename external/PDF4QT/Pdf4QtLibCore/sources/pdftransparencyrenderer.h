//    Copyright (C) 2021 Jakub Melka
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

#ifndef PDFTRANSPARENCYRENDERER_H
#define PDFTRANSPARENCYRENDERER_H

#include "pdfglobal.h"
#include "pdfcolorspaces.h"
#include "pdfpagecontentprocessor.h"
#include "pdfconstants.h"
#include "pdfutils.h"
#include "pdfprogress.h"

#include <QImage>

namespace pdf
{

/// Pixel format, describes color channels, both process colors (for example,
/// R, G, B, Gray, C, M, Y, K) or spot colors. Also, describes, if pixel
/// has shape channel and opacity channel. Two auxiliary channels are possible,
/// shape channel and opacity channel. Shape channel defines the shape (so, for
/// example, if we draw a rectangle onto the bitmap, shape value is 1.0 inside the
/// rectangle and 0.0 outside the rectangle). PDF transparency processing requires
/// shape and opacity values separated for correct transparency processing.
class PDFPixelFormat
{
public:
    inline explicit constexpr PDFPixelFormat() = default;

    constexpr static uint8_t INVALID_CHANNEL_INDEX = 0xFF;

    constexpr bool operator==(const PDFPixelFormat&) const = default;
    constexpr bool operator!=(const PDFPixelFormat&) const = default;

    constexpr bool hasProcessColors() const { return m_processColors > 0; }
    constexpr bool hasSpotColors() const { return m_spotColors > 0; }
    constexpr bool hasShapeChannel() const { return m_flags & FLAG_HAS_SHAPE_CHANNEL; }
    constexpr bool hasOpacityChannel() const { return m_flags & FLAG_HAS_OPACITY_CHANNEL; }
    constexpr bool hasProcessColorsSubtractive() const { return m_flags & FLAG_PROCESS_COLORS_SUBTRACTIVE; }
    constexpr bool hasSpotColorsSubtractive() const { return true; }
    constexpr bool hasActiveColorMask() const { return m_flags & FLAG_HAS_ACTIVE_COLOR_MASK; }

    constexpr uint8_t getFlags() const { return m_flags; }
    constexpr uint8_t getMaximalColorChannelCount() const { return 32; }
    constexpr uint8_t getProcessColorChannelCount() const { return m_processColors; }
    constexpr uint8_t getSpotColorChannelCount() const { return m_spotColors; }
    constexpr uint8_t getColorChannelCount() const { return getProcessColorChannelCount() + getSpotColorChannelCount(); }
    constexpr uint8_t getShapeChannelCount() const { return hasShapeChannel() ? 1 : 0; }
    constexpr uint8_t getOpacityChannelCount() const { return hasOpacityChannel() ? 1 : 0; }
    constexpr uint8_t getAuxiliaryChannelCount() const { return getShapeChannelCount() + getOpacityChannelCount(); }
    constexpr uint8_t getChannelCount() const { return getColorChannelCount() + getAuxiliaryChannelCount(); }

    constexpr uint8_t getProcessColorChannelIndexStart() const { return hasProcessColors() ? 0 : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getProcessColorChannelIndexEnd() const { return hasProcessColors() ? getProcessColorChannelCount() : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getSpotColorChannelIndexStart() const { return hasSpotColors() ? getProcessColorChannelCount() : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getSpotColorChannelIndexEnd() const { return hasSpotColors() ? getSpotColorChannelIndexStart() + getSpotColorChannelCount() : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getColorChannelIndexStart() const { return (hasProcessColors() || hasSpotColors()) ? 0 : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getColorChannelIndexEnd() const { return (hasProcessColors() || hasSpotColors()) ? (m_processColors + m_spotColors) : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getShapeChannelIndex() const { return hasShapeChannel() ? getProcessColorChannelCount() + getSpotColorChannelCount() : INVALID_CHANNEL_INDEX; }
    constexpr uint8_t getOpacityChannelIndex() const { return hasOpacityChannel() ? getProcessColorChannelCount() + getSpotColorChannelCount() + getShapeChannelCount() : INVALID_CHANNEL_INDEX; }

    /// Pixel format is valid, if we have at least one color channel
    /// (it doesn't matter, if it is process color, or spot color)
    constexpr bool isValid() const { return getChannelCount() > 0; }

    inline void setProcessColors(const uint8_t& processColors) { m_processColors = processColors; }
    inline void setSpotColors(const uint8_t& spotColors) { m_spotColors = spotColors; }
    inline void setProcessColorsSubtractive(bool subtractive)
    {
        if (subtractive)
        {
            m_flags |= FLAG_PROCESS_COLORS_SUBTRACTIVE;
        }
        else
        {
            m_flags &= ~FLAG_PROCESS_COLORS_SUBTRACTIVE;
        }
    }

    static constexpr PDFPixelFormat createOpacityMask() { return PDFPixelFormat(0, 0, FLAG_HAS_OPACITY_CHANNEL); }
    static constexpr PDFPixelFormat createFormatDefaultGray(uint8_t spotColors) { return createFormat(1, spotColors, true, false, false); }
    static constexpr PDFPixelFormat createFormatDefaultRGB(uint8_t spotColors) { return createFormat(3, spotColors, true, false, false); }
    static constexpr PDFPixelFormat createFormatDefaultCMYK(uint8_t spotColors) { return createFormat(4, spotColors, true, true, false); }

    static constexpr PDFPixelFormat removeProcessColors(PDFPixelFormat format) { return PDFPixelFormat(0, format.getSpotColorChannelCount(), format.getFlags()); }
    static constexpr PDFPixelFormat removeSpotColors(PDFPixelFormat format) { return PDFPixelFormat(format.getProcessColorChannelCount(), 0, format.getFlags()); }
    static constexpr PDFPixelFormat removeShapeAndOpacity(PDFPixelFormat format) { return PDFPixelFormat(format.getProcessColorChannelCount(), format.getSpotColorChannelCount(), format.hasProcessColorsSubtractive() ? FLAG_PROCESS_COLORS_SUBTRACTIVE : 0); }

    static constexpr uint32_t getAllColorsMask() { return 0xFFFF; }

    static constexpr PDFPixelFormat createFormat(uint8_t processColors, uint8_t spotColors, bool withShapeAndOpacity, bool processColorSubtractive, bool hasActiveColorMask)
    {
        const uint8_t flags = (withShapeAndOpacity ? FLAG_HAS_SHAPE_CHANNEL + FLAG_HAS_OPACITY_CHANNEL : 0) +
                              (processColorSubtractive ? FLAG_PROCESS_COLORS_SUBTRACTIVE : 0) +
                              (hasActiveColorMask ? FLAG_HAS_ACTIVE_COLOR_MASK : 0);
        return PDFPixelFormat(processColors, spotColors, flags);
    }

    /// Calculates bitmap data length required to store bitmapt with given pixel format.
    /// \param width Bitmap width
    /// \param height Bitmap height
    size_t calculateBitmapDataLength(size_t width, size_t height) const { return width * height * size_t(getChannelCount()); }

private:
    inline explicit constexpr PDFPixelFormat(uint8_t processColors, uint8_t spotColors, uint8_t flags) :
        m_processColors(processColors),
        m_spotColors(spotColors),
        m_flags(flags)
    {

    }

    constexpr static uint8_t FLAG_HAS_SHAPE_CHANNEL = 0x01;
    constexpr static uint8_t FLAG_HAS_OPACITY_CHANNEL = 0x02;
    constexpr static uint8_t FLAG_PROCESS_COLORS_SUBTRACTIVE = 0x04;
    constexpr static uint8_t FLAG_HAS_ACTIVE_COLOR_MASK = 0x08;

    uint8_t m_processColors = 0;
    uint8_t m_spotColors = 0;
    uint8_t m_flags = 0;
};

/// Represents float bitmap with arbitrary color channel count. Bitmap can also
/// have auxiliary channels, such as shape and opacity channels.
class PDF4QTLIBCORESHARED_EXPORT PDFFloatBitmap
{
public:
    explicit PDFFloatBitmap();
    explicit PDFFloatBitmap(size_t width, size_t height, PDFPixelFormat format);

    PDFFloatBitmap(const PDFFloatBitmap&) = default;
    PDFFloatBitmap(PDFFloatBitmap&&) = default;

    PDFFloatBitmap& operator=(const PDFFloatBitmap&) = default;
    PDFFloatBitmap& operator=(PDFFloatBitmap&&) = default;

    /// Returns buffer with pixel channels
    PDFColorBuffer getPixel(size_t x, size_t y);

    /// Returns constant buffer with pixel channels
    PDFConstColorBuffer getPixel(size_t x, size_t y) const;

    /// Returns buffer with all pixels
    PDFColorBuffer getPixels();

    /// Returns ink coverage
    PDFColorComponent getPixelInkCoverage(size_t x, size_t y) const;

    /// Returns ink coverage bitmap. Bitmap consists of one color channel,
    /// which consists of ink coverage.
    PDFFloatBitmap getInkCoverageBitmap() const;

    const PDFColorComponent* begin() const;
    const PDFColorComponent* end() const;

    PDFColorComponent* begin();
    PDFColorComponent* end();

    size_t getWidth() const { return m_width; }
    size_t getHeight() const { return m_height; }
    size_t getPixelSize() const { return m_pixelSize; }
    PDFPixelFormat getPixelFormat() const { return m_format; }

    /// Fills both shape and opacity channel with zero value.
    /// If bitmap doesn't have shape/opacity channel, nothing happens.
    void makeTransparent();

    /// Fills both shape and opacity channel with 1.0 value.
    /// If bitmap doesn't have shape/opacity channel, nothing happens.
    void makeOpaque();

    /// Fillss process color channels to a value, that corresponds to black,
    /// so 0.0 for additive colors, 1.0 for subtractive colors.
    void makeColorBlack();

    /// Fillss process color channels to a value, that corresponds to white,
    /// so 1.0 for additive colors, 0.0 for subtractive colors.
    void makeColorWhite();

    /// Returns index where given pixel starts in the data block
    /// \param x Horizontal coordinate of the pixel
    /// \param y Vertical coordinate of the pixel
    size_t getPixelIndex(size_t x, size_t y) const;

    /// Returns true, if bitmap has active color mask
    bool hasActiveColorMask() const { return !m_activeColorMask.empty(); }

    /// Get color mask for given pixel. This function must be called
    /// only on bitmaps, which use active color mask.
    /// \param x Horizontal coordinate of the pixel
    /// \param y Vertical coordinate of the pixel
    uint32_t getPixelActiveColorMask(size_t x, size_t y) const;

    /// Marks active color channels for given pixel as active
    /// \param x Horizontal coordinate of the pixel
    /// \param y Vertical coordinate of the pixel
    /// \param activeColorMask Active color mask
    void markPixelActiveColorMask(size_t x, size_t y, uint32_t activeColorMask);

    /// Sets active color channels for given pixel
    /// \param x Horizontal coordinate of the pixel
    /// \param y Vertical coordinate of the pixel
    /// \param activeColorMask Active color mask
    void setPixelActiveColorMask(size_t x, size_t y, uint32_t activeColorMask);

    /// Sets all colors as active
    void setAllColorActive();

    /// Sets all colors as inactive
    void setAllColorInactive();

    /// Sets color activity to all pixels
    /// \param mask Color activity
    void setColorActivity(uint32_t mask);

    /// Returns gray image created from color channel. If color channel
    /// is invalid, then empty image is returned.
    /// \param channelIndex Channel index
    QImage getChannelImage(uint8_t channelIndex) const;

    /// Extract process colors into another bitmap
    PDFFloatBitmap extractProcessColors() const;

    /// Extract spot channel
    /// \param channel Channel
    PDFFloatBitmap extractSpotChannel(uint8_t channel) const;

    /// Extract opacity channel. If bitmap doesn't have opacity channel,
    /// opaque opacity bitmap of same size is returned.
    PDFFloatBitmap extractOpacityChannel() const;

    /// Extract luminosity channel as opacity channel. Bitmap should have
    /// 1 (gray), 3 (red, green, blue) or 4 (cyan, magenta, yellow, black)
    /// process colors. Otherwise opaque bitmap of same size is returned.
    PDFFloatBitmap extractLuminosityChannel() const;

    /// Copies channel from source bitmap to target channel in this bitmap
    /// \param sourceBitmap Source bitmap
    /// \param channelFrom Source channel
    /// \param channelTo Target channel
    void copyChannel(const PDFFloatBitmap& sourceBitmap, uint8_t channelFrom, uint8_t channelTo);

    /// Resize the bitmap using given transformation mode. Fast transformation mode
    /// uses nearest neighbour mapping, smooth transformation mode uses weighted
    /// averaging algorithm.
    /// \param mode Transformation mode
    PDFFloatBitmap resize(size_t width, size_t height, Qt::TransformationMode mode) const;

    enum class OverprintMode
    {
        NoOveprint,         ///< No oveprint performed
        Overprint_Mode_0,   ///< Overprint performed (either backdrop or source color is selected)
        Overprint_Mode_1,   ///< Overprint performed (only nonzero source color is selected, otherwise backdrop)
    };

    /// Performs bitmap blending, pixel format of source and target must be the same.
    /// Blending algorithm uses the one from chapter 11.4.8 in the PDF 2.0 specification.
    /// Bitmap size must be equal for all three bitmaps (source, target and soft mask).
    /// Oveprinting is also handled. You can specify a mask with active color channels.
    /// If n-th bit in \p activeColorChannels variable is 1, then color channel is active;
    /// otherwise backdrop color is selected (if overprint is active).
    /// \param source Source bitmap
    /// \param target Target bitmap
    /// \param backdrop Backdrop
    /// \param initialBackdrop Initial backdrop
    /// \param softMask Soft mask
    /// \param alphaIsShape Both soft mask and constant alpha are shapes and not opacity?
    /// \param constantAlpha Constant alpha, can mean shape or opacity
    /// \param mode Blend mode
    /// \param overprintMode Overprint mode
    /// \param blendRegion Blend region
    static void blend(const PDFFloatBitmap& source,
                      PDFFloatBitmap& target,
                      const PDFFloatBitmap& backdrop,
                      const PDFFloatBitmap& initialBackdrop,
                      const PDFFloatBitmap& softMask,
                      bool alphaIsShape,
                      PDFColorComponent constantAlpha,
                      BlendMode mode,
                      bool knockoutGroup,
                      OverprintMode overprintMode,
                      QRect blendRegion);

    /// Blends converted spot colors, which are in \p convertedSpotColors bitmap.
    /// Process colors must match.
    /// \param convertedSpotColors Bitmap with converted spot colors
    void blendConvertedSpots(const PDFFloatBitmap& convertedSpotColors);

    void fillProcessColorChannels(PDFColorComponent value);
    void fillChannel(size_t channel, PDFColorComponent value);

    /// Creates opaque soft mask of given size
    /// \param width Width
    /// \param height Height
    static PDFFloatBitmap createOpaqueSoftMask(size_t width, size_t height);

private:
    PDFPixelFormat m_format;
    std::size_t m_width;
    std::size_t m_height;
    std::size_t m_pixelSize;
    std::vector<PDFColorComponent> m_data;
    std::vector<uint32_t> m_activeColorMask;
};

/// Float bitmap with color space
class PDF4QTLIBCORESHARED_EXPORT PDFFloatBitmapWithColorSpace : public PDFFloatBitmap
{
public:
    explicit PDFFloatBitmapWithColorSpace();
    explicit PDFFloatBitmapWithColorSpace(size_t width, size_t height, PDFPixelFormat format);
    explicit PDFFloatBitmapWithColorSpace(size_t width, size_t height, PDFPixelFormat format, PDFColorSpacePointer blendColorSpace);

    PDFColorSpacePointer getColorSpace() const;
    void setColorSpace(const PDFColorSpacePointer& colorSpace);

    /// Converts bitmap to target color space
    /// \param cms Color management system
    /// \param targetColorSpace Target color space
    void convertToColorSpace(const PDFCMS* cms,
                             RenderingIntent intent,
                             const PDFColorSpacePointer& targetColorSpace,
                             PDFRenderErrorReporter* reporter);

private:
    PDFColorSpacePointer m_colorSpace;
};

/// Ink mapping
struct PDFInkMapping
{
    inline bool isValid() const { return !mapping.empty(); }
    inline void reserve(size_t size) { mapping.reserve(size); }
    inline void map(uint8_t source, uint8_t target) { mapping.emplace_back(Mapping{ source, target, Pass}); activeChannels |= 1 << target; }

    enum Type
    {
        Pass
    };

    struct Mapping
    {
        uint8_t source = 0;
        uint8_t target = 0;
        Type type = Pass;
    };

    std::vector<Mapping> mapping;
    uint32_t activeChannels = 0;
};

/// Ink mapper for mapping device inks (device colors) and spot inks (spot colors).
class PDF4QTLIBCORESHARED_EXPORT PDFInkMapper
{
public:
    explicit PDFInkMapper(const PDFCMSManager* manager, const PDFDocument* document);

    struct ColorInfo
    {
        QByteArray name;
        QString textName;
        uint32_t spotColorIndex = 0; ///< Index of this spot color
        uint32_t colorSpaceIndex = 0; ///< Index into DeviceN color space (index of colorant)
        PDFColorSpacePointer colorSpace;
        bool canBeActive = false; ///< Can spot color be activated?
        bool active = false; ///< Is spot color active?
        bool isSpot = true;
        QColor color; ///< Spot/process color transformed to RGB color space
        PDFAbstractColorSpace::ColorSpace colorSpaceType = PDFAbstractColorSpace::ColorSpace::Invalid;
    };

    static constexpr const uint32_t MAX_COLOR_COMPONENTS = PDF_MAX_COLOR_COMPONENTS;
    static constexpr const uint32_t MAX_DEVICE_COLOR_COMPONENTS = 4;
    static constexpr const uint32_t MAX_SPOT_COLOR_COMPONENTS = MAX_COLOR_COMPONENTS - MAX_DEVICE_COLOR_COMPONENTS - 2;

    /// Returns a vector of separations correspoding to the process colors
    /// and spot colors. Only active spot colors are added. Only 1, 3 and 4
    /// process colors are supported.
    /// \param processColorCount Process color count
    /// \param withSpots Add active spot colors?
    std::vector<ColorInfo> getSeparations(uint32_t processColorCount, bool withSpots = true) const;

    /// Scan document for spot colors and fills color info
    /// \param activate Set spot colors active?
    void createSpotColors(bool activate);

    /// Returns true, if mapper contains given spot color
    /// \param colorName Color name
    bool containsSpotColor(const QByteArray& colorName) const;

    /// Returns true, if mapper contains given process color
    /// \param colorName Color name
    bool containsProcessColor(const QByteArray& colorName) const;

    /// Returns number of active spot colors
    size_t getActiveSpotColorCount() const { return m_activeSpotColors; }

    /// Returns spot color information (or nullptr, if spot color is not present)
    /// \param colorName Color name
    const ColorInfo* getSpotColor(const QByteArray& colorName) const;

    /// Returns process color information (or nullptr, if process color is not present)
    /// \param colorName Color name
    const ColorInfo* getProcessColor(const QByteArray& colorName) const;

    /// Returns process color information (or nullptr, if process color is not present or is inactive)
    /// \param colorName Color name
    /// \param colorSpace Color space (actively used color space)
    const ColorInfo* getActiveProcessColor(const QByteArray& colorName, PDFAbstractColorSpace::ColorSpace colorSpace) const;

    /// Returns active spot color with given index. If index
    /// of the spot color is invalid, or no active spot color
    /// is found, then nullptr is returned.
    /// \param index Active color index
    const ColorInfo* getActiveSpotColor(size_t index) const;

    /// Activates / deactivates spot colors
    /// \param active Make spot colors active?
    void setSpotColorsActive(bool active);

    /// Creates color mapping from source color space to the target color space.
    /// If mapping  cannot be created, then invalid mapping is returned. Target
    /// color space must be blending color space and must correspond to active
    /// blending space, if used when painting.
    /// \param sourceColorSpace Source color space
    /// \param targetColorSpace Target color space
    /// \param targetPixelFormat
    PDFInkMapping createMapping(const PDFAbstractColorSpace* sourceColorSpace,
                                const PDFAbstractColorSpace* targetColorSpace,
                                PDFPixelFormat targetPixelFormat) const;

private:
    const PDFCMSManager* m_cmsManager;
    const PDFDocument* m_document;
    std::vector<ColorInfo> m_spotColors;
    std::vector<ColorInfo> m_deviceColors; ///< Device color space separations
    size_t m_activeSpotColors = 0;
};

/// Painter path sampler. Returns shape value of pixel. This sampler
/// uses MSAA with regular grid.
class PDFPainterPathSampler
{
public:
    /// Creates new painter path sampler, using given painter path,
    /// sample count (in one direction) and default shape used, when painter path is empty.
    /// Fill rectangle is used to precompute winding numbers for samples. Points outside
    /// of fill rectangle are considered as outside and defaultShape is returned.
    /// \param path Sampled path
    /// \param samplesCount Samples count in one direction
    /// \param defaultShape Default shape returned, if path is empty
    /// \param fillRect Fill rectangle (sample point must be in this rectangle)
    /// \param precise Use precise painter path computation
    PDFPainterPathSampler(QPainterPath path,
                          int samplesCount,
                          PDFColorComponent defaultShape,
                          QRect fillRect,
                          bool precise);

    /// Return sample value for a given pixel
    PDFColorComponent sample(QPoint point) const;

private:
    struct ScanLineSample
    {
        inline constexpr ScanLineSample() = default;
        inline constexpr ScanLineSample(PDFReal x, int windingNumber) :
            x(x),
            windingNumber(windingNumber)
        {

        }

        bool operator<(const ScanLineSample& other) const { return x < other.x; }
        bool operator<(PDFReal ordinate) const { return x < ordinate; }

        PDFReal x = 0.0;
        int windingNumber = 0;
    };

    struct ScanLineInfo
    {
        size_t indexStart = 0;
        size_t indexEnd = 0;
    };

    /// Compute sample by using scan lines
    PDFColorComponent sampleByScanLine(QPoint point) const;

    /// Returns number of scan lines per pixel
    size_t getScanLineCountPerPixel() const;

    /// Creates scan lines using fill rectangle
    void prepareScanLines();

    /// Creates scan line for given y coordinate and
    /// returns info about this scan line
    /// \param y Vertical coordinate of the sample line
    ScanLineInfo createScanLine(qreal y);

    /// Creates scan line sample (if horizontal line of y coordinate
    /// is intersecting boundary line segment p1-p2.
    /// \param p1 First point of the oriented boundary line segment
    /// \param p2 Second point of the oriented boundary line segment
    /// \param y Vertical coordinate of the sample line
    void createScanLineSample(const QPointF& p1, const QPointF& p2, qreal y);

    PDFColorComponent m_defaultShape = 0.0;
    int m_samplesCount = 0; ///< Samples count in one direction
    QPainterPath m_path;
    QPolygonF m_fillPolygon;
    QRect m_fillRect;
    std::vector<ScanLineSample> m_scanLineSamples;
    std::vector<ScanLineInfo> m_scanLineInfo;
    bool m_precise;
};

/// Represents draw buffer, into which is current graphics drawn
class PDFDrawBuffer : public PDFFloatBitmap
{
public:
    using PDFFloatBitmap::PDFFloatBitmap;

    /// Clears the draw buffer
    void clear();

    /// Marks given area as modified
    void modify(QRect rect, bool containsFilling, bool containsStroking);

    /// Returns true, if draw buffer is modified and needs to be flushed
    bool isModified() const { return m_modifiedRect.isValid(); }

    /// Returns modified rectangle
    QRect getModifiedRect() const { return m_modifiedRect; }

    bool isContainsFilling() const { return m_containsFilling; }
    bool isContainsStroking() const { return m_containsStroking; }

private:
    bool m_containsFilling = false;
    bool m_containsStroking = false;
    QRect m_modifiedRect;
};

struct PDFTransparencyRendererSettings
{
    /// Sample count for MSAA antialiasing
    int samplesCount = 16;

    /// Threshold for turning on painter path
    /// multithreaded painting. When number of potential
    /// pixels of painter path is greater than this constant,
    /// and MultithreadedPathSampler flag is turned on,
    /// multithreaded painting is performed.
    int multithreadingPathSampleThreshold = 128;

    /// Maximal number of steps performed in numerical algorithm
    /// used when some shadings are being sampled.
    int shadingAlgorithmLimit = 64;

    enum Flag
    {
        None               = 0x0000,

        /// Use precise path sampler, which uses paths instead
        /// of filling polygon.
        PrecisePathSampler          = 0x0001,

        /// Use multithreading when painter paths are painted?
        /// Multithreading is used to
        MultithreadedPathSampler    = 0x0002,

        /// When using CMYK process color space, transfer spot
        /// colors to the CMYK color space.
        SeparationSimulation        = 0x0004,

        /// Use active color mask (so we can clear channels,
        /// which are not active)
        ActiveColorMask             = 0x0008,

        /// Use smooth image transform, if it is possible. For
        /// images, which doesn't have Interpolate set to true,
        /// fast image transformation is used.
        SmoothImageTransformation   = 0x0010,

        /// Display images (if this flag is false, images aren't processed)
        DisplayImages               = 0x0020,

        /// Display text (if this flag is false, text isnn't processed)
        DisplayText                 = 0x0040,

        /// Display vector graphics (if this flag is false, vector graphics isn't processed)
        DisplayVectorGraphics       = 0x0080,

        /// Display shading patterns (if this flag is false, shading patterns aren't processed)
        DisplayShadings             = 0x0100,

        /// Display tiling patterns (if this flag is false, tiling patterns aren't processed)
        DisplayTilingPatterns       = 0x0200,

        /// Saves process image before it is transformed into device space
        /// and before separation simulation is applied. Active color mask
        /// is still applied to this image.
        SaveOriginalProcessImage    = 0x0400,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    /// Flags
    Flags flags = static_cast<Flags>(DisplayImages | DisplayText | DisplayVectorGraphics | DisplayShadings | DisplayTilingPatterns);

    /// Active color mask
    uint32_t activeColorMask = PDFPixelFormat::getAllColorsMask();
};

/// Renders PDF pages with transparency, using 32-bit floating point precision.
/// Both device color space and blending color space can be defined. It implements
/// page blending space and device blending space. So, painted graphics is being
/// blended to the page blending space, and then converted to the device blending
/// space.
class PDF4QTLIBCORESHARED_EXPORT PDFTransparencyRenderer : public PDFPageContentProcessor
{
private:
    using BaseClass = PDFPageContentProcessor;

public:
    PDFTransparencyRenderer(const PDFPage* page,
                            const PDFDocument* document,
                            const PDFFontCache* fontCache,
                            const PDFCMS* cms,
                            const PDFOptionalContentActivity* optionalContentActivity,
                            const PDFInkMapper* inkMapper,
                            PDFTransparencyRendererSettings settings,
                            QTransform pagePointToDevicePointMatrix);

    /// Sets device color space. This is final color space, to which
    /// is painted page transformed.
    /// \param colorSpace Color space
    void setDeviceColorSpace(PDFColorSpacePointer colorSpace);

    /// Sets process color space. This color space is used for blending
    /// and intermediate results. If page has transparency group, then
    /// blending color space from transparency group is used.
    /// \param colorSpace Color space
    void setProcessColorSpace(PDFColorSpacePointer colorSpace);

    /// Starts painting on the device. This function must be called before page
    /// content stream is being processed (and must be called exactly once).
    void beginPaint(QSize pixelSize);

    /// Finishes painting on the device. This function must be called after page
    /// content stream is processed and all result graphics is being drawn. Page
    /// transparency group collapses nad contents are draw onto device transparency
    /// group.
    const PDFFloatBitmap& endPaint();

    /// This function should be called only after call to \p endPaint. After painting,
    /// when it is finished, the result float image can be converted to QImage with
    /// this function, but only, if the float image is RGB. If error occurs, empty
    /// image is returned. Also, result image can be painted onto opaque paper
    /// with paper color \p paperColor.
    /// \param use16bit Produce 16-bit image instead of standard 8-bit
    /// \param usePaper Blend image with opaque paper, with color \p paperColor
    /// \param paperColor Paper color
    QImage toImage(bool use16Bit, bool usePaper, const PDFRGB& paperColor) const;

    /// Clear color buffer with given color (this affects all process colors). If a number
    /// of process colors are different from a number of colors in color, then error is triggered,
    /// and most min(process color count, colors in color) process color channels are filled
    /// with color.
    /// \param color Color
    void clearColor(const PDFColor& color);

    /// Returns original process bitmap, before it is transformed into device space,
    /// and before separation simulation is being processed. Active color mask is still
    /// applied to this image.
    PDFFloatBitmapWithColorSpace getOriginalProcessBitmap() const { return m_originalProcessBitmap; }

    virtual bool isContentKindSuppressed(ContentKind kind) const override;
    virtual void performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule) override;
    virtual bool performPathPaintingUsingShading(const QPainterPath& path, bool stroke, bool fill, const PDFShadingPattern* shadingPattern) override;
    virtual void performFinishPathPainting() override;
    virtual void performClipping(const QPainterPath& path, Qt::FillRule fillRule) override;
    virtual void performUpdateGraphicsState(const PDFPageContentProcessorState& state) override;
    virtual void performSaveGraphicState(ProcessOrder order) override;
    virtual void performRestoreGraphicState(ProcessOrder order) override;
    virtual void performBeginTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup) override;
    virtual void performEndTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup) override;
    virtual void performTextBegin(ProcessOrder order) override;
    virtual void performTextEnd(ProcessOrder order) override;
    virtual bool performOriginalImagePainting(const PDFImage& image) override;
    virtual void performImagePainting(const QImage& image) override;
    virtual void performMeshPainting(const PDFMesh& mesh) override;

private:

    PDFReal getShapeStroking() const;
    PDFReal getOpacityStroking() const;
    PDFReal getShapeFilling() const;
    PDFReal getOpacityFilling() const;

    struct PDFTransparencySoftMaskImpl : public QSharedData
    {
        PDFTransparencySoftMaskImpl() = default;
        PDFTransparencySoftMaskImpl(bool isOpaque, PDFFloatBitmap softMask) :
            isOpaque(isOpaque),
            softMask(qMove(softMask))
        {

        }

        bool isOpaque = false;
        PDFFloatBitmap softMask;
    };

    class PDFTransparencySoftMask
    {
    public:
        PDFTransparencySoftMask() { m_data = new PDFTransparencySoftMaskImpl; }
        PDFTransparencySoftMask(bool isOpaque, PDFFloatBitmap softMask) { m_data = new PDFTransparencySoftMaskImpl(isOpaque, qMove(softMask)); }

        bool isOpaque() const { return m_data->isOpaque; }
        const PDFFloatBitmap* getSoftMask() const { return &m_data->softMask; }

        void makeOpaque();

    private:
        QSharedDataPointer<PDFTransparencySoftMaskImpl> m_data;
    };

    struct PDFTransparencyGroupPainterData
    {
        void makeInitialBackdropTransparent();
        void makeImmediateBackdropTransparent();

        PDFTransparencyGroup group;
        bool alphaIsShape = false;
        PDFReal alphaStroke = 1.0;
        PDFReal alphaFill = 1.0;
        BlendMode blendMode = BlendMode::Normal;
        BlackPointCompensationMode blackPointCompensationMode = BlackPointCompensationMode::Default;
        RenderingIntent renderingIntent = RenderingIntent::RelativeColorimetric;
        PDFFloatBitmapWithColorSpace initialBackdrop;   ///< Initial backdrop
        PDFFloatBitmapWithColorSpace immediateBackdrop; ///< Immediate backdrop
        PDFTransparencySoftMask softMask; ///< Soft mask for this group
        PDFColorSpacePointer blendColorSpace;
        bool filterColorsUsingMask = false;
        uint32_t activeColorMask = PDFPixelFormat::getAllColorsMask();
        bool transformSpotsToDevice = false;
        bool saveOriginalImage = false;
    };

    struct PDFTransparencyPainterState
    {
        QPainterPath clipPath; ///< Clipping path in device state coordinates
        PDFTransparencySoftMask softMask;
    };

    struct PDFMappedColor
    {
        PDFColor mappedColor;
        uint32_t activeChannels = 0;
    };

    void invalidateCachedItems();
    void removeInitialBackdrop();

    void fillMappedColorUsingMapping(const PDFPixelFormat pixelFormat,
                                     PDFMappedColor& result,
                                     const PDFInkMapping& inkMapping,
                                     const PDFColor& sourceColor);

    PDFMappedColor createMappedColor(const PDFColor& sourceColor,
                                     const PDFAbstractColorSpace* sourceColorSpace);

    /// Converts image in some other color space to the blend color space,
    /// so pixel format is equal to the draw buffer's pixel format and active
    /// colors are set.
    /// \param image Image
    PDFFloatBitmapWithColorSpace convertImageToBlendSpace(const PDFFloatBitmapWithColorSpace& image);

    /// Converts RGB bitmap to the image.
    QImage toImageImpl(const PDFFloatBitmapWithColorSpace& floatImage, bool use16Bit) const;

    PDFFloatBitmapWithColorSpace* getInitialBackdrop();
    PDFFloatBitmapWithColorSpace* getImmediateBackdrop();
    PDFFloatBitmapWithColorSpace* getBackdrop();
    const PDFFloatBitmapWithColorSpace* getInitialBackdrop() const;
    const PDFFloatBitmapWithColorSpace* getImmediateBackdrop() const;
    const PDFFloatBitmapWithColorSpace* getBackdrop() const;
    const PDFColorSpacePointer& getBlendColorSpace() const;

    PDFTransparencyPainterState* getPainterState() { return &m_painterStateStack.top(); }

    bool isTransparencyGroupIsolated() const;
    bool isTransparencyGroupKnockout() const;

    const PDFMappedColor& getMappedStrokeColor();
    const PDFMappedColor& getMappedFillColor();

    PDFMappedColor getMappedStrokeColorImpl();
    PDFMappedColor getMappedFillColorImpl();

    /// Returns painting rectangle (i.e. rectangle, which has topleft coordinate 0,0
    /// and has width/height equal to bitmap width/height)
    QRect getPaintRect() const;

    /// Returns fill area from fill rectangle
    /// \param fillRect Fill rectangle
    QRect getActualFillRect(const QRectF& fillRect) const;

    /// Flushes draw buffer
    void flushDrawBuffer();

    /// Returns true, if multithreaded painter path sampling should be used
    /// for a given fill rectangle.
    /// \param fillRect Fill rectangle
    /// \returns true, if multithreading should be used
    bool isMultithreadedPathSamplingUsed(QRect fillRect) const;

    /// Performs sampling of single pixel. Sampled pixel is painted
    /// into the draw buffer.
    /// \param shape Constant shape value
    /// \param opacity Constant opacity value
    /// \param x Horizontal coordinate of the pixel
    /// \param y Vertical coordinate of the pixel
    /// \param shapeChannel Shape channel (draw buffer)
    /// \param opacityChannel Opacity channel (draw buffer)
    /// \param colorChannelStart Color channel start (draw buffer)
    /// \param colorChannelEnd Color channel end (draw buffer)
    /// \param fillColor Fill color
    /// \param clipSampler Clipping sampler
    /// \param pathSampler Path sampler
    void performPixelSampling(const PDFReal shape,
                              const PDFReal opacity,
                              const uint8_t shapeChannel,
                              const uint8_t opacityChannel,
                              const uint8_t colorChannelStart,
                              const uint8_t colorChannelEnd,
                              int x,
                              int y,
                              const PDFMappedColor& fillColor,
                              const PDFPainterPathSampler& clipSampler,
                              const PDFPainterPathSampler& pathSampler);

    /// Performs fragment fill from texture. Sampled pixel is painted
    /// into the draw buffer.
    /// \param shape Constant shape value
    /// \param opacity Constant opacity value
    /// \param shapeChannel Shape channel (draw buffer)
    /// \param opacityChannel Opacity channel (draw buffer)
    /// \param colorChannelStart Color channel start (draw buffer)
    /// \param colorChannelEnd Color channel end (draw buffer)
    /// \param x Horizontal coordinate of the fragment pixel
    /// \param y Vertical coordinate of the fragment pixel
    /// \param worldToTextureMatrix World to texture matrix
    /// \param texture Texture
    /// \param clipSampler Clipping sampler
    void performFillFragmentFromTexture(const PDFReal shape,
                                        const PDFReal opacity,
                                        const uint8_t shapeChannel,
                                        const uint8_t opacityChannel,
                                        const uint8_t colorChannelStart,
                                        const uint8_t colorChannelEnd,
                                        int x,
                                        int y,
                                        const QTransform& worldToTextureMatrix,
                                        const PDFFloatBitmap& texture,
                                        const PDFPainterPathSampler& clipSampler);

    /// Collapses spot colors to device colors
    /// \param data Bitmap with data
    void collapseSpotColorsToDeviceColors(PDFFloatBitmapWithColorSpace& bitmap);

    /// Transforms image to float image in actual blending color space,
    /// with marked colors. Function for internal use only.
    /// \param sourceImage
    PDFFloatBitmapWithColorSpace getImage(const PDFImage& sourceImage);

    /// Transforms colored image to float image in actual blending color space,
    /// with marked colors. Function for internal use only.
    /// \param sourceImage
    PDFFloatBitmapWithColorSpace getColoredImage(const PDFImage& sourceImage);

    /// Create soft mask from image data. Image data must contain proper soft
    /// mask, otherwise function will throw exception.
    /// \param imageData Soft mask data
    PDFFloatBitmap getAlphaMaskFromSoftMask(const PDFImageData& softMask);

    /// Processes soft mask and sets it as active
    /// \param softMask Soft mask
    void processSoftMask(const PDFDictionary* softMask);

    static void createOpaqueBitmap(PDFFloatBitmap& bitmap);
    static void createPaperBitmap(PDFFloatBitmap& bitmap, const PDFRGB& paperColor);
    static void createOpaqueSoftMask(PDFFloatBitmap& softMask, size_t width, size_t height) { softMask = PDFFloatBitmap::createOpaqueSoftMask(width, height); }

    PDFColorSpacePointer m_deviceColorSpace;    ///< Device color space (color space for final result)
    PDFColorSpacePointer m_processColorSpace;   ///< Process color space (color space, in which is page graphic's blended)
    std::unique_ptr<PDFTransparencyGroupGuard> m_pageTransparencyGroupGuard;
    std::unique_ptr<PDFTransparencyGroupGuard> m_textTransparencyGroupGuard;
    std::vector<PDFTransparencyGroupPainterData> m_transparencyGroupDataStack;
    std::stack<PDFTransparencyPainterState> m_painterStateStack;
    const PDFInkMapper* m_inkMapper;
    bool m_active;
    PDFCachedItem<PDFMappedColor> m_mappedStrokeColor;
    PDFCachedItem<PDFMappedColor> m_mappedFillColor;
    PDFTransparencyRendererSettings m_settings;
    PDFDrawBuffer m_drawBuffer;
    PDFFloatBitmapWithColorSpace m_originalProcessBitmap;
};

/// Ink coverage calculator. Calculates ink coverage for a given
/// page range. Calculates ink coverage of both cmyk colors and spot colors.
class PDF4QTLIBCORESHARED_EXPORT PDFInkCoverageCalculator
{
public:
    PDFInkCoverageCalculator(const PDFDocument* document,
                             const PDFFontCache* fontCache,
                             const PDFCMSManager* cmsManager,
                             const PDFOptionalContentActivity* optionalContentActivity,
                             const PDFInkMapper* inkMapper,
                             PDFProgress* progress,
                             PDFTransparencyRendererSettings settings);

    struct InkCoverageChannelInfo
    {
        QByteArray name;
        QString textName;
        bool isSpot = true;
        QColor color;
        PDFColorComponent coveredArea = 0.0f;
        PDFColorComponent ratio = 0.0f;
    };

    /// Perform ink coverage calculations on given pages. Results are stored
    /// in this object. Page images are rendered using \p size resolution,
    /// and in this resolution, ink coverage is calculated.
    /// \param size Resolution size (for ink coverage calculation)
    /// \param pages Page indices
    void perform(QSize size, const std::vector<PDFInteger>& pages);

    /// Clear all calculated ink coverage results
    void clear();

    /// Clears calculated ink coverage for all pages. If ink coverage is not
    /// calculated for given page index, empty vector is returned.
    /// \param pageIndex Page index
    const std::vector<InkCoverageChannelInfo>* getInkCoverage(PDFInteger pageIndex) const;

    /// Find coverage info in vector by colorant name. If coverage info with a given colorant
    /// name is not found, then nullptr is returned.
    /// \param infos Vector of coverage info
    /// \param name Colornant name
    /// \returns Info for a given colorant name, or nullptr, if it is not found
    static const InkCoverageChannelInfo* findCoverageInfoByName(const std::vector<InkCoverageChannelInfo>& infos, const QByteArray& name);

    /// Find coverage info in vector by colorant name. If coverage info with a given colorant
    /// name is not found, then nullptr is returned.
    /// \param infos Vector of coverage info
    /// \param name Colornant name
    /// \returns Info for a given colorant name, or nullptr, if it is not found
    static InkCoverageChannelInfo* findCoverageInfoByName(std::vector<InkCoverageChannelInfo>& infos, const QByteArray& name);

private:
    const PDFDocument* m_document;
    const PDFFontCache* m_fontCache;
    const PDFCMSManager* m_cmsManager;
    const PDFOptionalContentActivity* m_optionalContentActivity;
    const PDFInkMapper* m_inkMapper;
    PDFProgress* m_progress;
    PDFTransparencyRendererSettings m_settings;

    QMutex m_mutex;
    std::map<pdf::PDFInteger, std::vector<InkCoverageChannelInfo>> m_inkCoverageResults;
};

}   // namespace pdf

#endif // PDFTRANSPARENCYRENDERER_H
