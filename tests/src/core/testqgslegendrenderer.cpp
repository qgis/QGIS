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

#include "qgstest.h"
#include <QObject>

#include "qgsapplication.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsfontutils.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsmaplayerlegend.h"
#include "qgsproject.h"
#include "qgslegendrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrenderchecker.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsdiagramrenderer.h"
#include "diagram/qgspiediagram.h"

static QString _fileNameForTest( const QString &testName )
{
  return QDir::tempPath() + '/' + testName + ".png";
}

static void _setStandardTestFont( QgsLegendSettings &settings, const QString &style = QStringLiteral( "Roman" ) )
{
  QList< QgsLegendStyle::Style> styles;
  styles << QgsLegendStyle::Title
         << QgsLegendStyle::Group
         << QgsLegendStyle::Subgroup
         << QgsLegendStyle::SymbolLabel;
  Q_FOREACH ( QgsLegendStyle::Style st, styles )
  {
    QFont font( QgsFontUtils::getStandardTestFont( style ) );
    font.setPointSizeF( settings.style( st ).font().pointSizeF() );
    settings.rstyle( st ).setFont( font );
  }
}

static void _renderLegend( const QString &testName, QgsLayerTreeModel *legendModel, QgsLegendSettings &settings )
{
  settings.setTitle( QStringLiteral( "Legend" ) );
  QgsLegendRenderer legendRenderer( legendModel, settings );
  QSizeF size = legendRenderer.minimumSize();

  int dpi = 96;
  qreal dpmm = dpi / 25.4;
  QSize s( size.width() * dpmm, size.height() * dpmm );
  qDebug() << QStringLiteral( "testName:%1 size=%2x%3 dpmm=%4 s=%5x%6" ).arg( testName ).arg( size.width() ).arg( size.height() ).arg( dpmm ).arg( s.width() ).arg( s.height() );
  QImage img( s, QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::white );

  QPainter painter( &img );
  painter.scale( dpmm, dpmm );
  legendRenderer.drawLegend( &painter );
  painter.end();

  img.save( _fileNameForTest( testName ) );
}

static bool _verifyImage( const QString &testName, QString &report )
{
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "legend" ) );
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
    TestQgsLegendRenderer() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testModel();

    void testBasic();
    void testBigMarker();
    void testMapUnits();
    void testTallSymbol();
    void testLineSpacing();
    void testLongSymbolText();
    void testThreeColumns();
    void testFilterByMap();
    void testFilterByMapSameSymbol();
    void testColumns_data();
    void testColumns();
    void testRasterStroke();
    void testFilterByPolygon();
    void testFilterByExpression();
    void testDiagramAttributeLegend();
    void testDiagramSizeLegend();
    void testDataDefinedSizeCollapsed();
    void testTextOnSymbol();

  private:
    QgsLayerTree *mRoot = nullptr;
    QgsVectorLayer *mVL1 =  nullptr ; // line
    QgsVectorLayer *mVL2 =  nullptr ; // polygon
    QgsVectorLayer *mVL3 =  nullptr ; // point
    QgsRasterLayer *mRL = nullptr;
    QString mReport;
    bool _testLegendColumns( int itemCount, int columnCount, const QString &testName );
};


void TestQgsLegendRenderer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport += QLatin1String( "<h1>Legend Renderer Tests</h1>\n" );
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
  mVL1 = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "Line Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( mVL1 );

  QgsLineSymbol *sym1 = new QgsLineSymbol();
  sym1->setColor( Qt::magenta );
  mVL1->setRenderer( new QgsSingleSymbolRenderer( sym1 ) );

  mVL2 = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "Polygon Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( mVL2 );

  QgsFillSymbol *sym2 = new QgsFillSymbol();
  sym2->setColor( Qt::cyan );
  mVL2->setRenderer( new QgsSingleSymbolRenderer( sym2 ) );

  mVL3 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  {
    QgsVectorDataProvider *pr = mVL3->dataProvider();
    QList<QgsField> attrs;
    attrs << QgsField( QStringLiteral( "test_attr" ), QVariant::Int );
    pr->addAttributes( attrs );

    QgsFields fields;
    fields.append( attrs.back() );

    QList<QgsFeature> features;
    QgsFeature f1( fields, 1 );
    f1.setAttribute( 0, 1 );
    QgsGeometry f1G = QgsGeometry::fromPointXY( QgsPointXY( 1.0, 1.0 ) );
    f1.setGeometry( f1G );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 2 );
    QgsGeometry f2G = QgsGeometry::fromPointXY( QgsPointXY( 9.0, 1.0 ) );
    f2.setGeometry( f2G );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 3 );
    QgsGeometry f3G = QgsGeometry::fromPointXY( QgsPointXY( 5.0, 5.0 ) ) ;
    f3.setGeometry( f3G );
    features << f1 << f2 << f3;
    pr->addFeatures( features );
    mVL3->updateFields();
  }
  QgsProject::instance()->addMapLayer( mVL3 );

  static const char RASTER_ARRAY[] = { 1, 2, 2, 1 };
  QString rasterUri = QStringLiteral( "MEM:::DATAPOINTER=%1,PIXELS=2,LINES=2" ).arg( ( qulonglong ) RASTER_ARRAY );
  mRL = new QgsRasterLayer( rasterUri, QStringLiteral( "Raster Layer" ), QStringLiteral( "gdal" ) );
  QgsProject::instance()->addMapLayer( mRL );

  QgsCategoryList cats;
  QgsMarkerSymbol *sym3_1 = new QgsMarkerSymbol();
  sym3_1->setColor( Qt::red );
  cats << QgsRendererCategory( 1, sym3_1, QStringLiteral( "Red" ) );
  QgsMarkerSymbol *sym3_2 = new QgsMarkerSymbol();
  sym3_2->setColor( Qt::green );
  cats << QgsRendererCategory( 2, sym3_2, QStringLiteral( "Green" ) );
  QgsMarkerSymbol *sym3_3 = new QgsMarkerSymbol();
  sym3_3->setColor( Qt::blue );
  cats << QgsRendererCategory( 3, sym3_3, QStringLiteral( "Blue" ) );
  QgsCategorizedSymbolRenderer *r3 = new QgsCategorizedSymbolRenderer( QStringLiteral( "test_attr" ), cats );
  mVL3->setRenderer( r3 );

  mRoot = new QgsLayerTree();
  QgsLayerTreeGroup *grp1 = mRoot->addGroup( QStringLiteral( "Line + Polygon" ) );
  grp1->addLayer( mVL1 );
  grp1->addLayer( mVL2 );
  mRoot->addLayer( mVL3 );
  mRoot->addLayer( mRL );
}

void TestQgsLegendRenderer::cleanup()
{
  delete mRoot;
  mRoot = nullptr;

  QgsProject::instance()->removeAllMapLayers();
}


void TestQgsLegendRenderer::testModel()
{
  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeNode *nodeGroup0 = mRoot->children().at( 0 );
  QVERIFY( nodeGroup0 );
  QgsLayerTreeNode *nodeLayer0 = nodeGroup0->children().at( 0 );
  QVERIFY( QgsLayerTree::isLayer( nodeLayer0 ) );
  QModelIndex idx = legendModel.node2index( nodeLayer0 );
  QVERIFY( idx.isValid() );
  QgsLayerTreeLayer *nodeVL1 = QgsLayerTree::toLayer( nodeLayer0 );
  QVERIFY( nodeVL1 );

  QList<QgsLayerTreeModelLegendNode *> lstNodes = legendModel.layerLegendNodes( nodeVL1 );
  QVERIFY( lstNodes.count() == 1 );
  QCOMPARE( lstNodes[0]->data( Qt::DisplayRole ).toString(), QString( "Line Layer" ) );

  // set user text
  QgsMapLayerLegendUtils::setLegendNodeUserLabel( nodeVL1, 0, QStringLiteral( "Hurray" ) );

  legendModel.refreshLayerLegend( nodeVL1 );

  QList<QgsLayerTreeModelLegendNode *> lstNodes2 = legendModel.layerLegendNodes( nodeVL1 );
  QCOMPARE( lstNodes2[0]->data( Qt::DisplayRole ).toString(), QString( "Hurray" ) );

  // reset user text
  QgsMapLayerLegendUtils::setLegendNodeUserLabel( nodeVL1, 0, QString() );
}


void TestQgsLegendRenderer::testBasic()
{
  QString testName = QStringLiteral( "legend_basic" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testBigMarker()
{
  QString testName = QStringLiteral( "legend_big_marker" );

  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  //dynamic_cast<QgsCategorizedSymbolRenderer*>( mVL3->renderer() )->updateCategoryLabel( 2, "This is a long symbol label" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testMapUnits()
{
  QString testName = QStringLiteral( "legend_mapunits" );

  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( 100 );
  sym->setSizeUnit( QgsUnitTypes::RenderMapUnits );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  sym = new QgsMarkerSymbol();
  sym->setColor( Qt::green );
  sym->setSize( 300 );
  sym->setSizeUnit( QgsUnitTypes::RenderMapUnits );
  catRenderer->updateCategorySymbol( 1, sym );

  sym = new QgsMarkerSymbol();
  sym->setColor( Qt::blue );
  sym->setSize( 5 );
  sym->setSizeUnit( QgsUnitTypes::RenderMillimeters );
  catRenderer->updateCategorySymbol( 2, sym );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );
  root->addLayer( mVL3 );
  QgsLayerTreeModel legendModel( root.get() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  settings.setMmPerMapUnit( 0.1 );
  settings.setMapScale( 1000 );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testTallSymbol()
{
  QString testName = QStringLiteral( "legend_tall_symbol" );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, QStringLiteral( "This is\nthree lines\nlong label" ) );

  mVL2->setName( QStringLiteral( "This is a two lines\nlong label" ) );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "\n" ) );
  settings.setSymbolSize( QSizeF( 10.0, 10.0 ) );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  mVL2->setName( QStringLiteral( "Polygon Layer" ) );
}

void TestQgsLegendRenderer::testLineSpacing()
{
  QString testName = QStringLiteral( "legend_line_spacing" );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, QStringLiteral( "This is\nthree lines\nlong label" ) );

  mVL2->setName( QStringLiteral( "This is a two lines\nlong label" ) );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "\n" ) );
  settings.setLineSpacing( 3 );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  mVL2->setName( QStringLiteral( "Polygon Layer" ) );
}

void TestQgsLegendRenderer::testLongSymbolText()
{
  QString testName = QStringLiteral( "legend_long_symbol_text" );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, QStringLiteral( "This is\nthree lines\nlong label" ) );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "\n" ) );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testThreeColumns()
{
  QString testName = QStringLiteral( "legend_three_columns" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setColumnCount( 3 );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByMap()
{
  QString testName = QStringLiteral( "legend_filter_by_map" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QgsProject::instance()->mapLayers().values() );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByMapSameSymbol()
{
  QgsVectorLayer *vl4 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  {
    QgsVectorDataProvider *pr = vl4->dataProvider();
    QList<QgsField> attrs;
    attrs << QgsField( QStringLiteral( "test_attr" ), QVariant::Int );
    pr->addAttributes( attrs );

    QgsFields fields;
    fields.append( attrs.back() );

    QList<QgsFeature> features;
    QgsFeature f1( fields, 1 );
    f1.setAttribute( 0, 1 );
    QgsGeometry f1G = QgsGeometry::fromPointXY( QgsPointXY( 1.0, 1.0 ) );
    f1.setGeometry( f1G );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 2 );
    QgsGeometry f2G =  QgsGeometry::fromPointXY( QgsPointXY( 9.0, 1.0 ) );
    f2.setGeometry( f2G );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 3 );
    QgsGeometry f3G = QgsGeometry::fromPointXY( QgsPointXY( 5.0, 5.0 ) );
    f3.setGeometry( f3G );
    features << f1 << f2 << f3;
    pr->addFeatures( features );
    vl4->updateFields();
  }
  QgsProject::instance()->addMapLayer( vl4 );

  //setup categorized renderer with duplicate symbols
  QgsCategoryList cats;
  QgsMarkerSymbol *sym4_1 = new QgsMarkerSymbol();
  sym4_1->setColor( Qt::red );
  cats << QgsRendererCategory( 1, sym4_1, QStringLiteral( "Red1" ) );
  QgsMarkerSymbol *sym4_2 = new QgsMarkerSymbol();
  sym4_2->setColor( Qt::red );
  cats << QgsRendererCategory( 2, sym4_2, QStringLiteral( "Red2" ) );
  QgsMarkerSymbol *sym4_3 = new QgsMarkerSymbol();
  sym4_3->setColor( Qt::red );
  cats << QgsRendererCategory( 3, sym4_3, QStringLiteral( "Red3" ) );
  QgsCategorizedSymbolRenderer *r4 = new QgsCategorizedSymbolRenderer( QStringLiteral( "test_attr" ), cats );
  vl4->setRenderer( r4 );

  QString testName = QStringLiteral( "legend_filter_by_map_dupe" );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );
  root->addLayer( vl4 );
  QgsLayerTreeModel legendModel( root.get() );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl4 );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  QgsProject::instance()->removeMapLayer( vl4 );
}

bool TestQgsLegendRenderer::_testLegendColumns( int itemCount, int columnCount, const QString &testName )
{
  QgsFillSymbol *sym = new QgsFillSymbol();
  sym->setColor( Qt::cyan );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );

  QList< QgsVectorLayer * > layers;
  for ( int i = 1; i <= itemCount; ++i )
  {
    QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "Layer %1" ).arg( i ), QStringLiteral( "memory" ) );
    QgsProject::instance()->addMapLayer( vl );
    vl->setRenderer( new QgsSingleSymbolRenderer( sym->clone() ) );
    root->addLayer( vl );
    layers << vl;
  }
  delete sym;

  QgsLayerTreeModel legendModel( root.get() );
  QgsLegendSettings settings;
  settings.setColumnCount( columnCount );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  bool result = _verifyImage( testName, mReport );

  Q_FOREACH ( QgsVectorLayer *l, layers )
  {
    QgsProject::instance()->removeMapLayer( l );
  }
  return result;
}

void TestQgsLegendRenderer::testColumns_data()
{
  QTest::addColumn<QString>( "testName" );
  QTest::addColumn<int>( "items" );
  QTest::addColumn<int>( "columns" );

  QTest::newRow( "2 items, 2 columns" ) << "legend_2_by_2" << 2 << 2;
  QTest::newRow( "3 items, 2 columns" ) << "legend_3_by_2" << 3 << 2;
  QTest::newRow( "4 items, 2 columns" ) << "legend_4_by_2" << 4 << 2;
  QTest::newRow( "5 items, 2 columns" ) << "legend_5_by_2" << 5 << 2;
  QTest::newRow( "3 items, 3 columns" ) << "legend_3_by_3" << 3 << 3;
  QTest::newRow( "4 items, 3 columns" ) << "legend_4_by_3" << 4 << 3;
  QTest::newRow( "5 items, 3 columns" ) << "legend_5_by_3" << 5 << 3;
  QTest::newRow( "6 items, 3 columns" ) << "legend_6_by_3" << 6 << 3;
  QTest::newRow( "7 items, 3 columns" ) << "legend_7_by_3" << 7 << 3;
}

void TestQgsLegendRenderer::testColumns()
{
  //test rendering legend with different combinations of columns and items

  QFETCH( QString, testName );
  QFETCH( int, items );
  QFETCH( int, columns );
  QVERIFY( _testLegendColumns( items, columns, testName ) );
}

void TestQgsLegendRenderer::testRasterStroke()
{
  QString testName = QStringLiteral( "legend_raster_border" );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );
  root->addLayer( mRL );

  QgsLayerTreeModel legendModel( root.get() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  settings.setRasterStrokeWidth( 2 );
  settings.setRasterStrokeColor( Qt::green );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByPolygon()
{
  QString testName = QStringLiteral( "legend_filter_by_polygon" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QgsProject::instance()->mapLayers().values() );

  // select only within a polygon
  QgsGeometry geom( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((0 0,2 0,2 2,0 2,0 0))" ) ) );
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ false, geom );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  // again with useExtent to true
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ true, geom );

  QString testName2 = testName + "2";
  QString report2 = mReport + "2";
  _setStandardTestFont( settings );
  _renderLegend( testName2, &legendModel, settings );
  QVERIFY( _verifyImage( testName2, report2 ) );
}

void TestQgsLegendRenderer::testFilterByExpression()
{
  QString testName = QStringLiteral( "legend_filter_by_expression" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QgsProject::instance()->mapLayers().values() );

  // use an expression to only include the red point
  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3->id() );
  QVERIFY( layer );
  QgsLayerTreeUtils::setLegendFilterByExpression( *layer, QStringLiteral( "test_attr=1" ) );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  // test again with setLegendFilter and only expressions
  legendModel.setLegendFilterByMap( nullptr );
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ false );

  QString testName2 = testName + "2";
  QString report2 = mReport + "2";
  _setStandardTestFont( settings );
  _renderLegend( testName2, &legendModel, settings );
  QVERIFY( _verifyImage( testName2, report2 ) );
}

void TestQgsLegendRenderer::testDiagramAttributeLegend()
{
  QgsVectorLayer *vl4 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( vl4 );

  QgsDiagramSettings ds;
  ds.categoryColors = QList<QColor>() << QColor( 255, 0, 0 ) << QColor( 0, 255, 0 );
  ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"cat1\"" ) << QStringLiteral( "\"cat2\"" );
  ds.categoryLabels = QStringList() << QStringLiteral( "cat 1" ) << QStringLiteral( "cat 2" );

  QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
  dr->setLowerValue( 0.0 );
  dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
  dr->setUpperValue( 10 );
  dr->setUpperSize( QSizeF( 40, 40 ) );
  dr->setClassificationField( QString() );
  dr->setDiagram( new QgsPieDiagram() );
  dr->setDiagramSettings( ds );
  dr->setDataDefinedSizeLegend( nullptr );
  dr->setAttributeLegend( true );
  vl4->setDiagramRenderer( dr );

  QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
  dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
  dls.setShowAllDiagrams( true );
  vl4->setDiagramLayerSettings( dls );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );
  root->addLayer( vl4 );
  QgsLayerTreeModel legendModel( root.get() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_diagram_attributes" ), &legendModel, settings );
  QVERIFY( _verifyImage( "legend_diagram_attributes", mReport ) );

  QgsProject::instance()->removeMapLayer( vl4 );
}

void TestQgsLegendRenderer::testDiagramSizeLegend()
{
  QgsVectorLayer *vl4 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( vl4 );

  QgsDiagramSettings ds;
  ds.categoryColors = QList<QColor>() << QColor( 255, 0, 0 ) << QColor( 0, 255, 0 );
  ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"cat1\"" ) << QStringLiteral( "\"cat2\"" );
  ds.categoryLabels = QStringList() << QStringLiteral( "cat 1" ) << QStringLiteral( "cat 2" );
  ds.scaleByArea = false;

  QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
  dr->setLowerValue( 0.0 );
  dr->setLowerSize( QSizeF( 1, 1 ) );
  dr->setUpperValue( 10 );
  dr->setUpperSize( QSizeF( 20, 20 ) );
  dr->setClassificationField( QStringLiteral( "a" ) );
  dr->setDiagram( new QgsPieDiagram() );
  dr->setDiagramSettings( ds );
  dr->setDataDefinedSizeLegend( new QgsDataDefinedSizeLegend() );
  dr->setAttributeLegend( false );
  vl4->setDiagramRenderer( dr );

  QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
  dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
  dls.setShowAllDiagrams( true );
  vl4->setDiagramLayerSettings( dls );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );
  root->addLayer( vl4 );
  QgsLayerTreeModel legendModel( root.get() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_diagram_size" ), &legendModel, settings );
  QVERIFY( _verifyImage( "legend_diagram_size", mReport ) );

  QgsProject::instance()->removeMapLayer( vl4 );
}


void TestQgsLegendRenderer::testDataDefinedSizeCollapsed()
{
  QString testName = QStringLiteral( "legend_data_defined_size_collapsed" );

  QgsVectorLayer *vlDataDefinedSize = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  {
    QgsVectorDataProvider *pr = vlDataDefinedSize->dataProvider();
    QList<QgsField> attrs;
    attrs << QgsField( QStringLiteral( "test_attr" ), QVariant::Int );
    pr->addAttributes( attrs );

    QgsFields fields;
    fields.append( attrs.back() );

    QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( 1.0, 1.0 ) );

    QList<QgsFeature> features;
    QgsFeature f1( fields, 1 );
    f1.setAttribute( 0, 100 );
    f1.setGeometry( g );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 200 );
    f2.setGeometry( g );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 300 );
    f3.setGeometry( g );
    features << f1 << f2 << f3;
    pr->addFeatures( features );
    vlDataDefinedSize->updateFields();
  }

  QgsStringMap props;
  props[QStringLiteral( "name" )] = QStringLiteral( "circle" );
  props[QStringLiteral( "color" )] = QStringLiteral( "200,200,200" );
  props[QStringLiteral( "outline_color" )] = QStringLiteral( "0,0,0" );
  QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple( props );
  QgsProperty ddsProperty = QgsProperty::fromField( QStringLiteral( "test_attr" ) );
  ddsProperty.setTransformer( new QgsSizeScaleTransformer( QgsSizeScaleTransformer::Linear, 100, 300, 10, 30 ) );  // takes ownership
  symbol->setDataDefinedSize( ddsProperty );

  QgsDataDefinedSizeLegend *ddsLegend = new QgsDataDefinedSizeLegend();
  ddsLegend->setLegendType( QgsDataDefinedSizeLegend::LegendCollapsed );
  ddsLegend->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );

  QgsSingleSymbolRenderer *r = new QgsSingleSymbolRenderer( symbol );   // takes ownership
  r->setDataDefinedSizeLegend( ddsLegend );
  vlDataDefinedSize->setRenderer( r );

  QgsLayerTree *root = new QgsLayerTree();
  root->addLayer( vlDataDefinedSize );

  QgsLayerTreeModel legendModel( root );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  delete root;
}

void TestQgsLegendRenderer::testTextOnSymbol()
{
  QString testName = QStringLiteral( "legend_text_on_symbol" );

  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "Polygon Layer" ), QStringLiteral( "memory" ) );

  QgsCategoryList cats;
  QgsFillSymbol *sym_1 = new QgsFillSymbol();
  sym_1->setColor( Qt::red );
  cats << QgsRendererCategory( 1, sym_1, QStringLiteral( "Red" ) );
  QgsFillSymbol *sym_2 = new QgsFillSymbol();
  sym_2->setColor( Qt::green );
  cats << QgsRendererCategory( 2, sym_2, QStringLiteral( "Green" ) );
  QgsFillSymbol *sym_3 = new QgsFillSymbol();
  sym_3->setColor( Qt::blue );
  cats << QgsRendererCategory( 3, sym_3, QStringLiteral( "Blue" ) );
  QgsCategorizedSymbolRenderer *r = new QgsCategorizedSymbolRenderer( QStringLiteral( "test_attr" ), cats );
  vl->setRenderer( r );

  QgsDefaultVectorLayerLegend *legend = new QgsDefaultVectorLayerLegend( vl );
  legend->setTextOnSymbolEnabled( true );
  QHash<QString, QString> content;
  content["0"] = "Rd";
  content["2"] = "Bl";
  legend->setTextOnSymbolContent( content );
  QgsTextFormat textFormat;
  textFormat.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Roman" ) ) );
  textFormat.setSize( 9 );
  legend->setTextOnSymbolTextFormat( textFormat );
  vl->setLegend( legend );

  QgsLayerTree *root = new QgsLayerTree();
  root->addLayer( vl );

  QgsLayerTreeModel legendModel( root );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  delete root;
}


QGSTEST_MAIN( TestQgsLegendRenderer )
#include "testqgslegendrenderer.moc"
