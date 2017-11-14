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
#include "qgsmultirenderchecker.h"
#include "qgscomposermap.h"
#include "qgscomposermapoverview.h"
#include "qgsatlascomposition.h"
#include "qgscomposerlabel.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QtTest/QSignalSpy>
#include "qgstest.h"

class TestQgsAtlasComposition : public QObject
{
    Q_OBJECT

  public:
    TestQgsAtlasComposition() = default;

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

  private:
    QgsComposition *mComposition = nullptr;
    QgsComposerLabel *mLabel1 = nullptr;
    QgsComposerLabel *mLabel2 = nullptr;
    QgsComposerMap *mAtlasMap = nullptr;
    QgsComposerMap *mOverview = nullptr;
    QgsVectorLayer *mVectorLayer = nullptr;
    QgsVectorLayer *mVectorLayer2 = nullptr;
    QgsAtlasComposition *mAtlas = nullptr;
    QString mReport;
};

void TestQgsAtlasComposition::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );
  mVectorLayer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
                                      vectorFileInfo.completeBaseName(),
                                      QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mVectorLayer->setSimplifyMethod( simplifyMethod );

  mReport = QStringLiteral( "<h1>Composer Atlas Tests</h1>\n" );
}

void TestQgsAtlasComposition::cleanupTestCase()
{
  delete mComposition;
  delete mVectorLayer;
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

void TestQgsAtlasComposition::init()
{
  //create composition with composer map

  // select epsg:2154
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 2154 );
  QgsProject::instance()->setCrs( crs );
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  // fix the renderer, fill with green
  QgsStringMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "0,127,0" ) );
  QgsFillSymbol *fillSymbol = QgsFillSymbol::createSimple( props );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( fillSymbol );
  mVectorLayer->setRenderer( renderer );

  // the atlas map
  mAtlasMap = new QgsComposerMap( mComposition, 20, 20, 130, 130 );
  mAtlasMap->setFrameEnabled( true );
  mComposition->addComposerMap( mAtlasMap );
  mAtlasMap->setLayers( QList<QgsMapLayer *>() << mVectorLayer );

  mAtlas = &mComposition->atlasComposition();
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setEnabled( true );
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );

  // an overview
  mOverview = new QgsComposerMap( mComposition, 180, 20, 50, 50 );
  mOverview->setFrameEnabled( true );
  mOverview->overview()->setFrameMap( mAtlasMap->id() );
  mOverview->setLayers( QList<QgsMapLayer *>() << mVectorLayer );
  mComposition->addComposerMap( mOverview );
  mOverview->setNewExtent( QgsRectangle( 49670.718, 6415139.086, 699672.519, 7065140.887 ) );

  // set the fill symbol of the overview map
  QgsStringMap props2;
  props2.insert( QStringLiteral( "color" ), QStringLiteral( "127,0,0,127" ) );
  QgsFillSymbol *fillSymbol2 = QgsFillSymbol::createSimple( props2 );
  mOverview->overview()->setFrameSymbol( fillSymbol2 );

  // header label
  mLabel1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mLabel1 );
  mLabel1->setText( QStringLiteral( "[% \"NAME_1\" %] area" ) );
  mLabel1->setFont( QgsFontUtils::getStandardTestFont() );
  //need to explicitly set width, since expression hasn't been evaluated against
  //an atlas feature yet and will be shorter than required
  mLabel1->setSceneRect( QRectF( 150, 5, 60, 15 ) );

  // feature number label
  mLabel2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mLabel2 );
  mLabel2->setText( QStringLiteral( "# [%@atlas_featurenumber || ' / ' || @atlas_totalfeatures%]" ) );
  mLabel2->setFont( QgsFontUtils::getStandardTestFont() );
  mLabel2->setSceneRect( QRectF( 150, 200, 60, 15 ) );

  qDebug() << "header label font: " << mLabel1->font().toString() << " exactMatch:" << mLabel1->font().exactMatch();
  qDebug() << "feature number label font: " << mLabel2->font().toString() << " exactMatch:" << mLabel2->font().exactMatch();
}

void TestQgsAtlasComposition::cleanup()
{
  delete mComposition;
  mComposition = 0;
}

void TestQgsAtlasComposition::filename()
{
  mAtlas->setFilenamePattern( QStringLiteral( "'output_' || @atlas_featurenumber" ) );
  mAtlas->beginRender();
  for ( int fi = 0; fi < mAtlas->numFeatures(); ++fi )
  {
    mAtlas->prepareForFeature( fi );
    QString expected = QStringLiteral( "output_%1" ).arg( ( int )( fi + 1 ) );
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

    QgsCompositionChecker checker( QStringLiteral( "atlas_autoscale%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::fixedscale_render()
{
  //TODO QGIS3.0 - setting the extent AFTER setting atlas driven/fixed scaling mode should
  //also update the set fixed scale
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Fixed );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QStringLiteral( "atlas_fixedscale%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
}

void TestQgsAtlasComposition::predefinedscales_render()
{
  //TODO QGIS3.0 - setting the extent AFTER setting atlas driven/predefined scaling mode should
  //also update the atlas map scale
  mAtlasMap->setNewExtent( QgsRectangle( 209838.166, 6528781.020, 610491.166, 6920530.620 ) );
  mAtlasMap->setAtlasDriven( true );
  mAtlasMap->setAtlasScalingMode( QgsComposerMap::Predefined );

  QVector<qreal> scales;
  scales << 1800000.0;
  scales << 5000000.0;
  mAtlas->setPredefinedScales( scales );

  {
    const QVector<qreal> &setScales = mAtlas->predefinedScales();
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

    QgsCompositionChecker checker( QStringLiteral( "atlas_predefinedscales%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
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

    QgsCompositionChecker checker( QStringLiteral( "atlas_two_maps%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
    QVERIFY( checker.testComposition( mReport, 0, 100 ) );
  }
  mAtlas->endRender();
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

    QgsCompositionChecker checker( QStringLiteral( "atlas_hiding%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
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
  mAtlas->setSortKeyAttributeName( QStringLiteral( "NAME_1" ) ); // departement name
  mAtlas->setSortAscending( false );

  mAtlas->beginRender();

  for ( int fit = 0; fit < 2; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QStringLiteral( "atlas_sorting%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
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
  mAtlas->setFeatureFilter( QStringLiteral( "substr(NAME_1,1,1)='P'" ) ); // select only 'Pays de la Loire'

  mAtlas->beginRender();

  for ( int fit = 0; fit < 1; ++fit )
  {
    mAtlas->prepareForFeature( fit );
    mLabel1->adjustSizeToText();

    QgsCompositionChecker checker( QStringLiteral( "atlas_filtering%1" ).arg( ( ( int )fit ) + 1 ), mComposition );
    checker.setControlPathPrefix( QStringLiteral( "atlas" ) );
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
  QgsProject::instance()->addMapLayer( mVectorLayer2 );
  mAtlas->setCoverageLayer( mVectorLayer2 );
  mAtlas->setEnabled( true );

  QSignalSpy spyToggled( mAtlas, SIGNAL( toggled( bool ) ) );

  //remove coverage layer while atlas is enabled
  QgsProject::instance()->removeMapLayer( mVectorLayer2->id() );
  mVectorLayer2 = 0;

  QVERIFY( !mAtlas->enabled() );
  QVERIFY( spyToggled.count() == 1 );
}

QGSTEST_MAIN( TestQgsAtlasComposition )
#include "testqgsatlascomposition.moc"
