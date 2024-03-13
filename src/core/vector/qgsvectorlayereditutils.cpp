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

#include "qgsunsetattributevalue.h"
#include "qgsvectordataprovider.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgis.h"
#include "qgswkbtypes.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsgeometryoptions.h"
#include "qgsabstractgeometry.h"
#include "qgssettingsregistrycore.h"
#include "qgssettingsentryimpl.h"

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

  // If original point is not 3D but destination yes, check if it can be promoted
  if ( p.is3D() && !geometry.constGet()->is3D() && QgsWkbTypes::hasZ( mLayer->wkbType() ) )
  {
    if ( !geometry.get()->addZValue( QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->value() ) )
      return false;
  }

  // If original point has not M-value but destination yes, check if it can be promoted
  if ( p.isMeasure() && !geometry.constGet()->isMeasure() && QgsWkbTypes::hasM( mLayer->wkbType() ) )
  {
    if ( !geometry.get()->addMValue( QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->value() ) )
      return false;
  }

  if ( !geometry.moveVertex( p, atVertex ) )
    return false;

  return mLayer->changeGeometry( atFeatureId, geometry );
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


static
Qgis::GeometryOperationResult staticAddRing( QgsVectorLayer *layer, std::unique_ptr< QgsCurve > &ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureIds *modifiedFeatureIds, bool firstOne = true )
{

  if ( !layer || !layer->isSpatial() )
  {
    return Qgis::GeometryOperationResult::AddRingNotInExistingFeature;
  }

  if ( !ring )
  {
    return Qgis::GeometryOperationResult::InvalidInputGeometryType;
  }

  if ( !ring->isClosed() )
  {
    return Qgis::GeometryOperationResult::AddRingNotClosed;
  }

  if ( !layer->isValid() || !layer->editBuffer() || !layer->dataProvider() )
  {
    return Qgis::GeometryOperationResult::LayerNotEditable;
  }

  Qgis::GeometryOperationResult addRingReturnCode = Qgis::GeometryOperationResult::AddRingNotInExistingFeature; //default: return code for 'ring not inserted'
  QgsFeature f;

  QgsFeatureIterator fit;
  if ( !targetFeatureIds.isEmpty() )
  {
    //check only specified features
    fit = layer->getFeatures( QgsFeatureRequest().setFilterFids( targetFeatureIds ) );
  }
  else
  {
    //check all intersecting features
    QgsRectangle bBox = ring->boundingBox();
    fit = layer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) );
  }

  //find first valid feature we can add the ring to
  while ( fit.nextFeature( f ) )
  {
    if ( !f.hasGeometry() )
      continue;

    //add ring takes ownership of ring, and deletes it if there's an error
    QgsGeometry g = f.geometry();

    if ( ring->orientation() != g.polygonOrientation() )
    {
      addRingReturnCode = g.addRing( static_cast< QgsCurve * >( ring->clone() ) );
    }
    else
    {
      addRingReturnCode = g.addRing( static_cast< QgsCurve * >( ring->reversed() ) );
    }
    if ( addRingReturnCode == Qgis::GeometryOperationResult::Success )
    {
      layer->changeGeometry( f.id(), g );
      if ( modifiedFeatureIds )
      {
        modifiedFeatureIds->insert( f.id() );
        if ( firstOne )
        {
          break;
        }
      }

    }
  }

  return addRingReturnCode;
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
  std::unique_ptr<QgsCurve> uniquePtrRing( ring );
  if ( modifiedFeatureId )
  {
    QgsFeatureIds *modifiedFeatureIds = new QgsFeatureIds;
    Qgis::GeometryOperationResult result = staticAddRing( mLayer, uniquePtrRing, targetFeatureIds, modifiedFeatureIds, true );
    *modifiedFeatureId = *modifiedFeatureIds->begin();
    return result;
  }
  return staticAddRing( mLayer, uniquePtrRing, targetFeatureIds, nullptr, true );
}

Qgis::GeometryOperationResult QgsVectorLayerEditUtils::addRingV2( QgsCurve *ring, const QgsFeatureIds &targetFeatureIds, QgsFeatureIds *modifiedFeatureIds )
{

  std::unique_ptr<QgsCurve> uniquePtrRing( ring );
  return staticAddRing( mLayer, uniquePtrRing, targetFeatureIds, modifiedFeatureIds, false );
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
    if ( ring->orientation() != geometry.polygonOrientation() )
    {
      ring = ring->reversed();
    }
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

    features = mLayer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) );
  }

  QgsVectorLayerUtils::QgsFeaturesDataList featuresDataToAdd;

  const int fieldCount = mLayer->fields().count();

  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    if ( !feat.hasGeometry() )
    {
      continue;
    }
    QVector<QgsGeometry> newGeometries;
    QgsPointSequence featureTopologyTestPoints;
    const QgsGeometry originalGeom = feat.geometry();
    QgsGeometry featureGeom = originalGeom;
    splitFunctionReturn = featureGeom.splitGeometry( curve, newGeometries, preserveCircular, topologicalEditing, featureTopologyTestPoints );
    topologyTestPoints.append( featureTopologyTestPoints );
    if ( splitFunctionReturn == Qgis::GeometryOperationResult::Success )
    {
      //change this geometry
      mLayer->changeGeometry( feat.id(), featureGeom );

      //update any attributes for original feature which are set to GeometryRatio split policy
      QgsAttributeMap attributeMap;
      for ( int fieldIdx = 0; fieldIdx < fieldCount; ++fieldIdx )
      {
        const QgsField field = mLayer->fields().at( fieldIdx );
        switch ( field.splitPolicy() )
        {
          case Qgis::FieldDomainSplitPolicy::DefaultValue:
          case Qgis::FieldDomainSplitPolicy::Duplicate:
          case Qgis::FieldDomainSplitPolicy::UnsetField:
            break;

          case Qgis::FieldDomainSplitPolicy::GeometryRatio:
          {
            if ( field.isNumeric() )
            {
              const double originalValue = feat.attribute( fieldIdx ).toDouble();

              double originalSize = 0;

              switch ( originalGeom.type() )
              {
                case Qgis::GeometryType::Point:
                case Qgis::GeometryType::Unknown:
                case Qgis::GeometryType::Null:
                  originalSize = 0;
                  break;
                case Qgis::GeometryType::Line:
                  originalSize = originalGeom.length();
                  break;
                case Qgis::GeometryType::Polygon:
                  originalSize = originalGeom.area();
                  break;
              }

              double newSize = 0;
              switch ( featureGeom.type() )
              {
                case Qgis::GeometryType::Point:
                case Qgis::GeometryType::Unknown:
                case Qgis::GeometryType::Null:
                  newSize = 0;
                  break;
                case Qgis::GeometryType::Line:
                  newSize = featureGeom.length();
                  break;
                case Qgis::GeometryType::Polygon:
                  newSize = featureGeom.area();
                  break;
              }

              attributeMap.insert( fieldIdx, originalSize > 0 ? ( originalValue * newSize / originalSize ) : originalValue );
            }
            break;
          }
        }
      }

      if ( !attributeMap.isEmpty() )
      {
        mLayer->changeAttributeValues( feat.id(), attributeMap );
      }

      //insert new features
      for ( const QgsGeometry &geom : std::as_const( newGeometries ) )
      {
        QgsAttributeMap attributeMap;
        for ( int fieldIdx = 0; fieldIdx < fieldCount; ++fieldIdx )
        {
          const QgsField field = mLayer->fields().at( fieldIdx );
          // respect field split policy
          switch ( field.splitPolicy() )
          {
            case Qgis::FieldDomainSplitPolicy::DefaultValue:
              // TODO!!!

              break;

            case Qgis::FieldDomainSplitPolicy::Duplicate:
              attributeMap.insert( fieldIdx, feat.attribute( fieldIdx ) );
              break;

            case Qgis::FieldDomainSplitPolicy::GeometryRatio:
            {
              if ( !field.isNumeric() )
              {
                attributeMap.insert( fieldIdx, feat.attribute( fieldIdx ) );
              }
              else
              {
                const double originalValue = feat.attribute( fieldIdx ).toDouble();

                double originalSize = 0;

                switch ( originalGeom.type() )
                {
                  case Qgis::GeometryType::Point:
                  case Qgis::GeometryType::Unknown:
                  case Qgis::GeometryType::Null:
                    originalSize = 0;
                    break;
                  case Qgis::GeometryType::Line:
                    originalSize = originalGeom.length();
                    break;
                  case Qgis::GeometryType::Polygon:
                    originalSize = originalGeom.area();
                    break;
                }

                double newSize = 0;
                switch ( geom.type() )
                {
                  case Qgis::GeometryType::Point:
                  case Qgis::GeometryType::Unknown:
                  case Qgis::GeometryType::Null:
                    newSize = 0;
                    break;
                  case Qgis::GeometryType::Line:
                    newSize = geom.length();
                    break;
                  case Qgis::GeometryType::Polygon:
                    newSize = geom.area();
                    break;
                }

                attributeMap.insert( fieldIdx, originalSize > 0 ? ( originalValue * newSize / originalSize ) : originalValue );
              }
              break;
            }

            case Qgis::FieldDomainSplitPolicy::UnsetField:
              attributeMap.insert( fieldIdx, QgsUnsetAttributeValue() );
              break;
          }
        }

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

    fit = mLayer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) );
  }

  QgsFeature feat;
  while ( fit.nextFeature( feat ) )
  {
    QgsGeometry featureGeom = feat.geometry();

    const QVector<QgsGeometry> geomCollection = featureGeom.asGeometryCollection();
    QVector<QgsGeometry> resultCollection;
    QgsPointSequence topologyTestPoints;
    for ( QgsGeometry part : geomCollection )
    {
      QVector<QgsGeometry> newGeometries;
      QgsPointSequence partTopologyTestPoints;

      const Qgis::GeometryOperationResult splitFunctionReturn = part.splitGeometry( splitLine, newGeometries, topologicalEditing, partTopologyTestPoints, false );

      if ( splitFunctionReturn == Qgis::GeometryOperationResult::Success && !newGeometries.isEmpty() )
      {
        for ( int i = 0; i < newGeometries.size(); ++i )
        {
          resultCollection.append( newGeometries.at( i ).asGeometryCollection() );
        }

        topologyTestPoints.append( partTopologyTestPoints );

        ++numberOfSplitParts;
      }
      // Note: For multilinestring layers, when the split line does not intersect the feature part,
      // QgsGeometry::splitGeometry returns InvalidBaseGeometry instead of NothingHappened
      else if ( splitFunctionReturn == Qgis::GeometryOperationResult::NothingHappened ||
                splitFunctionReturn == Qgis::GeometryOperationResult::InvalidBaseGeometry )
      {
        // Add part as is
        resultCollection.append( part );
      }
      else if ( splitFunctionReturn != Qgis::GeometryOperationResult::Success )
      {
        return splitFunctionReturn;
      }
    }

    QgsGeometry newGeom = QgsGeometry::collectGeometry( resultCollection );
    mLayer->changeGeometry( feat.id(), newGeom ) ;

    if ( topologicalEditing )
    {
      QgsPointSequence::const_iterator topol_it = topologyTestPoints.constBegin();
      for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
      {
        addTopologicalPoints( *topol_it );
      }
    }

  }
  if ( numberOfSplitParts == 0 && mLayer->selectedFeatureCount() > 0 )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    return Qgis::GeometryOperationResult::NothingHappened;
  }

  return Qgis::GeometryOperationResult::Success;
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
    threshold = 1e-8;

    if ( mLayer->crs().mapUnits() == Qgis::DistanceUnit::Meters )
    {
      threshold = 0.001;
    }
    else if ( mLayer->crs().mapUnits() == Qgis::DistanceUnit::Feet )
    {
      threshold = 0.0001;
    }
  }

  QgsRectangle searchRect( p, p, false );
  searchRect.grow( threshold );

  QgsFeature f;
  QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest()
                           .setFilterRect( searchRect )
                           .setFlags( Qgis::FeatureRequestFlag::ExactIntersect )
                           .setNoAttributes() );

  bool pointsAdded = false;
  while ( fit.nextFeature( f ) )
  {
    QgsGeometry geom = f.geometry();
    if ( geom.addTopologicalPoint( p, threshold, segmentSearchEpsilon ) )
    {
      pointsAdded = true;
      mLayer->changeGeometry( f.id(), geom );
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

bool QgsVectorLayerEditUtils::mergeFeatures( const QgsFeatureId &targetFeatureId, const QgsFeatureIds &mergeFeatureIds, const QgsAttributes &mergeAttributes, const QgsGeometry &unionGeometry, QString &errorMessage )
{
  errorMessage.clear();

  if ( mergeFeatureIds.isEmpty() )
  {
    errorMessage = QObject::tr( "List of features to merge is empty" );
    return false;
  }

  QgsAttributeMap newAttributes;
  for ( int i = 0; i < mergeAttributes.count(); ++i )
  {
    QVariant val = mergeAttributes.at( i );

    bool isDefaultValue = mLayer->fields().fieldOrigin( i ) == QgsFields::OriginProvider &&
                          mLayer->dataProvider() &&
                          mLayer->dataProvider()->defaultValueClause( mLayer->fields().fieldOriginIndex( i ) ) == val;

    // convert to destination data type
    QString errorMessageConvertCompatible;
    if ( !isDefaultValue && !mLayer->fields().at( i ).convertCompatible( val, &errorMessageConvertCompatible ) )
    {
      if ( errorMessage.isEmpty() )
        errorMessage = QObject::tr( "Could not store value '%1' in field of type %2: %3" ).arg( mergeAttributes.at( i ).toString(), mLayer->fields().at( i ).typeName(), errorMessageConvertCompatible );
    }
    newAttributes[ i ] = val;
  }

  mLayer->beginEditCommand( QObject::tr( "Merged features" ) );

  // Delete other features but the target feature
  QgsFeatureIds::const_iterator feature_it = mergeFeatureIds.constBegin();
  for ( ; feature_it != mergeFeatureIds.constEnd(); ++feature_it )
  {
    if ( *feature_it != targetFeatureId )
      mLayer->deleteFeature( *feature_it );
  }

  // Modify target feature or create a new one if invalid
  QgsGeometry mergeGeometry = unionGeometry;
  if ( targetFeatureId == FID_NULL )
  {
    QgsFeature mergeFeature = QgsVectorLayerUtils::createFeature( mLayer, mergeGeometry, newAttributes );
    mLayer->addFeature( mergeFeature );
  }
  else
  {
    mLayer->changeGeometry( targetFeatureId, mergeGeometry );
    mLayer->changeAttributeValues( targetFeatureId, newAttributes );
  }

  mLayer->endEditCommand();

  mLayer->triggerRepaint();

  return true;
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
