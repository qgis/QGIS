class QgsSingleBandGrayRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandgrayrenderer.h"
%End
  public:
    QgsSingleBandGrayRenderer( QgsRasterDataProvider* provider, int grayBand );
    ~QgsSingleBandGrayRenderer();
    QgsRasterInterface * clone() /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    QgsRasterBlock * block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;

    int grayBand() const;
    void setGrayBand( int band );
    const QgsContrastEnhancement* contrastEnhancement() const;
    /**Takes ownership*/
    void setContrastEnhancement( QgsContrastEnhancement* ce /Transfer/ );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
};
