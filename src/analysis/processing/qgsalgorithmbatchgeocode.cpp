/***************************************************************************
                         qgsalgorithmbatchgeocode.cpp
                         ------------------
    begin                : August 2020
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

#include "qgsalgorithmbatchgeocode.h"
#include "qgsgeocoder.h"
#include "qgsgeocoderresult.h"
#include "qgsgeocodercontext.h"

QgsBatchGeocodeAlgorithm::QgsBatchGeocodeAlgorithm( QgsGeocoderInterface *geocoder )
  : QgsProcessingFeatureBasedAlgorithm()
  , mGeocoder( geocoder )
{

}

QStringList QgsBatchGeocodeAlgorithm::tags() const
{
  return QObject::tr( "geocode" ).split( ',' );
}

QString QgsBatchGeocodeAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsBatchGeocodeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

void QgsBatchGeocodeAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Address field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::String ) );
}

QList<int> QgsBatchGeocodeAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVector;
}

QString QgsBatchGeocodeAlgorithm::outputName() const
{
  return QObject::tr( "Geocoded" );
}

bool QgsBatchGeocodeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAddressField = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  return true;
}

QgsWkbTypes::Type QgsBatchGeocodeAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::Point;
}

QgsCoordinateReferenceSystem QgsBatchGeocodeAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mOutputCrs = inputCrs;
  return mOutputCrs;
}

QgsFields QgsBatchGeocodeAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  // append any additional fields created by the geocoder
  QgsFields res = inputFields;
  const QgsFields newFields = mGeocoder->appendedFields();
  mAdditionalFields = newFields.names();
  res.extend( newFields );
  return res;
}

QgsFeatureList QgsBatchGeocodeAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;

  QgsAttributes attr = f.attributes();

  const QString address = f.attribute( mAddressField ).toString();
  if ( address.isEmpty() )
  {
    // append null attributes
    for ( int i = 0; i < mAdditionalFields.count(); ++ i )
      attr.append( QVariant() );
    f.setAttributes( attr );
    feedback->reportError( QObject::tr( "Empty address field for feature %1" ).arg( feature.id() ) );
    return QgsFeatureList() << f;
  }

  QgsGeocoderContext geocodeContext( context.transformContext() );
  const QList< QgsGeocoderResult > results = mGeocoder->geocodeString( address, geocodeContext, feedback );
  if ( results.empty() )
  {
    // append null attributes
    for ( int i = 0; i < mAdditionalFields.count(); ++ i )
      attr.append( QVariant() );
    f.setAttributes( attr );
    feedback->reportError( QObject::tr( "No result for %1" ).arg( address ) );
    return QgsFeatureList() << f;
  }

  if ( !results.at( 0 ).isValid() )
  {
    // append null attributes
    for ( int i = 0; i < mAdditionalFields.count(); ++ i )
      attr.append( QVariant() );
    f.setAttributes( attr );

    feedback->reportError( QObject::tr( "Error geocoding %1: %2" ).arg( address, results.at( 0 ).error() ) );
    return QgsFeatureList() << f;
  }

  const QVariantMap additionalAttributes = results.at( 0 ).additionalAttributes();
  for ( const QString &additionalField : mAdditionalFields )
  {
    attr.append( additionalAttributes.value( additionalField ) );
  }
  f.setAttributes( attr );

  QgsCoordinateTransform transform = QgsCoordinateTransform( results.at( 0 ).crs(), mOutputCrs, context.transformContext() );
  QgsGeometry g = results.at( 0 ).geometry();
  try
  {
    g.transform( transform );
  }
  catch ( QgsCsException & )
  {
    feedback->reportError( QObject::tr( "Error transforming %1 to layer CRS" ).arg( address ) );
    return QgsFeatureList() << f;
  }

  f.setGeometry( g );
  return QgsFeatureList() << f;
}
