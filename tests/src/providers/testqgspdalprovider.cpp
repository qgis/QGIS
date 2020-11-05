/***************************************************************************
     testqgspdalprovider.cpp
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
 * This is a unit test for the PDAL provider
 */
class TestQgsPdalProvider : public QObject
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

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsPdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>PDAL Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsPdalProvider::cleanupTestCase()
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

void TestQgsPdalProvider::filters()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterPointCloud ), QStringLiteral( "PDAL Point Clouds (*.laz *.las)" ) );
  QCOMPARE( metadata->filters( QgsProviderMetadata::FilterType::FilterVector ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters();
  QVERIFY( registryPointCloudFilters.contains( "(*.laz *.las)" ) );
}

void TestQgsPdalProvider::encodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( metadata->encodeUri( parts ), QStringLiteral( "/home/point_clouds/cloud.las" ) );
}

void TestQgsPdalProvider::decodeUri()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  QVERIFY( metadata );

  const QVariantMap parts = metadata->decodeUri( QStringLiteral( "/home/point_clouds/cloud.las" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/point_clouds/cloud.las" ) );
}

void TestQgsPdalProvider::preferredUri()
{
  // test that EPT is the preferred provider for las/laz uris
  QCOMPARE( QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/cloud.las" ) ), QList< QgsProviderMetadata * >() << QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) ) );
  QCOMPARE( QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/CLOUD.LAS" ) ), QList< QgsProviderMetadata * >() << QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) ) );
  QCOMPARE( QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/cloud.laz" ) ), QList< QgsProviderMetadata * >() << QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) ) );
  QCOMPARE( QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "/home/test/CLOUD.LAZ" ) ), QList< QgsProviderMetadata * >() << QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) ) );

  QVERIFY( !QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/cloud.las" ), QStringLiteral( "pdal" ) ) );
  QVERIFY( QgsProviderRegistry::instance()->shouldDeferUriForOtherProviders( QStringLiteral( "/home/test/cloud.las" ), QStringLiteral( "ept" ) ) );
}


QGSTEST_MAIN( TestQgsPdalProvider )
#include "testqgspdalprovider.moc"
