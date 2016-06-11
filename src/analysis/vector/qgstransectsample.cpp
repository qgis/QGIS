/***************************************************************************
    qgstransectsample.cpp
    ---------------------
    begin                : July 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstransectsample.h"
#include "qgsdistancearea.h"
#include "qgsgeometry.h"
#include "qgsspatialindex.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include <QProgressDialog>
#include <QFileInfo>
#ifndef _MSC_VER
#include <stdint.h>
#endif
#include "mersenne-twister.h"
#include <limits>

QgsTransectSample::QgsTransectSample( QgsVectorLayer* strataLayer, const QString& strataIdAttribute, const QString& minDistanceAttribute, const QString& nPointsAttribute, DistanceUnits minDistUnits,
                                      QgsVectorLayer* baselineLayer, bool shareBaseline, const QString& baselineStrataId, const QString& outputPointLayer,
                                      const QString& outputLineLayer, const QString& usedBaselineLayer, double minTransectLength,
                                      double baselineBufferDistance, double baselineSimplificationTolerance )
    : mStrataLayer( strataLayer )
    , mStrataIdAttribute( strataIdAttribute )
    , mMinDistanceAttribute( minDistanceAttribute )
    , mNPointsAttribute( nPointsAttribute )
    , mBaselineLayer( baselineLayer )
    , mShareBaseline( shareBaseline )
    , mBaselineStrataId( baselineStrataId )
    , mOutputPointLayer( outputPointLayer )
    , mOutputLineLayer( outputLineLayer )
    , mUsedBaselineLayer( usedBaselineLayer )
    , mMinDistanceUnits( minDistUnits )
    , mMinTransectLength( minTransectLength )
    , mBaselineBufferDistance( baselineBufferDistance )
    , mBaselineSimplificationTolerance( baselineSimplificationTolerance )
{
}

QgsTransectSample::QgsTransectSample()
    : mStrataLayer( nullptr )
    , mBaselineLayer( nullptr )
    , mShareBaseline( false )
    , mMinDistanceUnits( Meters )
    , mMinTransectLength( 0.0 )
    , mBaselineBufferDistance( -1.0 )
    , mBaselineSimplificationTolerance( -1.0 )
{
}

int QgsTransectSample::createSample( QProgressDialog* pd )
{
  Q_UNUSED( pd );

  if ( !mStrataLayer || !mStrataLayer->isValid() )
  {
    return 1;
  }

  if ( !mBaselineLayer || !mBaselineLayer->isValid() )
  {
    return 2;
  }

  //stratum id is not necessarily an integer
  QVariant::Type stratumIdType = QVariant::Int;
  if ( !mStrataIdAttribute.isEmpty() )
  {
    stratumIdType = mStrataLayer->fields().field( mStrataIdAttribute ).type();
  }

  //create vector file writers for output
  QgsFields outputPointFields;
  outputPointFields.append( QgsField( "id", stratumIdType ) );
  outputPointFields.append( QgsField( "station_id", QVariant::Int ) );
  outputPointFields.append( QgsField( "stratum_id", stratumIdType ) );
  outputPointFields.append( QgsField( "station_code", QVariant::String ) );
  outputPointFields.append( QgsField( "start_lat", QVariant::Double ) );
  outputPointFields.append( QgsField( "start_long", QVariant::Double ) );

  QgsVectorFileWriter outputPointWriter( mOutputPointLayer, "utf-8", outputPointFields, QGis::WKBPoint,
                                         &( mStrataLayer->crs() ) );
  if ( outputPointWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 3;
  }

  outputPointFields.append( QgsField( "bearing", QVariant::Double ) ); //add bearing attribute for lines
  QgsVectorFileWriter outputLineWriter( mOutputLineLayer, "utf-8", outputPointFields, QGis::WKBLineString,
                                        &( mStrataLayer->crs() ) );
  if ( outputLineWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 4;
  }

  QgsFields usedBaselineFields;
  usedBaselineFields.append( QgsField( "stratum_id", stratumIdType ) );
  usedBaselineFields.append( QgsField( "ok", QVariant::String ) );
  QgsVectorFileWriter usedBaselineWriter( mUsedBaselineLayer, "utf-8", usedBaselineFields, QGis::WKBLineString,
                                          &( mStrataLayer->crs() ) );
  if ( usedBaselineWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 5;
  }

  //debug: write clipped buffer bounds with stratum id to same directory as out_point
  QFileInfo outputPointInfo( mOutputPointLayer );
  QString bufferClipLineOutput = outputPointInfo.absolutePath() + "/out_buffer_clip_line.shp";
  QgsFields bufferClipLineFields;
  bufferClipLineFields.append( QgsField( "id", stratumIdType ) );
  QgsVectorFileWriter bufferClipLineWriter( bufferClipLineOutput, "utf-8", bufferClipLineFields, QGis::WKBLineString, &( mStrataLayer->crs() ) );

  //configure distanceArea depending on minDistance units and output CRS
  QgsDistanceArea distanceArea;
  distanceArea.setSourceCrs( mStrataLayer->crs().srsid() );
  if ( mMinDistanceUnits == Meters )
  {
    distanceArea.setEllipsoidalMode( true );
  }
  else
  {
    distanceArea.setEllipsoidalMode( false );
  }

  //possibility to transform output points to lat/long
  QgsCoordinateTransform toLatLongTransform( mStrataLayer->crs(), QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId ) );

  //init random number generator
  mt_srand( QTime::currentTime().msec() );

  QgsFeatureRequest fr;
  fr.setSubsetOfAttributes( QStringList() << mStrataIdAttribute << mMinDistanceAttribute << mNPointsAttribute, mStrataLayer->fields() );
  QgsFeatureIterator strataIt = mStrataLayer->getFeatures( fr );

  QgsFeature fet;
  int nTotalTransects = 0;
  int nFeatures = 0;

  if ( pd )
  {
    pd->setMaximum( mStrataLayer->featureCount() );
  }

  while ( strataIt.nextFeature( fet ) )
  {
    if ( pd )
    {
      pd->setValue( nFeatures );
    }
    if ( pd && pd->wasCanceled() )
    {
      break;
    }

    if ( !fet.constGeometry() )
    {
      continue;
    }
    const QgsGeometry* strataGeom = fet.constGeometry();

    //find baseline for strata
    QVariant strataId = fet.attribute( mStrataIdAttribute );
    QgsGeometry* baselineGeom = findBaselineGeometry( strataId.isValid() ? strataId : -1 );
    if ( !baselineGeom )
    {
      continue;
    }

    double minDistance = fet.attribute( mMinDistanceAttribute ).toDouble();
    double minDistanceLayerUnits = minDistance;
    //if minDistance is in meters and the data in degrees, we need to apply a rough conversion for the buffer distance
    double bufferDist = bufferDistance( minDistance );
    if ( mMinDistanceUnits == Meters && mStrataLayer->crs().mapUnits() == QGis::DecimalDegrees )
    {
      minDistanceLayerUnits = minDistance / 111319.9;
    }

    QgsGeometry* clippedBaseline = strataGeom->intersection( baselineGeom );
    if ( !clippedBaseline || clippedBaseline->wkbType() == QGis::WKBUnknown )
    {
      delete clippedBaseline;
      continue;
    }
    QgsGeometry* bufferLineClipped = clipBufferLine( strataGeom, clippedBaseline, bufferDist );
    if ( !bufferLineClipped )
    {
      delete clippedBaseline;
      continue;
    }

    //save clipped baseline to file
    QgsFeature blFeature( usedBaselineFields );
    blFeature.setGeometry( *clippedBaseline );
    blFeature.setAttribute( "stratum_id", strataId );
    blFeature.setAttribute( "ok", "f" );
    usedBaselineWriter.addFeature( blFeature );

    //start loop to create random points along the baseline
    int nTransects = fet.attribute( mNPointsAttribute ).toInt();
    int nCreatedTransects = 0;
    int nIterations = 0;
    int nMaxIterations = nTransects * 50;

    QgsSpatialIndex sIndex; //to check minimum distance
    QMap< QgsFeatureId, QgsGeometry* > lineFeatureMap;

    while ( nCreatedTransects < nTransects && nIterations < nMaxIterations )
    {
      double randomPosition = (( double )mt_rand() / MD_RAND_MAX ) * clippedBaseline->length();
      QgsGeometry* samplePoint = clippedBaseline->interpolate( randomPosition );
      ++nIterations;
      if ( !samplePoint )
      {
        continue;
      }
      QgsPoint sampleQgsPoint = samplePoint->asPoint();
      QgsPoint latLongSamplePoint = toLatLongTransform.transform( sampleQgsPoint );

      QgsFeature samplePointFeature( outputPointFields );
      samplePointFeature.setGeometry( samplePoint );
      samplePointFeature.setAttribute( "id", nTotalTransects + 1 );
      samplePointFeature.setAttribute( "station_id", nCreatedTransects + 1 );
      samplePointFeature.setAttribute( "stratum_id", strataId );
      samplePointFeature.setAttribute( "station_code", strataId.toString() + '_' + QString::number( nCreatedTransects + 1 ) );
      samplePointFeature.setAttribute( "start_lat", latLongSamplePoint.y() );
      samplePointFeature.setAttribute( "start_long", latLongSamplePoint.x() );

      //find closest point on clipped buffer line
      QgsPoint minDistPoint;

      int afterVertex;
      if ( bufferLineClipped->closestSegmentWithContext( sampleQgsPoint, minDistPoint, afterVertex ) < 0 )
      {
        continue;
      }

      //bearing between sample point and min dist point (transect direction)
      double bearing = distanceArea.bearing( sampleQgsPoint, minDistPoint ) / M_PI * 180.0;

      QgsPolyline sampleLinePolyline;
      QgsPoint ptFarAway( sampleQgsPoint.x() + ( minDistPoint.x() - sampleQgsPoint.x() ) * 1000000,
                          sampleQgsPoint.y() + ( minDistPoint.y() - sampleQgsPoint.y() ) * 1000000 );
      QgsPolyline lineFarAway;
      lineFarAway << sampleQgsPoint << ptFarAway;
      QgsGeometry* lineFarAwayGeom = QgsGeometry::fromPolyline( lineFarAway );
      QgsGeometry* lineClipStratum = lineFarAwayGeom->intersection( strataGeom );
      if ( !lineClipStratum )
      {
        delete lineFarAwayGeom;
        delete lineClipStratum;
        continue;
      }

      //cancel if distance between sample point and line is too large (line does not start at point
      if ( lineClipStratum->distance( *samplePoint ) > 0.000001 )
      {
        delete lineFarAwayGeom;
        delete lineClipStratum;
        continue;
      }

      //if lineClipStratum is a multiline, take the part line closest to sampleQgsPoint
      if ( lineClipStratum->wkbType() == QGis::WKBMultiLineString
           || lineClipStratum->wkbType() == QGis::WKBMultiLineString25D )
      {
        QgsGeometry* singleLine = closestMultilineElement( sampleQgsPoint, lineClipStratum );
        if ( singleLine )
        {
          delete lineClipStratum;
          lineClipStratum = singleLine;
        }
      }

      //cancel if length of lineClipStratum is too small
      double transectLength = distanceArea.measureLength( lineClipStratum );
      if ( transectLength < mMinTransectLength )
      {
        delete lineFarAwayGeom;
        delete lineClipStratum;
        continue;
      }

      //search closest existing profile. Cancel if dist < minDist
      if ( otherTransectWithinDistance( lineClipStratum, minDistanceLayerUnits, minDistance, sIndex, lineFeatureMap, distanceArea ) )
      {
        delete lineFarAwayGeom;
        delete lineClipStratum;
        continue;
      }

      QgsFeatureId fid( nCreatedTransects );
      QgsFeature sampleLineFeature( outputPointFields, fid );
      sampleLineFeature.setGeometry( lineClipStratum );
      sampleLineFeature.setAttribute( "id", nTotalTransects + 1 );
      sampleLineFeature.setAttribute( "station_id", nCreatedTransects + 1 );
      sampleLineFeature.setAttribute( "stratum_id", strataId );
      sampleLineFeature.setAttribute( "station_code", strataId.toString() + '_' + QString::number( nCreatedTransects + 1 ) );
      sampleLineFeature.setAttribute( "start_lat", latLongSamplePoint.y() );
      sampleLineFeature.setAttribute( "start_long", latLongSamplePoint.x() );
      sampleLineFeature.setAttribute( "bearing", bearing );
      outputLineWriter.addFeature( sampleLineFeature );

      //add point to file writer here.
      //It can only be written if the corresponding transect has been as well
      outputPointWriter.addFeature( samplePointFeature );

      sIndex.insertFeature( sampleLineFeature );
      Q_NOWARN_DEPRECATED_PUSH
      lineFeatureMap.insert( fid, sampleLineFeature.geometryAndOwnership() );
      Q_NOWARN_DEPRECATED_POP

      delete lineFarAwayGeom;
      ++nTotalTransects;
      ++nCreatedTransects;
    }
    delete clippedBaseline;

    QgsFeature bufferClipFeature;
    bufferClipFeature.setGeometry( bufferLineClipped );
    bufferClipFeature.setAttribute( "id", strataId );
    bufferClipLineWriter.addFeature( bufferClipFeature );
    //delete bufferLineClipped;

    //delete all line geometries in spatial index
    QMap< QgsFeatureId, QgsGeometry* >::iterator featureMapIt = lineFeatureMap.begin();
    for ( ; featureMapIt != lineFeatureMap.end(); ++featureMapIt )
    {
      delete( featureMapIt.value() );
    }
    lineFeatureMap.clear();
    delete baselineGeom;

    ++nFeatures;
  }

  if ( pd )
  {
    pd->setValue( mStrataLayer->featureCount() );
  }

  return 0;
}

QgsGeometry* QgsTransectSample::findBaselineGeometry( const QVariant& strataId )
{
  if ( !mBaselineLayer )
  {
    return nullptr;
  }

  QgsFeatureIterator baseLineIt = mBaselineLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QStringList( mBaselineStrataId ), mBaselineLayer->fields() ) );
  QgsFeature fet;

  while ( baseLineIt.nextFeature( fet ) ) //todo: cache this in case there are many baslines
  {
    if ( strataId == fet.attribute( mBaselineStrataId ) || mShareBaseline )
    {
      Q_NOWARN_DEPRECATED_PUSH
      return fet.geometryAndOwnership();
      Q_NOWARN_DEPRECATED_POP
    }
  }
  return nullptr;
}

bool QgsTransectSample::otherTransectWithinDistance( QgsGeometry* geom, double minDistLayerUnit, double minDistance, QgsSpatialIndex& sIndex,
    const QMap< QgsFeatureId, QgsGeometry* >& lineFeatureMap, QgsDistanceArea& da )
{
  if ( !geom )
  {
    return false;
  }

  QgsGeometry* buffer = geom->buffer( minDistLayerUnit, 8 );
  if ( !buffer )
  {
    return false;
  }
  QgsRectangle rect = buffer->boundingBox();
  QList<QgsFeatureId> lineIdList = sIndex.intersects( rect );

  QList<QgsFeatureId>::const_iterator lineIdIt = lineIdList.constBegin();
  for ( ; lineIdIt != lineIdList.constEnd(); ++lineIdIt )
  {
    const QMap< QgsFeatureId, QgsGeometry* >::const_iterator idMapIt = lineFeatureMap.find( *lineIdIt );
    if ( idMapIt != lineFeatureMap.constEnd() )
    {
      double dist = 0;
      QgsPoint pt1, pt2;
      closestSegmentPoints( *geom, *( idMapIt.value() ), dist, pt1, pt2 );
      dist = da.measureLine( pt1, pt2 ); //convert degrees to meters if necessary

      if ( dist < minDistance )
      {
        delete buffer;
        return true;
      }
    }
  }

  delete buffer;
  return false;
}

bool QgsTransectSample::closestSegmentPoints( QgsGeometry& g1, QgsGeometry& g2, double& dist, QgsPoint& pt1, QgsPoint& pt2 )
{
  QGis::WkbType t1 = g1.wkbType();
  if ( t1 != QGis::WKBLineString && t1 != QGis::WKBLineString25D )
  {
    return false;
  }

  QGis::WkbType t2 = g2.wkbType();
  if ( t2 != QGis::WKBLineString && t2 != QGis::WKBLineString25D )
  {
    return false;
  }

  QgsPolyline pl1 = g1.asPolyline();
  QgsPolyline pl2 = g2.asPolyline();

  if ( pl1.size() < 2 || pl2.size() < 2 )
  {
    return false;
  }

  QgsPoint p11 = pl1.at( 0 );
  QgsPoint p12 = pl1.at( 1 );
  QgsPoint p21 = pl2.at( 0 );
  QgsPoint p22 = pl2.at( 1 );

  double p1x = p11.x();
  double p1y = p11.y();
  double v1x = p12.x() - p11.x();
  double v1y = p12.y() - p11.y();
  double p2x = p21.x();
  double p2y = p21.y();
  double v2x = p22.x() - p21.x();
  double v2y = p22.y() - p21.y();

  double denominatorU = v2x * v1y - v2y * v1x;
  double denominatorT = v1x * v2y - v1y * v2x;

  if ( qgsDoubleNear( denominatorU, 0 ) || qgsDoubleNear( denominatorT, 0 ) )
  {
    //lines are parallel
    //project all points on the other segment and take the one with the smallest distance
    QgsPoint minDistPoint1;
    double d1 = p11.sqrDistToSegment( p21.x(), p21.y(), p22.x(), p22.y(), minDistPoint1 );
    QgsPoint minDistPoint2;
    double d2 = p12.sqrDistToSegment( p21.x(), p21.y(), p22.x(), p22.y(), minDistPoint2 );
    QgsPoint minDistPoint3;
    double d3 = p21.sqrDistToSegment( p11.x(), p11.y(), p12.x(), p12.y(), minDistPoint3 );
    QgsPoint minDistPoint4;
    double d4 = p22.sqrDistToSegment( p11.x(), p11.y(), p12.x(), p12.y(), minDistPoint4 );

    if ( d1 <= d2 && d1 <= d3 && d1 <= d4 )
    {
      dist = sqrt( d1 );
      pt1 = p11;
      pt2 = minDistPoint1;
      return true;
    }
    else if ( d2 <= d1 && d2 <= d3 && d2 <= d4 )
    {
      dist = sqrt( d2 );
      pt1 = p12;
      pt2 = minDistPoint2;
      return true;
    }
    else if ( d3 <= d1 && d3 <= d2 && d3 <= d4 )
    {
      dist = sqrt( d3 );
      pt1 = p21;
      pt2 = minDistPoint3;
      return true;
    }
    else
    {
      dist = sqrt( d4 );
      pt1 = p21;
      pt2 = minDistPoint4;
      return true;
    }
  }

  double u = ( p1x * v1y - p1y * v1x - p2x * v1y + p2y * v1x ) / denominatorU;
  double t = ( p2x * v2y - p2y * v2x - p1x * v2y + p1y * v2x ) / denominatorT;

  if ( u >= 0 && u <= 1.0 && t >= 0 && t <= 1.0 )
  {
    dist = 0;
    pt1.setX( p2x + u * v2x );
    pt1.setY( p2y + u * v2y );
    pt2 = pt1;
    dist = 0;
    return true;
  }

  if ( t > 1.0 )
  {
    pt1.setX( p12.x() );
    pt1.setY( p12.y() );
  }
  else if ( t < 0.0 )
  {
    pt1.setX( p11.x() );
    pt1.setY( p11.y() );
  }
  if ( u > 1.0 )
  {
    pt2.setX( p22.x() );
    pt2.setY( p22.y() );
  }
  if ( u < 0.0 )
  {
    pt2.setX( p21.x() );
    pt2.setY( p21.y() );
  }
  if ( t >= 0.0 && t <= 1.0 )
  {
    //project pt2 onto g1
    pt2.sqrDistToSegment( p11.x(), p11.y(), p12.x(), p12.y(), pt1 );
  }
  if ( u >= 0.0 && u <= 1.0 )
  {
    //project pt1 onto g2
    pt1.sqrDistToSegment( p21.x(), p21.y(), p22.x(), p22.y(), pt2 );
  }

  dist = sqrt( pt1.sqrDist( pt2 ) );
  return true;
}

QgsGeometry* QgsTransectSample::closestMultilineElement( const QgsPoint& pt, QgsGeometry* multiLine )
{
  if ( !multiLine || ( multiLine->wkbType() != QGis::WKBMultiLineString
                       && multiLine->wkbType() != QGis::WKBMultiLineString25D ) )
  {
    return nullptr;
  }

  double minDist = DBL_MAX;
  double currentDist = 0;
  QgsGeometry* currentLine = nullptr;
  QScopedPointer<QgsGeometry> closestLine;
  QgsGeometry* pointGeom = QgsGeometry::fromPoint( pt );

  QgsMultiPolyline multiPolyline = multiLine->asMultiPolyline();
  QgsMultiPolyline::const_iterator it = multiPolyline.constBegin();
  for ( ; it != multiPolyline.constEnd(); ++it )
  {
    currentLine = QgsGeometry::fromPolyline( *it );
    currentDist = pointGeom->distance( *currentLine );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      closestLine.reset( currentLine );
    }
    else
    {
      delete currentLine;
    }
  }

  delete pointGeom;
  return closestLine.take();
}

QgsGeometry* QgsTransectSample::clipBufferLine( const QgsGeometry* stratumGeom, QgsGeometry* clippedBaseline, double tolerance )
{
  if ( !stratumGeom || !clippedBaseline || clippedBaseline->wkbType() == QGis::WKBUnknown )
  {
    return nullptr;
  }

  QgsGeometry* usedBaseline = clippedBaseline;
  if ( mBaselineSimplificationTolerance >= 0 )
  {
    //int verticesBefore = usedBaseline->asMultiPolyline().count();
    usedBaseline = clippedBaseline->simplify( mBaselineSimplificationTolerance );
    if ( !usedBaseline )
    {
      return nullptr;
    }
    //int verticesAfter = usedBaseline->asMultiPolyline().count();

    //debug: write to file
    /*QgsVectorFileWriter debugWriter( "/tmp/debug.shp", "utf-8", QgsFields(), QGis::WKBLineString, &( mStrataLayer->crs() ) );
    QgsFeature debugFeature; debugFeature.setGeometry( usedBaseline );
    debugWriter.addFeature( debugFeature );*/
  }

  double currentBufferDist = tolerance;
  int maxLoops = 10;

  for ( int i = 0; i < maxLoops; ++i )
  {
    //loop with tolerance: create buffer, convert buffer to line, clip line by stratum, test if result is (single) line
    QgsGeometry* clipBaselineBuffer = usedBaseline->buffer( currentBufferDist, 8 );
    if ( !clipBaselineBuffer )
    {
      delete clipBaselineBuffer;
      continue;
    }

    //it is also possible that clipBaselineBuffer is a multipolygon
    QgsGeometry* bufferLine = nullptr; //buffer line or multiline
    QgsGeometry* bufferLineClipped = nullptr;
    QgsMultiPolyline mpl;
    if ( clipBaselineBuffer->isMultipart() )
    {
      QgsMultiPolygon bufferMultiPolygon = clipBaselineBuffer->asMultiPolygon();
      if ( bufferMultiPolygon.size() < 1 )
      {
        delete clipBaselineBuffer;
        continue;
      }

      for ( int j = 0; j < bufferMultiPolygon.size(); ++j )
      {
        int size = bufferMultiPolygon.at( j ).size();
        for ( int k = 0; k < size; ++k )
        {
          mpl.append( bufferMultiPolygon.at( j ).at( k ) );
        }
      }
      bufferLine = QgsGeometry::fromMultiPolyline( mpl );
    }
    else
    {
      QgsPolygon bufferPolygon = clipBaselineBuffer->asPolygon();
      if ( bufferPolygon.size() < 1 )
      {
        delete clipBaselineBuffer;
        continue;
      }

      int size = bufferPolygon.size();
      mpl.reserve( size );
      for ( int j = 0; j < size; ++j )
      {
        mpl.append( bufferPolygon[j] );
      }
      bufferLine = QgsGeometry::fromMultiPolyline( mpl );
    }
    bufferLineClipped = bufferLine->intersection( stratumGeom );
    delete bufferLine;

    if ( bufferLineClipped && bufferLineClipped->type() == QGis::Line )
    {
      //if stratumGeom is a multipolygon, bufferLineClipped must intersect each part
      bool bufferLineClippedIntersectsStratum = true;
      if ( stratumGeom->wkbType() == QGis::WKBMultiPolygon || stratumGeom->wkbType() == QGis::WKBMultiPolygon25D )
      {
        QVector<QgsPolygon> multiPoly = stratumGeom->asMultiPolygon();
        QVector<QgsPolygon>::const_iterator multiIt = multiPoly.constBegin();
        for ( ; multiIt != multiPoly.constEnd(); ++multiIt )
        {
          QgsGeometry* poly = QgsGeometry::fromPolygon( *multiIt );
          if ( !poly->intersects( bufferLineClipped ) )
          {
            bufferLineClippedIntersectsStratum = false;
            delete poly;
            break;
          }
          delete poly;
        }
      }

      if ( bufferLineClippedIntersectsStratum )
      {
        delete clipBaselineBuffer;
        if ( mBaselineSimplificationTolerance >= 0 )
        {
          delete usedBaseline;
        }
        return bufferLineClipped;
      }
    }

    delete bufferLineClipped;
    delete clipBaselineBuffer;
    currentBufferDist /= 2;
  }

  if ( mBaselineSimplificationTolerance >= 0 )
  {
    delete usedBaseline;
  }
  return nullptr; //no solution found even with reduced tolerances
}

double QgsTransectSample::bufferDistance( double minDistanceFromAttribute ) const
{
  double bufferDist = minDistanceFromAttribute;
  if ( mBaselineBufferDistance >= 0 )
  {
    bufferDist = mBaselineBufferDistance;
  }

  if ( mMinDistanceUnits == Meters && mStrataLayer->crs().mapUnits() == QGis::DecimalDegrees )
  {
    bufferDist /= 111319.9;
  }

  return bufferDist;
}
