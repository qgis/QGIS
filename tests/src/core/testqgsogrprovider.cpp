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
      : QgsTest( u"OGR Provider Tests"_s ) {}

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
    void testJSONFields_data();
    void testJSONFields();

  private:
    QString mTestDataDir;
  signals:

  public slots:
};


//runs before all tests
void TestQgsOgrProvider::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
    settings.setValue( u"proxy/proxyEnabled"_s, true );
    settings.setValue( u"proxy/proxyPort"_s, u"38124"_s );
    settings.setValue( u"proxy/proxyHost"_s, u"myproxyhostname.com"_s );
    settings.setValue( u"proxy/proxyUser"_s, u"username"_s );
    settings.setValue( u"proxy/proxyPassword"_s, u"password"_s );
    settings.setValue( u"proxy/proxyExcludedUrls"_s, u"http://www.myhost.com|http://www.myotherhost.com"_s );
    QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
    const QgsVectorLayer vl( mTestDataDir + '/' + u"lines.shp"_s, u"proxy_test"_s, "ogr"_L1 );
    QVERIFY( vl.isValid() );
    const char *proxyConfig = CPLGetConfigOption( "GDAL_HTTP_PROXY", nullptr );
    QCOMPARE( proxyConfig, "myproxyhostname.com:38124" );
    const char *proxyCredentials = CPLGetConfigOption( "GDAL_HTTP_PROXYUSERPWD", nullptr );
    QCOMPARE( proxyCredentials, "username:password" );
  }

  {
    // Test partial config
    settings.setValue( u"proxy/proxyEnabled"_s, true );
    settings.remove( u"proxy/proxyPort"_s );
    settings.setValue( u"proxy/proxyHost"_s, u"myproxyhostname.com"_s );
    settings.setValue( u"proxy/proxyUser"_s, u"username"_s );
    settings.remove( u"proxy/proxyPassword"_s );
    QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
    const QgsVectorLayer vl( mTestDataDir + '/' + u"lines.shp"_s, u"proxy_test"_s, "ogr"_L1 );
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
  auto parts( QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"MySQL:database_name,host=localhost,port=3306 authcfg='f8wwfx8'"_s ) );
  QCOMPARE( parts.size(), 5 );
  QCOMPARE( parts.value( u"databaseName"_s ).toString(), QString( "database_name" ) );
  QVERIFY( parts.value( u"layerName"_s ).toString().isEmpty() );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "MySQL:database_name,host=localhost,port=3306" ) );
  QCOMPARE( parts.value( u"authcfg"_s ).toString(), QString( "f8wwfx8" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"MySQL:database_name,host=localhost,port=3306 authcfg='f8wwfx8'"_s );

  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"MYSQL:westholland,user=root,password=psv9570,port=3306,tables=bedrijven"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"databaseName"_s ).toString(), QString( "westholland" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "MYSQL:westholland,user=root,password=psv9570,port=3306,tables=bedrijven" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"MYSQL:westholland|layername=foo"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"databaseName"_s ).toString(), QString( "westholland" ) );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "foo" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "MYSQL:westholland" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layername=a_layer"_s );
  QCOMPARE( parts.size(), 3 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layerid=4"_s );
  QCOMPARE( parts.size(), 3 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QCOMPARE( parts.value( u"layerId"_s ).toInt(), 4 );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layerid=4|layername=a_layer4"_s );
  QCOMPARE( parts.size(), 3 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "a_layer4" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() ); // layername should take preference
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layername=a_layer|geometrytype=point"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|geometrytype=point|layername=a_layer"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|geometrytype=point|layerid=4"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QCOMPARE( parts.value( u"layerId"_s ).toInt(), 4 );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|geometrytype=point|layerid=4|layername=a_layer"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "a_layer" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|geometrytype=point|layername=a_layer_with_geometrytype"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "a_layer_with_geometrytype" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|geometrytype=point"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "point" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|subset=A IN (3,4,5) or \"b\"='x|y'"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"subset"_s ).toString(), QString( "A IN (3,4,5) or \"b\"='x|y'" ) );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layername=my_subset|subset=A IN (3,4,5) or \"b\"='x|layerid'"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "my_subset" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"subset"_s ).toString(), QString( "A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|subset=A IN (3,4,5) or \"b\"='x|layerid'|layername=my_subset"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "my_subset" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"subset"_s ).toString(), QString( "A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layerid=4|subset=A IN (3,4,5) or \"b\"='x|layerid'|layername=my_subset"_s );
  QCOMPARE( parts.size(), 4 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "my_subset" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"subset"_s ).toString(), QString( "A IN (3,4,5) or \"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|layerid=4|subset=A IN (3,4,5) or \n\"b\"='x|layerid'|geometrytype=polygonz|layername=my_subset"_s );
  QCOMPARE( parts.size(), 5 );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString( "my_subset" ) );
  QCOMPARE( parts.value( u"geometryType"_s ).toString(), QString( "polygonz" ) );
  QVERIFY( !parts.value( u"layerId"_s ).isValid() );
  QCOMPARE( parts.value( u"subset"_s ).toString(), QString( "A IN (3,4,5) or \n\"b\"='x|layerid'" ) );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );

  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/path/to/a/geopackage.gpkg|option:FOO=BAR|option:FOO2=BAR2"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "/path/to/a/geopackage.gpkg" ) );
  QCOMPARE( parts.value( u"openOptions"_s ).toStringList(), QStringList() << u"FOO=BAR"_s << u"FOO2=BAR2"_s );

  // test authcfg with vsicurl URI
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/vsicurl/https://www.qgis.org/dataset.gpkg authcfg='1234567'"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "https://www.qgis.org/dataset.gpkg" ) );
  QCOMPARE( parts.value( u"vsiPrefix"_s ).toString(), QString( "/vsicurl/" ) );
  QCOMPARE( parts.value( u"authcfg"_s ).toString(), QString( "1234567" ) );

  // vsis3
  parts = QgsProviderRegistry::instance()->decodeUri( u"ogr"_s, u"/vsis3/nz-elevation/auckland/auckland-north_2016-2018/auckland.shp"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), QString( "nz-elevation/auckland/auckland-north_2016-2018/auckland.shp" ) );
  QCOMPARE( parts.value( u"vsiPrefix"_s ).toString(), QString( "/vsis3/" ) );
}

void TestQgsOgrProvider::encodeUri()
{
  QVariantMap parts;
  parts.insert( u"path"_s, u"/home/user/test.gpkg"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg"_s );

  // layerName only
  parts.insert( u"layerName"_s, u"test"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg|layername=test"_s );
  parts.remove( u"layerName"_s );

  // layerId only
  parts.insert( u"layerId"_s, u"testid"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg|layerid=testid"_s );

  // Both layerName and layerId: layerName takes precedence
  parts.insert( u"layerName"_s, u"test"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg|layername=test"_s );

  parts.insert( u"geometryType"_s, u"point"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg|layername=test|geometrytype=point"_s );

  parts.insert( u"subset"_s, u"\"a\"='b'"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg|layername=test|geometrytype=point|subset=\"a\"='b'"_s );

  parts.clear();
  parts.insert( u"path"_s, u"/home/user/test.gpkg"_s );
  parts.insert( u"openOptions"_s, QStringList() << u"FOO=BAR"_s << u"FOO2=BAR2"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/home/user/test.gpkg|option:FOO=BAR|option:FOO2=BAR2"_s );

  // test authcfg with vsicurl
  parts.clear();
  parts.insert( u"path"_s, u"/vsicurl/https://www.qgis.org/dataset.gpkg"_s );
  parts.insert( u"authcfg"_s, u"1234567"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/vsicurl/https://www.qgis.org/dataset.gpkg authcfg='1234567'"_s );

  parts.clear();
  parts.insert( u"path"_s, u"https://www.qgis.org/dataset.gpkg"_s );
  parts.insert( u"vsiPrefix"_s, u"/vsicurl/"_s );
  parts.insert( u"authcfg"_s, u"1234567"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/vsicurl/https://www.qgis.org/dataset.gpkg authcfg='1234567'"_s );

  // vsis3
  parts.clear();
  parts.insert( u"vsiPrefix"_s, u"/vsis3/"_s );
  parts.insert( u"path"_s, u"nz-elevation/auckland/auckland-north_2016-2018/auckland.gpkg"_s );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, parts ), u"/vsis3/nz-elevation/auckland/auckland-north_2016-2018/auckland.gpkg"_s );
}

class ReadVectorLayer : public QThread
{
    Q_OBJECT

  public:
    ReadVectorLayer( const QString &filePath, QMutex &mutex, QWaitCondition &waitForVlCreation, QWaitCondition &waitForProcessEvents )
      : _filePath( filePath ), _mutex( mutex ), _waitForVlCreation( waitForVlCreation ), _waitForProcessEvents( waitForProcessEvents ) {}

    void run() override
    {
      QgsVectorLayer *vl2 = new QgsVectorLayer( _filePath, u"thread_test"_s, "ogr"_L1 );

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

  const QString filePath = mTestDataDir + '/' + u"lines.shp"_s;
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
  qInstallMessageHandler( nullptr );
}

void TestQgsOgrProvider::testCsvFeatureAddition()
{
  const QString csvFilename = QDir::tempPath() + "/csvfeatureadditiontest.csv";
  QFile csvFile( csvFilename );
  if ( csvFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream textStream( &csvFile );
    textStream << "col1,col2,col3\n0,0,\"csv0\"\n"_L1;
    csvFile.close();
  }

  QgsVectorLayer *csvLayer = new QgsVectorLayer( csvFilename, u"csv"_s );
  QVERIFY( csvLayer->isValid() );
  QCOMPARE( csvLayer->featureCount(), 1 );

  QgsFeature f1( csvLayer->fields() );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 1 );
  f1.setAttribute( 2, "csv1"_L1 );
  QgsFeature f2( csvLayer->fields() );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 2 );
  f2.setAttribute( 2, "csv2"_L1 );

  QgsFeatureList features;
  features << f1 << f2;
  csvLayer->dataProvider()->addFeatures( features );
  QCOMPARE( features.at( 0 ).id(), 2 );
  QCOMPARE( features.at( 1 ).id(), 3 );

  csvLayer->setSubsetString( u"col1 = '2'"_s );
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
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );

  QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( "ogr" );
  QVERIFY( ogrMetadata );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  QString relativeUri = u"./points.shp"_s;
  QCOMPARE( ogrMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( ogrMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );

  absoluteUri = QStringLiteral( TEST_DATA_DIR ) + u"/points_gpkg.gpkg|layername=points_small"_s;
  relativeUri = u"./points_gpkg.gpkg|layername=points_small"_s;
  QCOMPARE( ogrMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( ogrMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsOgrProvider::testExtent()
{
  QString uri2D = QStringLiteral( TEST_DATA_DIR ) + u"/points_gpkg.gpkg|layername=points_small"_s;
  QgsVectorLayer *layer2D = new QgsVectorLayer( uri2D, u"gpkg"_s, "ogr"_L1 );
  QVERIFY( layer2D->isValid() );
  QCOMPARE( layer2D->extent(), QgsRectangle( -102.436, 40.578, -93.1608, 41.2405 ) );
  QCOMPARE( layer2D->extent3D(), QgsBox3D( -102.436, 40.578, std::numeric_limits<double>::quiet_NaN(), -93.1608, 41.2405, std::numeric_limits<double>::quiet_NaN() ) );
  delete layer2D;

  QString uri3D = QStringLiteral( TEST_DATA_DIR ) + u"/3d/points_with_z.shp"_s;
  // QString uri3D = QStringLiteral( TEST_DATA_DIR ) + u"/3d/earth_size_sphere_4978.gpkg|layername=earth_size_sphere_4978"_s;
  QgsVectorLayer *layer3D = new QgsVectorLayer( uri3D, u"shp"_s, "ogr"_L1 );
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

  uri3D = QStringLiteral( TEST_DATA_DIR ) + u"/3d/points_with_z.gpkg|layername=points_with_z"_s;
  // QString uri3D = QStringLiteral( TEST_DATA_DIR ) + u"/3d/earth_size_sphere_4978.gpkg|layername=earth_size_sphere_4978"_s;
  layer3D = new QgsVectorLayer( uri3D, u"gpkg"_s, "ogr"_L1 );
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
  // std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( u"/vsis3/testbucket/test|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-1|credential:AWS_S3_ENDPOINT=localhost"_s, u"test"_s, u"ogr"_s );
  auto vl = std::make_unique<QgsVectorLayer>( u"/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES"_s, u"test"_s, u"ogr"_s );

  // confirm that GDAL VSI configuration options are set
  QString noSign( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, u"YES"_s );
  QString region( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  // different bucket
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QString() );
  region = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  QCOMPARE( vl->dataProvider()->dataSourceUri(), u"/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES"_s );

  // credentials should be bucket specific
  auto vl2 = std::make_unique<QgsVectorLayer>( u"/vsis3/ogranother/subfolder/subfolder2/test|credential:AWS_NO_SIGN_REQUEST=NO|credential:AWS_REGION=eu-central-2|credential:AWS_S3_ENDPOINT=localhost"_s, u"test"_s, u"ogr"_s );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, u"YES"_s );
  region = QString( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/ogranother/subfolder/subfolder2", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, u"NO"_s );
  region = QString( VSIGetPathSpecificOption( "/vsis3/ogranother/subfolder/subfolder2", "AWS_REGION", nullptr ) );
  QCOMPARE( region, u"eu-central-2"_s );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/ogranother", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QString() );
  region = QString( VSIGetPathSpecificOption( "/vsis3/ogranother", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  QCOMPARE( vl2->dataProvider()->dataSourceUri(), u"/vsis3/ogranother/subfolder/subfolder2/test|credential:AWS_NO_SIGN_REQUEST=NO|credential:AWS_REGION=eu-central-2|credential:AWS_S3_ENDPOINT=localhost"_s );

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
  // QList< QgsProviderSublayerDetails> subLayers = ogrMetadata->querySublayers( u"/vsis3/sublayerstestbucket/test.shp|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-3|credential:AWS_S3_ENDPOINT=localhost"_s );
  QList<QgsProviderSublayerDetails> subLayers = ogrMetadata->querySublayers( u"/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES"_s );

  QCOMPARE( subLayers.size(), 1 );
  QCOMPARE( subLayers.at( 0 ).name(), u"files"_s );
  QCOMPARE( subLayers.at( 0 ).uri(), u"/vsis3/cdn.proj.org/files.geojson|layername=files|credential:AWS_NO_SIGN_REQUEST=YES"_s );
  QCOMPARE( subLayers.at( 0 ).providerKey(), u"ogr"_s );
  QCOMPARE( subLayers.at( 0 ).type(), Qgis::LayerType::Vector );
  QCOMPARE( subLayers.at( 0 ).wkbType(), Qgis::WkbType::Unknown );

  // confirm that GDAL VSI configuration options are set
  QString noSign( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, u"YES"_s );
  QString region( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  //subLayers = ogrMetadata->querySublayers( u"/vsis3/sublayerstestbucket/test.shp|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-3|credential:AWS_S3_ENDPOINT=localhost"_s, Qgis::SublayerQueryFlag::FastScan );
  subLayers = ogrMetadata->querySublayers( u"/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES"_s, Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( subLayers.size(), 1 );
  QCOMPARE( subLayers.at( 0 ).name(), u"files"_s );
  QCOMPARE( subLayers.at( 0 ).uri(), u"/vsis3/cdn.proj.org/files.geojson|credential:AWS_NO_SIGN_REQUEST=YES"_s );
  QCOMPARE( subLayers.at( 0 ).providerKey(), u"ogr"_s );
  QCOMPARE( subLayers.at( 0 ).type(), Qgis::LayerType::Vector );
  QCOMPARE( subLayers.at( 0 ).wkbType(), Qgis::WkbType::Unknown );

  // cleanup
  VSIClearPathSpecificOptions( "/vsis3/cdn.proj.org" );
#endif
}


void TestQgsOgrProvider::testJSONFields_data()
{
  QTest::addColumn<QString>( "jsonData" );
  QTest::addColumn<int>( "expectedType" );
  QTest::addColumn<int>( "expectedSubType" );

  QTest::newRow( "array of map string fallback" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "array_of_map": [
          {
            "a": 1,
            "b": 2.0
          }
        ]
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QString )
                                                  << static_cast<int>( QMetaType::Type::UnknownType );

  QTest::newRow( "simple map" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "map": {
          "a": 1,
          "b": 2.0
        }
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QVariantMap )
                                << static_cast<int>( QMetaType::Type::QString );

  QTest::newRow( "complex map" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "map": {
          "a": 1,
          "b": [2.0, "c"]
        }
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QVariantMap )
                                 << static_cast<int>( QMetaType::Type::QString );

  QTest::newRow( "int" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "int": 1
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::Int )
                         << static_cast<int>( QMetaType::Type::UnknownType );

  QTest::newRow( "stringlist" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "string_list": [ "a", "b", "c" ]
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QStringList )
                                << static_cast<int>( QMetaType::Type::QString );

  QTest::newRow( "string" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "string": "a"
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QString )
                            << static_cast<int>( QMetaType::Type::UnknownType );

  QTest::newRow( "double" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "double": 1.0
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::Double )
                            << static_cast<int>( QMetaType::Type::UnknownType );

  QTest::newRow( "bool" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "bool": true
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::Bool )
                          << static_cast<int>( QMetaType::Type::UnknownType );

  QTest::newRow( "int list" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "int_list": [1, 2, 3]
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QVariantList )
                              << static_cast<int>( QMetaType::Type::Int );

  QTest::newRow( "real list" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "real_list": [1.0, 2.1, 3]
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QVariantList )
                               << static_cast<int>( QMetaType::Type::Double );


  QTest::newRow( "array mixed types string fallback" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "mixed_list": [1, 2.0, "a", true]
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QString )
                                                       << static_cast<int>( QMetaType::Type::UnknownType );

  QTest::newRow( "array mixed numeric types" ) << QStringLiteral( R"json(
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "mixed_numeric_list": [1, 2.3]
      }
    }
  ]
}
)json" ) << static_cast<int>( QMetaType::Type::QVariantList )
                                               << static_cast<int>( QMetaType::Type::Double );
}

void TestQgsOgrProvider::testJSONFields()
{
  QFETCH( QString, jsonData );
  QFETCH( int, expectedType );
  QFETCH( int, expectedSubType );

  QTemporaryDir dir;
  QString filePath = dir.path() + "/test.json";
  QFile file( filePath );
  if ( file.open( QIODevice::WriteOnly ) )
  {
    QTextStream textStream( &file );
    textStream << jsonData;
    file.close();
  }
  QgsVectorLayer layer { filePath, u"json"_s, "ogr"_L1 };
  QVERIFY( layer.isValid() );
  QgsFields fields = layer.fields();
  QCOMPARE( fields.count(), 1 );
  QgsField field = fields.at( 0 );
  QCOMPARE( static_cast<int>( field.type() ), expectedType );
  QCOMPARE( static_cast<int>( field.subType() ), expectedSubType );
}


QGSTEST_MAIN( TestQgsOgrProvider )
#include "testqgsogrprovider.moc"
