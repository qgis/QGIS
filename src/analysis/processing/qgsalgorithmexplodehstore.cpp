/***************************************************************************
                         qgsalgorithmexplodehstore.h
                         ---------------------
    begin                : September 2018
    copyright            : (C) 2018 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsalgorithmexplodehstore.h"
#include "qgshstoreutils.h"
#include "qgsprocessingutils.h"

///@cond PRIVATE

QString QgsExplodeHstoreAlgorithm::name() const
{
  return QStringLiteral( "explodehstorefield" );
}

QString QgsExplodeHstoreAlgorithm::displayName() const
{
  return QObject::tr( "Explode HStore Field" );
}

QStringList QgsExplodeHstoreAlgorithm::tags() const
{
  return QObject::tr( "field,explode,hstore,osm,openstreetmap" ).split( ',' );
}

QString QgsExplodeHstoreAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsExplodeHstoreAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QString QgsExplodeHstoreAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a copy of the input layer and adds a new field for every unique key in the HStore field.\n"
                      "The expected field list is an optional comma separated list. By default, all unique keys are added. If this list is specified, only these fields are added and the HStore field is updated." );
}

QgsProcessingAlgorithm *QgsExplodeHstoreAlgorithm::createInstance() const
{
  return new QgsExplodeHstoreAlgorithm();
}

void QgsExplodeHstoreAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ),
                QObject::tr( "HStore field" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "EXPECTED_FIELDS" ), QObject::tr( "Expected list of fields separated by a comma" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Exploded" ) ) );
}

QVariantMap QgsExplodeHstoreAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );
  int attrSourceCount = source->fields().count();

  QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  int fieldIndex = source->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid HStore field" ) );

  QStringList expectedFields;
  QString fieldList = parameterAsString( parameters, QStringLiteral( "EXPECTED_FIELDS" ), context );
  if ( ! fieldList.trimmed().isEmpty() )
  {
    expectedFields = fieldList.split( ',' );
  }

  QList<QString> fieldsToAdd;
  QHash<QgsFeatureId, QVariantMap> hstoreFeatures;
  QList<QgsFeature> features;

  double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 1;
  int i = 0;
  QgsFeatureIterator featIterator = source->getFeatures( );
  QgsFeature feat;
  while ( featIterator.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
      break;

    double progress = i * step;
    if ( progress >= 50 )
      feedback->setProgress( 50.0 );
    else
      feedback->setProgress( progress );

    QVariantMap currentHStore = QgsHstoreUtils::parse( feat.attribute( fieldName ).toString() );
    for ( auto key = currentHStore.keyBegin(); key != currentHStore.keyEnd(); key++ )
    {
      if ( expectedFields.isEmpty() && ! fieldsToAdd.contains( *key ) )
        fieldsToAdd.insert( 0, *key );
    }
    hstoreFeatures.insert( feat.id(), currentHStore );
    features.append( feat );
  }

  if ( ! expectedFields.isEmpty() )
  {
    fieldsToAdd = expectedFields;
  }

  QgsFields hstoreFields;
  for ( const QString &fieldName : fieldsToAdd )
  {
    hstoreFields.append( QgsField( fieldName, QVariant::String ) );
  }

  QgsFields outFields = QgsProcessingUtils::combineFields( source->fields(), hstoreFields );

  QString sinkId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, sinkId, outFields, source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QList<int> fieldIndicesInput = QgsProcessingUtils::fieldNamesToIndices( QStringList(), source->fields() );
  int attrCount = attrSourceCount + fieldsToAdd.count();
  QgsFeature outFeature;
  step = !features.empty() ? 50.0 / features.count() : 1;
  i = 0;
  for ( const QgsFeature &feat : std::as_const( features ) )
  {
    i++;
    if ( feedback->isCanceled() )
      break;

    feedback->setProgress( i * step + 50.0 );

    QgsAttributes outAttributes( attrCount );

    const QgsAttributes attrs( feat.attributes() );
    for ( int i = 0; i < fieldIndicesInput.count(); ++i )
      outAttributes[i] = attrs[fieldIndicesInput[i]];

    QVariantMap currentHStore = hstoreFeatures.take( feat.id() );

    QString current;
    for ( int i = 0; i < fieldsToAdd.count(); ++i )
    {
      current = fieldsToAdd.at( i );
      if ( currentHStore.contains( current ) )
      {
        outAttributes[attrSourceCount + i] = currentHStore.take( current );
      }
    }

    if ( ! expectedFields.isEmpty() )
    {
      outAttributes[fieldIndex] = QgsHstoreUtils::build( currentHStore );
    }

    outFeature.setGeometry( QgsGeometry( feat.geometry() ) );
    outFeature.setAttributes( outAttributes );
    if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), sinkId );
  return outputs;
}

///@endcond
