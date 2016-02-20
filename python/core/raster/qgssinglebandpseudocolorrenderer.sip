class QgsSingleBandPseudoColorRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandpseudocolorrenderer.h"
%End
  public:
    /** Note: takes ownership of QgsRasterShader*/
    QgsSingleBandPseudoColorRenderer( QgsRasterDataProvider* provider, int band, QgsRasterShader* shader /Transfer/ );
    ~QgsSingleBandPseudoColorRenderer();
    virtual QgsSingleBandPseudoColorRenderer * clone() const /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider ) /Factory/;

    QgsRasterBlock* block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;

    /** Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader /Transfer/ );
    QgsRasterShader* shader();
    //! @note available in python as constShader
    const QgsRasterShader* shader() const /PyName=constShader/;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

    QList<int> usesBands() const;

    /** Returns the band used by the renderer
     * @note added in QGIS 2.7
     */
    int band() const;

    /** Sets the band used by the renderer.
     * @see band
     * @note added in QGIS 2.10
     */
    void setBand(int bandNo);

    double classificationMin() const;
    double classificationMax() const;
    void setClassificationMin( double min );
    void setClassificationMax( double max );
    int classificationMinMaxOrigin() const;
    void setClassificationMinMaxOrigin( int origin );

  private:

    QgsSingleBandPseudoColorRenderer( const QgsSingleBandPseudoColorRenderer& );
    const QgsSingleBandPseudoColorRenderer& operator=( const QgsSingleBandPseudoColorRenderer& );

};
