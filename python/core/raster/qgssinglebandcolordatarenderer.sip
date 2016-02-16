class QgsSingleBandColorDataRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandcolordatarenderer.h"
%End
  public:
    QgsSingleBandColorDataRenderer( QgsRasterDataProvider* provider, int band );
    ~QgsSingleBandColorDataRenderer();
    virtual QgsSingleBandColorDataRenderer * clone() const /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    bool setInput( QgsRasterInterface* input );

    QgsRasterBlock* block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    QList<int> usesBands() const;

  private:

    QgsSingleBandColorDataRenderer( const QgsSingleBandColorDataRenderer& );
    const QgsSingleBandColorDataRenderer& operator=( const QgsSingleBandColorDataRenderer& );
};
