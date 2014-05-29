class QgsSingleBandColorDataRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandcolordatarenderer.h"
%End
  public:
    QgsSingleBandColorDataRenderer( QgsRasterDataProvider* provider, int band );
    ~QgsSingleBandColorDataRenderer();
    QgsRasterInterface * clone() const /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    bool setInput( QgsRasterInterface* input );

    QgsRasterBlock* block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    QList<int> usesBands() const;
};
