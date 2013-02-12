/***************************************************************************
                         testqgsatlascomposition.cpp
                         ---------------------------
    begin                : Sept 2012
    copyright            : (C) 2012 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgscomposermap.h"
#include "qgsatlascomposition.h"
#include "qgscomposerlabel.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgssymbolv2.h"
#include "qgssinglesymbolrendererv2.h"
#include <QObject>
#include <QtTest>

class TestQgsAtlasComposition: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    // test filename pattern evaluation
    void filename();
    // test rendering with an autoscale atlas
    void autoscale_render();
    // test rendering with a fixed scale atlas
    void fixedscale_render();
    // test rendering with a hidden coverage
    void hiding_render();
    // test rendering with feature sorting
    void sorting_render();
    // test rendering with feature filtering
    void filtering_render();
  private:
    QgsComposition* mComposition;
    QgsComposerLabel* mLabel1;
    QgsComposerLabel* mLabel2;
    QgsComposerMap* mAtlasMap;
    QgsComposerMap* mOverview;
    QgsMapRenderer* mMapRenderer;
    QgsVectorLayer* mVectorLayer;
    QgsAtlasComposition* mAtlas;
};

void TestQgsAtlasComposition::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     "ogr" );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mVectorLayer );

  //create composition with composer map
  mMapRenderer = new QgsMapRenderer();
  mMapRenderer->setLayerSet( QStringList() << mVectorLayer->id() );
  mMapRenderer->setProjectionsEnabled( true );

  // select epsg:2154
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 2154 );
  mMapRenderer->setDestinationCrs( crs );
  mComposition = new QgsComposition( mMapRenderer );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  // fix the renderer, fill with green
  QgsStringMap props;
  props.insert( "color", "0,127,0" );
  QgsFillSymbolV2* fillSymbol = QgsFillSymbolV2::createSimple( props );
  QgsSingleSymbolRendererV2* renderer = new QgsSingleSymbolRendererV2( fillSymbol );
  mVectorLayer->setRendererV2( renderer );

  // the atlas map
  mAtlasMap = new QgsComposerMap( mComposition, 20, 20, 130, 130 );
  mAtlasMap->setFrameEnabled( true );
  mComposition->addComposerMap( mAtlasMap );

  mAtlas = new QgsAtlasComposition( mComposition );
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setComposerMap( mAtlasMap );

  // an overview
  mOverview = new QgsComposerMap( mComposition, 180, 20, 50, 50 );
  mOverview->setFrameEnabled( true );
  mOverview->setOverviewFrameMap( mAtlasMap->id() );
  mComposition->addComposerMap( mOverview );
  mOverview->setNewExtent( QgsRectangle( 49670.718, 6415139.086, 699672.519, 7065140.887 ) );

  // set the fill symbol of the overview map
  QgsStringMap props2;
  props2.insert( "color", "127,0,0,127" );
  QgsFillSymbolV2* fillSymbol2 = QgsFillSymbolV2::createSimple( props2 );
  mOverview->setOverviewFrameMapSymbol( fillSymbol2 );

  // header label
  mLabel1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mLabel1 );
  mLabel1->setText( "[% \"NAME_1\" %] area" );
  mLabel1->adjustSizeToText();
  mLabel1->setItemPosition( 150, 5 );

  // feature number label
  mLabel2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mLabel2 );
  mLabel2->setText( "# [%$feature || ' / ' || $numfeatures%]" );
  mLabel2->adjustSizeToText();
  mLabel2->setItemPosition( 150, 200 );
}

void TestQgsAtlasComposition::cleanupTestCase()
{
  delete mComposition;
  delete mMapRenderer;
  delete mVectorLayer;
}

void TestQgsAtlasComposition::init()
{

}

void TestQgsAtlasComposition::cleanup()
{

}

void TestQgsAtlasComposition::filename()
{
  mAtlas->setFilenamePattern( "'output_' || $feature" );
  mAtlas->beginRender();
  for ( size_t fi = 0; fi < mAtlas->numFeatures(); ++fi )
  {
    mAtlas->prepareForFeature( fi );
    QString expected = QString( "output_%1" ).arg(( int )( fi + 1 ) );
    QCOMPARE( mAtlas->currentFilename(), expected );
  }
  mAtlas->endRender();
}


void TestQgsAtlasComposition::autoscale_render()
{
  mAtlas->setFixedScale( false );
  mAtlas->setMargin( 0.10f );

  mAtlas->beginRender();

  for ( size_t fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( "Atlas autoscale test", mComposition,
                                   QString( TEST_DATA_DIR ) + QDir::separator() + "control_images" + QDir::separator() +
                                   "expected_composermapatlas" + QDir::separator() +
                                   QString( "autoscale_%1.png" ).arg(( int )fit ) );
    QVERIFY( checker.testComposition( 0 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::fixedscale_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlas->setFixedScale( true );

  mAtlas->beginRender();

  for ( size_t fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( "Atlas fixedscale test", mComposition,
                                   QString( TEST_DATA_DIR ) + QDir::separator() + "control_images" + QDir::separator() +
                                   "expected_composermapatlas" + QDir::separator() +
                                   QString( "fixedscale_%1.png" ).arg(( int )fit ) );
    QVERIFY( checker.testComposition( 0 ) );
  }
  mAtlas->endRender();

}

void TestQgsAtlasComposition::hiding_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlas->setFixedScale( true );
  mAtlas->setHideCoverage( true );

  mAtlas->beginRender();

  for ( size_t fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( "Atlas hidden test", mComposition,
                                   QString( TEST_DATA_DIR ) + QDir::separator() + "control_images" + QDir::separator() +
                                   "expected_composermapatlas" + QDir::separator() +
                                   QString( "hiding_%1.png" ).arg(( int )fit ) );
    QVERIFY( checker.testComposition( 0 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::sorting_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlas->setFixedScale( true );
  mAtlas->setHideCoverage( false );

  mAtlas->setSortFeatures( true );
  mAtlas->setSortKeyAttributeIndex( 4 ); // departement name
  mAtlas->setSortAscending( false );

  mAtlas->beginRender();

  for ( size_t fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( "Atlas sorting test", mComposition,
                                   QString( TEST_DATA_DIR ) + QDir::separator() + "control_images" + QDir::separator() +
                                   "expected_composermapatlas" + QDir::separator() +
                                   QString( "sorting_%1.png" ).arg(( int )fit ) );
    QVERIFY( checker.testComposition( 0 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::filtering_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlas->setFixedScale( true );
  mAtlas->setHideCoverage( false );

  mAtlas->setSortFeatures( false );

  mAtlas->setFeatureFilter( "substr(NAME_1,1,1)='P'" ); // select only 'Pays de la Loire'

  mAtlas->beginRender();

  for ( size_t fit = 0; fit < 1; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( "Atlas filtering test", mComposition,
                                   QString( TEST_DATA_DIR ) + QDir::separator() + "control_images" + QDir::separator() +
                                   "expected_composermapatlas" + QDir::separator() +
                                   QString( "filtering_%1.png" ).arg(( int )fit ) );
    QVERIFY( checker.testComposition( 0 ) );
  }
  mAtlas->endRender();
}

QTEST_MAIN( TestQgsAtlasComposition )
#include "moc_testqgsatlascomposition.cxx"
