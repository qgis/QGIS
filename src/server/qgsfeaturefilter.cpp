/***************************************************************************
                      qgsfeaturefilter.cpp
                      --------------------
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

#include "qgsfeaturefilter.h"
#include "qgsfeaturerequest.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"

void QgsFeatureFilter::filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const
{
  const QString expr = mFilters[layer->id()];
  if ( !expr.isEmpty() )
  {
    filterFeatures.setFilterExpression( expr );
  }
}

QgsFeatureFilterProvider *QgsFeatureFilter::clone() const
{
  auto result = new QgsFeatureFilter();
  result->mFilters = mFilters;
  return result;
}

void QgsFeatureFilter::setFilter( const QgsVectorLayer *layer, const QgsExpression &filter )
{
  mFilters[layer->id()] = filter.dump();
}
