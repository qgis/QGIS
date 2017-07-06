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

    QgsLayout *mLayout = nullptr;
    QString mReport;

    bool renderCheck( QString testName, QImage &image, int mismatchCount );

};

void TestQgsLayoutItem::initTestCase()
{
  mLayout = new QgsLayout();
  mReport = "<h1>Layout Item Tests</h1>\n";
}

void TestQgsLayoutItem::cleanupTestCase()
{
  delete mLayout;

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
  TestItem *item = new TestItem( mLayout );
  QVERIFY( item );
  delete item;
}

void TestQgsLayoutItem::registry()
{
  // test QgsLayoutItemRegistry
  QgsLayoutItemRegistry registry;

  // empty registry
  QVERIFY( !registry.itemMetadata( -1 ) );
  QVERIFY( registry.itemTypes().isEmpty() );
  QVERIFY( !registry.createItem( 1, nullptr ) );
  QVERIFY( !registry.createItemWidget( 1 ) );

  auto create = []( QgsLayout * layout, const QVariantMap & )->QgsLayoutItem*
  {
    return new TestItem( layout );
  };
  auto createWidget = []()->QWidget*
  {
    return new QWidget();
  };
  auto resolve = []( QVariantMap & props, const QgsPathResolver &, bool )
  {
    props.clear();
  };

  QSignalSpy spyTypeAdded( &registry, &QgsLayoutItemRegistry::typeAdded );

  QgsLayoutItemMetadata *metadata = new QgsLayoutItemMetadata( 2, QStringLiteral( "my type" ), QIcon(), create, resolve, createWidget );
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
  QWidget *config = registry.createItemWidget( 2 );
  QVERIFY( config );
  delete config;
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

bool TestQgsLayoutItem::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator();
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsLayoutItem )
#include "testqgslayoutitem.moc"
