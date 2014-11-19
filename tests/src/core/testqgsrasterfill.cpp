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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

#include <iostream>
//qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgsfillsymbollayerv2.h>
//qgis test includes
#include "qgsmultirenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for raster fill types.
 */
class TestQgsRasterFill: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void rasterFillSymbol();
    void coordinateMode();
    void alpha();
    void offset();
    void width();

  private:
    bool mTestHasError;
    bool setQml( QString theType );
    bool imageCheck( QString theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPolysLayer;
    QgsRasterFillSymbolLayer* mRasterFill;
    QgsFillSymbolV2* mFillSymbol;
    QgsSingleSymbolRendererV2* mSymbolRenderer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsRasterFill::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + QDir::separator();

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

  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );

  //setup raster fill
  mRasterFill = new QgsRasterFillSymbolLayer();
  mFillSymbol = new QgsFillSymbolV2();
  mFillSymbol->changeSymbolLayer( 0, mRasterFill );
  mSymbolRenderer = new QgsSingleSymbolRendererV2( mFillSymbol );
  mpPolysLayer->setRendererV2( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QStringList() << mpPolysLayer->id() );
  mReport += "<h1>Raster Fill Renderer Tests</h1>\n";

}
void TestQgsRasterFill::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsRasterFill::init()
{
  mRasterFill->setImageFilePath( mTestDataDir + QString( "sample_image.png" ) );
  mRasterFill->setWidth( 30.0 );
  mRasterFill->setWidthUnit( QgsSymbolV2::Pixel );
  mRasterFill->setCoordinateMode( QgsRasterFillSymbolLayer::Feature );
  mRasterFill->setAlpha( 1.0 );
  mRasterFill->setOffset( QPointF( 0, 0 ) );
}

void TestQgsRasterFill::cleanup()
{

}

void TestQgsRasterFill::rasterFillSymbol()
{
  mReport += "<h2>Raster fill symbol renderer test</h2>\n";
  bool result = imageCheck( "rasterfill" );
  QVERIFY( result );
}

void TestQgsRasterFill::coordinateMode()
{
  mReport += "<h2>Raster fill viewport mode</h2>\n";
  mRasterFill->setCoordinateMode( QgsRasterFillSymbolLayer::Viewport );
  bool result = imageCheck( "rasterfill_viewport" );
  QVERIFY( result );
}

void TestQgsRasterFill::alpha()
{
  mReport += "<h2>Raster fill alpha</h2>\n";
  mRasterFill->setAlpha( 0.5 );
  bool result = imageCheck( "rasterfill_alpha" );
  QVERIFY( result );
}

void TestQgsRasterFill::offset()
{
  mReport += "<h2>Raster fill offset</h2>\n";
  mRasterFill->setOffset( QPointF( 5, 10 ) );;
  bool result = imageCheck( "rasterfill_offset" );
  QVERIFY( result );
}

void TestQgsRasterFill::width()
{
  mReport += "<h2>Raster fill width</h2>\n";
  mRasterFill->setWidthUnit( QgsSymbolV2::MM );
  mRasterFill->setWidth( 5.0 );
  bool result = imageCheck( "rasterfill_width" );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRasterFill::setQml( QString theType )
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  QString myFileName = mTestDataDir + "polys_" + theType + "_symbol.qml";
  bool myStyleFlag = false;
  QString error = mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
  }
  return myStyleFlag;
}

bool TestQgsRasterFill::imageCheck( QString theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  bool myResultFlag = myChecker.runTest( theTestType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsRasterFill )
#include "testqgsrasterfill.moc"
