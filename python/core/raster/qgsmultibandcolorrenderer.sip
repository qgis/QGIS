/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsmultibandcolorrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMultiBandColorRenderer: QgsRasterRenderer
{
%Docstring
Renderer for multiband images with the color components
%End

%TypeHeaderCode
#include "qgsmultibandcolorrenderer.h"
%End
  public:
    QgsMultiBandColorRenderer( QgsRasterInterface *input, int redBand, int greenBand, int blueBand,
                               QgsContrastEnhancement *redEnhancement = 0, QgsContrastEnhancement *greenEnhancement = 0,
                               QgsContrastEnhancement *blueEnhancement = 0 );
    ~QgsMultiBandColorRenderer();


    virtual QgsMultiBandColorRenderer *clone() const /Factory/;

%Docstring
QgsMultiBandColorRenderer cannot be copied. Use clone() instead.

:rtype: QgsMultiBandColorRenderer
%End

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) /Factory/;
%Docstring

:rtype: QgsRasterRenderer
%End

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) /Factory/;


    int redBand() const;
%Docstring

:rtype: int
%End
    void setRedBand( int band );
    int greenBand() const;
%Docstring

:rtype: int
%End
    void setGreenBand( int band );
    int blueBand() const;
%Docstring

:rtype: int
%End
    void setBlueBand( int band );

    const QgsContrastEnhancement *redContrastEnhancement() const;
%Docstring

:rtype: QgsContrastEnhancement
%End
    void setRedContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    const QgsContrastEnhancement *greenContrastEnhancement() const;
%Docstring

:rtype: QgsContrastEnhancement
%End
    void setGreenContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    const QgsContrastEnhancement *blueContrastEnhancement() const;
%Docstring

:rtype: QgsContrastEnhancement
%End
    void setBlueContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual QList<int> usesBands() const;


  private:
    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer & );
    const QgsMultiBandColorRenderer &operator=( const QgsMultiBandColorRenderer & );
%Docstring

:rtype: QgsMultiBandColorRenderer
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsmultibandcolorrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
