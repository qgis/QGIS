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
#include "qgscolorrampimpl.h"
//qgis test includes
#include "qgsrenderchecker.h"
#include "qgsfillsymbol.h"

/**
 * \ingroup UnitTests
 * This is a unit test for gradient fill types.
 */
class TestQgsGradients : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsGradients()
      : QgsTest( QStringLiteral( "Gradient Renderer Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

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
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();
    void gradientSymbolFromQml();

  private:
    bool mTestHasError = false;
    bool setQml( const QString &type );
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QString mTestDataDir;
};


void TestQgsGradients::initTestCase()
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
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(), myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
}
void TestQgsGradients::cleanupTestCase()
{
  delete mpPolysLayer;

  QgsApplication::exitQgis();
}

void TestQgsGradients::gradientSymbol()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  QVERIFY( imageCheck( "gradient" ) );
}

void TestQgsGradients::gradientSymbolColors()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  gradientFill->setColor( QColor( "green" ) );
  gradientFill->setColor2( QColor( "white" ) );
  QVERIFY( imageCheck( "gradient_colors" ) );
}

void TestQgsGradients::gradientSymbolRamp()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );

  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( Qt::white ) ) );
  gradientRamp->setStops( stops );

  gradientFill->setColorRamp( gradientRamp );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::ColorRamp );
  QVERIFY( imageCheck( "gradient_ramp" ) );
}

void TestQgsGradients::gradientSymbolRadial()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  gradientFill->setGradientType( Qgis::GradientType::Radial );

  QVERIFY( imageCheck( "gradient_radial" ) );
}

void TestQgsGradients::gradientSymbolConical()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  gradientFill->setGradientType( Qgis::GradientType::Conical );
  gradientFill->setReferencePoint1( QPointF( 0.5, 0.5 ) );

  QVERIFY( imageCheck( "gradient_conical" ) );
}

void TestQgsGradients::gradientSymbolViewport()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );

  QVERIFY( imageCheck( "gradient_viewport" ) );
}

void TestQgsGradients::gradientSymbolReferencePoints()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setReferencePoint1( QPointF( 0.5, 0.4 ) );
  gradientFill->setReferencePoint2( QPointF( 0, 0.2 ) );

  QVERIFY( imageCheck( "gradient_refpoints" ) );
}

void TestQgsGradients::gradientSymbolCentroid()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );

  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );

  gradientFill->setReferencePoint1IsCentroid( true );
  QVERIFY( imageCheck( "gradient_ref1centroid" ) );
  gradientFill->setReferencePoint1IsCentroid( false );
  gradientFill->setReferencePoint2IsCentroid( true );
  QVERIFY( imageCheck( "gradient_ref2centroid" ) );
  gradientFill->setReferencePoint2IsCentroid( false );
}

void TestQgsGradients::gradientSymbolReflectSpread()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );

  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 0.5, 0.5 ) );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Reflect );
  QVERIFY( imageCheck( "gradient_reflect" ) );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolRepeatSpread()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 0.5, 0.5 ) );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Repeat );

  QVERIFY( imageCheck( "gradient_repeat" ) );
}

void TestQgsGradients::gradientSymbolRotate()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );

  gradientFill->setAngle( 90 );
  QVERIFY( imageCheck( "gradient_rotate" ) );
}

void TestQgsGradients::opacityWithDataDefinedColor()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );

  gradientFill->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  gradientFill->setDataDefinedProperty( QgsSymbolLayer::Property::SecondaryColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  fillSymbol->setOpacity( 0.5 );

  const bool result = imageCheck( QStringLiteral( "gradient_opacityddcolor" ) );
  QVERIFY( result );
}

void TestQgsGradients::dataDefinedOpacity()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  gradientFill->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  gradientFill->setDataDefinedProperty( QgsSymbolLayer::Property::SecondaryColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  fillSymbol->setOpacity( 1.0 );
  fillSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Value\" >10, 25, 50)" ) ) );

  const bool result = imageCheck( QStringLiteral( "gradient_ddopacity" ) );
  QVERIFY( result );
}

void TestQgsGradients::gradientSymbolFromQml()
{
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );
  mpPolysLayer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVERIFY( setQml( "gradient" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );
  QVERIFY( imageCheck( "gradient_from_qml" ) );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsGradients::setQml( const QString &type )
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  const QString myFileName = mTestDataDir + "polys_" + type + "_symbol.qml";
  bool myStyleFlag = false;
  const QString error = mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
  }
  return myStyleFlag;
}

bool TestQgsGradients::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_gradient" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  const bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsGradients )
#include "testqgsgradients.moc"
