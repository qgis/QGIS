/***************************************************************************
                         testqgscalloutregistry.cpp
                         -----------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgscalloutsregistry.h"
#include "qgscallout.h"
#include "qgsrendercontext.h"

#include <QObject>
#include "qgstest.h"

//dummy callout for testing
class DummyCallout : public QgsCallout
{
  public:
    DummyCallout() = default;
    QString type() const override { return QStringLiteral( "Dummy" ); }
    QgsCallout *clone() const override { return new DummyCallout(); }
    static QgsCallout *create( const QVariantMap &, const QgsReadWriteContext & ) { return new DummyCallout(); }
  protected:
    void draw( QgsRenderContext &, const QRectF &, const double, const QgsGeometry &, QgsCallout::QgsCalloutContext & ) override {}

};

class TestQgsCalloutRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void metadata();
    void createInstance();
    void instanceHasDefaultCallouts();
    void addCallout();
    void fetchTypes();
    void createCallout();
    void defaultCallout();

  private:

};

void TestQgsCalloutRegistry::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgsCalloutRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCalloutRegistry::init()
{

}

void TestQgsCalloutRegistry::cleanup()
{

}

void TestQgsCalloutRegistry::metadata()
{
  QgsCalloutMetadata metadata = QgsCalloutMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ), QIcon(), DummyCallout::create );
  QCOMPARE( metadata.name(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating callout from metadata
  const QVariantMap map;
  const std::unique_ptr< QgsCallout > callout( metadata.createCallout( map, QgsReadWriteContext() ) );
  QVERIFY( callout );
  DummyCallout *dummyCallout = dynamic_cast<DummyCallout *>( callout.get() );
  QVERIFY( dummyCallout );
}

void TestQgsCalloutRegistry::createInstance()
{
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  QVERIFY( registry );
}

void TestQgsCalloutRegistry::instanceHasDefaultCallouts()
{
  //check that callout registry is initially populated with some callouts
  //(assumes that there is some default callouts)
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  QVERIFY( registry->calloutTypes().length() > 0 );
}

void TestQgsCalloutRegistry::addCallout()
{
  //create an empty registry
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  const int previousCount = registry->calloutTypes().length();

  registry->addCalloutType( new QgsCalloutMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy callout" ), QIcon(), DummyCallout::create ) );
  QCOMPARE( registry->calloutTypes().length(), previousCount + 1 );
  //try adding again, should have no effect
  QgsCalloutMetadata *dupe = new QgsCalloutMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy callout" ), QIcon(), DummyCallout::create );
  QVERIFY( ! registry->addCalloutType( dupe ) );
  QCOMPARE( registry->calloutTypes().length(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addCalloutType( nullptr );
  QCOMPARE( registry->calloutTypes().length(), previousCount + 1 );
}

void TestQgsCalloutRegistry::fetchTypes()
{
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  const QStringList types = registry->calloutTypes();

  QVERIFY( types.contains( "Dummy" ) );

  QgsCalloutAbstractMetadata *metadata = registry->calloutMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->name(), QString( "Dummy" ) );

  //metadata for bad callout
  metadata = registry->calloutMetadata( QStringLiteral( "bad callout" ) );
  QVERIFY( !metadata );
}

void TestQgsCalloutRegistry::createCallout()
{
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  std::unique_ptr< QgsCallout > callout( registry->createCallout( QStringLiteral( "Dummy" ) ) );

  QVERIFY( callout.get() );
  DummyCallout *dummyCallout = dynamic_cast<DummyCallout *>( callout.get() );
  QVERIFY( dummyCallout );

  //try creating a bad callout
  callout.reset( registry->createCallout( QStringLiteral( "bad callout" ) ) );
  QVERIFY( !callout.get() );
}

void TestQgsCalloutRegistry::defaultCallout()
{
  const std::unique_ptr< QgsCallout > callout( QgsCalloutRegistry::defaultCallout() );
  QVERIFY( callout.get() );
}

QGSTEST_MAIN( TestQgsCalloutRegistry )
#include "testqgscalloutregistry.moc"
