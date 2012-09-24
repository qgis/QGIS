class QgsSingleBandPseudoColorRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandpseudocolorrenderer.h"
%End
  public:
    /**Note: takes ownership of QgsRasterShader*/
    QgsSingleBandPseudoColorRenderer( QgsRasterDataProvider* provider, int band, QgsRasterShader* shader /Transfer/ );
    ~QgsSingleBandPseudoColorRenderer();
    QgsRasterInterface * clone();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    /**Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader );
    QgsRasterShader* shader();

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;
};
