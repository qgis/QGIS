/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrendererrasterpropertieswidget.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRendererRasterPropertiesWidget : QgsMapLayerConfigWidget
{

%TypeHeaderCode
#include "qgsrendererrasterpropertieswidget.h"
%End
  public:

    QgsRendererRasterPropertiesWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = 0 );
%Docstring
A widget to hold the renderer properties for a raster layer.

:param layer: The raster layer to style
:param canvas: The canvas object used to calculate the max and min values from the extent.
:param parent: Parent object
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
map scale and other properties from the canvas.

:param canvas: map canvas

.. versionadded:: 2.12
%End

    QgsRasterRendererWidget *currentRenderWidget();
%Docstring
Returns the active render widget. Can be ``None``.
%End

  public slots:
    void rendererChanged();
%Docstring
called when user changes renderer type
%End

    virtual void apply();

%Docstring
Apply the changes from the dialog to the layer.
%End

    void syncToLayer( QgsRasterLayer *layer );
%Docstring
Sync the widget to the given layer.

:param layer: The layer to use for the widget
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrendererrasterpropertieswidget.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
