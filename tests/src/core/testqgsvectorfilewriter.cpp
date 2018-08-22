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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>

#include "qgsvectorlayer.h" //defines QgsFieldMap
#include "qgsvectorfilewriter.h" //logic for writing shpfiles
#include "qgsfeature.h" //we will need to pass a bunch of these for each rec
#include "qgsgeometry.h" //each feature needs a geometry
#include "qgspointxy.h" //we will use point geometry
#include "qgscoordinatereferencesystem.h" //needed for creating a srs
#include "qgsapplication.h" //search path for srs.db
#include "qgslogger.h"
#include "qgsfield.h"
#include "qgis.h" //defines GEOWkt

#if defined(linux)
#include <langinfo.h>
#endif

/**
 * \ingroup UnitTests
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
    void cleanupTestCase();// will be called after the last testfunction was executed.

    //! This method tests writing a point to a shapefile
    void createPoint();
    //! This method tests writing a polyline to a shapefile
    void createLine();
    //! This method tests writing a polygon to a shapefile
    void createPolygon();
    //! This method test writing multiple features to a shapefile
    void polygonGridTest();
    //! As above but using a projected CRS
    void projectedPlygonGridTest();
    //! This is a regression test ticket 1141 (broken Polish characters support since r8592) https://issues.qgis.org/issues/1141
    void regression1141();
    //! Test prepareWriteAsVectorFormat
    void prepareWriteAsVectorFormat();

  private:
    // a little util fn used by all tests
    bool cleanupFile( QString fileBase );
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError =  QgsVectorFileWriter::NoError ;
    QgsCoordinateReferenceSystem mCRS;
    QgsFields mFields;
    QgsPointXY mPoint1;
    QgsPointXY mPoint2;
    QgsPointXY mPoint3;
};

TestQgsVectorFileWriter::TestQgsVectorFileWriter() = default;

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
  QgsApplication::initQgis();
  //create some objects that will be used in all tests...

  mEncoding = QStringLiteral( "UTF-8" );
  QgsField myField1( QStringLiteral( "Field1" ), QVariant::String, QStringLiteral( "String" ), 10, 0, QStringLiteral( "Field 1 comment" ) );
  mFields.append( myField1 );
  mCRS = QgsCoordinateReferenceSystem( GEOWKT );
  mPoint1 = QgsPointXY( 10.0, 10.0 );
  mPoint2 = QgsPointXY( 15.0, 10.0 );
  mPoint3 = QgsPointXY( 15.0, 12.0 );
}

void TestQgsVectorFileWriter::cleanupTestCase()
{
  // Runs after all tests are done
  QgsApplication::exitQgis();
}


void TestQgsVectorFileWriter::createPoint()
{

  //
  // Remove old copies that may be lying around
  //
  QString myFileName = QStringLiteral( "/testpt.shp" );
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QgsWkbTypes::Point,
                                mCRS );
  //
  // Create a feature
  //
  //
  QgsGeometry mypPointGeometry = QgsGeometry::fromPointXY( mPoint1 );
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
  QString myFileName = QStringLiteral( "/testln.shp" );
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QgsWkbTypes::LineString,
                                mCRS );
  //
  // Create a feature
  //
  QgsPolylineXY myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3;
  QgsGeometry mypLineGeometry = QgsGeometry::fromPolylineXY( myPolyline );
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
  QString myFileName = QStringLiteral( "/testply.shp" );
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QgsWkbTypes::Polygon,
                                mCRS );
  //
  // Create a polygon feature
  //
  QgsPolylineXY myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3 << mPoint1;
  QgsPolygonXY myPolygon;
  myPolygon << myPolyline;
  //polygon: first item of the list is outer ring,
  // inner rings (if any) start from second item
  //
  QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
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
  QString myFileName = QStringLiteral( "/testgrid.shp" );
  myFileName = QDir::tempPath() + myFileName;
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( myFileName ) );
  QgsVectorFileWriter myWriter( myFileName,
                                mEncoding,
                                mFields,
                                QgsWkbTypes::Polygon,
                                mCRS );
  double myInterval = 5.0;
  for ( double i = -180.0; i <= 180.0; i += myInterval )
  {
    for ( double j = -90.0; j <= 90.0; j += myInterval )
    {
      //
      // Create a polygon feature
      //
      QgsPolylineXY myPolyline;
      QgsPointXY myPoint1 = QgsPointXY( i, j );
      QgsPointXY myPoint2 = QgsPointXY( i + myInterval, j );
      QgsPointXY myPoint3 = QgsPointXY( i + myInterval, j + myInterval );
      QgsPointXY myPoint4 = QgsPointXY( i, j + myInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygonXY myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
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
  QString myFileName = QStringLiteral( "/testprjgrid.shp" );
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
                                QgsWkbTypes::Polygon,
                                mCRS );
  double myInterval = 1000.0; //1km2
  for ( double i = 0.0; i <= 10000.0; i += myInterval ) //10km
  {
    for ( double j = 0.0; j <= 10000.0; j += myInterval )//10km
    {
      //
      // Create a polygon feature
      //
      QgsPolylineXY myPolyline;
      QgsPointXY myPoint1 = QgsPointXY( i, j );
      QgsPointXY myPoint2 = QgsPointXY( i + myInterval, j );
      QgsPointXY myPoint3 = QgsPointXY( i + myInterval, j + myInterval );
      QgsPointXY myPoint4 = QgsPointXY( i, j + myInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygonXY myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
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

void TestQgsVectorFileWriter::regression1141()
{
#if defined(linux)
  const char *cs = nl_langinfo( CODESET );
  QgsDebugMsg( QString( "CODESET:%1" ).arg( cs ? cs : "unset" ) );
  if ( !cs || strcmp( cs, "UTF-8" ) != 0 )
  {
    QSKIP( "This test requires a UTF-8 locale", SkipSingle );
    return;
  }
#endif

  //create some objects that will be used in all tests...
  QString encoding = QStringLiteral( "UTF-8" );
  QgsField myField( QStringLiteral( "ąęćń" ), QVariant::Int, QStringLiteral( "int" ), 10, 0, QStringLiteral( "Value on lon" ) );
  QgsFields fields;
  fields.append( myField );
  QgsCoordinateReferenceSystem crs;
  crs = QgsCoordinateReferenceSystem( GEOWKT );
  QString tmpDir = QDir::tempPath() + '/';
  QString fileName = tmpDir +  "ąęćń.shp";

  QVERIFY2( !QFile::exists( fileName ), QString( "File %1 already exists, cannot run test" ).arg( fileName ).toLocal8Bit().constData() );

  qDebug( "Creating test dataset: " );

  {
    QgsVectorFileWriter myWriter( fileName,
                                  encoding,
                                  fields,
                                  QgsWkbTypes::Point,
                                  crs );

    QgsPointXY myPoint = QgsPointXY( 10.0, 10.0 );
    QgsGeometry mypPointGeometry = QgsGeometry::fromPointXY( myPoint );
    QgsFeature myFeature;
    myFeature.setGeometry( mypPointGeometry );
    myFeature.initAttributes( 1 );
    myFeature.setAttribute( 0, 10 );
    //
    // Write the feature to the filewriter
    // and check for errors
    //
    QVERIFY( myWriter.addFeature( myFeature ) );
    QgsVectorFileWriter::WriterError error = myWriter.hasError();

    if ( error == QgsVectorFileWriter::ErrDriverNotFound )
    {
      std::cout << "Driver not found error" << std::endl;
    }
    else if ( error == QgsVectorFileWriter::ErrCreateDataSource )
    {
      std::cout << "Create data source error" << std::endl;
    }
    else if ( error == QgsVectorFileWriter::ErrCreateLayer )
    {
      std::cout << "Create layer error" << std::endl;
    }

    QVERIFY( error == QgsVectorFileWriter::NoError );
  }

  // Now check we can delete it again OK
  QVERIFY( QgsVectorFileWriter::deleteShapeFile( fileName ) );
}

void TestQgsVectorFileWriter::prepareWriteAsVectorFormat()
{
  QgsVectorFileWriter::PreparedWriterDetails details;
  QgsVectorFileWriter::SaveVectorOptions options;
  QgsVectorLayer ml( "Point?field=firstfield:int&field=secondfield:int", "test", "memory" );
  QgsFeature ft( ml.fields( ) );
  ft.setAttribute( 0, 4 );
  ft.setAttribute( 1, -10 );
  ml.dataProvider()->addFeature( ft );
  QVERIFY( ml.isValid() );
  QTemporaryFile tmpFile( QDir::tempPath() +  "/test_qgsvectorfilewriter_XXXXXX.gpkg" );
  tmpFile.open();
  QString fileName( tmpFile.fileName( ) );
  options.driverName = "GPKG";
  options.layerName = "test";
  QString errorMessage;
  QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormat(
      &ml,
      fileName,
      options,
      &errorMessage ) );

  QCOMPARE( error, QgsVectorFileWriter::WriterError::NoError );
  QCOMPARE( errorMessage, fileName );
  QgsVectorLayer vl( QStringLiteral( "%1|layername=test" ).arg( fileName ), "src_test", "ogr" );
  QVERIFY( vl.isValid() );
  QgsVectorFileWriter::prepareWriteAsVectorFormat( &vl, options, details );
  QCOMPARE( details.providerUriParams.value( "layerName" ), QStringLiteral( "test" ) );
  QCOMPARE( details.providerUriParams.value( "path" ), fileName );
}

QGSTEST_MAIN( TestQgsVectorFileWriter )
#include "testqgsvectorfilewriter.moc"
