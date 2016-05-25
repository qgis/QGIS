/***************************************************************************
     testqgs25drenderer.cpp
     --------------------------------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgs25drenderer.h>
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgsmultirenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for 25d renderer.
 */
class TestQgs25DRenderer : public QObject
{
    Q_OBJECT
  public:
    TestQgs25DRenderer()
        : mpPolysLayer( nullptr )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void render();
    void renderComposition();

  private:
    bool imageCheck( const QString& theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPolysLayer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgs25DRenderer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  //need a very high height to check for stacking
  QgsExpressionContextUtils::setLayerVariable( mpPolysLayer, "qgis_25d_height", 8 );
  QgsExpressionContextUtils::setLayerVariable( mpPolysLayer, "qgis_25d_angle", 45 );

  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );

  mMapSettings.setLayers( QStringList() << mpPolysLayer->id() );
  mReport += "<h1>25D Renderer Tests</h1>\n";

}
void TestQgs25DRenderer::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgs25DRenderer::render()
{
  mReport += "<h2>Render</h2>\n";

  //setup 25d renderer
  Qgs25DRenderer* renderer = new Qgs25DRenderer( );
  renderer->setShadowEnabled( false );
  renderer->setWallShadingEnabled( false );
  renderer->setRoofColor( QColor( "#fdbf6f" ) );
  mpPolysLayer->setRendererV2( renderer );

  QVERIFY( imageCheck( "25d_render" ) );
}

void TestQgs25DRenderer::renderComposition()
{
  QgsComposition* composition = new QgsComposition( mMapSettings );
  composition->setPaperSize( 297, 210 ); //A4 landscape
  QgsComposerMap* map = new QgsComposerMap( composition, 20, 20, 200, 100 );
  map->setFrameEnabled( true );
  composition->addComposerMap( map );

  map->setNewExtent( mpPolysLayer->extent() );
  QgsCompositionChecker checker( "25d_composer", composition );
  checker.setControlPathPrefix( "25d_renderer" );

  QVERIFY( checker.testComposition( mReport, 0, 100 ) );
}

bool TestQgs25DRenderer::imageCheck( const QString& theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( mMapSettings );
  mMapSettings.setExpressionContext( context );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( "25d_renderer" );
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  bool myResultFlag = myChecker.runTest( theTestType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgs25DRenderer )
#include "testqgs25drenderer.moc"
