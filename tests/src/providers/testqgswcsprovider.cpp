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

#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QApplication>

#include <qgsdatasourceuri.h>
#include <qgsrasterlayer.h>
#include <qgsrasterdataprovider.h>
#include <qgsproviderregistry.h>
#include <qgsapplication.h>

#define TINY_VALUE  std::numeric_limits<double>::epsilon() * 20

/** \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsWcsProvider: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void read();
  private:
    bool read( QString theIdentifier, QString theWcsUri, QString theFilePath, QString & theReport );
    // Log error in html
    void error( QString theMessage, QString &theReport );
    // compare values and add table row in html report, set ok to false if not equal
    QString compareHead();
    bool compare( double wcsVal, double gdalVal, double theTolerance );
    void compare( QString theParamName, int wcsVal, int gdalVal, QString &theReport, bool &theOk );
    void compare( QString theParamName, double wcsVal, double gdalVal, QString &theReport, bool &theOk, double theTolerance = 0 );
    void compareRow( QString theParamName, QString wcsVal, QString gdalVal, QString &theReport, bool theOk, QString theDifference = "", QString theTolerance = "" );
    double tolerance( double val, int places = 6 );
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
  mySettings = mySettings.replace( "\n", "<br />" );
  mReport += "<h1>WCS provider tests</h1>\n";
  mReport += "<p>" + mySettings + "</p>";

  mReport += "<style>";
  mReport += ".tab { border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid; }";
  mReport += ".cell { border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center}";
  mReport += ".ok { background: #00ff00; }";
  mReport += ".err { background: #ff0000; }";
  mReport += ".errmsg { color: #ff0000; }";
  mReport += "</style>";


  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QString( TEST_DATA_DIR ) + "/raster";
  qDebug() << "mTestDataDir = " << mTestDataDir;

  mUrl =  QString( TEST_SERVER_URL ) + "/wcs";
}

//runs after all tests
void TestQgsWcsProvider::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgiswcstest.html";
  QFile myFile( myReportFile );
  //if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsWcsProvider::read( )
{
  bool ok = true;
  QStringList versions;

  versions << "1.0" << "1.1";

  QStringList identifiers;

  // identifiers in mapfile have the same name as files without .tif extension
  identifiers << "band1_byte_noct_epsg4326";
  identifiers << "band1_int16_noct_epsg4326";
  identifiers << "band1_float32_noct_epsg4326";
  identifiers << "band3_byte_noct_epsg4326";
  identifiers << "band3_int16_noct_epsg4326";
  identifiers << "band3_float32_noct_epsg4326";

  // How to reasonably log multiple fails within this loop?
  foreach ( QString version, versions )
  {
    foreach ( QString identifier, identifiers )
    {
      QString filePath = mTestDataDir + "/" + identifier + ".tif";

      QgsDataSourceURI uri;
      uri.setParam( "url", mUrl );
      uri.setParam( "identifier", identifier );
      uri.setParam( "crs", "epsg:4326" );
      uri.setParam( "version", version );

      if ( !read( identifier, uri.encodedUri(), filePath, mReport ) )
      {
        ok = false;
      }
    }
  }
  QVERIFY2( ok, "Reading data failed. See report for details." );
}

bool TestQgsWcsProvider::read( QString theIdentifier, QString theWcsUri, QString theFilePath, QString & theReport )
{
  bool ok = true;

  theReport += QString( "<h2>Identifier (coverage): %1</h2>" ).arg( theIdentifier );

  QgsRasterDataProvider* wcsProvider = QgsRasterLayer::loadProvider( "wcs", theWcsUri );
  if ( !wcsProvider || !wcsProvider->isValid() )
  {
    error( QString( "Cannot load WCS provider with URI: %1" ).arg( QString( theWcsUri ) ), theReport );
    ok = false;
  }

  QgsRasterDataProvider* gdalProvider = QgsRasterLayer::loadProvider( "gdal", theFilePath );
  if ( !gdalProvider || !gdalProvider->isValid() )
  {
    error( QString( "Cannot load GDAL provider with URI: %1" ).arg( theFilePath ), theReport );
    ok = false;
  }

  if ( !ok ) return false;

  theReport += QString( "WCS URI: %1<br>" ).arg( theWcsUri.replace( "&", "&amp;" ) );
  theReport += QString( "GDAL URI: %1<br>" ).arg( theFilePath );

  theReport += "<br>";
  theReport += "<table class='tab'>";
  theReport += compareHead();

  compare( "Band count", wcsProvider->bandCount(), gdalProvider->bandCount(), theReport, ok );

  compare( "Width", wcsProvider->xSize(), gdalProvider->xSize(), theReport, ok );
  compare( "Height", wcsProvider->ySize(), gdalProvider->ySize(), theReport, ok );

  compareRow( "Extent", wcsProvider->extent().toString(), gdalProvider->extent().toString(), theReport, wcsProvider->extent() == gdalProvider->extent() );
  if ( wcsProvider->extent() != gdalProvider->extent() ) ok = false;

  if ( !ok ) return false;

  compare( "No data (NULL) value", wcsProvider->noDataValue(), gdalProvider->noDataValue(), theReport, ok );

  theReport += "</table>";


  bool allOk = true;
  for ( int band = 1; band <= gdalProvider->bandCount(); band++ )
  {
    bool bandOk = true;
    theReport += QString( "<h3>Band %1</h3>" ).arg( band );
    theReport += "<table class='tab'>";
    theReport += compareHead();

    // Data types may differ (?)
    bool typesOk = true;
    compare( "Source data type", wcsProvider->srcDataType( band ), gdalProvider->srcDataType( band ), theReport, typesOk );
    compare( "Data type", wcsProvider->dataType( band ), gdalProvider->dataType( band ), theReport, typesOk ) ;

    bool statsOk = true;
    QgsRasterBandStats wcsStats =  wcsProvider->bandStatistics( band );
    QgsRasterBandStats gdalStats =  gdalProvider->bandStatistics( band );

    // Min/max may 'slightly' differ, for big numbers however, the difference may
    // be quite big, for example for Float32 with max -3.332e+38, the difference is 1.47338e+24
    double tol = tolerance( gdalStats.minimumValue );
    compare( "Minimum value", wcsStats.minimumValue, gdalStats.minimumValue, theReport, statsOk, tol );
    tol = tolerance( gdalStats.maximumValue );
    compare( "Maximum value", wcsStats.maximumValue, gdalStats.maximumValue, theReport, statsOk, tol );

    // TODO: enable once fixed (WCS excludes nulls but GDAL does not)
    //compare( "Cells count", wcsStats.elementCount, gdalStats.elementCount, theReport, statsOk );

    tol = tolerance( gdalStats.mean );
    compare( "Mean", wcsStats.mean, gdalStats.mean, theReport, statsOk, tol );

    // stdDev usually differ significantly
    tol = tolerance( gdalStats.stdDev, 1 );
    compare( "Standard deviation", wcsStats.stdDev, gdalStats.stdDev, theReport, statsOk, tol );

    theReport += "</table>";
    theReport += "<br>";

    if ( !bandOk )
    {
      allOk = false;
      continue;
    }

    if ( !statsOk || !typesOk )
    {
      allOk = false;
      // create values table anyway so that values are available
    }

    theReport += "<table><tr>";
    theReport += "<td>Data comparison</td>";
    theReport += "<td class='cell ok' style='border: 1px solid'>correct&nbsp;value</td>";
    theReport += "<td></td>";
    theReport += "<td class='cell err' style='border: 1px solid'>wrong&nbsp;value<br>expected value</td></tr>";
    theReport += "</tr></table>";
    theReport += "<br>";

    int width = gdalProvider->xSize();
    int height = gdalProvider->ySize();
    int blockSize =  width * height * gdalProvider->typeSize( gdalProvider->dataType( band ) ) ;
    void * gdalData = malloc( blockSize );
    void * wcsData = malloc( blockSize );

    gdalProvider->readBlock( band, gdalProvider->extent(), width, height, gdalData );
    wcsProvider->readBlock( band, gdalProvider->extent(), width, height, wcsData );

    // compare data values
    QString htmlTable = "<table class='tab'>";
    for ( int row = 0; row < height; row ++ )
    {
      htmlTable += "<tr>";
      for ( int col = 0; col < width; col ++ )
      {
        bool cellOk = true;
        double wcsVal =  wcsProvider->readValue( wcsData,  wcsProvider->dataType( band ), row * width + col );
        double gdalVal =  gdalProvider->readValue( gdalData,  gdalProvider->dataType( band ), row * width + col );

        QString valStr;
        if ( compare( wcsVal, gdalVal, 0 ) )
        {
          valStr = QString( "%1" ).arg( wcsVal );
        }
        else
        {
          cellOk = false;
          allOk = false;
          valStr = QString( "%1<br>%2" ).arg( wcsVal ).arg( gdalVal );
        }
        htmlTable += QString( "<td class='cell %1'>%2</td>" ).arg( cellOk ? "ok" : "err" ).arg( valStr );
      }
      htmlTable += "</tr>";
    }
    htmlTable += "</table>";

    theReport += htmlTable;

    free( gdalData );
    free( wcsData );
  }
  delete wcsProvider;
  delete gdalProvider;
  return allOk;
}

void TestQgsWcsProvider::error( QString theMessage, QString &theReport )
{
  theReport += "<font class='errmsg'>Error: ";
  theReport += theMessage;
  theReport += "</font>";
}

double TestQgsWcsProvider::tolerance( double val, int places )
{
  // float precission is about 7 decimal digits, double about 16
  // default places = 6
  return 1. * qPow( 10, qRound( log10( qAbs( val ) ) - places ) );
}

QString TestQgsWcsProvider::compareHead()
{
  return "<tr><th class='cell'>Param name</th><th class='cell'>WCS (tested) value</th><th class='cell'>GDAL (expected) value</th><th class='cell'>Difference</th><th class='cell'>Tolerance</th></tr>";
}

void TestQgsWcsProvider::compare( QString theParamName, int wcsVal, int gdalVal, QString &theReport, bool &theOk )
{
  bool ok = wcsVal == gdalVal;
  compareRow( theParamName, QString::number( wcsVal ), QString::number( gdalVal ), theReport, ok, QString::number( wcsVal - gdalVal ) );
  if ( !ok ) theOk = false;
}

bool TestQgsWcsProvider::compare( double wcsVal, double gdalVal, double theTolerance )
{
  // values may be nan
  return ( qIsNaN( wcsVal ) && qIsNaN( gdalVal ) ) || ( qAbs( wcsVal - gdalVal ) <= theTolerance );
}

void TestQgsWcsProvider::compare( QString theParamName, double wcsVal, double gdalVal, QString &theReport, bool &theOk, double theTolerance )
{
  bool ok = compare( wcsVal, gdalVal, theTolerance );
  compareRow( theParamName, QString::number( wcsVal ), QString::number( gdalVal ), theReport, ok, QString::number( wcsVal - gdalVal ), QString::number( theTolerance ) );
  if ( !ok ) theOk = false;
}

void TestQgsWcsProvider::compareRow( QString theParamName, QString wcsVal, QString gdalVal, QString &theReport, bool theOk, QString theDifference, QString theTolerance )
{
  theReport += "<tr>";
  theReport += QString( "<td class='cell'>%1</td><td class='cell %2'>%3</td><td class='cell'>%4</td>" ).arg( theParamName ).arg( theOk ? "ok" : "err" ).arg( wcsVal ).arg( gdalVal );
  theReport += QString( "<td class='cell'>%1</td>" ).arg( theDifference );
  theReport += QString( "<td class='cell'>%1</td>" ).arg( theTolerance );
  theReport += "</tr>";
}

QTEST_MAIN( TestQgsWcsProvider )
#include "moc_testqgswcsprovider.cxx"
