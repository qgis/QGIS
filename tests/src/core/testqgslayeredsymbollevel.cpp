/***************************************************************************
     testqgslayeredsymbollevel.cpp
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
#include <qgssinglesymbolrenderer.h>
#include <qgsfeaturerequest.h>
#include <qgslinesymbol.h>
#include <qgslinesymbollayer.h>
#include <qgssymbol.h>
#include "qgsmultirenderchecker.h"
#include "qgsexpressioncontextutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for layered symbol level rendering.
 */
class TestQgsLayeredSymbolLevel : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsLayeredSymbolLevel()
      : QgsTest( QStringLiteral( "Layered Symbol Level Rendering Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void render();

  private:
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpRoadsLayer = nullptr;
    QString mTestDataDir;
};


void TestQgsLayeredSymbolLevel::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a roads layer that will be used in all tests...
  //
  const QString myRoadsFileName = mTestDataDir + "layered_roads.shp";
  const QFileInfo myRoadsFileInfo( myRoadsFileName );
  mpRoadsLayer = new QgsVectorLayer( myRoadsFileInfo.filePath(), myRoadsFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpRoadsLayer->setSimplifyMethod( simplifyMethod );

  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpRoadsLayer );
}
void TestQgsLayeredSymbolLevel::cleanupTestCase()
{
  delete mpRoadsLayer;

  QgsApplication::exitQgis();
}

void TestQgsLayeredSymbolLevel::render()
{
  QgsSimpleLineSymbolLayer *lineSymbolLayer0 = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer0->setColor( Qt::black );
  lineSymbolLayer0->setWidth( 3 );

  QgsSimpleLineSymbolLayer *lineSymbolLayer1 = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer1->setColor( Qt::yellow );
  lineSymbolLayer1->setWidth( 2 );

  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer0 << lineSymbolLayer1 );

  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( lineSymbol );
  mpRoadsLayer->setRenderer( renderer );

  renderer->setUsingSymbolLevels( true );
  QVERIFY( imageCheck( "with_levels_no_layers" ) );

  renderer->setOrderBy( QgsFeatureRequest::OrderBy() << QgsFeatureRequest::OrderByClause( "layer", false ) );
  renderer->setOrderByEnabled( true );
  QVERIFY( imageCheck( "with_levels_with_layers" ) );

  renderer->setUsingSymbolLevels( false );
  QVERIFY( imageCheck( "no_levels_with_layers" ) );
}

bool TestQgsLayeredSymbolLevel::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpRoadsLayer->extent() );
  mMapSettings.setOutputSize( QSize( 400, 400 ) );
  mMapSettings.setOutputDpi( 96 );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( mMapSettings );
  mMapSettings.setExpressionContext( context );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "layered_symbol_levels" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  const bool myResultFlag = myChecker.runTest( testType, 0 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsLayeredSymbolLevel )
#include "testqgslayeredsymbollevel.moc"
