/***************************************************************************
                    qgspluginlayerregistry.cpp - class for
                    registering plugin layer creators
                             -------------------
    begin                : Mon Nov 30 2009
    copyright            : (C) 2009 by Mathias Walker, Sourcepole
    email                : mwa at sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspluginlayerregistry.h"
#include "qgslogger.h"
#include "qgspluginlayer.h"
#include "qgsproject.h"

QgsPluginLayerType::QgsPluginLayerType( const QString &name )
  : mName( name )
{
}

QString QgsPluginLayerType::name() const
{
  return mName;
}

QgsPluginLayer *QgsPluginLayerType::createLayer()
{
  return nullptr;
}

QgsPluginLayer *QgsPluginLayerType::createLayer( const QString &uri )
{
  Q_UNUSED( uri )
  return nullptr;
}

bool QgsPluginLayerType::showLayerProperties( QgsPluginLayer *layer )
{
  Q_UNUSED( layer )
  return false;
}

//
// QgsPluginLayerRegistry
//

QgsPluginLayerRegistry::~QgsPluginLayerRegistry()
{
  if ( !mPluginLayerTypes.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "QgsPluginLayerRegistry::~QgsPluginLayerRegistry(): creator list not empty" ) );
    const QStringList keys = mPluginLayerTypes.keys();
    for ( const QString &key : keys )
    {
      removePluginLayerType( key );
    }
  }
}

QStringList QgsPluginLayerRegistry::pluginLayerTypes()
{
  return mPluginLayerTypes.keys();
}

bool QgsPluginLayerRegistry::addPluginLayerType( QgsPluginLayerType *type )
{
  if ( !type )
    return false;

  if ( mPluginLayerTypes.contains( type->name() ) )
    return false;

  mPluginLayerTypes[type->name()] = type;
  return true;
}


bool QgsPluginLayerRegistry::removePluginLayerType( const QString &typeName )
{
  if ( !mPluginLayerTypes.contains( typeName ) )
    return false;

  // remove all remaining layers of this type - to avoid invalid behavior
  const QList<QgsMapLayer *> layers = QgsProject::instance()->mapLayers().values();
  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( layer->type() == QgsMapLayerType::PluginLayer )
    {
      QgsPluginLayer *pl = qobject_cast<QgsPluginLayer *>( layer );
      if ( pl->pluginLayerType() == typeName )
      {
        QgsProject::instance()->removeMapLayers(
          QStringList() << layer->id() );
      }
    }
  }

  delete mPluginLayerTypes.take( typeName );
  return true;
}

QgsPluginLayerType *QgsPluginLayerRegistry::pluginLayerType( const QString &typeName )
{
  return mPluginLayerTypes.value( typeName, nullptr );
}


QgsPluginLayer *QgsPluginLayerRegistry::createLayer( const QString &typeName, const QString &uri )
{
  QgsPluginLayerType *type = pluginLayerType( typeName );
  if ( !type )
  {
    QgsDebugMsg( "Unknown plugin layer type: " + typeName );
    return nullptr;
  }

  if ( !uri.isEmpty() )
    return type->createLayer( uri );
  else
    return type->createLayer();
}
