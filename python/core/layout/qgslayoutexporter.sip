/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutexporter.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutExporter
{
%Docstring
 Handles rendering and exports of layouts to various formats.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutexporter.h"
%End
  public:

    struct PageExportDetails
    {
      QString directory;
%Docstring
Target folder
%End

      QString baseName;
%Docstring
Base part of filename (i.e. file name without extension or '.')
%End

      QString extension;
%Docstring
File suffix/extension (without the leading '.')
%End

      int page;
%Docstring
Page number, where 0 = first page.
%End
    };

    QgsLayoutExporter( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutExporter, for the specified ``layout``.
%End

    virtual ~QgsLayoutExporter();

    QgsLayout *layout() const;
%Docstring
 Returns the layout linked to this exporter.
 :rtype: QgsLayout
%End

    void renderPage( QPainter *painter, int page ) const;
%Docstring
Renders a full page to a destination ``painter``.

The ``page`` argument specifies the page number to render. Page numbers
are 0 based, such that the first page in a layout is page 0.

.. seealso:: :py:func:`renderRect()`
%End

    QImage renderPageToImage( int page, QSize imageSize = QSize(), double dpi = 0 ) const;
%Docstring
 Renders a full page to an image.

 The ``page`` argument specifies the page number to render. Page numbers
 are 0 based, such that the first page in a layout is page 0.

 The optional ``imageSize`` parameter can specify the target image size, in pixels.
 It is the caller's responsibility to ensure that the ratio of the target image size
 matches the ratio of the corresponding layout page size.

 The ``dpi`` parameter is an optional dpi override. Set to 0 to use the default layout print
 resolution. This parameter has no effect if ``imageSize`` is specified.

 Returns the rendered image, or a null QImage if the image does not fit into available memory.

.. seealso:: :py:func:`renderPage()`
.. seealso:: :py:func:`renderRegionToImage()`
 :rtype: QImage
%End

    void renderRegion( QPainter *painter, const QRectF &region ) const;
%Docstring
Renders a ``region`` from the layout to a ``painter``. This method can be used
to render sections of pages rather than full pages.

.. seealso:: :py:func:`renderPage()`
.. seealso:: :py:func:`renderRegionToImage()`
%End

    QImage renderRegionToImage( const QRectF &region, QSize imageSize = QSize(), double dpi = 0 ) const;
%Docstring
 Renders a ``region`` of the layout to an image. This method can be used to render
 sections of pages rather than full pages.

 The optional ``imageSize`` parameter can specify the target image size, in pixels.
 It is the caller's responsibility to ensure that the ratio of the target image size
 matches the ratio of the specified region of the layout.

 The ``dpi`` parameter is an optional dpi override. Set to 0 to use the default layout print
 resolution. This parameter has no effect if ``imageSize`` is specified.

 Returns the rendered image, or a null QImage if the image does not fit into available memory.

.. seealso:: :py:func:`renderRegion()`
.. seealso:: :py:func:`renderPageToImage()`
 :rtype: QImage
%End


    enum ExportResult
    {
      Success,
      MemoryError,
      FileError,
      PrintError,
    };

    struct ImageExportSettings
    {
      ImageExportSettings();
%Docstring
Constructor for ImageExportSettings
%End

      double dpi;
%Docstring
Resolution to export layout at. If dpi <= 0 the default layout dpi will be used.
%End

      QSize imageSize;
%Docstring
 Manual size in pixels for output image. If imageSize is not
 set then it will be automatically calculated based on the
 output dpi and layout size.

 If cropToContents is true then imageSize has no effect.

 Be careful when specifying manual sizes if pages in the layout
 have differing sizes! It's likely not going to give a reasonable
 output in this case, and the automatic dpi-based image size should be
 used instead.
%End

      bool cropToContents;
%Docstring
 Set to true if image should be cropped so only parts of the layout
 containing items are exported.
%End

      QgsMargins cropMargins;
%Docstring
 Crop to content margins, in pixels. These margins will be added
 to the bounds of the exported layout if cropToContents is true.
%End

      QList< int > pages;
%Docstring
 List of specific pages to export, or an empty list to
 export all pages.

 Page numbers are 0 index based, so the first page in the
 layout corresponds to page 0.
%End

      bool generateWorldFile;
%Docstring
 Set to true to generate an external world file alongside
 exported images.
%End

      QgsLayoutContext::Flags flags;
%Docstring
 Layout context flags, which control how the export will be created.
%End

    };

    ExportResult exportToImage( const QString &filePath, const QgsLayoutExporter::ImageExportSettings &settings );
%Docstring
 Exports the layout to the a ``filePath``, using the specified export ``settings``.

 If the layout is a multi-page layout, then filenames for each page will automatically
 be generated by appending "_1", "_2", etc to the image file's base name.

 Returns a result code indicating whether the export was successful or an
 error was encountered. If an error code is returned, errorFile() can be called
 to determine the filename for the export which encountered the error.
 :rtype: ExportResult
%End

    struct PdfExportSettings
    {
      PdfExportSettings();
%Docstring
Constructor for PdfExportSettings
%End

      double dpi;
%Docstring
Resolution to export layout at. If dpi <= 0 the default layout dpi will be used.
%End

      bool rasterizeWholeImage;
%Docstring
Set to true to force whole layout to be rasterized while exporting
%End

      QgsLayoutContext::Flags flags;
%Docstring
 Layout context flags, which control how the export will be created.
%End

    };

    ExportResult exportToPdf( const QString &filePath, const QgsLayoutExporter::PdfExportSettings &settings );
%Docstring
 Exports the layout as a PDF to the a ``filePath``, using the specified export ``settings``.

 Returns a result code indicating whether the export was successful or an
 error was encountered.
 :rtype: ExportResult
%End

    QString errorFile() const;
%Docstring
 Returns the file name corresponding to the last error encountered during
 an export.
 :rtype: str
%End

    bool georeferenceOutput( const QString &file, QgsLayoutItemMap *referenceMap = 0,
                             const QRectF &exportRegion = QRectF(), double dpi = -1 ) const;
%Docstring
 Georeferences a ``file`` (image of PDF) exported from the layout.

 The ``referenceMap`` argument specifies a map item to use for georeferencing. If left as None, the
 default layout QgsLayout.referenceMap() will be used.

 The ``exportRegion`` argument can be set to a valid rectangle to indicate that only part of the layout was
 exported.

 Similarly, the ``dpi`` can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.

 The function will return true if the output was successfully georeferenced.

.. seealso:: :py:func:`computeGeoTransform()`
 :rtype: bool
%End

    void computeWorldFileParameters( double &a, double &b, double &c, double &d, double &e, double &f, double dpi = -1 ) const;
%Docstring
 Compute world file parameters. Assumes the whole page containing the reference map item
 will be exported.

 The ``dpi`` argument can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
%End

    void computeWorldFileParameters( const QRectF &region, double &a, double &b, double &c, double &d, double &e, double &f, double dpi = -1 ) const;
%Docstring
 Computes the world file parameters for a specified ``region`` of the layout.

 The ``dpi`` argument can be set to the actual DPI of exported file, or left as -1 to use the layout's default DPI.
%End

  protected:

    virtual QString generateFileName( const PageExportDetails &details ) const;
%Docstring
 Generates the file name for a page during export.

 Subclasses can override this method to customise page file naming.
 :rtype: str
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutexporter.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
