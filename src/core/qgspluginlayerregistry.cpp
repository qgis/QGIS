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
#include "qgsmaplayerregistry.h"

QgsPluginLayerType::QgsPluginLayerType( const QString& name )
    : mName( name )
{
}

QgsPluginLayerType::~QgsPluginLayerType()
{
}

QString QgsPluginLayerType::name()
{
  return mName;
}

QgsPluginLayer* QgsPluginLayerType::createLayer()
{
  return nullptr;
}

QgsPluginLayer* QgsPluginLayerType::createLayer( const QString& uri )
{
  Q_UNUSED( uri );
  return nullptr;
}

bool QgsPluginLayerType::showLayerProperties( QgsPluginLayer *layer )
{
  Q_UNUSED( layer );
  return false;
}

//=============================================================================

/** Static calls to enforce singleton behaviour */
QgsPluginLayerRegistry* QgsPluginLayerRegistry::_instance = nullptr;
QgsPluginLayerRegistry* QgsPluginLayerRegistry::instance()
{
  if ( !_instance )
  {
    _instance = new QgsPluginLayerRegistry();
  }
  return _instance;
}


QgsPluginLayerRegistry::QgsPluginLayerRegistry()
{
}

QgsPluginLayerRegistry::~QgsPluginLayerRegistry()
{
  if ( !mPluginLayerTypes.isEmpty() )
  {
    QgsDebugMsg( "QgsPluginLayerRegistry::~QgsPluginLayerRegistry(): creator list not empty" );
    PluginLayerTypes::const_iterator it = mPluginLayerTypes.constBegin();
    for ( ; it != mPluginLayerTypes.constEnd(); ++it )
    {
      removePluginLayerType( it.key() );
    }
  }
}

QStringList QgsPluginLayerRegistry::pluginLayerTypes()
{
  return mPluginLayerTypes.keys();
}

bool QgsPluginLayerRegistry::addPluginLayerType( QgsPluginLayerType* type )
{
  if ( !type )
    return false;

  if ( mPluginLayerTypes.contains( type->name() ) )
    return false;

  mPluginLayerTypes[type->name()] = type;
  return true;
}


bool QgsPluginLayerRegistry::removePluginLayerType( const QString& typeName )
{
  if ( !mPluginLayerTypes.contains( typeName ) )
    return false;

  // remove all remaining layers of this type - to avoid invalid behaviour
  QList<QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers().values();
  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    if ( layer->type() == QgsMapLayer::PluginLayer )
    {
      QgsPluginLayer* pl = qobject_cast<QgsPluginLayer*>( layer );
      if ( pl->pluginLayerType() == typeName )
      {
        QgsMapLayerRegistry::instance()->removeMapLayers(
          QStringList() << layer->id() );
      }
    }
  }

  delete mPluginLayerTypes.take( typeName );
  return true;
}

QgsPluginLayerType* QgsPluginLayerRegistry::pluginLayerType( const QString& typeName )
{
  return mPluginLayerTypes.value( typeName, nullptr );
}


QgsPluginLayer* QgsPluginLayerRegistry::createLayer( const QString& typeName, const QString& uri )
{
  QgsPluginLayerType* type = pluginLayerType( typeName );
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
