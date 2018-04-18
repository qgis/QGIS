/***************************************************************************
  qgsalgorithmintersection.cpp
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmintersection.h"

#include "qgsgeometrycollection.h"
#include "qgsgeometryengine.h"

///@cond PRIVATE


QString QgsIntersectionAlgorithm::name() const
{
  return QStringLiteral( "intersection" );
}

QString QgsIntersectionAlgorithm::displayName() const
{
  return QObject::tr( "Intersection" );
}

QString QgsIntersectionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsIntersectionAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the overlapping portions of features in the Input and Overlay layers. Features in the Overlay layer are assigned the attributes of the overlapping features from both the Input and Overlay layers." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Optionally, the rotation can occur around a preset point. If not set the rotation occurs around each feature's centroid." );
}

QgsProcessingAlgorithm *QgsIntersectionAlgorithm::createInstance() const
{
  return new QgsIntersectionAlgorithm();
}

void QgsIntersectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Intersection layer" ) ) );

  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "INPUT_FIELDS" ),
                  QObject::tr( "Input fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "OVERLAY_FIELDS" ),
                  QObject::tr( "Intersect fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "OVERLAY" ), QgsProcessingParameterField::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Intersection" ) ) );

}


QVariantMap QgsIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for OVERLAY" ) );

  QgsWkbTypes::Type geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  const QStringList fieldsA = parameterAsFields( parameters, QStringLiteral( "INPUT_FIELDS" ), context );
  const QStringList fieldsB = parameterAsFields( parameters, QStringLiteral( "OVERLAY_FIELDS" ), context );

  QgsFields fieldListA;
  QList<int> fieldIndicesA;
  if ( !fieldsA.isEmpty() )
  {
    for ( const QString &f : fieldsA )
    {
      int idxA = sourceA->fields().lookupField( f );
      if ( idxA >= 0 )
      {
        fieldIndicesA.append( idxA );
        fieldListA.append( sourceA->fields()[idxA] );
      }
    }
  }
  else
  {
    fieldListA = sourceA->fields();
    for ( int i = 0; i < fieldListA.count(); ++i )
      fieldIndicesA.append( i );
  }

  QgsFields fieldListB;
  QList<int> fieldIndicesB;
  if ( !fieldsB.isEmpty() )
  {
    for ( const QString &f : fieldsB )
    {
      int idxB = sourceB->fields().lookupField( f );
      if ( idxB >= 0 )
      {
        fieldIndicesB.append( idxB );
        fieldListB.append( sourceB->fields()[idxB] );
      }
    }
  }
  else
  {
    fieldListB = sourceB->fields();
    for ( int i = 0; i < fieldListB.count(); ++i )
      fieldIndicesB.append( i );
  }


  QgsFields outputFields = QgsProcessingUtils::combineFields( fieldListA, fieldListB );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, geomType, sourceA->sourceCrs() ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  request.setDestinationCrs( sourceA->sourceCrs(), context.transformContext() );

  QgsFeature outFeat;
  QgsSpatialIndex indexB( sourceB->getFeatures( request ), feedback );

  double total = 100.0 / ( sourceA->featureCount() ? sourceA->featureCount() : 1 );
  int count = 0;

  QgsFeature featA;
  QgsFeatureIterator fitA = sourceA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( fieldIndicesA ) );
  while ( fitA.nextFeature( featA ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !featA.hasGeometry() )
      continue;

    QgsGeometry geom( featA.geometry() );
    QgsFeatureIds intersects = indexB.intersects( geom.boundingBox() ).toSet();

    QgsFeatureRequest request;
    request.setFilterFids( intersects );
    request.setDestinationCrs( sourceA->sourceCrs(), context.transformContext() );
    request.setSubsetOfAttributes( fieldIndicesB );

    std::unique_ptr< QgsGeometryEngine > engine;
    if ( !intersects.isEmpty() )
    {
      // use prepared geometries for faster intersection tests
      engine.reset( QgsGeometry::createGeometryEngine( geom.constGet() ) );
      engine->prepareGeometry();
    }

    QgsAttributes outAttributes( outputFields.count() );
    const QgsAttributes attrsA( featA.attributes() );
    for ( int i = 0; i < fieldIndicesA.count(); ++i )
      outAttributes[i] = attrsA[fieldIndicesA[i]];

    QgsFeature featB;
    QgsFeatureIterator fitB = sourceB->getFeatures( request );
    while ( fitB.nextFeature( featB ) )
    {
      if ( feedback->isCanceled() )
        break;

      QgsGeometry tmpGeom( featB.geometry() );
      if ( !engine->intersects( tmpGeom.constGet() ) )
        continue;

      QgsGeometry intGeom = geom.intersection( tmpGeom );

      if ( intGeom.isNull() )
      {
        // TODO: not sure if this ever happens - if it does, that means GEOS failed badly - would be good to have a test for such situation
        throw QgsProcessingException( QStringLiteral( "%1\n\n%2" ).arg( QObject::tr( "GEOS geoprocessing error: intersection failed." ), intGeom.lastError() ) );
      }

      // Intersection of geometries may give use also geometries we do not want in our results.
      // For example, two square polygons touching at the corner have a point as the intersection, but no area.
      // In other cases we may get a mixture of geometries in the output - we want to keep only the expected types.
      if ( QgsWkbTypes::flatType( intGeom.wkbType() ) == QgsWkbTypes::GeometryCollection )
      {
        // try to filter out irrelevant parts with different geometry type than what we want
        intGeom.convertGeometryCollectionToSubclass( QgsWkbTypes::geometryType( geomType ) );
        if ( intGeom.isEmpty() )
          continue;
      }

      if ( QgsWkbTypes::geometryType( intGeom.wkbType() ) != QgsWkbTypes::geometryType( geomType ) )
      {
        // we can't make use of this resulting geometry
        continue;
      }

      const QgsAttributes attrsB( featB.attributes() );
      for ( int i = 0; i < fieldIndicesB.count(); ++i )
        outAttributes[fieldIndicesA.count() + i] = attrsB[fieldIndicesB[i]];

      intGeom.convertToMultiType();
      outFeat.setGeometry( intGeom );
      outFeat.setAttributes( outAttributes );
      sink->addFeature( outFeat, QgsFeatureSink::FastInsert );
    }

    ++count;
    feedback->setProgress( int( count * total ) );
  }

  return outputs;
}

///@endcond
