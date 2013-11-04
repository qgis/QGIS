
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
};
