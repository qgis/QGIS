/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorfilewritertask.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsVectorFileWriterTask : QgsTask
{
%Docstring
QgsTask task which performs a QgsVectorFileWriter layer saving operation as a background
task. This can be used to save a vector layer out to a file without blocking the
QGIS interface.

.. seealso:: :py:class:`QgsVectorLayerExporterTask`

.. seealso:: :py:class:`QgsRasterFileWriterTask`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorfilewritertask.h"
%End
  public:

    QgsVectorFileWriterTask( QgsVectorLayer *layer,
                             const QString &fileName,
                             const QgsVectorFileWriter::SaveVectorOptions &options );
%Docstring
Constructor for QgsVectorFileWriterTask. Takes a source ``layer``, destination ``fileName``
and save ``options``.
%End

    virtual void cancel();


  signals:

    void writeComplete( const QString &newFilename );
%Docstring
Emitted when writing the layer is successfully completed. The ``newFilename``
parameter indicates the file path for the written file.

.. note::

   this signal is deprecated in favor of completed().
%End


    void errorOccurred( int error, const QString &errorMessage );
%Docstring
Emitted when an error occurs which prevented the file being written (or if
the task is canceled). The writing ``error`` and ``errorMessage`` will be reported.
%End

  protected:

    virtual bool run();

    virtual void finished( bool result );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorfilewritertask.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
