/***************************************************************************
                         qgsalgorithmlineintersection.cpp
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

#include "qgsalgorithmlineintersection.h"
#include "qgsgeometryengine.h"
#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsLineIntersectionAlgorithm::name() const
{
  return QStringLiteral( "lineintersections" );
}

QString QgsLineIntersectionAlgorithm::displayName() const
{
  return QObject::tr( "Line intersections" );
}

QStringList QgsLineIntersectionAlgorithm::tags() const
{
  return QObject::tr( "line,intersection" ).split( ',' );
}

QString QgsLineIntersectionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsLineIntersectionAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

void QgsLineIntersectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ), QObject::tr( "Intersect layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );

  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "INPUT_FIELDS" ),
    QObject::tr( "Input fields to keep (leave empty to keep all fields)" ), QVariant(),
    QStringLiteral( "INPUT" ), Qgis::ProcessingFieldParameterDataType::Any,
    true, true
  ) );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "INTERSECT_FIELDS" ),
    QObject::tr( "Intersect fields to keep (leave empty to keep all fields)" ), QVariant(),
    QStringLiteral( "INTERSECT" ), Qgis::ProcessingFieldParameterDataType::Any,
    true, true
  ) );

  std::unique_ptr<QgsProcessingParameterString> prefix = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "INTERSECT_FIELDS_PREFIX" ), QObject::tr( "Intersect fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Intersections" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

QString QgsLineIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates point features where the lines in the Intersect layer intersect the lines in the Input layer." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsLineIntersectionAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsLineIntersectionAlgorithm *QgsLineIntersectionAlgorithm::createInstance() const
{
  return new QgsLineIntersectionAlgorithm();
}

QVariantMap QgsLineIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr<QgsFeatureSource> sourceB( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INTERSECT" ) ) );

  const QStringList fieldsA = parameterAsStrings( parameters, QStringLiteral( "INPUT_FIELDS" ), context );
  const QStringList fieldsB = parameterAsStrings( parameters, QStringLiteral( "INTERSECT_FIELDS" ), context );

  QgsAttributeList fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( fieldsA, sourceA->fields() );
  QgsAttributeList fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( fieldsB, sourceB->fields() );

  QString intersectFieldsPrefix = parameterAsString( parameters, QStringLiteral( "INTERSECT_FIELDS_PREFIX" ), context );
  QgsFields outFields = QgsProcessingUtils::combineFields(
    QgsProcessingUtils::indicesToFields( fieldIndicesA, sourceA->fields() ),
    QgsProcessingUtils::indicesToFields( fieldIndicesB, sourceB->fields() ),
    intersectFieldsPrefix
  );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields, Qgis::WkbType::Point, sourceA->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsSpatialIndex spatialIndex( sourceB->getFeatures( QgsFeatureRequest().setNoAttributes().setDestinationCrs( sourceA->sourceCrs(), context.transformContext() ) ), feedback );
  QgsFeature outFeature;
  QgsFeatureIterator features = sourceA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( fieldIndicesA ) );
  double step = sourceA->featureCount() > 0 ? 100.0 / sourceA->featureCount() : 1;
  int i = 0;
  QgsFeature inFeatureA;
  while ( features.nextFeature( inFeatureA ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !inFeatureA.hasGeometry() )
      continue;

    QgsGeometry inGeom = inFeatureA.geometry();
    QgsFeatureIds lines = qgis::listToSet( spatialIndex.intersects( inGeom.boundingBox() ) );
    if ( !lines.empty() )
    {
      // use prepared geometries for faster intersection tests
      std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( inGeom.constGet() ) );
      engine->prepareGeometry();

      QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( lines );
      request.setDestinationCrs( sourceA->sourceCrs(), context.transformContext() );
      request.setSubsetOfAttributes( fieldIndicesB );

      QgsFeature inFeatureB;
      QgsFeatureIterator featuresB = sourceB->getFeatures( request );
      while ( featuresB.nextFeature( inFeatureB ) )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }

        QgsGeometry tmpGeom = inFeatureB.geometry();
        if ( engine->intersects( tmpGeom.constGet() ) )
        {
          QgsMultiPointXY points;
          QgsGeometry intersectGeom = inGeom.intersection( tmpGeom );
          QgsAttributes outAttributes;
          for ( int a : std::as_const( fieldIndicesA ) )
          {
            outAttributes.append( inFeatureA.attribute( a ) );
          }
          for ( int b : std::as_const( fieldIndicesB ) )
          {
            outAttributes.append( inFeatureB.attribute( b ) );
          }
          if ( QgsWkbTypes::flatType( intersectGeom.wkbType() ) == Qgis::WkbType::GeometryCollection )
          {
            const QVector<QgsGeometry> geomCollection = intersectGeom.asGeometryCollection();
            for ( const QgsGeometry &part : geomCollection )
            {
              if ( part.type() == Qgis::GeometryType::Point )
              {
                if ( part.isMultipart() )
                {
                  points = part.asMultiPoint();
                }
                else
                {
                  points.append( part.asPoint() );
                }
              }
            }
          }
          else if ( intersectGeom.type() == Qgis::GeometryType::Point )
          {
            if ( intersectGeom.isMultipart() )
            {
              points = intersectGeom.asMultiPoint();
            }
            else
            {
              points.append( intersectGeom.asPoint() );
            }
          }
          for ( const QgsPointXY &j : std::as_const( points ) )
          {
            outFeature.setGeometry( QgsGeometry::fromPointXY( j ) );
            outFeature.setAttributes( outAttributes );
            if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
          }
        }
      }
    }

    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
