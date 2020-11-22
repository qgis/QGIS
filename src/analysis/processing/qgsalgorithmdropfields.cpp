/***************************************************************************
                         qgsalgorithmdropfields.cpp
                         ---------------------------------
    begin                : November 2020
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

#include "qgsalgorithmdropfields.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsDropTableFieldsAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() & ~QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
}

QString QgsDropTableFieldsAlgorithm::name() const
{
  return QStringLiteral( "deletecolumn" );
}

QString QgsDropTableFieldsAlgorithm::displayName() const
{
  return QObject::tr( "Drop field(s)" );
}

QString QgsDropTableFieldsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and generates a new one that has the exact same content but without the selected columns." );
}

QString QgsDropTableFieldsAlgorithm::shortDescription() const
{
  return QObject::tr( "Deletes fields from a vector layer." );
}

QStringList QgsDropTableFieldsAlgorithm::tags() const
{
  return QObject::tr( "drop,delete,remove,fields,columns,attributes" ).split( ',' );
}

QString QgsDropTableFieldsAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsDropTableFieldsAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QString QgsDropTableFieldsAlgorithm::outputName() const
{
  return QObject::tr( "Remaining fields" );
}

QList<int> QgsDropTableFieldsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVector;
}

QgsProcessingFeatureSource::Flag QgsDropTableFieldsAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsDropTableFieldsAlgorithm *QgsDropTableFieldsAlgorithm::createInstance() const
{
  return new QgsDropTableFieldsAlgorithm();
}

void QgsDropTableFieldsAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterField( QStringLiteral( "COLUMN" ), QObject::tr( "Fields to drop" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true ) );
}

QgsFields QgsDropTableFieldsAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  // loop through twice - first we need to build up a list of original attribute indices
  for ( const QString &field : mFieldsToDelete )
  {
    const int index = inputFields.lookupField( field );
    if ( index >= 0 )
      mFieldIndices.append( index );
  }

  // important - make sure we remove from the end so we aren't changing used indices as we go
  std::sort( mFieldIndices.begin(), mFieldIndices.end(), std::greater< int >() );

  // this second time we make a cleaned version of the fields
  for ( const int index : qgis::as_const( mFieldIndices ) )
  {
    outFields.remove( index );
  }
  return outFields;
}

bool QgsDropTableFieldsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mFieldsToDelete = parameterAsFields( parameters, QStringLiteral( "COLUMN" ), context );

  if ( feedback )
  {
    std::unique_ptr< QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
    if ( source )
    {
      for ( const QString &field : qgis::as_const( mFieldsToDelete ) )
      {
        const int index = source->fields().lookupField( field );
        if ( index < 0 )
        {
          feedback->pushInfo( QObject::tr( "Field “%1” does not exist in input layer " ).arg( field ) );
        }
      }
    }
  }

  return true;
}

QgsFeatureList QgsDropTableFieldsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  QgsAttributes attributes = f.attributes();
  for ( const int index : mFieldIndices )
  {
    attributes.remove( index );
  }
  f.setAttributes( attributes );
  return QgsFeatureList() << f;
}

bool QgsDropTableFieldsAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
