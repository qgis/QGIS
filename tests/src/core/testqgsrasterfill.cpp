/***************************************************************************
     testqgsrasterfill.cpp
     ---------------------
    Date                 : November 2014
    Copyright            : (C) 2014 Nyall Dawson
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

// qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
// qgis test includes
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for raster fill types.
 */
class TestQgsRasterFill : public QObject
{
  Q_OBJECT

  public:
    TestQgsRasterFill() = default;

  private slots:
    void initTestCase();		// will be called before the first testfunction is executed.
    void cleanupTestCase();	// will be called after the last testfunction was executed.
    void init();						// will be called before each testfunction is executed.
    void cleanup();					// will be called after every testfunction.

    void rasterFillSymbol();
    void coordinateMode();
    void offset();
    void alpha();

    void percentage();
    void percentageCoordinateMode() { coordinateMode(); }
    void percentageOffset() { offset(); }
    void percentageAlpha() { alpha(); }

  private:
    bool mTestHasError = false;
    bool setQml( const QString &type );
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsRasterFillSymbolLayer *mRasterFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
    QString mPercentageName;
};


void TestQgsRasterFill::initTestCase()
{
  mTestHasError = false;

  // Init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); // defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  // Create a poly layer that will be used in all tests...
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
    myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mpPolysLayer );

  // Setup raster fill
  mRasterFill = new QgsRasterFillSymbolLayer();
  mRasterFill->setImageFilePath( mTestDataDir + QStringLiteral( "sample_image.png" ) );
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPixels );
  mRasterFill->setWidth( 30.0 );
  mRasterFill->setCoordinateMode( QgsRasterFillSymbolLayer::Feature );
  mRasterFill->setOpacity( 1.0 );
  mRasterFill->setOffset( QPointF( 0, 0 ) );
  mRasterFill->setOffsetUnit( QgsUnitTypes::RenderPixels );

  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mRasterFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Raster Fill Renderer Tests</h1>\n" );
}

void TestQgsRasterFill::cleanupTestCase()
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

void TestQgsRasterFill::init()
{
}

void TestQgsRasterFill::cleanup()
{
}

void TestQgsRasterFill::rasterFillSymbol()
{
  mReport += QLatin1String( "<h2>Raster fill symbol renderer</h2>\n" );
  bool result = imageCheck( QStringLiteral( "rasterfill" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::coordinateMode()
{
  mReport += QString( "<h2>Raster fill" + mPercentageName + " viewport mode</h2>\n" );
  mRasterFill->setCoordinateMode( QgsRasterFillSymbolLayer::Viewport );
  bool result = imageCheck( QStringLiteral( "rasterfill_viewport" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterFill::offset()
{
  mReport += QString( "<h2>Raster fill" + mPercentageName + " offset (12 px; 15 px)</h2>\n" );
  mRasterFill->setOffset( QPointF( 12, 15 ) );
  bool result = imageCheck( QStringLiteral( "rasterfill_offset" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterFill::alpha()
{
  mReport += QString( "<h2>Raster fill" + mPercentageName + " alpha (0.5)</h2>\n" );
  mRasterFill->setOpacity( 0.5 );
  bool result = imageCheck( QStringLiteral( "rasterfill_alpha" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterFill::percentage()
{
  mPercentageName = "_percentage";
  mRasterFill->setCoordinateMode( QgsRasterFillSymbolLayer::Feature );
  mRasterFill->setOffset( QPointF( 0, 0 ) );
  mRasterFill->setOpacity( 1.0 );

  mReport += QString( "<h2>Raster fill_percentage (12.3 %)</h2>\n" );
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPercentage );
  mRasterFill->setWidth( 12.3 );
  bool result = imageCheck( QStringLiteral( "rasterfill_percentage" ) );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRasterFill::setQml( const QString &type )
{
  // Load a qml style and apply to our layer
  // the style will correspond to the renderer
  // type we are testing
  QString myFileName = mTestDataDir + "polys_" + type + "_symbol.qml";
  bool myStyleFlag = false;
  QString error = mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
  }
  return myStyleFlag;
}

bool TestQgsRasterFill::imageCheck( const QString &testType )
{
  // Use the QgsRenderChecker test utility class to
  // ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_rasterfill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsRasterFill )
#include "testqgsrasterfill.moc"
