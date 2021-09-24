/***************************************************************************
                              qgsdimensionfilter.cpp
                              -------------------
  begin                : September 2021
  copyright            : (C) 2021 Matthias Kuhn
  email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdimensionfilter.h"

QgsDimensionFilter::QgsDimensionFilter( const QMap<const QgsVectorLayer *, QStringList> dimensionFilter )
  : mDimensionFilter( dimensionFilter )
{

}

void QgsDimensionFilter::filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &featureRequest ) const
{
  const QStringList dimFilters = mDimensionFilter.value( layer );

  for ( const QString &flt : dimFilters )
    featureRequest.combineFilterExpression( flt );
}

QStringList QgsDimensionFilter::layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const
{
  Q_UNUSED( layer )
  return attributes;
}

QgsDimensionFilter *QgsDimensionFilter::clone() const
{
  return new QgsDimensionFilter( mDimensionFilter );
}
