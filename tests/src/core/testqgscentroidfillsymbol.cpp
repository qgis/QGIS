/***************************************************************************
     TestQgsCentroidFillSymbol.cpp
     -------------------------
    Date                 : Apr 2015
    Copyright            : (C) 2015 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
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
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbol.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for line fill symbol types.
 */
class TestQgsCentroidFillSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsCentroidFillSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void centroidFillSymbol();
    void centroidFillSymbolPointOnSurface();
    void centroidFillSymbolPartBiggest();
    void centroidFillClipPoints();
    void centroidFillClipOnCurrentPartOnly();
    void centroidFillClipOnCurrentPartOnlyBiggest();
    void centroidFillClipMultiplayerPoints();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsCentroidFillSymbolLayer *mCentroidFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsCentroidFillSymbol::initTestCase()
{
  mTestHasError = false;
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

  //setup gradient fill
  mCentroidFill = new QgsCentroidFillSymbolLayer();
  static_cast< QgsSimpleMarkerSymbolLayer * >( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mCentroidFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Centroid Fill Symbol Tests</h1>\n" );

}
void TestQgsCentroidFillSymbol::cleanupTestCase()
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

void TestQgsCentroidFillSymbol::centroidFillSymbol()
{
  mReport += QLatin1String( "<h2>Line fill symbol renderer test</h2>\n" );

  QVERIFY( imageCheck( "symbol_centroidfill" ) );
}

void TestQgsCentroidFillSymbol::centroidFillSymbolPointOnSurface()
{
  mCentroidFill->setPointOnSurface( true );
  QVERIFY( imageCheck( "symbol_centroidfill_point_on_surface" ) );
  mCentroidFill->setPointOnSurface( false );
}

void TestQgsCentroidFillSymbol::centroidFillSymbolPartBiggest()
{
  mCentroidFill->setPointOnAllParts( false );
  QVERIFY( imageCheck( "symbol_centroidfill_part_biggest" ) );
  mCentroidFill->setPointOnAllParts( true );
}

void TestQgsCentroidFillSymbol::centroidFillClipPoints()
{
  mCentroidFill->setClipPoints( true );
  QVERIFY( imageCheck( "symbol_centroidfill_clip_points" ) );
  mCentroidFill->setClipPoints( false );
}

void TestQgsCentroidFillSymbol::centroidFillClipOnCurrentPartOnly()
{
  mCentroidFill->setClipPoints( true );
  mCentroidFill->setClipOnCurrentPartOnly( true );
  QVERIFY( imageCheck( "symbol_centroidfill_clip_current_only" ) );
  mCentroidFill->setClipPoints( false );
  mCentroidFill->setClipOnCurrentPartOnly( false );
}

void TestQgsCentroidFillSymbol::centroidFillClipOnCurrentPartOnlyBiggest()
{
  mCentroidFill->setClipPoints( true );
  mCentroidFill->setClipOnCurrentPartOnly( true );
  mCentroidFill->setPointOnAllParts( false );
  QVERIFY( imageCheck( "symbol_centroidfill_clip_current_biggest" ) );
  mCentroidFill->setClipPoints( false );
  mCentroidFill->setClipOnCurrentPartOnly( false );
  mCentroidFill->setPointOnAllParts( true );
}

void TestQgsCentroidFillSymbol::centroidFillClipMultiplayerPoints()
{
  const QgsSimpleFillSymbolLayer simpleFill( QColor( 255, 255, 255, 100 ) );

  mCentroidFill = mCentroidFill->clone();
  mCentroidFill->setClipPoints( true );

  mFillSymbol->deleteSymbolLayer( 0 );
  mFillSymbol->appendSymbolLayer( simpleFill.clone() );
  mFillSymbol->appendSymbolLayer( mCentroidFill->clone() );
  mFillSymbol->appendSymbolLayer( simpleFill.clone() );

  QVERIFY( imageCheck( "symbol_centroidfill_clip_multilayer" ) );

  mCentroidFill->setClipPoints( false );
  mFillSymbol->deleteSymbolLayer( 0 );
  mFillSymbol->deleteSymbolLayer( 1 );
  mFillSymbol->deleteSymbolLayer( 2 );
  mFillSymbol->changeSymbolLayer( 0, mCentroidFill );
}


void TestQgsCentroidFillSymbol::opacityWithDataDefinedColor()
{
  const QgsSimpleFillSymbolLayer simpleFill( QColor( 255, 255, 255, 100 ) );

  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'red', 'green')" ) ) );
  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'blue', 'magenta')" ) ) );
  qgis::down_cast< QgsSimpleMarkerSymbolLayer * >( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeWidth( 0.5 );
  qgis::down_cast< QgsSimpleMarkerSymbolLayer * >( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setSize( 5 );
  mCentroidFill->subSymbol()->setOpacity( 0.5 );
  mFillSymbol->setOpacity( 0.5 );

  QVERIFY( imageCheck( "symbol_centroidfill_opacityddcolor" ) );
}

void TestQgsCentroidFillSymbol::dataDefinedOpacity()
{
  const QgsSimpleFillSymbolLayer simpleFill( QColor( 255, 255, 255, 100 ) );

  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'red', 'green')" ) ) );
  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'blue', 'magenta')" ) ) );
  qgis::down_cast< QgsSimpleMarkerSymbolLayer * >( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeWidth( 0.5 );
  qgis::down_cast< QgsSimpleMarkerSymbolLayer * >( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setSize( 5 );
  mCentroidFill->subSymbol()->setOpacity( 0.5 );
  mFillSymbol->setOpacity( 1.0 );
  mFillSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Value\" >10, 25, 50)" ) ) );

  QVERIFY( imageCheck( "symbol_centroidfill_ddopacity" ) );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsCentroidFillSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_centroidfill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  const bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsCentroidFillSymbol )
#include "testqgscentroidfillsymbol.moc"
