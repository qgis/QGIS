/***************************************************************************
     testqgswcsprovider.cpp
     --------------------------------------
    Date                 : July 2012
    Copyright            : (C) 2012 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QTemporaryFile>

#include <qgsdatasourceuri.h>
#include <qgsrasterlayer.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasterchecker.h>
#include <qgsproviderregistry.h>
#include <qgsapplication.h>
#include "qgsprovidermetadata.h"

#define TINY_VALUE  std::numeric_limits<double>::epsilon() * 20

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsWcsProvider: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void providerUriUpdates();
    void read();
  private:
    bool read( const QString &identifier, const QString &wcsUri, const QString &filePath, QString &report );
    QString mTestDataDir;
    QString mReport;
    QString mUrl;
};

//runs before all tests
void TestQgsWcsProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( '\n', QLatin1String( "<br />" ) );
  mReport += QLatin1String( "<h1>WCS provider tests</h1>\n" );
  mReport += "<p>" + mySettings + "</p>";
  // Style is now inlined by QgsRasterChecker
#if 0
  mReport += "<style>";
  mReport += ".tab { border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid; }";
  mReport += ".cell { border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center}";
  mReport += ".ok { background: #00ff00; }";
  mReport += ".err { background: #ff0000; }";
  mReport += ".errmsg { color: #ff0000; }";
  mReport += "</style>";
#endif
  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + "/raster";
  qDebug() << "mTestDataDir = " << mTestDataDir;

  mUrl =  QStringLiteral( TEST_SERVER_URL ) + "/wcs";
}

//runs after all tests
void TestQgsWcsProvider::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsWcsProvider::read()
{
  bool ok = true;
  QStringList versions;

  // TODO: 1.1 test disabled for now because it was failing, UMN Mapserver is giving
  // 1x1 pixel response if GRIDORIGIN coordinate has a negative value, but it has to be
  // verified if the problem is really on Mapserver side
  //versions << "1.0" << "1.1";
  versions << QStringLiteral( "1.0" );

  QStringList identifiers;

  // identifiers in mapfile have the same name as files without .tif extension
  identifiers << QStringLiteral( "band1_byte_noct_epsg4326" );
  identifiers << QStringLiteral( "band1_int16_noct_epsg4326" );
  identifiers << QStringLiteral( "band1_float32_noct_epsg4326" );
  identifiers << QStringLiteral( "band3_byte_noct_epsg4326" );
  identifiers << QStringLiteral( "band3_int16_noct_epsg4326" );
  identifiers << QStringLiteral( "band3_float32_noct_epsg4326" );

  // How to reasonably log multiple fails within this loop?
  QTemporaryFile *tmpFile = new QTemporaryFile( QStringLiteral( "qgis-wcs-test-XXXXXX.tif" ) );
  tmpFile->open();
  const QString tmpFilePath = tmpFile->fileName();
  delete tmpFile; // removes the file
  for ( const QString &version : versions )
  {
    for ( const QString &identifier : identifiers )
    {
      // copy to temporary to avoid creation/changes/use of GDAL .aux.xml files
      const QString testFilePath = mTestDataDir + '/' + identifier + ".tif";
      qDebug() << "copy " <<  testFilePath << " to " << tmpFilePath;
      if ( !QFile::copy( testFilePath, tmpFilePath ) )
      {
        mReport += QStringLiteral( "Cannot copy %1 to %2" ).arg( testFilePath, tmpFilePath );
        ok = false;
        continue;
      }

      QgsDataSourceUri uri;
      uri.setParam( QStringLiteral( "url" ), mUrl );
      uri.setParam( QStringLiteral( "identifier" ), identifier );
      uri.setParam( QStringLiteral( "crs" ), QStringLiteral( "epsg:4326" ) );
      uri.setParam( QStringLiteral( "version" ), version );
      uri.setParam( QStringLiteral( "cache" ), QStringLiteral( "AlwaysNetwork" ) );

      if ( !read( identifier, uri.encodedUri(), tmpFilePath, mReport ) )
      {
        ok = false;
      }
      QFile::remove( tmpFilePath );
    }
  }
  QVERIFY2( ok, "Reading data failed. See report for details." );
}

bool TestQgsWcsProvider::read( const QString &identifier, const QString &wcsUri, const QString &filePath, QString &report )
{
  report += QStringLiteral( "<h2>Identifier (coverage): %1</h2>" ).arg( identifier );

  QgsRasterChecker checker;
  const bool ok = checker.runTest( QStringLiteral( "wcs" ), wcsUri, QStringLiteral( "gdal" ), filePath );

  report += checker.report();
  return ok;
}

void TestQgsWcsProvider::providerUriUpdates()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( "wcs" );
  QString uriString = QStringLiteral( "crs=EPSG:4326&dpiMode=7&"
                                      "layers=testlayer&styles&"
                                      "url=http://localhost:8380/mapserv&"
                                      "testParam=true" );

  QVariantMap parts = metadata->decodeUri( uriString );
  QVariantMap expectedParts { { QString( "crs" ), QVariant( "EPSG:4326" ) },  { QString( "dpiMode" ), QVariant( "7" ) },
    { QString( "testParam" ), QVariant( "true" ) },  { QString( "layers" ), QVariant( "testlayer" ) },
    { QString( "styles" ), QString() },  { QString( "url" ), QVariant( "http://localhost:8380/mapserv" ) } };
  QCOMPARE( parts, expectedParts );

  parts["testParam"] = QVariant( "false" );

  QCOMPARE( parts["testParam"], QVariant( "false" ) );

  QString updatedUri = metadata->encodeUri( parts );
  QString expectedUri = QStringLiteral( "crs=EPSG:4326&dpiMode=7&"
                                        "layers=testlayer&styles&"
                                        "testParam=false&"
                                        "url=http://localhost:8380/mapserv" );
  QCOMPARE( updatedUri, expectedUri );

}


QGSTEST_MAIN( TestQgsWcsProvider )
#include "testqgswcsprovider.moc"
