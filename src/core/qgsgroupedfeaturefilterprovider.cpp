/***************************************************************************
                      qgsgroupedfeaturefilterprovider.cpp
                      --------------------------------
  begin                : 2025-07-26
  copyright            : (C) 2025 by Mathieu Pellerin
  email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgroupedfeaturefilterprovider.h"
#include "qgsfeaturerequest.h"
#include "qgsvectorlayer.h"

bool QgsGroupedFeatureFilterProvider::isFilterThreadSafe() const
{
  for ( const QgsFeatureFilterProvider *provider : mProviders )
  {
    Q_NOWARN_DEPRECATED_PUSH
    if ( !provider->isFilterThreadSafe() )
    {
      return false;
    }
    Q_NOWARN_DEPRECATED_POP
  }
  return true;
}

void QgsGroupedFeatureFilterProvider::filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const
{
  for ( const QgsFeatureFilterProvider *provider : mProviders )
  {
    QgsFeatureRequest temp;
    Q_NOWARN_DEPRECATED_PUSH
    if ( provider->isFilterThreadSafe() )
    {
      provider->filterFeatures( layer->id(), temp );
    }
    else
    {
      provider->filterFeatures( layer, temp );
    }
    Q_NOWARN_DEPRECATED_POP
    if ( auto *lFilterExpression = temp.filterExpression() )
    {
      filterFeatures.combineFilterExpression( lFilterExpression->dump() );
    }
  }
}

void QgsGroupedFeatureFilterProvider::filterFeatures( const QString &layerId, QgsFeatureRequest &filterFeatures ) const
{
  for ( const QgsFeatureFilterProvider *provider : mProviders )
  {
    QgsFeatureRequest temp;
    provider->filterFeatures( layerId, temp );
    if ( auto *lFilterExpression = temp.filterExpression() )
    {
      filterFeatures.combineFilterExpression( lFilterExpression->dump() );
    }
  }
}

QStringList QgsGroupedFeatureFilterProvider::layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const
{
  QStringList allowedAttributes { attributes };
  for ( const QgsFeatureFilterProvider *provider : mProviders )
  {
    const QgsFeatureRequest temp;
    allowedAttributes = provider->layerAttributes( layer, allowedAttributes );
  }
  return allowedAttributes;
}

QgsGroupedFeatureFilterProvider *QgsGroupedFeatureFilterProvider::clone() const
{
  QgsGroupedFeatureFilterProvider *filter = new QgsGroupedFeatureFilterProvider();
  filter->mProviders = mProviders;
  return filter;
}

QgsGroupedFeatureFilterProvider &QgsGroupedFeatureFilterProvider::addProvider( const QgsFeatureFilterProvider *provider )
{
  if ( provider )
  {
    mProviders.append( provider );
  }
  return *this;
}
