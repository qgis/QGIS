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

QGSTEST_MAIN( TestQgsOgrProvider )
#include "testqgsogrprovider.moc"
