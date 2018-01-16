/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgshillshaderenderer.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsHillshadeRenderer : QgsRasterRenderer
{
%Docstring
 A renderer for generating live hillshade models.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgshillshaderenderer.h"
%End
  public:

    QgsHillshadeRenderer( QgsRasterInterface *input, int band, double lightAzimuth, double lightAltitude );
%Docstring
 A renderer for generating live hillshade models.

:param input: The input raster interface
:param band: The band in the raster to use
:param lightAzimuth: The azimuth of the light source
:param lightAltitude: The altitude of the light source
%End

    virtual QgsHillshadeRenderer *clone() const /Factory/;


    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) /Factory/;
%Docstring
 Factory method to create a new renderer

:param elem: A DOM element to create the renderer from.
:param input: The raster input interface.

:return: A new QgsHillshadeRenderer.
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 );


    virtual QList<int> usesBands() const;


    int band() const;
%Docstring
Returns the band used by the renderer
%End

    void setBand( int bandNo );
%Docstring
Sets the band used by the renderer.

.. seealso:: :py:func:`band`
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
Returns true if the renderer is using multi-directional hillshading.

.. seealso:: :py:func:`setMultiDirectional`
%End

    void setAzimuth( double azimuth );
%Docstring
 Set the azimuth of the light source.

:param azimuth: The azimuth of the light source, between 0 and 360.0

.. seealso:: :py:func:`azimuth`
%End

    void setAltitude( double altitude );
%Docstring
 Set the altitude of the light source

:param altitude: the altitude

.. seealso:: :py:func:`altitude`
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
 * src/core/raster/qgshillshaderenderer.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
