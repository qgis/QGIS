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

#ifndef PDFIMAGEOPTIMIZER_H
#define PDFIMAGEOPTIMIZER_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfimage.h"
#include "pdfimagecompressor.h"

#include <QImage>
#include <QPointF>
#include <QString>

#include <map>
#include <optional>
#include <vector>

namespace pdf
{
class PDFDocument;
class PDFRenderErrorReporter;
class PDFProgress;

/// Comparator for ordering PDF object references by object number and generation.
struct PDFObjectReferenceLess
{
    bool operator()(const PDFObjectReference& left, const PDFObjectReference& right) const
    {
        if (left.objectNumber != right.objectNumber)
        {
            return left.objectNumber < right.objectNumber;
        }
        return left.generation < right.generation;
    }
};

/// Image optimization helper that analyzes document images and rewrites image
/// streams to reduce size while preserving visual fidelity.
/// The optimizer is stateless; the returned document is a copy with updated
/// image streams, and the source document is not modified.
class PDF4QTLIBCORESHARED_EXPORT PDFImageOptimizer
{
public:
    /// High-level goal used to balance quality and size.
    enum class OptimizationGoal
    {
        PreferQuality, ///< Prefer visual quality; use conservative resampling and compression.
        MinimumSize    ///< Prefer smaller size; allow more aggressive resampling/compression.
    };

    /// Requested output color characteristics.
    enum class ColorMode
    {
        Auto,      ///< Decide based on image analysis and settings.
        Preserve,  ///< Preserve original color characteristics.
        Color,     ///< Force color output (DeviceRGB).
        Grayscale, ///< Force grayscale output (DeviceGray).
        Bitonal    ///< Force 1-bit output (DeviceGray).
    };

    /// Output compression algorithm for encoded image streams.
    enum class CompressionAlgorithm
    {
        Auto,        ///< Choose algorithm based on image analysis and color mode.
        Flate,       ///< Flate (PNG predictor).
        JPEG,        ///< Baseline JPEG (DCT).
        JPEG2000,    ///< JPEG 2000 (JPX).
        RunLength,   ///< RunLength encoding.
        CCITTGroup4, ///< CCITT Group 4 for bi-tonal images (if supported).
        JBIG2        ///< JBIG2 for bi-tonal images (if supported).
    };

    /// Compression and resampling settings for a single color mode.
    struct CompressionProfile
    {
        CompressionAlgorithm algorithm = CompressionAlgorithm::Auto; ///< Selected compression algorithm.
        int targetDpi = 0; ///< Target DPI for downsampling (0 keeps original).
        PDFImage::ResampleFilter resampleFilter = PDFImage::ResampleFilter::Bicubic; ///< Resampling filter.
        int jpegQuality = 85; ///< JPEG quality (0-100).
        float jpeg2000Rate = 0.0f; ///< JPEG2000 rate (>0 lossy, 0 lossless).
        int monochromeThreshold = -1; ///< <0 means automatic threshold for bitonal conversion.
        bool enablePngPredictor = true; ///< Enable PNG predictor for Flate compression.
    };

    /// Global optimization settings.
    struct PDF4QTLIBCORESHARED_EXPORT Settings
    {
        bool enabled = false; ///< If false, no optimization is performed.
        bool autoMode = true; ///< If true, use image analysis to pick modes/algorithms.
        ColorMode colorMode = ColorMode::Auto; ///< Requested color mode.
        OptimizationGoal goal = OptimizationGoal::PreferQuality; ///< Size/quality trade-off.
        bool keepOriginalIfLarger = true; ///< Keep original image if re-encode is not smaller.
        bool preserveTransparency = true; ///< Preserve transparency via soft mask when possible.
        CompressionProfile colorProfile; ///< Settings used for color images.
        CompressionProfile grayProfile; ///< Settings used for grayscale images.
        CompressionProfile bitonalProfile; ///< Settings used for bi-tonal images.

        /// Returns default settings suitable for typical optimization runs.
        static Settings createDefault();
    };

    /// Collected metadata and decoded image for optimization decisions.
    struct ImageInfo
    {
        PDFObjectReference reference; ///< Object reference of the image stream.
        QImage image; ///< Decoded image data.
        QPointF minimalDpi; ///< Minimal effective DPI (per-axis) observed in the document.
        QSize pixelSize; ///< Pixel dimensions of the image.
        QString colorSpaceName; ///< Name of the image color space, if known.
        QString filterName; ///< Name of the last applied filter, if known.
        int bitsPerComponent = 0; ///< Bits per component from the image dictionary.
        bool hasSoftMask = false; ///< True if the image has a soft mask.
        bool hasTransparency = false; ///< True if decoded pixels include transparency.
        bool isImageMask = false; ///< True if the image is an ImageMask.
        int originalBytes = 0; ///< Combined size of image and mask streams.
    };

    /// Per-image override for enabling/disabling or using custom settings.
    struct ImageOverride
    {
        bool enabled = true; ///< If false, the image is skipped.
        bool useCustomSettings = false; ///< If true, use custom settings instead of global ones.
        Settings settings; ///< Custom settings for this image.
    };

    /// Overrides indexed by image object reference.
    using ImageOverrides = std::map<PDFObjectReference, ImageOverride, PDFObjectReferenceLess>;

    /// Image analysis results used to choose color mode and compression.
    struct ImageAnalysis
    {
        bool grayscale = false; ///< True if image appears grayscale.
        int uniqueColors = 0; ///< Approximate count of unique colors (quantized).
        double edgeDensity = 0.0; ///< Edge density estimate.
        double variance = 0.0; ///< Luma variance estimate.
        bool hasTransparency = false; ///< True if any pixels are transparent.

        /// Heuristic classification of the image content.
        /// The optimizer uses this to choose a default color mode and a
        /// compression family in auto mode. The classification is based on a
        /// downscaled sample, approximate color count and simple edge/luma
        /// statistics; it is intentionally lightweight and not guaranteed to
        /// match the semantic content perfectly.
        enum class Kind
        {
            Photo,    ///< Photographic content.
            LineArt,  ///< Line art / graphics.
            TextScan  ///< Bi-tonal or text scan.
        } kind = Kind::Photo;
    };

    /// Resolved plan describing how the image will be encoded.
    struct ResolvedPlan
    {
        PDFImage::ImageEncodeOptions encodeOptions; ///< Encoding options passed to PDFImage.
        CompressionAlgorithm algorithm = CompressionAlgorithm::Flate; ///< Resolved compression algorithm.
        ColorMode resolvedColorMode = ColorMode::Color; ///< Resolved color mode.
        QSize targetSize; ///< Target resampled size (invalid means original).
        bool useSoftMask = false; ///< True if a soft mask should be preserved.
        bool hadUnsupportedCompression = false; ///< True if a requested algorithm was not available.
        int originalBytes = 0; ///< Original size of the image streams in bytes.
    };

    /// Final result for one optimized image.
    struct ImageResult
    {
        PDFObjectReference reference; ///< Object reference of the image stream.
        bool keptOriginal = true; ///< True if original stream was kept unchanged.
        int originalBytes = 0; ///< Original total byte size.
        int newBytes = 0; ///< New total byte size (0 if not re-encoded).
        QString message; ///< Optional diagnostic message.
    };

    PDFImageOptimizer() = default;

    /// Collects information about all image XObjects in the document.
    /// \param document Source document.
    /// \returns List of image infos (empty if none or document is null).
    static std::vector<ImageInfo> collectImageInfos(const PDFDocument* document);

    /// Analyzes an image and classifies its content.
    /// \param image Image to analyze.
    /// \returns Analysis results (default values if image is null).
    static ImageAnalysis analyzeImage(const QImage& image);

    /// Builds an encoding plan for a specific image and settings.
    /// \param info Image metadata and pixels.
    /// \param settings Global or per-image settings.
    /// \returns Resolved plan with encoding options.
    ///
    /// Resolution rules:
    /// - `Auto` mode uses image analysis heuristics to choose color mode and
    ///   compression.
    /// - `Preserve` keeps the closest practical representation inferred from
    ///   source metadata and decoded pixels. For example, a 1-bit gray image
    ///   stays bitonal instead of being expanded to 8-bit grayscale.
    /// - Unsupported requested algorithms are downgraded to Flate and marked
    ///   through `hadUnsupportedCompression`.
    static ResolvedPlan resolvePlan(const ImageInfo& info, const Settings& settings);

    /// Creates a preview image by applying the resolved plan.
    /// \param info Image metadata and pixels.
    /// \param plan Resolved plan.
    /// \param simulateCompression If true, simulate lossy compression artifacts.
    /// \returns Preview image (null if input image is null).
    static QImage createPreviewImage(const ImageInfo& info, const ResolvedPlan& plan, bool simulateCompression);

    /// Estimates the encoded byte size using the resolved plan.
    /// \param info Image metadata and pixels.
    /// \param plan Resolved plan.
    /// \param reporter Optional error reporter for encoding failures.
    /// \returns Estimated size in bytes (0 on failure).
    static int estimateEncodedBytes(const ImageInfo& info, const ResolvedPlan& plan, PDFRenderErrorReporter* reporter = nullptr);

    /// Optimizes images in a document and returns the updated document.
    /// \param document Source document.
    /// \param settings Global settings for optimization.
    /// \param overrides Optional per-image overrides.
    /// \param progress Optional progress reporter.
    /// \param reporter Optional error reporter for encoding issues.
    /// \param results Optional output vector with per-image results.
    /// \returns New document with optimized image streams (or a copy if disabled).
    PDFDocument optimize(const PDFDocument* document,
                         const Settings& settings,
                         const ImageOverrides& overrides = ImageOverrides(),
                         PDFProgress* progress = nullptr,
                         PDFRenderErrorReporter* reporter = nullptr,
                         std::vector<ImageResult>* results = nullptr) const;
};

}   // namespace pdf

#endif // PDFIMAGEOPTIMIZER_H
