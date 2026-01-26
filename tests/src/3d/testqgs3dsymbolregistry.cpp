/***************************************************************************
                         TestQgs3DSymbolRegistry.cpp
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

#include "qgs3d.h"
#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dsymbol.h"
#include "qgstest.h"

#include <QObject>

//dummy symbol for testing
class Dummy3DSymbol : public QgsAbstract3DSymbol
{
  public:
    Dummy3DSymbol() = default;
    QString type() const override { return u"Dummy"_s; }
    QgsAbstract3DSymbol *clone() const override { return new Dummy3DSymbol(); }
    void writeXml( QDomElement &, const QgsReadWriteContext & ) const override {}
    void readXml( const QDomElement &, const QgsReadWriteContext & ) override {}

    static QgsAbstract3DSymbol *create() { return new Dummy3DSymbol(); }
};

class TestQgs3DSymbolRegistry : public QgsTest
{
    Q_OBJECT
  public:
    TestQgs3DSymbolRegistry()
      : QgsTest( u"3D Symbol Registry Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void metadata();
    void createInstance();
    void instanceHasDefaultSymbols();
    void addSymbol();
    void fetchTypes();
    void createSymbol();
    void defaultSymbolForGeometryType();

  private:
};

void TestQgs3DSymbolRegistry::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgs3DSymbolRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3DSymbolRegistry::init()
{
}

void TestQgs3DSymbolRegistry::cleanup()
{
}

void TestQgs3DSymbolRegistry::metadata()
{
  Qgs3DSymbolMetadata metadata = Qgs3DSymbolMetadata( u"name"_s, u"display name"_s, Dummy3DSymbol::create );
  QCOMPARE( metadata.type(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating symbol from metadata
  const std::unique_ptr<QgsAbstract3DSymbol> symbol( metadata.create() );
  QVERIFY( symbol );
  Dummy3DSymbol *dummySymbol = dynamic_cast<Dummy3DSymbol *>( symbol.get() );
  QVERIFY( dummySymbol );
}

void TestQgs3DSymbolRegistry::createInstance()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  QVERIFY( registry );
}

void TestQgs3DSymbolRegistry::instanceHasDefaultSymbols()
{
  //check that symbol registry is initially populated with some symbols
  //(assumes that there is some default symbols)
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();

  // should be empty until initialized
  QVERIFY( registry->symbolTypes().empty() );
  Qgs3D::initialize();
  QVERIFY( registry->symbolTypes().length() > 0 );
}

void TestQgs3DSymbolRegistry::addSymbol()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  const int previousCount = registry->symbolTypes().length();

  registry->addSymbolType( new Qgs3DSymbolMetadata( u"Dummy"_s, u"Dummy symbol"_s, Dummy3DSymbol::create ) );
  QCOMPARE( registry->symbolTypes().length(), previousCount + 1 );
  //try adding again, should have no effect
  Qgs3DSymbolMetadata *dupe = new Qgs3DSymbolMetadata( u"Dummy"_s, u"Dummy symbol"_s, Dummy3DSymbol::create );
  QVERIFY( !registry->addSymbolType( dupe ) );
  QCOMPARE( registry->symbolTypes().length(), previousCount + 1 );
  delete dupe;

  //try adding empty metadata
  registry->addSymbolType( nullptr );
  QCOMPARE( registry->symbolTypes().length(), previousCount + 1 );
}

void TestQgs3DSymbolRegistry::fetchTypes()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  const QStringList types = registry->symbolTypes();

  QVERIFY( types.contains( "Dummy" ) );

  Qgs3DSymbolAbstractMetadata *metadata = registry->symbolMetadata( u"Dummy"_s );
  QCOMPARE( metadata->type(), QString( "Dummy" ) );

  //metadata for bad symbol
  metadata = registry->symbolMetadata( u"bad symbol"_s );
  QVERIFY( !metadata );
}

void TestQgs3DSymbolRegistry::createSymbol()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  std::unique_ptr<QgsAbstract3DSymbol> symbol( registry->createSymbol( u"Dummy"_s ) );

  QVERIFY( symbol.get() );
  Dummy3DSymbol *dummySymbol = dynamic_cast<Dummy3DSymbol *>( symbol.get() );
  QVERIFY( dummySymbol );

  //try creating a bad symbol
  symbol.reset( registry->createSymbol( u"bad symbol"_s ) );
  QVERIFY( !symbol.get() );
}

void TestQgs3DSymbolRegistry::defaultSymbolForGeometryType()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  std::unique_ptr<QgsAbstract3DSymbol> symbol( registry->defaultSymbolForGeometryType( Qgis::GeometryType::Point ) );
  QCOMPARE( symbol->type(), u"point"_s );
  symbol.reset( registry->defaultSymbolForGeometryType( Qgis::GeometryType::Line ) );
  QCOMPARE( symbol->type(), u"line"_s );
  symbol.reset( registry->defaultSymbolForGeometryType( Qgis::GeometryType::Polygon ) );
  QCOMPARE( symbol->type(), u"polygon"_s );
  symbol.reset( registry->defaultSymbolForGeometryType( Qgis::GeometryType::Null ) );
  QVERIFY( !symbol );
}

QGSTEST_MAIN( TestQgs3DSymbolRegistry )
#include "testqgs3dsymbolregistry.moc"
