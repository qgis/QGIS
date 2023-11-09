/***************************************************************************
     TestQgsMapDevicePixelRatio.cpp
     --------------------------------------
    Date                 : October 2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QIODevice>

#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsfontutils.h"
#include "qgsmapsettings.h"

//qgis unit test includes
#include <qgsrenderchecker.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the map rotation feature
 */
class TestQgsMapDevicePixelRatio : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsMapDevicePixelRatio()
      : QgsTest( QStringLiteral( "Map Device Pixel Ratio Tests" ) )
    {
      mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
    }

    ~TestQgsMapDevicePixelRatio() override;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void pointsLayer();

  private:
    bool render( const QString &fileName, float dpr );

    QString mTestDataDir;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
};

//runs before all tests
void TestQgsMapDevicePixelRatio::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  //create a point layer that will be used in all tests...
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

TestQgsMapDevicePixelRatio::~TestQgsMapDevicePixelRatio() = default;

//runs after all tests
void TestQgsMapDevicePixelRatio::cleanupTestCase()
{
  delete mMapSettings;
  delete mPointsLayer;
  QgsApplication::exitQgis();
}

void TestQgsMapDevicePixelRatio::pointsLayer()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mPointsLayer );

  const QString qml = mTestDataDir + "points.qml";
  bool success = false;
  mPointsLayer->loadNamedStyle( qml, success );
  QVERIFY( success );
  QVERIFY( render( "dpr-normal", 1 ) );
  QVERIFY( render( "dpr-2", 2 ) );
  QVERIFY( render( "dpr-float", 2.5 ) );
}


bool TestQgsMapDevicePixelRatio::render( const QString &testType, float dpr )
{
  mMapSettings->setOutputSize( QSize( 256, 256 ) );
  mMapSettings->setOutputDpi( 96 );
  mMapSettings->setDevicePixelRatio( dpr );
  mMapSettings->setExtent( QgsRectangle( -105.5, 37, -97.5, 45 ) );
  qDebug() << "scale" << QString::number( mMapSettings->scale(), 'f' );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "mapdevicepixelratio" ) );
  checker.setControlName( "expected_" + testType );
  checker.setMapSettings( *mMapSettings );
  const bool result = checker.runTest( testType );
  mReport += checker.report();
  return result;
}

QGSTEST_MAIN( TestQgsMapDevicePixelRatio )
#include "testqgsmapdevicepixelratio.moc"
