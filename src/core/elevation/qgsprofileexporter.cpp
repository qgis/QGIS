/***************************************************************************
                         qgsprofileexporter.cpp
                         ---------------
    begin                : May 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#include "qgsprofileexporter.h"
#include "qgsabstractprofilesource.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsprofilerenderer.h"
#include "qgsmemoryproviderutils.h"
#include "qgsvectorlayer.h"

QgsProfileExporter::QgsProfileExporter( const QList<QgsAbstractProfileSource *> &sources, const QgsProfileRequest &request, Qgis::ProfileExportType type )
  : mType( type )
  , mRequest( request )
{
  for ( QgsAbstractProfileSource *source : sources )
  {
    if ( source )
    {
      if ( std::unique_ptr< QgsAbstractProfileGenerator > generator{ source->createProfileGenerator( mRequest ) } )
        mGenerators.emplace_back( std::move( generator ) );
    }
  }
}

QgsProfileExporter::~QgsProfileExporter() = default;

void QgsProfileExporter::run( QgsFeedback *feedback )
{
  if ( mGenerators.empty() )
    return;

  QgsProfilePlotRenderer renderer( std::move( mGenerators ), mRequest );
  renderer.startGeneration();
  renderer.waitForFinished();

  mFeatures = renderer.asFeatures( mType, feedback );
}

QList< QgsVectorLayer *> QgsProfileExporter::toLayers()
{
  if ( mFeatures.empty() )
    return {};

  // collect all features with the same geometry types together
  QHash< quint32, QVector< QgsAbstractProfileResults::Feature > > featuresByGeometryType;
  for ( const QgsAbstractProfileResults::Feature &feature : std::as_const( mFeatures ) )
  {
    featuresByGeometryType[static_cast< quint32 >( feature.geometry.wkbType() )].append( feature );
  }

  // generate a new memory provider layer for each geometry type
  QList< QgsVectorLayer * > res;
  for ( auto wkbTypeIt = featuresByGeometryType.constBegin(); wkbTypeIt != featuresByGeometryType.constEnd(); ++wkbTypeIt )
  {
    // first collate a master list of fields for this geometry type
    QgsFields outputFields;
    outputFields.append( QgsField( QStringLiteral( "layer" ), QVariant::String ) );

    for ( const QgsAbstractProfileResults::Feature &feature : std::as_const( wkbTypeIt.value() ) )
    {
      for ( auto attributeIt = feature.attributes.constBegin(); attributeIt != feature.attributes.constEnd(); ++attributeIt )
      {
        const int existingFieldIndex = outputFields.lookupField( attributeIt.key() );
        if ( existingFieldIndex < 0 )
        {
          outputFields.append( QgsField( attributeIt.key(), attributeIt.value().type() ) );
        }
        else
        {
          if ( outputFields.at( existingFieldIndex ).type() != QVariant::String && outputFields.at( existingFieldIndex ).type() != attributeIt.value().type() )
          {
            // attribute type mismatch across fields, just promote to string types to be flexible
            outputFields[ existingFieldIndex ].setType( QVariant::String );
          }
        }
      }
    }

    std::unique_ptr< QgsVectorLayer > outputLayer( QgsMemoryProviderUtils::createMemoryLayer(
          QStringLiteral( "profile" ),
          outputFields,
          static_cast< Qgis::WkbType >( wkbTypeIt.key() ),
          mRequest.crs(),
          false ) );

    QList< QgsFeature > featuresToAdd;
    featuresToAdd.reserve( wkbTypeIt.value().size() );
    for ( const QgsAbstractProfileResults::Feature &feature : std::as_const( wkbTypeIt.value() ) )
    {
      QgsFeature out( outputFields );
      out.setAttribute( 0, feature.layerIdentifier );
      out.setGeometry( feature.geometry );
      for ( auto attributeIt = feature.attributes.constBegin(); attributeIt != feature.attributes.constEnd(); ++attributeIt )
      {
        const int outputFieldIndex = outputFields.lookupField( attributeIt.key() );
        const QgsField &targetField = outputFields.at( outputFieldIndex );
        QVariant value = attributeIt.value();
        targetField.convertCompatible( value );
        out.setAttribute( outputFieldIndex, value );
      }
      featuresToAdd << out;
    }

    outputLayer->dataProvider()->addFeatures( featuresToAdd, QgsFeatureSink::FastInsert );
    res << outputLayer.release();
  }
  return res;
}
