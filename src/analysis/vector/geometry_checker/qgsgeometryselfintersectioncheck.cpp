/***************************************************************************
    qgsgeometryselfintersectioncheck.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryselfintersectioncheck.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsgeometryengine.h"
#include "qgsmultipolygon.h"
#include "qgsmultilinestring.h"
#include "qgsgeometryutils.h"
#include "qgsfeaturepool.h"

bool QgsGeometrySelfIntersectionCheckError::isEqual( QgsGeometryCheckError *other ) const
{
  return QgsGeometryCheckError::isEqual( other ) &&
         static_cast<QgsGeometrySelfIntersectionCheckError *>( other )->intersection().segment1 == intersection().segment1 &&
         static_cast<QgsGeometrySelfIntersectionCheckError *>( other )->intersection().segment2 == intersection().segment2;
}

bool QgsGeometrySelfIntersectionCheckError::handleChanges( const QgsGeometryCheck::Changes &changes )
{
  if ( !QgsGeometryCheckError::handleChanges( changes ) )
  {
    return false;
  }
  for ( const QgsGeometryCheck::Change &change : changes[layerId()].value( featureId() ) )
  {
    if ( change.vidx.vertex == mInter.segment1 ||
         change.vidx.vertex == mInter.segment1 + 1 ||
         change.vidx.vertex == mInter.segment2 ||
         change.vidx.vertex == mInter.segment2 + 1 )
    {
      return false;
    }
    else if ( change.vidx.vertex >= 0 )
    {
      if ( change.vidx.vertex < mInter.segment1 )
      {
        mInter.segment1 += change.type == QgsGeometryCheck::ChangeAdded ? 1 : -1;
      }
      if ( change.vidx.vertex < mInter.segment2 )
      {
        mInter.segment2 += change.type == QgsGeometryCheck::ChangeAdded ? 1 : -1;
      }
    }
  }
  return true;
}


void QgsGeometrySelfIntersectionCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        for ( const QgsGeometryUtils::SelfIntersection &inter : QgsGeometryUtils::selfIntersections( geom, iPart, iRing, mContext->tolerance ) )
        {
          errors.append( new QgsGeometrySelfIntersectionCheckError( this, layerFeature, inter.point, QgsVertexId( iPart, iRing ), inter ) );
        }
      }
    }
  }
}

void QgsGeometrySelfIntersectionCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->getFeature( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.get();
  QgsVertexId vidx = error->vidx();

  // Check if ring still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  const QgsGeometryUtils::SelfIntersection &inter = static_cast<QgsGeometrySelfIntersectionCheckError *>( error )->intersection();
  // Check if error still applies
  bool ringIsClosed = false;
  int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, vidx.part, vidx.ring, &ringIsClosed );
  if ( nVerts == 0 || inter.segment1 >= nVerts || inter.segment2 >= nVerts )
  {
    error->setObsolete();
    return;
  }
  QgsPoint p1 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, inter.segment1 ) );
  QgsPoint q1 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, inter.segment2 ) );
  QgsPoint p2 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( inter.segment1 + 1 ) % nVerts ) );
  QgsPoint q2 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( inter.segment2 + 1 ) % nVerts ) );
  QgsPoint s;
  bool intersection = false;
  if ( !QgsGeometryUtils::segmentIntersection( p1, p2, q1, q2, s, intersection, mContext->tolerance ) )
  {
    error->setObsolete();
    return;
  }

  // Fix with selected method
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == ToMultiObject || method == ToSingleObjects )
  {
    // Extract rings
    QgsPointSequence ring1, ring2;
    bool ring1EndsWithS = false;
    bool ring2EndsWithS = false;
    for ( int i = 0; i < nVerts; ++i )
    {
      if ( i <= inter.segment1 || i >= inter.segment2 + 1 )
      {
        ring1.append( geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, i ) ) );
        ring1EndsWithS = false;
        if ( i == inter.segment2 + 1 )
        {
          ring2.append( s );
          ring2EndsWithS = true;
        }
      }
      else
      {
        ring2.append( geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, i ) ) );
        ring2EndsWithS = true;
        if ( i == inter.segment1 + 1 )
        {
          ring1.append( s );
          ring1EndsWithS = false;
        }
      }
    }
    if ( nVerts == inter.segment2 + 1 )
    {
      ring2.append( s );
      ring2EndsWithS = true;
    }
    if ( ringIsClosed || ring1EndsWithS )
      ring1.append( ring1.front() ); // Ensure ring is closed
    if ( ringIsClosed || ring2EndsWithS )
      ring2.append( ring2.front() ); // Ensure ring is closed

    if ( ring1.size() < 3 + ( ringIsClosed || ring1EndsWithS ) || ring2.size() < 3 + ( ringIsClosed || ring2EndsWithS ) )
    {
      error->setFixFailed( tr( "Resulting geometry is degenerate" ) );
      return;
    }
    QgsLineString *ringGeom1 = new QgsLineString();
    ringGeom1->setPoints( ring1 );
    QgsLineString *ringGeom2 = new QgsLineString();
    ringGeom2->setPoints( ring2 );

    QgsAbstractGeometry *part = QgsGeometryCheckerUtils::getGeomPart( geom, vidx.part );
    // If is a polygon...
    if ( dynamic_cast<QgsCurvePolygon *>( part ) )
    {
      QgsCurvePolygon *poly = static_cast<QgsCurvePolygon *>( part );
      // If self-intersecting ring is an interior ring, create separate holes
      if ( vidx.ring > 0 )
      {
        poly->removeInteriorRing( vidx.ring );
        poly->addInteriorRing( ringGeom1 );
        poly->addInteriorRing( ringGeom2 );
        changes[error->layerId()][feature.id()].append( Change( ChangeRing, ChangeRemoved, vidx ) );
        changes[error->layerId()][feature.id()].append( Change( ChangeRing, ChangeAdded, QgsVertexId( vidx.part, poly->ringCount() - 2 ) ) );
        changes[error->layerId()][feature.id()].append( Change( ChangeRing, ChangeAdded, QgsVertexId( vidx.part, poly->ringCount() - 1 ) ) );
        feature.setGeometry( featureGeom );
        featurePool->updateFeature( feature );
      }
      else
      {
        // If ring is exterior, build two polygons, and reassign interiors as necessary
        poly->setExteriorRing( ringGeom1 );

        // If original feature was a linear polygon, also create the new part as a linear polygon
        QgsCurvePolygon *poly2 = dynamic_cast<QgsPolygon *>( part ) ? new QgsPolygon() : new QgsCurvePolygon();
        poly2->setExteriorRing( ringGeom2 );

        // Reassing interiors as necessary
        std::unique_ptr< QgsGeometryEngine > geomEnginePoly1 = QgsGeometryCheckerUtils::createGeomEngine( poly, mContext->tolerance );
        std::unique_ptr< QgsGeometryEngine > geomEnginePoly2 = QgsGeometryCheckerUtils::createGeomEngine( poly2, mContext->tolerance );
        for ( int n = poly->numInteriorRings(), i = n - 1; i >= 0; --i )
        {
          if ( !geomEnginePoly1->contains( poly->interiorRing( i ) ) )
          {
            if ( geomEnginePoly2->contains( poly->interiorRing( i ) ) )
            {
              poly2->addInteriorRing( static_cast<QgsCurve *>( poly->interiorRing( i )->clone() ) );
              // No point in adding ChangeAdded changes, since the entire poly2 is added anyways later on
            }
            poly->removeInteriorRing( i );
            changes[error->layerId()][feature.id()].append( Change( ChangeRing, ChangeRemoved, QgsVertexId( vidx.part, 1 + i ) ) );
          }
        }

        if ( method == ToMultiObject )
        {
          // If is already a geometry collection, just add the new polygon.
          if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
          {
            static_cast<QgsGeometryCollection *>( geom )->addGeometry( poly2 );
            changes[error->layerId()][feature.id()].append( Change( ChangeRing, ChangeChanged, QgsVertexId( vidx.part, vidx.ring ) ) );
            changes[error->layerId()][feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geom->partCount() - 1 ) ) );
            feature.setGeometry( featureGeom );
            featurePool->updateFeature( feature );
          }
          // Otherwise, create multipolygon
          else
          {
            QgsMultiPolygon *multiPoly = new QgsMultiPolygon();
            multiPoly->addGeometry( poly->clone() );
            multiPoly->addGeometry( poly2 );
            feature.setGeometry( QgsGeometry( multiPoly ) );
            featurePool->updateFeature( feature );
            changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
          }
        }
        else // if ( method == ToSingleObjects )
        {
          QgsFeature newFeature;
          newFeature.setAttributes( feature.attributes() );
          newFeature.setGeometry( QgsGeometry( poly2 ) );
          feature.setGeometry( featureGeom );
          featurePool->updateFeature( feature );
          featurePool->addFeature( newFeature );
          changes[error->layerId()][feature.id()].append( Change( ChangeRing, ChangeChanged, QgsVertexId( vidx.part, vidx.ring ) ) );
          changes[error->layerId()][newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
        }
      }
    }
    else if ( dynamic_cast<QgsCurve *>( part ) )
    {
      if ( method == ToMultiObject )
      {
        if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
        {
          QgsGeometryCollection *geomCollection = static_cast<QgsGeometryCollection *>( geom );
          geomCollection->removeGeometry( vidx.part );
          geomCollection->addGeometry( ringGeom1 );
          geomCollection->addGeometry( ringGeom2 );
          feature.setGeometry( featureGeom );
          featurePool->updateFeature( feature );
          changes[error->layerId()][feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( vidx.part ) ) );
          changes[error->layerId()][feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 2 ) ) );
          changes[error->layerId()][feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 1 ) ) );
        }
        else
        {
          QgsMultiCurve *geomCollection = new QgsMultiLineString();
          geomCollection->addGeometry( ringGeom1 );
          geomCollection->addGeometry( ringGeom2 );
          feature.setGeometry( QgsGeometry( geomCollection ) );
          featurePool->updateFeature( feature );
          changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
        }
      }
      else // if(method == ToSingleObjects)
      {
        if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
        {
          QgsGeometryCollection *geomCollection = static_cast<QgsGeometryCollection *>( geom );
          geomCollection->removeGeometry( vidx.part );
          geomCollection->addGeometry( ringGeom1 );
          feature.setGeometry( featureGeom );
          featurePool->updateFeature( feature );
          changes[error->layerId()][feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( vidx.part ) ) );
          changes[error->layerId()][feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 1 ) ) );
        }
        else
        {
          feature.setGeometry( QgsGeometry( ringGeom1 ) );
          featurePool->updateFeature( feature );
          changes[error->layerId()][feature.id()].append( Change( ChangeFeature, ChangeChanged, QgsVertexId( vidx.part ) ) );
        }
        QgsFeature newFeature;
        newFeature.setAttributes( feature.attributes() );
        newFeature.setGeometry( QgsGeometry( ringGeom2 ) );
        featurePool->addFeature( newFeature );
        changes[error->layerId()][newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
      }
    }
    else
    {
      delete ringGeom1;
      delete ringGeom2;
    }
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometrySelfIntersectionCheck::resolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Split feature into a multi-object feature" )
                               << tr( "Split feature into multiple single-object features" )
                               << tr( "No action" );
  return methods;
}
