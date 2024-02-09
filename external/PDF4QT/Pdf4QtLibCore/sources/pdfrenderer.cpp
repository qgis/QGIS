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

#include "pdfrenderer.h"
#include "pdfpainter.h"
#include "pdfdocument.h"
#include "pdfexecutionpolicy.h"
#include "pdfprogress.h"
#include "pdfannotation.h"

#include <QDir>
#include <QElapsedTimer>
#include <QtMath>

#ifdef PDF4QT_ENABLE_OPENGL
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLPaintDevice>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#endif

#include "pdfdbgheap.h"

namespace pdf
{

PDFRenderer::PDFRenderer(const PDFDocument* document,
                         const PDFFontCache* fontCache,
                         const PDFCMS* cms,
                         const PDFOptionalContentActivity* optionalContentActivity,
                         Features features,
                         const PDFMeshQualitySettings& meshQualitySettings) :
    m_document(document),
    m_fontCache(fontCache),
    m_cms(cms),
    m_optionalContentActivity(optionalContentActivity),
    m_operationControl(nullptr),
    m_features(features),
    m_meshQualitySettings(meshQualitySettings)
{
    Q_ASSERT(document);
}

QTransform PDFRenderer::createPagePointToDevicePointMatrix(const PDFPage* page,
                                                           const QRectF& rectangle,
                                                           PageRotation extraRotation)
{
    PageRotation pageRotation = getPageRotationCombined(page->getPageRotation(), extraRotation);
    QRectF mediaBox = page->getRotatedBox(page->getMediaBox(), pageRotation);

    return createMediaBoxToDevicePointMatrix(mediaBox, rectangle, pageRotation);
}

QTransform PDFRenderer::createMediaBoxToDevicePointMatrix(const QRectF& mediaBox,
                                                          const QRectF& rectangle,
                                                          PageRotation rotation)
{
    QTransform matrix;
    switch (rotation)
    {
        case PageRotation::None:
        {
            matrix.translate(rectangle.left(), rectangle.bottom());
            matrix.scale(rectangle.width() / mediaBox.width(), -rectangle.height() / mediaBox.height());
            matrix.translate(-mediaBox.left(), -mediaBox.top());
            break;
        }

        case PageRotation::Rotate90:
        {
            matrix.translate(rectangle.left(), rectangle.top());
            matrix.rotate(90);
            matrix.scale(rectangle.width() / mediaBox.width(), -rectangle.height() / mediaBox.height());
            matrix.translate(-mediaBox.left(), -mediaBox.top());
            break;
        }

        case PageRotation::Rotate270:
        {
            matrix.translate(rectangle.right(), rectangle.top());
            matrix.rotate(-90);
            matrix.translate(-rectangle.height(), 0);
            matrix.scale(rectangle.width() / mediaBox.width(), -rectangle.height() / mediaBox.height());
            matrix.translate(-mediaBox.left(), -mediaBox.top());
            break;
        }

        case PageRotation::Rotate180:
        {
            matrix.translate(rectangle.left(), rectangle.top());
            matrix.scale(rectangle.width() / mediaBox.width(), rectangle.height() / mediaBox.height());
            matrix.translate(mediaBox.width(), 0);
            matrix.translate(-mediaBox.left(), -mediaBox.top());
            matrix.scale(-1.0, 1.0);
            break;
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return matrix;
}

void PDFRenderer::applyFeaturesToColorConvertor(const Features& features, PDFColorConvertor& convertor)
{
    convertor.setMode(PDFColorConvertor::Mode::Normal);

    if (features.testFlag(ColorAdjust_Invert))
    {
        convertor.setMode(PDFColorConvertor::Mode::InvertedColors);
    }

    if (features.testFlag(ColorAdjust_Grayscale))
    {
        convertor.setMode(PDFColorConvertor::Mode::Grayscale);
    }

    if (features.testFlag(ColorAdjust_HighContrast))
    {
        convertor.setMode(PDFColorConvertor::Mode::HighContrast);
    }

    if (features.testFlag(ColorAdjust_Bitonal))
    {
        convertor.setMode(PDFColorConvertor::Mode::Bitonal);
    }

    if (features.testFlag(ColorAdjust_CustomColors))
    {
        convertor.setMode(PDFColorConvertor::Mode::CustomColors);
    }
}

const PDFOperationControl* PDFRenderer::getOperationControl() const
{
    return m_operationControl;
}

void PDFRenderer::setOperationControl(const PDFOperationControl* newOperationControl)
{
    m_operationControl = newOperationControl;
}

QList<PDFRenderError> PDFRenderer::render(QPainter* painter, const QRectF& rectangle, size_t pageIndex) const
{
    const PDFCatalog* catalog = m_document->getCatalog();
    if (pageIndex >= catalog->getPageCount() || !catalog->getPage(pageIndex))
    {
        // Invalid page index
        return { PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Page %1 doesn't exist.").arg(pageIndex + 1)) };
    }

    const PDFPage* page = catalog->getPage(pageIndex);
    Q_ASSERT(page);

    QTransform matrix = createPagePointToDevicePointMatrix(page, rectangle);

    PDFPainter processor(painter, m_features, matrix, page, m_document, m_fontCache, m_cms, m_optionalContentActivity, m_meshQualitySettings);
    processor.setOperationControl(m_operationControl);
    return processor.processContents();
}

QList<PDFRenderError> PDFRenderer::render(QPainter* painter, const QTransform& matrix, size_t pageIndex) const
{
    const PDFCatalog* catalog = m_document->getCatalog();
    if (pageIndex >= catalog->getPageCount() || !catalog->getPage(pageIndex))
    {
        // Invalid page index
        return { PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Page %1 doesn't exist.").arg(pageIndex + 1)) };
    }

    const PDFPage* page = catalog->getPage(pageIndex);
    Q_ASSERT(page);

    PDFPainter processor(painter, m_features, matrix, page, m_document, m_fontCache, m_cms, m_optionalContentActivity, m_meshQualitySettings);
    processor.setOperationControl(m_operationControl);
    return processor.processContents();
}

void PDFRenderer::compile(PDFPrecompiledPage* precompiledPage, size_t pageIndex) const
{
    const PDFCatalog* catalog = m_document->getCatalog();
    if (pageIndex >= catalog->getPageCount() || !catalog->getPage(pageIndex))
    {
        // Invalid page index
        precompiledPage->finalize(0, { PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Page %1 doesn't exist.").arg(pageIndex + 1)) });
        return;
    }

    const PDFPage* page = catalog->getPage(pageIndex);
    Q_ASSERT(page);

    QElapsedTimer timer;
    timer.start();

    PDFPrecompiledPageGenerator generator(precompiledPage, m_features, page, m_document, m_fontCache, m_cms, m_optionalContentActivity, m_meshQualitySettings);
    generator.setOperationControl(m_operationControl);
    QList<PDFRenderError> errors = generator.processContents();

    PDFColorConvertor colorConvertor = m_cms->getColorConvertor();
    PDFRenderer::applyFeaturesToColorConvertor(m_features, colorConvertor);
    precompiledPage->convertColors(colorConvertor);

    precompiledPage->optimize();
    precompiledPage->finalize(timer.nsecsElapsed(), qMove(errors));
    timer.invalidate();
}

PDFRasterizer::PDFRasterizer(QObject* parent) :
    BaseClass(parent),
#ifdef PDF4QT_ENABLE_OPENGL
    m_features(),
    m_surfaceFormat(),
    m_surface(nullptr),
    m_context(nullptr),
    m_fbo(nullptr)
#else
    m_features()
#endif
{

}

PDFRasterizer::~PDFRasterizer()
{
#ifdef PDF4QT_ENABLE_OPENGL
    releaseOpenGL();
#endif
}

void PDFRasterizer::reset(bool useOpenGL, const QSurfaceFormat& surfaceFormat)
{
    if (!PDFRendererInfo::isHardwareAccelerationSupported())
    {
        m_features.setFlag(FailedOpenGL, true);
        m_features.setFlag(ValidOpenGL, false);
    }

#ifdef PDF4QT_ENABLE_OPENGL
    if (useOpenGL != m_features.testFlag(UseOpenGL) || surfaceFormat != m_surfaceFormat)
    {
        // In either case, we must reset OpenGL
        releaseOpenGL();

        m_features.setFlag(UseOpenGL, useOpenGL);
        m_surfaceFormat = surfaceFormat;

        // We create new OpenGL renderer, but only if it hasn't failed (we do not try
        // again to create new OpenGL renderer.
        if (m_features.testFlag(UseOpenGL) && !m_features.testFlag(FailedOpenGL))
        {
            initializeOpenGL();
        }
    }
#else
    Q_UNUSED(surfaceFormat);
    m_features.setFlag(UseOpenGL, useOpenGL);
#endif
}

QImage PDFRasterizer::render(PDFInteger pageIndex,
                             const PDFPage* page,
                             const PDFPrecompiledPage* compiledPage,
                             QSize size,
                             PDFRenderer::Features features,
                             const PDFAnnotationManager* annotationManager,
                             PageRotation extraRotation)
{
    QImage image;

    QTransform matrix = PDFRenderer::createPagePointToDevicePointMatrix(page, QRect(QPoint(0, 0), size), extraRotation);

#ifdef PDF4QT_ENABLE_OPENGL
    if (m_features.testFlag(UseOpenGL) && m_features.testFlag(ValidOpenGL))
    {
        // We have valid OpenGL context, try to select it and possibly create framebuffer object
        // for target image (to paint it using paint device).
        Q_ASSERT(m_surface && m_context);
        if (m_context->makeCurrent(m_surface))
        {
            if (!m_fbo || m_fbo->width() != size.width() || m_fbo->height() != size.height())
            {
                // Delete old framebuffer object
                delete m_fbo;

                // Create a new framebuffer object
                QOpenGLFramebufferObjectFormat format;
                format.setSamples(m_surfaceFormat.samples());
                m_fbo = new QOpenGLFramebufferObject(size.width(), size.height(), format);
            }

            Q_ASSERT(m_fbo);
            if (m_fbo->isValid() && m_fbo->bind())
            {
                // Now, we have bind the buffer. Due to bug in Qt's OpenGL drawing subsystem,
                // we must render it two times, otherwise painter paths will be sometimes
                // replaced by filled rectangles.
                for (int i = 0; i < 2; ++i)
                {
                    QOpenGLPaintDevice device(size);
                    QPainter painter(&device);
                    painter.fillRect(QRect(QPoint(0, 0), size), compiledPage->getPaperColor());
                    compiledPage->draw(&painter, page->getCropBox(), matrix, features, 1.0);

                    if (annotationManager)
                    {
                        QList<PDFRenderError> errors;
                        PDFTextLayoutGetter textLayoutGetter(nullptr, pageIndex);
                        annotationManager->drawPage(&painter, pageIndex, compiledPage, textLayoutGetter, matrix, errors);
                    }
                }

                m_fbo->release();

                image = m_fbo->toImage();
            }
            else
            {
                m_features.setFlag(FailedOpenGL, true);
                m_features.setFlag(ValidOpenGL, false);
            }

            m_context->doneCurrent();
        }
    }
#endif

    if (image.isNull())
    {
        // If we can't use OpenGL, or user doesn't want to use OpenGL, then fallback
        // to standard software rasterizer.
        image = QImage(size, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);

        QPainter painter(&image);
        compiledPage->draw(&painter, page->getCropBox(), matrix, features, 1.0);

        if (annotationManager)
        {
            QList<PDFRenderError> errors;
            PDFTextLayoutGetter textLayoutGetter(nullptr, pageIndex);
            annotationManager->drawPage(&painter, pageIndex, compiledPage, textLayoutGetter, matrix, errors);
        }
    }

    // Jakub Melka: Convert the image into format Format_ARGB32_Premultiplied for fast drawing.
    // If this format is used, then no image conversion is performed while drawing.
    if (image.format() != QImage::Format_ARGB32_Premultiplied)
    {
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
    }

    // Calculate image DPI
    QSizeF rotatedSizeInMeters = page->getRotatedMediaBoxMM().size() / 1000.0;
    QSizeF rotatedSizeInPixels = image.size();
    qreal dpiX = rotatedSizeInPixels.width() / rotatedSizeInMeters.width();
    qreal dpiY = rotatedSizeInPixels.height() / rotatedSizeInMeters.height();
    image.setDotsPerMeterX(qCeil(dpiX));
    image.setDotsPerMeterY(qCeil(dpiY));

    return image;
}

#ifdef PDF4QT_ENABLE_OPENGL
void PDFRasterizer::initializeOpenGL()
{
    Q_ASSERT(!m_surface);
    Q_ASSERT(!m_context);
    Q_ASSERT(!m_fbo);

    m_features.setFlag(ValidOpenGL, false);
    m_features.setFlag(FailedOpenGL, false);

    // Create context
    m_context = new QOpenGLContext(this);
    m_context->setFormat(m_surfaceFormat);
    if (!m_context->create())
    {
        m_features.setFlag(FailedOpenGL, true);

        delete m_context;
        m_context = nullptr;
    }

    // Create surface
    m_surface = new QOffscreenSurface(nullptr, this);
    m_surface->setFormat(m_surfaceFormat);
    m_surface->create();
    if (!m_surface->isValid())
    {
        m_features.setFlag(FailedOpenGL, true);

        delete m_context;
        delete m_surface;

        m_context = nullptr;
        m_surface = nullptr;
    }

    // Check, if we can make it current
    if (m_context->makeCurrent(m_surface))
    {
        m_features.setFlag(ValidOpenGL, true);
        m_context->doneCurrent();
    }
    else
    {
        m_features.setFlag(FailedOpenGL, true);
        releaseOpenGL();
    }
}
#endif

#ifdef PDF4QT_ENABLE_OPENGL
void PDFRasterizer::releaseOpenGL()
{
    if (m_surface)
    {
        Q_ASSERT(m_context);

        // Delete framebuffer
        if (m_fbo)
        {
            m_context->makeCurrent(m_surface);
            delete m_fbo;
            m_fbo = nullptr;
            m_context->doneCurrent();
        }

        // Delete OpenGL context
        delete m_context;
        m_context = nullptr;

        // Delete surface
        m_surface->destroy();
        delete m_surface;
        m_surface = nullptr;

        // Set flag, that we do not have valid OpenGL
        m_features.setFlag(ValidOpenGL, false);
    }
}
#endif

PDFRasterizer* PDFRasterizerPool::acquire()
{
    m_semaphore.acquire();

    QMutexLocker guard(&m_mutex);
    Q_ASSERT(!m_rasterizers.empty());
    PDFRasterizer* rasterizer = m_rasterizers.back();
    m_rasterizers.pop_back();
    return rasterizer;
}

void PDFRasterizerPool::release(pdf::PDFRasterizer* rasterizer)
{
    QMutexLocker guard(&m_mutex);
    Q_ASSERT(std::find(m_rasterizers.cbegin(), m_rasterizers.cend(), rasterizer) == m_rasterizers.cend());
    m_rasterizers.push_back(rasterizer);

    // Jakub Melka: we must release it at the end, to ensure rasterizer is in the array before
    // semaphore is released, to avoid race condition.
    m_semaphore.release();
}

void PDFRasterizerPool::render(const std::vector<PDFInteger>& pageIndices,
                               const PDFRasterizerPool::PageImageSizeGetter& imageSizeGetter,
                               const PDFRasterizerPool::ProcessImageMethod& processImage,
                               PDFProgress* progress)
{
    if (pageIndices.empty())
    {
        return;
    }

    Q_ASSERT(imageSizeGetter);
    Q_ASSERT(processImage);

    QElapsedTimer timer;
    timer.start();

    Q_EMIT renderError(PDFCatalog::INVALID_PAGE_INDEX, PDFRenderError(RenderErrorType::Information, PDFTranslationContext::tr("Start at %1...").arg(QTime::currentTime().toString(Qt::TextDate))));

    if (progress)
    {
        ProgressStartupInfo info;
        info.showDialog = true;
        info.text = PDFTranslationContext::tr("Rendering document into images.");
        progress->start(pageIndices.size(), qMove(info));
    }
    auto processPage = [this, progress, &imageSizeGetter, &processImage](const PDFInteger pageIndex)
    {
        const PDFPage* page = m_document->getCatalog()->getPage(pageIndex);

        if (!page)
        {
            if (progress)
            {
                progress->step();
            }
            Q_EMIT renderError(pageIndex, PDFRenderError(RenderErrorType::Error, PDFTranslationContext::tr("Page %1 not found.").arg(pageIndex)));
            return;
        }

        QElapsedTimer totalPageTimer;
        totalPageTimer.start();

        QElapsedTimer pageTimer;
        pageTimer.start();

        // Precompile the page
        PDFPrecompiledPage precompiledPage;
        PDFCMSPointer cms = m_cmsManager->getCurrentCMS();
        PDFRenderer renderer(m_document, m_fontCache, cms.data(), m_optionalContentActivity, m_features, m_meshQualitySettings);
        renderer.compile(&precompiledPage, pageIndex);

        qint64 pageCompileTime = pageTimer.restart();

        for (const PDFRenderError& error : precompiledPage.getErrors())
        {
            Q_EMIT renderError(pageIndex, error);
        }

        // We can const-cast here, because we do not modify the document in annotation manager.
        // Annotations are just rendered to the target picture.
        PDFModifiedDocument modifiedDocument(const_cast<PDFDocument*>(m_document), const_cast<PDFOptionalContentActivity*>(m_optionalContentActivity));

        // Annotation manager
        PDFAnnotationManager annotationManager(m_fontCache, m_cmsManager, m_optionalContentActivity, m_meshQualitySettings, m_features, PDFAnnotationManager::Target::Print, nullptr);
        annotationManager.setDocument(modifiedDocument);

        // Render page to image
        pageTimer.restart();
        PDFRasterizer* rasterizer = acquire();
        qint64 pageWaitTime = pageTimer.restart();
        QImage image = rasterizer->render(pageIndex, page, &precompiledPage, imageSizeGetter(page), m_features, &annotationManager, PageRotation::None);
        qint64 pageRenderTime = pageTimer.elapsed();
        release(rasterizer);

        // Now, process the image
        PDFRenderedPageImage renderedPageImage;
        renderedPageImage.pageIndex = pageIndex;
        renderedPageImage.pageImage = qMove(image);
        renderedPageImage.pageCompileTime = pageCompileTime;
        renderedPageImage.pageWaitTime = pageWaitTime;
        renderedPageImage.pageRenderTime = pageRenderTime;
        renderedPageImage.pageTotalTime = totalPageTimer.elapsed();
        processImage(renderedPageImage);

        if (progress)
        {
            progress->step();
        }
    };
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, pageIndices.cbegin(), pageIndices.cend(), processPage);

    if (progress)
    {
        progress->finish();
    }

    Q_EMIT renderError(PDFCatalog::INVALID_PAGE_INDEX, PDFRenderError(RenderErrorType::Information, PDFTranslationContext::tr("Finished at %1...").arg(QTime::currentTime().toString(Qt::TextDate))));
    Q_EMIT renderError(PDFCatalog::INVALID_PAGE_INDEX, PDFRenderError(RenderErrorType::Information, PDFTranslationContext::tr("%1 miliseconds elapsed to render %2 pages...").arg(timer.nsecsElapsed() / 1000000).arg(pageIndices.size())));
}

int PDFRasterizerPool::getDefaultRasterizerCount()
{
    int hint = QThread::idealThreadCount() / 2;
    return getCorrectedRasterizerCount(hint);
}

int PDFRasterizerPool::getCorrectedRasterizerCount(int rasterizerCount)
{
    return qBound(1, rasterizerCount, 256);
}

PDFImageWriterSettings::PDFImageWriterSettings()
{
    m_formats = QImageWriter::supportedImageFormats();

    constexpr const char* DEFAULT_FORMAT = "png";
    if (m_formats.count(DEFAULT_FORMAT))
    {
        selectFormat(DEFAULT_FORMAT);
    }
    else
    {
        selectFormat(m_formats.front());
    }
}

void PDFImageWriterSettings::selectFormat(const QByteArray& format)
{
    if (m_currentFormat != format)
    {
        m_currentFormat = format;

        QImageWriter writer;
        writer.setFormat(format);

        m_compression = 0;
        m_quality = 0;
        m_gamma = 0;
        m_optimizedWrite = false;
        m_progressiveScanWrite = false;
        m_subtypes = writer.supportedSubTypes();
        m_currentSubtype = !m_subtypes.isEmpty() ? m_subtypes.front() : QByteArray();

        // Jakub Melka: init default values based on image handler. Unfortunately,
        // image writer doesn't give us access to these values, so they are hardcoded.
        if (format == "jpeg" || format == "jpg")
        {
            m_quality = 75;
            m_optimizedWrite = false;
            m_progressiveScanWrite = false;
        }
        else if (format == "png")
        {
            m_compression = 50;
            m_quality = 50;
            m_gamma = 0;
        }
        else if (format == "tif" || format == "tiff")
        {
            m_compression = 1;
        }
        else if (format == "webp")
        {
            m_quality = 75;
        }

        m_supportedOptions.clear();
        for (QImageIOHandler::ImageOption imageOption : { QImageIOHandler::CompressionRatio, QImageIOHandler::Quality,
                                                          QImageIOHandler::Gamma, QImageIOHandler::OptimizedWrite,
                                                          QImageIOHandler::ProgressiveScanWrite, QImageIOHandler::SupportedSubTypes })
        {
            if (writer.supportsOption(imageOption))
            {
                m_supportedOptions.insert(imageOption);
            }
        }
    }
}

int PDFImageWriterSettings::getCompression() const
{
    return m_compression;
}

void PDFImageWriterSettings::setCompression(int compression)
{
    m_compression = compression;
}

int PDFImageWriterSettings::getQuality() const
{
    return m_quality;
}

void PDFImageWriterSettings::setQuality(int quality)
{
    m_quality = quality;
}

float PDFImageWriterSettings::getGamma() const
{
    return m_gamma;
}

void PDFImageWriterSettings::setGamma(float gamma)
{
    m_gamma = gamma;
}

bool PDFImageWriterSettings::hasOptimizedWrite() const
{
    return m_optimizedWrite;
}

void PDFImageWriterSettings::setOptimizedWrite(bool optimizedWrite)
{
    m_optimizedWrite = optimizedWrite;
}

bool PDFImageWriterSettings::hasProgressiveScanWrite() const
{
    return m_progressiveScanWrite;
}

void PDFImageWriterSettings::setProgressiveScanWrite(bool progressiveScanWrite)
{
    m_progressiveScanWrite = progressiveScanWrite;
}

QByteArray PDFImageWriterSettings::getCurrentFormat() const
{
    return m_currentFormat;
}

QByteArray PDFImageWriterSettings::getCurrentSubtype() const
{
    return m_currentSubtype;
}

void PDFImageWriterSettings::setCurrentSubtype(const QByteArray& currentSubtype)
{
    m_currentSubtype = currentSubtype;
}

PDFPageImageExportSettings::PDFPageImageExportSettings(const PDFDocument* document) :
    m_document(document)
{
    m_fileTemplate = PDFTranslationContext::tr("Image_%");
}

PDFPageImageExportSettings::ResolutionMode PDFPageImageExportSettings::getResolutionMode() const
{
    return m_resolutionMode;
}

void PDFPageImageExportSettings::setResolutionMode(ResolutionMode resolution)
{
    m_resolutionMode = resolution;
}

PDFPageImageExportSettings::PageSelectionMode PDFPageImageExportSettings::getPageSelectionMode() const
{
    return m_pageSelectionMode;
}

void PDFPageImageExportSettings::setPageSelectionMode(PageSelectionMode pageSelectionMode)
{
    m_pageSelectionMode = pageSelectionMode;
}

QString PDFPageImageExportSettings::getDirectory() const
{
    return m_directory;
}

void PDFPageImageExportSettings::setDirectory(const QString& directory)
{
    m_directory = directory;
}

QString PDFPageImageExportSettings::getFileTemplate() const
{
    return m_fileTemplate;
}

void PDFPageImageExportSettings::setFileTemplate(const QString& fileTemplate)
{
    m_fileTemplate = fileTemplate;
}

QString PDFPageImageExportSettings::getPageSelection() const
{
    return m_pageSelection;
}

void PDFPageImageExportSettings::setPageSelection(const QString& pageSelection)
{
    m_pageSelection = pageSelection;
}

int PDFPageImageExportSettings::getDpiResolution() const
{
    return m_dpiResolution;
}

void PDFPageImageExportSettings::setDpiResolution(int dpiResolution)
{
    m_dpiResolution = dpiResolution;
}

int PDFPageImageExportSettings::getPixelResolution() const
{
    return m_pixelResolution;
}

void PDFPageImageExportSettings::setPixelResolution(int pixelResolution)
{
    m_pixelResolution = pixelResolution;
}

bool PDFPageImageExportSettings::validate(QString* errorMessagePtr, bool validatePageSelection, bool validateFileSettings, bool validateResolution) const
{
    QString dummy;
    QString& errorMessage = errorMessagePtr ? *errorMessagePtr : dummy;

    if (validateFileSettings)
    {
        if (m_directory.isEmpty())
        {
            errorMessage = PDFTranslationContext::tr("Target directory is empty.");
            return false;
        }

        // Check, if target directory exists
        QDir directory(m_directory);
        if (!directory.exists())
        {
            errorMessage = PDFTranslationContext::tr("Target directory '%1' doesn't exist.").arg(m_directory);
            return false;
        }

        if (m_fileTemplate.isEmpty())
        {
            errorMessage = PDFTranslationContext::tr("File template is empty.");
            return false;
        }

        if (!m_fileTemplate.contains("%"))
        {
            errorMessage = PDFTranslationContext::tr("File template must contain character '%' for page number.");
            return false;
        }
    }

    // Check page selection
    if (validatePageSelection)
    {
        if (m_pageSelectionMode == PageSelectionMode::Selection)
        {
            std::vector<PDFInteger> pages = getPages();
            if (pages.empty())
            {
                errorMessage = PDFTranslationContext::tr("Page list is invalid. It should have form such as '1-12,17,24,27-29'.");
                return false;
            }

            if (pages.back() >= PDFInteger(m_document->getCatalog()->getPageCount()))
            {
                errorMessage = PDFTranslationContext::tr("Page list contains page, which is not in the document (%1).").arg(pages.back());
                return false;
            }
        }
    }

    if (validateResolution)
    {
        if (m_resolutionMode == ResolutionMode::DPI && (m_dpiResolution < getMinDPIResolution() || m_dpiResolution > getMaxDPIResolution()))
        {
            errorMessage = PDFTranslationContext::tr("DPI resolution should be in range %1 to %2.").arg(getMinDPIResolution()).arg(getMaxDPIResolution());
            return false;
        }

        if (m_resolutionMode == ResolutionMode::Pixels && (m_pixelResolution < getMinPixelResolution() || m_pixelResolution > getMaxPixelResolution()))
        {
            errorMessage = PDFTranslationContext::tr("Pixel resolution should be in range %1 to %2.").arg(getMinPixelResolution()).arg(getMaxPixelResolution());
            return false;
        }
    }

    return true;
}

std::vector<PDFInteger> PDFPageImageExportSettings::getPages() const
{
    std::vector<PDFInteger> result;

    switch (m_pageSelectionMode)
    {
        case PageSelectionMode::All:
        {
            result.resize(m_document->getCatalog()->getPageCount(), 0);
            std::iota(result.begin(), result.end(), 0);
            break;
        }

        case PageSelectionMode::Selection:
        {
            bool ok = false;
            QStringList parts = m_pageSelection.split(QChar(','), Qt::SkipEmptyParts, Qt::CaseSensitive);
            for (const QString& part : parts)
            {
                QStringList numbers = part.split(QChar('-'), Qt::KeepEmptyParts, Qt::CaseSensitive);
                switch (numbers.size())
                {
                    case 1:
                    {
                        const QString& numberString = numbers.front();
                        result.push_back(numberString.toLongLong(&ok) - 1);
                        break;
                    }

                    case 2:
                    {
                        bool ok1 = false;
                        bool ok2 = false;
                        const QString& lowString = numbers.front();
                        const QString& highString = numbers.back();
                        const PDFInteger low = lowString.toLongLong(&ok1) - 1;
                        const PDFInteger high = highString.toLongLong(&ok2) - 1;
                        ok = ok1 && ok2 && low <= high && low >= 0;
                        if (ok)
                        {
                            const PDFInteger count = high - low + 1;
                            result.resize(result.size() + count, 0);
                            std::iota(std::prev(result.end(), count), result.end(), low);
                        }
                        break;
                    }

                    default:
                    {
                        ok = true;
                        break;
                    }
                }

                // If error is detected, do not continue in parsing
                if (!ok)
                {
                    break;
                }

                // We must remove duplicate pages
                std::sort(result.begin(), result.end());
                result.erase(std::unique(result.begin(), result.end()), result.end());
            }

            if (!ok)
            {
                result.clear();
            }

            break;
        }

        default:
            break;
    }

    return result;
}

QString PDFPageImageExportSettings::getOutputFileName(PDFInteger pageIndex, const QByteArray& outputFormat) const
{
    QString fileName = m_fileTemplate;
    fileName.replace('%', QString::number(pageIndex + 1));

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix() != outputFormat)
    {
        fileName = QString("%1.%2").arg(fileName, QString::fromLatin1(outputFormat));
    }

    // Add directory
    QString fileNameWithDirectory = QString("%1/%2").arg(m_directory, fileName);
    return QDir::toNativeSeparators(fileNameWithDirectory);
}

PDFRasterizerPool::PDFRasterizerPool(const PDFDocument* document,
                                     PDFFontCache* fontCache,
                                     const PDFCMSManager* cmsManager,
                                     const PDFOptionalContentActivity* optionalContentActivity,
                                     PDFRenderer::Features features,
                                     const PDFMeshQualitySettings& meshQualitySettings,
                                     int rasterizerCount,
                                     bool useOpenGL,
                                     const QSurfaceFormat& surfaceFormat,
                                     QObject* parent) :
    BaseClass(parent),
    m_document(document),
    m_fontCache(fontCache),
    m_cmsManager(cmsManager),
    m_optionalContentActivity(optionalContentActivity),
    m_features(features),
    m_meshQualitySettings(meshQualitySettings),
    m_semaphore(rasterizerCount)
{
    m_rasterizers.reserve(rasterizerCount);
    for (int i = 0; i < rasterizerCount; ++i)
    {
        m_rasterizers.push_back(new PDFRasterizer(this));
        m_rasterizers.back()->reset(useOpenGL, surfaceFormat);
    }
}

PDFCachedItem<PDFRendererInfo::Info> PDFRendererInfo::s_info;

const PDFRendererInfo::Info& PDFRendererInfo::getHardwareAccelerationSupportedInfo()
{
    auto getInfo = []()
    {
        Info info;

#ifdef PDF4QT_ENABLE_OPENGL
        QOffscreenSurface surface;
        surface.create();

        if (!surface.isValid())
        {
            info.renderer = PDFTranslationContext::tr("GDI Generic");
            info.version = PDFTranslationContext::tr("1.1");
            info.vendor = PDFTranslationContext::tr("System");
            return info;
        }

        QOpenGLContext context;

        if (!context.create())
        {
            info.renderer = PDFTranslationContext::tr("GDI Generic");
            info.version = PDFTranslationContext::tr("1.1");
            info.vendor = PDFTranslationContext::tr("System");
            surface.destroy();
            return info;
        }

        if (!context.makeCurrent(&surface))
        {
            info.renderer = PDFTranslationContext::tr("GDI Generic");
            info.version = PDFTranslationContext::tr("1.1");
            info.vendor = PDFTranslationContext::tr("System");
            surface.destroy();
            return info;
        }

        const char* versionStr = reinterpret_cast<const char*>(context.functions()->glGetString(GL_VERSION));
        const char* vendorStr = reinterpret_cast<const char*>(context.functions()->glGetString(GL_VENDOR));
        const char* rendererStr = reinterpret_cast<const char*>(context.functions()->glGetString(GL_RENDERER));

        QString versionString = QString::fromLocal8Bit(versionStr, std::strlen(versionStr));
        QString vendorString = QString::fromLocal8Bit(vendorStr, std::strlen(vendorStr));
        QString rendererString = QString::fromLocal8Bit(rendererStr, std::strlen(rendererStr));

        context.doneCurrent();
        surface.destroy();

        versionString = versionString.trimmed();

        int spaceIndex = versionString.indexOf(QChar(QChar::Space));
        if (spaceIndex != -1)
        {
            versionString = versionString.left(spaceIndex);
        }

        info.vendor = vendorString;
        info.renderer = rendererString;
        info.version = versionString;

        QStringList versionStrSplitted = versionString.split('.', Qt::KeepEmptyParts);

        if (versionStrSplitted.size() >= 2)
        {
            info.majorOpenGLVersion = versionStrSplitted[0].toInt();
            info.minorOpenGLVersion = versionStrSplitted[1].toInt();
        }
#endif

        return info;
    };

    return s_info.get(getInfo);
}

bool PDFRendererInfo::isHardwareAccelerationSupported()
{
#ifdef PDF4QT_ENABLE_OPENGL
    const Info& info = getHardwareAccelerationSupportedInfo();
    return std::make_pair(info.majorOpenGLVersion, info.minorOpenGLVersion) >= std::make_pair(REQUIRED_OPENGL_MAJOR_VERSION, REQUIRED_OPENGL_MINOR_VERSION);
#else
    return false;
#endif
}

}   // namespace pdf
