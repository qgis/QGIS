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

#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dsymbol.h"

#include <QObject>
#include "qgstest.h"
#include "qgs3d.h"

//dummy symbol for testing
class Dummy3DSymbol : public QgsAbstract3DSymbol
{
  public:
    Dummy3DSymbol() = default;
    QString type() const override { return QStringLiteral( "Dummy" ); }
    QgsAbstract3DSymbol *clone() const override { return new Dummy3DSymbol(); }
    void writeXml( QDomElement &, const QgsReadWriteContext & ) const override {}
    void readXml( const QDomElement &, const QgsReadWriteContext & ) override {}

    static QgsAbstract3DSymbol *create() { return new Dummy3DSymbol(); }

};

class TestQgs3DSymbolRegistry : public QObject
{
    Q_OBJECT

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
  Qgs3DSymbolMetadata metadata = Qgs3DSymbolMetadata( QStringLiteral( "name" ), QStringLiteral( "display name" ), Dummy3DSymbol::create );
  QCOMPARE( metadata.type(), QString( "name" ) );
  QCOMPARE( metadata.visibleName(), QString( "display name" ) );

  //test creating symbol from metadata
  const std::unique_ptr< QgsAbstract3DSymbol > symbol( metadata.create() );
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

  registry->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy symbol" ), Dummy3DSymbol::create ) );
  QCOMPARE( registry->symbolTypes().length(), previousCount + 1 );
  //try adding again, should have no effect
  Qgs3DSymbolMetadata *dupe = new Qgs3DSymbolMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy symbol" ), Dummy3DSymbol::create );
  QVERIFY( ! registry->addSymbolType( dupe ) );
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

  Qgs3DSymbolAbstractMetadata *metadata = registry->symbolMetadata( QStringLiteral( "Dummy" ) );
  QCOMPARE( metadata->type(), QString( "Dummy" ) );

  //metadata for bad symbol
  metadata = registry->symbolMetadata( QStringLiteral( "bad symbol" ) );
  QVERIFY( !metadata );
}

void TestQgs3DSymbolRegistry::createSymbol()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  std::unique_ptr< QgsAbstract3DSymbol > symbol( registry->createSymbol( QStringLiteral( "Dummy" ) ) );

  QVERIFY( symbol.get() );
  Dummy3DSymbol *dummySymbol = dynamic_cast<Dummy3DSymbol *>( symbol.get() );
  QVERIFY( dummySymbol );

  //try creating a bad symbol
  symbol.reset( registry->createSymbol( QStringLiteral( "bad symbol" ) ) );
  QVERIFY( !symbol.get() );
}

void TestQgs3DSymbolRegistry::defaultSymbolForGeometryType()
{
  Qgs3DSymbolRegistry *registry = QgsApplication::symbol3DRegistry();
  std::unique_ptr< QgsAbstract3DSymbol > symbol( registry->defaultSymbolForGeometryType( QgsWkbTypes::PointGeometry ) );
  QCOMPARE( symbol->type(), QStringLiteral( "point" ) );
  symbol.reset( registry->defaultSymbolForGeometryType( QgsWkbTypes::LineGeometry ) );
  QCOMPARE( symbol->type(), QStringLiteral( "line" ) );
  symbol.reset( registry->defaultSymbolForGeometryType( QgsWkbTypes::PolygonGeometry ) );
  QCOMPARE( symbol->type(), QStringLiteral( "polygon" ) );
  symbol.reset( registry->defaultSymbolForGeometryType( QgsWkbTypes::NullGeometry ) );
  QVERIFY( !symbol );
}

QGSTEST_MAIN( TestQgs3DSymbolRegistry )
#include "testqgs3dsymbolregistry.moc"
