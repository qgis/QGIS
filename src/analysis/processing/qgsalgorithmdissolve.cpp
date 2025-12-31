/***************************************************************************
                         qgsalgorithmdissolve.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmdissolve.h"

///@cond PRIVATE

//
// QgsCollectorAlgorithm
//

QVariantMap QgsCollectorAlgorithm::processCollection( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, const std::function<QgsGeometry( const QVector<QgsGeometry> & )> &collector, int maxQueueLength, Qgis::ProcessingFeatureSourceFlags sourceFlags, bool separateDisjoint )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, source->fields(), QgsWkbTypes::multiType( source->wkbType() ), source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );

  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );

  const QStringList fields = parameterAsStrings( parameters, u"FIELD"_s, context );

  const long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), sourceFlags );

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( fields.isEmpty() )
  {
    // dissolve all - not using fields
    bool firstFeature = true;
    // we dissolve geometries in blocks using unaryUnion
    QVector<QgsGeometry> geomQueue;
    QgsFeature outputFeature;

    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( firstFeature )
      {
        outputFeature = f;
        firstFeature = false;
      }

      if ( f.hasGeometry() && !f.geometry().isEmpty() )
      {
        geomQueue.append( f.geometry() );
        if ( maxQueueLength > 0 && geomQueue.length() > maxQueueLength )
        {
          // queue too long, combine it
          const QgsGeometry tempOutputGeometry = collector( geomQueue );
          geomQueue.clear();
          geomQueue << tempOutputGeometry;
        }
      }

      feedback->setProgress( current * step );
      current++;
    }

    if ( geomQueue.isEmpty() )
    {
      outputFeature.clearGeometry();
      if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
    else if ( !separateDisjoint )
    {
      outputFeature.setGeometry( collector( geomQueue ) );
      if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
    else
    {
      const QgsGeometry combinedGeometry = collector( geomQueue );
      for ( auto it = combinedGeometry.const_parts_begin(); it != combinedGeometry.const_parts_end(); ++it )
      {
        QgsGeometry partGeom( ( ( *it )->clone() ) );
        partGeom.convertToMultiType();
        outputFeature.setGeometry( partGeom );
        if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
    }
  }
  else
  {
    QList<int> fieldIndexes;
    fieldIndexes.reserve( fields.size() );
    for ( const QString &field : fields )
    {
      const int index = source->fields().lookupField( field );
      if ( index >= 0 )
        fieldIndexes << index;
    }

    QHash<QVariant, QgsAttributes> attributeHash;
    QHash<QVariant, QVector<QgsGeometry>> geometryHash;

    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QVariantList indexAttributes;
      indexAttributes.reserve( fieldIndexes.size() );
      for ( const int index : std::as_const( fieldIndexes ) )
      {
        indexAttributes << f.attribute( index );
      }

      if ( !attributeHash.contains( indexAttributes ) )
      {
        // keep attributes of first feature
        attributeHash.insert( indexAttributes, f.attributes() );
      }

      if ( f.hasGeometry() && !f.geometry().isEmpty() )
      {
        geometryHash[indexAttributes].append( f.geometry() );
      }
    }

    const int numberFeatures = attributeHash.count();
    QHash<QVariant, QgsAttributes>::const_iterator attrIt = attributeHash.constBegin();
    for ( ; attrIt != attributeHash.constEnd(); ++attrIt )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QgsFeature outputFeature;
      outputFeature.setAttributes( attrIt.value() );
      auto geometryHashIt = geometryHash.find( attrIt.key() );
      if ( geometryHashIt != geometryHash.end() )
      {
        QgsGeometry geom = collector( geometryHashIt.value() );
        if ( !geom.isMultipart() )
        {
          geom.convertToMultiType();
        }
        if ( !separateDisjoint )
        {
          outputFeature.setGeometry( geom );
          if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
        else
        {
          for ( auto it = geom.const_parts_begin(); it != geom.const_parts_end(); ++it )
          {
            QgsGeometry partGeom( ( ( *it )->clone() ) );
            partGeom.convertToMultiType();
            outputFeature.setGeometry( partGeom );
            if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
          }
        }
      }
      else
      {
        if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }

      feedback->setProgress( current * 100.0 / numberFeatures );
      current++;
    }
  }

  sink->finalize();

  return outputs;
}


//
// QgsDissolveAlgorithm
//

QString QgsDissolveAlgorithm::name() const
{
  return u"dissolve"_s;
}

QString QgsDissolveAlgorithm::displayName() const
{
  return QObject::tr( "Dissolve" );
}

QStringList QgsDissolveAlgorithm::tags() const
{
  return QObject::tr( "dissolve,union,combine,collect" ).split( ',' );
}

QString QgsDissolveAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDissolveAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}


void QgsDissolveAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Dissolve field(s)" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  auto disjointParam = std::make_unique<QgsProcessingParameterBoolean>( u"SEPARATE_DISJOINT"_s, QObject::tr( "Keep disjoint features separate" ), false );
  disjointParam->setFlags( disjointParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( disjointParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Dissolved" ) ) );
}

QString QgsDissolveAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and combines their features into new features. One or more attributes can "
                      "be specified to dissolve features belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all features can be dissolved in a single one.\n\n"
                      "All output geometries will be converted to multi geometries. "
                      "In case the input is a polygon layer, common boundaries of adjacent polygons being dissolved will get erased.\n\n"
                      "If enabled, the optional \"Keep disjoint features separate\" setting will cause features and parts that do not overlap or touch to be exported "
                      "as separate features (instead of parts of a single multipart feature)." );
}

QString QgsDissolveAlgorithm::shortDescription() const
{
  return QObject::tr( "Combines features of a vector layer into new features, optionally grouped by common attributes." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsDissolveAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsDissolveAlgorithm *QgsDissolveAlgorithm::createInstance() const
{
  return new QgsDissolveAlgorithm();
}

QVariantMap QgsDissolveAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const bool separateDisjoint = parameterAsBool( parameters, u"SEPARATE_DISJOINT"_s, context );

  return processCollection( parameters, context, feedback, [&]( const QVector<QgsGeometry> &parts ) -> QgsGeometry {
    QgsGeometry result( QgsGeometry::unaryUnion( parts ) );
    if ( QgsWkbTypes::geometryType( result.wkbType() ) == Qgis::GeometryType::Line )
      result = result.mergeLines();
    // Geos may fail in some cases, let's try a slower but safer approach
    // See: https://github.com/qgis/QGIS/issues/28411 - Dissolve tool failing to produce outputs
    if ( ! result.lastError().isEmpty() && parts.count() >  2 )
    {
      if ( feedback->isCanceled() )
        return result;

      feedback->pushDebugInfo( QObject::tr( "GEOS exception: taking the slower route ..." ) );
      result = QgsGeometry();
      for ( const auto &p : parts )
      {
        result = QgsGeometry::unaryUnion( QVector< QgsGeometry >() << result << p );
        if ( QgsWkbTypes::geometryType( result.wkbType() ) == Qgis::GeometryType::Line )
          result = result.mergeLines();
        if ( feedback->isCanceled() )
          return result;
      }
    }
    if ( ! result.lastError().isEmpty() )
    {
      feedback->reportError( result.lastError(), true );
      if ( result.isEmpty() )
        throw QgsProcessingException( QObject::tr( "The algorithm returned no output." ) );
    }
    return result; }, 10000, Qgis::ProcessingFeatureSourceFlags(), separateDisjoint );
}

//
// QgsCollectAlgorithm
//

QString QgsCollectAlgorithm::name() const
{
  return u"collect"_s;
}

QString QgsCollectAlgorithm::displayName() const
{
  return QObject::tr( "Collect geometries" );
}

QStringList QgsCollectAlgorithm::tags() const
{
  return QObject::tr( "union,combine,collect,multipart,parts,single" ).split( ',' );
}

QString QgsCollectAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsCollectAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QVariantMap QgsCollectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processCollection( parameters, context, feedback, []( const QVector<QgsGeometry> &parts ) -> QgsGeometry { return QgsGeometry::collectGeometry( parts ); }, 0, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
}


void QgsCollectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Unique ID fields" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Collected" ) ) );
}

QString QgsCollectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and collects its geometries into new multipart geometries. One or more attributes can "
                      "be specified to collect only geometries belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all geometries can be collected." )
         + u"\n\n"_s + QObject::tr( "All output geometries will be converted to multi geometries, even those with just a single part. "
                                    "This algorithm does not dissolve overlapping geometries - they will be collected together without modifying the shape of each geometry part." )
         + u"\n\n"_s + QObject::tr( "See the 'Promote to multipart' or 'Aggregate' algorithms for alternative options." );
}

QString QgsCollectAlgorithm::shortDescription() const
{
  return QObject::tr( "Collects geometries of a vector layer into new multipart geometries, optionally grouped by common attributes." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsCollectAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsCollectAlgorithm *QgsCollectAlgorithm::createInstance() const
{
  return new QgsCollectAlgorithm();
}


///@endcond
