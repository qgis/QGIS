#include "qgspluginlayer.h"

QgsPluginLayer::QgsPluginLayer( QString layerType, QString layerName )
    : QgsMapLayer( PluginLayer, layerName ), mPluginLayerType( layerType )
{
}

QString QgsPluginLayer::pluginLayerType()
{
  return mPluginLayerType;
}

void QgsPluginLayer::setExtent( const QgsRectangle & extent )
{
  mLayerExtent = extent;
}
