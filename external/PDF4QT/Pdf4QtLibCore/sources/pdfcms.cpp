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

#include "pdfcms.h"
#include "pdfdocument.h"
#include "pdfexecutionpolicy.h"

#include <QDir>
#include <QFile>
#include <QBuffer>
#include <QCoreApplication>
#include <QReadWriteLock>

#include "pdfdbgheap.h"

#ifdef PDF4QT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"
#endif

#ifdef PDF4QT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:5033)
#endif
#ifndef CMS_NO_REGISTER_KEYWORD
#define CMS_NO_REGISTER_KEYWORD
#endif
#include <lcms2.h>
#include <lcms2_plugin.h>
#ifdef PDF4QT_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifdef PDF4QT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <Icm.h>
#if defined(PDF4QT_USE_PRAGMA_LIB)
#pragma comment(lib, "Mscms")
#endif
#endif

#include <unordered_map>

namespace pdf
{

class PDFLittleCMS : public PDFCMS
{
public:
    explicit PDFLittleCMS(const PDFCMSManager* manager,
                          const PDFCMSSettings& settings,
                          const PDFColorConvertor& colorConvertor);
    virtual ~PDFLittleCMS() override;

    virtual bool isCompatible(const PDFCMSSettings& settings) const override;
    virtual QColor getPaperColor() const override;
    virtual QColor getColorFromDeviceGray(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromDeviceRGB(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromDeviceCMYK(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromXYZ(const PDFColor3& whitePoint, const PDFColor3& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromICC(const PDFColor& color, RenderingIntent renderingIntent, const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromDeviceGray(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromDeviceRGB(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromDeviceCMYK(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromXYZ(const PDFColor3& whitePoint, const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromICC(const std::vector<float>& colors, RenderingIntent renderingIntent, unsigned char* outputBuffer, const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const override;
    virtual bool transformColorSpace(const ColorSpaceTransformParams& params) const override;
    virtual PDFColorConvertor getColorConvertor() const override;

private:
    void init();

    static int installCmsPlugins();

    static cmsBool optimizePipeline(cmsPipeline** Lut,
                                    cmsUInt32Number  Intent,
                                    cmsUInt32Number* InputFormat,
                                    cmsUInt32Number* OutputFormat,
                                    cmsUInt32Number* dwFlags);

    enum Profile
    {
        Output,
        Gray,
        RGB,
        CMYK,
        XYZ,
        SoftProofing,
        ProfileCount
    };

    /// Returns true, if we are doing soft-proofing
    bool isSoftProofing() const;

    /// Creates a profile using provided id and a list of profile descriptors.
    /// If profile can't be created, then null handle is returned. If \p preferOutputProfile
    /// is set to true, and given profile is not output profile, then first output profile
    /// is being selected.
    /// \param id Id of color profile
    /// \param profileDescriptors Profile descriptor list
    /// \param preferOutputProfile
    cmsHPROFILE createProfile(const QString& id, const PDFColorProfileIdentifiers& profileDescriptors, bool preferOutputProfile) const;

    /// Gets transform from cache. If transform doesn't exist, then it is created.
    /// \param profile Color profile
    /// \param intent Rendering intent
    /// \param isRGB888Buffer If true, 8-bit RGB output buffer is used, otherwise FLOAT RGB output buffer is used
    cmsHTRANSFORM getTransform(Profile profile, RenderingIntent intent, bool isRGB888Buffer) const;

    /// Gets transform for ICC profile from cache. If transform doesn't exist, then it is created.
    /// \param iccData Data of icc profile
    /// \param iccID Icc profile id
    /// \param renderingIntent Rendering intent
    /// \param isRGB888Buffer If true, 8-bit RGB output buffer is used, otherwise FLOAT RGB output buffer is used
    cmsHTRANSFORM getTransformFromICCProfile(const QByteArray& iccData, const QByteArray& iccID, RenderingIntent renderingIntent, bool isRGB888Buffer) const;

    /// Returns transformation flags according to the current settings
    cmsUInt32Number getTransformationFlags() const;

    /// Calculates effective rendering intent. If rendering intent is auto,
    /// then \p intent is used, otherwise intent is overriden.
    RenderingIntent getEffectiveRenderingIntent(RenderingIntent intent) const;

    /// Gets transform from cache key.
    /// \param profile Color profile
    /// \param intent Rendering intent
    /// \param isRGB888Buffer If true, 8-bit RGB output buffer is used, otherwise FLOAT RGB output buffer is used
    static constexpr int getCacheKey(Profile profile, RenderingIntent intent, bool isRGB888Buffer) { return ((int(intent) * ProfileCount + profile) << 1) + (isRGB888Buffer ? 1 : 0); }

    /// Returns little CMS rendering intent
    /// \param intent Rendering intent
    static cmsUInt32Number getLittleCMSRenderingIntent(RenderingIntent intent);

    /// Returns little CMS data format for profile
    /// \param profile Color profile handle
    static cmsUInt32Number getProfileDataFormat(cmsHPROFILE profile);

    /// Returns color from output color. Clamps invalid rgb output values to range [0.0, 1.0].
    /// \param color01 Rgb color (range 0-1 is assumed).
    static QColor getColorFromOutputColor(std::array<float, 3> color01);

    /// Returns transform key for transformation between various color spaces
    static QByteArray getTransformColorSpaceKey(const ColorSpaceTransformParams& params);

    cmsHTRANSFORM getTransformBetweenColorSpaces(const ColorSpaceTransformParams& params) const;

    const PDFCMSManager* m_manager;
    PDFCMSSettings m_settings;
    QColor m_paperColor;
    std::array<cmsHPROFILE, ProfileCount> m_profiles;
    PDFColorConvertor m_colorConvertor;

    mutable QReadWriteLock m_transformationCacheLock;
    mutable std::unordered_map<int, cmsHTRANSFORM> m_transformationCache;

    mutable QReadWriteLock m_customIccProfileCacheLock;
    mutable std::map<std::pair<QByteArray, RenderingIntent>, cmsHTRANSFORM> m_customIccProfileCache;

    mutable QReadWriteLock m_transformColorSpaceCacheLock;
    mutable std::map<QByteArray, cmsHTRANSFORM> m_transformColorSpaceCache;
};

bool PDFLittleCMS::fillRGBBufferFromDeviceGray(const std::vector<float>& colors,
                                               RenderingIntent intent,
                                               unsigned char* outputBuffer,
                                               PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(Gray, getEffectiveRenderingIntent(intent), true);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from gray to output device using CMS failed."));
        return false;
    }

    if (cmsGetTransformInputFormat(transform) == TYPE_GRAY_FLT)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_8);
        cmsDoTransform(transform, colors.data(), outputBuffer, static_cast<cmsUInt32Number>(colors.size()));
        return true;
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from gray to output device using CMS failed - invalid data format."));
    }

    return false;
}

bool PDFLittleCMS::fillRGBBufferFromDeviceRGB(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(RGB, getEffectiveRenderingIntent(intent), true);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from RGB to output device using CMS failed."));
        return false;
    }

    const cmsUInt32Number inputFormat = cmsGetTransformInputFormat(transform);
    if (inputFormat == TYPE_RGB_FLT && (colors.size()) % T_CHANNELS(inputFormat) == 0)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_8);
        cmsDoTransform(transform, colors.data(), outputBuffer, static_cast<cmsUInt32Number>(colors.size()) / T_CHANNELS(inputFormat));
        return true;
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from RGB to output device using CMS failed - invalid data format."));
    }

    return false;
}

bool PDFLittleCMS::fillRGBBufferFromDeviceCMYK(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(CMYK, getEffectiveRenderingIntent(intent), true);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from CMYK to output device using CMS failed."));
        return false;
    }

    const cmsUInt32Number inputFormat = cmsGetTransformInputFormat(transform);
    if (inputFormat == TYPE_CMYK_FLT && (colors.size()) % T_CHANNELS(inputFormat) == 0)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_8);
        std::vector<float> fixedColors = colors;
        for (size_t i = 0, count = fixedColors.size(); i < count; ++i)
        {
            fixedColors[i] = fixedColors[i] * 100.0f;
        }
        cmsDoTransform(transform, fixedColors.data(), outputBuffer, static_cast<cmsUInt32Number>(colors.size()) / T_CHANNELS(inputFormat));
        return true;
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from CMYK to output device using CMS failed - invalid data format."));
    }

    return false;
}

bool PDFLittleCMS::fillRGBBufferFromXYZ(const PDFColor3& whitePoint, const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(XYZ, getEffectiveRenderingIntent(intent), true);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from XYZ to output device using CMS failed."));
        return false;
    }

    const cmsUInt32Number inputFormat = cmsGetTransformInputFormat(transform);
    if (inputFormat == TYPE_XYZ_FLT && (colors.size()) % T_CHANNELS(inputFormat) == 0)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_8);

        PDFColorComponentMatrix_3x3 adaptationMatrix = PDFChromaticAdaptationXYZ::createWhitepointChromaticAdaptation(getDefaultXYZWhitepoint(), whitePoint, m_settings.colorAdaptationXYZ);
        std::vector<float> fixedColors = colors;

        const size_t count = fixedColors.size() / 3;
        for (size_t i = 0; i < count; ++i)
        {
            const size_t indexX = i * 3;
            const size_t indexY = i * 3 + 1;
            const size_t indexZ = i * 3 + 2;
            const PDFColor3 sourceXYZ = { fixedColors[indexX], fixedColors[indexY], fixedColors[indexZ] };
            const PDFColor3 adaptedXYZ = adaptationMatrix * sourceXYZ;
            fixedColors[indexX] = adaptedXYZ[0];
            fixedColors[indexY] = adaptedXYZ[1];
            fixedColors[indexZ] = adaptedXYZ[2];
        }

        cmsDoTransform(transform, fixedColors.data(), outputBuffer, static_cast<cmsUInt32Number>(colors.size()) / T_CHANNELS(inputFormat));
        return true;
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from XYZ to output device using CMS failed - invalid data format."));
    }

    return false;
}

bool PDFLittleCMS::fillRGBBufferFromICC(const std::vector<float>& colors, RenderingIntent renderingIntent, unsigned char* outputBuffer, const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransformFromICCProfile(iccData, iccID, renderingIntent, true);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from icc profile space to output device using CMS failed."));
        return false;
    }

    const cmsUInt32Number format = cmsGetTransformInputFormat(transform);
    const cmsUInt32Number channels = T_CHANNELS(format);
    const cmsUInt32Number colorSpace = T_COLORSPACE(format);
    const bool isCMYK = colorSpace == PT_CMYK;
    const float* inputColors = colors.data();
    std::vector<float> cmykColors;

    if (isCMYK)
    {
        cmykColors = colors;
        for (size_t i = 0; i < cmykColors.size(); ++i)
        {
            cmykColors[i] = cmykColors[i] * 100.0f;
        }
        inputColors = cmykColors.data();
    }

    if (colors.size() % channels == 0)
    {
        const cmsUInt32Number pixels = static_cast<cmsUInt32Number>(colors.size()) / channels;
        cmsDoTransform(transform, inputColors, outputBuffer, pixels);
        return true;
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from icc profile space to output device using CMS failed - invalid data format."));
    }

    return false;
}

bool PDFLittleCMS::transformColorSpace(const PDFCMS::ColorSpaceTransformParams& params) const
{
    PDFCMS::ColorSpaceTransformParams transformedParams = params;
    transformedParams.intent = getEffectiveRenderingIntent(transformedParams.intent);

    cmsHTRANSFORM transform = getTransformBetweenColorSpaces(transformedParams);
    if (!transform)
    {
        return false;
    }

    const cmsUInt32Number inputProfileFormat = cmsGetTransformInputFormat(transform);
    const cmsUInt32Number inputChannels = T_CHANNELS(inputProfileFormat);
    const cmsUInt32Number inputColorSpace = T_COLORSPACE(inputProfileFormat);
    const bool isInputCMYK = inputColorSpace == PT_CMYK;
    const float* inputColors = params.input.begin();
    std::vector<float> cmykColors;

    const cmsUInt32Number outputProfileFormat = cmsGetTransformOutputFormat(transform);
    const cmsUInt32Number outputChannels = T_CHANNELS(outputProfileFormat);
    const cmsUInt32Number outputColorSpace = T_COLORSPACE(outputProfileFormat);
    const bool isOutputCMYK = outputColorSpace == PT_CMYK;

    if (isInputCMYK)
    {
        cmykColors = std::vector<float>(params.input.cbegin(), params.input.cend());
        for (size_t i = 0; i < cmykColors.size(); ++i)
        {
            cmykColors[i] = cmykColors[i] * 100.0f;
        }
        inputColors = cmykColors.data();
    }

    const cmsUInt32Number inputPixelCount = static_cast<cmsUInt32Number>(params.input.size()) / inputChannels;
    const cmsUInt32Number outputPixelCount = static_cast<cmsUInt32Number>(params.output.size()) / outputChannels;

    if (params.input.size() % inputChannels == 0 &&
        params.output.size() % outputChannels == 0 &&
        inputPixelCount == outputPixelCount)
    {
        PDFColorBuffer outputBuffer = params.output;

        if (inputPixelCount > params.multithreadingThreshold)
        {
            struct TransformInfo
            {
                inline TransformInfo(const float* source, float* target, cmsUInt32Number pixelCount) :
                    source(source),
                    target(target),
                    pixelCount(pixelCount)
                {

                }

                const float* source = nullptr;
                float* target = nullptr;
                cmsUInt32Number pixelCount = 0;
            };

            const cmsUInt32Number blockSize = 4096;
            std::vector<TransformInfo> infos;
            infos.reserve(inputPixelCount / blockSize + 1);

            const float* sourcePointer = inputColors;
            float* targetPointer = outputBuffer.begin();

            cmsUInt32Number remainingPixelCount = inputPixelCount;
            while (remainingPixelCount > 0)
            {
                const cmsUInt32Number currentPixelCount = qMin(blockSize, remainingPixelCount);
                infos.emplace_back(sourcePointer, targetPointer, currentPixelCount);
                sourcePointer += currentPixelCount * inputChannels;
                targetPointer += currentPixelCount * outputChannels;
                remainingPixelCount -= currentPixelCount;
            }

            auto processEntry = [transform](const TransformInfo& info)
            {
                cmsDoTransform(transform, info.source, info.target, info.pixelCount);
            };
            PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, infos.begin(), infos.end(), processEntry);
        }
        else
        {
            // Single-threaded transform
            cmsDoTransform(transform, inputColors, outputBuffer.begin(), inputPixelCount);
        }

        if (isOutputCMYK)
        {
            const PDFColorComponent colorQuotient = 1.0f / 100.0f;
            for (PDFColorComponent& color : outputBuffer)
            {
                color *= colorQuotient;
            }
        }

        return true;
    }
    else
    {
        return false;
    }

    return false;
}

PDFColorConvertor PDFLittleCMS::getColorConvertor() const
{
    return m_colorConvertor;
}

PDFLittleCMS::PDFLittleCMS(const PDFCMSManager* manager,
                           const PDFCMSSettings& settings,
                           const PDFColorConvertor& colorConvertor) :
    m_manager(manager),
    m_settings(settings),
    m_paperColor(Qt::white),
    m_profiles(),
    m_colorConvertor(colorConvertor)
{
    static const int installed = installCmsPlugins();
    Q_UNUSED(installed);

    init();
}

PDFLittleCMS::~PDFLittleCMS()
{
    for (const auto& transformItem : m_transformationCache)
    {
        cmsHTRANSFORM transform = transformItem.second;
        if (transform)
        {
            cmsDeleteTransform(transform);
        }
    }

    for (const auto& transformItem : m_customIccProfileCache)
    {
        cmsHTRANSFORM transform = transformItem.second;
        if (transform)
        {
            cmsDeleteTransform(transform);
        }
    }

    for (const auto& transformItem : m_transformColorSpaceCache)
    {
        cmsHTRANSFORM transform = transformItem.second;
        if (transform)
        {
            cmsDeleteTransform(transform);
        }
    }

    for (cmsHPROFILE profile : m_profiles)
    {
        if (profile)
        {
            cmsCloseProfile(profile);
        }
    }
}

bool PDFLittleCMS::isCompatible(const PDFCMSSettings& settings) const
{
    return m_settings == settings;
}

QColor PDFLittleCMS::getPaperColor() const
{
    return m_paperColor;
}

QColor PDFLittleCMS::getColorFromDeviceGray(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(Gray, getEffectiveRenderingIntent(intent), false);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from gray to output device using CMS failed."));
        return QColor();
    }

    if (cmsGetTransformInputFormat(transform) == TYPE_GRAY_FLT && color.size() == 1)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_FLT);

        const float grayColor = color[0];
        std::array<float, 3> rgbOutputColor = { };
        cmsDoTransform(transform, &grayColor, rgbOutputColor.data(), 1);
        return getColorFromOutputColor(rgbOutputColor);
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from gray to output device using CMS failed - invalid data format."));
    }

    return QColor();
}

QColor PDFLittleCMS::getColorFromDeviceRGB(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(RGB, getEffectiveRenderingIntent(intent), false);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from RGB to output device using CMS failed."));
        return QColor();
    }

    if (cmsGetTransformInputFormat(transform) == TYPE_RGB_FLT && color.size() == 3)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_FLT);

        std::array<float, 3> rgbInputColor = { color[0], color[1], color[2] };
        std::array<float, 3> rgbOutputColor = { };
        cmsDoTransform(transform, rgbInputColor.data(), rgbOutputColor.data(), 1);
        return getColorFromOutputColor(rgbOutputColor);
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from RGB to output device using CMS failed - invalid data format."));
    }

    return QColor();
}

QColor PDFLittleCMS::getColorFromDeviceCMYK(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(CMYK, getEffectiveRenderingIntent(intent), false);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from CMYK to output device using CMS failed."));
        return QColor();
    }

    if (cmsGetTransformInputFormat(transform) == TYPE_CMYK_FLT && color.size() == 4)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_FLT);

        std::array<float, 4> cmykInputColor = { color[0] * 100.0f, color[1] * 100.0f, color[2] * 100.0f, color[3] * 100.0f };
        std::array<float, 3> rgbOutputColor = { };
        cmsDoTransform(transform, cmykInputColor.data(), rgbOutputColor.data(), 1);
        return getColorFromOutputColor(rgbOutputColor);
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from CMYK to output device using CMS failed - invalid data format."));
    }

    return QColor();
}

QColor PDFLittleCMS::getColorFromXYZ(const PDFColor3& whitePoint, const PDFColor3& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransform(XYZ, getEffectiveRenderingIntent(intent), false);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from XYZ to output device using CMS failed."));
        return QColor();
    }

    if (cmsGetTransformInputFormat(transform) == TYPE_XYZ_FLT && color.size() == 3)
    {
        Q_ASSERT(cmsGetTransformOutputFormat(transform) == TYPE_RGB_FLT);

        const PDFColorComponentMatrix_3x3 adaptationMatrix = PDFChromaticAdaptationXYZ::createWhitepointChromaticAdaptation(getDefaultXYZWhitepoint(), whitePoint, m_settings.colorAdaptationXYZ);
        const PDFColor3 xyzInputColor = adaptationMatrix * color;
        std::array<float, 3> rgbOutputColor = { };
        cmsDoTransform(transform, xyzInputColor.data(), rgbOutputColor.data(), 1);
        return getColorFromOutputColor(rgbOutputColor);
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from XYZ to output device using CMS failed - invalid data format."));
    }

    return QColor();
}

cmsHTRANSFORM PDFLittleCMS::getTransformFromICCProfile(const QByteArray& iccData, const QByteArray& iccID, RenderingIntent renderingIntent, bool isRGB888Buffer) const
{
    RenderingIntent effectiveRenderingIntent = getEffectiveRenderingIntent(renderingIntent);
    const auto key = std::make_pair(iccID + (isRGB888Buffer ? "RGB_888" : "FLT"), effectiveRenderingIntent);
    QReadLocker lock(&m_customIccProfileCacheLock);
    auto it = m_customIccProfileCache.find(key);
    if (it == m_customIccProfileCache.cend())
    {
        lock.unlock();
        QWriteLocker writeLock(&m_customIccProfileCacheLock);

        // Now, we have locked cache for writing. We must find out,
        // if some other thread doesn't created the transformation already.
        it = m_customIccProfileCache.find(key);
        if (it == m_customIccProfileCache.cend())
        {
            cmsHTRANSFORM transform = cmsHTRANSFORM();
            cmsHPROFILE profile = cmsOpenProfileFromMem(iccData.data(), iccData.size());
            if (profile)
            {
                if (const cmsUInt32Number inputDataFormat = getProfileDataFormat(profile))
                {
                    cmsUInt32Number lcmsIntent = getLittleCMSRenderingIntent(effectiveRenderingIntent);

                    if (isSoftProofing())
                    {
                        cmsHPROFILE proofingProfile = m_profiles[SoftProofing];
                        RenderingIntent proofingIntent = m_settings.proofingIntent;
                        if (m_settings.proofingIntent == RenderingIntent::Auto)
                        {
                            proofingIntent = effectiveRenderingIntent;
                        }

                        transform = cmsCreateProofingTransform(profile, inputDataFormat, m_profiles[Output], isRGB888Buffer ? TYPE_RGB_8 : TYPE_RGB_FLT, proofingProfile,
                                                               lcmsIntent, getLittleCMSRenderingIntent(proofingIntent), getTransformationFlags());
                    }
                    else
                    {
                        transform = cmsCreateTransform(profile, inputDataFormat, m_profiles[Output], isRGB888Buffer ? TYPE_RGB_8 : TYPE_RGB_FLT, lcmsIntent, getTransformationFlags());
                    }
                }
                cmsCloseProfile(profile);
            }

            it = m_customIccProfileCache.insert(std::make_pair(key, transform)).first;
        }

        return it->second;
    }
    else
    {
        return it->second;
    }

    return cmsHTRANSFORM();
}

QColor PDFLittleCMS::getColorFromICC(const PDFColor& color, RenderingIntent renderingIntent, const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const
{
    cmsHTRANSFORM transform = getTransformFromICCProfile(iccData, iccID, renderingIntent, false);

    if (!transform)
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from icc profile space to output device using CMS failed."));
        return QColor();
    }

    std::array<float, 4> inputBuffer = { };
    const cmsUInt32Number format = cmsGetTransformInputFormat(transform);
    const cmsUInt32Number channels = T_CHANNELS(format);
    const cmsUInt32Number colorSpace = T_COLORSPACE(format);
    const bool isCMYK = colorSpace == PT_CMYK;
    if (channels == color.size() && channels <= inputBuffer.size())
    {
        for (size_t i = 0; i < color.size(); ++i)
        {
            inputBuffer[i] = isCMYK ? color[i] * 100.0f : color[i];
        }

        std::array<float, 3> rgbOutputColor = { };
        cmsDoTransform(transform, inputBuffer.data(), rgbOutputColor.data(), 1);
        return getColorFromOutputColor(rgbOutputColor);
    }
    else
    {
        reporter->reportRenderErrorOnce(RenderErrorType::Error, PDFTranslationContext::tr("Conversion from icc profile space to output device using CMS failed - invalid data format."));
    }

    return QColor();
}

void PDFLittleCMS::init()
{
    // Jakub Melka: initialize all color profiles
    m_profiles[Output] = createProfile(m_settings.outputCS, m_manager->getOutputProfiles(), false);
    m_profiles[Gray] = createProfile(m_settings.deviceGray, m_manager->getGrayProfiles(), m_settings.isConsiderOutputIntent);
    m_profiles[RGB] = createProfile(m_settings.deviceRGB, m_manager->getRGBProfiles(), m_settings.isConsiderOutputIntent);
    m_profiles[CMYK] = createProfile(m_settings.deviceCMYK, m_manager->getCMYKProfiles(), m_settings.isConsiderOutputIntent);
    m_profiles[SoftProofing] = createProfile(m_settings.softProofingProfile, m_manager->getCMYKProfiles(), false);
    m_profiles[XYZ] = cmsCreateXYZProfile();

    cmsUInt16Number outOfGamutR = m_settings.outOfGamutColor.redF() * 0xFFFF;
    cmsUInt16Number outOfGamutG = m_settings.outOfGamutColor.greenF() * 0xFFFF;
    cmsUInt16Number outOfGamutB = m_settings.outOfGamutColor.blueF() * 0xFFFF;

    cmsUInt16Number alarmCodes[cmsMAXCHANNELS] = { outOfGamutR, outOfGamutG, outOfGamutB };
    cmsSetAlarmCodes(alarmCodes);

    if (m_settings.isWhitePaperColorTransformed)
    {
        m_paperColor = getColorFromDeviceRGB(PDFColor(1.0f, 1.0f, 1.0f), RenderingIntent::AbsoluteColorimetric, nullptr);

        // We must check color of the paper, it can be invalid, if error occurs...
        if (!m_paperColor.isValid())
        {
            m_paperColor = QColor(Qt::white);
        }
    }

    // 64 should be enough, because we can have 4 input color spaces (gray, RGB, CMYK and XYZ),
    // and 4 rendering intents. We have 4 * 4 = 16 input tables, so 64 will suffice enough
    // (because we then have 25% load factor).
    m_transformationCache.reserve(64);
}

int PDFLittleCMS::installCmsPlugins()
{
    static cmsPluginOptimization optimizationPlugin = { };
    optimizationPlugin.base.Magic = cmsPluginMagicNumber;
    optimizationPlugin.base.Type = cmsPluginOptimizationSig;
    optimizationPlugin.base.Next = nullptr;
    optimizationPlugin.base.ExpectedVersion = LCMS_VERSION;
    optimizationPlugin.OptimizePtr = &PDFLittleCMS::optimizePipeline;

    cmsPlugin(&optimizationPlugin);

    return 0;
}

cmsBool PDFLittleCMS::optimizePipeline(cmsPipeline** Lut, cmsUInt32Number Intent, cmsUInt32Number* InputFormat, cmsUInt32Number* OutputFormat, cmsUInt32Number* dwFlags)
{
    if (!(*dwFlags & cmsFLAGS_LOWRESPRECALC))
    {
        // Optimize only on low resolution precalculation
        return FALSE;
    }

    Q_UNUSED(Intent);

    // We will find, if we can optimize...
    bool shouldOptimize = false;
    for (auto stage = cmsPipelineGetPtrToFirstStage(*Lut); stage; stage = cmsStageNext(stage))
    {
        if (cmsStageType(stage) == cmsSigCurveSetElemType)
        {
            _cmsStageToneCurvesData* data = reinterpret_cast<_cmsStageToneCurvesData*>(cmsStageData(stage));
            for (cmsUInt32Number i = 0; i < data->nCurves; ++i)
            {
                const cmsToneCurve* curve = data->TheCurves[i];
                const cmsInt32Number type = cmsGetToneCurveParametricType(curve);

                if (type != 0 && !cmsIsToneCurveMultisegment(curve))
                {
                    shouldOptimize = true;
                }
            }
        }
    }

    if (shouldOptimize)
    {
        cmsContext contextId = cmsGetPipelineContextID(*Lut);
        cmsPipeline* pipeline = cmsPipelineAlloc(contextId, T_CHANNELS(*InputFormat), T_CHANNELS(*OutputFormat));

        if (!pipeline)
        {
            return FALSE;
        }

        for (auto stage = cmsPipelineGetPtrToFirstStage(*Lut); stage; stage = cmsStageNext(stage))
        {
            if (cmsStageType(stage) == cmsSigCurveSetElemType)
            {
                _cmsStageToneCurvesData* data = reinterpret_cast<_cmsStageToneCurvesData*>(cmsStageData(stage));
                std::vector<cmsToneCurve*> curves(data->nCurves, nullptr);

                for (cmsUInt32Number i = 0; i < data->nCurves; ++i)
                {
                    const cmsToneCurve* curve = data->TheCurves[i];
                    const cmsInt32Number type = cmsGetToneCurveParametricType(curve);

                    if (type != 0)
                    {
                        const cmsUInt32Number tableEntryCount = cmsGetToneCurveEstimatedTableEntries(curve);
                        const cmsUInt16Number* tableEntries = cmsGetToneCurveEstimatedTable(curve);

                        if (tableEntryCount > 0)
                        {
                            curves[i] = cmsBuildTabulatedToneCurve16(contextId, tableEntryCount, tableEntries);
                        }
                        else
                        {
                            curves[i] = cmsDupToneCurve(curve);
                        }
                    }
                    else
                    {
                        curves[i] = cmsDupToneCurve(curve);
                    }
                }

                cmsStageAllocToneCurves(contextId, cmsFloat32Number(curves.size()), curves.data());

                for (cmsToneCurve* curve : curves)
                {
                    cmsFreeToneCurve(curve);
                }
            }
            else
            {
                cmsPipelineInsertStage(pipeline, cmsAT_END, cmsStageDup(stage));
            }
        }

        cmsPipelineFree(*Lut);
        *Lut = pipeline;
    }

    return FALSE;
}

bool PDFLittleCMS::isSoftProofing() const
{
    return (m_settings.isSoftProofing || m_settings.isGamutChecking) && m_profiles[SoftProofing];
}

cmsHPROFILE PDFLittleCMS::createProfile(const QString& id, const PDFColorProfileIdentifiers& profileDescriptors, bool preferOutputProfile) const
{
    auto it = std::find_if(profileDescriptors.cbegin(), profileDescriptors.cend(), [&id](const PDFColorProfileIdentifier& identifier) { return identifier.id == id; });
    if (preferOutputProfile && it != profileDescriptors.end())
    {
        const PDFColorProfileIdentifier& identifier = *it;
        if (!identifier.isOutputIntentProfile)
        {
            // Find first output intent color profile
            auto itOutputIntentColorProfile = std::find_if(profileDescriptors.cbegin(), profileDescriptors.cend(), [](const PDFColorProfileIdentifier& identifier) { return identifier.isOutputIntentProfile; });
            if (itOutputIntentColorProfile != profileDescriptors.end())
            {
                it = itOutputIntentColorProfile;
            }
        }
    }

    if (it != profileDescriptors.cend())
    {
        const PDFColorProfileIdentifier& identifier = *it;
        switch (identifier.type)
        {
            case PDFColorProfileIdentifier::Type::Gray:
            {
                cmsCIExyY whitePoint{ };
                if (cmsWhitePointFromTemp(&whitePoint, identifier.temperature))
                {
                    cmsToneCurve* gammaCurve = cmsBuildGamma(cmsContext(), identifier.gamma);
                    cmsHPROFILE profile = cmsCreateGrayProfile(&whitePoint, gammaCurve);
                    cmsFreeToneCurve(gammaCurve);
                    return profile;
                }
                break;
            }

            case PDFColorProfileIdentifier::Type::sRGB:
                return cmsCreate_sRGBProfile();

            case PDFColorProfileIdentifier::Type::RGB:
            {
                cmsCIExyY whitePoint{ };
                if (cmsWhitePointFromTemp(&whitePoint, identifier.temperature))
                {
                    cmsCIExyYTRIPLE primaries;
                    primaries.Red = { identifier.primaryR.x(), identifier.primaryR.y(), 1.0 };
                    primaries.Green = { identifier.primaryG.x(), identifier.primaryG.y(), 1.0 };
                    primaries.Blue = { identifier.primaryB.x(), identifier.primaryB.y(), 1.0 };
                    cmsToneCurve* gammaCurve = cmsBuildGamma(cmsContext(), identifier.gamma);
                    cmsToneCurve* toneCurves[3] = { gammaCurve, cmsDupToneCurve(gammaCurve), cmsDupToneCurve(gammaCurve) };
                    cmsHPROFILE profile = cmsCreateRGBProfile(&whitePoint, &primaries, toneCurves);
                    cmsFreeToneCurveTriple(toneCurves);
                    return profile;
                }
                break;
            }
            case PDFColorProfileIdentifier::Type::FileGray:
            case PDFColorProfileIdentifier::Type::FileRGB:
            case PDFColorProfileIdentifier::Type::FileCMYK:
            {
                QFile file(identifier.id);
                if (file.open(QFile::ReadOnly))
                {
                    QByteArray fileContent = file.readAll();
                    file.close();

                    return cmsOpenProfileFromMem(fileContent.data(), fileContent.size());
                }

                break;
            }

            case PDFColorProfileIdentifier::Type::MemoryGray:
            case PDFColorProfileIdentifier::Type::MemoryRGB:
            case PDFColorProfileIdentifier::Type::MemoryCMYK:
                return cmsOpenProfileFromMem(identifier.profileMemoryData.data(), identifier.profileMemoryData.size());

            default:
                Q_ASSERT(false);
                break;
        }
    }

    return cmsHPROFILE();
}

cmsHTRANSFORM PDFLittleCMS::getTransform(Profile profile, RenderingIntent intent, bool isRGB888Buffer) const
{
    const int key = getCacheKey(profile, intent, isRGB888Buffer);

    QReadLocker lock(&m_transformationCacheLock);
    auto it = m_transformationCache.find(key);
    if (it == m_transformationCache.cend())
    {
        lock.unlock();
        QWriteLocker writeLock(&m_transformationCacheLock);

        // Now, we have locked cache for writing. We must find out,
        // if some other thread doesn't created the transformation already.
        it = m_transformationCache.find(key);
        if (it == m_transformationCache.cend())
        {
            cmsHTRANSFORM transform = cmsHTRANSFORM();
            cmsHPROFILE input = m_profiles[profile];
            cmsHPROFILE output = m_profiles[Output];

            if (input && output)
            {
                if (isSoftProofing())
                {
                    cmsHPROFILE proofingProfile = m_profiles[SoftProofing];
                    RenderingIntent proofingIntent = m_settings.proofingIntent;
                    if (m_settings.proofingIntent == RenderingIntent::Auto)
                    {
                        proofingIntent = intent;
                    }

                    transform = cmsCreateProofingTransform(input, getProfileDataFormat(input), output, isRGB888Buffer ? TYPE_RGB_8 : TYPE_RGB_FLT, proofingProfile,
                                                           getLittleCMSRenderingIntent(intent), getLittleCMSRenderingIntent(proofingIntent), getTransformationFlags());
                }
                else
                {
                    transform = cmsCreateTransform(input, getProfileDataFormat(input), output, isRGB888Buffer ? TYPE_RGB_8 : TYPE_RGB_FLT, getLittleCMSRenderingIntent(intent), getTransformationFlags());
                }
            }

            it = m_transformationCache.insert(std::make_pair(key, transform)).first;
        }

        // We must return it here to avoid race condition (after current block,
        // lock is not locked, because we unlocked lock for reading).
        return it->second;
    }

    return it->second;
}

cmsUInt32Number PDFLittleCMS::getTransformationFlags() const
{
    // Flag cmsFLAGS_NONEGATIVES is used here to avoid invalid transformation
    // between CMYK color space and RGB color space in the Ghent output suite examples.
    cmsUInt32Number flags = cmsFLAGS_NOCACHE | cmsFLAGS_NONEGATIVES;

    if (m_settings.isBlackPointCompensationActive)
    {
        flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
    }

    switch (m_settings.accuracy)
    {
        case PDFCMSSettings::Accuracy::Low:
            flags |= cmsFLAGS_LOWRESPRECALC;
            break;

        case PDFCMSSettings::Accuracy::Medium:
            break;

        case PDFCMSSettings::Accuracy::High:
            flags |= cmsFLAGS_HIGHRESPRECALC;
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    if (m_settings.isGamutChecking)
    {
        flags |= cmsFLAGS_GAMUTCHECK;
    }

    if (m_settings.isSoftProofing)
    {
        flags |= cmsFLAGS_SOFTPROOFING;
    }

    return flags;
}

RenderingIntent PDFLittleCMS::getEffectiveRenderingIntent(RenderingIntent intent) const
{
    if (m_settings.intent != RenderingIntent::Auto)
    {
        return m_settings.intent;
    }

    return intent;
}

cmsUInt32Number PDFLittleCMS::getLittleCMSRenderingIntent(RenderingIntent intent)
{
    switch (intent)
    {
        case RenderingIntent::Perceptual:
            return INTENT_PERCEPTUAL;

        case RenderingIntent::AbsoluteColorimetric:
            return INTENT_ABSOLUTE_COLORIMETRIC;

        case RenderingIntent::RelativeColorimetric:
            return INTENT_RELATIVE_COLORIMETRIC;

        case RenderingIntent::Saturation:
            return INTENT_SATURATION;

        default:
            Q_ASSERT(false);
            break;
    }

    return INTENT_PERCEPTUAL;
}

cmsUInt32Number PDFLittleCMS::getProfileDataFormat(cmsHPROFILE profile)
{
    cmsColorSpaceSignature signature = cmsGetColorSpace(profile);
    switch (signature)
    {
        case cmsSigGrayData:
            return TYPE_GRAY_FLT;

        case cmsSigRgbData:
            return TYPE_RGB_FLT;

        case cmsSigCmykData:
            return TYPE_CMYK_FLT;

        case cmsSigXYZData:
            return TYPE_XYZ_FLT;

        default:
            break;
    }

    return 0;
}

QColor PDFLittleCMS::getColorFromOutputColor(std::array<float, 3> color01)
{
    QColor color(QColor::Rgb);
    color.setRgbF(qBound(0.0f, color01[0], 1.0f), qBound(0.0f, color01[1], 1.0f), qBound(0.0f, color01[2], 1.0f));
    return color;
}

QByteArray PDFLittleCMS::getTransformColorSpaceKey(const PDFCMS::ColorSpaceTransformParams& params)
{
    QByteArray key;

    QBuffer buffer(&key);
    buffer.open(QBuffer::WriteOnly);

    QDataStream stream(&buffer);
    stream << params.sourceType;
    stream << params.sourceIccId;
    stream << params.targetType;
    stream << params.targetIccId;
    stream << params.intent;

    buffer.close();

    return key;
}

cmsHTRANSFORM PDFLittleCMS::getTransformBetweenColorSpaces(const PDFCMS::ColorSpaceTransformParams& params) const
{
    QByteArray key = getTransformColorSpaceKey(params);
    QReadLocker lock(&m_transformColorSpaceCacheLock);
    auto it = m_transformColorSpaceCache.find(key);
    if (it == m_transformColorSpaceCache.cend())
    {
        lock.unlock();
        QWriteLocker writeLock(&m_transformColorSpaceCacheLock);

        // Now, we have locked cache for writing. We must find out,
        // if some other thread doesn't created the transformation already.
        it = m_transformColorSpaceCache.find(key);
        if (it == m_transformColorSpaceCache.cend())
        {
            cmsHPROFILE inputProfile = cmsHPROFILE();
            cmsHPROFILE outputProfile = cmsHPROFILE();
            cmsHTRANSFORM transform = cmsHTRANSFORM();

            switch (params.sourceType)
            {
                case ColorSpaceType::DeviceGray:
                    inputProfile = m_profiles[Gray];
                    break;

                case ColorSpaceType::DeviceRGB:
                    inputProfile = m_profiles[RGB];
                    break;

                case ColorSpaceType::DeviceCMYK:
                    inputProfile = m_profiles[CMYK];
                    break;

                case ColorSpaceType::XYZ:
                    inputProfile = m_profiles[XYZ];
                    break;

                case ColorSpaceType::ICC:
                    inputProfile = cmsOpenProfileFromMem(params.sourceIccData.data(), params.sourceIccData.size());
                    break;

                default:
                    Q_ASSERT(false);
                    break;
            }

            switch (params.targetType)
            {
                case ColorSpaceType::DeviceGray:
                    outputProfile = m_profiles[Gray];
                    break;

                case ColorSpaceType::DeviceRGB:
                    outputProfile = m_profiles[RGB];
                    break;

                case ColorSpaceType::DeviceCMYK:
                    outputProfile = m_profiles[CMYK];
                    break;

                case ColorSpaceType::XYZ:
                    outputProfile = m_profiles[XYZ];
                    break;

                case ColorSpaceType::ICC:
                    outputProfile = cmsOpenProfileFromMem(params.targetIccData.data(), params.targetIccData.size());
                    break;

                default:
                    Q_ASSERT(false);
                    break;
            }

            if (inputProfile && outputProfile)
            {
                transform = cmsCreateTransform(inputProfile, getProfileDataFormat(inputProfile), outputProfile, getProfileDataFormat(outputProfile), getLittleCMSRenderingIntent(params.intent), getTransformationFlags());
            }

            if (params.sourceType == ColorSpaceType::ICC)
            {
                cmsCloseProfile(inputProfile);
            }

            if (params.targetType == ColorSpaceType::ICC)
            {
                cmsCloseProfile(outputProfile);
            }

            it = m_transformColorSpaceCache.insert(std::make_pair(key, transform)).first;
        }

        return it->second;
    }
    else
    {
        return it->second;
    }

    return cmsHTRANSFORM();
}

QString getInfoFromProfile(cmsHPROFILE profile, cmsInfoType infoType)
{
    QLocale locale;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString country = QLocale::territoryToString(locale.territory());
#else
    QString country = QLocale::countryToString(locale.country());
#endif
    QString language = QLocale::languageToString(locale.language());

    char countryCode[3] = { };
    char languageCode[3] = { };
    if (country.size() == 2)
    {
        countryCode[0] = country[0].toLatin1();
        countryCode[1] = country[1].toLatin1();
    }
    if (language.size() == 2)
    {
        languageCode[0] = language[0].toLatin1();
        languageCode[1] = language[1].toLatin1();
    }

    // Jakub Melka: try to get profile info from current language/country.
    // If it fails, then pick any language/any country.
    cmsUInt32Number bufferSize = cmsGetProfileInfo(profile, infoType, languageCode, countryCode, nullptr, 0);
    if (bufferSize)
    {
        std::vector<wchar_t> buffer(bufferSize, 0);
        cmsGetProfileInfo(profile, infoType, languageCode, countryCode, buffer.data(), static_cast<cmsUInt32Number>(buffer.size()));
        return QString::fromWCharArray(buffer.data());
    }

    bufferSize = cmsGetProfileInfo(profile, infoType, cmsNoLanguage, cmsNoCountry, nullptr, 0);
    if (bufferSize)
    {
        std::vector<wchar_t> buffer(bufferSize, 0);
        cmsGetProfileInfo(profile, infoType, cmsNoLanguage, cmsNoCountry, buffer.data(), static_cast<cmsUInt32Number>(buffer.size()));
        return QString::fromWCharArray(buffer.data());
    }

    return QString();
}

PDFCMSGeneric::PDFCMSGeneric(const PDFColorConvertor& colorConvertor) :
    m_colorConvertor(colorConvertor)
{

}

bool PDFCMSGeneric::isCompatible(const PDFCMSSettings& settings) const
{
    return settings.system == PDFCMSSettings::System::Generic;
}

QColor PDFCMSGeneric::getPaperColor() const
{
    return QColor(Qt::white);
}

QColor PDFCMSGeneric::getColorFromDeviceGray(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(color);
    Q_UNUSED(intent);
    Q_UNUSED(reporter);
    return QColor();
}

QColor PDFCMSGeneric::getColorFromDeviceRGB(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(color);
    Q_UNUSED(intent);
    Q_UNUSED(reporter);
    return QColor();
}

QColor PDFCMSGeneric::getColorFromDeviceCMYK(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(color);
    Q_UNUSED(intent);
    Q_UNUSED(reporter);
    return QColor();
}

QColor PDFCMSGeneric::getColorFromXYZ(const PDFColor3& whitePoint, const PDFColor3& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(color);
    Q_UNUSED(intent);
    Q_UNUSED(reporter);
    Q_UNUSED(whitePoint);
    return QColor();
}

QColor PDFCMSGeneric::getColorFromICC(const PDFColor& color,
                                      RenderingIntent renderingIntent,
                                      const QByteArray& iccID,
                                      const QByteArray& iccData,
                                      PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(color);
    Q_UNUSED(renderingIntent);
    Q_UNUSED(iccID);
    Q_UNUSED(iccData);
    Q_UNUSED(reporter);
    return QColor();
}

bool PDFCMSGeneric::fillRGBBufferFromDeviceGray(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(colors);
    Q_UNUSED(intent);
    Q_UNUSED(outputBuffer);
    Q_UNUSED(reporter);
    return false;
}

bool PDFCMSGeneric::fillRGBBufferFromDeviceRGB(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(colors);
    Q_UNUSED(intent);
    Q_UNUSED(outputBuffer);
    Q_UNUSED(reporter);
    return false;
}

bool PDFCMSGeneric::fillRGBBufferFromDeviceCMYK(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(colors);
    Q_UNUSED(intent);
    Q_UNUSED(outputBuffer);
    Q_UNUSED(reporter);
    return false;
}

bool PDFCMSGeneric::fillRGBBufferFromXYZ(const PDFColor3& whitePoint, const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(whitePoint);
    Q_UNUSED(colors);
    Q_UNUSED(intent);
    Q_UNUSED(outputBuffer);
    Q_UNUSED(reporter);
    return false;
}

bool PDFCMSGeneric::fillRGBBufferFromICC(const std::vector<float>& colors, RenderingIntent renderingIntent, unsigned char* outputBuffer, const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const
{
    Q_UNUSED(colors);
    Q_UNUSED(renderingIntent);
    Q_UNUSED(outputBuffer);
    Q_UNUSED(iccID);
    Q_UNUSED(iccData);
    Q_UNUSED(reporter);
    return false;
}

bool PDFCMSGeneric::transformColorSpace(const PDFCMS::ColorSpaceTransformParams& params) const
{
    Q_UNUSED(params);
    return false;
}

PDFColorConvertor PDFCMSGeneric::getColorConvertor() const
{
    return m_colorConvertor;
}

PDFCMSManager::PDFCMSManager(QObject* parent) :
    BaseClass(parent),
    m_document(nullptr),
    m_mutex()
{

}

void PDFCMSManager::finalize()
{
    cmsUnregisterPlugins();
}

PDFCMSPointer PDFCMSManager::getCurrentCMS() const
{
    QMutexLocker lock(&m_mutex);
    return m_CMS.get(this, &PDFCMSManager::getCurrentCMSImpl);
}

void PDFCMSManager::setSettings(const PDFCMSSettings& settings)
{
    if (m_settings != settings)
    {
        // We must ensure, that mutex is not locked, while we are
        // sending signal about CMS change.
        {
            QMutexLocker lock(&m_mutex);
            m_settings = settings;
            clearCache();
        }

        Q_EMIT colorManagementSystemChanged();
    }
}

PDFColorConvertor PDFCMSManager::getColorConvertor() const
{
    QMutexLocker lock(&m_mutex);

    PDFColorConvertor colorConvertor;
    colorConvertor.setBackgroundColor(m_settings.backgroundColor);
    colorConvertor.setForegroundColor(m_settings.foregroundColor);
    colorConvertor.setHighContrastBrightnessFactor(m_settings.sigmoidSlopeFactor);
    colorConvertor.setBitonalThreshold(m_settings.bitonalThreshold);
    return colorConvertor;
}

const PDFColorProfileIdentifiers& PDFCMSManager::getOutputProfiles() const
{
    QMutexLocker lock(&m_mutex);
    return m_outputProfiles.get(this, &PDFCMSManager::getOutputProfilesImpl);
}

const PDFColorProfileIdentifiers& PDFCMSManager::getGrayProfiles() const
{
    QMutexLocker lock(&m_mutex);
    return m_grayProfiles.get(this, &PDFCMSManager::getGrayProfilesImpl);
}

const PDFColorProfileIdentifiers& PDFCMSManager::getRGBProfiles() const
{
    QMutexLocker lock(&m_mutex);
    return m_RGBProfiles.get(this, &PDFCMSManager::getRGBProfilesImpl);
}

const PDFColorProfileIdentifiers& PDFCMSManager::getCMYKProfiles() const
{
    QMutexLocker lock(&m_mutex);
    return m_CMYKProfiles.get(this, &PDFCMSManager::getCMYKProfilesImpl);
}

const PDFColorProfileIdentifiers& PDFCMSManager::getExternalProfiles() const
{
    // Jakub Melka: do not protect this by mutex, this function is private
    // and must be called only from mutex-protected code.
    return m_externalProfiles.get(this, &PDFCMSManager::getExternalProfilesImpl);
}

PDFCMSSettings PDFCMSManager::getDefaultSettings() const
{
    PDFCMSSettings settings;

    auto getFirstProfileId = [](const PDFColorProfileIdentifiers& identifiers)
    {
        if (!identifiers.empty())
        {
            return identifiers.front().id;
        }
        return QString();
    };

    settings.system = PDFCMSSettings::System::LittleCMS2;
    settings.outputCS = getFirstProfileId(getOutputProfiles());
    settings.deviceGray = getFirstProfileId(getGrayProfiles());
    settings.deviceRGB = getFirstProfileId(getRGBProfiles());
    settings.deviceCMYK = getFirstProfileId(getCMYKProfiles());

    return settings;
}

void PDFCMSManager::setDocument(const PDFDocument* document)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    std::optional<QMutexLocker<QRecursiveMutex>> lock;
#else
    std::optional<QMutexLocker> lock;
#endif
    lock.emplace(&m_mutex);

    if (m_document == document)
    {
        return;
    }

    m_document = document;

    int i = 0;
    PDFColorProfileIdentifiers outputIntentProfiles;

    if (m_document)
    {
        for (const PDFOutputIntent& outputIntent : m_document->getCatalog()->getOutputIntents())
        {
            QByteArray content;

            try
            {
                // Try to read the profile from the output intent stream. If it fails, then do nothing.
                PDFObject outputProfileObject = m_document->getObject(outputIntent.getOutputProfile());
                if (outputProfileObject.isStream())
                {
                    content = m_document->getDecodedStream(outputProfileObject.getStream());
                }
            }
            catch (const PDFException&)
            {
                continue;
            }

            if (content.isEmpty())
            {
                // Decoding of output profile failed. Continue
                // with next output profile.
                continue;
            }

            cmsHPROFILE profile = cmsOpenProfileFromMem(content.data(), content.size());
            if (profile)
            {
                PDFColorProfileIdentifier::Type csiType = PDFColorProfileIdentifier::Type::Invalid;
                const cmsColorSpaceSignature colorSpace = cmsGetColorSpace(profile);
                switch (colorSpace)
                {
                    case cmsSigGrayData:
                        csiType = PDFColorProfileIdentifier::Type::MemoryGray;
                        break;

                    case cmsSigRgbData:
                        csiType = PDFColorProfileIdentifier::Type::MemoryRGB;
                        break;

                    case cmsSigCmykData:
                        csiType = PDFColorProfileIdentifier::Type::MemoryCMYK;
                        break;

                    default:
                        break;
                }

                QString description = getInfoFromProfile(profile, cmsInfoDescription);
                cmsCloseProfile(profile);

                // If we have a valid profile, then add it
                if (csiType != PDFColorProfileIdentifier::Type::Invalid)
                {
                    outputIntentProfiles.emplace_back(PDFColorProfileIdentifier::createOutputIntent(csiType, qMove(description), QString("@@OUTPUT_INTENT_PROFILE_%1").arg(++i), qMove(content)));
                }
            }
        }
    }

    bool outputIntentProfilesChanged = false;
    if (m_outputIntentProfiles != outputIntentProfiles)
    {
        m_outputIntentProfiles = qMove(outputIntentProfiles);
        clearCache();
        outputIntentProfilesChanged = true;
    }

    if (outputIntentProfilesChanged)
    {
        lock = std::nullopt;
        Q_EMIT colorManagementSystemChanged();
    }
}

QString PDFCMSManager::getSystemName(PDFCMSSettings::System system)
{
    switch (system)
    {
        case PDFCMSSettings::System::Generic:
            return tr("Generic");

        case PDFCMSSettings::System::LittleCMS2:
        {
            const int major = LCMS_VERSION / 1000;
            const int minor = (LCMS_VERSION % 1000) / 10;
            return tr("Little CMS %1.%2").arg(major).arg(minor);
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return QString();
}

PDFCMSPointer PDFCMSManager::getCurrentCMSImpl() const
{
    switch (m_settings.system)
    {
        case PDFCMSSettings::System::Generic:
            return PDFCMSPointer(new PDFCMSGeneric(getColorConvertor()));

        case PDFCMSSettings::System::LittleCMS2:
            return PDFCMSPointer(new PDFLittleCMS(this, m_settings, getColorConvertor()));

        default:
            Q_ASSERT(false);
            break;
    }

    return PDFCMSPointer(new PDFCMSGeneric());
}

void PDFCMSManager::clearCache()
{
    QMutexLocker lock(&m_mutex);
    m_CMS.dirty();
    m_outputProfiles.dirty();
    m_grayProfiles.dirty();
    m_RGBProfiles.dirty();
    m_CMYKProfiles.dirty();
    m_externalProfiles.dirty();
}

PDFColorProfileIdentifiers PDFCMSManager::getOutputProfilesImpl() const
{
    // Currently, we only support sRGB output color profile.
    return { PDFColorProfileIdentifier::createSRGB() };
}

PDFColorProfileIdentifiers PDFCMSManager::getGrayProfilesImpl() const
{
    // Jakub Melka: We create gray profiles for temperature 5000K, 6500K and 9300K.
    // We also use linear gamma and gamma value 2.2.
    PDFColorProfileIdentifiers result =
    {
        PDFColorProfileIdentifier::createGray(tr("Gray D65, γ = 2.2"), "@GENERIC_Gray_D65_g22", 6500.0, 2.2),
        PDFColorProfileIdentifier::createGray(tr("Gray D50, γ = 2.2"), "@GENERIC_Gray_D50_g22", 5000.0, 2.2),
        PDFColorProfileIdentifier::createGray(tr("Gray D93, γ = 2.2"), "@GENERIC_Gray_D93_g22", 9300.0, 2.2),
        PDFColorProfileIdentifier::createGray(tr("Gray D65, γ = 1.0 (linear)"), "@GENERIC_Gray_D65_g10", 6500.0, 1.0),
        PDFColorProfileIdentifier::createGray(tr("Gray D50, γ = 1.0 (linear)"), "@GENERIC_Gray_D50_g10", 5000.0, 1.0),
        PDFColorProfileIdentifier::createGray(tr("Gray D93, γ = 1.0 (linear)"), "@GENERIC_Gray_D93_g10", 9300.0, 1.0)
    };

    PDFColorProfileIdentifiers externalGrayProfiles = getFilteredExternalProfiles(PDFColorProfileIdentifier::Type::FileGray);
    result.insert(result.end(), std::make_move_iterator(externalGrayProfiles.begin()), std::make_move_iterator(externalGrayProfiles.end()));
    PDFColorProfileIdentifiers outputIntentRGBProfiles = getFilteredOutputIntentProfiles(PDFColorProfileIdentifier::Type::MemoryGray);
    result.insert(result.end(), std::make_move_iterator(outputIntentRGBProfiles.begin()), std::make_move_iterator(outputIntentRGBProfiles.end()));
    return result;
}

PDFColorProfileIdentifiers PDFCMSManager::getRGBProfilesImpl() const
{
    // Jakub Melka: We create RGB profiles for common standards and also for
    // default standard sRGB. See https://en.wikipedia.org/wiki/Color_spaces_with_RGB_primaries.
    PDFColorProfileIdentifiers result =
    {
        PDFColorProfileIdentifier::createSRGB(),
        PDFColorProfileIdentifier::createRGB(tr("HDTV (ITU-R BT.709)"), "@GENERIC_RGB_HDTV", 6500, QPointF(0.64, 0.33), QPointF(0.30, 0.60), QPointF(0.15, 0.06), 20.0 / 9.0),
        PDFColorProfileIdentifier::createRGB(tr("Adobe RGB 1998"), "@GENERIC_RGB_Adobe1998", 6500, QPointF(0.64, 0.33), QPointF(0.30, 0.60), QPointF(0.15, 0.06), 563.0 / 256.0),
        PDFColorProfileIdentifier::createRGB(tr("PAL / SECAM"), "@GENERIC_RGB_PalSecam", 6500, QPointF(0.64, 0.33), QPointF(0.29, 0.60), QPointF(0.15, 0.06), 14.0 / 5.0),
        PDFColorProfileIdentifier::createRGB(tr("NTSC"), "@GENERIC_RGB_NTSC", 6500, QPointF(0.64, 0.34), QPointF(0.31, 0.595), QPointF(0.155, 0.07), 20.0 / 9.0),
        PDFColorProfileIdentifier::createRGB(tr("Adobe Wide Gamut RGB"), "@GENERIC_RGB_AdobeWideGamut", 5000, QPointF(0.735, 0.265), QPointF(0.115, 0.826), QPointF(0.157, 0.018), 563.0 / 256.0),
        PDFColorProfileIdentifier::createRGB(tr("ProPhoto RGB"), "@GENERIC_RGB_ProPhoto", 5000, QPointF(0.7347, 0.2653), QPointF(0.1596, 0.8404), QPointF(0.0366, 0.0001), 9.0 / 5.0)
    };

    PDFColorProfileIdentifiers externalRGBProfiles = getFilteredExternalProfiles(PDFColorProfileIdentifier::Type::FileRGB);
    result.insert(result.end(), externalRGBProfiles.begin(), externalRGBProfiles.end());
    PDFColorProfileIdentifiers outputIntentRGBProfiles = getFilteredOutputIntentProfiles(PDFColorProfileIdentifier::Type::MemoryRGB);
    result.insert(result.end(), std::make_move_iterator(outputIntentRGBProfiles.begin()), std::make_move_iterator(outputIntentRGBProfiles.end()));
    return result;
}

PDFColorProfileIdentifiers PDFCMSManager::getCMYKProfilesImpl() const
{
    PDFColorProfileIdentifiers result;

    PDFColorProfileIdentifiers externalCMYKProfiles = getFilteredExternalProfiles(PDFColorProfileIdentifier::Type::FileCMYK);
    result.insert(result.end(), externalCMYKProfiles.begin(), externalCMYKProfiles.end());
    PDFColorProfileIdentifiers outputIntentCMYKProfiles = getFilteredOutputIntentProfiles(PDFColorProfileIdentifier::Type::MemoryCMYK);
    result.insert(result.end(), std::make_move_iterator(outputIntentCMYKProfiles.begin()), std::make_move_iterator(outputIntentCMYKProfiles.end()));

    return result;
}

PDFColorProfileIdentifiers PDFCMSManager::getExternalColorProfiles(QString profileDirectory) const
{
    PDFColorProfileIdentifiers result;

    QDir directory(profileDirectory);
    QDir applicationDirectory(QCoreApplication::applicationDirPath());
    if (!profileDirectory.isEmpty() && directory.exists())
    {
        QStringList iccProfiles = directory.entryList({ "*.icc" }, QDir::Files | QDir::Readable | QDir::NoDotAndDotDot, QDir::NoSort);
        for (const QString& fileName : iccProfiles)
        {
            QString filePath = QDir::cleanPath(applicationDirectory.relativeFilePath(directory.absoluteFilePath(fileName)));

            // Try to read the profile from the file. If it fails, then do nothing.
            QFile file(filePath);
            if (file.open(QFile::ReadOnly))
            {
                QByteArray content = file.readAll();
                file.close();

                cmsHPROFILE profile = cmsOpenProfileFromMem(content.data(), content.size());
                if (profile)
                {
                    PDFColorProfileIdentifier::Type csiType = PDFColorProfileIdentifier::Type::Invalid;
                    const cmsColorSpaceSignature colorSpace = cmsGetColorSpace(profile);
                    switch (colorSpace)
                    {
                        case cmsSigGrayData:
                            csiType = PDFColorProfileIdentifier::Type::FileGray;
                            break;

                        case cmsSigRgbData:
                            csiType = PDFColorProfileIdentifier::Type::FileRGB;
                            break;

                        case cmsSigCmykData:
                            csiType = PDFColorProfileIdentifier::Type::FileCMYK;
                            break;

                        default:
                            break;
                    }

                    QString description = getInfoFromProfile(profile, cmsInfoDescription);
                    cmsCloseProfile(profile);

                    // If we have a valid profile, then add it
                    if (csiType != PDFColorProfileIdentifier::Type::Invalid)
                    {
                        result.emplace_back(PDFColorProfileIdentifier::createFile(csiType, qMove(description), filePath));
                    }
                }
            }
        }
    }

    return result;
}

PDFColorProfileIdentifiers PDFCMSManager::getExternalProfilesImpl() const
{
    PDFColorProfileIdentifiers result;

    QStringList directories(m_settings.profileDirectory);

#if defined(Q_OS_WIN)
    std::array<WCHAR, _MAX_PATH> buffer = { };
    DWORD bufferSize = DWORD(buffer.size() * sizeof(WCHAR));
    if (GetColorDirectoryW(NULL, buffer.data(), &bufferSize))
    {
        const DWORD charactersWithNull = bufferSize / sizeof(WCHAR);
        const DWORD charactersWithoutNull = bufferSize > 0 ? charactersWithNull - 1 : 0;

        QString directory = QString::fromWCharArray(buffer.data(), int(charactersWithoutNull));
        directories << QDir::fromNativeSeparators(directory);
    }
#elif defined(Q_OS_UNIX)
    QDir directory(QStringLiteral("/usr/share/color/icc"));
    if (directory.exists())
    {
        QStringList colorDirectories = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& colorDirectory : colorDirectories)
        {
            QString colorDirectoryName = directory.absoluteFilePath(colorDirectory);
            directories << QDir::fromNativeSeparators(colorDirectoryName);
        }
    }
#else
    static_assert(false, "Implement this for another OS!");
#endif
    for (const QString& directory : directories)
    {
        PDFColorProfileIdentifiers externalProfiles = getExternalColorProfiles(directory);
        result.insert(result.end(), externalProfiles.begin(), externalProfiles.end());
    }

    return result;
}

PDFColorProfileIdentifiers PDFCMSManager::getFilteredExternalProfiles(PDFColorProfileIdentifier::Type type) const
{
    PDFColorProfileIdentifiers result;
    const PDFColorProfileIdentifiers& externalProfiles = getExternalProfiles();
    std::copy_if(externalProfiles.cbegin(), externalProfiles.cend(), std::back_inserter(result), [type](const PDFColorProfileIdentifier& identifier) { return identifier.type == type; });
    return result;
}

PDFColorProfileIdentifiers PDFCMSManager::getFilteredOutputIntentProfiles(PDFColorProfileIdentifier::Type type) const
{
    PDFColorProfileIdentifiers result;
    std::copy_if(m_outputIntentProfiles.cbegin(), m_outputIntentProfiles.cend(), std::back_inserter(result), [type](const PDFColorProfileIdentifier& identifier) { return identifier.type == type; });
    return result;
}

PDFColorProfileIdentifier PDFColorProfileIdentifier::createGray(QString name, QString id, PDFReal temperature, PDFReal gamma)
{
    PDFColorProfileIdentifier result;
    result.type = Type::Gray;
    result.name = qMove(name);
    result.id = qMove(id);
    result.temperature = temperature;
    result.gamma = gamma;
    return result;
}

PDFColorProfileIdentifier PDFColorProfileIdentifier::createSRGB()
{
    PDFColorProfileIdentifier result;
    result.type = Type::sRGB;
    result.name = PDFCMSManager::tr("sRGB");
    result.id = "@GENERIC_sRGB";
    return result;
}

PDFColorProfileIdentifier PDFColorProfileIdentifier::createRGB(QString name, QString id, PDFReal temperature, QPointF primaryR, QPointF primaryG, QPointF primaryB, PDFReal gamma)
{
    PDFColorProfileIdentifier result;
    result.type = Type::RGB;
    result.name = qMove(name);
    result.id = qMove(id);
    result.temperature = temperature;
    result.primaryR = primaryR;
    result.primaryG = primaryG;
    result.primaryB = primaryB;
    result.gamma = gamma;
    return result;
}

PDFColorProfileIdentifier PDFColorProfileIdentifier::createFile(Type type, QString name, QString id)
{
    PDFColorProfileIdentifier result;
    result.type = type;
    result.name = qMove(name);
    result.id = qMove(id);
    return result;
}

PDFColorProfileIdentifier PDFColorProfileIdentifier::createOutputIntent(PDFColorProfileIdentifier::Type type, QString name, QString id, QByteArray profileData)
{
    PDFColorProfileIdentifier result;
    result.type = type;
    result.name = qMove(name);
    result.id = qMove(id);
    result.profileMemoryData = qMove(profileData);
    result.isOutputIntentProfile = true;
    return result;
}

PDFColor3 PDFCMS::getDefaultXYZWhitepoint()
{
    const cmsCIEXYZ* whitePoint = cmsD50_XYZ();
    return PDFColor3{ PDFColorComponent(whitePoint->X), PDFColorComponent(whitePoint->Y), PDFColorComponent(whitePoint->Z) };
}

PDFColorComponentMatrix_3x3 PDFChromaticAdaptationXYZ::createWhitepointChromaticAdaptation(const PDFColor3& targetWhitePoint,
                                                                                           const PDFColor3& sourceWhitePoint,
                                                                                           PDFCMSSettings::ColorAdaptationXYZ method)
{
    PDFColorComponentMatrix_3x3 matrix;
    matrix.makeIdentity();

    switch (method)
    {
        case pdf::PDFCMSSettings::ColorAdaptationXYZ::None:
            // No scaling performed, just return identity matrix
            break;

        case pdf::PDFCMSSettings::ColorAdaptationXYZ::XYZScaling:
            matrix.makeDiagonal(std::array{ targetWhitePoint[0] / sourceWhitePoint[0],
                                  targetWhitePoint[1] / sourceWhitePoint[1],
                                  targetWhitePoint[2] / sourceWhitePoint[2]
                                });
            break;

        case pdf::PDFCMSSettings::ColorAdaptationXYZ::CAT97:
        {
            // CAT97 matrix, as defined in https://en.wikipedia.org/wiki/LMS_color_space
            constexpr PDFColorComponentMatrix_3x3 cat97Matrix(
                         0.8562f,  0.3372f, -0.1934f,
                        -0.8360f,  1.8327f,  0.0033f,
                         0.0357f, -0.0469f,  1.0112f);

            // Inverse of CAT97 matrix (using wxMaxima to compute it)
            constexpr PDFColorComponentMatrix_3x3 inverseCat97Matrix(
                         0.9873999149199271f,   -0.1768250198556842f,   0.1894251049357571f,
                         0.4504351090445315f,    0.464932897752711f,    0.08463199320275755f,
                        -0.01396832510725165f,   0.027806572501434f,    0.9861617526058175f);

            PDFColor3 adaptedTargetWhitePoint = cat97Matrix * targetWhitePoint;
            PDFColor3 adaptedSourceWhitePoint = cat97Matrix * sourceWhitePoint;
            PDFColor3 gain = {
                adaptedTargetWhitePoint[0] / adaptedSourceWhitePoint[0],
                adaptedTargetWhitePoint[1] / adaptedSourceWhitePoint[1],
                adaptedTargetWhitePoint[2] / adaptedSourceWhitePoint[2]
            };

            PDFColorComponentMatrix_3x3 gainMatrix;
            gainMatrix.makeDiagonal(gain);

            matrix = inverseCat97Matrix * gainMatrix * cat97Matrix;
            break;
        }

        case pdf::PDFCMSSettings::ColorAdaptationXYZ::CAT02:
        {
            // CAT02 matrix, as defined in https://en.wikipedia.org/wiki/LMS_color_space
            constexpr PDFColorComponentMatrix_3x3 cat02Matrix(
                         0.7328f,  0.4296f, -0.1624f,
                        -0.7036f,  1.6975f,  0.0061f,
                         0.0030f,  0.0136f,  0.9834f);

            // Inverse of CAT02 matrix (using wxMaxima to compute it)
            constexpr PDFColorComponentMatrix_3x3 inverseCat02Matrix(
                        1.096123820835514f,     -0.2788690002182872f,   0.182745179382773f,
                        0.4543690419753592f,     0.4735331543074117f,   0.0720978037172291f,
                       -0.009627608738429352f,  -0.005698031216113419f, 1.015325639954543f);

            PDFColor3 adaptedTargetWhitePoint = cat02Matrix * targetWhitePoint;
            PDFColor3 adaptedSourceWhitePoint = cat02Matrix * sourceWhitePoint;
            PDFColor3 gain = {
                adaptedTargetWhitePoint[0] / adaptedSourceWhitePoint[0],
                adaptedTargetWhitePoint[1] / adaptedSourceWhitePoint[1],
                adaptedTargetWhitePoint[2] / adaptedSourceWhitePoint[2]
            };

            PDFColorComponentMatrix_3x3 gainMatrix;
            gainMatrix.makeDiagonal(gain);

            matrix = inverseCat02Matrix * gainMatrix * cat02Matrix;
            break;
        }

        case pdf::PDFCMSSettings::ColorAdaptationXYZ::Bradford:
        {
            // Bradford matrix, as defined in https://en.wikipedia.org/wiki/LMS_color_space
            constexpr PDFColorComponentMatrix_3x3 bradfordMatrix(
                 0.8951f,  0.2264f, -0.1614f,
                -0.7502f,  1.7135f,  0.0367f,
                 0.0389f, -0.0685f,  1.0296f);

            // Inverse of bradford matrix (using wxMaxima to compute it)
            constexpr PDFColorComponentMatrix_3x3 inverseBradfordMatrix(
                    1.004360519274085f,     -0.1262294327613208f,   0.1619428982062721f,
                    0.4399123264001572f,     0.527481594455384f,    0.05015858096782513f,
                   -0.008678739162151443f,   0.03986287311053728f,  0.9684695843590444f);

            PDFColor3 adaptedTargetWhitePoint = bradfordMatrix * targetWhitePoint;
            PDFColor3 adaptedSourceWhitePoint = bradfordMatrix * sourceWhitePoint;
            PDFColor3 gain = {
                adaptedTargetWhitePoint[0] / adaptedSourceWhitePoint[0],
                adaptedTargetWhitePoint[1] / adaptedSourceWhitePoint[1],
                adaptedTargetWhitePoint[2] / adaptedSourceWhitePoint[2]
            };

            PDFColorComponentMatrix_3x3 gainMatrix;
            gainMatrix.makeDiagonal(gain);

            matrix = inverseBradfordMatrix * gainMatrix * bradfordMatrix;
            break;
        }

        default:
            Q_ASSERT(false);
            break;
    }

    return matrix;
}

}   // namespace pdf
