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

    void filters();

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
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  QString meshFilters = QgsProviderRegistry::instance()->fileMeshFilters();
  QVERIFY( meshFilters.contains( "*.2dm" ) );

  QString datasetFilters = QgsProviderRegistry::instance()->fileMeshDatasetFilters();
  QVERIFY( datasetFilters.contains( "*.dat" ) );
}

QGSTEST_MAIN( TestQgsMdalProvider )
#include "testqgsmdalprovider.moc"
