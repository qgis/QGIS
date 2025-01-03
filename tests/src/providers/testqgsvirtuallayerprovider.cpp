/***************************************************************************
     testqgsvirtuallayerprovider.cpp
     ------------------------------------
    Date                 : October 2023
    Copyright            : (C) 2023 by Sandro Santilli
    Email                : strk at kbt dot net
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
#include <fstream>
#include <QVector>
#include <QTest>
#include <QStandardPaths>
#include <QQueue>

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
//#include "qgsproviderregistry.h"
#include "qgsvirtuallayerprovider.h"
//#include "qgsgeometry.h"
//#include "qgsfeedback.h"
//#include "qgsprovidermetadata.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the Virtual Layer provider
 */
class TestQgsVirtualLayerProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsVirtualLayerProvider()
      : QgsTest( QStringLiteral( "Virtual Layer Provider Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testConstructor();

  private:
    QString mTestDataDir;
};

//runs before all tests
void TestQgsVirtualLayerProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}

//runs after all tests
void TestQgsVirtualLayerProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVirtualLayerProvider::testConstructor()
{
  QgsDataProvider::ProviderOptions opts;
  QString uri;
  std::unique_ptr<QgsVectorDataProvider> prov( new QgsVirtualLayerProvider( uri, opts ) );

  const QgsRectangle rectNull;
  QCOMPARE( prov->sourceExtent(), rectNull );
}

QGSTEST_MAIN( TestQgsVirtualLayerProvider )
#include "testqgsvirtuallayerprovider.moc"
