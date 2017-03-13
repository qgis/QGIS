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

    QgsRasterBlock* block( int bandNo, const QgsRectangle & extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) / Factory /;

    /** Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader /Transfer/ );

    QgsRasterShader* shader();

    //! @note available in python as constShader
    const QgsRasterShader* shader() const /PyName=constShader/;

    /** Creates a color ramp shader
     * @param colorRamp vector color ramp
     * @param colorRampType type of color ramp shader
     * @param classificationMode classification mode
     * @param classes number of classes
     * @param clip clip out of range values
     * @param extent extent used in classification (only used in quantile mode)
     */
    void createShader( QgsColorRamp* colorRamp = nullptr, QgsColorRampShader::Type colorRampType  = QgsColorRampShader::Interpolated, QgsColorRampShader::ClassificationMode classificationMode = QgsColorRampShader::Continuous, int classes = 0, bool clip = false, const QgsRectangle& extent = QgsRectangle() );

    void writeXml( QDomDocument& doc, QDomElement& parentElem ) const;

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

  private:

    QgsSingleBandPseudoColorRenderer( const QgsSingleBandPseudoColorRenderer& );
    const QgsSingleBandPseudoColorRenderer& operator=( const QgsSingleBandPseudoColorRenderer& );

};
