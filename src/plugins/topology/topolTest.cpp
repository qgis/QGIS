/***************************************************************************
  topolTest.cpp
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "topolTest.h"

#include <qgsvectorlayer.h>
#include <qgsmaplayer.h>
#include <qgsmapcanvas.h>
#include <qgsgeometry.h>
#include <qgsfeature.h>
#include <qgsspatialindex.h>
#include <qgisinterface.h>
#include <qgslogger.h>
#include <qgsmessagelog.h>
#include <cmath>
#include <set>
#include <map>

topolTest::topolTest( QgisInterface* qgsIface )
{
  theQgsInterface = qgsIface;
  mTestCancelled = false;

  // one layer tests
  mTopologyRuleMap[QObject::tr( "must not have invalid geometries" )].f = &topolTest::checkValid;
  mTopologyRuleMap[QObject::tr( "must not have invalid geometries" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not have invalid geometries" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not have invalid geometries" )].useSpatialIndex = false;
  mTopologyRuleMap[QObject::tr( "must not have invalid geometries" )].layer1SupportedTypes << QGis::Point << QGis::Polygon << QGis::Line;

  //mTopologyRuleMap[QObject::tr("segments must have minimum length")].f = &topolTest::checkSegmentLength;
  //mTopologyRuleMap[QObject::tr("segments must have minimum length")].useTolerance = true;
  //mTopologyRuleMap[QObject::tr("segments must have minimum length")].useSecondLayer = false;
  //mTopologyRuleMap[QObject::tr("segments must have minimum length")].useSpatialIndex = false;

  mTopologyRuleMap[QObject::tr( "must not have dangles" )].f = &topolTest::checkDanglingLines;
  mTopologyRuleMap[QObject::tr( "must not have dangles" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not have dangles" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not have dangles" )].useSpatialIndex = false;
  mTopologyRuleMap[QObject::tr( "must not have dangles" )].layer1SupportedTypes << QGis::Line;

  mTopologyRuleMap[QObject::tr( "must not have duplicates" )].f = &topolTest::checkDuplicates;
  mTopologyRuleMap[QObject::tr( "must not have duplicates" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not have duplicates" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not have duplicates" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must not have duplicates" )].layer1SupportedTypes << QGis::Point << QGis::Polygon << QGis::Line;

  mTopologyRuleMap[QObject::tr( "must not have pseudos" )].f = &topolTest::checkPseudos;
  mTopologyRuleMap[QObject::tr( "must not have pseudos" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not have pseudos" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not have pseudos" )].useSpatialIndex = false;
  mTopologyRuleMap[QObject::tr( "must not have pseudos" )].layer1SupportedTypes << QGis::Line;

  mTopologyRuleMap[QObject::tr( "must not overlap" )].f = &topolTest::checkOverlaps;
  mTopologyRuleMap[QObject::tr( "must not overlap" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not overlap" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not overlap" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must not overlap" )].layer1SupportedTypes << QGis::Polygon;

  mTopologyRuleMap[QObject::tr( "must not have gaps" )].f = &topolTest::checkGaps;
  mTopologyRuleMap[QObject::tr( "must not have gaps" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not have gaps" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not have gaps" )].useSpatialIndex = false;
  mTopologyRuleMap[QObject::tr( "must not have gaps" )].layer1SupportedTypes << QGis::Polygon;

  mTopologyRuleMap[QObject::tr( "must not have multi-part geometries" )].f = &topolTest::checkMultipart;
  mTopologyRuleMap[QObject::tr( "must not have multi-part geometries" )].useSecondLayer = false;
  mTopologyRuleMap[QObject::tr( "must not have multi-part geometries" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not have multi-part geometries" )].useSpatialIndex = false;
  mTopologyRuleMap[QObject::tr( "must not have multi-part geometries" )].layer1SupportedTypes << QGis::Point << QGis::Polygon << QGis::Line;

  // two layer tests
  mTopologyRuleMap[QObject::tr( "must not overlap with" )].f = &topolTest::checkOverlapWithLayer;
  mTopologyRuleMap[QObject::tr( "must not overlap with" )].useSecondLayer = true;
  mTopologyRuleMap[QObject::tr( "must not overlap with" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must not overlap with" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must not overlap with" )].layer1SupportedTypes << QGis::Polygon;
  mTopologyRuleMap[QObject::tr( "must not overlap with" )].layer2SupportedTypes << QGis::Polygon;

  mTopologyRuleMap[QObject::tr( "must be covered by" )].f = &topolTest::checkPointCoveredBySegment;
  mTopologyRuleMap[QObject::tr( "must be covered by" )].useSecondLayer = true;
  mTopologyRuleMap[QObject::tr( "must be covered by" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must be covered by" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must be covered by" )].layer1SupportedTypes << QGis::Point;
  mTopologyRuleMap[QObject::tr( "must be covered by" )].layer2SupportedTypes << QGis::Line << QGis::Polygon;

  //mTopologyRuleMap[QObject::tr("features must not be closer than tolerance")].f = &topolTest::checkCloseFeature;
  //mTopologyRuleMap[QObject::tr("features must not be closer than tolerance")].useSecondLayer = true;
  //mTopologyRuleMap[QObject::tr("features must not be closer than tolerance")].useTolerance = false;
  //mTopologyRuleMap[QObject::tr("features must not be closer than tolerance")].useSpatialIndex = false;

  mTopologyRuleMap[QObject::tr( "must be covered by endpoints of" )].f = &topolTest::checkPointCoveredByLineEnds;
  mTopologyRuleMap[QObject::tr( "must be covered by endpoints of" )].useSecondLayer = true;
  mTopologyRuleMap[QObject::tr( "must be covered by endpoints of" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must be covered by endpoints of" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must be covered by endpoints of" )].layer1SupportedTypes << QGis::Point;
  mTopologyRuleMap[QObject::tr( "must be covered by endpoints of" )].layer2SupportedTypes << QGis::Line;

  mTopologyRuleMap[QObject::tr( "end points must be covered by" )].f = &topolTest::checkyLineEndsCoveredByPoints;
  mTopologyRuleMap[QObject::tr( "end points must be covered by" )].useSecondLayer = true;
  mTopologyRuleMap[QObject::tr( "end points must be covered by" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "end points must be covered by" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "end points must be covered by" )].layer1SupportedTypes << QGis::Line;
  mTopologyRuleMap[QObject::tr( "end points must be covered by" )].layer2SupportedTypes << QGis::Point;

  mTopologyRuleMap[QObject::tr( "must be inside" )].f = &topolTest::checkPointInPolygon;
  mTopologyRuleMap[QObject::tr( "must be inside" )].useSecondLayer = true;
  mTopologyRuleMap[QObject::tr( "must be inside" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must be inside" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must be inside" )].layer1SupportedTypes << QGis::Point;
  mTopologyRuleMap[QObject::tr( "must be inside" )].layer2SupportedTypes << QGis::Polygon;

  mTopologyRuleMap[QObject::tr( "must contain" )].f = &topolTest::checkPolygonContainsPoint;
  mTopologyRuleMap[QObject::tr( "must contain" )].useSecondLayer = true;
  mTopologyRuleMap[QObject::tr( "must contain" )].useTolerance = false;
  mTopologyRuleMap[QObject::tr( "must contain" )].useSpatialIndex = true;
  mTopologyRuleMap[QObject::tr( "must contain" )].layer1SupportedTypes << QGis::Polygon;
  mTopologyRuleMap[QObject::tr( "must contain" )].layer2SupportedTypes << QGis::Point;
}

topolTest::~topolTest()
{
  QMap<QString, QgsSpatialIndex*>::Iterator lit = mLayerIndexes.begin();
  for ( ; lit != mLayerIndexes.end(); ++lit )
    delete *lit;
}

void topolTest::setTestCancelled()
{
  mTestCancelled = true;
}

bool topolTest::testCancelled()
{
  if ( mTestCancelled )
  {
    mTestCancelled = false;
    return true;
  }

  return false;
}

ErrorList topolTest::checkCloseFeature( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent )
{
  Q_UNUSED( isExtent );
  ErrorList errorList;
  QgsSpatialIndex* index = 0;

  bool badG1 = false, badG2 = false;
  bool skipItself = layer1 == layer2;

  int i = 0;
  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for ( it = mFeatureList1.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    // increase bounding box by tolerance
    QgsRectangle frame( bb.xMinimum() - tolerance, bb.yMinimum() - tolerance, bb.xMaximum() + tolerance, bb.yMaximum() + tolerance );

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( frame );

    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      // skip itself, when invoked with the same layer
      if ( skipItself && f.id() == it->feature.id() )
        continue;

      if ( !g2 || !g2->asGeos() )
      {
        badG2 = true;
        continue;
      }

      if ( !g1 || !g1->asGeos() )
      {
        badG1 = true;
        continue;

      }

      if ( g1->distance( *g2 ) < tolerance )
      {
        QgsRectangle r = g2->boundingBox();
        r.combineExtentWith( &bb );

        QList<FeatureLayer> fls;
        FeatureLayer fl;
        fl.feature = f;
        fl.layer = layer2;
        fls << *it << fl;
        QgsGeometry* conflict = new QgsGeometry( *g2 );
        TopolErrorClose* err = new TopolErrorClose( r, conflict, fls );
        //TopolErrorClose* err = new TopolErrorClose(r, g2, fls);

        errorList << err;
      }
    }
  }

  if ( badG2 )
    QgsMessageLog::logMessage( tr( "Invalid second geometry." ), tr( "Topology plugin" ) );

  if ( badG1 )
    QgsMessageLog::logMessage( tr( "Invalid first geometry." ), tr( "Topology plugin" ) );

  return errorList;
}

ErrorList topolTest::checkDanglingLines( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer2 );

  int i = 0;
  ErrorList errorList;
  QgsFeature f;

  if ( layer1->geometryType() != QGis::Line )
  {
    return errorList;
  }

  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();

  qDebug() << mFeatureList1.count();

  QgsPoint startPoint;
  QgsPoint endPoint;

  std::multimap<QgsPoint, QgsFeatureId, PointComparer> endVerticesMap;

  for ( it = mFeatureList1.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();

    if ( !g1 )
    {
      QgsMessageLog::logMessage( tr( "First geometry invalid in line test." ), tr( "Topology plugin" ) );
      continue;
    }

    if ( !g1->asGeos() )
    {
      QgsMessageLog::logMessage( tr( "Failed to import first geometry into GEOS in line test." ), tr( "Topology plugin" ) );
      continue;
    }

    if ( g1->isMultipart() )
    {
      QgsMultiPolyline lines = g1->asMultiPolyline();
      for ( int m = 0; m < lines.count(); m++ )
      {
        QgsPolyline line = lines[m];
        startPoint = line[0];
        endPoint = line[line.size() - 1];

        endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( startPoint, it->feature.id() ) );
        endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( endPoint, it->feature.id() ) );

      }
    }
    else
    {
      QgsPolyline polyline = g1->asPolyline();
      startPoint = polyline[0];
      endPoint = polyline[polyline.size()-1];
      endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( startPoint, it->feature.id() ) );
      endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( endPoint, it->feature.id() ) );
    }
  }

  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );


  for ( std::multimap<QgsPoint, QgsFeatureId, PointComparer>::iterator pointIt = endVerticesMap.begin(), end = endVerticesMap.end(); pointIt != end; pointIt = endVerticesMap.upper_bound( pointIt->first ) )
  {
    QgsPoint p = pointIt->first;
    QgsFeatureId k = pointIt->second;

    int repetitions = endVerticesMap.count( p );

    //QgsGeometry* extentPoly =
    if ( repetitions == 1 )
    {

      QgsGeometry* conflictGeom = QgsGeometry::fromPoint( p );
      if ( isExtent )
      {
        if ( canvasExtentPoly->disjoint( conflictGeom ) )
        {
          continue;
        }
      }

      QgsRectangle bBox = conflictGeom->boundingBox();
      QgsFeature feat;

      FeatureLayer ftrLayer1;
      //need to fetch attributes?? being safe side by fetching..
      layer1->getFeatures( QgsFeatureRequest().setFilterFid( k ) ).nextFeature( feat );
      ftrLayer1.feature = feat;
      ftrLayer1.layer = layer1;

      QList<FeatureLayer> errorFtrLayers;
      errorFtrLayers << ftrLayer1 << ftrLayer1;

      TopolErrorDangle* err = new TopolErrorDangle( bBox, conflictGeom, errorFtrLayers );
      errorList << err;

    }
  }

  return errorList;
}

ErrorList topolTest::checkDuplicates( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer2 );
  //TODO: multilines - check all separate pieces
  int i = 0;
  ErrorList errorList;

  QList<int>* duplicateIds = new QList<int>();

  QgsSpatialIndex* index = mLayerIndexes[layer1->id()];

  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );


  QMap<int, FeatureLayer>::Iterator it;
  QMap<int, FeatureLayer>::ConstIterator FeatureListEnd = mFeatureMap2.end();
  for ( it = mFeatureMap2.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    int currentId = it->feature.id();

    if ( duplicateIds->contains( currentId ) )
    {
      //is already a duplicate geometry..skip..
      continue;
    }

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );

    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    bool duplicate = false;

    for ( ; cit != crossingIdsEnd; ++cit )
    {
      duplicate = false;
      // skip itself
      if ( mFeatureMap2[*cit].feature.id() == it->feature.id() )
        continue;

      QgsGeometry* g2 = mFeatureMap2[*cit].feature.geometry();
      if ( !g2 )
      {
        QgsMessageLog::logMessage( tr( "Invalid second geometry in dangling line test." ), tr( "Topology plugin" ) );
        continue;
      }

      if ( !g2->asGeos() )
      {
        QgsMessageLog::logMessage( tr( "Failed to import second geometry into GEOS in dangling line test." ), tr( "Topology plugin" ) );
        continue;
      }

      if ( g1->equals( g2 ) )
      {
        duplicate = true;
        duplicateIds->append( mFeatureMap2[*cit].feature.id() );
      }

      if ( duplicate )
      {


        QList<FeatureLayer> fls;
        fls << *it << *it;
        QgsGeometry* conflict = new QgsGeometry( *g1 );

        if ( isExtent )
        {
          if ( canvasExtentPoly->disjoint( conflict ) )
          {
            continue;
          }
          if ( canvasExtentPoly->crosses( conflict ) )
          {
            conflict = conflict->intersection( canvasExtentPoly );
          }
        }

        TopolErrorDuplicates* err = new TopolErrorDuplicates( bb, conflict, fls );

        errorList << err;
      }

    }

  }

  return errorList;

}

ErrorList topolTest::checkOverlaps( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer2 );
  int i = 0;
  ErrorList errorList;

  // could be enabled for lines and points too
  // so duplicate rule may be removed?

  if ( layer1->geometryType() != QGis::Polygon )
  {
    return errorList;
  }

  QList<int>* duplicateIds = new QList<int>();

  QgsSpatialIndex* index;
  index = mLayerIndexes[layer1->id()];

  if ( !index )
  {
    qDebug() << "no index present";
    return errorList;
  }

  QMap<int, FeatureLayer>::Iterator it;
  QMap<int, FeatureLayer>::ConstIterator FeatureListEnd = mFeatureMap2.end();
  for ( it = mFeatureMap2.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    int currentId = it->feature.id();

    if ( duplicateIds->contains( currentId ) )
    {
      //is already a duplicate geometry..skip..
      continue;
    }

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();

    if ( g1->isGeosValid() == false )
    {
      qDebug() << "invalid geometry(g1) found..skipping.." << it->feature.id();
      continue;
    }

    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );

    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    bool duplicate = false;

    QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );

    for ( ; cit != crossingIdsEnd; ++cit )
    {
      duplicate = false;
      // skip itself
      if ( mFeatureMap2[*cit].feature.id() == it->feature.id() )
        continue;

      QgsGeometry* g2 = mFeatureMap2[*cit].feature.geometry();
      if ( !g2 )
      {
        QgsMessageLog::logMessage( tr( "Invalid second geometry in dangling line test." ), tr( "Topology plugin" ) );
        continue;
      }

      if ( !g2->asGeos() )
      {
        QgsMessageLog::logMessage( tr( "Failed to import second geometry into GEOS in dangling line test." ), tr( "Topology plugin" ) );
        continue;
      }

      if ( g2->isGeosValid() == false )
      {
        QgsMessageLog::logMessage( tr( "Skipping invalid second geometry of feature %1 in dangling line test." ).arg( it->feature.id() ), tr( "Topology plugin" ) );
        continue;
      }


      qDebug() << "checking overlap for" << it->feature.id();
      if ( g1->overlaps( g2 ) )
      {
        duplicate = true;
        duplicateIds->append( mFeatureMap2[*cit].feature.id() );
      }



      if ( duplicate )
      {
        QList<FeatureLayer> fls;
        fls << *it << *it;
        QgsGeometry* conflictGeom = g1->intersection( g2 );

        if ( isExtent )
        {
          if ( canvasExtentPoly->disjoint( conflictGeom ) )
          {
            continue;
          }
          if ( canvasExtentPoly->crosses( conflictGeom ) )
          {
            conflictGeom = conflictGeom->intersection( canvasExtentPoly );
          }
        }

        TopolErrorOverlaps* err = new TopolErrorOverlaps( bb, conflictGeom, fls );

        errorList << err;
      }

    }

  }

  return errorList;
}

ErrorList topolTest::checkGaps( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer2 );

  int i = 0;
  ErrorList errorList;

  // could be enabled for lines and points too
  // so duplicate rule may be removed?

  if ( layer1->geometryType() != QGis::Polygon )
  {
    return errorList;
  }

  QList<FeatureLayer>::Iterator it;
  QgsGeometry* g1;

  QList<GEOSGeometry*> geomList;

  qDebug() << mFeatureList1.count() << " features in list!";
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for ( it = mFeatureList1.begin(); it != FeatureListEnd; ++it )
  {
    qDebug() << "reading features-" << i;

    if ( !( ++i % 100 ) )
    {
      emit progress( i );
    }

    if ( testCancelled() )
    {
      break;
    }

    g1 = it->feature.geometry();

    if ( !g1 )
    {
      continue;
    }

    if ( !g1->asGeos() )
    {
      continue;
    }

    if ( g1->isGeosValid() == false )
    {
      qDebug() << "invalid geometry found..skipping.." << it->feature.id();
      continue;
    }

    if ( g1->isMultipart() )
    {
      QgsMultiPolygon polys = g1->asMultiPolygon();
      for ( int m = 0; m < polys.count(); m++ )
      {
        QgsPolygon polygon = polys[m];

        QgsGeometry* polyGeom = QgsGeometry::fromPolygon( polygon );

        geomList.push_back( GEOSGeom_clone( polyGeom->asGeos() ) );
      }

    }
    else
    {
      geomList.push_back( GEOSGeom_clone( g1->asGeos() ) );
    }
  }

  GEOSGeometry** geomArray = new GEOSGeometry*[geomList.size()];
  for ( int i = 0; i < geomList.size(); ++i )
  {
    //qDebug() << "filling geometry array-" << i;
    geomArray[i] = geomList.at( i );
  }

  qDebug() << "creating geometry collection-";

  if ( geomList.size() == 0 )
  {
    //qDebug() << "geometry list is empty!";
    return errorList;
  }

  GEOSGeometry* collection = 0;
  collection = GEOSGeom_createCollection( GEOS_MULTIPOLYGON, geomArray, geomList.size() );


  qDebug() << "performing cascaded union..might take time..-";
  GEOSGeometry* unionGeom = GEOSUnionCascaded( collection );
  //delete[] geomArray;

  QgsGeometry test;
  test.fromGeos( unionGeom );


  //qDebug() << "wktmerged - " << test.exportToWkt();

  QString extentWkt =  test.boundingBox().asWktPolygon();
  QgsGeometry* extentGeom = QgsGeometry::fromWkt( extentWkt );
  QgsGeometry* bufferExtent = extentGeom->buffer( 2, 3 );

  //qDebug() << "extent wkt - " << bufferExtent->exportToWkt();

  QgsGeometry* diffGeoms = bufferExtent->difference( &test );

  //qDebug() << "difference gometry - " << diffGeoms->exportToWkt() ;

  QList<QgsGeometry*> geomColl = diffGeoms->asGeometryCollection();

  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );

  for ( int i = 1; i < geomColl.count() ; ++i )
  {
    QgsGeometry* conflictGeom = geomColl[i];
    if ( isExtent )
    {
      if ( canvasExtentPoly->disjoint( conflictGeom ) )
      {
        continue;
      }
      if ( canvasExtentPoly->crosses( conflictGeom ) )
      {
        conflictGeom = conflictGeom->intersection( canvasExtentPoly );
      }
    }
    QgsRectangle bBox = conflictGeom->boundingBox();
    QgsFeature feat;
    FeatureLayer ftrLayer1;
    ftrLayer1.layer = layer1;
    QList<FeatureLayer> errorFtrLayers;
    errorFtrLayers << ftrLayer1 << ftrLayer1;
    TopolErrorGaps* err = new TopolErrorGaps( bBox, conflictGeom, errorFtrLayers );
    errorList << err;
  }

  return errorList;
}

ErrorList topolTest::checkPseudos( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer2 );

  int i = 0;
  ErrorList errorList;
  QgsFeature f;

  if ( layer1->geometryType() != QGis::Line )
  {
    return errorList;
  }

  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();

  qDebug() << mFeatureList1.count();

  QgsPoint startPoint;
  QgsPoint endPoint;

  std::multimap<QgsPoint, QgsFeatureId, PointComparer> endVerticesMap;

  for ( it = mFeatureList1.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();

    if ( !g1 )
    {
      QgsMessageLog::logMessage( tr( "Skipping invalid first geometry in pseudo line test." ), tr( "Topology plugin" ) );
      continue;
    }

    if ( !g1->asGeos() )
    {
      QgsMessageLog::logMessage( tr( "Failed to import first geometry into GEOS in pseudo line test." ), tr( "Topology plugin" ) );
      continue;
    }

    if ( g1->isMultipart() )
    {
      QgsMultiPolyline lines = g1->asMultiPolyline();
      for ( int m = 0; m < lines.count(); m++ )
      {
        QgsPolyline line = lines[m];
        startPoint = line[0];
        endPoint = line[line.size() - 1];

        endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( startPoint, it->feature.id() ) );
        endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( endPoint, it->feature.id() ) );

      }
    }
    else
    {
      QgsPolyline polyline = g1->asPolyline();
      startPoint = polyline[0];
      endPoint = polyline[polyline.size()-1];
      endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( startPoint, it->feature.id() ) );
      endVerticesMap.insert( std::pair<QgsPoint, QgsFeatureId>( endPoint, it->feature.id() ) );
    }
  }


  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );


  for ( std::multimap<QgsPoint, QgsFeatureId, PointComparer>::iterator pointIt = endVerticesMap.begin(), end = endVerticesMap.end(); pointIt != end; pointIt = endVerticesMap.upper_bound( pointIt->first ) )
  {
    QgsPoint p = pointIt->first;
    QgsFeatureId k = pointIt->second;

    int repetitions = endVerticesMap.count( p );

    if ( repetitions == 2 )
    {
      QgsGeometry* conflictGeom = QgsGeometry::fromPoint( p );

      if ( isExtent )
      {
        if ( canvasExtentPoly->disjoint( conflictGeom ) )
        {
          continue;
        }
      }

      QgsRectangle bBox = conflictGeom->boundingBox();
      QgsFeature feat;

      FeatureLayer ftrLayer1;
      //need to fetch attributes?? being safe side by fetching..
      layer1->getFeatures( QgsFeatureRequest().setFilterFid( k ) ).nextFeature( feat );
      ftrLayer1.feature = feat;
      ftrLayer1.layer = layer1;

      QList<FeatureLayer> errorFtrLayers;
      errorFtrLayers << ftrLayer1 << ftrLayer1;

      TopolErrorPseudos* err = new TopolErrorPseudos( bBox, conflictGeom, errorFtrLayers );
      errorList << err;

    }
  }

  return errorList;
}

ErrorList topolTest::checkValid( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer1 );
  Q_UNUSED( layer2 );
  Q_UNUSED( isExtent );

  int i = 0;
  ErrorList errorList;
  QgsFeature f;

  QList<FeatureLayer>::Iterator it;

  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( ++i );
    if ( testCancelled() )
      break;

    QgsGeometry* g = it->feature.geometry();
    if ( !g )
    {
      QgsMessageLog::logMessage( tr( "Invalid geometry in validity test." ), tr( "Topology plugin" ) );
      continue;
    }

    if ( !g->asGeos() )
      continue;

    if ( !GEOSisValid( g->asGeos() ) )
    {
      QgsRectangle r = g->boundingBox();
      QList<FeatureLayer> fls;
      fls << *it << *it;

      QgsGeometry* conflict = new QgsGeometry( *g );
      TopolErrorValid* err = new TopolErrorValid( r, conflict, fls );
      errorList << err;
    }
  }

  return errorList;
}


ErrorList topolTest::checkPointCoveredBySegment( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent )
{
  Q_UNUSED( tolerance );

  int i = 0;

  ErrorList errorList;

  if ( layer1->geometryType() != QGis::Point )
  {
    return errorList;
  }
  if ( layer2->geometryType() == QGis::Point )
  {
    return errorList;
  }

  QgsSpatialIndex* index = mLayerIndexes[layer2->id()];
  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );


  QList<FeatureLayer>::Iterator it;
  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );

    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    bool touched = false;

    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      if ( !g2 )
      {
        QgsMessageLog::logMessage( tr( "Invalid geometry in covering test." ), tr( "Topology plugin" ) );
        continue;
      }

      // test if point touches other geometry
      if ( g1->touches( g2 ) )
      {
        touched = true;
        break;
      }
    }

    if ( !touched )
    {
      QgsGeometry* conflictGeom = new QgsGeometry( *g1 );

      if ( isExtent )
      {
        if ( canvasExtentPoly->disjoint( conflictGeom ) )
        {
          continue;
        }
      }

      QList<FeatureLayer> fls;
      fls << *it << *it;
      //bb.scale(10);

      TopolErrorCovered* err = new TopolErrorCovered( bb, conflictGeom, fls );

      errorList << err;
    }
  }

  return errorList;
}

ErrorList topolTest::checkSegmentLength( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent )
{
  Q_UNUSED( layer1 );
  Q_UNUSED( layer2 );
  Q_UNUSED( isExtent );

  int i = 0;
  ErrorList errorList;
  QgsFeature f;


  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();

  QgsPolygon pol;

  QgsMultiPolygon mpol;
  QgsPolyline segm;
  QgsPolyline ls;
  QgsMultiPolyline mls;
  QList<FeatureLayer> fls;
  TopolErrorShort* err;
  double distance;

  for ( it = mFeatureList1.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
    {
      emit progress( i );
    }

    if ( testCancelled() )
    {
      break;
    }

    QgsGeometry* g1 = it->feature.geometry();


    // switching by type here, because layer can contain both single and multi version geometries
    switch ( g1->wkbType() )
    {
      case QGis::WKBLineString:
      case QGis::WKBLineString25D:
        ls = g1->asPolyline();


        for ( int i = 1; i < ls.size(); ++i )
        {
          distance = sqrt( ls[i-1].sqrDist( ls[i] ) );
          if ( distance < tolerance )
          {
            fls.clear();
            fls << *it << *it;
            segm.clear();
            segm << ls[i-1] << ls[i];
            QgsGeometry* conflict = QgsGeometry::fromPolyline( segm );
            err = new TopolErrorShort( g1->boundingBox(), conflict, fls );
            //err = new TopolErrorShort(g1->boundingBox(), QgsGeometry::fromPolyline(segm), fls);
            errorList << err;
            //break on getting the first error
            break;
          }
        }
        break;

      case QGis::WKBPolygon:
      case QGis::WKBPolygon25D:
        pol = g1->asPolygon();

        for ( int i = 0; i < pol.size(); ++i )
        {
          for ( int j = 1; j < pol[i].size(); ++j )
          {
            distance =  sqrt( pol[i][j-1].sqrDist( pol[i][j] ) );
            if ( distance < tolerance )
            {
              fls.clear();
              fls << *it << *it;
              segm.clear();
              segm << pol[i][j-1] << pol[i][j];
              QgsGeometry* conflict = QgsGeometry::fromPolyline( segm );
              err = new TopolErrorShort( g1->boundingBox(), conflict, fls );
              errorList << err;
              //break on getting the first error
              break;
            }
          }
        }

        break;

      case QGis::WKBMultiLineString:
      case QGis::WKBMultiLineString25D:
        mls = g1->asMultiPolyline();

        for ( int k = 0; k < mls.size(); ++k )
        {
          QgsPolyline& ls = mls[k];
          for ( int i = 1; i < ls.size(); ++i )
          {
            distance = sqrt( ls[i-1].sqrDist( ls[i] ) );
            if ( distance < tolerance )
            {
              fls.clear();
              fls << *it << *it;
              segm.clear();
              segm << ls[i-1] << ls[i];
              QgsGeometry* conflict = QgsGeometry::fromPolyline( segm );
              err = new TopolErrorShort( g1->boundingBox(), conflict, fls );
              errorList << err;
              //break on getting the first error
              break;
            }
          }
        }
        break;

      case QGis::WKBMultiPolygon:
      case QGis::WKBMultiPolygon25D:
        mpol = g1->asMultiPolygon();

        for ( int k = 0; k < mpol.size(); ++k )
        {
          QgsPolygon& pol = mpol[k];
          for ( int i = 0; i < pol.size(); ++i )
          {
            for ( int j = 1; j < pol[i].size(); ++j )
            {
              distance = pol[i][j-1].sqrDist( pol[i][j] );
              if ( distance < tolerance )
              {
                fls.clear();
                fls << *it << *it;
                segm.clear();
                segm << pol[i][j-1] << pol[i][j];
                QgsGeometry* conflict = QgsGeometry::fromPolyline( segm );
                err = new TopolErrorShort( g1->boundingBox(), conflict, fls );
                errorList << err;
                //break on getting the first error
                break;
              }
            }
          }
        }
        break;

      default:
        continue;
    }
  }

  return errorList;
}

ErrorList topolTest::checkOverlapWithLayer( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent )
{
  Q_UNUSED( tolerance );

  int i = 0;
  ErrorList errorList;

  bool skipItself = layer1 == layer2;
  QgsSpatialIndex* index = mLayerIndexes[layer2->id()];

  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );


  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for ( it = mFeatureList1.begin(); it != FeatureListEnd; ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    if ( testCancelled() )
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );

    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();
    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      // skip itself, when invoked with the same layer
      if ( skipItself && f.id() == it->feature.id() )
        continue;

      if ( !g2 )
      {
        QgsMessageLog::logMessage( tr( "Second geometry missing." ), tr( "Topology plugin" ) );
        continue;
      }

      if ( g1->overlaps( g2 ) )
      {
        QgsRectangle r = bb;
        QgsRectangle r2 = g2->boundingBox();
        r.combineExtentWith( &r2 );

        QgsGeometry* conflictGeom = g1->intersection( g2 );
        // could this for some reason return NULL?
        if ( !conflictGeom )
        {
          continue;
        }

        if ( isExtent )
        {
          if ( canvasExtentPoly->disjoint( conflictGeom ) )
          {
            continue;
          }
          if ( canvasExtentPoly->crosses( conflictGeom ) )
          {
            conflictGeom = conflictGeom->intersection( canvasExtentPoly );
          }
        }

        //c = new QgsGeometry;

        QList<FeatureLayer> fls;
        FeatureLayer fl;
        fl.feature = f;
        fl.layer = layer2;
        fls << *it << fl;
        TopolErrorIntersection* err = new TopolErrorIntersection( r, conflictGeom, fls );

        errorList << err;
      }
    }
  }
  return errorList;
}



ErrorList topolTest::checkPointCoveredByLineEnds( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );

  int i = 0;
  ErrorList errorList;


  if ( layer1->geometryType() != QGis::Point )
  {
    return errorList;
  }

  if ( layer2->geometryType() != QGis::Line )
  {
    return errorList;
  }

  QgsSpatialIndex* index = mLayerIndexes[layer2->id()];
  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );


  QList<FeatureLayer>::Iterator it;
  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );
    if ( testCancelled() )
      break;
    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();
    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();
    bool touched = false;
    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();
      if ( !g2 || !g2->asGeos() )
      {
        QgsMessageLog::logMessage( tr( "No second geometry missing or GEOS import failed." ), tr( "Topology plugin" ) );
        continue;
      }
      QgsGeometry* startPoint = QgsGeometry::fromPoint( g2->asPolyline().first() );
      QgsGeometry* endPoint = QgsGeometry::fromPoint( g2->asPolyline().last() );

      if ( g1->intersects( startPoint ) || g1->intersects( endPoint ) )
      {
        touched = true;
        break;
      }
    }
    if ( !touched )
    {
      QgsGeometry* conflictGeom = new QgsGeometry( *g1 );
      if ( isExtent )
      {
        if ( canvasExtentPoly->disjoint( conflictGeom ) )
        {
          continue;
        }
      }

      QList<FeatureLayer> fls;
      fls << *it << *it;
      //bb.scale(10);

      TopolErrorPointNotCoveredByLineEnds* err = new TopolErrorPointNotCoveredByLineEnds( bb, conflictGeom, fls );
      errorList << err;
    }
  }
  return errorList;

}

ErrorList topolTest::checkyLineEndsCoveredByPoints( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );

  int i = 0;
  ErrorList errorList;


  if ( layer1->geometryType() != QGis::Line )
  {
    return errorList;
  }

  if ( layer2->geometryType() != QGis::Point )
  {
    return errorList;
  }

  QgsSpatialIndex* index = mLayerIndexes[layer2->id()];

  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );

  QList<FeatureLayer>::Iterator it;
  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );
    if ( testCancelled() )
      break;
    QgsGeometry* g1 = it->feature.geometry();

    QgsGeometry* startPoint = QgsGeometry::fromPoint( g1->asPolyline().first() );
    QgsGeometry* endPoint = QgsGeometry::fromPoint( g1->asPolyline().last() );

    QgsRectangle bb = g1->boundingBox();
    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();
    bool touched = false;

    bool touchStartPoint = false;
    bool touchEndPoint = false;

    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();
      if ( !g2 || !g2->asGeos() )
      {
        QgsMessageLog::logMessage( tr( "No second geometry missing or GEOS import failed." ), tr( "Topology plugin" ) );
        continue;
      }


      if ( g2->intersects( startPoint ) )
      {
        touchStartPoint = true;
      }

      if ( g2->intersects( endPoint ) )
      {
        touchEndPoint = true ;
      }

      if ( touchStartPoint && touchEndPoint )
      {
        touched = true;
        break;
      }

    }
    if ( !touched )
    {
      QgsGeometry* conflictGeom = new QgsGeometry( *g1 );

      if ( isExtent )
      {
        if ( canvasExtentPoly->disjoint( conflictGeom ) )
        {
          continue;
        }
        if ( canvasExtentPoly->crosses( conflictGeom ) )
        {
          conflictGeom = conflictGeom->intersection( canvasExtentPoly );
        }
      }
      QList<FeatureLayer> fls;
      fls << *it << *it;
      //bb.scale(10);


      TopolErrorLineEndsNotCoveredByPoints* err = new TopolErrorLineEndsNotCoveredByPoints( bb, conflictGeom, fls );
      errorList << err;
    }
  }
  return errorList;

}

ErrorList topolTest::checkPointInPolygon( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );

  int i = 0;
  ErrorList errorList;

  if ( layer1->geometryType() != QGis::Point )
  {
    return errorList;
  }

  if ( layer2->geometryType() != QGis::Polygon )
  {
    return errorList;
  }

  QgsSpatialIndex* index = mLayerIndexes[layer2->id()];

  QgsGeometry* canvasExtentPoly = QgsGeometry::fromWkt( theQgsInterface->mapCanvas()->extent().asWktPolygon() );

  QList<FeatureLayer>::Iterator it;
  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );
    if ( testCancelled() )
      break;
    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();
    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();
    bool touched = false;
    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();
      if ( !g2 || !g2->asGeos() )
      {
        QgsMessageLog::logMessage( tr( "No second geometry missing or GEOS import failed." ), tr( "Topology plugin" ) );
        continue;
      }
      if ( g2->contains( g1 ) )
      {
        touched = true;
        break;
      }
    }
    if ( !touched )
    {
      QgsGeometry* conflictGeom = new QgsGeometry( *g1 );

      if ( isExtent )
      {
        if ( canvasExtentPoly->disjoint( conflictGeom ) )
        {
          continue;
        }
      }

      QList<FeatureLayer> fls;
      fls << *it << *it;
      //bb.scale(10);

      TopolErrorPointNotInPolygon* err = new TopolErrorPointNotInPolygon( bb, conflictGeom, fls );
      errorList << err;
    }
  }
  return errorList;

}


ErrorList topolTest::checkPolygonContainsPoint( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( isExtent );

  int i = 0;
  ErrorList errorList;

  if ( layer1->geometryType() != QGis::Polygon )
  {
    return errorList;
  }

  if ( layer2->geometryType() != QGis::Point )
  {
    return errorList;
  }

  QgsSpatialIndex* index = mLayerIndexes[layer2->id()];

  QList<FeatureLayer>::Iterator it;
  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );
    if ( testCancelled() )
      break;
    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();
    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects( bb );
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();
    bool touched = false;
    for ( ; cit != crossingIdsEnd; ++cit )
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();
      if ( !g2 || !g2->asGeos() )
      {
        QgsMessageLog::logMessage( tr( "No second geometry missing or GEOS import failed." ), tr( "Topology plugin" ) );
        continue;
      }
      if ( g1->contains( g2 ) )
      {
        touched = true;
        break;
      }
    }
    if ( !touched )
    {
      QList<FeatureLayer> fls;
      fls << *it << *it;
      //bb.scale(10);
      QgsGeometry* conflict = new QgsGeometry( *g1 );
      TopolErrorPolygonContainsPoint* err = new TopolErrorPolygonContainsPoint( bb, conflict, fls );
      errorList << err;
    }
  }
  return errorList;
}

ErrorList topolTest::checkMultipart( double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent )
{
  Q_UNUSED( tolerance );
  Q_UNUSED( layer2 );
  Q_UNUSED( layer1 );
  Q_UNUSED( isExtent );

  int i = 0;
  ErrorList errorList;
  QList<FeatureLayer>::Iterator it;
  for ( it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it )
  {
    if ( !( ++i % 100 ) )
      emit progress( ++i );
    if ( testCancelled() )
      break;
    QgsGeometry* g = it->feature.geometry();
    if ( !g )
    {
      QgsMessageLog::logMessage( tr( "Missing geometry in multipart check." ), tr( "Topology plugin" ) );
      continue;
    }
    if ( !g->asGeos() )
      continue;
    if ( g->isMultipart() )
    {
      QgsRectangle r = g->boundingBox();
      QList<FeatureLayer> fls;
      fls << *it << *it;
      QgsGeometry* conflict = new QgsGeometry( *g );
      TopolErroMultiPart* err = new TopolErroMultiPart( r, conflict, fls );
      errorList << err;
    }
  }
  return errorList;
}

void topolTest::fillFeatureMap( QgsVectorLayer* layer, QgsRectangle extent )
{
  QgsFeatureIterator fit;
  if ( extent.isEmpty() )
  {
    fit = layer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
  }
  else
  {
    fit = layer->getFeatures( QgsFeatureRequest()
                              .setFilterRect( extent )
                              .setFlags( QgsFeatureRequest::ExactIntersect )
                              .setSubsetOfAttributes( QgsAttributeList() ) );
  }

  QgsFeature f;

  while ( fit.nextFeature( f ) )
  {
    if ( f.geometry() )
    {
      mFeatureMap2[f.id()] = FeatureLayer( layer, f );
    }
  }
}

void topolTest::fillFeatureList( QgsVectorLayer* layer, QgsRectangle extent )
{
  QgsFeatureIterator fit;
  if ( extent.isEmpty() )
  {
    fit = layer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
  }
  else
  {
    fit = layer->getFeatures( QgsFeatureRequest()
                              .setFilterRect( extent )
                              .setFlags( QgsFeatureRequest::ExactIntersect )
                              .setSubsetOfAttributes( QgsAttributeList() ) );
  }

  QgsFeature f;

  while ( fit.nextFeature( f ) )
  {
    if ( f.geometry() )
    {
      mFeatureList1 << FeatureLayer( layer, f );
    }
  }

}

QgsSpatialIndex* topolTest::createIndex( QgsVectorLayer* layer, QgsRectangle extent )
{
  QgsSpatialIndex* index = new QgsSpatialIndex();

  QgsFeatureIterator fit;
  if ( extent.isEmpty() )
  {
    fit = layer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
  }
  else
  {
    fit = layer->getFeatures( QgsFeatureRequest()
                              .setFilterRect( extent )
                              .setFlags( QgsFeatureRequest::ExactIntersect )
                              .setSubsetOfAttributes( QgsAttributeList() ) );
  }


  int i = 0;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    if ( !( ++i % 100 ) )
      emit progress( i );

    if ( testCancelled() )
    {
      delete index;
      return 0;
    }

    if ( f.geometry() )
    {
      index->insertFeature( f );
      mFeatureMap2[f.id()] = FeatureLayer( layer, f );
    }
  }

  return index;
}

ErrorList topolTest::runTest( QString testName, QgsVectorLayer* layer1, QgsVectorLayer* layer2, ValidateType type, double tolerance )
{
  QgsDebugMsg( QString( "Running test %1" ).arg( testName ) );
  ErrorList errors;

  if ( !layer1 )
  {
    QgsMessageLog::logMessage( tr( "First layer not found in registry." ), tr( "Topology plugin" ) );
    return errors;
  }

  if ( !layer2 && mTopologyRuleMap[testName].useSecondLayer )
  {
    QgsMessageLog::logMessage( tr( "Second layer not found in registry." ), tr( "Topology plugin" ) );
    return errors;
  }

  QString secondLayerId;
  mFeatureList1.clear();
  mFeatureMap2.clear();

  //checking if new features are not
  //being recognised due to indexing not being upto date

  mLayerIndexes.clear();

  if ( mTopologyRuleMap[testName].useSecondLayer )
  {
    // validate all features or current extent
    QgsRectangle extent;
    if ( type == ValidateExtent )
    {
      extent = theQgsInterface->mapCanvas()->extent();
    }
    else
    {
      extent = QgsRectangle();
    }

    fillFeatureList( layer1, extent );
    //fillFeatureMap( layer1, extent );

    QString secondLayerId = layer2->id();

    if ( !mLayerIndexes.contains( layer2->id() ) )
    {
      mLayerIndexes[layer2->id()] = createIndex( layer2, extent );
    }
  }
  else
  {
    // validate all features or current extent
    QgsRectangle extent;
    if ( type == ValidateExtent )
    {
      extent = theQgsInterface->mapCanvas()->extent();
      if ( mTopologyRuleMap[testName].useSpatialIndex )
      {
        mLayerIndexes[layer1->id()] = createIndex( layer1, theQgsInterface->mapCanvas()->extent() );
      }
      else
      {
        fillFeatureList( layer1, extent );
      }
    }
    else
    {
      if ( mTopologyRuleMap[testName].useSpatialIndex )
      {
        if ( !mLayerIndexes.contains( layer1->id() ) )
        {

          mLayerIndexes[layer1->id()] = createIndex( layer1, QgsRectangle() );
        }
      }
      else
      {
        fillFeatureList( layer1, QgsRectangle() );
      }
    }

  }

  //call test routine
  bool isValidatingExtent;
  if ( type == ValidateExtent )
  {
    isValidatingExtent = true;
  }
  else
  {
    isValidatingExtent = false;
  }

  return ( this->*( mTopologyRuleMap[testName].f ) )( tolerance, layer1, layer2, isValidatingExtent );
}
