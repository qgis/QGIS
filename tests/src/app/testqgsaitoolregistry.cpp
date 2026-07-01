/***************************************************************************
  testqgsaitoolregistry.cpp
  --------------------------
  begin                : April 2026
  copyright            : (C) 2026
***************************************************************************/

#include <memory>

#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaiworkspacetrust.h"
#include "ai/tools/qgsaidownloadfiletool.h"
#include "ai/tools/qgsailayertools.h"
#include "ai/tools/qgsaireadtools.h"
#include "ai/tools/qgsaitoolregistry.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayoutmanager.h"
#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgssettings.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgstest.h"

#include <cmath>
#include <QColor>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
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
      explicit FakeEchoTool( const QString &name, bool requiresApproval = false )
        : mName( name )
        , mRequiresApproval( requiresApproval )
      {}
      FakeEchoTool( const QString &name, bool requiresApproval, bool available )
        : mName( name )
        , mRequiresApproval( requiresApproval )
        , mAvailable( available )
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
      bool isAvailable() const override { return mAvailable; }
      QString availabilityReason() const override { return u"tool unavailable"_s; }

    private:
      QString mName;
      bool mRequiresApproval;
      bool mAvailable = true;
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
    void captureMapCanvasRequiresConsent();
    void captureMapCanvasCreatesCappedPng();
    void styleLayerAppliesNativeChanges();
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
  QVERIFY( std::abs( layer->opacity() - 0.35 ) < 0.001 );

  QgsLayerTreeLayer *node = project.layerTreeRoot()->findLayer( layer->id() );
  QVERIFY( node );
  QVERIFY( !node->itemVisibilityChecked() );

  QgsSingleSymbolRenderer *renderer = dynamic_cast<QgsSingleSymbolRenderer *>( layer->renderer() );
  QVERIFY( renderer );
  QCOMPARE( renderer->symbol()->color(), QColor( u"#ff0000"_s ) );
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
  QVERIFY( QFileInfo::exists( layoutPath ) );
  QVERIFY( QFileInfo( layoutPath ).size() > 0 );
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
