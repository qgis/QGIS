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


#include "qgsabstractproviderconnection.h"
#include "qgsdataitem.h"
#include "qgsnewdatabasetablenamewidget.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgstest.h"

#include <QDialog>
#include <QObject>
#include <QSignalSpy>
#include <QTemporaryDir>

class TestQgsNewDatabaseTableNameWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsNewDatabaseTableNameWidget() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testWidgetFilters();
    void testWidgetSignalsPostgres();
    void testWidgetSignalsGeopackage();

  private:
    std::unique_ptr<QgsAbstractProviderConnection> mPgConn;
    std::unique_ptr<QgsAbstractProviderConnection> mGpkgConn;
    QTemporaryDir mDir;
    QString mGpkgPath;
};

void TestQgsNewDatabaseTableNameWidget::initTestCase()
{
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST-NEW-DBTABLE-WIDGET"_s );

  QgsApplication::init();
  QgsApplication::initQgis();

  // Add some connections to test with
  QgsProviderMetadata *md = nullptr;
#ifdef ENABLE_PGTEST
  md = QgsProviderRegistry::instance()->providerMetadata( u"postgres"_s );
  mPgConn.reset( md->createConnection( qgetenv( "QGIS_PGTEST_DB" ), {} ) );
  md->saveConnection( mPgConn.get(), u"PG_1"_s );
  md->saveConnection( mPgConn.get(), u"PG_2"_s );
#endif

  md = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s );
  QString errCause;
  QMap<int, int> m;
  mGpkgPath = mDir.filePath( u"test.gpkg"_s );
  const QMap<QString, QVariant> options { { u"layerName"_s, QString( "test_layer" ) } };
  QString createdLayerUri;
  QVERIFY( md->createEmptyLayer( mGpkgPath, QgsFields(), Qgis::WkbType::Point, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), true, m, errCause, &options, createdLayerUri ) == Qgis::VectorExportResult::Success );
  QCOMPARE( createdLayerUri, mGpkgPath + "|layername=test_layer" );
  QVERIFY( errCause.isEmpty() );
  mGpkgConn.reset( md->createConnection( mDir.filePath( u"test.gpkg"_s ), {} ) );
  md->saveConnection( mGpkgConn.get(), u"GPKG_1"_s );
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
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w { std::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList { "NOT_EXISTS" } ) };
  QCOMPARE( w->mBrowserProxyModel.rowCount(), 0 );
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w2 { std::make_unique<QgsNewDatabaseTableNameWidget>( nullptr ) };
  QVERIFY( w2->mBrowserProxyModel.rowCount() > 0 );
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w3 { std::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList { "postgres" } ) };
  QVERIFY( w3->mBrowserProxyModel.rowCount() > 0 );
}


void TestQgsNewDatabaseTableNameWidget::testWidgetSignalsPostgres()
{
#ifdef ENABLE_PGTEST
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w { std::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList { "postgres" } ) };

  auto index = w->mBrowserModel->findPath( u"pg:/PG_1"_s );
  QVERIFY( index.isValid() );
  w->mBrowserModel->dataItem( index )->populate( true );
  w->mBrowserTreeView->expandAll();

  QVERIFY( !w->isValid() );

  QSignalSpy validationSpy( w.get(), SIGNAL( validationChanged( bool ) ) );
  QSignalSpy schemaSpy( w.get(), SIGNAL( schemaNameChanged( QString ) ) );
  QSignalSpy tableSpy( w.get(), SIGNAL( tableNameChanged( QString ) ) );
  QSignalSpy providerSpy( w.get(), SIGNAL( providerKeyChanged( QString ) ) );
  QSignalSpy uriSpy( w.get(), SIGNAL( uriChanged( QString ) ) );

  index = w->mBrowserProxyModel.mapToSource( w->mBrowserProxyModel.index( 0, 0 ) );
  QVERIFY( index.isValid() );
  QCOMPARE( w->mBrowserModel->data( index, Qt::DisplayRole ).toString(), QString( "PostgreSQL" ) );
  QRect rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.topLeft() );

  QVERIFY( !w->isValid() );

  /*
  QDialog d;
  QVBoxLayout l;
  l.addWidget( w.get() );
  d.setLayout( &l );
  d.exec();
  //*/

  QCOMPARE( providerSpy.count(), 1 );
  QCOMPARE( uriSpy.count(), 0 );
  QCOMPARE( tableSpy.count(), 0 );
  QCOMPARE( schemaSpy.count(), 0 );
  QCOMPARE( validationSpy.count(), 0 );
  auto arguments = providerSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "postgres" ) );

  // Find qgis_test schema item
  index = w->mBrowserModel->findPath( u"pg:/PG_1/qgis_test"_s );
  QVERIFY( index.isValid() );
  w->mBrowserTreeView->scrollTo( w->mBrowserProxyModel.mapFromSource( index ) );
  rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center() );

  QVERIFY( !w->isValid() );

  QCOMPARE( validationSpy.count(), 0 );
  QCOMPARE( schemaSpy.count(), 1 );
  QCOMPARE( uriSpy.count(), 1 );
  arguments = schemaSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "qgis_test" ) );
  arguments = uriSpy.takeLast();
  QVERIFY( !arguments.at( 0 ).toString().isEmpty() );

  w->mNewTableName->setText( u"someNewTableData"_s ); //#spellok
  QCOMPARE( tableSpy.count(), 1 );
  arguments = tableSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "someNewTableData" ) ); //#spellok
  QCOMPARE( validationSpy.count(), 1 );
  arguments = validationSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toBool(), true );
  QVERIFY( w->isValid() );

  // Test getters
  QCOMPARE( w->table(), QString( "someNewTableData" ) ); //#spellok
  QCOMPARE( w->schema(), QString( "qgis_test" ) );
  QCOMPARE( w->dataProviderKey(), QString( "postgres" ) );
  QVERIFY( w->uri().contains( R"("qgis_test"."someNewTableData")" ) ); //#spellok

  // Test unique and make it invalid again so we get a status change
  w->mNewTableName->setText( u"someData"_s );
  QVERIFY( !w->isValid() );
  QCOMPARE( tableSpy.count(), 1 );
  arguments = tableSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), QString( "someData" ) );
  QCOMPARE( validationSpy.count(), 1 );
  arguments = validationSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toBool(), false );

  // Now select another schema
  index = w->mBrowserModel->findPath( u"pg:/PG_1/public"_s );
  QVERIFY( index.isValid() );
  w->mBrowserTreeView->scrollTo( w->mBrowserProxyModel.mapFromSource( index ) );
  rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center() );
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
  QVERIFY( w->uri().contains( R"("public"."someData")" ) );
#endif
}

void TestQgsNewDatabaseTableNameWidget::testWidgetSignalsGeopackage()
{
#ifdef ENABLE_PGTEST
  std::unique_ptr<QgsNewDatabaseTableNameWidget> w { std::make_unique<QgsNewDatabaseTableNameWidget>( nullptr, QStringList { "ogr" } ) };

  auto index = w->mBrowserModel->findPath( u"pg:/PG_1"_s );
  QVERIFY( index.isValid() );
  w->mBrowserModel->dataItem( index )->populate( true );
  w->mBrowserTreeView->expandAll();

  QVERIFY( !w->isValid() );

  QSignalSpy validationSpy( w.get(), SIGNAL( validationChanged( bool ) ) );
  QSignalSpy schemaSpy( w.get(), SIGNAL( schemaNameChanged( QString ) ) );
  const QSignalSpy tableSpy( w.get(), SIGNAL( tableNameChanged( QString ) ) );
  const QSignalSpy providerSpy( w.get(), SIGNAL( providerKeyChanged( QString ) ) );
  QSignalSpy uriSpy( w.get(), SIGNAL( uriChanged( QString ) ) );

  /*
  QDialog d;
  QVBoxLayout l;
  l.addWidget( w.get() );
  d.setLayout( &l );
  d.exec();
  //*/

  uriSpy.clear();
  index = w->mBrowserModel->findPath( u"gpkg:/%1"_s.arg( mGpkgPath ) );
  QVERIFY( index.isValid() );
  w->mBrowserTreeView->scrollTo( w->mBrowserProxyModel.mapFromSource( index ) );
  const auto rect = w->mBrowserTreeView->visualRect( w->mBrowserProxyModel.mapFromSource( index ) );
  QVERIFY( rect.isValid() );
  QTest::mouseClick( w->mBrowserTreeView->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center() );

  QVERIFY( !w->isValid() );
  QCOMPARE( schemaSpy.count(), 1 );
  auto arguments = schemaSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), mGpkgPath );
  QCOMPARE( uriSpy.count(), 1 );
  arguments = uriSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toString(), mGpkgPath );

  w->mNewTableName->setText( u"newTableName"_s );
  QVERIFY( w->isValid() );
  QCOMPARE( validationSpy.count(), 1 );
  arguments = validationSpy.takeLast();
  QCOMPARE( arguments.at( 0 ).toBool(), true );

  // Test getters
  QCOMPARE( w->table(), QString( "newTableName" ) );
  QCOMPARE( w->schema(), mGpkgPath );
  QCOMPARE( w->dataProviderKey(), QString( "ogr" ) );
  QCOMPARE( w->uri(), QString( mGpkgPath + u"|layername=newTableName"_s ) );
#endif
}

QGSTEST_MAIN( TestQgsNewDatabaseTableNameWidget )
#include "testqgsnewdatabasetablewidget.moc"
