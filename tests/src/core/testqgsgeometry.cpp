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
    QString getQgisPath(); // Gets the path to QGIS installation
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void intersectionCheck();
    void unionCheck();
  private:
    QgsPoint mPoint1; /*     +1  +A          */
    QgsPoint mPoint2; /*    / \ / \          */
    QgsPoint mPoint3; /*   /   X   \         */
    QgsPoint mPointA; /* 2+---/-+3  \        */
    QgsPoint mPointB; /*    B+-------+C      */
    QgsPoint mPointC; /*                     */
    QgsPoint mPointD; /*         +D          */
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

QString TestQgsGeometry::getQgisPath()
{
#ifdef Q_OS_LINUX 
  QString qgisPath = QCoreApplication::applicationDirPath () + "/../";
#else //mac and win
  QString qgisPath = QCoreApplication::applicationDirPath () ;
#endif
  return qgisPath;
}

void TestQgsGeometry::init()
{
  //
  // Reset / reinitialise the geometries before each test is run
  //
  mPoint1 = QgsPoint(20.0,10.0); /*     +1  +A          */
  mPoint2 = QgsPoint(10.0,30.0); /*    / \ / \          */
  mPoint3 = QgsPoint(30.0,30.0); /*   /   X   \         */
  mPointA = QgsPoint(40.0,10.0); /* 2+---/-+3  \        */
  mPointB = QgsPoint(20.0,40.0); /*    B+-------+C      */
  mPointC = QgsPoint(50.0,40.0); /*                     */
  mPointD = QgsPoint(20.0,60.0); /*                     */

  mPolylineA << mPoint1 << mPoint2 << mPoint3 << mPoint1;
  mPolygonA << mPolylineA;
  //Polygon B intersects Polygon A
  mPolylineB << mPointA << mPointB << mPointC << mPointA;
  mPolygonB << mPolylineB;
  // Polygon C should intersect no other polys
  mPolylineC << mPointD << mPointB << mPointC << mPointD;
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
  //QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(getQgisPath(), TRUE);
#ifdef Q_OS_LINUX
//  QgsApplication::setPkgDataPath(qgisPath + "/../share/qgis");
//  QgsApplication::setPluginPath(qgisPath + "/../lib/qgis");
#endif

  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;
  
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

void TestQgsGeometry::unionCheck()
{

  // should be no union as A does not intersect C
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->Union(mpPolygonGeometryC);
  QgsPolyline myPolyline = mypUnionGeometry->asPolyline();
  QVERIFY (myPolyline.size() == 0); //check that the union failed properly
  // should be a union as A intersect B
  mypUnionGeometry  =  mpPolygonGeometryA->Union(mpPolygonGeometryB);
  myPolyline = mypUnionGeometry->asPolyline();
  QVERIFY (myPolyline.size() > 0); //check that the union created a feature
  for (int i = 0; i < myPolyline.size(); i++)
  {
    QgsPoint myPoint = myPolyline.at(i);
    qDebug(myPoint.stringRep().toLocal8Bit());
  }
  delete mypUnionGeometry;
}

QTEST_MAIN(TestQgsGeometry)
#include "moc_testqgsgeometry.cxx"

