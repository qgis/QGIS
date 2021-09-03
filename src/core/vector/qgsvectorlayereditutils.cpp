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
#include "qgsvectorlayer.h"
#include "qgsgeometryoptions.h"
#include "qgsabstractgeometry.h"

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
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setNoAttributes() ).nextFeature( f ) || !f.hasGeometry() )
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
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setNoAttributes() ).nextFeature( f ) || !f.hasGeometry() )
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
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setNoAttributes() ).nextFeature( f ) || !f.hasGeometry() )
    return false; // geometry not found

  QgsGeometry geometry = f.geometry();

  geometry.moveVertex( p, atVertex );

  mLayer->changeGeometry( atFeatureId, geometry );
  return true;
}


Qgis::VectorEditResult QgsVectorLayerEditUtils::deleteVertex( QgsFeatureId featureId, int vertex )
{
  if ( !mLayer->isSpatial() )
    return Qgis::VectorEditResult::InvalidLayer;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setNoAttributes() ).nextFeature( f ) || !f.hasGeometry() )
    return Qgis::VectorEditResult::FetchFeatureFailed; // geometry not found

  QgsGeometry geometry = f.geometry();

  if ( !geometry.deleteVertex( vertex ) )
    return Qgis::VectorEditResult::EditFailed;

  if ( geometry.constGet() && geometry.constGet()->nCoordinates() == 0 )
  {
    //last vertex deleted, set geometry to null
    geometry.set( nullptr );
  }

  mLayer->changeGeometry( featureId, geometry );
  return !geometry.isNull() ? Qgis::VectorEditResult::Success : Qgis::VectorEditResult::EmptyGeometry;
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addRing( const QVector<QgsPointXY> &ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureId *modifiedFeatureId )
{
  QgsPointSequence l;
  for ( QVector<QgsPointXY>::const_iterator it = ring.constBegin(); it != ring.constEnd(); ++it )
  {
    l <<  QgsPoint( *it );
  }
  return addRing( l, targetFeatureIds,  modifiedFeatureId );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addRing( const QgsPointSequence &ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureId *modifiedFeatureId )
{
  QgsLineString *ringLine = new QgsLineString( ring );
  return addRing( ringLine, targetFeatureIds,  modifiedFeatureId );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addRing( QgsCurve *ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureId *modifiedFeatureId )
{
  if ( !mLayer->isSpatial() )
  {
    delete ring;
    return Qgis::GeometryOperationResult::AddRingNotInExistingFeature;
  }

  Qgis::GeometryOperationResult addRingReturnCode = Qgis::GeometryOperationResult::AddRingNotInExistingFeature; //default: return code for 'ring not inserted'
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
    if ( addRingReturnCode == Qgis::GeometryOperationResult::Success )
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

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addPart( const QVector<QgsPointXY> &points, QgsFeatureId featureId )
{
  QgsPointSequence l;
  for ( QVector<QgsPointXY>::const_iterator it = points.constBegin(); it != points.constEnd(); ++it )
  {
    l <<  QgsPoint( *it );
  }
  return addPart( l, featureId );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addPart( const QgsPointSequence &points, QgsFeatureId featureId )
{
  if ( !mLayer->isSpatial() )
    return Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound;

  QgsGeometry geometry;
  bool firstPart = false;
  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setNoAttributes() ).nextFeature( f ) )
    return Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound; //not found

  if ( !f.hasGeometry() )
  {
    //no existing geometry, so adding first part to null geometry
    firstPart = true;
  }
  else
  {
    geometry = f.geometry();
  }

  Qgis::GeometryOperationResult errorCode = geometry.addPart( points,  mLayer->geometryType() );
  if ( errorCode == Qgis::GeometryOperationResult::Success )
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

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addPart( QgsCurve *ring, QgsFeatureId featureId )
{
  if ( !mLayer->isSpatial() )
    return Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound;

  QgsGeometry geometry;
  bool firstPart = false;
  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setNoAttributes() ).nextFeature( f ) )
    return Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound;

  if ( !f.hasGeometry() )
  {
    //no existing geometry, so adding first part to null geometry
    firstPart = true;
  }
  else
  {
    geometry = f.geometry();
  }

  Qgis::GeometryOperationResult errorCode = geometry.addPart( ring, mLayer->geometryType() );
  if ( errorCode == Qgis::GeometryOperationResult::Success )
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

// TODO QGIS 4.0 -- this should return Qgis::GeometryOperationResult
int QgsVectorLayerEditUtils::translateFeature( QgsFeatureId featureId, double dx, double dy )
{
  if ( !mLayer->isSpatial() )
    return 1;

  QgsFeature f;
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setNoAttributes() ).nextFeature( f ) || !f.hasGeometry() )
    return 1; //geometry not found

  QgsGeometry geometry = f.geometry();

  Qgis::GeometryOperationResult errorCode = geometry.translate( dx, dy );
  if ( errorCode == Qgis::GeometryOperationResult::Success )
  {
    mLayer->changeGeometry( featureId, geometry );
  }
  return errorCode == Qgis::GeometryOperationResult::Success ? 0 : 1;
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::splitFeatures( const QVector<QgsPointXY> &splitLine, bool topologicalEditing )
{
  QgsPointSequence l;
  for ( QVector<QgsPointXY>::const_iterator it = splitLine.constBegin(); it != splitLine.constEnd(); ++it )
  {
    l <<  QgsPoint( *it );
  }
  return splitFeatures( l, topologicalEditing );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::splitFeatures( const QgsPointSequence &splitLine, bool topologicalEditing )
{
  QgsLineString lineString( splitLine );
  QgsPointSequence topologyTestPoints;
  bool preserveCircular = false;
  return splitFeatures( &lineString, topologyTestPoints, preserveCircular, topologicalEditing );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::splitFeatures( const QgsCurve *curve, QgsPointSequence &topologyTestPoints, bool preserveCircular, bool topologicalEditing )
{
  if ( !mLayer->isSpatial() )
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;

  QgsRectangle bBox; //bounding box of the split line
  Qgis::GeometryOperationResult returnCode = Qgis::GeometryOperationResult::Success;
  Qgis::GeometryOperationResult splitFunctionReturn; //return code of QgsGeometry::splitGeometry
  int numberOfSplitFeatures = 0;

  QgsFeatureIterator features;
  const QgsFeatureIds selectedIds = mLayer->selectedFeatureIds();

  // deactivate preserving circular if the curve contains only straight segments to avoid transforming Polygon to CurvePolygon
  preserveCircular &= curve->hasCurvedSegments();

  if ( !selectedIds.isEmpty() ) //consider only the selected features if there is a selection
  {
    features = mLayer->getSelectedFeatures();
  }
  else //else consider all the feature that intersect the bounding box of the split line
  {

    bBox = curve->boundingBox();

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

  QgsVectorLayerUtils::QgsFeaturesDataList featuresDataToAdd;

  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    if ( !feat.hasGeometry() )
    {
      continue;
    }
    QVector<QgsGeometry> newGeometries;
    QgsPointSequence featureTopologyTestPoints;
    QgsGeometry featureGeom = feat.geometry();
    splitFunctionReturn = featureGeom.splitGeometry( curve, newGeometries, preserveCircular, topologicalEditing, featureTopologyTestPoints );
    topologyTestPoints.append( featureTopologyTestPoints );
    if ( splitFunctionReturn == Qgis::GeometryOperationResult::Success )
    {
      //change this geometry
      mLayer->changeGeometry( feat.id(), featureGeom );

      //insert new features
      QgsAttributeMap attributeMap = feat.attributes().toMap();
      for ( const QgsGeometry &geom : std::as_const( newGeometries ) )
      {
        featuresDataToAdd << QgsVectorLayerUtils::QgsFeatureData( geom, attributeMap );
      }

      if ( topologicalEditing )
      {
        QgsPointSequence::const_iterator topol_it = featureTopologyTestPoints.constBegin();
        for ( ; topol_it != featureTopologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplitFeatures;
    }
    else if ( splitFunctionReturn != Qgis::GeometryOperationResult::Success && splitFunctionReturn != Qgis::GeometryOperationResult::NothingHappened ) // i.e. no split but no error occurred
    {
      returnCode = splitFunctionReturn;
    }
  }

  if ( !featuresDataToAdd.isEmpty() )
  {
    // finally create and add all bits of geometries cut off the original geometries
    // (this is much faster than creating features one by one)
    QgsFeatureList featuresListToAdd = QgsVectorLayerUtils::createFeatures( mLayer, featuresDataToAdd );
    mLayer->addFeatures( featuresListToAdd );
  }

  if ( numberOfSplitFeatures == 0 )
  {
    returnCode = Qgis::GeometryOperationResult::NothingHappened;
  }

  return returnCode;
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::splitParts( const QVector<QgsPointXY> &splitLine, bool topologicalEditing )
{
  QgsPointSequence l;
  for ( QVector<QgsPointXY>::const_iterator it = splitLine.constBegin(); it != splitLine.constEnd(); ++it )
  {
    l <<  QgsPoint( *it );
  }
  return splitParts( l, topologicalEditing );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::splitParts( const QgsPointSequence &splitLine, bool topologicalEditing )
{
  if ( !mLayer->isSpatial() )
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;

  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox; //bounding box of the split line
  Qgis::GeometryOperationResult returnCode = Qgis::GeometryOperationResult::Success;
  Qgis::GeometryOperationResult splitFunctionReturn; //return code of QgsGeometry::splitGeometry
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
      return Qgis::GeometryOperationResult::InvalidInputGeometryType;
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

  QgsFeature feat;
  while ( fit.nextFeature( feat ) )
  {
    QVector<QgsGeometry> newGeometries;
    QgsPointSequence topologyTestPoints;
    QgsGeometry featureGeom = feat.geometry();
    splitFunctionReturn = featureGeom.splitGeometry( splitLine, newGeometries, topologicalEditing, topologyTestPoints, false );

    if ( splitFunctionReturn == Qgis::GeometryOperationResult::Success && !newGeometries.isEmpty() )
    {
      QgsGeometry newGeom( newGeometries.at( 0 ) );
      newGeom.convertToMultiType();

      for ( int i = 1; i < newGeometries.size(); ++i )
      {
        QgsGeometry part = newGeometries.at( i );
        part.convertToSingleType();
        newGeom.addPart( part );
      }

      mLayer->changeGeometry( feat.id(), newGeom );

      if ( topologicalEditing )
      {
        QgsPointSequence::const_iterator topol_it = topologyTestPoints.constBegin();
        for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplitParts;
    }
    else if ( splitFunctionReturn != Qgis::GeometryOperationResult::Success && splitFunctionReturn != Qgis::GeometryOperationResult::NothingHappened )
    {
      returnCode = splitFunctionReturn;
    }
  }

  if ( numberOfSplitParts == 0 && mLayer->selectedFeatureCount() > 0  && returnCode == Qgis::GeometryOperationResult::Success )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    returnCode = Qgis::GeometryOperationResult::NothingHappened;
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

  bool pointsAdded = false;

  QgsAbstractGeometry::vertex_iterator it = geom.vertices_begin();
  while ( it != geom.vertices_end() )
  {
    if ( addTopologicalPoints( *it ) == 0 )
    {
      pointsAdded = true;
    }
    ++it;
  }

  return pointsAdded ? 0 : 2;
}

int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsPoint &p )
{
  if ( !mLayer->isSpatial() )
    return 1;

  double segmentSearchEpsilon = mLayer->crs().isGeographic() ? 1e-12 : 1e-8;

  //work with a tolerance because coordinate projection may introduce some rounding
  double threshold = mLayer->geometryOptions()->geometryPrecision();

  if ( qgsDoubleNear( threshold, 0.0 ) )
  {
    threshold = 0.0000001;

    if ( mLayer->crs().mapUnits() == QgsUnitTypes::DistanceMeters )
    {
      threshold = 0.001;
    }
    else if ( mLayer->crs().mapUnits() == QgsUnitTypes::DistanceFeet )
    {
      threshold = 0.0001;
    }
  }

  QgsRectangle searchRect( p.x() - threshold, p.y() - threshold,
                           p.x() + threshold, p.y() + threshold );
  double sqrSnappingTolerance = threshold * threshold;

  QgsFeature f;
  QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest()
                           .setFilterRect( searchRect )
                           .setFlags( QgsFeatureRequest::ExactIntersect )
                           .setNoAttributes() );

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

  bool pointsAdded = false;
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

    if ( !mLayer->insertVertex( p, fid, segmentAfterVertex ) )
    {
      QgsDebugMsg( QStringLiteral( "failed to insert topo point" ) );
    }
    else
    {
      pointsAdded = true;
    }
  }

  return pointsAdded ? 0 : 2;
}

int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsPointSequence &ps )
{
  if ( !mLayer->isSpatial() )
    return 1;

  if ( ps.isEmpty() )
  {
    return 1;
  }

  bool pointsAdded = false;

  QgsPointSequence::const_iterator it = ps.constBegin();
  while ( it != ps.constEnd() )
  {
    if ( addTopologicalPoints( *it ) == 0 )
    {
      pointsAdded = true;
    }
    ++it;
  }

  return pointsAdded ? 0 : 2;
}

int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsPointXY &p )
{
  return addTopologicalPoints( QgsPoint( p ) );
}


bool QgsVectorLayerEditUtils::boundingBoxFromPointList( const QgsPointSequence &list, double &xmin, double &ymin, double &xmax, double &ymax ) const
{
  if ( list.empty() )
  {
    return false;
  }

  xmin = std::numeric_limits<double>::max();
  xmax = -std::numeric_limits<double>::max();
  ymin = std::numeric_limits<double>::max();
  ymax = -std::numeric_limits<double>::max();

  for ( QgsPointSequence::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
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
