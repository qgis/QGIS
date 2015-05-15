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
#include "qgscurvev2.h"
#include "qgscurvepolygonv2.h"
#include "qgspolygonv2.h"
#include "qgsgeometryutils.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsmaplayerregistry.h"
#include "qgsmultisurfacev2.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include <limits>

int QgsGeometryEditUtils::addRing( QgsAbstractGeometryV2* geom, QgsCurveV2* ring )
{
  if ( !ring )
  {
    return 1;
  }

  QList< QgsCurvePolygonV2* > polygonList;
  QgsCurvePolygonV2* curvePoly = dynamic_cast< QgsCurvePolygonV2* >( geom );
  QgsGeometryCollectionV2* multiGeom = dynamic_cast< QgsGeometryCollectionV2* >( geom );
  if ( curvePoly )
  {
    polygonList.append( curvePoly );
  }
  else if ( multiGeom )
  {
    for ( int i = 0; i < multiGeom->numGeometries(); ++i )
    {
      polygonList.append( dynamic_cast< QgsCurvePolygonV2* >( multiGeom->geometryN( i ) ) );
    }
  }
  else
  {
    delete ring; return 1; //not polygon / multipolygon;
  }

  //ring must be closed
  if ( !ring->isClosed() )
  {
    delete ring; return 2;
  }
  else if ( !ring->isRing() )
  {
    delete ring; return 3;
  }

  QScopedPointer<QgsGeometryEngine> ringGeom( createGeometryEngine( ring ) );
  ringGeom->prepareGeometry();

  //for each polygon, test if inside outer ring and no intersection with other interior ring
  QList< QgsCurvePolygonV2* >::iterator polyIter = polygonList.begin();
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
          delete ring; return 4;
        }
      }
      ( *polyIter )->addInteriorRing( ring );
      return 0; //success
    }
  }
  delete ring; return 5; //not contained in any outer ring
}

int QgsGeometryEditUtils::addPart( QgsAbstractGeometryV2* geom, QgsAbstractGeometryV2* part )
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
  QgsGeometryCollectionV2* geomCollection = dynamic_cast<QgsGeometryCollectionV2*>( geom );
  if ( !geomCollection )
  {
    return 1;
  }

  bool added = false;
  if ( geom->geometryType() == "MultiSurface" || geom->geometryType() == "MultiPolygon" )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( part );
    if ( !curve || !curve->isClosed() || curve->numPoints() < 4 )
    {
      delete part; return 2;
    }

    QgsCurvePolygonV2* poly = 0;
    if ( curve->geometryType() == "LineString" )
    {
      poly = new QgsPolygonV2();
    }
    else
    {
      poly = new QgsCurvePolygonV2();
    }
    poly->setExteriorRing( curve );
    added = geomCollection->addGeometry( poly );
  }
  else
  {
    added = geomCollection->addGeometry( part );
  }
  return added ? 0 : 2;
}

bool QgsGeometryEditUtils::deleteRing( QgsAbstractGeometryV2* geom, int ringNum, int partNum )
{
  if ( !geom )
  {
    return false;
  }

  if ( ringNum < 1 ) //cannot remove exterior ring
  {
    return false;
  }

  QgsAbstractGeometryV2* g = geom;
  if ( partNum > 0 )
  {
    QgsMultiSurfaceV2* multiSurface = dynamic_cast<QgsMultiSurfaceV2*>( geom );
    if ( !multiSurface )
    {
      return false;
    }
    g = multiSurface->geometryN( partNum );
  }

  QgsCurvePolygonV2* cpoly = dynamic_cast<QgsCurvePolygonV2*>( g );
  if ( !cpoly )
  {
    return false;
  }

  return cpoly->removeInteriorRing( ringNum - 1 );
}

bool QgsGeometryEditUtils::deletePart( QgsAbstractGeometryV2* geom, int partNum )
{
  if ( !geom )
  {
    return false;
  }

  QgsGeometryCollectionV2* c = dynamic_cast<QgsGeometryCollectionV2*>( geom );
  if ( !c )
  {
    return false;
  }

  return c->removeGeometry( partNum );
}

QgsAbstractGeometryV2* QgsGeometryEditUtils::avoidIntersections( const QgsAbstractGeometryV2& geom, QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures )
{
  QScopedPointer<QgsGeometryEngine> geomEngine( createGeometryEngine( &geom ) );
  if ( geomEngine.isNull() )
  {
    return 0;
  }
  QgsWKBTypes::Type geomTypeBeforeModification = geom.wkbType();


  //check if g has polygon type
  if ( QgsWKBTypes::geometryType( geomTypeBeforeModification ) != QgsWKBTypes::PolygonGeometry )
  {
    return 0;
  }

  //read avoid intersections list from project properties
  bool listReadOk;
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", QStringList(), &listReadOk );
  if ( !listReadOk )
    return 0; //no intersections stored in project does not mean error

  QList< const QgsAbstractGeometryV2* > nearGeometries;

  //go through list, convert each layer to vector layer and call QgsVectorLayer::removePolygonIntersections for each
  QgsVectorLayer* currentLayer = 0;
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

        if ( !f.geometry() )
          continue;

        nearGeometries << f.geometry()->geometry()->clone();
      }
    }
  }

  if ( nearGeometries.isEmpty() )
  {
    return 0;
  }


  QgsAbstractGeometryV2* combinedGeometries = geomEngine.data()->combine( nearGeometries );
  qDeleteAll( nearGeometries );
  if ( !combinedGeometries )
  {
    return 0;
  }

  QgsAbstractGeometryV2* diffGeom = geomEngine.data()->difference( *combinedGeometries );

  delete combinedGeometries;
  return diffGeom;
}

QgsGeometryEngine* QgsGeometryEditUtils::createGeometryEngine( const QgsAbstractGeometryV2* geometry )
{
  return new QgsGeos( geometry );
}
