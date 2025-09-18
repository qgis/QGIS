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
class TestQgsMarkerLineSymbol : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsMarkerLineSymbol()
      : QgsTest( QStringLiteral( "Line Marker Symbol Tests" ) )
    {
      mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
    }

    ~TestQgsMarkerLineSymbol() override;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void lineOffset();
    void pointNumInterval();
    void pointNumVertex();
    void collectPoints_data();
    void collectPoints();
    void parseBlankSegments_data();
    void parseBlankSegments();
    void parseBlankSegmentsMapUnits();

  private:
    bool render( const QString &fileName );

    QString mTestDataDir;
    QgsVectorLayer *mLinesLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
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
  mLinesLayer = new QgsVectorLayer( myLinesFileInfo.filePath(), myLinesFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  mapLayers << mLinesLayer;

  // Register all layers with the registry
  QgsProject::instance()->addMapLayers( mapLayers );

  // This is needed to correctly set rotation center,
  // the actual size doesn't matter as QgsRenderChecker will
  // re-set it to the size of the expected image
  mMapSettings->setOutputSize( QSize( 256, 256 ) );

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

TestQgsMarkerLineSymbol::~TestQgsMarkerLineSymbol() = default;

//runs after all tests
void TestQgsMarkerLineSymbol::cleanupTestCase()
{
  delete mMapSettings;
  QgsApplication::exitQgis();
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
  QgsSimpleMarkerSymbolLayer *marker = static_cast<QgsSimpleMarkerSymbolLayer *>( QgsSimpleMarkerSymbolLayer::create( props ) );

  marker->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "@geometry_point_num * 2" ) ) );

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
  QgsSimpleMarkerSymbolLayer *marker = static_cast<QgsSimpleMarkerSymbolLayer *>( QgsSimpleMarkerSymbolLayer::create( props ) );

  marker->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "@geometry_point_num * 2" ) ) );

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
    << QVector<QPointF>()
    << 1.0 << 0.0 << 0.0 << 0
    << QVector<QPointF>();

  QTest::newRow( "a" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) )
    << 1.0 << 0.0 << 0.0 << 0
    << ( QVector<QPointF>() );

  QTest::newRow( "b" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 1.0 << 0.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 2, 2 ) << QPointF( 3, 2 )
                            << QPointF( 4, 2 ) << QPointF( 5, 2 ) << QPointF( 6, 2 ) << QPointF( 7, 2 ) << QPointF( 8, 2 )
                            << QPointF( 9, 2 ) << QPointF( 10, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "b maxpoints" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 1.0 << 0.0 << 0.0 << 3
    << ( QVector<QPointF>() << QPointF( 2, 2 ) << QPointF( 3, 2 )
                            << QPointF( 4, 2 ) );

  QTest::newRow( "b pad points" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 1.0 << 0.0 << 0.0 << 13
    << ( QVector<QPointF>() << QPointF( 2, 2 ) << QPointF( 3, 2 )
                            << QPointF( 4, 2 ) << QPointF( 5, 2 ) << QPointF( 6, 2 ) << QPointF( 7, 2 ) << QPointF( 8, 2 )
                            << QPointF( 9, 2 ) << QPointF( 10, 2 ) << QPointF( 11, 2 ) << QPointF( 11, 2 ) << QPointF( 11, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "c" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 1.0 << 1.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 3, 2 )
                            << QPointF( 4, 2 ) << QPointF( 5, 2 ) << QPointF( 6, 2 ) << QPointF( 7, 2 ) << QPointF( 8, 2 )
                            << QPointF( 9, 2 ) << QPointF( 10, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "c3" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 2.0 << 0.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 3, 2 ) << QPointF( 5, 2 )
                            << QPointF( 7, 2 ) << QPointF( 9, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "d" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 2.0 << 1.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 2, 2 ) << QPointF( 4, 2 )
                            << QPointF( 6, 2 ) << QPointF( 8, 2 ) << QPointF( 10, 2 ) );

  QTest::newRow( "e" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 2.0 << 2.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 3, 2 ) << QPointF( 5, 2 )
                            << QPointF( 7, 2 ) << QPointF( 9, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "f" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 2.0 << 0.0 << 1.0 << 0
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 )
                            << QPointF( 10, 2 ) );

  QTest::newRow( "g" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 2.0 << 0.0 << 2.0 << 0
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 1, 2 ) << QPointF( 3, 2 ) << QPointF( 5, 2 ) << QPointF( 7, 2 )
                            << QPointF( 9, 2 ) << QPointF( 11, 2 ) );

  QTest::newRow( "h" )
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 11, 2 ) )
    << 2.0 << 0.0 << 2.1 << 0
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 1, 2 ) << QPointF( 2.9, 2 ) << QPointF( 4.9, 2 ) << QPointF( 6.9, 2 )
                            << QPointF( 8.9, 2 ) << QPointF( 10.9, 2 ) );

  QTest::newRow( "i" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 8, 2 ) )
    << 2.0 << 2.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 ) );

  QTest::newRow( "j" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 8, 2 ) )
    << 2.0 << 0.0 << 2.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 ) );

  QTest::newRow( "k" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 8, 2 ) )
    << 2.0 << 0.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 2, 2 ) << QPointF( 4, 2 ) << QPointF( 6, 2 ) << QPointF( 8, 2 ) );

  QTest::newRow( "closed ring" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 2.0 << 2.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) );

  QTest::newRow( "closed ring required points" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 2.0 << 2.0 << 0.0 << 7
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) );
  QTest::newRow( "closed ring 1.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 1.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 1.0 << 0.0 << 11
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) );
  QTest::newRow( "closed ring initial offset 1.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring initial offset 1.0 num points" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 0.0 << 10
    << ( QVector<QPointF>() << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) );

  QTest::newRow( "closed ring 1.0 initial lag 1.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 1.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 2.0 initial lag" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 2.0 << 0.0 << 1.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 1 ) << QPointF( 1, 2 ) << QPointF( 2, 1 ) << QPointF( 1, 0 ) << QPointF( 0, 1 ) );
  QTest::newRow( "closed ring 1.0 initial lag 0.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 0.5 << 0
    << ( QVector<QPointF>() << QPointF( 0, 1.5 ) << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 ) << QPointF( 1.5, 0 )
                            << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) );
  QTest::newRow( "closed ring 1.0 initial offset 0.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.5 << 0.0 << 10
    << ( QVector<QPointF>() << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 ) << QPointF( 1.5, 0 )
                            << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) << QPointF( 0.5, 2.0 ) << QPointF( 1.5, 2.0 ) );
  QTest::newRow( "closed ring 1.0 initial lag 1.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 1.5 << 0
    << ( QVector<QPointF>() << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 ) << QPointF( 1.5, 0 )
                            << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) );
  QTest::newRow( "closed ring 1.0 initial lag 2.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 2.0 << 0
    << ( QVector<QPointF>() << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0 initial lag 3.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 3.0 << 0
    << ( QVector<QPointF>() << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0 initial lag 3.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 3.5 << 0
    << ( QVector<QPointF>() << QPointF( 1.5, 0 ) << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) << QPointF( 0.5, 2 ) << QPointF( 1.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 2, 0.5 )
                            << QPointF( 1.5, 0 ) << QPointF( 0.5, 0 ) << QPointF( 0, 0.5 ) << QPointF( 0, 1.5 ) );
  QTest::newRow( "closed ring 1.0 initial lag 4.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 4.0 << 0
    << ( QVector<QPointF>() << QPointF( 2, 0 ) << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );
  QTest::newRow( "closed ring 1.0 initial lag 5.0" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 1.0 << 0.0 << 5.0 << 0
    << ( QVector<QPointF>() << QPointF( 2, 1 ) << QPointF( 2, 0 ) << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) << QPointF( 1, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 1 ) << QPointF( 2, 0 )
                            << QPointF( 1, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 1 ) << QPointF( 0, 2 ) );

  QTest::newRow( "simulate initial offset 0.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
    << 2.0 << 1.5 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0.5, 2 ) << QPointF( 2, 1.5 ) );
  QTest::newRow( "simulate initial offset 0.5 lag 0.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
    << 2.0 << 2.0 - ( 0.5 - 0.5 ) << 0.5 - 0.5 << 0
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) );

  QTest::newRow( "simulate initial offset 0.5 lag 0.1" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
    << 2.0 << 2.0 - ( 0.5 - 0.1 ) << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0.4, 2 ) << QPointF( 2, 1.6 ) );
  QTest::newRow( "simulate initial offset 0.1 lag 0.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
    << 2.0 << 0.0 << 0.5 - 0.1 << 0
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 1.6, 2.0 ) << QPointF( 2.0, 0.4 ) );

  QTest::newRow( "simulate initial offset 0.5 lag -0.1" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
    << 2.0 << 2.0 - 0.5 - 0.1 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0.6, 2 ) << QPointF( 2, 1.4 ) );
  QTest::newRow( "simulate initial offset 0.1 lag -0.5" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) )
    << 2.0 << 2.0 - 0.1 - 0.5 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0.6, 2 ) << QPointF( 2.0, 1.4 ) );

  QTest::newRow( "simulate initial offset 0.5 closed" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 2.0 << 1.5 << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0.5, 2 ) << QPointF( 2, 1.5 ) << QPointF( 1.5, 0 ) << QPointF( 0.0, 0.5 ) );
  QTest::newRow( "simulate initial offset 0.5 lag 0.1 closed" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 2.0 << 2.0 - ( 0.5 - 0.1 ) << 0.0 << 0
    << ( QVector<QPointF>() << QPointF( 0.4, 2 ) << QPointF( 2, 1.6 ) << QPointF( 1.6, 0 ) << QPointF( 0, 0.4 ) );
  QTest::newRow( "simulate initial offset 0.1 lag 0.5 closed" )
    << ( QVector<QPointF>() << QPointF( 0, 2 ) << QPointF( 2, 2 ) << QPointF( 2, 0 ) << QPointF( 0, 0 ) << QPointF( 0, 2 ) )
    << 2.0 << 0.0 << 0.5 - 0.1 << 0
    << ( QVector<QPointF>() << QPointF( 0, 1.6 ) << QPointF( 1.6, 2 ) << QPointF( 2.0, 0.4 ) << QPointF( 0.4, 0.0 ) << QPointF( 0.0, 1.6 ) );
}

void TestQgsMarkerLineSymbol::collectPoints()
{
  QFETCH( QVector<QPointF>, input );
  QFETCH( double, interval );
  QFETCH( double, initialOffset );
  QFETCH( double, initialLag );
  QFETCH( int, numberPointsRequired );
  QFETCH( QVector<QPointF>, expected );

  QVector<QPointF> dest;
  QgsTemplatedLineSymbolLayerBase::collectOffsetPoints( input, dest, interval, initialOffset, nullptr, initialLag, numberPointsRequired );
  QCOMPARE( dest, expected );
}

void TestQgsMarkerLineSymbol::parseBlankSegments_data()
{
  QTest::addColumn<QString>( "strBlankSegments" );
  QTest::addColumn<int>( "partNum" );
  QTest::addColumn<int>( "iRing" );
  QTest::addColumn<QgsTemplatedLineSymbolLayerBase::BlankSegments>( "expectedBlankSegments" );

  QTest::newRow( "simple" ) << QStringLiteral( "(((1 2, 3 4)))" ) << 1 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments { { 1, 2 }, { 3, 4 } };
  QTest::newRow( "multipart and ring, part 1, ring 0" ) << QStringLiteral( "(((1 2, 3 4),(5 6, 7 8)),((9 10, 11 12),(13 14, 15 16)))" ) << 1 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments { { 1, 2 }, { 3, 4 } };
  QTest::newRow( "multipart and ring, part 2, ring 0" ) << QStringLiteral( "(((1 2, 3 4),(5 6, 7 8)),((9 10, 11 12),(13 14, 15 16)))" ) << 2 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments { { 9, 10 }, { 11, 12 } };
  QTest::newRow( "multipart and ring, part 1, ring 1" ) << QStringLiteral( "(((1 2, 3 4),(5 6, 7 8)),((9 10, 11 12),(13 14, 15 16)))" ) << 1 << 1 << QgsTemplatedLineSymbolLayerBase::BlankSegments { { 5, 6 }, { 7, 8 } };
  QTest::newRow( "multipart and ring, part 2, ring 1" ) << QStringLiteral( "(((1 2, 3 4),(5 6, 7 8)),((9 10, 11 12),(13 14, 15 16)))" ) << 2 << 1 << QgsTemplatedLineSymbolLayerBase::BlankSegments { { 13, 14 }, { 15, 16 } };
  QTest::newRow( "multipart and ring, invalid part" ) << QStringLiteral( "(((1 2, 3 4),(5 6, 7 8)),((9 10, 11 12),(13 14, 15 16)))" ) << 3 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments {};
  QTest::newRow( "multipart and ring, invalid ring" ) << QStringLiteral( "(((1 2, 3 4),(5 6, 7 8)),((9 10, 11 12),(13 14, 15 16)))" ) << 1 << 2 << QgsTemplatedLineSymbolLayerBase::BlankSegments {};
  QTest::newRow( "malformed" ) << QStringLiteral( "(((test)))" ) << 1 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments {};
  QTest::newRow( "Distances not ordered" ) << QStringLiteral( "(((3 4,1 2)))" ) << 1 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments {};
  QTest::newRow( "start > end" ) << QStringLiteral( "(((2 1,3 4)))" ) << 1 << 0 << QgsTemplatedLineSymbolLayerBase::BlankSegments {};
}

void TestQgsMarkerLineSymbol::parseBlankSegments()
{
  QFETCH( QString, strBlankSegments );
  QFETCH( int, partNum );
  QFETCH( int, iRing );
  QFETCH( QgsTemplatedLineSymbolLayerBase::BlankSegments, expectedBlankSegments );

  QgsRenderContext rc;
  QgsTemplatedLineSymbolLayerBase::BlankSegments blanksegments = QgsTemplatedLineSymbolLayerBase::parseBlankSegments( strBlankSegments, rc, Qgis::RenderUnit::Pixels, partNum, iRing );

  QCOMPARE( blanksegments, expectedBlankSegments );
}

void TestQgsMarkerLineSymbol::parseBlankSegmentsMapUnits()
{
  QgsRenderContext rc;
  QgsMapToPixel m2p( 2 );
  rc.setMapToPixel( m2p );

  QgsTemplatedLineSymbolLayerBase::BlankSegments blanksegments = QgsTemplatedLineSymbolLayerBase::parseBlankSegments( QStringLiteral( "(((1 2, 3 4)))" ), rc, Qgis::RenderUnit::MapUnits, 1, 0 );
  QgsTemplatedLineSymbolLayerBase::BlankSegments expectedBlankSegments { { 0.5, 1 }, { 1.5, 2 } };
  QCOMPARE( blanksegments, expectedBlankSegments );
}

bool TestQgsMarkerLineSymbol::render( const QString &testType )
{
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "symbol_markerline" ) );
  checker.setControlName( "expected_" + testType );
  checker.setMapSettings( *mMapSettings );
  const bool result = checker.runTest( testType );
  mReport += checker.report();
  return result;
}

QGSTEST_MAIN( TestQgsMarkerLineSymbol )
#include "testqgsmarkerlinesymbol.moc"
