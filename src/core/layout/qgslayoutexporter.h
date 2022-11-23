/***************************************************************************
                              qgslayoutexporter.h
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTEXPORTER_H
#define QGSLAYOUTEXPORTER_H

#include "qgis_core.h"
#include "qgsmargins.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoutitem.h"
#include <QPointer>
#include <QSize>
#include <QRectF>
#include <QVector>
#include <functional>

#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

class QgsLayout;
class QPainter;
class QgsLayoutItemMap;
class QgsAbstractLayoutIterator;
class QgsFeedback;
class QgsLabelingResults;

/**
 * \ingroup core
 * \class QgsLayoutExporter
 * \brief Handles rendering and exports of layouts to various formats.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutExporter
{

  public:

    //! Contains details of a page being exported by the class
    struct PageExportDetails
    {
      //! Target folder
      QString directory;

      //! Base part of filename (i.e. file name without extension or '.')
      QString baseName;

      //! File suffix/extension (without the leading '.')
      QString extension;

      //! Page number, where 0 = first page.
      int page = 0;
    };

    /**
     * Constructor for QgsLayoutExporter, for the specified \a layout.
     */
    QgsLayoutExporter( QgsLayout *layout );

    virtual ~QgsLayoutExporter();

    /**
     * Returns the layout linked to this exporter.
     */
    QgsLayout *layout() const;

    /**
     * Renders a full page to a destination \a painter.
     *
     * The \a page argument specifies the page number to render. Page numbers
     * are 0 based, such that the first page in a layout is page 0.
     *
     * \see renderRegion()
     */
    void renderPage( QPainter *painter, int page ) const;

    /**
     * Renders a full page to an image.
     *
     * The \a page argument specifies the page number to render. Page numbers
     * are 0 based, such that the first page in a layout is page 0.
     *
     * The optional \a imageSize parameter can specify the target image size, in pixels.
     * It is the caller's responsibility to ensure that the ratio of the target image size
     * matches the ratio of the corresponding layout page size.
     *
     * The \a dpi parameter is an optional dpi override. Set to -1 to use the default layout print
     * resolution. This parameter has no effect if \a imageSize is specified.
     *
     * Returns the rendered image, or a null QImage if the image does not fit into available memory.
     *
     * \see renderPage()
     * \see renderRegionToImage()
     */
    QImage renderPageToImage( int page, QSize imageSize = QSize(), double dpi = -1 ) const;

    /**
     * Renders a \a region from the layout to a \a painter. This method can be used
     * to render sections of pages rather than full pages.
     *
     * \see renderPage()
     * \see renderRegionToImage()
     */
    void renderRegion( QPainter *painter, const QRectF &region ) const;

    /**
     * Renders a \a region of the layout to an image. This method can be used to render
     * sections of pages rather than full pages.
     *
     * The optional \a imageSize parameter can specify the target image size, in pixels.
     * It is the caller's responsibility to ensure that the ratio of the target image size
     * matches the ratio of the specified region of the layout.
     *
     * The \a dpi parameter is an optional dpi override. Set to -1 to use the default layout print
     * resolution. This parameter has no effect if \a imageSize is specified.
     *
     * Returns the rendered image, or a null QImage if the image does not fit into available memory.
     *
     * \see renderRegion()
     * \see renderPageToImage()
     */
    QImage renderRegionToImage( const QRectF &region, QSize imageSize = QSize(), double dpi = -1 ) const;


    //! Result codes for exporting layouts
    enum ExportResult
    {
      Success, //!< Export was successful
      Canceled, //!< Export was canceled
      MemoryError, //!< Unable to allocate memory required to export
      FileError, //!< Could not write to destination file, likely due to a lock held by another application
      PrintError, //!< Could not start printing to destination device
      SvgLayerError, //!< Could not create layered SVG file
      IteratorError, //!< Error iterating over layout
    };

    //! Contains settings relating to exporting layouts to raster images
    struct ImageExportSettings
    {
      //! Constructor for ImageExportSettings
      ImageExportSettings()
        : flags( QgsLayoutRenderContext::FlagAntialiasing | QgsLayoutRenderContext::FlagUseAdvancedEffects )
      {}

      //! Resolution to export layout at. If dpi <= 0 the default layout dpi will be used.
      double dpi = -1;

      /**
       * Manual size in pixels for output image. If imageSize is not
       * set then it will be automatically calculated based on the
       * output dpi and layout size.
       *
       * If cropToContents is TRUE then imageSize has no effect.
       *
       * Be careful when specifying manual sizes if pages in the layout
       * have differing sizes! It's likely not going to give a reasonable
       * output in this case, and the automatic dpi-based image size should be
       * used instead.
       */
      QSize imageSize;

      /**
       * Set to TRUE if image should be cropped so only parts of the layout
       * containing items are exported.
       */
      bool cropToContents = false;

      /**
       * Crop to content margins, in pixels. These margins will be added
       * to the bounds of the exported layout if cropToContents is TRUE.
       */
      QgsMargins cropMargins;

      /**
       * List of specific pages to export, or an empty list to
       * export all pages.
       *
       * Page numbers are 0 index based, so the first page in the
       * layout corresponds to page 0.
       */
      QList< int > pages;

      /**
       * Set to TRUE to generate an external world file alongside
       * exported images.
       */
      bool generateWorldFile = false;

      /**
       * Indicates whether image export should include metadata generated
       * from the layout's project's metadata.
       *
       * \since QGIS 3.2
       */
      bool exportMetadata = true;


      /**
       * Layout context flags, which control how the export will be created.
       */
      QgsLayoutRenderContext::Flags flags = QgsLayoutRenderContext::Flags();

      /**
       * A list of predefined scales to use with the layout. This is used
       * for maps which are set to the predefined atlas scaling mode.
       * \since QGIS 3.10
       */
      QVector<qreal> predefinedMapScales;

    };

    /**
     * Exports the layout to the \a filePath, using the specified export \a settings.
     *
     * If the layout is a multi-page layout, then filenames for each page will automatically
     * be generated by appending "_1", "_2", etc to the image file's base name.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error code is returned, errorFile() can be called
     * to determine the filename for the export which encountered the error.
     */
    ExportResult exportToImage( const QString &filePath, const QgsLayoutExporter::ImageExportSettings &settings );


    /**
     * Exports a layout \a iterator to raster images, with the specified export \a settings.
     *
     * The \a baseFilePath argument gives a base file path, which is modified by the
     * iterator to obtain file paths for each iterator feature.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error was obtained then \a error will be set
     * to the error description.
     */
    static ExportResult exportToImage( QgsAbstractLayoutIterator *iterator, const QString &baseFilePath,
                                       const QString &extension, const QgsLayoutExporter::ImageExportSettings &settings,
                                       QString &error SIP_OUT, QgsFeedback *feedback = nullptr );


    //! Contains settings relating to exporting layouts to PDF
    struct PdfExportSettings
    {
      //! Constructor for PdfExportSettings
      PdfExportSettings()
        : flags( QgsLayoutRenderContext::FlagAntialiasing | QgsLayoutRenderContext::FlagUseAdvancedEffects )
      {}

      //! Resolution to export layout at. If dpi <= 0 the default layout dpi will be used.
      double dpi = -1;

      /**
       * Set to TRUE to force whole layout to be rasterized while exporting.
       *
       * This option is mutually exclusive with forceVectorOutput.
       */
      bool rasterizeWholeImage = false;

      /**
       * Set to TRUE to force vector object exports, even when the resultant appearance will differ
       * from the layout. If FALSE, some items may be rasterized in order to maintain their
       * correct appearance in the output.
       *
       * This option is mutually exclusive with rasterizeWholeImage.
       */
      bool forceVectorOutput = false;

      /**
       * Indicates whether PDF export should append georeference data
       *
       * \since QGIS 3.10
       */
      bool appendGeoreference = true;

      /**
       * Indicates whether PDF export should include metadata generated
       * from the layout's project's metadata.
       *
       * \since QGIS 3.2
       */
      bool exportMetadata = true;

      /**
       * Layout context flags, which control how the export will be created.
       */
      QgsLayoutRenderContext::Flags flags = QgsLayoutRenderContext::Flags();

      /**
       * Text rendering format, which controls how text should be rendered in the export (e.g.
       * as paths or real text objects).
       *
       * \since QGIS 3.4.3
       */
      Qgis::TextRenderFormat textRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;

      /**
       * Indicates whether vector geometries should be simplified to avoid redundant extraneous detail,
       * such as vertices which are not visible at the specified dpi of the output.
       *
       * \since QGIS 3.10
       */
      bool simplifyGeometries = true;

      /**
       * TRUE if GeoPDF files should be created, instead of normal PDF files.
       *
       * Whilst GeoPDF files can include some desirable properties like the ability to interactively
       * query map features, they also can result in lower-quality output files, or forced rasterization
       * of layers.
       *
       * \note Requires builds based on GDAL 3.0 or greater.
       *
       * \since QGIS 3.10
       */
      bool writeGeoPdf = false;

      /**
       * TRUE if individual layers from the layout should be rendered to separate PDF files.
       *
       * This option allows for separation of logic layout layers to individual PDF files. For instance,
       * if this option is TRUE, then a separate PDF file will be created per layer per map item in the
       * layout. Additionally, separate PDF files may be created for other complex layout items, resulting
       * in a set of PDF files which contain logical atomic components of the layout.
       *
       * This option is designed to allow the PDF files to be composited back together in an external
       * application (e.g. Adobe Illustrator) as a non-QGIS, post-production step.
       *
       * \since QGIS 3.14
       */
      bool exportLayersAsSeperateFiles = false; // TODO QGIS 4 fix typo  //#spellok

      /**
       * TRUE if ISO3200 extension format georeferencing should be used.
       *
       * This is a recommended setting which results in Geospatial PDF files compatible
       * with the built-in Acrobat geospatial tools.
       *
       * If PdfExportSettings::writeGeoPdf is FALSE than this option has no effect.
       */
      bool useIso32000ExtensionFormatGeoreferencing = true;

      /**
       * TRUE if OGC "best practice" format georeferencing should be used.
       *
       * \warning This results in GeoPDF files compatible with the TerraGo suite of tools, but
       * can break compatibility with the built-in Acrobat geospatial tools (yes, GeoPDF
       * format is a mess!).
       *
       * If PdfExportSettings::writeGeoPdf is FALSE than this option has no effect.
      */
      bool useOgcBestPracticeFormatGeoreferencing = false;

      /**
       * TRUE if feature vector information (such as attributes) should be exported during GeoPDF exports.
       *
       * If PdfExportSettings::writeGeoPdf is FALSE than this option has no effect.
       */
      bool includeGeoPdfFeatures = true;

      /**
       * Optional list of map themes to export as GeoPDF layer groups.
       *
       * If set, map item's which are not assigned a specific map theme will iterate through all listed
       * themes and a GeoPDF layer group will be created for each.
       *
       * If PdfExportSettings::writeGeoPdf is FALSE than this option has no effect.
       */
      QStringList exportThemes;

      /**
       * A list of predefined scales to use with the layout. This is used
       * for maps which are set to the predefined atlas scaling mode.
       * \since QGIS 3.10
       */
      QVector<qreal> predefinedMapScales;

    };

    /**
     * Exports the layout as a PDF to the \a filePath, using the specified export \a settings.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered.
     */
    ExportResult exportToPdf( const QString &filePath, const QgsLayoutExporter::PdfExportSettings &settings );

    /**
     * Exports a layout \a iterator to a single PDF file, with the specified export \a settings.
     *
     * The \a fileName argument gives the destination file name for the output PDF.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error was obtained then \a error will be set
     * to the error description.
     *
     * \see exportToPdfs()
     */
    static ExportResult exportToPdf( QgsAbstractLayoutIterator *iterator, const QString &fileName,
                                     const QgsLayoutExporter::PdfExportSettings &settings,
                                     QString &error SIP_OUT, QgsFeedback *feedback = nullptr );

    /**
     * Exports a layout \a iterator to multiple PDF files, with the specified export \a settings.
     *
     * The \a baseFilePath argument gives a base file path, which is modified by the
     * iterator to obtain file paths for each iterator feature.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error was obtained then \a error will be set
     * to the error description.
     *
     * \see exportToPdf()
     */
    static ExportResult exportToPdfs( QgsAbstractLayoutIterator *iterator, const QString &baseFilePath,
                                      const QgsLayoutExporter::PdfExportSettings &settings,
                                      QString &error SIP_OUT, QgsFeedback *feedback = nullptr );


    //! Contains settings relating to printing layouts
    struct PrintExportSettings
    {
      //! Constructor for PrintExportSettings
      PrintExportSettings()
        : flags( QgsLayoutRenderContext::FlagAntialiasing | QgsLayoutRenderContext::FlagUseAdvancedEffects )
      {}

      //! Resolution to export layout at. If dpi <= 0 the default layout dpi will be used.
      double dpi = -1;

      /**
       * Set to TRUE to force whole layout to be rasterized while exporting.
       *
       * This option is mutually exclusive with forceVectorOutput.
       */
      bool rasterizeWholeImage = false;

      /**
       * Layout context flags, which control how the export will be created.
       */
      QgsLayoutRenderContext::Flags flags = QgsLayoutRenderContext::Flags();

      /**
       * A list of predefined scales to use with the layout. This is used
       * for maps which are set to the predefined atlas scaling mode.
       * \since QGIS 3.10
       */
      QVector<qreal> predefinedMapScales;

    };

#ifndef QT_NO_PRINTER

    /**
     * Prints the layout to a \a printer, using the specified export \a settings.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered.
     */
    ExportResult print( QPrinter &printer, const QgsLayoutExporter::PrintExportSettings &settings );

    /**
     * Exports a layout \a iterator to a \a printer, with the specified export \a settings.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error was obtained then \a error will be set
     * to the error description.
     */
    static ExportResult print( QgsAbstractLayoutIterator *iterator, QPrinter &printer,
                               const QgsLayoutExporter::PrintExportSettings &settings,
                               QString &error SIP_OUT, QgsFeedback *feedback = nullptr );
#endif

    //! Contains settings relating to exporting layouts to SVG
    struct SvgExportSettings
    {
      //! Constructor for SvgExportSettings
      SvgExportSettings()
        : flags( QgsLayoutRenderContext::FlagAntialiasing | QgsLayoutRenderContext::FlagUseAdvancedEffects )
      {}

      //! Resolution to export layout at. If dpi <= 0 the default layout dpi will be used.
      double dpi = -1;

      /**
       * Set to TRUE to force vector object exports, even when the resultant appearance will differ
       * from the layout. If FALSE, some items may be rasterized in order to maintain their
       * correct appearance in the output.
       *
       * This option is mutually exclusive with rasterizeWholeImage.
       */
      bool forceVectorOutput = false;

      /**
       * Set to TRUE if image should be cropped so only parts of the layout
       * containing items are exported.
       */
      bool cropToContents = false;

      /**
       * Crop to content margins, in layout units. These margins will be added
       * to the bounds of the exported layout if cropToContents is TRUE.
       */
      QgsMargins cropMargins;

      /**
       * Set to TRUE to export as a layered SVG file.
       * Note that this option is considered experimental, and the generated
       * SVG may differ from the expected appearance of the layout.
       */
      bool exportAsLayers = false;

      /**
       * Set to TRUE to export labels to separate layers (grouped by map layer)
       * in layered SVG exports.
       *
       * This option is only used if exportAsLayers is TRUE.
       *
       * \since QGIS 3.10
       */
      bool exportLabelsToSeparateLayers = true;

      /**
       * Indicates whether SVG export should include RDF metadata generated
       * from the layout's project's metadata.
       *
       * \since QGIS 3.2
       */
      bool exportMetadata = true;

      /**
       * Layout context flags, which control how the export will be created.
       */
      QgsLayoutRenderContext::Flags flags = QgsLayoutRenderContext::Flags();

      /**
       * Text rendering format, which controls how text should be rendered in the export (e.g.
       * as paths or real text objects).
       *
       * \since QGIS 3.4.3
       */
      Qgis::TextRenderFormat textRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;

      /**
       * Indicates whether vector geometries should be simplified to avoid redundant extraneous detail,
       * such as vertices which are not visible at the specified dpi of the output.
       *
       * \since QGIS 3.10
       */
      bool simplifyGeometries = true;

      /**
       * A list of predefined scales to use with the layout. This is used
       * for maps which are set to the predefined atlas scaling mode.
       * \since QGIS 3.10
       */
      QVector<qreal> predefinedMapScales;
    };

    /**
     * Exports the layout as an SVG to the \a filePath, using the specified export \a settings.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered.
     */
    ExportResult exportToSvg( const QString &filePath, const QgsLayoutExporter::SvgExportSettings &settings );

    /**
     * Exports a layout \a iterator to SVG files, with the specified export \a settings.
     *
     * The \a baseFilePath argument gives a base file path, which is modified by the
     * iterator to obtain file paths for each iterator feature.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error was obtained then \a error will be set
     * to the error description.
     */
    static ExportResult exportToSvg( QgsAbstractLayoutIterator *iterator, const QString &baseFilePath,
                                     const QgsLayoutExporter::SvgExportSettings &settings,
                                     QString &error SIP_OUT, QgsFeedback *feedback = nullptr );


    /**
     * Returns the file name corresponding to the last error encountered during
     * an export.
     */
    QString errorFile() const { return mErrorFileName; }

    /**
     * Returns the labeling results for all map items included in the export. Map keys are the item UUIDs (see QgsLayoutItem::uuid()).
     *
     * Ownership of the results remains with the layout exporter.
     *
     * \since QGIS 3.20
     */
    QMap< QString, QgsLabelingResults * > labelingResults();

#ifndef SIP_RUN

    /**
     * Takes the labeling results for all map items included in the export. Map keys are the item UUIDs (see QgsLayoutItem::uuid()).
     *
     * Ownership of the results is transferred to the caller.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.20
     */
    QMap< QString, QgsLabelingResults * > takeLabelingResults();
#endif

    /**
     * Georeferences a \a file (image of PDF) exported from the layout.
     *
     * The \a referenceMap argument specifies a map item to use for georeferencing. If left as NULLPTR, the
     * default layout QgsLayout::referenceMap() will be used.
     *
     * The \a exportRegion argument can be set to a valid rectangle to indicate that only part of the layout was
     * exported.
     *
     * Similarly, the \a dpi can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
     *
     * The function will return TRUE if the output was successfully georeferenced.
     *
     * \see computeWorldFileParameters()
     */
    bool georeferenceOutput( const QString &file, QgsLayoutItemMap *referenceMap = nullptr,
                             const QRectF &exportRegion = QRectF(), double dpi = -1 ) const;

    /**
     * Compute world file parameters. Assumes the whole page containing the reference map item
     * will be exported.
     *
     * The \a dpi argument can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
     */
    void computeWorldFileParameters( double &a, double &b, double &c, double &d, double &e, double &f, double dpi = -1 ) const;

    /**
     * Computes the world file parameters for a specified \a region of the layout.
     *
     * The \a dpi argument can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
     */
    void computeWorldFileParameters( const QRectF &region, double &a, double &b, double &c, double &d, double &e, double &f, double dpi = -1 ) const;

    /**
     * Returns TRUE if the specified \a layout contains visible items which have settings
     * that require rasterization of the entire export layout in order to reproduce the desired
     * appearance.
     *
     * \see containsAdvancedEffects()
     * \since QGIS 3.20
     */
    static bool requiresRasterization( const QgsLayout *layout );

    /**
     * Returns TRUE if the specified \a layout contains visible items which have settings
     * such as opacity which will prevent these individual items from being exported as vector artwork.
     *
     * \see requiresRasterization()
     * \since QGIS 3.20
     */
    static bool containsAdvancedEffects( const QgsLayout *layout );

  protected:

    /**
     * Generates the file name for a page during export.
     *
     * Subclasses can override this method to customize page file naming.
     */
    virtual QString generateFileName( const PageExportDetails &details ) const;

  private:

    QPointer< QgsLayout > mLayout;

    void captureLabelingResults();
    QMap< QString, QgsLabelingResults * > mLabelingResults;

    mutable QString mErrorFileName;

    QImage createImage( const ImageExportSettings &settings, int page, QRectF &bounds, bool &skipPage ) const;

    /**
     * Returns the page number of the first page to be exported from the layout, skipping any pages
     * which have been excluded from export.
     */
    static int firstPageToBeExported( QgsLayout *layout );

    /**
     * Saves an image to a file, possibly using format specific options (e.g. LZW compression for tiff)
    */
    static bool saveImage( const QImage &image, const QString &imageFilename, const QString &imageFormat, QgsProject *projectForMetadata );

    /**
     * Computes a GDAL style geotransform for georeferencing a layout.
     *
     * The \a referenceMap argument specifies a map item to use for georeferencing. If left as NULLPTR, the
     * default layout QgsLayout::referenceMap() will be used.
     *
     * The \a exportRegion argument can be set to a valid rectangle to indicate that only part of the layout was
     * exported.
     *
     * Similarly, the \a dpi can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
     *
     * \see georeferenceOutput()
     */
    std::unique_ptr<double[]> computeGeoTransform( const QgsLayoutItemMap *referenceMap = nullptr, const QRectF &exportRegion = QRectF(), double dpi = -1 ) const;

    //! Write a world file
    void writeWorldFile( const QString &fileName, double a, double b, double c, double d, double e, double f ) const;

#ifndef QT_NO_PRINTER

    /**
     * Prepare a \a printer for printing a layout as a PDF, to the destination \a filePath.
     */
    static void preparePrintAsPdf( QgsLayout *layout, QPrinter &printer, const QString &filePath );

    static void preparePrint( QgsLayout *layout, QPrinter &printer, bool setFirstPageSize = false );

    /**
     * Convenience function that prepares the printer and prints.
     */
    ExportResult print( QPrinter &printer );

    /**
     * Print on a preconfigured printer
     * \param printer QPrinter destination
     * \param painter QPainter source
     * \param startNewPage set to TRUE to begin the print on a new page
     * \param dpi set to a value > 0 to manually override the layout's default dpi
     * \param rasterize set to TRUE to force print as a raster image
     */
    ExportResult printPrivate( QPrinter &printer, QPainter &painter, bool startNewPage = false, double dpi = -1, bool rasterize = false );

    static void updatePrinterPageSize( QgsLayout *layout, QPrinter &printer, int page );
#endif

    ExportResult renderToLayeredSvg( const SvgExportSettings &settings, double width, double height, int page, const QRectF &bounds,
                                     const QString &filename, unsigned int svgLayerId, const QString &layerName,
                                     QDomDocument &svg, QDomNode &svgDocRoot, bool includeMetadata ) const;

    void appendMetadataToSvg( QDomDocument &svg ) const;

    bool georeferenceOutputPrivate( const QString &file, QgsLayoutItemMap *referenceMap = nullptr,
                                    const QRectF &exportRegion = QRectF(), double dpi = -1, bool includeGeoreference = true, bool includeMetadata = false ) const;

    ExportResult handleLayeredExport( const QList<QGraphicsItem *> &items, const std::function<QgsLayoutExporter::ExportResult( unsigned int layerId, const QgsLayoutItem::ExportLayerDetail &layerDetails )> &exportFunc );

    static QgsVectorSimplifyMethod createExportSimplifyMethod();
    friend class TestQgsLayout;
    friend class TestQgsLayoutExporter;

};

#endif //QGSLAYOUTEXPORTER_H



