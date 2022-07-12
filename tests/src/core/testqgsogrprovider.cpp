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
#include <qgis.h>
#include <qgssettings.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsvectorlayer.h>
#include <qgsnetworkaccessmanager.h>

#include <QObject>
#include <QThread>

#include <cpl_conv.h>


/**
 * \ingroup UnitTests
 * This is a unit test for the ogr provider
 */
class TestQgsOgrProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void setupProxy();
    void decodeUri();
    void encodeUri();
    void testThread();
    void testCsvFeatureAddition();

  private:
    QString mTestDataDir;
    QString mReport;
  signals:

  public slots:
};



//runs before all tests
void TestQgsOgrProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>OGR Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsOgrProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsOgrProvider::setupProxy()
{

  QgsSettings settings;
  {
    settings.setValue( QStringLiteral( "proxy/proxyEnabled" ), true );
    settings.setValue( QStringLiteral( "proxy/proxyPort" ), QStringLiteral( "1234" ) );
    settings.setValue( QStringLiteral( "proxy/proxyHost" ), QStringLiteral( "myproxyhostname.com" ) );
    settings.setValue( QStringLiteral( "proxy/proxyUser" ), QStringLiteral( "username" ) );
    settings.setValue( QStringLiteral( "proxy/proxyPassword" ), QStringLiteral( "password" ) );
    settings.setValue( QStringLiteral( "proxy/proxyExcludedUrls" ), QStringLiteral( "http://www.myhost.com|http://www.myotherhost.com" ) );
    QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
    const QgsVectorLayer vl( mTestDataDir + '/' + QStringLiteral( "lines.shp" ), QStringLiteral( "proxy_test" ), QLatin1String( "ogr" ) );
    QVERIFY( vl.isValid() );
    const char *proxyConfig = CPLGetConfigOption( "GDAL_HTTP_PROXY", nullptr );
    QCOMPARE( proxyConfig, "myproxyhostname.com:1234" );
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
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QString( "/vsicurl/https://www.qgis.org/dataset.gpkg" ) );
  QCOMPARE( parts.value( QStringLiteral( "authcfg" ) ).toString(), QString( "1234567" ) );
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
}

class ReadVectorLayer : public QThread
{
    Q_OBJECT

  public :
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
  if ( !QgsTest::runFlakyTests() )
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


QGSTEST_MAIN( TestQgsOgrProvider )
#include "testqgsogrprovider.moc"
