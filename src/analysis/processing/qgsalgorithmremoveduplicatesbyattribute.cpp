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
  return u"removeduplicatesbyattribute"_s;
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
  return u"vectorgeneral"_s;
}

void QgsRemoveDuplicatesByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELDS"_s, QObject::tr( "Field to match duplicates by" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Filtered (no duplicates)" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( u"DUPLICATES"_s, QObject::tr( "Filtered (duplicates)" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );

  addOutput( new QgsProcessingOutputNumber( u"RETAINED_COUNT"_s, QObject::tr( "Count of retained records" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"DUPLICATE_COUNT"_s, QObject::tr( "Count of discarded duplicate records" ) ) );
}

QString QgsRemoveDuplicatesByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm removes duplicate rows by a field value (or multiple field values). The first matching row will be retained, and duplicates will be discarded.\n\n"
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
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QStringList fieldNames = parameterAsStrings( parameters, u"FIELDS"_s, context );

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
  std::unique_ptr<QgsFeatureSink> noDupeSink( parameterAsSink( parameters, u"OUTPUT"_s, context, noDupeSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !noDupeSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString dupeSinkId;
  std::unique_ptr<QgsFeatureSink> dupesSink( parameterAsSink( parameters, u"DUPLICATES"_s, context, dupeSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );

  const long count = source->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  long long keptCount = 0;
  long long discardedCount = 0;

  QSet<QVariantList> matched;

  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  QgsFeature f;

  QVariantList dupeKey;
  dupeKey.reserve( attributes.size() );
  for ( const int i : attributes )
  {
    ( void ) i;
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
          throw QgsProcessingException( writeFeatureError( dupesSink.get(), parameters, u"DUPLICATES"_s ) );
      }
    }
    else
    {
      // not duplicate
      keptCount++;
      matched.insert( dupeKey );
      if ( !noDupeSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( noDupeSink.get(), parameters, u"OUTPUT"_s ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  if ( noDupeSink )
    noDupeSink->finalize();

  QVariantMap outputs;
  outputs.insert( u"RETAINED_COUNT"_s, keptCount );
  outputs.insert( u"DUPLICATE_COUNT"_s, discardedCount );
  outputs.insert( u"OUTPUT"_s, noDupeSinkId );
  if ( dupesSink )
  {
    dupesSink->finalize();
    outputs.insert( u"DUPLICATES"_s, dupeSinkId );
  }
  return outputs;
}

///@endcond
