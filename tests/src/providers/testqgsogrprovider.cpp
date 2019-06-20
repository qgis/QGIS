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
#include <qgsgeopackagedataitems.h>
#include <qgsdataitem.h>

#include <QObject>

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
    void testThread();
    //! Test GPKG data items rename
    void testGpkgDataItemRename();

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
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
    QgsVectorLayer vl( mTestDataDir + '/' + QStringLiteral( "lines.shp" ), QStringLiteral( "proxy_test" ), QLatin1Literal( "ogr" ) );
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
    QgsVectorLayer vl( mTestDataDir + '/' + QStringLiteral( "lines.shp" ), QStringLiteral( "proxy_test" ), QLatin1Literal( "ogr" ) );
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
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "database_name" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "MYSQL:westholland,user=root,password=psv9570,port=3306,tables=bedrijven" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "westholland" ) );
  parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), QStringLiteral( "/path/to/a/geopackage.gpkg|layername=a_layer" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString( "a_layer" ) );
}


class ReadVectorLayer : public QThread
{

  public :
    ReadVectorLayer( const QString &filePath, QMutex &mutex, QWaitCondition &waitForVlCreation, QWaitCondition &waitForProcessEvents )
      : _filePath( filePath ), _mutex( mutex ), _waitForVlCreation( waitForVlCreation ), _waitForProcessEvents( waitForProcessEvents ) {}

    void run() override
    {

      QgsVectorLayer *vl2 = new QgsVectorLayer( _filePath, QStringLiteral( "thread_test" ), QLatin1Literal( "ogr" ) );

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

  QString filePath = mTestDataDir + '/' + QStringLiteral( "lines.shp" );
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

void TestQgsOgrProvider::testGpkgDataItemRename()
{
  QTemporaryFile f( QStringLiteral( "qgis-XXXXXX.gpkg" ) );
  f.open();
  f.close();
  QString fileName { f.fileName( ) };
  f.remove();
  QVERIFY( QFile::copy( QStringLiteral( "%1/provider/bug_21227-rename-styles.gpkg" ).arg( mTestDataDir ),  fileName ) );
  QgsGeoPackageVectorLayerItem item( nullptr,
                                     QStringLiteral( "Layer 1" ),
                                     QStringLiteral( "gpkg:/%1|layername=layer 1" )
                                     .arg( fileName ),
                                     QStringLiteral( "%1|layername=layer 1" ).arg( fileName ),
                                     QgsLayerItem::LayerType::TableLayer );
  item.rename( "layer 3" );
  // Check that the style is still available
  QgsVectorLayer metadataLayer( QStringLiteral( "/%1|layername=layer_styles" ).arg( fileName ) );
  QVERIFY( metadataLayer.isValid() );
  QgsFeature feature;
  QgsFeatureIterator it = metadataLayer.getFeatures( QgsFeatureRequest( QgsExpression( QStringLiteral( "\"f_table_name\" = 'layer 3'" ) ) ) );
  QVERIFY( it.nextFeature( feature ) );
  QVERIFY( feature.isValid() );
  QCOMPARE( feature.attribute( QStringLiteral( "styleName" ) ).toString(), QString( "style for layer 1" ) );
  it = metadataLayer.getFeatures( QgsFeatureRequest( QgsExpression( QStringLiteral( "\"f_table_name\" = 'layer 1' " ) ) ) );
  QVERIFY( !it.nextFeature( feature ) );
  it = metadataLayer.getFeatures( QgsFeatureRequest( QgsExpression( QStringLiteral( "\"f_table_name\" = 'layer 2' " ) ) ) );
  QVERIFY( it.nextFeature( feature ) );
  QVERIFY( feature.isValid() );
  QCOMPARE( feature.attribute( QStringLiteral( "styleName" ) ).toString(), QString( "style for layer 2" ) );
}


QGSTEST_MAIN( TestQgsOgrProvider )
#include "testqgsogrprovider.moc"
