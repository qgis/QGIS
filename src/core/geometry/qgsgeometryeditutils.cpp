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
#include "qgsmaplayerregistry.h"
#include "qgsmultisurface.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include <limits>

int QgsGeometryEditUtils::addRing( QgsAbstractGeometry* geom, QgsCurve* ring )
{
  if ( !ring )
  {
    return 1;
  }

  QList< QgsCurvePolygon* > polygonList;
  QgsCurvePolygon* curvePoly = dynamic_cast< QgsCurvePolygon* >( geom );
  QgsGeometryCollection* multiGeom = dynamic_cast< QgsGeometryCollection* >( geom );
  if ( curvePoly )
  {
    polygonList.append( curvePoly );
  }
  else if ( multiGeom )
  {
    polygonList.reserve( multiGeom->numGeometries() );
    for ( int i = 0; i < multiGeom->numGeometries(); ++i )
    {
      polygonList.append( dynamic_cast< QgsCurvePolygon* >( multiGeom->geometryN( i ) ) );
    }
  }
  else
  {
    delete ring;
    return 1; //not polygon / multipolygon;
  }

  //ring must be closed
  if ( !ring->isClosed() )
  {
    delete ring;
    return 2;
  }
  else if ( !ring->isRing() )
  {
    delete ring;
    return 3;
  }

  QScopedPointer<QgsGeometryEngine> ringGeom( QgsGeometry::createGeometryEngine( ring ) );
  ringGeom->prepareGeometry();

  //for each polygon, test if inside outer ring and no intersection with other interior ring
  QList< QgsCurvePolygon* >::iterator polyIter = polygonList.begin();
  for ( ; polyIter != polygonList.end(); ++polyIter )
  {
    if ( ringGeom->within( **polyIter ) )
    {
      //check if disjoint with other interior rings
      int nInnerRings = ( *polyIter )->numInteriorRings();
      for ( int i = 0; i < nInnerRings; ++i )
      {
        if ( !ringGeom->disjoint( *( *polyIter )->interiorRing( i ) ) )
        {
          delete ring;
          return 4;
        }
      }

      //make sure dimensionality of ring matches geometry
      if ( QgsWkbTypes::hasZ( geom->wkbType() ) )
        ring->addZValue( 0 );
      if ( QgsWkbTypes::hasM( geom->wkbType() ) )
        ring->addMValue( 0 );

      ( *polyIter )->addInteriorRing( ring );
      return 0; //success
    }
  }
  delete ring;
  return 5; //not contained in any outer ring
}

int QgsGeometryEditUtils::addPart( QgsAbstractGeometry* geom, QgsAbstractGeometry* part )
{
  if ( !geom )
  {
    return 1;
  }

  if ( !part )
  {
    return 2;
  }

  //multitype?
  QgsGeometryCollection* geomCollection = dynamic_cast<QgsGeometryCollection*>( geom );
  if ( !geomCollection )
  {
    return 1;
  }

  bool added = false;
  if ( QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiSurface
       || QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::MultiPolygon )
  {
    QgsCurve* curve = dynamic_cast<QgsCurve*>( part );
    if ( curve && curve->isClosed() && curve->numPoints() >= 4 )
    {
      QgsCurvePolygon *poly = nullptr;
      if ( QgsWkbTypes::flatType( curve->wkbType() ) == QgsWkbTypes::LineString )
      {
        poly = new QgsPolygonV2();
      }
      else
      {
        poly = new QgsCurvePolygon();
      }
      poly->setExteriorRing( curve );
      added = geomCollection->addGeometry( poly );
    }
    else if ( QgsWkbTypes::flatType( part->wkbType() ) == QgsWkbTypes::Polygon )
    {
      added = geomCollection->addGeometry( part );
    }
    else if ( QgsWkbTypes::flatType( part->wkbType() ) == QgsWkbTypes::MultiPolygon )
    {
      QgsGeometryCollection *parts = static_cast<QgsGeometryCollection*>( part );

      int i;
      int n = geomCollection->numGeometries();
      for ( i = 0; i < parts->numGeometries() && geomCollection->addGeometry( parts->geometryN( i )->clone() ); i++ )
        ;

      added = i == parts->numGeometries();
      if ( !added )
      {
        while ( geomCollection->numGeometries() > n )
          geomCollection->removeGeometry( n );
        delete part;
        return 2;
      }

      delete part;
    }
    else
    {
      delete part;
      return 2;
    }
  }
  else
  {
    added = geomCollection->addGeometry( part );
  }
  return added ? 0 : 2;
}

bool QgsGeometryEditUtils::deleteRing( QgsAbstractGeometry* geom, int ringNum, int partNum )
{
  if ( !geom || partNum < 0 )
  {
    return false;
  }

  if ( ringNum < 1 ) //cannot remove exterior ring
  {
    return false;
  }

  QgsAbstractGeometry* g = geom;
  QgsGeometryCollection* c = dynamic_cast<QgsGeometryCollection*>( geom );
  if ( c )
  {
    g = c->geometryN( partNum );
  }
  else if ( partNum > 0 )
  {
    //part num specified, but not a multi part geometry type
    return false;
  }

  QgsCurvePolygon* cpoly = dynamic_cast<QgsCurvePolygon*>( g );
  if ( !cpoly )
  {
    return false;
  }

  return cpoly->removeInteriorRing( ringNum - 1 );
}

bool QgsGeometryEditUtils::deletePart( QgsAbstractGeometry* geom, int partNum )
{
  if ( !geom )
  {
    return false;
  }

  QgsGeometryCollection* c = dynamic_cast<QgsGeometryCollection*>( geom );
  if ( !c )
  {
    return false;
  }

  return c->removeGeometry( partNum );
}

QgsAbstractGeometry* QgsGeometryEditUtils::avoidIntersections( const QgsAbstractGeometry& geom, QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures )
{
  QScopedPointer<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( &geom ) );
  if ( geomEngine.isNull() )
  {
    return nullptr;
  }
  QgsWkbTypes::Type geomTypeBeforeModification = geom.wkbType();


  //check if g has polygon type
  if ( QgsWkbTypes::geometryType( geomTypeBeforeModification ) != QgsWkbTypes::PolygonGeometry )
  {
    return nullptr;
  }

  //read avoid intersections list from project properties
  bool listReadOk;
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", QStringList(), &listReadOk );
  if ( !listReadOk )
    return nullptr; //no intersections stored in project does not mean error

  QList< QgsAbstractGeometry* > nearGeometries;

  //go through list, convert each layer to vector layer and call QgsVectorLayer::removePolygonIntersections for each
  QgsVectorLayer* currentLayer = nullptr;
  QStringList::const_iterator aIt = avoidIntersectionsList.constBegin();
  for ( ; aIt != avoidIntersectionsList.constEnd(); ++aIt )
  {
    currentLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( *aIt ) );
    if ( currentLayer )
    {
      QgsFeatureIds ignoreIds;
      QMap<QgsVectorLayer*, QSet<qint64> >::const_iterator ignoreIt = ignoreFeatures.find( currentLayer );
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

        nearGeometries << f.geometry().geometry()->clone();
      }
    }
  }

  if ( nearGeometries.isEmpty() )
  {
    return nullptr;
  }


  QgsAbstractGeometry* combinedGeometries = geomEngine.data()->combine( nearGeometries );
  qDeleteAll( nearGeometries );
  if ( !combinedGeometries )
  {
    return nullptr;
  }

  QgsAbstractGeometry* diffGeom = geomEngine.data()->difference( *combinedGeometries );

  delete combinedGeometries;
  return diffGeom;
}
