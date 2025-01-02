/***************************************************************************
  testqgsogrprovider.cpp - TestQgsOgrProvider

 ---------------------
 begin                : 10.11.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

//qgis includes...
#include "qgis.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsprovidermetadata.h"
#include "qgsprovidersublayerdetails.h"

#include <QObject>
#include <QThread>

#include <cpl_conv.h>
#include <gdal.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the ogr provider
 */
class TestQgsOgrProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsOgrProvider()
      : QgsTest( QStringLiteral( "OGR Provider Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void setupProxy();
    void decodeUri();
    void encodeUri();
    void testThread();
    void testCsvFeatureAddition();
    void absoluteRelativeUri();
    void testExtent();
    void testVsiCredentialOptions();
    void testVsiCredentialOptionsQuerySublayers();

  private:
    QString mTestDataDir;
  signals:

  public slots:
};


//runs before all tests
void TestQgsOgrProvider::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsSettings().clear();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}

//runs after all tests
void TestQgsOgrProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOgrProvider::setupProxy()
{
  QgsSettings settings;
  {
    settings.setValue( QStringLiteral( "proxy/proxyEnabled" ), true );
    settings.setValue( QStringLiteral( "proxy/proxyPort" ), QStringLiteral( "38124" ) );
    settings.setValue( QStringLiteral( "proxy/proxyHost" ), QStringLiteral( "myproxyhostname.com" ) );
    settings.setValue( QStringLiteral( "proxy/proxyUser" ), QStringLiteral( "username" ) );
    settings.setValue( QStringLiteral( "proxy/proxyPassword" ), QStringLiteral( "password" ) );
    settings.setValue( QStringLiteral( "proxy/proxyExcludedUrls" ), QStringLiteral( "http://www.myhost.com|http://www.myotherhost.com" ) );
    QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
    const QgsVectorLayer vl( mTestDataDir + '/' + QStringLiteral( "lines.shp" ), QStringLiteral( "proxy_test" ), QLatin1String( "ogr" ) );
    QVERIFY( vl.isValid() );
    const char *proxyConfig = CPLGetConfigOption( "GDAL_HTTP_PROXY", nullptr );
    QCOMPARE( proxyConfig, "myproxyhostname.com:38124" );
    const char *proxyCredentials = CPLGetConfigOption( "GDAL_HTTP_PROXYUSERPWD", nullptr );
    QCOMPARE( proxyCredentials, "username:password" );
  }

  {
    // Test partial config
    settings.setValue( QStringLiteral( "proxy/proxyEnabled" ), true );
    settings.remove( QStringLiteral( "proxy/proxyPort" ) );
    settings.setValue( QStringLiteral( "proxy/proxyHost" ), QStringLiteral( "myproxyhostname.com" ) );
    settings.setValue( QStringLiteral( "proxy/proxyUser" ), QStringLiteral( "username" ) );
    settings.remove( QStringLiteral( "proxy/proxyPassword" ) );
    QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
    const QgsVectorLayer vl( mTestDataDir + '/' + QStringLiteral( "lines.shp" ), QStringLiteral( "proxy_test" ), QLatin1String( "ogr" ) );
    QVERIFY( vl.isValid() );
    const char *proxyConfig = CPLGetConfigOption( "GDAL_HTTP_PROXY", nullptr );
    QCOMPARE( proxyConfig, "myproxyhostname.com" );
    const char *proxyCredentials = CPLGetConfigOption( "GDAL_HTTP_PROXYUSERPWD", nullptr );
    QCOMPARE( proxyCredentials, "username" );
  }

  // cleanup
  QgsSettings().clear();
  CPLSetConfigOption( "GDAL_HTTP_PROXY", nullptr );
  CPLSetConfigOption( "GDAL_HTTP_PROXYUSERPWD", nullptr );
}

void TestQgsOgrProvider::decodeUri()
{
  auto parts( QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "MySQL:database_name,host=localhost,port=3306 authcfg='f8wwfx8'" ) ) );
  QCOMPARE( parts.size(), 5 );
  QCOMPARE( parts.value( QStringLiteral( "databaseName" ) ).toString(), QString( "database_name" ) );
  QVERIFY( parts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "MySQL:database_name,host=localhost,port=3306" ) );
  QCOMPARE( parts.value( QStringLiteral( "authcfg" ) ).toString(), QString( "f8wwfx8" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "MySQL:database_name,host=localhost,port=3306 authcfg='f8wwfx8'" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "MYSQL:westholland,user=root,password=psv9570,port=3306,tables=bedrijven" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "databaseName" ) ).toString(), QString( "westholland" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "MYSQL:westholland,user=root,password=psv9570,port=3306,tables=bedrijven" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "MYSQL:westholland|layername=foo" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "databaseName" ) ).toString(), QString( "westholland" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "foo" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "MYSQL:westholland" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layername=a_layer" ) );
  QCOMPARE( parts.size(), 3 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layerid=4" ) );
  QCOMPARE( parts.size(), 3 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( parts.value( QStringLiteral( "layerId" ) ).toInt(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layerid=4|layername=a_layer4" ) );
  QCOMPARE( parts.size(), 3 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer4" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() ); // layername should take preference
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layername=a_layer|geometrytype=point" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|geometrytype=point|layername=a_layer" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|geometrytype=point|layerid=4" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( parts.value( QStringLiteral( "layerId" ) ).toInt(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|geometrytype=point|layerid=4|layername=a_layer" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|geometrytype=point|layername=a_layer_with_geometrytype" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer_with_geometrytype" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|geometrytype=point" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|subset=A IN (3,4,5) or \"b\"='x|y'" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "subset" ) ).toString(), QString( "A IN (3,4,5) or \"b\"='x|y'" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layername=my_subset|subset=A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "my_subset" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "subset" ) ).toString(), QString( "A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|subset=A IN (3,4,5) or \"b\"='x|layerid'|layername=my_subset" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "my_subset" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "subset" ) ).toString(), QString( "A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layerid=4|subset=A IN (3,4,5) or \"b\"='x|layerid'|layername=my_subset" ) );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "my_subset" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "subset" ) ).toString(), QString( "A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layerid=4|subset=A IN (3,4,5) or \n\"b\"='x|layerid'|geometrytype=polygonz|layername=my_subset" ) );
  QCOMPARE( parts.size(), 5 );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "my_subset" ) );
  QCOMPARE( parts.value( QStringLiteral( "geometryType" ) ).toString(), QString( "polygonz" ) );
  QVERIFY( !parts.value( QStringLiteral( "layerId" ) ).isValid() );
  QCOMPARE( parts.value( QStringLiteral( "subset" ) ).toString(), QString( "A IN (3,4,5) or \n\"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|option:FOO=BAR|option:FOO2=BAR2" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "openOptions" ) ).toStringList(), QStringList() << QStringLiteral( "FOO=BAR" ) << QStringLiteral( "FOO2=BAR2" ) );

  // test authcfg with vsicurl URI
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.gpkg authcfg='1234567'" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "https://www.qgis.org/dataset.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "vsiPrefix" ) ).toString(), QString( "/vsicurl/" ) );
  QCOMPARE( parts.value( QStringLiteral( "authcfg" ) ).toString(), QString( "1234567" ) );

  // vsis3
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/vsis3/nz-elevation/auckland/auckland-north_2016-2018/auckland.shp" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "nz-elevation/auckland/auckland-north_2016-2018/auckland.shp" ) );
  QCOMPARE( parts.value( QStringLiteral( "vsiPrefix" ) ).toString(), QString( "/vsis3/" ) );
}

void TestQgsOgrProvider::encodeUri()
{
  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/user/test.gpkg" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg" ) );

  // layerName only
  parts.insert( QStringLiteral( "layerName" ), QStringLiteral( "test" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg|layername=test" ) );
  parts.remove( QStringLiteral( "layerName" ) );

  // layerId only
  parts.insert( QStringLiteral( "layerId" ), QStringLiteral( "testid" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg|layerid=testid" ) );

  // Both layerName and layerId: layerName takes precedence
  parts.insert( QStringLiteral( "layerName" ), QStringLiteral( "test" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg|layername=test" ) );

  parts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "point" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg|layername=test|geometrytype=point" ) );

  parts.insert( QStringLiteral( "subset" ), QStringLiteral( "\"a\"='b'" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg|layername=test|geometrytype=point|subset=\"a\"='b'" ) );

  parts.clear();
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/user/test.gpkg" ) );
  parts.insert( QStringLiteral( "openOptions" ), QStringList() << QStringLiteral( "FOO=BAR" ) << QStringLiteral( "FOO2=BAR2" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/home/user/test.gpkg|option:FOO=BAR|option:FOO2=BAR2" ) );

  // test authcfg with vsicurl
  parts.clear();
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.gpkg" ) );
  parts.insert( QStringLiteral( "authcfg" ), QStringLiteral( "1234567" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.gpkg authcfg='1234567'" ) );

  parts.clear();
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "https://www.qgis.org/dataset.gpkg" ) );
  parts.insert( QStringLiteral( "vsiPrefix" ), QStringLiteral( "/vsicurl/" ) );
  parts.insert( QStringLiteral( "authcfg" ), QStringLiteral( "1234567" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.gpkg authcfg='1234567'" ) );

  // vsis3
  parts.clear();
  parts.insert( QStringLiteral( "vsiPrefix" ), QStringLiteral( "/vsis3/" ) );
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "nz-elevation/auckland/auckland-north_2016-2018/auckland.gpkg" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts ), QStringLiteral( "/vsis3/nz-elevation/auckland/auckland-north_2016-2018/auckland.gpkg" ) );
}

class ReadVectorLayer : public QThread
{
    Q_OBJECT

  public:
    ReadVectorLayer( const QString &filePath, QMutex &mutex, QWaitCondition &waitForVlCreation, QWaitCondition &waitForProcessEvents )
      : _filePath( filePath ), _mutex( mutex ), _waitForVlCreation( waitForVlCreation ), _waitForProcessEvents( waitForProcessEvents ) {}

    void run() override
    {
      QgsVectorLayer *vl2 = new QgsVectorLayer( _filePath, QStringLiteral( "thread_test" ), QLatin1String( "ogr" ) );

      QgsFeature f;
      QVERIFY( vl2->getFeatures().nextFeature( f ) );

      _mutex.lock();
      _waitForVlCreation.wakeAll();
      _mutex.unlock();

      _mutex.lock();
      _waitForProcessEvents.wait( &_mutex );
      _mutex.unlock();

      delete vl2;
    }

  private:
    QString _filePath;
    QMutex &_mutex;
    QWaitCondition &_waitForVlCreation;
    QWaitCondition &_waitForProcessEvents;
};

void failOnWarning( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
  Q_UNUSED( context );

  switch ( type )
  {
    case QtWarningMsg:
      QFAIL( QString( "No Qt warning message expect : %1" ).arg( msg ).toUtf8() );
    default:;
  }
}

void TestQgsOgrProvider::testThread()
{
  // Disabled by @m-kuhn
  // This test is flaky
  // See https://travis-ci.org/qgis/QGIS/jobs/505008602#L6464-L7108
  if ( QgsTest::isCIRun() )
    QSKIP( "This test is disabled on Travis CI environment" );

  // After reading a QgsVectorLayer (getFeatures) from another thread the QgsOgrConnPoolGroup starts
  // an expiration timer. The timer belongs to the main thread in order to listening the event
  // loop and is parented to its QgsOgrConnPoolGroup. So when we delete the QgsVectorLayer, the
  // QgsConnPoolGroup and the timer are subsequently deleted from another thread. This leads to
  // segfault later when the expiration time reaches its timeout.

  QMutex mutex;
  QWaitCondition waitForVlCreation;
  QWaitCondition waitForProcessEvents;

  const QString filePath = mTestDataDir + '/' + QStringLiteral( "lines.shp" );
  QThread *thread = new ReadVectorLayer( filePath, mutex, waitForVlCreation, waitForProcessEvents );

  thread->start();

  mutex.lock();
  waitForVlCreation.wait( &mutex );
  mutex.unlock();

  // make sure timer as been started
  QCoreApplication::processEvents();

  qInstallMessageHandler( failOnWarning );

  mutex.lock();
  waitForProcessEvents.wakeAll();
  mutex.unlock();

  thread->wait();
  qInstallMessageHandler( 0 );
}

void TestQgsOgrProvider::testCsvFeatureAddition()
{
  const QString csvFilename = QDir::tempPath() + "/csvfeatureadditiontest.csv";
  QFile csvFile( csvFilename );
  if ( csvFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream textStream( &csvFile );
    textStream << QLatin1String( "col1,col2,col3\n0,0,\"csv0\"\n" );
    csvFile.close();
  }

  QgsVectorLayer *csvLayer = new QgsVectorLayer( csvFilename, QStringLiteral( "csv" ) );
  QVERIFY( csvLayer->isValid() );
  QCOMPARE( csvLayer->featureCount(), 1 );

  QgsFeature f1( csvLayer->fields() );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 1 );
  f1.setAttribute( 2, QLatin1String( "csv1" ) );
  QgsFeature f2( csvLayer->fields() );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 2 );
  f2.setAttribute( 2, QLatin1String( "csv2" ) );

  QgsFeatureList features;
  features << f1 << f2;
  csvLayer->dataProvider()->addFeatures( features );
  QCOMPARE( features.at( 0 ).id(), 2 );
  QCOMPARE( features.at( 1 ).id(), 3 );

  csvLayer->setSubsetString( QStringLiteral( "col1 = '2'" ) );
  QCOMPARE( csvLayer->featureCount(), 1 );

  features.clear();
  features << f1;
  csvLayer->dataProvider()->addFeatures( features );
  QCOMPARE( features.at( 0 ).id(), 4 );

  delete csvLayer;
  QFile::remove( csvFilename );
}

void TestQgsOgrProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/project.qgs" ) ) );

  QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( "ogr" );
  QVERIFY( ogrMetadata );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/points.shp" );
  QString relativeUri = QStringLiteral( "./points.shp" );
  QCOMPARE( ogrMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( ogrMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );

  absoluteUri = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/points_gpkg.gpkg|layername=points_small" );
  relativeUri = QStringLiteral( "./points_gpkg.gpkg|layername=points_small" );
  QCOMPARE( ogrMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( ogrMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsOgrProvider::testExtent()
{
  QString uri2D = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/points_gpkg.gpkg|layername=points_small" );
  QgsVectorLayer *layer2D = new QgsVectorLayer( uri2D, QStringLiteral( "gpkg" ), QLatin1String( "ogr" ) );
  QVERIFY( layer2D->isValid() );
  QCOMPARE( layer2D->extent(), QgsRectangle( -102.436, 40.578, -93.1608, 41.2405 ) );
  QCOMPARE( layer2D->extent3D(), QgsBox3D( -102.436, 40.578, std::numeric_limits<double>::quiet_NaN(), -93.1608, 41.2405, std::numeric_limits<double>::quiet_NaN() ) );
  delete layer2D;

  QString uri3D = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/3d/points_with_z.shp" );
  // QString uri3D = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/3d/earth_size_sphere_4978.gpkg|layername=earth_size_sphere_4978" );
  QgsVectorLayer *layer3D = new QgsVectorLayer( uri3D, QStringLiteral( "shp" ), QLatin1String( "ogr" ) );
  QVERIFY( layer3D->isValid() );
  QGSCOMPARENEAR( layer3D->extent().xMinimum(), 321384.94, 0.001 );
  QGSCOMPARENEAR( layer3D->extent().xMaximum(), 322342.3, 0.001 );
  QGSCOMPARENEAR( layer3D->extent().yMinimum(), 129147.09, 0.001 );
  QGSCOMPARENEAR( layer3D->extent().yMaximum(), 130554.6, 0.001 );

  QGSCOMPARENEAR( layer3D->extent3D().xMinimum(), 321384.94, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().xMaximum(), 322342.3, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().yMinimum(), 129147.09, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().yMaximum(), 130554.6, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().zMinimum(), 64.9, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().zMaximum(), 105.6, 0.001 );
  delete layer3D;

  uri3D = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/3d/points_with_z.gpkg|layername=points_with_z" );
  // QString uri3D = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/3d/earth_size_sphere_4978.gpkg|layername=earth_size_sphere_4978" );
  layer3D = new QgsVectorLayer( uri3D, QStringLiteral( "gpkg" ), QLatin1String( "ogr" ) );
  QVERIFY( layer3D->isValid() );
  QGSCOMPARENEAR( layer3D->extent().xMinimum(), -102.436, 0.001 );
  QGSCOMPARENEAR( layer3D->extent().xMaximum(), -93.160, 0.001 );
  QGSCOMPARENEAR( layer3D->extent().yMinimum(), 40.577, 0.001 );
  QGSCOMPARENEAR( layer3D->extent().yMaximum(), 41.240, 0.001 );

  QGSCOMPARENEAR( layer3D->extent3D().xMinimum(), -102.436, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().xMaximum(), -93.160, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().yMinimum(), 40.577, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().yMaximum(), 41.240, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().zMinimum(), -50.0, 0.001 );
  QGSCOMPARENEAR( layer3D->extent3D().zMaximum(), 75.0, 0.001 );
  delete layer3D;
}

void TestQgsOgrProvider::testVsiCredentialOptions()
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 6, 0 )
  // test that credential options are correctly set when layer URI specifies them

  // if actual aws dataset proves flaky, use this instead:
  // std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "/vsis3/testbucket/test|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-1|credential:AWS_S3_ENDPOINT=localhost" ), QStringLiteral( "test" ), QStringLiteral( "ogr" ) );
  std::unique_ptr<QgsVectorLayer> vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES" ), QStringLiteral( "test" ), QStringLiteral( "ogr" ) );

  // confirm that GDAL VSI configuration options are set
  QString noSign( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "YES" ) );
  QString region( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  // different bucket
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QString() );
  region = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  QCOMPARE( vl->dataProvider()->dataSourceUri(), QStringLiteral( "/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES" ) );

  // credentials should be bucket specific
  std::unique_ptr<QgsVectorLayer> vl2 = std::make_unique<QgsVectorLayer>( QStringLiteral( "/vsis3/ogranother/subfolder/subfolder2/test|credential:AWS_NO_SIGN_REQUEST=NO|credential:AWS_REGION=eu-central-2|credential:AWS_S3_ENDPOINT=localhost" ), QStringLiteral( "test" ), QStringLiteral( "ogr" ) );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "YES" ) );
  region = QString( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/ogranother/subfolder/subfolder2", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "NO" ) );
  region = QString( VSIGetPathSpecificOption( "/vsis3/ogranother/subfolder/subfolder2", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QStringLiteral( "eu-central-2" ) );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/ogranother", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QString() );
  region = QString( VSIGetPathSpecificOption( "/vsis3/ogranother", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  QCOMPARE( vl2->dataProvider()->dataSourceUri(), QStringLiteral( "/vsis3/ogranother/subfolder/subfolder2/test|credential:AWS_NO_SIGN_REQUEST=NO|credential:AWS_REGION=eu-central-2|credential:AWS_S3_ENDPOINT=localhost" ) );

  // cleanup
  VSIClearPathSpecificOptions( "/vsis3/cdn.proj.org" );
  VSIClearPathSpecificOptions( "/vsis3/ogranother/subfolder/subfolder2" );
#endif
}

void TestQgsOgrProvider::testVsiCredentialOptionsQuerySublayers()
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 6, 0 )
  QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( "ogr" );
  QVERIFY( ogrMetadata );

  // test that credential options are correctly handled when querying sublayers

  // if actual aws dataset proves flaky, use this instead:
  // QList< QgsProviderSublayerDetails> subLayers = ogrMetadata->querySublayers( QStringLiteral( "/vsis3/sublayerstestbucket/test.shp|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-3|credential:AWS_S3_ENDPOINT=localhost" ) );
  QList<QgsProviderSublayerDetails> subLayers = ogrMetadata->querySublayers( QStringLiteral( "/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES" ) );

  QCOMPARE( subLayers.size(), 1 );
  QCOMPARE( subLayers.at( 0 ).name(), QStringLiteral( "files" ) );
  QCOMPARE( subLayers.at( 0 ).uri(), QStringLiteral( "/vsis3/cdn.proj.org/files.geojson|layername=files|credential:AWS_NO_SIGN_REQUEST=YES" ) );
  QCOMPARE( subLayers.at( 0 ).providerKey(), QStringLiteral( "ogr" ) );
  QCOMPARE( subLayers.at( 0 ).type(), Qgis::LayerType::Vector );
  QCOMPARE( subLayers.at( 0 ).wkbType(), Qgis::WkbType::Unknown );

  // confirm that GDAL VSI configuration options are set
  QString noSign( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "YES" ) );
  QString region( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  //subLayers = ogrMetadata->querySublayers( QStringLiteral( "/vsis3/sublayerstestbucket/test.shp|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-3|credential:AWS_S3_ENDPOINT=localhost" ), Qgis::SublayerQueryFlag::FastScan );
  subLayers = ogrMetadata->querySublayers( QStringLiteral( "/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES" ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( subLayers.size(), 1 );
  QCOMPARE( subLayers.at( 0 ).name(), QStringLiteral( "files" ) );
  QCOMPARE( subLayers.at( 0 ).uri(), QStringLiteral( "/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES" ) );
  QCOMPARE( subLayers.at( 0 ).providerKey(), QStringLiteral( "ogr" ) );
  QCOMPARE( subLayers.at( 0 ).type(), Qgis::LayerType::Vector );
  QCOMPARE( subLayers.at( 0 ).wkbType(), Qgis::WkbType::Unknown );

  // cleanup
  VSIClearPathSpecificOptions( "/vsis3/cdn.proj.org" );
#endif
}


QGSTEST_MAIN( TestQgsOgrProvider )
#include "testqgsogrprovider.moc"
