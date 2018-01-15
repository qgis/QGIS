/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterrenderer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsRasterRenderer : QgsRasterInterface
{
%Docstring
Raster renderer pipe that applies colors to a raster.
%End

%TypeHeaderCode
#include "qgsrasterrenderer.h"
%End
  public:

    static const QRgb NODATA_COLOR;

    QgsRasterRenderer( QgsRasterInterface *input = 0, const QString &type = QString() );
%Docstring
Constructor for QgsRasterRenderer.
%End
    ~QgsRasterRenderer();


    virtual QgsRasterRenderer *clone() const = 0 /Factory/;

%Docstring
QgsRasterRenderer cannot be copied. Use clone() instead.
%End

    virtual int bandCount() const;


    virtual Qgis::DataType dataType( int bandNo ) const;


    virtual QString type() const;

    virtual bool setInput( QgsRasterInterface *input );


     virtual QgsRasterBlock *block( int bandNo,
                           const QgsRectangle &extent,
                           int width,
                           int height,
                           QgsRasterBlockFeedback *feedback = 0 ) = 0 /Factory/;

    bool usesTransparency() const;

    void setOpacity( double opacity );
%Docstring
Sets the ``opacity`` for the renderer, where ``opacity`` is a value between 0 (totally transparent)
and 1.0 (fully opaque).

.. seealso:: :py:func:`opacity`
%End

    double opacity() const;
%Docstring
Returns the opacity for the renderer, where opacity is a value between 0 (totally transparent)
and 1.0 (fully opaque).

.. seealso:: :py:func:`setOpacity`
%End

    void setRasterTransparency( QgsRasterTransparency *t /Transfer/ );
    const QgsRasterTransparency *rasterTransparency() const;

    void setAlphaBand( int band );
    int alphaBand() const;

    virtual void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems /Out/ ) const;
%Docstring
Get symbology items if provided by renderer
%End

    virtual void readXml( const QDomElement &rendererElem );

%Docstring
Sets base class members from xml. Usually called from create() methods of subclasses
%End

    void copyCommonProperties( const QgsRasterRenderer *other, bool copyMinMaxOrigin = true );
%Docstring
Copies common properties like opacity / transparency data from other renderer.
Useful when cloning renderers.

.. versionadded:: 2.16
%End

    virtual QList<int> usesBands() const;
%Docstring
Returns a list of band numbers used by the renderer
%End

    const QgsRasterMinMaxOrigin &minMaxOrigin() const;
%Docstring
Returns const reference to origin of min/max values
%End

    void setMinMaxOrigin( const QgsRasterMinMaxOrigin &origin );
%Docstring
Sets origin of min/max values
%End

  protected:

    void _writeXml( QDomDocument &doc, QDomElement &rasterRendererElem ) const;
%Docstring
Write upper class info into rasterrenderer element (called by writeXml method of subclasses)
%End





  private:
    QgsRasterRenderer( const QgsRasterRenderer & );
    const QgsRasterRenderer &operator=( const QgsRasterRenderer & );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterrenderer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
