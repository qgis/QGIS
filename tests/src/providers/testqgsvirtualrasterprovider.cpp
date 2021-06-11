/***************************************************************************
    testqgsvirtualrasterprovider.cpp
    --------------------------------------
   Date                 : June 2021
   Copyright            : (C) 2021 by Francesco Bursi
   Email                : francesco.bursi@hotmail.it
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
#include <QTemporaryDir>

//qgis includes...
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsrasterdataprovider.h>
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include <qgsrectangle.h>

/**
* \ingroup UnitTests
* This is a unit test for the virtualraster provider
*/


class TestQgsVirtualRasterProvider : public QObject
{
   Q_OBJECT

 private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void validLayer();

private:
    QString mTestDataDir;
    QString mReport;

};

//runs before all tests
void TestQgsVirtualRasterProvider::initTestCase()
{
    // init QGIS's paths - true means that all path will be inited from prefix
    QgsApplication::init();
    QgsApplication::initQgis();

    mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
    mReport = QStringLiteral( "<h1>Virtual Raster Provider Tests</h1>\n" );
}

void TestQgsVirtualRasterProvider::validLayer()
{
  QgsRasterLayer::LayerOptions options;

  std::unique_ptr< QgsRasterLayer > layer = std::make_unique< QgsRasterLayer >(
        mTestDataDir + QStringLiteral( "raster/dem.tif" ),
        QStringLiteral( "layer" ),
        QStringLiteral( "virtualraster" ),
        options
      );
  QVERIFY( layer->isValid() );

}

//runs after all tests
void TestQgsVirtualRasterProvider::cleanupTestCase()
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

QGSTEST_MAIN( TestQgsVirtualRasterProvider )
#include "testqgsvirtualrasterprovider.moc"
