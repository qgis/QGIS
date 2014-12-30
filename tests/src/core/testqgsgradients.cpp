/***************************************************************************
     testqgsgradients.cpp
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
#include <QtTest/QtTest>
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
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgsfillsymbollayerv2.h>
#include <qgsvectorcolorrampv2.h>
//qgis test includes
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for gradient fill types.
 */
class TestQgsGradients : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void gradientSymbol();
    void gradientSymbolColors();
    void gradientSymbolRamp();
    void gradientSymbolRadial();
    void gradientSymbolConical();
    void gradientSymbolViewport();
    void gradientSymbolReferencePoints();
    void gradientSymbolCentroid();
    void gradientSymbolReflectSpread();
    void gradientSymbolRepeatSpread();
    void gradientSymbolRotate();
    void gradientSymbolFromQml();
  private:
    bool mTestHasError;
    bool setQml( QString theType );
    bool imageCheck( QString theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPolysLayer;
    QgsGradientFillSymbolLayerV2* mGradientFill;
    QgsFillSymbolV2* mFillSymbol;
    QgsSingleSymbolRendererV2* mSymbolRenderer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsGradients::initTestCase()
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

  //setup gradient fill
  mGradientFill = new QgsGradientFillSymbolLayerV2();
  mFillSymbol = new QgsFillSymbolV2();
  mFillSymbol->changeSymbolLayer( 0, mGradientFill );
  mSymbolRenderer = new QgsSingleSymbolRendererV2( mFillSymbol );
  mpPolysLayer->setRendererV2( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QStringList() << mpPolysLayer->id() );
  mReport += "<h1>Gradient Renderer Tests</h1>\n";

}
void TestQgsGradients::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsGradients::gradientSymbol()
{
  mReport += "<h2>Gradient symbol renderer test</h2>\n";
  mGradientFill->setColor( QColor( "red" ) );
  mGradientFill->setColor2( QColor( "blue" ) );
  mGradientFill->setGradientType( QgsGradientFillSymbolLayerV2::Linear );
  mGradientFill->setGradientColorType( QgsGradientFillSymbolLayerV2::SimpleTwoColor );
  mGradientFill->setCoordinateMode( QgsGradientFillSymbolLayerV2::Feature );
  mGradientFill->setGradientSpread( QgsGradientFillSymbolLayerV2::Pad );
  mGradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  QVERIFY( imageCheck( "gradient" ) );
}

void TestQgsGradients::gradientSymbolColors()
{
  mReport += "<h2>Gradient symbol renderer color test</h2>\n";
  mGradientFill->setColor( QColor( "green" ) );
  mGradientFill->setColor2( QColor( "white" ) );
  QVERIFY( imageCheck( "gradient_colors" ) );
  mGradientFill->setColor( QColor( "red" ) );
  mGradientFill->setColor2( QColor( "blue" ) );
}

void TestQgsGradients::gradientSymbolRamp()
{
  QgsVectorGradientColorRampV2* gradientRamp = new QgsVectorGradientColorRampV2( QColor( Qt::red ), QColor( Qt::blue ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( Qt::white ) ) );
  gradientRamp->setStops( stops );

  mGradientFill->setColorRamp( gradientRamp );
  mGradientFill->setGradientColorType( QgsGradientFillSymbolLayerV2::ColorRamp );
  QVERIFY( imageCheck( "gradient_ramp" ) );
  mGradientFill->setGradientColorType( QgsGradientFillSymbolLayerV2::SimpleTwoColor );
}

void TestQgsGradients::gradientSymbolRadial()
{
  mReport += "<h2>Gradient symbol renderer radial test</h2>\n";
  mGradientFill->setGradientType( QgsGradientFillSymbolLayerV2::Radial );
  QVERIFY( imageCheck( "gradient_radial" ) );
  mGradientFill->setGradientType( QgsGradientFillSymbolLayerV2::Linear );
}

void TestQgsGradients::gradientSymbolConical()
{
  mReport += "<h2>Gradient symbol renderer conical test</h2>\n";
  mGradientFill->setGradientType( QgsGradientFillSymbolLayerV2::Conical );
  mGradientFill->setReferencePoint1( QPointF( 0.5, 0.5 ) );
  QVERIFY( imageCheck( "gradient_conical" ) );
  mGradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  mGradientFill->setGradientType( QgsGradientFillSymbolLayerV2::Linear );
}

void TestQgsGradients::gradientSymbolViewport()
{
  mReport += "<h2>Gradient symbol renderer viewport test</h2>\n";
  mGradientFill->setCoordinateMode( QgsGradientFillSymbolLayerV2::Viewport );
  QVERIFY( imageCheck( "gradient_viewport" ) );
  mGradientFill->setCoordinateMode( QgsGradientFillSymbolLayerV2::Feature );
}

void TestQgsGradients::gradientSymbolReferencePoints()
{
  mReport += "<h2>Gradient symbol renderer reference points test</h2>\n";
  mGradientFill->setReferencePoint1( QPointF( 0.5, 0.4 ) );
  mGradientFill->setReferencePoint2( QPointF( 0, 0.2 ) );
  QVERIFY( imageCheck( "gradient_refpoints" ) );
  mGradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolCentroid()
{
  mReport += "<h2>Gradient symbol renderer centroid reference point test</h2>\n";
  mGradientFill->setReferencePoint1IsCentroid( true );
  QVERIFY( imageCheck( "gradient_ref1centroid" ) );
  mGradientFill->setReferencePoint1IsCentroid( false );
  mGradientFill->setReferencePoint2IsCentroid( true );
  QVERIFY( imageCheck( "gradient_ref2centroid" ) );
  mGradientFill->setReferencePoint2IsCentroid( false );
}

void TestQgsGradients::gradientSymbolReflectSpread()
{
  mReport += "<h2>Gradient symbol renderer reflect spread test</h2>\n";
  mGradientFill->setReferencePoint2( QPointF( 0.5, 0.5 ) );
  mGradientFill->setGradientSpread( QgsGradientFillSymbolLayerV2::Reflect );
  QVERIFY( imageCheck( "gradient_reflect" ) );
  mGradientFill->setGradientSpread( QgsGradientFillSymbolLayerV2::Pad );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolRepeatSpread()
{
  mReport += "<h2>Gradient symbol renderer repeat spread test</h2>\n";
  mGradientFill->setReferencePoint2( QPointF( 0.5, 0.5 ) );
  mGradientFill->setGradientSpread( QgsGradientFillSymbolLayerV2::Repeat );
  QVERIFY( imageCheck( "gradient_repeat" ) );
  mGradientFill->setGradientSpread( QgsGradientFillSymbolLayerV2::Pad );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolRotate()
{
  mReport += "<h2>Gradient symbol renderer rotate test</h2>\n";
  mGradientFill->setAngle( 90 );
  QVERIFY( imageCheck( "gradient_rotate" ) );
  mGradientFill->setAngle( 0 );
}

void TestQgsGradients::gradientSymbolFromQml()
{
  mReport += "<h2>Gradient symbol from QML test</h2>\n";
  QVERIFY( setQml( "gradient" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );
  QVERIFY( imageCheck( "gradient_from_qml" ) );
}



//
// Private helper functions not called directly by CTest
//

bool TestQgsGradients::setQml( QString theType )
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

bool TestQgsGradients::imageCheck( QString theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( theTestType );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsGradients )
#include "testqgsgradients.moc"
