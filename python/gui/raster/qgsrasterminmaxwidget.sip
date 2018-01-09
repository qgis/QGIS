/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterminmaxwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterMinMaxWidget: QWidget
{

%TypeHeaderCode
#include "qgsrasterminmaxwidget.h"
%End
  public:

    QgsRasterMinMaxWidget( QgsRasterLayer *layer, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsRasterMinMaxWidget
%End

    void setExtent( const QgsRectangle &extent );
%Docstring
Sets the extent to use for minimum and maximum value calculation.

:param extent: extent in raster layer's CRS

.. note::

   if a map canvas is set using setMapCanvas(), its extent will take
precedence over any extent set using this method.
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the widget. This allows the widget to retrieve the current
map extent from the canvas. If a canvas is set it will take precedence over any extent
set from calling setExtent().

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

    void setBands( const QList<int> &bands );

    QgsRectangle extent();
%Docstring
Return the extent selected by the user.
Either an empty extent for 'full' or the current visible extent.
%End

    int sampleSize();
%Docstring
Return the selected sample size.
%End

    void setFromMinMaxOrigin( const QgsRasterMinMaxOrigin & );
%Docstring
 Set the "source" of min/max values.
%End

    QgsRasterMinMaxOrigin minMaxOrigin();
%Docstring
 Return a QgsRasterMinMaxOrigin object with the widget values.
%End

    void hideUpdatedExtent();
%Docstring
Hide updated extent choice
%End

    void doComputations();
%Docstring
Load programmatically with current values
%End

    void userHasSetManualMinMaxValues();
%Docstring
Uncheck cumulative cut, min/max, std-dev radio buttons
%End

    bool isCollapsed() const;
%Docstring
Return if the widget is collaped.
%End

    void setCollapsed( bool b );
%Docstring
Set collapsed state of widget
%End

  signals:

    void widgetChanged();
%Docstring
Emitted when something on the widget has changed.
All widgets will fire this event to notify of an internal change.
%End

    void load( int bandNo, double min, double max );
%Docstring
signal emitted when new min/max values are computed from statistics.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterminmaxwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
