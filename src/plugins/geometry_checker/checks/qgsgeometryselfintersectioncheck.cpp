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
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"
#include "qgsgeometryengine.h"
#include "qgsmultipolygonv2.h"
#include "qgsmultilinestringv2.h"
#include "qgsgeometryutils.h"
#include "../utils/qgsfeaturepool.h"

bool QgsGeometrySelfIntersectionCheckError::isEqual( QgsGeometryCheckError* other ) const
{
  return QgsGeometryCheckError::isEqual( other ) &&
         other->featureId() == featureId() &&
         other->vidx() == vidx() &&
         static_cast<QgsGeometrySelfIntersectionCheckError*>( other )->intersection().segment1 == intersection().segment1 &&
         static_cast<QgsGeometrySelfIntersectionCheckError*>( other )->intersection().segment2 == intersection().segment2;
}

bool QgsGeometrySelfIntersectionCheckError::handleChanges( const QgsGeometryCheck::Changes& changes )
{
  if ( !QgsGeometryCheckError::handleChanges( changes ) )
  {
    return false;
  }
  Q_FOREACH ( const QgsGeometryCheck::Change& change, changes.value( featureId() ) )
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


void QgsGeometrySelfIntersectionCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds& featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( QgsFeatureId featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }
    QgsAbstractGeometryV2* geom = feature.geometry()->geometry();

    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        Q_FOREACH ( const QgsGeometryUtils::SelfIntersection& inter, QgsGeometryUtils::getSelfIntersections( geom, iPart, iRing, QgsGeometryCheckPrecision::tolerance() ) )
        {
          errors.append( new QgsGeometrySelfIntersectionCheckError( this, featureid, inter.point, QgsVertexId( iPart, iRing ), inter ) );
        }
      }
    }
  }
}

void QgsGeometrySelfIntersectionCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
  QgsVertexId vidx = error->vidx();

  // Check if ring still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  const QgsGeometryUtils::SelfIntersection& inter = static_cast<QgsGeometrySelfIntersectionCheckError*>( error )->intersection();
  // Check if error still applies
  bool ringIsClosed = false;
  int nVerts = QgsGeomUtils::polyLineSize( geom, vidx.part, vidx.ring, &ringIsClosed );
  if ( inter.segment1 >= nVerts || inter.segment2 >= nVerts )
  {
    error->setObsolete();
    return;
  }
  QgsPointV2 p1 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, inter.segment1 ) );
  QgsPointV2 q1 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, inter.segment2 ) );
  QgsPointV2 p2 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( inter.segment1 + 1 ) % nVerts ) );
  QgsPointV2 q2 = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( inter.segment2 + 1 ) % nVerts ) );
  QgsPointV2 s;
  if ( !QgsGeometryUtils::segmentIntersection( p1, p2, q1, q2, s, QgsGeometryCheckPrecision::tolerance() ) )
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
    QgsPointSequenceV2 ring1, ring2;
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
    QgsLineStringV2* ringGeom1 = new QgsLineStringV2();
    ringGeom1->setPoints( ring1 );
    QgsLineStringV2* ringGeom2 = new QgsLineStringV2();
    ringGeom2->setPoints( ring2 );

    QgsAbstractGeometryV2* part = QgsGeomUtils::getGeomPart( geom, vidx.part );
    // If is a polygon...
    if ( dynamic_cast<QgsCurvePolygonV2*>( part ) )
    {
      QgsCurvePolygonV2* poly = static_cast<QgsCurvePolygonV2*>( part );
      // If ring is interior ring, create separate holes
      if ( vidx.ring > 0 )
      {
        poly->removeInteriorRing( vidx.ring );
        poly->addInteriorRing( ringGeom1 );
        poly->addInteriorRing( ringGeom2 );
        changes[feature.id()].append( Change( ChangeRing, ChangeRemoved, vidx ) );
        changes[feature.id()].append( Change( ChangeRing, ChangeAdded, QgsVertexId( vidx.part, poly->ringCount() - 2 ) ) );
        changes[feature.id()].append( Change( ChangeRing, ChangeAdded, QgsVertexId( vidx.part, poly->ringCount() - 1 ) ) );
        mFeaturePool->updateFeature( feature );
      }
      else
      {
        // If ring is exterior, build two polygons, and reassign interiors as necessary
        poly->setExteriorRing( ringGeom1 );

        // If original feature was a linear polygon, also create the new part as a linear polygon
        QgsCurvePolygonV2* poly2 = dynamic_cast<QgsPolygonV2*>( part ) ? new QgsPolygonV2() : new QgsCurvePolygonV2();
        poly2->setExteriorRing( ringGeom2 );

        // Reassing interiors as necessary
        QgsGeometryEngine* geomEnginePoly1 = QgsGeomUtils::createGeomEngine( poly, QgsGeometryCheckPrecision::tolerance() );
        QgsGeometryEngine* geomEnginePoly2 = QgsGeomUtils::createGeomEngine( poly2, QgsGeometryCheckPrecision::tolerance() );
        for ( int n = poly->numInteriorRings(), i = n - 1; i >= 0; --i )
        {
          if ( !geomEnginePoly1->contains( *poly->interiorRing( i ) ) )
          {
            if ( geomEnginePoly2->contains( *poly->interiorRing( i ) ) )
            {
              poly2->addInteriorRing( static_cast<QgsCurveV2*>( poly->interiorRing( i )->clone() ) );
              // No point in adding ChangeAdded changes, since the entire poly2 is added anyways later on
            }
            poly->removeInteriorRing( i );
            changes[feature.id()].append( Change( ChangeRing, ChangeRemoved, QgsVertexId( vidx.part, i ) ) );
          }
        }
        delete geomEnginePoly1;
        delete geomEnginePoly2;

        if ( method == ToMultiObject )
        {
          // If is alreay a geometry collection, just add the new polygon.
          if ( dynamic_cast<QgsGeometryCollectionV2*>( geom ) )
          {
            static_cast<QgsGeometryCollectionV2*>( geom )->addGeometry( poly2 );
            changes[feature.id()].append( Change( ChangeRing, ChangeChanged, QgsVertexId( vidx.part, vidx.ring ) ) );
            changes[feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geom->partCount() - 1 ) ) );
            mFeaturePool->updateFeature( feature );
          }
          // Otherwise, create multipolygon
          else
          {
            QgsMultiPolygonV2* multiPoly = new QgsMultiPolygonV2();
            multiPoly->addGeometry( poly->clone() );
            multiPoly->addGeometry( poly2 );
            feature.setGeometry( new QgsGeometry( multiPoly ) );
            mFeaturePool->updateFeature( feature );
            changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
          }
        }
        else // if ( method == ToSingleObjects )
        {
          QgsFeature newFeature;
          newFeature.setAttributes( feature.attributes() );
          newFeature.setGeometry( new QgsGeometry( poly2 ) );
          mFeaturePool->updateFeature( feature );
          mFeaturePool->addFeature( newFeature );
          changes[feature.id()].append( Change( ChangeRing, ChangeChanged, QgsVertexId( vidx.part, vidx.ring ) ) );
          changes[newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
        }
      }
    }
    else if ( dynamic_cast<QgsCurveV2*>( part ) )
    {
      if ( method == ToMultiObject )
      {
        if ( dynamic_cast<QgsGeometryCollectionV2*>( geom ) )
        {
          QgsGeometryCollectionV2* geomCollection = static_cast<QgsGeometryCollectionV2*>( geom );
          geomCollection->removeGeometry( vidx.part );
          geomCollection->addGeometry( ringGeom1 );
          geomCollection->addGeometry( ringGeom2 );
          mFeaturePool->updateFeature( feature );
          changes[feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( vidx.part ) ) );
          changes[feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 2 ) ) );
          changes[feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 1 ) ) );
        }
        else
        {
          QgsMultiCurveV2* geomCollection = new QgsMultiLineStringV2();
          geomCollection->addGeometry( ringGeom1 );
          geomCollection->addGeometry( ringGeom2 );
          feature.setGeometry( new QgsGeometry( geomCollection ) );
          mFeaturePool->updateFeature( feature );
          changes[feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
        }
      }
      else // if(method == ToSingleObjects)
      {
        if ( dynamic_cast<QgsGeometryCollectionV2*>( geom ) )
        {
          QgsGeometryCollectionV2* geomCollection = static_cast<QgsGeometryCollectionV2*>( geom );
          geomCollection->removeGeometry( vidx.part );
          geomCollection->addGeometry( ringGeom1 );
          mFeaturePool->updateFeature( feature );
          changes[feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( vidx.part ) ) );
          changes[feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 1 ) ) );
        }
        else
        {
          feature.setGeometry( new QgsGeometry( ringGeom1 ) );
          mFeaturePool->updateFeature( feature );
          changes[feature.id()].append( Change( ChangeFeature, ChangeChanged, QgsVertexId( vidx.part ) ) );
        }
        QgsFeature newFeature;
        newFeature.setAttributes( feature.attributes() );
        newFeature.setGeometry( new QgsGeometry( ringGeom2 ) );
        mFeaturePool->addFeature( newFeature );
        changes[newFeature.id()].append( Change( ChangeFeature, ChangeAdded ) );
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

const QStringList& QgsGeometrySelfIntersectionCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Split feature into a multi-object feature" )
                               << tr( "Split feature into multiple single-object features" )
                               << tr( "No action" );
  return methods;
}
