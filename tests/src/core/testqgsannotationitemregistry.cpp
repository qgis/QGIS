/***************************************************************************
                         testqgsannotationitemregistry.cpp
                         -----------------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsannotationitem.h"
#include "qgsannotationitemregistry.h"
#include "qgsmultirenderchecker.h"
#include "qgstest.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"

#include <QObject>
#include <QPainter>
#include <QImage>
#include <QtTest/QSignalSpy>


//simple item for testing, since some methods in QgsAnnotationItem are pure virtual
class TestItem : public QgsAnnotationItem
{

  public:

    TestItem() : QgsAnnotationItem()
    {
    }

    static QgsAnnotationItem *create() { return new TestItem(); }

    //implement pure virtual methods
    QString type() const override { return QStringLiteral( "test_item" ); }

    void render( QgsRenderContext &, QgsFeedback * ) override
    {
    }

    TestItem *clone() override
    {
      return new TestItem();
    }

    bool writeXml( QDomElement &, QDomDocument &, const QgsReadWriteContext & ) const override
    {
      return true;
    }

    bool readXml( const QDomElement &, const QgsReadWriteContext & ) override
    {
      return true;
    }

    QgsRectangle boundingBox() const override { return QgsRectangle(); }
};


class TestQgsAnnotationItemRegistry: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void metadata();
    void createInstance();
    void instanceHasItems();
    void addItem();
    void fetchTypes();
    void createItem();
};

void TestQgsAnnotationItemRegistry::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAnnotationItemRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAnnotationItemRegistry::init()
{

}

void TestQgsAnnotationItemRegistry::cleanup()
{

}

void TestQgsAnnotationItemRegistry::metadata()
{
  QgsAnnotationItemMetadata metadata = QgsAnnotationItemMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ),
                                       QStringLiteral( "display names" ),
                                       TestItem::create );
  QCOMPARE( metadata.type(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating item from metadata
  const std::unique_ptr< QgsAnnotationItem > item( metadata.createItem() );
  QVERIFY( item );
  TestItem *dummyItem = dynamic_cast<TestItem *>( item.get() );
  QVERIFY( dummyItem );
}

void TestQgsAnnotationItemRegistry::createInstance()
{
  QgsAnnotationItemRegistry *registry = QgsApplication::annotationItemRegistry();
  QVERIFY( registry );
}

void TestQgsAnnotationItemRegistry::instanceHasItems()
{
  //check that annotation item registry is initially populated with some items
  //(assumes that there is some default items)
  QgsAnnotationItemRegistry registry;

  // should be empty until initialized
  QVERIFY( registry.itemTypes().empty() );
  registry.populate();
  QVERIFY( registry.itemTypes().size() > 0 );
}

void TestQgsAnnotationItemRegistry::addItem()
{
  QgsAnnotationItemRegistry *registry = QgsApplication::annotationItemRegistry();
  const int previousCount = registry->itemTypes().size();

  const QSignalSpy spyTypeAdded( registry, &QgsAnnotationItemRegistry::typeAdded );

  registry->addItemType( new QgsAnnotationItemMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "display name" ),
                         QStringLiteral( "display names" ),
                         TestItem::create ) );
  QCOMPARE( registry->itemTypes().size(), previousCount + 1 );
  QCOMPARE( spyTypeAdded.count(), 1 );
  //try adding again, should have no effect
  QgsAnnotationItemMetadata *dupe = new QgsAnnotationItemMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "display name" ),
      QStringLiteral( "display names" ),
      TestItem::create );
  QVERIFY( ! registry->addItemType( dupe ) );
  QCOMPARE( spyTypeAdded.count(), 1 );
  QCOMPARE( registry->itemTypes().size(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addItemType( nullptr );
  QCOMPARE( registry->itemTypes().size(), previousCount + 1 );
  QCOMPARE( spyTypeAdded.count(), 1 );
}

void TestQgsAnnotationItemRegistry::fetchTypes()
{
  QgsAnnotationItemRegistry *registry = QgsApplication::annotationItemRegistry();
  const QStringList types = registry->itemTypes().keys();

  QVERIFY( types.contains( "Dummy" ) );

  QgsAnnotationItemAbstractMetadata *metadata = registry->itemMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->type(), QString( "Dummy" ) );

  //metadata for bad item
  metadata = registry->itemMetadata( QStringLiteral( "bad item" ) );
  QVERIFY( !metadata );
}

void TestQgsAnnotationItemRegistry::createItem()
{
  QgsAnnotationItemRegistry *registry = QgsApplication::annotationItemRegistry();
  std::unique_ptr< QgsAnnotationItem > item( registry->createItem( QStringLiteral( "Dummy" ) ) );

  QVERIFY( item.get() );
  TestItem *dummyItem = dynamic_cast<TestItem *>( item.get() );
  QVERIFY( dummyItem );

  //try creating a bad symbol
  item.reset( registry->createItem( QStringLiteral( "bad item" ) ) );
  QVERIFY( !item.get() );
}

QGSTEST_MAIN( TestQgsAnnotationItemRegistry )
#include "testqgsannotationitemregistry.moc"
