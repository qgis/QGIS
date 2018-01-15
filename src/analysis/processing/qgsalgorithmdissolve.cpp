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

QgsProcessingAlgorithm::Flags QgsCollectorAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QVariantMap QgsCollectorAlgorithm::processCollection( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback,
    const std::function<QgsGeometry( const QVector< QgsGeometry >& )> &collector, int maxQueueLength )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), QgsWkbTypes::multiType( source->wkbType() ), source->sourceCrs() ) );

  if ( !sink )
    return QVariantMap();

  QStringList fields = parameterAsFields( parameters, QStringLiteral( "FIELD" ), context );

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( fields.isEmpty() )
  {
    // dissolve all - not using fields
    bool firstFeature = true;
    // we dissolve geometries in blocks using unaryUnion
    QVector< QgsGeometry > geomQueue;
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

      if ( f.hasGeometry() && f.geometry() )
      {
        geomQueue.append( f.geometry() );
        if ( maxQueueLength > 0 && geomQueue.length() > maxQueueLength )
        {
          // queue too long, combine it
          QgsGeometry tempOutputGeometry = collector( geomQueue );
          geomQueue.clear();
          geomQueue << tempOutputGeometry;
        }
      }

      feedback->setProgress( current * step );
      current++;
    }

    outputFeature.setGeometry( collector( geomQueue ) );
    sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
  }
  else
  {
    QList< int > fieldIndexes;
    Q_FOREACH ( const QString &field, fields )
    {
      int index = source->fields().lookupField( field );
      if ( index >= 0 )
        fieldIndexes << index;
    }

    QHash< QVariant, QgsAttributes > attributeHash;
    QHash< QVariant, QVector< QgsGeometry > > geometryHash;

    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QVariantList indexAttributes;
      Q_FOREACH ( int index, fieldIndexes )
      {
        indexAttributes << f.attribute( index );
      }

      if ( !attributeHash.contains( indexAttributes ) )
      {
        // keep attributes of first feature
        attributeHash.insert( indexAttributes, f.attributes() );
      }

      if ( f.hasGeometry() && f.geometry() )
      {
        geometryHash[ indexAttributes ].append( f.geometry() );
      }
    }

    int numberFeatures = attributeHash.count();
    QHash< QVariant, QgsAttributes >::const_iterator attrIt = attributeHash.constBegin();
    for ( ; attrIt != attributeHash.constEnd(); ++attrIt )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QgsFeature outputFeature;
      if ( geometryHash.contains( attrIt.key() ) )
      {
        QgsGeometry geom = collector( geometryHash.value( attrIt.key() ) );
        if ( !geom.isMultipart() )
        {
          geom.convertToMultiType();
        }
        outputFeature.setGeometry( geom );
      }
      outputFeature.setAttributes( attrIt.value() );
      sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * 100.0 / numberFeatures );
      current++;
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


//
// QgsDissolveAlgorithm
//

QString QgsDissolveAlgorithm::name() const
{
  return QStringLiteral( "dissolve" );
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
  return QStringLiteral( "vectorgeometry" );
}


void QgsDissolveAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID fields" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Dissolved" ) ) );
}

QString QgsDissolveAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon or line vector layer and combines their geometries into new geometries. One or more attributes can "
                      "be specified to dissolve only geometries belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all geometries can be dissolved.\n\n"
                      "All output geometries will be converted to multi geometries. "
                      "In case the input is a polygon layer, common boundaries of adjacent polygons being dissolved will get erased." );
}

QgsDissolveAlgorithm *QgsDissolveAlgorithm::createInstance() const
{
  return new QgsDissolveAlgorithm();
}

QVariantMap QgsDissolveAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processCollection( parameters, context, feedback, []( const QVector< QgsGeometry > &parts )->QgsGeometry
  {
    return QgsGeometry::unaryUnion( parts );
  }, 10000 );
}

//
// QgsCollectAlgorithm
//

QString QgsCollectAlgorithm::name() const
{
  return QStringLiteral( "collect" );
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
  return QStringLiteral( "vectorgeometry" );
}

QVariantMap QgsCollectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processCollection( parameters, context, feedback, []( const QVector< QgsGeometry > &parts )->QgsGeometry
  {
    return QgsGeometry::collectGeometry( parts );
  } );
}


void QgsCollectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID fields" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Collected" ) ) );
}

QString QgsCollectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and collects its geometries into new multipart geometries. One or more attributes can "
                      "be specified to collect only geometries belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all geometries can be collected." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "All output geometries will be converted to multi geometries, even those with just a single part. "
                      "This algorithm does not dissolve overlapping geometries - they will be collected together without modifying the shape of each geometry part." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Promote to multipart' or 'Aggregate' algorithms for alternative options." );
}

QgsCollectAlgorithm *QgsCollectAlgorithm::createInstance() const
{
  return new QgsCollectAlgorithm();
}




///@endcond
