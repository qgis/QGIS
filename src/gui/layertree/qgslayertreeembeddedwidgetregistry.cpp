/***************************************************************************
  qgslayertreeembeddedwidgetregistry.cpp
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeembeddedwidgetregistry.h"

#include "qgslayertreeembeddedwidgetsimpl.h"



QgsLayerTreeEmbeddedWidgetRegistry* QgsLayerTreeEmbeddedWidgetRegistry::instance()
{
  static QgsLayerTreeEmbeddedWidgetRegistry* sInstance( new QgsLayerTreeEmbeddedWidgetRegistry() );
  return sInstance;
}

QgsLayerTreeEmbeddedWidgetRegistry::QgsLayerTreeEmbeddedWidgetRegistry()
{
  // populate with default implementations
  addProvider( new QgsLayerTreeTransparencyWidget::Provider() );
}

QgsLayerTreeEmbeddedWidgetRegistry::~QgsLayerTreeEmbeddedWidgetRegistry()
{
  Q_FOREACH ( QgsLayerTreeEmbeddedWidgetProvider* provider, mProviders )
  {
    removeProvider( provider->id() );
  }
}

QStringList QgsLayerTreeEmbeddedWidgetRegistry::providers() const
{
  return mProviders.keys();
}

QgsLayerTreeEmbeddedWidgetProvider* QgsLayerTreeEmbeddedWidgetRegistry::provider( const QString& providerId ) const
{
  return mProviders.value( providerId );
}

bool QgsLayerTreeEmbeddedWidgetRegistry::addProvider( QgsLayerTreeEmbeddedWidgetProvider* provider )
{
  if ( mProviders.contains( provider->id() ) )
    return false;

  mProviders.insert( provider->id(), provider );
  return true;
}

bool QgsLayerTreeEmbeddedWidgetRegistry::removeProvider( const QString& providerId )
{
  if ( !mProviders.contains( providerId ) )
    return false;

  delete mProviders.take( providerId );
  return true;
}
