/***************************************************************************
                         testqgsstac.cpp
                         -------------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>

//qgis includes...
#include "qgsstaccontroller.h"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstacitem.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgis.h"

/**
 * \ingroup UnitTests
 * This is a unit test for STAC
 */
class TestQgsStac : public QObject
{
    Q_OBJECT

  public:
    TestQgsStac() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testParseLocalCatalog();
    void testParseLocalCollection();
    void testParseLocalItem();

  private:
    QString mDataDir;
};

void TestQgsStac::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QStringLiteral( "%1/stac/" ).arg( TEST_DATA_DIR );
}

void TestQgsStac::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsStac::testParseLocalCatalog()
{
  const QString fileName = mDataDir + QStringLiteral( "catalog.json" );
  QgsStacController c;
  QgsStacObject *obj = c.fetchStacObject( QStringLiteral( "file://%1" ).arg( fileName ) );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Catalog );
  QgsStacCatalog *cat = dynamic_cast< QgsStacCatalog * >( obj );

  QVERIFY( cat );
  QCOMPARE( cat->id(), QLatin1String( "examples" ) );
  QCOMPARE( cat->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( cat->title(), QLatin1String( "Example Catalog" ) );
  QCOMPARE( cat->description(), QLatin1String( "This catalog is a simple demonstration of an example catalog that is used to organize a hierarchy of collections and their items." ) );
  QCOMPARE( cat->links().size(), 6 );
  QVERIFY( cat->stacExtensions().isEmpty() );

  delete cat;
}

void TestQgsStac::testParseLocalCollection()
{
  const QString fileName = mDataDir + QStringLiteral( "collection.json" );
  QgsStacController c;
  QgsStacObject *obj = c.fetchStacObject( QStringLiteral( "file://%1" ).arg( fileName ) );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Collection );
  QgsStacCollection *col = dynamic_cast< QgsStacCollection * >( obj );

  QVERIFY( col );
  QCOMPARE( col->id(), QLatin1String( "simple-collection" ) );
  QCOMPARE( col->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( col->title(), QLatin1String( "Simple Example Collection" ) );
  QCOMPARE( col->description(), QLatin1String( "A simple collection demonstrating core catalog fields with links to a couple of items" ) );
  QCOMPARE( col->links().size(), 5 );
  QCOMPARE( col->providers().size(), 1 );
  QCOMPARE( col->stacExtensions().size(), 3 );
  QCOMPARE( col->license(), QLatin1String( "CC-BY-4.0" ) );
  QVERIFY( col->assets().isEmpty() );

  // extent
  QgsStacExtent ext( col->extent() );
  QVERIFY( !ext.hasDetailedSpatialExtents() );
  QCOMPARE( ext.spatialExtent().xMinimum(), 172.91173669923782 );
  QCOMPARE( ext.spatialExtent().yMinimum(), 1.3438851951615003 );
  QCOMPARE( ext.spatialExtent().xMaximum(), 172.95469614953714 );
  QCOMPARE( ext.spatialExtent().yMaximum(), 1.3690476620161975 );

  QVERIFY( !ext.hasDetailedTemporalExtents() );
  QCOMPARE( ext.temporalExtent().begin(), QDateTime::fromString( QStringLiteral( "2020-12-11T22:38:32.125Z" ), Qt::ISODateWithMs ) );
  QCOMPARE( ext.temporalExtent().end(), QDateTime::fromString( QStringLiteral( "2020-12-14T18:02:31.437Z" ), Qt::ISODateWithMs ) );

  // summaries
  QVariantMap sum( col->summaries() );
  QCOMPARE( sum.size(), 9 );
  QCOMPARE( sum.value( QStringLiteral( "platform" ) ).toStringList(), QStringList() << QLatin1String( "cool_sat1" ) << QLatin1String( "cool_sat2" ) );

  delete col;
}

void TestQgsStac::testParseLocalItem()
{
  const QString fileName = mDataDir + QStringLiteral( "core-item.json" );
  QgsStacController c;
  QgsStacObject *obj = c.fetchStacObject( QStringLiteral( "file://%1" ).arg( fileName ) );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Item );
  QgsStacItem *item = dynamic_cast<QgsStacItem *>( obj );

  QVERIFY( item );
  QCOMPARE( item->id(), QLatin1String( "20201211_223832_CS2" ) );
  QCOMPARE( item->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( item->links().size(), 4 );
  QCOMPARE( item->stacExtensions().size(), 0 );
  QCOMPARE( item->assets().size(), 6 );

  delete item;
}

QGSTEST_MAIN( TestQgsStac )
#include "testqgsstac.moc"
