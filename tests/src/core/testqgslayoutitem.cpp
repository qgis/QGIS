/***************************************************************************
                         testqgslayoutitem.cpp
                         -----------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgstest.h"
#include "qgsproject.h"
#include <QObject>
#include <QPainter>
#include <QImage>
#include <QtTest/QSignalSpy>

class TestQgsLayoutItem: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsLayoutItem
    void registry();
    void shouldDrawDebug();
    void shouldDrawAntialiased();
    void preparePainter();
    void debugRect();
    void draw();
    void resize();
    void referencePoint();
    void itemPositionReferencePoint();
    void adjustPointForReference();
    void positionAtReferencePoint();
    void fixedSize();
    void minSize();
    void move();
    void positionWithUnits();
    void sizeWithUnits();
    void dataDefinedPosition();
    void dataDefinedSize();
    void combinedDataDefinedPositionAndSize();
    void rotation();

  private:

    //simple item for testing, since some methods in QgsLayoutItem are pure virtual
    class TestItem : public QgsLayoutItem
    {
      public:

        TestItem( QgsLayout *layout ) : QgsLayoutItem( layout ) {}
        ~TestItem() {}

        //implement pure virtual methods
        int type() const override { return QgsLayoutItemRegistry::LayoutItem + 101; }

      protected:
        void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * = nullptr ) override
        {
          QPainter *painter = context.painter();
          painter->save();
          painter->setRenderHint( QPainter::Antialiasing, false );
          painter->setPen( Qt::NoPen );
          painter->setBrush( QColor( 255, 100, 100, 200 ) );
          painter->drawRect( rect() );
          painter->restore();
        }
    };

    //item with minimum size
    class MinSizedItem : public TestItem
    {
      public:
        MinSizedItem( QgsLayout *layout ) : TestItem( layout )
        {
          setMinimumSize( QgsLayoutSize( 5.0, 10.0, QgsUnitTypes::LayoutCentimeters ) );
        }

        void updateMinSize( QgsLayoutSize size )
        {
          setMinimumSize( size );
        }

        ~MinSizedItem() {}
    };

    //item with fixed size
    class FixedSizedItem : public TestItem
    {
      public:

        FixedSizedItem( QgsLayout *layout ) : TestItem( layout )
        {
          setFixedSize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutInches ) );
        }

        void updateFixedSize( QgsLayoutSize size )
        {
          setFixedSize( size );
        }
        ~FixedSizedItem() {}
    };

    //item with both conflicting fixed and minimum size
    class FixedMinSizedItem : public TestItem
    {
      public:

        FixedMinSizedItem( QgsLayout *layout ) : TestItem( layout )
        {
          setFixedSize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutCentimeters ) );
          setMinimumSize( QgsLayoutSize( 5.0, 9.0, QgsUnitTypes::LayoutCentimeters ) );
        }
        ~FixedMinSizedItem() {}
    };

    QString mReport;

    bool renderCheck( QString testName, QImage &image, int mismatchCount );

};

void TestQgsLayoutItem::initTestCase()
{
  mReport = "<h1>Layout Item Tests</h1>\n";
}

void TestQgsLayoutItem::cleanupTestCase()
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

void TestQgsLayoutItem::init()
{

}

void TestQgsLayoutItem::cleanup()
{

}

void TestQgsLayoutItem::creation()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  TestItem *item = new TestItem( layout );
  QVERIFY( item );
  delete item;
  delete layout;
}

void TestQgsLayoutItem::registry()
{
  // test QgsLayoutItemRegistry
  QgsLayoutItemRegistry registry;

  // empty registry
  QVERIFY( !registry.itemMetadata( -1 ) );
  QVERIFY( registry.itemTypes().isEmpty() );
  QVERIFY( !registry.createItem( 1, nullptr ) );

  auto create = []( QgsLayout * layout, const QVariantMap & )->QgsLayoutItem*
  {
    return new TestItem( layout );
  };
  auto resolve = []( QVariantMap & props, const QgsPathResolver &, bool )
  {
    props.clear();
  };

  QSignalSpy spyTypeAdded( &registry, &QgsLayoutItemRegistry::typeAdded );

  QgsLayoutItemMetadata *metadata = new QgsLayoutItemMetadata( 2, QStringLiteral( "my type" ), QIcon(), create, resolve );
  QVERIFY( registry.addLayoutItemType( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 1 );
  QCOMPARE( spyTypeAdded.value( 0 ).at( 0 ).toInt(), 2 );
  QCOMPARE( spyTypeAdded.value( 0 ).at( 1 ).toString(), QStringLiteral( "my type" ) );
  // duplicate type id
  QVERIFY( !registry.addLayoutItemType( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 1 );

  //retrieve metadata
  QVERIFY( !registry.itemMetadata( -1 ) );
  QCOMPARE( registry.itemMetadata( 2 )->visibleName(), QStringLiteral( "my type" ) );
  QCOMPARE( registry.itemTypes().count(), 1 );
  QCOMPARE( registry.itemTypes().value( 2 ), QStringLiteral( "my type" ) );
  QgsLayoutItem *item = registry.createItem( 2, nullptr );
  QVERIFY( item );
  QVERIFY( dynamic_cast< TestItem *>( item ) );
  delete item;
  QVariantMap props;
  props.insert( QStringLiteral( "a" ), 5 );
  registry.resolvePaths( 1, props, QgsPathResolver(), true );
  QCOMPARE( props.size(), 1 );
  registry.resolvePaths( 2, props, QgsPathResolver(), true );
  QVERIFY( props.isEmpty() );

  //test populate
  QgsLayoutItemRegistry reg2;
  QVERIFY( reg2.itemTypes().isEmpty() );
  QVERIFY( reg2.populate() );
  QVERIFY( !reg2.itemTypes().isEmpty() );
  QVERIFY( !reg2.populate() );
}

void TestQgsLayoutItem::shouldDrawDebug()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem *item = new TestItem( &l );
  l.context().setFlag( QgsLayoutContext::FlagDebug, true );
  QVERIFY( item->shouldDrawDebugRect() );
  l.context().setFlag( QgsLayoutContext::FlagDebug, false );
  QVERIFY( !item->shouldDrawDebugRect() );
  delete item;
}

void TestQgsLayoutItem::shouldDrawAntialiased()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem *item = new TestItem( &l );
  l.context().setFlag( QgsLayoutContext::FlagAntialiasing, false );
  QVERIFY( !item->shouldDrawAntialiased() );
  l.context().setFlag( QgsLayoutContext::FlagAntialiasing, true );
  QVERIFY( item->shouldDrawAntialiased() );
  delete item;
}

void TestQgsLayoutItem::preparePainter()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem *item = new TestItem( &l );
  //test with no painter
  item->preparePainter( nullptr );

  //test antialiasing correctly set for painter
  QImage image( QSize( 100, 100 ), QImage::Format_ARGB32 );
  QPainter painter;
  painter.begin( &image );
  l.context().setFlag( QgsLayoutContext::FlagAntialiasing, false );
  item->preparePainter( &painter );
  QVERIFY( !( painter.renderHints() & QPainter::Antialiasing ) );
  l.context().setFlag( QgsLayoutContext::FlagAntialiasing, true );
  item->preparePainter( &painter );
  QVERIFY( painter.renderHints() & QPainter::Antialiasing );
  delete item;
}

void TestQgsLayoutItem::debugRect()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem *item = new TestItem( &l );
  l.addItem( item );
  item->setPos( 100, 100 );
  item->setRect( 0, 0, 200, 200 );
  l.setSceneRect( 0, 0, 400, 400 );
  l.context().setFlag( QgsLayoutContext::FlagDebug, true );
  QImage image( l.sceneRect().size().toSize(), QImage::Format_ARGB32 );
  image.fill( 0 );
  QPainter painter( &image );
  l.render( &painter );
  painter.end();

  bool result = renderCheck( "layoutitem_debugrect", image, 0 );
  QVERIFY( result );
}

void TestQgsLayoutItem::draw()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem *item = new TestItem( &l );
  l.addItem( item );
  item->setPos( 100, 100 );
  item->setRect( 0, 0, 200, 200 );
  l.setSceneRect( 0, 0, 400, 400 );
  l.context().setFlag( QgsLayoutContext::FlagAntialiasing, false ); //disable antialiasing to limit cross platform differences
  QImage image( l.sceneRect().size().toSize(), QImage::Format_ARGB32 );
  image.fill( 0 );
  QPainter painter( &image );
  l.render( &painter );
  painter.end();
  bool result = renderCheck( "layoutitem_draw", image, 0 );
  QVERIFY( result );
}

bool TestQgsLayoutItem::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator();
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( "layouts" );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsLayoutItem::positionWithUnits()
{
  QgsProject p;
  QgsLayout l( &p );

  TestItem *item = new TestItem( &l );
  item->attemptMove( QgsLayoutPoint( 60.0, 15.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->positionWithUnits().x(), 60.0 );
  QCOMPARE( item->positionWithUnits().y(), 15.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutMillimeters );
  item->attemptMove( QgsLayoutPoint( 50.0, 100.0, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->positionWithUnits().x(), 50.0 );
  QCOMPARE( item->positionWithUnits().y(), 100.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutPixels );
}

void TestQgsLayoutItem::sizeWithUnits()
{
  QgsProject p;
  QgsLayout l( &p );

  TestItem *item = new TestItem( &l );
  item->attemptResize( QgsLayoutSize( 60.0, 15.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 60.0 );
  QCOMPARE( item->sizeWithUnits().height(), 15.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutMillimeters );
  item->attemptResize( QgsLayoutSize( 50.0, 100.0, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->sizeWithUnits().width(), 50.0 );
  QCOMPARE( item->sizeWithUnits().height(), 100.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutPixels );

  delete item;
}

void TestQgsLayoutItem::dataDefinedPosition()
{
  QgsProject p;
  QgsLayout l( &p );

  //test setting data defined position
  TestItem *item = new TestItem( &l );
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  item->attemptMove( QgsLayoutPoint( 6.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  item->attemptResize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutCentimeters ) );

  // position x
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "4+7" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::PositionX );
  QCOMPARE( item->positionWithUnits().x(), 11.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 110.0 ); //mm

  //position y
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+3" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::PositionY );
  QCOMPARE( item->positionWithUnits().y(), 5.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().y(), 50.0 ); //mm

  //refreshPosition should also respect data defined positioning
  item->setPos( 0, 0 );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  item->refreshItemPosition();
  QCOMPARE( item->positionWithUnits().x(), 12.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 120.0 ); //mm
  QCOMPARE( item->scenePos().y(), 60.0 ); //mm

  //also check that data defined position overrides when attempting to move
  item->attemptMove( QgsLayoutPoint( 6.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->positionWithUnits().x(), 12.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 120.0 ); //mm
  QCOMPARE( item->scenePos().y(), 60.0 ); //mm
  //restriction only for x position
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty() );
  item->attemptMove( QgsLayoutPoint( 6.0, 1.5, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->positionWithUnits().x(), 12.0 );
  QCOMPARE( item->positionWithUnits().y(), 1.5 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 120.0 ); //mm
  QCOMPARE( item->scenePos().y(), 15.0 ); //mm
  //restriction only for y position
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  item->attemptMove( QgsLayoutPoint( 7.0, 1.5, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->positionWithUnits().x(), 7.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 70.0 ); //mm
  QCOMPARE( item->scenePos().y(), 60.0 ); //mm

  //check change of units should apply to data defined position
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  //first set to same as existing position, but with different units
  item->attemptMove( QgsLayoutPoint( 120.0, 60.0, QgsUnitTypes::LayoutMillimeters ) );
  //data defined position should utilize new units
  QCOMPARE( item->positionWithUnits().x(), 12.0 ); //mm
  QCOMPARE( item->positionWithUnits().y(), 6.0 ); //mm
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( item->scenePos().x(), 12.0 ); //mm
  QCOMPARE( item->scenePos().y(), 6.0 ); //mm

  //test that data defined position applies to item's reference point
  item->attemptMove( QgsLayoutPoint( 12.0, 6.0, QgsUnitTypes::LayoutCentimeters ) );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  QCOMPARE( item->positionWithUnits().x(), 12.0 ); //cm
  QCOMPARE( item->positionWithUnits().y(), 6.0 ); //cm
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 100.0 ); //mm
  QCOMPARE( item->scenePos().y(), 20.0 ); //mm

  //also check setting data defined position AFTER setting reference point
  item->setPos( 0, 0 );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "6+10" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+6" ) ) );
  item->refreshItemPosition();
  QCOMPARE( item->positionWithUnits().x(), 16.0 ); //cm
  QCOMPARE( item->positionWithUnits().y(), 8.0 ); //cm
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 140.0 ); //mm
  QCOMPARE( item->scenePos().y(), 40.0 ); //mm

  delete item;
}

void TestQgsLayoutItem::dataDefinedSize()
{
  QgsProject p;
  QgsLayout l( &p );

  //test setting data defined size
  TestItem *item = new TestItem( &l );
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  item->attemptMove( QgsLayoutPoint( 6.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  item->attemptResize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutCentimeters ) );

  //width
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "4+7" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::ItemWidth );
  QCOMPARE( item->sizeWithUnits().width(), 11.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 110.0 ); //mm

  //height
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "2+3" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::ItemHeight );
  QCOMPARE( item->sizeWithUnits().height(), 5.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().height(), 50.0 ); //mm

  //refreshSize should also respect data defined size
  item->setRect( 0.0, 0.0, 9.0, 8.0 );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  item->refreshItemSize();
  QCOMPARE( item->sizeWithUnits().width(), 12.0 );
  QCOMPARE( item->sizeWithUnits().height(), 6.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 120.0 ); //mm
  QCOMPARE( item->rect().height(), 60.0 ); //mm

  //also check that data defined size overrides when attempting to resize
  item->attemptResize( QgsLayoutSize( 6.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 12.0 );
  QCOMPARE( item->sizeWithUnits().height(), 6.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 120.0 ); //mm
  QCOMPARE( item->rect().height(), 60.0 ); //mm
  //restriction only for width
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty() );
  item->attemptResize( QgsLayoutSize( 6.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 12.0 );
  QCOMPARE( item->sizeWithUnits().height(), 1.5 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 120.0 ); //mm
  QCOMPARE( item->rect().height(), 15.0 ); //mm
  //restriction only for y position
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  item->attemptResize( QgsLayoutSize( 7.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 7.0 );
  QCOMPARE( item->sizeWithUnits().height(), 6.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 70.0 ); //mm
  QCOMPARE( item->rect().height(), 60.0 ); //mm

  //check change of units should apply to data defined size
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  //first set to same as existing size, but with different units
  item->attemptResize( QgsLayoutSize( 120.0, 60.0, QgsUnitTypes::LayoutMillimeters ) );
  //data defined size should utilize new units
  QCOMPARE( item->sizeWithUnits().width(), 12.0 ); //mm
  QCOMPARE( item->sizeWithUnits().height(), 6.0 ); //mm
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( item->rect().width(), 12.0 ); //mm
  QCOMPARE( item->rect().height(), 6.0 ); //mm

  //test that data defined size applies to item's reference point
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty() );
  item->attemptResize( QgsLayoutSize( 10.0, 5.0, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 20.0, 10.0, QgsUnitTypes::LayoutMillimeters ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "5" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "6" ) ) );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  item->refreshItemSize();
  QCOMPARE( item->scenePos().x(), 25.0 ); //mm
  QCOMPARE( item->scenePos().y(), 9.0 ); //mm

  //test that data defined size applied after setting item's reference point respects reference
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty() );
  item->setReferencePoint( QgsLayoutItem::UpperLeft );
  item->attemptResize( QgsLayoutSize( 10.0, 5.0, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 20.0, 10.0, QgsUnitTypes::LayoutMillimeters ) );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "7" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "9" ) ) );
  item->refreshItemSize();
  QCOMPARE( item->scenePos().x(), 23.0 ); //mm
  QCOMPARE( item->scenePos().y(), 6.0 ); //mm

  delete item;
}

void TestQgsLayoutItem::combinedDataDefinedPositionAndSize()
{
  QgsProject p;
  QgsLayout l( &p );

  //test setting data defined size
  TestItem *item = new TestItem( &l );
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  item->attemptMove( QgsLayoutPoint( 6.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  item->attemptResize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutCentimeters ) );

  //test item with all of data defined x, y, width, height set
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "4+7" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+3" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "4+9" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::AllProperties );
  QCOMPARE( item->positionWithUnits().x(), 11.0 );
  QCOMPARE( item->positionWithUnits().y(), 5.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->sizeWithUnits().width(), 13.0 );
  QCOMPARE( item->sizeWithUnits().height(), 6.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 110.0 ); //mm
  QCOMPARE( item->scenePos().y(), 50.0 ); //mm
  QCOMPARE( item->rect().width(), 130.0 ); //mm
  QCOMPARE( item->rect().height(), 60.0 ); //mm

  //also try with reference point set
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionX, QgsProperty::fromExpression( QStringLiteral( "4+8" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PositionY, QgsProperty::fromExpression( QStringLiteral( "2+4" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromExpression( QStringLiteral( "3+7" ) ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromExpression( QStringLiteral( "1+3" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::AllProperties );
  QCOMPARE( item->positionWithUnits().x(), 12.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->sizeWithUnits().width(), 10.0 );
  QCOMPARE( item->sizeWithUnits().height(), 4.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 70.0 ); //mm
  QCOMPARE( item->scenePos().y(), 40.0 ); //mm
  QCOMPARE( item->rect().width(), 100.0 ); //mm
  QCOMPARE( item->rect().height(), 40.0 ); //mm

  delete item;
}

//TODO rotation

void TestQgsLayoutItem::resize()
{
  QgsProject p;
  QgsLayout l( &p );

  //resize test item (no restrictions), same units as layout
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  TestItem *item = new TestItem( &l );
  item->setRect( 0, 0, 55, 45 );
  item->attemptMove( QgsLayoutPoint( 27, 29 ) );
  item->attemptResize( QgsLayoutSize( 100.0, 200.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->rect().width(), 100.0 );
  QCOMPARE( item->rect().height(), 200.0 );
  QCOMPARE( item->scenePos().x(), 27.0 ); //item should not move
  QCOMPARE( item->scenePos().y(), 29.0 );

  //test conversion of units
  l.setUnits( QgsUnitTypes::LayoutCentimeters );
  item->setRect( 0, 0, 100, 200 );
  item->attemptResize( QgsLayoutSize( 0.30, 0.45, QgsUnitTypes::LayoutMeters ) );
  QCOMPARE( item->rect().width(), 30.0 );
  QCOMPARE( item->rect().height(), 45.0 );

  //test pixel -> page conversion
  l.setUnits( QgsUnitTypes::LayoutInches );
  l.context().setDpi( 100.0 );
  item->refresh();
  item->setRect( 0, 0, 1, 2 );
  item->attemptResize( QgsLayoutSize( 140, 280, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->rect().width(), 1.4 );
  QCOMPARE( item->rect().height(), 2.8 );
  //changing the dpi should resize the item
  l.context().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->rect().width(), 0.7 );
  QCOMPARE( item->rect().height(), 1.4 );

  //test page -> pixel conversion
  l.setUnits( QgsUnitTypes::LayoutPixels );
  l.context().setDpi( 100.0 );
  item->refresh();
  item->setRect( 0, 0, 2, 2 );
  item->attemptResize( QgsLayoutSize( 1, 3, QgsUnitTypes::LayoutInches ) );
  QCOMPARE( item->rect().width(), 100.0 );
  QCOMPARE( item->rect().height(), 300.0 );
  //changing dpi results in item resize
  l.context().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->rect().width(), 200.0 );
  QCOMPARE( item->rect().height(), 600.0 );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );
}

void TestQgsLayoutItem::referencePoint()
{
  QgsProject p;
  QgsLayout l( &p );

  //test setting/getting reference point
  TestItem *item = new TestItem( &l );
  item->setReferencePoint( QgsLayoutItem::LowerMiddle );
  QCOMPARE( item->referencePoint(), QgsLayoutItem::LowerMiddle );

  //test that setting reference point results in positionWithUnits returning position at new reference
  //point
  item->setReferencePoint( QgsLayoutItem::UpperLeft );
  item->attemptResize( QgsLayoutSize( 2, 4 ) );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->positionWithUnits().x(), 1.0 );
  QCOMPARE( item->positionWithUnits().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperLeft );
  QCOMPARE( item->positionWithUnits().x(), 1.0 );
  QCOMPARE( item->positionWithUnits().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperMiddle );
  QCOMPARE( item->positionWithUnits().x(), 2.0 );
  QCOMPARE( item->positionWithUnits().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperRight );
  QCOMPARE( item->positionWithUnits().x(), 3.0 );
  QCOMPARE( item->positionWithUnits().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleLeft );
  QCOMPARE( item->positionWithUnits().x(), 1.0 );
  QCOMPARE( item->positionWithUnits().y(), 4.0 );
  item->setReferencePoint( QgsLayoutItem::Middle );
  QCOMPARE( item->positionWithUnits().x(), 2.0 );
  QCOMPARE( item->positionWithUnits().y(), 4.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleRight );
  QCOMPARE( item->positionWithUnits().x(), 3.0 );
  QCOMPARE( item->positionWithUnits().y(), 4.0 );
  item->setReferencePoint( QgsLayoutItem::LowerLeft );
  QCOMPARE( item->positionWithUnits().x(), 1.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  item->setReferencePoint( QgsLayoutItem::LowerMiddle );
  QCOMPARE( item->positionWithUnits().x(), 2.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  QCOMPARE( item->positionWithUnits().x(), 3.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );

  delete item;
  item = new TestItem( &l );

  //test that setting item position is done relative to reference point
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  item->attemptResize( QgsLayoutSize( 2, 4 ) );
  item->setReferencePoint( QgsLayoutItem::UpperLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), 1.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperMiddle );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), 0.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperRight );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), -1.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), 1.0 );
  QCOMPARE( item->scenePos().y(), 0.0 );
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), 0.0 );
  QCOMPARE( item->scenePos().y(), 0.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleRight );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), -1.0 );
  QCOMPARE( item->scenePos().y(), 0.0 );
  item->setReferencePoint( QgsLayoutItem::LowerLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), 1.0 );
  QCOMPARE( item->scenePos().y(), -2.0 );
  item->setReferencePoint( QgsLayoutItem::LowerMiddle );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), 0.0 );
  QCOMPARE( item->scenePos().y(), -2.0 );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->scenePos().x(), -1.0 );
  QCOMPARE( item->scenePos().y(), -2.0 );

  delete item;
  item = new TestItem( &l );

  //test that resizing is done relative to reference point
  item->attemptResize( QgsLayoutSize( 2, 4 ) );
  item->setReferencePoint( QgsLayoutItem::UpperLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->scenePos().x(), 1.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperMiddle );
  item->attemptResize( QgsLayoutSize( 6, 4 ) );
  QCOMPARE( item->scenePos().x(), 0.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperRight );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->scenePos().x(), 2.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleLeft );
  item->attemptResize( QgsLayoutSize( 6, 4 ) );
  QCOMPARE( item->scenePos().x(), 2.0 );
  QCOMPARE( item->scenePos().y(), 3.0 );
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->scenePos().x(), 3.0 );
  QCOMPARE( item->scenePos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleRight );
  item->attemptResize( QgsLayoutSize( 6, 4 ) );
  QCOMPARE( item->scenePos().x(), 1.0 );
  QCOMPARE( item->scenePos().y(), 3.0 );
  item->setReferencePoint( QgsLayoutItem::LowerLeft );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->scenePos().x(), 1.0 );
  QCOMPARE( item->scenePos().y(), 1.0 );
  item->setReferencePoint( QgsLayoutItem::LowerMiddle );
  item->attemptResize( QgsLayoutSize( 6, 4 ) );
  QCOMPARE( item->scenePos().x(), 0.0 );
  QCOMPARE( item->scenePos().y(), 3.0 );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->scenePos().x(), 2.0 );
  QCOMPARE( item->scenePos().y(), 1.0 );
}

void TestQgsLayoutItem::itemPositionReferencePoint()
{
  QgsProject p;
  QgsLayout l( &p );

  TestItem *item = new TestItem( &l );
  QPointF result = item->itemPositionAtReferencePoint( QgsLayoutItem::UpperLeft, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 0.0 );
  QCOMPARE( result.y(), 0.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::UpperMiddle, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 1.0 );
  QCOMPARE( result.y(), 0.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::UpperRight, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 2.0 );
  QCOMPARE( result.y(), 0.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::MiddleLeft, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 0.0 );
  QCOMPARE( result.y(), 2.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::Middle, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 1.0 );
  QCOMPARE( result.y(), 2.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::MiddleRight, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 2.0 );
  QCOMPARE( result.y(), 2.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::LowerLeft, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 0.0 );
  QCOMPARE( result.y(), 4.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::LowerMiddle, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 1.0 );
  QCOMPARE( result.y(), 4.0 );
  result = item->itemPositionAtReferencePoint( QgsLayoutItem::LowerRight, QSizeF( 2, 4 ) );
  QCOMPARE( result.x(), 2.0 );
  QCOMPARE( result.y(), 4.0 );

  delete item;
}

void TestQgsLayoutItem::adjustPointForReference()
{
  QgsProject p;
  QgsLayout l( &p );

  TestItem *item = new TestItem( &l );
  QPointF result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::UpperLeft );
  QCOMPARE( result.x(), 5.0 );
  QCOMPARE( result.y(), 7.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::UpperMiddle );
  QCOMPARE( result.x(), 4.0 );
  QCOMPARE( result.y(), 7.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::UpperRight );
  QCOMPARE( result.x(), 3.0 );
  QCOMPARE( result.y(), 7.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::MiddleLeft );
  QCOMPARE( result.x(), 5.0 );
  QCOMPARE( result.y(), 5.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::Middle );
  QCOMPARE( result.x(), 4.0 );
  QCOMPARE( result.y(), 5.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::MiddleRight );
  QCOMPARE( result.x(), 3.0 );
  QCOMPARE( result.y(), 5.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::LowerLeft );
  QCOMPARE( result.x(), 5.0 );
  QCOMPARE( result.y(), 3.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::LowerMiddle );
  QCOMPARE( result.x(), 4.0 );
  QCOMPARE( result.y(), 3.0 );
  result = item->adjustPointForReferencePosition( QPointF( 5, 7 ), QSizeF( 2, 4 ), QgsLayoutItem::LowerRight );
  QCOMPARE( result.x(), 3.0 );
  QCOMPARE( result.y(), 3.0 );
}

void TestQgsLayoutItem::positionAtReferencePoint()
{
  QgsProject p;
  QgsLayout l( &p );

  TestItem *item = new TestItem( &l );
  item->setPos( 8.0, 6.0 );
  item->setRect( 0.0, 0.0, 4.0, 6.0 );
  QPointF result = item->positionAtReferencePoint( QgsLayoutItem::UpperLeft );
  QCOMPARE( result.x(), 8.0 );
  QCOMPARE( result.y(), 6.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::UpperMiddle );
  QCOMPARE( result.x(), 10.0 );
  QCOMPARE( result.y(), 6.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::UpperRight );
  QCOMPARE( result.x(), 12.0 );
  QCOMPARE( result.y(), 6.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::MiddleLeft );
  QCOMPARE( result.x(), 8.0 );
  QCOMPARE( result.y(), 9.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::Middle );
  QCOMPARE( result.x(), 10.0 );
  QCOMPARE( result.y(), 9.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::MiddleRight );
  QCOMPARE( result.x(), 12.0 );
  QCOMPARE( result.y(), 9.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::LowerLeft );
  QCOMPARE( result.x(), 8.0 );
  QCOMPARE( result.y(), 12.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::LowerMiddle );
  QCOMPARE( result.x(), 10.0 );
  QCOMPARE( result.y(), 12.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::LowerRight );
  QCOMPARE( result.x(), 12.0 );
  QCOMPARE( result.y(), 12.0 );

  //test with a rotated item
  item->setItemRotation( 90 );
  result = item->positionAtReferencePoint( QgsLayoutItem::UpperLeft );
  QCOMPARE( result.x(), 13.0 );
  QCOMPARE( result.y(), 7.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::UpperMiddle );
  QCOMPARE( result.x(), 13.0 );
  QCOMPARE( result.y(), 9.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::UpperRight );
  QCOMPARE( result.x(), 13.0 );
  QCOMPARE( result.y(), 11.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::MiddleLeft );
  QCOMPARE( result.x(), 10.0 );
  QCOMPARE( result.y(), 7.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::Middle );
  QCOMPARE( result.x(), 10.0 );
  QCOMPARE( result.y(), 9.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::MiddleRight );
  QCOMPARE( result.x(), 10.0 );
  QCOMPARE( result.y(), 11.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::LowerLeft );
  QCOMPARE( result.x(), 7.0 );
  QCOMPARE( result.y(), 7.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::LowerMiddle );
  QCOMPARE( result.x(), 7.0 );
  QCOMPARE( result.y(), 9.0 );
  result = item->positionAtReferencePoint( QgsLayoutItem::LowerRight );
  QCOMPARE( result.x(), 7.0 );
  QCOMPARE( result.y(), 11.0 );

  delete item;
}

void TestQgsLayoutItem::fixedSize()
{
  QgsProject p;
  QgsLayout l( &p );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  FixedSizedItem *item = new FixedSizedItem( &l );
  QCOMPARE( item->fixedSize().width(), 2.0 );
  QCOMPARE( item->fixedSize().height(), 4.0 );
  QCOMPARE( item->fixedSize().units(), QgsUnitTypes::LayoutInches );

  item->setRect( 0, 0, 5.0, 6.0 ); //temporarily set rect to random size
  item->attemptResize( QgsLayoutSize( 7.0, 8.0, QgsUnitTypes::LayoutPoints ) );
  //check size matches fixed item size converted to mm
  QVERIFY( qgsDoubleNear( item->rect().width(), 2.0 * 25.4 ) );
  QVERIFY( qgsDoubleNear( item->rect().height(), 4.0 * 25.4 ) );

  //check that setting a fixed size applies this size immediately
  item->updateFixedSize( QgsLayoutSize( 150, 250, QgsUnitTypes::LayoutMillimeters ) );
  QVERIFY( qgsDoubleNear( item->rect().width(), 150.0 ) );
  QVERIFY( qgsDoubleNear( item->rect().height(), 250.0 ) );
}

void TestQgsLayoutItem::minSize()
{
  QgsProject p;
  QgsLayout l( &p );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  MinSizedItem *item = new MinSizedItem( &l );
  QCOMPARE( item->minimumSize().width(), 5.0 );
  QCOMPARE( item->minimumSize().height(), 10.0 );
  QCOMPARE( item->minimumSize().units(), QgsUnitTypes::LayoutCentimeters );

  item->setRect( 0, 0, 9.0, 6.0 ); //temporarily set rect to random size
  //try to resize to less than minimum size
  item->attemptResize( QgsLayoutSize( 1.0, 0.5, QgsUnitTypes::LayoutPoints ) );
  //check size matches min item size converted to mm
  QVERIFY( qgsDoubleNear( item->rect().width(), 50.0 ) );
  QVERIFY( qgsDoubleNear( item->rect().height(), 100.0 ) );

  //check that resize to larger than min size works
  item->attemptResize( QgsLayoutSize( 0.1, 0.2, QgsUnitTypes::LayoutMeters ) );
  QVERIFY( qgsDoubleNear( item->rect().width(), 100.0 ) );
  QVERIFY( qgsDoubleNear( item->rect().height(), 200.0 ) );

  //check that setting a minimum size applies this size immediately
  item->updateMinSize( QgsLayoutSize( 150, 250, QgsUnitTypes::LayoutMillimeters ) );
  QVERIFY( qgsDoubleNear( item->rect().width(), 150.0 ) );
  QVERIFY( qgsDoubleNear( item->rect().height(), 250.0 ) );

  //also need check that fixed size trumps min size
  FixedMinSizedItem *fixedMinItem = new FixedMinSizedItem( &l );
  QCOMPARE( fixedMinItem->minimumSize().width(), 5.0 );
  QCOMPARE( fixedMinItem->minimumSize().height(), 9.0 );
  QCOMPARE( fixedMinItem->minimumSize().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( fixedMinItem->fixedSize().width(), 2.0 );
  QCOMPARE( fixedMinItem->fixedSize().height(), 4.0 );
  QCOMPARE( fixedMinItem->fixedSize().units(), QgsUnitTypes::LayoutCentimeters );
  //try to resize to less than minimum size
  fixedMinItem->attemptResize( QgsLayoutSize( 1.0, 0.5, QgsUnitTypes::LayoutPoints ) );
  //check size matches fixed item size, not minimum size (converted to mm)
  QVERIFY( qgsDoubleNear( fixedMinItem->rect().width(), 50.0 ) );
  QVERIFY( qgsDoubleNear( fixedMinItem->rect().height(), 90.0 ) );
}

void TestQgsLayoutItem::move()
{
  QgsProject p;
  QgsLayout l( &p );

  //move test item, same units as layout
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  TestItem *item = new TestItem( &l );
  item->setRect( 0, 0, 55, 45 );
  item->setPos( 27, 29 );
  item->attemptMove( QgsLayoutPoint( 60.0, 15.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->rect().width(), 55.0 ); //size should not change
  QCOMPARE( item->rect().height(), 45.0 );
  QCOMPARE( item->scenePos().x(), 60.0 );
  QCOMPARE( item->scenePos().y(), 15.0 );

  //test conversion of units
  l.setUnits( QgsUnitTypes::LayoutCentimeters );
  item->setPos( 100, 200 );
  item->attemptMove( QgsLayoutPoint( 0.30, 0.45, QgsUnitTypes::LayoutMeters ) );
  QCOMPARE( item->scenePos().x(), 30.0 );
  QCOMPARE( item->scenePos().y(), 45.0 );

  //test pixel -> page conversion
  l.setUnits( QgsUnitTypes::LayoutInches );
  l.context().setDpi( 100.0 );
  item->refresh();
  item->setPos( 1, 2 );
  item->attemptMove( QgsLayoutPoint( 140, 280, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->scenePos().x(), 1.4 );
  QCOMPARE( item->scenePos().y(), 2.8 );
  //changing the dpi should move the item
  l.context().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->scenePos().x(), 0.7 );
  QCOMPARE( item->scenePos().y(), 1.4 );

  //test page -> pixel conversion
  l.setUnits( QgsUnitTypes::LayoutPixels );
  l.context().setDpi( 100.0 );
  item->refresh();
  item->setPos( 2, 2 );
  item->attemptMove( QgsLayoutPoint( 1, 3, QgsUnitTypes::LayoutInches ) );
  QCOMPARE( item->scenePos().x(), 100.0 );
  QCOMPARE( item->scenePos().y(), 300.0 );
  //changing dpi results in item move
  l.context().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->scenePos().x(), 200.0 );
  QCOMPARE( item->scenePos().y(), 600.0 );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );

  //TODO - reference points
}

void TestQgsLayoutItem::rotation()
{
  QgsProject proj;
  QgsLayout l( &proj );

  TestItem *item = new TestItem( &l );
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  item->setPos( 6.0, 10.0 );
  item->setRect( 0.0, 0.0, 10.0, 8.0 );
  item->setPen( Qt::NoPen );
  QRectF bounds = item->sceneBoundingRect();
  QCOMPARE( bounds.left(), 6.0 );
  QCOMPARE( bounds.right(), 16.0 );
  QCOMPARE( bounds.top(), 10.0 );
  QCOMPARE( bounds.bottom(), 18.0 );

  item->setItemRotation( 90.0 );
  QCOMPARE( item->itemRotation(), 90.0 );
  QCOMPARE( item->rotation(), 90.0 );
  bounds = item->sceneBoundingRect();
  QCOMPARE( bounds.left(), 7.0 );
  QCOMPARE( bounds.right(), 15.0 );
  QCOMPARE( bounds.top(), 9.0 );
  QCOMPARE( bounds.bottom(), 19.0 );

  //check that negative angles are preserved as negative
  item->setItemRotation( -90.0 );
  QCOMPARE( item->itemRotation(), -90.0 );
  QCOMPARE( item->rotation(), -90.0 );
  bounds = item->sceneBoundingRect();
  QCOMPARE( bounds.width(), 8.0 );
  QCOMPARE( bounds.height(), 10.0 );

  //check that rotating changes stored item position for reference point
  item->setItemRotation( 0.0 );
  item->attemptMove( QgsLayoutPoint( 5.0, 8.0 ) );
  item->attemptResize( QgsLayoutSize( 10.0, 6.0 ) );
  item->setItemRotation( 90.0 );
  QCOMPARE( item->positionWithUnits().x(), 13.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );

  //setting item position (for reference point) respects rotation
  item->attemptMove( QgsLayoutPoint( 10.0, 8.0 ) );
  QCOMPARE( item->scenePos().x(), 10.0 );
  QCOMPARE( item->scenePos().y(), 8.0 );
  QRectF p = item->sceneBoundingRect();
  qDebug() << p.left();
  QCOMPARE( item->sceneBoundingRect().left(), 4.0 );
  QCOMPARE( item->sceneBoundingRect().right(), 10.0 );
  QCOMPARE( item->sceneBoundingRect().top(), 8.0 );
  QCOMPARE( item->sceneBoundingRect().bottom(), 18.0 );

  //TODO also changing size?


  //data defined rotation
  item->setItemRotation( 0.0 );
  item->attemptMove( QgsLayoutPoint( 5.0, 8.0 ) );
  item->attemptResize( QgsLayoutSize( 10.0, 6.0 ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemRotation, QgsProperty::fromExpression( QStringLiteral( "90" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::ItemRotation );
  QCOMPARE( item->itemRotation(), 90.0 );
  //also check when refreshing all properties
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemRotation, QgsProperty::fromExpression( QStringLiteral( "45" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::AllProperties );
  QCOMPARE( item->itemRotation(), 45.0 );

  delete item;

  //render check
  item = new TestItem( &l );
  item->setItemRotation( 0.0 );
  item->setPos( 100, 150 );
  item->setRect( 0, 0, 200, 100 );
  l.addItem( item );
  item->setItemRotation( 45 );
  l.setSceneRect( 0, 0, 400, 400 );
  l.context().setFlag( QgsLayoutContext::FlagDebug, true );
  QImage image( l.sceneRect().size().toSize(), QImage::Format_ARGB32 );
  image.fill( 0 );
  QPainter painter( &image );
  l.render( &painter );
  painter.end();

  bool result = renderCheck( "layoutitem_rotation", image, 0 );
  delete item;
  QVERIFY( result );
}

//TODO rotation tests:
//restoring item from xml respects rotation/position
//rotate item around layout point

QGSTEST_MAIN( TestQgsLayoutItem )
#include "testqgslayoutitem.moc"
