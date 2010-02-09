#ifndef QGSPLUGINLAYER_H
#define QGSPLUGINLAYER_H

#include "qgsmaplayer.h"

/** \ingroup core
  Base class for plugin layers. These can be implemented by plugins
  and registered in QgsPluginLayerRegistry.

  In order to be readable from project files, they should set these attributes in layer DOM node:
   "type" = "plugin"
   "name" = "your_layer_type"

  \note added in v1.5
 */
class CORE_EXPORT QgsPluginLayer : public QgsMapLayer
{
    Q_OBJECT

  public:
    QgsPluginLayer( QString layerType, QString layerName = QString() );

    /** return plugin layer type (the same as used in QgsPluginLayerRegistry) */
    QString pluginLayerType();

    void setExtent( const QgsRectangle & extent );

  protected:
    QString mPluginLayerType;
};

#endif // QGSPLUGINLAYER_H
