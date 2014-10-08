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
#include "qgscomposermapoverview.h"
#include "qgsatlascomposition.h"
#include "qgscomposerlabel.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgssymbolv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QSignalSpy>
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
    // test rendering with an autoscale atlas using the old api
    void autoscale_render_2_0_api();
    // test rendering with a fixed scale atlas
    void fixedscale_render();
    // test rendering with a fixed scale atlas using the old api
    void fixedscale_render_2_0_api();
    // test rendering with predefined scales
    void predefinedscales_render();
    // test rendering with two atlas-driven maps
    void two_map_autoscale_render();
    // test rendering with a hidden coverage
    void hiding_render();
    // test rendering with feature sorting
    void sorting_render();
    // test rendering with feature filtering
    void filtering_render();
    // test render signals
    void test_signals();
    // test removing coverage layer while atlas is enabled
    void test_remove_layer();

  private:
    QgsComposition* mComposition;
    QgsComposerLabel* mLabel1;
    QgsComposerLabel* mLabel2;
    QgsComposerMap* mAtlasMap;
    QgsComposerMap* mOverview;
    //QgsMapRenderer* mMapRenderer;
    QgsMapSettings mMapSettings;
    QgsVectorLayer* mVectorLayer;
    QgsVectorLayer* mVectorLayer2;
    QgsAtlasComposition* mAtlas;
    QString mReport;
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
  mVectorLayer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
                                      vectorFileInfo.completeBaseName(),
                                      "ogr" );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mVectorLayer->setSimplifyMethod( simplifyMethod );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mVectorLayer );

  //create composition with composer map
  mMapSettings.setLayers( QStringList() << mVectorLayer->id() );
  mMapSettings.setCrsTransformEnabled( true );
  mMapSettings.setMapUnits( QGis::Meters );

  // select epsg:2154
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 2154 );
  mMapSettings.setDestinationCrs( crs );
  mComposition = new QgsComposition( mMapSettings );
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

  mAtlas = &mComposition->atlasComposition();
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setEnabled( true );
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );

  // an overview
  mOverview = new QgsComposerMap( mComposition, 180, 20, 50, 50 );
  mOverview->setFrameEnabled( true );
  mOverview->overview()->setFrameMap( mAtlasMap->id() );
  mComposition->addComposerMap( mOverview );
  mOverview->setNewExtent( QgsRectangle( 49670.718, 6415139.086, 699672.519, 7065140.887 ) );

  // set the fill symbol of the overview map
  QgsStringMap props2;
  props2.insert( "color", "127,0,0,127" );
  QgsFillSymbolV2* fillSymbol2 = QgsFillSymbolV2::createSimple( props2 );
  mOverview->overview()->setFrameSymbol( fillSymbol2 );

  // header label
  mLabel1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mLabel1 );
  mLabel1->setText( "[% \"NAME_1\" %] area" );
  mLabel1->setFont( QgsFontUtils::getStandardTestFont() );
  //need to explictly set width, since expression hasn't been evaluated against
  //an atlas feature yet and will be shorter than required
  mLabel1->setSceneRect( QRectF( 150, 5, 60, 15 ) );

  // feature number label
  mLabel2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mLabel2 );
  mLabel2->setText( "# [%$feature || ' / ' || $numfeatures%]" );
  mLabel2->setFont( QgsFontUtils::getStandardTestFont() );
  mLabel2->setSceneRect( QRectF( 150, 200, 60, 15 ) );

  qWarning() << "header label font: " << mLabel1->font().toString() << " exactMatch:" << mLabel1->font().exactMatch();
  qWarning() << "feature number label font: " << mLabel2->font().toString() << " exactMatch:" << mLabel2->font().exactMatch();

  mReport = "<h1>Composer Atlas Tests</h1>\n";
}

void TestQgsAtlasComposition::cleanupTestCase()
{
  delete mComposition;
  delete mVectorLayer;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
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
  for ( int fi = 0; fi < mAtlas->numFeatures(); ++fi )
  {
    mAtlas->prepareForFeature( fi );
    QString expected = QString( "output_%1" ).arg(( int )( fi + 1 ) );
    QCOMPARE( mAtlas->currentFilename(), expected );
  }
  mAtlas->endRender();
}


void TestQgsAtlasComposition::autoscale_render()
{
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Auto );
  mAtlasMap->setAtlasMargin( 0.10 );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_autoscale%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
  mAtlasMap->setAtlasDriven( false );
  mAtlasMap->setAtlasMargin( 0 );
}

void TestQgsAtlasComposition::autoscale_render_2_0_api()
{
  Q_NOWARN_DEPRECATED_PUSH
  mAtlas->setComposerMap( mAtlasMap );
  mAtlas->setFixedScale( false );
  mAtlas->setMargin( 0.10f );
  Q_NOWARN_DEPRECATED_POP

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_autoscale_old_api%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
  Q_NOWARN_DEPRECATED_PUSH
  mAtlas->setFixedScale( false );
  mAtlas->setMargin( 0 );
  mAtlas->setComposerMap( 0 );
  mAtlasMap->setAtlasDriven( false );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsAtlasComposition::fixedscale_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Fixed );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_fixedscale%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();

  mAtlasMap->setAtlasDriven( false );
}

void TestQgsAtlasComposition::fixedscale_render_2_0_api()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  Q_NOWARN_DEPRECATED_PUSH
  mAtlas->setComposerMap( mAtlasMap );
  mAtlas->setFixedScale( true );
  Q_NOWARN_DEPRECATED_POP
  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_fixedscale_old_api%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
  Q_NOWARN_DEPRECATED_PUSH
  mAtlas->setFixedScale( false );
  mAtlas->setComposerMap( 0 );
  mAtlasMap->setAtlasDriven( false );

  Q_NOWARN_DEPRECATED_POP
}

void TestQgsAtlasComposition::predefinedscales_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Predefined );

  QVector<double> scales;
  scales << 1800000;
  scales << 5000000;
  mAtlas->setPredefinedScales( scales );

  {
    const QVector<double>& setScales = mAtlas->predefinedScales();
    for ( int i = 0; i < setScales.size(); i++ )
    {
      QVERIFY( setScales[i] == scales[i] );
    }
  }

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_predefinedscales%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();

  mAtlasMap->setAtlasDriven( false );
}

void TestQgsAtlasComposition::two_map_autoscale_render()
{
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Auto );
  mAtlasMap->setAtlasMargin( 0.10 );
  mOverview->setAtlasDriven( true );
  mOverview->setAtlasScalingMode( QgsComposerMap::Auto );
  mOverview->setAtlasMargin( 2.0 );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_two_maps%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
  mAtlasMap->setAtlasDriven( false );
  mAtlasMap->setAtlasMargin( 0 );
  mOverview->setAtlasDriven( false );
}

void TestQgsAtlasComposition::hiding_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  mAtlas->setHideCoverage( true );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_hiding%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::sorting_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  mAtlas->setHideCoverage( false );

  mAtlas->setSortFeatures( true );
  mAtlas->setSortKeyAttributeName( "NAME_1" ); // departement name
  mAtlas->setSortAscending( false );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_sorting%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::filtering_render()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  mAtlas->setHideCoverage( false );

  mAtlas->setSortFeatures( false );

  mAtlas->setFilterFeatures( true );
  mAtlas->setFeatureFilter( "substr(NAME_1,1,1)='P'" ); // select only 'Pays de la Loire'

  mAtlas->beginRender();

  for ( int fit = 0; fit < 1; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QString( "atlas_filtering%1" ).arg((( int )fit ) + 1 ), mComposition );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::test_signals()
{
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  mAtlas->setHideCoverage( false );
  mAtlas->setSortFeatures( false );
  mAtlas->setFilterFeatures( false );

  QSignalSpy spyRenderBegun( mAtlas, SIGNAL( renderBegun() ) );
  QSignalSpy spyRenderEnded( mAtlas, SIGNAL( renderEnded() ) );
  QSignalSpy spyPreparedForAtlas( mAtlasMap, SIGNAL( preparedForAtlas() ) );
  mAtlas->beginRender();

  QVERIFY( spyRenderBegun.count() == 1 );

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();
  }
  QVERIFY( spyPreparedForAtlas.count() == 2 );
  mAtlas->endRender();
  QVERIFY( spyRenderEnded.count() == 1 );
}

void TestQgsAtlasComposition::test_remove_layer()
{
  mAtlas->setCoverageLayer( mVectorLayer2 );
  mAtlas->setEnabled( true );

  QSignalSpy spyToggled( mAtlas, SIGNAL( toggled( bool ) ) );

  //remove coverage layer while atlas is enabled
  QgsMapLayerRegistry::instance()->removeMapLayer( mVectorLayer2->id() );
  mVectorLayer2 = 0;

  QVERIFY( !mAtlas->enabled() );
  QVERIFY( spyToggled.count() == 1 );

  //clean up
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setEnabled( true );
}

QTEST_MAIN( TestQgsAtlasComposition )
#include "moc_testqgsatlascomposition.cxx"
