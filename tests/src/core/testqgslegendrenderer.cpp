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
#include <QJsonArray>

#include "qgsapplication.h"
#include "qgsblureffect.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgseffectstack.h"
#include "qgsfillsymbollayer.h"
#include "qgsfontutils.h"
#include "qgsgloweffect.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslinesymbollayer.h"
#include "qgsmaplayerlegend.h"
#include "qgspainteffect.h"
#include "qgsproject.h"
#include "qgslegendrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrenderchecker.h"
#include "qgsshadoweffect.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsdiagramrenderer.h"
#include "qgspalettedrasterrenderer.h"
#include "diagram/qgspiediagram.h"
#include "qgspropertytransformer.h"
#include "qgsrulebasedlabeling.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

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
  for ( const QgsLegendStyle::Style st : styles )
  {
    QFont font( QgsFontUtils::getStandardTestFont( style ) );
    font.setPointSizeF( settings.style( st ).font().pointSizeF() );
    settings.rstyle( st ).setFont( font );
  }
}

static QImage _base64ToImage( const QString &base64 )
{
  const QByteArray bytearray = QByteArray::fromBase64( base64.toStdString().c_str() );
  return QImage::fromData( bytearray, "PNG" );
}

static void _renderLegend( const QString &testName, QgsLayerTreeModel *legendModel, QgsLegendSettings &settings )
{
  settings.setTitle( QStringLiteral( "Legend" ) );
  QgsLegendRenderer legendRenderer( legendModel, settings );
  const QSizeF size = legendRenderer.minimumSize();

  const int dpi = 96;
  const qreal dpmm = dpi / 25.4;
  const QSize s( size.width() * dpmm, size.height() * dpmm );
  qDebug() << QStringLiteral( "testName:%1 size=%2x%3 dpmm=%4 s=%5x%6" ).arg( testName ).arg( size.width() ).arg( size.height() ).arg( dpmm ).arg( s.width() ).arg( s.height() );
  QImage img( s, QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::white );

  QPainter painter( &img );
  painter.setRenderHint( QPainter::Antialiasing, true );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );

  const QgsScopedRenderContextScaleToMm scaleToMm( context );
  context.setRendererScale( 1000 );
  context.setMapToPixel( QgsMapToPixel( 1 / ( 0.1 * context.scaleFactor() ) ) );

  legendRenderer.drawLegend( context );
  painter.end();

  img.save( _fileNameForTest( testName ) );
}

static QJsonObject _renderJsonLegend( QgsLayerTreeModel *legendModel, const QgsLegendSettings &settings )
{
  QgsLegendRenderer legendRenderer( legendModel, settings );

  QgsRenderContext context;
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  return legendRenderer.exportLegendToJson( context );
}

static bool _verifyImage( const QString &testName, QString &report, int diff = 30 )
{
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "legend" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( _fileNameForTest( testName ) );
  checker.setSizeTolerance( 3, 3 );
  const bool equal = checker.compareImages( testName, diff );
  report += checker.report();
  return equal;
}

class TestRasterRenderer : public QgsPalettedRasterRenderer
{
  public:

    TestRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes )
      : QgsPalettedRasterRenderer( input, bandNumber, classes )
    {}

    // don't create the default legend nodes for this layer!
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) override
    {
      QList<QgsLayerTreeModelLegendNode *> res;

      const QList< QPair< QString, QColor > > items = legendSymbologyItems();
      res.reserve( res.size() + items.size() );
      for ( const QPair< QString, QColor > &item : items )
      {
        res << new QgsRasterSymbolLegendNode( nodeLayer, item.second, item.first );
      }

      return res;
    }

};

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
    void testMultiline();
    void testOverrideSize();
    void testSpacing();
    void testEffects();
    void testBigMarker();
    void testBigMarkerMaxSize();
    void testOverrideSymbol();

    void testRightAlignText();
    void testCenterAlignText();
    void testLeftAlignTextRightAlignSymbol();
    void testCenterAlignTextRightAlignSymbol();
    void testRightAlignTextRightAlignSymbol();

    void testGroupHeadingSpacing();
    void testGroupIndent();

    void testMapUnits();
    void testTallSymbol();
    void testLineSpacing();
    void testLongSymbolText();
    void testThreeColumns();
    void testFilterByMap();
    void testFilterByMapSameSymbol();
    void testColumns_data();
    void testColumns();
    void testColumnBreaks();
    void testColumnBreaks2();
    void testColumnBreaks3();
    void testColumnBreaks4();
    void testColumnBreaks5();
    void testLayerColumnSplittingAlwaysAllow();
    void testLayerColumnSplittingAlwaysPrevent();
    void testRasterStroke();
    void testFilterByPolygon();
    void testFilterByExpression();
    void testDiagramAttributeLegend();
    void testDiagramSizeLegend();
    void testDataDefinedSizeCollapsed();
    void testTextOnSymbol();

    void testBasicJson();
    void testOpacityJson();
    void testBigMarkerJson();

    void testLabelLegend();

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
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
    const QgsGeometry f1G = QgsGeometry::fromPointXY( QgsPointXY( 1.0, 1.0 ) );
    f1.setGeometry( f1G );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 2 );
    const QgsGeometry f2G = QgsGeometry::fromPointXY( QgsPointXY( 9.0, 1.0 ) );
    f2.setGeometry( f2G );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 3 );
    const QgsGeometry f3G = QgsGeometry::fromPointXY( QgsPointXY( 5.0, 5.0 ) ) ;
    f3.setGeometry( f3G );
    features << f1 << f2 << f3;
    pr->addFeatures( features );
    mVL3->updateFields();
  }
  QgsProject::instance()->addMapLayer( mVL3 );

  static const char RASTER_ARRAY[] = { 1, 2, 2, 1 };
  const QString rasterUri = QStringLiteral( "MEM:::DATAPOINTER=%1,PIXELS=2,LINES=2" ).arg( ( qulonglong ) RASTER_ARRAY );
  mRL = new QgsRasterLayer( rasterUri, QStringLiteral( "Raster Layer" ), QStringLiteral( "gdal" ) );

  std::unique_ptr< TestRasterRenderer > rasterRenderer( new  TestRasterRenderer( mRL->dataProvider(), 1,
  {
    QgsPalettedRasterRenderer::Class( 1, QColor( 0, 0, 0 ), QStringLiteral( "1" ) ),
    QgsPalettedRasterRenderer::Class( 2, QColor( 255, 255, 255 ), QStringLiteral( "2" ) )
  } ) );
  mRL->setRenderer( rasterRenderer.release() );

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
  const QModelIndex idx = legendModel.node2index( nodeLayer0 );
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
  const QString testName = QStringLiteral( "legend_basic" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testMultiline()
{
  const QString testName = QStringLiteral( "legend_multiline" );

  QgsLayerTreeModel legendModel( mRoot );

  legendModel.findLegendNode( mVL1->id(), QString() );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL1 );
  layer->setCustomProperty( QStringLiteral( "legend/title-label" ), QStringLiteral( "some legend text\nwith newline\ncharacters in it" ) );

  QgsLayerTreeModelLegendNode *embeddedNode = legendModel.legendNodeEmbeddedInParent( layer );
  embeddedNode->setUserLabel( QString() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testOverrideSize()
{
  const QString testName = QStringLiteral( "legend_override_size" );

  QgsLayerTreeModel legendModel( mRoot );

  legendModel.findLegendNode( mVL1->id(), QString() );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL1 );
  layer->setPatchSize( QSizeF( 30, 0 ) );

  QgsLayerTreeModelLegendNode *embeddedNode = legendModel.legendNodeEmbeddedInParent( layer );
  embeddedNode->setUserLabel( QString() );

  layer = legendModel.rootGroup()->findLayer( mVL3 );
  QgsMapLayerLegendUtils::setLegendNodeSymbolSize( layer, 1, QSizeF( 0, 30 ) );
  legendModel.refreshLayerLegend( layer );

  layer = legendModel.rootGroup()->findLayer( mRL );
  QgsMapLayerLegendUtils::setLegendNodeSymbolSize( layer, 0, QSizeF( 50, 30 ) );
  legendModel.refreshLayerLegend( layer );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testSpacing()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;

  settings.rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Left, 7 );
  settings.rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Left, 11 );
  settings.rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Left, 5 );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );

  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignLeft );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignLeft );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignLeft );

  _renderLegend( QStringLiteral( "legend_left_align_side_space" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_left_align_side_space" ), mReport ) );

  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignRight );
  settings.setSymbolAlignment( Qt::AlignRight );

  _renderLegend( QStringLiteral( "legend_right_align_side_space" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_right_align_side_space" ), mReport ) );
}

void TestQgsLegendRenderer::testEffects()
{
  const QString testName = QStringLiteral( "legend_effects" );

  QgsEffectStack *effect = new QgsEffectStack();
  QgsSingleSymbolRenderer *renderer;
  QgsSymbol *symbol;

  renderer = dynamic_cast<QgsSingleSymbolRenderer *>( mVL1->renderer() );
  QVERIFY( renderer );
  symbol = renderer->symbol();
  QgsSimpleLineSymbolLayer *lineLayer = dynamic_cast<QgsSimpleLineSymbolLayer *>( symbol->symbolLayer( 0 ) );
  QVERIFY( lineLayer );
  lineLayer->setWidth( 1.8 );
  lineLayer->setColor( Qt::cyan );
  effect = new QgsEffectStack();
  effect->appendEffect( new QgsDropShadowEffect() );
  effect->appendEffect( new QgsDrawSourceEffect() );
  lineLayer->setPaintEffect( effect );

  renderer = dynamic_cast<QgsSingleSymbolRenderer *>( mVL2->renderer() );
  symbol = renderer->symbol();
  QVERIFY( renderer );
  QgsSimpleFillSymbolLayer *fillLayer = dynamic_cast<QgsSimpleFillSymbolLayer *>( symbol->takeSymbolLayer( 0 ) );
  QVERIFY( fillLayer );
  fillLayer->setColor( Qt::blue );
  effect = new QgsEffectStack();
  effect->appendEffect( new QgsDrawSourceEffect() );
  effect->appendEffect( new QgsInnerGlowEffect() );
  fillLayer->setPaintEffect( effect );

  lineLayer = new QgsSimpleLineSymbolLayer();
  lineLayer->setColor( Qt::cyan );
  lineLayer->setWidth( 1.8 );
  effect = new QgsEffectStack();
  effect->appendEffect( new QgsDropShadowEffect() );
  effect->appendEffect( new QgsDrawSourceEffect() );
  lineLayer->setPaintEffect( effect );

  symbol->appendSymbolLayer( lineLayer );
  symbol->appendSymbolLayer( fillLayer );

  symbol = new QgsMarkerSymbol();
  symbol->setColor( Qt::black );
  effect = new QgsEffectStack();
  effect->appendEffect( new QgsDropShadowEffect() );
  effect->appendEffect( new QgsDrawSourceEffect() );
  symbol->symbolLayer( 0 )->setPaintEffect( effect );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, symbol );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testBigMarker()
{
  const QString testName = QStringLiteral( "legend_big_marker" );

  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  //dynamic_cast<QgsCategorizedSymbolRenderer*>( mVL3->renderer() )->updateCategoryLabel( 2, "This is a long symbol label" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testBigMarkerMaxSize()
{
  const QString testName = QStringLiteral( "legend_big_marker_max_size" );
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.setMaximumSymbolSize( 5 ); //restrict maximum size to 5 mm
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testOverrideSymbol()
{
  const QString testName = QStringLiteral( "legend_override_symbol" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL2 );

  std::unique_ptr< QgsFillSymbol > sym2 = std::make_unique< QgsFillSymbol >();
  sym2->setColor( Qt::red );

  QgsLayerTreeModelLegendNode *embeddedNode = legendModel.legendNodeEmbeddedInParent( layer );
  qgis::down_cast< QgsSymbolLegendNode * >( embeddedNode )->setCustomSymbol( sym2.release() );

  std::unique_ptr< QgsMarkerSymbol > sym3 = std::make_unique< QgsMarkerSymbol >();
  sym3->setColor( QColor( 0, 150, 0 ) );
  sym3->setSize( 6 );

  layer = legendModel.rootGroup()->findLayer( mVL3 );
  QgsMapLayerLegendUtils::setLegendNodeCustomSymbol( layer, 1, sym3.get() );
  legendModel.refreshLayerLegend( layer );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testCenterAlignText()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignHCenter );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignHCenter );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignHCenter );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_center_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_center_align_text" ), mReport ) );

  settings.setColumnCount( 2 );
  _renderLegend( QStringLiteral( "legend_two_cols_center_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_two_cols_center_align_text" ), mReport ) );
}

void TestQgsLegendRenderer::testLeftAlignTextRightAlignSymbol()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignLeft );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignLeft );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignLeft );
  settings.setSymbolAlignment( Qt::AlignRight );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_right_symbol_left_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_right_symbol_left_align_text" ), mReport ) );

  settings.setColumnCount( 2 );
  _renderLegend( QStringLiteral( "legend_two_cols_right_align_symbol_left_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_two_cols_right_align_symbol_left_align_text" ), mReport ) );
}

void TestQgsLegendRenderer::testCenterAlignTextRightAlignSymbol()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignHCenter );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignHCenter );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignHCenter );
  settings.setSymbolAlignment( Qt::AlignRight );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_right_symbol_center_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_right_symbol_center_align_text" ), mReport ) );

  settings.setColumnCount( 2 );
  _renderLegend( QStringLiteral( "legend_two_cols_right_align_symbol_center_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_two_cols_right_align_symbol_center_align_text" ), mReport ) );
}

void TestQgsLegendRenderer::testRightAlignTextRightAlignSymbol()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignRight );
  settings.setSymbolAlignment( Qt::AlignRight );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_right_symbol_right_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_right_symbol_right_align_text" ), mReport ) );

  settings.setColumnCount( 2 );
  _renderLegend( QStringLiteral( "legend_two_cols_right_align_symbol_right_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_two_cols_right_align_symbol_right_align_text" ), mReport ) );
}

void TestQgsLegendRenderer::testGroupHeadingSpacing()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, 5 );
  settings.rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Bottom, 17 );
  settings.rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, 13 );
  settings.rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Bottom, 9 );
  settings.setSymbolAlignment( Qt::AlignRight );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_group_heading_spacing" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_group_heading_spacing" ), mReport ) );

}

void TestQgsLegendRenderer::testGroupIndent()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;

  QgsLayerTreeGroup *grp2 = mRoot->addGroup( QStringLiteral( "Subgroup" ) );
  sym->setSize( sym->size() / 6 );
  grp2->setCustomProperty( QStringLiteral( "legend/title-style" ), QLatin1String( "subgroup" ) );
  for ( int i = 1; i <= 4; ++i )
  {
    QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "Layer %1" ).arg( i ), QStringLiteral( "memory" ) );
    QgsProject::instance()->addMapLayer( vl );
    vl->setRenderer( new QgsSingleSymbolRenderer( sym->clone() ) );
    grp2->addLayer( vl );
  }

  settings.rstyle( QgsLegendStyle::Group ).setIndent( 10 );
  settings.rstyle( QgsLegendStyle::Subgroup ).setIndent( 5 );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_group_indent" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_group_indent" ), mReport ) );

  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignRight );
  _renderLegend( QStringLiteral( "legend_group_indent_right_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_group_indent_right_align_text" ), mReport ) );

  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignLeft );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignLeft );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignLeft );
  settings.setSymbolAlignment( Qt::AlignRight );
  _renderLegend( QStringLiteral( "legend_group_indent_right_align_symbol" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_group_indent_right_align_symbol" ), mReport ) );

  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignRight );
  settings.setSymbolAlignment( Qt::AlignRight );
  _renderLegend( QStringLiteral( "legend_group_indent_right_align_symbol_right_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_group_indent_right_align_symbol_right_align_text" ), mReport ) );
}

void TestQgsLegendRenderer::testRightAlignText()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;
  settings.rstyle( QgsLegendStyle::Group ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::Subgroup ).setAlignment( Qt::AlignRight );
  settings.rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( Qt::AlignRight );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( QStringLiteral( "legend_right_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_right_align_text" ), mReport ) );

  settings.setColumnCount( 2 );
  _renderLegend( QStringLiteral( "legend_two_cols_right_align_text" ), &legendModel, settings );
  QVERIFY( _verifyImage( QStringLiteral( "legend_two_cols_right_align_text" ), mReport ) );
}

void TestQgsLegendRenderer::testMapUnits()
{
  const QString testName = QStringLiteral( "legend_mapunits" );

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
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );

  Q_NOWARN_DEPRECATED_PUSH
  // TODO QGIS 4.0 -- move these to parameters on _renderLegend, and set the render context to match
  settings.setMmPerMapUnit( 0.1 );
  settings.setMapScale( 1000 );
  Q_NOWARN_DEPRECATED_POP

  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testTallSymbol()
{
  const QString testName = QStringLiteral( "legend_tall_symbol" );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, QStringLiteral( "This is\nthree lines\nlong label" ) );

  mVL2->setName( QStringLiteral( "This is a two lines\nlong label" ) );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "\n" ) );
  settings.setSymbolSize( QSizeF( 10.0, 10.0 ) );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  mVL2->setName( QStringLiteral( "Polygon Layer" ) );
}

void TestQgsLegendRenderer::testLineSpacing()
{
  const QString testName = QStringLiteral( "legend_line_spacing" );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, QStringLiteral( "This is\nthree lines\nlong label" ) );

  mVL2->setName( QStringLiteral( "This is a two lines\nlong label" ) );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "\n" ) );
  settings.setLineSpacing( 3 );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  mVL2->setName( QStringLiteral( "Polygon Layer" ) );
}

void TestQgsLegendRenderer::testLongSymbolText()
{
  const QString testName = QStringLiteral( "legend_long_symbol_text" );

  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategoryLabel( 1, QStringLiteral( "This is\nthree lines\nlong label" ) );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "\n" ) );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testThreeColumns()
{
  const QString testName = QStringLiteral( "legend_three_columns" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setColumnCount( 3 );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByMap()
{
  const QString testName = QStringLiteral( "legend_filter_by_map" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QgsProject::instance()->mapLayers().values() );

  legendModel.setLegendFilterByMap( &mapSettings );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
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
    const QgsGeometry f1G = QgsGeometry::fromPointXY( QgsPointXY( 1.0, 1.0 ) );
    f1.setGeometry( f1G );
    QgsFeature f2( fields, 2 );
    f2.setAttribute( 0, 2 );
    const QgsGeometry f2G =  QgsGeometry::fromPointXY( QgsPointXY( 9.0, 1.0 ) );
    f2.setGeometry( f2G );
    QgsFeature f3( fields, 3 );
    f3.setAttribute( 0, 3 );
    const QgsGeometry f3G = QgsGeometry::fromPointXY( QgsPointXY( 5.0, 5.0 ) );
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

  const QString testName = QStringLiteral( "legend_filter_by_map_dupe" );

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
  const bool result = _verifyImage( testName, mReport );

  for ( QgsVectorLayer *l : layers )
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
  QTest::newRow( "27 items, 3 columns" ) << "legend_27_by_3" << 27 << 3;
  QTest::newRow( "27 items, 9 columns" ) << "legend_27_by_9" << 27 << 9;
}

void TestQgsLegendRenderer::testColumns()
{
  //test rendering legend with different combinations of columns and items

  QFETCH( QString, testName );
  QFETCH( int, items );
  QFETCH( int, columns );
  QVERIFY( _testLegendColumns( items, columns, testName ) );
}

void TestQgsLegendRenderer::testColumnBreaks()
{
  const QString testName = QStringLiteral( "legend_column_breaks" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL2 );
  layer->setCustomProperty( QStringLiteral( "legend/column-break" ), true );

  layer = legendModel.rootGroup()->findLayer( mVL3 );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 1, true );
  legendModel.refreshLayerLegend( layer );

  layer = legendModel.rootGroup()->findLayer( mRL );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 1, true );
  legendModel.refreshLayerLegend( layer );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testColumnBreaks2()
{
  const QString testName = QStringLiteral( "legend_column_breaks2" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3 );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 0, true );
  legendModel.refreshLayerLegend( layer );

  layer = legendModel.rootGroup()->findLayer( mRL );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 0, true );
  legendModel.refreshLayerLegend( layer );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testColumnBreaks3()
{
  const QString testName = QStringLiteral( "legend_column_breaks3" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3 );
  layer->setCustomProperty( QStringLiteral( "legend/column-break" ), true );

  layer = legendModel.rootGroup()->findLayer( mRL );
  layer->setCustomProperty( QStringLiteral( "legend/column-break" ), true );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testColumnBreaks4()
{
  const QString testName = QStringLiteral( "legend_column_breaks4" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3 );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 0, true );
  legendModel.refreshLayerLegend( layer );

  layer = legendModel.rootGroup()->findLayer( mRL );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 0, true );
  legendModel.refreshLayerLegend( layer );

  QgsLegendSettings settings;
  settings.setColumnCount( 5 );
  settings.setSplitLayer( true );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testColumnBreaks5()
{
  const QString testName = QStringLiteral( "legend_column_breaks5" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3 );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 0, true );
  legendModel.refreshLayerLegend( layer );

  layer = legendModel.rootGroup()->findLayer( mRL );
  QgsMapLayerLegendUtils::setLegendNodeColumnBreak( layer, 0, true );
  legendModel.refreshLayerLegend( layer );

  QgsLegendSettings settings;
  settings.setColumnCount( 4 );
  settings.setSplitLayer( false );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testLayerColumnSplittingAlwaysAllow()
{
  const QString testName = QStringLiteral( "legend_layer_column_splitting_allow" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3 );
  layer->setLegendSplitBehavior( QgsLayerTreeLayer::AllowSplittingLegendNodesOverMultipleColumns );

  QgsLegendSettings settings;
  settings.setColumnCount( 4 );
  settings.setSplitLayer( false );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testLayerColumnSplittingAlwaysPrevent()
{
  const QString testName = QStringLiteral( "legend_layer_column_splitting_prevent" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLayerTreeLayer *layer = legendModel.rootGroup()->findLayer( mVL3 );
  layer->setLegendSplitBehavior( QgsLayerTreeLayer::PreventSplittingLegendNodesOverMultipleColumns );

  QgsLegendSettings settings;
  settings.setColumnCount( 4 );
  settings.setSplitLayer( true );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testRasterStroke()
{
  const QString testName = QStringLiteral( "legend_raster_border" );

  std::unique_ptr< QgsLayerTree > root( new QgsLayerTree() );
  root->addLayer( mRL );

  QgsLayerTreeModel legendModel( root.get() );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  settings.setRasterStrokeWidth( 2 );
  settings.setRasterStrokeColor( Qt::green );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );
}

void TestQgsLegendRenderer::testFilterByPolygon()
{
  const QString testName = QStringLiteral( "legend_filter_by_polygon" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsMapSettings mapSettings;
  // extent and size to include only the red and green points
  mapSettings.setExtent( QgsRectangle( 0, 0, 10.0, 4.0 ) );
  mapSettings.setOutputSize( QSize( 400, 100 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setLayers( QgsProject::instance()->mapLayers().values() );

  // select only within a polygon
  const QgsGeometry geom( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((0 0,2 0,2 2,0 2,0 0))" ) ) );
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ false, geom );

  QgsLegendSettings settings;
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  // again with useExtent to true
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ true, geom );

  const QString testName2 = testName + "2";
  QString report2 = mReport + "2";
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName2, &legendModel, settings );
  QVERIFY( _verifyImage( testName2, report2 ) );
}

void TestQgsLegendRenderer::testFilterByExpression()
{
  const QString testName = QStringLiteral( "legend_filter_by_expression" );

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
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  // test again with setLegendFilter and only expressions
  legendModel.setLegendFilterByMap( nullptr );
  legendModel.setLegendFilter( &mapSettings, /*useExtent*/ false );

  const QString testName2 = testName + "2";
  QString report2 = mReport + "2";
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName2, &legendModel, settings );
  QVERIFY( _verifyImage( testName2, report2 ) );
}

void TestQgsLegendRenderer::testDiagramAttributeLegend()
{
  QgsVectorLayer *vl4 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( vl4 );

  QgsMarkerSymbol *sym3_1 = new QgsMarkerSymbol();
  sym3_1->setColor( Qt::red );
  vl4->setRenderer( new QgsSingleSymbolRenderer( sym3_1 ) );

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
  QgsMarkerSymbol *sym3_1 = new QgsMarkerSymbol();
  sym3_1->setColor( Qt::red );
  vl4->setRenderer( new QgsSingleSymbolRenderer( sym3_1 ) );

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
  const QString testName = QStringLiteral( "legend_data_defined_size_collapsed" );

  QgsVectorLayer *vlDataDefinedSize = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "Point Layer" ), QStringLiteral( "memory" ) );
  {
    QgsVectorDataProvider *pr = vlDataDefinedSize->dataProvider();
    QList<QgsField> attrs;
    attrs << QgsField( QStringLiteral( "test_attr" ), QVariant::Int );
    pr->addAttributes( attrs );

    QgsFields fields;
    fields.append( attrs.back() );

    const QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( 1.0, 1.0 ) );

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

  QVariantMap props;
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
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  delete root;
}

void TestQgsLegendRenderer::testTextOnSymbol()
{
  const QString testName = QStringLiteral( "legend_text_on_symbol" );

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
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  delete root;
}

void TestQgsLegendRenderer::testBasicJson()
{
  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setTitle( QStringLiteral( "Legend" ) );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  const QJsonObject json = _renderJsonLegend( &legendModel, settings );

  QCOMPARE( json[ "title" ].toString(), QString( "Legend" ) );

  const QJsonArray root = json["nodes"].toArray();

  const QJsonObject grp1 = root[0].toObject();
  QCOMPARE( grp1["title"].toString(), QString( "Line + Polygon" ) );
  QCOMPARE( grp1["type"].toString(), QString( "group" ) );

  const QJsonArray grp1_nodes = grp1["nodes"].toArray();

  const QJsonObject line_layer = grp1_nodes[0].toObject();
  QCOMPARE( line_layer["title"].toString(), QString( "Line Layer" ) );
  QCOMPARE( line_layer["type"].toString(), QString( "layer" ) );
  const QImage line_layer_icon = _base64ToImage( line_layer["icon"].toString() );
  QString test_name = "line_layer_icon";
  line_layer_icon.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject polygon_layer = grp1_nodes[1].toObject();
  QCOMPARE( polygon_layer["title"].toString(), QString( "Polygon Layer" ) );
  QCOMPARE( polygon_layer["type"].toString(), QString( "layer" ) );
  const QImage polygon_layer_icon = _base64ToImage( polygon_layer["icon"].toString() );
  test_name = "polygon_layer_icon";
  polygon_layer_icon.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject point_layer = root[1].toObject();
  QCOMPARE( point_layer["title"].toString(), QString( "Point Layer" ) );
  QCOMPARE( point_layer["type"].toString(), QString( "layer" ) );
  const QJsonArray point_layer_symbols = point_layer["symbols"].toArray();

  const QJsonObject point_layer_symbol_red = point_layer_symbols[0].toObject();
  QCOMPARE( point_layer_symbol_red["title"].toString(), QString( "Red" ) );
  const QImage point_layer_icon_red = _base64ToImage( point_layer_symbol_red["icon"].toString() );
  test_name = "point_layer_icon_red";
  point_layer_icon_red.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject point_layer_symbol_green = point_layer_symbols[1].toObject();
  QCOMPARE( point_layer_symbol_green["title"].toString(), QString( "Green" ) );
  const QImage point_layer_icon_green = _base64ToImage( point_layer_symbol_green["icon"].toString() );
  test_name = "point_layer_icon_green";
  point_layer_icon_green.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject point_layer_symbol_blue = point_layer_symbols[2].toObject();
  QCOMPARE( point_layer_symbol_blue["title"].toString(), QString( "Blue" ) );
  const QImage point_layer_icon_blue = _base64ToImage( point_layer_symbol_blue["icon"].toString() );
  test_name = "point_layer_icon_blue";
  point_layer_icon_blue.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject raster_layer = root[2].toObject();
  QCOMPARE( raster_layer["title"].toString(), QString( "Raster Layer" ) );
  QCOMPARE( raster_layer["type"].toString(), QString( "layer" ) );
  const QJsonArray raster_layer_symbols = raster_layer["symbols"].toArray();

  const QJsonObject raster_layer_symbol_1 = raster_layer_symbols[0].toObject();
  QCOMPARE( raster_layer_symbol_1["title"].toString(), QString( "1" ) );
  const QImage raster_layer_icon_1 = _base64ToImage( raster_layer_symbol_1["icon"].toString() );
  test_name = "raster_layer_icon_1";
  raster_layer_icon_1.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject raster_layer_symbol_2 = raster_layer_symbols[1].toObject();
  QCOMPARE( raster_layer_symbol_2["title"].toString(), QString( "2" ) );
  const QImage raster_layer_icon_2 = _base64ToImage( raster_layer_symbol_2["icon"].toString() );
  test_name = "raster_layer_icon_2";
  raster_layer_icon_2.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );
}

void TestQgsLegendRenderer::testOpacityJson()
{
  const int opacity = mVL3->opacity();
  mVL3->setOpacity( 0.5 );
  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setTitle( QStringLiteral( "Legend" ) );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  const QJsonObject json = _renderJsonLegend( &legendModel, settings );

  const QJsonArray root = json["nodes"].toArray();

  const QJsonObject point_layer = root[1].toObject();
  const QJsonArray point_layer_symbols = point_layer["symbols"].toArray();

  const QJsonObject point_layer_symbol_red = point_layer_symbols[0].toObject();
  const QImage point_layer_icon_red = _base64ToImage( point_layer_symbol_red["icon"].toString() );
  QString test_name = "point_layer_icon_red_opacity";
  point_layer_icon_red.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject point_layer_symbol_green = point_layer_symbols[1].toObject();
  const QImage point_layer_icon_green = _base64ToImage( point_layer_symbol_green["icon"].toString() );
  test_name = "point_layer_icon_green_opacity";
  point_layer_icon_green.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  const QJsonObject point_layer_symbol_blue = point_layer_symbols[2].toObject();
  const QImage point_layer_icon_blue = _base64ToImage( point_layer_symbol_blue["icon"].toString() );
  test_name = "point_layer_icon_blue_opacity";
  point_layer_icon_blue.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 5 ) );

  mVL3->setOpacity( opacity );
}

void TestQgsLegendRenderer::testBigMarkerJson()
{
  QgsMarkerSymbol *sym = new QgsMarkerSymbol();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( mVL3->renderer() );
  QVERIFY( catRenderer );
  catRenderer->updateCategorySymbol( 0, sym );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setTitle( QStringLiteral( "Legend" ) );
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  const QJsonObject json = _renderJsonLegend( &legendModel, settings );

  const QJsonArray root = json["nodes"].toArray();

  const QJsonObject point_layer = root[1].toObject();
  const QJsonArray point_layer_symbols = point_layer["symbols"].toArray();

  const QJsonObject point_layer_symbol_red = point_layer_symbols[0].toObject();
  const QImage point_layer_icon_red = _base64ToImage( point_layer_symbol_red["icon"].toString() );
  const QString test_name = "point_layer_icon_red_big";
  point_layer_icon_red.save( _fileNameForTest( test_name ) );
  QVERIFY( _verifyImage( test_name, mReport, 50 ) );
}

void TestQgsLegendRenderer::testLabelLegend()
{
  const QString testName( "test_label_legend" );
  QgsPalLayerSettings *labelSettings = new QgsPalLayerSettings();
  labelSettings->fieldName = QStringLiteral( "test_attr" );
  QgsRuleBasedLabeling::Rule *rootRule = new QgsRuleBasedLabeling::Rule( nullptr ); //root rule
  QgsRuleBasedLabeling::Rule *labelingRule = new QgsRuleBasedLabeling::Rule( labelSettings, 0, 0, QString(), QStringLiteral( "labelingRule" ) );
  rootRule->appendChild( labelingRule );
  QgsRuleBasedLabeling *labeling = new QgsRuleBasedLabeling( rootRule );
  mVL3->setLabeling( labeling );
  const bool bkLabelsEnabled = mVL3->labelsEnabled();
  mVL3->setLabelsEnabled( true );

  QgsDefaultVectorLayerLegend *vLayerLegend = dynamic_cast<QgsDefaultVectorLayerLegend *>( mVL3->legend() );
  if ( !vLayerLegend )
  {
    QFAIL( "No vector layer legend" );
  }
  const bool bkLabelLegendEnabled = vLayerLegend->showLabelLegend();
  vLayerLegend->setShowLabelLegend( true );

  QgsLayerTreeModel legendModel( mRoot );
  QgsLegendSettings settings;

  //first test if label legend nodes are present in json
  const QJsonObject json = _renderJsonLegend( &legendModel, settings );
  const QJsonArray nodes = json["nodes"].toArray();
  const QJsonObject point_layer = nodes[1].toObject();
  const QJsonArray point_layer_symbols = point_layer["symbols"].toArray();
  const QJsonObject point_layer_labeling_symbol = point_layer_symbols[3].toObject();
  const QString labelTitle = point_layer_labeling_symbol["title"].toString();

  QVERIFY( labelTitle == "labelingRule" );

  //test rendered legend against reference image
  _setStandardTestFont( settings, QStringLiteral( "Bold" ) );
  _renderLegend( testName, &legendModel, settings );
  QVERIFY( _verifyImage( testName, mReport ) );

  vLayerLegend->setShowLabelLegend( bkLabelLegendEnabled );
  mVL3->setLabelsEnabled( bkLabelsEnabled );
}


QGSTEST_MAIN( TestQgsLegendRenderer )
#include "testqgslegendrenderer.moc"
