/***************************************************************************
                        qgsgeometryeditutils.cpp
  -------------------------------------------------------------------
Date                 : 21 Jan 2015
Copyright            : (C) 2015 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryeditutils.h"
#include "qgsfeatureiterator.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgspolygon.h"
#include "qgsgeometryutils.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsmultisurface.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include <limits>

QgsGeometry::OperationResult QgsGeometryEditUtils::addRing( QgsAbstractGeometry *geom, std::unique_ptr<QgsCurve> ring )
{
  if ( !ring )
  {
    return QgsGeometry::InvalidInput;
  }

  QVector< QgsCurvePolygon * > polygonList;
  QgsCurvePolygon *curvePoly = qgsgeometry_cast< QgsCurvePolygon * >( geom );
  QgsGeometryCollection *multiGeom = qgsgeometry_cast< QgsGeometryCollection * >( geom );
  if ( curvePoly )
  {
    polygonList.append( curvePoly );
  }
  else if ( multiGeom )
  {
    polygonList.reserve( multiGeom->numGeometries() );
    for ( int i = 0; i < multiGeom->numGeometries(); ++i )
    {
      polygonList.append( qgsgeometry_cast< QgsCurvePolygon * >( multiGeom->geometryN( i ) ) );
    }
  }
  else
  {
    return QgsGeometry::InvalidInput; //not polygon / multipolygon;
  }

  //ring must be closed
  if ( !ring->isClosed() )
  {
    return QgsGeometry::AddRingNotClosed;
  }
  else if ( !ring->isRing() )
  {
    return QgsGeometry::AddRingNotValid;
  }

  std::unique_ptr<QgsGeometryEngine> ringGeom( QgsGeometry::createGeometryEngine( ring.get() ) );
  ringGeom->prepareGeometry();

  //for each polygon, test if inside outer ring and no intersection with other interior ring
  QVector< QgsCurvePolygon * >::const_iterator polyIter = polygonList.constBegin();
  for ( ; polyIter != polygonList.constEnd(); ++polyIter )
  {
    if ( ringGeom->within( *polyIter ) )
    {
      //check if disjoint with other interior rings
      int nInnerRings = ( *polyIter )->numInteriorRings();
      for ( int i = 0; i < nInnerRings; ++i )
      {
        if ( !ringGeom->disjoint( ( *polyIter )->interiorRing( i ) ) )
        {
          return QgsGeometry::AddRingCrossesExistingRings;
        }
      }

      //make sure dimensionality of ring matches geometry
      if ( QgsWkbTypes::hasZ( geom->wkbType() ) )
        ring->addZValue( 0 );
      if ( QgsWkbTypes::hasM( geom->wkbType() ) )
        ring->addMValue( 0 );

      ( *polyIter )->addInteriorRing( ring.release() );
      return QgsGeometry::Success; //success
    }
  }
  return QgsGeometry::AddRingNotInExistingFeature; //not contained in any outer ring
}

QgsGeometry::OperationResult QgsGeometryEditUtils::addPart( QgsAbstractGeometry *geom, std::unique_ptr<QgsAbstractGeometry> part )
{
  if ( !geom )
  {
    return QgsGeometry::InvalidBaseGeometry;
  }

  if ( !part )
  {
    return QgsGeometry::InvalidInput;
  }

  //multitype?
  QgsGeometryCollection *geomCollection = qgsgeometry_cast<QgsGeometryCollection *>( geom );
  if ( !geomCollection )
  {
    return QgsGeometry::AddPartNotMultiGeometry;
  }

  bool added = false;
  if ( QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiSurface
       || QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiPolygon )
  {
    QgsCurve *curve = qgsgeometry_cast<QgsCurve *>( part.get() );

    if ( curve && curve->isClosed() && curve->numPoints() >= 4 )
    {
      std::unique_ptr<QgsCurvePolygon> poly;
      if ( QgsWkbTypes::flatType( curve->wkbType() ) == QgsWkbTypes::LineString )
      {
        poly = qgis::make_unique< QgsPolygon >();
      }
      else
      {
        poly = qgis::make_unique< QgsCurvePolygon >();
      }
      // Ownership is still with part, curve points to the same object and is transferred
      // to poly here.
      part.release();
      poly->setExteriorRing( curve );
      added = geomCollection->addGeometry( poly.release() );
    }
    else if ( QgsWkbTypes::flatType( part->wkbType() ) == QgsWkbTypes::Polygon )
    {
      added = geomCollection->addGeometry( part.release() );
    }
    else if ( QgsWkbTypes::flatType( part->wkbType() ) == QgsWkbTypes::MultiPolygon )
    {
      std::unique_ptr<QgsGeometryCollection> parts( static_cast<QgsGeometryCollection *>( part.release() ) );

      int i;
      int n = geomCollection->numGeometries();
      for ( i = 0; i < parts->numGeometries() && geomCollection->addGeometry( parts->geometryN( i )->clone() ); i++ )
        ;

      added = i == parts->numGeometries();
      if ( !added )
      {
        while ( geomCollection->numGeometries() > n )
          geomCollection->removeGeometry( n );
        return QgsGeometry::InvalidInput;
      }
    }
    else
    {
      return QgsGeometry::InvalidInput;
    }
  }
  else
  {
    added = geomCollection->addGeometry( part.release() );
  }
  return added ? QgsGeometry::Success : QgsGeometry::InvalidInput;
}

bool QgsGeometryEditUtils::deleteRing( QgsAbstractGeometry *geom, int ringNum, int partNum )
{
  if ( !geom || partNum < 0 )
  {
    return false;
  }

  if ( ringNum < 1 ) //cannot remove exterior ring
  {
    return false;
  }

  QgsAbstractGeometry *g = geom;
  QgsGeometryCollection *c = qgsgeometry_cast<QgsGeometryCollection *>( geom );
  if ( c )
  {
    g = c->geometryN( partNum );
  }
  else if ( partNum > 0 )
  {
    //part num specified, but not a multi part geometry type
    return false;
  }

  QgsCurvePolygon *cpoly = qgsgeometry_cast<QgsCurvePolygon *>( g );
  if ( !cpoly )
  {
    return false;
  }

  return cpoly->removeInteriorRing( ringNum - 1 );
}

bool QgsGeometryEditUtils::deletePart( QgsAbstractGeometry *geom, int partNum )
{
  if ( !geom )
  {
    return false;
  }

  QgsGeometryCollection *c = qgsgeometry_cast<QgsGeometryCollection *>( geom );
  if ( !c )
  {
    return false;
  }

  return c->removeGeometry( partNum );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryEditUtils::avoidIntersections( const QgsAbstractGeometry &geom,
    const QList<QgsVectorLayer *> &avoidIntersectionsLayers,
    const QHash<QgsVectorLayer *, QSet<QgsFeatureId> > &ignoreFeatures )
{
  std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( &geom ) );
  if ( !geomEngine )
  {
    return nullptr;
  }
  QgsWkbTypes::Type geomTypeBeforeModification = geom.wkbType();


  //check if g has polygon type
  if ( QgsWkbTypes::geometryType( geomTypeBeforeModification ) != QgsWkbTypes::PolygonGeometry )
  {
    return nullptr;
  }

  if ( avoidIntersectionsLayers.isEmpty() )
    return nullptr; //no intersections stored in project does not mean error

  QVector< QgsGeometry > nearGeometries;

  //go through list, convert each layer to vector layer and call QgsVectorLayer::removePolygonIntersections for each
  for ( QgsVectorLayer *currentLayer : avoidIntersectionsLayers )
  {
    QgsFeatureIds ignoreIds;
    QHash<QgsVectorLayer *, QSet<qint64> >::const_iterator ignoreIt = ignoreFeatures.constFind( currentLayer );
    if ( ignoreIt != ignoreFeatures.constEnd() )
      ignoreIds = ignoreIt.value();

    QgsFeatureIterator fi = currentLayer->getFeatures( QgsFeatureRequest( geom.boundingBox() )
                            .setFlags( QgsFeatureRequest::ExactIntersect )
                            .setSubsetOfAttributes( QgsAttributeList() ) );
    QgsFeature f;
    while ( fi.nextFeature( f ) )
    {
      if ( ignoreIds.contains( f.id() ) )
        continue;

      if ( !f.hasGeometry() )
        continue;

      nearGeometries << f.geometry();
    }
  }

  if ( nearGeometries.isEmpty() )
  {
    return nullptr;
  }

  std::unique_ptr< QgsAbstractGeometry > combinedGeometries( geomEngine->combine( nearGeometries ) );
  if ( !combinedGeometries )
  {
    return nullptr;
  }

  std::unique_ptr< QgsAbstractGeometry > diffGeom( geomEngine->difference( combinedGeometries.get() ) );

  return diffGeom;
}
