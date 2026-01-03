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
#include "qgsapplication.h"
#include "qgstest.h"
#include "qgsziputils.h"

#include <QDirIterator>
#include <QObject>
#include <QString>
#include <QStringList>

class TestQgsZipUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void unzipWithSubdirs();
    void unzipWithSubdirs2();
    void specialChars();
    void testZip();

    void extractFileFromZip_nonExistentZip();
    void extractFileFromZip_emptyZipPath();
    void extractFileFromZip_emptyFilePathInZip();
    void extractFileFromZip_fileNotInZip();
    void extractFileFromZip_rootSuccess();

  private:
    void genericTest( QString zipName, int expectedEntries, bool includeFolders, const QStringList &testFileNames );
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
  QStringList testFileNames;
  testFileNames << "/folder/folder2/landsat_b2.tif" << "/folder/points.geojson" << "/points.qml";
  genericTest( QString( "testzip" ), 9, true, testFileNames );
}

/**
 * Test unzips zip file with a following structure. Note that subfolder is not included in the structure as
 * usually expected. Zip file has been made with the python zipstream lib (https://github.com/allanlei/python-zipstream).
 *
 * output of zipinfo diff_structured.zip:
 * Archive:  diff_structured.zip
 * Zip file size: 452 bytes, number of entries: 3
 * ?rw-------  2.0 unx       16 bl defN 18-Dec-18 13:27 subfolder/second_level.txt
 * ?rw-------  2.0 unx        5 bl defN 18-Dec-18 13:27 subfolder/3.txt
 * ?rw-------  2.0 unx       15 bl defN 18-Dec-18 13:27 first_level.txt
 *
*/
void TestQgsZipUtils::unzipWithSubdirs2()
{
  genericTest( QString( "diff_structured" ), 3, false, QStringList() << "/subfolder/3.txt" );
}

void TestQgsZipUtils::specialChars()
{
  genericTest( "Chars èêæýì", 1, false, QStringList() << "/èêæýì.txt" );
}

void TestQgsZipUtils::testZip()
{
  const QString txtFile = QString( TEST_DATA_DIR ) + "/zip/aæýì.txt";

  const QString zipDirPath = QDir::tempPath() + "/test_special_chars æì";
  QStringList files;

  // Create a root folder otherwise nothing is unzipped
  QDir dir( zipDirPath );
  if ( dir.exists( zipDirPath ) )
  {
    dir.remove( zipDirPath );
    QFile::remove( zipDirPath + "/special_zip æì.zip" );
    QFile::remove( zipDirPath + "/aæýì.txt" );
  }
  else
  {
    dir.mkdir( zipDirPath );
  }

  QVERIFY( QgsZipUtils::zip( zipDirPath + "/special_zip æì.zip", QStringList() << txtFile ) );
  QVERIFY( QgsZipUtils::unzip( zipDirPath + "/special_zip æì.zip", zipDirPath, files ) );
  QCOMPARE( files.count(), 1 );
  QCOMPARE( files.at( 0 ), QString( zipDirPath + "/aæýì.txt" ) );
  QVERIFY( QFile::exists( zipDirPath + "/aæýì.txt" ) );
}

/**
 * \brief TestQgsZipUtils::genericTest
 * \param zipName File to unzip
 * \param expectedEntries number of expected entries in given file
 * \param includeFolders Tag if a folder should be count as an entry
 * \param testFileNames List of file names to check if files were unzipped successfully
 */
void TestQgsZipUtils::genericTest( QString zipName, int expectedEntries, bool includeFolders, const QStringList &testFileNames )
{
  const QFile zipFile( QString( TEST_DATA_DIR ) + u"/zip/%1.zip"_s.arg( zipName ) );
  QVERIFY( zipFile.exists() );

  const QFileInfo fileInfo( zipFile );
  const QString unzipDirPath = QDir::tempPath() + '/' + zipName;
  QStringList files;

  // Create a root folder otherwise nothing is unzipped
  QDir dir( unzipDirPath );
  if ( !dir.exists( unzipDirPath ) )
  {
    dir.mkdir( unzipDirPath );
  }

  QgsZipUtils::unzip( fileInfo.absoluteFilePath(), unzipDirPath, files );
  // Test number of unzipped files
  QCOMPARE( files.count(), expectedEntries );

  if ( includeFolders )
  {
    dir.setFilter( QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs );
  }
  else
  {
    dir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
  }
  // Get list of entries from the root folder
  QDirIterator it( dir, QDirIterator::Subdirectories );
  QStringList filesFromResultDir;
  while ( it.hasNext() )
  {
    it.next();
    if ( !it.fileInfo().isDir() )
    {
      filesFromResultDir << it.filePath();
    }
  }

  // Test if ziplib matches number of files in the root folder
  QCOMPARE( files.count(), filesFromResultDir.count() );

  // Test if specific files are included in the root folder
  for ( const QString &fileName : testFileNames )
  {
    QVERIFY( filesFromResultDir.contains( unzipDirPath + fileName ) );
  }

  // Delete unzipped data
  const bool testDataRemoved = dir.removeRecursively();
  QVERIFY( testDataRemoved );
}

void TestQgsZipUtils::extractFileFromZip_nonExistentZip()
{
  QByteArray data;
  QVERIFY( !QgsZipUtils::extractFileFromZip( "/path/to/nothing", "none.txt", data ) );
  QVERIFY( data.isEmpty() );
}

void TestQgsZipUtils::extractFileFromZip_emptyZipPath()
{
  QByteArray content;
  QVERIFY( !QgsZipUtils::extractFileFromZip( QString(), "none.txt", content ) );
  QVERIFY( content.isEmpty() );
}

void TestQgsZipUtils::extractFileFromZip_emptyFilePathInZip()
{
  const QString zipPath = QString( TEST_DATA_DIR ) + "/zip/testzip.zip";
  QVERIFY( QFile::exists( zipPath ) );
  QByteArray content;
  QVERIFY( !QgsZipUtils::extractFileFromZip( zipPath, QString(), content ) );
  QVERIFY( content.isEmpty() );
}

void TestQgsZipUtils::extractFileFromZip_fileNotInZip()
{
  const QString zipPath = QString( TEST_DATA_DIR ) + "/zip/testzip.zip";
  QVERIFY( QFile::exists( zipPath ) );
  QByteArray content;
  QVERIFY( !QgsZipUtils::extractFileFromZip( zipPath, "move_along.txt", content ) );
  QVERIFY( content.isEmpty() );
}

void TestQgsZipUtils::extractFileFromZip_rootSuccess()
{
  const QString zipPath = QString( TEST_DATA_DIR ) + "/zip/testzip.zip";
  QVERIFY( QFile::exists( zipPath ) );

  const QByteArray expectedContent = R"(GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]])";

  QByteArray extractedContent;
  QVERIFY( QgsZipUtils::extractFileFromZip( zipPath, "points.prj", extractedContent ) );
  QCOMPARE( extractedContent, expectedContent );
}

QGSTEST_MAIN( TestQgsZipUtils )
#include "testqgsziputils.moc"
