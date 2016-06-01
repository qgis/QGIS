/**
 * @brief A renderer for generating live hillshade models.
 * @note added in QGIS 2.16
 */

class QgsHillshadeRenderer : QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgshillshaderenderer.h"
%End
  public:

    /**
     * @brief A renderer for generating live hillshade models.
     * @param input The input raster interface
     * @param band The band in the raster to use
     * @param lightAzimuth The azimuth of the light source
     * @param lightAltitude The altitude of the light source
     */
    QgsHillshadeRenderer( QgsRasterInterface* input, int band, double lightAzimuth, double lightAngle );

    ~QgsHillshadeRenderer();

    virtual QgsHillshadeRenderer * clone() const /Factory/;

    /**
     * @brief Factory method to create a new renderer
     * @param elem A DOM element to create the renderer from.
     * @param input The raster input interface.
     * @return A new QgsHillshadeRenderer.
     */
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
     * Returns the direction of the light over the raster between 0-360.
     * @see setAzimuth()
     */
    double azimuth() const;

    /** Returns the angle of the light source over the raster.
     * @see setAltitude()
     */
    double altitude()  const;

    /** Returns the Z scaling factor.
     * @see setZFactor()
     */
    double zFactor()  const;

    /** Returns true if the renderer is using multi-directional hillshading.
     * @see setMultiDirectional()
     */
    bool multiDirectional() const;


    /**
     * @brief Set the azimuth of the light source.
     * @param azimuth The azimuth of the light source, between 0 and 360.0
     * @see azimuth()
     */
    void setAzimuth( double azimuth );

    /**
     * @brief Set the altitude of the light source
     * @param altitude the altitude
     * @see altitude()
     */
    void setAltitude( double angle );

    /**
     * @brief Set the Z scaling factor of the result image.
     * @param zfactor The z factor
     * @see zFactor()
     */
    void setZFactor( double zfactor );

    /** Sets whether to render using a multi-directional hillshade algorithm.
     * @param isMultiDirectional set to true to use multi directional rendering
     * @see multiDirectional()
     */
    void setMultiDirectional( bool isMultiDirectional );
};
