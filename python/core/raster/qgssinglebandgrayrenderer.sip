/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgssinglebandgrayrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSingleBandGrayRenderer: QgsRasterRenderer
{
%Docstring
 Raster renderer pipe for single band gray.
%End

%TypeHeaderCode
#include "qgssinglebandgrayrenderer.h"
%End
  public:
    enum Gradient
    {
      BlackToWhite,
      WhiteToBlack
    };

    QgsSingleBandGrayRenderer( QgsRasterInterface *input, int grayBand );


    virtual QgsSingleBandGrayRenderer *clone() const /Factory/;

%Docstring
QgsSingleBandGrayRenderer cannot be copied. Use clone() instead.
 :rtype: QgsSingleBandGrayRenderer
%End

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) /Factory/;
%Docstring
 :rtype: QgsRasterRenderer
%End

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) /Factory/;


    int grayBand() const;
%Docstring
 :rtype: int
%End
    void setGrayBand( int band );
    const QgsContrastEnhancement *contrastEnhancement() const;
%Docstring
 :rtype: QgsContrastEnhancement
%End
    void setContrastEnhancement( QgsContrastEnhancement *ce /Transfer/ );
%Docstring
Takes ownership
%End

    void setGradient( Gradient gradient );
    Gradient gradient() const;
%Docstring
 :rtype: Gradient
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems /Out/ ) const;


    virtual QList<int> usesBands() const;


  private:
    QgsSingleBandGrayRenderer( const QgsSingleBandGrayRenderer & );
    const QgsSingleBandGrayRenderer &operator=( const QgsSingleBandGrayRenderer & );
%Docstring
 :rtype: QgsSingleBandGrayRenderer
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgssinglebandgrayrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
