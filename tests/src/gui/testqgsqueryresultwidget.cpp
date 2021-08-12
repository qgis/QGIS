/***************************************************************************
    testqgsqueryresultwidget.cpp
     ----------------------
    Date                 : Jan 2021
    Copyright            : (C) 2021 Alessandro Pasotti
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

#include "qgsqueryresultwidget.h"
#include "qgsqueryresultmodel.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include <QApplication>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>

class TestQgsQueryResultWidget: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

  private slots:
    void testWidget();
    void testWidgetCrash();
    void testWidgetInvalid();
    void testCodeEditorApis();

  private:

    QgsAbstractDatabaseProviderConnection *makeConn();

    std::unique_ptr<QgsAbstractDatabaseProviderConnection> mConn;

};

QgsAbstractDatabaseProviderConnection *TestQgsQueryResultWidget::makeConn()
{
  return static_cast<QgsAbstractDatabaseProviderConnection *>( QgsProviderRegistry::instance( )->providerMetadata( QStringLiteral( "postgres" ) )->createConnection( qgetenv( "QGIS_PGTEST_DB" ), QVariantMap() ) );
}

void TestQgsQueryResultWidget::initTestCase()
{
  QgsApplication::initQgis();
  mConn.reset( makeConn() );
  // Prepare data for fetching test
  mConn->execSql( QStringLiteral( "DROP TABLE IF EXISTS qgis_test.random_big_data" ) );
  mConn->execSql( QStringLiteral( "SELECT * INTO qgis_test.random_big_data FROM generate_series(1,100000) AS id, md5(random()::text) AS descr" ) );
}

void TestQgsQueryResultWidget::cleanupTestCase()
{
  mConn.reset( makeConn() );
  mConn->execSql( QStringLiteral( "DROP TABLE IF EXISTS qgis_test.random_big_data" ) );
}

void TestQgsQueryResultWidget::init()
{
}

void TestQgsQueryResultWidget::cleanup()
{
}

// Test do not crash when deleting the result while the model fetcher is running
void TestQgsQueryResultWidget::testWidgetCrash()
{
  // Make a copy
  mConn.reset( makeConn() );
  auto res = new QgsAbstractDatabaseProviderConnection::QueryResult( mConn->execSql( QStringLiteral( "SELECT * FROM qgis_test.random_big_data" ) ) );
  auto model = new QgsQueryResultModel( *res );
  bool exited { false };
  QTimer::singleShot( 1, model, [ & ] { delete res; } );
  QTimer::singleShot( 2, model, [ & ] { exited = true; } );
  while ( ! exited )
  {
    model->fetchMore( QModelIndex( ) );
    QgsApplication::processEvents();
  }
  const auto rowCount { model->rowCount( model->index( -1, -1 ) ) };
  QVERIFY( rowCount > 0 && rowCount < 100000 );
  delete model;

  // Test widget closed while fetching
  auto d = std::make_unique<QDialog>( );
  QVBoxLayout *l = new QVBoxLayout();
  QgsQueryResultWidget *w = new QgsQueryResultWidget( d.get(), makeConn() );
  w->setQuery( QStringLiteral( "SELECT * FROM qgis_test.random_big_data" ) );
  l->addWidget( w );
  d->setLayout( l );
  w->executeQuery();
  exited = false;
  QTimer::singleShot( 1, d.get(), [ & ] { exited = true; } );
  while ( ! exited )
    QgsApplication::processEvents();
}


void TestQgsQueryResultWidget::testWidget()
{
  auto d = std::make_unique<QDialog>( );
  QVBoxLayout *l = new QVBoxLayout();
  QgsQueryResultWidget *w = new QgsQueryResultWidget( d.get(), makeConn() );
  w->setQuery( QStringLiteral( "SELECT * FROM qgis_test.random_big_data" ) );
  l->addWidget( w );
  d->setLayout( l );
  // Uncomment for interactive testing:
  //d->exec();
  w->executeQuery();
  bool exited = false;
  connect( w, &QgsQueryResultWidget::firstResultBatchFetched, d.get(), [ & ] { exited = true; } );
  while ( ! exited )
    QgsApplication::processEvents();
  const auto rowCount { w->mModel->rowCount( w->mModel->index( -1, -1 ) ) };
  QVERIFY( rowCount > 0 && rowCount < 100000 );
}

void TestQgsQueryResultWidget::testWidgetInvalid()
{
  const QgsQueryResultWidget w( nullptr, nullptr );
  Q_UNUSED( w )
}

void TestQgsQueryResultWidget::testCodeEditorApis()
{
  auto w = std::make_unique<QgsQueryResultWidget>( nullptr, makeConn() );
  bool exited = false;
  connect( w->mApiFetcher.get(), &QgsConnectionsApiFetcher::fetchingFinished, w.get(), [ & ] { exited = true; } );
  while ( ! exited )
    QgsApplication::processEvents();
  QVERIFY( w->mSqlEditor->extraKeywords().contains( QStringLiteral( "qgis_test" ) ) );
  QVERIFY( w->mSqlEditor->extraKeywords().contains( QStringLiteral( "random_big_data" ) ) );
  QVERIFY( w->mSqlEditor->extraKeywords().contains( QStringLiteral( "descr" ) ) );

  // Test feedback interrupt
  w = std::make_unique<QgsQueryResultWidget>( nullptr, makeConn() );
  QTimer::singleShot( 0, w.get(), [ & ]
  {
    QTest::mousePress( w->mStopButton, Qt::MouseButton::LeftButton );
  } );
  connect( w->mApiFetcher.get(), &QgsConnectionsApiFetcher::fetchingFinished, w.get(), [ & ] { exited = true; } );
  while ( ! exited )
    QgsApplication::processEvents();

}


QGSTEST_MAIN( TestQgsQueryResultWidget )
#include "testqgsqueryresultwidget.moc"
