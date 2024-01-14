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
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFCMS_H
#define PDFCMS_H

#include "pdfglobal.h"
#include "pdfcolorspaces.h"
#include "pdfexception.h"
#include "pdfutils.h"
#include "pdfcolorconvertor.h"

#include <QRecursiveMutex>
#include <QSharedPointer>

#include <compare>

namespace pdf
{

/// This simple structure stores settings for color management system, and what
/// color management system should be used. At default, two color management
/// system are available - generic (which uses default imprecise color management),
/// and CMS using engine LittleCMS2, which was written by Marti Maria, and is
/// linked as separate library.
struct PDFCMSSettings
{
    /// Type of color management system
    enum class System
    {
        Generic,
        LittleCMS2
    };

    /// Controls accuracy of the color transformations. High accuracy
    /// could mean high memory consumption, but better color accuracy,
    /// low accuracy means low memory consumption and low color accuracy.
    enum class Accuracy
    {
        Low,
        Medium,
        High
    };

    enum class ColorAdaptationXYZ
    {
        None,
        XYZScaling,
        CAT97,
        CAT02,
        Bradford
    };

    bool operator==(const PDFCMSSettings&) const = default;

    System system = System::Generic;
    Accuracy accuracy = Accuracy::Medium;
    RenderingIntent intent = RenderingIntent::Auto;
    RenderingIntent proofingIntent = RenderingIntent::RelativeColorimetric;
    ColorAdaptationXYZ colorAdaptationXYZ = ColorAdaptationXYZ::Bradford;
    bool isBlackPointCompensationActive = true;
    bool isWhitePaperColorTransformed = false;
    bool isGamutChecking = false;
    bool isSoftProofing = false;
    bool isConsiderOutputIntent = true;
    QColor outOfGamutColor = Qt::red; ///< Color, which marks out-of-gamut when soft-proofing is proceeded
    QString outputCS;               ///< Output (rendering) color space
    QString deviceGray;             ///< Identifiers for color space (device gray)
    QString deviceRGB;              ///< Identifiers for color space (device RGB)
    QString deviceCMYK;             ///< Identifiers for color space (device CMYK)
    QString softProofingProfile;    ///< Identifiers for soft proofing profile
    QString profileDirectory;       ///< Directory containing color profiles

    // Postprocessing
    QColor foregroundColor = Qt::green;
    QColor backgroundColor = Qt::black;
    int bitonalThreshold = 128;
    double sigmoidSlopeFactor = 10.0;
};

/// Color management system base class. It contains functions to transform
/// colors from various color system to device color system. If color management
/// system can't handle color transform, it should return invalid color.
class PDFCMS
{
public:
    explicit inline PDFCMS() = default;
    virtual ~PDFCMS() = default;

    /// This function should decide, if color management system is compatible with these
    /// settings (so, it transforms colors according to this setting). If this
    /// function returns false, then this color management system should be replaced
    /// by newly created one, according these settings.
    virtual bool isCompatible(const PDFCMSSettings& settings) const = 0;

    /// Returns color of the white paper
    virtual QColor getPaperColor() const = 0;

    /// Converts color in Device Gray color space to the target device
    /// color space. If error occurs, then invalid color is returned.
    /// Caller then should handle this - try to convert color as accurate
    /// as possible.
    /// \param color Single color channel value
    /// \param intent Rendering intent
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual QColor getColorFromDeviceGray(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const = 0;

    /// Converts color in Device RGB color space to the target device
    /// color space. If error occurs, then invalid color is returned.
    /// Caller then should handle this - try to convert color as accurate
    /// as possible.
    /// \param color Three color channel value (R,G,B channel)
    /// \param intent Rendering intent
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual QColor getColorFromDeviceRGB(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const = 0;

    /// Converts color in Device CMYK color space to the target device
    /// color space. If error occurs, then invalid color is returned.
    /// Caller then should handle this - try to convert color as accurate
    /// as possible.
    /// \param color Four color channel value (C,M,Y,K channel)
    /// \param intent Rendering intent
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual QColor getColorFromDeviceCMYK(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const = 0;

    /// Converts color in XYZ color space to the target device
    /// color space. If error occurs, then invalid color is returned.
    /// Caller then should handle this - try to convert color as accurate
    /// as possible.
    /// \param whitePoint White point of source XYZ color space
    /// \param Three color channel value (X,Y,Z channel)
    /// \param intent Rendering intent
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual QColor getColorFromXYZ(const PDFColor3& whitePoint,
                                   const PDFColor3& color,
                                   RenderingIntent intent,
                                   PDFRenderErrorReporter* reporter) const = 0;

    /// Computes color from ICC color profile
    /// \param color Input color
    /// \param iccID Unique ICC profile identifier
    /// \param iccData Color profile data
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual QColor getColorFromICC(const PDFColor& color,
                                   RenderingIntent renderingIntent,
                                   const QByteArray& iccID,
                                   const QByteArray& iccData,
                                   PDFRenderErrorReporter* reporter) const = 0;

    /// Fills colors in Device Gray color space to the RGB buffer. If error occurs, then false is returned.
    /// Caller then should handle this - try to convert color as accurate as possible.
    /// \param color Gray values
    /// \param intent Rendering intent
    /// \param outputBuffer Output buffer in format RGB_888 (8-bit RGB values)
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual bool fillRGBBufferFromDeviceGray(const std::vector<float>& colors,
                                             RenderingIntent intent,
                                             unsigned char* outputBuffer,
                                             PDFRenderErrorReporter* reporter) const = 0;

    /// Fills colors in Device RGB color space to RGB buffer. If error occurs, then false is returned.
    /// Caller then should handle this - try to convert color as accurate as possible.
    /// \param colors Buffer with three color channels, so it has pixels * tuple(R, G, B) size
    /// \param intent Rendering intent
    /// \param outputBuffer Output buffer in format RGB_888 (8-bit RGB values)
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual bool fillRGBBufferFromDeviceRGB(const std::vector<float>& colors,
                                            RenderingIntent intent,
                                            unsigned char* outputBuffer,
                                            PDFRenderErrorReporter* reporter) const = 0;

    /// Fills colors in Device CMYK color space to the RGB buffer. If error occurs, then false is returned.
    /// Caller then should handle this - try to convert color as accurate as possible.
    /// \param colors FBuffer with four color channels (C,M,Y,K channel), so it has pixels * tuple(C, M, Y, K) size
    /// \param intent Rendering intent
    /// \param outputBuffer Output buffer in format RGB_888 (8-bit RGB values)
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual bool fillRGBBufferFromDeviceCMYK(const std::vector<float>& colors,
                                             RenderingIntent intent,
                                             unsigned char* outputBuffer,
                                             PDFRenderErrorReporter* reporter) const = 0;

    /// Fills colors in XYZ color space to the RGB buffer. If error occurs, then false is returned.
    /// Caller then should handle this - try to convert color as accurate as possible.
    /// \param whitePoint White point of source XYZ color space
    /// \param Three color channel value (X,Y,Z channel)
    /// \param intent Rendering intent
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual bool fillRGBBufferFromXYZ(const PDFColor3& whitePoint,
                                      const std::vector<float>& colors,
                                      RenderingIntent intent,
                                      unsigned char* outputBuffer,
                                      PDFRenderErrorReporter* reporter) const = 0;

    /// Fills RGB buffer from ICC color profile colors
    /// \param colors Input colors
    /// \param iccID Unique ICC profile identifier
    /// \param iccData Color profile data
    /// \param reporter Render error reporter (used, when color transform fails)
    virtual bool fillRGBBufferFromICC(const std::vector<float>& colors,
                                      RenderingIntent renderingIntent,
                                      unsigned char* outputBuffer,
                                      const QByteArray& iccID,
                                      const QByteArray& iccData,
                                      PDFRenderErrorReporter* reporter) const = 0;

    /// Returns color convertor of post-processing colors
    /// produced by color management system. Color convertor
    /// does not have set color conversion mode, it must be
    /// set manually.
    /// \return Color convertor according to current settings.
    virtual PDFColorConvertor getColorConvertor() const = 0;

    enum ColorSpaceType
    {
        Invalid,
        DeviceGray,
        DeviceRGB,
        DeviceCMYK,
        XYZ,
        ICC
    };

    struct ColorSpaceTransformParams
    {
        ColorSpaceType sourceType = ColorSpaceType::Invalid;
        ColorSpaceType targetType = ColorSpaceType::Invalid;

        QByteArray sourceIccId;
        QByteArray targetIccId;

        QByteArray sourceIccData;
        QByteArray targetIccData;

        PDFColorBuffer input;
        PDFColorBuffer output;

        RenderingIntent intent = RenderingIntent::Unknown;

        PDFInteger multithreadingThreshold = 4096;
    };

    /// Transforms color between two color spaces. Doesn't do soft-proofing,
    /// it just transforms two float buffers from input color space to output color space.
    virtual bool transformColorSpace(const ColorSpaceTransformParams& params) const = 0;

    /// Get D50 white point for XYZ color space
    static PDFColor3 getDefaultXYZWhitepoint();
};

using PDFCMSPointer = QSharedPointer<PDFCMS>;

class PDF4QTLIBCORESHARED_EXPORT PDFCMSGeneric : public PDFCMS
{
public:
    explicit inline PDFCMSGeneric() = default;
    explicit inline PDFCMSGeneric(const PDFColorConvertor& colorConvertor);

    virtual bool isCompatible(const PDFCMSSettings& settings) const override;
    virtual QColor getPaperColor() const override;
    virtual QColor getColorFromDeviceGray(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromDeviceRGB(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromDeviceCMYK(const PDFColor& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromXYZ(const PDFColor3& whitePoint, const PDFColor3& color, RenderingIntent intent, PDFRenderErrorReporter* reporter) const override;
    virtual QColor getColorFromICC(const PDFColor& color, RenderingIntent renderingIntent,  const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromDeviceGray(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromDeviceRGB(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromDeviceCMYK(const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromXYZ(const PDFColor3& whitePoint, const std::vector<float>& colors, RenderingIntent intent, unsigned char* outputBuffer, PDFRenderErrorReporter* reporter) const override;
    virtual bool fillRGBBufferFromICC(const std::vector<float>& colors, RenderingIntent renderingIntent, unsigned char* outputBuffer, const QByteArray& iccID, const QByteArray& iccData, PDFRenderErrorReporter* reporter) const override;
    virtual bool transformColorSpace(const ColorSpaceTransformParams& params) const override;
    virtual PDFColorConvertor getColorConvertor() const override;

private:
    PDFColorConvertor m_colorConvertor;
};

struct PDFColorProfileIdentifier
{
    enum class Type
    {
        Gray,
        sRGB,
        RGB,
        FileGray,
        FileRGB,
        FileCMYK,
        MemoryGray,
        MemoryRGB,
        MemoryCMYK,
        Invalid
    };

    bool operator==(const PDFColorProfileIdentifier&) const = default;
    bool operator!=(const PDFColorProfileIdentifier&) const = default;

    Type type = Type::sRGB;
    QString name;
    QString id;
    PDFReal temperature = 6500.0;
    QPointF primaryR;
    QPointF primaryG;
    QPointF primaryB;
    PDFReal gamma = 1.0;
    bool isOutputIntentProfile = false;

    /// Data for MemoryGray, MemoryRGB and MemoryCMYK
    QByteArray profileMemoryData;

    /// Creates gray color profile identifier
    /// \param name Name of color profile
    /// \param id Identifier of color profile
    /// \param temperature White point temperature
    /// \param gamma Gamma correction
    static PDFColorProfileIdentifier createGray(QString name, QString id, PDFReal temperature, PDFReal gamma);

    /// Creates sRGB color profile identifier
    static PDFColorProfileIdentifier createSRGB();

    /// Creates RGB color space identifier
    /// \param name Name of color profile
    /// \param id Identifier of color profile
    /// \param temperature White point temperature
    /// \param primaryR Primary red
    /// \param primaryG Primary green
    /// \param primaryB Primary blue
    /// \param gamma Gamma correction
    static PDFColorProfileIdentifier createRGB(QString name, QString id, PDFReal temperature, QPointF primaryR, QPointF primaryG, QPointF primaryB, PDFReal gamma);

    /// Create file color profile identifier
    static PDFColorProfileIdentifier createFile(Type type, QString name, QString id);

    /// Create memory output intent color profile identifier
    static PDFColorProfileIdentifier createOutputIntent(Type type, QString name, QString id, QByteArray profileData);
};

using PDFColorProfileIdentifiers = std::vector<PDFColorProfileIdentifier>;

/// Manager, that manages current color management system and also list
/// of usable input and output color profiles. It has color profiles
/// for outout device, and color profiles for input (gray/RGB/CMYK).
/// It also handles settings, and it's changes. Constant functions
/// is save to call from multiple threads, this also holds for some
/// non-constant functions - manager is protected by mutexes.
class PDF4QTLIBCORESHARED_EXPORT PDFCMSManager : public QObject
{
    Q_OBJECT

private:
    using BaseClass = QObject;

public:
    explicit PDFCMSManager(QObject* parent);

    /// Finalizes cms manager. Call this function
    /// only at program exit. Frees all allocated
    /// resources. Function is not thread-safe.
    static void finalize();

    /// Returns current CMS. This function possibly creates CMS,
    /// of no CMS is found.
    PDFCMSPointer getCurrentCMS() const;

    const PDFCMSSettings& getSettings() const { return m_settings; }
    void setSettings(const PDFCMSSettings& settings);

    /// Returns color convertor of post-processing colors
    /// produced by color management system. Color convertor
    /// does not have set color conversion mode, it must be
    /// set manually.
    /// \return Color convertor according to current settings.
    PDFColorConvertor getColorConvertor() const;

    const PDFColorProfileIdentifiers& getOutputProfiles() const;
    const PDFColorProfileIdentifiers& getGrayProfiles() const;
    const PDFColorProfileIdentifiers& getRGBProfiles() const;
    const PDFColorProfileIdentifiers& getCMYKProfiles() const;

    /// Returns default color management settings
    PDFCMSSettings getDefaultSettings() const;

    /// Set current document. Document is examined for output
    /// rendering intents and they can be used when rendering.
    /// \param document Document
    void setDocument(const PDFDocument* document);

    /// Get translated name for color management system
    /// \param system System
    static QString getSystemName(PDFCMSSettings::System system);

signals:
    void colorManagementSystemChanged();

private:
    /// Creates new CMS based on current settings
    PDFCMSPointer getCurrentCMSImpl() const;

    /// Clear cached items
    void clearCache();

    /// This function returns external profiles. It is not protected by mutex,
    /// so it is not thread-safe. For this reason, it is not in public
    /// interface.
    const PDFColorProfileIdentifiers& getExternalProfiles() const;

    PDFColorProfileIdentifiers getOutputProfilesImpl() const;
    PDFColorProfileIdentifiers getGrayProfilesImpl() const;
    PDFColorProfileIdentifiers getRGBProfilesImpl() const;
    PDFColorProfileIdentifiers getCMYKProfilesImpl() const;
    PDFColorProfileIdentifiers getExternalProfilesImpl() const;

    /// Returns filtered list of external profiles (list are filtered by type,
    /// so, for example, only CMYK profiles are returned)
    /// \param type Type of profile
    PDFColorProfileIdentifiers getFilteredExternalProfiles(PDFColorProfileIdentifier::Type type) const;

    /// Returns filtered list of output intent profiles (list are filtered by type,
    /// so, for example, only CMYK profiles are returned)
    /// \param type Type of profile
    PDFColorProfileIdentifiers getFilteredOutputIntentProfiles(PDFColorProfileIdentifier::Type type) const;

    /// Gets list of color profiles from external directory
    /// \param profileDirectory Directory with profiles
    PDFColorProfileIdentifiers getExternalColorProfiles(QString profileDirectory) const;

    PDFCMSSettings m_settings;
    const PDFDocument* m_document;
    PDFColorProfileIdentifiers m_outputIntentProfiles;

    mutable QRecursiveMutex m_mutex;
    mutable PDFCachedItem<PDFCMSPointer> m_CMS;
    mutable PDFCachedItem<PDFColorProfileIdentifiers> m_outputProfiles;
    mutable PDFCachedItem<PDFColorProfileIdentifiers> m_grayProfiles;
    mutable PDFCachedItem<PDFColorProfileIdentifiers> m_RGBProfiles;
    mutable PDFCachedItem<PDFColorProfileIdentifiers> m_CMYKProfiles;
    mutable PDFCachedItem<PDFColorProfileIdentifiers> m_externalProfiles;
};

/// Class providing chromatic adaptation of whitepoints
/// using various method.
class PDFChromaticAdaptationXYZ
{
public:
    PDFChromaticAdaptationXYZ() = delete;

    static PDFColorComponentMatrix_3x3 createWhitepointChromaticAdaptation(const PDFColor3& targetWhitePoint,
                                                                           const PDFColor3& sourceWhitePoint,
                                                                           PDFCMSSettings::ColorAdaptationXYZ method);
};

}   // namespace pdf

#endif // PDFCMS_H
