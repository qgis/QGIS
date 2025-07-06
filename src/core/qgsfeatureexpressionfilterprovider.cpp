/***************************************************************************
                         qgsfeatureexpressionfilterprovider.cpp
                         ------------------
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

#include "qgsfeatureexpressionfilterprovider.h"
#include "qgsfeaturerequest.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"

void QgsFeatureExpressionFilterProvider::filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const
{
  this->filterFeatures( layer->id(), filterFeatures );
}

void QgsFeatureExpressionFilterProvider::filterFeatures( const QString &layerId, QgsFeatureRequest &filterFeatures ) const
{
  const QString expr = mFilters[layerId];
  if ( !expr.isEmpty() )
  {
    filterFeatures.setFilterExpression( expr );
  }
}

QStringList QgsFeatureExpressionFilterProvider::layerAttributes( const QgsVectorLayer *, const QStringList &attributes ) const
{
  // Do nothing
  return attributes;
}

QgsFeatureExpressionFilterProvider *QgsFeatureExpressionFilterProvider::clone() const
{
  QgsFeatureExpressionFilterProvider *filter = new QgsFeatureExpressionFilterProvider();
  filter->mFilters = mFilters;
  return filter;
}

void QgsFeatureExpressionFilterProvider::setFilter( const QString &layerId, const QgsExpression &filter )
{
  mFilters[layerId] = filter.dump();
}
