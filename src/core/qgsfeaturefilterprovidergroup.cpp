/***************************************************************************
                      qgsfeaturefilterprovidergroup.cpp
                      --------------------------------
  begin                : 26-10-2017
  copyright            : (C) 2017 by Patrick Valsecchi
  email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturefilterprovidergroup.h"
#include "qgsfeaturerequest.h"

void QgsFeatureFilterProviderGroup::filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const
{
  for ( const QgsFeatureFilterProvider *provider : mProviders )
  {
    QgsFeatureRequest temp;
    provider->filterFeatures( layer, temp );
    if ( auto *lFilterExpression = temp.filterExpression() )
    {
      filterFeatures.combineFilterExpression( lFilterExpression->dump() );
    }
  }
}

QStringList QgsFeatureFilterProviderGroup::layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const
{
  QStringList allowedAttributes { attributes };
  for ( const QgsFeatureFilterProvider *provider : mProviders )
  {
    const QgsFeatureRequest temp;
    allowedAttributes = provider->layerAttributes( layer, allowedAttributes );
  }
  return allowedAttributes;
}

QgsFeatureFilterProvider *QgsFeatureFilterProviderGroup::clone() const
{
  auto result = new QgsFeatureFilterProviderGroup();
  result->mProviders = mProviders;
  return result;
}

QgsFeatureFilterProviderGroup &QgsFeatureFilterProviderGroup::addProvider( const QgsFeatureFilterProvider *provider )
{
  if ( provider )
  {
    mProviders.append( provider );
  }
  return *this;
}
