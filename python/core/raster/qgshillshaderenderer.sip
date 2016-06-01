class QgsHillshadeRenderer : QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgshillshaderenderer.h"
%End
  public:
    /** Renderer owns color array*/
    QgsHillshadeRenderer( QgsRasterInterface* input, int band , double lightAzimuth, double lightAngle );

    ~QgsHillshadeRenderer();

    virtual QgsHillshadeRenderer * clone() const /Factory/;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterInterface* input ) /Factory/;

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) /Factory/;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    QList<int> usesBands() const;

    /** Returns the band used by the renderer
     */
    int band() const;

    /** Sets the band used by the renderer.
     * @see band
     */
    void setBand( int bandNo );

    /**
     * @brief The direction of the light over the raster between 0-360
     * @return The direction of the light over the raster
     */
    double azimuth() const;

    /**
     * @brief The angle of the light source over the raster
     * @return The angle of the light source over the raster
     */
    double altitude()  const;

    /**
     * @brief Z Factor
     * @return Z Factor
     */
    double zFactor()  const;

    /**
     * @brief Is Multi Directional
     * @return Is Multi Directional
     */
    bool multiDirectional() const;


    /**
     * @brief Set the azimith of the light source.
     * @param azimuth The azimuth of the light source.
     */
    void setAzimuth( double azimuth );

    /**
     * @brief Set the altitude of the light source
     * @param altitude The altitude
     */
    void setAltitude( double angle );

    /**
     * @brief Set the Z factor of the result image.
     * @param zfactor The z factor.
     */
    void setZFactor( double zfactor );

    /**
     * @brief Set Is Multi Directional
     * @param isMultiDirectional Is multi directional
     */
    void setMultiDirectional( bool isMultiDirectional );
};
