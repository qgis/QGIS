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

#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include "pdfpage.h"
#include "pdfexception.h"
#include "pdfoperationcontrol.h"
#include "pdfmeshqualitysettings.h"
#include "pdfutils.h"
#include "pdfcolorconvertor.h"

#include <QMutex>
#include <QSemaphore>
#include <QImageWriter>
#include <QSurfaceFormat>
#include <QImage>

class QPainter;
class QOpenGLContext;
class QOffscreenSurface;
class QOpenGLFramebufferObject;

namespace pdf
{
class PDFCMS;
class PDFProgress;
class PDFFontCache;
class PDFCMSManager;
class PDFPrecompiledPage;
class PDFAnnotationManager;
class PDFOptionalContentActivity;

class PDF4QTLIBCORESHARED_EXPORT PDFRendererInfo
{
public:
    PDFRendererInfo() = delete;

    struct Info
    {
        QString vendor;
        QString renderer;
        QString version;
        int majorOpenGLVersion = 0;
        int minorOpenGLVersion = 0;
    };

    static const Info& getHardwareAccelerationSupportedInfo();
    static bool isHardwareAccelerationSupported();

    static constexpr int REQUIRED_OPENGL_MAJOR_VERSION = 3;
    static constexpr int REQUIRED_OPENGL_MINOR_VERSION = 2;

private:
    static PDFCachedItem<Info> s_info;
};

/// Renders the PDF page on the painter, or onto an image.
class PDF4QTLIBCORESHARED_EXPORT PDFRenderer
{
public:

    enum Feature
    {
        None                        = 0x0000,
        Antialiasing                = 0x0001,   ///< Antialiasing for lines, shapes, etc.
        TextAntialiasing            = 0x0002,   ///< Antialiasing for drawing text
        SmoothImages                = 0x0004,   ///< Adjust images to the device space using smooth transformation (slower, but better image quality)
        IgnoreOptionalContent       = 0x0008,   ///< Ignore optional content (so all is drawn ignoring settings of optional content)
        ClipToCropBox               = 0x0010,   ///< Clip page content to crop box (items outside crop box will not be visible)
        DisplayTimes                = 0x0020,   ///< Display page compile/draw time
        DebugTextBlocks             = 0x0040,   ///< Debug text block layout algorithm
        DebugTextLines              = 0x0080,   ///< Debug text line layout algorithm
        DenyExtraGraphics           = 0x0100,   ///< Do not display additional graphics, for example from tools
        DisplayAnnotations          = 0x0200,   ///< Display annotations
        LogicalSizeZooming          = 0x0400,   ///< Use logical pixel resolution instead of physical one when zooming

        ColorAdjust_Invert          = 0x0800,   ///< Invert colors
        ColorAdjust_Grayscale       = 0x1000,   ///< Convert colors to grayscale
        ColorAdjust_HighContrast    = 0x2000,   ///< Convert colors to high constrast colors
        ColorAdjust_Bitonal         = 0x4000,   ///< Convert colors to bitonal (monochromatic)
        ColorAdjust_CustomColors    = 0x8000,   ///< Convert colors to custom color settings
    };

    Q_DECLARE_FLAGS(Features, Feature)

    explicit PDFRenderer(const PDFDocument* document,
                         const PDFFontCache* fontCache,
                         const PDFCMS* cms,
                         const PDFOptionalContentActivity* optionalContentActivity,
                         Features features,
                         const PDFMeshQualitySettings& meshQualitySettings);

    /// Paints desired page onto the painter. Page is painted in the rectangle using best-fit method.
    /// If the page doesn't exist, then error is returned. No exception is thrown. Rendering errors
    /// are reported and returned in the error list. If no error occured, empty list is returned.
    /// \param painter Painter
    /// \param rectangle Paint area for the page
    /// \param pageIndex Index of the page to be painted
    QList<PDFRenderError> render(QPainter* painter, const QRectF& rectangle, size_t pageIndex) const;

    /// Paints desired page onto the painter. Page is painted using \p matrix, which maps page coordinates
    /// to the device coordinates. If the page doesn't exist, then error is returned. No exception is thrown.
    /// Rendering errors are reported and returned in the error list. If no error occured, empty list is returned.
    QList<PDFRenderError> render(QPainter* painter, const QTransform& matrix, size_t pageIndex) const;

    /// Compiles page (i.e. prepares compiled page). \p page should be empty page, onto which
    /// are graphics commands written. No exception is thrown. Rendering errors are reported and written
    /// to the compiled page.
    /// \param precompiledPage Precompiled page pointer
    /// \param pageIndex Index of page to be compiled
    void compile(PDFPrecompiledPage* precompiledPage, size_t pageIndex) const;

    /// Creates page point to device point matrix for the given rectangle. It creates transformation
    /// from page's media box to the target rectangle.
    /// \param page Page, for which we want to create matrix
    /// \param rectangle Page rectangle, to which is page media box transformed
    /// \param extraRotation Extra rotation applied to the page rotation
    static QTransform createPagePointToDevicePointMatrix(const PDFPage* page,
                                                         const QRectF& rectangle,
                                                         PageRotation extraRotation = PageRotation::None);

    /// Creates media box to device point matrix for the given media box.
    /// \param mediaBox Media box
    /// \param rectangle Page rectangle, to which is page media box transformed
    /// \param rotation Rotation
    static QTransform createMediaBoxToDevicePointMatrix(const QRectF& mediaBox,
                                                        const QRectF& rectangle,
                                                        PageRotation rotation);

    /// Applies rendering flags to the color convertor
    static void applyFeaturesToColorConvertor(const Features& features, PDFColorConvertor& convertor);

    /// Returns default renderer features
    static constexpr Features getDefaultFeatures() { return Features(Antialiasing | TextAntialiasing | ClipToCropBox | DisplayAnnotations); }

    /// Returns color transformation features
    static constexpr Features getColorFeatures() { return Features(ColorAdjust_Invert | ColorAdjust_Grayscale | ColorAdjust_HighContrast | ColorAdjust_Bitonal | ColorAdjust_CustomColors); }

    const PDFOperationControl* getOperationControl() const;
    void setOperationControl(const PDFOperationControl* newOperationControl);

private:
    const PDFDocument* m_document;
    const PDFFontCache* m_fontCache;
    const PDFCMS* m_cms;
    const PDFOptionalContentActivity* m_optionalContentActivity;
    const PDFOperationControl* m_operationControl;
    Features m_features;
    PDFMeshQualitySettings m_meshQualitySettings;
};

/// Renders PDF pages to bitmap images (QImage). It can use OpenGL for painting,
/// if it is enabled, if this is the case, offscreen rendering to framebuffer
/// is used.
/// \note Construct this object only in main GUI thread
class PDF4QTLIBCORESHARED_EXPORT PDFRasterizer : public QObject
{
    Q_OBJECT

private:
    using BaseClass = QObject;

public:
    explicit PDFRasterizer(QObject* parent);
    virtual ~PDFRasterizer() override;

    /// Resets the renderer. This function must be called from main GUI thread,
    /// it cannot be called from deferred threads, because it can create hidden
    /// window (offscreen surface). If hardware renderer is required, and
    /// none is available, then software renderer is used.
    /// \param useOpenGL Use OpenGL for rendering (ignored if not available)
    /// \param surfaceFormat Surface format to render
    void reset(bool useOpenGL, const QSurfaceFormat& surfaceFormat);

    enum Feature
    {
        UseOpenGL               = 0x0001,   ///< Use OpenGL for rendering
        ValidOpenGL             = 0x0002,   ///< OpenGL is initialized and valid
        FailedOpenGL            = 0x0004,   ///< OpenGL creation has failed
    };

    Q_DECLARE_FLAGS(Features, Feature)

    /// Renders page to the image of given size. If some error occurs, then
    /// empty image is returned. Warning: this function can modify this object,
    /// so it is not const and is not thread safe. We can also draw annotations,
    /// if \p annotationManager is not nullptr and annotations are enabled.
    /// \param pageIndex Page index
    /// \param page Page
    /// \param compiledPage Compiled page contents
    /// \param size Size of the target image
    /// \param features Renderer features
    /// \param annotationManager Annotation manager (can be nullptr)
    /// \param extraRotation Extra page rotation
    QImage render(PDFInteger pageIndex,
                  const PDFPage* page,
                  const PDFPrecompiledPage* compiledPage,
                  QSize size,
                  PDFRenderer::Features features,
                  const PDFAnnotationManager* annotationManager,
                  PageRotation extraRotation);

private:
#ifdef PDF4QT_ENABLE_OPENGL
    void initializeOpenGL();
    void releaseOpenGL();
#endif

    Features m_features;
#ifdef PDF4QT_ENABLE_OPENGL
    QSurfaceFormat m_surfaceFormat;
    QOffscreenSurface* m_surface;
    QOpenGLContext* m_context;
    QOpenGLFramebufferObject* m_fbo;
#endif
};

/// Simple structure for storing rendered page images
struct PDFRenderedPageImage
{
    qint64 pageCompileTime = 0;
    qint64 pageWaitTime = 0;
    qint64 pageRenderTime = 0;
    qint64 pageTotalTime = 0;
    PDFInteger pageIndex;
    QImage pageImage;
};

/// Pool of page image renderers. It can use predefined number of renderers to
/// render page images asynchronously. You can use this object in two ways -
/// first one is as standard object pool, second one is to directly render
/// page images asynchronously.
class PDF4QTLIBCORESHARED_EXPORT PDFRasterizerPool : public QObject
{
    Q_OBJECT

private:
    using BaseClass = QObject;

public:


    using PageImageSizeGetter = std::function<QSize(const PDFPage*)>;
    using ProcessImageMethod = std::function<void(PDFRenderedPageImage&)>;

    /// Creates new rasterizer pool
    /// \param document Document
    /// \param fontCache Font cache
    /// \param cmsManager Color management system manager
    /// \param optionalContentActivity Optional content activity
    /// \param features Renderer features
    /// \param meshQualitySettings Mesh quality settings
    /// \param rasterizerCount Number of rasterizers
    /// \param useOpenGL Use OpenGL for rendering?
    /// \param surfaceFormat Surface format
    /// \param parent Parent object
    explicit PDFRasterizerPool(const PDFDocument* document,
                               PDFFontCache* fontCache,
                               const PDFCMSManager* cmsManager,
                               const PDFOptionalContentActivity* optionalContentActivity,
                               PDFRenderer::Features features,
                               const PDFMeshQualitySettings& meshQualitySettings,
                               int rasterizerCount,
                               bool useOpenGL,
                               const QSurfaceFormat& surfaceFormat,
                               QObject* parent);

    /// Acquire rasterizer. This function is thread safe.
    PDFRasterizer* acquire();

    /// Return back (release) rasterizer into rasterizer pool
    /// This function is thread safe.
    /// \param rasterizer Rasterizer
    void release(PDFRasterizer* rasterizer);

    /// Renders pages asynchronously to images, using given page indices,
    /// function which returns rendered size and process image function,
    /// which processes rendered images.
    /// \param pageIndices Page indices for rendered pages
    /// \param imageSizeGetter Getter, which computes image size from page index
    /// \param processImage Method, which processes rendered page images
    /// \param progress Progress indicator
    void render(const std::vector<PDFInteger>& pageIndices,
                const PageImageSizeGetter& imageSizeGetter,
                const ProcessImageMethod& processImage,
                PDFProgress* progress);

    /// Returns default rasterizer count
    static int getDefaultRasterizerCount();

    /// Returns corrected rasterizer count (so, if user
    /// select too high or too low rasterizer count, this function
    /// corrects it to acceptable number.
    /// \param rasterizerCount Requested number of rasterizers
    /// \returns Corrected number of rasterizers
    static int getCorrectedRasterizerCount(int rasterizerCount);

signals:
    void renderError(PDFInteger pageIndex, PDFRenderError error);

private:
    const PDFDocument* m_document;
    PDFFontCache* m_fontCache;
    const PDFCMSManager* m_cmsManager;
    const PDFOptionalContentActivity* m_optionalContentActivity;
    PDFRenderer::Features m_features;
    const PDFMeshQualitySettings& m_meshQualitySettings;

    QSemaphore m_semaphore;
    QMutex m_mutex;
    std::vector<PDFRasterizer*> m_rasterizers;
};

/// Settings object for image writer
class PDF4QTLIBCORESHARED_EXPORT PDFImageWriterSettings
{
public:
    explicit PDFImageWriterSettings();

    /// Returns true, if image option is supported
    bool isOptionSupported(QImageIOHandler::ImageOption option) const { return m_supportedOptions.count(option); }

    /// Returns a list of available image formats
    const QList<QByteArray>& getFormats() const { return m_formats; }

    /// Returns a list of available subtypes
    const QList<QByteArray>& getSubtypes() const { return m_subtypes; }

    /// Selects image format (and initializes default values)
    void selectFormat(const QByteArray& format);

    int getCompression() const;
    void setCompression(int compression);

    int getQuality() const;
    void setQuality(int quality);

    float getGamma() const;
    void setGamma(float gamma);

    bool hasOptimizedWrite() const;
    void setOptimizedWrite(bool optimizedWrite);

    bool hasProgressiveScanWrite() const;
    void setProgressiveScanWrite(bool progressiveScanWrite);

    QByteArray getCurrentFormat() const;

    QByteArray getCurrentSubtype() const;
    void setCurrentSubtype(const QByteArray& currentSubtype);

private:
    int m_compression = 9;
    int m_quality = 100;
    float m_gamma = 1.0;
    bool m_optimizedWrite = false;
    bool m_progressiveScanWrite = false;
    QByteArray m_currentFormat;
    QByteArray m_currentSubtype;
    std::set<QImageIOHandler::ImageOption> m_supportedOptions;

    QList<QByteArray> m_formats;
    QList<QByteArray> m_subtypes;
};

/// This class is for setup of page image exporter
class PDF4QTLIBCORESHARED_EXPORT PDFPageImageExportSettings
{
public:
    explicit PDFPageImageExportSettings() : PDFPageImageExportSettings(nullptr) { }
    explicit PDFPageImageExportSettings(const PDFDocument* document);

    enum class PageSelectionMode
    {
        All,
        Selection
    };

    enum class ResolutionMode
    {
        DPI,
        Pixels
    };

    ResolutionMode getResolutionMode() const;
    void setResolutionMode(ResolutionMode resolution);

    PageSelectionMode getPageSelectionMode() const;
    void setPageSelectionMode(PageSelectionMode pageSelectionMode);

    QString getDirectory() const;
    void setDirectory(const QString& directory);

    QString getFileTemplate() const;
    void setFileTemplate(const QString& fileTemplate);

    QString getPageSelection() const;
    void setPageSelection(const QString& pageSelection);

    int getDpiResolution() const;
    void setDpiResolution(int dpiResolution);

    int getPixelResolution() const;
    void setPixelResolution(int pixelResolution);

    /// Validates the settings, if they can be used for image generation
    bool validate(QString* errorMessagePtr, bool validatePageSelection = true, bool validateFileSettings = true, bool validateResolution = true) const;

    /// Returns list of selected pages
    std::vector<PDFInteger> getPages() const;

    /// Returns output file name for given page
    QString getOutputFileName(PDFInteger pageIndex, const QByteArray& outputFormat) const;

    static constexpr int getMinDPIResolution() { return 72; }
    static constexpr int getMaxDPIResolution() { return 6000; }

    static constexpr int getMinPixelResolution() { return 100; }
    static constexpr int getMaxPixelResolution() { return 16384; }

private:
    const PDFDocument* m_document;
    ResolutionMode m_resolutionMode = ResolutionMode::DPI;
    PageSelectionMode m_pageSelectionMode = PageSelectionMode::All;
    QString m_directory;
    QString m_fileTemplate;
    QString m_pageSelection;
    int m_dpiResolution = 300;
    int m_pixelResolution = 100;
};

}   // namespace pdf

Q_DECLARE_OPERATORS_FOR_FLAGS(pdf::PDFRenderer::Features)

#endif // PDFRENDERER_H
