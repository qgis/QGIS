/***************************************************************************
     testqgsmdalprovider.cpp
     --------------------------------------
    Date                 : Decemeber 2018
    Copyright            : (C) 2018 by Peter Petrik
    Email                : zilolv@gmail.com
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
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmeshdataprovider.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsMdalProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void load();
    void filters();
    void encodeDecodeUri();

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsMdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>MDAL Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsMdalProvider::cleanupTestCase()
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

void TestQgsMdalProvider::filters()
{
  const QString meshFilters = QgsProviderRegistry::instance()->fileMeshFilters();
  QVERIFY( meshFilters.contains( "*.2dm" ) );

  const QString datasetFilters = QgsProviderRegistry::instance()->fileMeshDatasetFilters();
  QVERIFY( datasetFilters.contains( "*.dat" ) );
}

void TestQgsMdalProvider::encodeDecodeUri()
{
  QgsProviderMetadata *mdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );

  // simple file uri
  QVariantMap parts = mdalMetadata->decodeUri( QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QString() );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "/home/data/test.nc" ) );

  // uri with driver and layer name
  parts = mdalMetadata->decodeUri( QStringLiteral( "netcdf:\"/home/data/test.nc\":layer3" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "netcdf" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QStringLiteral( "layer3" ) );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "netcdf:\"/home/data/test.nc\":layer3" ) );

  // uri with driver and layer name with space
  parts = mdalMetadata->decodeUri( QStringLiteral( "netcdf:\"/home/data/test.nc\":layer 3" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "netcdf" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QStringLiteral( "layer 3" ) );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "netcdf:\"/home/data/test.nc\":layer 3" ) );

  // uri with driver
  parts = mdalMetadata->decodeUri( QStringLiteral( "Ugrid:\"/home/data/test.nc\"" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "Ugrid" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "Ugrid:\"/home/data/test.nc\"" ) );

  parts = mdalMetadata->decodeUri( QStringLiteral( "ESRI_TIN:\"/home/data/tdenv9.adf\"" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/tdenv9.adf" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "ESRI_TIN" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "ESRI_TIN:\"/home/data/tdenv9.adf\"" ) );
}

void TestQgsMdalProvider::load()
{
  {
    const QString file = QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_flower.2dm";
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
                                  QStringLiteral( "mdal" ),
                                  file,
                                  QgsDataProvider::ProviderOptions()
                                );

    QgsMeshDataProvider *mp = dynamic_cast< QgsMeshDataProvider * >( provider );
    QVERIFY( mp );
    QVERIFY( mp->isValid() );
    delete provider;
  }
  {
    const QString file = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/goodluckwiththisfilename.2dm" );
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
                                  QStringLiteral( "mdal" ),
                                  file,
                                  QgsDataProvider::ProviderOptions()
                                );

    QgsMeshDataProvider *mp = dynamic_cast< QgsMeshDataProvider * >( provider );
    QVERIFY( mp );
    QVERIFY( !mp->isValid() );
    delete provider;
  }
}

QGSTEST_MAIN( TestQgsMdalProvider )
#include "testqgsmdalprovider.moc"
