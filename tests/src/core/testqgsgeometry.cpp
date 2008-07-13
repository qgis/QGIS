/***************************************************************************
     testqgsgeometry.cpp
     --------------------------------------
    Date                 : 20 Jan 2008
    Copyright            : (C) 2008 by Tim Sutton
    Email                : tim @ linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

#include <iostream>
//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgspoint.h>

/** \ingroup UnitTests
 * This is a unit test for the different geometry operations on vector features.
 */
class TestQgsGeometry: public QObject
{
  Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void intersectionCheck();
    void unionCheck1();
    void unionCheck2();
  private:
    /** A helper method to return wkb geometry type as a string */
    QString wkbTypeAsString( QGis::WKBTYPE theType );
    /** A helper method to dump to qdebug the geometry of a multipolygon */
    void dumpMultiPolygon( QgsMultiPolygon &theMultiPolygon );
    /** A helper method to dump to qdebug the geometry of a polygon */
    void dumpPolygon( QgsPolygon &thePolygon );
    QgsPoint mPoint1; 
    QgsPoint mPoint2; 
    QgsPoint mPoint3; 
    QgsPoint mPoint4; 
    QgsPoint mPointA; 
    QgsPoint mPointB; 
    QgsPoint mPointC; 
    QgsPoint mPointD; 
    QgsPoint mPointW; 
    QgsPoint mPointX; 
    QgsPoint mPointY; 
    QgsPoint mPointZ; 
    QgsPolyline mPolylineA;
    QgsPolyline mPolylineB;
    QgsPolyline mPolylineC;
    QgsPolygon mPolygonA;
    QgsPolygon mPolygonB;
    QgsPolygon mPolygonC;
    QgsGeometry * mpPolygonGeometryA;
    QgsGeometry * mpPolygonGeometryB;
    QgsGeometry * mpPolygonGeometryC;

    QString mTestDataDir;
};


void TestQgsGeometry::init()
{
  //
  // Reset / reinitialise the geometries before each test is run
  //
  mPoint1 = QgsPoint(20.0,20.0); 
  mPoint2 = QgsPoint(80.0,20.0); 
  mPoint3 = QgsPoint(80.0,80.0);
  mPoint4 = QgsPoint(20.0,80.0);
  mPointA = QgsPoint(40.0,40.0);
  mPointB = QgsPoint(100.0,40.0); 
  mPointC = QgsPoint(100.0,100.0); 
  mPointD = QgsPoint(40.0,100.0); 
  mPointW = QgsPoint(1000.0,1000.0); 
  mPointX = QgsPoint(1040.0,1000.0); 
  mPointY = QgsPoint(1040.0,1040.0); 
  mPointZ = QgsPoint(1000.0,1040.0); 
  
  mPolygonA.clear();
  mPolygonB.clear();
  mPolygonC.clear();
  mPolylineA.clear();
  mPolylineB.clear();
  mPolylineC.clear();
  mPolylineA << mPoint1 << mPoint2 << mPoint3 << mPoint4 << mPoint1;
  mPolygonA << mPolylineA;
  //Polygon B intersects Polygon A
  mPolylineB << mPointA << mPointB << mPointC << mPointD << mPointA;
  mPolygonB << mPolylineB;
  // Polygon C should intersect no other polys
  mPolylineC << mPointW << mPointX << mPointY << mPointZ << mPointW;
  mPolygonC << mPolylineC;

  //polygon: first item of the list is outer ring, 
  // inner rings (if any) start from second item 
  mpPolygonGeometryA = QgsGeometry::fromPolygon(mPolygonA);
  mpPolygonGeometryB = QgsGeometry::fromPolygon(mPolygonB);
  mpPolygonGeometryC = QgsGeometry::fromPolygon(mPolygonC);

}

void TestQgsGeometry::cleanup()
{
  // will be called after every testfunction.
  delete mpPolygonGeometryA;
  delete mpPolygonGeometryB;
  delete mpPolygonGeometryC;
}

void TestQgsGeometry::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(INSTALL_PREFIX, true);
  QgsApplication::showSettings();
}

void TestQgsGeometry::cleanupTestCase()
{
  //
  // Runs once after all tests are run
  //
  
}

void TestQgsGeometry::intersectionCheck()
{
  QVERIFY ( mpPolygonGeometryA->intersects(mpPolygonGeometryB));
  QVERIFY ( !mpPolygonGeometryA->intersects(mpPolygonGeometryC));
}

void TestQgsGeometry::unionCheck1()
{
  // should be a multipolygon with 2 parts as A does not intersect C
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->Union(mpPolygonGeometryC);
  qDebug ("Geometry Type: " + wkbTypeAsString(mypUnionGeometry->wkbType()).toLocal8Bit());
  QVERIFY (mypUnionGeometry->wkbType() == QGis::WKBMultiPolygon);
  QgsMultiPolygon myMultiPolygon = mypUnionGeometry->asMultiPolygon();
  QVERIFY (myMultiPolygon.size() > 0); //check that the union did not fail
  dumpMultiPolygon(myMultiPolygon);
}
void TestQgsGeometry::unionCheck2()
{
  // should be a single polygon as A intersect B
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->Union(mpPolygonGeometryB);
  qDebug ("Geometry Type: " + wkbTypeAsString(mypUnionGeometry->wkbType()).toLocal8Bit());
  QVERIFY (mypUnionGeometry->wkbType() == QGis::WKBPolygon);
  QgsPolygon myPolygon = mypUnionGeometry->asPolygon();
  QVERIFY (myPolygon.size() > 0); //check that the union created a feature
  dumpPolygon(myPolygon);
  delete mypUnionGeometry;
}

void TestQgsGeometry::dumpMultiPolygon( QgsMultiPolygon &theMultiPolygon )
{
  qDebug ("Multipolygon Geometry Dump");
  for (int i = 0; i < theMultiPolygon.size(); i++)
  {
    QgsPolygon myPolygon = theMultiPolygon.at(i);
    qDebug("\tPolygon in multipolygon: " + QString::number(i).toLocal8Bit());    
    dumpPolygon(myPolygon);
  }
}

void TestQgsGeometry::dumpPolygon( QgsPolygon &thePolygon )
{
  for ( int j = 0; j < thePolygon.size(); j++ )
  {
    QgsPolyline myPolyline = thePolygon.at( j ); //rings of polygon
    qDebug( "\t\tRing  in polygon: " + QString::number( j ).toLocal8Bit() );

    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      QgsPoint myPoint = myPolyline.at( k );
      qDebug( "\t\t\tPoint in ring " + QString::number( k ).toLocal8Bit() + " :" + myPoint.stringRep().toLocal8Bit() );
    }
  }
}

QString TestQgsGeometry::wkbTypeAsString( QGis::WKBTYPE theType )
{
  switch ( theType )
  {
    case QGis::WKBPoint:
      return "WKBPoint";
    case QGis::WKBLineString:
      return "WKBLineString";
    case QGis::WKBPolygon:
      return "WKBPolygon";
    case QGis::WKBMultiPoint:
      return "WKBMultiPoint";
    case QGis::WKBMultiLineString:
      return "WKBMultiLineString";
    case QGis::WKBMultiPolygon:
      return "WKBMultiPolygon";
    case QGis::WKBUnknown:
      return "WKBUnknown";
    case QGis::WKBPoint25D:
      return "WKBPoint25D";
    case QGis::WKBLineString25D:
      return "WKBLineString25D";
    case QGis::WKBPolygon25D:
      return "WKBPolygon25D";
    case QGis::WKBMultiPoint25D:
      return "WKBMultiPoint25D";
    case QGis::WKBMultiLineString25D:
      return "WKBMultiLineString25D";
    case QGis::WKBMultiPolygon25D:
      return "WKBMultiPolygon25D";
    default:
      return "Unknown type";
  }
}

QTEST_MAIN(TestQgsGeometry)
#include "moc_testqgsgeometry.cxx"

