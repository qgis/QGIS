/***************************************************************************
     testqgsblendmodes.cpp
     --------------------------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Nyall Dawson, Tim Sutton
    Email                : nyall dot dawson at gmail.com
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
#include <QDir>
#include <QPainter>

//qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgsrasterlayer.h>
#include "qgsrasterdataprovider.h"
//qgis test includes
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for layer blend modes
 */
class TestQgsBlendModes : public QObject
{
    Q_OBJECT

  public:
    TestQgsBlendModes() = default;
    ~TestQgsBlendModes() override
    {
      delete mMapSettings;
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void vectorBlending();
    void featureBlending();
    void vectorLayerTransparency();
    void rasterBlending();
  private:
    bool imageCheck( const QString &type ); //as above
    QgsMapSettings *mMapSettings = nullptr;
    QgsMapLayer *mpPointsLayer = nullptr;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsVectorLayer *mpLinesLayer = nullptr;
    QgsRasterLayer *mRasterLayer1 = nullptr;
    QgsRasterLayer *mRasterLayer2 = nullptr;
    QString mTestDataDir;
    QgsRectangle mExtent;
    QString mReport;
};


void TestQgsBlendModes::initTestCase()
{

  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  mMapSettings = new QgsMapSettings();

  //create some objects that will be used in tests

  //create a point layer
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                      myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  //create a poly layer that will be used in tests
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );

  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  //create a line layer that will be used in tests
  const QString myLinesFileName = mTestDataDir + "lines.shp";
  const QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  mpLinesLayer->setSimplifyMethod( simplifyMethod );

  //create two raster layers
  const QFileInfo rasterFileInfo( mTestDataDir + "rgb256x256.png" );
  mRasterLayer1 = new QgsRasterLayer( rasterFileInfo.filePath(),
                                      rasterFileInfo.completeBaseName() );
  mRasterLayer2 = new QgsRasterLayer( rasterFileInfo.filePath(),
                                      rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer1->dataProvider(), 1, 2, 3 );
  mRasterLayer1->setRenderer( rasterRenderer );
  mRasterLayer2->setRenderer( ( QgsRasterRenderer * ) rasterRenderer->clone() );

  // points extent was not always reliable
  mExtent = QgsRectangle( -118.8888888888887720, 22.8002070393376783, -83.3333333333331581, 46.8719806763287536 );
}
void TestQgsBlendModes::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mpPointsLayer;
  delete mpPolysLayer;
  delete mpLinesLayer;
  delete mRasterLayer1;
  delete mRasterLayer2;

  QgsApplication::exitQgis();
}

void TestQgsBlendModes::vectorBlending()
{
  //Add two vector layers
  QList<QgsMapLayer *> myLayers;
  myLayers << mpLinesLayer;
  myLayers << mpPolysLayer;
  mMapSettings->setLayers( myLayers );

  //Set blending modes for both layers
  mpLinesLayer->setBlendMode( QPainter::CompositionMode_Difference );
  mpPolysLayer->setBlendMode( QPainter::CompositionMode_Difference );
  mMapSettings->setExtent( mExtent );
  const bool res = imageCheck( QStringLiteral( "vector_blendmodes" ) );

  //Reset layers
  mpLinesLayer->setBlendMode( QPainter::CompositionMode_SourceOver );
  mpPolysLayer->setBlendMode( QPainter::CompositionMode_SourceOver );

  QVERIFY( res );
}

void TestQgsBlendModes::featureBlending()
{
  //Add two vector layers
  QList<QgsMapLayer *> myLayers;
  myLayers << mpLinesLayer;
  myLayers << mpPolysLayer;
  mMapSettings->setLayers( myLayers );

  //Set feature blending modes for point layer
  mpLinesLayer->setFeatureBlendMode( QPainter::CompositionMode_Plus );
  mMapSettings->setExtent( mExtent );
  const bool res = imageCheck( QStringLiteral( "vector_featureblendmodes" ) );

  //Reset layers
  mpLinesLayer->setFeatureBlendMode( QPainter::CompositionMode_SourceOver );

  QVERIFY( res );
}

void TestQgsBlendModes::vectorLayerTransparency()
{
  //Add two vector layers
  QList<QgsMapLayer *> myLayers;
  myLayers << mpLinesLayer;
  myLayers << mpPolysLayer;
  mMapSettings->setLayers( myLayers );

  //Set feature blending modes for point layer
  mpLinesLayer->setOpacity( 0.50 );
  mMapSettings->setExtent( mExtent );
  const bool res = imageCheck( QStringLiteral( "vector_layertransparency" ) );

  //Reset layers
  mpLinesLayer->setOpacity( 1.0 );

  QVERIFY( res );
}

void TestQgsBlendModes::rasterBlending()
{
  //Add two raster layers to map renderer
  QList<QgsMapLayer *> myLayers;
  myLayers << mRasterLayer1;
  myLayers << mRasterLayer2;
  mMapSettings->setLayers( myLayers );
  mMapSettings->setExtent( mRasterLayer1->extent() );

  // set blending mode for top layer
  mRasterLayer1->setBlendMode( QPainter::CompositionMode_Difference );
  QVERIFY( imageCheck( "raster_blendmodes" ) );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsBlendModes::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings->setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 5 );
  const bool myResultFlag = myChecker.runTest( testType, 20 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsBlendModes )
#include "testqgsblendmodes.moc"
