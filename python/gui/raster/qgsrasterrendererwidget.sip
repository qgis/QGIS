/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterrendererwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterRendererWidget: QWidget
{

%TypeHeaderCode
#include "qgsrasterrendererwidget.h"
%End
  public:

    QgsRasterRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent );

    virtual QgsRasterRenderer *renderer() = 0 /Factory/;

    void setRasterLayer( QgsRasterLayer *layer );
    const QgsRasterLayer *rasterLayer() const;

    virtual void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the widget. This allows the widget to retrieve the current
map extent and other properties from the canvas.

:param canvas: map canvas

.. seealso:: :py:func:`mapCanvas`

.. versionadded:: 2.16
%End

    QgsMapCanvas *mapCanvas();
%Docstring
Returns the map canvas associated with the widget.

.. seealso:: :py:func:`setMapCanvas`

.. seealso:: :py:func:`canvasExtent`

.. versionadded:: 2.16
%End

    virtual QString min( int index = 0 );
    virtual QString max( int index = 0 );
    virtual void setMin( const QString &value, int index = 0 );
    virtual void setMax( const QString &value, int index = 0 );
    virtual QString stdDev();
    virtual void setStdDev( const QString &value );
    virtual int selectedBand( int index = 0 );

    virtual void doComputations();
%Docstring
Load programmatically with current values
%End

    virtual QgsRasterMinMaxWidget *minMaxWidget();
%Docstring
Return min/max widget when it exists.
%End

  signals:

    void widgetChanged();
%Docstring
Emitted when something on the widget has changed.
All widgets will fire this event to notify of an internal change.
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterrendererwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
