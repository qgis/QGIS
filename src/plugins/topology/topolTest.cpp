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
#include <cmath>
#include <set>
#include <map>


#include "geosFunctions.h"
//#include "../../app/qgisapp.h"

topolTest::topolTest(QgisInterface* qgsIface)
{
  theQgsInterface = qgsIface;
  mTestCancelled = false;

  // one layer tests
  mTestMap["must not have invalid geometries"].f = &topolTest::checkValid;
  mTestMap["must not have invalid geometries"].useSecondLayer = false;

  mTestMap["segments must have minimum length"].f = &topolTest::checkSegmentLength;
  mTestMap["segments must have minimum length"].useTolerance = true;
  mTestMap["segments must have minimum length"].useSecondLayer = false;

  mTestMap["must not have hanging lines"].f = &topolTest::checkDanglingLines;
  mTestMap["must not have hanging lines"].useSecondLayer = false;

  mTestMap["must not have duplicates"].f = &topolTest::checkDuplicates;
  mTestMap["must not have duplicates"].useTolerance = false;
  mTestMap["must not have duplicates"].useSecondLayer = false;

  mTestMap["must not have pseudos"].f = &topolTest::checkPseudos;
  mTestMap["must not have pseudos"].useTolerance = false;
  mTestMap["must not have pseudos"].useSecondLayer = false;

  // two layer tests
  mTestMap["layers must not intersect"].f = &topolTest::checkIntersections;
  mTestMap["layers must not intersect"].useSecondLayer = true;
  mTestMap["layers must not intersect"].useTolerance = false;

  mTestMap["polygons must contain feature"].f = &topolTest::checkPolygonContains;
  mTestMap["polygons must contain feature"].useSecondLayer = true;
  mTestMap["polygons must contain feature"].useTolerance = false;

  mTestMap["points must be covered by segments"].f = &topolTest::checkPointCoveredBySegment;
  mTestMap["features must not be closer than tolerance"].f = &topolTest::checkCloseFeature;
  mTestMap["features must not be closer than tolerance"].useTolerance = true;
}

topolTest::~topolTest()
{
  QMap<QString, QgsSpatialIndex*>::Iterator lit = mLayerIndexes.begin();
  for (; lit != mLayerIndexes.end(); ++lit)
    delete *lit;
}

void topolTest::setTestCancelled()
{
  mTestCancelled = true;
}

bool topolTest::testCancelled()
{
  if (mTestCancelled)
  {
    mTestCancelled = false;
    return true;
  }

  return false;
}

ErrorList topolTest::checkCloseFeature(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  ErrorList errorList;
  QString secondLayerId = layer2->getLayerID();
  QgsSpatialIndex* index = mLayerIndexes[secondLayerId];
  if (!index)
  {
    std::cout << "No index for layer " << secondLayerId.toStdString() << "!\n";
    return errorList;
  }

  bool badG1 = false, badG2 = false;
  bool skipItself = layer1 == layer2;

  int i = 0;
  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
  {
    if (!(++i % 100))
      emit progress(i);

    if (testCancelled())
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    // increase bounding box by tolerance
    QgsRectangle frame(bb.xMinimum() - tolerance, bb.yMinimum() - tolerance, bb.xMaximum() + tolerance, bb.yMaximum() + tolerance); 

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects(frame);
    int crossSize = crossingIds.size();
    
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    for (; cit != crossingIdsEnd; ++cit)
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      // skip itself, when invoked with the same layer
      if (skipItself && f.id() == it->feature.id())
        continue;

      if (!g2 || !g2->asGeos())
      {
	badG2 = true;
        continue;
      }

      if (!g1 || !g1->asGeos())
      {
	badG1 = true;
        continue;
      }

      if (g1->distance(*g2) < tolerance)
      {
        QgsRectangle r = g2->boundingBox();
	r.combineExtentWith(&bb);

	QList<FeatureLayer> fls;
	FeatureLayer fl;
	fl.feature = f;
	fl.layer = layer2;
	fls << *it << fl;
	QgsGeometry* conflict = new QgsGeometry(*g2);
	TopolErrorClose* err = new TopolErrorClose(r, conflict, fls);
	//TopolErrorClose* err = new TopolErrorClose(r, g2, fls);

	errorList << err;
      }
    }
  }

  if (badG2)
    std::cout << "g2 or g2->asGeos() == NULL in close\n" << std::flush;

  if (badG1)
    std::cout << "g1 or g1->asGeos() == NULL in close\n" << std::flush;

  return errorList;
}

ErrorList topolTest::checkDanglingLines(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  //TODO: multilines - check all separate pieces
  int i = 0;
  ErrorList errorList;
  QString layerId = layer1->getLayerID();
  QgsSpatialIndex* index = mLayerIndexes[layerId];
  if (!index)
  {
    // attempt to create new index - it was not built in runtest()
    mLayerIndexes[layerId] = createIndex(layer1);
    index = mLayerIndexes[layerId];

    if (!index)
    {
      std::cout << "No index for layer " << layerId.toStdString() << "!\n";
      return errorList;
    }
  }
  else // map was not filled in runtest()
    fillFeatureMap(layer1);
  
  if (layer1->geometryType() != QGis::Line)
    return errorList;

  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
  {
    if (!(++i % 100))
      emit progress(i);

    if (testCancelled())
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects(bb);
    int crossSize = crossingIds.size();
    
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    // QList for multilines
    // QList<QgsPoint> endPoints;
    // QGis::WkbType type = g1->wkbType();
    // if (type == WKBMultiLineString || type == WKBMultiLineString25D)
    //   for (...)
    // else
    QgsGeometry* startPoint = QgsGeometry::fromPoint(g1->asPolyline().first());
    QgsGeometry* endPoint = QgsGeometry::fromPoint(g1->asPolyline().last());
    if (!startPoint || !endPoint)
      continue;

    bool touches = false;
    for (; cit != crossingIdsEnd; ++cit)
    {
      // skip itself
      if (mFeatureMap2[*cit].feature.id() == it->feature.id())
	continue;

      QgsGeometry* g2 = mFeatureMap2[*cit].feature.geometry();
      if (!g2)
      {
	std::cout << "g2 == NULL in dangling line test\n" << std::flush;
        continue;
      }

      if (!g2->asGeos())
      {
	std::cout << "g2->asGeos() == NULL in dangling line test\n" << std::flush;
        continue;
      }

      //if (touchesPoints(endPoints, g2))
      // test both endpoints
      if (geosTouches(startPoint, g2) || geosTouches(endPoint, g2))
      {
	touches = true;
	break;
      }
    }

    if (!touches)
    {
      QList<FeatureLayer> fls;
      fls << *it << *it;
      QgsGeometry* conflict = new QgsGeometry(*g1);
      TopolErrorDangle* err = new TopolErrorDangle(bb, conflict, fls);

      errorList << err;
    }

    delete startPoint, endPoint;
  }

  return errorList;
}

/*bool touchesPoints(QList<QgsPoint endPoints, QgsGeometry* g)
{
}
*/

ErrorList topolTest::checkDuplicates(double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2)
{
    //TODO: multilines - check all separate pieces
    int i = 0;
    ErrorList errorList;
    QString layerId = layer1->getLayerID();
    QgsSpatialIndex* index = mLayerIndexes[layerId];
    if (!index)
    {
      // attempt to create new index - it was not built in runtest()
      mLayerIndexes[layerId] = createIndex(layer1);
      index = mLayerIndexes[layerId];

      if (!index)
      {
        std::cout << "No index for layer " << layerId.toStdString() << "!\n";
        return errorList;
      }
    }
    else // map was not filled in runtest()
      fillFeatureMap(layer1);

    QList<int>* duplicateIds = new QList<int>();

    QList<FeatureLayer>::Iterator it;
    QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
    for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
    {
      if (!(++i % 100))
        emit progress(i);

      int currentId = it->feature.id();

      if(duplicateIds->contains(currentId))
      {
          //is already a duplicate geometry..skip..
          continue;
      }

      if (testCancelled())
        break;

      QgsGeometry* g1 = it->feature.geometry();
      QgsRectangle bb = g1->boundingBox();

      QList<QgsFeatureId> crossingIds;
      crossingIds = index->intersects(bb);
      int crossSize = crossingIds.size();

      QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
      QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

      bool duplicate = false;

      for (; cit != crossingIdsEnd; ++cit)
      {
          duplicate = false;
        // skip itself
        if (mFeatureMap2[*cit].feature.id() == it->feature.id())
      continue;

        QgsGeometry* g2 = mFeatureMap2[*cit].feature.geometry();
        if (!g2)
        {
      std::cout << "g2 == NULL in dangling line test\n" << std::flush;
          continue;
        }

        if (!g2->asGeos())
        {
      std::cout << "g2->asGeos() == NULL in dangling line test\n" << std::flush;
          continue;
        }

        if(geosEquals(g1,g2))
        {
            duplicate = true;
            duplicateIds->append(mFeatureMap2[*cit].feature.id());
        }




      if (duplicate)
      {
        QList<FeatureLayer> fls;
        fls << *it << *it;
        QgsGeometry* conflict = new QgsGeometry(*g1);
        TopolErrorDuplicates* err = new TopolErrorDuplicates(bb, conflict, fls);

        errorList << err;
      }

      }

    }

    return errorList;

}

ErrorList topolTest::checkPseudos(double tolerance, QgsVectorLayer *layer1, QgsVectorLayer *layer2)
{    
    int i = 0;
    ErrorList errorList;
    QString layerId = layer1->getLayerID();
    QgsSpatialIndex* index = mLayerIndexes[layerId];
    if (!index)
    {
      mLayerIndexes[layerId] = createIndex(layer1);
      index = mLayerIndexes[layerId];

      if (!index)
      {
        std::cout << "No index for layer " << layerId.toStdString() << "!\n";
        return errorList;
      }
    }
    else // map was not filled in runtest()
      fillFeatureMap(layer1);

    if (layer1->geometryType() != QGis::Line)
      return errorList;

    QList<FeatureLayer>::Iterator it;
    QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();

    qDebug() << mFeatureList1.count();

    QgsPoint startPoint;
    QgsPoint endPoint;

    std::multimap<QgsPoint,QgsFeatureId,PointComparer> endVerticesMap;

    for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
    {
      if (!(++i % 100))
        emit progress(i);

      if (testCancelled())
        break;

      QgsGeometry* g1 = it->feature.geometry();

      if (!g1)
      {
        std::cout << "g2 == NULL in pseudo line test\n" << std::flush;
        continue;
      }

      if (!g1->asGeos())
      {
        std::cout << "g2->asGeos() == NULL in pseudo line test\n" << std::flush;
        continue;
      }

      if(g1->isMultipart())
      {
           QgsMultiPolyline lines = g1->asMultiPolyline();
           for (int m = 0; m < lines.count(); m++)
           {
               QgsPolyline line = lines[m];
               startPoint = line[0];
               endPoint = line[line.size() - 1];

               endVerticesMap.insert(std::pair<QgsPoint,QgsFeatureId>(startPoint,it->feature.id()));
               endVerticesMap.insert(std::pair<QgsPoint,QgsFeatureId>(endPoint,it->feature.id()));

           }
      }
      else
      {
        QgsPolyline polyline = g1->asPolyline();
        startPoint = polyline[0];
        endPoint = polyline[polyline.size()-1];
        endVerticesMap.insert(std::pair<QgsPoint,QgsFeatureId>(startPoint,it->feature.id()));
        endVerticesMap.insert(std::pair<QgsPoint,QgsFeatureId>(endPoint,it->feature.id()));
      }
    }


    for(std::multimap<QgsPoint, QgsFeatureId,PointComparer>::iterator pointIt = endVerticesMap.begin(), end = endVerticesMap.end(); pointIt != end; pointIt = endVerticesMap.upper_bound(pointIt->first))
    {
        QgsPoint p = pointIt->first;
        QgsFeatureId k = pointIt->second;

        int repetitions = endVerticesMap.count(p);

        if(repetitions == 2)
        {
            QgsGeometry* conflictGeom = QgsGeometry::fromPoint(p);
            QgsRectangle bBox = conflictGeom->boundingBox();
            QgsFeature feat;

            FeatureLayer ftrLayer1;
            //need to fetch attributes?? being safe side by fetching..
            layer1->featureAtId(k,feat,true,true);
            ftrLayer1.feature = feat;
            ftrLayer1.layer = layer1;

            QList<FeatureLayer> errorFtrLayers;
            errorFtrLayers << ftrLayer1 << ftrLayer1;

            TopolErrorPseudos* err = new TopolErrorPseudos(bBox, conflictGeom, errorFtrLayers);
            errorList << err;

        }
    }

    return errorList;
}

ErrorList topolTest::checkValid(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  int i = 0;
  ErrorList errorList;
  QList<FeatureLayer>::Iterator it;

  for (it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it)
  {
    if (!(++i % 100))
      emit progress(++i);
    if (testCancelled())
      break;

    QgsGeometry* g = it->feature.geometry();
    if (!g)
    {
      std::cout << "validity test: invalid QgsGeometry pointer\n" << std::flush;
      continue;
    }

    if (!g->asGeos())
      continue;

    if (!GEOSisValid(g->asGeos()))
    {
      QgsRectangle r = g->boundingBox();
      QList<FeatureLayer> fls;
      fls << *it << *it;

      QgsGeometry* conflict = new QgsGeometry(*g);
      TopolErrorValid* err = new TopolErrorValid(r, conflict, fls);
      errorList << err;
    }
  }

  return errorList;
}

ErrorList topolTest::checkPolygonContains(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  int i = 0;
  ErrorList errorList;
  QString secondLayerId = layer2->getLayerID();
  QgsSpatialIndex* index = mLayerIndexes[secondLayerId];

  bool skipItself = layer1 == layer2;

  if (!index)
  {
    std::cout << "No index for layer " << secondLayerId.toStdString() << "!\n";
    return errorList;
  }

  if (layer1->geometryType() != QGis::Polygon)
    return errorList;
  
  bool contains = false;
  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
  {
    if (!(++i % 100))
      emit progress(i);

    if (testCancelled())
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects(bb);
    int crossSize = crossingIds.size();
    
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    for (; cit != crossingIdsEnd; ++cit)
    {
      contains = false;
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      // skip itself, when invoked with the same layer
      if (skipItself && f.id() == it->feature.id())
        continue;

      if (!g2)
      {
        std::cout << "g2 == NULL in contains\n" << std::flush;
        continue;
      }

      if (!g2->asGeos())
      {
        std::cout << "g2->asGeos() == NULL in contains\n" << std::flush;
        continue;
      }

      if (geosContains(g1, g2) == true)
      {
          contains = true;
          continue;
      }
    }

    if(contains == false)
    {
        QList<FeatureLayer> fls;
        FeatureLayer fl;
        fl.feature = it->feature;
        fl.layer = layer2;
        fls << *it << fl;
        QgsGeometry* conflict = new QgsGeometry(*g1);
        TopolErrorInside* err = new TopolErrorInside(bb, conflict, fls);

        errorList << err;
    }
  }

  return errorList;
}

ErrorList topolTest::checkPointCoveredBySegment(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  int i = 0;
  ErrorList errorList;
  QString secondLayerId = layer2->getLayerID();
  QgsSpatialIndex* index = mLayerIndexes[secondLayerId];

  if (!index)
  {
    std::cout << "No index for layer " << secondLayerId.toStdString() << "!\n";
    return errorList;
  }

  if (layer1->geometryType() != QGis::Point)
    return errorList;
  if (layer2->geometryType() == QGis::Point)
    return errorList;

  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for (it = mFeatureList1.begin(); it != mFeatureList1.end(); ++it)
  {
    if (!(++i % 100))
      emit progress(i);

    if (testCancelled())
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects(bb);
    int crossSize = crossingIds.size();
    
    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();

    bool touched = false;

    for (; cit != crossingIdsEnd; ++cit)
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      if (!g2 || !g2->asGeos())
      {
	std::cout << "g2 or g2->asGeos() == NULL in covered\n" << std::flush;
        continue;
      }

      // test if point touches other geometry
      if (geosTouches(g1, g2))
      {
	touched = true;
	break;
      }
    }

    if (!touched)
    {
      QList<FeatureLayer> fls;
      fls << *it << *it;
      //bb.scale(10);
      QgsGeometry* conflict = new QgsGeometry(*g1);
      TopolErrorCovered* err = new TopolErrorCovered(bb, conflict, fls);

      errorList << err;
    }
  }

  return errorList;
}

ErrorList topolTest::checkSegmentLength(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  int i = 0;
  ErrorList errorList;
  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();

  for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
  {
    QgsGeometry* g1 = it->feature.geometry();
    QgsPolygon pol;
    QgsMultiPolygon mpol;
    QgsPolyline segm;
    QgsPolyline ls;
    QgsMultiPolyline mls;
    QList<FeatureLayer> fls;
    TopolErrorShort* err;

    // switching by type here, because layer can contain both single and multi version geometries
    switch (g1->wkbType()) {
      case QGis::WKBLineString:
      case QGis::WKBLineString25D:
        ls = g1->asPolyline();

	for (int i = 1; i < ls.size(); ++i)
	{

      double distance = sqrt(ls[i-1].sqrDist(ls[i]));
      if (distance < tolerance)
	  {
	    fls.clear();
            fls << *it << *it;
	    segm.clear();
	    segm << ls[i-1] << ls[i];
            QgsGeometry* conflict = QgsGeometry::fromPolyline(segm);
            err = new TopolErrorShort(g1->boundingBox(), conflict, fls);
            //err = new TopolErrorShort(g1->boundingBox(), QgsGeometry::fromPolyline(segm), fls);
            errorList << err;
	  }
	}
      break;

      case QGis::WKBPolygon:
      case QGis::WKBPolygon25D:
        pol = g1->asPolygon();

	for (int i = 0; i < pol.size(); ++i)
	  for (int j = 1; j < pol[i].size(); ++j)
	    if (pol[i][j-1].sqrDist(pol[i][j]) < tolerance)
	    {
	      fls.clear();
              fls << *it << *it;
	      segm.clear();
	      segm << pol[i][j-1] << pol[i][j];
              QgsGeometry* conflict = QgsGeometry::fromPolyline(segm);
              err = new TopolErrorShort(g1->boundingBox(), conflict, fls);
              errorList << err;
	    }
      break;

      case QGis::WKBMultiLineString:
      case QGis::WKBMultiLineString25D:
        mls = g1->asMultiPolyline();

	for (int k = 0; k < mls.size(); ++k)
	{
	  QgsPolyline& ls = mls[k];
	  for (int i = 1; i < ls.size(); ++i)
	  {
	    if (ls[i-1].sqrDist(ls[i]) < tolerance)
	    {
	      fls.clear();
              fls << *it << *it;
	      segm.clear();
	      segm << ls[i-1] << ls[i];
              QgsGeometry* conflict = QgsGeometry::fromPolyline(segm);
              err = new TopolErrorShort(g1->boundingBox(), conflict, fls);
              errorList << err;
	    }
	  }
	}
      break;

      case QGis::WKBMultiPolygon:
      case QGis::WKBMultiPolygon25D:
        mpol = g1->asMultiPolygon();

	for (int k = 0; k < mpol.size(); ++k)
	{
	  QgsPolygon& pol = mpol[k];
	  for (int i = 0; i < pol.size(); ++i)
	    for (int j = 1; j < pol[i].size(); ++j)
	      if (pol[i][j-1].sqrDist(pol[i][j]) < tolerance)
	      {
	        fls.clear();
                fls << *it << *it;
	        segm.clear();
	        segm << pol[i][j-1] << pol[i][j];
                QgsGeometry* conflict = QgsGeometry::fromPolyline(segm);
                err = new TopolErrorShort(g1->boundingBox(), conflict, fls);
                errorList << err;
	      }
	}
      break;

      default:
        continue;
    }
  }

  return errorList;
}

ErrorList topolTest::checkIntersections(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2)
{
  int i = 0;
  ErrorList errorList;
  QString secondLayerId = layer2->getLayerID();
  QgsSpatialIndex* index = mLayerIndexes[secondLayerId];

  bool skipItself = layer1 == layer2;

  if (!index)
  {
    std::cout << "No index for layer " << secondLayerId.toStdString() << "!\n";
    return errorList;
  }

  QList<FeatureLayer>::Iterator it;
  QList<FeatureLayer>::ConstIterator FeatureListEnd = mFeatureList1.end();
  for (it = mFeatureList1.begin(); it != FeatureListEnd; ++it)
  {
    if (!(++i % 100))
      emit progress(i);

    if (testCancelled())
      break;

    QgsGeometry* g1 = it->feature.geometry();
    QgsRectangle bb = g1->boundingBox();

    QList<QgsFeatureId> crossingIds;
    crossingIds = index->intersects(bb);
    int crossSize = crossingIds.size();

    QList<QgsFeatureId>::Iterator cit = crossingIds.begin();
    QList<QgsFeatureId>::ConstIterator crossingIdsEnd = crossingIds.end();
    for (; cit != crossingIdsEnd; ++cit)
    {
      QgsFeature& f = mFeatureMap2[*cit].feature;
      QgsGeometry* g2 = f.geometry();

      // skip itself, when invoked with the same layer
      if (skipItself && f.id() == it->feature.id())
	continue;

      if (!g2)
      {
        std::cout << "no second geometry\n";
	continue;
      }

      if (g1->intersects(g2))
      {
        QgsRectangle r = bb;
	QgsRectangle r2 = g2->boundingBox();
	r.combineExtentWith(&r2);

	QgsGeometry* conflict = g1->intersection(g2);
	// could this for some reason return NULL?
	if (!conflict)
          continue;
	  //c = new QgsGeometry;

	QList<FeatureLayer> fls;
	FeatureLayer fl;
	fl.feature = f;
	fl.layer = layer2;
	fls << *it << fl;
	TopolErrorIntersection* err = new TopolErrorIntersection(r, conflict, fls);

	errorList << err;
      }
    }
  }

  return errorList;
}

void topolTest::fillFeatureMap(QgsVectorLayer* layer)
{
  layer->select(QgsAttributeList(), QgsRectangle());

  QgsFeature f;
  while (layer->nextFeature(f))
  {
    if (f.geometry())
    { 
      mFeatureMap2[f.id()] = FeatureLayer(layer, f);
    }
  }
}

QgsSpatialIndex* topolTest::createIndex(QgsVectorLayer* layer)
{
  QgsSpatialIndex* index = new QgsSpatialIndex();
  layer->select(QgsAttributeList(), QgsRectangle());

  int i = 0;
  QgsFeature f;
  while (layer->nextFeature(f))
  {
    if (!(++i % 100))
      emit progress(i);

    if (testCancelled())
    {
      delete index;
      return 0;
    }

    if (f.geometry())
    {
      index->insertFeature(f);
      mFeatureMap2[f.id()] = FeatureLayer(layer, f);
    }
  }

  return index;
}

ErrorList topolTest::runTest(QString testName, QgsVectorLayer* layer1, QgsVectorLayer* layer2, ValidateType type, double tolerance)
{
  std::cout << testName.toStdString();
  ErrorList errors;

  if (!layer1)
  {
    std::cout << "First layer not found in registry!\n" << std::flush;
    return errors;
  }

  if (!layer2 && mTestMap[testName].useSecondLayer)
  {
    std::cout << "Second layer not found in registry!\n" << std::flush;
    return errors;
  }

  QString secondLayerId;
  mFeatureList1.clear();
  mFeatureMap2.clear();

  //checking if new features are not
  //being recognised due to indexing not being upto date

  mLayerIndexes.clear();
  QgsFeature f;

  if (layer2)
  {
    secondLayerId = layer2->getLayerID();
    if (!mLayerIndexes.contains(secondLayerId))
      mLayerIndexes[secondLayerId] = createIndex(layer2);
    else
      fillFeatureMap(layer2);
  }

  // validate only selected features
  if (type == ValidateSelected)
  {
      QgsFeatureList flist = layer1->selectedFeatures();
      QgsFeatureList::Iterator fit = flist.begin();
      QgsFeatureList::Iterator flistEnd = flist.end();
  
      for (; fit != flist.end(); ++fit)
	if (fit->geometry())
          mFeatureList1 << FeatureLayer(layer1, *fit);

  }
  // validate all features or current extent
  else {
    QgsRectangle extent;
    if (type == ValidateExtent)
        extent = theQgsInterface->mapCanvas()->extent();
        //extent = theQgsInterface.mapCanvas()->extent(); // QgisApp::instance()->mapCanvas()->extent();

    layer1->select(QgsAttributeList(), extent);
    while (layer1->nextFeature(f))
      if (f.geometry())
        mFeatureList1 << FeatureLayer(layer1, f);
  }

  //call test routine
  return (this->*(mTestMap[testName].f))(tolerance, layer1, layer2);
}
