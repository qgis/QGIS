
class QgsPluginLayer : QgsMapLayer
{
%TypeHeaderCode
#include "qgspluginlayer.h"
%End

  public:
    QgsPluginLayer( QString layerType, QString layerName = QString() );

    /** return plugin layer type (the same as used in QgsPluginLayerRegistry) */
    QString pluginLayerType();

    void setExtent( const QgsRectangle &extent );

    //! return a list of symbology items for the legend
    //! (defult implementation returns nothing)
    //! @note Added in v2.1
    virtual QgsLegendSymbologyList legendSymbologyItems( const QSize& iconSize );

    /** Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     *
     * The default implementation returns map layer renderer which just calls draw().
     * This may work, but it is unsafe for multi-threaded rendering because of the run
     * conditions that may happen (e.g. something is changed in the layer while it is
     * being rendered).
     *
     * @note added in 2.4
     */
    virtual QgsMapLayerRenderer* createMapRenderer( QgsRenderContext& rendererContext ) /Factory/;
};
