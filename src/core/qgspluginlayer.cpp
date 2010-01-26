#include "qgspluginlayer.h"

QgsPluginLayer::QgsPluginLayer( QString layerType, QString layerName )
    : QgsMapLayer( PluginLayer, layerName ), mPluginLayerType( layerType )
{
}

QString QgsPluginLayer::pluginLayerType()
{
  return mPluginLayerType;
}
