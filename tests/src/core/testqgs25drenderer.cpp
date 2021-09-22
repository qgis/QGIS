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
#include "qgstest.h"
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
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgs25drenderer.h>
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgsmultirenderchecker.h"
#include "qgsexpressioncontextutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for 25d renderer.
 */
class TestQgs25DRenderer : public QObject
{
    Q_OBJECT
  public:
    TestQgs25DRenderer() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void render();
    void renderLayout();

  private:
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
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
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  //need a very high height to check for stacking
  QgsExpressionContextUtils::setLayerVariable( mpPolysLayer, QStringLiteral( "qgis_25d_height" ), 8 );
  QgsExpressionContextUtils::setLayerVariable( mpPolysLayer, QStringLiteral( "qgis_25d_angle" ), 45 );

  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>25D Renderer Tests</h1>\n" );

}
void TestQgs25DRenderer::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mpPolysLayer;

  QgsApplication::exitQgis();
}

void TestQgs25DRenderer::render()
{
  mReport += QLatin1String( "<h2>Render</h2>\n" );

  //setup 25d renderer
  Qgs25DRenderer *renderer = new Qgs25DRenderer();
  renderer->setShadowEnabled( false );
  renderer->setWallShadingEnabled( false );
  renderer->setRoofColor( QColor( 253, 191, 111 ) );
  mpPolysLayer->setRenderer( renderer );

  QVERIFY( imageCheck( "25d_render" ) );
}

void TestQgs25DRenderer::renderLayout()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList< QgsMapLayer * >() << mpPolysLayer );
  l.addLayoutItem( map );

  map->setExtent( mpPolysLayer->extent() );
  QgsLayoutChecker checker( QStringLiteral( "25d_composer" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "25d_renderer" ) );

  QVERIFY( checker.testLayout( mReport, 0, 100 ) );
}

bool TestQgs25DRenderer::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputSize( QSize( 400, 400 ) );
  mMapSettings.setOutputDpi( 96 );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( mMapSettings );
  mMapSettings.setExpressionContext( context );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "25d_renderer" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  const bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgs25DRenderer )
#include "testqgs25drenderer.moc"
