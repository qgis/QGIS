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

#include "pdfimage.h"
#include "pdfdocument.h"
#include "pdfconstants.h"
#include "pdfexception.h"
#include "pdfutils.h"
#include "pdfjbig2decoder.h"
#include "pdfccittfaxdecoder.h"
#include "pdfstreamfilters.h"
#include "pdfimageconversion.h"

#include "config.h"

#include <openjpeg.h>
#include <jpeglib.h>

#include <array>
#include <algorithm>
#include <limits>
#include <cstring>
#include <memory>
#include <QString>
#include <cstdint>

#include "pdfdbgheap.h"

namespace pdf
{

namespace
{

struct PreparedImageData
{
    QByteArray pixels;
    int width = 0;
    int height = 0;
    int components = 0;
    int bitsPerComponent = 0;
    int stride = 0;
    PDFImage::ImageColorMode colorMode = PDFImage::ImageColorMode::Color;
    std::vector<PDFReal> decode;
};

static int clampThreshold(int threshold)
{
    return std::clamp(threshold, 0, 255);
}

static Qt::TransformationMode getTransformationMode(PDFImage::ResampleFilter filter)
{
    switch (filter)
    {
        case PDFImage::ResampleFilter::Nearest:
            return Qt::FastTransformation;

        case PDFImage::ResampleFilter::Bilinear:
        case PDFImage::ResampleFilter::Bicubic:
        case PDFImage::ResampleFilter::Lanczos:
            return Qt::SmoothTransformation;
    }

    return Qt::SmoothTransformation;
}

static QImage normalizeImageToArgb32(const QImage& source,
                                     PDFImage::AlphaHandling alphaHandling,
                                     bool& hadTransparency)
{
    QImage normalized = source.convertToFormat(QImage::Format_RGBA8888);
    if (normalized.isNull())
    {
        throw PDFException(PDFTranslationContext::tr("Failed to normalize image for monochrome conversion."));
    }

    QImage result(normalized.size(), QImage::Format_ARGB32);
    if (result.isNull())
    {
        throw PDFException(PDFTranslationContext::tr("Failed to allocate intermediate image buffer."));
    }

    hadTransparency = false;

    for (int y = 0; y < normalized.height(); ++y)
    {
        const uchar* src = normalized.constScanLine(y);
        QRgb* dst = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < normalized.width(); ++x)
        {
            const int r = src[0];
            const int g = src[1];
            const int b = src[2];
            const int a = src[3];

            if (a != 255)
            {
                hadTransparency = true;
            }

            if (alphaHandling == PDFImage::AlphaHandling::DropAlphaPreserveColors)
            {
                dst[x] = qRgba(r, g, b, 255);
            }
            else
            {
                const int blendedR = (r * a + 255 * (255 - a)) / 255;
                const int blendedG = (g * a + 255 * (255 - a)) / 255;
                const int blendedB = (b * a + 255 * (255 - a)) / 255;
                dst[x] = qRgba(blendedR, blendedG, blendedB, 255);
            }
            src += 4;
        }
    }

    return result;
}

static PreparedImageData prepareImageData(const QImage& source,
                                          const PDFImage::ImageEncodeOptions& options,
                                          PDFRenderErrorReporter* reporter)
{
    if (source.isNull())
    {
        throw PDFException(PDFTranslationContext::tr("Cannot encode empty image."));
    }

    QImage working = source;
    if (options.targetSize.isValid() && !options.targetSize.isEmpty() && options.targetSize != working.size())
    {
        working = source.scaled(options.targetSize, Qt::IgnoreAspectRatio, getTransformationMode(options.resampleFilter));
    }

    if (working.width() <= 0 || working.height() <= 0)
    {
        throw PDFException(PDFTranslationContext::tr("Invalid target size for image encoding."));
    }

    PDFImage::ImageColorMode resolvedMode = options.colorMode;
    if (resolvedMode == PDFImage::ImageColorMode::Preserve)
    {
        // Preserve the most specific source representation we can infer from
        // QImage itself. Monochrome inputs should remain 1-bit instead of being
        // widened to 8-bit grayscale during a round-trip encode.
        if (working.format() == QImage::Format_Mono || working.format() == QImage::Format_MonoLSB)
        {
            resolvedMode = PDFImage::ImageColorMode::Monochrome;
        }
        else
        {
            resolvedMode = working.isGrayscale() ? PDFImage::ImageColorMode::Grayscale : PDFImage::ImageColorMode::Color;
        }
    }

    const bool supportsBinaryMonochrome =
        options.compression == PDFImage::ImageCompression::Flate ||
        options.compression == PDFImage::ImageCompression::RunLength;

    PreparedImageData prepared;
    prepared.width = working.width();
    prepared.height = working.height();
    bool hadTransparency = false;
    QImage flattened = normalizeImageToArgb32(working, options.alphaHandling, hadTransparency);

    if (resolvedMode == PDFImage::ImageColorMode::Monochrome && !supportsBinaryMonochrome)
    {
        if (reporter)
        {
            reporter->reportRenderErrorOnce(RenderErrorType::Warning,
                PDFTranslationContext::tr("Selected compression does not support 1-bit monochrome images; grayscale encoding will be used instead."));
        }
        resolvedMode = PDFImage::ImageColorMode::Grayscale;
    }

    prepared.colorMode = resolvedMode;

    switch (resolvedMode)
    {
        case PDFImage::ImageColorMode::Color:
        {
            prepared.components = 3;
            prepared.bitsPerComponent = 8;
            prepared.stride = prepared.components * prepared.width;
            prepared.pixels.resize(prepared.stride * prepared.height);

            for (int y = 0; y < prepared.height; ++y)
            {
                const QRgb* src = reinterpret_cast<const QRgb*>(flattened.constScanLine(y));
                uchar* dst = reinterpret_cast<uchar*>(prepared.pixels.data() + y * prepared.stride);

                for (int x = 0; x < prepared.width; ++x)
                {
                    const QRgb pixel = src[x];
                    *dst++ = static_cast<uchar>(qRed(pixel));
                    *dst++ = static_cast<uchar>(qGreen(pixel));
                    *dst++ = static_cast<uchar>(qBlue(pixel));
                }
            }
            break;
        }

        case PDFImage::ImageColorMode::Grayscale:
        {
            prepared.components = 1;
            prepared.bitsPerComponent = 8;
            prepared.stride = prepared.width;
            prepared.pixels.resize(prepared.stride * prepared.height);

            for (int y = 0; y < prepared.height; ++y)
            {
                const QRgb* src = reinterpret_cast<const QRgb*>(flattened.constScanLine(y));
                uchar* dst = reinterpret_cast<uchar*>(prepared.pixels.data() + y * prepared.stride);

                for (int x = 0; x < prepared.width; ++x)
                {
                    dst[x] = static_cast<uchar>(qGray(src[x]));
                }
            }
            break;
        }

        case PDFImage::ImageColorMode::Monochrome:
        {
            prepared.components = 1;
            prepared.bitsPerComponent = 1;
            prepared.stride = (prepared.width + 7) / 8;
            prepared.decode = { 0.0, 1.0 };
            prepared.pixels.resize(prepared.stride * prepared.height);
            std::memset(prepared.pixels.data(), 0, prepared.pixels.size());

            PDFImageConversion conversion;
            conversion.setImage(flattened);
            if (options.monochromeThreshold < 0)
            {
                conversion.setConversionMethod(PDFImageConversion::ConversionMethod::Automatic);
            }
            else
            {
                conversion.setConversionMethod(PDFImageConversion::ConversionMethod::Manual);
                conversion.setThreshold(clampThreshold(options.monochromeThreshold));
            }

            if (!conversion.convert())
            {
                throw PDFException(PDFTranslationContext::tr("Failed to convert image to monochrome."));
            }

            const QImage bitonal = conversion.getConvertedImage();
            if (bitonal.isNull() || bitonal.format() != QImage::Format_Mono)
            {
                throw PDFException(PDFTranslationContext::tr("Unexpected pixel format after monochrome conversion."));
            }

            const int bytesPerLine = bitonal.bytesPerLine();
            for (int y = 0; y < prepared.height; ++y)
            {
                const uchar* srcLine = bitonal.constScanLine(y);
                uchar* dstLine = reinterpret_cast<uchar*>(prepared.pixels.data() + y * prepared.stride);
                std::memcpy(dstLine, srcLine, std::min(prepared.stride, bytesPerLine));
            }
            break;
        }

        default:
            throw PDFException(PDFTranslationContext::tr("Unsupported image color mode."));
    }

    if (hadTransparency && reporter && options.alphaHandling == PDFImage::AlphaHandling::FlattenToWhite)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Warning,
            PDFTranslationContext::tr("Image alpha channel was composited onto white background during encoding."));
    }

    return prepared;
}

static QByteArray createPngPredictorBuffer(const PreparedImageData& data, bool enablePredictor)
{
    if (!enablePredictor)
    {
        return data.pixels;
    }

    const int rowLength = data.stride;
    QByteArray result((rowLength + 1) * data.height, 0);
    const char* src = data.pixels.constData();
    char* dst = result.data();

    for (int y = 0; y < data.height; ++y)
    {
        *dst++ = 0;
        std::memcpy(dst, src, rowLength);
        dst += rowLength;
        src += rowLength;
    }

    return result;
}

static QByteArray encodeRunLength(const QByteArray& input)
{
    QByteArray output;
    output.reserve(input.size() + input.size() / 2);

    const auto* data = reinterpret_cast<const uint8_t*>(input.constData());
    const int length = input.size();
    int index = 0;

    while (index < length)
    {
        int runLength = 1;
        while (runLength < 128 && index + runLength < length && data[index] == data[index + runLength])
        {
            ++runLength;
        }

        if (runLength > 1)
        {
            output.append(static_cast<char>(257 - runLength));
            output.append(static_cast<char>(data[index]));
            index += runLength;
        }
        else
        {
            int literalStart = index;
            ++index;

            while (index < length)
            {
                runLength = 1;
                while (runLength < 128 && index + runLength < length && data[index] == data[index + runLength])
                {
                    ++runLength;
                }

                if (runLength > 1 || (index - literalStart) >= 128)
                {
                    break;
                }

                ++index;
            }

            const int literalLength = index - literalStart;
            output.append(static_cast<char>(literalLength - 1));
            output.append(reinterpret_cast<const char*>(data + literalStart), literalLength);
        }
    }

    output.append(static_cast<char>(128));
    return output;
}

struct PDFJPEGDestination
{
    jpeg_destination_mgr manager;
    QByteArray* buffer = nullptr;
    std::array<JOCTET, 4096> storage = { };
};

static void jpegInitDestination(j_compress_ptr cinfo)
{
    auto destination = reinterpret_cast<PDFJPEGDestination*>(cinfo->dest);
    destination->manager.next_output_byte = destination->storage.data();
    destination->manager.free_in_buffer = destination->storage.size();
}

static boolean jpegEmptyOutputBuffer(j_compress_ptr cinfo)
{
    auto destination = reinterpret_cast<PDFJPEGDestination*>(cinfo->dest);
    destination->buffer->append(reinterpret_cast<const char*>(destination->storage.data()),
                                static_cast<int>(destination->storage.size()));
    destination->manager.next_output_byte = destination->storage.data();
    destination->manager.free_in_buffer = destination->storage.size();
    return TRUE;
}

static void jpegTermDestination(j_compress_ptr cinfo)
{
    auto destination = reinterpret_cast<PDFJPEGDestination*>(cinfo->dest);
    const size_t remaining = destination->storage.size() - destination->manager.free_in_buffer;
    if (remaining)
    {
        destination->buffer->append(reinterpret_cast<const char*>(destination->storage.data()),
                                    static_cast<int>(remaining));
    }
}

static QByteArray encodeJPEG(const PreparedImageData& data, int quality)
{
    if (data.bitsPerComponent != 8 || (data.components != 1 && data.components != 3))
    {
        throw PDFException(PDFTranslationContext::tr("JPEG encoder supports only 8-bit grayscale or RGB images."));
    }

    jpeg_compress_struct codec;
    jpeg_error_mgr errorManager;
    std::memset(&codec, 0, sizeof(codec));
    std::memset(&errorManager, 0, sizeof(errorManager));

    auto errorExit = [](j_common_ptr ptr)
    {
        char buffer[JMSG_LENGTH_MAX] = { };
        (ptr->err->format_message)(ptr, buffer);
        throw PDFException(PDFTranslationContext::tr("Error writing JPEG image: %1.").arg(QString::fromLatin1(buffer)));
    };

    jpeg_std_error(&errorManager);
    errorManager.error_exit = errorExit;
    codec.err = &errorManager;

    jpeg_create_compress(&codec);

    PDFJPEGDestination destination;
    destination.manager.init_destination = jpegInitDestination;
    destination.manager.empty_output_buffer = jpegEmptyOutputBuffer;
    destination.manager.term_destination = jpegTermDestination;
    destination.manager.next_output_byte = nullptr;
    destination.manager.free_in_buffer = 0;
    codec.dest = reinterpret_cast<jpeg_destination_mgr*>(&destination);

    QByteArray result;
    destination.buffer = &result;

    try
    {
        codec.image_width = data.width;
        codec.image_height = data.height;
        codec.input_components = data.components;
        codec.in_color_space = (data.components == 3) ? JCS_RGB : JCS_GRAYSCALE;

        jpeg_set_defaults(&codec);
        jpeg_set_quality(&codec, std::clamp(quality, 0, 100), TRUE);
        jpeg_start_compress(&codec, TRUE);

        while (codec.next_scanline < codec.image_height)
        {
            const JSAMPLE* rowPtr =
                reinterpret_cast<const JSAMPLE*>(data.pixels.constData() + codec.next_scanline * data.stride);
            JSAMPROW row = const_cast<JSAMPROW>(rowPtr);
            jpeg_write_scanlines(&codec, &row, 1);
        }

        jpeg_finish_compress(&codec);
        jpeg_destroy_compress(&codec);
    }
    catch (...)
    {
        jpeg_destroy_compress(&codec);
        throw;
    }

    return result;
}

struct PDFJPEG2000EncodeStream
{
    QByteArray* buffer = nullptr;
    size_t position = 0;
};

static OPJ_SIZE_T jpeg2000Write(void* p_buffer, OPJ_SIZE_T p_nb_bytes, void* p_user_data)
{
    auto stream = reinterpret_cast<PDFJPEG2000EncodeStream*>(p_user_data);
    if (!stream || !stream->buffer)
    {
        return static_cast<OPJ_SIZE_T>(-1);
    }

    const size_t requiredSize = stream->position + static_cast<size_t>(p_nb_bytes);
    if (stream->buffer->size() < static_cast<int>(requiredSize))
    {
        stream->buffer->resize(static_cast<int>(requiredSize));
    }

    std::memcpy(stream->buffer->data() + stream->position, p_buffer, static_cast<size_t>(p_nb_bytes));
    stream->position = requiredSize;
    return p_nb_bytes;
}

static OPJ_OFF_T jpeg2000Skip(OPJ_OFF_T p_nb_bytes, void* p_user_data)
{
    if (p_nb_bytes < 0)
    {
        return -1;
    }

    auto stream = reinterpret_cast<PDFJPEG2000EncodeStream*>(p_user_data);
    if (!stream || !stream->buffer)
    {
        return -1;
    }

    const size_t newPosition = stream->position + static_cast<size_t>(p_nb_bytes);
    if (stream->buffer->size() < static_cast<int>(newPosition))
    {
        stream->buffer->resize(static_cast<int>(newPosition));
    }

    stream->position = newPosition;
    return p_nb_bytes;
}

static OPJ_BOOL jpeg2000Seek(OPJ_OFF_T p_nb_bytes, void* p_user_data)
{
    if (p_nb_bytes < 0)
    {
        return OPJ_FALSE;
    }

    auto stream = reinterpret_cast<PDFJPEG2000EncodeStream*>(p_user_data);
    if (!stream || !stream->buffer)
    {
        return OPJ_FALSE;
    }

    const size_t newPosition = static_cast<size_t>(p_nb_bytes);
    if (stream->buffer->size() < static_cast<int>(newPosition))
    {
        stream->buffer->resize(static_cast<int>(newPosition));
    }

    stream->position = newPosition;
    return OPJ_TRUE;
}

struct PDFJPEG2000EncodeMessages
{
    std::vector<QString> warnings;
    std::vector<QString> errors;
};

static void jpeg2000WarningCallback(const char* message, void* userData)
{
    auto context = reinterpret_cast<PDFJPEG2000EncodeMessages*>(userData);
    context->warnings.emplace_back(QString::fromLatin1(message));
}

static void jpeg2000ErrorCallback(const char* message, void* userData)
{
    auto context = reinterpret_cast<PDFJPEG2000EncodeMessages*>(userData);
    context->errors.emplace_back(QString::fromLatin1(message));
}

static QByteArray encodeJPEG2000(const PreparedImageData& data,
                                 float rate,
                                 PDFRenderErrorReporter* reporter)
{
    if (data.bitsPerComponent != 8 || (data.components != 1 && data.components != 3))
    {
        throw PDFException(PDFTranslationContext::tr("JPEG 2000 encoder supports only 8-bit grayscale or RGB images."));
    }

    opj_cparameters_t parameters;
    opj_set_default_encoder_parameters(&parameters);
    parameters.tcp_numlayers = 1;
    parameters.irreversible = rate > 0.0f;
    parameters.tcp_rates[0] = rate > 0.0f ? rate : 0.0f;
    parameters.cp_disto_alloc = 1;
    parameters.cp_fixed_quality = 0;
    parameters.tcp_mct = (data.components == 3) ? 1 : 0;

    std::vector<opj_image_cmptparm_t> componentParameters(static_cast<size_t>(data.components));
    for (auto& component : componentParameters)
    {
        std::memset(&component, 0, sizeof(component));
        component.prec = data.bitsPerComponent;
        Q_NOWARN_DEPRECATED_PUSH
        component.bpp = data.bitsPerComponent;
        Q_NOWARN_DEPRECATED_POP
        component.sgnd = 0;
        component.dx = 1;
        component.dy = 1;
        component.w = data.width;
        component.h = data.height;
    }

    OPJ_COLOR_SPACE colorSpace = (data.components == 3) ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY;
    opj_image_t* image = opj_image_create(data.components, componentParameters.data(), colorSpace);
    if (!image)
    {
        throw PDFException(PDFTranslationContext::tr("Failed to allocate JPEG 2000 image structure."));
    }

    image->x0 = 0;
    image->y0 = 0;
    image->x1 = data.width;
    image->y1 = data.height;

    const uint8_t* src = reinterpret_cast<const uint8_t*>(data.pixels.constData());
    for (int comp = 0; comp < data.components; ++comp)
    {
        image->comps[comp].prec = data.bitsPerComponent;
        Q_NOWARN_DEPRECATED_PUSH
        image->comps[comp].bpp = data.bitsPerComponent;
        Q_NOWARN_DEPRECATED_POP
        image->comps[comp].sgnd = 0;
        image->comps[comp].dx = 1;
        image->comps[comp].dy = 1;
    }

    for (int y = 0; y < data.height; ++y)
    {
        for (int x = 0; x < data.width; ++x)
        {
            const int index = y * data.width + x;
            if (data.components == 3)
            {
                const uint8_t* pixel = src + y * data.stride + x * 3;
                image->comps[0].data[index] = pixel[0];
                image->comps[1].data[index] = pixel[1];
                image->comps[2].data[index] = pixel[2];
            }
            else
            {
                image->comps[0].data[index] = src[y * data.stride + x];
            }
        }
    }

    opj_codec_t* codec = opj_create_compress(OPJ_CODEC_JP2);
    if (!codec)
    {
        opj_image_destroy(image);
        throw PDFException(PDFTranslationContext::tr("Failed to create JPEG 2000 encoder."));
    }

    PDFJPEG2000EncodeMessages messages;
    opj_set_warning_handler(codec, jpeg2000WarningCallback, &messages);
    opj_set_error_handler(codec, jpeg2000ErrorCallback, &messages);

    if (!opj_setup_encoder(codec, &parameters, image))
    {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        throw PDFException(PDFTranslationContext::tr("Failed to setup JPEG 2000 encoder."));
    }

    PDFJPEG2000EncodeStream streamContext;
    QByteArray result;
    streamContext.buffer = &result;
    streamContext.position = 0;

    opj_stream_t* stream = opj_stream_create(1 << 16, OPJ_FALSE);
    if (!stream)
    {
        opj_destroy_codec(codec);
        opj_image_destroy(image);
        throw PDFException(PDFTranslationContext::tr("Failed to create JPEG 2000 stream."));
    }

    opj_stream_set_user_data(stream, &streamContext, nullptr);
    opj_stream_set_user_data_length(stream, 0);
    opj_stream_set_write_function(stream, jpeg2000Write);
    opj_stream_set_skip_function(stream, jpeg2000Skip);
    opj_stream_set_seek_function(stream, jpeg2000Seek);

    bool success = opj_start_compress(codec, image, stream);
    if (success)
    {
        success = opj_encode(codec, stream);
    }
    if (success)
    {
        success = opj_end_compress(codec, stream);
    }

    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);

    result.resize(static_cast<int>(streamContext.position));

    if (!messages.errors.empty())
    {
        throw PDFException(PDFTranslationContext::tr("JPEG 2000 encoder error: %1").arg(messages.errors.front()));
    }

    if (!success)
    {
        throw PDFException(PDFTranslationContext::tr("JPEG 2000 encoder failed to write image."));
    }

    if (reporter)
    {
        for (const QString& warning : messages.warnings)
        {
            reporter->reportRenderErrorOnce(RenderErrorType::Warning,
                PDFTranslationContext::tr("JPEG 2000 warning: %1").arg(warning));
        }
    }

    return result;
}

} // namespace

struct PDFJPEG2000ImageData
{
    const QByteArray* byteArray = nullptr;
    OPJ_SIZE_T position = 0;
    std::vector<PDFRenderError> errors;

    static OPJ_SIZE_T read(void* p_buffer, OPJ_SIZE_T p_nb_bytes, void* p_user_data);
    static OPJ_BOOL seek(OPJ_OFF_T p_nb_bytes, void* p_user_data);
    static OPJ_OFF_T skip(OPJ_OFF_T p_nb_bytes, void* p_user_data);
};

struct PDFJPEGDCTSource
{
    jpeg_source_mgr sourceManager;
    const QByteArray* buffer = nullptr;
    int startByte = 0;
};

PDFImage PDFImage::createImage(const PDFDocument* document,
                               const PDFStream* stream,
                               PDFColorSpacePointer colorSpace,
                               bool isSoftMask,
                               RenderingIntent renderingIntent,
                               PDFRenderErrorReporter* errorReporter)
{
    PDFImage image;
    image.m_colorSpace = colorSpace;
    image.m_renderingIntent = renderingIntent;

    const PDFDictionary* dictionary = stream->getDictionary();
    QByteArray content = document->getDecodedStream(stream);
    PDFDocumentDataLoaderDecorator loader(document);

    if (content.isEmpty())
    {
        throw PDFException(PDFTranslationContext::tr("Image has not data."));
    }

    PDFImageData::MaskingType maskingType = PDFImageData::MaskingType::None;
    std::vector<PDFInteger> mask;
    std::vector<PDFReal> decode = loader.readNumberArrayFromDictionary(dictionary, "Decode");
    bool imageMask = loader.readBooleanFromDictionary(dictionary, "ImageMask", false);
    std::vector<PDFReal> matte = loader.readNumberArrayFromDictionary(dictionary, "Matte");
    PDFInteger sMaskInData = loader.readIntegerFromDictionary(dictionary, "SMaskInData", 0);
    image.m_interpolate = loader.readBooleanFromDictionary(dictionary, "Interpolate", false);
    image.m_alternates = loader.readObjectList<PDFAlternateImage>(dictionary->get("Alternates"));
    image.m_name = loader.readNameFromDictionary(dictionary, "Name");
    image.m_structuralParent = loader.readIntegerFromDictionary(dictionary, "StructParent", 0);
    image.m_webCaptureContentSetId = loader.readStringFromDictionary(dictionary, "ID");
    image.m_OPI = dictionary->get("OPI");
    image.m_OC = dictionary->get("OC");
    image.m_metadata = dictionary->get("Metadata");
    image.m_associatedFiles = dictionary->get("AF");
    image.m_measure = dictionary->get("Measure");
    image.m_pointData = dictionary->get("PtData");

    if (isSoftMask && (imageMask || dictionary->hasKey("Mask") || dictionary->hasKey("SMask")))
    {
        throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Soft mask image can't have mask / soft mask itself."));
    }

    if (!isSoftMask && !matte.empty())
    {
        throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Regular image can't have Matte entry (used for soft masks)."));
    }

    // Set rendering intent
    if (dictionary->hasKey("Intent"))
    {
        QByteArray renderingIntentName = loader.readNameFromDictionary(dictionary, "Intent");
        if (renderingIntentName == "Perceptual")
        {
            image.m_renderingIntent = RenderingIntent::Perceptual;
        }
        else if (renderingIntentName == "AbsoluteColorimetric")
        {
            image.m_renderingIntent = RenderingIntent::AbsoluteColorimetric;
        }
        else if (renderingIntentName == "RelativeColorimetric")
        {
            image.m_renderingIntent = RenderingIntent::RelativeColorimetric;
        }
        else if (renderingIntentName == "Saturation")
        {
            image.m_renderingIntent = RenderingIntent::Saturation;
        }
    }

    // Fill Mask
    if (dictionary->hasKey("Mask"))
    {
        const PDFObject& object = document->getObject(dictionary->get("Mask"));
        if (object.isArray())
        {
            maskingType = PDFImageData::MaskingType::ColorKeyMasking;
            mask = loader.readIntegerArray(object);
        }
        else if (object.isStream())
        {
            PDFImage softMaskImage = createImage(document, object.getStream(), PDFColorSpacePointer(new PDFDeviceGrayColorSpace()), false, renderingIntent, errorReporter);

            if (softMaskImage.m_imageData.getMaskingType() != PDFImageData::MaskingType::ImageMask ||
                softMaskImage.m_imageData.getColorChannels() != 1 ||
                softMaskImage.m_imageData.getBitsPerComponent() != 1)
            {
                throw PDFRendererException(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Invalid mask image."));
            }

            // We must alter decode, because it has opposite meaning (it is transparency)
            std::vector<PDFReal> adjustedDecode = softMaskImage.m_imageData.getDecode();
            if (adjustedDecode.size() < 2)
            {
                adjustedDecode = { 0.0, 1.0};
            }
            std::swap(adjustedDecode[0], adjustedDecode[1]);

            // Create soft mask from image
            maskingType = PDFImageData::MaskingType::SoftMask;
            image.m_softMask = qMove(softMaskImage.m_imageData);
            image.m_softMask.setMaskingType(PDFImageData::MaskingType::None);
            image.m_softMask.setDecode(qMove(adjustedDecode));
        }
    }
    else if (dictionary->hasKey("SMask"))
    {
        // Parse soft mask image
        const PDFObject& softMaskObject = document->getObject(dictionary->get("SMask"));

        if (softMaskObject.isStream())
        {
            PDFImage softMaskImage = createImage(document, softMaskObject.getStream(), PDFColorSpacePointer(new PDFDeviceGrayColorSpace()), true, renderingIntent, errorReporter);
            maskingType = PDFImageData::MaskingType::SoftMask;
            image.m_softMask = qMove(softMaskImage.m_imageData);
        }
        else if (!softMaskObject.isNull())
        {
            throw PDFRendererException(RenderErrorType::NotImplemented, PDFTranslationContext::tr("Invalid soft mask object."));
        }
    }

    if (imageMask)
    {
        maskingType = PDFImageData::MaskingType::ImageMask;
    }

    // Retrieve filters
    PDFObject filters;
    if (dictionary->hasKey(PDF_STREAM_DICT_FILTER))
    {
        filters = document->getObject(dictionary->get(PDF_STREAM_DICT_FILTER));
    }
    else if (dictionary->hasKey(PDF_STREAM_DICT_FILE_FILTER))
    {
        filters = document->getObject(dictionary->get(PDF_STREAM_DICT_FILE_FILTER));
    }

    // Retrieve filter parameters
    PDFObject filterParameters;
    if (dictionary->hasKey(PDF_STREAM_DICT_DECODE_PARMS))
    {
        filterParameters = document->getObject(dictionary->get(PDF_STREAM_DICT_DECODE_PARMS));
    }
    else if (dictionary->hasKey(PDF_STREAM_DICT_FDECODE_PARMS))
    {
        filterParameters = document->getObject(dictionary->get(PDF_STREAM_DICT_FDECODE_PARMS));
    }

    QByteArray imageFilterName;
    if (filters.isName())
    {
        imageFilterName = filters.getString();
    }
    else if (filters.isArray())
    {
        const PDFArray* filterArray = filters.getArray();
        const size_t filterCount = filterArray->getCount();

        if (filterCount)
        {
            const PDFObject& object = document->getObject(filterArray->getItem(filterCount - 1));
            if (object.isName())
            {
                imageFilterName = object.getString();
            }
        }
    }

    const PDFDictionary* filterParamsDictionary = nullptr;
    if (filterParameters.isDictionary())
    {
        filterParamsDictionary = filterParameters.getDictionary();
    }
    else if (filterParameters.isArray())
    {
        const PDFArray* filterParametersArray = filterParameters.getArray();
        const size_t filterParamsCount = filterParametersArray->getCount();

        if (filterParamsCount)
        {
            const PDFObject& object = document->getObject(filterParametersArray->getItem(filterParamsCount - 1));
            if (object.isDictionary())
            {
                filterParamsDictionary = object.getDictionary();
            }
        }
    }

    if (imageFilterName == "DCTDecode" || imageFilterName == "DCT")
    {
        int colorTransform = loader.readIntegerFromDictionary(dictionary, "ColorTransform", -1);

        jpeg_decompress_struct codec;
        jpeg_error_mgr errorManager;
        std::memset(&codec, 0, sizeof(jpeg_decompress_struct));
        std::memset(&errorManager, 0, sizeof(errorManager));

        PDFJPEGDCTSource source;
        source.buffer = &content;
        std::memset(&source.sourceManager, 0, sizeof(jpeg_source_mgr));

        // Fix issue, that image doesn't start with FFD8 (start of image marker). If this
        // occurs, try to find sequence FFD8, and if we can find it, then advance the buffer.
        source.startByte = qMax(content.indexOf("\xFF\xD8"), 0);
        if (source.startByte > 0)
        {
            errorReporter->reportRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("Malformed data while reading JPEG stream. %1 bytes skipped.").arg(source.startByte));
        }

        auto errorMethod = [](j_common_ptr ptr)
        {
            char buffer[JMSG_LENGTH_MAX] = { };
            (ptr->err->format_message)(ptr, buffer);

            jpeg_destroy(ptr);
            throw PDFException(PDFTranslationContext::tr("Error reading JPEG (DCT) image: %1.").arg(QString::fromLatin1(buffer)));
        };

        auto fillInputBufferMethod = [](j_decompress_ptr decompress) -> boolean
        {
            PDFJPEGDCTSource* source = reinterpret_cast<PDFJPEGDCTSource*>(decompress->src);

            if (!source->sourceManager.next_input_byte)
            {
                const QByteArray* buffer = source->buffer;
                source->sourceManager.next_input_byte = reinterpret_cast<const JOCTET*>(buffer->constData());
                source->sourceManager.bytes_in_buffer = buffer->size();
                source->sourceManager.next_input_byte += source->startByte;
                source->sourceManager.bytes_in_buffer -= source->startByte;
                return TRUE;
            }

            return FALSE;
        };

        auto skipInputDataMethod = [](j_decompress_ptr decompress, long num_bytes)
        {
            PDFJPEGDCTSource* source = reinterpret_cast<PDFJPEGDCTSource*>(decompress->src);

            const size_t skippedBytes = qMin(source->sourceManager.bytes_in_buffer, static_cast<size_t>(num_bytes));
            source->sourceManager.next_input_byte += skippedBytes;
            source->sourceManager.bytes_in_buffer -= skippedBytes;
        };

        source.sourceManager.bytes_in_buffer = 0;
        source.sourceManager.next_input_byte = nullptr;
        source.sourceManager.init_source = [](j_decompress_ptr) { };
        source.sourceManager.fill_input_buffer = fillInputBufferMethod;
        source.sourceManager.skip_input_data = skipInputDataMethod;
        source.sourceManager.resync_to_restart = jpeg_resync_to_restart;
        source.sourceManager.term_source = [](j_decompress_ptr) { };

        jpeg_std_error(&errorManager);
        errorManager.error_exit = errorMethod;
        codec.err = &errorManager;

        jpeg_create_decompress(&codec);
        codec.src = reinterpret_cast<jpeg_source_mgr*>(&source);

        if (jpeg_read_header(&codec, TRUE) == JPEG_HEADER_OK)
        {
            // Determine color transform
            if (codec.saw_Adobe_marker)
            {
                colorTransform = codec.Adobe_transform;
            }

            // Set the input transform
            if (colorTransform > -1)
            {
                switch (codec.num_components)
                {
                    case 3:
                    {
                        codec.jpeg_color_space = colorTransform ? JCS_YCbCr : JCS_RGB;
                        break;
                    }

                    case 4:
                    {
                        codec.jpeg_color_space = colorTransform ? JCS_YCCK : JCS_CMYK;
                        break;
                    }

                    default:
                        break;
                }
            }

            jpeg_start_decompress(&codec);

            const JDIMENSION rowStride = codec.output_width * codec.output_components;
            JSAMPARRAY samples = codec.mem->alloc_sarray(reinterpret_cast<j_common_ptr>(&codec), JPOOL_IMAGE, rowStride, 1);
            JDIMENSION scanLineCount = codec.output_height;

            const unsigned int width = codec.output_width;
            const unsigned int height = codec.output_height;
            const unsigned int components = codec.output_components;
            const unsigned int bitsPerComponent =  8;
            QByteArray buffer(rowStride * height, 0);
            JSAMPROW rowData = reinterpret_cast<JSAMPROW>(buffer.data());

            while (scanLineCount)
            {
                JDIMENSION readCount = jpeg_read_scanlines(&codec, samples, 1);
                std::memcpy(rowData, samples[0], rowStride);
                scanLineCount -= readCount;
                rowData += rowStride;
            }

            jpeg_finish_decompress(&codec);
            image.m_imageData = PDFImageData(components, bitsPerComponent, width, height, rowStride, maskingType, qMove(buffer), qMove(mask), qMove(decode), qMove(matte));
        }

        jpeg_destroy_decompress(&codec);
    }
    else if (imageFilterName == "JPXDecode")
    {
        PDFJPEG2000ImageData imageData;
        imageData.byteArray = &content;
        imageData.position = 0;

        auto warningCallback = [](const char* message, void* userData)
        {
            PDFJPEG2000ImageData* data = reinterpret_cast<PDFJPEG2000ImageData*>(userData);
            data->errors.push_back(PDFRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("JPEG 2000 Warning: %1").arg(QString::fromLatin1(message))));
        };

        auto errorCallback = [](const char* message, void* userData)
        {
            PDFJPEG2000ImageData* data = reinterpret_cast<PDFJPEG2000ImageData*>(userData);
            data->errors.push_back(PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("JPEG 2000 Error: %1").arg(QString::fromLatin1(message))));
        };

        opj_dparameters_t decompressParameters;
        opj_set_default_decoder_parameters(&decompressParameters);

        const bool isIndexed = dynamic_cast<const PDFIndexedColorSpace*>(image.m_colorSpace.data());
        if (isIndexed)
        {
            // What is this flag for? When we have indexed color space, we do not want to resolve index to color
            // using the color map in the image. Instead of that, we just get indices and resolve them using
            // our color space.
            decompressParameters.flags |= OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;
        }

        constexpr CODEC_FORMAT formats[] = { OPJ_CODEC_J2K, OPJ_CODEC_JP2, OPJ_CODEC_JPT, OPJ_CODEC_JPP, OPJ_CODEC_JPX };
        for (CODEC_FORMAT format : formats)
        {
            opj_codec_t* codec = opj_create_decompress(format);

            if (!codec)
            {
                // Codec is not present
                continue;
            }

            opj_set_warning_handler(codec, warningCallback, &imageData);
            opj_set_error_handler(codec, errorCallback, &imageData);

            opj_stream_t* opjStream = opj_stream_create(content.size(), OPJ_TRUE);
            opj_stream_set_user_data(opjStream, &imageData, nullptr);
            opj_stream_set_user_data_length(opjStream, content.size());
            opj_stream_set_read_function(opjStream, &PDFJPEG2000ImageData::read);
            opj_stream_set_seek_function(opjStream, &PDFJPEG2000ImageData::seek);
            opj_stream_set_skip_function(opjStream, &PDFJPEG2000ImageData::skip);

            // Reset the stream position, clear the data
            imageData.position = 0;
            imageData.errors.clear();

            opj_image_t* jpegImage = nullptr;

            // Setup the decoder
            if (opj_setup_decoder(codec, &decompressParameters))
            {
                // Try to read the header

                if (opj_read_header(opjStream, codec, &jpegImage))
                {
                    if (opj_set_decode_area(codec, jpegImage, decompressParameters.DA_x0, decompressParameters.DA_y0, decompressParameters.DA_x1, decompressParameters.DA_y1))
                    {
                        if (opj_decode(codec, opjStream, jpegImage))
                        {
                            if (opj_end_decompress(codec, opjStream))
                            {

                            }
                        }
                    }
                }
            }

            opj_stream_destroy(opjStream);
            opj_destroy_codec(codec);

            opjStream = nullptr;
            codec = nullptr;

            // If we have a valid image, then adjust it
            if (jpegImage)
            {
                // This image type can have color space defined in the data (definition of color space in PDF
                // is only optional). So, if we doesn't have a color space, then we must determine it from the data.
                if (!image.m_colorSpace)
                {
                    switch (jpegImage->color_space)
                    {
                        case OPJ_CLRSPC_SRGB:
                            image.m_colorSpace.reset(new PDFDeviceRGBColorSpace());
                            break;

                        case OPJ_CLRSPC_GRAY:
                            image.m_colorSpace.reset(new PDFDeviceGrayColorSpace());
                            break;

                        case OPJ_CLRSPC_CMYK:
                            image.m_colorSpace.reset(new PDFDeviceCMYKColorSpace());
                            break;

                        default:
                            imageData.errors.push_back(PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Unknown color space for JPEG 2000 image.")));
                            break;
                    }

                    // Jakub Melka: Try to use ICC profile, if image has it
                    if (jpegImage->icc_profile_buf && jpegImage->icc_profile_len > 0 && image.m_colorSpace)
                    {
                        QByteArray iccProfileData(reinterpret_cast<const char*>(jpegImage->icc_profile_buf), jpegImage->icc_profile_len);
                        PDFICCBasedColorSpace::Ranges ranges = { 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0 };
                        image.m_colorSpace.reset(new PDFICCBasedColorSpace(image.m_colorSpace, ranges, qMove(iccProfileData), PDFObjectReference()));
                    }
                }

                // First we must check, if all components are valid (i.e has same width/height/precision)

                std::vector<OPJ_UINT32> ordinaryComponents;
                std::vector<OPJ_UINT32> alphaComponents;

                bool valid = true;
                const OPJ_UINT32 componentCount = jpegImage->numcomps;
                ordinaryComponents.reserve(componentCount);
                for (OPJ_UINT32 i = 0; i < componentCount; ++i)
                {
                    if (jpegImage->comps[0].w != jpegImage->comps[i].w ||
                        jpegImage->comps[0].h != jpegImage->comps[i].h ||
                        jpegImage->comps[0].prec != jpegImage->comps[i].prec ||
                        jpegImage->comps[0].sgnd != jpegImage->comps[i].sgnd)
                    {
                        valid = false;
                        break;
                    }
                    else
                    {
                        // Fill in ordinary component, or alpha component
                        if (!jpegImage->comps[i].alpha)
                        {
                            ordinaryComponents.push_back(i);
                        }
                        else
                        {
                            alphaComponents.push_back(i);
                        }
                    }
                }

                if (valid)
                {
                    const size_t colorSpaceComponentCount = image.m_colorSpace->getColorComponentCount();
                    const bool hasAlphaChannel = !alphaComponents.empty();

                    if (colorSpaceComponentCount < ordinaryComponents.size())
                    {
                        // We have too much ordinary components
                        imageData.errors.push_back(PDFRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("JPEG 2000 image has too much non-alpha channels. Ignoring %1 channels.").arg(ordinaryComponents.size() - colorSpaceComponentCount)));
                    }

                    if (alphaComponents.size() > 1)
                    {
                        // We support only one alpha channel component
                        imageData.errors.push_back(PDFRenderError(RenderErrorType::Warning, PDFTranslationContext::tr("JPEG 2000 image has too much alpha channels. Ignoring %1 alpha channels.").arg(alphaComponents.size() - 1)));
                    }

                    const OPJ_UINT32 w = jpegImage->comps[0].w;
                    const OPJ_UINT32 h = jpegImage->comps[0].h;
                    const OPJ_UINT32 prec = jpegImage->comps[0].prec;
                    const OPJ_UINT32 sgnd = jpegImage->comps[0].sgnd;

                    int signumCorrection = (sgnd) ? (1 << (prec - 1)) : 0;
                    int shiftLeft = (jpegImage->comps[0].prec < 8) ? 8 - jpegImage->comps[0].prec : 0;
                    int shiftRight = (jpegImage->comps[0].prec > 8) ? jpegImage->comps[0].prec - 8 : 0;

                    auto transformValue = [signumCorrection, isIndexed, shiftLeft, shiftRight](int value) -> unsigned char
                    {
                        value += signumCorrection;

                        if (!isIndexed)
                        {
                            // Indexed color space should have at most 255 indices, do not modify indices in this case

                            if (shiftLeft > 0)
                            {
                                value = value << shiftLeft;
                            }
                            else if (shiftRight > 0)
                            {
                                // We clamp value to the lower part (so, we use similar algorithm as in 'floor' function).
                                //
                                value = value >> shiftRight;
                            }
                        }

                        value = qBound(0, value, 255);
                        return static_cast<unsigned char>(value);
                    };

                    // Variables for image data. We convert all components to the 8-bit format
                    const size_t ordinaryComponentCount = ordinaryComponents.size();
                    unsigned int components = static_cast<unsigned int>(qMin(ordinaryComponentCount, colorSpaceComponentCount));
                    unsigned int bitsPerComponent = 8;
                    unsigned int width = w;
                    unsigned int height = h;
                    unsigned int stride = w * components;

                    QByteArray imageDataBuffer(components * width * height, 0);
                    for (unsigned int row = 0; row < h; ++row)
                    {
                        for (unsigned int col = 0; col < w; ++col)
                        {
                            for (unsigned int componentIndex = 0; componentIndex < components; ++ componentIndex)
                            {
                                int index = stride * row + col * components + componentIndex;
                                Q_ASSERT(index < imageDataBuffer.size());

                                imageDataBuffer[index] = transformValue(jpegImage->comps[ordinaryComponents[componentIndex]].data[w * row + col]);
                            }
                        }
                    }

                    image.m_imageData = PDFImageData(components, bitsPerComponent, width, height, stride, maskingType, qMove(imageDataBuffer), qMove(mask), qMove(decode), qMove(matte));
                    valid = image.m_imageData.isValid();

                    // Handle the alpha channel buffer - create soft mask. If SMaskInData equals to 1, then alpha channel is used.
                    // If SMaskInData equals to 2, then premultiplied alpha channel is used.
                    if (hasAlphaChannel && (sMaskInData == 1 || sMaskInData == 2))
                    {
                        const int alphaStride = w;
                        QByteArray alphaDataBuffer(width * height, 0);
                        const OPJ_UINT32 alphaComponentIndex = alphaComponents.front();
                        for (unsigned int row = 0; row < h; ++row)
                        {
                            for (unsigned int col = 0; col < w; ++col)
                            {
                                int index = alphaStride * row + col;
                                Q_ASSERT(index < alphaDataBuffer.size());

                                alphaDataBuffer[index] = transformValue(jpegImage->comps[alphaComponentIndex].data[w * row + col]);
                            }
                        }

                        if (sMaskInData == 2)
                        {
                            matte.resize(ordinaryComponentCount, 0.0);
                        }

                        image.m_softMask = PDFImageData(1, bitsPerComponent, width, height, alphaStride, PDFImageData::MaskingType::None, qMove(alphaDataBuffer), { }, { }, qMove(matte));
                        image.m_imageData.setMaskingType(PDFImageData::MaskingType::SoftMask);
                    }
                }
                else
                {
                    // Easiest way is to just add errors to the error list
                    imageData.errors.push_back(PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Incompatible color components for JPEG 2000 image.")));
                }

                opj_image_destroy(jpegImage);

                if (valid)
                {
                    // Image was successfully decoded
                    break;
                }
            }
        }

        // Report errors, if we have any
        if (!imageData.errors.empty())
        {
            for (const PDFRenderError& error : imageData.errors)
            {
                QString message = error.message.simplified().trimmed();
                if (error.type == RenderErrorType::Error)
                {
                    throw PDFRendererException(error.type, message);
                }
                else
                {
                    errorReporter->reportRenderError(error.type, message);
                }
            }
        }
    }
    else if (imageFilterName == "CCITTFaxDecode" || imageFilterName == "CCF")
    {
        if (!filterParamsDictionary)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid parameters for filter CCITT fax decode."));
        }

        PDFCCITTFaxDecoderParameters parameters;
        parameters.maskingType = maskingType;

        parameters.K = loader.readIntegerFromDictionary(filterParamsDictionary, "K", 0);
        parameters.hasEndOfLine = loader.readBooleanFromDictionary(filterParamsDictionary, "EndOfLine", false);
        parameters.hasEncodedByteAlign = loader.readBooleanFromDictionary(filterParamsDictionary, "EncodedByteAlign", false);
        parameters.columns = loader.readIntegerFromDictionary(filterParamsDictionary, "Columns", 1728);
        parameters.rows = loader.readIntegerFromDictionary(filterParamsDictionary, "Rows", 0);
        parameters.hasEndOfBlock = loader.readBooleanFromDictionary(filterParamsDictionary, "EndOfBlock", true);
        parameters.hasBlackIsOne = loader.readBooleanFromDictionary(filterParamsDictionary, "BlackIs1", false);
        parameters.damagedRowsBeforeError = loader.readIntegerFromDictionary(filterParamsDictionary, "DamagedRowsBeforeError", 0);
        parameters.decode = !decode.empty() ? qMove(decode) : std::vector<PDFReal>({ 0.0, 1.0 });

        QByteArray imageDataBuffer = document->getDecodedStream(stream);
        PDFCCITTFaxDecoder decoder(&imageDataBuffer, parameters);
        image.m_imageData = decoder.decode();
    }
    else if (imageFilterName == "JBIG2Decode")
    {
        QByteArray data = document->getDecodedStream(stream);
        QByteArray globalData;
        if (filterParamsDictionary)
        {
            const PDFObject& globalDataObject = document->getObject(filterParamsDictionary->get("JBIG2Globals"));
            if (globalDataObject.isStream())
            {
                globalData = document->getDecodedStream(globalDataObject.getStream());
            }
        }

        PDFJBIG2Decoder decoder(qMove(data), qMove(globalData), errorReporter);
        image.m_imageData = decoder.decode(maskingType);
        image.m_imageData.setDecode(!decode.empty() ? qMove(decode) : std::vector<PDFReal>({ 0.0, 1.0 }));
    }
    else if (colorSpace || isSoftMask)
    {
        // We treat data as binary maybe compressed stream (for example by Flate/LZW method), but data can also be not compressed.
        const unsigned int components = static_cast<unsigned int>(colorSpace->getColorComponentCount());
        const unsigned int bitsPerComponent = static_cast<unsigned int>(loader.readIntegerFromDictionary(dictionary, "BitsPerComponent", 8));
        const unsigned int width = static_cast<unsigned int>(loader.readIntegerFromDictionary(dictionary, "Width", 0));
        const unsigned int height = static_cast<unsigned int>(loader.readIntegerFromDictionary(dictionary, "Height", 0));

        if (bitsPerComponent < 1 || bitsPerComponent > 32)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid number of bits per component (%1).").arg(bitsPerComponent));
        }

        if (width == 0 || height == 0)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid size of image (%1x%2)").arg(width).arg(height));
        }

        // Calculate stride
        const unsigned int stride = (components * bitsPerComponent * width + 7) / 8;

        QByteArray imageDataBuffer = document->getDecodedStream(stream);
        image.m_imageData = PDFImageData(components, bitsPerComponent, width, height, stride, maskingType, qMove(imageDataBuffer), qMove(mask), qMove(decode), qMove(matte));
    }
    else if (imageMask)
    {
        // If ImageMask is set to true, then "BitsPerComponent" should have always value of 1.
        // If this entry is not specified, then the value should be implicitly 1.
        const unsigned int bitsPerComponent = static_cast<unsigned int>(loader.readIntegerFromDictionary(dictionary, "BitsPerComponent", 1));

        if (bitsPerComponent != 1)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid number bits of image mask (should be 1 bit instead of %1 bits).").arg(bitsPerComponent));
        }

        const unsigned int width = static_cast<unsigned int>(loader.readIntegerFromDictionary(dictionary, "Width", 0));
        const unsigned int height = static_cast<unsigned int>(loader.readIntegerFromDictionary(dictionary, "Height", 0));

        if (width == 0 || height == 0)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid size of image (%1x%2)").arg(width).arg(height));
        }

        // Calculate stride
        const unsigned int stride = (width + 7) / 8;

        QByteArray imageDataBuffer = document->getDecodedStream(stream);
        image.m_imageData = PDFImageData(1, bitsPerComponent, width, height, stride, maskingType, qMove(imageDataBuffer), qMove(mask), qMove(decode), qMove(matte));
    }

    return image;
}

PDFStream PDFImage::createStreamFromImage(const QImage& image,
                                          const ImageEncodeOptions& options,
                                          PDFRenderErrorReporter* reporter)
{
    PDFRenderErrorReporterDummy dummyReporter;
    if (!reporter)
    {
        reporter = &dummyReporter;
    }

    PreparedImageData prepared = prepareImageData(image, options, reporter);

    QByteArray encodedData;

    switch (options.compression)
    {
        case ImageCompression::Flate:
        {
            if (options.enablePngPredictor)
            {
                QByteArray predictorBuffer = createPngPredictorBuffer(prepared, true);
                encodedData = PDFFlateDecodeFilter::compress(predictorBuffer);
            }
            else
            {
                encodedData = PDFFlateDecodeFilter::compress(prepared.pixels);
            }
            break;
        }

        case ImageCompression::RunLength:
            encodedData = encodeRunLength(prepared.pixels);
            break;

        case ImageCompression::JPEG:
            encodedData = encodeJPEG(prepared, options.jpegQuality);
            break;

        case ImageCompression::JPEG2000:
            encodedData = encodeJPEG2000(prepared, options.jpeg2000Rate, reporter);
            break;
    }

    if (encodedData.isEmpty())
    {
        throw PDFException(PDFTranslationContext::tr("Encoded image stream is empty."));
    }

    PDFDictionary dictionary;
    dictionary.addEntry(PDFInplaceOrMemoryString("Type"), PDFObject::createName("XObject"));
    dictionary.addEntry(PDFInplaceOrMemoryString("Subtype"), PDFObject::createName("Image"));
    dictionary.addEntry(PDFInplaceOrMemoryString("Width"), PDFObject::createInteger(prepared.width));
    dictionary.addEntry(PDFInplaceOrMemoryString("Height"), PDFObject::createInteger(prepared.height));
    dictionary.addEntry(PDFInplaceOrMemoryString("BitsPerComponent"), PDFObject::createInteger(prepared.bitsPerComponent));

    const char* filterName = nullptr;
    switch (options.compression)
    {
        case ImageCompression::Flate:
            filterName = "FlateDecode";
            break;

        case ImageCompression::RunLength:
            filterName = "RunLengthDecode";
            break;

        case ImageCompression::JPEG:
            filterName = "DCTDecode";
            break;

        case ImageCompression::JPEG2000:
            filterName = "JPXDecode";
            break;
    }

    dictionary.addEntry(PDFInplaceOrMemoryString(PDF_STREAM_DICT_FILTER), PDFObject::createName(filterName));

    const char* colorSpaceName = (prepared.components == 3) ? COLOR_SPACE_NAME_DEVICE_RGB : COLOR_SPACE_NAME_DEVICE_GRAY;
    dictionary.addEntry(PDFInplaceOrMemoryString("ColorSpace"), PDFObject::createName(colorSpaceName));

    if (!prepared.decode.empty())
    {
        PDFArray decodeArray;
        for (PDFReal value : prepared.decode)
        {
            decodeArray.appendItem(PDFObject::createReal(value));
        }

        dictionary.addEntry(PDFInplaceOrMemoryString("Decode"),
                            PDFObject::createArray(std::make_shared<PDFArray>(std::move(decodeArray))));
    }

    if (options.compression == ImageCompression::Flate && options.enablePngPredictor)
    {
        PDFDictionary decodeParams;
        decodeParams.addEntry(PDFInplaceOrMemoryString("Predictor"), PDFObject::createInteger(15));
        decodeParams.addEntry(PDFInplaceOrMemoryString("Columns"), PDFObject::createInteger(prepared.width));
        decodeParams.addEntry(PDFInplaceOrMemoryString("Colors"), PDFObject::createInteger(prepared.components));
        decodeParams.addEntry(PDFInplaceOrMemoryString("BitsPerComponent"), PDFObject::createInteger(prepared.bitsPerComponent));

        dictionary.addEntry(PDFInplaceOrMemoryString(PDF_STREAM_DICT_DECODE_PARMS),
                            PDFObject::createDictionary(std::make_shared<PDFDictionary>(std::move(decodeParams))));
    }

    dictionary.addEntry(PDFInplaceOrMemoryString(PDF_STREAM_DICT_LENGTH), PDFObject::createInteger(encodedData.size()));

    return PDFStream(std::move(dictionary), std::move(encodedData));
}

QImage PDFImage::getImage(const PDFCMS* cms,
                          PDFRenderErrorReporter* reporter,
                          const PDFOperationControl* operationControl) const
{
    const bool isImageMask = m_imageData.getMaskingType() == PDFImageData::MaskingType::ImageMask;
    if (m_colorSpace && !isImageMask)
    {
        return m_colorSpace->getImage(m_imageData, m_softMask, cms, m_renderingIntent, reporter, operationControl);
    }
    else if (isImageMask)
    {
        if (m_imageData.getBitsPerComponent() != 1)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid number bits of image mask (should be 1 bit instead of %1 bits).").arg(m_imageData.getBitsPerComponent()));
        }

        if (m_imageData.getWidth() == 0 || m_imageData.getHeight() == 0)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid size of image (%1x%2)").arg(m_imageData.getWidth()).arg(m_imageData.getHeight()));
        }

        QImage image(m_imageData.getWidth(), m_imageData.getHeight(), QImage::Format_Alpha8);

        const bool flip01 = !m_imageData.getDecode().empty() && qFuzzyCompare(m_imageData.getDecode().front(), 1.0);
        PDFBitReader reader(&m_imageData.getData(), m_imageData.getBitsPerComponent());

        for (unsigned int i = 0, rowCount = m_imageData.getHeight(); i < rowCount; ++i)
        {
            reader.seek(i * m_imageData.getStride());
            unsigned char* outputLine = image.scanLine(i);

            for (unsigned int j = 0; j < m_imageData.getWidth(); ++j)
            {
                const bool transparent = flip01 != static_cast<bool>(reader.read());
                *outputLine++ = transparent ? 0x00 : 0xFF;
            }
        }

        return image;
    }

    return QImage();
}

bool PDFImage::canBeConvertedToMonochromatic(const QImage& image)
{
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            QRgb pixel = image.pixel(x, y);
            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);
            int alpha = qAlpha(pixel);

            if (alpha != 255)
            {
                return false;
            }

            // Zkontrolujte, zda jsou kanály stejné (odstín šedi) a zda jsou pouze 0 (černá) nebo 255 (bílá)
            if ((red != green || green != blue) || (red != 0 && red != 255)) {
                return false;
            }
        }
    }
    return true;
}

OPJ_SIZE_T PDFJPEG2000ImageData::read(void* p_buffer, OPJ_SIZE_T p_nb_bytes, void* p_user_data)
{
    PDFJPEG2000ImageData* data = reinterpret_cast<PDFJPEG2000ImageData*>(p_user_data);

    // Remaining length
    OPJ_OFF_T length = static_cast<OPJ_OFF_T>(data->byteArray->size()) - data->position;

    if (length < 0)
    {
        length = 0;
    }

    if (length > static_cast<OPJ_OFF_T>(p_nb_bytes))
    {
        length = static_cast<OPJ_OFF_T>(p_nb_bytes);
    }

    if (length > 0)
    {
        std::memcpy(p_buffer, data->byteArray->constData() + data->position, length);
        data->position += length;
    }

    if (length == 0)
    {
        return (OPJ_SIZE_T) - 1;
    }

    return length;
}

OPJ_BOOL PDFJPEG2000ImageData::seek(OPJ_OFF_T p_nb_bytes, void* p_user_data)
{
    PDFJPEG2000ImageData* data = reinterpret_cast<PDFJPEG2000ImageData*>(p_user_data);

    if (p_nb_bytes >= data->byteArray->size())
    {
        return OPJ_FALSE;
    }

    data->position = p_nb_bytes;
    return OPJ_TRUE;
}

OPJ_OFF_T PDFJPEG2000ImageData::skip(OPJ_OFF_T p_nb_bytes, void* p_user_data)
{
    PDFJPEG2000ImageData* data = reinterpret_cast<PDFJPEG2000ImageData*>(p_user_data);

    // Remaining length
    OPJ_OFF_T length = static_cast<OPJ_OFF_T>(data->byteArray->size()) - data->position;

    if (length < 0)
    {
        length = 0;
    }

    if (length > static_cast<OPJ_OFF_T>(p_nb_bytes))
    {
        length = static_cast<OPJ_OFF_T>(p_nb_bytes);
    }

    data->position += length;
    return length;
}

PDFAlternateImage PDFAlternateImage::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFAlternateImage result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_image = loader.readReferenceFromDictionary(dictionary, "Image");
        result.m_oc = loader.readReferenceFromDictionary(dictionary, "OC");
        result.m_defaultForPrinting = loader.readBooleanFromDictionary(dictionary, "DefaultForPrinting", false);
    }

    return result;
}

}   // namespace pdf
