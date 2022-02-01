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
#include <QTemporaryFile>

#include "qgsvectorlayer.h" //defines QgsFieldMap
#include "qgsvectorfilewriter.h" //logic for writing shpfiles
#include "qgsfeature.h" //we will need to pass a bunch of these for each rec
#include "qgsgeometry.h" //each feature needs a geometry
#include "qgspointxy.h" //we will use point geometry
#include "qgscoordinatereferencesystem.h" //needed for creating a srs
#include "qgscoordinatetransformcontext.h"
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

    void _testExportToGpx( const QString &geomTypeName,
                           const QString &wkt,
                           const QString &expectedLayerName,
                           const QString &inputLayerName = QStringLiteral( "test" ),
                           const QStringList &layerOptions = QStringList() );

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
    //! This is a regression test ticket 1141 (broken Polish characters support since r8592) https://github.com/qgis/QGIS/issues/11201
    void regression1141();
    //! Test prepareWriteAsVectorFormat
    void prepareWriteAsVectorFormat();
    //! Test regression #21714 (Exported GeoPackages have wrong field definitions)
    void testTextFieldLength();
    //! Test export of array fields to GeoPackages
    void testExportArrayToGpkg();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxPoint();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxPointTrackPoints();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxPointRoutePoints();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxLineString();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxLineStringForceTrack();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxMultiLineString();
    //! Test https://github.com/qgis/QGIS/issues/29819
    void testExportToGpxMultiLineStringForceRoute();

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
  const QgsField myField1( QStringLiteral( "Field1" ), QVariant::String, QStringLiteral( "String" ), 10, 0, QStringLiteral( "Field 1 comment" ) );
  mFields.append( myField1 );
  mCRS = QgsCoordinateReferenceSystem( geoWkt() );
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

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = mEncoding;
  std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( myFileName, mFields, QgsWkbTypes::Point, mCRS, QgsCoordinateTransformContext(), saveOptions ) );
  //
  // Create a feature
  //
  //
  const QgsGeometry mypPointGeometry = QgsGeometry::fromPointXY( mPoint1 );
  QgsFeature myFeature;
  myFeature.setGeometry( mypPointGeometry );
  myFeature.initAttributes( 1 );
  myFeature.setAttribute( 0, "HelloWorld" );
  //
  // Write the feature to the filewriter
  // and check for errors
  //
  QVERIFY( writer->addFeature( myFeature ) );
  mError = writer->hasError();
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

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = mEncoding;
  std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( myFileName, mFields, QgsWkbTypes::LineString, mCRS, QgsCoordinateTransformContext(), saveOptions ) );
  //
  // Create a feature
  //
  QgsPolylineXY myPolyline;
  myPolyline << mPoint1 << mPoint2 << mPoint3;
  const QgsGeometry mypLineGeometry = QgsGeometry::fromPolylineXY( myPolyline );
  QgsFeature myFeature;
  myFeature.setGeometry( mypLineGeometry );
  myFeature.initAttributes( 1 );
  myFeature.setAttribute( 0, "HelloWorld" );
  //
  // Write the feature to the filewriter
  // and check for errors
  //
  QVERIFY( writer->addFeature( myFeature ) );
  mError = writer->hasError();
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

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = mEncoding;
  std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( myFileName, mFields, QgsWkbTypes::Polygon, mCRS, QgsCoordinateTransformContext(), saveOptions ) );
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
  const QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
  QgsFeature myFeature;
  myFeature.setGeometry( mypPolygonGeometry );
  myFeature.initAttributes( 1 );
  myFeature.setAttribute( 0, "HelloWorld" );
  //
  // Write the feature to the filewriter
  // and check for errors
  //
  QVERIFY( writer->addFeature( myFeature ) );
  mError = writer->hasError();
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

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = mEncoding;
  std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( myFileName, mFields, QgsWkbTypes::Polygon, mCRS, QgsCoordinateTransformContext(), saveOptions ) );
  const double myInterval = 5.0;
  for ( double i = -180.0; i <= 180.0; i += myInterval )
  {
    for ( double j = -90.0; j <= 90.0; j += myInterval )
    {
      //
      // Create a polygon feature
      //
      QgsPolylineXY myPolyline;
      const QgsPointXY myPoint1 = QgsPointXY( i, j );
      const QgsPointXY myPoint2 = QgsPointXY( i + myInterval, j );
      const QgsPointXY myPoint3 = QgsPointXY( i + myInterval, j + myInterval );
      const QgsPointXY myPoint4 = QgsPointXY( i, j + myInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygonXY myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      const QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
      QgsFeature myFeature;
      myFeature.setGeometry( mypPolygonGeometry );
      myFeature.initAttributes( 1 );
      myFeature.setAttribute( 0, "HelloWorld" );
      //
      // Write the feature to the filewriter
      // and check for errors
      //
      QVERIFY( writer->addFeature( myFeature ) );
      mError = writer->hasError();
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

  // We are testing projected coordinate
  // system vector writing...
  mCRS = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3328" ) );

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = mEncoding;
  std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( myFileName, mFields, QgsWkbTypes::Polygon, mCRS, QgsCoordinateTransformContext(), saveOptions ) );
  const double myInterval = 1000.0; //1km2
  for ( double i = 0.0; i <= 10000.0; i += myInterval ) //10km
  {
    for ( double j = 0.0; j <= 10000.0; j += myInterval )//10km
    {
      //
      // Create a polygon feature
      //
      QgsPolylineXY myPolyline;
      const QgsPointXY myPoint1 = QgsPointXY( i, j );
      const QgsPointXY myPoint2 = QgsPointXY( i + myInterval, j );
      const QgsPointXY myPoint3 = QgsPointXY( i + myInterval, j + myInterval );
      const QgsPointXY myPoint4 = QgsPointXY( i, j + myInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygonXY myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      const QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
      QgsFeature myFeature;
      myFeature.setGeometry( mypPolygonGeometry );
      myFeature.initAttributes( 1 );
      myFeature.setAttribute( 0, "HelloWorld" );
      //
      // Write the feature to the filewriter
      // and check for errors
      //
      QVERIFY( writer->addFeature( myFeature ) );
      mError = writer->hasError();
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
  QgsDebugMsg( QStringLiteral( "CODESET:%1" ).arg( cs ? cs : "unset" ) );
  if ( !cs || strcmp( cs, "UTF-8" ) != 0 )
  {
    QSKIP( "This test requires a UTF-8 locale", SkipSingle );
    return;
  }
#endif

  //create some objects that will be used in all tests...
  const QString encoding = QStringLiteral( "UTF-8" );
  const QgsField myField( QStringLiteral( "ąęćń" ), QVariant::Int, QStringLiteral( "int" ), 10, 0, QStringLiteral( "Value on lon" ) );
  QgsFields fields;
  fields.append( myField );
  QgsCoordinateReferenceSystem crs;
  crs = QgsCoordinateReferenceSystem( geoWkt() );
  const QString tmpDir = QDir::tempPath() + '/';
  const QString fileName = tmpDir +  "ąęćń.shp";

  QVERIFY2( !QFile::exists( fileName ), QString( "File %1 already exists, cannot run test" ).arg( fileName ).toLocal8Bit().constData() );

  qDebug( "Creating test dataset: " );

  {
    QgsVectorFileWriter::SaveVectorOptions saveOptions;
    saveOptions.fileEncoding = encoding;
    std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( fileName, fields, QgsWkbTypes::Point, crs, QgsCoordinateTransformContext(), saveOptions ) );

    const QgsPointXY myPoint = QgsPointXY( 10.0, 10.0 );
    const QgsGeometry mypPointGeometry = QgsGeometry::fromPointXY( myPoint );
    QgsFeature myFeature;
    myFeature.setGeometry( mypPointGeometry );
    myFeature.initAttributes( 1 );
    myFeature.setAttribute( 0, 10 );
    //
    // Write the feature to the filewriter
    // and check for errors
    //
    QVERIFY( writer->addFeature( myFeature ) );
    const QgsVectorFileWriter::WriterError error = writer->hasError();

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
  const QString fileName( tmpFile.fileName( ) );
  options.driverName = "GPKG";
  options.layerName = "test";
  QString newFilename;
  const QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormatV3(
        &ml,
        fileName,
        ml.transformContext(),
        options, nullptr,
        &newFilename ) );

  QCOMPARE( error, QgsVectorFileWriter::WriterError::NoError );
  QCOMPARE( newFilename, fileName );
  QgsVectorLayer vl( QStringLiteral( "%1|layername=test" ).arg( fileName ), "src_test", "ogr" );
  QVERIFY( vl.isValid() );
  QgsVectorFileWriter::prepareWriteAsVectorFormat( &vl, options, details );
  QCOMPARE( details.providerUriParams.value( "layerName" ).toString(), QStringLiteral( "test" ) );
  QCOMPARE( details.providerUriParams.value( "path" ).toString(), fileName );
}

void TestQgsVectorFileWriter::testTextFieldLength()
{
  QTemporaryFile tmpFile( QDir::tempPath() +  "/test_qgsvectorfilewriter2_XXXXXX.gpkg" );
  tmpFile.open();
  const QString fileName( tmpFile.fileName( ) );
  QgsVectorLayer vl( "Point?field=firstfield:string(1024)", "test", "memory" );
  QCOMPARE( vl.fields().at( 0 ).length(), 1024 );
  QgsFeature f { vl.fields() };
  f.setAttribute( 0, QString( 1024, 'x' ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "point(9 45)" ) ) );
  QVERIFY( vl.startEditing() );
  QVERIFY( vl.addFeature( f ) );
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = "GPKG";
  options.layerName = "test";
  QString newFilename;
  const QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormatV3(
        &vl,
        fileName,
        vl.transformContext(),
        options, nullptr,
        &newFilename ) );
  QCOMPARE( error, QgsVectorFileWriter::WriterError::NoError );
  QCOMPARE( newFilename, fileName );
  const QgsVectorLayer vl2( QStringLiteral( "%1|layername=test" ).arg( fileName ), "src_test", "ogr" );
  QVERIFY( vl2.isValid() );
  QCOMPARE( vl2.featureCount(), 1L );
  QCOMPARE( vl2.fields().at( 1 ).length(), 1024 );
  QCOMPARE( vl2.getFeature( 1 ).attribute( 1 ).toString(), QString( 1024, 'x' ) );

}

void TestQgsVectorFileWriter::testExportArrayToGpkg()
{
  QTemporaryFile tmpFile( QDir::tempPath() +  "/test_qgsvectorfilewriter3_XXXXXX.gpkg" );
  tmpFile.open();
  const QString fileName( tmpFile.fileName( ) );
  QgsVectorLayer vl( "Point?field=arrayfield:integerlist&field=arrayfield2:stringlist", "test", "memory" );
  QCOMPARE( vl.fields().at( 0 ).type(), QVariant::List );
  QCOMPARE( vl.fields().at( 0 ).subType(), QVariant::Int );
  QCOMPARE( vl.fields().at( 1 ).type(), QVariant::StringList );
  QCOMPARE( vl.fields().at( 1 ).subType(), QVariant::String );
  QgsFeature f { vl.fields() };
  f.setAttribute( 0, QVariantList() << 1 << 2 << 3 );
  f.setAttribute( 1, QStringList() << "a" << "b" << "c" );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "point(9 45)" ) ) );
  QVERIFY( vl.startEditing() );
  QVERIFY( vl.addFeature( f ) );
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = "GPKG";
  options.layerName = "test";
  QString newFilename;
  const QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormatV3(
        &vl,
        fileName,
        vl.transformContext(),
        options, nullptr,
        &newFilename ) );
  QCOMPARE( error, QgsVectorFileWriter::WriterError::NoError );
  QCOMPARE( newFilename, fileName );
  const QgsVectorLayer vl2( QStringLiteral( "%1|layername=test" ).arg( fileName ), "src_test", "ogr" );
  QVERIFY( vl2.isValid() );
  QCOMPARE( vl2.featureCount(), 1L );
  QCOMPARE( vl2.fields().at( 1 ).type(), QVariant::Map );
  QCOMPARE( vl2.fields().at( 1 ).subType(), QVariant::String );
  QCOMPARE( vl2.fields().at( 1 ).typeName(), QStringLiteral( "JSON" ) );
  QCOMPARE( vl2.fields().at( 2 ).type(), QVariant::Map );
  QCOMPARE( vl2.fields().at( 2 ).subType(), QVariant::String );
  QCOMPARE( vl2.fields().at( 2 ).typeName(), QStringLiteral( "JSON" ) );
  QCOMPARE( vl2.getFeature( 1 ).attribute( 1 ).toList(), QVariantList() << 1 << 2 << 3 );
  QCOMPARE( vl2.getFeature( 1 ).attribute( 2 ).toStringList(), QStringList() << "a" << "b" << "c" );
}

void TestQgsVectorFileWriter::_testExportToGpx( const QString &geomTypeName,
    const QString &wkt,
    const QString &expectedLayerName,
    const QString &inputLayerName,
    const QStringList &layerOptions )
{
  QTemporaryFile tmpFile( QDir::tempPath() +  "/test_qgsvectorfilewriter_testExportToGpx" + geomTypeName + "_XXXXXX.gpx" );
  tmpFile.open();
  const QString fileName( tmpFile.fileName( ) );
  QString memLayerDef( geomTypeName );
  if ( inputLayerName == QLatin1String( "track_points" ) )
  {
    memLayerDef += QLatin1String( "?field=track_fid:int&field=track_seg_id:int" );
  }
  else if ( inputLayerName == QLatin1String( "route_points" ) )
  {
    memLayerDef += QLatin1String( "?field=route_fid:int" );
  }
  QgsVectorLayer vl( memLayerDef, "test", "memory" );
  QgsFeature f { vl.fields() };
  if ( inputLayerName == QLatin1String( "track_points" ) )
  {
    f.setAttribute( 0, 1 );
    f.setAttribute( 1, 1 );
  }
  else if ( inputLayerName == QLatin1String( "route_points" ) )
  {
    f.setAttribute( 0, 1 );
  }
  f.setGeometry( QgsGeometry::fromWkt( wkt ) );
  QVERIFY( vl.startEditing() );
  QVERIFY( vl.addFeature( f ) );
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = "GPX";
  options.layerName = inputLayerName;
  options.layerOptions = layerOptions;
  QString outLayerName;
  const QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormatV3(
        &vl,
        fileName,
        vl.transformContext(),
        options, nullptr,
        nullptr, // newFilename
        &outLayerName ) );
  QCOMPARE( error, QgsVectorFileWriter::WriterError::NoError );
  QCOMPARE( outLayerName, expectedLayerName );
  const QgsVectorLayer vl2( QStringLiteral( "%1|layername=%2" ).arg( fileName ).arg( outLayerName ), "src_test", "ogr" );
  QVERIFY( vl2.isValid() );
  QCOMPARE( vl2.featureCount(), 1L );
}

void TestQgsVectorFileWriter::testExportToGpxPoint()
{
  _testExportToGpx( QStringLiteral( "Point" ),
                    QStringLiteral( "point(9 45)" ),
                    QStringLiteral( "waypoints" ) );
}

void TestQgsVectorFileWriter::testExportToGpxPointTrackPoints()
{
  _testExportToGpx( QStringLiteral( "Point" ),
                    QStringLiteral( "point(9 45)" ),
                    QStringLiteral( "track_points" ),
                    QStringLiteral( "track_points" ) );
}

void TestQgsVectorFileWriter::testExportToGpxPointRoutePoints()
{
  _testExportToGpx( QStringLiteral( "Point" ),
                    QStringLiteral( "point(9 45)" ),
                    QStringLiteral( "route_points" ),
                    QStringLiteral( "route_points" ) );
}

void TestQgsVectorFileWriter::testExportToGpxLineString()
{
  _testExportToGpx( QStringLiteral( "LineString" ),
                    QStringLiteral( "linestring(9 45,10 46)" ),
                    QStringLiteral( "routes" ) );
}

void TestQgsVectorFileWriter::testExportToGpxLineStringForceTrack()
{
  _testExportToGpx( QStringLiteral( "LineString" ),
                    QStringLiteral( "linestring(9 45,10 46)" ),
                    QStringLiteral( "tracks" ),
                    QStringLiteral( "test" ),
                    QStringList() << QStringLiteral( "FORCE_GPX_TRACK=YES" ) );
}

void TestQgsVectorFileWriter::testExportToGpxMultiLineString()
{
  _testExportToGpx( QStringLiteral( "MultiLineString" ),
                    QStringLiteral( "multilinestring((9 45,10 46))" ),
                    QStringLiteral( "tracks" ) );
}

void TestQgsVectorFileWriter::testExportToGpxMultiLineStringForceRoute()
{
  _testExportToGpx( QStringLiteral( "MultiLineString" ),
                    QStringLiteral( "multilinestring((9 45,10 46))" ),
                    QStringLiteral( "routes" ),
                    QStringLiteral( "test" ),
                    QStringList() << QStringLiteral( "FORCE_GPX_ROUTE=YES" ) );
}

QGSTEST_MAIN( TestQgsVectorFileWriter )
#include "testqgsvectorfilewriter.moc"
