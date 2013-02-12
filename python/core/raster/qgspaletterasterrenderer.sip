class QgsPalettedRasterRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgspalettedrasterrenderer.h"
%End
  public:
    /**Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber, QColor* colorArray /Array,Transfer/, int nColors /ArraySize/ );
    ~QgsPalettedRasterRenderer();
    QgsRasterInterface * clone() /Factory/;
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    QgsRasterBlock * block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;

    /**Returns number of colors*/
    int nColors() const;
    /**Returns copy of color array (caller takes ownership)*/
    QColor* colors() const /Factory/;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
 };
