/***************************************************************************
                         qgsremappingproxyfeaturesink.cpp
                         ----------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsremappingproxyfeaturesink.h"
#include "qgslogger.h"

QgsRemappingProxyFeatureSink::QgsRemappingProxyFeatureSink( const QgsRemappingSinkDefinition &mappingDefinition, QgsFeatureSink *sink )
  : QgsFeatureSink()
  , mDefinition( mappingDefinition )
  , mSink( sink )
{}

void QgsRemappingProxyFeatureSink::setExpressionContext( const QgsExpressionContext &context )
{
  mContext = context;
}

QgsFeatureList QgsRemappingProxyFeatureSink::remapFeature( const QgsFeature &feature ) const
{
  QgsFeatureList res;

  mContext.setFeature( feature );

  // remap fields first
  QgsFeature f;
  f.setFields( mDefinition.destinationFields(), true );
  QgsAttributes attributes;
  const QMap< QString, QgsProperty > fieldMap = mDefinition.fieldMap();
  for ( const QgsField &field : mDefinition.destinationFields() )
  {
    if ( fieldMap.contains( field.name() ) )
    {
      attributes.append( fieldMap.value( field.name() ).value( mContext ) );
    }
    else
    {
      attributes.append( QVariant() );
    }
  }
  f.setAttributes( attributes );

  // make geometries compatible, and reproject if necessary
  if ( feature.hasGeometry() )
  {
    const QVector< QgsGeometry > geometries = feature.geometry().coerceToType( mDefinition.destinationWkbType() );
    if ( !geometries.isEmpty() )
    {
      res.reserve( geometries.size() );
      for ( const QgsGeometry &geometry : geometries )
      {
        QgsFeature featurePart = f;

        QgsGeometry reproject = geometry;
        try
        {
          reproject.transform( mDefinition.transform() );
          featurePart.setGeometry( reproject );
        }
        catch ( QgsCsException & )
        {
          QgsLogger::warning( QObject::tr( "Error reprojecting feature geometry" ) );
          featurePart.clearGeometry();
        }
        res << featurePart;
      }
    }
    else
    {
      f.clearGeometry();
      res << f;
    }
  }
  else
  {
    res << f;
  }
  return res;
}

bool QgsRemappingProxyFeatureSink::addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags )
{
  QgsFeatureList features = remapFeature( feature );
  return mSink->addFeatures( features, flags );
}

bool QgsRemappingProxyFeatureSink::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags )
{
  bool res = true;
  for ( QgsFeature &f : features )
  {
    res = addFeature( f, flags ) && res;
  }
  return res;
}

bool QgsRemappingProxyFeatureSink::addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags )
{
  QgsFeature f;
  bool res = true;
  while ( iterator.nextFeature( f ) )
  {
    res = addFeature( f, flags ) && res;
  }
  return res;
}
