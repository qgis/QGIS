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
    double Azimuth() const;

    /**
     * @brief The angle of the light source over the raster
     * @return The angle of the light source over the raster
     */
    double Angle()  const;

    /**
     * @brief Z Factor
     * @return Z Factor
     */
    double zFactor()  const;

    void setAzimuth( double azimuth );
    void setAngle( double angle );
    void setZFactor( double zfactor );

};
