/***************************************************************************
                         qgsalgorithmremoveduplicatesbyattribute.cpp
                         ----------------------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmremoveduplicatesbyattribute.h"

///@cond PRIVATE

QString QgsRemoveDuplicatesByAttributeAlgorithm::name() const
{
  return QStringLiteral( "removeduplicatesbyattribute" );
}

QString QgsRemoveDuplicatesByAttributeAlgorithm::displayName() const
{
  return QObject::tr( "Delete duplicates by attribute" );
}

QStringList QgsRemoveDuplicatesByAttributeAlgorithm::tags() const
{
  return QObject::tr( "drop,remove,field,value,same,filter" ).split( ',' );
}

QString QgsRemoveDuplicatesByAttributeAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsRemoveDuplicatesByAttributeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

void QgsRemoveDuplicatesByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELDS" ), QObject::tr( "Field to match duplicates by" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Filtered (no duplicates)" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "DUPLICATES" ),  QObject::tr( "Filtered (duplicates)" ),
      QgsProcessing::TypeVectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "RETAINED_COUNT" ), QObject::tr( "Count of retained records" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "DUPLICATE_COUNT" ), QObject::tr( "Count of discarded duplicate records" ) ) );
}

QString QgsRemoveDuplicatesByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "Removes duplicate rows by a field value (or multiple field values). The first matching row will be retained, and duplicates will be discarded.\n\n"
                      "Optionally, these duplicate records can be saved to a separate output for analysis." );
}

QString QgsRemoveDuplicatesByAttributeAlgorithm::shortDescription() const
{
  return QObject::tr( "Removes duplicate rows by a field value (or multiple field values)." );
}

QgsRemoveDuplicatesByAttributeAlgorithm *QgsRemoveDuplicatesByAttributeAlgorithm::createInstance() const
{
  return new QgsRemoveDuplicatesByAttributeAlgorithm();
}

QVariantMap QgsRemoveDuplicatesByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QStringList fieldNames = parameterAsFields( parameters, QStringLiteral( "FIELDS" ), context );

  QgsAttributeList attributes;
  for ( const QString &field : fieldNames )
  {
    const int index = source->fields().lookupField( field );
    if ( index < 0 )
      feedback->reportError( QObject::tr( "Field %1 not found in INPUT layer, skipping" ).arg( field ) );
    else
      attributes.append( index );
  }
  if ( attributes.isEmpty() )
    throw QgsProcessingException( QObject::tr( "No input fields found" ) );


  QString noDupeSinkId;
  std::unique_ptr< QgsFeatureSink > noDupeSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, noDupeSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );
  if ( !noDupeSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString dupeSinkId;
  std::unique_ptr< QgsFeatureSink > dupesSink( parameterAsSink( parameters, QStringLiteral( "DUPLICATES" ), context, dupeSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );

  const long count = source->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  long long keptCount = 0;
  long long discardedCount = 0;

  QSet< QVariantList > matched;

  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature f;

  QVariantList dupeKey;
  dupeKey.reserve( attributes.size() );
  for ( const int i : attributes )
  {
    ( void )i;
    dupeKey.append( QVariant() );
  }

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    int i = 0;
    for ( const int attr : attributes )
      dupeKey[i++] = f.attribute( attr );

    if ( matched.contains( dupeKey ) )
    {
      // duplicate
      discardedCount++;
      if ( dupesSink )
      {
        if ( !dupesSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( dupesSink.get(), parameters, QStringLiteral( "DUPLICATES" ) ) );
      }
    }
    else
    {
      // not duplicate
      keptCount++;
      matched.insert( dupeKey );
      if ( !noDupeSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( noDupeSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "RETAINED_COUNT" ), keptCount );
  outputs.insert( QStringLiteral( "DUPLICATE_COUNT" ), discardedCount );
  outputs.insert( QStringLiteral( "OUTPUT" ), noDupeSinkId );
  if ( dupesSink )
    outputs.insert( QStringLiteral( "DUPLICATES" ), dupeSinkId );
  return outputs;
}

///@endcond


