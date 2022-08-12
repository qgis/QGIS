/***************************************************************************
     testqgssimplemarker.cpp
     -----------------------
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
#include "qgsproperty.h"
#include "qgsmarkersymbol.h"


/**
 * \ingroup UnitTests
 * This is a unit test for simple marker symbol types.
 */
class TestQgsSimpleMarkerSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsSimpleMarkerSymbol() : QgsTest( QStringLiteral( "Simple Marker Tests" ),
                                             QStringLiteral( "symbol_simplemarker" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void decodeShape_data();
    void decodeShape();
    void simpleMarkerSymbol();
    void simpleMarkerSymbolRotation();
    void simpleMarkerSymbolPreviewRotation();
    void simpleMarkerSymbolPreviewRotation_data();
    void simpleMarkerSymbolBevelJoin();
    void simpleMarkerSymbolMiterJoin();
    void simpleMarkerSymbolRoundJoin();
    void simpleMarkerSymbolCapStyle();
    void simpleMarkerOctagon();
    void simpleMarkerDecagon();
    void simpleMarkerDiamondStar();
    void simpleMarkerHeart();
    void simpleMarkerSquareWithCorners();
    void simpleMarkerRoundedSquare();
    void simpleMarkerShield();
    void simpleMarkerTrapezoid();
    void simpleMarkerParallelogramLeft();
    void simpleMarkerParallelogramRight();
    void simpleMarkerAsterisk();
    void bounds();
    void boundsWithOffset();
    void boundsWithRotation();
    void boundsWithRotationAndOffset();
    void colors();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();

  private:
    bool mTestHasError =  false ;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsSimpleMarkerSymbolLayer *mSimpleMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsSimpleMarkerSymbol::initTestCase()
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
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                      pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //setup symbol
  mSimpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mSimpleMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
  mMapSettings.setOutputDpi( 96 );
  mMapSettings.setExtent( mpPointsLayer->extent() );
}
void TestQgsSimpleMarkerSymbol::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSimpleMarkerSymbol::decodeShape_data()
{
  QTest::addColumn<QString>( "string" );
  QTest::addColumn<int>( "shape" );
  QTest::addColumn<bool>( "ok" );

  QTest::newRow( "empty string" ) << "" << static_cast< int >( Qgis::MarkerShape::Circle ) << false;
  QTest::newRow( "invalid character" ) << "@" << static_cast< int >( Qgis::MarkerShape::Circle ) << false;
  QTest::newRow( "square" ) << "square" << static_cast< int >( Qgis::MarkerShape::Square ) << true;
  QTest::newRow( "square case" ) << "SQUARE" << static_cast< int >( Qgis::MarkerShape::Square ) << true;
  QTest::newRow( "square case spaces" ) << "  SQUARE  " << static_cast< int >( Qgis::MarkerShape::Square ) << true;
  QTest::newRow( "square_with_corners" ) << "square_with_corners" << static_cast< int >( Qgis::MarkerShape::SquareWithCorners ) << true;
  QTest::newRow( "shield" ) << "shield" << static_cast< int >( Qgis::MarkerShape::Shield ) << true;
  QTest::newRow( "rounded_square" ) << "rounded_square" << static_cast< int >( Qgis::MarkerShape::RoundedSquare ) << true;
  QTest::newRow( "trapezoid" ) << "trapezoid" << static_cast< int >( Qgis::MarkerShape::Trapezoid ) << true;
  QTest::newRow( "parallelogram_left" ) << "parallelogram_left" << static_cast< int >( Qgis::MarkerShape::ParallelogramLeft ) << true;
  QTest::newRow( "rectangle" ) << "rectangle" << static_cast< int >( Qgis::MarkerShape::Square ) << true;
  QTest::newRow( "diamond" ) << "diamond" << static_cast< int >( Qgis::MarkerShape::Diamond ) << true;
  QTest::newRow( "pentagon" ) << "pentagon" << static_cast< int >( Qgis::MarkerShape::Pentagon ) << true;
  QTest::newRow( "hexagon" ) << "hexagon" << static_cast< int >( Qgis::MarkerShape::Hexagon ) << true;
  QTest::newRow( "octagon" ) << "octagon" << static_cast< int >( Qgis::MarkerShape::Octagon ) << true;
  QTest::newRow( "decagon" ) << "decagon" << static_cast< int >( Qgis::MarkerShape::Decagon ) << true;
  QTest::newRow( "triangle" ) << "triangle" << static_cast< int >( Qgis::MarkerShape::Triangle ) << true;
  QTest::newRow( "equilateral_triangle" ) << "equilateral_triangle" << static_cast< int >( Qgis::MarkerShape::EquilateralTriangle ) << true;
  QTest::newRow( "star_diamond" ) << "star_diamond" << static_cast< int >( Qgis::MarkerShape::DiamondStar ) << true;
  QTest::newRow( "star" ) << "star" << static_cast< int >( Qgis::MarkerShape::Star ) << true;
  QTest::newRow( "regular_star" ) << "regular_star" << static_cast< int >( Qgis::MarkerShape::Star ) << true;
  QTest::newRow( "heart" ) << "heart" << static_cast< int >( Qgis::MarkerShape::Heart ) << true;
  QTest::newRow( "arrow" ) << "arrow" << static_cast< int >( Qgis::MarkerShape::Arrow ) << true;
  QTest::newRow( "circle" ) << "circle" << static_cast< int >( Qgis::MarkerShape::Circle ) << true;
  QTest::newRow( "cross" ) << "cross" << static_cast< int >( Qgis::MarkerShape::Cross ) << true;
  QTest::newRow( "cross_fill" ) << "cross_fill" << static_cast< int >( Qgis::MarkerShape::CrossFill ) << true;
  QTest::newRow( "cross2" ) << "cross2" << static_cast< int >( Qgis::MarkerShape::Cross2 ) << true;
  QTest::newRow( "x" ) << "x" << static_cast< int >( Qgis::MarkerShape::Cross2 ) << true;
  QTest::newRow( "line" ) << "line" << static_cast< int >( Qgis::MarkerShape::Line ) << true;
  QTest::newRow( "arrowhead" ) << "arrowhead" << static_cast< int >( Qgis::MarkerShape::ArrowHead ) << true;
  QTest::newRow( "filled_arrowhead" ) << "filled_arrowhead" << static_cast< int >( Qgis::MarkerShape::ArrowHeadFilled ) << true;
  QTest::newRow( "semi_circle" ) << "semi_circle" << static_cast< int >( Qgis::MarkerShape::SemiCircle ) << true;
  QTest::newRow( "third_circle" ) << "third_circle" << static_cast< int >( Qgis::MarkerShape::ThirdCircle ) << true;
  QTest::newRow( "quarter_circle" ) << "quarter_circle" << static_cast< int >( Qgis::MarkerShape::QuarterCircle ) << true;
  QTest::newRow( "quarter_square" ) << "quarter_square" << static_cast< int >( Qgis::MarkerShape::QuarterSquare ) << true;
  QTest::newRow( "half_square" ) << "half_square" << static_cast< int >( Qgis::MarkerShape::HalfSquare ) << true;
  QTest::newRow( "diagonal_half_square" ) << "diagonal_half_square" << static_cast< int >( Qgis::MarkerShape::DiagonalHalfSquare ) << true;
  QTest::newRow( "right_half_triangle" ) << "right_half_triangle" << static_cast< int >( Qgis::MarkerShape::RightHalfTriangle ) << true;
  QTest::newRow( "left_half_triangle" ) << "left_half_triangle" << static_cast< int >( Qgis::MarkerShape::LeftHalfTriangle ) << true;
  QTest::newRow( "asterisk_fill" ) << "asterisk_fill" << static_cast< int >( Qgis::MarkerShape::AsteriskFill ) << true;
}

void TestQgsSimpleMarkerSymbol::decodeShape()
{
  QFETCH( QString, string );
  QFETCH( int, shape );
  QFETCH( bool, ok );

  bool res = false;
  QCOMPARE( static_cast< int >( QgsSimpleMarkerSymbolLayerBase::decodeShape( string, &res ) ), shape );
  QCOMPARE( res, ok );

  // round trip through encode
  QCOMPARE( static_cast< int >( QgsSimpleMarkerSymbolLayerBase::decodeShape( QgsSimpleMarkerSymbolLayerBase::encodeShape( static_cast< Qgis::MarkerShape >( shape ) ) ) ), shape );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbol()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Circle );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setStrokeWidth( 1 );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker" ), QStringLiteral( "simplemarker" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolRotation()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Square );
  mSimpleMarkerLayer->setSize( 15 );
  mSimpleMarkerLayer->setAngle( 45 );
  mSimpleMarkerLayer->setStrokeWidth( 0.2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::BevelJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_rotation" ), QStringLiteral( "simplemarker_rotation" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolPreviewRotation()
{
  QFETCH( QString, name );
  QFETCH( double, angle );
  QFETCH( QString, expression );
  QgsMarkerSymbol markerSymbol;
  QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  markerSymbol.changeSymbolLayer( 0, simpleMarkerLayer );

  simpleMarkerLayer->setShape( Qgis::MarkerShape::Arrow );
  simpleMarkerLayer->setAngle( angle );
  simpleMarkerLayer->setSize( 20 );
  simpleMarkerLayer->setColor( Qt::red );
  simpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromExpression( expression ) );

  QgsExpressionContext ec;
  const QImage image = markerSymbol.bigSymbolPreviewImage( &ec, Qgis::SymbolPreviewFlags() );
  QVERIFY( imageCheck( name, name, image, QString(), 500 ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolPreviewRotation_data()
{
  QTest::addColumn<QString>( "name" );
  QTest::addColumn<double>( "angle" );
  QTest::addColumn<QString>( "expression" );

  QTest::newRow( "field_based" ) << QStringLiteral( "field_based" ) << 20. << QStringLiteral( "orientation" ); // Should fallback to 20 because orientation is not available
  QTest::newRow( "static_expression" ) << QStringLiteral( "static_expression" ) << 20. << QStringLiteral( "40" ); // Should use 40 because expression has precedence
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolBevelJoin()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Triangle );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setAngle( 0 );
  mSimpleMarkerLayer->setStrokeWidth( 3 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::BevelJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_beveljoin" ), QStringLiteral( "simplemarker_beveljoin" ), mMapSettings ) );

}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolMiterJoin()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Triangle );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 3 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_miterjoin" ), QStringLiteral( "simplemarker_miterjoin" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolRoundJoin()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Triangle );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 3 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::RoundJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_roundjoin" ), QStringLiteral( "simplemarker_roundjoin" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolCapStyle()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::ArrowHead );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 3 );
  mSimpleMarkerLayer->setPenCapStyle( Qt::RoundCap );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_roundcap" ), QStringLiteral( "simplemarker_roundcap" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerOctagon()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Octagon );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_octagon" ), QStringLiteral( "simplemarker_octagon" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerDecagon()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Decagon );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_decagon" ), QStringLiteral( "simplemarker_decagon" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerDiamondStar()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::DiamondStar );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_diamondstar" ), QStringLiteral( "simplemarker_diamondstar" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerHeart()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Heart );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_heart" ), QStringLiteral( "simplemarker_heart" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSquareWithCorners()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::SquareWithCorners );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_square_with_corners" ), QStringLiteral( "simplemarker_square_with_corners" ), mMapSettings ) );

}

void TestQgsSimpleMarkerSymbol::simpleMarkerRoundedSquare()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::RoundedSquare );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_roundedsquare" ), QStringLiteral( "simplemarker_roundedsquare" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerShield()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Shield );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_shield" ), QStringLiteral( "simplemarker_shield" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerTrapezoid()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Trapezoid );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_trapezoid" ), QStringLiteral( "simplemarker_trapezoid" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerParallelogramLeft()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::ParallelogramLeft );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_parallelogramleft" ), QStringLiteral( "simplemarker_parallelogramleft" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerParallelogramRight()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::ParallelogramRight );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_parallelogramright" ), QStringLiteral( "simplemarker_parallelogramright" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerAsterisk()
{
  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setStrokeColor( Qt::black );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::AsteriskFill );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setStrokeWidth( 2 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( renderMapSettingsCheck( QStringLiteral( "simplemarker_asterisk" ), QStringLiteral( "simplemarker_asterisk" ), mMapSettings ) );
}

void TestQgsSimpleMarkerSymbol::bounds()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Circle );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );
  mSimpleMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = renderMapSettingsCheck( QStringLiteral( "simplemarker_bounds" ), QStringLiteral( "simplemarker_bounds" ), mMapSettings );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::boundsWithOffset()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Circle );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, '5,10', '10, 5')" ) ) );
  mSimpleMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = renderMapSettingsCheck( QStringLiteral( "simplemarker_boundsoffset" ), QStringLiteral( "simplemarker_boundsoffset" ), mMapSettings );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::boundsWithRotation()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Square );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromExpression( QStringLiteral( "importance * 20" ) ) );
  mSimpleMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = renderMapSettingsCheck( QStringLiteral( "simplemarker_boundsrotation" ), QStringLiteral( "simplemarker_boundsrotation" ), mMapSettings );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::boundsWithRotationAndOffset()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Square );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, '5,10', '10, 5')" ) ) );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromExpression( QStringLiteral( "heading" ) ) );
  mSimpleMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = renderMapSettingsCheck( QStringLiteral( "simplemarker_boundsrotationoffset" ), QStringLiteral( "simplemarker_boundsrotationoffset" ), mMapSettings );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty() );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::colors()
{
  //test logic for setting/retrieving symbol color

  QgsSimpleMarkerSymbolLayer marker;
  marker.setStrokeColor( QColor( 200, 200, 200 ) );
  marker.setFillColor( QColor( 100, 100, 100 ) );

  //start with a filled shape - color should be fill color
  marker.setShape( Qgis::MarkerShape::Circle );
  QCOMPARE( marker.color(), QColor( 100, 100, 100 ) );
  marker.setColor( QColor( 150, 150, 150 ) );
  QCOMPARE( marker.fillColor(), QColor( 150, 150, 150 ) );

  //now try with a non-filled (stroke only) shape - color should be stroke color
  marker.setShape( Qgis::MarkerShape::Cross );
  QCOMPARE( marker.color(), QColor( 200, 200, 200 ) );
  marker.setColor( QColor( 250, 250, 250 ) );
  QCOMPARE( marker.strokeColor(), QColor( 250, 250, 250 ) );
}

void TestQgsSimpleMarkerSymbol::opacityWithDataDefinedColor()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Square );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mSimpleMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setOpacity( 0.5 );

  const bool result = renderMapSettingsCheck( QStringLiteral( "simplemarker_opacityddcolor" ), QStringLiteral( "simplemarker_opacityddcolor" ), mMapSettings );

  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty() );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty() );
  mMarkerSymbol->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::dataDefinedOpacity()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( Qgis::MarkerShape::Square );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mSimpleMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  const bool result = renderMapSettingsCheck( QStringLiteral( "simplemarker_ddopacity" ), QStringLiteral( "simplemarker_ddopacity" ), mMapSettings );

  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty() );
  mSimpleMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty() );
  QVERIFY( result );
}


QGSTEST_MAIN( TestQgsSimpleMarkerSymbol )
#include "testqgssimplemarker.moc"
