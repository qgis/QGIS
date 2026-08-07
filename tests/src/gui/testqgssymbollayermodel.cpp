/***************************************************************************
                         testqgssymbollayermodel.cpp
                         --------------------------
    begin                : July 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgssettings.h"
#include "qgssymbollayermodel.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif


class TestQgsSymbolLayerModel : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void testModel();
};

void TestQgsSymbolLayerModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsSettings().clear();
}

void TestQgsSymbolLayerModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSymbolLayerModel::testModel()
{
  QgsVectorLayer *vl = new QgsVectorLayer( u"Polygon"_s, u"polygon"_s, u"memory"_s );
  QgsSymbolLayerModel model( vl, nullptr );

  std::unique_ptr<QgsFillSymbol> fillSymbol(
    QgsFillSymbol::createSimple( { { u"color"_s, u"#ff00ff"_s }, { u"line_color"_s, u"#0000ff"_s }, { u"line_width"_s, u"3"_s }, { u"joinstyle"_s, u"round"_s } } )
  );


  model.setSymbol( fillSymbol.get() );

  QCOMPARE( model.columnCount(), 1 );
  QCOMPARE( model.rowCount(), 1 );


  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), u"Fill"_s );
  QVERIFY( !model.index2node( model.index( 0, 0, QModelIndex() ) )->isLayer() );

  QModelIndex fillIndex = model.index( 0, 0, QModelIndex() );
  QCOMPARE( model.rowCount( fillIndex ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, fillIndex ), Qt::DisplayRole ).toString(), u"Simple Fill"_s );
  QVERIFY( model.index2node( model.index( 0, 0, fillIndex ) )->isLayer() );


  auto centroidFillLayer = std::make_unique<QgsCentroidFillSymbolLayer>();

  fillSymbol->appendSymbolLayer( centroidFillLayer.release() );
  model.setSymbol( fillSymbol.get() );

  QCOMPARE( model.rowCount( fillIndex ), 2 );

  QModelIndex centroidIndex = model.index( 0, 0, fillIndex );
  QModelIndex markerIndex = model.index( 0, 0, centroidIndex );
  QModelIndex simpleMarkerIndex = model.index( 0, 0, markerIndex );

  QCOMPARE( model.data( centroidIndex, Qt::DisplayRole ).toString(), u"Centroid Fill"_s );

  QCOMPARE( model.data( markerIndex, Qt::DisplayRole ).toString(), u"Marker"_s );
  QVERIFY( !model.index2node( markerIndex )->isLayer() );
  QCOMPARE( model.data( simpleMarkerIndex, Qt::DisplayRole ).toString(), u"Simple Marker"_s );
  QVERIFY( model.index2node( simpleMarkerIndex )->isLayer() );


  model.addLayer( simpleMarkerIndex );
  QCOMPARE( model.rowCount( markerIndex ), 2 );

  model.duplicateLayer( model.index2node( simpleMarkerIndex ) );
  QCOMPARE( model.rowCount( markerIndex ), 3 );

  model.removeLayer( model.index2node( simpleMarkerIndex ) );
  QCOMPARE( model.rowCount( markerIndex ), 2 );

  auto gradientFillLayer = std::make_unique<QgsGradientFillSymbolLayer>();
  model.changeLayer( model.index2node( centroidIndex ), gradientFillLayer.release() );
  QCOMPARE( model.rowCount( fillIndex ), 2 );
  QCOMPARE( model.data( model.index( 0, 0, fillIndex ), Qt::DisplayRole ).toString(), u"Gradient Fill"_s );
  QCOMPARE( model.data( model.index( 1, 0, fillIndex ), Qt::DisplayRole ).toString(), u"Simple Fill"_s );

  delete vl;
}


QGSTEST_MAIN( TestQgsSymbolLayerModel )
#include "testqgssymbollayermodel.moc"
