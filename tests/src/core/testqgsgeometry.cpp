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
#include <QVector>
#include <QPointF>
#include <QImage>
#include <QPainter>

#include <iostream>
//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgspoint.h>

//qgs unit test utility class
#include "qgsrenderchecker.h"

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

    void intersectionCheck1();
    void intersectionCheck2();
    void unionCheck1();
    void unionCheck2();
    void differenceCheck1();
    void differenceCheck2();
    void bufferCheck();
  private:
    /** A helper method to do a render check to see if the geometry op is as expected */
    bool renderCheck( QString theTestName, QString theComment = "" );
    /** A helper method to return wkb geometry type as a string */
    QString wkbTypeAsString( QGis::WkbType theType );
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
    QImage mImage;
    QPainter * mpPainter;
    QPen mPen1;
    QPen mPen2;
    QString mReport;
};


void TestQgsGeometry::init()
{
  //
  // Reset / reinitialise the geometries before each test is run
  //
  mPoint1 = QgsPoint( 20.0, 20.0 );
  mPoint2 = QgsPoint( 80.0, 20.0 );
  mPoint3 = QgsPoint( 80.0, 80.0 );
  mPoint4 = QgsPoint( 20.0, 80.0 );
  mPointA = QgsPoint( 40.0, 40.0 );
  mPointB = QgsPoint( 100.0, 40.0 );
  mPointC = QgsPoint( 100.0, 100.0 );
  mPointD = QgsPoint( 40.0, 100.0 );
  mPointW = QgsPoint( 200.0, 200.0 );
  mPointX = QgsPoint( 240.0, 200.0 );
  mPointY = QgsPoint( 240.0, 240.0 );
  mPointZ = QgsPoint( 200.0, 240.0 );

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
  mpPolygonGeometryA = QgsGeometry::fromPolygon( mPolygonA );
  mpPolygonGeometryB = QgsGeometry::fromPolygon( mPolygonB );
  mpPolygonGeometryC = QgsGeometry::fromPolygon( mPolygonC );

  mImage = QImage( 250, 250, QImage::Format_RGB32 );
  mImage.fill( qRgb( 152, 219, 249 ) );
  mpPainter = new QPainter( &mImage );

  // Draw the test shapes first
  mPen1 = QPen();
  mPen1.setWidth( 5 );
  mPen1.setBrush( Qt::green );
  mpPainter->setPen( mPen1 );
  dumpPolygon( mPolygonA );
  mPen1.setBrush( Qt::red );
  mpPainter->setPen( mPen1 );
  dumpPolygon( mPolygonB );
  mPen1.setBrush( Qt::blue );
  mpPainter->setPen( mPen1 );
  dumpPolygon( mPolygonC );

  mPen2 = QPen();
  mPen2.setWidth( 1 );
  mPen2.setBrush( Qt::black );
  QBrush myBrush( Qt::DiagCrossPattern );


  //set the pen to a different colour -
  //any test outs will be drawn in pen2
  mpPainter->setPen( mPen2 );
  mpPainter->setBrush( myBrush );
}

void TestQgsGeometry::cleanup()
{
  // will be called after every testfunction.
  delete mpPolygonGeometryA;
  delete mpPolygonGeometryB;
  delete mpPolygonGeometryC;
  delete mpPainter;
}

void TestQgsGeometry::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath();
  QgsApplication::setPrefixPath( INSTALL_PREFIX, true );
  QgsApplication::showSettings();
  mReport += "<h1>Geometry Tests</h1>\n";
  mReport += "<p><font color=\"green\">Green = polygonA</font></p>\n";
  mReport += "<p><font color=\"red\">Red = polygonB</font></p>\n";
  mReport += "<p><font color=\"blue\">Blue = polygonC</font></p>\n";
}


void TestQgsGeometry::cleanupTestCase()
{
  //
  // Runs once after all tests are run
  //
  QString myReportFile = QDir::tempPath() + QDir::separator() + "geometrytest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    QDesktopServices::openUrl( "file://" + myReportFile );
  }

}



void TestQgsGeometry::intersectionCheck1()
{
  QVERIFY( mpPolygonGeometryA->intersects( mpPolygonGeometryB ) );
  // should be a single polygon as A intersect B
  QgsGeometry * mypIntersectionGeometry  =  mpPolygonGeometryA->intersection( mpPolygonGeometryB );
  qDebug( "Geometry Type: " + wkbTypeAsString( mypIntersectionGeometry->wkbType() ).toLocal8Bit() );
  QVERIFY( mypIntersectionGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypIntersectionGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  delete mypIntersectionGeometry;
  QVERIFY( renderCheck( "geometry_intersectionCheck1", "Checking if A intersects B" ) );
}
void TestQgsGeometry::intersectionCheck2()
{
  QVERIFY( !mpPolygonGeometryA->intersects( mpPolygonGeometryC ) );
}

void TestQgsGeometry::unionCheck1()
{
  // should be a multipolygon with 2 parts as A does not intersect C
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->Union( mpPolygonGeometryC );
  qDebug( "Geometry Type: " + wkbTypeAsString( mypUnionGeometry->wkbType() ).toLocal8Bit() );
  QVERIFY( mypUnionGeometry->wkbType() == QGis::WKBMultiPolygon );
  QgsMultiPolygon myMultiPolygon = mypUnionGeometry->asMultiPolygon();
  QVERIFY( myMultiPolygon.size() > 0 ); //check that the union did not fail
  dumpMultiPolygon( myMultiPolygon );
  delete mypUnionGeometry;
  QVERIFY( renderCheck( "geometry_unionCheck1", "Checking A union C produces 2 polys" ) );
}

void TestQgsGeometry::unionCheck2()
{
  // should be a single polygon as A intersect B
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->Union( mpPolygonGeometryB );
  qDebug( "Geometry Type: " + wkbTypeAsString( mypUnionGeometry->wkbType() ).toLocal8Bit() );
  QVERIFY( mypUnionGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypUnionGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  delete mypUnionGeometry;
  QVERIFY( renderCheck( "geometry_unionCheck2", "Checking A union B produces single union poly" ) );
}

void TestQgsGeometry::differenceCheck1()
{
  // should be same as A since A does not intersect C so diff is 100% of A
  QgsGeometry * mypDifferenceGeometry  =  mpPolygonGeometryA->difference( mpPolygonGeometryC );
  qDebug( "Geometry Type: " + wkbTypeAsString( mypDifferenceGeometry->wkbType() ).toLocal8Bit() );
  QVERIFY( mypDifferenceGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypDifferenceGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union did not fail
  dumpPolygon( myPolygon );
  delete mypDifferenceGeometry;
  QVERIFY( renderCheck( "geometry_differenceCheck1", "Checking (A - C) = A" ) );
}

void TestQgsGeometry::differenceCheck2()
{
  // should be a single polygon as (A - B) = subset of A
  QgsGeometry * mypDifferenceGeometry  =  mpPolygonGeometryA->difference( mpPolygonGeometryB );
  qDebug( "Geometry Type: " + wkbTypeAsString( mypDifferenceGeometry->wkbType() ).toLocal8Bit() );
  QVERIFY( mypDifferenceGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypDifferenceGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  delete mypDifferenceGeometry;
  QVERIFY( renderCheck( "geometry_differenceCheck2", "Checking (A - B) = subset of A" ) );
}
void TestQgsGeometry::bufferCheck()
{
  // should be a single polygon
  QgsGeometry * mypBufferGeometry  =  mpPolygonGeometryB->buffer( 10, 10 );
  qDebug( "Geometry Type: " + wkbTypeAsString( mypBufferGeometry->wkbType() ).toLocal8Bit() );
  QVERIFY( mypBufferGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypBufferGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the buffer created a feature
  dumpPolygon( myPolygon );
  delete mypBufferGeometry;
  QVERIFY( renderCheck( "geometry_bufferCheck", "Checking buffer(10,10) of B" ) );
}
bool TestQgsGeometry::renderCheck( QString theTestName, QString theComment )
{
  mReport += "<h2>" + theTestName + "</h2>\n";
  mReport += "<h3>" + theComment + "</h3>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir + theTestName + ".png";
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + QDir::separator();
  mImage.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setExpectedImage( myTestDataDir + "expected_" + theTestName + ".png" );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( theTestName );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsGeometry::dumpMultiPolygon( QgsMultiPolygon &theMultiPolygon )
{
  qDebug( "Multipolygon Geometry Dump" );
  for ( int i = 0; i < theMultiPolygon.size(); i++ )
  {
    QgsPolygon myPolygon = theMultiPolygon.at( i );
    qDebug( "\tPolygon in multipolygon: " + QString::number( i ).toLocal8Bit() );
    dumpPolygon( myPolygon );
  }
}

void TestQgsGeometry::dumpPolygon( QgsPolygon &thePolygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < thePolygon.size(); j++ )
  {
    QgsPolyline myPolyline = thePolygon.at( j ); //rings of polygon
    qDebug( "\t\tRing  in polygon: " + QString::number( j ).toLocal8Bit() );

    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      QgsPoint myPoint = myPolyline.at( k );
      qDebug( "\t\t\tPoint in ring " + QString::number( k ).toLocal8Bit() + " :" + myPoint.toString().toLocal8Bit() );
      myPoints << QPointF( myPoint.x(), myPoint.y() );
    }
  }
  mpPainter->drawPolygon( myPoints );
}

QString TestQgsGeometry::wkbTypeAsString( QGis::WkbType theType )
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

QTEST_MAIN( TestQgsGeometry )
#include "moc_testqgsgeometry.cxx"

