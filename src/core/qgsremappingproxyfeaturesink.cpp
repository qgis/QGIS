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

QgsRemappingProxyFeatureSink::QgsRemappingProxyFeatureSink( const QgsRemappingSinkDefinition &mappingDefinition, QgsFeatureSink *sink, bool ownsSink )
  : QgsFeatureSink()
  , mDefinition( mappingDefinition )
  , mSink( sink )
  , mOwnsSink( ownsSink )
{}

QgsRemappingProxyFeatureSink::~QgsRemappingProxyFeatureSink()
{
  if ( mOwnsSink )
    delete mSink;
}

void QgsRemappingProxyFeatureSink::setExpressionContext( const QgsExpressionContext &context ) const
{
  mContext = context;
}

void QgsRemappingProxyFeatureSink::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransform = QgsCoordinateTransform( mDefinition.sourceCrs(), mDefinition.destinationCrs(), context );
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
          reproject.transform( mTransform );
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

QString QgsRemappingProxyFeatureSink::lastError() const
{
  return mSink->lastError();
}

QVariant QgsRemappingSinkDefinition::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "wkb_type" ), mDestinationWkbType );
  // we only really care about names here
  QVariantList fieldNames;
  for ( const QgsField &field : mDestinationFields )
    fieldNames << field.name();
  map.insert( QStringLiteral( "destination_field_names" ), fieldNames );
  map.insert( QStringLiteral( "transform_source" ), mSourceCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) );
  map.insert( QStringLiteral( "transform_dest" ), mDestinationCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) );

  QVariantMap fieldMap;
  for ( auto it = mFieldMap.constBegin(); it != mFieldMap.constEnd(); ++it )
  {
    fieldMap.insert( it.key(), it.value().toVariant() );
  }
  map.insert( QStringLiteral( "field_map" ), fieldMap );

  return map;
}

bool QgsRemappingSinkDefinition::loadVariant( const QVariantMap &map )
{
  mDestinationWkbType = static_cast< QgsWkbTypes::Type >( map.value( QStringLiteral( "wkb_type" ), QgsWkbTypes::Unknown ).toInt() );

  const QVariantList fieldNames = map.value( QStringLiteral( "destination_field_names" ) ).toList();
  QgsFields fields;
  for ( const QVariant &field : fieldNames )
  {
    fields.append( QgsField( field.toString() ) );
  }
  mDestinationFields = fields;

  mSourceCrs = QgsCoordinateReferenceSystem::fromWkt( map.value( QStringLiteral( "transform_source" ) ).toString() );
  mDestinationCrs = QgsCoordinateReferenceSystem::fromWkt( map.value( QStringLiteral( "transform_dest" ) ).toString() );

  const QVariantMap fieldMap = map.value( QStringLiteral( "field_map" ) ).toMap();
  mFieldMap.clear();
  for ( auto it = fieldMap.constBegin(); it != fieldMap.constEnd(); ++it )
  {
    QgsProperty p;
    p.loadVariant( it.value() );
    mFieldMap.insert( it.key(), p );
  }

  return true;
}

bool QgsRemappingSinkDefinition::operator==( const QgsRemappingSinkDefinition &other ) const
{
  return mDestinationWkbType == other.mDestinationWkbType
         && mDestinationFields == other.mDestinationFields
         && mFieldMap == other.mFieldMap
         && mSourceCrs == other.mSourceCrs
         && mDestinationCrs == other.mDestinationCrs;
}

bool QgsRemappingSinkDefinition::operator!=( const QgsRemappingSinkDefinition &other ) const
{
  return !( *this == other );
}
