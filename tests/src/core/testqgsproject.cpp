/***************************************************************************
     testqgsproject.cpp
     --------------------------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Martin Dobias
    Email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsarchive.h"
#include "qgslayertree.h"
#include "qgslayoutmanager.h"
#include "qgsmaplayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbollayerutils.h"
#include "qgstest.h"
#include "qgsunittypes.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QObject>
#include <QSignalSpy>
#include <QTimer>

class TestQgsProject : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testDirtySet();
    void testReadPath();
    void testPathResolver();
    void testPathResolverSvg();
    void testProjectUnits();
    void variablesChanged();
    void testLayerFlags();
    void testLocalFiles();
    void testLocalUrlFiles();
    void testReadFlags();
    void testSetGetCrs();
    void testEmbeddedLayerGroupFromQgz();
    void projectSaveUser();
    void testCrsExpressions();
    void testCrsValidAfterReadingProjectFile();
    void testFilePathType();
    void testDefaultRelativePaths();
    void testAttachmentsQgs();
    void testAttachmentsQgz();
    void testAttachmentIdentifier();
    void testEmbeddedGroupWithJoins();
    void testAsynchronousLayerLoading();
    void testSymlinks1LayerRasterChange();
    void testSymlinks2LayerFolder();
    void testSymlinks3LayerShapefile();
    void testSymlinks4LayerShapefileBroken();
    void testSymlinks5ProjectFile();
    void testSymlinks6ProjectFolder();
    void regression60100();
    //! Test issue GH #63007 (autorefresh timer not started after loading the project file)
    void testAutorefreshModeRestore();
};

void TestQgsProject::init()
{
}

void TestQgsProject::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsProject::initTestCase()
{
  // Runs once before any tests are run

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsSettings().clear();
}


void TestQgsProject::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}

void TestQgsProject::testDirtySet()
{
  QgsProject project;
  QSignalSpy dirtySetSpy( &project, &QgsProject::dirtySet );
  project.setDirty( true );
  QVERIFY( dirtySetSpy.count() == 1 );
}

void TestQgsProject::testReadPath()
{
  QgsProject *prj = new QgsProject;
  // this is a bit hacky as we do not really load such project
#if defined( Q_OS_WIN )
  const QString prefix( "C:" );
#else
  const QString prefix;
#endif
  prj->setFileName( prefix + "/home/qgis/a-project-file.qgs" ); // not expected to exist
  // make sure we work with relative paths!
  prj->writeEntry( u"Paths"_s, u"Absolute"_s, false );

  QCOMPARE( prj->readPath( "./x.shp" ), QString( prefix + "/home/qgis/x.shp" ) );
  QCOMPARE( prj->readPath( "../x.shp" ), QString( prefix + "/home/x.shp" ) );

  // TODO: old style (seems QGIS < 1.3) - needs existing project file and existing file
  // QCOMPARE( prj->readPath( "x.shp" ), QString( "/home/qgis/x.shp" ) );

  // VSI: /vsizip, /vsitar, /vsigzip, *.zip, *.gz, *.tgz, ...

  QCOMPARE( prj->readPath( "./x.gz" ), QString( prefix + "/home/qgis/x.gz" ) );
  QCOMPARE( prj->readPath( "/vsigzip/./x.gz" ), QString( "/vsigzip/%1/home/qgis/x.gz" ).arg( prefix ) ); // not sure how useful this really is...

  delete prj;
}

void TestQgsProject::testPathResolver()
{
  // Test resolver with a non existing file path
  const QgsPathResolver resolverLegacy( u"/home/qgis/test.qgs"_s );
  QCOMPARE( resolverLegacy.readPath( QString() ), QString() );
  QCOMPARE( resolverLegacy.writePath( QString() ), QString() );
  QCOMPARE( resolverLegacy.writePath( "/home/qgis/file1.txt" ), QString( "./file1.txt" ) );
  QCOMPARE( resolverLegacy.writePath( "/home/qgis/subdir/file1.txt" ), QString( "./subdir/file1.txt" ) );
  QCOMPARE( resolverLegacy.writePath( "/home/file1.txt" ), QString( "../file1.txt" ) );
  QCOMPARE( resolverLegacy.readPath( "./file1.txt" ), QString( "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverLegacy.readPath( "./subdir/file1.txt" ), QString( "/home/qgis/subdir/file1.txt" ) );
  QCOMPARE( resolverLegacy.readPath( "../file1.txt" ), QString( "/home/file1.txt" ) );
  QCOMPARE( resolverLegacy.readPath( "/home/qgis/file1.txt" ), QString( "/home/qgis/file1.txt" ) );

  // Test resolver with existing file path
  const QTemporaryDir tmpDir;
  const QString tmpDirName = tmpDir.path();
  const QDir dir( tmpDirName );
  dir.mkpath( tmpDirName + "/home/qgis/" );

  const QgsPathResolver resolverRel( QString( tmpDirName + "/home/qgis/test.qgs" ) );
  QCOMPARE( resolverRel.readPath( QString() ), QString() );
  QCOMPARE( resolverRel.writePath( QString() ), QString() );
  QCOMPARE( resolverRel.writePath( tmpDirName + "/home/qgis/file1.txt" ), QString( "./file1.txt" ) );
  QCOMPARE( resolverRel.writePath( tmpDirName + "/home/qgis/subdir/file1.txt" ), QString( "./subdir/file1.txt" ) );
  QCOMPARE( resolverRel.writePath( tmpDirName + "/home/file1.txt" ), QString( "../file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "./file1.txt" ), QString( tmpDirName + "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "./subdir/file1.txt" ), QString( tmpDirName + "/home/qgis/subdir/file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "../file1.txt" ), QString( tmpDirName + "/home/file1.txt" ) );
  QCOMPARE( resolverRel.readPath( tmpDirName + "/home/qgis/file1.txt" ), QString( tmpDirName + "/home/qgis/file1.txt" ) );

  // test older style relative path - file must exist for this to work
  QTemporaryFile tmpFile;
  QVERIFY( tmpFile.open() ); // fileName is not available until we open the file
  const QString tmpName = tmpFile.fileName();
  tmpFile.close();
  const QgsPathResolver tempRel( tmpName );
  const QFileInfo fi( tmpName );
  QFile testFile( fi.path() + u"/file1.txt"_s );
  QVERIFY( testFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
  testFile.close();
  QVERIFY( QFile::exists( fi.path() + u"/file1.txt"_s ) );
  QCOMPARE( tempRel.readPath( "file1.txt" ), QString( fi.path() + u"/file1.txt"_s ) );

  const QgsPathResolver resolverAbs;
  QCOMPARE( resolverAbs.writePath( "/home/qgis/file1.txt" ), QString( "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverAbs.readPath( "/home/qgis/file1.txt" ), QString( "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverAbs.readPath( "./file1.txt" ), QString( "./file1.txt" ) );

  // TODO: test non-canonical paths - there are inconsistencies in the implementation
  // e.g. base filename "/home/qgis/../test.qgs" resolving "/home/qgis/../file1.txt" back and forth
}

static void _useRendererWithSvgSymbol( QgsVectorLayer *layer, const QString &path )
{
  QgsSvgMarkerSymbolLayer *sl = new QgsSvgMarkerSymbolLayer( path );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << sl );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( markerSymbol );
  layer->setRenderer( renderer );
}

static QString _getLayerSvgMarkerPath( const QgsProject &prj, const QString &layerName )
{
  QList<QgsMapLayer *> layers = prj.mapLayersByName( layerName );
  Q_ASSERT( layers.count() == 1 );
  Q_ASSERT( layers[0]->type() == Qgis::LayerType::Vector );
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( layers[0] );
  Q_ASSERT( layer->renderer() );
  Q_ASSERT( layer->renderer()->type() == "singleSymbol" );
  QgsSingleSymbolRenderer *r = static_cast<QgsSingleSymbolRenderer *>( layer->renderer() );
  QgsSymbol *s = r->symbol();
  Q_ASSERT( s && s->symbolLayerCount() == 1 );
  Q_ASSERT( s->symbolLayer( 0 )->layerType() == "SvgMarker" );
  QgsSvgMarkerSymbolLayer *sl = static_cast<QgsSvgMarkerSymbolLayer *>( s->symbolLayer( 0 ) );
  return sl->path();
}

static QHash<QString, QString> _parseSvgPathsForLayers( const QString &projectFilename )
{
  QHash<QString, QString> projectFileSvgPaths; // key = layer name, value = svg path

  QDomDocument doc;
  QFile projectFile( projectFilename );
  bool res = projectFile.open( QIODevice::ReadOnly );
  Q_ASSERT( res );
  res = static_cast<bool>( doc.setContent( &projectFile ) );
  Q_ASSERT( res );
  projectFile.close();

  const QDomElement docElem = doc.documentElement();
  const QDomElement layersElem = docElem.firstChildElement( u"projectlayers"_s );
  QDomElement layerElem = layersElem.firstChildElement();
  while ( !layerElem.isNull() )
  {
    const QString layerName = layerElem.firstChildElement( u"layername"_s ).text();
    const QDomElement symbolElem = layerElem.firstChildElement( u"renderer-v2"_s ).firstChildElement( u"symbols"_s ).firstChildElement( u"symbol"_s ).firstChildElement( u"layer"_s );
    QVariantMap props = QgsSymbolLayerUtils::parseProperties( symbolElem );
    QString svgPath = props.value( "name"_L1 ).toString();
    projectFileSvgPaths[layerName] = svgPath;
    layerElem = layerElem.nextSiblingElement();
  }
  return projectFileSvgPaths;
}

void TestQgsProject::testPathResolverSvg()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + "/points.shp";

  QVERIFY( QgsSymbolLayerUtils::svgSymbolNameToPath( QString(), QgsPathResolver() ).isEmpty() );
  QVERIFY( QgsSymbolLayerUtils::svgSymbolPathToName( QString(), QgsPathResolver() ).isEmpty() );

  // build a project with 3 layers, each having a simple renderer with SVG marker
  // - existing SVG file in project dir
  // - existing SVG file in QGIS dir
  // - non-exsiting SVG file

  const QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  // on mac the returned path was not canonical and the resolver failed to convert paths properly
  const QString dirPath = QFileInfo( dir.path() ).canonicalFilePath();

  const QString projectFilename = dirPath + "/project.qgs";
  const QString ourSvgPath = dirPath + "/valid.svg";
  const QString invalidSvgPath = dirPath + "/invalid.svg";

  QFile svgFile( ourSvgPath );
  QVERIFY( svgFile.open( QIODevice::WriteOnly ) );
  svgFile.write( "<svg/>" ); // not a proper SVG, but good enough for this case
  svgFile.close();

  QVERIFY( QFileInfo::exists( ourSvgPath ) ); // should exist now

  const QString librarySvgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( u"transport/transport_airport.svg"_s, QgsPathResolver() );
  QCOMPARE( QgsSymbolLayerUtils::svgSymbolPathToName( librarySvgPath, QgsPathResolver() ), u"transport/transport_airport.svg"_s );

  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, u"points 1"_s, u"ogr"_s );
  _useRendererWithSvgSymbol( layer1, ourSvgPath );

  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, u"points 2"_s, u"ogr"_s );
  _useRendererWithSvgSymbol( layer2, invalidSvgPath );

  QgsVectorLayer *layer3 = new QgsVectorLayer( layerPath, u"points 3"_s, u"ogr"_s );
  _useRendererWithSvgSymbol( layer3, librarySvgPath );

  QVERIFY( layer1->isValid() );

  QgsProject project;
  project.addMapLayers( QList<QgsMapLayer *>() << layer1 << layer2 << layer3 );
  project.write( projectFilename );

  // make sure the path resolver works with relative paths (enabled by default)
  QCOMPARE( project.pathResolver().readPath( "./a.txt" ), QString( dirPath + "/a.txt" ) );
  QCOMPARE( project.pathResolver().writePath( dirPath + "/a.txt" ), QString( "./a.txt" ) );

  // check that the saved paths are relative

  // key = layer name, value = svg path
  QHash<QString, QString> projectFileSvgPaths = _parseSvgPathsForLayers( projectFilename );

  QCOMPARE( projectFileSvgPaths.count(), 3 );
  QCOMPARE( projectFileSvgPaths["points 1"], QString( "./valid.svg" ) );                     // relative path to project
  QCOMPARE( projectFileSvgPaths["points 2"], invalidSvgPath );                               // full path to non-existent file (not sure why - but that's how it works now)
  QCOMPARE( projectFileSvgPaths["points 3"], QString( "transport/transport_airport.svg" ) ); // relative path to library

  // load project again, check that the paths are absolute
  QgsProject projectLoaded;
  projectLoaded.read( projectFilename );
  const QString svg1 = _getLayerSvgMarkerPath( projectLoaded, u"points 1"_s );
  const QString svg2 = _getLayerSvgMarkerPath( projectLoaded, u"points 2"_s );
  const QString svg3 = _getLayerSvgMarkerPath( projectLoaded, u"points 3"_s );
  QCOMPARE( svg1, ourSvgPath );
  QCOMPARE( svg2, invalidSvgPath );
  QCOMPARE( svg3, librarySvgPath );

  //
  // now let's use these layers in embedded in another project...
  //

  QList<QDomNode> brokenNodes;
  QgsProject projectMaster;
  QVERIFY( projectMaster.createEmbeddedLayer( layer1->id(), projectFilename, brokenNodes ) );
  QVERIFY( projectMaster.createEmbeddedLayer( layer2->id(), projectFilename, brokenNodes ) );
  QVERIFY( projectMaster.createEmbeddedLayer( layer3->id(), projectFilename, brokenNodes ) );

  const QString svg1x = _getLayerSvgMarkerPath( projectMaster, u"points 1"_s );
  const QString svg2x = _getLayerSvgMarkerPath( projectLoaded, u"points 2"_s );
  const QString svg3x = _getLayerSvgMarkerPath( projectLoaded, u"points 3"_s );
  QCOMPARE( svg1x, ourSvgPath );
  QCOMPARE( svg2x, invalidSvgPath );
  QCOMPARE( svg3x, librarySvgPath );
}


void TestQgsProject::testProjectUnits()
{
  //test setting and retrieving project units

  // DISTANCE

  //first set a default QGIS distance unit
  QgsSettings s;
  s.setValue( u"/qgis/measure/displayunits"_s, QgsUnitTypes::encodeUnit( Qgis::DistanceUnit::Feet ) );

  QgsProject *prj = new QgsProject;
  // new project should inherit QGIS default distance unit
  prj->clear();
  QCOMPARE( prj->distanceUnits(), Qgis::DistanceUnit::Feet );

  //changing default QGIS unit should not affect existing project
  s.setValue( u"/qgis/measure/displayunits"_s, QgsUnitTypes::encodeUnit( Qgis::DistanceUnit::NauticalMiles ) );
  QCOMPARE( prj->distanceUnits(), Qgis::DistanceUnit::Feet );

  //test setting new units for project
  prj->setDistanceUnits( Qgis::DistanceUnit::NauticalMiles );
  QCOMPARE( prj->distanceUnits(), Qgis::DistanceUnit::NauticalMiles );

  // AREA

  //first set a default QGIS area unit
  s.setValue( u"/qgis/measure/areaunits"_s, QgsUnitTypes::encodeUnit( Qgis::AreaUnit::SquareYards ) );

  // new project should inherit QGIS default area unit
  prj->clear();
  QCOMPARE( prj->areaUnits(), Qgis::AreaUnit::SquareYards );

  //changing default QGIS unit should not affect existing project
  s.setValue( u"/qgis/measure/areaunits"_s, QgsUnitTypes::encodeUnit( Qgis::AreaUnit::Acres ) );
  QCOMPARE( prj->areaUnits(), Qgis::AreaUnit::SquareYards );

  //test setting new units for project
  prj->setAreaUnits( Qgis::AreaUnit::Acres );
  QCOMPARE( prj->areaUnits(), Qgis::AreaUnit::Acres );

  delete prj;
}

void TestQgsProject::variablesChanged()
{
  QgsProject *prj = new QgsProject;
  const QSignalSpy spyVariablesChanged( prj, &QgsProject::customVariablesChanged );
  QVariantMap vars;
  vars.insert( u"variable"_s, u"1"_s );
  prj->setCustomVariables( vars );
  QVERIFY( spyVariablesChanged.count() == 1 );
  delete prj;
}

void TestQgsProject::testLayerFlags()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + "/points.shp";
  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, u"points 1"_s, u"ogr"_s );
  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, u"points 2"_s, u"ogr"_s );

  QgsProject prj;
  prj.addMapLayer( layer1 );
  prj.addMapLayer( layer2 );

  layer2->setFlags( layer2->flags() & ~QgsMapLayer::Removable );

  const QString layer2id = layer2->id();

  QTemporaryFile f;
  QVERIFY( f.open() );
  f.close();
  prj.setFileName( f.fileName() );
  prj.write();

  // test reading required layers back
  QgsProject prj2;
  prj2.setFileName( f.fileName() );
  QVERIFY( prj2.read() );
  QgsMapLayer *layer = prj2.mapLayer( layer2id );
  QVERIFY( layer );
  QVERIFY( !layer->flags().testFlag( QgsMapLayer::Removable ) );
  QVERIFY( !layer->mReadFlags.testFlag( QgsMapLayer::FlagDontResolveLayers ) );
  QVERIFY( !layer->mReadFlags.testFlag( QgsMapLayer::FlagTrustLayerMetadata ) );
  QVERIFY( !layer->mReadFlags.testFlag( QgsMapLayer::FlagReadExtentFromXml ) );
  QVERIFY( !layer->mReadFlags.testFlag( QgsMapLayer::FlagForceReadOnly ) );

  // test setFlags modifies correctly existing layer settings
  QVERIFY( !prj2.isDirty() );
  prj2.setFlag( Qgis::ProjectFlag::TrustStoredLayerStatistics, true );
  prj2.setFlag( Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide, true );
  QVERIFY( prj2.isDirty() );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( prj2.mapLayer( layer2id ) );
  QVERIFY( vlayer->readExtentFromXml() );
  // vlayer doesn't have trust because it will be done for new layer or when reloading the project
  // no need to set trust on a layer which has already loaded everything
  QVERIFY( !vlayer->dataProvider()->mReadFlags.testFlag( Qgis::DataProviderReadFlag::TrustDataSource ) );
  QVERIFY( vlayer->dataProvider()->providerProperty( QgsVectorDataProvider::EvaluateDefaultValues ).toBool() );

  prj2.write();

  // check reload of project sets the correct flags and layer properties
  QgsProject prj3;
  prj3.setFileName( f.fileName() );
  QVERIFY( prj3.read() );
  QVERIFY( prj3.flags().testFlag( Qgis::ProjectFlag::TrustStoredLayerStatistics ) );
  QVERIFY( prj3.flags().testFlag( Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide ) );
  vlayer = qobject_cast<QgsVectorLayer *>( prj3.mapLayer( layer2id ) );
  QVERIFY( vlayer );
  QVERIFY( !vlayer->flags().testFlag( QgsMapLayer::Removable ) );
  QVERIFY( !vlayer->mReadFlags.testFlag( QgsMapLayer::FlagDontResolveLayers ) );
  QVERIFY( vlayer->mReadFlags.testFlag( QgsMapLayer::FlagTrustLayerMetadata ) );
  QVERIFY( vlayer->mReadFlags.testFlag( QgsMapLayer::FlagReadExtentFromXml ) );
  QVERIFY( !vlayer->mReadFlags.testFlag( QgsMapLayer::FlagForceReadOnly ) );
  QVERIFY( !prj3.isDirty() );
  QVERIFY( vlayer->readExtentFromXml() );
  QVERIFY( vlayer->dataProvider()->mReadFlags.testFlag( Qgis::DataProviderReadFlag::TrustDataSource ) );
  QVERIFY( vlayer->dataProvider()->providerProperty( QgsVectorDataProvider::EvaluateDefaultValues ).toBool() );

  // check reload of project with read fags that sets the correct layer properties
  QgsProject prj4;
  prj4.setFileName( f.fileName() );
  Qgis::ProjectReadFlags readFlags = Qgis::ProjectReadFlag::DontResolveLayers
                                     | Qgis::ProjectReadFlag::TrustLayerMetadata
                                     | Qgis::ProjectReadFlag::ForceReadOnlyLayers;
  QVERIFY( prj4.read( readFlags ) );
  vlayer = qobject_cast<QgsVectorLayer *>( prj4.mapLayer( layer2id ) );
  QVERIFY( vlayer );
  QVERIFY( !vlayer->flags().testFlag( QgsMapLayer::Removable ) );
  QVERIFY( vlayer->mReadFlags.testFlag( QgsMapLayer::FlagDontResolveLayers ) );
  QVERIFY( vlayer->mReadFlags.testFlag( QgsMapLayer::FlagTrustLayerMetadata ) );
  QVERIFY( vlayer->mReadFlags.testFlag( QgsMapLayer::FlagReadExtentFromXml ) );
  QVERIFY( vlayer->mReadFlags.testFlag( QgsMapLayer::FlagForceReadOnly ) );
}

void TestQgsProject::testLocalFiles()
{
  QTemporaryFile f;
  QVERIFY( f.open() );
  f.close();
  QgsProject prj;
  const QFileInfo info( f.fileName() );
  prj.setFileName( f.fileName() );
  prj.write();
  const QString shpPath = info.dir().path() + '/' + info.baseName() + ".shp";
  const QString layerPath = "file://" + shpPath;
  QFile f2( shpPath );
  QVERIFY( f2.open( QFile::ReadWrite ) );
  f2.close();
  const QgsPathResolver resolver( f.fileName() );
  QCOMPARE( resolver.writePath( layerPath ), QString( "./" + info.baseName() + ".shp" ) );
}

void TestQgsProject::testLocalUrlFiles()
{
  QTemporaryFile f;
  QVERIFY( f.open() );
  f.close();
  QgsProject prj;
  const QFileInfo info( f.fileName() );
  prj.setFileName( f.fileName() );
  prj.write();
  const QString shpPath = info.dir().path() + '/' + info.baseName() + ".shp";
  const QString extraStuff { "?someVar=someValue&someOtherVar=someOtherValue" };
  const QString layerPath = "file://" + shpPath + extraStuff;
  QFile f2( shpPath );
  QVERIFY( f2.open( QFile::ReadWrite ) );
  f2.close();
  const QgsPathResolver resolver( f.fileName() );
  QCOMPARE( resolver.writePath( layerPath ), QString( "./" + info.baseName() + ".shp" + extraStuff ) );
}

void TestQgsProject::testReadFlags()
{
  const QString project1Path = QString( TEST_DATA_DIR ) + u"/embedded_groups/project1.qgs"_s;
  QgsProject p;
  QVERIFY( p.read( project1Path, Qgis::ProjectReadFlag::DontResolveLayers ) );
  auto layers = p.mapLayers();
  QCOMPARE( layers.count(), 3 );
  // layers should be invalid - we skipped loading them!
  QVERIFY( !layers.value( u"points20170310142652246"_s )->isValid() );
  QVERIFY( !layers.value( u"lines20170310142652255"_s )->isValid() );
  QVERIFY( !layers.value( u"polys20170310142652234"_s )->isValid() );

  // but they should have renderers (and other stuff!)
  QCOMPARE( qobject_cast<QgsVectorLayer *>( layers.value( u"points20170310142652246"_s ) )->renderer()->type(), u"categorizedSymbol"_s );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( layers.value( u"lines20170310142652255"_s ) )->renderer()->type(), u"categorizedSymbol"_s );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( layers.value( u"polys20170310142652234"_s ) )->renderer()->type(), u"categorizedSymbol"_s );
  QVERIFY( !layers.value( u"polys20170310142652234"_s )->originalXmlProperties().isEmpty() );

  // do not store styles
  QVERIFY( p.read( project1Path, Qgis::ProjectReadFlag::DontStoreOriginalStyles ) );
  layers = p.mapLayers();
  QVERIFY( layers.value( u"polys20170310142652234"_s )->originalXmlProperties().isEmpty() );

  // project with embedded groups
  const QString project2Path = QString( TEST_DATA_DIR ) + u"/embedded_groups/project2.qgs"_s;
  QgsProject p2;
  QVERIFY( p2.read( project2Path, Qgis::ProjectReadFlag::DontResolveLayers ) );
  // layers should be invalid - we skipped loading them!
  layers = p2.mapLayers();
  QCOMPARE( layers.count(), 2 );
  QVERIFY( !layers.value( u"lines20170310142652255"_s )->isValid() );
  QVERIFY( !layers.value( u"polys20170310142652234"_s )->isValid() );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( layers.value( u"lines20170310142652255"_s ) )->renderer()->type(), u"categorizedSymbol"_s );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( layers.value( u"polys20170310142652234"_s ) )->renderer()->type(), u"categorizedSymbol"_s );


  const QString project3Path = QString( TEST_DATA_DIR ) + u"/layouts/layout_casting.qgs"_s;
  QgsProject p3;
  QVERIFY( p3.read( project3Path, Qgis::ProjectReadFlag::DontLoadLayouts ) );
  QCOMPARE( p3.layoutManager()->layouts().count(), 0 );
}

void TestQgsProject::testEmbeddedLayerGroupFromQgz()
{
  QString path = QString( TEST_DATA_DIR ) + u"/embedded_groups/project1.qgz"_s;
  QList<QDomNode> brokenNodes;

  QgsProject p0;
  p0.read( path );
  QgsMapLayer *points = p0.mapLayersByName( "points" )[0];
  QgsMapLayer *polys = p0.mapLayersByName( "polys" )[0];

  QgsProject p1;
  p1.createEmbeddedLayer( points->id(), p0.fileName(), brokenNodes );
  std::unique_ptr< QgsLayerTreeGroup > group = p1.createEmbeddedGroup( "group1", p0.fileName(), QStringList() );

  QCOMPARE( p1.layerIsEmbedded( points->id() ), path );
  QCOMPARE( p1.layerIsEmbedded( polys->id() ), path );

  // test embedded layers when origin project is something like ../XXX
  path = QString( TEST_DATA_DIR ) + u"/embedded_layers/project.qgz"_s;
  QgsProject p2;
  p2.read( path );

  QgsMapLayer *points2 = p0.mapLayersByName( "points" )[0];
  const bool saveFlag = p2.mEmbeddedLayers[points2->id()].second;
  QCOMPARE( saveFlag, true );

  const bool valid = p2.loadEmbeddedNodes( p2.layerTreeRoot() );
  QCOMPARE( valid, true );
}

void TestQgsProject::projectSaveUser()
{
  QgsProject p;
  QVERIFY( p.saveUser().isEmpty() );
  QVERIFY( p.saveUserFullName().isEmpty() );
  QVERIFY( !p.lastSaveDateTime().isValid() );

  QTemporaryFile f;
  QVERIFY( f.open() );
  f.close();
  p.setFileName( f.fileName() );
  p.write();

  QCOMPARE( p.saveUser(), QgsApplication::userLoginName() );
  QCOMPARE( p.saveUserFullName(), QgsApplication::userFullName() );
  QCOMPARE( p.lastSaveDateTime().date(), QDateTime::currentDateTime().date() );
  QCOMPARE( p.lastSaveVersion().text(), QgsProjectVersion( Qgis::version() ).text() );

  QgsSettings s;
  s.setValue( u"projects/anonymize_saved_projects"_s, true, QgsSettings::Core );

  p.write();

  QVERIFY( p.saveUser().isEmpty() );
  QVERIFY( p.saveUserFullName().isEmpty() );
  QVERIFY( p.metadata().author().isEmpty() );
  QVERIFY( !p.metadata().creationDateTime().isValid() );
  QVERIFY( !p.lastSaveDateTime().isValid() );

  s.setValue( u"projects/anonymize_saved_projects"_s, false, QgsSettings::Core );

  p.write();
  QCOMPARE( p.saveUser(), QgsApplication::userLoginName() );
  QCOMPARE( p.saveUserFullName(), QgsApplication::userFullName() );
  QCOMPARE( p.metadata().author(), QgsApplication::userFullName() );
  QCOMPARE( p.metadata().creationDateTime().date(), QDateTime::currentDateTime().date() );
  QCOMPARE( p.lastSaveDateTime().date(), QDateTime::currentDateTime().date() );

  QgsProject p2;
  QVERIFY( p2.read( QString( TEST_DATA_DIR ) + u"/embedded_groups/project1.qgs"_s ) );
  QCOMPARE( p2.lastSaveVersion().text(), u"2.99.0-Master"_s );
  p2.clear();
  QVERIFY( p2.lastSaveVersion().isNull() );
}

void TestQgsProject::testSetGetCrs()
{
  QgsProject p;

  // Set 4326
  //  - CRS changes
  //  - ellipsoid stays as NONE
  QSignalSpy crsChangedSpy( &p, &QgsProject::crsChanged );
  QSignalSpy ellipsoidChangedSpy( &p, &QgsProject::ellipsoidChanged );

  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );

  QCOMPARE( crsChangedSpy.count(), 1 );
  QCOMPARE( ellipsoidChangedSpy.count(), 0 );

  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
  QCOMPARE( p.ellipsoid(), u"NONE"_s );

  crsChangedSpy.clear();
  ellipsoidChangedSpy.clear();

  // Set 21781
  //  - CRS changes
  //  - ellipsoid stays as NONE

  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 21781 ) );

  QCOMPARE( crsChangedSpy.count(), 1 );
  QCOMPARE( ellipsoidChangedSpy.count(), 0 );

  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 21781 ) );
  QCOMPARE( p.ellipsoid(), u"NONE"_s );

  crsChangedSpy.clear();
  ellipsoidChangedSpy.clear();

  // Set 21781 again, including adjustEllipsoid flag
  //  - CRS changes
  //  - ellipsoid changes to Bessel

  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 21781 ), true );

  QCOMPARE( crsChangedSpy.count(), 0 );
  QCOMPARE( ellipsoidChangedSpy.count(), 1 );

  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 21781 ) );
  QCOMPARE( p.ellipsoid(), u"EPSG:7004"_s );

  crsChangedSpy.clear();
  ellipsoidChangedSpy.clear();

  // Set 2056, including adjustEllipsoid flag
  //  - CRS changes
  //  - ellipsoid stays

  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 2056 ), true );

  QCOMPARE( crsChangedSpy.count(), 1 );
  QCOMPARE( ellipsoidChangedSpy.count(), 0 );

  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 2056 ) );
  QCOMPARE( p.ellipsoid(), u"EPSG:7004"_s );
}

void TestQgsProject::testCrsValidAfterReadingProjectFile()
{
  QgsProject p;
  const QSignalSpy crsChangedSpy( &p, &QgsProject::crsChanged );

  //  - new project
  //  - set CRS tp 4326, the crs changes
  //  - save the project
  //  - clear()
  //  - load the project, the CRS should be 4326
  const QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  // on mac the returned path was not canonical and the resolver failed to convert paths properly
  const QString dirPath = QFileInfo( dir.path() ).canonicalFilePath();
  const QString projectFilename = dirPath + "/project.qgs";

  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );

  QCOMPARE( crsChangedSpy.count(), 1 );
  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );

  QVERIFY( p.write( projectFilename ) );
  p.clear();

  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem() );
  QCOMPARE( crsChangedSpy.count(), 1 );

  QVERIFY( p.read( projectFilename ) );
  QCOMPARE( p.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
  QCOMPARE( crsChangedSpy.count(), 2 );
}

void TestQgsProject::testFilePathType()
{
  QgsProject p;
  p.setFilePathStorage( Qgis::FilePathType::Absolute );
  QCOMPARE( p.filePathStorage(), Qgis::FilePathType::Absolute );

  p.setFilePathStorage( Qgis::FilePathType::Relative );
  QCOMPARE( p.filePathStorage(), Qgis::FilePathType::Relative );
}

void TestQgsProject::testCrsExpressions()
{
  QgsProject p;
  QVariant r;

  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );

  const QgsExpressionContext c = p.createExpressionContext();

  QgsExpression e2( u"@project_crs"_s );
  r = e2.evaluate( &c );
  QCOMPARE( r.toString(), QString( "EPSG:4326" ) );

  QgsExpression e3( u"@project_crs_definition"_s );
  r = e3.evaluate( &c );
  QCOMPARE( r.toString(), QString( "+proj=longlat +datum=WGS84 +no_defs" ) );

  QgsExpression e4( u"@project_units"_s );
  r = e4.evaluate( &c );
  QCOMPARE( r.toString(), QString( "degrees" ) );

  QgsExpression e5( u"@project_crs_description"_s );
  r = e5.evaluate( &c );
  QCOMPARE( r.toString(), QString( "WGS 84" ) );

  QgsExpression e6( u"@project_crs_acronym"_s );
  r = e6.evaluate( &c );
  QCOMPARE( r.toString(), QString( "longlat" ) );

  QgsExpression e7( u"@project_crs_proj4"_s );
  r = e7.evaluate( &c );
  QCOMPARE( r.toString(), QString( "+proj=longlat +datum=WGS84 +no_defs" ) );

  QgsExpression e8( u"@project_crs_wkt"_s );
  r = e8.evaluate( &c );
  QVERIFY( r.toString().length() >= 15 );

  QgsExpression e9( u"@project_crs_ellipsoid"_s );
  r = e9.evaluate( &c );
  QCOMPARE( r.toString(), QString( "EPSG:7030" ) );
}

void TestQgsProject::testDefaultRelativePaths()
{
  QgsSettings s;
  const bool bk_defaultRelativePaths = s.value( u"/qgis/defaultProjectPathsRelative"_s, QVariant( true ) ).toBool();

  s.setValue( u"/qgis/defaultProjectPathsRelative"_s, true );
  QgsProject p1;
  const bool p1PathsAbsolute = p1.readBoolEntry( u"Paths"_s, u"/Absolute"_s, false );
  const Qgis::FilePathType p1Type = p1.filePathStorage();

  s.setValue( u"/qgis/defaultProjectPathsRelative"_s, false );
  p1.clear();
  const bool p1PathsAbsolute_2 = p1.readBoolEntry( u"Paths"_s, u"/Absolute"_s, false );
  const Qgis::FilePathType p2Type = p1.filePathStorage();

  s.setValue( u"/qgis/defaultProjectPathsRelative"_s, bk_defaultRelativePaths );

  QCOMPARE( p1PathsAbsolute, false );
  QCOMPARE( p1PathsAbsolute_2, true );
  QCOMPARE( p1Type, Qgis::FilePathType::Relative );
  QCOMPARE( p2Type, Qgis::FilePathType::Absolute );
}

void TestQgsProject::testAttachmentsQgs()
{
  // Test QgsProject::{createAttachedFile,attachedFiles,removeAttachedFile}
  int defaultAttachmentSize = 0;
  {
    QgsProject p;
    defaultAttachmentSize = p.attachedFiles().size();

    const QString fileName = p.createAttachedFile( "myattachment" );
    QVERIFY( QFile( fileName ).exists() );
    QVERIFY( p.attachedFiles().contains( fileName ) );
    QVERIFY( p.removeAttachedFile( fileName ) );
    QVERIFY( !p.attachedFiles().contains( fileName ) );
    QVERIFY( !p.removeAttachedFile( fileName ) );
    QCOMPARE( p.attachedFiles().size(), defaultAttachmentSize );
  }

  // Verify that attachment is exists after re-reading project
  {
    QTemporaryFile projFile( QDir::temp().absoluteFilePath( "XXXXXX_test.qgs" ) );
    QVERIFY( projFile.open() );

    QgsProject p;
    QFile file;
    const QString fileName = p.createAttachedFile( "myattachment" );

    file.setFileName( fileName );
    QVERIFY( file.open( QIODevice::WriteOnly ) );
    file.write( "Attachment" );
    file.close();

    p.write( projFile.fileName() );

    const QFileInfo finfo( projFile.fileName() );
    const QString attachmentsZip = finfo.absoluteDir().absoluteFilePath( u"%1_attachments.zip"_s.arg( finfo.completeBaseName() ) );
    QVERIFY( QFile( attachmentsZip ).exists() );

    QgsProject p2;
    p2.read( projFile.fileName() );
    QCOMPARE( p2.attachedFiles().size(), defaultAttachmentSize + 1 );

    file.setFileName( p2.attachedFiles().at( defaultAttachmentSize ) );
    QVERIFY( file.open( QIODevice::ReadOnly ) );
    QVERIFY( file.readAll() == QByteArray( "Attachment" ) );
  }

  // Verify that attachment paths can be used as layer filenames
  {
    QTemporaryFile projFile( QDir::temp().absoluteFilePath( "XXXXXX_test.qgs" ) );
    QVERIFY( projFile.open() );

    QgsProject p;
    const QString fileName = p.createAttachedFile( "testlayer.gpx" );
    QFile file( fileName );
    QVERIFY( file.open( QIODevice::WriteOnly ) );
    file.write( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" );
    file.write( "<gpx version=\"1.0\">" );
    file.write( "<name>Example gpx</name>" );
    file.write( "<wpt lat=\"0.0\" lon=\"0.0\">" );
    file.write( "<name>NULL Island</name>" );
    file.write( "</wpt>" );
    file.write( "</gpx>" );
    file.close();

    QgsVectorLayer *layer = new QgsVectorLayer( fileName, "gpx" );
    p.addMapLayer( layer );
    p.write( projFile.fileName() );

    const QFileInfo finfo( projFile.fileName() );
    const QString attachmentsZip = finfo.absoluteDir().absoluteFilePath( u"%1_attachments.zip"_s.arg( finfo.completeBaseName() ) );
    QVERIFY( QFile( attachmentsZip ).exists() );

    QgsProject p2;
    p2.read( projFile.fileName() );
    QCOMPARE( p2.attachedFiles().size(), defaultAttachmentSize + 1 );
    QCOMPARE( p2.mapLayers().size(), 1 );
    QCOMPARE( p2.mapLayer( p2.mapLayers().firstKey() )->source(), p2.attachedFiles().at( defaultAttachmentSize ) );

    // Verify that attachment file is removed when layer is deleted
    QgsMapLayer *p2layer = p2.mapLayer( p2.mapLayers().firstKey() );
    QString path = p2layer->source();
    QVERIFY( QFile( path ).exists() );
    p2.removeMapLayer( p2layer->id() );
    QVERIFY( !QFile( path ).exists() );
  }
}

void TestQgsProject::testAttachmentsQgz()
{
  int defaultAttachmentSize = 0;
  // Test QgsProject::{createAttachedFile,attachedFiles,removeAttachedFile}
  {
    QgsProject p;
    defaultAttachmentSize = p.attachedFiles().size();

    const QString fileName = p.createAttachedFile( "myattachment" );
    QVERIFY( QFile( fileName ).exists() );
    QVERIFY( p.attachedFiles().contains( fileName ) );
    QVERIFY( p.removeAttachedFile( fileName ) );
    QVERIFY( !p.attachedFiles().contains( fileName ) );
    QVERIFY( !p.removeAttachedFile( fileName ) );
    QCOMPARE( p.attachedFiles().size(), defaultAttachmentSize );
  }

  // Verify that attachment is exists after re-reading project
  {
    QTemporaryFile projFile( QDir::temp().absoluteFilePath( "XXXXXX_test.qgz" ) );
    QVERIFY( projFile.open() );

    QgsProject p;
    QFile file;
    const QString fileName = p.createAttachedFile( "myattachment" );

    file.setFileName( fileName );
    QVERIFY( file.open( QIODevice::WriteOnly ) );
    file.write( "Attachment" );
    file.close();

    p.write( projFile.fileName() );

    QgsProject p2;
    p2.read( projFile.fileName() );
    QCOMPARE( p2.attachedFiles().size(), defaultAttachmentSize + 1 );

    file.setFileName( p2.attachedFiles().at( defaultAttachmentSize ) );
    QVERIFY( file.open( QIODevice::ReadOnly ) );
    QVERIFY( file.readAll() == QByteArray( "Attachment" ) );
  }

  // Verify that attachment paths can be used as layer filenames
  {
    QTemporaryFile projFile( QDir::temp().absoluteFilePath( "XXXXXX_test.qgz" ) );
    QVERIFY( projFile.open() );

    QgsProject p;
    const QString fileName = p.createAttachedFile( "testlayer.gpx" );
    QFile file( fileName );
    QVERIFY( file.open( QIODevice::WriteOnly ) );
    file.write( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" );
    file.write( "<gpx version=\"1.0\">" );
    file.write( "<name>Example gpx</name>" );
    file.write( "<wpt lat=\"0.0\" lon=\"0.0\">" );
    file.write( "<name>NULL Island</name>" );
    file.write( "</wpt>" );
    file.write( "</gpx>" );
    file.close();

    QgsVectorLayer *layer = new QgsVectorLayer( fileName, "gpx" );
    p.addMapLayer( layer );
    p.write( projFile.fileName() );

    QgsProject p2;
    p2.read( projFile.fileName() );
    QCOMPARE( p2.attachedFiles().size(), defaultAttachmentSize + 1 );
    QCOMPARE( p2.mapLayers().size(), 1 );
    QCOMPARE( p2.mapLayer( p2.mapLayers().firstKey() )->source(), p2.attachedFiles().at( defaultAttachmentSize ) );

    // Verify that attachment file is removed when layer is deleted
    QgsMapLayer *p2layer = p2.mapLayer( p2.mapLayers().firstKey() );
    QString path = p2layer->source();
    QVERIFY( QFile( path ).exists() );
    p2.removeMapLayer( p2layer->id() );
    QVERIFY( !QFile( path ).exists() );
  }
}

void TestQgsProject::testAttachmentIdentifier()
{
  // Verify attachment identifiers
  {
    QTemporaryFile projFile( QDir::temp().absoluteFilePath( "XXXXXX_test.qgz" ) );
    QVERIFY( projFile.open() );

    QgsProject p;
    const QString attachmentFileName = p.createAttachedFile( "test.jpg" );
    const QString attachmentId = p.attachmentIdentifier( attachmentFileName );
    QCOMPARE( p.resolveAttachmentIdentifier( attachmentId ), attachmentFileName );
    p.write( projFile.fileName() );

    QgsProject p2;
    p2.read( projFile.fileName() );
    QVERIFY( QFile( p2.resolveAttachmentIdentifier( attachmentId ) ).exists() );
  }
}


void TestQgsProject::testEmbeddedGroupWithJoins()
{
  const QString projectPath = QString( TEST_DATA_DIR ) + u"/embedded_groups/joins2.qgz"_s;
  QgsProject p;
  p.read( projectPath );

  QCOMPARE( p.layers<QgsVectorLayer *>().count(), 2 );

  QgsVectorLayer *vl = p.mapLayer<QgsVectorLayer *>( u"polys_with_id_32002f94_eebe_40a5_a182_44198ba1bc5a"_s );
  QCOMPARE( vl->fields().count(), 5 );
}

void TestQgsProject::testAsynchronousLayerLoading()
{
  auto project = std::make_unique<QgsProject>();

  QStringList meshFilters;
  meshFilters << u"*.nc"_s << u"*.2dm"_s;
  QStringList rasterFilters;
  rasterFilters << u"*.asc"_s << u"*.tif"_s;
  QStringList vectorFilters;
  vectorFilters << u"*.shp"_s;

  QStringList rasterFiles;
  rasterFiles << u"band1_byte_attribute_table_epsg4326.tif"_s
              << u"band1_byte_ct_epsg4326.tif"_s
              << u"band1_byte_noct_epsg4326.tif"_s
              << u"band1_int16_noct_epsg4326.tif"_s
              << u"band3_byte_noct_epsg4326.tif"_s
              << u"band3_float32_noct_epsg4326.tif"_s
              << u"band3_int16_noct_epsg4326.tif"_s
              << u"byte.tif"_s
              << u"byte_with_nan_nodata.tif"_s
              << u"dem.tif"_s
              << u"gtiff_desc.tif"_s
              << u"gtiff_tags.tif"_s
              << u"raster_shading.tif"_s
              << u"rgb_with_mask.tif"_s
              << u"rnd_percentile_raster1_byte.tif"_s
              << u"rnd_percentile_raster1_float64.tif"_s
              << u"rnd_percentile_raster2_byte.tif"_s
              << u"rnd_percentile_raster2_float64.tif"_s
              << u"rnd_percentile_raster3_byte.tif"_s
              << u"rnd_percentile_raster3_float64.tif"_s
              << u"rnd_percentile_raster4_byte.tif"_s
              << u"rnd_percentile_raster4_float64.tif"_s
              << u"rnd_percentile_raster5_byte.tif"_s
              << u"rnd_percentile_raster5_float64.tif"_s
              << u"rnd_percentrank_valueraster_float64.tif"_s
              << u"scale0ingdal23.tif"_s
              << u"statisticsRas1_float64.asc"_s
              << u"statisticsRas1_int32.tif"_s
              << u"statistXXXX_XXXXXX.asc"_s //invalid name
              << u"statisticsRas2_float64.asc"_s
              << u"statisticsRas2_int32.tif"_s
              << u"statisticsRas3_float64.asc"_s
              << u"statisticsRas3_int32.tif"_s
              << u"statisticsRas4_float64.asc"_s
              << u"test.asc"_s
              << u"unique_1.tif"_s
              << u"valueRas1_float64.asc"_s
              << u"valueRas2_float64.asc"_s
              << u"valueRas3_float64.asc"_s
              << u"with_color_table.tif"_s;
  QStringList vectorFiles;
  vectorFiles << u"bug5598.shp"_s
              << u"empty_spatial_layer.shp"_s
              << u"filter_test.shp"_s
              << u"france_parts.shp"_s
              << u"lines.shp"_s
              << u"lines_cardinals.shp"_s
              << u"lines_touching.shp"_s
              << u"linestXXXX_XXXXXX.shp"_s //invalid name
              << u"multipatch.shp"_s
              << u"multipoint.shp"_s
              << u"points.shp"_s
              << u"points_relations.shp"_s
              << u"polys.shp"_s
              << u"polys_overlapping.shp"_s
              << u"polys_overlapping_with_cat.shp"_s
              << u"polys_overlapping_with_id.shp"_s
              << u"polys_with_id.shp"_s
              << u"rectangles.shp"_s
              << u"test_852.shp"_s;


  QList<QgsMapLayer *> layers;

  for ( const QString &rasterFile : std::as_const( rasterFiles ) )
  {
    layers << new QgsRasterLayer( QString( TEST_DATA_DIR ) + u"/raster/"_s + rasterFile, rasterFile, u"gdal"_s );
    if ( layers.last()->name() == "statistXXXX_XXXXXX.asc"_L1 )
      QVERIFY( !layers.last()->isValid() );
    else
      QVERIFY( layers.last()->isValid() );
  }

  for ( const QString &vectorFile : std::as_const( vectorFiles ) )
  {
    layers << new QgsVectorLayer( QString( TEST_DATA_DIR ) + QString( '/' ) + vectorFile, vectorFile, u"ogr"_s );
    if ( layers.last()->name() == "linestXXXX_XXXXXX.shp"_L1 )
      QVERIFY( !layers.last()->isValid() );
    else
      QVERIFY( layers.last()->isValid() );
  }

  int layersCount = layers.count();

  project->addMapLayers( layers );

  QCOMPARE( project->mapLayers( true ).count(), layersCount - 2 );
  QCOMPARE( project->mapLayers( false ).count(), layersCount );

  QTemporaryFile projFile( QDir::temp().absoluteFilePath( "XXXXXX_test.qgs" ) );
  QVERIFY( projFile.open() );
  QVERIFY( project->write( projFile.fileName() ) );

  project = std::make_unique<QgsProject>();
  QgsSettingsRegistryCore::settingsLayerParallelLoading->setValue( false );

  QVERIFY( project->readProjectFile( projFile.fileName() ) );
  QCOMPARE( project->mapLayers( true ).count(), layersCount - 2 );
  QCOMPARE( project->mapLayers( false ).count(), layersCount );

  QVERIFY( project->write( projFile.fileName() ) );
  project = std::make_unique<QgsProject>();

  QgsSettingsRegistryCore::settingsLayerParallelLoading->setValue( true );
  QVERIFY( project->readProjectFile( projFile.fileName() ) );
  QCOMPARE( project->mapLayers( true ).count(), layersCount - 2 );
  QCOMPARE( project->mapLayers( false ).count(), layersCount );
}

QString getProjectXmlContent( const QString &projectPath )
{
  if ( projectPath.endsWith( ".qgz"_L1 ) )
  {
    QgsProjectArchive archive;
    if ( !archive.unzip( projectPath ) )
      return QString();

    const QString qgsFile = archive.projectFile();
    if ( qgsFile.isEmpty() )
      return QString();

    QFile file( qgsFile );
    if ( !file.open( QIODevice::ReadOnly ) )
      return QString();
    return file.readAll();
  }

  QFile file( projectPath );
  if ( !file.open( QIODevice::ReadOnly ) )
    return QString();
  return file.readAll();
}

QString getLayerSourceFromProjectXml( const QString &projectPath, const QString &layerName )
{
  // Get XML content
  const QString xmlContent = getProjectXmlContent( projectPath );
  if ( xmlContent.isEmpty() )
    return QString();

  // Parse XML
  QDomDocument doc;
  if ( !doc.setContent( xmlContent ) )
    return QString();

  // Find layer by name in XML
  const QDomNodeList layers = doc.elementsByTagName( u"maplayer"_s );
  for ( int i = 0; i < layers.count(); ++i )
  {
    const QDomElement layerElem = layers.at( i ).toElement();
    if ( layerElem.firstChildElement( u"layername"_s ).text() == layerName )
    {
      return layerElem.firstChildElement( u"datasource"_s ).text();
    }
  }
  return QString();
}

void TestQgsProject::testSymlinks1LayerRasterChange()
{
  // Verify that symlinked raster layer behaves well when target is changed

  // ++SETUP++
  // Create directory structure
  QTemporaryDir tempDir;
  const QString rootPath = tempDir.path();
  const QString projectDir = rootPath + "/projects/qgis/test1";
  const QString dataDir = rootPath + "/data";
  const QString projectPath = projectDir + "/proj.qgs";
  QDir().mkpath( projectDir );
  QDir().mkpath( dataDir );

  // Copy test rasters to data dir
  const QString testDataDir( TEST_DATA_DIR );
  const QStringList rasters = { "rnd_percentile_raster1_byte.tif", "rnd_percentile_raster2_byte.tif", "rnd_percentile_raster3_byte.tif" };
  for ( const QString &raster : rasters )
  {
    QVERIFY( QFile::copy( testDataDir + "/raster/" + raster, dataDir + "/" + raster ) );
  }

  // Create symlink pointing to raster1
  QVERIFY( QFile::link( dataDir + "/" + rasters[0], projectDir + "/latest.tif" ) );

  // Create project with layer pointing to symlink
  auto project = std::make_unique<QgsProject>();
  auto layer = std::make_unique<QgsRasterLayer>( "./latest.tif", u"Latest"_s, u"gdal"_s );
  project->addMapLayer( layer.release() );
  project->write( projectPath );
  project.reset();

  // ++Verify symlink changes are detected++
  // Initial state - points to raster1
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  QgsRasterLayer *loadedLayer = qobject_cast<QgsRasterLayer *>( project->mapLayersByName( u"Latest"_s ).at( 0 ) );
  QCOMPARE( QFileInfo( loadedLayer->source() ).canonicalFilePath(), dataDir + "/" + rasters[0] );
  project->write( projectPath );
  project.reset();
  // Change to raster2
  QFile::remove( projectDir + "/latest.tif" );
  QVERIFY( QFile::link( dataDir + "/" + rasters[1], projectDir + "/latest.tif" ) );
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  loadedLayer = qobject_cast<QgsRasterLayer *>( project->mapLayersByName( u"Latest"_s ).at( 0 ) );
  QCOMPARE( QFileInfo( loadedLayer->source() ).canonicalFilePath(), dataDir + "/" + rasters[1] );
  project->write( projectPath );
  project.reset();
  // Change to raster3
  QFile::remove( projectDir + "/latest.tif" );
  QVERIFY( QFile::link( dataDir + "/" + rasters[2], projectDir + "/latest.tif" ) );
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  loadedLayer = qobject_cast<QgsRasterLayer *>( project->mapLayersByName( u"Latest"_s ).at( 0 ) );
  QCOMPARE( QFileInfo( loadedLayer->source() ).canonicalFilePath(), dataDir + "/" + rasters[2] );
}

void TestQgsProject::testSymlinks2LayerFolder()
{
  // Verify that shapefile layer added via symlinked data folder
  // maintains correct relative paths in .qgz on save

  // ++SETUP++
  // Create directory structure (QGZ file)
  QTemporaryDir tempDir;
  const QString rootPath = tempDir.path();
  const QString testDataDir( TEST_DATA_DIR );
  const QString projectDir = rootPath + "/projects/qgis/test1";
  const QString dataDir = rootPath + "/data";
  const QString projectPath = projectDir + "/proj.qgz";
  QDir().mkpath( projectDir );
  QDir().mkpath( dataDir );

  // Copy shapefile components
  const QStringList components = { "dbf", "prj", "shp", "shx" };
  for ( const QString &ext : components )
  {
    QVERIFY( QFile::copy( testDataDir + "/points." + ext, dataDir + "/points." + ext ) );
  }

  // Symlink data folder
  QVERIFY( QFile::link( dataDir, projectDir + "/data" ) );

  // Create project with relative layer
  auto project = std::make_unique<QgsProject>();
  auto layer = std::make_unique<QgsVectorLayer>( "./data/points.shp", u"Points"_s, u"ogr"_s );
  project->addMapLayer( layer.release() );
  project->write( projectPath );
  project.reset();

  // ++Verify paths after re-opening++
  // XML datasource is "./data/points.shp" NOT "../../../data/points.shp"
  const QString layerSource = getLayerSourceFromProjectXml( projectPath, u"Points"_s );
  QCOMPARE( layerSource, u"./data/points.shp"_s );

  // Absolute layer source still in projectDir
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  QgsVectorLayer *loadedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayersByName( u"Points"_s ).at( 0 ) );
  QCOMPARE( loadedLayer->source(), projectDir + "/data/points.shp" );
}

void TestQgsProject::testSymlinks3LayerShapefile()
{
  // Verify that individually symlinked shapefile components
  // maintain correct relative paths in .qgs on save and shapefile edit

  // ++SETUP++
  // Create directory structure (QGS file)
  QTemporaryDir tempDir;
  const QString rootPath = tempDir.path();
  const QString testDataDir( TEST_DATA_DIR );
  const QString projectDir = rootPath + "/projects/qgis/test2";
  const QString dataDir = rootPath + "/data";
  const QString projectPath = projectDir + "/proj.qgs";
  QDir().mkpath( projectDir );
  QDir().mkpath( dataDir );

  // Copy and symlink shapefile components
  const QStringList components = { "dbf", "prj", "shp", "shx" };
  for ( const QString &ext : components )
  {
    QVERIFY( QFile::copy( testDataDir + "/points." + ext, dataDir + "/points." + ext ) );
    QVERIFY( QFile::link( dataDir + "/points." + ext, projectDir + "/points." + ext ) );
  }

  // Create project with relative layer
  auto project = std::make_unique<QgsProject>();
  auto layer = std::make_unique<QgsVectorLayer>( "./points.shp", u"Points"_s, u"ogr"_s );
  project->addMapLayer( layer.release() );
  project->write( projectPath );
  project.reset();

  // ++Verify paths after re-opening++
  // XML datasource is "./points.shp" NOT "../../../data/points.shp"
  const QString layerSource = getLayerSourceFromProjectXml( projectPath, u"Points"_s );
  QCOMPARE( layerSource, u"./points.shp"_s );

  // Absolute layer source still in projectDir
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  QgsVectorLayer *loadedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayersByName( u"Points"_s ).at( 0 ) );
  QCOMPARE( loadedLayer->source(), projectDir + "/points.shp" );

  // ++Verify that layer edit follows symlinks++
  const long initialCount = loadedLayer->featureCount();

  // Add new feature
  loadedLayer->startEditing();
  QgsFeature feat( loadedLayer->fields() );
  QgsGeometry geom = QgsGeometry::fromWkt( "POINT(1 2)" );
  feat.setGeometry( geom );
  loadedLayer->addFeature( feat );
  loadedLayer->commitChanges();
  project.reset();

  // Symlinks still exist and point to correct files
  for ( const QString &ext : components )
  {
    const QString symlink = projectDir + "/points." + ext;
    const QString target = dataDir + "/points." + ext;
    // Check symlink exists
    QVERIFY( QFileInfo( symlink ).isSymLink() );
    // Check canonical paths match
    QFileInfo symlinkInfo( symlink );
    QFileInfo targetInfo( target );
    QCOMPARE( symlinkInfo.canonicalFilePath(), targetInfo.canonicalFilePath() );
  }

  // Feature count has increased
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  loadedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayersByName( u"Points"_s ).at( 0 ) );
  QCOMPARE( loadedLayer->featureCount(), initialCount + 1 );
}

void TestQgsProject::testSymlinks4LayerShapefileBroken()
{
  // Verify that saving a new layer to location with existing broken
  // shapefile symlinks maintains the symlinks and properly saves the data

  // ++SETUP++
  // Create directory structure (QGS file)
  QTemporaryDir tempDir;
  const QString rootPath = tempDir.path();
  const QString projectDir = rootPath + "/projects/qgis/test3";
  const QString dataDir = rootPath + "/data";
  const QString projectPath = projectDir + "/proj.qgz";
  QDir().mkpath( projectDir );
  QDir().mkpath( dataDir );

  // Create broken symlinks for shapefile components, also symlink ".cpg" since it WILL be created
  const QStringList components = { "dbf", "prj", "shp", "shx", "cpg" };
  for ( const QString &ext : components )
  {
    QVERIFY( QFile::link( dataDir + "/points." + ext, projectDir + "/points." + ext ) );
  }

  // ++Verify that layer creation follows the (broken) symlink++
  // Create memory layer with single point
  auto memLayer = std::make_unique<QgsVectorLayer>( "Point", u"Points"_s, u"memory"_s );
  QgsFeature feat( memLayer->fields() );
  feat.setGeometry( QgsGeometry::fromWkt( "POINT(1 2)" ) );
  memLayer->startEditing();
  memLayer->addFeature( feat );
  memLayer->commitChanges();

  // Save memory layer to shapefile at symlink location
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = u"ESRI Shapefile"_s;
  QgsVectorFileWriter::writeAsVectorFormatV3( memLayer.get(), projectDir + "/points.shp", QgsCoordinateTransformContext(), options );

  // Create project with the layer
  auto project = std::make_unique<QgsProject>();
  auto layer = std::make_unique<QgsVectorLayer>( "./points.shp", u"Points"_s, u"ogr"_s );
  project->addMapLayer( layer.release() );
  project->write( projectPath );
  project.reset();

  // Verify symlinks and data
  for ( const QString &ext : components )
  {
    const QString symlink = projectDir + "/points." + ext;
    const QString target = dataDir + "/points." + ext;
    // Check symlink exists
    QVERIFY( QFileInfo( symlink ).isSymLink() );
    // Check canonical paths match
    QFileInfo symlinkInfo( symlink );
    QFileInfo targetInfo( target );
    QCOMPARE( symlinkInfo.canonicalFilePath(), targetInfo.canonicalFilePath() );
  }

  // Verify layer has 1 feature
  project = std::make_unique<QgsProject>();
  project->read( projectPath );
  QgsVectorLayer *loadedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayersByName( u"Points"_s ).at( 0 ) );
  QCOMPARE( loadedLayer->featureCount(), 1L );
}

void TestQgsProject::testSymlinks5ProjectFile()
{
  // Verify that symlinked project file maintains relative paths
  // and test writing broken project links

  // ++SETUP++
  // Create directory structure
  QTemporaryDir tempDir;
  const QString rootPath = tempDir.path();
  const QString projectDir = rootPath + "/projects/qgis/test4";
  const QString symlinkprojDir = rootPath + "/symlinkproj";
  QDir().mkpath( projectDir );
  QDir().mkpath( symlinkprojDir );

  // Copy shapefile components to project dir
  const QString testDataDir( TEST_DATA_DIR );
  const QStringList components = { "dbf", "prj", "shp", "shx" };
  for ( const QString &ext : components )
  {
    QFile::copy( testDataDir + "/points." + ext, projectDir + "/points." + ext );
  }

  // Create initial project in project dir
  const QString originalPath = projectDir + "/project.qgs";
  const QString originalAttachPath = projectDir + "/project_attachments.zip";
  auto project = std::make_unique<QgsProject>();
  auto layer = std::make_unique<QgsVectorLayer>( "./points.shp", u"Points"_s, u"ogr"_s );
  project->addMapLayer( layer.release() );
  project->write( originalPath );
  project.reset();

  // ++Verify that moved project behaves well++
  // Move project file and create symlink
  QVERIFY( QFile::rename( originalPath, symlinkprojDir + "/project.qgs" ) );
  QVERIFY( QFile::rename( originalAttachPath, symlinkprojDir + "/project_attachments.zip" ) );
  QVERIFY( QFile::link( symlinkprojDir + "/project.qgs", originalPath ) );
  QVERIFY( QFile::link( symlinkprojDir + "/project_attachments.zip", originalAttachPath ) );

  // Open symlinked project and verify paths
  project = std::make_unique<QgsProject>();
  project->read( originalPath );
  QgsVectorLayer *loadedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayersByName( u"Points"_s ).at( 0 ) );
  QCOMPARE( loadedLayer->source(), projectDir + "/points.shp" );

  // Save and verify XML content
  project->write( originalPath );
  const QString layerSource = getLayerSourceFromProjectXml( originalPath, u"Points"_s );
  QCOMPARE( layerSource, u"./points.shp"_s );

  // ++Change project settings, verify symlinks still good++
  project->setDistanceUnits( Qgis::DistanceUnit::NauticalMiles );
  project->write( originalPath );

  // Verify symlinks and canonical paths
  const QStringList symlinks = { originalPath, originalAttachPath };
  for ( const QString &symlink : symlinks )
  {
    QVERIFY( QFileInfo( symlink ).isSymLink() );
    QFileInfo symlinkInfo( symlink );
    QFileInfo targetInfo( symlinkprojDir + "/" + QFileInfo( symlink ).fileName() );
    QCOMPARE( symlinkInfo.canonicalFilePath(), targetInfo.canonicalFilePath() );
  }

  // ++Break symlinks and create new project++
  // Remove symlink destinations
  QVERIFY( QFile::remove( symlinkprojDir + "/project.qgs" ) );
  QVERIFY( QFile::remove( symlinkprojDir + "/project_attachments.zip" ) );

  // Create a new project, writing to the broken symlink
  project = std::make_unique<QgsProject>();
  layer = std::make_unique<QgsVectorLayer>( "./points.shp", u"Points"_s, u"ogr"_s );
  project->addMapLayer( layer.release() );
  project->write( originalPath );

  // Verify symlinks are now active and well-behaved
  for ( const QString &symlink : symlinks )
  {
    QVERIFY( QFileInfo( symlink ).isSymLink() );
    QFileInfo symlinkInfo( symlink );
    QFileInfo targetInfo( symlinkprojDir + "/" + QFileInfo( symlink ).fileName() );
    QCOMPARE( symlinkInfo.canonicalFilePath(), targetInfo.canonicalFilePath() );
  }
}

void TestQgsProject::testSymlinks6ProjectFolder()
{
  // Replicate this test: python/test_qgsproject.py:testSymbolicLinkInProjectPath
  // Check functionality if the immediate parent is a symlink

  // ++SETUP++
  // Create directory structure
  QTemporaryDir tempDir;
  const QString rootPath = tempDir.path();
  const QString projectDir = rootPath + "/projects/qgis/test4";
  const QString symlinkprojparentDir = rootPath + "/another/directory";
  QDir().mkpath( projectDir );
  QDir().mkpath( symlinkprojparentDir );

  // Copy shapefile components to project dir
  const QString testDataDir( TEST_DATA_DIR );
  const QStringList components = { "dbf", "prj", "shp", "shx" };
  for ( const QString &ext : components )
  {
    QFile::copy( testDataDir + "/points." + ext, projectDir + "/points." + ext );
  }

  // Create initial project in project dir
  const QString originalPath = projectDir + "/project.qgs";
  auto project = std::make_unique<QgsProject>();
  auto layer = std::make_unique<QgsVectorLayer>( "./points.shp", u"Points"_s, u"ogr"_s );
  project->addMapLayer( layer.release() );
  project->write( originalPath );
  project.reset();

  // Create a new temporary directory and a symbolic link to the original folder
  const QString symlinkprojDir = symlinkprojparentDir + "/symlink_projdir";
  QVERIFY( QFile::link( projectDir, symlinkprojDir ) );
  const QString symlinkprojPath = symlinkprojDir + "/project.qgs";

  // ++Open the project through a symlink and re-save++
  project = std::make_unique<QgsProject>();
  QVERIFY( project->read( symlinkprojPath ) );
  QVERIFY( project->write( symlinkprojPath ) );
  project.reset();

  // ++Verify paths after re-opening++
  // XML datasource is still "./points.shp"
  const QString layerSource = getLayerSourceFromProjectXml( symlinkprojPath, u"Points"_s );
  QCOMPARE( layerSource, u"./points.shp"_s );
  // Absolute layer source does NOT resolve the symlink
  project = std::make_unique<QgsProject>();
  project->read( symlinkprojPath );
  QgsVectorLayer *loadedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayersByName( u"Points"_s ).at( 0 ) );
  QCOMPARE( loadedLayer->source(), symlinkprojDir + "/points.shp" );
}

void TestQgsProject::regression60100()
{
  /*
  * Regression test for QGIS issue #60100 (https://github.com/qgis/QGIS/issues/60100)
  * This test ensures that when saving a QGIS project with relative paths,
  * the correct layer datasource is preserved, even when the current working
  * directory (CWD) contains a file with the same name as the layer datasource.
  *
  * Previous behavior:
  * - If a file with the same name as a layer datasource existed in the CWD,
  *   the layer path in the saved project would point to the file in the CWD,
  *   rather than the intended file in the project directory (PROJDIR).
  *
  * Test steps:
  * 1. Create a temporary directory structure with two subfolders: WORKDIR and PROJDIR.
  * 2. Copy a `points.geojson` file to both WORKDIR and PROJDIR.
  * 3. Create a new QGIS project in PROJDIR and add the `points.geojson` file from PROJDIR as a layer.
  * 4. Change the working directory to WORKDIR and save the project.
  * 5. Verify that the saved project references the correct datasource (`./points.geojson` in PROJDIR)
  *    and does not erroneously reference the file in WORKDIR.
  */
  // Create directory structure with 2 subfolders
  const QTemporaryDir baseDir;
  const QDir base( baseDir.path() );
  base.mkdir( u"WORKDIR"_s );
  base.mkdir( u"PROJDIR"_s );
  const QString workDirPath = baseDir.path() + u"/WORKDIR"_s;
  const QString projDirPath = baseDir.path() + u"/PROJDIR"_s;

  // Save our old CWD and switch to the new WORKDIR
  const QString oldCWD = QDir::currentPath();
  QVERIFY( QDir::setCurrent( workDirPath ) );

  // Copy points.geojson to both subfolders
  const QString testDataDir( TEST_DATA_DIR );
  const QString pointsPath = testDataDir + u"/points.geojson"_s;
  QFile::copy( pointsPath, workDirPath + u"/points.geojson"_s );
  QFile::copy( pointsPath, projDirPath + u"/points.geojson"_s );

  // Create a new/empty project in PROJDIR
  const QString projectPath = projDirPath + u"/project.qgs"_s;
  auto project = std::make_unique<QgsProject>();

  // Add the local points.geojson (in PROJDIR) as a layer
  auto layer = std::make_unique<QgsVectorLayer>(
    projDirPath + u"/points.geojson"_s,
    u"Test Points"_s,
    u"ogr"_s
  );
  project->addMapLayer( layer.release() );

  // Write (save) the project to disk. This used to pick up the WRONG file and save it to the proj.
  project->write( projectPath );

  // Restore old working directory
  QVERIFY( QDir::setCurrent( oldCWD ) );

  // Verify the layer path in the project file
  QDomDocument doc;
  QFile projectFile( projectPath );
  bool res = projectFile.open( QIODevice::ReadOnly );
  Q_ASSERT( res );
  res = static_cast<bool>( doc.setContent( &projectFile ) );
  Q_ASSERT( res );
  projectFile.close();

  const QDomElement docElem = doc.documentElement();
  const QDomElement layersElem = docElem.firstChildElement( u"projectlayers"_s );
  QDomElement layerElem = layersElem.firstChildElement();
  while ( !layerElem.isNull() )
  {
    const QString layerSource = layerElem.firstChildElement( u"datasource"_s ).text();
    // Should NOT be "../WORKDIR/points.geojson"
    QCOMPARE( layerSource, u"./points.geojson"_s );
    layerElem = layerElem.nextSiblingElement();
  }
}

void TestQgsProject::testAutorefreshModeRestore()
{
  QgsVectorLayer vl( u"Point?field=fldtxt:string"_s, u"layer"_s, u"memory"_s );
  QgsVectorLayer vl2( u"Point?field=fldtxt:string"_s, u"layer"_s, u"memory"_s );
  QDomDocument doc( u"testdoc"_s );
  QDomElement elem = doc.createElement( u"maplayer"_s );
  vl.setAutoRefreshInterval( 123 );
  vl.setAutoRefreshMode( Qgis::AutoRefreshMode::RedrawOnly );
  QgsReadWriteContext ctx;
  vl.writeLayerXml( elem, doc, ctx );

  vl2.readLayerXml( elem, ctx );
  QCOMPARE( vl2.autoRefreshInterval(), 123 );
  QCOMPARE( vl2.autoRefreshMode(), Qgis::AutoRefreshMode::RedrawOnly );
  QVERIFY( vl2.mRefreshTimer->isActive() );
}

QGSTEST_MAIN( TestQgsProject )
#include "testqgsproject.moc"
