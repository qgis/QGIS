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
    TestQgsGradients() : QgsTest( QStringLiteral( "Gradient Renderer Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

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
    bool mTestHasError =  false ;
    bool setQml( const QString &type );
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsGradientFillSymbolLayer *mGradientFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
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
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  mGradientFill = new QgsGradientFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mGradientFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

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
  mGradientFill->setColor( QColor( "red" ) );
  mGradientFill->setColor2( QColor( "blue" ) );
  mGradientFill->setGradientType( Qgis::GradientType::Linear );
  mGradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  mGradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  mGradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  mGradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  QVERIFY( imageCheck( "gradient" ) );
}

void TestQgsGradients::gradientSymbolColors()
{
  mGradientFill->setColor( QColor( "green" ) );
  mGradientFill->setColor2( QColor( "white" ) );
  QVERIFY( imageCheck( "gradient_colors" ) );
  mGradientFill->setColor( QColor( "red" ) );
  mGradientFill->setColor2( QColor( "blue" ) );
}

void TestQgsGradients::gradientSymbolRamp()
{
  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( Qt::white ) ) );
  gradientRamp->setStops( stops );

  mGradientFill->setColorRamp( gradientRamp );
  mGradientFill->setGradientColorType( Qgis::GradientColorSource::ColorRamp );
  QVERIFY( imageCheck( "gradient_ramp" ) );
  mGradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
}

void TestQgsGradients::gradientSymbolRadial()
{
  mGradientFill->setGradientType( Qgis::GradientType::Radial );
  QVERIFY( imageCheck( "gradient_radial" ) );
  mGradientFill->setGradientType( Qgis::GradientType::Linear );
}

void TestQgsGradients::gradientSymbolConical()
{
  mGradientFill->setGradientType( Qgis::GradientType::Conical );
  mGradientFill->setReferencePoint1( QPointF( 0.5, 0.5 ) );
  QVERIFY( imageCheck( "gradient_conical" ) );
  mGradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  mGradientFill->setGradientType( Qgis::GradientType::Linear );
}

void TestQgsGradients::gradientSymbolViewport()
{
  mGradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );
  QVERIFY( imageCheck( "gradient_viewport" ) );
  mGradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
}

void TestQgsGradients::gradientSymbolReferencePoints()
{
  mGradientFill->setReferencePoint1( QPointF( 0.5, 0.4 ) );
  mGradientFill->setReferencePoint2( QPointF( 0, 0.2 ) );
  QVERIFY( imageCheck( "gradient_refpoints" ) );
  mGradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolCentroid()
{
  mGradientFill->setReferencePoint1IsCentroid( true );
  QVERIFY( imageCheck( "gradient_ref1centroid" ) );
  mGradientFill->setReferencePoint1IsCentroid( false );
  mGradientFill->setReferencePoint2IsCentroid( true );
  QVERIFY( imageCheck( "gradient_ref2centroid" ) );
  mGradientFill->setReferencePoint2IsCentroid( false );
}

void TestQgsGradients::gradientSymbolReflectSpread()
{
  mGradientFill->setReferencePoint2( QPointF( 0.5, 0.5 ) );
  mGradientFill->setGradientSpread( Qgis::GradientSpread::Reflect );
  QVERIFY( imageCheck( "gradient_reflect" ) );
  mGradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolRepeatSpread()
{
  mGradientFill->setReferencePoint2( QPointF( 0.5, 0.5 ) );
  mGradientFill->setGradientSpread( Qgis::GradientSpread::Repeat );
  QVERIFY( imageCheck( "gradient_repeat" ) );
  mGradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  mGradientFill->setReferencePoint2( QPointF( 1, 1 ) );
}

void TestQgsGradients::gradientSymbolRotate()
{
  mGradientFill->setAngle( 90 );
  QVERIFY( imageCheck( "gradient_rotate" ) );
  mGradientFill->setAngle( 0 );
}

void TestQgsGradients::opacityWithDataDefinedColor()
{
  mGradientFill->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mGradientFill->setDataDefinedProperty( QgsSymbolLayer::PropertySecondaryColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mFillSymbol->setOpacity( 0.5 );

  const bool result = imageCheck( QStringLiteral( "gradient_opacityddcolor" ) );
  QVERIFY( result );
}

void TestQgsGradients::dataDefinedOpacity()
{
  mGradientFill->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mGradientFill->setDataDefinedProperty( QgsSymbolLayer::PropertySecondaryColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mFillSymbol->setOpacity( 1.0 );
  mFillSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Value\" >10, 25, 50)" ) ) );

  const bool result = imageCheck( QStringLiteral( "gradient_ddopacity" ) );
  QVERIFY( result );
}

void TestQgsGradients::gradientSymbolFromQml()
{
  QVERIFY( setQml( "gradient" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
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
