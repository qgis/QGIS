/***************************************************************************
  testqgsaitoolregistry.cpp
  --------------------------
  begin                : April 2026
  copyright            : (C) 2026
***************************************************************************/

#include <cmath>
#include <memory>

#include "ai/qgsaiauditlog.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaiworkspacetrust.h"
#include "ai/tools/qgsaidownloadfiletool.h"
#include "ai/tools/qgsailayertools.h"
#include "ai/tools/qgsaireadtools.h"
#include "ai/tools/qgsaitoolregistry.h"
#include "qgsapplication.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgslayoutmanager.h"
#include "qgsmapcanvas.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgssettings.h"
#include "qgssinglesymbolrenderer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectordataprovider.h"

#include <QColor>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

namespace
{
  class FakeEchoTool : public QgsAiTool
  {
    public:
      explicit FakeEchoTool( const QString &name, bool requiresApproval = false, QgsAiToolRiskLevel riskLevel = QgsAiToolRiskLevel::Low )
        : mName( name )
        , mRequiresApproval( requiresApproval )
        , mRiskLevel( riskLevel )
      {}
      FakeEchoTool( const QString &name, bool requiresApproval, bool available, QgsAiToolRiskLevel riskLevel = QgsAiToolRiskLevel::Low )
        : mName( name )
        , mRequiresApproval( requiresApproval )
        , mAvailable( available )
        , mRiskLevel( riskLevel )
      {}

      QString name() const override { return mName; }
      QString description() const override { return u"Echoes the 'text' argument."_s; }

      QJsonObject schema() const override
      {
        QJsonObject properties;
        QJsonObject textProp;
        textProp.insert( u"type"_s, u"string"_s );
        properties.insert( u"text"_s, textProp );

        QJsonObject schemaObject;
        schemaObject.insert( u"type"_s, u"object"_s );
        schemaObject.insert( u"properties"_s, properties );

        QJsonArray required;
        required.append( u"text"_s );
        schemaObject.insert( u"required"_s, required );
        return schemaObject;
      }

      QgsAiToolResult execute( const QJsonObject &args ) override
      {
        if ( !args.contains( u"text"_s ) )
          return QgsAiToolResult::error( u"missing 'text'"_s );
        return QgsAiToolResult::ok( args.value( u"text"_s ) );
      }

      bool requiresApproval() const override { return mRequiresApproval; }
      QgsAiToolRiskLevel riskLevel() const override { return mRiskLevel; }
      bool isAvailable() const override { return mAvailable; }
      QString availabilityReason() const override { return u"tool unavailable"_s; }

    private:
      QString mName;
      bool mRequiresApproval;
      bool mAvailable = true;
      QgsAiToolRiskLevel mRiskLevel = QgsAiToolRiskLevel::Low;
  };
} //namespace

class TestQgsAiToolRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void registerAndLookup();
    void rejectsDuplicateNames();
    void rejectsEmptyNameAndNull();
    void schemasJsonContainsAllTools();
    void schemasJsonFilter();
    void unavailableToolsAreHiddenAndNotExecuted();
    void executeRoundTrip();
    void registryAuditsRiskyToolMetadataOnly();
    void captureMapCanvasRequiresConsent();
    void captureMapCanvasCreatesCappedPng();
    void addLayerFromServiceLoadsXyzAndRollsBack();
    void styleLayerAppliesNativeChanges();
    void advancedStyleLayerAppliesRenderersLabelsAndRollback();
    void createPrintLayoutAndExportMap();
    void processingToolReportsMissingAlgorithm();
    void clearEmptiesRegistry();
    void trustGatingHidesRiskyTools();
};

void TestQgsAiToolRegistry::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiToolRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiToolRegistry::registerAndLookup()
{
  QgsAiToolRegistry registry;
  QSignalSpy spy( &registry, &QgsAiToolRegistry::toolRegistered );

  QVERIFY( registry.registerTool( std::make_unique<FakeEchoTool>( u"echo"_s ) ) );
  QCOMPARE( registry.count(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.takeFirst().at( 0 ).toString(), u"echo"_s );

  QgsAiTool *tool = registry.find( u"echo"_s );
  QVERIFY( tool );
  QCOMPARE( tool->name(), u"echo"_s );
  QVERIFY( !registry.find( u"missing"_s ) );
  QCOMPARE( registry.toolNames(), QStringList() << u"echo"_s );
}

void TestQgsAiToolRegistry::rejectsDuplicateNames()
{
  QgsAiToolRegistry registry;
  QVERIFY( registry.registerTool( std::make_unique<FakeEchoTool>( u"echo"_s ) ) );
  QVERIFY( !registry.registerTool( std::make_unique<FakeEchoTool>( u"echo"_s ) ) );
  QCOMPARE( registry.count(), 1 );
}

void TestQgsAiToolRegistry::rejectsEmptyNameAndNull()
{
  QgsAiToolRegistry registry;
  QVERIFY( !registry.registerTool( nullptr ) );
  QVERIFY( !registry.registerTool( std::make_unique<FakeEchoTool>( QString() ) ) );
  QCOMPARE( registry.count(), 0 );
}

void TestQgsAiToolRegistry::schemasJsonContainsAllTools()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( u"alpha"_s ) );
  registry.registerTool( std::make_unique<FakeEchoTool>( u"beta"_s ) );

  const QJsonArray schemas = registry.schemasJson();
  QCOMPARE( schemas.size(), 2 );

  QSet<QString> names;
  for ( const QJsonValue &value : schemas )
  {
    const QJsonObject obj = value.toObject();
    QVERIFY( obj.contains( u"name"_s ) );
    QVERIFY( obj.contains( u"description"_s ) );
    QVERIFY( obj.contains( u"input_schema"_s ) );
    names.insert( obj.value( u"name"_s ).toString() );
  }
  QVERIFY( names.contains( u"alpha"_s ) );
  QVERIFY( names.contains( u"beta"_s ) );
}

void TestQgsAiToolRegistry::schemasJsonFilter()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( u"read_file"_s ) );
  registry.registerTool( std::make_unique<FakeEchoTool>( u"apply_patch"_s, true ) );

  const QJsonArray filtered = registry.schemasJson( QStringList() << u"read_file"_s );
  QCOMPARE( filtered.size(), 1 );
  QCOMPARE( filtered.first().toObject().value( u"name"_s ).toString(), u"read_file"_s );
}

void TestQgsAiToolRegistry::unavailableToolsAreHiddenAndNotExecuted()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( u"run_python"_s, true, false ) );

  QVERIFY( registry.availableToolNames().isEmpty() );
  QVERIFY( registry.schemasJson().isEmpty() );

  QJsonObject args;
  args.insert( u"text"_s, u"hello"_s );
  const QgsAiToolResult result = registry.execute( u"run_python"_s, args );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( u"unavailable"_s ) );
}

void TestQgsAiToolRegistry::executeRoundTrip()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( u"echo"_s ) );

  QgsAiTool *tool = registry.find( u"echo"_s );
  QVERIFY( tool );

  QJsonObject args;
  args.insert( u"text"_s, u"hello"_s );
  const QgsAiToolResult result = tool->execute( args );
  QVERIFY( result.success );
  QCOMPARE( result.output.toString(), u"hello"_s );

  const QgsAiToolResult missing = tool->execute( QJsonObject() );
  QVERIFY( !missing.success );
  QVERIFY( !missing.errorMessage.isEmpty() );
}

void TestQgsAiToolRegistry::registryAuditsRiskyToolMetadataOnly()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString auditPath = tempDir.filePath( u"audit.log"_s );
  QgsAiAuditLog::setFilePathOverride( auditPath );
  const auto cleanup = qScopeGuard( []() { QgsAiAuditLog::setFilePathOverride( QString() ); } );

  QgsAiToolRegistry registry;
  QVERIFY( registry.registerTool( std::make_unique<FakeEchoTool>( u"mutating_tool"_s, true, QgsAiToolRiskLevel::Medium ) ) );

  QJsonObject args;
  args.insert( u"text"_s, u"secret payload that must not be logged"_s );
  const QgsAiToolResult result = registry.execute( u"mutating_tool"_s, args );
  QVERIFY( result.success );

  QFile file( auditPath );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  const QString line = QString::fromUtf8( file.readAll() );
  QVERIFY2( line.contains( u"| tool_event |"_s ), qPrintable( line ) );
  QVERIFY( line.contains( u"event=execute"_s ) );
  QVERIFY( line.contains( u"tool=mutating_tool"_s ) );
  QVERIFY( line.contains( u"risk=medium"_s ) );
  QVERIFY( line.contains( u"success=true"_s ) );
  QVERIFY( line.contains( u"args_sha256"_s ) );
  QVERIFY( !line.contains( u"secret payload"_s ) );
}

void TestQgsAiToolRegistry::captureMapCanvasRequiresConsent()
{
  QgsSettings settings;
  settings.remove( u"strata/visual_context/image_send_consent"_s );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );

  QgsMapCanvas canvas;
  QgsAiCaptureMapCanvasTool tool( &canvas );
  const QgsAiToolResult result = tool.execute( QJsonObject() );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( u"consent"_s, Qt::CaseInsensitive ) );
}

void TestQgsAiToolRegistry::captureMapCanvasCreatesCappedPng()
{
  QgsSettings settings;
  settings.setValue( u"strata/visual_context/image_send_consent"_s, true );

  QgsMapCanvas canvas;
  canvas.resize( 2000, 1000 );
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 5 ) );

  QgsAiCaptureMapCanvasTool tool( &canvas );
  QJsonObject args;
  args.insert( u"max_longest_side"_s, 500 );
  const QgsAiToolResult result = tool.execute( args );
  QVERIFY( result.success );

  const QJsonObject output = result.output.toObject();
  const QJsonObject imageJson = output.value( u"image"_s ).toObject();
  const QString path = imageJson.value( u"path"_s ).toString();
  QVERIFY( QFileInfo::exists( path ) );
  QCOMPARE( imageJson.value( u"mime_type"_s ).toString(), u"image/png"_s );

  const QImage image( path );
  QVERIFY( !image.isNull() );
  QVERIFY( image.width() <= 500 );
  QVERIFY( image.height() <= 500 );
  QCOMPARE( image.width(), imageJson.value( u"width"_s ).toInt() );
  QCOMPARE( image.height(), imageJson.value( u"height"_s ).toInt() );

  settings.remove( u"strata/visual_context/image_send_consent"_s );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );
}

void TestQgsAiToolRegistry::addLayerFromServiceLoadsXyzAndRollsBack()
{
  QgsProject project;
  QgsAiAddLayerFromServiceTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::High );

  QJsonObject args;
  args.insert( u"provider"_s, u"xyz"_s );
  args.insert( u"uri"_s, u"type=xyz&url=file://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0"_s );
  args.insert( u"name"_s, u"Local XYZ"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QJsonObject output = result.output.toObject();
  const QString layerId = output.value( u"layer_id"_s ).toString();
  QVERIFY( !layerId.isEmpty() );
  QCOMPARE( output.value( u"provider"_s ).toString(), u"xyz"_s );
  QCOMPARE( output.value( u"provider_key"_s ).toString(), u"wms"_s );
  QVERIFY( output.contains( u"diff"_s ) );
  QCOMPARE( project.mapLayers().size(), 1 );
  QVERIFY( project.mapLayer( layerId ) );

  const QString rollbackToken = output.value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );
  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );
  QCOMPARE( project.mapLayers().size(), 0 );
}

void TestQgsAiToolRegistry::styleLayerAppliesNativeChanges()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=name:string"_s, u"Points"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  project.addMapLayer( layer );

  QgsAiStyleLayerTool tool( &project );
  QVERIFY( tool.requiresApproval() );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"opacity"_s, 0.35 );
  args.insert( u"visible"_s, false );
  args.insert( u"color"_s, u"#ff0000"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );
  QVERIFY( result.output.toObject().contains( u"diff"_s ) );
  QVERIFY( std::abs( layer->opacity() - 0.35 ) < 0.001 );

  QgsLayerTreeLayer *node = project.layerTreeRoot()->findLayer( layer->id() );
  QVERIFY( node );
  QVERIFY( !node->itemVisibilityChecked() );

  QgsSingleSymbolRenderer *renderer = dynamic_cast<QgsSingleSymbolRenderer *>( layer->renderer() );
  QVERIFY( renderer );
  QCOMPARE( renderer->symbol()->color(), QColor( u"#ff0000"_s ) );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );
  QVERIFY( std::abs( layer->opacity() - 1.0 ) < 0.001 );
  QVERIFY( node->itemVisibilityChecked() );
}

void TestQgsAiToolRegistry::advancedStyleLayerAppliesRenderersLabelsAndRollback()
{
  QgsProject project;
  QgsVectorLayer *categorizedLayer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=category:string&field=name:string"_s, u"Categories"_s, u"memory"_s );
  QVERIFY( categorizedLayer->isValid() );

  QgsFeature first( categorizedLayer->fields() );
  first.setAttribute( u"category"_s, u"A"_s );
  first.setAttribute( u"name"_s, u"Alpha"_s );
  QgsFeature second( categorizedLayer->fields() );
  second.setAttribute( u"category"_s, u"B"_s );
  second.setAttribute( u"name"_s, u"Beta"_s );
  QgsFeature third( categorizedLayer->fields() );
  third.setAttribute( u"category"_s, u"A"_s );
  third.setAttribute( u"name"_s, u"Again"_s );
  QVERIFY( categorizedLayer->dataProvider()->addFeatures( QgsFeatureList() << first << second << third ) );
  project.addMapLayer( categorizedLayer );

  QgsAiAdvancedStyleTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::Medium );

  QJsonObject labels;
  labels.insert( u"enabled"_s, true );
  labels.insert( u"field"_s, u"name"_s );

  QJsonObject categorizedArgs;
  categorizedArgs.insert( u"layer_id"_s, categorizedLayer->id() );
  categorizedArgs.insert( u"renderer"_s, u"categorized"_s );
  categorizedArgs.insert( u"field"_s, u"category"_s );
  categorizedArgs.insert( u"labels"_s, labels );
  const QgsAiToolResult categorizedResult = tool.execute( categorizedArgs );
  QVERIFY2( categorizedResult.success, qPrintable( categorizedResult.errorMessage ) );
  const QString categorizedRollbackToken = categorizedResult.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !categorizedRollbackToken.isEmpty() );

  QgsCategorizedSymbolRenderer *categorizedRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( categorizedLayer->renderer() );
  QVERIFY( categorizedRenderer );
  QCOMPARE( categorizedRenderer->categories().size(), 2 );
  QVERIFY( categorizedLayer->labelsEnabled() );
  QVERIFY( categorizedLayer->labeling() );
  QCOMPARE( categorizedLayer->labeling()->settings().fieldName, u"name"_s );

  QJsonObject categorizedRollbackArgs;
  categorizedRollbackArgs.insert( u"rollback_token"_s, categorizedRollbackToken );
  const QgsAiToolResult categorizedRollback = tool.execute( categorizedRollbackArgs );
  QVERIFY2( categorizedRollback.success, qPrintable( categorizedRollback.errorMessage ) );
  QVERIFY( dynamic_cast<QgsSingleSymbolRenderer *>( categorizedLayer->renderer() ) );
  QVERIFY( !categorizedLayer->labelsEnabled() );

  QgsVectorLayer *graduatedLayer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=value:double"_s, u"Values"_s, u"memory"_s );
  QVERIFY( graduatedLayer->isValid() );
  QgsFeature low( graduatedLayer->fields() );
  low.setAttribute( u"value"_s, 1.0 );
  QgsFeature mid( graduatedLayer->fields() );
  mid.setAttribute( u"value"_s, 5.0 );
  QgsFeature high( graduatedLayer->fields() );
  high.setAttribute( u"value"_s, 9.0 );
  QVERIFY( graduatedLayer->dataProvider()->addFeatures( QgsFeatureList() << low << mid << high ) );
  project.addMapLayer( graduatedLayer );

  QJsonObject graduatedArgs;
  graduatedArgs.insert( u"layer_id"_s, graduatedLayer->id() );
  graduatedArgs.insert( u"renderer"_s, u"graduated"_s );
  graduatedArgs.insert( u"field"_s, u"value"_s );
  graduatedArgs.insert( u"classes"_s, 3 );
  const QgsAiToolResult graduatedResult = tool.execute( graduatedArgs );
  QVERIFY2( graduatedResult.success, qPrintable( graduatedResult.errorMessage ) );
  const QString graduatedRollbackToken = graduatedResult.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !graduatedRollbackToken.isEmpty() );

  QgsGraduatedSymbolRenderer *graduatedRenderer = dynamic_cast<QgsGraduatedSymbolRenderer *>( graduatedLayer->renderer() );
  QVERIFY( graduatedRenderer );
  QCOMPARE( graduatedRenderer->ranges().size(), 3 );

  QJsonObject graduatedRollbackArgs;
  graduatedRollbackArgs.insert( u"rollback_token"_s, graduatedRollbackToken );
  const QgsAiToolResult graduatedRollback = tool.execute( graduatedRollbackArgs );
  QVERIFY2( graduatedRollback.success, qPrintable( graduatedRollback.errorMessage ) );
  QVERIFY( dynamic_cast<QgsSingleSymbolRenderer *>( graduatedLayer->renderer() ) );
}

void TestQgsAiToolRegistry::createPrintLayoutAndExportMap()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=name:string"_s, u"Points"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  project.addMapLayer( layer );

  QgsMapCanvas canvas;
  canvas.resize( 640, 360 );
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setExtent( QgsRectangle( -10, -5, 10, 5 ) );
  canvas.setLayers( QList<QgsMapLayer *>() << layer );

  QgsAiCreatePrintLayoutTool createTool( &project, &canvas );
  QVERIFY( createTool.requiresApproval() );

  QJsonObject createArgs;
  createArgs.insert( u"name"_s, u"AI Layout"_s );
  createArgs.insert( u"title"_s, u"Project map"_s );
  const QgsAiToolResult createResult = createTool.execute( createArgs );
  QVERIFY2( createResult.success, qPrintable( createResult.errorMessage ) );
  const QString layoutRollbackToken = createResult.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !layoutRollbackToken.isEmpty() );
  QVERIFY( createResult.output.toObject().contains( u"diff"_s ) );
  QVERIFY( project.layoutManager()->layoutByName( u"AI Layout"_s ) );

  QgsAiExportMapTool exportTool( &contextProvider, &project, &canvas );
  QVERIFY( exportTool.requiresApproval() );

  QJsonObject canvasExportArgs;
  canvasExportArgs.insert( u"path"_s, u"exports/canvas.png"_s );
  canvasExportArgs.insert( u"format"_s, u"png"_s );
  canvasExportArgs.insert( u"width"_s, 320 );
  canvasExportArgs.insert( u"height"_s, 180 );
  const QgsAiToolResult canvasExport = exportTool.execute( canvasExportArgs );
  QVERIFY2( canvasExport.success, qPrintable( canvasExport.errorMessage ) );
  const QString canvasPath = canvasExport.output.toObject().value( u"absolute_path"_s ).toString();
  const QString canvasRollbackToken = canvasExport.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !canvasRollbackToken.isEmpty() );
  QVERIFY( canvasExport.output.toObject().contains( u"diff"_s ) );
  QVERIFY( QFileInfo::exists( canvasPath ) );
  const QImage canvasImage( canvasPath );
  QVERIFY( !canvasImage.isNull() );
  QCOMPARE( canvasImage.size(), QSize( 320, 180 ) );

  QJsonObject layoutExportArgs;
  layoutExportArgs.insert( u"path"_s, u"exports/layout.png"_s );
  layoutExportArgs.insert( u"format"_s, u"png"_s );
  layoutExportArgs.insert( u"layout_name"_s, u"AI Layout"_s );
  const QgsAiToolResult layoutExport = exportTool.execute( layoutExportArgs );
  QVERIFY2( layoutExport.success, qPrintable( layoutExport.errorMessage ) );
  const QString layoutPath = layoutExport.output.toObject().value( u"absolute_path"_s ).toString();
  const QString layoutExportRollbackToken = layoutExport.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !layoutExportRollbackToken.isEmpty() );
  QVERIFY( QFileInfo::exists( layoutPath ) );
  QVERIFY( QFileInfo( layoutPath ).size() > 0 );

  QJsonObject canvasRollbackArgs;
  canvasRollbackArgs.insert( u"rollback_token"_s, canvasRollbackToken );
  const QgsAiToolResult canvasRollback = exportTool.execute( canvasRollbackArgs );
  QVERIFY2( canvasRollback.success, qPrintable( canvasRollback.errorMessage ) );
  QVERIFY( !QFileInfo::exists( canvasPath ) );

  QJsonObject layoutExportRollbackArgs;
  layoutExportRollbackArgs.insert( u"rollback_token"_s, layoutExportRollbackToken );
  const QgsAiToolResult layoutExportRollback = exportTool.execute( layoutExportRollbackArgs );
  QVERIFY2( layoutExportRollback.success, qPrintable( layoutExportRollback.errorMessage ) );
  QVERIFY( !QFileInfo::exists( layoutPath ) );

  QJsonObject layoutRollbackArgs;
  layoutRollbackArgs.insert( u"rollback_token"_s, layoutRollbackToken );
  const QgsAiToolResult layoutRollback = createTool.execute( layoutRollbackArgs );
  QVERIFY2( layoutRollback.success, qPrintable( layoutRollback.errorMessage ) );
  QVERIFY( !project.layoutManager()->layoutByName( u"AI Layout"_s ) );
}

void TestQgsAiToolRegistry::processingToolReportsMissingAlgorithm()
{
  QgsProject project;
  QgsAiRunProcessingAlgorithmTool tool( &project );
  QVERIFY( tool.requiresApproval() );

  QJsonObject args;
  args.insert( u"algorithm_id"_s, u"strata:missing_algorithm"_s );
  args.insert( u"parameters"_s, QJsonObject() );
  const QgsAiToolResult result = tool.execute( args );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( u"Unknown Processing algorithm"_s ) || result.errorMessage.contains( u"not available"_s ) );
}

void TestQgsAiToolRegistry::clearEmptiesRegistry()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( u"echo"_s ) );
  QCOMPARE( registry.count(), 1 );
  registry.clear();
  QCOMPARE( registry.count(), 0 );
  QVERIFY( !registry.find( u"echo"_s ) );
}

void TestQgsAiToolRegistry::trustGatingHidesRiskyTools()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<QgsAiDownloadFileTool>( &contextProvider, nullptr ) );

  // Unknown trust state ⇒ restricted: the risky tool is neither advertised nor executable.
  QCOMPARE( QgsAiWorkspaceTrust::state( tempDir.path() ), QgsAiWorkspaceTrust::State::Unknown );
  QVERIFY( !registry.availableToolNames().contains( u"download_file"_s ) );
  QgsAiToolResult blocked = registry.execute( u"download_file"_s, QJsonObject() );
  QVERIFY( !blocked.success );
  QVERIFY2( blocked.errorMessage.contains( u"not trusted"_s ), qPrintable( blocked.errorMessage ) );

  // Trusted ⇒ advertised again.
  QgsAiWorkspaceTrust::setState( tempDir.path(), QgsAiWorkspaceTrust::State::Trusted );
  QVERIFY( registry.availableToolNames().contains( u"download_file"_s ) );

  // Revoking trust hides it once more.
  QgsAiWorkspaceTrust::setState( tempDir.path(), QgsAiWorkspaceTrust::State::Untrusted );
  QVERIFY( !registry.availableToolNames().contains( u"download_file"_s ) );
}

QGSTEST_MAIN( TestQgsAiToolRegistry )
#include "testqgsaitoolregistry.moc"
