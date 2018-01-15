/***************************************************************************
     testqgsellipsemarker.cpp
     ------------------------
    Date                 : Nov 2015
    Copyright            : (C) 2015 by Nyall Dawson
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
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include "qgsmarkersymbollayer.h"
#include "qgsellipsesymbollayer.h"
#include "qgsproperty.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for ellipse marker symbol types.
 */
class TestQgsEllipseMarkerSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsEllipseMarkerSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void ellipseMarkerSymbol();
    void ellipseMarkerSymbolBevelJoin();
    void ellipseMarkerSymbolMiterJoin();
    void ellipseMarkerSymbolRoundJoin();
    void bounds();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsEllipseSymbolLayer *mEllipseMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsEllipseMarkerSymbol::initTestCase()
{
  mTestHasError = false;
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
  QString pointFileName = mTestDataDir + "points.shp";
  QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                      pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  //setup symbol
  mEllipseMarkerLayer = new QgsEllipseSymbolLayer();
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mEllipseMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
  mReport += QLatin1String( "<h1>Ellipse Marker Tests</h1>\n" );

}
void TestQgsEllipseMarkerSymbol::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mpPointsLayer;

  QgsApplication::exitQgis();
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbol()
{
  mReport += QLatin1String( "<h2>Ellipse marker symbol layer test</h2>\n" );

  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setSymbolName( QStringLiteral( "circle" ) );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );
  QVERIFY( imageCheck( "ellipsemarker" ) );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolBevelJoin()
{
  mReport += QLatin1String( "<h2>Ellipse marker symbol layer test</h2>\n" );

  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setSymbolName( QStringLiteral( "triangle" ) );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::BevelJoin );
  QVERIFY( imageCheck( "ellipsemarker_beveljoin" ) );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolMiterJoin()
{
  mReport += QLatin1String( "<h2>Ellipse marker symbol layer test</h2>\n" );

  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setSymbolName( QStringLiteral( "triangle" ) );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( imageCheck( "ellipsemarker_miterjoin" ) );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolRoundJoin()
{
  mReport += QLatin1String( "<h2>Ellipse marker symbol layer test</h2>\n" );

  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setSymbolName( QStringLiteral( "triangle" ) );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::RoundJoin );
  QVERIFY( imageCheck( "ellipsemarker_roundjoin" ) );
}

void TestQgsEllipseMarkerSymbol::bounds()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setSymbolName( QStringLiteral( "circle" ) );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );
  mEllipseMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( QStringLiteral( "ellipsemarker_bounds" ) );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  QVERIFY( result );
}


//
// Private helper functions not called directly by CTest
//


bool TestQgsEllipseMarkerSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_ellipsemarker" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsEllipseMarkerSymbol )
#include "testqgsellipsemarker.moc"
