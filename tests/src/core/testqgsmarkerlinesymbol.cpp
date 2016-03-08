/***************************************************************************
     testqgsmarkerlinesymbol.cpp
     --------------------------------------
    Date                 : Nov 12  2015
    Copyright            : (C) 2015 by Sandro Santilli
    Email                : strk@keybit.net
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
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsmaplayerregistry.h"
#include "qgsapplication.h"
#include "qgsmaprenderer.h"
#include "qgspallabeling.h"
#include "qgsfontutils.h"

//qgis unit test includes
#include <qgsrenderchecker.h>

/** \ingroup UnitTests
 * This is a unit test for the Marker Line symbol
 */
class TestQgsMarkerLineSymbol : public QObject
{
    Q_OBJECT
  public:
    TestQgsMarkerLineSymbol()
        : mLinesLayer( 0 )
        , mMapSettings( 0 )
    {
      mTestDataDir = QString( TEST_DATA_DIR ) + '/';
    }

    ~TestQgsMarkerLineSymbol();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void lineOffset();

  private:
    bool render( const QString& theFileName );

    QString mTestDataDir;
    QgsVectorLayer* mLinesLayer;
    QgsMapSettings *mMapSettings;
    QString mReport;
};

//runs before all tests
void TestQgsMarkerLineSymbol::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  QList<QgsMapLayer *> mapLayers;

  //create a line layer that will be used in all tests...
  QString myLinesFileName = mTestDataDir + "lines_cardinals.shp";
  QFileInfo myLinesFileInfo( myLinesFileName );
  mLinesLayer = new QgsVectorLayer( myLinesFileInfo.filePath(),
                                    myLinesFileInfo.completeBaseName(), "ogr" );
  mapLayers << mLinesLayer;

  // Register all layers with the registry
  QgsMapLayerRegistry::instance()->addMapLayers( mapLayers );

  // This is needed to correctly set rotation center,
  // the actual size doesn't matter as QgsRenderChecker will
  // re-set it to the size of the expected image
  mMapSettings->setOutputSize( QSize( 256, 256 ) );

  mReport += "<h1>Line Marker Symbol Tests</h1>\n";

  QgsFontUtils::loadStandardTestFonts( QStringList() << "Bold" );
}

TestQgsMarkerLineSymbol::~TestQgsMarkerLineSymbol()
{

}

//runs after all tests
void TestQgsMarkerLineSymbol::cleanupTestCase()
{
  delete mMapSettings;
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

void TestQgsMarkerLineSymbol::lineOffset()
{
  mMapSettings->setLayers( QStringList() << mLinesLayer->id() );

  // Negative offset on marker line
  // See http://hub.qgis.org/issues/13811

  QString qml = mTestDataDir + "marker_line_offset.qml";
  bool success = false;
  mLinesLayer->loadNamedStyle( qml, success );

  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -140, -140, 140, 140 ) );
  QVERIFY( render( "line_offset" ) );

  // TODO: -0.0 offset, see
  // http://hub.qgis.org/issues/13811#note-1
}

bool TestQgsMarkerLineSymbol::render( const QString& theTestType )
{
  mReport += "<h2>" + theTestType + "</h2>\n";
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( "symbol_markerline" );
  checker.setControlName( "expected_" + theTestType );
  checker.setMapSettings( *mMapSettings );
  bool result = checker.runTest( theTestType );
  mReport += "\n\n\n" + checker.report();
  return result;
}

QTEST_MAIN( TestQgsMarkerLineSymbol )
#include "testqgsmarkerlinesymbol.moc"
