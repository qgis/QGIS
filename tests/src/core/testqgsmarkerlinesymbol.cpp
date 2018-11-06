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
#include "qgstest.h"
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
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgspallabeling.h"
#include "qgsfontutils.h"
#include "qgslinesymbollayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsproperty.h"

//qgis unit test includes
#include <qgsrenderchecker.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the Marker Line symbol
 */
class TestQgsMarkerLineSymbol : public QObject
{
    Q_OBJECT
  public:
    TestQgsMarkerLineSymbol()
    {
      mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
    }

    ~TestQgsMarkerLineSymbol() override;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void lineOffset();
    void pointNumInterval();
    void pointNumVertex();
    void ringFilter();

  private:
    bool render( const QString &fileName );

    QString mTestDataDir;
    QgsVectorLayer *mLinesLayer = nullptr;
    QgsVectorLayer *mPolygonsLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
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
                                    myLinesFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  mapLayers << mLinesLayer;

  // Register all layers with the registry
  QgsProject::instance()->addMapLayers( mapLayers );

  // This is needed to correctly set rotation center,
  // the actual size doesn't matter as QgsRenderChecker will
  // re-set it to the size of the expected image
  mMapSettings->setOutputSize( QSize( 256, 256 ) );

  mReport += QLatin1String( "<h1>Line Marker Symbol Tests</h1>\n" );

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

TestQgsMarkerLineSymbol::~TestQgsMarkerLineSymbol() = default;

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
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  // Negative offset on marker line
  // See https://issues.qgis.org/issues/13811

  QString qml = mTestDataDir + "marker_line_offset.qml";
  bool success = false;
  mLinesLayer->loadNamedStyle( qml, success );

  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -140, -140, 140, 140 ) );
  QVERIFY( render( "line_offset" ) );

  // TODO: -0.0 offset, see
  // https://issues.qgis.org/issues/13811#note-1
}

void TestQgsMarkerLineSymbol::pointNumInterval()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  QgsMarkerLineSymbolLayer *ml = new QgsMarkerLineSymbolLayer();
  ml->setPlacement( QgsMarkerLineSymbolLayer::Interval );
  ml->setInterval( 4 );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, ml );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( lineSymbol );

  // make sub-symbol
  QgsStringMap props;
  props[QStringLiteral( "color" )] = QStringLiteral( "255,0,0" );
  props[QStringLiteral( "size" )] = QStringLiteral( "2" );
  props[QStringLiteral( "outline_style" )] = QStringLiteral( "no" );
  QgsSimpleMarkerSymbolLayer *marker = static_cast< QgsSimpleMarkerSymbolLayer * >( QgsSimpleMarkerSymbolLayer::create( props ) );

  marker->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "@geometry_point_num * 2" ) ) );

  QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
  subSymbol->changeSymbolLayer( 0, marker );
  ml->setSubSymbol( subSymbol );

  mLinesLayer->setRenderer( r );

  mMapSettings->setExtent( QgsRectangle( -140, -140, 140, 140 ) );
  QVERIFY( render( "point_num_interval" ) );
}

void TestQgsMarkerLineSymbol::pointNumVertex()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  QgsMarkerLineSymbolLayer *ml = new QgsMarkerLineSymbolLayer();
  ml->setPlacement( QgsMarkerLineSymbolLayer::Vertex );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, ml );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( lineSymbol );

  // make sub-symbol
  QgsStringMap props;
  props[QStringLiteral( "color" )] = QStringLiteral( "255,0,0" );
  props[QStringLiteral( "size" )] = QStringLiteral( "2" );
  props[QStringLiteral( "outline_style" )] = QStringLiteral( "no" );
  QgsSimpleMarkerSymbolLayer *marker = static_cast< QgsSimpleMarkerSymbolLayer * >( QgsSimpleMarkerSymbolLayer::create( props ) );

  marker->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "@geometry_point_num * 2" ) ) );

  QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
  subSymbol->changeSymbolLayer( 0, marker );
  ml->setSubSymbol( subSymbol );

  mLinesLayer->setRenderer( r );

  mMapSettings->setExtent( QgsRectangle( -140, -140, 140, 140 ) );
  QVERIFY( render( "point_num_vertex" ) );
}

void TestQgsMarkerLineSymbol::ringFilter()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  QgsMarkerLineSymbolLayer *ml = new QgsMarkerLineSymbolLayer();
  ml->setPlacement( QgsMarkerLineSymbolLayer::Vertex );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, ml );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( lineSymbol );

  // make sub-symbol
  QgsStringMap props;
  props[QStringLiteral( "color" )] = QStringLiteral( "255,0,0" );
  props[QStringLiteral( "size" )] = QStringLiteral( "2" );
  props[QStringLiteral( "outline_style" )] = QStringLiteral( "no" );
  QgsSimpleMarkerSymbolLayer *marker = static_cast< QgsSimpleMarkerSymbolLayer * >( QgsSimpleMarkerSymbolLayer::create( props ) );

  marker->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "@geometry_point_num * 2" ) ) );

  QgsMarkerSymbol *subSymbol = new QgsMarkerSymbol();
  subSymbol->changeSymbolLayer( 0, marker );
  ml->setSubSymbol( subSymbol );

  mLinesLayer->setRenderer( r );

  mMapSettings->setExtent( QgsRectangle( -140, -140, 140, 140 ) );
  QVERIFY( render( "point_num_vertex" ) );
}

bool TestQgsMarkerLineSymbol::render( const QString &testType )
{
  mReport += "<h2>" + testType + "</h2>\n";
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "symbol_markerline" ) );
  checker.setControlName( "expected_" + testType );
  checker.setMapSettings( *mMapSettings );
  bool result = checker.runTest( testType );
  mReport += "\n\n\n" + checker.report();
  return result;
}

QGSTEST_MAIN( TestQgsMarkerLineSymbol )
#include "testqgsmarkerlinesymbol.moc"
