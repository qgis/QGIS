/***************************************************************************
    testqgsfilefiledownloader.cpp
     --------------------------------------
    Date                 : 09.03.2020
    Copyright            : (C) 2020 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include <QObject>
#include <QDialog>
#include <QSignalSpy>

#include "qgsnewdatabasetablenamewidget.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsabstractproviderconnection.h"

class TestQgsNewDatabaseTableNameWidget: public QObject
{
    Q_OBJECT
  public:
    TestQgsNewDatabaseTableNameWidget() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testWidgetFilters();
    void testWidgetSignals();

};

void TestQgsNewDatabaseTableNameWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  // Add some connections to test with
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) ) };
  QgsAbstractProviderConnection *conn { md->createConnection( qgetenv( "QGIS_PGTEST_DB" ) ) };
  md->saveConnection( conn, QStringLiteral( "PG_1" ) );
  conn = md->createConnection( qgetenv( " QGIS_PGTEST_DB" ) );
  md->saveConnection( conn, QStringLiteral( "PG_2" ) );
}

void TestQgsNewDatabaseTableNameWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsNewDatabaseTableNameWidget::init()
{
}

void TestQgsNewDatabaseTableNameWidget::cleanup()
{
}

void TestQgsNewDatabaseTableNameWidget::testWidgetFilters()
{
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w { qgis::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList{ "NOT_EXISTS" } ) };
  QCOMPARE( w->mBrowserProxyModel.rowCount(), 0 );
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w2 { qgis::make_unique<QgsNewDatabaseTableNameWidget>( nullptr ) };
  QVERIFY( w2->mBrowserProxyModel.rowCount() > 0 );
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w3 { qgis::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList{ "postgres" } ) };
  QVERIFY( w3->mBrowserProxyModel.rowCount() > 0 );
}


void TestQgsNewDatabaseTableNameWidget::testWidgetSignals()
{
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w { qgis::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList{ "postgres" } ) };

  auto index = w->mBrowserModel->findPath( QStringLiteral( "pg:/PG_1" ) );
  QVERIFY( index.isValid() );
  w->mBrowserModel->dataItem( index )->populate( true );
  w->mBrowserTreeView->expandAll();

  QVERIFY( ! w->isValid() );

  QSignalSpy validationSpy( w.get(), SIGNAL( validationChanged( bool ) ) );
  QSignalSpy schemaSpy( w.get(), SIGNAL( schemaNameChanged( QString ) ) );
  QSignalSpy tableSpy( w.get(), SIGNAL( tableNameChanged( QString ) ) );
  QSignalSpy providerSpy( w.get(), SIGNAL( providerKeyChanged( QString ) ) );

  index = w->mBrowserProxyModel.mapToSource( w->mBrowserProxyModel.index( 0, 0 ) );
  QVERIFY( index.isValid() );
  QCOMPARE( w->mBrowserModel->data( index, Qt::DisplayRole ).toString(), QString( "PostGIS" ) );
  QRect rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, 0, rect.topLeft() );

  QVERIFY( ! w->isValid() );

  QCOMPARE( providerSpy.count(), 1 );
  QCOMPARE( tableSpy.count(), 0 );
  QCOMPARE( schemaSpy.count(), 0 );
  QCOMPARE( validationSpy.count(), 0 );
  auto arguments = providerSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "postgres" ) );

  // Find qgis_test schema item
  index = w->mBrowserModel->findPath( QStringLiteral( "pg:/PG_1/qgis_test" ), Qt::MatchFlag::MatchStartsWith );
  QVERIFY( index.isValid() );
  w->mBrowserTreeView->scrollTo( w->mBrowserProxyModel.mapFromSource( index ) );
  rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, 0, rect.center() );

  QVERIFY( ! w->isValid() );

  QCOMPARE( validationSpy.count(), 0 );
  QCOMPARE( schemaSpy.count(), 1 );
  arguments = schemaSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "qgis_test" ) );

  w->mNewTableName->setText( QStringLiteral( "someNewTableData" ) );
  QCOMPARE( tableSpy.count(), 1 );
  arguments = tableSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "someNewTableData" ) );
  QCOMPARE( validationSpy.count(), 1 );
  arguments = validationSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toBool(), true );
  QVERIFY( w->isValid() );

  // Test getters
  QCOMPARE( w->table(), QString( "someNewTableData" ) );
  QCOMPARE( w->schema(), QString( "qgis_test" ) );
  QCOMPARE( w->dataProviderKey(), QString( "postgres" ) );

  // Test unique and make it invalid again so we get a status change
  w->mNewTableName->setText( QStringLiteral( "someData" ) );
  QVERIFY( ! w->isValid() );
  QCOMPARE( tableSpy.count(), 1 );
  arguments = tableSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "someData" ) );
  QCOMPARE( validationSpy.count(), 1 );
  arguments = validationSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toBool(), false );

  // Now select another schema
  index = w->mBrowserModel->findPath( QStringLiteral( "pg:/PG_1/public" ), Qt::MatchFlag::MatchStartsWith );
  QVERIFY( index.isValid() );
  w->mBrowserTreeView->scrollTo( w->mBrowserProxyModel.mapFromSource( index ) );
  rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, 0, rect.center() );
  QCOMPARE( w->schema(), QString( "public" ) );
  QVERIFY( w->isValid() );
  QCOMPARE( validationSpy.count(), 1 );
  arguments = validationSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toBool(), true );
  QCOMPARE( schemaSpy.count(), 1 );
  arguments = schemaSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "public" ) );

  // Test getters
  QCOMPARE( w->table(), QString( "someData" ) );
  QCOMPARE( w->schema(), QString( "public" ) );
  QCOMPARE( w->dataProviderKey(), QString( "postgres" ) );

}

QGSTEST_MAIN( TestQgsNewDatabaseTableNameWidget )
#include "testqgsnewdatabasetablewidget.moc"


