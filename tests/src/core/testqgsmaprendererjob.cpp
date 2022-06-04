/***************************************************************************
     testqgsmaprendererjob.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim @ linfiniti.com
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
#include <QPainter>
#include <QTime>
#include <QApplication>
#include <QDesktopServices>

#include "qgsvectorlayer.h"
#include "qgsvectorfilewriter.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgis.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsnullpainterdevice.h"
#include "qgsmaplayer.h"
#include "qgsreadwritecontext.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsmaprendererstagedrenderjob.h"
#include "qgsmultirenderchecker.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsfontutils.h"
#include "qgsrasterlayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgslinesymbol.h"
#include "qgslabelsink.h"

//qgs unit test utility class
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsMapRendererJob class.
 * It will do some performance testing too
 *
 */
class TestQgsMapRendererJob : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapRendererJob() = default;

    ~TestQgsMapRendererJob() override
    {
      delete mMapSettings;
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    //! This method tests render performance
    void performanceTest();

    /**
     * This unit test checks if rendering of adjacent tiles (e.g. to render images for tile caches)
     * does not result in border effects
     */
    void testFourAdjacentTiles_data();
    void testFourAdjacentTiles();

    void testRenderedFeatureHandlers();
    void stagedRenderer();
    void stagedRendererWithStagedLabeling();

    void vectorLayerBoundsWithReprojection();

    void temporalRender();

    void labelSink();
    void skipSymbolRendering();

    void customNullPainterJob();

  private:
    bool imageCheck( const QString &type, const QImage &image, int mismatchCount = 0 );

    QString mEncoding;
    QgsVectorFileWriter::WriterError mError =  QgsVectorFileWriter::NoError ;
    QgsCoordinateReferenceSystem mCRS;
    QgsFields mFields;
    QgsMapSettings *mMapSettings = nullptr;
    QgsMapLayer *mpPolysLayer = nullptr;
    QString mReport;
};


void TestQgsMapRendererJob::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  mMapSettings = new QgsMapSettings();

  //create some objects that will be used in all tests...
  mEncoding = QStringLiteral( "UTF-8" );
  QgsField myField1( QStringLiteral( "Value" ), QVariant::Int, QStringLiteral( "int" ), 10, 0, QStringLiteral( "Value on lon" ) );
  mFields.append( myField1 );
  mCRS = QgsCoordinateReferenceSystem( geoWkt() );
  //
  // Create the test dataset if it doesn't exist
  //
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + '/';
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir +  "maprender_testdata.gpkg";
  //copy over the default qml for our generated layer
  QString myQmlFileName = myTestDataDir +  "maprender_testdata.qml";
  QFile::remove( myTmpDir + "maprender_testdata.qml" );
  QVERIFY( QFile::copy( myQmlFileName, myTmpDir + "maprender_testdata.qml" ) );
  qDebug( "Checking test dataset exists...\n%s", myFileName.toLocal8Bit().constData() );
  if ( !QFile::exists( myFileName ) )
  {
    qDebug( "Creating test dataset: " );

    QgsVectorFileWriter::SaveVectorOptions saveOptions;
    saveOptions.fileEncoding = mEncoding;
    std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( myFileName, mFields, QgsWkbTypes::Polygon, mCRS, QgsCoordinateTransformContext(), saveOptions ) );
    double myInterval = 0.5;
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
        // NOTE: don't delete this pointer again -
        // ownership is passed to the feature which will
        // delete it in its dtor!
        QgsGeometry mypPolygonGeometry = QgsGeometry::fromPolygonXY( myPolygon );
        QgsFeature myFeature;
        myFeature.setGeometry( mypPolygonGeometry );
        myFeature.initAttributes( 1 );
        myFeature.setAttribute( 0, i );
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
  } //file exists
  //
  //create a poly layer that will be used in all tests...
  //
  QFileInfo myPolyFileInfo( myFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  QVERIFY( mpPolysLayer->isValid() );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  // add the test layer to the maprender
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Map Render Tests</h1>\n" );
}

void TestQgsMapRendererJob::cleanupTestCase()
{
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
}

void TestQgsMapRendererJob::performanceTest()
{
  mMapSettings->setExtent( mpPolysLayer->extent() );
  QgsRenderChecker myChecker;
  myChecker.setControlName( QStringLiteral( "expected_maprender" ) );
  mMapSettings->setFlag( Qgis::MapSettingsFlag::Antialiasing );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 5 );
  bool myResultFlag = myChecker.runTest( QStringLiteral( "maprender" ) );
  mReport += myChecker.report();
  QVERIFY( myResultFlag );
}

void TestQgsMapRendererJob::testFourAdjacentTiles_data()
{
  QTest::addColumn<QStringList>( "bboxList" );
  QTest::addColumn<QString>( "controlName" );
  QTest::addColumn<QString>( "shapeFile" );
  QTest::addColumn<QString>( "qmlFile" );

  QString shapeFile = TEST_DATA_DIR + QStringLiteral( "/france_parts.shp" );
  QString qmlFile = TEST_DATA_DIR + QStringLiteral( "/adjacent_tiles/line_pattern_30_degree.qml" );
  QString controlName = QStringLiteral( "expected_adjacent_line_fill" );

  QStringList bboxList1;
  bboxList1 << QStringLiteral( "-1.5,48,-0.5,49" );
  bboxList1 << QStringLiteral( "-0.5,48,0.5,49" );
  bboxList1 << QStringLiteral( "-1.5,47,-0.5,48" );
  bboxList1 << QStringLiteral( "-0.5,47,0.5,48" );

  QTest::newRow( "adjacent_line_fill" ) << bboxList1 << controlName << shapeFile << qmlFile;

  qmlFile = TEST_DATA_DIR + QStringLiteral( "/adjacent_tiles/point_pattern_simple_marker.qml" );
  controlName = QStringLiteral( "expected_adjacent_marker_fill" );

  QTest::newRow( "adjacent_marker_fill" ) << bboxList1 << controlName << shapeFile << qmlFile;

  shapeFile = TEST_DATA_DIR + QStringLiteral( "/lines.shp" );
  qmlFile = TEST_DATA_DIR + QStringLiteral( "/adjacent_tiles/simple_line_dashed.qml" );
  controlName = QStringLiteral( "expected_adjacent_dashed_line" );

  QStringList bboxList2;
  bboxList2 << QStringLiteral( "-105,35,-95,45" );
  bboxList2 << QStringLiteral( "-95,35,-85,45" );
  bboxList2 << QStringLiteral( "-105,25,-95,35" );
  bboxList2 << QStringLiteral( "-95,25,-85,35" );

  QTest::newRow( "adjacent_dashed_line" ) << bboxList2 << controlName << shapeFile << qmlFile;
}

void TestQgsMapRendererJob::testFourAdjacentTiles()
{
  QFETCH( QStringList, bboxList );
  QFETCH( QString, controlName );
  QFETCH( QString, shapeFile );
  QFETCH( QString, qmlFile );

  QVERIFY( bboxList.size() == 4 );

  //create maplayer, set QML and add to maplayer registry
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( shapeFile, QStringLiteral( "testshape" ), QStringLiteral( "ogr" ) );

  //todo: read QML
  QFile symbologyFile( qmlFile );
  if ( !symbologyFile.open( QIODevice::ReadOnly ) )
  {
    QFAIL( "Open symbology file failed" );
  }

  QDomDocument qmlDoc;
  if ( !qmlDoc.setContent( &symbologyFile ) )
  {
    QFAIL( "QML file not valid" );
  }

  QString errorMsg;
  QgsReadWriteContext context = QgsReadWriteContext();
  if ( !vectorLayer->readSymbology( qmlDoc.documentElement(), errorMsg, context ) )
  {
    QFAIL( errorMsg.toLocal8Bit().data() );
  }

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << vectorLayer );

  QImage globalImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  globalImage.fill( Qt::white );
  QPainter globalPainter( &globalImage );

  for ( int i = 0; i < 4; ++i )
  {
    QgsMapSettings mapSettings;

    //extent
    QStringList rectCoords = bboxList.at( i ).split( ',' );
    if ( rectCoords.size() != 4 )
    {
      QFAIL( "bbox string invalid" );
    }
    QgsRectangle rect( rectCoords[0].toDouble(), rectCoords[1].toDouble(), rectCoords[2].toDouble(), rectCoords[3].toDouble() );
    mapSettings.setExtent( rect );
    mapSettings.setOutputSize( QSize( 256, 256 ) );
    mapSettings.setLayers( QList<QgsMapLayer *>() << vectorLayer );
    mapSettings.setFlags( Qgis::MapSettingsFlag::RenderMapTile );
    mapSettings.setOutputDpi( 96 );

    QgsMapRendererSequentialJob renderJob( mapSettings );
    renderJob.start();
    renderJob.waitForFinished();
    QImage img = renderJob.renderedImage();
    int globalImageX = ( i % 2 ) * 256;
    int globalImageY = ( i < 2 ) ? 0 : 256;
    globalPainter.drawImage( globalImageX, globalImageY, img );
  }

  QgsProject::instance()->removeMapLayers( QStringList() << vectorLayer->id() );

  QString renderedImagePath = QDir::tempPath() + "/" + QTest::currentDataTag() + QStringLiteral( ".png" );
  globalImage.save( renderedImagePath );

  QgsRenderChecker checker;

  checker.setControlPathPrefix( QStringLiteral( "adjacent_tiles" ) );
  checker.setControlName( controlName );
  bool result = checker.compareImages( QTest::currentDataTag(), 100, renderedImagePath );
  mReport += checker.report();
  QVERIFY( result );
}


class TestHandler : public QgsRenderedFeatureHandlerInterface
{
  public:

    TestHandler( QList< QgsFeature > &features, QList< QgsGeometry > &geometries, bool allAttributes = false )
      : features( features )
      , geometries( geometries )
      , mAllAttributes( allAttributes )
    {}

    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &geom, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext & ) override
    {
      features.append( feature );
      geometries.append( geom );
    }

    QSet< QString > usedAttributes( QgsVectorLayer *, const QgsRenderContext & ) const override
    {
      if ( !mAllAttributes )
        return QSet< QString >();
      else
        return QSet< QString >() << QgsFeatureRequest::ALL_ATTRIBUTES;
    }

    QList< QgsFeature > &features;
    QList< QgsGeometry > &geometries;

    bool mAllAttributes = false;

};


void TestQgsMapRendererJob::testRenderedFeatureHandlers()
{
  std::unique_ptr< QgsVectorLayer > pointsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );
  std::unique_ptr< QgsVectorLayer > linesLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  std::unique_ptr< QgsVectorLayer > polygonsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/polys.shp" ),
      QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QVERIFY( polygonsLayer->isValid() );

  QgsMapSettings mapSettings;
  mapSettings.setExtent( linesLayer->extent() );
  mapSettings.setDestinationCrs( linesLayer->crs() );
  mapSettings.setOutputSize( QSize( 256, 256 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << pointsLayer.get() << linesLayer.get() << polygonsLayer.get() );
  mapSettings.setFlags( Qgis::MapSettingsFlag::RenderMapTile );
  mapSettings.setOutputDpi( 96 );

  QList< QgsFeature > features1;
  QList< QgsGeometry > geometries1;
  TestHandler handler1( features1, geometries1 );
  QList< QgsFeature > features2;
  QList< QgsGeometry > geometries2;
  TestHandler handler2( features2, geometries2 );
  mapSettings.addRenderedFeatureHandler( &handler1 );
  mapSettings.addRenderedFeatureHandler( &handler2 );

  QgsMapRendererSequentialJob renderJob( mapSettings );
  renderJob.start();
  renderJob.waitForFinished();

  QCOMPARE( features1.count(), 33 );
  QCOMPARE( geometries1.count(), 33 );
  QCOMPARE( features2.count(), 33 );
  QCOMPARE( geometries2.count(), 33 );
  features1.clear();
  geometries1.clear();
  features2.clear();
  geometries2.clear();

  polygonsLayer->setSubsetString( QStringLiteral( "value=13" ) );
  pointsLayer->setSubsetString( QStringLiteral( "importance<=3 and \"Cabin Crew\">=1 and \"Cabin Crew\"<=2 and \"heading\" > 90 and \"heading\" < 290" ) );
  linesLayer->setSubsetString( QStringLiteral( "name='Highway'" ) );

  QgsMapRendererSequentialJob renderJob2( mapSettings );
  renderJob2.start();
  renderJob2.waitForFinished();

  QCOMPARE( features1.count(), 5 );
  QCOMPARE( geometries1.count(), 5 );
  QCOMPARE( features2.count(), 5 );
  QCOMPARE( geometries2.count(), 5 );

  QStringList attributes;
  for ( const QgsFeature &f : std::as_const( features1 ) )
  {
    QStringList bits;
    for ( const QVariant &v : f.attributes() )
      bits << v.toString();
    attributes << bits.join( ',' );
  }
  attributes.sort();
  QCOMPARE( attributes.at( 0 ), QStringLiteral( "Biplane,,,,," ) );
  QCOMPARE( attributes.at( 1 ), QStringLiteral( "Dam," ) );
  QCOMPARE( attributes.at( 2 ), QStringLiteral( "Highway," ) );
  QCOMPARE( attributes.at( 3 ), QStringLiteral( "Highway," ) );
  QCOMPARE( attributes.at( 4 ), QStringLiteral( "Jet,,,,," ) );

  QStringList wkts;
  for ( const QgsGeometry &g : std::as_const( geometries1 ) )
  {
    QgsDebugMsg( g.asWkt( 1 ) );
    wkts << g.asWkt( 1 );
  }
  wkts.sort();
  QCOMPARE( wkts.at( 0 ), QStringLiteral( "LineString (0 124.4, 0.3 124.4, 4 123.4, 5.5 121.9, 7.3 119.5, 8.2 118.9, 10.6 116.4, 11.2 115.8, 15.8 113.1, 20.4 111, 24 110.1, 30.7 108.5, 32.8 108.5, 36.5 108.5, 42.6 109.1, 44.4 109.5, 47.4 110.4, 49.9 110.4, 54.1 111.9, 55 112.5, 57.2 113.1, 58.4 113.7, 60.8 114.9, 62 115.5, 63.8 116.4, 64.8 116.8, 66.3 117.4, 72.4 119.2, 80 119.5, 83.9 120.7, 85.7 121.3, 87.3 121.6, 89.4 121.6, 91.8 121.6, 93.6 121.9, 95.5 122.2, 99.7 122.2, 100.3 122.2, 103.7 122.8, 106.4 122.8, 109.8 122.5, 112.8 122.5, 117.1 122.5, 121 121, 125.6 119.5, 127.4 118.3, 134.7 118, 144.7 120.4, 148.4 121.3, 156 121.9, 159.6 121.9, 168.4 119.2, 173.6 116.4, 179.1 112.5, 183.9 110.1, 188.5 107.6, 197.3 104.9, 197.3 104.9, 198.5 104.6, 204 105.2, 206.4 106.4, 211.3 107.6, 216.2 109.5, 218 110.1, 221 111, 229.5 112.2, 230.8 112.2, 243.2 111.6, 243.5 111, 251.4 107.3, 253.3 106.7)" ) );
  QCOMPARE( wkts.at( 1 ), QStringLiteral( "LineString (93.3 45.3, 94.6 48.3, 95.2 49.6, 95.8 50.5, 97.9 52.3, 104 55.3, 110.4 58.1, 112.2 59.6, 115.2 62.6, 115.5 63.2, 116.1 66.9, 116.4 68.7, 117.4 71.4, 117.7 72.7, 118 75.7, 118.6 76.6, 119.5 78.4, 120.4 80.3, 122.5 82.7, 122.8 83.3, 123.1 85.1, 123.1 87.9, 123.1 89.1, 123.7 91.5, 124.4 92.7, 126.8 94.6, 128.9 95.8, 131 97, 133.2 97.9, 133.5 99.7, 133.5 100.9, 132.3 103.1, 131.6 103.7, 130.7 105.5, 129.8 106.7, 128.9 107.6, 128.6 108.2, 128.3 109.8, 128 111, 128 112.2, 128.9 114.3, 129.5 115.5, 129.5 115.5, 132.6 122.2, 131 125.3, 129.8 126.5, 127.7 130.1, 127.7 132.6, 125 135, 124 136.5, 122.5 138.9, 122.5 140.2, 122.5 141.7, 122.5 142.9, 122.5 144.4, 123.4 145.6, 123.7 147.2, 124.4 148.4, 125.6 149.6, 126.8 151.4, 128.6 153.5, 131.6 159.3, 132 159.6, 134.1 161.7, 135 162.7, 137.1 165.7, 138.6 166.9, 141.4 168.7, 143.2 170.3, 145.3 172.4, 146.9 173.3, 149 175.1, 151.7 176.6, 153.8 178.2, 157.2 180.6, 158.4 181.8, 159.3 183.9, 159.3 185.8, 159.3 187.6, 159 188.2, 157.8 190, 156 192.8, 154.8 194, 154.5 195.2, 154.5 196.7, 154.8 197.9, 155.7 200.4, 156 201.6, 157.8 203.7, 158.7 204.6, 159.9 206.4, 161.7 209.2, 166.6 211.3)" ) );
  QCOMPARE( wkts.at( 2 ), QStringLiteral( "Polygon ((122.4 105.6, 165.2 105.6, 165.2 148.4, 122.4 148.4, 122.4 105.6))" ) );
  QCOMPARE( wkts.at( 3 ), QStringLiteral( "Polygon ((26.5 151.7, 31.3 153.8, 32.2 155.7, 32.8 156.3, 38 156.6, 41.3 156.6, 43.8 156.6, 48.3 157.5, 50.8 158.7, 52.9 160.8, 55 165.1, 55 165.1, 52.6 180.6, 52.6 182.7, 52.3 186.4, 50.2 190.3, 47.7 192.8, 45.9 194, 45 194.3, 30.4 193.7, 25.8 192.8, 20.4 190.9, 15.8 190, 13.4 188.8, 12.8 188.5, 11.6 187.9, 9.7 186.1, 8.8 184.9, 7.9 183.6, 7 181.8, 6.7 180.3, 7 177.9, 7.6 176.3, 8.5 173.9, 8.5 172.4, 7.9 170.6, 6.7 170, 4 168.4, -0.3 165.7, -1.8 163, -1.8 159.6, -1.5 156.6, 0 152.3, 7.6 147.5, 18.9 148.4, 25.2 150.5, 26.5 151.7))" ) );
  QCOMPARE( wkts.at( 4 ), QStringLiteral( "Polygon ((4.2 59.5, 73.5 59.5, 73.5 128.8, 4.2 128.8, 4.2 59.5))" ) );

  // now, use a handler which requires all attributes to be fetched
  QList< QgsFeature > features3;
  QList< QgsGeometry > geometries3;
  TestHandler handler3( features3, geometries3, true );
  mapSettings.addRenderedFeatureHandler( &handler3 );

  QgsMapRendererSequentialJob renderJob3( mapSettings );
  renderJob3.start();
  renderJob3.waitForFinished();

  QCOMPARE( features3.count(), 5 );
  QCOMPARE( geometries3.count(), 5 );
  attributes.clear();
  for ( const QgsFeature &f : std::as_const( features3 ) )
  {
    QStringList bits;
    for ( const QVariant &v : f.attributes() )
      bits << v.toString();
    attributes << bits.join( ',' );
  }
  attributes.sort();
  QCOMPARE( attributes.at( 0 ), QStringLiteral( "Biplane,240,1,3,2,5" ) );
  QCOMPARE( attributes.at( 1 ), QStringLiteral( "Dam,13" ) );
  QCOMPARE( attributes.at( 2 ), QStringLiteral( "Highway,1" ) );
  QCOMPARE( attributes.at( 3 ), QStringLiteral( "Highway,1" ) );
  QCOMPARE( attributes.at( 4 ), QStringLiteral( "Jet,95,3,1,1,2" ) );
}

void TestQgsMapRendererJob::stagedRenderer()
{
  // test the staged map renderer job subclass

  std::unique_ptr< QgsVectorLayer > pointsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );
  std::unique_ptr< QgsVectorLayer > linesLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  std::unique_ptr< QgsVectorLayer > polygonsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/polys.shp" ),
      QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QVERIFY( polygonsLayer->isValid() );
  std::unique_ptr< QgsRasterLayer > rasterLayer = std::make_unique< QgsRasterLayer >( TEST_DATA_DIR + QStringLiteral( "/raster_layer.tiff" ),
      QStringLiteral( "raster" ), QStringLiteral( "gdal" ) );
  QVERIFY( rasterLayer->isValid() );

  QgsMapSettings mapSettings;
  mapSettings.setExtent( linesLayer->extent() );
  mapSettings.setDestinationCrs( linesLayer->crs() );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, false );
  mapSettings.setOutputDpi( 96 );

  std::unique_ptr< QgsMapRendererStagedRenderJob > job = std::make_unique< QgsMapRendererStagedRenderJob >( mapSettings );
  job->start();
  // nothing to render
  QVERIFY( job->isFinished() );
  QVERIFY( !job->renderCurrentPart( nullptr ) );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Finished );

  // with layers
  mapSettings.setLayers( QList<QgsMapLayer *>() << pointsLayer.get() << linesLayer.get() << rasterLayer.get() << polygonsLayer.get() );
  job = std::make_unique< QgsMapRendererStagedRenderJob >( mapSettings );
  job->start();
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), polygonsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  mapSettings.setBackgroundColor( QColor( 255, 255, 0 ) ); // should be ignored in this job
  QImage im( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  QPainter painter( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render1" ), im ) );
  QVERIFY( !job->isFinished() );
  QVERIFY( job->nextPart() );
  QCOMPARE( job->currentLayerId(), rasterLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // second layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render_raster" ), im ) );
  QVERIFY( !job->isFinished() );
  QVERIFY( job->nextPart() );
  QCOMPARE( job->currentLayerId(), linesLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // third layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render2" ), im ) );
  QVERIFY( !job->isFinished() );
  QVERIFY( job->nextPart() );
  QCOMPARE( job->currentLayerId(), pointsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // fourth layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render3" ), im ) );

  // nothing left!
  QVERIFY( !job->nextPart() );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Finished );
  QVERIFY( job->isFinished() );
  QVERIFY( !job->renderCurrentPart( &painter ) );
  // double check...
  QVERIFY( !job->renderCurrentPart( &painter ) );

  // job with labels
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, true );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  pointsLayer->setLabelsEnabled( true );

  job = std::make_unique< QgsMapRendererStagedRenderJob >( mapSettings );
  job->start();
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), polygonsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  mapSettings.setBackgroundColor( QColor( 255, 255, 0 ) ); // should be ignored in this job
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render1" ), im ) );
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), rasterLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // second layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render_raster" ), im ) );
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), linesLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // third layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render2" ), im ) );
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), pointsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // fourth layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render3" ), im ) );
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Labels );
  // labels
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render_points_labels" ), im ) );

  // nothing left!
  QVERIFY( !job->nextPart() );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Finished );
  QVERIFY( job->isFinished() );
  QVERIFY( !job->renderCurrentPart( &painter ) );
  // double check...
  QVERIFY( !job->renderCurrentPart( &painter ) );
}

void TestQgsMapRendererJob::stagedRendererWithStagedLabeling()
{
  // test the staged map renderer job subclass, when using staged labeling

  std::unique_ptr< QgsVectorLayer > pointsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );
  std::unique_ptr< QgsVectorLayer > linesLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  std::unique_ptr< QgsVectorLayer > polygonsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/polys.shp" ),
      QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QVERIFY( polygonsLayer->isValid() );

  QgsMapSettings mapSettings;
  mapSettings.setExtent( linesLayer->extent() );
  mapSettings.setDestinationCrs( linesLayer->crs() );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, false );
  mapSettings.setOutputDpi( 96 );

  std::unique_ptr< QgsMapRendererStagedRenderJob > job = std::make_unique< QgsMapRendererStagedRenderJob >( mapSettings, QgsMapRendererStagedRenderJob::RenderLabelsByMapLayer );
  job->start();
  // nothing to render
  QVERIFY( job->isFinished() );
  QVERIFY( !job->renderCurrentPart( nullptr ) );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Finished );

  // with layers
  mapSettings.setLayers( QList<QgsMapLayer *>() << pointsLayer.get() << linesLayer.get() << polygonsLayer.get() );
  job = std::make_unique< QgsMapRendererStagedRenderJob >( mapSettings, QgsMapRendererStagedRenderJob::RenderLabelsByMapLayer );
  job->start();
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), polygonsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  mapSettings.setBackgroundColor( QColor( 255, 255, 0 ) ); // should be ignored in this job
  QImage im( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  QPainter painter( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render1" ), im ) );
  QVERIFY( !job->isFinished() );
  QVERIFY( job->nextPart() );
  QCOMPARE( job->currentLayerId(), linesLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // second layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render2" ), im ) );
  QVERIFY( !job->isFinished() );
  QVERIFY( job->nextPart() );
  QCOMPARE( job->currentLayerId(), pointsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // third layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render3" ), im ) );

  // nothing left!
  QVERIFY( !job->nextPart() );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Finished );
  QVERIFY( job->isFinished() );
  QVERIFY( !job->renderCurrentPart( &painter ) );
  // double check...
  QVERIFY( !job->renderCurrentPart( &painter ) );

  // job with labels
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, true );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  settings.zIndex = 1;

  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  pointsLayer->setLabelsEnabled( true );

  settings.fieldName = QStringLiteral( "Name" );
  settings.placement = Qgis::LabelPlacement::Line;
  settings.zIndex = 3;
  linesLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  linesLayer->setLabelsEnabled( true );

  settings.fieldName = QStringLiteral( "Name" );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.zIndex = 2;
  settings.obstacleSettings().setType( QgsLabelObstacleSettings::PolygonInterior );
  polygonsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  polygonsLayer->setLabelsEnabled( true );

  job = std::make_unique< QgsMapRendererStagedRenderJob >( mapSettings, QgsMapRendererStagedRenderJob::RenderLabelsByMapLayer );
  job->start();
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), polygonsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  mapSettings.setBackgroundColor( QColor( 255, 255, 0 ) ); // should be ignored in this job
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render1" ), im ) );
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), linesLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // second layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render2" ), im ) );
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), pointsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Symbology );

  // third layer
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render3" ), im ) );

  // points labels (these must be in z-order!)
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), pointsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Labels );
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render_points_staged_labels" ), im ) );

  // polygon labels
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), polygonsLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Labels );
  // labels
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render_polygons_staged_labels" ), im ) );

  // line labels
  QVERIFY( job->nextPart() );
  QVERIFY( !job->isFinished() );
  QCOMPARE( job->currentLayerId(), linesLayer->id() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Labels );
  // labels
  im = QImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  im.fill( Qt::transparent );
  painter.begin( &im );
  QVERIFY( job->renderCurrentPart( &painter ) );
  painter.end();
  QVERIFY( imageCheck( QStringLiteral( "staged_render_lines_staged_labels" ), im ) );

  // nothing left!
  QVERIFY( !job->nextPart() );
  QVERIFY( job->currentLayerId().isEmpty() );
  QCOMPARE( job->currentStage(), QgsMapRendererStagedRenderJob::Finished );
  QVERIFY( job->isFinished() );
  QVERIFY( !job->renderCurrentPart( &painter ) );
  // double check...
  QVERIFY( !job->renderCurrentPart( &painter ) );
}

void TestQgsMapRendererJob::vectorLayerBoundsWithReprojection()
{
  std::unique_ptr< QgsVectorLayer > gridLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/grid_4326.geojson" ),
      QStringLiteral( "grid" ), QStringLiteral( "ogr" ) );
  QVERIFY( gridLayer->isValid() );

  std::unique_ptr< QgsLineSymbol > symbol = std::make_unique< QgsLineSymbol >();
  symbol->setColor( QColor( 255, 0, 255 ) );
  symbol->setWidth( 2 );
  std::unique_ptr< QgsSingleSymbolRenderer > renderer = std::make_unique< QgsSingleSymbolRenderer >( symbol.release() );
  gridLayer->setRenderer( renderer.release() );

  QgsMapSettings mapSettings;

  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mapSettings.setExtent( QgsRectangle( -37000835.1, -20182273.7, 37000835.1, 20182273.7 ) );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, false );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QList< QgsMapLayer * >() << gridLayer.get() );

  QgsMapRendererSequentialJob renderJob( mapSettings );
  renderJob.start();
  renderJob.waitForFinished();
  QImage img = renderJob.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "vector_layer_bounds_with_reprojection" ), img ) );
}

void TestQgsMapRendererJob::temporalRender()
{
  std::unique_ptr< QgsRasterLayer > rasterLayer = std::make_unique< QgsRasterLayer >( TEST_DATA_DIR + QStringLiteral( "/raster_layer.tiff" ),
      QStringLiteral( "raster" ), QStringLiteral( "gdal" ) );
  QVERIFY( rasterLayer->isValid() );

  QgsMapSettings mapSettings;
  mapSettings.setExtent( rasterLayer->extent() );
  mapSettings.setDestinationCrs( rasterLayer->crs() );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, false );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QList< QgsMapLayer * >() << rasterLayer.get() );

  QgsMapRendererSequentialJob renderJob( mapSettings );
  renderJob.start();
  renderJob.waitForFinished();
  QImage img = renderJob.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "temporal_render_visible" ), img ) );

  // set temporal properties for layer
  QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< QgsRasterLayerTemporalProperties * >( rasterLayer->temporalProperties() );
  temporalProperties->setIsActive( true );
  temporalProperties->setMode( Qgis::RasterTemporalMode::FixedTemporalRange );
  temporalProperties->setFixedTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
      QDateTime( QDate( 2020, 1, 5 ), QTime( 0, 0, 0 ) ) ) );

  // should still be visible -- map render job isn't temporal
  QgsMapRendererSequentialJob renderJob2( mapSettings );
  renderJob2.start();
  renderJob2.waitForFinished();
  img = renderJob2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "temporal_render_visible" ), img ) );

  // make render job temporal, outside of layer's fixed range
  mapSettings.setIsTemporal( true );
  mapSettings.setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2021, 1, 1 ), QTime( 0, 0, 0 ) ),
                                QDateTime( QDate( 2021, 1, 5 ), QTime( 0, 0, 0 ) ) ) );
  // should no longer be visible
  QgsMapRendererSequentialJob renderJob3( mapSettings );
  renderJob3.start();
  renderJob3.waitForFinished();
  img = renderJob3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "temporal_render_invisible" ), img ) );

  // temporal range ok for layer
  mapSettings.setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 2 ), QTime( 0, 0, 0 ) ),
                                QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ) ) ) );
  // should be visible
  QgsMapRendererSequentialJob renderJob4( mapSettings );
  renderJob4.start();
  renderJob4.waitForFinished();
  img = renderJob4.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "temporal_render_visible" ), img ) );

}

class TestLabelSink : public QgsLabelSink
{
  public:
    TestLabelSink() {};

    void drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) override
    {
      Q_UNUSED( layerId )
      Q_UNUSED( context )
      Q_UNUSED( label )
      Q_UNUSED( settings )
      drawnCount++;
    };

    int drawnCount = 0;
};

void TestQgsMapRendererJob::labelSink()
{
  std::unique_ptr< QgsVectorLayer > pointsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  settings.zIndex = 1;

  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  pointsLayer->setLabelsEnabled( true );

  QgsMapSettings mapSettings;
  mapSettings.setDestinationCrs( pointsLayer->crs() );
  mapSettings.setExtent( pointsLayer->extent() );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, true );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QList< QgsMapLayer * >() << pointsLayer.get() );


  QgsMapRendererSequentialJob renderJob( mapSettings );

  std::unique_ptr<TestLabelSink> labelSink = std::make_unique<TestLabelSink>();
  renderJob.setLabelSink( labelSink.get() );
  renderJob.start();
  renderJob.waitForFinished();
  QImage img = renderJob.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_sink" ), img ) );
  QCOMPARE( labelSink->drawnCount, 17 );
}

void TestQgsMapRendererJob::skipSymbolRendering()
{
  std::unique_ptr< QgsVectorLayer > pointsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  settings.zIndex = 1;

  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  pointsLayer->setLabelsEnabled( true );

  QgsMapSettings mapSettings;
  mapSettings.setDestinationCrs( pointsLayer->crs() );
  mapSettings.setExtent( pointsLayer->extent() );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, true );
  mapSettings.setFlag( Qgis::MapSettingsFlag::SkipSymbolRendering, true );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QList< QgsMapLayer * >() << pointsLayer.get() );

  QgsMapRendererSequentialJob renderJob( mapSettings );
  renderJob.start();
  renderJob.waitForFinished();
  QImage img = renderJob.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "skip_symbol_rendering" ), img ) );
}

void TestQgsMapRendererJob::customNullPainterJob()
{
  std::unique_ptr< QgsVectorLayer > pointsLayer = std::make_unique< QgsVectorLayer >( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  settings.zIndex = 1;

  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  pointsLayer->setLabelsEnabled( true );

  QgsMapSettings mapSettings;
  mapSettings.setDestinationCrs( pointsLayer->crs() );
  mapSettings.setExtent( pointsLayer->extent() );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, true );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QList< QgsMapLayer * >() << pointsLayer.get() );

  std::unique_ptr<QgsNullPaintDevice> nullPaintDevice = std::make_unique<QgsNullPaintDevice>();
  nullPaintDevice->setOutputSize( QSize( 512, 512 ) );
  nullPaintDevice->setOutputDpi( 96 );
  std::unique_ptr<QPainter> painter = std::make_unique<QPainter>( nullPaintDevice.get() );

  QgsMapRendererCustomPainterJob renderJob( mapSettings, painter.get() );

  std::unique_ptr<TestLabelSink> labelSink = std::make_unique<TestLabelSink>();
  renderJob.setLabelSink( labelSink.get() );

  renderJob.start();
  renderJob.waitForFinished();

  QCOMPARE( labelSink->drawnCount, 17 );
}

bool TestQgsMapRendererJob::imageCheck( const QString &testName, const QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "map_renderer" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  myChecker.setColorTolerance( 2 );
  bool myResultFlag = myChecker.runTest( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}


QGSTEST_MAIN( TestQgsMapRendererJob )
#include "testqgsmaprendererjob.moc"


