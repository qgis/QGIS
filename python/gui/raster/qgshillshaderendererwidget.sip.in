/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgshillshaderendererwidget.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsHillshadeRendererWidget: QgsRasterRendererWidget
{
%Docstring
 Renderer widget for the hill shade renderer.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgshillshaderendererwidget.h"
%End
  public:

    QgsHillshadeRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );
%Docstring
 Renderer widget for the hill shade renderer.

:param layer: The layer attached for this widget.
:param extent: The current extent.
%End

    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) /Factory/;
%Docstring
Factory method to create the renderer for this type.
%End

    virtual QgsRasterRenderer *renderer();

%Docstring
 The renderer for the widget.

:return: A new renderer for the the config in the widget
%End

    void setFromRenderer( const QgsRasterRenderer *renderer );
%Docstring
 Set the widget state from the given renderer.

:param renderer: The renderer to take the state from.
%End

    double azimuth() const;
%Docstring
Returns the direction of the light over the raster between 0-360.

.. seealso:: :py:func:`setAzimuth`
%End

    double altitude()  const;
%Docstring
Returns the angle of the light source over the raster.

.. seealso:: :py:func:`setAltitude`
%End

    double zFactor()  const;
%Docstring
Returns the Z scaling factor.

.. seealso:: :py:func:`setZFactor`
%End

    bool multiDirectional() const;
%Docstring
Returns true if the renderer should use the multi-directional hillshade algorithm.

.. seealso:: :py:func:`setMultiDirectional`
%End

  public slots:

    void setAltitude( double altitude );
%Docstring
 Set the altitude of the light source

:param altitude: the altitude

.. seealso:: :py:func:`altitude`
%End

    void setAzimuth( double azimuth );
%Docstring
 Set the azimuth of the light source.

:param azimuth: The azimuth of the light source, between 0 and 360.0

.. seealso:: :py:func:`azimuth`
%End

    void setZFactor( double zfactor );
%Docstring
 Set the Z scaling factor of the result image.

:param zfactor: The z factor

.. seealso:: :py:func:`zFactor`
%End

    void setMultiDirectional( bool isMultiDirectional );
%Docstring
Sets whether to render using a multi-directional hillshade algorithm.

:param isMultiDirectional: set to true to use multi directional rendering

.. seealso:: :py:func:`multiDirectional`
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgshillshaderendererwidget.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
