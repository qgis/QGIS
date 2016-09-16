
class QgsPluginLayer : QgsMapLayer
{
%TypeHeaderCode
#include "qgspluginlayer.h"
%End

  public:
    QgsPluginLayer( const QString& layerType, const QString& layerName = QString() );

    /** Return plugin layer type (the same as used in QgsPluginLayerRegistry) */
    QString pluginLayerType();

    /** Set extent of the layer */
    void setExtent( const QgsRectangle &extent );

    /** Set source string. This is used for example in layer tree to show tooltip.
     * @note added in 2.16
     */
    void setSource( const QString& source );

    //! return a list of symbology items for the legend
    //! (defult implementation returns nothing)
    //! @note Added in v2.1
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

};
