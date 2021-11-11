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
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
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
    void collectPoints_data();
    void collectPoints();

  private:
    bool render( const QString &fileName );

    QString mTestDataDir;
    QgsVectorLayer *mLinesLayer = nullptr;
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
  const QString myLinesFileName = mTestDataDir + "lines_cardinals.shp";
  const QFileInfo myLinesFileInfo( myLinesFileName );
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

  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  // See https://github.com/qgis/QGIS/issues/21836

  const QString qml = mTestDataDir + "marker_line_offset.qml";
  bool success = false;
  mLinesLayer->loadNamedStyle( qml, success );

  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -140, -140, 140, 140 ) );
  QVERIFY( render( QStringLiteral( "line_offset" ) ) );

  // TODO: -0.0 offset, see
  // https://github.com/qgis/QGIS/issues/21836#issuecomment-495853073
}

void TestQgsMarkerLineSymbol::pointNumInterval()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  QgsMarkerLineSymbolLayer *ml = new QgsMarkerLineSymbolLayer();
  ml->setPlacements( Qgis::MarkerLinePlacement::Interval );
  ml->setInterval( 4 );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, ml );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( lineSymbol );

  // make sub-symbol
  QVariantMap props;
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
  QVERIFY( render( QStringLiteral( "point_num_interval" ) ) );
}

void TestQgsMarkerLineSymbol::pointNumVertex()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  QgsMarkerLineSymbolLayer *ml = new QgsMarkerLineSymbolLayer();
  ml->setPlacements( Qgis::MarkerLinePlacement::Vertex );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, ml );
  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( lineSymbol );

  // make sub-symbol
  QVariantMap props;
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
  QVERIFY( render( QStringLiteral( "point_num_vertex" ) ) );
}

void TestQgsMarkerLineSymbol::collectPoints_data()
{
  QTest::addColumn<QVector<QPointF>>( "input" );
  QTest::addColumn<double>( "interval" );
  QTest::addColumn<double>( "initialOffset" );
  QTest::addColumn<double>( "initialLag" );
  QTest::addColumn<int>( "numberPointsRequired" );
  QTest::addColumn<QVector<QPointF>>( "expected" );

  QTest::newRow( "empty" )
      << QVector< QPointF >()
      << 1.0 << 0.0 << 0.0 << 0
      << QVector< QPointF >();

  QTest::newRow( "a" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) )
      << 1.0 << 0.0 << 0.0 << 0
      << ( QVector< QPointF >() );

  QTest::newRow( "b" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 1.0 << 0.0 << 0.0 << 0
      << ( QVector< QPointF >()  << QPointF( 2, 2 ) << QPointF( 3, 2 )
           << QPointF( 4, 2 ) << QPointF( 5, 2 ) << QPointF( 6, 2 ) << QPointF( 7, 2 ) << QPointF( 8, 2 )
           << QPointF( 9, 2 ) << QPointF( 10, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "b maxpoints" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 1.0 << 0.0 << 0.0 << 3
      << ( QVector< QPointF >()  << QPointF( 2, 2 ) << QPointF( 3, 2 )
           << QPointF( 4, 2 ) );

  QTest::newRow( "b pad points" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 1.0 << 0.0 << 0.0 << 13
      << ( QVector< QPointF >()  << QPointF( 2, 2 ) << QPointF( 3, 2 )
           << QPointF( 4, 2 ) << QPointF( 5, 2 ) << QPointF( 6, 2 ) << QPointF( 7, 2 ) << QPointF( 8, 2 )
           << QPointF( 9, 2 ) << QPointF( 10, 2 ) << QPointF( 11, 2 ) << QPointF( 11, 2 ) << QPointF( 11, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "c" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 1.0 << 1.0 << 0.0 << 0
      << ( QVector< QPointF >()  << QPointF( 1, 2 )  << QPointF( 2, 2 ) << QPointF( 3, 2 )
           << QPointF( 4, 2 ) << QPointF( 5, 2 ) << QPointF( 6, 2 ) << QPointF( 7, 2 ) << QPointF( 8, 2 )
           << QPointF( 9, 2 ) << QPointF( 10, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "c3" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 2.0 << 0.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 3, 2 ) << QPointF( 5, 2 )
           << QPointF( 7, 2 ) << QPointF( 9, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "d" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 2.0 << 1.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 2, 2 ) << QPointF( 4, 2 )
           << QPointF( 6, 2 ) << QPointF( 8, 2 ) << QPointF( 10, 2 ) );

  QTest::newRow( "e" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 2.0 << 2.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 3, 2 ) << QPointF( 5, 2 )
           << QPointF( 7, 2 ) << QPointF( 9, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "f" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 2.0 << 0.0 << 1.0 << 0
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 2, 2 )  << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 )
           << QPointF( 10, 2 ) );

  QTest::newRow( "g" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 2.0 << 0.0 << 2.0 << 0
      << ( QVector< QPointF >() << QPointF( 1, 2 )  << QPointF( 1, 2 ) << QPointF( 3, 2 ) << QPointF( 5, 2 ) << QPointF( 7, 2 )
           << QPointF( 9, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "h" )
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
      << 2.0 << 0.0 << 2.1 << 0
      << ( QVector< QPointF >() << QPointF( 1, 2 )  << QPointF( 1, 2 ) << QPointF( 2.9, 2 ) << QPointF( 4.9, 2 ) << QPointF( 6.9, 2 )
           << QPointF( 8.9, 2 ) << QPointF( 10.9, 2 ) );

  QTest::newRow( "i" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 8, 2 ) )
      << 2.0 << 2.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 ) );

  QTest::newRow( "j" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 8, 2 ) )
      << 2.0 << 0.0 << 2.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 2 )  << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 ) );

  QTest::newRow( "k" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 8, 2 ) )
      << 2.0 << 0.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 ) );

  QTest::newRow( "closed ring" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 2.0 << 2.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) );

  QTest::newRow( "closed ring required points" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 2.0 << 2.0 << 0.0 << 7
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) );
  QTest::newRow( "closed ring 1.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 1.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 1.0 << 0.0 << 11
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) );
  QTest::newRow( "closed ring initial offset 1.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring initial offset 1.0 num points" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 0.0 << 10
      << ( QVector< QPointF >() << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) );

  QTest::newRow( "closed ring 1.0 initial lag 1.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 1.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 2.0 initial lag" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 2.0 << 0.0 << 1.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 1 ) << QPointF( 1, 2 ) << QPointF( 2, 1 ) << QPointF( 1, 0 ) << QPointF( 0, 1 ) );
  QTest::newRow( "closed ring 1.0 initial lag 0.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 0.5 << 0
      << ( QVector< QPointF >() << QPointF( 0, 1.5 ) << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 ) << QPointF( 1.5, 0 )
           << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) );
  QTest::newRow( "closed ring 1.0 initial offset 0.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.5 << 0.0 << 10
      << ( QVector< QPointF >() << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 ) << QPointF( 1.5, 0 )
           << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) << QPointF( 0.5, 2.0 ) << QPointF( 1.5, 2.0 ) );
  QTest::newRow( "closed ring 1.0 initial lag 1.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 1.5 << 0
      << ( QVector< QPointF >() << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 ) << QPointF( 1.5, 0 )
           << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) );
  QTest::newRow( "closed ring 1.0 initial lag 2.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 2.0 << 0
      << ( QVector< QPointF >() << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0 initial lag 3.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 3.0 << 0
      << ( QVector< QPointF >() << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0 initial lag 3.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 3.5 << 0
      << ( QVector< QPointF >() << QPointF( 1.5, 0 ) << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 )
           << QPointF( 1.5, 0 ) << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) );
  QTest::newRow( "closed ring 1.0 initial lag 4.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 4.0 << 0
      << ( QVector< QPointF >() << QPointF( 2, 0 ) << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0 initial lag 5.0" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 1.0 << 0.0 << 5.0 << 0
      << ( QVector< QPointF >() << QPointF( 2, 1 ) << QPointF( 2, 0 ) << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
           << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );

  QTest::newRow( "simulate initial offset 0.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
      << 2.0 << 1.5 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0.5, 2 ) << QPointF( 2, 1.5 ) );
  QTest::newRow( "simulate initial offset 0.5 lag 0.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
      << 2.0 << 2.0 - ( 0.5 - 0.5 ) << 0.5 - 0.5 << 0
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 )  << QPointF( 2, 0 ) );

  QTest::newRow( "simulate initial offset 0.5 lag 0.1" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
      << 2.0 << 2.0 - ( 0.5 - 0.1 ) << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0.4, 2 ) << QPointF( 2, 1.6 ) );
  QTest::newRow( "simulate initial offset 0.1 lag 0.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
      << 2.0 << 0.0 << 0.5 - 0.1 << 0
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 1.6, 2.0 ) << QPointF( 2.0, 0.4 ) );

  QTest::newRow( "simulate initial offset 0.5 lag -0.1" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
      << 2.0 << 2.0 - 0.5 - 0.1 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0.6, 2 ) << QPointF( 2, 1.4 ) );
  QTest::newRow( "simulate initial offset 0.1 lag -0.5" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
      << 2.0 << 2.0 - 0.1 - 0.5 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0.6, 2 ) << QPointF( 2.0, 1.4 ) );

  QTest::newRow( "simulate initial offset 0.5 closed" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 2.0 << 1.5 << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 1.5, 0 ) << QPointF( 0.0, 0.5 ) );
  QTest::newRow( "simulate initial offset 0.5 lag 0.1 closed" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 2.0 << 2.0 - ( 0.5 - 0.1 ) << 0.0 << 0
      << ( QVector< QPointF >() << QPointF( 0.4, 2 ) << QPointF( 2, 1.6 ) << QPointF( 1.6, 0 ) << QPointF( 0, 0.4 ) );
  QTest::newRow( "simulate initial offset 0.1 lag 0.5 closed" )
      << ( QVector< QPointF >() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
      << 2.0 << 0.0 << 0.5 - 0.1 << 0
      << ( QVector< QPointF >() << QPointF( 0, 1.6 ) << QPointF( 1.6, 2 ) << QPointF( 2.0, 0.4 ) << QPointF( 0.4, 0.0 ) << QPointF( 0.0, 1.6 ) );

}

void TestQgsMarkerLineSymbol::collectPoints()
{
  QFETCH( QVector< QPointF >, input );
  QFETCH( double, interval );
  QFETCH( double, initialOffset );
  QFETCH( double, initialLag );
  QFETCH( int, numberPointsRequired );
  QFETCH( QVector< QPointF >, expected );

  QVector <QPointF> dest;
  QgsTemplatedLineSymbolLayerBase::collectOffsetPoints( input, dest, interval, initialOffset, initialLag, numberPointsRequired );
  QCOMPARE( dest, expected );
}

bool TestQgsMarkerLineSymbol::render( const QString &testType )
{
  mReport += "<h2>" + testType + "</h2>\n";
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "symbol_markerline" ) );
  checker.setControlName( "expected_" + testType );
  checker.setMapSettings( *mMapSettings );
  const bool result = checker.runTest( testType );
  mReport += "\n\n\n" + checker.report();
  return result;
}

QGSTEST_MAIN( TestQgsMarkerLineSymbol )
#include "testqgsmarkerlinesymbol.moc"
