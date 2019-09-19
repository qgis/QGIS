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
%End

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) /Factory/;

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) /Factory/;


    int redBand() const;
    void setRedBand( int band );
    int greenBand() const;
    void setGreenBand( int band );
    int blueBand() const;
    void setBlueBand( int band );

    const QgsContrastEnhancement *redContrastEnhancement() const;
    void setRedContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    const QgsContrastEnhancement *greenContrastEnhancement() const;
    void setGreenContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    const QgsContrastEnhancement *blueContrastEnhancement() const;
    void setBlueContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual QList<int> usesBands() const;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;


  private:
    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer & );
    const QgsMultiBandColorRenderer &operator=( const QgsMultiBandColorRenderer & );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsmultibandcolorrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
