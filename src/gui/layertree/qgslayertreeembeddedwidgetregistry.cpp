/***************************************************************************
  qgslayertreeembeddedwidgetregistry.cpp
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeembeddedwidgetregistry.h"

#include "qgslayertreeembeddedwidgetsimpl.h"


QgsLayerTreeEmbeddedWidgetRegistry::QgsLayerTreeEmbeddedWidgetRegistry()
{
  // populate with default implementations
  addProvider( new QgsLayerTreeOpacityWidget::Provider() );
}

QgsLayerTreeEmbeddedWidgetRegistry::~QgsLayerTreeEmbeddedWidgetRegistry()
{
  const auto constMProviders = mProviders;
  for ( QgsLayerTreeEmbeddedWidgetProvider *provider : constMProviders )
  {
    removeProvider( provider->id() );
  }
}

QStringList QgsLayerTreeEmbeddedWidgetRegistry::providers() const
{
  return mProviders.keys();
}

QgsLayerTreeEmbeddedWidgetProvider *QgsLayerTreeEmbeddedWidgetRegistry::provider( const QString &providerId ) const
{
  return mProviders.value( providerId );
}

bool QgsLayerTreeEmbeddedWidgetRegistry::addProvider( QgsLayerTreeEmbeddedWidgetProvider *provider )
{
  if ( mProviders.contains( provider->id() ) )
    return false;

  mProviders.insert( provider->id(), provider );
  return true;
}

bool QgsLayerTreeEmbeddedWidgetRegistry::removeProvider( const QString &providerId )
{
  if ( !mProviders.contains( providerId ) )
    return false;

  delete mProviders.take( providerId );
  return true;
}
