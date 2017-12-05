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

QgsProcessingAlgorithm::Flags QgsLineIntersectionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsLineIntersectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ),
                QObject::tr( "Intersect layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );

  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "INPUT_FIELDS" ),
                  QObject::tr( "Input fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any,
                  true, true ) );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "INTERSECT_FIELDS" ),
                  QObject::tr( "Intersect fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "INTERSECT" ), QgsProcessingParameterField::Any,
                  true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Intersections" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsLineIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates point features where the lines in the Intersect layer intersect the lines in the Input layer." );
}

QgsLineIntersectionAlgorithm *QgsLineIntersectionAlgorithm::createInstance() const
{
  return new QgsLineIntersectionAlgorithm();
}

QVariantMap QgsLineIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    return QVariantMap();

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  if ( !sourceB )
    return QVariantMap();

  const QStringList fieldsA = parameterAsFields( parameters, QStringLiteral( "INPUT_FIELDS" ), context );
  const QStringList fieldsB = parameterAsFields( parameters, QStringLiteral( "INTERSECT_FIELDS" ), context );

  QgsFields outFieldsA;
  QgsAttributeList fieldsAIndices;

  if ( fieldsA.empty() )
  {
    outFieldsA = sourceA->fields();
    for ( int i = 0; i < outFieldsA.count(); ++i )
    {
      fieldsAIndices << i;
    }
  }
  else
  {
    for ( const QString &field : fieldsA )
    {
      int index = sourceA->fields().lookupField( field );
      if ( index >= 0 )
      {
        fieldsAIndices << index;
        outFieldsA.append( sourceA->fields().at( index ) );
      }
    }
  }

  QgsFields outFieldsB;
  QgsAttributeList fieldsBIndices;

  if ( fieldsB.empty() )
  {
    outFieldsB = sourceB->fields();
    for ( int i = 0; i < outFieldsB.count(); ++i )
    {
      fieldsBIndices << i;
    }
  }
  else
  {
    for ( const QString &field : fieldsB )
    {
      int index = sourceB->fields().lookupField( field );
      if ( index >= 0 )
      {
        fieldsBIndices << index;
        outFieldsB.append( sourceB->fields().at( index ) );
      }
    }
  }

  QgsFields outFields = QgsProcessingUtils::combineFields( outFieldsA, outFieldsB );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields, QgsWkbTypes::Point,  sourceA->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsSpatialIndex spatialIndex( sourceB->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ).setDestinationCrs( sourceA->sourceCrs(), context.transformContext() ) ), feedback );
  QgsFeature outFeature;
  QgsFeatureIterator features = sourceA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( fieldsAIndices ) );
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
    QgsFeatureIds lines = spatialIndex.intersects( inGeom.boundingBox() ).toSet();
    if ( !lines.empty() )
    {
      // use prepared geometries for faster intersection tests
      std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( inGeom.constGet() ) );
      engine->prepareGeometry();

      QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( lines );
      request.setDestinationCrs( sourceA->sourceCrs(), context.transformContext() );
      request.setSubsetOfAttributes( fieldsBIndices );

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
          for ( int a : qgis::as_const( fieldsAIndices ) )
          {
            outAttributes.append( inFeatureA.attribute( a ) );
          }
          for ( int b : qgis::as_const( fieldsBIndices ) )
          {
            outAttributes.append( inFeatureB.attribute( b ) );
          }
          if ( intersectGeom.type() == QgsWkbTypes::PointGeometry )
          {
            if ( intersectGeom.isMultipart() )
            {
              points = intersectGeom.asMultiPoint();
            }
            else
            {
              points.append( intersectGeom.asPoint() );
            }

            for ( const QgsPointXY &j : qgis::as_const( points ) )
            {
              outFeature.setGeometry( QgsGeometry::fromPointXY( j ) );
              outFeature.setAttributes( outAttributes );
              sink->addFeature( outFeature, QgsFeatureSink::FastInsert );
            }
          }
        }
      }
    }

    feedback->setProgress( i * step );

  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond


