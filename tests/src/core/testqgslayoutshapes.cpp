/***************************************************************************
                         testqgslayoutshapes.cpp
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemshape.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsreadwritecontext.h"
#include "qgsfillsymbol.h"
#include "qgslayout.h"

#include <QObject>
#include "qgstest.h"
#include <QColor>
#include <QPainter>

class TestQgsLayoutShapes : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutShapes() : QgsTest( QStringLiteral( "Layout Shape Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void rectangle(); //test if rectangle shape is functioning
    void triangle(); //test if triangle shape is functioning
    void ellipse(); //test if ellipse shape is functioning
    void roundedRectangle(); //test if rounded rectangle shape is functioning
    void symbol(); //test if styling shapes via symbol is working
    void readWriteXml();
    void bounds();
    void shapeRotation();
};

void TestQgsLayoutShapes::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayoutShapes::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutShapes::rectangle()
{
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  shape->attemptMove( QgsLayoutPoint( 20, 20 ) );
  shape->attemptResize( QgsLayoutSize( 150, 100 ) );


  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol( new QgsFillSymbol() );
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( QColor( 0, 0, 0 ) );
  simpleFill->setStrokeWidth( 0.3 );
  simpleFill->setPenJoinStyle( Qt::MiterJoin );
  shape->setSymbol( fillSymbol.get() );

  l.addLayoutItem( shape );

  QgsLayoutChecker checker( QStringLiteral( "composershapes_rectangle" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutShapes::triangle()
{
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  shape->setShapeType( QgsLayoutItemShape::Triangle );
  shape->attemptMove( QgsLayoutPoint( 20, 20 ) );
  shape->attemptResize( QgsLayoutSize( 150, 100 ) );


  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol( new QgsFillSymbol() );
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( QColor( 0, 0, 0 ) );
  simpleFill->setStrokeWidth( 0.3 );
  simpleFill->setPenJoinStyle( Qt::MiterJoin );
  shape->setSymbol( fillSymbol.get() );

  l.addLayoutItem( shape );

  QgsLayoutChecker checker( QStringLiteral( "composershapes_triangle" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutShapes::ellipse()
{
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  shape->setShapeType( QgsLayoutItemShape::Ellipse );
  shape->attemptMove( QgsLayoutPoint( 20, 20 ) );
  shape->attemptResize( QgsLayoutSize( 150, 100 ) );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol( new QgsFillSymbol() );
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( QColor( 0, 0, 0 ) );
  simpleFill->setStrokeWidth( 0.3 );
  simpleFill->setPenJoinStyle( Qt::MiterJoin );
  shape->setSymbol( fillSymbol.get() );

  l.addLayoutItem( shape );

  QgsLayoutChecker checker( QStringLiteral( "composershapes_ellipse" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutShapes::roundedRectangle()
{
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  shape->attemptMove( QgsLayoutPoint( 20, 20 ) );
  shape->attemptResize( QgsLayoutSize( 150, 100 ) );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol( new QgsFillSymbol() );
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( QColor( 0, 0, 0 ) );
  simpleFill->setStrokeWidth( 0.3 );
  simpleFill->setPenJoinStyle( Qt::MiterJoin );
  shape->setSymbol( fillSymbol.get() );

  l.addLayoutItem( shape );

  shape->setCornerRadius( QgsLayoutMeasurement( 30 ) );

  QgsLayoutChecker checker( QStringLiteral( "composershapes_roundedrect" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutShapes::symbol()
{
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  shape->attemptMove( QgsLayoutPoint( 20, 20 ) );
  shape->attemptResize( QgsLayoutSize( 150, 100 ) );

  //setup simple fill
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::green );
  simpleFill->setStrokeColor( Qt::yellow );
  simpleFill->setStrokeWidth( 6 );
  shape->setSymbol( fillSymbol );
  delete fillSymbol;

  l.addLayoutItem( shape );
  QgsLayoutChecker checker( QStringLiteral( "composershapes_symbol" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutShapes::readWriteXml()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemShape > shape = std::make_unique< QgsLayoutItemShape >( &l );
  shape->setShapeType( QgsLayoutItemShape::Triangle );
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::green );
  simpleFill->setStrokeColor( Qt::yellow );
  simpleFill->setStrokeWidth( 6 );
  shape->setSymbol( fillSymbol );
  delete fillSymbol;

  //save original item to xml
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );

  shape->writeXml( rootNode, doc, QgsReadWriteContext() );

  //create new item and restore settings from xml
  std::unique_ptr< QgsLayoutItemShape > copy = std::make_unique< QgsLayoutItemShape >( &l );
  QVERIFY( copy->readXml( rootNode.firstChildElement(), doc, QgsReadWriteContext() ) );
  QCOMPARE( copy->shapeType(), QgsLayoutItemShape::Triangle );
  QCOMPARE( copy->symbol()->symbolLayer( 0 )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( copy->symbol()->symbolLayer( 0 )->strokeColor().name(), QStringLiteral( "#ffff00" ) );
}

void TestQgsLayoutShapes::bounds()
{
  QgsProject p;
  QgsLayout l( &p );
  std::unique_ptr< QgsLayoutItemShape > shape = std::make_unique< QgsLayoutItemShape >( &l );
  shape->attemptMove( QgsLayoutPoint( 20, 20 ) );
  shape->attemptResize( QgsLayoutSize( 150, 100 ) );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::green );
  simpleFill->setStrokeColor( Qt::yellow );
  simpleFill->setStrokeWidth( 6 );
  shape->setSymbol( fillSymbol );
  delete fillSymbol;

  // scene bounding rect should include symbol outline
  QRectF bounds = shape->sceneBoundingRect();
  QCOMPARE( bounds.left(), 17.0 );
  QCOMPARE( bounds.right(), 173.0 );
  QCOMPARE( bounds.top(), 17.0 );
  QCOMPARE( bounds.bottom(), 123.0 );

  // rectWithFrame should include symbol outline too
  bounds = shape->rectWithFrame();
  QCOMPARE( bounds.left(), -3.0 );
  QCOMPARE( bounds.right(), 153.0 );
  QCOMPARE( bounds.top(), -3.0 );
  QCOMPARE( bounds.bottom(), 103.0 );
}

void TestQgsLayoutShapes::shapeRotation()
{
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  shape->attemptSetSceneRect( QRectF( 70, 70, 150, 100 ) );
  shape->setItemRotation( 45 );

  //setup simple fill
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( Qt::black );
  //simpleFill->setStrokeColor( Qt::yellow );
  //simpleFill->setStrokeWidth( 6 );
  shape->setSymbol( fillSymbol );
  delete fillSymbol;

  l.addLayoutItem( shape );
  QgsLayoutChecker checker( QStringLiteral( "composerrotation_shape" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

QGSTEST_MAIN( TestQgsLayoutShapes )
#include "testqgslayoutshapes.moc"
