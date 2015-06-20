/***************************************************************************
     testqgsrasterfilewriter.cpp
     --------------------------------------
    Date                 : 5 Sep 2012
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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <iostream>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QTime>
#include <QDesktopServices>

#include "cpl_conv.h"

//qgis includes...
#include <qgsrasterchecker.h>
#include <qgsrasterlayer.h>
#include <qgsrasterfilewriter.h>
#include <qgsrasternuller.h>
#include <qgsapplication.h>

/** \ingroup UnitTests
 * This is a unit test for the QgsRasterFileWriter class.
 */
class TestQgsRasterFileWriter: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void writeTest();
  private:
    bool writeTest( QString rasterName );
    void log( QString msg );
    void logError( QString msg );
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsRasterFileWriter::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // disable any PAM stuff to make sure stats are consistent
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n", "<br />" );
  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QString( TEST_DATA_DIR ) + QDir::separator(); //defined in CmakeLists.txt
  mReport += "<h1>Raster File Writer Tests</h1>\n";
  mReport += "<p>" + mySettings + "</p>";
}
//runs after all tests
void TestQgsRasterFileWriter::cleanupTestCase()
{
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsRasterFileWriter::writeTest()
{
  QDir dir( mTestDataDir + "/raster" );

  QStringList filters;
  filters << "*.tif";
  QStringList rasterNames = dir.entryList( filters, QDir::Files );
  bool allOK = true;
  foreach ( QString rasterName, rasterNames )
  {
    bool ok = writeTest( "raster/" + rasterName );
    if ( !ok ) allOK = false;
  }

  QVERIFY( allOK );
}

bool TestQgsRasterFileWriter::writeTest( QString theRasterName )
{
  mReport += "<h2>" + theRasterName + "</h2>\n";

  QString myFileName = mTestDataDir + "/" + theRasterName;
  qDebug() << myFileName;
  QFileInfo myRasterFileInfo( myFileName );

  QgsRasterLayer * mpRasterLayer =  new QgsRasterLayer( myRasterFileInfo.filePath(),
      myRasterFileInfo.completeBaseName() );
  qDebug() << theRasterName <<  " metadata: " << mpRasterLayer->dataProvider()->metadata();

  if ( !mpRasterLayer->isValid() ) return false;

  // Open provider only (avoid layer)?
  QgsRasterDataProvider * provider = mpRasterLayer->dataProvider();

  // I don't see any method to get only a name without opening file
  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is no avialable until open
  QString tmpName =  tmpFile.fileName();
  tmpFile.close();
  // do not remove when class is destroyd so that we can read the file and see difference
  tmpFile.setAutoRemove( false );
  qDebug() << "temporary output file: " << tmpName;
  mReport += "temporary output file: " + tmpName + "<br>";

  QgsRasterFileWriter fileWriter( tmpName );
  QgsRasterPipe* pipe = new QgsRasterPipe();
  if ( !pipe->set( provider->clone() ) )
  {
    logError( "Cannot set pipe provider" );
    delete pipe;
    return false;
  }
  qDebug() << "provider set";

  // Nuller currently is not really used
  QgsRasterNuller *nuller = new QgsRasterNuller();
  for ( int band = 1; band <= provider->bandCount(); band++ )
  {
    //nuller->setNoData( ... );
  }
  if ( !pipe->insert( 1, nuller ) )
  {
    logError( "Cannot set pipe nuller" );
    delete pipe;
    return false;
  }
  qDebug() << "nuller set";

  // Reprojection not really done
  QgsRasterProjector *projector = new QgsRasterProjector;
  projector->setCRS( provider->crs(), provider->crs() );
  if ( !pipe->insert( 2, projector ) )
  {
    logError( "Cannot set pipe projector" );
    delete pipe;
    return false;
  }
  qDebug() << "projector set";

  fileWriter.writeRaster( pipe, provider->xSize(), provider->ySize(), provider->extent(), provider->crs() );

  delete pipe;

  QgsRasterChecker checker;
  bool ok = checker.runTest( "gdal", tmpName, "gdal", myRasterFileInfo.filePath() );
  mReport += checker.report();

  // All OK, we can delete the file
  tmpFile.setAutoRemove( ok );

  return ok;
}

void TestQgsRasterFileWriter::log( QString msg )
{
  mReport += msg + "<br>";
}

void TestQgsRasterFileWriter::logError( QString msg )
{
  mReport += "Error:<font color='red'>" + msg + "</font><br>";
  qDebug() << msg;
}

QTEST_MAIN( TestQgsRasterFileWriter )
#include "testqgsrasterfilewriter.moc"
