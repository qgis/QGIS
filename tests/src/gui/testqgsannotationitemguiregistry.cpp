/***************************************************************************
    testqgsannotationitemguiregistry.cpp
     --------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
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
#include "qgsannotationitem.h"
#include "qgsannotationitemguiregistry.h"
#include "qgsannotationitemwidget.h"
#include "qgsapplication.h"
#include "qgsannotationitemregistry.h"
#include <QSignalSpy>

class TestQgsAnnotationItemGuiRegistry: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void guiRegistry();

  private:

};

void TestQgsAnnotationItemGuiRegistry::initTestCase()
{

}

void TestQgsAnnotationItemGuiRegistry::cleanupTestCase()
{
}

void TestQgsAnnotationItemGuiRegistry::init()
{
}

void TestQgsAnnotationItemGuiRegistry::cleanup()
{
}

//simple item for testing, since some methods in QgsAnnotationItem are pure virtual
class TestItem : public QgsAnnotationItem // clazy:exclude=missing-qobject-macro
{
  public:

    TestItem() : QgsAnnotationItem() {}

    int mFlag = 0;

    //implement pure virtual methods
    QString type() const override { return QStringLiteral( "mytype" ); }
    TestItem *clone() override { return new TestItem(); }
    QgsRectangle boundingBox() const override { return QgsRectangle();}
    void render( QgsRenderContext &, QgsFeedback * ) override {}
    bool writeXml( QDomElement &, QDomDocument &, const QgsReadWriteContext & ) const override { return true; }
    bool readXml( const QDomElement &, const QgsReadWriteContext & ) override { return true; }
};

class TestItemWidget: public QgsAnnotationItemBaseWidget
{
  public:

    TestItemWidget( QWidget *parent )
      : QgsAnnotationItemBaseWidget( parent )
    {}

    QgsAnnotationItem *createItem() override { return nullptr; }
    void updateItem( QgsAnnotationItem * ) override {}

};

void TestQgsAnnotationItemGuiRegistry::guiRegistry()
{
  // test QgsAnnotationItemGuiRegistry
  QgsAnnotationItemGuiRegistry registry;

  // empty registry
  QVERIFY( !registry.itemMetadata( -1 ) );
  QVERIFY( registry.itemMetadataIds().isEmpty() );
  QCOMPARE( registry.metadataIdForItemType( QString() ), -1 );
  QVERIFY( !registry.createItemWidget( nullptr ) );
  QVERIFY( !registry.createItemWidget( nullptr ) );
  const std::unique_ptr< TestItem > testItem = std::make_unique< TestItem >();
  QVERIFY( !registry.createItemWidget( testItem.get() ) ); // not in registry

  const QSignalSpy spyTypeAdded( &registry, &QgsAnnotationItemGuiRegistry::typeAdded );

  // add a dummy item to registry
  auto createWidget = []( QgsAnnotationItem * )->QgsAnnotationItemBaseWidget *
  {
    return new TestItemWidget( nullptr );
  };

  QgsAnnotationItemGuiMetadata *metadata = new QgsAnnotationItemGuiMetadata( QStringLiteral( "mytype" ), QStringLiteral( "My Type" ), QIcon(), createWidget );
  QVERIFY( registry.addAnnotationItemGuiMetadata( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 1 );
  int uuid = registry.itemMetadataIds().value( 0 );
  QCOMPARE( spyTypeAdded.value( 0 ).at( 0 ).toInt(), uuid );
  QCOMPARE( registry.metadataIdForItemType( QStringLiteral( "mytype" ) ), uuid );

  // duplicate type id is allowed
  metadata = new QgsAnnotationItemGuiMetadata( QStringLiteral( "mytype" ), QStringLiteral( "My Type" ), QIcon(), createWidget );
  QVERIFY( registry.addAnnotationItemGuiMetadata( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 2 );
  //retrieve metadata
  QVERIFY( !registry.itemMetadata( -1 ) );
  QCOMPARE( registry.itemMetadataIds().count(), 2 );
  QCOMPARE( registry.metadataIdForItemType( QStringLiteral( "mytype" ) ), uuid );

  QVERIFY( registry.itemMetadata( uuid ) );
  QCOMPARE( registry.itemMetadata( uuid )->visibleName(), QStringLiteral( "My Type" ) );

  QWidget *widget = registry.createItemWidget( testItem.get() );
  QVERIFY( widget );
  delete widget;

  // groups
  QVERIFY( registry.addItemGroup( QgsAnnotationItemGuiGroup( QStringLiteral( "g1" ) ) ) );
  QCOMPARE( registry.itemGroup( QStringLiteral( "g1" ) ).id, QStringLiteral( "g1" ) );
  // can't add duplicate group
  QVERIFY( !registry.addItemGroup( QgsAnnotationItemGuiGroup( QStringLiteral( "g1" ) ) ) );

  //creating item
  QgsAnnotationItem *item = registry.createItem( uuid );
  QVERIFY( !item );
  QgsApplication::annotationItemRegistry()->addItemType( new QgsAnnotationItemMetadata( QStringLiteral( "mytype" ), QStringLiteral( "My Type" ), QStringLiteral( "My Types" ), []( )->QgsAnnotationItem*
  {
    return new TestItem();
  } ) );

  item = registry.createItem( uuid );
  QVERIFY( item );
  QCOMPARE( item->type(), QStringLiteral( "mytype" ) );
  QCOMPARE( static_cast< TestItem * >( item )->mFlag, 0 );
  delete item;

  // override create func
  metadata = new QgsAnnotationItemGuiMetadata( QStringLiteral( "mytype" ), QStringLiteral( "mytype" ), QIcon(), createWidget );
  metadata->setItemCreationFunction( []()->QgsAnnotationItem*
  {
    TestItem *item = new TestItem();
    item->mFlag = 2;
    return item;
  } );
  QVERIFY( registry.addAnnotationItemGuiMetadata( metadata ) );
  uuid = spyTypeAdded.at( spyTypeAdded.count() - 1 ).at( 0 ).toInt();
  item = registry.createItem( uuid );
  QVERIFY( item );
  QCOMPARE( item->type(), QStringLiteral( "mytype" ) );
  QCOMPARE( static_cast< TestItem * >( item )->mFlag, 2 );
  delete item;
}


QGSTEST_MAIN( TestQgsAnnotationItemGuiRegistry )
#include "testqgsannotationitemguiregistry.moc"
