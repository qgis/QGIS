/***************************************************************************
                         testqgslayoutatlas.cpp
                         ---------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
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
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmapoverview.h"
#include "qgslayoutatlas.h"
#include "qgslayoutitemlabel.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfontutils.h"
#include "qgsprintlayout.h"
#include <QObject>
#include <QtTest/QSignalSpy>
#include "qgstest.h"
#include "qgsfillsymbol.h"

class TestQgsLayoutAtlas : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutAtlas() : QgsTest( QStringLiteral( "Layout Atlas Tests" ) ) {}

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

    void context();

  private:
    QgsPrintLayout *mLayout = nullptr;
    QgsLayoutItemLabel *mLabel1 = nullptr;
    QgsLayoutItemLabel *mLabel2 = nullptr;
    QgsLayoutItemMap *mAtlasMap = nullptr;
    QgsLayoutItemMap *mOverview = nullptr;
    QgsVectorLayer *mVectorLayer = nullptr;
    QgsVectorLayer *mVectorLayer2 = nullptr;
    QgsLayoutAtlas *mAtlas = nullptr;
};

void TestQgsLayoutAtlas::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  const QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );
  mVectorLayer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
                                      vectorFileInfo.completeBaseName(),
                                      QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mVectorLayer->setSimplifyMethod( simplifyMethod );
}

void TestQgsLayoutAtlas::cleanupTestCase()
{
  delete mLayout;
  delete mVectorLayer;
  QgsApplication::exitQgis();
}

void TestQgsLayoutAtlas::init()
{
  //create composition with composer map

  const QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:2154" ) );
  QgsProject::instance()->setCrs( crs );
  mLayout = new QgsPrintLayout( QgsProject::instance() );
  mLayout->initializeDefaults();

  // fix the renderer, fill with green
  QVariantMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "0,127,0" ) );
  props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "0,0,0" ) );
  QgsFillSymbol *fillSymbol = QgsFillSymbol::createSimple( props );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( fillSymbol );
  mVectorLayer->setRenderer( renderer );

  // the atlas map
  mAtlasMap = new QgsLayoutItemMap( mLayout );
  mAtlasMap->attemptSetSceneRect( QRectF( 20, 20, 130, 130 ) );
  mAtlasMap->setFrameEnabled( true );
  mLayout->addLayoutItem( mAtlasMap );
  mAtlasMap->setLayers( QList<QgsMapLayer *>() << mVectorLayer );

  mAtlas = mLayout->atlas();
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setEnabled( true );

  // an overview
  mOverview = new QgsLayoutItemMap( mLayout );
  mOverview->attemptSetSceneRect( QRectF( 180, 20, 50, 50 ) );
  mOverview->setFrameEnabled( true );
  mOverview->overview()->setLinkedMap( mAtlasMap );
  mOverview->setLayers( QList<QgsMapLayer *>() << mVectorLayer );
  mLayout->addLayoutItem( mOverview );
  mOverview->setExtent( QgsRectangle( 49670.718, 6415139.086, 699672.519, 7065140.887 ) );

  // set the fill symbol of the overview map
  QVariantMap props2;
  props2.insert( QStringLiteral( "color" ), QStringLiteral( "127,0,0,127" ) );
  props2.insert( QStringLiteral( "outline_color" ), QStringLiteral( "0,0,0" ) );
  QgsFillSymbol *fillSymbol2 = QgsFillSymbol::createSimple( props2 );
  mOverview->overview()->setFrameSymbol( fillSymbol2 );

  // header label
  mLabel1 = new QgsLayoutItemLabel( mLayout );
  mLayout->addLayoutItem( mLabel1 );
  mLabel1->setText( QStringLiteral( "[% \"NAME_1\" %] area" ) );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont() );
  format.setSize( 12 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );
  mLabel1->setTextFormat( format );
  mLabel1->setMarginX( 1 );
  mLabel1->setMarginY( 1 );
  //need to explicitly set width, since expression hasn't been evaluated against
  //an atlas feature yet and will be shorter than required
  mLabel1->attemptSetSceneRect( QRectF( 150, 5, 60, 15 ) );

  // feature number label
  mLabel2 = new QgsLayoutItemLabel( mLayout );
  mLayout->addLayoutItem( mLabel2 );
  mLabel2->setText( QStringLiteral( "# [%@atlas_featurenumber || ' / ' || @atlas_totalfeatures%]" ) );
  mLabel2->setTextFormat( format );
  mLabel2->attemptSetSceneRect( QRectF( 150, 200, 60, 15 ) );
  mLabel2->setMarginX( 1 );
  mLabel2->setMarginY( 1 );


  qDebug() << "header label font: " << mLabel1->textFormat().font().toString() << " exactMatch:" << mLabel1->textFormat().font().exactMatch();
  qDebug() << "feature number label font: " << mLabel2->textFormat().font().toString() << " exactMatch:" << mLabel2->textFormat().font().exactMatch();
}

void TestQgsLayoutAtlas::cleanup()
{
  delete mLayout;
  mLayout = nullptr;
}

void TestQgsLayoutAtlas::filename()
{
  QString error;
  mAtlas->setFilenameExpression( QStringLiteral( "'output_' || @atlas_featurenumber" ), error );
  mAtlas->beginRender();
  for ( int fi = 0; fi < mAtlas->count(); ++fi )
  {
    mAtlas->seekTo( fi );
    const QString expected = QStringLiteral( "output_%1" ).arg( ( int )( fi + 1 ) );
    QCOMPARE( mAtlas->currentFilename(), expected );
  }
  mAtlas->endRender();
}


void TestQgsLayoutAtlas::autoscale_render()
{
  mAtlasMap->setExtent( QgsRectangle( 332719.06221504929, 6765214.5887386119, 560957.85090677091, 6993453.3774303338 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Auto );
  mAtlasMap->setAtlasMargin( 0.10 );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_autoscale%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::fixedscale_render()
{
  //TODO QGIS3.0 - setting the extent AFTER setting atlas driven/fixed scaling mode should
  //also update the set fixed scale
  mAtlasMap->setExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Fixed );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_fixedscale%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::predefinedscales_render()
{
  //TODO QGIS3.0 - setting the extent AFTER setting atlas driven/predefined scaling mode should
  //also update the atlas map scale
  mAtlasMap->setExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Predefined );

  QVector<qreal> scales;
  scales << 1800000.0;
  scales << 5000000.0;
  mLayout->renderContext().setPredefinedScales( scales );
  {
    const QVector<qreal> &setScales = mLayout->renderContext().predefinedScales();
    for ( int i = 0; i < setScales.size(); i++ )
    {
      QVERIFY( setScales[i] == scales[i] );
    }
  }

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_predefinedscales%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::two_map_autoscale_render()
{
  mAtlasMap->setExtent( QgsRectangle( 332719.06221504929, 6765214.5887386119, 560957.85090677091, 6993453.3774303338 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Auto );
  mAtlasMap->setAtlasMargin( 0.10 );
  mOverview->setAtlasDriven( true );
  mOverview->setAtlasScalingMode( QgsLayoutItemMap::Auto );
  mOverview->setAtlasMargin( 2.0 );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_two_maps%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::hiding_render()
{
  mAtlasMap->setExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Fixed );
  mAtlas->setHideCoverage( true );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_hiding%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::sorting_render()
{
  mAtlasMap->setExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Fixed );
  mAtlas->setHideCoverage( false );

  mAtlas->setSortFeatures( true );
  mAtlas->setSortExpression( QStringLiteral( "NAME_1" ) ); // departement name
  mAtlas->setSortAscending( false );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_sorting%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::filtering_render()
{
  mAtlasMap->setExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Fixed );
  mAtlas->setHideCoverage( false );

  mAtlas->setSortFeatures( false );

  mAtlas->setFilterFeatures( true );
  QString error;
  mAtlas->setFilterExpression( QStringLiteral( "substr(NAME_1,1,1)='P'" ), error ); // select only 'Pays de la Loire'

  mAtlas->beginRender();

  for ( int fit = 0; fit < 1; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();

    QgsLayoutChecker checker( QStringLiteral( "atlas_filtering%1" ).arg( ( ( int )fit ) + 1 ), mLayout );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testLayout( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsLayoutAtlas::test_signals()
{
  mAtlasMap->setExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsLayoutItemMap::Fixed );
  mAtlas->setHideCoverage( false );
  mAtlas->setSortFeatures( false );
  mAtlas->setFilterFeatures( false );

  const QSignalSpy spyRenderBegun( mAtlas, &QgsLayoutAtlas::renderBegun );
  const QSignalSpy spyRenderEnded( mAtlas, &QgsLayoutAtlas::renderEnded );
  const QSignalSpy spyFeatureChanged( mAtlas, &QgsLayoutAtlas::featureChanged );
  const QSignalSpy spyPreparedForAtlas( mAtlasMap, &QgsLayoutItemMap::preparedForAtlas );
  mAtlas->beginRender();

  QCOMPARE( spyRenderBegun.count(), 1 );

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->seekTo( fit );
    mLabel1->adjustSizeToText();
  }
  QCOMPARE( spyPreparedForAtlas.count(), 2 );
  QCOMPARE( spyFeatureChanged.count(), 2 );
  mAtlas->endRender();
  QCOMPARE( spyRenderEnded.count(), 1 );
}

void TestQgsLayoutAtlas::test_remove_layer()
{
  QgsProject::instance()->addMapLayer( mVectorLayer2 );
  mAtlas->setCoverageLayer( mVectorLayer2 );
  mAtlas->setEnabled( true );

  const QSignalSpy spyToggled( mAtlas, SIGNAL( toggled( bool ) ) );

  //remove coverage layer while atlas is enabled
  QgsProject::instance()->removeMapLayer( mVectorLayer2->id() );
  mVectorLayer2 = nullptr;

  QVERIFY( !mAtlas->enabled() );
  QVERIFY( spyToggled.count() == 1 );
}

void TestQgsLayoutAtlas::context()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer&field=labelx:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFeature f;
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  QgsFeature f2;
  QVERIFY( vl2->dataProvider()->addFeature( f2 ) );

  mAtlas->setCoverageLayer( vl2.get() );
  mAtlas->setEnabled( true );

  const QgsExpressionContext context = mAtlas->createExpressionContext();
  QVERIFY( context.hasVariable( QStringLiteral( "project_title" ) ) );
  QVERIFY( context.hasVariable( QStringLiteral( "layout_name" ) ) );
  QVERIFY( context.hasVariable( QStringLiteral( "atlas_totalfeatures" ) ) );
  QVERIFY( context.hasVariable( QStringLiteral( "layer_id" ) ) );
  QCOMPARE( context.fields().at( 1 ).name(), QStringLiteral( "labelx" ) );
  QVERIFY( context.hasFeature() );

  mAtlas->setCoverageLayer( nullptr );
}

QGSTEST_MAIN( TestQgsLayoutAtlas )
#include "testqgslayoutatlas.moc"
