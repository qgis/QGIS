class QgsSingleBandPseudoColorRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandpseudocolorrenderer.h"
%End
  public:
    /**Note: takes ownership of QgsRasterShader*/
    QgsSingleBandPseudoColorRenderer( QgsRasterDataProvider* provider, int band, QgsRasterShader* shader /Transfer/ );
    ~QgsSingleBandPseudoColorRenderer();
    QgsRasterInterface * clone() /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    /**Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader /Transfer/ );
    QgsRasterShader* shader();

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
};
