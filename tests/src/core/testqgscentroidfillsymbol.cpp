/***************************************************************************
     TestQgsCentroidFillSymbol.cpp
     -------------------------
    Date                 : Apr 2015
    Copyright            : (C) 2015 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
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
#include <QDesktopServices>

//qgis includes...
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbol.h"

/**
 * \ingroup UnitTests
 * This is a unit test for line fill symbol types.
 */
class TestQgsCentroidFillSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCentroidFillSymbol()
      : QgsTest( QStringLiteral( "Centroid Fill Symbol Tests" ), QStringLiteral( "symbol_centroidfill" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void centroidFillSymbol();
    void centroidFillSymbolPointOnSurface();
    void centroidFillSymbolPartBiggest();
    void centroidFillClipPoints();
    void centroidFillClipOnCurrentPartOnly();
    void centroidFillClipOnCurrentPartOnlyBiggest();
    void centroidFillClipMultiplayerPoints();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();

  private:
    bool mTestHasError = false;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsCentroidFillSymbolLayer *mCentroidFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsCentroidFillSymbol::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(), myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  mCentroidFill = new QgsCentroidFillSymbolLayer();
  static_cast<QgsSimpleMarkerSymbolLayer *>( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mCentroidFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
}
void TestQgsCentroidFillSymbol::cleanupTestCase()
{
  delete mpPolysLayer;

  QgsApplication::exitQgis();
}

void TestQgsCentroidFillSymbol::centroidFillSymbol()
{
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill", "symbol_centroidfill", mMapSettings );
}

void TestQgsCentroidFillSymbol::centroidFillSymbolPointOnSurface()
{
  mCentroidFill->setPointOnSurface( true );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_point_on_surface", "symbol_centroidfill_point_on_surface", mMapSettings );
  mCentroidFill->setPointOnSurface( false );
}

void TestQgsCentroidFillSymbol::centroidFillSymbolPartBiggest()
{
  mCentroidFill->setPointOnAllParts( false );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_part_biggest", "symbol_centroidfill_part_biggest", mMapSettings );
  mCentroidFill->setPointOnAllParts( true );
}

void TestQgsCentroidFillSymbol::centroidFillClipPoints()
{
  mCentroidFill->setClipPoints( true );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_clip_points", "symbol_centroidfill_clip_points", mMapSettings );
  mCentroidFill->setClipPoints( false );
}

void TestQgsCentroidFillSymbol::centroidFillClipOnCurrentPartOnly()
{
  mCentroidFill->setClipPoints( true );
  mCentroidFill->setClipOnCurrentPartOnly( true );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_clip_current_only", "symbol_centroidfill_clip_current_only", mMapSettings );
  mCentroidFill->setClipPoints( false );
  mCentroidFill->setClipOnCurrentPartOnly( false );
}

void TestQgsCentroidFillSymbol::centroidFillClipOnCurrentPartOnlyBiggest()
{
  mCentroidFill->setClipPoints( true );
  mCentroidFill->setClipOnCurrentPartOnly( true );
  mCentroidFill->setPointOnAllParts( false );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_clip_current_biggest", "symbol_centroidfill_clip_current_biggest", mMapSettings );
  mCentroidFill->setClipPoints( false );
  mCentroidFill->setClipOnCurrentPartOnly( false );
  mCentroidFill->setPointOnAllParts( true );
}

void TestQgsCentroidFillSymbol::centroidFillClipMultiplayerPoints()
{
  const QgsSimpleFillSymbolLayer simpleFill( QColor( 255, 255, 255, 100 ) );

  mCentroidFill = mCentroidFill->clone();
  mCentroidFill->setClipPoints( true );

  mFillSymbol->deleteSymbolLayer( 0 );
  mFillSymbol->appendSymbolLayer( simpleFill.clone() );
  mFillSymbol->appendSymbolLayer( mCentroidFill->clone() );
  mFillSymbol->appendSymbolLayer( simpleFill.clone() );

  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_clip_multilayer", "symbol_centroidfill_clip_multilayer", mMapSettings );

  mCentroidFill->setClipPoints( false );
  mFillSymbol->deleteSymbolLayer( 0 );
  mFillSymbol->deleteSymbolLayer( 1 );
  mFillSymbol->deleteSymbolLayer( 2 );
  mFillSymbol->changeSymbolLayer( 0, mCentroidFill );
}


void TestQgsCentroidFillSymbol::opacityWithDataDefinedColor()
{
  const QgsSimpleFillSymbolLayer simpleFill( QColor( 255, 255, 255, 100 ) );

  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'red', 'green')" ) ) );
  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'blue', 'magenta')" ) ) );
  qgis::down_cast<QgsSimpleMarkerSymbolLayer *>( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeWidth( 0.5 );
  qgis::down_cast<QgsSimpleMarkerSymbolLayer *>( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setSize( 5 );
  mCentroidFill->subSymbol()->setOpacity( 0.5 );
  mFillSymbol->setOpacity( 0.5 );

  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_opacityddcolor", "symbol_centroidfill_opacityddcolor", mMapSettings );
}

void TestQgsCentroidFillSymbol::dataDefinedOpacity()
{
  const QgsSimpleFillSymbolLayer simpleFill( QColor( 255, 255, 255, 100 ) );

  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'red', 'green')" ) ) );
  mCentroidFill->subSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(Name='Dam', 'blue', 'magenta')" ) ) );
  qgis::down_cast<QgsSimpleMarkerSymbolLayer *>( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeWidth( 0.5 );
  qgis::down_cast<QgsSimpleMarkerSymbolLayer *>( mCentroidFill->subSymbol()->symbolLayer( 0 ) )->setSize( 5 );
  mCentroidFill->subSymbol()->setOpacity( 0.5 );
  mFillSymbol->setOpacity( 1.0 );
  mFillSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Value\" >10, 25, 50)" ) ) );

  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_centroidfill_ddopacity", "symbol_centroidfill_ddopacity", mMapSettings );
}

QGSTEST_MAIN( TestQgsCentroidFillSymbol )
#include "testqgscentroidfillsymbol.moc"
