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
#include "qgsgeometrycache.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgslinestringv2.h"
#include "qgslogger.h"
#include "qgspointv2.h"

#include <limits>


QgsVectorLayerEditUtils::QgsVectorLayerEditUtils( QgsVectorLayer* layer )
    : L( layer )
{
}

bool QgsVectorLayerEditUtils::insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex )
{
  if ( !L->hasGeometryType() )
    return false;

  QgsGeometry geometry;
  if ( !cache()->geometry( atFeatureId, geometry ) )
  {
    // it's not in cache: let's fetch it from layer
    QgsFeature f;
    if ( !L->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.constGeometry() )
      return false; // geometry not found

    geometry = *f.constGeometry();
  }

  geometry.insertVertex( x, y, beforeVertex );

  L->editBuffer()->changeGeometry( atFeatureId, &geometry );
  return true;
}


bool QgsVectorLayerEditUtils::moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex )
{
  QgsPointV2 p( x, y );
  return moveVertex( p, atFeatureId, atVertex );
}

bool QgsVectorLayerEditUtils::moveVertex( const QgsPointV2& p, QgsFeatureId atFeatureId, int atVertex )
{
  if ( !L->hasGeometryType() )
    return false;

  QgsGeometry geometry;
  if ( !cache()->geometry( atFeatureId, geometry ) )
  {
    // it's not in cache: let's fetch it from layer
    QgsFeature f;
    if ( !L->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.constGeometry() )
      return false; // geometry not found

    geometry = *f.constGeometry();
  }

  geometry.moveVertex( p, atVertex );

  L->editBuffer()->changeGeometry( atFeatureId, &geometry );
  return true;
}


bool QgsVectorLayerEditUtils::deleteVertex( QgsFeatureId atFeatureId, int atVertex )
{
  if ( !L->hasGeometryType() )
    return false;

  QgsGeometry geometry;
  if ( !cache()->geometry( atFeatureId, geometry ) )
  {
    // it's not in cache: let's fetch it from layer
    QgsFeature f;
    if ( !L->getFeatures( QgsFeatureRequest().setFilterFid( atFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.constGeometry() )
      return false; // geometry not found

    geometry = *f.constGeometry();
  }

  if ( !geometry.deleteVertex( atVertex ) )
    return false;

  L->editBuffer()->changeGeometry( atFeatureId, &geometry );
  return true;
}

int QgsVectorLayerEditUtils::addRing( const QList<QgsPoint>& ring )
{
  QgsLineStringV2* ringLine = new QgsLineStringV2();
  QList< QgsPointV2 > ringPoints;
  QList<QgsPoint>::const_iterator ringIt = ring.constBegin();
  for ( ; ringIt != ring.constEnd(); ++ringIt )
  {
    ringPoints.append( QgsPointV2( ringIt->x(), ringIt->y() ) );
  }
  ringLine->setPoints( ringPoints );
  return addRing( ringLine );
}

int QgsVectorLayerEditUtils::addRing( QgsCurveV2* ring )
{
  if ( !L->hasGeometryType() )
    return 5;

  int addRingReturnCode = 5; //default: return code for 'ring not inserted'
  QgsRectangle bBox = ring->boundingBox();
  QgsFeatureIterator fit = L->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    addRingReturnCode = f.geometry()->addRing( ring );
    if ( addRingReturnCode == 0 )
    {
      L->editBuffer()->changeGeometry( f.id(), f.geometry() );

      //setModified( true, true );
      break;
    }
  }

  return addRingReturnCode;
}

int QgsVectorLayerEditUtils::addPart( const QList<QgsPoint> &points, QgsFeatureId featureId )
{
  if ( !L->hasGeometryType() )
    return 6;

  QgsGeometry geometry;
  if ( !cache()->geometry( featureId, geometry ) ) // maybe it's in cache
  {
    // it's not in cache: let's fetch it from layer
    QgsFeature f;
    if ( !L->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.geometry() )
      return 6; //geometry not found

    geometry = *f.geometry();
  }

  int errorCode = geometry.addPart( points, L->geometryType() );
  if ( errorCode == 0 )
  {
    L->editBuffer()->changeGeometry( featureId, &geometry );
  }
  return errorCode;
}

int QgsVectorLayerEditUtils::addPart( QgsCurveV2* ring, QgsFeatureId featureId )
{
  if ( !L->hasGeometryType() )
    return 6;

  QgsGeometry geometry;
  if ( !cache()->geometry( featureId, geometry ) ) // maybe it's in cache
  {
    // it's not in cache: let's fetch it from layer
    QgsFeature f;
    if ( !L->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.geometry() )
      return 6; //geometry not found

    geometry = *f.geometry();
  }

  int errorCode = geometry.addPart( ring );
  if ( errorCode == 0 )
  {
    L->editBuffer()->changeGeometry( featureId, &geometry );
  }
  return errorCode;
}


int QgsVectorLayerEditUtils::translateFeature( QgsFeatureId featureId, double dx, double dy )
{
  if ( !L->hasGeometryType() )
    return 1;

  QgsGeometry geometry;
  if ( !cache()->geometry( featureId, geometry ) ) // maybe it's in cache
  {
    // it's not in cache: let's fetch it from layer
    QgsFeature f;
    if ( !L->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) || !f.constGeometry() )
      return 1; //geometry not found

    geometry = *f.constGeometry();
  }

  int errorCode = geometry.translate( dx, dy );
  if ( errorCode == 0 )
  {
    L->editBuffer()->changeGeometry( featureId, &geometry );
  }
  return errorCode;
}


int QgsVectorLayerEditUtils::splitFeatures( const QList<QgsPoint>& splitLine, bool topologicalEditing )
{
  if ( !L->hasGeometryType() )
    return 4;

  QgsFeatureList newFeatures; //store all the newly created features
  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox; //bounding box of the split line
  int returnCode = 0;
  int splitFunctionReturn; //return code of QgsGeometry::splitGeometry
  int numberOfSplittedFeatures = 0;

  QgsFeatureIterator features;
  const QgsFeatureIds selectedIds = L->selectedFeaturesIds();

  if ( selectedIds.size() > 0 ) //consider only the selected features if there is a selection
  {
    features = L->selectedFeaturesIterator();
  }
  else //else consider all the feature that intersect the bounding box of the split line
  {
    if ( boundingBoxFromPointList( splitLine, xMin, yMin, xMax, yMax ) == 0 )
    {
      bBox.setXMinimum( xMin ); bBox.setYMinimum( yMin );
      bBox.setXMaximum( xMax ); bBox.setYMaximum( yMax );
    }
    else
    {
      return 1;
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
        if ( L->crs().geographicFlag() )
          bufferDistance = 0.00000001;
        bBox.setXMinimum( bBox.xMinimum() - bufferDistance );
        bBox.setXMaximum( bBox.xMaximum() + bufferDistance );
        bBox.setYMinimum( bBox.yMinimum() - bufferDistance );
        bBox.setYMaximum( bBox.yMaximum() + bufferDistance );
      }
    }

    features = L->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  }

  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    if ( !feat.constGeometry() )
    {
      continue;
    }
    QList<QgsGeometry*> newGeometries;
    QList<QgsPoint> topologyTestPoints;
    QgsGeometry* newGeometry = 0;
    splitFunctionReturn = feat.geometry()->splitGeometry( splitLine, newGeometries, topologicalEditing, topologyTestPoints );
    if ( splitFunctionReturn == 0 )
    {
      //change this geometry
      L->editBuffer()->changeGeometry( feat.id(), feat.geometry() );

      //insert new features
      for ( int i = 0; i < newGeometries.size(); ++i )
      {
        newGeometry = newGeometries.at( i );
        QgsFeature newFeature;
        newFeature.setGeometry( newGeometry );

        //use default value where possible for primary key (e.g. autoincrement),
        //and use the value from the original (split) feature if not primary key
        QgsAttributes newAttributes = feat.attributes();
        foreach ( int pkIdx, L->dataProvider()->pkAttributeIndexes() )
        {
          const QVariant defaultValue = L->dataProvider()->defaultValue( pkIdx );
          if ( !defaultValue.isNull() )
          {
            newAttributes[ pkIdx ] = defaultValue;
          }
          else //try with NULL
          {
            newAttributes[ pkIdx ] = QVariant();
          }
        }

        newFeature.setAttributes( newAttributes );

        newFeatures.append( newFeature );
      }

      if ( topologicalEditing )
      {
        QList<QgsPoint>::const_iterator topol_it = topologyTestPoints.constBegin();
        for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplittedFeatures;
    }
    else if ( splitFunctionReturn > 1 ) //1 means no split but also no error
    {
      returnCode = splitFunctionReturn;
    }
  }

  if ( numberOfSplittedFeatures == 0 && selectedIds.size() > 0 )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    returnCode = 4;
  }


  //now add the new features to this vectorlayer
  L->editBuffer()->addFeatures( newFeatures );

  return returnCode;
}

int QgsVectorLayerEditUtils::splitParts( const QList<QgsPoint>& splitLine, bool topologicalEditing )
{
  if ( !L->hasGeometryType() )
    return 4;

  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox; //bounding box of the split line
  int returnCode = 0;
  int splitFunctionReturn; //return code of QgsGeometry::splitGeometry
  int numberOfSplittedParts = 0;

  QgsFeatureIterator fit;

  if ( L->selectedFeatureCount() > 0 ) //consider only the selected features if there is a selection
  {
    fit = L->selectedFeaturesIterator();
  }
  else //else consider all the feature that intersect the bounding box of the split line
  {
    if ( boundingBoxFromPointList( splitLine, xMin, yMin, xMax, yMax ) == 0 )
    {
      bBox.setXMinimum( xMin ); bBox.setYMinimum( yMin );
      bBox.setXMaximum( xMax ); bBox.setYMaximum( yMax );
    }
    else
    {
      return 1;
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
        if ( L->crs().geographicFlag() )
          bufferDistance = 0.00000001;
        bBox.setXMinimum( bBox.xMinimum() - bufferDistance );
        bBox.setXMaximum( bBox.xMaximum() + bufferDistance );
        bBox.setYMinimum( bBox.yMinimum() - bufferDistance );
        bBox.setYMaximum( bBox.yMaximum() + bufferDistance );
      }
    }

    fit = L->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  }

  int addPartRet = 0;

  QgsFeature feat;
  while ( fit.nextFeature( feat ) )
  {
    QList<QgsGeometry*> newGeometries;
    QList<QgsPoint> topologyTestPoints;
    splitFunctionReturn = feat.geometry()->splitGeometry( splitLine, newGeometries, topologicalEditing, topologyTestPoints );
    if ( splitFunctionReturn == 0 )
    {
      //add new parts
      for ( int i = 0; i < newGeometries.size(); ++i )
      {
        addPartRet = feat.geometry()->addPart( newGeometries.at( i ) );
        if ( addPartRet )
          break;
      }

      // For test only: Exception already thrown here...
      // feat.geometry()->asWkb();

      if ( !addPartRet )
      {
        L->editBuffer()->changeGeometry( feat.id(), feat.geometry() );
      }
      else
      {
        // Test addPartRet
        switch ( addPartRet )
        {
          case 1:
            QgsDebugMsg( "Not a multipolygon" );
            break;

          case 2:
            QgsDebugMsg( "Not a valid geometry" );
            break;

          case 3:
            QgsDebugMsg( "New polygon ring" );
            break;
        }
      }
      L->editBuffer()->changeGeometry( feat.id(), feat.geometry() );

      if ( topologicalEditing )
      {
        QList<QgsPoint>::const_iterator topol_it = topologyTestPoints.constBegin();
        for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplittedParts;
    }
    else if ( splitFunctionReturn > 1 ) //1 means no split but also no error
    {
      returnCode = splitFunctionReturn;
    }

    qDeleteAll( newGeometries );
  }

  if ( numberOfSplittedParts == 0 && L->selectedFeatureCount() > 0  && returnCode == 0 )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    returnCode = 4;
  }

  return returnCode;
}


int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsGeometry* geom )
{
  if ( !L->hasGeometryType() )
    return 1;

  if ( !geom )
  {
    return 1;
  }

  int returnVal = 0;

  QGis::WkbType wkbType = geom->wkbType();

  switch ( wkbType )
  {
      //line
    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    {
      QgsPolyline theLine = geom->asPolyline();
      QgsPolyline::const_iterator line_it = theLine.constBegin();
      for ( ; line_it != theLine.constEnd(); ++line_it )
      {
        if ( addTopologicalPoints( *line_it ) != 0 )
        {
          returnVal = 2;
        }
      }
      break;
    }

    //multiline
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      QgsMultiPolyline theMultiLine = geom->asMultiPolyline();
      QgsPolyline currentPolyline;

      for ( int i = 0; i < theMultiLine.size(); ++i )
      {
        QgsPolyline::const_iterator line_it = currentPolyline.constBegin();
        for ( ; line_it != currentPolyline.constEnd(); ++line_it )
        {
          if ( addTopologicalPoints( *line_it ) != 0 )
          {
            returnVal = 2;
          }
        }
      }
      break;
    }

    //polygon
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      QgsPolygon thePolygon = geom->asPolygon();
      QgsPolyline currentRing;

      for ( int i = 0; i < thePolygon.size(); ++i )
      {
        currentRing = thePolygon.at( i );
        QgsPolyline::const_iterator line_it = currentRing.constBegin();
        for ( ; line_it != currentRing.constEnd(); ++line_it )
        {
          if ( addTopologicalPoints( *line_it ) != 0 )
          {
            returnVal = 2;
          }
        }
      }
      break;
    }

    //multipolygon
    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon theMultiPolygon = geom->asMultiPolygon();
      QgsPolygon currentPolygon;
      QgsPolyline currentRing;

      for ( int i = 0; i < theMultiPolygon.size(); ++i )
      {
        currentPolygon = theMultiPolygon.at( i );
        for ( int j = 0; j < currentPolygon.size(); ++j )
        {
          currentRing = currentPolygon.at( j );
          QgsPolyline::const_iterator line_it = currentRing.constBegin();
          for ( ; line_it != currentRing.constEnd(); ++line_it )
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
    default:
      break;
  }
  return returnVal;
}


int QgsVectorLayerEditUtils::addTopologicalPoints( const QgsPoint& p )
{
  if ( !L->hasGeometryType() )
    return 1;

  QMultiMap<double, QgsSnappingResult> snapResults; //results from the snapper object
  //we also need to snap to vertex to make sure the vertex does not already exist in this geometry
  QMultiMap<double, QgsSnappingResult> vertexSnapResults;

  QList<QgsSnappingResult> filteredSnapResults; //we filter out the results that are on existing vertices

  //work with a tolerance because coordinate projection may introduce some rounding
  double threshold =  0.0000001;
  if ( L->crs().mapUnits() == QGis::Meters )
  {
    threshold = 0.001;
  }
  else if ( L->crs().mapUnits() == QGis::Feet )
  {
    threshold = 0.0001;
  }


  if ( L->snapWithContext( p, threshold, snapResults, QgsSnapper::SnapToSegment ) != 0 )
  {
    return 2;
  }

  QMultiMap<double, QgsSnappingResult>::const_iterator snap_it = snapResults.constBegin();
  QMultiMap<double, QgsSnappingResult>::const_iterator vertex_snap_it;
  for ( ; snap_it != snapResults.constEnd(); ++snap_it )
  {
    //test if p is already a vertex of this geometry. If yes, don't insert it
    bool vertexAlreadyExists = false;
    if ( L->snapWithContext( p, threshold, vertexSnapResults, QgsSnapper::SnapToVertex ) != 0 )
    {
      continue;
    }

    vertex_snap_it = vertexSnapResults.constBegin();
    for ( ; vertex_snap_it != vertexSnapResults.constEnd(); ++vertex_snap_it )
    {
      if ( snap_it.value().snappedAtGeometry == vertex_snap_it.value().snappedAtGeometry )
      {
        vertexAlreadyExists = true;
      }
    }

    if ( !vertexAlreadyExists )
    {
      filteredSnapResults.push_back( *snap_it );
    }
  }
  insertSegmentVerticesForSnap( filteredSnapResults );
  return 0;
}


int QgsVectorLayerEditUtils::insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults )
{
  if ( !L->hasGeometryType() )
    return 1;

  int returnval = 0;
  QgsPoint layerPoint;

  QList<QgsSnappingResult>::const_iterator it = snapResults.constBegin();
  for ( ; it != snapResults.constEnd(); ++it )
  {
    if ( it->snappedVertexNr == -1 ) // segment snap
    {
      layerPoint = it->snappedVertex;
      if ( !insertVertex( layerPoint.x(), layerPoint.y(), it->snappedAtGeometry, it->afterVertexNr ) )
      {
        returnval = 3;
      }
    }
  }
  return returnval;
}




int QgsVectorLayerEditUtils::boundingBoxFromPointList( const QList<QgsPoint>& list, double& xmin, double& ymin, double& xmax, double& ymax ) const
{
  if ( list.size() < 1 )
  {
    return 1;
  }

  xmin = std::numeric_limits<double>::max();
  xmax = -std::numeric_limits<double>::max();
  ymin = std::numeric_limits<double>::max();
  ymax = -std::numeric_limits<double>::max();

  for ( QList<QgsPoint>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
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

  return 0;
}
