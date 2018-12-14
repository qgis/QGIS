/***************************************************************************
     testqgsziputils.cpp
     --------------------
    begin                : December 2018
    copyright            : (C) 2018 Viktor Sklencar
    email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDirIterator>

#include "qgsziputils.h"
#include "qgsapplication.h"

class TestQgsZipUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void unzipWithSubdirs();

  private:
};

void TestQgsZipUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsZipUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsZipUtils::init()
{

}

void TestQgsZipUtils::cleanup()
{

}

void TestQgsZipUtils::unzipWithSubdirs()
{

  QFile zipFile( QString( TEST_DATA_DIR ) + "/zip/testzip.zip" );
  QVERIFY( zipFile.exists() );

  QFileInfo fileInfo( zipFile );
  QString unzipDirPath = QDir::tempPath() + "testzip";
  QStringList files;

  // Create a root folder otherwise nothing is unzipped
  QDir dir( unzipDirPath );
  if ( !dir.exists( unzipDirPath ) )
  {
    dir.mkdir( unzipDirPath );
  }

  QgsZipUtils::unzip( fileInfo.absoluteFilePath(), unzipDirPath, files );
  // Test number of unzipped files included folders
  QVERIFY( files.count() == 11 );

  // Get list of files from the root folder
  dir.setFilter( QDir::Dirs | QDir::Files |  QDir::NoDotAndDotDot );
  QDirIterator it( dir, QDirIterator::Subdirectories );
  QStringList filesFromResultDir;
  while ( it.hasNext() )
    filesFromResultDir << it.next();

  // Test if ziplib matches number of files in the root folder
  QVERIFY( files.count() == filesFromResultDir.count() );

  // Test if random files have been unzipped into the root folder
  QVERIFY( filesFromResultDir.contains( unzipDirPath + "/folder/folder2/landsat_b2.tif" ) );
  QVERIFY( filesFromResultDir.contains( unzipDirPath + "/folder/points.geojson" ) );
  QVERIFY( filesFromResultDir.contains( unzipDirPath + "/points.qml" ) );

  // Delete unzipped data
  bool testDataRemoved = dir.removeRecursively();
  QVERIFY( testDataRemoved );
}

QGSTEST_MAIN( TestQgsZipUtils )
#include "testqgsziputils.moc"
