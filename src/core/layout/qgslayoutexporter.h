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
#include "qgslayoutcontext.h"
#include <QPointer>
#include <QSize>
#include <QRectF>

class QgsLayout;
class QPainter;
class QgsLayoutItemMap;

/**
 * \ingroup core
 * \class QgsLayoutExporter
 * \brief Handles rendering and exports of layouts to various formats.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutExporter
{

  public:

    /**
     * Constructor for QgsLayoutExporter, for the specified \a layout.
     */
    QgsLayoutExporter( QgsLayout *layout );

    /**
     * Renders a full page to a destination \a painter.
     *
     * The \a page argument specifies the page number to render. Page numbers
     * are 0 based, such that the first page in a layout is page 0.
     *
     * \see renderRect()
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
     * The \a dpi parameter is an optional dpi override. Set to 0 to use the default layout print
     * resolution. This parameter has no effect if \a imageSize is specified.
     *
     * Returns the rendered image, or a null QImage if the image does not fit into available memory.
     *
     * \see renderPage()
     * \see renderRegionToImage()
     */
    QImage renderPageToImage( int page, QSize imageSize = QSize(), double dpi = 0 ) const;

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
     * The \a dpi parameter is an optional dpi override. Set to 0 to use the default layout print
     * resolution. This parameter has no effect if \a imageSize is specified.
     *
     * Returns the rendered image, or a null QImage if the image does not fit into available memory.
     *
     * \see renderRegion()
     * \see renderPageToImage()
     */
    QImage renderRegionToImage( const QRectF &region, QSize imageSize = QSize(), double dpi = 0 ) const;


    //! Result codes for exporting layouts
    enum ExportResult
    {
      Success, //!< Export was successful
      MemoryError, //!< Unable to allocate memory required to export
      FileError, //!< Could not write to destination file, likely due to a lock held by another application
    };

    //! Contains settings relating to exporting layouts to raster images
    struct ImageExportSettings
    {
      //! Constructor for ImageExportSettings
      ImageExportSettings()
        : flags( QgsLayoutContext::FlagAntialiasing | QgsLayoutContext::FlagUseAdvancedEffects )
      {}

      //! Resolution to export layout at
      double dpi;

      /**
       * Manual size in pixels for output image. If imageSize is not
       * set then it will be automatically calculated based on the
       * output dpi and layout size.
       *
       * If cropToContents is true then imageSize has no effect.
       *
       * Be careful when specifying manual sizes if pages in the layout
       * have differing sizes! It's likely not going to give a reasonable
       * output in this case, and the automatic dpi-based image size should be
       * used instead.
       */
      QSize imageSize;

      /**
       * Set to true if image should be cropped so only parts of the layout
       * containing items are exported.
       */
      bool cropToContents = false;

      /**
       * Crop to content margins, in pixels. These margins will be added
       * to the bounds of the exported layout if cropToContents is true.
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
       * Set to true to generate an external world file alonside
       * exported images.
       */
      bool generateWorldFile = false;

      /**
       * Layout context flags, which control how the export will be created.
       */
      QgsLayoutContext::Flags flags = 0;

    };

    /**
     * Exports the layout to the a \a filePath, using the specified export \a settings.
     *
     * If the layout is a multi-page layout, then filenames for each page will automatically
     * be generated by appending "_1", "_2", etc to the image file's base name.
     *
     * Returns a result code indicating whether the export was successful or an
     * error was encountered. If an error code is returned, errorFile() can be called
     * to determine the filename for the export which encountered the error.
     */
    ExportResult exportToImage( const QString &filePath, const ImageExportSettings &settings );

    /**
     * Returns the file name corresponding to the last error encountered during
     * an export.
     */
    QString errorFile() const { return mErrorFileName; }

    /**
     * Georeferences a \a file (image of PDF) exported from the layout.
     *
     * The \a referenceMap argument specifies a map item to use for georeferencing. If left as nullptr, the
     * default layout QgsLayout::referenceMap() will be used.
     *
     * The \a exportRegion argument can be set to a valid rectangle to indicate that only part of the layout was
     * exported.
     *
     * Similarly, the \a dpi can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
     *
     * The function will return true if the output was successfully georeferenced.
     *
     * \see computeGeoTransform()
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


  private:

    QPointer< QgsLayout > mLayout;

    QString mErrorFileName;

    QImage createImage( const ImageExportSettings &settings, int page, QRectF &bounds, bool &skipPage ) const;

    QString generateFileName( const QString &path, const QString &baseName, const QString &suffix, int page ) const;

    /**
     * Saves an image to a file, possibly using format specific options (e.g. LZW compression for tiff)
    */
    static bool saveImage( const QImage &image, const QString &imageFilename, const QString &imageFormat );

    /**
     * Computes a GDAL style geotransform for georeferencing a layout.
     *
     * The \a referenceMap argument specifies a map item to use for georeferencing. If left as nullptr, the
     * default layout QgsLayout::referenceMap() will be used.
     *
     * The \a exportRegion argument can be set to a valid rectangle to indicate that only part of the layout was
     * exported.
     *
     * Similarly, the \a dpi can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
     *
     * \see georeferenceOutput()
     */
    double *computeGeoTransform( const QgsLayoutItemMap *referenceMap = nullptr, const QRectF &exportRegion = QRectF(), double dpi = -1 ) const;

    //! Write a world file
    void writeWorldFile( const QString &fileName, double a, double b, double c, double d, double e, double f ) const;


    friend class TestQgsLayout;

};

#endif //QGSLAYOUTEXPORTER_H



