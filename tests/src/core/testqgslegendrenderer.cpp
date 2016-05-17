/***************************************************************************
    testqgslegendrenderer.cpp
    ---------------------
    begin                : July 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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

#include "qgsapplication.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgscomposerlegenditem.h"
#include "qgsfontutils.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendmodel.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgslegendrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrenderchecker.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsdiagramrendererv2.h"
#include "diagram/qgspiediagram.h"

static QString _fileNameForTest( const QString& testName )
{
  return QDir::tempPath() + '/' + testName + ".png";
}

static void _setStandardTestFont( QgsLegendSettings& settings, const QString& style = "Roman" )
{
  QList< QgsComposerLegendStyle::Style> styles;
  styles << QgsComposerLegendStyle::Title
  << QgsComposerLegendStyle::Group
  << QgsComposerLegendStyle::Subgroup
  << QgsComposerLegendStyle::SymbolLabel;
  Q_FOREACH ( QgsComposerLegendStyle::Style st, styles )
  {
    QFont font( QgsFontUtils::getStandardTestFont( style ) );
    font.setPointSizeF( settings.style( st ).font().pointSizeF() );
    settings.rstyle( st ).setFont( font );
  }
}

static void _renderLegend( const QString& testName, QgsLayerTreeModel* legendModel, const QgsLegendSettings& settings )
{
  QgsLegendRenderer legendRenderer( legendModel, settings );
  QSizeF size = legendRenderer.minimumSize();

  int dpi = 96;
  qreal dpmm = dpi / 25.4;
  QSize s( size.width() * dpmm, size.height() * dpmm );
  qDebug() << QString( "testName:%1 size=%2x%3 dpmm=%4 s=%5x%6" ).arg( testName ).arg( size.width() ).arg( size.height() ).arg( dpmm ).arg( s.width() ).arg( s.height() );
  QImage img( s, QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::white );

  QPainter painter( &img );
  painter.scale( dpmm, dpmm );
  legendRenderer.drawLegend( &painter );
  painter.end();

  img.save( _fileNameForTest( testName ) );
}

static bool _verifyImage( const QString& testName, QString &report )
{
  QgsRenderChecker checker;
  checker.setControlPathPrefix( "legend" );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( _fileNameForTest( testName ) );
  checker.setSizeTolerance( 3, 3 );
  bool equal = checker.compareImages( testName, 500 );
  report += checker.report();
  return equal;
}



class TestQgsLegendRenderer : public QObject
{
    Q_OBJECT

  public:
    TestQgsLegendRenderer()
        : mRoot( 0 )
        , mVL1( 0 )
        , mVL2( 0 )
        , mVL3( 0 )
        , mRL( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testModel();

    void testBasic();
    void testBigMarker();
    void testMapUnits();
    void testLongSymbolText();
    void testThreeColumns();
    void testFilterByMap();
    void testFilterByMapSameSymbol();
    void testRasterBorder();
    void testFilterByPolygon();
    void testFilterByExpression();
    void testDiagramAttributeLegend();
    void testDiagramSizeLegend();

  private:
    QgsLayerTreeGroup* mRoot;
    QgsVectorLayer* mVL1; // line
    QgsVectorLayer* mVL2; // polygon
    QgsVectorLayer* mVL3; // point
    QgsRasterLayer* mRL;
    QString mReport;
};


void TestQgsLegendRenderer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport += "<h1>Legend Renderer Tests</h1>\n";
}

void TestQgsLegendRenderer::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsLegendRenderer::init()
{
  mVL1 = new QgsVectorLayer( "LineString", "Line Layer", "memory" );
  QgsMapLayerRegistry::instance()->addMapLayer( mVL1 );

  QgsLineSymbolV2* sym1 = new QgsLineSymbolV2();
  sym1->setColor( Qt::magenta );
  mVL1->setRendererV2( new QgsSingleSymbolRendererV2( sym1 ) );

  mVL2 = new QgsVectorLayer( "Polygon", "Polygon Layer", "memory" );
  QgsMapLayerRegistry::instance()->addMapLayer( mVL2 );

  QgsFillSymbolV2* sym2 = new QgsFillSymbolV2();
  sym2->setColor( Qt::cyan );
  mVL2->setRendererV2( new QgsSingleSymbolRendererV2( sym2 ) );

  mVL3 = new QgsVectorLayer( "Point", "Point Layer", "memory" );
  {
    QgsVectorDataProvider* pr = mVL3->dataProvider();
    QList<QgsField> attrs;
    attrs << QgsField( "test_attr", QVariant::Int );
    pr->addAttributes( attrs );

    QgsFields fields;
    fields.append( attrs.back() );

    QList<QgsFeature> features;
    QgsFeature f1( fields, 1 );
    f1.setAttribute( 0, 1 );
    f1.setGeometry( QgsGeometry::fromPoint( QgsPoint( 1.0, 1.0 ) ) );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 2 );
    f2.setGeometry( QgsGeometry::fromPoint( QgsPoint( 9.0, 1.0 ) ) );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 3 );
    f3.setGeometry( QgsGeometry::fromPoint( QgsPoint( 5.0, 5.0 ) ) );
    features << f1 << f2 << f3;
    pr->addFeatures( features );
    mVL3->updateFields();
  }
  QgsMapLayerRegistry::instance()->addMapLayer( mVL3 );

  static char raster_array[] = { 1, 2, 2, 1 };
  QString rasterUri = QString( "MEM:::DATAPOINTER=%1,PIXELS=2,LINES=2" ).arg(( qulonglong ) raster_array );
  mRL = new QgsRasterLayer( rasterUri, QString( "Raster Layer" ), QString( "gdal" ) );
  QgsMapLayerRegistry::instance()->addMapLayer( mRL );

  QgsCategoryList cats;
  QgsMarkerSymbolV2* sym3_1 = new QgsMarkerSymbolV2();
  sym3_1->setColor( Qt::red );
  cats << QgsRendererCategoryV2( 1, sym3_1, "Red" );
  QgsMarkerSymbolV2* sym3_2 = new QgsMarkerSymbolV2();
  sym3_2->setColor( Qt::green );
  cats << QgsRendererCategoryV2( 2, sym3_2, "Green" );
  QgsMarkerSymbolV2* sym3_3 = new QgsMarkerSymbolV2();
  sym3_3->setColor( Qt::blue );
  cats << QgsRendererCategoryV2( 3, sym3_3, "Blue" );
  QgsCategorizedSymbolRendererV2* r3 = new QgsCategorizedSymbolRendererV2( "test_attr", cats );
  mVL3->setRendererV2( r3 );

  mRoot = new QgsLayerTreeGroup();
  QgsLayerTreeGroup* grp1 = mRoot->addGroup( "Line + Polygon" );
  grp1->addLayer( mVL1 );
  grp1->addLayer( mVL2 );
  mRoot->addLayer( mVL3 );
  mRoot->addLayer( mRL );
}

void TestQgsLegendRenderer::cleanup()
{
  delete mRoot;
  mRoot = 0;

  QgsMapLayerRegistry::instance()->removeAllMapLayers();
}


void TestQgsLegendRenderer::testModel()
{
  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeNode* nodeGroup0 = mRoot->children().at( 0 );
  QVERIFY( nodeGroup0 );
  QgsLayerTreeNode* nodeLayer0 = nodeGroup0->children().at( 0 );
  QVERIFY( QgsLayerTree::isLayer( nodeLayer0 ) );
  QModelIndex idx = legendModel.node2index( nodeLayer0 );
  QVERIFY( idx.isValid() );
  QgsLayerTreeLayer* nodeVL1 = QgsLayerTree::toLayer( nodeLayer0 );
  QVERIFY( nodeVL1 );

  QList<QgsLayerTreeModelLegendNode*> lstNodes = legendModel.layerLegendNodes( nodeVL1 );
  QVERIFY( lstNodes.count() == 1 );
  QCOMPARE( lstNodes[0]->data( Qt::DisplayRole ).toString(), QString( "Line Layer" ) );

  // set user text
  QgsMapLayerLegendUtils::setLegendNodeUserLabel( nodeVL1, 0, "Hurray" );

  legendModel.refreshLayerLegend( nodeVL1 );

  QList<QgsLayerTreeModelLegendNode*> lstNodes2 = legendModel.layerLegendNodes( nodeVL1 );
  QCOMPARE( lstNodes2[0]->data( Qt::DisplayRole ).toString(), QString( "Hurray" ) );

  // reset user text
  QgsMapLayerLegendUtils::setLegendNodeUserLabel( nodeVL1, 0, QString() );
}


void TestQgsLegendRenderer::testBasic()
{
  QString testName = "legend_basic";

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testBigMarker()
{
  QString testName = "legend_big_marker";

  QgsMarkerSymbolV2* sym = new QgsMarkerSymbolV2();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRendererV2* catRenderer = dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  //dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() )->updateCategoryLabel( 2, "This is a long symbol label" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testMapUnits()
{
  QString testName = "legend_mapunits";

  QgsMarkerSymbolV2* sym = new QgsMarkerSymbolV2();
  sym->setColor( Qt::red );
  sym->setSize( 100 );
  sym->setSizeUnit( QgsSymbolV2::MapUnit );
  QgsCategorizedSymbolRendererV2* catRenderer = dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  sym = new QgsMarkerSymbolV2();
  sym->setColor( Qt::green );
  sym->setSize( 300 );
  sym->setSizeUnit( QgsSymbolV2::MapUnit );
  catRenderer->updateCategorySymbol( 1, sym );

  sym = new QgsMarkerSymbolV2();
  sym->setColor( Qt::blue );
  sym->setSize( 5 );
  sym->setSizeUnit( QgsSymbolV2::MM );
  catRenderer->updateCategorySymbol( 2, sym );

  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  root->addLayer( mVL3 );
  QgsLayerTreeModel legendModel( root );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  settings.setMmPerMapUnit( 0.1 );
  settings.setMapScale( 1000 );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testLongSymbolText()
{
  QString testName = "legend_long_symbol_text";

  QgsCategorizedSymbolRendererV2* catRenderer = dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, "This is\nthree lines\nlong label" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( "\n" );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testThreeColumns()
{
  QString testName = "legend_three_columns";

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setColumnCount( 3 );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByMap()
{
  QString testName = "legend_filter_by_map";

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  QStringList ll;
  Q_FOREACH ( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    ll << l->id();
  }
  mapSettings.setLayers( ll );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByMapSameSymbol()
{
  QgsVectorLayer* vl4 = new QgsVectorLayer( "Point", "Point Layer", "memory" );
  {
    QgsVectorDataProvider* pr = vl4->dataProvider();
    QList<QgsField> attrs;
    attrs << QgsField( "test_attr", QVariant::Int );
    pr->addAttributes( attrs );

    QgsFields fields;
    fields.append( attrs.back() );

    QList<QgsFeature> features;
    QgsFeature f1( fields, 1 );
    f1.setAttribute( 0, 1 );
    f1.setGeometry( QgsGeometry::fromPoint( QgsPoint( 1.0, 1.0 ) ) );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 2 );
    f2.setGeometry( QgsGeometry::fromPoint( QgsPoint( 9.0, 1.0 ) ) );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 3 );
    f3.setGeometry( QgsGeometry::fromPoint( QgsPoint( 5.0, 5.0 ) ) );
    features << f1 << f2 << f3;
    pr->addFeatures( features );
    vl4->updateFields();
  }
  QgsMapLayerRegistry::instance()->addMapLayer( vl4 );

  //setup categorized renderer with duplicate symbols
  QgsCategoryList cats;
  QgsMarkerSymbolV2* sym4_1 = new QgsMarkerSymbolV2();
  sym4_1->setColor( Qt::red );
  cats << QgsRendererCategoryV2( 1, sym4_1, "Red1" );
  QgsMarkerSymbolV2* sym4_2 = new QgsMarkerSymbolV2();
  sym4_2->setColor( Qt::red );
  cats << QgsRendererCategoryV2( 2, sym4_2, "Red2" );
  QgsMarkerSymbolV2* sym4_3 = new QgsMarkerSymbolV2();
  sym4_3->setColor( Qt::red );
  cats << QgsRendererCategoryV2( 3, sym4_3, "Red3" );
  QgsCategorizedSymbolRendererV2* r4 = new QgsCategorizedSymbolRendererV2( "test_attr", cats );
  vl4->setRendererV2( r4 );

  QString testName = "legend_filter_by_map_dupe";

  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  root->addLayer( vl4 );
  QgsLayerTreeModel legendModel( root );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QStringList() << vl4->id() );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, "Bold" );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  QgsMapLayerRegistry::instance()->removeMapLayer( vl4 );
}

void TestQgsLegendRenderer::testRasterBorder()
{
  QString testName = "legend_raster_border";

  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  root->addLayer( mRL );

  QgsLayerTreeModel legendModel( root );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  settings.setRasterBorderWidth( 2 );
  settings.setRasterBorderColor( Qt::green );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByPolygon()
{
  QString testName = "legend_filter_by_polygon";

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  QStringList ll;
  Q_FOREACH ( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    ll << l->id();
  }
  mapSettings.setLayers( ll );

  // select only within a polygon
  QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( "POLYGON((0 0,2 0,2 2,0 2,0 0))" ) );
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ false, *geom.data() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  // again with useExtent to true
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ true, *geom.data() );

  QString testName2 = testName + "2";
  QString report2 = mReport + "2";
  _setStandardTestFont( settings );
  _renderLegend( testName2, &legendModel, settings );
  QVERIFY( _verifyImage( testName2, report2 ) );
}

void TestQgsLegendRenderer::testFilterByExpression()
{
  QString testName = "legend_filter_by_expression";

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  QStringList ll;
  Q_FOREACH ( QgsMapLayer *l, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    ll << l->id();
  }
  mapSettings.setLayers( ll );

  // use an expression to only include the red point
  QgsLayerTreeLayer* layer = legendModel.rootGroup()->findLayer( mVL3->id() );
  QVERIFY( layer );
  QgsLayerTreeUtils::setLegendFilterByExpression( *layer, "test_attr=1" );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  // test again with setLegendFilter and only expressions
  legendModel.setLegendFilterByMap( 0 );
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ false );

  QString testName2 = testName + "2";
  QString report2 = mReport + "2";
  _setStandardTestFont( settings );
  _renderLegend( testName2, &legendModel, settings );
  QVERIFY( _verifyImage( testName2, report2 ) );
}

void TestQgsLegendRenderer::testDiagramAttributeLegend()
{
  QgsVectorLayer* vl4 = new QgsVectorLayer( "Point", "Point Layer", "memory" );
  QgsMapLayerRegistry::instance()->addMapLayer( vl4 );

  QgsDiagramSettings ds;
  ds.categoryColors = QList<QColor>() << QColor( 255, 0, 0 ) << QColor( 0, 255, 0 );
  ds.categoryAttributes = QList<QString>() << "\"cat1\"" << "\"cat2\"";
  ds.categoryLabels = QStringList() << "cat 1" << "cat 2";

  QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
  dr->setLowerValue( 0.0 );
  dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
  dr->setUpperValue( 10 );
  dr->setUpperSize( QSizeF( 40, 40 ) );
  dr->setClassificationAttribute( 0 );
  dr->setDiagram( new QgsPieDiagram() );
  dr->setDiagramSettings( ds );
  dr->setSizeLegend( false );
  dr->setAttributeLegend( true );
  vl4->setDiagramRenderer( dr );

  QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
  dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
  dls.setShowAllDiagrams( true );
  vl4->setDiagramLayerSettings( dls );

  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  root->addLayer( vl4 );
  QgsLayerTreeModel legendModel( root );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, "Bold" );
  _renderLegend( "legend_diagram_attributes", &legendModel, settings );
  QVERIFY( _verifyImage( "legend_diagram_attributes", mReport ) );

  QgsMapLayerRegistry::instance()->removeMapLayer( vl4 );
}

void TestQgsLegendRenderer::testDiagramSizeLegend()
{
  QgsVectorLayer* vl4 = new QgsVectorLayer( "Point", "Point Layer", "memory" );
  QgsMapLayerRegistry::instance()->addMapLayer( vl4 );

  QgsDiagramSettings ds;
  ds.categoryColors = QList<QColor>() << QColor( 255, 0, 0 ) << QColor( 0, 255, 0 );
  ds.categoryAttributes = QList<QString>() << "\"cat1\"" << "\"cat2\"";
  ds.categoryLabels = QStringList() << "cat 1" << "cat 2";
  ds.scaleByArea = false;

  QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
  dr->setLowerValue( 0.0 );
  dr->setLowerSize( QSizeF( 1, 1 ) );
  dr->setUpperValue( 10 );
  dr->setUpperSize( QSizeF( 20, 20 ) );
  dr->setClassificationAttribute( 0 );
  dr->setDiagram( new QgsPieDiagram() );
  dr->setDiagramSettings( ds );
  dr->setSizeLegend( true );
  dr->setAttributeLegend( false );
  vl4->setDiagramRenderer( dr );

  QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
  dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
  dls.setShowAllDiagrams( true );
  vl4->setDiagramLayerSettings( dls );

  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  root->addLayer( vl4 );
  QgsLayerTreeModel legendModel( root );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, "Bold" );
  _renderLegend( "legend_diagram_size", &legendModel, settings );
  QVERIFY( _verifyImage( "legend_diagram_size", mReport ) );

  QgsMapLayerRegistry::instance()->removeMapLayer( vl4 );
}

QTEST_MAIN( TestQgsLegendRenderer )
#include "testqgslegendrenderer.moc"
