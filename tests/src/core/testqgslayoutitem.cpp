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
#include "qgsreadwritecontext.h"
#include "qgslayoutitemundocommand.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemshape.h"
#include "qgslayouteffect.h"
#include "qgsfillsymbollayer.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbol.h"

#include <QObject>
#include <QPainter>
#include <QImage>
#include <QtTest/QSignalSpy>


//simple item for testing, since some methods in QgsLayoutItem are pure virtual
class TestItem : public QgsLayoutItem
{
    Q_OBJECT

  public:

    TestItem( QgsLayout *layout ) : QgsLayoutItem( layout )
    {
      setFrameEnabled( false );
      setBackgroundEnabled( false );
    }

    //implement pure virtual methods
    int type() const override { return QgsLayoutItemRegistry::LayoutItem + 101; }

    bool forceResize = false;

  protected:
    void draw( QgsLayoutItemRenderContext &context ) override
    {
      QPainter *painter = context.renderContext().painter();
      painter->save();
      painter->setRenderHint( QPainter::Antialiasing, false );
      painter->setPen( Qt::NoPen );
      painter->setBrush( QColor( 255, 100, 100, 200 ) );
      painter->drawRect( rect() );
      painter->restore();
    }

    QSizeF applyItemSizeConstraint( QSizeF targetSize ) override
    {
      if ( !forceResize )
        return targetSize;

      return QSizeF( 17, 27 );
    }
};

//item with minimum size
class MinSizedItem : public TestItem
{
    Q_OBJECT

  public:
    MinSizedItem( QgsLayout *layout ) : TestItem( layout )
    {
      setMinimumSize( QgsLayoutSize( 5.0, 10.0, QgsUnitTypes::LayoutCentimeters ) );
    }

    void updateMinSize( QgsLayoutSize size )
    {
      setMinimumSize( size );
    }

};

//item with fixed size
class FixedSizedItem : public TestItem
{
    Q_OBJECT

  public:

    FixedSizedItem( QgsLayout *layout ) : TestItem( layout )
    {
      setFixedSize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutInches ) );
    }

    void updateFixedSize( QgsLayoutSize size )
    {
      setFixedSize( size );
    }
};

//item with both conflicting fixed and minimum size
class FixedMinSizedItem : public TestItem
{
    Q_OBJECT

  public:

    FixedMinSizedItem( QgsLayout *layout ) : TestItem( layout )
    {
      setFixedSize( QgsLayoutSize( 2.0, 4.0, QgsUnitTypes::LayoutCentimeters ) );
      setMinimumSize( QgsLayoutSize( 5.0, 9.0, QgsUnitTypes::LayoutCentimeters ) );
    }
};


class TestQgsLayoutItem: public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutItem() : QgsTest( QStringLiteral( "Layout Item Tests" ) ) {}

  private slots:
    void creation(); //test creation of QgsLayoutItem
    void uuid();
    void id();
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
    void writeXml();
    void readXml();
    void writeReadXmlProperties();
    void undoRedo();
    void multiItemUndo();
    void overlappingUndo();
    void blendMode();
    void opacity();
    void excludeFromExports();
    void setSceneRect();
    void page();
    void itemVariablesFunction();
    void variables();
    void mapCreditsFunction();

  private:

    bool renderCheck( QString testName, QImage &image, int mismatchCount );

    std::unique_ptr< QgsLayoutItem > createCopyViaXml( QgsLayout *layout, QgsLayoutItem *original );

};

void TestQgsLayoutItem::creation()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  TestItem *item = new TestItem( layout );
  QVERIFY( item );
  delete item;
  delete layout;
}

void TestQgsLayoutItem::uuid()
{
  QgsProject p;
  QgsLayout l( &p );

  //basic test of uuid
  const TestItem item( &l );
  const TestItem item2( &l );
  QVERIFY( item.uuid() != item2.uuid() );
}

void TestQgsLayoutItem::id()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem item( &l );
  item.setId( QStringLiteral( "test" ) );
  QCOMPARE( item.id(), QStringLiteral( "test" ) );
}

void TestQgsLayoutItem::registry()
{
  // test QgsLayoutItemRegistry
  QgsLayoutItemRegistry registry;

  // empty registry
  QVERIFY( !registry.itemMetadata( -1 ) );
  QVERIFY( registry.itemTypes().isEmpty() );
  QVERIFY( !registry.createItem( 1, nullptr ) );

  auto create = []( QgsLayout * layout )->QgsLayoutItem *
  {
    return new TestItem( layout );
  };
  auto resolve = []( QVariantMap & props, const QgsPathResolver &, bool )
  {
    props.clear();
  };

  const QSignalSpy spyTypeAdded( &registry, &QgsLayoutItemRegistry::typeAdded );

  QgsLayoutItemMetadata *metadata = new QgsLayoutItemMetadata( 2, QStringLiteral( "my type" ), QStringLiteral( "my types" ), create, resolve );
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
  QCOMPARE( registry.itemMetadata( 2 )->visiblePluralName(), QStringLiteral( "my types" ) );
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
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagDebug, true );
  QVERIFY( item->shouldDrawDebugRect() );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagDebug, false );
  QVERIFY( !item->shouldDrawDebugRect() );
  delete item;
}

void TestQgsLayoutItem::shouldDrawAntialiased()
{
  QgsProject p;
  QgsLayout l( &p );
  TestItem *item = new TestItem( &l );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing, false );
  QVERIFY( !item->shouldDrawAntialiased() );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing, true );
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
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing, false );
  item->preparePainter( &painter );
  QVERIFY( !( painter.renderHints() & QPainter::Antialiasing ) );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing, true );
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
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagDebug, true );
  QImage image( l.sceneRect().size().toSize(), QImage::Format_ARGB32 );
  image.fill( 0 );
  QPainter painter( &image );
  l.render( &painter );
  painter.end();

  const bool result = renderCheck( QStringLiteral( "layoutitem_debugrect" ), image, 0 );
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
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing, false ); //disable antialiasing to limit cross platform differences
  QImage image( l.sceneRect().size().toSize(), QImage::Format_ARGB32 );
  image.fill( 0 );
  QPainter painter( &image );
  l.render( &painter );
  painter.end();
  const bool result = renderCheck( QStringLiteral( "layoutitem_draw" ), image, 0 );
  QVERIFY( result );
}

bool TestQgsLayoutItem::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  const QString myTmpDir = QDir::tempPath() + QDir::separator();
  const QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "layouts" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  const bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsLayoutItem::positionWithUnits()
{
  QgsProject p;
  QgsLayout l( &p );

  std::unique_ptr< TestItem > item( new TestItem( &l ) );
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

  // data defined page size
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PresetPaperSize, QgsProperty::fromValue( QStringLiteral( "A5" ) ) );
  item->attemptResize( QgsLayoutSize( 7.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 14.8 );
  QCOMPARE( item->sizeWithUnits().height(), 21.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 148.0 ); //mm
  QCOMPARE( item->rect().height(), 210.0 ); //mm
  // data defined height/width should override page size
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty::fromValue( "13.0" ) );
  item->attemptResize( QgsLayoutSize( 7.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 13.0 );
  QCOMPARE( item->sizeWithUnits().height(), 21.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 130.0 ); //mm
  QCOMPARE( item->rect().height(), 210.0 ); //mm
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty::fromValue( "3.0" ) );
  item->attemptResize( QgsLayoutSize( 7.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 13.0 );
  QCOMPARE( item->sizeWithUnits().height(), 3.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 130.0 ); //mm
  QCOMPARE( item->rect().height(), 30.0 ); //mm
  // data defined orientation
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PaperOrientation, QgsProperty::fromValue( "portrait" ) );
  item->attemptResize( QgsLayoutSize( 7.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 3.0 );
  QCOMPARE( item->sizeWithUnits().height(), 13.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 30.0 ); //mm
  QCOMPARE( item->rect().height(), 130.0 ); //mm

  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemWidth, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemHeight, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PresetPaperSize, QgsProperty() );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PaperOrientation, QgsProperty::fromValue( "landscape" ) );
  item->attemptResize( QgsLayoutSize( 1.0, 1.50, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->sizeWithUnits().width(), 1.5 );
  QCOMPARE( item->sizeWithUnits().height(), 1.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->rect().width(), 15.0 ); //mm
  QCOMPARE( item->rect().height(), 10.0 ); //mm
  item->dataDefinedProperties().setProperty( QgsLayoutObject::PaperOrientation, QgsProperty() );

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
  std::unique_ptr< TestItem > item( new TestItem( &l ) );
  const QSignalSpy spySizeChanged( item.get(), &QgsLayoutItem::sizePositionChanged );

  item->setRect( 0, 0, 55, 45 );
  item->attemptMove( QgsLayoutPoint( 27, 29 ) );
  QCOMPARE( spySizeChanged.count(), 1 );
  item->attemptResize( QgsLayoutSize( 100.0, 200.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( spySizeChanged.count(), 2 );
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
  QCOMPARE( spySizeChanged.count(), 4 );

  //test pixel -> page conversion
  l.setUnits( QgsUnitTypes::LayoutInches );
  l.renderContext().setDpi( 100.0 );
  item->refresh();
  QCOMPARE( spySizeChanged.count(), 6 );
  item->setRect( 0, 0, 1, 2 );
  item->attemptResize( QgsLayoutSize( 140, 280, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->rect().width(), 1.4 );
  QCOMPARE( item->rect().height(), 2.8 );
  QCOMPARE( spySizeChanged.count(), 7 );
  //changing the dpi should resize the item
  l.renderContext().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->rect().width(), 0.7 );
  QCOMPARE( item->rect().height(), 1.4 );
  QCOMPARE( spySizeChanged.count(), 8 );

  //test page -> pixel conversion
  l.setUnits( QgsUnitTypes::LayoutPixels );
  l.renderContext().setDpi( 100.0 );
  item->refresh();
  item->setRect( 0, 0, 2, 2 );
  QCOMPARE( spySizeChanged.count(), 10 );
  item->attemptResize( QgsLayoutSize( 1, 3, QgsUnitTypes::LayoutInches ) );
  QCOMPARE( item->rect().width(), 100.0 );
  QCOMPARE( item->rect().height(), 300.0 );
  QCOMPARE( spySizeChanged.count(), 11 );
  //changing dpi results in item resize
  l.renderContext().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->rect().width(), 200.0 );
  QCOMPARE( item->rect().height(), 600.0 );
  QCOMPARE( spySizeChanged.count(), 13 );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );

  // try override item size in item
  item->forceResize = true;
  item->attemptResize( QgsLayoutSize( 10.0, 15.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->rect().width(), 17.0 );
  QCOMPARE( item->rect().height(), 27.0 );
}

void TestQgsLayoutItem::referencePoint()
{
  QgsProject p;
  QgsLayout l( &p );

  //test setting/getting reference point
  std::unique_ptr< TestItem > item( new TestItem( &l ) );
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

  item.reset( new TestItem( &l ) );

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

  item.reset( new TestItem( &l ) );

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

  item.reset( new TestItem( &l ) );

  //item with frame
  item->attemptResize( QgsLayoutSize( 2, 4 ) );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  item->setFrameEnabled( true );
  item->setFrameStrokeWidth( QgsLayoutMeasurement( 1 ) );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->sizeWithUnits().width(), 4.0 );
  QCOMPARE( item->sizeWithUnits().height(), 6.0 );
  item->attemptResize( QgsLayoutSize( 4, 6 ), true );
  QCOMPARE( item->sizeWithUnits().width(), 3.0 );
  QCOMPARE( item->sizeWithUnits().height(), 5.0 );
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

  std::unique_ptr< TestItem > item( new TestItem( &l ) );
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
  std::unique_ptr< FixedSizedItem > item( new FixedSizedItem( &l ) );
  QCOMPARE( item->fixedSize().width(), 2.0 );
  QCOMPARE( item->fixedSize().height(), 4.0 );
  QCOMPARE( item->fixedSize().units(), QgsUnitTypes::LayoutInches );

  item->setRect( 0, 0, 5.0, 6.0 ); //temporarily set rect to random size
  item->attemptResize( QgsLayoutSize( 7.0, 8.0, QgsUnitTypes::LayoutPoints ) );
  //check size matches fixed item size converted to mm
  QGSCOMPARENEAR( item->rect().width(), 2.0 * 25.4, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( item->rect().height(), 4.0 * 25.4, 4 * std::numeric_limits<double>::epsilon() );

  item->attemptResize( QgsLayoutSize( 7.0, 8.0, QgsUnitTypes::LayoutInches ) );
  //check size matches fixed item size converted to mm
  QGSCOMPARENEAR( item->rect().width(), 2.0 * 25.4, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( item->rect().height(), 4.0 * 25.4, 4 * std::numeric_limits<double>::epsilon() );

  //check that setting a fixed size applies this size immediately
  item->updateFixedSize( QgsLayoutSize( 150, 250, QgsUnitTypes::LayoutMillimeters ) );
  QGSCOMPARENEAR( item->rect().width(), 150.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( item->rect().height(), 250.0, 4 * std::numeric_limits<double>::epsilon() );
}

void TestQgsLayoutItem::minSize()
{
  QgsProject p;
  QgsLayout l( &p );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  std::unique_ptr< MinSizedItem > item( new MinSizedItem( &l ) );
  QCOMPARE( item->minimumSize().width(), 5.0 );
  QCOMPARE( item->minimumSize().height(), 10.0 );
  QCOMPARE( item->minimumSize().units(), QgsUnitTypes::LayoutCentimeters );

  item->setRect( 0, 0, 9.0, 6.0 ); //temporarily set rect to random size
  //try to resize to less than minimum size
  item->attemptResize( QgsLayoutSize( 1.0, 0.5, QgsUnitTypes::LayoutPoints ) );
  //check size matches min item size converted to mm
  QGSCOMPARENEAR( item->rect().width(), 50.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( item->rect().height(), 100.0, 4 * std::numeric_limits<double>::epsilon() );

  //check that resize to larger than min size works
  item->attemptResize( QgsLayoutSize( 0.1, 0.2, QgsUnitTypes::LayoutMeters ) );
  QGSCOMPARENEAR( item->rect().width(), 100.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( item->rect().height(), 200.0, 4 * std::numeric_limits<double>::epsilon() );

  //check that setting a minimum size applies this size immediately
  item->updateMinSize( QgsLayoutSize( 150, 250, QgsUnitTypes::LayoutMillimeters ) );
  QGSCOMPARENEAR( item->rect().width(), 150.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( item->rect().height(), 250.0, 4 * std::numeric_limits<double>::epsilon() );

  //also need check that fixed size trumps min size
  std::unique_ptr< FixedMinSizedItem > fixedMinItem( new FixedMinSizedItem( &l ) );
  QCOMPARE( fixedMinItem->minimumSize().width(), 5.0 );
  QCOMPARE( fixedMinItem->minimumSize().height(), 9.0 );
  QCOMPARE( fixedMinItem->minimumSize().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( fixedMinItem->fixedSize().width(), 2.0 );
  QCOMPARE( fixedMinItem->fixedSize().height(), 4.0 );
  QCOMPARE( fixedMinItem->fixedSize().units(), QgsUnitTypes::LayoutCentimeters );
  //try to resize to less than minimum size
  fixedMinItem->attemptResize( QgsLayoutSize( 1.0, 0.5, QgsUnitTypes::LayoutPoints ) );
  //check size matches fixed item size, not minimum size (converted to mm)
  QGSCOMPARENEAR( fixedMinItem->rect().width(), 20.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( fixedMinItem->rect().height(), 40.0, 4 * std::numeric_limits<double>::epsilon() );
}

void TestQgsLayoutItem::move()
{
  QgsProject p;
  QgsLayout l( &p );

  //move test item, same units as layout
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  std::unique_ptr< TestItem > item( new TestItem( &l ) );
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
  l.renderContext().setDpi( 100.0 );
  item->refresh();
  item->setPos( 1, 2 );
  item->attemptMove( QgsLayoutPoint( 140, 280, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->scenePos().x(), 1.4 );
  QCOMPARE( item->scenePos().y(), 2.8 );
  //changing the dpi should move the item
  l.renderContext().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->scenePos().x(), 0.7 );
  QCOMPARE( item->scenePos().y(), 1.4 );

  //test page -> pixel conversion
  l.setUnits( QgsUnitTypes::LayoutPixels );
  l.renderContext().setDpi( 100.0 );
  item->refresh();
  item->setPos( 2, 2 );
  item->attemptMove( QgsLayoutPoint( 1, 3, QgsUnitTypes::LayoutInches ) );
  QCOMPARE( item->scenePos().x(), 100.0 );
  QCOMPARE( item->scenePos().y(), 300.0 );
  //changing dpi results in item move
  l.renderContext().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->scenePos().x(), 200.0 );
  QCOMPARE( item->scenePos().y(), 600.0 );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );

  //reference points
  item->attemptMove( QgsLayoutPoint( 5, 9 ) );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  QCOMPARE( item->positionWithUnits().x(), 9.0 );
  QCOMPARE( item->positionWithUnits().y(), 15.0 );

  item->attemptMove( QgsLayoutPoint( 11, 13 ) );
  QCOMPARE( item->positionWithUnits().x(), 11.0 );
  QCOMPARE( item->positionWithUnits().y(), 13.0 );
  QCOMPARE( item->scenePos().x(), 7.0 );
  QCOMPARE( item->scenePos().y(), 7.0 );

  item->attemptMove( QgsLayoutPoint( 10, 12 ), false );
  QCOMPARE( item->positionWithUnits().x(), 14.0 );
  QCOMPARE( item->positionWithUnits().y(), 18.0 );
  QCOMPARE( item->scenePos().x(), 10.0 );
  QCOMPARE( item->scenePos().y(), 12.0 );

  //moveBy
  item.reset( new TestItem( &l ) );
  item->attemptMove( QgsLayoutPoint( 5, 9, QgsUnitTypes::LayoutCentimeters ) );
  item->attemptResize( QgsLayoutSize( 4, 6 ) );
  QCOMPARE( item->positionWithUnits().x(), 5.0 );
  QCOMPARE( item->positionWithUnits().y(), 9.0 );
  QCOMPARE( item->scenePos().x(), 50.0 );
  QCOMPARE( item->scenePos().y(), 90.0 );
  item->attemptMoveBy( 5, -6 );
  QCOMPARE( item->positionWithUnits().x(), 5.5 );
  QCOMPARE( item->positionWithUnits().y(), 8.4 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->scenePos().x(), 55.0 );
  QCOMPARE( item->scenePos().y(), 84.0 );

  //item with frame
  item.reset( new TestItem( &l ) );
  item->attemptResize( QgsLayoutSize( 2, 4 ) );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  item->setFrameEnabled( true );
  item->setFrameStrokeWidth( QgsLayoutMeasurement( 1 ) );
  item->attemptMove( QgsLayoutPoint( 1, 3 ) );
  QCOMPARE( item->positionWithUnits().x(), 1.0 );
  QCOMPARE( item->positionWithUnits().y(), 3.0 );
  item->attemptMove( QgsLayoutPoint( 4, 6 ), false, true );
  QCOMPARE( item->positionWithUnits().x(), 4.5 );
  QCOMPARE( item->positionWithUnits().y(), 6.5 );
}

void TestQgsLayoutItem::setSceneRect()
{
  QgsProject p;
  QgsLayout l( &p );

  //resize test item (no restrictions), same units as layout
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  std::unique_ptr< TestItem > item( new TestItem( &l ) );
  const QSignalSpy spySizeChanged( item.get(), &QgsLayoutItem::sizePositionChanged );

  item->attemptSetSceneRect( QRectF( 27.0, 29.0, 100, 200 ) );
  QCOMPARE( spySizeChanged.count(), 1 );
  QCOMPARE( item->rect().width(), 100.0 );
  QCOMPARE( item->rect().height(), 200.0 );
  QCOMPARE( item->scenePos().x(), 27.0 );
  QCOMPARE( item->scenePos().y(), 29.0 );
  QCOMPARE( item->positionWithUnits().x(), 27.0 );
  QCOMPARE( item->positionWithUnits().y(), 29.0 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( item->sizeWithUnits().width(), 100.0 );
  QCOMPARE( item->sizeWithUnits().height(), 200.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutMillimeters );

  //test conversion of units
  item->attemptMove( QgsLayoutPoint( 1, 2, QgsUnitTypes::LayoutCentimeters ) );
  item->attemptResize( QgsLayoutSize( 3, 4, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( spySizeChanged.count(), 3 );
  item->attemptSetSceneRect( QRectF( 27.0, 29.0, 100, 200 ) );
  QCOMPARE( item->rect().width(), 100.0 );
  QCOMPARE( item->rect().height(), 200.0 );
  QCOMPARE( item->scenePos().x(), 27.0 );
  QCOMPARE( item->scenePos().y(), 29.0 );
  QCOMPARE( spySizeChanged.count(), 4 );

  QCOMPARE( item->positionWithUnits().x(), 2.70 );
  QCOMPARE( item->positionWithUnits().y(), 2.90 );
  QCOMPARE( item->positionWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( item->sizeWithUnits().width(), 10.0 );
  QCOMPARE( item->sizeWithUnits().height(), 20.0 );
  QCOMPARE( item->sizeWithUnits().units(), QgsUnitTypes::LayoutCentimeters );
}

void TestQgsLayoutItem::page()
{
  QgsProject proj;
  QgsLayout l( &proj );

  TestItem *item = new TestItem( nullptr );
  item->attemptMove( QgsLayoutPoint( 5, 5 ) );
  // no layout
  QCOMPARE( item->page(), -1 );
  QCOMPARE( item->pagePos(), QPointF( 5, 5 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 5 ) );

  delete item;
  item = new TestItem( &l );
  item->attemptMove( QgsLayoutPoint( 5, 5 ) );
  l.addLayoutItem( item );
  QCOMPARE( item->page(), -1 );
  QCOMPARE( item->pagePos(), QPointF( 5, 5 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 5 ) );

  // add pages
  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 500, 100, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );
  QCOMPARE( item->page(), 0 );
  QCOMPARE( item->pagePos(), QPointF( 5, 5 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 5 ) );
  item->attemptMove( QgsLayoutPoint( 5, 5 ) );
  QCOMPARE( item->page(), 0 );
  QCOMPARE( item->pagePos(), QPointF( 5, 5 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 5 ) );
  item->attemptMove( QgsLayoutPoint( 5, 120 ) );
  QCOMPARE( item->page(), 0 );
  QCOMPARE( item->pagePos(), QPointF( 5, 120 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 120 ) );

  // second page
  page.reset( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 500, 200, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );
  QCOMPARE( item->page(), 1 );
  item->attemptMove( QgsLayoutPoint( 5, 190 ) );
  QCOMPARE( item->pagePos(), QPointF( 5, 80 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 80 ) );
  QCOMPARE( item->page(), 1 );
  item->attemptMove( QgsLayoutPoint( 5, 350 ) );
  QCOMPARE( item->page(), 1 );
  QCOMPARE( item->pagePos(), QPointF( 5, 240 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 240 ) );

  page.reset( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 500, 200, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );
  QCOMPARE( item->page(), 2 );
  QCOMPARE( item->pagePos(), QPointF( 5, 30 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 30 ) );

  // x position should not matter
  item->attemptMove( QgsLayoutPoint( -50, 350 ) );
  QCOMPARE( item->page(), 2 );
  QCOMPARE( item->pagePos(), QPointF( -50, 30 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( -50, 30 ) );
  item->attemptMove( QgsLayoutPoint( 55555, 350 ) );
  QCOMPARE( item->page(), 2 );
  QCOMPARE( item->pagePos(), QPointF( 55555, 30 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 55555, 30 ) );

  // with units
  item->attemptMove( QgsLayoutPoint( 5, 35, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->page(), 2 );
  QCOMPARE( item->pagePos(), QPointF( 50, 30 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 3, QgsUnitTypes::LayoutCentimeters ) );

  // move with page
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, 0 );
  QCOMPARE( item->page(), 0 );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, -1 );
  QCOMPARE( item->page(), 0 );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, 10000 );
  QCOMPARE( item->page(), 0 );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, 1 );
  QCOMPARE( item->page(), 1 );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 5, 116, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, 2 );
  QCOMPARE( item->page(), 2 );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 5, 326, QgsUnitTypes::LayoutMillimeters ) );
  item->attemptMove( QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutCentimeters ), true, false, 2 );
  QCOMPARE( item->page(), 2 );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 5, 38, QgsUnitTypes::LayoutCentimeters ) );

  // non-top-left reference
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, 0 );
  QCOMPARE( item->pagePos(), QPointF( 5, 6 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 6 ) );
  item->attemptMove( QgsLayoutPoint( 5, 6 ), true, false, 1 );
  QCOMPARE( item->page(), 1 );
  QCOMPARE( item->pagePos(), QPointF( 5, 6 ) );
  QCOMPARE( item->pagePositionWithUnits(), QgsLayoutPoint( 5, 6, QgsUnitTypes::LayoutMillimeters ) );
}

void TestQgsLayoutItem::itemVariablesFunction()
{
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );

  QgsExpression e( QStringLiteral( "map_get( item_variables( 'Map_id' ), 'map_scale' )" ) );
  // no map
  QgsExpressionContext c = l.createExpressionContext();
  QVariant r = e.evaluate( &c );
  QVERIFY( !r.isValid() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );
  map->setId( QStringLiteral( "Map_id" ) );

  c = l.createExpressionContext();
  e.prepare( &c );
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 184764103, 100 );

  QgsExpression e2( QStringLiteral( "map_get( item_variables( 'Map_id' ), 'map_crs' )" ) );
  r = e2.evaluate( &c );
  QCOMPARE( r.toString(), QString( "EPSG:4326" ) );

  QgsExpression e3( QStringLiteral( "map_get( item_variables( 'Map_id' ), 'map_crs_definition' )" ) );
  r = e3.evaluate( &c );
  QCOMPARE( r.toString(), QString( "+proj=longlat +datum=WGS84 +no_defs" ) );

  QgsExpression e4( QStringLiteral( "map_get( item_variables( 'Map_id' ), 'map_units' )" ) );
  r = e4.evaluate( &c );
  QCOMPARE( r.toString(), QString( "degrees" ) );

  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  std::unique_ptr< QgsVectorLayer > layer2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "B" ), QStringLiteral( "memory" ) );
  map->setLayers( QList<QgsMapLayer *>() << layer.get() << layer2.get() );
  QgsExpression e5( QStringLiteral( "map_get( item_variables( 'Map_id' ), 'map_layer_ids' )" ) );
  r = e5.evaluate( &c );
  QCOMPARE( r.toStringList().join( ',' ), QStringLiteral( "%1,%2" ).arg( layer->id(), layer2->id() ) );
  e5 = QgsExpression( QStringLiteral( "array_foreach(map_get( item_variables( 'Map_id' ), 'map_layers' ), layer_property(@element, 'name'))" ) );
  r = e5.evaluate( &c );
  QCOMPARE( r.toStringList().join( ',' ), QStringLiteral( "A,B" ) );
}

void TestQgsLayoutItem::variables()
{
  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::layoutItemScope( map ) );
  const int before = scope->variableCount();

  QgsExpressionContextUtils::setLayoutItemVariable( map, QStringLiteral( "var" ), 5 );
  scope.reset( QgsExpressionContextUtils::layoutItemScope( map ) );
  QCOMPARE( scope->variableCount(), before + 1 );
  QCOMPARE( scope->variable( QStringLiteral( "var" ) ).toInt(), 5 );

  QVariantMap vars;
  vars.insert( QStringLiteral( "var2" ), 7 );
  QgsExpressionContextUtils::setLayoutItemVariables( map, vars );
  scope.reset( QgsExpressionContextUtils::layoutItemScope( map ) );
  QCOMPARE( scope->variableCount(), before + 1 );
  QVERIFY( !scope->hasVariable( QStringLiteral( "var" ) ) );
  QCOMPARE( scope->variable( QStringLiteral( "var2" ) ).toInt(), 7 );
}

void TestQgsLayoutItem::mapCreditsFunction()
{
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );

  QgsExpression e( QStringLiteral( "array_to_string( map_credits( 'Map_id' ) )" ) );
  // no map
  QgsExpressionContext c = l.createExpressionContext();
  QVariant r = e.evaluate( &c );
  QVERIFY( !r.isValid() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );
  map->setId( QStringLiteral( "Map_id" ) );

  c = l.createExpressionContext();
  e.prepare( &c );
  r = e.evaluate( &c );
  // no layers
  QCOMPARE( r.toString(), QString() );

  // with layers
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  QgsLayerMetadata metadata;
  metadata.setRights( QStringList() << QStringLiteral( "CC BY SA" ) );
  layer->setMetadata( metadata );
  std::unique_ptr< QgsVectorLayer > layer2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "B" ), QStringLiteral( "memory" ) );
  metadata.setRights( QStringList() << QStringLiteral( "CC NC" ) );
  layer2->setMetadata( metadata );
  std::unique_ptr< QgsVectorLayer > layer3 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "C" ), QStringLiteral( "memory" ) );
  metadata.setRights( QStringList() << QStringLiteral( "CC BY SA" ) );
  layer3->setMetadata( metadata );
  const std::unique_ptr< QgsVectorLayer > layer4 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "C" ), QStringLiteral( "memory" ) );

  map->setLayers( QList<QgsMapLayer *>() << layer.get() << layer2.get() << layer3.get()  << layer4.get() );
  e.prepare( &c );
  QCOMPARE( e.evaluate( &c ).toString(), QStringLiteral( "CC BY SA,CC NC" ) );
  map->setLayers( QList<QgsMapLayer *>() << layer.get() << layer3.get()  << layer4.get() );
  e.prepare( &c );
  QCOMPARE( e.evaluate( &c ).toString(), QStringLiteral( "CC BY SA" ) );

  QgsExpression e2( QStringLiteral( "array_to_string( map_credits( 'Map_id', include_layer_names:=true ) )" ) );
  e2.prepare( &c );
  QCOMPARE( e2.evaluate( &c ).toString(), QStringLiteral( "A: CC BY SA,C: CC BY SA" ) );
  map->setLayers( QList<QgsMapLayer *>() << layer.get() << layer2.get() << layer3.get()  << layer4.get() );
  QgsExpression e3( QStringLiteral( "array_to_string( map_credits( 'Map_id', include_layer_names:=true, layer_name_separator:='|' ) )" ) );
  e3.prepare( &c );
  QCOMPARE( e3.evaluate( &c ).toString(), QStringLiteral( "A|CC BY SA,B|CC NC,C|CC BY SA" ) );

  // second map
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map2->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map2->setExtent( extent );
  l.addLayoutItem( map2 );
  map2->setId( QStringLiteral( "Map_2" ) );
  map2->setLayers( QList<QgsMapLayer *>() << layer.get()  << layer4.get() );
  QgsExpression e4( QStringLiteral( "array_to_string( map_credits( 'Map_2', include_layer_names:=true ) )" ) );
  e4.prepare( &c );
  QCOMPARE( e4.evaluate( &c ).toString(), QStringLiteral( "A: CC BY SA" ) );
}

void TestQgsLayoutItem::rotation()
{
  QgsProject proj;
  QgsLayout l( &proj );

  TestItem *item = new TestItem( &l );

  const QSignalSpy spyRotationChanged( item, &QgsLayoutItem::rotationChanged );

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
  QCOMPARE( spyRotationChanged.count(), 1 );
  QCOMPARE( spyRotationChanged.at( 0 ).at( 0 ).toDouble(), 90.0 );


  //check that negative angles are preserved as negative
  item->setItemRotation( -90.0 );
  QCOMPARE( item->itemRotation(), -90.0 );
  QCOMPARE( item->rotation(), -90.0 );
  bounds = item->sceneBoundingRect();
  QCOMPARE( bounds.width(), 8.0 );
  QCOMPARE( bounds.height(), 10.0 );
  QCOMPARE( spyRotationChanged.count(), 2 );
  QCOMPARE( spyRotationChanged.at( 1 ).at( 0 ).toDouble(), -90.0 );

  //check that rotating changes stored item position for reference point
  item->setItemRotation( 0.0 );
  QCOMPARE( spyRotationChanged.count(), 3 );
  QCOMPARE( spyRotationChanged.at( 2 ).at( 0 ).toDouble(), 0.0 );

  item->attemptMove( QgsLayoutPoint( 5.0, 8.0 ) );
  item->attemptResize( QgsLayoutSize( 10.0, 6.0 ) );
  item->setItemRotation( 90.0 );
  QCOMPARE( item->positionWithUnits().x(), 13.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  QCOMPARE( spyRotationChanged.count(), 4 );
  QCOMPARE( spyRotationChanged.at( 3 ).at( 0 ).toDouble(), 90.0 );

  //setting item position (for reference point) respects rotation
  item->attemptMove( QgsLayoutPoint( 10.0, 8.0 ) );
  QCOMPARE( item->scenePos().x(), 10.0 );
  QCOMPARE( item->scenePos().y(), 8.0 );
  const QRectF p = item->sceneBoundingRect();
  qDebug() << p.left();
  QCOMPARE( item->sceneBoundingRect().left(), 4.0 );
  QCOMPARE( item->sceneBoundingRect().right(), 10.0 );
  QCOMPARE( item->sceneBoundingRect().top(), 8.0 );
  QCOMPARE( item->sceneBoundingRect().bottom(), 18.0 );

  // set rotation, using top left
  std::unique_ptr< TestItem > item2( new TestItem( &l ) );
  item2->attemptMove( QgsLayoutPoint( 5.0, 8.0 ) );
  item2->attemptResize( QgsLayoutSize( 10.0, 6.0 ) );
  item2->setItemRotation( 90, false );
  QCOMPARE( item2->positionWithUnits().x(), 5.0 );
  QCOMPARE( item2->positionWithUnits().y(), 8.0 );
  QCOMPARE( item2->pos().x(), 5.0 );
  QCOMPARE( item2->pos().y(), 8.0 );
  item2->setItemRotation( 180, true );
  QCOMPARE( item2->positionWithUnits().x(), 7.0 );
  QCOMPARE( item2->positionWithUnits().y(), 16.0 );
  QCOMPARE( item2->pos().x(), 7.0 );
  QCOMPARE( item2->pos().y(), 16.0 );

  // test that refresh rotation doesn't move item (#18037)
  item2 = std::make_unique< TestItem >( &l );
  item2->setReferencePoint( QgsLayoutItem::Middle );
  item2->attemptMove( QgsLayoutPoint( 5.0, 8.0 ) );
  item2->attemptResize( QgsLayoutSize( 10.0, 6.0 ) );
  item2->setItemRotation( 45 );
  QCOMPARE( item2->positionWithUnits().x(), 5.0 );
  QCOMPARE( item2->positionWithUnits().y(), 8.0 );
  QGSCOMPARENEAR( item2->pos().x(), 3.58, 0.01 );
  QGSCOMPARENEAR( item2->pos().y(), 2.343146, 0.01 );
  QCOMPARE( item2->rotation(), 45.0 );
  item2->refresh();
  QCOMPARE( item2->positionWithUnits().x(), 5.0 );
  QCOMPARE( item2->positionWithUnits().y(), 8.0 );
  QGSCOMPARENEAR( item2->pos().x(), 3.58, 0.01 );
  QGSCOMPARENEAR( item2->pos().y(), 2.343146, 0.01 );
  QCOMPARE( item2->rotation(), 45.0 );


  //TODO also changing size?


  //data defined rotation
  item->setItemRotation( 0.0 );
  QCOMPARE( spyRotationChanged.count(), 5 );
  QCOMPARE( spyRotationChanged.at( 4 ).at( 0 ).toDouble(), 0.0 );

  item->attemptMove( QgsLayoutPoint( 5.0, 8.0 ) );
  item->attemptResize( QgsLayoutSize( 10.0, 6.0 ) );
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemRotation, QgsProperty::fromExpression( QStringLiteral( "90" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::ItemRotation );
  QCOMPARE( item->itemRotation(), 0.0 ); // should be unchanged
  QCOMPARE( item->rotation(), 90.0 );
  QCOMPARE( spyRotationChanged.count(), 6 );
  QCOMPARE( spyRotationChanged.at( 5 ).at( 0 ).toDouble(), 90.0 );

  // rotation should have applied around item center
  QCOMPARE( item->positionWithUnits().x(), 13.0 );
  QCOMPARE( item->positionWithUnits().y(), 6.0 );
  QCOMPARE( item->pos().x(), 13.0 );
  QCOMPARE( item->pos().y(), 6.0 );

  //also check when refreshing all properties
  item->dataDefinedProperties().setProperty( QgsLayoutObject::ItemRotation, QgsProperty::fromExpression( QStringLiteral( "180" ) ) );
  item->refreshDataDefinedProperty( QgsLayoutObject::AllProperties );
  QCOMPARE( item->itemRotation(), 0.0 ); // should be unchanged
  QCOMPARE( item->rotation(), 180.0 );
  QCOMPARE( spyRotationChanged.count(), 7 );
  QCOMPARE( spyRotationChanged.at( 6 ).at( 0 ).toDouble(), 180.0 );
  QCOMPARE( item->positionWithUnits().x(), 15.0 );
  QCOMPARE( item->positionWithUnits().y(), 14.0 );
  QCOMPARE( item->pos().x(), 15.0 );
  QCOMPARE( item->pos().y(), 14.0 );

  delete item;

  //render check
  item = new TestItem( &l );
  item->setItemRotation( 0.0 );
  item->setPos( 100, 150 );
  item->setRect( 0, 0, 200, 100 );
  l.addItem( item );
  item->setItemRotation( 45 );
  l.setSceneRect( 0, 0, 400, 400 );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagDebug, true );
  QImage image( l.sceneRect().size().toSize(), QImage::Format_ARGB32 );
  image.fill( 0 );
  QPainter painter( &image );
  l.render( &painter );
  painter.end();

  const bool result = renderCheck( QStringLiteral( "layoutitem_rotation" ), image, 0 );
  delete item;
  QVERIFY( result );
}

//TODO rotation tests:
//rotate item around layout point


void TestQgsLayoutItem::writeXml()
{
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );

  QgsProject proj;
  QgsLayout l( &proj );
  TestItem *item = new TestItem( &l );
  QVERIFY( item->writeXml( rootNode, doc, QgsReadWriteContext() ) );

  //make sure type was written
  const QDomElement element = rootNode.firstChildElement();

  QCOMPARE( element.nodeName(), QString( "LayoutItem" ) );
  QCOMPARE( element.attribute( "type", "" ).toInt(), item->type() );

  //check that element has an object node
  const QDomNodeList objectNodeList = element.elementsByTagName( QStringLiteral( "LayoutObject" ) );
  QCOMPARE( objectNodeList.count(), 1 );

  delete item;
}

void TestQgsLayoutItem::readXml()
{
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  QgsProject proj;
  QgsLayout l( &proj );
  TestItem *item = new TestItem( &l );

  //try reading bad elements
  const QDomElement badElement = doc.createElement( QStringLiteral( "bad" ) );
  const QDomElement noNode;
  QVERIFY( !item->readXml( badElement, doc, QgsReadWriteContext() ) );
  QVERIFY( !item->readXml( noNode, doc, QgsReadWriteContext() ) );

  //try good element
  QDomElement goodElement = doc.createElement( QStringLiteral( "LayoutItem" ) );
  goodElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "TestItemType" ) );
  QVERIFY( item->readXml( goodElement, doc, QgsReadWriteContext() ) );
  delete item;
}

void TestQgsLayoutItem::writeReadXmlProperties()
{
  QgsProject proj;
  QgsLayout l( &proj );
  TestItem *original = new TestItem( &l );

  original->dataDefinedProperties().setProperty( QgsLayoutObject::TestProperty, QgsProperty::fromExpression( QStringLiteral( "10 + 40" ) ) );

  original->setReferencePoint( QgsLayoutItem::MiddleRight );
  original->attemptResize( QgsLayoutSize( 6, 8, QgsUnitTypes::LayoutCentimeters ) );
  original->attemptMove( QgsLayoutPoint( 0.05, 0.09, QgsUnitTypes::LayoutMeters ) );
  original->setItemRotation( 45.0 );
  original->setId( QStringLiteral( "test" ) );
  original->setLocked( true );
  original->setZValue( 55 );
  original->setVisible( false );
  original->setFrameEnabled( true );
  original->setFrameStrokeColor( QColor( 100, 150, 200 ) );
  original->setFrameStrokeWidth( QgsLayoutMeasurement( 5, QgsUnitTypes::LayoutCentimeters ) );
  original->setFrameJoinStyle( Qt::MiterJoin );
  original->setBackgroundEnabled( false );
  original->setBackgroundColor( QColor( 200, 150, 100 ) );
  original->setBlendMode( QPainter::CompositionMode_Darken );
  original->setExcludeFromExports( true );
  original->setItemOpacity( 0.75 );

  std::unique_ptr< QgsLayoutItem > copy = createCopyViaXml( &l, original );

  QCOMPARE( copy->uuid(), original->uuid() );
  QCOMPARE( copy->id(), original->id() );
  const QgsProperty dd = copy->dataDefinedProperties().property( QgsLayoutObject::TestProperty );
  QVERIFY( dd );
  QVERIFY( dd.isActive() );
  QCOMPARE( dd.propertyType(), QgsProperty::ExpressionBasedProperty );
  QCOMPARE( copy->referencePoint(), original->referencePoint() );
  QCOMPARE( copy->sizeWithUnits(), original->sizeWithUnits() );
  QGSCOMPARENEAR( copy->positionWithUnits().x(), original->positionWithUnits().x(), 0.001 );
  QGSCOMPARENEAR( copy->positionWithUnits().y(), original->positionWithUnits().y(), 0.001 );
  QCOMPARE( copy->positionWithUnits().units(), original->positionWithUnits().units() );
  QCOMPARE( copy->itemRotation(), original->itemRotation() );
  QGSCOMPARENEAR( copy->pos().x(), original->pos().x(), 0.001 );
  QGSCOMPARENEAR( copy->pos().y(), original->pos().y(), 0.001 );
  QVERIFY( copy->isLocked() );
  QCOMPARE( copy->zValue(), 55.0 );
  QVERIFY( !copy->isVisible() );
  QVERIFY( copy->frameEnabled() );
  QCOMPARE( copy->frameStrokeColor(), QColor( 100, 150, 200 ) );
  QCOMPARE( copy->frameStrokeWidth(), QgsLayoutMeasurement( 5, QgsUnitTypes::LayoutCentimeters ) );
  QCOMPARE( copy->frameJoinStyle(), Qt::MiterJoin );
  QVERIFY( !copy->hasBackground() );
  QCOMPARE( copy->backgroundColor(), QColor( 200, 150, 100 ) );
  QCOMPARE( copy->blendMode(), QPainter::CompositionMode_Darken );
  QVERIFY( copy->excludeFromExports( ) );
  QCOMPARE( copy->itemOpacity(), 0.75 );
  delete original;
}

void TestQgsLayoutItem::undoRedo()
{
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  const QString uuid = item->uuid();
  QPointer< QgsLayoutItemShape > pItem( item ); // for testing deletion
  item->setFrameStrokeColor( QColor( 255, 100, 200 ) );
  l.addLayoutItem( item );

  QVERIFY( pItem );
  QVERIFY( l.items().contains( item ) );
  QCOMPARE( l.itemByUuid( uuid ), item );

  // undo should delete item
  l.undoStack()->stack()->undo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );
  QVERIFY( !l.items().contains( item ) );
  QVERIFY( !l.itemByUuid( uuid ) );

  // redo should restore
  l.undoStack()->stack()->redo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  item = dynamic_cast< QgsLayoutItemShape * >( l.itemByUuid( uuid ) );
  QVERIFY( item );
  QVERIFY( l.items().contains( item ) );
  pItem = item;
  QCOMPARE( item->frameStrokeColor().name(), QColor( 255, 100, 200 ).name() );

  //... and repeat!

  // undo should delete item
  l.undoStack()->stack()->undo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );
  QVERIFY( !l.items().contains( item ) );
  QVERIFY( !l.itemByUuid( uuid ) );

  // redo should restore
  l.undoStack()->stack()->redo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  item = dynamic_cast< QgsLayoutItemShape * >( l.itemByUuid( uuid ) );
  QVERIFY( item );
  QVERIFY( l.items().contains( item ) );
  pItem = item;
  QCOMPARE( item->frameStrokeColor().name(), QColor( 255, 100, 200 ).name() );

  // delete item
  l.removeLayoutItem( item );

  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );
  QVERIFY( !l.items().contains( item ) );
  QVERIFY( !l.itemByUuid( uuid ) );

  // undo should restore
  l.undoStack()->stack()->undo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  item = dynamic_cast< QgsLayoutItemShape * >( l.itemByUuid( uuid ) );
  QVERIFY( item );
  QVERIFY( l.items().contains( item ) );
  pItem = item;
  QCOMPARE( item->frameStrokeColor().name(), QColor( 255, 100, 200 ).name() );

  // another undo should delete item
  l.undoStack()->stack()->undo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );
  QVERIFY( !l.items().contains( item ) );
  QVERIFY( !l.itemByUuid( uuid ) );

  // redo should restore
  l.undoStack()->stack()->redo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  item = dynamic_cast< QgsLayoutItemShape * >( l.itemByUuid( uuid ) );
  QVERIFY( item );
  QVERIFY( l.items().contains( item ) );
  pItem = item;
  QCOMPARE( item->frameStrokeColor().name(), QColor( 255, 100, 200 ).name() );

  // another redo should delete item
  l.undoStack()->stack()->redo();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QVERIFY( !pItem );
  QVERIFY( !l.items().contains( item ) );
  QVERIFY( !l.itemByUuid( uuid ) );

}

void TestQgsLayoutItem::multiItemUndo()
{
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item );
  item->attemptMove( QgsLayoutPoint( 10, 10 ) );
  QgsLayoutItemShape *item2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item2 );
  item2->attemptMove( QgsLayoutPoint( 20, 20 ) );

  l.undoStack()->beginCommand( item, tr( "Item moved" ), QgsLayoutItem::UndoIncrementalMove );
  item->attemptMove( QgsLayoutPoint( 1, 1 ) );
  l.undoStack()->endCommand();

  l.undoStack()->beginCommand( item2, tr( "Item moved" ), QgsLayoutItem::UndoIncrementalMove );
  item2->attemptMove( QgsLayoutPoint( 21, 21 ) );
  l.undoStack()->endCommand();

  // undo should only remove item2 move
  l.undoStack()->stack()->undo();
  QCOMPARE( item2->positionWithUnits(), QgsLayoutPoint( 20, 20 ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 1, 1 ) );
  l.undoStack()->stack()->undo();
  QCOMPARE( item2->positionWithUnits(), QgsLayoutPoint( 20, 20 ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 10, 10 ) );
}

void TestQgsLayoutItem::overlappingUndo()
{
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item );
  item->attemptMove( QgsLayoutPoint( 10, 10 ) );
  QgsLayoutItemShape *item2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item2 );
  item2->attemptMove( QgsLayoutPoint( 20, 20 ) );

  //commands overlap
  l.undoStack()->beginCommand( item, tr( "Item moved" ), QgsLayoutItem::UndoIncrementalMove );
  item->attemptMove( QgsLayoutPoint( 1, 1 ) );
  l.undoStack()->beginCommand( item2, tr( "Item moved" ), QgsLayoutItem::UndoIncrementalMove );
  item2->attemptMove( QgsLayoutPoint( 21, 21 ) );
  l.undoStack()->endCommand();
  l.undoStack()->endCommand();

  // undo should remove item move
  l.undoStack()->stack()->undo();
  QCOMPARE( item2->positionWithUnits(), QgsLayoutPoint( 21, 21 ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 10, 10 ) );
  l.undoStack()->stack()->undo();
  QCOMPARE( item2->positionWithUnits(), QgsLayoutPoint( 20, 20 ) );
  QCOMPARE( item->positionWithUnits(), QgsLayoutPoint( 10, 10 ) );

}

void TestQgsLayoutItem::blendMode()
{
  QgsProject proj;
  QgsLayout l( &proj );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item );

  item->setBlendMode( QPainter::CompositionMode_Darken );
  QCOMPARE( item->blendMode(), QPainter::CompositionMode_Darken );
  QVERIFY( item->mEffect->isEnabled() );

  l.renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects, false );
  QVERIFY( !item->mEffect->isEnabled() );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects, true );
  QVERIFY( item->mEffect->isEnabled() );

  item->dataDefinedProperties().setProperty( QgsLayoutObject::BlendMode, QgsProperty::fromExpression( "'lighten'" ) );
  item->refreshDataDefinedProperty();
  QCOMPARE( item->blendMode(), QPainter::CompositionMode_Darken ); // should not change
  QCOMPARE( item->mEffect->compositionMode(), QPainter::CompositionMode_Lighten );

  QgsLayout l2( QgsProject::instance() );
  l2.initializeDefaults();
  QgsLayoutItemShape *mComposerRect1 = new QgsLayoutItemShape( &l2 );
  mComposerRect1->attemptSetSceneRect( QRectF( 20, 20, 150, 100 ) );
  mComposerRect1->setShapeType( QgsLayoutItemShape::Rectangle );
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( Qt::black );
  mComposerRect1->setSymbol( fillSymbol );
  delete fillSymbol;

  l2.addLayoutItem( mComposerRect1 );
  QgsLayoutItemShape *mComposerRect2 = new QgsLayoutItemShape( &l2 );
  mComposerRect2->attemptSetSceneRect( QRectF( 50, 50, 150, 100 ) );
  mComposerRect2->setShapeType( QgsLayoutItemShape::Rectangle );
  l2.addLayoutItem( mComposerRect2 );
  QgsSimpleFillSymbolLayer *simpleFill2 = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol2 = new QgsFillSymbol();
  fillSymbol2->changeSymbolLayer( 0, simpleFill2 );
  simpleFill2->setColor( QColor( 0, 100, 150 ) );
  simpleFill2->setStrokeColor( Qt::black );
  mComposerRect2->setSymbol( fillSymbol2 );
  delete fillSymbol2;

  mComposerRect2->setBlendMode( QPainter::CompositionMode_Multiply );

  QgsLayoutChecker checker( QStringLiteral( "composereffects_blend" ), &l2 );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutItem::opacity()
{
  QgsProject proj;
  QgsLayout l( &proj );
  l.initializeDefaults();

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( QColor( 255, 150, 0 ) );
  simpleFill->setStrokeColor( Qt::black );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  item->setShapeType( QgsLayoutItemShape::Rectangle );
  item->attemptSetSceneRect( QRectF( 50, 50, 150, 100 ) );
  item->setSymbol( fillSymbol->clone() );

  l.addLayoutItem( item );

  item->setItemOpacity( 0.75 );
  QCOMPARE( item->itemOpacity(), 0.75 );

  // we handle opacity ourselves, so QGraphicsItem opacity should never be set
  QCOMPARE( item->opacity(), 1.0 );

  QgsLayoutChecker checker( QStringLiteral( "composereffects_transparency75" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testLayout( mReport ) );

  item->dataDefinedProperties().setProperty( QgsLayoutObject::Opacity, QgsProperty::fromExpression( "35" ) );
  item->refreshDataDefinedProperty();
  QCOMPARE( item->itemOpacity(), 0.75 ); // should not change
  QCOMPARE( item->opacity(), 1.0 );

  checker = QgsLayoutChecker( QStringLiteral( "composereffects_transparency35" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testLayout( mReport ) );

  // with background and frame
  l.removeLayoutItem( item );

  QgsLayoutItemLabel *labelItem = new QgsLayoutItemLabel( &l );
  l.addLayoutItem( labelItem );
  labelItem->attemptSetSceneRect( QRectF( 50, 50, 150, 100 ) );
  labelItem->setBackgroundEnabled( true );
  labelItem->setBackgroundColor( QColor( 40, 140, 240 ) );
  labelItem->setFrameEnabled( true );
  labelItem->setFrameStrokeColor( QColor( 40, 30, 20 ) );
  labelItem->setItemOpacity( 0.5 );
  checker = QgsLayoutChecker( QStringLiteral( "composereffects_transparency_bgframe" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testLayout( mReport ) );

  QgsLayout l2( QgsProject::instance() );
  l2.initializeDefaults();
  QgsLayoutItemShape *mComposerRect1 = new QgsLayoutItemShape( &l2 );
  mComposerRect1->attemptSetSceneRect( QRectF( 20, 20, 150, 100 ) );
  mComposerRect1->setShapeType( QgsLayoutItemShape::Rectangle );
  mComposerRect1->setSymbol( fillSymbol->clone() );
  delete fillSymbol;

  l2.addLayoutItem( mComposerRect1 );
  QgsLayoutItemShape *mComposerRect2 = new QgsLayoutItemShape( &l2 );
  mComposerRect2->attemptSetSceneRect( QRectF( 50, 50, 150, 100 ) );
  mComposerRect2->setShapeType( QgsLayoutItemShape::Rectangle );
  l2.addLayoutItem( mComposerRect2 );
  QgsSimpleFillSymbolLayer *simpleFill2 = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol2 = new QgsFillSymbol();
  fillSymbol2->changeSymbolLayer( 0, simpleFill2 );
  simpleFill2->setColor( QColor( 0, 100, 150 ) );
  simpleFill2->setStrokeColor( Qt::black );
  mComposerRect2->setSymbol( fillSymbol2 );
  delete fillSymbol2;

  mComposerRect2->setItemOpacity( 0.5 );

  checker = QgsLayoutChecker( QStringLiteral( "composereffects_transparency" ), &l2 );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testLayout( mReport ) );
}

void TestQgsLayoutItem::excludeFromExports()
{
  QgsProject proj;
  QgsLayout l( &proj );

  std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( &l ) );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  l.pageCollection()->addPage( page.release() );

  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  std::unique_ptr< QgsFillSymbol > fillSymbol( new QgsFillSymbol() );
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::transparent );
  simpleFill->setStrokeColor( Qt::transparent );
  l.pageCollection()->setPageStyleSymbol( fillSymbol.get() );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item );

  item->setExcludeFromExports( true );
  QVERIFY( item->excludeFromExports() );
  item->setExcludeFromExports( false );
  QVERIFY( !item->excludeFromExports() );

  item->dataDefinedProperties().setProperty( QgsLayoutObject::ExcludeFromExports, QgsProperty::fromExpression( "1" ) );
  item->refreshDataDefinedProperty();
  QVERIFY( !item->excludeFromExports() ); // should not change
  QVERIFY( item->mEvaluatedExcludeFromExports );

  item->attemptMove( QgsLayoutPoint( 100, 100 ) );
  item->attemptResize( QgsLayoutSize( 200, 200 ) );
  l.updateBounds();

  QgsLayoutChecker checker( QStringLiteral( "layoutitem_excluded" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layouts" ) );
  checker.setSize( QSize( 400, 400 ) );
  QVERIFY( checker.testLayout( mReport ) );
}

std::unique_ptr<QgsLayoutItem> TestQgsLayoutItem::createCopyViaXml( QgsLayout *layout, QgsLayoutItem *original )
{
  //save original item to xml
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );

  original->writeXml( rootNode, doc, QgsReadWriteContext() );

  //create new item and restore settings from xml
  std::unique_ptr< TestItem > copy = std::make_unique< TestItem >( layout );
  copy->readXml( rootNode.firstChildElement(), doc, QgsReadWriteContext() );

  return std::move( copy );
}

QGSTEST_MAIN( TestQgsLayoutItem )
#include "testqgslayoutitem.moc"
