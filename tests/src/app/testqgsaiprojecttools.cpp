/***************************************************************************
  testqgsaiprojecttools.cpp
  --------------------------
  begin                : July 2026
  copyright            : (C) 2026
***************************************************************************/

#include "ai/tools/qgsaiprojecttools.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsproject.h"
#include "qgssnappingconfig.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

class TestQgsAiProjectTools : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void manageProjectSavesAndRollsBack();
    void manageProjectSetsCrsAndRollsBack();
    void manageProjectSetsPropertyAndReportsIt();
    void manageProjectRejectsUnsafeSaveAsAndInvalidCrs();
    void configureSnappingSetsAndGets();
    void configureSnappingRejectsInvalidValues();
};

void TestQgsAiProjectTools::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiProjectTools::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiProjectTools::manageProjectSavesAndRollsBack()
{
  QgsProject project;
  QgsAiManageProjectTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::High );

  QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  const QString projectPath = dir.filePath( u"ai_project.qgs"_s );

  QJsonObject saveAsArgs;
  saveAsArgs.insert( u"action"_s, u"save_as"_s );
  saveAsArgs.insert( u"path"_s, projectPath );

  const QgsAiToolResult saveAs = tool.execute( saveAsArgs );
  QVERIFY2( saveAs.success, qPrintable( saveAs.errorMessage ) );
  QVERIFY( QFileInfo( projectPath ).exists() );
  QCOMPARE( project.fileName(), projectPath );
  const QString saveAsRollbackToken = saveAs.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !saveAsRollbackToken.isEmpty() );

  QJsonObject saveArgs;
  saveArgs.insert( u"action"_s, u"save"_s );
  const QgsAiToolResult save = tool.execute( saveArgs );
  QVERIFY2( save.success, qPrintable( save.errorMessage ) );
  const QString saveRollbackToken = save.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !saveRollbackToken.isEmpty() );

  QJsonObject rollbackSaveArgs;
  rollbackSaveArgs.insert( u"rollback_token"_s, saveRollbackToken );
  const QgsAiToolResult rollbackSave = tool.execute( rollbackSaveArgs );
  QVERIFY2( rollbackSave.success, qPrintable( rollbackSave.errorMessage ) );
  QVERIFY( QFileInfo( projectPath ).exists() );
  QCOMPARE( project.fileName(), projectPath );

  QJsonObject rollbackSaveAsArgs;
  rollbackSaveAsArgs.insert( u"rollback_token"_s, saveAsRollbackToken );
  const QgsAiToolResult rollbackSaveAs = tool.execute( rollbackSaveAsArgs );
  QVERIFY2( rollbackSaveAs.success, qPrintable( rollbackSaveAs.errorMessage ) );
  QVERIFY( !QFileInfo( projectPath ).exists() );
  QVERIFY( project.fileName().isEmpty() );
}

void TestQgsAiProjectTools::manageProjectSetsCrsAndRollsBack()
{
  QgsProject project;
  project.setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  QgsAiManageProjectTool tool( &project );
  QJsonObject args;
  args.insert( u"action"_s, u"set_crs"_s );
  args.insert( u"crs"_s, u"EPSG:3857"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  QCOMPARE( project.crs().authid(), u"EPSG:3857"_s );
  QVERIFY( result.output.toObject().contains( u"diff"_s ) );

  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );
  QCOMPARE( project.crs().authid(), u"EPSG:4326"_s );
}

void TestQgsAiProjectTools::manageProjectSetsPropertyAndReportsIt()
{
  QgsProject project;
  QgsAiManageProjectTool tool( &project );

  QJsonObject args;
  args.insert( u"action"_s, u"set_property"_s );
  args.insert( u"key"_s, u"AI/test_property"_s );
  args.insert( u"value"_s, u"enabled"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );

  bool ok = false;
  QCOMPARE( project.readEntry( u"AI"_s, u"/test_property"_s, QString(), &ok ), u"enabled"_s );
  QVERIFY( ok );

  QJsonObject getArgs;
  getArgs.insert( u"action"_s, u"get_properties"_s );
  getArgs.insert( u"key"_s, u"AI/test_property"_s );
  const QgsAiToolResult properties = tool.execute( getArgs );
  QVERIFY2( properties.success, qPrintable( properties.errorMessage ) );
  const QJsonObject property = properties.output.toObject().value( u"property"_s ).toObject();
  QVERIFY( property.value( u"exists"_s ).toBool() );
  QCOMPARE( property.value( u"value"_s ).toString(), u"enabled"_s );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );
  project.readEntry( u"AI"_s, u"/test_property"_s, QString(), &ok );
  QVERIFY( !ok );
}

void TestQgsAiProjectTools::manageProjectRejectsUnsafeSaveAsAndInvalidCrs()
{
  QgsProject project;
  QgsAiManageProjectTool tool( &project );

  QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  const QString existingPath = dir.filePath( u"existing.qgs"_s );
  QFile existingFile( existingPath );
  QVERIFY( existingFile.open( QIODevice::WriteOnly ) );
  QVERIFY( existingFile.write( "existing" ) > 0 );
  existingFile.close();

  QJsonObject saveAsArgs;
  saveAsArgs.insert( u"action"_s, u"save_as"_s );
  saveAsArgs.insert( u"path"_s, existingPath );
  const QgsAiToolResult overwriteRejected = tool.execute( saveAsArgs );
  QVERIFY( !overwriteRejected.success );
  QVERIFY( overwriteRejected.errorMessage.contains( u"overwrite=true"_s ) );

  QJsonObject directorySaveArgs;
  directorySaveArgs.insert( u"action"_s, u"save_as"_s );
  directorySaveArgs.insert( u"path"_s, dir.path() );
  directorySaveArgs.insert( u"overwrite"_s, true );
  const QgsAiToolResult directoryRejected = tool.execute( directorySaveArgs );
  QVERIFY( !directoryRejected.success );
  QVERIFY( directoryRejected.errorMessage.contains( u"directory"_s ) );

  QJsonObject crsArgs;
  crsArgs.insert( u"action"_s, u"set_crs"_s );
  crsArgs.insert( u"crs"_s, u"NOT_A_CRS"_s );
  const QgsAiToolResult invalidCrs = tool.execute( crsArgs );
  QVERIFY( !invalidCrs.success );
  QVERIFY( invalidCrs.errorMessage.contains( u"Invalid CRS"_s ) );
}

void TestQgsAiProjectTools::configureSnappingSetsAndGets()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"LineString?crs=EPSG:4326"_s, u"Roads"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  project.addMapLayer( layer );

  QgsAiConfigureSnappingTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::Low );

  QJsonArray layerIds;
  layerIds.push_back( layer->id() );

  QJsonObject args;
  args.insert( u"action"_s, u"set"_s );
  args.insert( u"enabled"_s, true );
  args.insert( u"mode"_s, u"all_layers"_s );
  args.insert( u"type"_s, u"vertex"_s );
  args.insert( u"tolerance"_s, 10 );
  args.insert( u"units"_s, u"pixels"_s );
  args.insert( u"layer_ids"_s, layerIds );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QgsSnappingConfig config = project.snappingConfig();
  QVERIFY( config.enabled() );
  QCOMPARE( config.mode(), Qgis::SnappingMode::AdvancedConfiguration );
  QCOMPARE( config.typeFlag(), Qgis::SnappingTypes( Qgis::SnappingType::Vertex ) );
  QCOMPARE( config.tolerance(), 10.0 );
  QCOMPARE( config.units(), Qgis::MapToolUnit::Pixels );
  QVERIFY( config.individualLayerSettings( layer ).valid() );
  QVERIFY( config.individualLayerSettings( layer ).enabled() );

  QJsonObject getArgs;
  getArgs.insert( u"action"_s, u"get"_s );
  const QgsAiToolResult getResult = tool.execute( getArgs );
  QVERIFY2( getResult.success, qPrintable( getResult.errorMessage ) );
  QCOMPARE( getResult.output.toObject().value( u"enabled"_s ).toBool(), true );
  QCOMPARE( getResult.output.toObject().value( u"mode"_s ).toString(), u"advanced"_s );
  QCOMPARE( getResult.output.toObject().value( u"type"_s ).toString(), u"vertex"_s );
  QCOMPARE( getResult.output.toObject().value( u"tolerance"_s ).toDouble(), 10.0 );
  QCOMPARE( project.snappingConfig(), config );
}

void TestQgsAiProjectTools::configureSnappingRejectsInvalidValues()
{
  QgsProject project;
  QgsAiConfigureSnappingTool tool( &project );

  QJsonObject toleranceArgs;
  toleranceArgs.insert( u"action"_s, u"set"_s );
  toleranceArgs.insert( u"tolerance"_s, -1 );
  const QgsAiToolResult invalidTolerance = tool.execute( toleranceArgs );
  QVERIFY( !invalidTolerance.success );
  QVERIFY( invalidTolerance.errorMessage.contains( u"tolerance"_s ) );

  QJsonObject unitsArgs;
  unitsArgs.insert( u"action"_s, u"set"_s );
  unitsArgs.insert( u"units"_s, u"bananas"_s );
  const QgsAiToolResult invalidUnits = tool.execute( unitsArgs );
  QVERIFY( !invalidUnits.success );
  QVERIFY( invalidUnits.errorMessage.contains( u"units"_s ) );
}

QGSTEST_MAIN( TestQgsAiProjectTools )
#include "testqgsaiprojecttools.moc"
