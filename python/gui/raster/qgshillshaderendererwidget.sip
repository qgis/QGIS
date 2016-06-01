/**
 * @brief Renderer widget for the hill shade renderer.
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
     * @brief multiDirectional
     * @return multiDirectional
     */
    bool multiDirectional() const;

  public slots:
    /**
     * @brief Set the altitude of the light source
     * @param altitude The altitude
     */
    void setAltitude( double altitude );

    /**
     * @brief Set the azimith of the light source.
     * @param azimuth The azimuth of the light source.
     */
    void setAzimuth( double azimuth );

    /**
     * @brief Set the Z factor of the result image.
     * @param zfactor The z factor.
     */
    void setZFactor( double zfactor );

    /**
     * @brief set MultiDirectional
     * @param isMultiDirectional
     */
    void setMultiDirectional( bool isMultiDirectional);
};
