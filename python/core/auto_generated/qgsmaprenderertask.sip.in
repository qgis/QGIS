/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprenderertask.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapRendererTask : QgsTask
{
%Docstring
QgsTask task which draws a map to an image file or a painter as a background
task. This can be used to draw maps without blocking the QGIS interface.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsmaprenderertask.h"
%End
  public:

    enum ErrorType
    {
      ImageAllocationFail,
      ImageSaveFail,
      ImageUnsupportedFormat
    };

    QgsMapRendererTask( const QgsMapSettings &ms,
                        const QString &fileName,
                        const QString &fileFormat = QString( "PNG" ),
                        bool forceRaster = false );
%Docstring
Constructor for QgsMapRendererTask to render a map to an image file.
%End

    QgsMapRendererTask( const QgsMapSettings &ms,
                        QPainter *p );
%Docstring
Constructor for QgsMapRendererTask to render a map to a QPainter object.
%End

    void addAnnotations( QList< QgsAnnotation * > annotations );
%Docstring
Adds ``annotations`` to be rendered on the map.
%End

    void addDecorations( const QList<QgsMapDecoration *> &decorations );
%Docstring
Adds ``decorations`` to be rendered on the map.
%End

    void setSaveWorldFile( bool save );
%Docstring
Sets whether a world file will be created alongside an image file.
%End

    virtual void cancel();


  signals:

    void renderingComplete();
%Docstring
Emitted when the map rendering is successfully completed.
%End

    void errorOccurred( int error );
%Docstring
Emitted when map rendering failed.
%End

  protected:

    virtual bool run();

    virtual void finished( bool result );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprenderertask.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
