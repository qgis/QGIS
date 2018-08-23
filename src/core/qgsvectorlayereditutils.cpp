/***************************************************************************
    qgsvectorlayereditutils.cpp
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorlayereditutils.h"

#include "qgsvectordataprovider.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgsgeometryfactory.h"
#include "qgis.h"
#include "qgswkbtypes.h"
#include "qgsvectorlayerutils.h"

#include <limits>


QgsVectorLayerEditUtils::QgsVectorLayerEditUtils( QgsVectorLayer *layer )
  : mLayer( layer )
{
}

bool QgsVectorLayerEditUtils::insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex )
{
  if ( !mLayer->isSpatial() )
    return false;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.hasGeometry() )
    return false; // geometry not found

  QgsGeometry geometry = f.geometry();

  geometry.insertVertex( x, y, beforeVertex );

  mLayer->changeGeometry( atFeatureId, geometry );
  return true;
}

bool QgsVectorLayerEditUtils::insertVertex( const QgsPoint &point, QgsFeatureId atFeatureId, int beforeVertex )
{
  if ( !mLayer->isSpatial() )
    return false;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.hasGeometry() )
    return false; // geometry not found

  QgsGeometry geometry = f.geometry();

  geometry.insertVertex( point, beforeVertex );

  mLayer->changeGeometry( atFeatureId, geometry );
  return true;
}

bool QgsVectorLayerEditUtils::moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex )
{
  QgsPoint p( x, y );
  return moveVertex( p, atFeatureId, atVertex );
}

bool QgsVectorLayerEditUtils::moveVertex( const QgsPoint &p, QgsFeatureId atFeatureId, int atVertex )
{
  if ( !mLayer->isSpatial() )
    return false;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.hasGeometry() )
    return false; // geometry not found

  QgsGeometry geometry = f.geometry();

  geometry.moveVertex( p, atVertex );

  mLayer->changeGeometry( atFeatureId, geometry );
  return true;
}


QgsVectorLayer::EditResult QgsVectorLayerEditUtils::deleteVertex( QgsFeatureId featureId, int vertex )
{
  if ( !mLayer->isSpatial() )
    return QgsVectorLayer::InvalidLayer;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.hasGeometry() )
    return QgsVectorLayer::FetchFeatureFailed; // geometry not found

  QgsGeometry geometry = f.geometry();

  if ( !geometry.deleteVertex( vertex ) )
    return QgsVectorLayer::EditFailed;

  if ( geometry.constGet() && geometry.constGet()->nCoordinates() == 0 )
  {
    //last vertex deleted, set geometry to null
    geometry.set( nullptr );
  }

  mLayer->changeGeometry( featureId, geometry );
  return !geometry.isNull() ? QgsVectorLayer::Success : QgsVectorLayer::EmptyGeometry;
}

QgsGeometry::OperationResult QgsVectorLayerEditUtils::addRing( const QVector<QgsPointXY> &ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureId *modifiedFeatureId )
{
  QgsLineString *ringLine = new QgsLineString( ring );
  return addRing( ringLine, targetFeatureIds,  modifiedFeatureId );
}

QgsGeometry::OperationResult QgsVectorLayerEditUtils::addRing( QgsCurve *ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureId *modifiedFeatureId )
{
  if ( !mLayer->isSpatial() )
  {
    delete ring;
    return QgsGeometry::AddRingNotInExistingFeature;
  }

  QgsGeometry::OperationResult addRingReturnCode = QgsGeometry::AddRingNotInExistingFeature; //default: return code for 'ring not inserted'
  QgsFeature f;

  QgsFeatureIterator fit;
  if ( !targetFeatureIds.isEmpty() )
  {
    //check only specified features
    fit = mLayer->getFeatures( QgsFeatureRequest().setFilterFids( targetFeatureIds ) );
  }
  else
  {
    //check all intersecting features
    QgsRectangle bBox = ring->boundingBox();
    fit = mLayer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  }

  //find first valid feature we can add the ring to
  while ( fit.nextFeature( f ) )
  {
    if ( !f.hasGeometry() )
      continue;

    //add ring takes ownership of ring, and deletes it if there's an error
    QgsGeometry g = f.geometry();

    addRingReturnCode = g.addRing( static_cast< QgsCurve * >( ring->clone() ) );
    if ( addRingReturnCode == 0 )
      if ( addRingReturnCode == QgsGeometry::Success )
      {
        mLayer->changeGeometry( f.id(), g );
        if ( modifiedFeatureId )
          *modifiedFeatureId = f.id();

        //setModified( true, true );
        break;
      }
  }

  delete ring;
  return addRingReturnCode;
}

QgsGeometry::OperationResult QgsVectorLayerEditUtils::addPart( const QList<QgsPointXY> &points, QgsFeatureId featureId )
{
  QgsPointSequence l;
  for ( QList<QgsPointXY>::const_iterator it = points.constBegin(); it != points.constEnd(); ++it )
  {
    l <<  QgsPoint( *it );
  }
  return addPart( l, featureId );
}

QgsGeometry::OperationResult QgsVectorLayerEditUtils::addPart( const QgsPointSequence &points, QgsFeatureId featureId )
{
  if ( !mLayer->isSpatial() )
    return QgsGeometry::OperationResult::AddPartSelectedGeometryNotFound;

  QgsGeometry geometry;
  bool firstPart = false;
  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) )
    return QgsGeometry::OperationResult::AddPartSelectedGeometryNotFound; //not found

  if ( !f.hasGeometry() )
  {
    //no existing geometry, so adding first part to null geometry
    firstPart = true;
  }
  else
  {
    geometry = f.geometry();
  }

  QgsGeometry::OperationResult errorCode = geometry.addPart( points,  mLayer->geometryType() );
  if ( errorCode == QgsGeometry::Success )
  {
    if ( firstPart && QgsWkbTypes::isSingleType( mLayer->wkbType() )
         && mLayer->dataProvider()->doesStrictFeatureTypeCheck() )
    {
      //convert back to single part if required by layer
      geometry.convertToSingleType();
    }
    mLayer->changeGeometry( featureId, geometry );
  }
  return errorCode;
}

QgsGeometry::OperationResult QgsVectorLayerEditUtils::addPart( QgsCurve *ring, QgsFeatureId featureId )
{
  if ( !mLayer->isSpatial() )
    return QgsGeometry::AddPartSelectedGeometryNotFound;

  QgsGeometry geometry;
  bool firstPart = false;
  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) )
    return QgsGeometry::AddPartSelectedGeometryNotFound;

  if ( !f.hasGeometry() )
  {
    //no existing geometry, so adding first part to null geometry
    firstPart = true;
  }
  else
  {
    geometry = f.geometry();
  }

  QgsGeometry::OperationResult errorCode = geometry.addPart( ring, mLayer->geometryType() );
  if ( errorCode == QgsGeometry::Success )
  {
    if ( firstPart && QgsWkbTypes::isSingleType( mLayer->wkbType() )
         && mLayer->dataProvider()->doesStrictFeatureTypeCheck() )
    {
      //convert back to single part if required by layer
      geometry.convertToSingleType();
    }
    mLayer->changeGeometry( featureId, geometry );
  }
  return errorCode;
}


int QgsVectorLayerEditUtils::translateFeature( QgsFeatureId featureId, double dx, double dy )
{
  if ( !mLayer->isSpatial() )
    return 1;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.hasGeometry() )
    return 1; //geometry not found

  QgsGeometry geometry = f.geometry();

  int errorCode = geometry.translate( dx, dy );
  if ( errorCode == 0 )
  {
    mLayer->changeGeometry( featureId, geometry );
  }
  return errorCode;
}


QgsGeometry::OperationResult QgsVectorLayerEditUtils::splitFeatures( const QVector<QgsPointXY> &splitLine, bool topologicalEditing )
{
  if ( !mLayer->isSpatial() )
    return QgsGeometry::InvalidBaseGeometry;

  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox; //bounding box of the split line
  QgsGeometry::OperationResult returnCode = QgsGeometry::OperationResult::Success;
  QgsGeometry::OperationResult splitFunctionReturn; //return code of QgsGeometry::splitGeometry
  int numberOfSplitFeatures = 0;

  QgsFeatureIterator features;
  const QgsFeatureIds selectedIds = mLayer->selectedFeatureIds();

  if ( !selectedIds.isEmpty() ) //consider only the selected features if there is a selection
  {
    features = mLayer->getSelectedFeatures();
  }
  else //else consider all the feature that intersect the bounding box of the split line
  {
    if ( boundingBoxFromPointList( splitLine, xMin, yMin, xMax, yMax ) )
    {
      bBox.setXMinimum( xMin );
      bBox.setYMinimum( yMin );
      bBox.setXMaximum( xMax );
      bBox.setYMaximum( yMax );
    }
    else
    {
      return QgsGeometry::OperationResult::InvalidInputGeometryType;
    }

    if ( bBox.isEmpty() )
    {
      //if the bbox is a line, try to make a square out of it
      if ( bBox.width() == 0.0 && bBox.height() > 0 )
      {
        bBox.setXMinimum( bBox.xMinimum() - bBox.height() / 2 );
        bBox.setXMaximum( bBox.xMaximum() + bBox.height() / 2 );
      }
      else if ( bBox.height() == 0.0 && bBox.width() > 0 )
      {
        bBox.setYMinimum( bBox.yMinimum() - bBox.width() / 2 );
        bBox.setYMaximum( bBox.yMaximum() + bBox.width() / 2 );
      }
      else
      {
        //If we have a single point, we still create a non-null box
        double bufferDistance = 0.000001;
        if ( mLayer->crs().isGeographic() )
          bufferDistance = 0.00000001;
        bBox.setXMinimum( bBox.xMinimum() - bufferDistance );
        bBox.setXMaximum( bBox.xMaximum() + bufferDistance );
        bBox.setYMinimum( bBox.yMinimum() - bufferDistance );
        bBox.setYMaximum( bBox.yMaximum() + bufferDistance );
      }
    }

    features = mLayer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  }

  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    if ( !feat.hasGeometry() )
    {
      continue;
    }
    QVector<QgsGeometry> newGeometries;
    QVector<QgsPointXY> topologyTestPoints;
    QgsGeometry featureGeom = feat.geometry();
    splitFunctionReturn = featureGeom.splitGeometry( splitLine, newGeometries, topologicalEditing, topologyTestPoints );
    if ( splitFunctionReturn == QgsGeometry::OperationResult::Success )
    {
      //change this geometry
      mLayer->changeGeometry( feat.id(), featureGeom );

      //insert new features
      for ( int i = 0; i < newGeometries.size(); ++i )
      {
        QgsFeature f = QgsVectorLayerUtils::createFeature( mLayer, newGeometries.at( i ), feat.attributes().toMap() );
        mLayer->addFeature( f );
      }

      if ( topologicalEditing )
      {
        QVector<QgsPointXY>::const_iterator topol_it = topologyTestPoints.constBegin();
        for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplitFeatures;
    }
    else if ( splitFunctionReturn != QgsGeometry::OperationResult::Success && splitFunctionReturn != QgsGeometry::NothingHappened ) // i.e. no split but no error occurred
    {
      returnCode = splitFunctionReturn;
    }
  }

  if ( numberOfSplitFeatures == 0 && !selectedIds.isEmpty() )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    returnCode = QgsGeometry::OperationResult::NothingHappened;
  }

  return returnCode;
}

QgsGeometry::OperationResult QgsVectorLayerEditUtils::splitParts( const QVector<QgsPointXY> &splitLine, bool topologicalEditing )
{
  if ( !mLayer->isSpatial() )
    return QgsGeometry::InvalidBaseGeometry;

  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox; //bounding box of the split line
  QgsGeometry::OperationResult returnCode = QgsGeometry::OperationResult::Success;
  QgsGeometry::OperationResult splitFunctionReturn; //return code of QgsGeometry::splitGeometry
  int numberOfSplitParts = 0;

  QgsFeatureIterator fit;

  if ( mLayer->selectedFeatureCount() > 0 ) //consider only the selected features if there is a selection
  {
    fit = mLayer->getSelectedFeatures();
  }
  else //else consider all the feature that intersect the bounding box of the split line
  {
    if ( boundingBoxFromPointList( splitLine, xMin, yMin, xMax, yMax ) )
    {
      bBox.setXMinimum( xMin );
      bBox.setYMinimum( yMin );
      bBox.setXMaximum( xMax );
      bBox.setYMaximum( yMax );
    }
    else
    {
      return QgsGeometry::OperationResult::InvalidInputGeometryType;
    }

    if ( bBox.isEmpty() )
    {
      //if the bbox is a line, try to make a square out of it
      if ( bBox.width() == 0.0 && bBox.height() > 0 )
      {
        bBox.setXMinimum( bBox.xMinimum() - bBox.height() / 2 );
        bBox.setXMaximum( bBox.xMaximum() + bBox.height() / 2 );
      }
      else if ( bBox.height() == 0.0 && bBox.width() > 0 )
      {
        bBox.setYMinimum( bBox.yMinimum() - bBox.width() / 2 );
        bBox.setYMaximum( bBox.yMaximum() + bBox.width() / 2 );
      }
      else
      {
        //If we have a single point, we still create a non-null box
        double bufferDistance = 0.000001;
        if ( mLayer->crs().isGeographic() )
          bufferDistance = 0.00000001;
        bBox.setXMinimum( bBox.xMinimum() - bufferDistance );
        bBox.setXMaximum( bBox.xMaximum() + bufferDistance );
        bBox.setYMinimum( bBox.yMinimum() - bufferDistance );
        bBox.setYMaximum( bBox.yMaximum() + bufferDistance );
      }
    }

    fit = mLayer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  }

  QgsGeometry::OperationResult addPartRet = QgsGeometry::OperationResult::Success;

  QgsFeature feat;
  while ( fit.nextFeature( feat ) )
  {
    QVector<QgsGeometry> newGeometries;
    QVector<QgsPointXY> topologyTestPoints;
    QgsGeometry featureGeom = feat.geometry();
    splitFunctionReturn = featureGeom.splitGeometry( splitLine, newGeometries, topologicalEditing, topologyTestPoints );
    if ( splitFunctionReturn == 0 )
    {
      //add new parts
      if ( !newGeometries.isEmpty() )
        featureGeom.convertToMultiType();

      for ( int i = 0; i < newGeometries.size(); ++i )
      {
        addPartRet = featureGeom.addPart( newGeometries.at( i ) );
        if ( addPartRet )
          break;
      }

      // For test only: Exception already thrown here...
      // feat.geometry()->asWkb();

      if ( !addPartRet )
      {
        mLayer->changeGeometry( feat.id(), featureGeom );
      }

      if ( topologicalEditing )
      {
        QVector<QgsPointXY>::const_iterator topol_it = topologyTestPoints.constBegin();
        for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplitParts;
    }
    else if ( splitFunctionReturn != QgsGeometry::OperationResult::Success && splitFunctionReturn != QgsGeometry::OperationResult::NothingHappened )
    {
      returnCode = splitFunctionReturn;
    }
  }

  if ( numberOfSplitParts == 0 && mLayer->selectedFeatureCount() > 0  && returnCode == QgsGeometry::Success )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    returnCode = QgsGeometry::OperationResult::NothingHappened;
  }

  return returnCode;
}


int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsGeometry &geom )
{
  if ( !mLayer->isSpatial() )
    return 1;

  if ( geom.isNull() )
  {
    return 1;
  }

  int returnVal = 0;

  QgsWkbTypes::Type wkbType = geom.wkbType();

  switch ( QgsWkbTypes::geometryType( wkbType ) )
  {
    //line
    case QgsWkbTypes::LineGeometry:
    {
      if ( !QgsWkbTypes::isMultiType( wkbType ) )
      {
        QgsPolylineXY line = geom.asPolyline();
        QgsPolylineXY::const_iterator line_it = line.constBegin();
        for ( ; line_it != line.constEnd(); ++line_it )
        {
          if ( addTopologicalPoints( *line_it ) != 0 )
          {
            returnVal = 2;
          }
        }
      }
      else
      {
        QgsMultiPolylineXY multiLine = geom.asMultiPolyline();
        QgsPolylineXY currentPolyline;

        for ( int i = 0; i < multiLine.size(); ++i )
        {
          QgsPolylineXY::const_iterator line_it = currentPolyline.constBegin();
          for ( ; line_it != currentPolyline.constEnd(); ++line_it )
          {
            if ( addTopologicalPoints( *line_it ) != 0 )
            {
              returnVal = 2;
            }
          }
        }
      }
      break;
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      if ( !QgsWkbTypes::isMultiType( wkbType ) )
      {
        QgsPolygonXY polygon = geom.asPolygon();
        QgsPolylineXY currentRing;

        for ( int i = 0; i < polygon.size(); ++i )
        {
          currentRing = polygon.at( i );
          QgsPolylineXY::const_iterator line_it = currentRing.constBegin();
          for ( ; line_it != currentRing.constEnd(); ++line_it )
          {
            if ( addTopologicalPoints( *line_it ) != 0 )
            {
              returnVal = 2;
            }
          }
        }
      }
      else
      {
        QgsMultiPolygonXY multiPolygon = geom.asMultiPolygon();
        QgsPolygonXY currentPolygon;
        QgsPolylineXY currentRing;

        for ( int i = 0; i < multiPolygon.size(); ++i )
        {
          currentPolygon = multiPolygon.at( i );
          for ( int j = 0; j < currentPolygon.size(); ++j )
          {
            currentRing = currentPolygon.at( j );
            QgsPolylineXY::const_iterator line_it = currentRing.constBegin();
            for ( ; line_it != currentRing.constEnd(); ++line_it )
            {
              if ( addTopologicalPoints( *line_it ) != 0 )
              {
                returnVal = 2;
              }
            }
          }
        }
      }
      break;
    }

    case QgsWkbTypes::PointGeometry:
    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }
  return returnVal;
}


int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsPointXY &p )
{
  if ( !mLayer->isSpatial() )
    return 1;

  double segmentSearchEpsilon = mLayer->crs().isGeographic() ? 1e-12 : 1e-8;

  //work with a tolerance because coordinate projection may introduce some rounding
  double threshold = 0.0000001;
  if ( mLayer->crs().mapUnits() == QgsUnitTypes::DistanceMeters )
  {
    threshold = 0.001;
  }
  else if ( mLayer->crs().mapUnits() == QgsUnitTypes::DistanceFeet )
  {
    threshold = 0.0001;
  }

  QgsRectangle searchRect( p.x() - threshold, p.y() - threshold,
                           p.x() + threshold, p.y() + threshold );
  double sqrSnappingTolerance = threshold * threshold;

  QgsFeature f;
  QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest()
                           .setFilterRect( searchRect )
                           .setFlags( QgsFeatureRequest::ExactIntersect )
                           .setSubsetOfAttributes( QgsAttributeList() ) );

  QMap<QgsFeatureId, QgsGeometry> features;
  QMap<QgsFeatureId, int> segments;

  while ( fit.nextFeature( f ) )
  {
    int afterVertex;
    QgsPointXY snappedPoint;
    double sqrDistSegmentSnap = f.geometry().closestSegmentWithContext( p, snappedPoint, afterVertex, nullptr, segmentSearchEpsilon );
    if ( sqrDistSegmentSnap < sqrSnappingTolerance )
    {
      segments[f.id()] = afterVertex;
      features[f.id()] = f.geometry();
    }
  }

  if ( segments.isEmpty() )
    return 2;

  for ( QMap<QgsFeatureId, int>::const_iterator it = segments.constBegin(); it != segments.constEnd(); ++it )
  {
    QgsFeatureId fid = it.key();
    int segmentAfterVertex = it.value();
    QgsGeometry geom = features[fid];

    int atVertex, beforeVertex, afterVertex;
    double sqrDistVertexSnap;
    geom.closestVertex( p, atVertex, beforeVertex, afterVertex, sqrDistVertexSnap );

    if ( sqrDistVertexSnap < sqrSnappingTolerance )
      continue;  // the vertex already exists - do not insert it

    if ( !mLayer->insertVertex( p.x(), p.y(), fid, segmentAfterVertex ) )
    {
      QgsDebugMsg( "failed to insert topo point" );
    }
  }

  return 0;
}


bool QgsVectorLayerEditUtils::boundingBoxFromPointList( const QVector<QgsPointXY> &list, double &xmin, double &ymin, double &xmax, double &ymax ) const
{
  if ( list.empty() )
  {
    return false;
  }

  xmin = std::numeric_limits<double>::max();
  xmax = -std::numeric_limits<double>::max();
  ymin = std::numeric_limits<double>::max();
  ymax = -std::numeric_limits<double>::max();

  for ( QVector<QgsPointXY>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
  {
    if ( it->x() < xmin )
    {
      xmin = it->x();
    }
    if ( it->x() > xmax )
    {
      xmax = it->x();
    }
    if ( it->y() < ymin )
    {
      ymin = it->y();
    }
    if ( it->y() > ymax )
    {
      ymax = it->y();
    }
  }

  return true;
}
