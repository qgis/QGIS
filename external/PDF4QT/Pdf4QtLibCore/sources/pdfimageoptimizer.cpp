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

#include "pdfimageoptimizer.h"

#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfimageconversion.h"
#include "pdfprogress.h"
#include "pdfconstants.h"

#include <QBuffer>
#include <QImageWriter>

#include <cmath>
#include <unordered_set>

#include "pdfdbgheap.h"

namespace pdf
{
namespace
{
constexpr int ANALYSIS_MAX_SIZE = 256;
constexpr int MAX_UNIQUE_COLOR_SAMPLES = 512;
constexpr int GRAYSCALE_TOLERANCE = 3;

constexpr bool supportsJbig2Encoder()
{
    return false;
}

constexpr bool supportsCcittEncoder()
{
    return false;
}

QString readNameFromColorSpace(const PDFDocument* document, const PDFObject& object)
{
    if (!document)
    {
        return QString();
    }

    const PDFObject& resolved = document->getObject(object);
    if (resolved.isName())
    {
        return QString::fromLatin1(resolved.getString());
    }
    if (resolved.isArray())
    {
        const PDFArray* array = resolved.getArray();
        if (array && array->getCount() > 0)
        {
            const PDFObject& first = document->getObject(array->getItem(0));
            if (first.isName())
            {
                return QString::fromLatin1(first.getString());
            }
        }
    }

    return QString();
}

QString readFilterName(const PDFDocument* document, const PDFDictionary* dictionary)
{
    if (!document || !dictionary)
    {
        return QString();
    }

    PDFObject filters;
    if (dictionary->hasKey(PDF_STREAM_DICT_FILTER))
    {
        filters = document->getObject(dictionary->get(PDF_STREAM_DICT_FILTER));
    }
    else if (dictionary->hasKey(PDF_STREAM_DICT_FILE_FILTER))
    {
        filters = document->getObject(dictionary->get(PDF_STREAM_DICT_FILE_FILTER));
    }

    if (filters.isName())
    {
        return QString::fromLatin1(filters.getString());
    }
    if (filters.isArray())
    {
        const PDFArray* array = filters.getArray();
        if (array && array->getCount() > 0)
        {
            const PDFObject& last = document->getObject(array->getItem(array->getCount() - 1));
            if (last.isName())
            {
                return QString::fromLatin1(last.getString());
            }
        }
    }

    return QString();
}

bool hasNonOpaquePixel(const QImage& image)
{
    if (!image.hasAlphaChannel())
    {
        return false;
    }

    QImage rgba = image.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
        return false;
    }

    for (int y = 0; y < rgba.height(); ++y)
    {
        const uchar* line = rgba.constScanLine(y);
        for (int x = 0; x < rgba.width(); ++x)
        {
            if (line[3] != 255)
            {
                return true;
            }
            line += 4;
        }
    }

    return false;
}

QImage extractAlphaMask(const QImage& image)
{
    QImage rgba = image.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
        return QImage();
    }

    QImage mask(rgba.width(), rgba.height(), QImage::Format_Grayscale8);
    if (mask.isNull())
    {
        return QImage();
    }

    for (int y = 0; y < rgba.height(); ++y)
    {
        const uchar* src = rgba.constScanLine(y);
        uchar* dst = mask.scanLine(y);
        for (int x = 0; x < rgba.width(); ++x)
        {
            dst[x] = src[3];
            src += 4;
        }
    }

    return mask;
}

QSize computeTargetSize(const QSize& sourceSize,
                        const QPointF& minimalDpi,
                        int targetDpi,
                        PDFImageOptimizer::OptimizationGoal goal)
{
    if (targetDpi <= 0 || sourceSize.isEmpty())
    {
        return QSize();
    }

    const double threshold = (goal == PDFImageOptimizer::OptimizationGoal::PreferQuality) ? 1.15 : 1.0;
    const double minDpiX = minimalDpi.x();
    const double minDpiY = minimalDpi.y();

    const bool canDownsampleX = std::isfinite(minDpiX) && minDpiX > 0.0 && minDpiX > targetDpi * threshold;
    const bool canDownsampleY = std::isfinite(minDpiY) && minDpiY > 0.0 && minDpiY > targetDpi * threshold;

    if (!canDownsampleX && !canDownsampleY)
    {
        return QSize();
    }

    int newWidth = sourceSize.width();
    int newHeight = sourceSize.height();

    if (canDownsampleX)
    {
        const double scaleX = static_cast<double>(targetDpi) / minDpiX;
        newWidth = qMax(1, static_cast<int>(std::lround(sourceSize.width() * scaleX)));
    }
    if (canDownsampleY)
    {
        const double scaleY = static_cast<double>(targetDpi) / minDpiY;
        newHeight = qMax(1, static_cast<int>(std::lround(sourceSize.height() * scaleY)));
    }

    if (newWidth == sourceSize.width() && newHeight == sourceSize.height())
    {
        return QSize();
    }

    return QSize(newWidth, newHeight);
}

PDFImage::ImageCompression toImageCompression(PDFImageOptimizer::CompressionAlgorithm algorithm)
{
    switch (algorithm)
    {
        case PDFImageOptimizer::CompressionAlgorithm::Flate:
            return PDFImage::ImageCompression::Flate;
        case PDFImageOptimizer::CompressionAlgorithm::JPEG:
            return PDFImage::ImageCompression::JPEG;
        case PDFImageOptimizer::CompressionAlgorithm::JPEG2000:
            return PDFImage::ImageCompression::JPEG2000;
        case PDFImageOptimizer::CompressionAlgorithm::RunLength:
            return PDFImage::ImageCompression::RunLength;
        case PDFImageOptimizer::CompressionAlgorithm::CCITTGroup4:
        case PDFImageOptimizer::CompressionAlgorithm::JBIG2:
        case PDFImageOptimizer::CompressionAlgorithm::Auto:
            break;
    }

    return PDFImage::ImageCompression::Flate;
}

PDFImageOptimizer::CompressionProfile selectProfile(PDFImageOptimizer::ColorMode mode,
                                                    const PDFImageOptimizer::Settings& settings)
{
    switch (mode)
    {
        case PDFImageOptimizer::ColorMode::Color:
            return settings.colorProfile;
        case PDFImageOptimizer::ColorMode::Grayscale:
            return settings.grayProfile;
        case PDFImageOptimizer::ColorMode::Bitonal:
            return settings.bitonalProfile;
        case PDFImageOptimizer::ColorMode::Preserve:
        case PDFImageOptimizer::ColorMode::Auto:
            break;
    }

    return settings.colorProfile;
}

PDFImageOptimizer::ColorMode resolvePreservedColorMode(const PDFImageOptimizer::ImageInfo& info,
                                                       const PDFImageOptimizer::ImageAnalysis& analysis)
{
    // "Preserve" should keep the closest practical encoding class of the
    // original PDF image instead of routing everything through the color
    // profile. We therefore prefer original PDF metadata and fall back to
    // decoded-pixel analysis only when metadata is inconclusive.
    const QString colorSpaceName = info.colorSpaceName;

    if (info.bitsPerComponent == 1 &&
        !analysis.hasTransparency &&
        (analysis.grayscale ||
         colorSpaceName == COLOR_SPACE_NAME_DEVICE_GRAY ||
         colorSpaceName == COLOR_SPACE_NAME_CAL_GRAY))
    {
        return PDFImageOptimizer::ColorMode::Bitonal;
    }

    if (colorSpaceName == COLOR_SPACE_NAME_DEVICE_GRAY ||
        colorSpaceName == COLOR_SPACE_NAME_CAL_GRAY)
    {
        return PDFImageOptimizer::ColorMode::Grayscale;
    }

    if (colorSpaceName == COLOR_SPACE_NAME_DEVICE_RGB ||
        colorSpaceName == COLOR_SPACE_NAME_DEVICE_CMYK ||
        colorSpaceName == COLOR_SPACE_NAME_ICCBASED ||
        colorSpaceName == COLOR_SPACE_NAME_INDEXED)
    {
        return PDFImageOptimizer::ColorMode::Color;
    }

    if (analysis.grayscale)
    {
        return (info.bitsPerComponent == 1 && !analysis.hasTransparency)
            ? PDFImageOptimizer::ColorMode::Bitonal
            : PDFImageOptimizer::ColorMode::Grayscale;
    }

    return PDFImageOptimizer::ColorMode::Color;
}

QImage applyAlphaHandlingForPreview(const QImage& image, PDFImage::AlphaHandling alphaHandling)
{
    // Keep preview behavior aligned with the actual encoder. In particular,
    // "flatten to white" must be visible in the preview, otherwise the dialog
    // can suggest that transparency survives even when it will be removed.
    if (image.isNull() || !image.hasAlphaChannel() || alphaHandling == PDFImage::AlphaHandling::DropAlphaPreserveColors)
    {
        return image;
    }

    QImage rgba = image.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
        return image;
    }

    QImage flattened(rgba.size(), QImage::Format_RGB32);
    if (flattened.isNull())
    {
        return image;
    }

    for (int y = 0; y < rgba.height(); ++y)
    {
        const uchar* src = rgba.constScanLine(y);
        QRgb* dst = reinterpret_cast<QRgb*>(flattened.scanLine(y));

        for (int x = 0; x < rgba.width(); ++x)
        {
            const int alpha = src[3];
            const int red = (src[0] * alpha + 255 * (255 - alpha)) / 255;
            const int green = (src[1] * alpha + 255 * (255 - alpha)) / 255;
            const int blue = (src[2] * alpha + 255 * (255 - alpha)) / 255;
            dst[x] = qRgb(red, green, blue);
            src += 4;
        }
    }

    return flattened;
}

int adjustJpegQuality(int quality, PDFImageOptimizer::OptimizationGoal goal)
{
    if (goal == PDFImageOptimizer::OptimizationGoal::MinimumSize)
    {
        return qBound(20, quality - 10, 95);
    }

    return qBound(30, quality + 5, 100);
}

float adjustJpeg2000Rate(float rate, PDFImageOptimizer::OptimizationGoal goal)
{
    if (rate <= 0.0f)
    {
        return rate;
    }

    if (goal == PDFImageOptimizer::OptimizationGoal::MinimumSize)
    {
        return qMin(rate * 1.2f, 10.0f);
    }

    return qMax(rate * 0.85f, 0.1f);
}

PDFImageOptimizer::CompressionAlgorithm chooseAutoAlgorithm(const PDFImageOptimizer::ImageAnalysis& analysis,
                                                            PDFImageOptimizer::ColorMode colorMode)
{
    if (colorMode == PDFImageOptimizer::ColorMode::Bitonal)
    {
        return PDFImageOptimizer::CompressionAlgorithm::Flate;
    }

    if (analysis.kind == PDFImageOptimizer::ImageAnalysis::Kind::Photo)
    {
        return PDFImageOptimizer::CompressionAlgorithm::JPEG;
    }

    return PDFImageOptimizer::CompressionAlgorithm::Flate;
}

PDFImageOptimizer::ColorMode resolveColorMode(const PDFImageOptimizer::Settings& settings,
                                              const PDFImageOptimizer::ImageAnalysis& analysis)
{
    if (!settings.autoMode)
    {
        return settings.colorMode == PDFImageOptimizer::ColorMode::Auto ? PDFImageOptimizer::ColorMode::Preserve : settings.colorMode;
    }

    if (settings.colorMode != PDFImageOptimizer::ColorMode::Auto)
    {
        return settings.colorMode;
    }

    if (analysis.kind == PDFImageOptimizer::ImageAnalysis::Kind::TextScan)
    {
        return PDFImageOptimizer::ColorMode::Bitonal;
    }

    if (analysis.grayscale)
    {
        return PDFImageOptimizer::ColorMode::Grayscale;
    }

    return PDFImageOptimizer::ColorMode::Color;
}

QImage applyColorMode(const QImage& image,
                      PDFImageOptimizer::ColorMode mode,
                      int monochromeThreshold)
{
    switch (mode)
    {
        case PDFImageOptimizer::ColorMode::Grayscale:
            return image.convertToFormat(QImage::Format_Grayscale8);
        case PDFImageOptimizer::ColorMode::Bitonal:
        {
            PDFImageConversion conversion;
            conversion.setImage(image);
            if (monochromeThreshold >= 0)
            {
                conversion.setConversionMethod(PDFImageConversion::ConversionMethod::Manual);
                conversion.setThreshold(qBound(0, monochromeThreshold, 255));
            }
            else
            {
                conversion.setConversionMethod(PDFImageConversion::ConversionMethod::Automatic);
            }
            if (conversion.convert())
            {
                return conversion.getConvertedImage();
            }
            return image.convertToFormat(QImage::Format_Grayscale8);
        }
        case PDFImageOptimizer::ColorMode::Preserve:
        case PDFImageOptimizer::ColorMode::Color:
        case PDFImageOptimizer::ColorMode::Auto:
            return image;
    }

    return image;
}

QImage scaleForPreview(const QImage& image, const QSize& targetSize, PDFImage::ResampleFilter filter)
{
    if (!targetSize.isValid() || targetSize.isEmpty() || targetSize == image.size())
    {
        return image;
    }

    Qt::TransformationMode mode = (filter == PDFImage::ResampleFilter::Nearest) ? Qt::FastTransformation : Qt::SmoothTransformation;
    return image.scaled(targetSize, Qt::IgnoreAspectRatio, mode);
}

QImage simulateJpegCompression(const QImage& image, int quality)
{
    QByteArray buffer;
    QBuffer device(&buffer);
    if (!device.open(QIODevice::WriteOnly))
    {
        return image;
    }

    QImageWriter writer(&device, "jpg");
    writer.setQuality(quality);
    if (!writer.write(image))
    {
        return image;
    }

    QImage decoded;
    decoded.loadFromData(buffer, "jpg");
    return decoded.isNull() ? image : decoded;
}

PDFDictionary mergeDictionaries(const PDFDictionary& base,
                                const PDFDictionary& original,
                                const std::unordered_set<QByteArray>& blockedKeys)
{
    PDFDictionary merged = base;
    for (size_t i = 0; i < original.getCount(); ++i)
    {
        const PDFInplaceOrMemoryString& key = original.getKey(i);
        const QByteArray keyString = key.getString();
        if (blockedKeys.count(keyString) == 0 && !merged.hasKey(keyString))
        {
            merged.addEntry(key, PDFObject(original.getValue(i)));
        }
    }
    return merged;
}

} // namespace

PDFImageOptimizer::Settings PDFImageOptimizer::Settings::createDefault()
{
    Settings settings;
    settings.enabled = false;
    settings.autoMode = true;
    settings.colorMode = ColorMode::Auto;
    settings.goal = OptimizationGoal::PreferQuality;
    settings.keepOriginalIfLarger = true;
    settings.preserveTransparency = true;
    settings.colorProfile.targetDpi = 150;
    settings.grayProfile.targetDpi = 150;
    settings.bitonalProfile.targetDpi = 300;
    settings.colorProfile.jpegQuality = 85;
    settings.grayProfile.jpegQuality = 85;
    settings.colorProfile.enablePngPredictor = true;
    settings.grayProfile.enablePngPredictor = true;
    settings.bitonalProfile.enablePngPredictor = true;
    return settings;
}

std::vector<PDFImageOptimizer::ImageInfo> PDFImageOptimizer::collectImageInfos(const PDFDocument* document)
{
    std::vector<ImageInfo> results;
    if (!document)
    {
        return results;
    }

    PDFImageCompressor compressor;
    PDFImageCompressor::ImageStatisticsList stats = compressor.collectImages(document);
    results.reserve(stats.size());

    for (const PDFImageCompressor::ImageStatistics& stat : stats)
    {
        ImageInfo info;
        info.reference = stat.reference;
        info.image = stat.image;
        info.minimalDpi = stat.minimalDpi;
        info.pixelSize = stat.image.size();
        info.hasTransparency = hasNonOpaquePixel(stat.image);

        const PDFObject object = document->getObjectByReference(stat.reference);
        if (object.isStream())
        {
            const PDFStream* stream = object.getStream();
            const PDFDictionary* dictionary = stream->getDictionary();

            info.originalBytes = stream->getContent() ? stream->getContent()->size() : 0;
            if (dictionary)
            {
                info.filterName = readFilterName(document, dictionary);
                info.colorSpaceName = readNameFromColorSpace(document, dictionary->get("ColorSpace"));
                info.bitsPerComponent = PDFDocumentDataLoaderDecorator(document).readIntegerFromDictionary(dictionary, "BitsPerComponent", 8);
                info.isImageMask = PDFDocumentDataLoaderDecorator(document).readBooleanFromDictionary(dictionary, "ImageMask", false);
            }

            if (dictionary && dictionary->hasKey("SMask"))
            {
                const PDFObject& maskObject = document->getObject(dictionary->get("SMask"));
                if (maskObject.isStream())
                {
                    info.hasSoftMask = true;
                    const PDFStream* maskStream = maskObject.getStream();
                    if (maskStream && maskStream->getContent())
                    {
                        info.originalBytes += maskStream->getContent()->size();
                    }
                }
            }
        }

        results.push_back(std::move(info));
    }

    return results;
}

PDFImageOptimizer::ImageAnalysis PDFImageOptimizer::analyzeImage(const QImage& image)
{
    ImageAnalysis analysis;
    if (image.isNull())
    {
        return analysis;
    }

    analysis.grayscale = true;

    QImage sample = image;
    if (qMax(image.width(), image.height()) > ANALYSIS_MAX_SIZE)
    {
        sample = image.scaled(ANALYSIS_MAX_SIZE, ANALYSIS_MAX_SIZE, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    QImage rgba = sample.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
        return analysis;
    }

    std::unordered_set<int> uniqueColors;
    uniqueColors.reserve(256);

    const int width = rgba.width();
    const int height = rgba.height();
    const int pixelCount = width * height;

    double sum = 0.0;
    double sumSq = 0.0;
    int edgeCount = 0;

    for (int y = 0; y < height; ++y)
    {
        const uchar* row = rgba.constScanLine(y);
        const uchar* prevRow = y > 0 ? rgba.constScanLine(y - 1) : nullptr;
        for (int x = 0; x < width; ++x)
        {
            const int r = row[0];
            const int g = row[1];
            const int b = row[2];
            const int a = row[3];

            if (a != 255)
            {
                analysis.hasTransparency = true;
            }

            if (analysis.grayscale &&
                (std::abs(r - g) > GRAYSCALE_TOLERANCE || std::abs(g - b) > GRAYSCALE_TOLERANCE))
            {
                analysis.grayscale = false;
            }

            const int quantized = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
            if (uniqueColors.size() < MAX_UNIQUE_COLOR_SAMPLES)
            {
                uniqueColors.insert(quantized);
            }

            const int luma = qGray(r, g, b);
            sum += luma;
            sumSq += static_cast<double>(luma) * luma;

            if (prevRow && x > 0)
            {
                const int leftLuma = qGray(row[-4], row[-3], row[-2]);
                const int upLuma = qGray(prevRow[0], prevRow[1], prevRow[2]);
                const int gradient = std::abs(luma - leftLuma) + std::abs(luma - upLuma);
                if (gradient > 50)
                {
                    ++edgeCount;
                }
            }

            row += 4;
            if (prevRow)
            {
                prevRow += 4;
            }
        }
    }

    analysis.uniqueColors = static_cast<int>(uniqueColors.size());

    if (pixelCount > 0)
    {
        const double mean = sum / pixelCount;
        analysis.variance = (sumSq / pixelCount) - (mean * mean);
        analysis.edgeDensity = static_cast<double>(edgeCount) / pixelCount;
    }

    if (analysis.grayscale && analysis.uniqueColors <= 2)
    {
        analysis.kind = ImageAnalysis::Kind::TextScan;
    }
    else if (analysis.uniqueColors <= 32 || (analysis.edgeDensity > 0.22 && analysis.variance < 600.0))
    {
        analysis.kind = ImageAnalysis::Kind::LineArt;
    }
    else
    {
        analysis.kind = ImageAnalysis::Kind::Photo;
    }

    return analysis;
}

PDFImageOptimizer::ResolvedPlan PDFImageOptimizer::resolvePlan(const ImageInfo& info,
                                                              const Settings& settings)
{
    ResolvedPlan plan;
    plan.originalBytes = info.originalBytes;
    const ImageAnalysis analysis = analyzeImage(info.image);
    ColorMode resolvedMode = resolveColorMode(settings, analysis);
    if (resolvedMode == ColorMode::Preserve)
    {
        resolvedMode = resolvePreservedColorMode(info, analysis);
    }
    plan.resolvedColorMode = resolvedMode;

    const CompressionProfile profile = selectProfile(resolvedMode, settings);
    plan.targetSize = computeTargetSize(info.image.size(), info.minimalDpi, profile.targetDpi, settings.goal);

    CompressionAlgorithm algorithm = profile.algorithm;
    if (algorithm == CompressionAlgorithm::Auto)
    {
        if (settings.autoMode)
        {
            algorithm = chooseAutoAlgorithm(analysis, resolvedMode);
        }
        else
        {
            algorithm = (resolvedMode == ColorMode::Bitonal) ? CompressionAlgorithm::Flate : CompressionAlgorithm::JPEG;
        }
    }

    if (resolvedMode == ColorMode::Bitonal &&
        (algorithm == CompressionAlgorithm::JPEG || algorithm == CompressionAlgorithm::JPEG2000))
    {
        algorithm = CompressionAlgorithm::Flate;
        plan.hadUnsupportedCompression = true;
    }

    if ((algorithm == CompressionAlgorithm::JBIG2 && !supportsJbig2Encoder()) ||
        (algorithm == CompressionAlgorithm::CCITTGroup4 && !supportsCcittEncoder()))
    {
        algorithm = CompressionAlgorithm::Flate;
        plan.hadUnsupportedCompression = true;
    }

    plan.algorithm = algorithm;
    plan.useSoftMask = settings.preserveTransparency && analysis.hasTransparency;

    PDFImage::ImageEncodeOptions options;
    options.compression = toImageCompression(algorithm);
    options.colorMode = PDFImage::ImageColorMode::Preserve;
    options.targetSize = plan.targetSize;
    options.jpegQuality = adjustJpegQuality(profile.jpegQuality, settings.goal);
    options.jpeg2000Rate = adjustJpeg2000Rate(profile.jpeg2000Rate, settings.goal);
    options.monochromeThreshold = profile.monochromeThreshold;
    options.enablePngPredictor = profile.enablePngPredictor;
    options.resampleFilter = profile.resampleFilter;
    options.alphaHandling = plan.useSoftMask ? PDFImage::AlphaHandling::DropAlphaPreserveColors : PDFImage::AlphaHandling::FlattenToWhite;

    switch (resolvedMode)
    {
        case ColorMode::Color:
            options.colorMode = PDFImage::ImageColorMode::Color;
            break;
        case ColorMode::Preserve:
            options.colorMode = PDFImage::ImageColorMode::Preserve;
            break;
        case ColorMode::Grayscale:
            options.colorMode = PDFImage::ImageColorMode::Grayscale;
            break;
        case ColorMode::Bitonal:
            options.colorMode = PDFImage::ImageColorMode::Monochrome;
            if (options.monochromeThreshold >= 0)
            {
                options.monochromeThreshold = qBound(0, options.monochromeThreshold, 255);
            }
            break;
        case ColorMode::Auto:
            options.colorMode = PDFImage::ImageColorMode::Preserve;
            break;
    }

    plan.encodeOptions = options;
    return plan;
}

QImage PDFImageOptimizer::createPreviewImage(const ImageInfo& info, const ResolvedPlan& plan, bool simulateCompression)
{
    if (info.image.isNull())
    {
        return QImage();
    }

    QImage image = scaleForPreview(info.image, plan.targetSize, plan.encodeOptions.resampleFilter);
    image = applyAlphaHandlingForPreview(image, plan.encodeOptions.alphaHandling);
    image = applyColorMode(image, plan.resolvedColorMode, plan.encodeOptions.monochromeThreshold);

    if (simulateCompression && plan.algorithm == CompressionAlgorithm::JPEG)
    {
        image = simulateJpegCompression(image, plan.encodeOptions.jpegQuality);
    }

    return image;
}

int PDFImageOptimizer::estimateEncodedBytes(const ImageInfo& info,
                                            const ResolvedPlan& plan,
                                            PDFRenderErrorReporter* reporter)
{
    if (info.image.isNull())
    {
        return 0;
    }

    PDFRenderErrorReporterDummy dummyReporter;
    if (!reporter)
    {
        reporter = &dummyReporter;
    }

    int totalBytes = 0;
    try
    {
        PDFStream stream = PDFImage::createStreamFromImage(info.image, plan.encodeOptions, reporter);
        totalBytes += stream.getContent() ? stream.getContent()->size() : 0;
    }
    catch (const PDFException&)
    {
        return 0;
    }

    if (plan.useSoftMask)
    {
        QImage mask = extractAlphaMask(info.image);
        if (!mask.isNull())
        {
            PDFImage::ImageEncodeOptions maskOptions = plan.encodeOptions;
            maskOptions.compression = PDFImage::ImageCompression::Flate;
            maskOptions.colorMode = PDFImage::ImageColorMode::Grayscale;
            maskOptions.alphaHandling = PDFImage::AlphaHandling::FlattenToWhite;

            try
            {
                PDFStream maskStream = PDFImage::createStreamFromImage(mask, maskOptions, reporter);
                totalBytes += maskStream.getContent() ? maskStream.getContent()->size() : 0;
            }
            catch (const PDFException&)
            {
                return totalBytes;
            }
        }
    }

    return totalBytes;
}

PDFDocument PDFImageOptimizer::optimize(const PDFDocument* document,
                                        const Settings& settings,
                                        const ImageOverrides& overrides,
                                        PDFProgress* progress,
                                        PDFRenderErrorReporter* reporter,
                                        std::vector<ImageResult>* results) const
{
    if (!document || !settings.enabled)
    {
        if (!document)
        {
            return PDFDocument();
        }

        PDFObjectStorage storage = document->getStorage();
        return PDFDocument(std::move(storage), document->getInfo()->version, document->getSourceDataHash());
    }

    std::vector<ImageInfo> infos = collectImageInfos(document);
    if (infos.empty())
    {
        PDFObjectStorage storage = document->getStorage();
        return PDFDocument(std::move(storage), document->getInfo()->version, document->getSourceDataHash());
    }

    if (progress)
    {
        ProgressStartupInfo info;
        info.showDialog = true;
        info.text = PDFTranslationContext::tr("Optimizing images...");
        progress->start(infos.size(), std::move(info));
    }

    struct EncodedImage
    {
        bool keepOriginal = true;
        PDFObjectReference reference;
        PDFStream stream;
        std::optional<PDFStream> maskStream;
        std::optional<PDFObjectReference> existingMaskReference;
        int originalBytes = 0;
        int newBytes = 0;
    };

    std::vector<EncodedImage> encodedResults;
    encodedResults.reserve(infos.size());

    PDFRenderErrorReporterDummy dummyReporter;
    if (!reporter)
    {
        reporter = &dummyReporter;
    }

    for (const ImageInfo& info : infos)
    {
        EncodedImage encoded;
        encoded.reference = info.reference;
        encoded.originalBytes = info.originalBytes;

        auto overrideIt = overrides.find(info.reference);
        if (overrideIt != overrides.end() && !overrideIt->second.enabled)
        {
            encodedResults.push_back(std::move(encoded));
            if (progress)
            {
                progress->step();
            }
            continue;
        }

        const Settings& effectiveSettings = (overrideIt != overrides.end() && overrideIt->second.useCustomSettings) ? overrideIt->second.settings : settings;
        ResolvedPlan plan = resolvePlan(info, effectiveSettings);
        plan.originalBytes = info.originalBytes;

        if (info.isImageMask)
        {
            encodedResults.push_back(std::move(encoded));
            if (progress)
            {
                progress->step();
            }
            continue;
        }

        try
        {
            PDFStream stream = PDFImage::createStreamFromImage(info.image, plan.encodeOptions, reporter);
            encoded.stream = std::move(stream);
        }
        catch (const PDFException&)
        {
            encodedResults.push_back(std::move(encoded));
            if (progress)
            {
                progress->step();
            }
            continue;
        }

        encoded.newBytes = encoded.stream.getContent() ? encoded.stream.getContent()->size() : 0;

        if (plan.useSoftMask)
        {
            QImage mask = extractAlphaMask(info.image);
            if (!mask.isNull())
            {
                PDFImage::ImageEncodeOptions maskOptions = plan.encodeOptions;
                maskOptions.compression = PDFImage::ImageCompression::Flate;
                maskOptions.colorMode = PDFImage::ImageColorMode::Grayscale;
                maskOptions.alphaHandling = PDFImage::AlphaHandling::FlattenToWhite;

                try
                {
                    PDFStream maskStream = PDFImage::createStreamFromImage(mask, maskOptions, reporter);
                    encoded.maskStream = std::move(maskStream);
                    encoded.newBytes += encoded.maskStream->getContent() ? encoded.maskStream->getContent()->size() : 0;
                }
                catch (const PDFException&)
                {
                    encoded.maskStream.reset();
                }
            }
        }

        if (effectiveSettings.keepOriginalIfLarger && encoded.originalBytes > 0 && encoded.newBytes >= encoded.originalBytes)
        {
            encoded.keepOriginal = true;
        }
        else
        {
            encoded.keepOriginal = false;

            const PDFObject object = document->getObjectByReference(info.reference);
            if (object.isStream())
            {
                const PDFStream* originalStream = object.getStream();
                const PDFDictionary* originalDict = originalStream->getDictionary();
                if (originalDict)
                {
                    static const std::unordered_set<QByteArray> blocked = {
                        "Type", "Subtype", "Width", "Height", "BitsPerComponent",
                        "ColorSpace", "Filter", "DecodeParms", "Length", "Decode",
                        "Mask", "SMask", "ImageMask", "SMaskInData"
                    };

                    PDFDictionary merged = mergeDictionaries(*encoded.stream.getDictionary(), *originalDict, blocked);
                    if (originalDict->hasKey("SMask"))
                    {
                        const PDFObject& maskObject = originalDict->get("SMask");
                        if (maskObject.isReference())
                        {
                            encoded.existingMaskReference = maskObject.getReference();
                        }
                    }

                    const QByteArray* content = encoded.stream.getContent();
                    QByteArray contentCopy = content ? *content : QByteArray();
                    encoded.stream = PDFStream(std::move(merged), std::move(contentCopy));
                }
            }
        }

        encodedResults.push_back(std::move(encoded));
        if (progress)
        {
            progress->step();
        }
    }

    if (progress)
    {
        progress->finish();
    }

    PDFObjectStorage storage = document->getStorage();

    if (results)
    {
        results->clear();
        results->reserve(encodedResults.size());
    }

    for (const EncodedImage& encoded : encodedResults)
    {
        if (encoded.keepOriginal)
        {
            if (results)
            {
                results->push_back(ImageResult{ encoded.reference, true, encoded.originalBytes, encoded.newBytes, QString() });
            }
            continue;
        }

        PDFStream stream = encoded.stream;

        if (encoded.maskStream)
        {
            PDFObjectReference maskReference;
            if (encoded.existingMaskReference)
            {
                maskReference = *encoded.existingMaskReference;
                storage.setObject(maskReference, PDFObject::createStream(std::make_shared<PDFStream>(*encoded.maskStream)));
            }
            else
            {
                maskReference = storage.addObject(PDFObject::createStream(std::make_shared<PDFStream>(*encoded.maskStream)));
            }

            PDFDictionary updatedDictionary = *stream.getDictionary();
            updatedDictionary.setEntry(PDFInplaceOrMemoryString("SMask"), PDFObject::createReference(maskReference));
            stream = PDFStream(std::move(updatedDictionary), QByteArray(*stream.getContent()));
        }

        storage.setObject(encoded.reference, PDFObject::createStream(std::make_shared<PDFStream>(std::move(stream))));

        if (results)
        {
            results->push_back(ImageResult{ encoded.reference, false, encoded.originalBytes, encoded.newBytes, QString() });
        }
    }

    return PDFDocument(std::move(storage), document->getInfo()->version, document->getSourceDataHash());
}

}   // namespace pdf
