/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <iostream>

#include <QApplication>

#include <qgsvectorlayer.h> //defines QgsFieldMap 
#include <qgsvectorfilewriter.h> //logic for writing shpfiles
#include <qgsfeature.h> //we will need to pass a bunch of these for each rec
#include <qgsgeometry.h> //each feature needs a geometry
#include <qgspoint.h> //we will use point geometry
#include <qgscoordinatereferencesystem.h> //needed for creating a srs
#include <qgsapplication.h> //search path for srs.db
#include <qgsfield.h>
#include <qgis.h> //defines GEOWkt

/** \ingroup UnitTests
 * This is a unit test for the QgsVectorFileWriter class.
 *
 *  Possible QVariant::Type s
 *   QVariant::String
 *   QVariant::Int
 *   QVariant::Double
 *
 *   Allowed ogr prvider typeNames:
 *   Integer
 *   Real
 *   String
 *
 *   Constructor for QgsField:
 *   QgsField::QgsField(QString name,
 *                      QVariant::Type type,
 *                      QString typeName,
 *                      int len,
 *                      int prec,
 *                      QString comment)
 */
class TestQgsVectorFileWriter: public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorFileWriter();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    /** This method tests writing a point to a shapefile */
    void createPoint();
    /** This method tests writing a polyline to a shapefile */
    void createLine();
    /** This method tests writing a polygon to a shapefile */
    void createPolygon();
    /** This method test writing multiple features to a shapefile */
    void polygonGridTest();
    /** As above but using a projected CRS*/
    void projectedPlygonGridTest();

  private:
    // a little util fn used by all tests
    bool cleanupFile( QString theFileBase );
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError;
    QgsCoordinateReferenceSystem mCRS;
    QgsFields mFields;
    QgsPoint mPoint1;
    QgsPoint mPoint2;
    QgsPoint mPoint3;
};

TestQgsVectorFileWriter::TestQgsVectorFileWriter()
    : mError( QgsVectorFileWriter::NoError )
{

}

void TestQgsVectorFileWriter::initTestCase()
{
  qDebug( "\n\n **************\n"
          "Note: if you get a message like \n"
          "ERROR 1: /tmp/testpt.shp is not a directory.\n"
          "It is caused by the /tmp/testshp.* files already existing\n"
          "(the ERROR comes from OGR and is not very intuitive)\n"
          "******************\n" );
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::showSettings();
  //create some objects that will be used in all tests...

  mEncoding = "UTF-8";
  QgsField myField1( "Field1", QVariant::String, "String", 10, 0, "Field 1 comment" );
  mFields.append( myField1 );
  mCRS = QgsCoordinateReferenceSystem( GEOWKT );
  mPoint1 = QgsPoint( 10.0, 10.0 );
  mPoint2 = QgsPoint( 15.0, 10.0 );
  mPoint3 = QgsPoint( 15.0, 12.0 );
}


void TestQgsVectorFileWriter::createPoint()
{

  //
  // Remove old copies that may be lying around
  //
  QString myFileName = "/testpt.shp";
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBPoint,
                                &mCRS );
  //
  // Create a feature
  //
  //
  // NOTE: don't delete this pointer again -
  // ownership is passed to the feature which will
  // delete it in its dtor!
  QgsGeometry * mypPointGeometry = QgsGeometry::fromPoint( mPoint1 );
  QgsFeature myFeature;
  myFeature.setGeometry( mypPointGeometry );
  myFeature.initAttributes( 1 );
  myFeature.setAttribute( 0, "HelloWorld" );
  //
  // Write the feature to the filewriter
  // and check for errors
  //
  QVERIFY( myWriter.addFeature( myFeature ) );
  mError = myWriter.hasError();
  if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
  {
    std::cout << "Driver not found error" << std::endl;
  }
  else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
  {
    std::cout << "Create data source error" << std::endl;
  }
  else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
  {
    std::cout << "Create layer error" << std::endl;
  }
  QVERIFY( mError == QgsVectorFileWriter::NoError );
}

void TestQgsVectorFileWriter::createLine()
{
  //
  // Remove old copies that may be lying around
  //
  QString myFileName = "/testln.shp";
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBLineString,
                                &mCRS );
  //
  // Create a feature
  //
  QgsPolyline myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3;
  //
  // NOTE: don't delete this pointer again -
  // ownership is passed to the feature which will
  // delete it in its dtor!
  QgsGeometry * mypLineGeometry = QgsGeometry::fromPolyline( myPolyline );
  QgsFeature myFeature;
  myFeature.setGeometry( mypLineGeometry );
  myFeature.initAttributes( 1 );
  myFeature.setAttribute( 0, "HelloWorld" );
  //
  // Write the feature to the filewriter
  // and check for errors
  //
  QVERIFY( myWriter.addFeature( myFeature ) );
  mError = myWriter.hasError();
  if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
  {
    std::cout << "Driver not found error" << std::endl;
  }
  else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
  {
    std::cout << "Create data source error" << std::endl;
  }
  else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
  {
    std::cout << "Create layer error" << std::endl;
  }
  QVERIFY( mError == QgsVectorFileWriter::NoError );
}

void TestQgsVectorFileWriter::createPolygon()
{

  //
  // Remove old copies that may be lying around
  //
  QString myFileName = "/testply.shp";
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBPolygon,
                                &mCRS );
  //
  // Create a polygon feature
  //
  QgsPolyline myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3 << mPoint1;
  QgsPolygon myPolygon;
  myPolygon << myPolyline;
  //polygon: first item of the list is outer ring,
  // inner rings (if any) start from second item
  //
  // NOTE: don't delete this pointer again -
  // ownership is passed to the feature which will
  // delete it in its dtor!
  QgsGeometry * mypPolygonGeometry = QgsGeometry::fromPolygon( myPolygon );
  QgsFeature myFeature;
  myFeature.setGeometry( mypPolygonGeometry );
  myFeature.initAttributes( 1 );
  myFeature.setAttribute( 0, "HelloWorld" );
  //
  // Write the feature to the filewriter
  // and check for errors
  //
  QVERIFY( myWriter.addFeature( myFeature ) );
  mError = myWriter.hasError();
  if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
  {
    std::cout << "Driver not found error" << std::endl;
  }
  else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
  {
    std::cout << "Create data source error" << std::endl;
  }
  else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
  {
    std::cout << "Create layer error" << std::endl;
  }
  QVERIFY( mError == QgsVectorFileWriter::NoError );
}
void TestQgsVectorFileWriter::polygonGridTest()
{
  //
  // Remove old copies that may be lying around
  //
  QString myFileName = "/testgrid.shp";
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBPolygon,
                                &mCRS );
  double myInterval = 5.0;
  for ( double i = -180.0; i <= 180.0; i += myInterval )
  {
    for ( double j = -90.0; j <= 90.0; j += myInterval )
    {
      //
      // Create a polygon feature
      //
      QgsPolyline myPolyline;
      QgsPoint myPoint1 = QgsPoint( i, j );
      QgsPoint myPoint2 = QgsPoint( i + myInterval, j );
      QgsPoint myPoint3 = QgsPoint( i + myInterval, j + myInterval );
      QgsPoint myPoint4 = QgsPoint( i, j + myInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygon myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      // NOTE: don't delete this pointer again -
      // ownership is passed to the feature which will
      // delete it in its dtor!
      QgsGeometry * mypPolygonGeometry = QgsGeometry::fromPolygon( myPolygon );
      QgsFeature myFeature;
      myFeature.setGeometry( mypPolygonGeometry );
      myFeature.initAttributes( 1 );
      myFeature.setAttribute( 0, "HelloWorld" );
      //
      // Write the feature to the filewriter
      // and check for errors
      //
      QVERIFY( myWriter.addFeature( myFeature ) );
      mError = myWriter.hasError();
      if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
      {
        std::cout << "Driver not found error" << std::endl;
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
      {
        std::cout << "Create data source error" << std::endl;
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
      {
        std::cout << "Create layer error" << std::endl;
      }
      QVERIFY( mError == QgsVectorFileWriter::NoError );
    }
  }
}

void TestQgsVectorFileWriter::projectedPlygonGridTest()
{
  //
  // Remove old copies that may be lying around
  //
  QString myFileName = "/testprjgrid.shp";
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  //
  // We are testing projected coordinate
  // system vector writing to lets use something fun...
  // Jamaica National Grid
  // QGIS CRSID: 1286
  // PostGIS SRID: 24200
  // +proj=lcc +lat_1=18 +lat_0=18 +lon_0=-77 +k_0=1 +x_0=250000
  // +y_0=150000 +ellps=clrk66 +units=m +no_defs
  //
  mCRS = QgsCoordinateReferenceSystem( 1286, QgsCoordinateReferenceSystem::InternalCrsId );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBPolygon,
                                &mCRS );
  double myInterval = 1000.0; //1km2
  for ( double i = 0.0; i <= 10000.0; i += myInterval ) //10km
  {
    for ( double j = 0.0; j <= 10000.0; j += myInterval )//10km
    {
      //
      // Create a polygon feature
      //
      QgsPolyline myPolyline;
      QgsPoint myPoint1 = QgsPoint( i, j );
      QgsPoint myPoint2 = QgsPoint( i + myInterval, j );
      QgsPoint myPoint3 = QgsPoint( i + myInterval, j + myInterval );
      QgsPoint myPoint4 = QgsPoint( i, j + myInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygon myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      // NOTE: don't delete this pointer again -
      // ownership is passed to the feature which will
      // delete it in its dtor!
      QgsGeometry * mypPolygonGeometry = QgsGeometry::fromPolygon( myPolygon );
      QgsFeature myFeature;
      myFeature.setGeometry( mypPolygonGeometry );
      myFeature.initAttributes( 1 );
      myFeature.setAttribute( 0, "HelloWorld" );
      //
      // Write the feature to the filewriter
      // and check for errors
      //
      QVERIFY( myWriter.addFeature( myFeature ) );
      mError = myWriter.hasError();
      if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
      {
        std::cout << "Driver not found error" << std::endl;
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
      {
        std::cout << "Create data source error" << std::endl;
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
      {
        std::cout << "Create layer error" << std::endl;
      }
      QVERIFY( mError == QgsVectorFileWriter::NoError );
    }
  }
}

QTEST_MAIN( TestQgsVectorFileWriter )
#include "testqgsvectorfilewriter.moc"
