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
    void fixedSize();
    void minSize();
    void move();

  private:

    //simple item for testing, since some methods in QgsLayoutItem are pure virtual
    class TestItem : public QgsLayoutItem
    {
      public:

        TestItem( QgsLayout *layout ) : QgsLayoutItem( layout ) {}
        ~TestItem() {}

        //implement pure virtual methods
        int type() const { return QgsLayoutItemRegistry::LayoutItem + 101; }
        void draw( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
        {
          Q_UNUSED( itemStyle );
          Q_UNUSED( pWidget );
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

void TestQgsLayoutItem::resize()
{
  QgsProject p;
  QgsLayout l( &p );

  //resize test item (no restrictions), same units as layout
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  TestItem *item = new TestItem( &l );
  item->setRect( 0, 0, 55, 45 );
  item->setPos( 27, 29 );
  item->attemptResize( QgsLayoutSize( 100.0, 200.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( item->rect().width(), 100.0 );
  QCOMPARE( item->rect().height(), 200.0 );
  QCOMPARE( item->pos().x(), 27.0 ); //item should not move
  QCOMPARE( item->pos().y(), 29.0 );

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

  //test that setting item position is done relative to reference point
  l.setUnits( QgsUnitTypes::LayoutMillimeters );
  item->attemptResize( QgsLayoutSize( 2, 4 ) );
  item->setReferencePoint( QgsLayoutItem::UpperLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), 1.0 );
  QCOMPARE( item->pos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperMiddle );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), 0.0 );
  QCOMPARE( item->pos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::UpperRight );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), -1.0 );
  QCOMPARE( item->pos().y(), 2.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), 1.0 );
  QCOMPARE( item->pos().y(), 0.0 );
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), 0.0 );
  QCOMPARE( item->pos().y(), 0.0 );
  item->setReferencePoint( QgsLayoutItem::MiddleRight );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), -1.0 );
  QCOMPARE( item->pos().y(), 0.0 );
  item->setReferencePoint( QgsLayoutItem::LowerLeft );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), 1.0 );
  QCOMPARE( item->pos().y(), -2.0 );
  item->setReferencePoint( QgsLayoutItem::LowerMiddle );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), 0.0 );
  QCOMPARE( item->pos().y(), -2.0 );
  item->setReferencePoint( QgsLayoutItem::LowerRight );
  item->attemptMove( QgsLayoutPoint( 1, 2 ) );
  QCOMPARE( item->pos().x(), -1.0 );
  QCOMPARE( item->pos().y(), -2.0 );

  //test that resizing is done relative to reference point

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
  QCOMPARE( item->pos().x(), 60.0 );
  QCOMPARE( item->pos().y(), 15.0 );

  //test conversion of units
  l.setUnits( QgsUnitTypes::LayoutCentimeters );
  item->setPos( 100, 200 );
  item->attemptMove( QgsLayoutPoint( 0.30, 0.45, QgsUnitTypes::LayoutMeters ) );
  QCOMPARE( item->pos().x(), 30.0 );
  QCOMPARE( item->pos().y(), 45.0 );

  //test pixel -> page conversion
  l.setUnits( QgsUnitTypes::LayoutInches );
  l.context().setDpi( 100.0 );
  item->refresh();
  item->setPos( 1, 2 );
  item->attemptMove( QgsLayoutPoint( 140, 280, QgsUnitTypes::LayoutPixels ) );
  QCOMPARE( item->pos().x(), 1.4 );
  QCOMPARE( item->pos().y(), 2.8 );
  //changing the dpi should move the item
  l.context().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->pos().x(), 0.7 );
  QCOMPARE( item->pos().y(), 1.4 );

  //test page -> pixel conversion
  l.setUnits( QgsUnitTypes::LayoutPixels );
  l.context().setDpi( 100.0 );
  item->refresh();
  item->setPos( 2, 2 );
  item->attemptMove( QgsLayoutPoint( 1, 3, QgsUnitTypes::LayoutInches ) );
  QCOMPARE( item->pos().x(), 100.0 );
  QCOMPARE( item->pos().y(), 300.0 );
  //changing dpi results in item move
  l.context().setDpi( 200.0 );
  item->refresh();
  QCOMPARE( item->pos().x(), 200.0 );
  QCOMPARE( item->pos().y(), 600.0 );

  l.setUnits( QgsUnitTypes::LayoutMillimeters );

  //TODO - reference points
}

QGSTEST_MAIN( TestQgsLayoutItem )
#include "testqgslayoutitem.moc"
