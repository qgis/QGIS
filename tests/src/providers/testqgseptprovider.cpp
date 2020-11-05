/***************************************************************************
     testqgseptprovider.cpp
     --------------------------------------
    Date                 : November 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgseptprovider.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the EPT provider
 */
class TestQgsEptProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void filters();
    void encodeUri();
    void decodeUri();
    void preferredUri();
    void uriIsBlocklisted();

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsEptProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>EPT Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsEptProvider::cleanupTestCase()
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

void TestQgsEptProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterPointCloud ), QStringLiteral( "Entwine Point Clouds (ept.json EPT.JSON)" ) );
  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterVector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "(ept.json EPT.JSON)" ) );
}

void TestQgsEptProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/ept.json" ) );
  QCOMPARE( metadata->encodeUri( parts ), QStringLiteral( "/home/point_clouds/ept.json" ) );
}

void TestQgsEptProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( QStringLiteral( "/home/point_clouds/ept.json" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/ept.json" ) );
}

void TestQgsEptProvider::preferredUri()
{
  // test that EPT is the preferred provider for ept.json uris
  QCOMPARE( QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/ept.json" ) ), QList< QgsProviderMetadata * >() << QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) ) );
  QCOMPARE( QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/EPT.JSON" ) ), QList< QgsProviderMetadata * >() << QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) ) );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/ept.json" ), QStringLiteral( "ept" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/ept.json" ), QStringLiteral( "ogr" ) ) );
}

void TestQgsEptProvider::uriIsBlocklisted()
{
  QVERIFY( !QgsProviderRegistry::instance()->uriIsBlocklisted( QStringLiteral( "/home/nyall/ept.json" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->uriIsBlocklisted( QStringLiteral( "/home/nyall/ept-build.json" ) ) );
}


QGSTEST_MAIN( TestQgsEptProvider )
#include "testqgseptprovider.moc"
