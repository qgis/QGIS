
#include <QtTest>
#include <QObject>

#include "qgsapplication.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgscomposerlegenditem.h"
#include "qgsfontutils.h"
#include "qgslayertree.h"
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



static QString _fileNameForTest( QString testName )
{
  return QDir::tempPath() + QDir::separator() + testName + ".png";
}

static void _setStandardTestFont( QgsLegendSettings& settings )
{
  QList< QgsComposerLegendStyle::Style> styles;
  styles << QgsComposerLegendStyle::Title
  << QgsComposerLegendStyle::Group
  << QgsComposerLegendStyle::Subgroup
  << QgsComposerLegendStyle::SymbolLabel;
  foreach ( QgsComposerLegendStyle::Style st, styles )
  {
    QFont font( QgsFontUtils::getStandardTestFont() );
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
  QImage img( s, QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::white );

  QPainter painter( &img );
  painter.scale( dpmm, dpmm );
  legendRenderer.drawLegend( &painter );
  painter.end();

  img.save( _fileNameForTest( testName ) );
}

static void _verifyImage( const QString& testName )
{
  QgsRenderChecker checker;
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( _fileNameForTest( testName ) );
  bool equal = checker.compareImages( testName, 500 );
  QVERIFY( equal );
}



class TestQgsLegendRenderer : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testModel();

    void testBasic();
    void testBigMarker();
    void testLongSymbolText();
    void testThreeColumns();

  private:
    QgsLayerTreeGroup* mRoot;
    QgsVectorLayer* mVL1; // line
    QgsVectorLayer* mVL2; // polygon
    QgsVectorLayer* mVL3; // point
    QgsRasterLayer* mRL;
};


void TestQgsLegendRenderer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLegendRenderer::cleanupTestCase()
{
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
  QgsMapLayerRegistry::instance()->addMapLayer( mVL3 );

  static char raster_array[] = { 1, 2, 2, 1 };
  QString rasterUri = QString( "MEM:::DATAPOINTER=%1,PIXELS=2,LINES=2" ).arg(( qulonglong ) raster_array );
  mRL = new QgsRasterLayer( rasterUri, "Raster Layer", "gdal" );
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

  QgsLayerTreeNode* nodeGroup0 = mRoot->children()[0];
  QVERIFY( nodeGroup0 );
  QgsLayerTreeNode* nodeLayer0 = nodeGroup0->children()[0];
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
  _verifyImage( testName );
}

void TestQgsLegendRenderer::testBigMarker()
{
  QString testName = "legend_big_marker";

  QgsMarkerSymbolV2* sym = new QgsMarkerSymbolV2();
  sym->setColor( Qt::red );
  sym->setSize( sym->size() * 6 );
  dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() )->updateCategorySymbol( 0, sym );

  //dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() )->updateCategoryLabel( 2, "This is a long symbol label" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  _verifyImage( testName );
}

void TestQgsLegendRenderer::testLongSymbolText()
{
  QString testName = "legend_long_symbol_text";

  dynamic_cast<QgsCategorizedSymbolRendererV2*>( mVL3->rendererV2() )->updateCategoryLabel( 1, "This is\nthree lines\nlong label" );

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setWrapChar( "\n" );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  _verifyImage( testName );
}

void TestQgsLegendRenderer::testThreeColumns()
{
  QString testName = "legend_three_columns";

  QgsLayerTreeModel legendModel( mRoot );

  QgsLegendSettings settings;
  settings.setColumnCount( 3 );
  _setStandardTestFont( settings );
  _renderLegend( testName, &legendModel, settings );
  _verifyImage( testName );
}


QTEST_MAIN( TestQgsLegendRenderer )
#include "testqgslegendrenderer.moc"
