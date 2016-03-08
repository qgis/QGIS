class QgsMultiBandColorRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgsmultibandcolorrenderer.h"
%End
  public:
    QgsMultiBandColorRenderer( QgsRasterDataProvider* provider, int redBand, int greenBand, int blueBand,
                               QgsContrastEnhancement* redEnhancement = 0, QgsContrastEnhancement* greenEnhancement = 0,
                               QgsContrastEnhancement* blueEnhancement = 0 );
    ~QgsMultiBandColorRenderer();
    virtual QgsMultiBandColorRenderer * clone() const /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    QgsRasterBlock* block( int bandNo, const QgsRectangle & extent, int width, int height ) /Factory/;

    int redBand() const;
    void setRedBand( int band );
    int greenBand() const;
    void setGreenBand( int band );
    int blueBand() const;
    void setBlueBand( int band );

    const QgsContrastEnhancement* redContrastEnhancement() const;
    /** Takes ownership*/
    void setRedContrastEnhancement( QgsContrastEnhancement* ce /Transfer/ );

    const QgsContrastEnhancement* greenContrastEnhancement() const;
    /** Takes ownership*/
    void setGreenContrastEnhancement( QgsContrastEnhancement* ce /Transfer/ );

    const QgsContrastEnhancement* blueContrastEnhancement() const;
    /** Takes ownership*/
    void setBlueContrastEnhancement( QgsContrastEnhancement* ce /Transfer/ );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    QList<int> usesBands() const;

  private:

    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer& );
    const QgsMultiBandColorRenderer& operator=( const QgsMultiBandColorRenderer& );
};
