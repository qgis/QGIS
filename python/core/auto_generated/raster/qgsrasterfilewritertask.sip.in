/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterfilewritertask.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRasterFileWriterTask : QgsTask
{
%Docstring
QgsTask task which performs a QgsRasterFileWriter layer saving operation as a background
task. This can be used to save a raster layer out to a file without blocking the
QGIS interface.

.. seealso:: :py:class:`QgsVectorFileWriterTask`

.. seealso:: :py:class:`QgsVectorFileExporterTask`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsrasterfilewritertask.h"
%End
  public:

 QgsRasterFileWriterTask( const QgsRasterFileWriter &writer, QgsRasterPipe *pipe /Transfer/,
        int columns, int rows,
        const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs ) /Deprecated/;
%Docstring
Constructor for QgsRasterFileWriterTask. Takes a source ``writer``,
``columns``, ``rows``, ``outputExtent`` and destination ``crs``.
Ownership of the ``pipe`` is transferred to the writer task, and will
be deleted when the task completes.

.. deprecated:: since QGIS 3.8, use version with transformContext instead
%End


    QgsRasterFileWriterTask( const QgsRasterFileWriter &writer, QgsRasterPipe *pipe /Transfer/,
                             int columns, int rows,
                             const QgsRectangle &outputExtent,
                             const QgsCoordinateReferenceSystem &crs,
                             const QgsCoordinateTransformContext &transformContext
                           );
%Docstring
Constructor for QgsRasterFileWriterTask. Takes a source ``writer``,
``columns``, ``rows``, ``outputExtent``, destination ``crs`` and
coordinate ``transformContext`` .
Ownership of the ``pipe`` is transferred to the writer task, and will
be deleted when the task completes.
%End

    virtual void cancel();


  signals:

    void writeComplete( const QString &outputUrl );
%Docstring
Emitted when writing the layer is successfully completed. The ``outputUrl``
parameter indicates the file path for the written file(s).
%End

    void errorOccurred( int error );
%Docstring
Emitted when an error occurs which prevented the file being written (or if
the task is canceled). The writing ``error`` will be reported.
%End

  protected:

    virtual bool run();

    virtual void finished( bool result );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterfilewritertask.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
