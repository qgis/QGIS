/**
 * @brief Renderer widget for the hill shade renderer.
 * @ingroup gui
 * @note added in QGIS 2.16
 */
class QgsHillshadeRendererWidget: QgsRasterRendererWidget
{
%TypeHeaderCode
#include <qgshillshaderendererwidget.h>
%End
  public:

    /**
     * @brief Renderer widget for the hill shade renderer.
     * @param layer The layer attached for this widget.
     * @param extent The current extent.
     */
    QgsHillshadeRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsHillshadeRendererWidget();

    /**
      * Factory method to create the renderer for this type.
      */
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) /Factory/;

    /**
     * @brief The renderer for the widget.
     * @return A new renderer for the the config in the widget
     */
    QgsRasterRenderer* renderer();

    /**
     * @brief Set the widget state from the given renderer.
     * @param r The renderer to take the state from.
     */
    void setFromRenderer( const QgsRasterRenderer* r );

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

    /**
     * Returns true if the renderer should use the multi-directional hillshade algorithm.
     * @see setMultiDirectional()
     */
    bool multiDirectional() const;

  public slots:

    /**
     * @brief Set the altitude of the light source
     * @param altitude the altitude
     * @see altitude()
     */
    void setAltitude( double altitude );

    /**
     * @brief Set the azimuth of the light source.
     * @param azimuth The azimuth of the light source, between 0 and 360.0
     * @see azimuth()
     */
    void setAzimuth( double azimuth );

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
    void setMultiDirectional( bool isMultiDirectional);
};
