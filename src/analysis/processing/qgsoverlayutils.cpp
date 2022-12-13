/***************************************************************************
  qgsoverlayutils.cpp
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

#include "qgsoverlayutils.h"

#include "qgsgeometryengine.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfeaturesource.h"
#include "qgsprocessingcontext.h"

///@cond PRIVATE

bool QgsOverlayUtils::sanitizeIntersectionResult( QgsGeometry &geom, QgsWkbTypes::GeometryType geometryType, SanitizeFlags flags )
{
  if ( geom.isNull() )
  {
    // TODO: not sure if this ever happens - if it does, that means GEOS failed badly - would be good to have a test for such situation
    throw QgsProcessingException( QStringLiteral( "%1\n\n%2" ).arg( QObject::tr( "GEOS geoprocessing error: intersection failed." ), geom.lastError() ) );
  }

  // Intersection of geometries may give use also geometries we do not want in our results.
  // For example, two square polygons touching at the corner have a point as the intersection, but no area.
  // In other cases we may get a mixture of geometries in the output - we want to keep only the expected types.
  if ( QgsWkbTypes::flatType( geom.wkbType() ) == QgsWkbTypes::GeometryCollection )
  {
    // try to filter out irrelevant parts with different geometry type than what we want
    geom.convertGeometryCollectionToSubclass( geometryType );
    if ( geom.isEmpty() )
      return false;
  }

  if ( QgsWkbTypes::geometryType( geom.wkbType() ) != geometryType )
  {
    // we can't make use of this resulting geometry
    return false;
  }

  if ( geometryType != QgsWkbTypes::GeometryType::PointGeometry
       || !( flags & SanitizeFlag::DontPromotePointGeometryToMultiPoint ) )
  {
    // some data providers are picky about the geometries we pass to them: we can't add single-part geometries
    // when we promised multi-part geometries, so ensure we have the right type
    geom.convertToMultiType();
  }

  return true;
}


//! Makes sure that what came out from difference of two geometries is good to be used in the output
static bool sanitizeDifferenceResult( QgsGeometry &geom, QgsWkbTypes::GeometryType geometryType, QgsOverlayUtils::SanitizeFlags flags )
{
  if ( geom.isNull() )
  {
    // TODO: not sure if this ever happens - if it does, that means GEOS failed badly - would be good to have a test for such situation
    throw QgsProcessingException( QStringLiteral( "%1\n\n%2" ).arg( QObject::tr( "GEOS geoprocessing error: difference failed." ), geom.lastError() ) );
  }

  //fix geometry collections
  if ( QgsWkbTypes::flatType( geom.wkbType() ) == QgsWkbTypes::GeometryCollection )
  {
    // try to filter out irrelevant parts with different geometry type than what we want
    geom.convertGeometryCollectionToSubclass( geometryType );
  }


  // if geomB covers the whole source geometry, we get an empty geometry collection
  if ( geom.isEmpty() )
    return false;

  if ( geometryType != QgsWkbTypes::GeometryType::PointGeometry
       || !( flags & QgsOverlayUtils::SanitizeFlag::DontPromotePointGeometryToMultiPoint ) )
  {
    // some data providers are picky about the geometries we pass to them: we can't add single-part geometries
    // when we promised multi-part geometries, so ensure we have the right type
    geom.convertToMultiType();
  }

  return true;
}


static QString writeFeatureError()
{
  return QObject::tr( "Could not write feature" );
}

void QgsOverlayUtils::difference( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback, long &count, long totalCount, QgsOverlayUtils::DifferenceOutput outputAttrs, const QgsGeometryParameters &parameters, SanitizeFlags flags )
{
  const QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::geometryType( QgsWkbTypes::multiType( sourceA.wkbType() ) );
  QgsFeatureRequest requestB;
  requestB.setNoAttributes();
  if ( outputAttrs != OutputBA )
    requestB.setDestinationCrs( sourceA.sourceCrs(), context.transformContext() );

  double step = sourceB.featureCount() > 0 ? 100.0 / static_cast< double >( sourceB.featureCount() ) : 1;
  long long i = 0;
  QgsFeatureIterator fi = sourceB.getFeatures( requestB );

  feedback->setProgressText( QObject::tr( "Creating spatial index" ) );
  const QgsSpatialIndex indexB( fi, [&]( const QgsFeature & )->bool
  {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( static_cast< double >( i ) * step );

    return true;
  } );

  if ( feedback->isCanceled() )
    return;

  const int fieldsCountA = sourceA.fields().count();
  const int fieldsCountB = sourceB.fields().count();
  QgsAttributes attrs;
  attrs.resize( outputAttrs == OutputA ? fieldsCountA : ( fieldsCountA + fieldsCountB ) );

  if ( totalCount == 0 )
    totalCount = 1;  // avoid division by zero

  feedback->setProgressText( QObject::tr( "Calculating difference" ) );

  QgsFeature featA;
  QgsFeatureRequest requestA;
  requestA.setInvalidGeometryCheck( context.invalidGeometryCheck() );
  if ( outputAttrs == OutputBA )
    requestA.setDestinationCrs( sourceB.sourceCrs(), context.transformContext() );
  QgsFeatureIterator fitA = sourceA.getFeatures( requestA );
  while ( fitA.nextFeature( featA ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( featA.hasGeometry() )
    {
      QgsGeometry geom( featA.geometry() );
      const QgsFeatureIds intersects = qgis::listToSet( indexB.intersects( geom.boundingBox() ) );

      QgsFeatureRequest request;
      request.setFilterFids( intersects );
      request.setNoAttributes();
      if ( outputAttrs != OutputBA )
        request.setDestinationCrs( sourceA.sourceCrs(), context.transformContext() );

      std::unique_ptr< QgsGeometryEngine > engine;
      if ( !intersects.isEmpty() )
      {
        // use prepared geometries for faster intersection tests
        engine.reset( QgsGeometry::createGeometryEngine( geom.constGet() ) );
        engine->prepareGeometry();
      }

      QVector<QgsGeometry> geometriesB;
      QgsFeature featB;
      QgsFeatureIterator fitB = sourceB.getFeatures( request );
      while ( fitB.nextFeature( featB ) )
      {
        if ( feedback->isCanceled() )
          break;

        if ( engine->intersects( featB.geometry().constGet() ) )
          geometriesB << featB.geometry();
      }

      if ( !geometriesB.isEmpty() )
      {
        const QgsGeometry geomB = QgsGeometry::unaryUnion( geometriesB, parameters );
        if ( !geomB.lastError().isEmpty() )
        {
          // This may happen if input geometries from a layer do not line up well (for example polygons
          // that are nearly touching each other, but there is a very tiny overlap or gap at one of the edges).
          // It is possible to get rid of this issue in two steps:
          // 1. snap geometries with a small tolerance (e.g. 1cm) using QgsGeometrySnapperSingleSource
          // 2. fix geometries (removes polygons collapsed to lines etc.) using MakeValid
          throw QgsProcessingException( QStringLiteral( "%1\n\n%2" ).arg( QObject::tr( "GEOS geoprocessing error: unary union failed." ), geomB.lastError() ) );
        }
        geom = geom.difference( geomB, parameters );
      }

      if ( !geom.isNull() && !sanitizeDifferenceResult( geom, geometryType, flags ) )
        continue;

      const QgsAttributes attrsA( featA.attributes() );
      switch ( outputAttrs )
      {
        case OutputA:
          attrs = attrsA;
          break;
        case OutputAB:
          for ( int i = 0; i < fieldsCountA; ++i )
            attrs[i] = attrsA[i];
          break;
        case OutputBA:
          for ( int i = 0; i < fieldsCountA; ++i )
            attrs[i + fieldsCountB] = attrsA[i];
          break;
      }

      QgsFeature outFeat;
      outFeat.setGeometry( geom );
      outFeat.setAttributes( attrs );
      if ( !sink.addFeature( outFeat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError() );
    }
    else
    {
      // TODO: should we write out features that do not have geometry?
      if ( !sink.addFeature( featA, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError() );
    }

    ++count;
    feedback->setProgress( count / static_cast< double >( totalCount ) * 100. );
  }
}


void QgsOverlayUtils::intersection( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback, long &count, long totalCount, const QList<int> &fieldIndicesA, const QList<int> &fieldIndicesB, const QgsGeometryParameters &parameters )
{
  const QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::geometryType( QgsWkbTypes::multiType( sourceA.wkbType() ) );
  const int attrCount = fieldIndicesA.count() + fieldIndicesB.count();

  QgsFeatureRequest request;
  request.setNoAttributes();
  request.setDestinationCrs( sourceA.sourceCrs(), context.transformContext() );

  QgsFeature outFeat;

  double step = sourceB.featureCount() > 0 ? 100.0 / static_cast< double >( sourceB.featureCount() ) : 1;
  long long i = 0;
  QgsFeatureIterator fi = sourceB.getFeatures( request );
  feedback->setProgressText( QObject::tr( "Creating spatial index" ) );
  const QgsSpatialIndex indexB( fi, [&]( const QgsFeature & )->bool
  {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( static_cast< double >( i ) * step );

    return true;
  } );

  if ( feedback->isCanceled() )
    return;

  if ( totalCount == 0 )
    totalCount = 1;  // avoid division by zero

  feedback->setProgressText( QObject::tr( "Calculating intersection" ) );

  QgsFeature featA;
  QgsFeatureIterator fitA = sourceA.getFeatures( QgsFeatureRequest().setSubsetOfAttributes( fieldIndicesA ) );
  while ( fitA.nextFeature( featA ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !featA.hasGeometry() )
      continue;

    const QgsGeometry geom( featA.geometry() );
    const QgsFeatureIds intersects = qgis::listToSet( indexB.intersects( geom.boundingBox() ) );

    QgsFeatureRequest request;
    request.setFilterFids( intersects );
    request.setDestinationCrs( sourceA.sourceCrs(), context.transformContext() );
    request.setSubsetOfAttributes( fieldIndicesB );

    std::unique_ptr< QgsGeometryEngine > engine;
    if ( !intersects.isEmpty() )
    {
      // use prepared geometries for faster intersection tests
      engine.reset( QgsGeometry::createGeometryEngine( geom.constGet() ) );
      engine->prepareGeometry();
    }

    QgsAttributes outAttributes( attrCount );
    const QgsAttributes attrsA( featA.attributes() );
    for ( int i = 0; i < fieldIndicesA.count(); ++i )
      outAttributes[i] = attrsA[fieldIndicesA[i]];

    QgsFeature featB;
    QgsFeatureIterator fitB = sourceB.getFeatures( request );
    while ( fitB.nextFeature( featB ) )
    {
      if ( feedback->isCanceled() )
        break;

      const QgsGeometry tmpGeom( featB.geometry() );
      if ( !engine->intersects( tmpGeom.constGet() ) )
        continue;

      QgsGeometry intGeom = geom.intersection( tmpGeom, parameters );
      if ( !sanitizeIntersectionResult( intGeom, geometryType ) )
        continue;

      const QgsAttributes attrsB( featB.attributes() );
      for ( int i = 0; i < fieldIndicesB.count(); ++i )
        outAttributes[fieldIndicesA.count() + i] = attrsB[fieldIndicesB[i]];

      outFeat.setGeometry( intGeom );
      outFeat.setAttributes( outAttributes );
      if ( !sink.addFeature( outFeat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError() );
    }

    ++count;
    feedback->setProgress( count / static_cast<double >( totalCount ) * 100. );
  }
}

void QgsOverlayUtils::resolveOverlaps( const QgsFeatureSource &source, QgsFeatureSink &sink, QgsProcessingFeedback *feedback, const QgsGeometryParameters &parameters, SanitizeFlags flags )
{
  long count = 0;
  const long totalCount = source.featureCount();
  if ( totalCount == 0 )
    return;  // nothing to do here

  QgsFeatureId newFid = -1;

  const QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::geometryType( QgsWkbTypes::multiType( source.wkbType() ) );

  QgsFeatureRequest requestOnlyGeoms;
  requestOnlyGeoms.setNoAttributes();

  QgsFeatureRequest requestOnlyAttrs;
  requestOnlyAttrs.setFlags( QgsFeatureRequest::NoGeometry );

  QgsFeatureRequest requestOnlyIds;
  requestOnlyIds.setFlags( QgsFeatureRequest::NoGeometry );
  requestOnlyIds.setNoAttributes();

  // make a set of used feature IDs so that we do not try to reuse them for newly added features
  QgsFeature f;
  QSet<QgsFeatureId> fids;
  QgsFeatureIterator it = source.getFeatures( requestOnlyIds );
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return;

    fids.insert( f.id() );
  }

  QHash<QgsFeatureId, QgsGeometry> geometries;
  QgsSpatialIndex index;
  QHash<QgsFeatureId, QList<QgsFeatureId> > intersectingIds;  // which features overlap a particular area

  // resolve intersections

  it = source.getFeatures( requestOnlyGeoms );
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return;

    const QgsFeatureId fid1 = f.id();
    QgsGeometry g1 = f.geometry();
    std::unique_ptr< QgsGeometryEngine > g1engine;

    geometries.insert( fid1, g1 );
    index.addFeature( f );

    const QgsRectangle bbox( f.geometry().boundingBox() );
    const QList<QgsFeatureId> ids = index.intersects( bbox );
    for ( const QgsFeatureId fid2 : ids )
    {
      if ( fid1 == fid2 )
        continue;

      if ( !g1engine )
      {
        // use prepared geometries for faster intersection tests
        g1engine.reset( QgsGeometry::createGeometryEngine( g1.constGet() ) );
        g1engine->prepareGeometry();
      }

      const QgsGeometry g2 = geometries.value( fid2 );
      if ( !g1engine->intersects( g2.constGet() ) )
        continue;

      QgsGeometry geomIntersection = g1.intersection( g2, parameters );
      if ( !sanitizeIntersectionResult( geomIntersection, geometryType ) )
        continue;

      //
      // add intersection geometry
      //

      // figure out new fid
      while ( fids.contains( newFid ) )
        --newFid;
      fids.insert( newFid );

      geometries.insert( newFid, geomIntersection );
      QgsFeature fx( newFid );
      fx.setGeometry( geomIntersection );

      index.addFeature( fx );

      // figure out which feature IDs belong to this intersection. Some of the IDs can be of the newly
      // created geometries - in such case we need to retrieve original IDs
      QList<QgsFeatureId> lst;
      if ( intersectingIds.contains( fid1 ) )
        lst << intersectingIds.value( fid1 );
      else
        lst << fid1;
      if ( intersectingIds.contains( fid2 ) )
        lst << intersectingIds.value( fid2 );
      else
        lst << fid2;
      intersectingIds.insert( newFid, lst );

      //
      // update f1
      //

      QgsGeometry g12 = g1.difference( g2, parameters );

      index.deleteFeature( f );
      geometries.remove( fid1 );

      if ( sanitizeDifferenceResult( g12, geometryType, flags ) )
      {
        geometries.insert( fid1, g12 );

        QgsFeature f1x( fid1 );
        f1x.setGeometry( g12 );
        index.addFeature( f1x );
      }

      //
      // update f2
      //

      QgsGeometry g21 = g2.difference( g1, parameters );

      QgsFeature f2old( fid2 );
      f2old.setGeometry( g2 );
      index.deleteFeature( f2old );

      geometries.remove( fid2 );

      if ( sanitizeDifferenceResult( g21, geometryType, flags ) )
      {
        geometries.insert( fid2, g21 );

        QgsFeature f2x( fid2 );
        f2x.setGeometry( g21 );
        index.addFeature( f2x );
      }

      // update our temporary copy of the geometry to what is left from it
      g1 = g12;
      g1engine.reset();
    }

    ++count;
    feedback->setProgress( count / static_cast< double >( totalCount ) * 100. );
  }
  if ( feedback->isCanceled() )
    return;

  // release some memory of structures we don't need anymore

  fids.clear();
  index = QgsSpatialIndex();

  // load attributes

  QHash<QgsFeatureId, QgsAttributes> attributesHash;
  it = source.getFeatures( requestOnlyAttrs );
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return;

    attributesHash.insert( f.id(), f.attributes() );
  }

  // store stuff in the sink

  for ( auto i = geometries.constBegin(); i != geometries.constEnd(); ++i )
  {
    if ( feedback->isCanceled() )
      return;

    QgsFeature outFeature( i.key() );
    outFeature.setGeometry( i.value() );

    if ( intersectingIds.contains( i.key() ) )
    {
      const QList<QgsFeatureId> ids = intersectingIds.value( i.key() );
      for ( const QgsFeatureId id : ids )
      {
        outFeature.setAttributes( attributesHash.value( id ) );
        if ( !sink.addFeature( outFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError() );
      }
    }
    else
    {
      outFeature.setAttributes( attributesHash.value( i.key() ) );
      if ( !sink.addFeature( outFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError() );
    }
  }
}

///@endcond PRIVATE
