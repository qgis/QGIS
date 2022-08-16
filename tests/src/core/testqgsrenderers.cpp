/***************************************************************************
     testqgsrenderers.cpp
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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
//qgis test includes
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the different renderers for vector layers.
 */
class TestQgsRenderers : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRenderers() : QgsTest( QStringLiteral( "Vector Renderer Tests" ) ) {}

    ~TestQgsRenderers() override
    {
      delete mMapSettings;
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void singleSymbol();
    void emptyGeometry();
//    void uniqueValue();
//    void graduatedSymbol();
//    void continuousSymbol();
  private:
    bool mTestHasError =  false ;
    bool setQml( const QString &type ); //uniquevalue / continuous / single /
    bool imageCheck( const QString &type ); //as above
    bool checkEmptyRender( const QString &name, QgsVectorLayer *layer );
    QgsMapSettings *mMapSettings = nullptr;
    QgsMapLayer *mpPointsLayer = nullptr;
    QgsMapLayer *mpLinesLayer = nullptr;
    QgsMapLayer *mpPolysLayer = nullptr;
    QString mTestDataDir;
};


void TestQgsRenderers::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  mMapSettings = new QgsMapSettings();

  //create some objects that will be used in all tests...


  //
  //create a point layer that will be used in all tests...
  //
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                      myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );


  //
  // Create a line layer that will be used in all tests...
  //
  const QString myLinesFileName = mTestDataDir + "lines.shp";
  const QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLinesLayer );
  //
  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings->setLayers(
    QList<QgsMapLayer *>() << mpPointsLayer << mpPolysLayer << mpLinesLayer );
}
void TestQgsRenderers::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRenderers::singleSymbol()
{
  QVERIFY( setQml( "single" ) );
  QVERIFY( imageCheck( "single" ) );
}

void TestQgsRenderers::emptyGeometry()
{
  // test rendering an empty geometry
  // note - this test is of limited use, because the features with empty geometries should not be rendered
  // by the feature iterator given that renderer uses a filter rect on the request. It's placed here
  // for manual testing by commenting out the part of the renderer which places the filterrect on the
  // layer's feature request. The purpose of this test is to ensure that we do not crash for empty geometries,
  // as it's possible that malformed providers OR bugs in underlying libraries will still return empty geometries
  // even when a filter rect request was made, and we shouldn't crash for these.
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "MultiPolygon?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );

  QgsFeature f;
  std::unique_ptr< QgsMultiPolygon > mp = std::make_unique< QgsMultiPolygon >();
  mp->addGeometry( new QgsPolygon() );
  f.setGeometry( QgsGeometry( std::move( mp ) ) );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  QVERIFY( checkEmptyRender( "Multipolygon", vl ) );

  // polygon
  vl = new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );
  f.setGeometry( QgsGeometry( new QgsPolygon() ) );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  QVERIFY( checkEmptyRender( "Polygon", vl ) );

  // linestring
  vl = new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );
  f.setGeometry( QgsGeometry( new QgsLineString() ) );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  QVERIFY( checkEmptyRender( "LineString", vl ) );

  // multilinestring
  vl = new QgsVectorLayer( QStringLiteral( "MultiLineString?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );
  std::unique_ptr< QgsMultiLineString > mls = std::make_unique< QgsMultiLineString >();
  mls->addGeometry( new QgsLineString() );
  f.setGeometry( QgsGeometry( std::move( mls ) ) );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  QVERIFY( checkEmptyRender( "MultiLineString", vl ) );

  // multipoint
  vl = new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );
  std::unique_ptr< QgsMultiPoint > mlp = std::make_unique< QgsMultiPoint >();
  f.setGeometry( QgsGeometry( std::move( mlp ) ) );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  QVERIFY( checkEmptyRender( "MultiPoint", vl ) );
}

bool TestQgsRenderers::checkEmptyRender( const QString &testName, QgsVectorLayer *layer )
{
  QgsMapSettings ms;
  const QgsRectangle extent( -180, -90, 180, 90 );
  ms.setExtent( extent );
  ms.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
  ms.setOutputDpi( 96 );
  ms.setLayers( QList< QgsMapLayer * >() << layer );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlName( "expected_emptygeometry" );
  myChecker.setMapSettings( ms );
  myChecker.setControlPathPrefix( "map_renderer" );
  myChecker.setColorTolerance( 15 );
  const bool myResultFlag = myChecker.runTest( testName, 200 );
  mReport += myChecker.report();
  return myResultFlag;
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRenderers::setQml( const QString &type )
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  if ( ! mpPointsLayer->isValid() )
  {
    return false;
  }
  QString myFileName = mTestDataDir + "points_" + type + "_symbol.qml";
  bool myStyleFlag = false;
  const QString error = mpPointsLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
    return false;
  }
  else
  {
    myStyleFlag = false; //ready for next test
  }
  myFileName = mTestDataDir + "polys_" + type + "_symbol.qml";
  mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    return false;
  }
  else
  {
    myStyleFlag = false; //ready for next test
  }
  myFileName = mTestDataDir + "lines_" + type + "_symbol.qml";
  mpLinesLayer->loadNamedStyle( myFileName, myStyleFlag );
  return myStyleFlag;
}

bool TestQgsRenderers::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image

  // mpPointsLayer->extent() was giving wrong extent in QGIS 2.0 (xmin shifted,
  // the same wrong value is reported by ogrinfo). Since QGIS 2.1, the provider
  // gives correct extent. Forced to fixed extend however to avoid problems in future.
  const QgsRectangle extent( -118.8888888888887720, 22.8002070393376783, -83.3333333333331581, 46.8719806763287536 );
  mMapSettings->setExtent( extent );
  mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
  mMapSettings->setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  const bool myResultFlag = myChecker.runTest( testType, 200 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsRenderers )
#include "testqgsrenderers.moc"
