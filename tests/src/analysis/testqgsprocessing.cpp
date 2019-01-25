/***************************************************************************
                         testqgsprocessing.cpp
                         ---------------------
    begin                : January 2017
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

#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingparametertype.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsnativealgorithms.h"
#include <QObject>
#include <QtTest/QSignalSpy>
#include <QList>
#include "qgis.h"
#include "qgstest.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgsproject.h"
#include "qgspoint.h"
#include "qgsgeometry.h"
#include "qgsvectorfilewriter.h"
#include "qgsexpressioncontext.h"
#include "qgsxmlutils.h"
#include "qgsreferencedgeometry.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"


class DummyAlgorithm : public QgsProcessingAlgorithm
{
  public:

    DummyAlgorithm( const QString &name ) : mName( name ) { mFlags = QgsProcessingAlgorithm::flags(); }

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override {}
    QString name() const override { return mName; }
    QString displayName() const override { return mName; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override { return QVariantMap(); }

    Flags flags() const override { return mFlags; }
    DummyAlgorithm *createInstance() const override { return new DummyAlgorithm( name() ); }

    QString mName;

    Flags mFlags;

    void checkParameterVals()
    {
      addParameter( new QgsProcessingParameterString( "p1" ) );
      QVariantMap params;
      QgsProcessingContext context;

      QVERIFY( !checkParameterValues( params, context ) );
      params.insert( "p1", "a" );
      QVERIFY( checkParameterValues( params, context ) );
      // optional param
      addParameter( new QgsProcessingParameterString( "p2", QString(), QVariant(), false, true ) );
      QVERIFY( checkParameterValues( params, context ) );
      params.insert( "p2", "a" );
      QVERIFY( checkParameterValues( params, context ) );
    }

    void runParameterChecks()
    {
      QVERIFY( parameterDefinitions().isEmpty() );
      QVERIFY( addParameter( new QgsProcessingParameterBoolean( "p1" ) ) );
      QCOMPARE( parameterDefinitions().count(), 1 );
      QCOMPARE( parameterDefinitions().at( 0 )->name(), QString( "p1" ) );
      QCOMPARE( parameterDefinitions().at( 0 )->algorithm(), this );

      QVERIFY( !addParameter( nullptr ) );
      QCOMPARE( parameterDefinitions().count(), 1 );
      // duplicate name!
      QgsProcessingParameterBoolean *p2 = new QgsProcessingParameterBoolean( "p1" );
      QVERIFY( !addParameter( p2 ) );
      QCOMPARE( parameterDefinitions().count(), 1 );

      QCOMPARE( parameterDefinition( "p1" ), parameterDefinitions().at( 0 ) );
      // parameterDefinition should be case insensitive
      QCOMPARE( parameterDefinition( "P1" ), parameterDefinitions().at( 0 ) );
      QVERIFY( !parameterDefinition( "invalid" ) );

      QCOMPARE( countVisibleParameters(), 1 );
      QgsProcessingParameterBoolean *p3 = new QgsProcessingParameterBoolean( "p3" );
      QVERIFY( addParameter( p3 ) );
      QCOMPARE( countVisibleParameters(), 2 );
      QgsProcessingParameterBoolean *p4 = new QgsProcessingParameterBoolean( "p4" );
      p4->setFlags( QgsProcessingParameterDefinition::FlagHidden );
      QVERIFY( addParameter( p4 ) );
      QCOMPARE( countVisibleParameters(), 2 );


      //destination styleparameters
      QVERIFY( destinationParameterDefinitions().isEmpty() );
      QgsProcessingParameterFeatureSink *p5 = new QgsProcessingParameterFeatureSink( "p5" );
      QVERIFY( addParameter( p5, false ) );
      QCOMPARE( destinationParameterDefinitions(), QgsProcessingParameterDefinitions() << p5 );
      QgsProcessingParameterFeatureSink *p6 = new QgsProcessingParameterFeatureSink( "p6" );
      QVERIFY( addParameter( p6, false ) );
      QCOMPARE( destinationParameterDefinitions(), QgsProcessingParameterDefinitions() << p5 << p6 );

      // remove parameter
      removeParameter( "non existent" );
      removeParameter( "p6" );
      QCOMPARE( destinationParameterDefinitions(), QgsProcessingParameterDefinitions() << p5 );
      removeParameter( "p5" );
      QVERIFY( destinationParameterDefinitions().isEmpty() );

      // try with auto output creation
      QgsProcessingParameterVectorDestination *p7 = new QgsProcessingParameterVectorDestination( "p7", "my output" );
      QVERIFY( addParameter( p7 ) );
      QCOMPARE( destinationParameterDefinitions(), QgsProcessingParameterDefinitions() << p7 );
      QVERIFY( outputDefinition( "p7" ) );
      QCOMPARE( outputDefinition( "p7" )->name(), QStringLiteral( "p7" ) );
      QCOMPARE( outputDefinition( "p7" )->type(), QStringLiteral( "outputVector" ) );
      QCOMPARE( outputDefinition( "p7" )->description(), QStringLiteral( "my output" ) );

      // duplicate output name
      QVERIFY( addOutput( new QgsProcessingOutputVectorLayer( "p8" ) ) );
      QgsProcessingParameterVectorDestination *p8 = new QgsProcessingParameterVectorDestination( "p8" );
      // this should fail - it would result in a duplicate output name
      QVERIFY( !addParameter( p8 ) );

      // default vector format extension
      QgsProcessingParameterFeatureSink *sinkParam = new QgsProcessingParameterFeatureSink( "sink" );
      QCOMPARE( sinkParam->defaultFileExtension(), QStringLiteral( "shp" ) ); // before alg is accessible
      QVERIFY( !sinkParam->algorithm() );
      QVERIFY( !sinkParam->provider() );
      QVERIFY( addParameter( sinkParam ) );
      QCOMPARE( sinkParam->defaultFileExtension(), QStringLiteral( "shp" ) );
      QCOMPARE( sinkParam->algorithm(), this );
      QVERIFY( !sinkParam->provider() );

      // default raster format extension
      QgsProcessingParameterRasterDestination *rasterParam = new QgsProcessingParameterRasterDestination( "raster" );
      QCOMPARE( rasterParam->defaultFileExtension(), QStringLiteral( "tif" ) ); // before alg is accessible
      QVERIFY( addParameter( rasterParam ) );
      QCOMPARE( rasterParam->defaultFileExtension(), QStringLiteral( "tif" ) );

      // should allow parameters with same name but different case (required for grass provider)
      QgsProcessingParameterBoolean *p1C = new QgsProcessingParameterBoolean( "P1" );
      QVERIFY( addParameter( p1C ) );
      QCOMPARE( parameterDefinitions().count(), 8 );

      // parameterDefinition should be case insensitive, but prioritize correct case matches
      QCOMPARE( parameterDefinition( "p1" ), parameterDefinitions().at( 0 ) );
      QCOMPARE( parameterDefinition( "P1" ), parameterDefinitions().at( 7 ) );
    }

    void runParameterChecks2()
    {
      // default vector format extension, taken from provider
      QgsProcessingParameterFeatureSink *sinkParam = new QgsProcessingParameterFeatureSink( "sink2" );
      QCOMPARE( sinkParam->defaultFileExtension(), QStringLiteral( "shp" ) ); // before alg is accessible
      QVERIFY( !sinkParam->algorithm() );
      QVERIFY( !sinkParam->provider() );
      QVERIFY( addParameter( sinkParam ) );
      QCOMPARE( sinkParam->defaultFileExtension(), QStringLiteral( "xshp" ) );
      QCOMPARE( sinkParam->algorithm(), this );
      QCOMPARE( sinkParam->provider(), provider() );

      // default raster format extension
      QgsProcessingParameterRasterDestination *rasterParam = new QgsProcessingParameterRasterDestination( "raster2" );
      QCOMPARE( rasterParam->defaultFileExtension(), QStringLiteral( "tif" ) ); // before alg is accessible
      QVERIFY( addParameter( rasterParam ) );
      QCOMPARE( rasterParam->defaultFileExtension(), QStringLiteral( "pcx" ) );
    }

    void runOutputChecks()
    {
      QVERIFY( outputDefinitions().isEmpty() );
      QVERIFY( addOutput( new QgsProcessingOutputVectorLayer( "p1" ) ) );
      QCOMPARE( outputDefinitions().count(), 1 );
      QCOMPARE( outputDefinitions().at( 0 )->name(), QString( "p1" ) );

      QVERIFY( !addOutput( nullptr ) );
      QCOMPARE( outputDefinitions().count(), 1 );
      // duplicate name!
      QgsProcessingOutputVectorLayer *p2 = new QgsProcessingOutputVectorLayer( "p1" );
      QVERIFY( !addOutput( p2 ) );
      QCOMPARE( outputDefinitions().count(), 1 );

      QCOMPARE( outputDefinition( "p1" ), outputDefinitions().at( 0 ) );
      // parameterDefinition should be case insensitive
      QCOMPARE( outputDefinition( "P1" ), outputDefinitions().at( 0 ) );
      QVERIFY( !outputDefinition( "invalid" ) );

      QVERIFY( !hasHtmlOutputs() );
      QgsProcessingOutputHtml *p3 = new QgsProcessingOutputHtml( "p3" );
      QVERIFY( addOutput( p3 ) );
      QVERIFY( hasHtmlOutputs() );
    }

    void runValidateInputCrsChecks()
    {
      addParameter( new QgsProcessingParameterMapLayer( "p1" ) );
      addParameter( new QgsProcessingParameterMapLayer( "p2" ) );
      QVariantMap parameters;

      QgsVectorLayer *layer3111 = new QgsVectorLayer( "Point?crs=epsg:3111", "v1", "memory" );
      QgsProject p;
      p.addMapLayer( layer3111 );

      QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
      QString raster1 = testDataDir + "tenbytenraster.asc";
      QFileInfo fi1( raster1 );
      QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
      QVERIFY( r1->isValid() );
      p.addMapLayer( r1 );

      QgsVectorLayer *layer4326 = new QgsVectorLayer( "Point?crs=epsg:4326", "v1", "memory" );
      p.addMapLayer( layer4326 );

      QgsProcessingContext context;
      context.setProject( &p );

      // flag not set
      mFlags = nullptr;
      parameters.insert( "p1", QVariant::fromValue( layer3111 ) );
      QVERIFY( validateInputCrs( parameters, context ) );
      mFlags = FlagRequiresMatchingCrs;
      QVERIFY( validateInputCrs( parameters, context ) );

      // two layers, different crs
      parameters.insert( "p2", QVariant::fromValue( layer4326 ) );
      // flag not set
      mFlags = nullptr;
      QVERIFY( validateInputCrs( parameters, context ) );
      mFlags = FlagRequiresMatchingCrs;
      QVERIFY( !validateInputCrs( parameters, context ) );

      // raster layer
      parameters.remove( "p2" );
      addParameter( new QgsProcessingParameterRasterLayer( "p3" ) );
      parameters.insert( "p3", QVariant::fromValue( r1 ) );
      QVERIFY( !validateInputCrs( parameters, context ) );

      // feature source
      parameters.remove( "p3" );
      addParameter( new QgsProcessingParameterFeatureSource( "p4" ) );
      parameters.insert( "p4", layer4326->id() );
      QVERIFY( !validateInputCrs( parameters, context ) );

      parameters.remove( "p4" );
      addParameter( new QgsProcessingParameterMultipleLayers( "p5" ) );
      parameters.insert( "p5", QVariantList() << layer4326->id() << r1->id() );
      QVERIFY( !validateInputCrs( parameters, context ) );

      // extent
      parameters.clear();
      parameters.insert( "p1", QVariant::fromValue( layer3111 ) );
      addParameter( new QgsProcessingParameterExtent( "extent" ) );
      parameters.insert( "extent", QgsReferencedRectangle( QgsRectangle( 1, 2, 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
      QVERIFY( !validateInputCrs( parameters, context ) );
      parameters.insert( "extent", QgsReferencedRectangle( QgsRectangle( 1, 2, 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ) );
      QVERIFY( validateInputCrs( parameters, context ) );

      // point
      parameters.clear();
      parameters.insert( "p1", QVariant::fromValue( layer3111 ) );
      addParameter( new QgsProcessingParameterPoint( "point" ) );
      parameters.insert( "point", QgsReferencedPointXY( QgsPointXY( 1, 2 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
      QVERIFY( !validateInputCrs( parameters, context ) );
      parameters.insert( "point", QgsReferencedPointXY( QgsPointXY( 1, 2 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ) );
      QVERIFY( validateInputCrs( parameters, context ) );
    }

    void runAsPythonCommandChecks()
    {
      addParameter( new QgsProcessingParameterString( "p1" ) );
      addParameter( new QgsProcessingParameterString( "p2" ) );
      QgsProcessingParameterString *hidden = new QgsProcessingParameterString( "p3" );
      hidden->setFlags( QgsProcessingParameterDefinition::FlagHidden );
      addParameter( hidden );

      QVariantMap params;
      QgsProcessingContext context;

      QCOMPARE( asPythonCommand( params, context ), QStringLiteral( "processing.run(\"test\", {})" ) );
      params.insert( "p1", "a" );
      QCOMPARE( asPythonCommand( params, context ), QStringLiteral( "processing.run(\"test\", {'p1':'a'})" ) );
      params.insert( "p2", QVariant() );
      QCOMPARE( asPythonCommand( params, context ), QStringLiteral( "processing.run(\"test\", {'p1':'a','p2':None})" ) );
      params.insert( "p2", "b" );
      QCOMPARE( asPythonCommand( params, context ), QStringLiteral( "processing.run(\"test\", {'p1':'a','p2':'b'})" ) );

      // hidden, shouldn't be shown
      params.insert( "p3", "b" );
      QCOMPARE( asPythonCommand( params, context ), QStringLiteral( "processing.run(\"test\", {'p1':'a','p2':'b'})" ) );
    }

    void addDestParams()
    {
      QgsProcessingParameterFeatureSink *sinkParam1 = new QgsProcessingParameterFeatureSink( "supports" );
      sinkParam1->setSupportsNonFileBasedOutput( true );
      addParameter( sinkParam1 );
      QgsProcessingParameterFeatureSink *sinkParam2 = new QgsProcessingParameterFeatureSink( "non_supports" );
      sinkParam2->setSupportsNonFileBasedOutput( false );
      addParameter( sinkParam2 );
    }

};

//dummy provider for testing
class DummyProvider : public QgsProcessingProvider
{
  public:

    DummyProvider( const QString &id ) : mId( id ) {}

    QString id() const override { return mId; }

    QString name() const override { return "dummy"; }

    void unload() override { if ( unloaded ) { *unloaded = true; } }

    QString defaultVectorFileExtension( bool ) const override
    {
      return "xshp"; // shape-X. Just like shapefiles, but to the max!
    }

    QString defaultRasterFileExtension() const override
    {
      return "pcx"; // next-gen raster storage
    }

    bool supportsNonFileBasedOutput() const override
    {
      return supportsNonFileOutputs;
    }

    bool isActive() const override
    {
      return active;
    }

    bool active = true;
    bool *unloaded = nullptr;
    bool supportsNonFileOutputs = false;

  protected:

    void loadAlgorithms() override
    {
      QVERIFY( addAlgorithm( new DummyAlgorithm( "alg1" ) ) );
      QVERIFY( addAlgorithm( new DummyAlgorithm( "alg2" ) ) );

      //dupe name
      QgsProcessingAlgorithm *a = new DummyAlgorithm( "alg1" );
      QVERIFY( !addAlgorithm( a ) );
      delete a;

      QVERIFY( !addAlgorithm( nullptr ) );
    }

    QString mId;

    friend class TestQgsProcessing;
};

class DummyProviderNoLoad : public DummyProvider
{
  public:

    DummyProviderNoLoad( const QString &id ) : DummyProvider( id ) {}

    bool load() override
    {
      return false;
    }

};



class DummyAlgorithm2 : public QgsProcessingAlgorithm
{
  public:

    DummyAlgorithm2( const QString &name ) : mName( name ) { mFlags = QgsProcessingAlgorithm::flags(); }

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override
    {
      addParameter( new QgsProcessingParameterVectorDestination( QStringLiteral( "vector_dest" ) ) );
      addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "raster_dest" ) ) );
      addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "sink" ) ) );
    }
    QString name() const override { return mName; }
    QString displayName() const override { return mName; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override { return QVariantMap(); }

    Flags flags() const override { return mFlags; }
    DummyAlgorithm2 *createInstance() const override { return new DummyAlgorithm2( name() ); }

    QString mName;

    Flags mFlags;

};


class DummyProvider3 : public QgsProcessingProvider
{
  public:

    DummyProvider3()  = default;
    QString id() const override { return QStringLiteral( "dummy3" ); }
    QString name() const override { return QStringLiteral( "dummy3" ); }

    QStringList supportedOutputVectorLayerExtensions() const override
    {
      return QStringList() << QStringLiteral( "mif" ) << QStringLiteral( "tab" );
    }

    QStringList supportedOutputTableExtensions() const override
    {
      return QStringList() << QStringLiteral( "dbf" );
    }

    QStringList supportedOutputRasterLayerExtensions() const override
    {
      return QStringList() << QStringLiteral( "mig" ) << QStringLiteral( "asc" );
    }

    void loadAlgorithms() override
    {
      QVERIFY( addAlgorithm( new DummyAlgorithm2( "alg1" ) ) );
    }

};

class DummyProvider4 : public QgsProcessingProvider
{
  public:

    DummyProvider4()  = default;
    QString id() const override { return QStringLiteral( "dummy4" ); }
    QString name() const override { return QStringLiteral( "dummy4" ); }

    bool supportsNonFileBasedOutput() const override
    {
      return false;
    }

    QStringList supportedOutputVectorLayerExtensions() const override
    {
      return QStringList() << QStringLiteral( "mif" );
    }

    QStringList supportedOutputRasterLayerExtensions() const override
    {
      return QStringList() << QStringLiteral( "mig" );
    }

    void loadAlgorithms() override
    {
      QVERIFY( addAlgorithm( new DummyAlgorithm2( "alg1" ) ) );
    }

};

class DummyParameterType : public QgsProcessingParameterType
{


    // QgsProcessingParameterType interface
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override
    {
      return new QgsProcessingParameterString( name );
    }

    QString description() const override
    {
      return QStringLiteral( "Description" );
    }

    QString name() const override
    {
      return QStringLiteral( "ParamType" );
    }

    QString id() const override
    {
      return QStringLiteral( "paramType" );
    }
};

class TestQgsProcessing: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void instance();
    void addProvider();
    void providerById();
    void removeProvider();
    void compatibleLayers();
    void normalizeLayerSource();
    void context();
    void mapLayers();
    void mapLayerFromStore();
    void mapLayerFromString();
    void algorithm();
    void features();
    void uniqueValues();
    void createIndex();
    void parseDestinationString();
    void createFeatureSink();
    void parameters();
    void algorithmParameters();
    void algorithmOutputs();
    void parameterGeneral();
    void parameterBoolean();
    void parameterCrs();
    void parameterLayer();
    void parameterExtent();
    void parameterPoint();
    void parameterFile();
    void parameterMatrix();
    void parameterLayerList();
    void parameterNumber();
    void parameterDistance();
    void parameterRange();
    void parameterRasterLayer();
    void parameterEnum();
    void parameterString();
    void parameterAuthConfig();
    void parameterExpression();
    void parameterField();
    void parameterVectorLayer();
    void parameterMeshLayer();
    void parameterFeatureSource();
    void parameterFeatureSink();
    void parameterVectorOut();
    void parameterRasterOut();
    void parameterFileOut();
    void parameterFolderOut();
    void parameterBand();
    void checkParamValues();
    void combineLayerExtent();
    void processingFeatureSource();
    void processingFeatureSink();
    void algorithmScope();
    void validateInputCrs();
    void generateIteratingDestination();
    void asPythonCommand();
    void modelerAlgorithm();
    void modelExecution();
    void modelWithProviderWithLimitedTypes();
    void modelVectorOutputIsCompatibleType();
    void modelAcceptableValues();
    void tempUtils();
    void convertCompatible();
    void create();
    void combineFields();
    void fieldNamesToIndices();
    void indicesToFields();
    void stringToPythonLiteral();
    void defaultExtensionsForProvider();
    void supportedExtensions();
    void supportsNonFileBasedOutput();
    void addParameterType();
    void removeParameterType();
    void parameterTypes();
    void parameterType();

  private:

};

void TestQgsProcessing::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsSettings settings;
  settings.remove( QStringLiteral( "Processing/DefaultOutputVectorLayerExt" ), QgsSettings::Core );
  settings.remove( QStringLiteral( "Processing/DefaultOutputRasterLayerExt" ), QgsSettings::Core );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestQgsProcessing::cleanupTestCase()
{
  QFile::remove( QDir::tempPath() + "/create_feature_sink.tab" );
  QgsVectorFileWriter::deleteShapeFile( QDir::tempPath() + "/create_feature_sink2.gpkg" );

  QgsApplication::exitQgis();
}

void TestQgsProcessing::instance()
{
  // test that application has a registry instance
  QVERIFY( QgsApplication::processingRegistry() );
}

void TestQgsProcessing::addProvider()
{
  QgsProcessingRegistry r;
  QSignalSpy spyProviderAdded( &r, &QgsProcessingRegistry::providerAdded );

  QVERIFY( r.providers().isEmpty() );

  QVERIFY( !r.addProvider( nullptr ) );

  // add a provider
  DummyProvider *p = new DummyProvider( "p1" );
  QVERIFY( r.addProvider( p ) );
  QCOMPARE( r.providers(), QList< QgsProcessingProvider * >() << p );
  QCOMPARE( spyProviderAdded.count(), 1 );
  QCOMPARE( spyProviderAdded.last().at( 0 ).toString(), QString( "p1" ) );

  //try adding another provider
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( r.addProvider( p2 ) );
  QCOMPARE( r.providers().toSet(), QSet< QgsProcessingProvider * >() << p << p2 );
  QCOMPARE( spyProviderAdded.count(), 2 );
  QCOMPARE( spyProviderAdded.last().at( 0 ).toString(), QString( "p2" ) );

  //try adding a provider with duplicate id
  DummyProvider *p3 = new DummyProvider( "p2" );
  QVERIFY( !r.addProvider( p3 ) );
  QCOMPARE( r.providers().toSet(), QSet< QgsProcessingProvider * >() << p << p2 );
  QCOMPARE( spyProviderAdded.count(), 2 );

  // test that adding a provider which does not load means it is not added to registry
  DummyProviderNoLoad *p4 = new DummyProviderNoLoad( "p4" );
  QVERIFY( !r.addProvider( p4 ) );
  QCOMPARE( r.providers().toSet(), QSet< QgsProcessingProvider * >() << p << p2 );
  QCOMPARE( spyProviderAdded.count(), 2 );
}

void TestQgsProcessing::providerById()
{
  QgsProcessingRegistry r;

  // no providers
  QVERIFY( !r.providerById( "p1" ) );

  // add a provider
  DummyProvider *p = new DummyProvider( "p1" );
  QVERIFY( r.addProvider( p ) );
  QCOMPARE( r.providerById( "p1" ), p );
  QVERIFY( !r.providerById( "p2" ) );

  //try adding another provider
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( r.addProvider( p2 ) );
  QCOMPARE( r.providerById( "p1" ), p );
  QCOMPARE( r.providerById( "p2" ), p2 );
  QVERIFY( !r.providerById( "p3" ) );
}

void TestQgsProcessing::removeProvider()
{
  QgsProcessingRegistry r;
  QSignalSpy spyProviderRemoved( &r, &QgsProcessingRegistry::providerRemoved );

  QVERIFY( !r.removeProvider( nullptr ) );
  QVERIFY( !r.removeProvider( "p1" ) );
  // provider not in registry
  DummyProvider *p = new DummyProvider( "p1" );
  QVERIFY( !r.removeProvider( p ) );
  QCOMPARE( spyProviderRemoved.count(), 0 );

  // add some providers
  QVERIFY( r.addProvider( p ) );
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( r.addProvider( p2 ) );

  // remove one by pointer
  bool unloaded = false;
  p->unloaded = &unloaded;
  QVERIFY( r.removeProvider( p ) );
  QCOMPARE( spyProviderRemoved.count(), 1 );
  QCOMPARE( spyProviderRemoved.last().at( 0 ).toString(), QString( "p1" ) );
  QCOMPARE( r.providers(), QList< QgsProcessingProvider * >() << p2 );

  //test that provider was unloaded
  QVERIFY( unloaded );

  // should fail, already removed
  QVERIFY( !r.removeProvider( "p1" ) );

  // remove one by id
  QVERIFY( r.removeProvider( "p2" ) );
  QCOMPARE( spyProviderRemoved.count(), 2 );
  QCOMPARE( spyProviderRemoved.last().at( 0 ).toString(), QString( "p2" ) );
  QVERIFY( r.providers().isEmpty() );
}

void TestQgsProcessing::compatibleLayers()
{
  QgsProject p;

  // add a bunch of layers to a project
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QString raster3 = testDataDir + "/raster/band1_float32_noct_epsg4326.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  QFileInfo fi2( raster2 );
  QgsRasterLayer *r2 = new QgsRasterLayer( fi2.filePath(), "ar2" );
  QVERIFY( r2->isValid() );
  QFileInfo fi3( raster3 );
  QgsRasterLayer *r3 = new QgsRasterLayer( fi3.filePath(), "zz" );
  QVERIFY( r3->isValid() );

  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Point", "v1", "memory" );
  QgsVectorLayer *v3 = new QgsVectorLayer( "LineString", "v3", "memory" );
  QgsVectorLayer *v4 = new QgsVectorLayer( "none", "vvvv4", "memory" );

  QFileInfo fm( testDataDir + "/mesh/quad_and_triangle.2dm" );
  QgsMeshLayer *m1 = new QgsMeshLayer( fm.filePath(), "MX", "mdal" );
  QVERIFY( m1->isValid() );
  QgsMeshLayer *m2 = new QgsMeshLayer( fm.filePath(), "mA", "mdal" );
  QVERIFY( m2->isValid() );
  p.addMapLayers( QList<QgsMapLayer *>() << r1 << r2 << r3 << v1 << v2 << v3 << v4 << m1 << m2 );

  // compatibleRasterLayers
  QVERIFY( QgsProcessingUtils::compatibleRasterLayers( nullptr ).isEmpty() );

  // sorted
  QStringList lIds;
  Q_FOREACH ( QgsRasterLayer *rl, QgsProcessingUtils::compatibleRasterLayers( &p ) )
    lIds << rl->name();
  QCOMPARE( lIds, QStringList() << "ar2" << "R1" << "zz" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsRasterLayer *rl, QgsProcessingUtils::compatibleRasterLayers( &p, false ) )
    lIds << rl->name();
  QCOMPARE( lIds, QStringList() << "R1" << "ar2" << "zz" );

  // compatibleMeshLayers
  QVERIFY( QgsProcessingUtils::compatibleMeshLayers( nullptr ).isEmpty() );

  // sorted
  lIds.clear();
  Q_FOREACH ( QgsMeshLayer *rl, QgsProcessingUtils::compatibleMeshLayers( &p ) )
    lIds << rl->name();
  QCOMPARE( lIds, QStringList() << "mA" << "MX" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsMeshLayer *rl, QgsProcessingUtils::compatibleMeshLayers( &p, false ) )
    lIds << rl->name();
  QCOMPARE( lIds, QStringList() << "MX" << "mA" );

  // compatibleVectorLayers
  QVERIFY( QgsProcessingUtils::compatibleVectorLayers( nullptr ).isEmpty() );

  // sorted
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" << "v3" << "V4" << "vvvv4" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>(), false ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "V4" << "v1" << "v3" << "vvvv4" );

  // point only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>() << QgsProcessing::TypeVectorPoint ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" );

  // polygon only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>() << QgsProcessing::TypeVectorPolygon ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "V4" );

  // line only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>() << QgsProcessing::TypeVectorLine ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v3" );

  // point and line only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" << "v3" );

  // any vector w geometry
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>() << QgsProcessing::TypeVectorAnyGeometry ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" << "v3" << "V4" );

  // any vector
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<int>() << QgsProcessing::TypeVector ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" << "v3" << "V4" << "vvvv4" );

  // all layers
  QVERIFY( QgsProcessingUtils::compatibleLayers( nullptr ).isEmpty() );

  // sorted
  lIds.clear();
  Q_FOREACH ( QgsMapLayer *l, QgsProcessingUtils::compatibleLayers( &p ) )
    lIds << l->name();
  QCOMPARE( lIds, QStringList() << "ar2" << "mA" << "MX" << "R1" << "v1" << "v3" << "V4" << "vvvv4" <<  "zz" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsMapLayer *l, QgsProcessingUtils::compatibleLayers( &p, false ) )
    lIds << l->name();
  QCOMPARE( lIds, QStringList() << "R1" << "ar2" << "zz"  << "V4" << "v1" << "v3" << "vvvv4" << "MX" << "mA" );
}

void TestQgsProcessing::normalizeLayerSource()
{
  QCOMPARE( QgsProcessingUtils::normalizeLayerSource( "data\\layers\\test.shp" ), QString( "data/layers/test.shp" ) );
  QCOMPARE( QgsProcessingUtils::normalizeLayerSource( "data\\layers \"new\"\\test.shp" ), QString( "data/layers \"new\"/test.shp" ) );
}


class TestPostProcessor : public QgsProcessingLayerPostProcessorInterface
{
  public:

    TestPostProcessor( bool *deleted )
      : deleted( deleted )
    {}

    ~TestPostProcessor() override
    {
      *deleted = true;
    }

    bool *deleted = nullptr;

    void postProcessLayer( QgsMapLayer *, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
    }
};


void TestQgsProcessing::context()
{
  QgsProcessingContext context;

  // simple tests for getters/setters
  context.setDefaultEncoding( "my_enc" );
  QCOMPARE( context.defaultEncoding(), QStringLiteral( "my_enc" ) );

  context.setFlags( QgsProcessingContext::Flags( nullptr ) );
  QCOMPARE( context.flags(), QgsProcessingContext::Flags( nullptr ) );

  QgsProject p;
  context.setProject( &p );
  QCOMPARE( context.project(), &p );

  context.setInvalidGeometryCheck( QgsFeatureRequest::GeometrySkipInvalid );
  QCOMPARE( context.invalidGeometryCheck(), QgsFeatureRequest::GeometrySkipInvalid );

  QgsVectorLayer *vector = new QgsVectorLayer( "Polygon", "vector", "memory" );
  context.temporaryLayerStore()->addMapLayer( vector );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( vector->id() ), vector );

  QgsProcessingContext context2;
  context2.copyThreadSafeSettings( context );
  QCOMPARE( context2.defaultEncoding(), context.defaultEncoding() );
  QCOMPARE( context2.invalidGeometryCheck(), context.invalidGeometryCheck() );
  QCOMPARE( context2.flags(), context.flags() );
  QCOMPARE( context2.project(), context.project() );
  // layers from temporaryLayerStore must not be copied by copyThreadSafeSettings
  QVERIFY( context2.temporaryLayerStore()->mapLayers().isEmpty() );

  // layers to load on completion
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V1", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Polygon", "V2", "memory" );
  QVERIFY( context.layersToLoadOnCompletion().isEmpty() );
  QVERIFY( !context.willLoadLayerOnCompletion( v1->id() ) );
  QVERIFY( !context.willLoadLayerOnCompletion( v2->id() ) );
  QMap< QString, QgsProcessingContext::LayerDetails > layers;
  QgsProcessingContext::LayerDetails details( QStringLiteral( "v1" ), &p );
  bool ppDeleted = false;
  TestPostProcessor *pp = new TestPostProcessor( &ppDeleted );
  details.setPostProcessor( pp );
  layers.insert( v1->id(), details );
  context.setLayersToLoadOnCompletion( layers );
  QCOMPARE( context.layersToLoadOnCompletion().count(), 1 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), v1->id() );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "v1" ) );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).postProcessor(), pp );
  QVERIFY( context.willLoadLayerOnCompletion( v1->id() ) );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v1->id() ).name, QStringLiteral( "v1" ) );
  QVERIFY( !context.willLoadLayerOnCompletion( v2->id() ) );
  context.addLayerToLoadOnCompletion( v2->id(), QgsProcessingContext::LayerDetails( QStringLiteral( "v2" ), &p ) );
  QCOMPARE( context.layersToLoadOnCompletion().count(), 2 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), v1->id() );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "v1" ) );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).postProcessor(), pp );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 1 ), v2->id() );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 1 ).name, QStringLiteral( "v2" ) );
  QVERIFY( !context.layersToLoadOnCompletion().values().at( 1 ).postProcessor() );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v1->id() ).name, QStringLiteral( "v1" ) );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v2->id() ).name, QStringLiteral( "v2" ) );
  QVERIFY( context.willLoadLayerOnCompletion( v1->id() ) );
  QVERIFY( context.willLoadLayerOnCompletion( v2->id() ) );

  // ensure that copyThreadSafeSettings doesn't copy layersToLoadOnCompletion()
  context2.copyThreadSafeSettings( context );
  QVERIFY( context2.layersToLoadOnCompletion().isEmpty() );

  layers.clear();
  layers.insert( v2->id(), QgsProcessingContext::LayerDetails( QStringLiteral( "v2" ), &p ) );
  context.setLayersToLoadOnCompletion( layers );
  QVERIFY( ppDeleted );

  QCOMPARE( context.layersToLoadOnCompletion().count(), 1 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), v2->id() );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "v2" ) );
  context.addLayerToLoadOnCompletion( v1->id(), QgsProcessingContext::LayerDetails( QString(), &p ) );
  QCOMPARE( context.layersToLoadOnCompletion().count(), 2 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), v1->id() );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 1 ), v2->id() );

  context.temporaryLayerStore()->addMapLayer( v1 );
  context.temporaryLayerStore()->addMapLayer( v2 );

  // test takeResultsFrom
  context2.takeResultsFrom( context );
  QVERIFY( context.temporaryLayerStore()->mapLayers().isEmpty() );
  QVERIFY( context.layersToLoadOnCompletion().isEmpty() );
  // should now be in context2
  QCOMPARE( context2.temporaryLayerStore()->mapLayer( v1->id() ), v1 );
  QCOMPARE( context2.temporaryLayerStore()->mapLayer( v2->id() ), v2 );
  QCOMPARE( context2.layersToLoadOnCompletion().count(), 2 );
  QCOMPARE( context2.layersToLoadOnCompletion().keys().at( 0 ), v1->id() );
  QCOMPARE( context2.layersToLoadOnCompletion().keys().at( 1 ), v2->id() );

  // make sure postprocessor is correctly deleted
  ppDeleted = false;
  pp = new TestPostProcessor( &ppDeleted );
  details = QgsProcessingContext::LayerDetails( QStringLiteral( "v1" ), &p );
  details.setPostProcessor( pp );
  layers.insert( v1->id(), details );
  context.setLayersToLoadOnCompletion( layers );
  // overwrite with existing
  context.setLayersToLoadOnCompletion( layers );
  QVERIFY( !ppDeleted );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v1->id() ).postProcessor(), pp );
  bool pp2Deleted = false;
  TestPostProcessor *pp2 = new TestPostProcessor( &pp2Deleted );
  details = QgsProcessingContext::LayerDetails( QStringLiteral( "v1" ), &p );
  details.setPostProcessor( pp2 );
  layers.insert( v1->id(), details );
  context.setLayersToLoadOnCompletion( layers );
  QVERIFY( ppDeleted );
  QVERIFY( !pp2Deleted );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v1->id() ).postProcessor(), pp2 );
  ppDeleted = false;
  pp = new TestPostProcessor( &ppDeleted );
  details = QgsProcessingContext::LayerDetails( QStringLiteral( "v1" ), &p );
  details.setPostProcessor( pp );
  context.addLayerToLoadOnCompletion( v1->id(), details );
  QVERIFY( !ppDeleted );
  QVERIFY( pp2Deleted );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v1->id() ).postProcessor(), pp );
  pp2Deleted = false;
  pp2 = new TestPostProcessor( &pp2Deleted );
  context.layerToLoadOnCompletionDetails( v1->id() ).setPostProcessor( pp2 );
  QVERIFY( ppDeleted );
  QVERIFY( !pp2Deleted );
  QCOMPARE( context.layerToLoadOnCompletionDetails( v1->id() ).postProcessor(), pp2 );

  // take result layer
  QgsMapLayer *result = context2.takeResultLayer( v1->id() );
  QCOMPARE( result, v1 );
  QString id = v1->id();
  delete v1;
  QVERIFY( !context2.temporaryLayerStore()->mapLayer( id ) );
  QVERIFY( !context2.takeResultLayer( id ) );
  result = context2.takeResultLayer( v2->id() );
  QCOMPARE( result, v2 );
  id = v2->id();
  delete v2;
  QVERIFY( !context2.temporaryLayerStore()->mapLayer( id ) );
}

void TestQgsProcessing::mapLayers()
{
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster = testDataDir + "landsat.tif";
  QString vector = testDataDir + "points.shp";

  // test loadMapLayerFromString with raster
  QgsMapLayer *l = QgsProcessingUtils::loadMapLayerFromString( raster );
  QVERIFY( l->isValid() );
  QCOMPARE( l->type(), QgsMapLayer::RasterLayer );
  QCOMPARE( l->name(), QStringLiteral( "landsat" ) );

  delete l;

  //test with vector
  l = QgsProcessingUtils::loadMapLayerFromString( vector );
  QVERIFY( l->isValid() );
  QCOMPARE( l->type(), QgsMapLayer::VectorLayer );
  QCOMPARE( l->name(), QStringLiteral( "points" ) );
  delete l;

  l = QgsProcessingUtils::loadMapLayerFromString( QString() );
  QVERIFY( !l );
  l = QgsProcessingUtils::loadMapLayerFromString( QStringLiteral( "so much room for activities!" ) );
  QVERIFY( !l );
  l = QgsProcessingUtils::loadMapLayerFromString( testDataDir + "multipoint.shp" );
  QVERIFY( l->isValid() );
  QCOMPARE( l->type(), QgsMapLayer::VectorLayer );
  QCOMPARE( l->name(), QStringLiteral( "multipoint" ) );
  delete l;

  // Test layers from a string with parameters
  QString osmFilePath = testDataDir + "openstreetmap/testdata.xml";
  std::unique_ptr< QgsVectorLayer > osm( qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::loadMapLayerFromString( osmFilePath ) ) );
  QVERIFY( osm->isValid() );
  QCOMPARE( osm->geometryType(), QgsWkbTypes::PointGeometry );

  osm.reset( qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::loadMapLayerFromString( osmFilePath + "|layerid=3" ) ) );
  QVERIFY( osm->isValid() );
  QCOMPARE( osm->geometryType(), QgsWkbTypes::PolygonGeometry );

  osm.reset( qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::loadMapLayerFromString( osmFilePath + "|layerid=3|subset=\"building\" is not null" ) ) );
  QVERIFY( osm->isValid() );
  QCOMPARE( osm->geometryType(), QgsWkbTypes::PolygonGeometry );
  QCOMPARE( osm->subsetString(), QStringLiteral( "\"building\" is not null" ) );
}

void TestQgsProcessing::mapLayerFromStore()
{
  // test mapLayerFromStore

  QgsMapLayerStore store;

  // add a bunch of layers to a project
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  QFileInfo fi2( raster2 );
  QgsRasterLayer *r2 = new QgsRasterLayer( fi2.filePath(), "ar2" );
  QVERIFY( r2->isValid() );

  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Point", "v1", "memory" );
  store.addMapLayers( QList<QgsMapLayer *>() << r1 << r2 << v1 << v2 );

  QVERIFY( ! QgsProcessingUtils::mapLayerFromStore( QString(), nullptr ) );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromStore( QStringLiteral( "v1" ), nullptr ) );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromStore( QString(), &store ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( raster1, &store ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( raster2, &store ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "R1", &store ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "ar2", &store ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "V4", &store ), v1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "v1", &store ), v2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( r1->id(), &store ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( v1->id(), &store ), v1 );
}

void TestQgsProcessing::mapLayerFromString()
{
  // test mapLayerFromString

  QgsProcessingContext c;
  QgsProject p;

  // add a bunch of layers to a project
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  QFileInfo fi2( raster2 );
  QgsRasterLayer *r2 = new QgsRasterLayer( fi2.filePath(), "ar2" );
  QVERIFY( r2->isValid() );

  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Point", "v1", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << r1 << r2 << v1 << v2 );

  // no project set yet
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( QString(), c ) );
  QVERIFY( !c.getMapLayer( QString() ) );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( QStringLiteral( "v1" ), c ) );
  QVERIFY( !c.getMapLayer( QStringLiteral( "v1" ) ) );

  c.setProject( &p );

  // layers from current project
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( QString(), c ) );
  QVERIFY( !c.getMapLayer( QString() ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( raster1, c ), r1 );
  QCOMPARE( c.getMapLayer( raster1 ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( raster1, c, true, QgsProcessingUtils::Raster ), r1 );
  QVERIFY( !QgsProcessingUtils::mapLayerFromString( raster1, c, true, QgsProcessingUtils::Vector ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( raster2, c ), r2 );
  QCOMPARE( c.getMapLayer( raster2 ), r2 );
  QVERIFY( !QgsProcessingUtils::mapLayerFromString( raster2, c, true, QgsProcessingUtils::Vector ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "R1", c ), r1 );
  QCOMPARE( c.getMapLayer( "R1" ), r1 );
  QVERIFY( !QgsProcessingUtils::mapLayerFromString( "R1", c, true, QgsProcessingUtils::Vector ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "ar2", c ), r2 );
  QCOMPARE( c.getMapLayer( "ar2" ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "V4", c ), v1 );
  QCOMPARE( c.getMapLayer( "V4" ), v1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "V4", c, true, QgsProcessingUtils::Vector ), v1 );
  QVERIFY( !QgsProcessingUtils::mapLayerFromString( "V4", c, true, QgsProcessingUtils::Raster ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "v1", c ), v2 );
  QCOMPARE( c.getMapLayer( "v1" ), v2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( r1->id(), c ), r1 );
  QCOMPARE( c.getMapLayer( r1->id() ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( v1->id(), c ), v1 );
  QCOMPARE( c.getMapLayer( v1->id() ), v1 );

  // check that layers in context temporary store are used
  QgsVectorLayer *v5 = new QgsVectorLayer( "Polygon", "V5", "memory" );
  QgsVectorLayer *v6 = new QgsVectorLayer( "Point", "v6", "memory" );
  c.temporaryLayerStore()->addMapLayers( QList<QgsMapLayer *>() << v5 << v6 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "V5", c ), v5 );
  QCOMPARE( c.getMapLayer( "V5" ), v5 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "V5", c, true, QgsProcessingUtils::Vector ), v5 );
  QVERIFY( !QgsProcessingUtils::mapLayerFromString( "V5", c, true, QgsProcessingUtils::Raster ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "v6", c ), v6 );
  QCOMPARE( c.getMapLayer( "v6" ), v6 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( v5->id(), c ), v5 );
  QCOMPARE( c.getMapLayer( v5->id() ), v5 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( v6->id(), c ), v6 );
  QCOMPARE( c.getMapLayer( v6->id() ), v6 );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( "aaaaa", c ) );
  QVERIFY( !c.getMapLayer( "aaaa" ) );

  // if specified, check that layers can be loaded
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( "aaaaa", c ) );
  QString newRaster = testDataDir + "requires_warped_vrt.tif";
  // don't allow loading
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( newRaster, c, false ) );
  QVERIFY( !c.getMapLayer( newRaster ) );
  // allow loading
  QgsMapLayer *loadedLayer = QgsProcessingUtils::mapLayerFromString( newRaster, c, true );
  QVERIFY( loadedLayer->isValid() );
  QCOMPARE( loadedLayer->type(), QgsMapLayer::RasterLayer );
  // should now be in temporary store
  QCOMPARE( c.temporaryLayerStore()->mapLayer( loadedLayer->id() ), loadedLayer );

  // since it's now in temporary store, should be accessible even if we deny loading new layers
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( newRaster, c, false ), loadedLayer );
  QCOMPARE( c.getMapLayer( newRaster ), loadedLayer );
}

void TestQgsProcessing::algorithm()
{
  DummyAlgorithm alg( "test" );
  DummyProvider *p = new DummyProvider( "p1" );
  QCOMPARE( alg.id(), QString( "test" ) );
  alg.setProvider( p );
  QCOMPARE( alg.provider(), p );
  QCOMPARE( alg.id(), QString( "p1:test" ) );

  QVERIFY( p->algorithms().isEmpty() );

  QSignalSpy providerRefreshed( p, &DummyProvider::algorithmsLoaded );
  p->refreshAlgorithms();
  QCOMPARE( providerRefreshed.count(), 1 );

  for ( int i = 0; i < 2; ++i )
  {
    QCOMPARE( p->algorithms().size(), 2 );
    QCOMPARE( p->algorithm( "alg1" )->name(), QStringLiteral( "alg1" ) );
    QCOMPARE( p->algorithm( "alg1" )->provider(), p );
    QCOMPARE( p->algorithm( "alg2" )->provider(), p );
    QCOMPARE( p->algorithm( "alg2" )->name(), QStringLiteral( "alg2" ) );
    QVERIFY( !p->algorithm( "aaaa" ) );
    QVERIFY( p->algorithms().contains( p->algorithm( "alg1" ) ) );
    QVERIFY( p->algorithms().contains( p->algorithm( "alg2" ) ) );

    // reload, then retest on next loop
    // must be safe for providers to reload their algorithms
    p->refreshAlgorithms();
    QCOMPARE( providerRefreshed.count(), 2 + i );
  }

  // inactive provider, should not load algorithms
  p->active = false;
  p->refreshAlgorithms();
  QCOMPARE( providerRefreshed.count(), 3 );
  QVERIFY( p->algorithms().empty() );
  p->active = true;
  p->refreshAlgorithms();
  QCOMPARE( providerRefreshed.count(), 4 );
  QVERIFY( !p->algorithms().empty() );

  QgsProcessingRegistry r;
  QVERIFY( r.addProvider( p ) );
  QCOMPARE( r.algorithms().size(), 2 );
  QVERIFY( r.algorithms().contains( p->algorithm( "alg1" ) ) );
  QVERIFY( r.algorithms().contains( p->algorithm( "alg2" ) ) );

  // algorithmById
  QCOMPARE( r.algorithmById( "p1:alg1" ), p->algorithm( "alg1" ) );
  QCOMPARE( r.algorithmById( "p1:alg2" ), p->algorithm( "alg2" ) );
  QVERIFY( !r.algorithmById( "p1:alg3" ) );
  QVERIFY( !r.algorithmById( "px:alg1" ) );

  // test that algorithmById can transparently map 'qgis' algorithms across to matching 'native' algorithms
  // this allows us the freedom to convert qgis python algs to c++ without breaking api or existing models
  QCOMPARE( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "qgis:dissolve" ) )->id(), QStringLiteral( "native:dissolve" ) );
  QCOMPARE( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "qgis:clip" ) )->id(), QStringLiteral( "native:clip" ) );
  QVERIFY( !QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "qgis:notanalg" ) ) );

  // createAlgorithmById
  QVERIFY( !r.createAlgorithmById( "p1:alg3" ) );
  std::unique_ptr< QgsProcessingAlgorithm > creation( r.createAlgorithmById( "p1:alg1" ) );
  QVERIFY( creation.get() );
  QCOMPARE( creation->provider()->id(), QStringLiteral( "p1" ) );
  QCOMPARE( creation->id(), QStringLiteral( "p1:alg1" ) );
  creation.reset( r.createAlgorithmById( "p1:alg2" ) );
  QVERIFY( creation.get() );
  QCOMPARE( creation->provider()->id(), QStringLiteral( "p1" ) );
  QCOMPARE( creation->id(), QStringLiteral( "p1:alg2" ) );

  //test that loading a provider triggers an algorithm refresh
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( p2->algorithms().isEmpty() );
  p2->load();
  QCOMPARE( p2->algorithms().size(), 2 );
  delete p2;

  // test that adding a provider to the registry automatically refreshes algorithms (via load)
  DummyProvider *p3 = new DummyProvider( "p3" );
  QVERIFY( p3->algorithms().isEmpty() );
  QVERIFY( r.addProvider( p3 ) );
  QCOMPARE( p3->algorithms().size(), 2 );
}

void TestQgsProcessing::features()
{
  QgsVectorLayer *layer = new QgsVectorLayer( "Point", "v1", "memory" );
  for ( int i = 1; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  QgsProject p;
  p.addMapLayer( layer );

  QgsProcessingContext context;
  context.setProject( &p );
  // disable check for geometry validity
  context.setFlags( QgsProcessingContext::Flags( nullptr ) );

  std::function< QgsFeatureIds( QgsFeatureIterator it ) > getIds = []( QgsFeatureIterator it )
  {
    QgsFeature f;
    QgsFeatureIds ids;
    while ( it.nextFeature( f ) )
    {
      ids << f.id();
    }
    return ids;
  };

  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterString( QStringLiteral( "layer" ) ) );
  QVariantMap params;
  params.insert( QStringLiteral( "layer" ), layer->id() );

  std::unique_ptr< QgsFeatureSource > source( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );

  // test with all features
  QgsFeatureIds ids = getIds( source->getFeatures() );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 2 << 3 << 4 << 5 );
  QCOMPARE( source->featureCount(), 5L );

  // test with selected features
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), true ) ) );
  layer->selectByIds( QgsFeatureIds() << 2 << 4 );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures() );
  QCOMPARE( ids, QgsFeatureIds() << 2 << 4 );
  QCOMPARE( source->featureCount(), 2L );

  // selection, but not using selected features
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), false ) ) );
  layer->selectByIds( QgsFeatureIds() << 2 << 4 );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures() );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 2 << 3 << 4 << 5 );
  QCOMPARE( source->featureCount(), 5L );

  // using selected features, but no selection
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), true ) ) );
  layer->removeSelection();
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures() );
  QVERIFY( ids.isEmpty() );
  QCOMPARE( source->featureCount(), 0L );


  // test that feature request is honored
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), false ) ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures( QgsFeatureRequest().setFilterFids( QgsFeatureIds() << 1 << 3 << 5 ) ) );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 3 << 5 );

  // count is only rough - but we expect (for now) to see full layer count
  QCOMPARE( source->featureCount(), 5L );

  //test that feature request is honored when using selections
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), true ) ) );
  layer->selectByIds( QgsFeatureIds() << 2 << 4 );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) ) );
  QCOMPARE( ids, QgsFeatureIds() << 2 << 4 );

  // test callback is hit when filtering invalid geoms
  bool encountered = false;
  std::function< void( const QgsFeature & ) > callback = [ &encountered ]( const QgsFeature & )
  {
    encountered = true;
  };

  context.setInvalidGeometryCheck( QgsFeatureRequest::GeometryAbortOnInvalid );
  context.setInvalidGeometryCallback( callback );
  QgsVectorLayer *polyLayer = new QgsVectorLayer( "Polygon", "v2", "memory" );
  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 1 0, 0 1, 1 1, 0 0))" ) ) );
  polyLayer->dataProvider()->addFeatures( QgsFeatureList() << f );
  p.addMapLayer( polyLayer );
  params.insert( QStringLiteral( "layer" ), polyLayer->id() );

  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures() );
  QVERIFY( encountered );

  encountered = false;
  context.setInvalidGeometryCheck( QgsFeatureRequest::GeometryNoCheck );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  ids = getIds( source->getFeatures() );
  QVERIFY( !encountered );

  // equality operator
  QVERIFY( QgsProcessingFeatureSourceDefinition( layer->id(), true ) == QgsProcessingFeatureSourceDefinition( layer->id(), true ) );
  QVERIFY( QgsProcessingFeatureSourceDefinition( layer->id(), true ) != QgsProcessingFeatureSourceDefinition( "b", true ) );
  QVERIFY( QgsProcessingFeatureSourceDefinition( layer->id(), true ) != QgsProcessingFeatureSourceDefinition( layer->id(), false ) );
}

void TestQgsProcessing::uniqueValues()
{
  QgsVectorLayer *layer = new QgsVectorLayer( "Point?field=a:integer&field=b:string", "v1", "memory" );
  for ( int i = 0; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setAttributes( QgsAttributes() << i % 3 + 1 << QString( QChar( ( i % 3 ) + 65 ) ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  QgsProcessingContext context;
  context.setFlags( QgsProcessingContext::Flags( nullptr ) );

  QgsProject p;
  p.addMapLayer( layer );
  context.setProject( &p );

  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterString( QStringLiteral( "layer" ) ) );
  QVariantMap params;
  params.insert( QStringLiteral( "layer" ), layer->id() );

  std::unique_ptr< QgsFeatureSource > source( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );

  // some bad checks
  QVERIFY( source->uniqueValues( -1 ).isEmpty() );
  QVERIFY( source->uniqueValues( 10001 ).isEmpty() );

  // good checks
  QSet< QVariant > vals = source->uniqueValues( 0 );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( 1 ) );
  QVERIFY( vals.contains( 2 ) );
  QVERIFY( vals.contains( 3 ) );
  vals = source->uniqueValues( 1 );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( QString( "A" ) ) );
  QVERIFY( vals.contains( QString( "B" ) ) );
  QVERIFY( vals.contains( QString( "C" ) ) );

  //using only selected features
  layer->selectByIds( QgsFeatureIds() << 1 << 2 << 4 );
  // but not using selection yet...
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  vals = source->uniqueValues( 0 );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( 1 ) );
  QVERIFY( vals.contains( 2 ) );
  QVERIFY( vals.contains( 3 ) );
  vals = source->uniqueValues( 1 );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( QString( "A" ) ) );
  QVERIFY( vals.contains( QString( "B" ) ) );
  QVERIFY( vals.contains( QString( "C" ) ) );

  // selection and using selection
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), true ) ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  QVERIFY( source->uniqueValues( -1 ).isEmpty() );
  QVERIFY( source->uniqueValues( 10001 ).isEmpty() );
  vals = source->uniqueValues( 0 );
  QCOMPARE( vals.count(), 2 );
  QVERIFY( vals.contains( 1 ) );
  QVERIFY( vals.contains( 2 ) );
  vals = source->uniqueValues( 1 );
  QCOMPARE( vals.count(), 2 );
  QVERIFY( vals.contains( QString( "A" ) ) );
  QVERIFY( vals.contains( QString( "B" ) ) );
}

void TestQgsProcessing::createIndex()
{
  QgsVectorLayer *layer = new QgsVectorLayer( "Point", "v1", "memory" );
  for ( int i = 1; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setGeometry( QgsGeometry( new QgsPoint( i, 2 ) ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  QgsProcessingContext context;
  QgsProject p;
  p.addMapLayer( layer );
  context.setProject( &p );

  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterString( QStringLiteral( "layer" ) ) );
  QVariantMap params;
  params.insert( QStringLiteral( "layer" ), layer->id() );

  // disable selected features check
  std::unique_ptr< QgsFeatureSource > source( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  QVERIFY( source.get() );
  QgsSpatialIndex index( *source );
  QList<QgsFeatureId> ids = index.nearestNeighbor( QgsPointXY( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 2 );

  // selected features check, but none selected
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), true ) ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  index = QgsSpatialIndex( *source );
  ids = index.nearestNeighbor( QgsPointXY( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() );

  // create selection
  layer->selectByIds( QgsFeatureIds() << 4 << 5 );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  index = QgsSpatialIndex( *source );
  ids = index.nearestNeighbor( QgsPointXY( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 4 );

  // selection but not using selection mode
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layer->id(), false ) ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  index = QgsSpatialIndex( *source );
  ids = index.nearestNeighbor( QgsPointXY( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 2 );
}

void TestQgsProcessing::parseDestinationString()
{
  QString providerKey;
  QString uri;
  QString layerName;
  QString format;
  QVariantMap options;
  bool useWriter = false;

  // simple shapefile output
  QString destination = QStringLiteral( "d:/test.shp" );
  QgsProcessingUtils::parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );
  QCOMPARE( destination, QStringLiteral( "d:/test.shp" ) );
  QCOMPARE( providerKey, QStringLiteral( "ogr" ) );
  QCOMPARE( uri, QStringLiteral( "d:/test.shp" ) );
  QCOMPARE( format, QStringLiteral( "ESRI Shapefile" ) );
  QVERIFY( useWriter );

  // postgis output
  destination = QStringLiteral( "postgis:dbname='db' host=DBHOST port=5432 table=\"calcs\".\"output\" (geom) sql=" );
  QgsProcessingUtils::parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );
  QCOMPARE( providerKey, QStringLiteral( "postgres" ) );
  QCOMPARE( uri, QStringLiteral( "dbname='db' host=DBHOST port=5432 table=\"calcs\".\"output\" (geom) sql=" ) );
  QVERIFY( !useWriter );
  // postgres key should also work
  destination = QStringLiteral( "postgres:dbname='db' host=DBHOST port=5432 table=\"calcs\".\"output\" (geom) sql=" );
  QgsProcessingUtils::parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );
  QCOMPARE( providerKey, QStringLiteral( "postgres" ) );
  QCOMPARE( uri, QStringLiteral( "dbname='db' host=DBHOST port=5432 table=\"calcs\".\"output\" (geom) sql=" ) );
  QVERIFY( !useWriter );

  // full uri shp output
  options.clear();
  destination = QStringLiteral( "ogr:d:/test.shp" );
  QgsProcessingUtils::parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );
  QCOMPARE( providerKey, QStringLiteral( "ogr" ) );
  QCOMPARE( uri, QStringLiteral( "d:/test.shp" ) );
  QCOMPARE( options.value( QStringLiteral( "update" ) ).toBool(), true );
  QVERIFY( !options.contains( QStringLiteral( "layerName" ) ) );
  QVERIFY( !useWriter );

// full uri geopackage output
  options.clear();
  destination = QStringLiteral( "ogr:d:/test.gpkg" );
  QgsProcessingUtils::parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );
  QCOMPARE( providerKey, QStringLiteral( "ogr" ) );
  QCOMPARE( uri, QStringLiteral( "d:/test.gpkg" ) );
  QCOMPARE( options.value( QStringLiteral( "update" ) ).toBool(), true );
  QVERIFY( !options.contains( QStringLiteral( "layerName" ) ) );
  QVERIFY( !useWriter );

// full uri geopackage table output with layer name
  options.clear();
  destination = QStringLiteral( "ogr:dbname='d:/package.gpkg' table=\"mylayer\" (geom) sql=" );
  QgsProcessingUtils::parseDestinationString( destination, providerKey, uri, layerName, format, options, useWriter );
  QCOMPARE( providerKey, QStringLiteral( "ogr" ) );
  QCOMPARE( uri, QStringLiteral( "d:/package.gpkg" ) );
  QCOMPARE( options.value( QStringLiteral( "update" ) ).toBool(), true );
  QCOMPARE( options.value( QStringLiteral( "layerName" ) ).toString(), QStringLiteral( "mylayer" ) );
  QVERIFY( !useWriter );
}

void TestQgsProcessing::createFeatureSink()
{
  QgsProcessingContext context;

  // empty destination
  QString destination;
  destination = QString();
  QgsVectorLayer *layer = nullptr;

  // should create a memory layer
  std::unique_ptr< QgsFeatureSink > sink( QgsProcessingUtils::createFeatureSink( destination, context, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem() ) );
  QVERIFY( sink.get() );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink.get() )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QgsFeature f;
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();
  layer = nullptr;

  // specific memory layer output
  destination = QStringLiteral( "memory:mylayer" );
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem() ) );
  QVERIFY( sink.get() );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink.get() )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( layer->name(), QStringLiteral( "mylayer" ) );
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();
  layer = nullptr;

  // nameless memory layer
  destination = QStringLiteral( "memory:" );
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem() ) );
  QVERIFY( sink.get() );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink.get() )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( layer->name(), QString( "output" ) ); // should fall back to "output" name
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();
  layer = nullptr;

  // memory layer parameters
  destination = QStringLiteral( "memory:mylayer" );
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "my_field" ), QVariant::String, QString(), 100 ) );
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, fields, QgsWkbTypes::PointZM, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ) ) );
  QVERIFY( sink.get() );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink.get() )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( layer->name(), QStringLiteral( "mylayer" ) );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( layer->fields().size(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "my_field" ) );
  QCOMPARE( layer->fields().at( 0 ).type(), QVariant::String );
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();

  // non memory layer output
  destination = QDir::tempPath() + "/create_feature_sink.tab";
  QString prevDest = destination;
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, fields, QgsWkbTypes::Polygon, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ) ) );
  QVERIFY( sink.get() );
  f = QgsFeature( fields );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  f.setAttributes( QgsAttributes() << "val" );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( destination, prevDest );
  sink.reset( nullptr );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, true ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( layer->fields().size(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "my_field" ) );
  QCOMPARE( layer->fields().at( 0 ).type(), QVariant::String );
  QCOMPARE( layer->featureCount(), 1L );

  // no extension, should default to shp
  destination = QDir::tempPath() + "/create_feature_sink2";
  prevDest = QDir::tempPath() + "/create_feature_sink2.gpkg";
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, fields, QgsWkbTypes::Point25D, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ) ) );
  QVERIFY( sink.get() );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PointZ(1 2 3)" ) ) );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( destination, prevDest );
  sink.reset( nullptr );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, true ) );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::Point25D );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( layer->fields().size(), 2 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "fid" ) );
  QCOMPARE( layer->fields().at( 1 ).name(), QStringLiteral( "my_field" ) );
  QCOMPARE( layer->fields().at( 1 ).type(), QVariant::String );
  QCOMPARE( layer->featureCount(), 1L );

  //windows style path
  destination = "d:\\temp\\create_feature_sink.tab";
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, fields, QgsWkbTypes::Polygon, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ) ) );
  QVERIFY( sink.get() );

  // save to geopackage
  QString geopackagePath = QDir::tempPath() + "/packaged.gpkg";
  if ( QFileInfo::exists( geopackagePath ) )
    QFile::remove( geopackagePath );
  destination = QStringLiteral( "ogr:dbname='%1' table=\"polygons\" (geom) sql=" ).arg( geopackagePath );
  sink.reset( QgsProcessingUtils::createFeatureSink( destination, context, fields, QgsWkbTypes::Polygon, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ) ) );
  QVERIFY( sink.get() );
  f = QgsFeature( fields );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  f.setAttributes( QgsAttributes() << "val" );
  QVERIFY( sink->addFeature( f ) );
  sink.reset( nullptr );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, true ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( layer->getFeatures().nextFeature( f ) );
  QCOMPARE( f.attribute( "my_field" ).toString(), QStringLiteral( "val" ) );

  // add another output to the same geopackage
  QString destination2 = QStringLiteral( "ogr:dbname='%1' table=\"points\" (geom) sql=" ).arg( geopackagePath );
  sink.reset( QgsProcessingUtils::createFeatureSink( destination2, context, fields, QgsWkbTypes::Point, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ) ) );
  QVERIFY( sink.get() );
  f = QgsFeature( fields );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point(0 0)" ) ) );
  f.setAttributes( QgsAttributes() << "val2" );
  QVERIFY( sink->addFeature( f ) );
  sink.reset( nullptr );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination2, context, true ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::Point );
  QVERIFY( layer->getFeatures().nextFeature( f ) );
  QCOMPARE( f.attribute( "my_field" ).toString(), QStringLiteral( "val2" ) );

  // original polygon layer should remain
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, true ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( layer->getFeatures().nextFeature( f ) );
  QCOMPARE( f.attribute( "my_field" ).toString(), QStringLiteral( "val" ) );
}

void TestQgsProcessing::parameters()
{
  // test parameter utilities

  std::unique_ptr< QgsProcessingParameterDefinition > def;

  QVariantMap params;
  params.insert( QStringLiteral( "prop" ), QgsProperty::fromField( "a_field" ) );
  params.insert( QStringLiteral( "string" ), QStringLiteral( "a string" ) );
  params.insert( QStringLiteral( "double" ), 5.2 );
  params.insert( QStringLiteral( "int" ), 15 );
  params.insert( QStringLiteral( "ints" ), QVariant( QList<QVariant>() << 3 << 2 << 1 ) );
  params.insert( QStringLiteral( "bool" ), true );

  QgsProcessingContext context;

  // isDynamic
  QVERIFY( QgsProcessingParameters::isDynamic( params, QStringLiteral( "prop" ) ) );
  QVERIFY( !QgsProcessingParameters::isDynamic( params, QStringLiteral( "string" ) ) );
  QVERIFY( !QgsProcessingParameters::isDynamic( params, QStringLiteral( "bad" ) ) );

  // parameterAsString
  def.reset( new QgsProcessingParameterString( QStringLiteral( "string" ), QStringLiteral( "desc" ) ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QStringLiteral( "a string" ) );
  def->setName( QStringLiteral( "double" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ).left( 3 ), QStringLiteral( "5.2" ) );
  def->setName( QStringLiteral( "int" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QStringLiteral( "15" ) );
  def->setName( QStringLiteral( "bool" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QStringLiteral( "true" ) );
  def->setName( QStringLiteral( "bad" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString() );

  def->setIsDynamic( true );
  QVERIFY( def->isDynamic() );
  def->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Distance" ), QObject::tr( "Buffer distance" ), QgsPropertyDefinition::Double ) );
  QCOMPARE( def->dynamicPropertyDefinition().name(), QStringLiteral( "Distance" ) );
  def->setDynamicLayerParameterName( "parent" );
  QCOMPARE( def->dynamicLayerParameterName(), QStringLiteral( "parent" ) );

  // string with dynamic property (feature not set)
  def->setName( QStringLiteral( "prop" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString() );

  // correctly setup feature
  QgsFields fields;
  fields.append( QgsField( "a_field", QVariant::String, QString(), 30 ) );
  QgsFeature f( fields );
  f.setAttribute( 0, QStringLiteral( "field value" ) );
  context.expressionContext().setFeature( f );
  context.expressionContext().setFields( fields );
  def->setName( QStringLiteral( "prop" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QStringLiteral( "field value" ) );

  // as double
  def->setName( QStringLiteral( "double" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsDouble( def.get(), params, context ), 5.2 );
  def->setName( QStringLiteral( "int" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsDouble( def.get(), params, context ), 15.0 );
  f.setAttribute( 0, QStringLiteral( "6.2" ) );
  context.expressionContext().setFeature( f );
  def->setName( QStringLiteral( "prop" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsDouble( def.get(), params, context ), 6.2 );

  // as int
  def->setName( QStringLiteral( "double" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsInt( def.get(), params, context ), 5 );
  def->setName( QStringLiteral( "int" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsInt( def.get(), params, context ), 15 );
  def->setName( QStringLiteral( "prop" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsInt( def.get(), params, context ), 6 );

  // as ints
  def->setName( QStringLiteral( "int" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsInts( def.get(), params, context ), QList<int>() << 15 );
  def->setName( QStringLiteral( "ints" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsInts( def.get(), params, context ), QList<int>() << 3 << 2 << 1 );

  // as bool
  def->setName( QStringLiteral( "double" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  def->setName( QStringLiteral( "int" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  def->setName( QStringLiteral( "bool" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  def->setName( QStringLiteral( "prop" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  f.setAttribute( 0, false );
  context.expressionContext().setFeature( f );
  def->setName( QStringLiteral( "prop" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );

  // as layer
  def->setName( QStringLiteral( "double" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def.get(), params, context ) );
  def->setName( QStringLiteral( "int" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def.get(),  params, context ) );
  def->setName( QStringLiteral( "bool" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def.get(), params, context ) );
  def->setName( QStringLiteral( "prop" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def.get(), params, context ) );

  QVERIFY( context.temporaryLayerStore()->mapLayers().isEmpty() );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  f.setAttribute( 0, testDataDir + "/raster/band1_float32_noct_epsg4326.tif" );
  context.expressionContext().setFeature( f );
  def->setName( QStringLiteral( "prop" ) );
  QVERIFY( QgsProcessingParameters::parameterAsLayer( def.get(), params, context ) );
  // make sure layer was loaded
  QVERIFY( !context.temporaryLayerStore()->mapLayers().isEmpty() );

  // parameters as sinks

  QgsWkbTypes::Type wkbType = QgsWkbTypes::PolygonM;
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "epsg:3111" ) );
  QString destId;
  def->setName( QStringLiteral( "string" ) );
  params.insert( QStringLiteral( "string" ), QStringLiteral( "memory:mem" ) );
  std::unique_ptr< QgsFeatureSink > sink;
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, fields, wkbType, crs, context, destId ) );
  QVERIFY( sink.get() );
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destId, context ) );
  QVERIFY( layer );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->fields().count(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "a_field" ) );
  QCOMPARE( layer->wkbType(), wkbType );
  QCOMPARE( layer->crs(), crs );

  // property defined sink destination
  params.insert( QStringLiteral( "prop" ), QgsProperty::fromExpression( "'memory:mem2'" ) );
  def->setName( QStringLiteral( "prop" ) );
  crs = QgsCoordinateReferenceSystem( QStringLiteral( "epsg:3113" ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, fields, wkbType, crs, context, destId ) );
  QVERIFY( sink.get() );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destId, context ) );
  QVERIFY( layer );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->fields().count(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "a_field" ) );
  QCOMPARE( layer->wkbType(), wkbType );
  QCOMPARE( layer->crs(), crs );

  // QgsProcessingFeatureSinkDefinition as parameter
  QgsProject p;
  QgsProcessingOutputLayerDefinition fs( QStringLiteral( "test.shp" ) );
  fs.destinationProject = &p;
  QVERIFY( context.layersToLoadOnCompletion().isEmpty() );
  params.insert( QStringLiteral( "fs" ), QVariant::fromValue( fs ) );
  def->setName( QStringLiteral( "fs" ) );
  crs = QgsCoordinateReferenceSystem( QStringLiteral( "epsg:28356" ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, fields, wkbType, crs, context, destId ) );
  QVERIFY( sink.get() );
  QgsVectorFileWriter *writer = dynamic_cast< QgsVectorFileWriter *>( dynamic_cast< QgsProcessingFeatureSink * >( sink.get() )->destinationSink() );
  QVERIFY( writer );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destId, context ) );
  QVERIFY( layer );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->wkbType(), wkbType );
  QCOMPARE( layer->crs(), crs );

  // make sure layer was automatically added to list to load on completion
  QCOMPARE( context.layersToLoadOnCompletion().size(), 1 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), destId );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "desc" ) );

  // with name overloading
  QgsProcessingContext context2;
  fs = QgsProcessingOutputLayerDefinition( QStringLiteral( "test.shp" ) );
  fs.destinationProject = &p;
  fs.destinationName = QStringLiteral( "my_dest" );
  params.insert( QStringLiteral( "fs" ), QVariant::fromValue( fs ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, fields, wkbType, crs, context2, destId ) );
  QVERIFY( sink.get() );
  QCOMPARE( context2.layersToLoadOnCompletion().size(), 1 );
  QCOMPARE( context2.layersToLoadOnCompletion().keys().at( 0 ), destId );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "my_dest" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).outputName, QStringLiteral( "fs" ) );
}

void TestQgsProcessing::algorithmParameters()
{
  DummyAlgorithm *alg = new DummyAlgorithm( "test" );
  DummyProvider p( "test" );
  alg->runParameterChecks();

  p.addAlgorithm( alg );
  alg->runParameterChecks2();
}

void TestQgsProcessing::algorithmOutputs()
{
  DummyAlgorithm alg( "test" );
  alg.runOutputChecks();
}

void TestQgsProcessing::parameterGeneral()
{
  // test constructor
  QgsProcessingParameterBoolean param( "p1", "desc", true, true );
  QCOMPARE( param.name(), QString( "p1" ) );
  QCOMPARE( param.description(), QString( "desc" ) );
  QCOMPARE( param.defaultValue(), QVariant( true ) );
  QVERIFY( param.flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( param.dependsOnOtherParameters().isEmpty() );

  // test getters and setters
  param.setDescription( "p2" );
  QCOMPARE( param.description(), QString( "p2" ) );
  param.setDefaultValue( false );
  QCOMPARE( param.defaultValue(), QVariant( false ) );
  param.setFlags( QgsProcessingParameterDefinition::FlagHidden );
  QCOMPARE( param.flags(), QgsProcessingParameterDefinition::FlagHidden );
  param.setDefaultValue( true );
  QCOMPARE( param.defaultValue(), QVariant( true ) );
  param.setDefaultValue( QVariant() );
  QCOMPARE( param.defaultValue(), QVariant() );

  QVariantMap metadata;
  metadata.insert( "p1", 5 );
  metadata.insert( "p2", 7 );
  param.setMetadata( metadata );
  QCOMPARE( param.metadata(), metadata );
  param.metadata().insert( "p3", 9 );
  QCOMPARE( param.metadata().value( "p3" ).toInt(), 9 );

  QVariantMap map = param.toVariantMap();
  QgsProcessingParameterBoolean fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), param.name() );
  QCOMPARE( fromMap.description(), param.description() );
  QCOMPARE( fromMap.flags(), param.flags() );
  QCOMPARE( fromMap.defaultValue(), param.defaultValue() );
  QCOMPARE( fromMap.metadata(), param.metadata() );
}

void TestQgsProcessing::parameterBoolean()
{
  QgsProcessingContext context;

  // test no def
  QVariantMap params;
  params.insert( "no_def",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, context ), false );
  params.insert( "no_def",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, context ), false );
  params.insert( "no_def",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, context ), false );
  params.remove( "no_def" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, context ), false );

  // with defs

  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterBoolean( "non_optional_default_false" ) );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( "false" ) );
  QVERIFY( def->checkValueIsAcceptable( "true" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "non_optional_default_false",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  params.insert( "non_optional_default_false",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "non_optional_default_false",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "non_optional_default_false",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );

  //non-optional - behavior is undefined, but internally default to false
  params.insert( "non_optional_default_false",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  params.remove( "non_optional_default_false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );

  QCOMPARE( def->valueAsPythonString( false, context ), QStringLiteral( "False" ) );
  QCOMPARE( def->valueAsPythonString( true, context ), QStringLiteral( "True" ) );
  QCOMPARE( def->valueAsPythonString( "false", context ), QStringLiteral( "False" ) );
  QCOMPARE( def->valueAsPythonString( "true", context ), QStringLiteral( "True" ) );
  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional_default_false=boolean false" ) );
  std::unique_ptr< QgsProcessingParameterBoolean > fromCode( dynamic_cast< QgsProcessingParameterBoolean * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional default false" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toBool(), false );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterBoolean fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( QgsProcessingParameters::parameterFromVariantMap( map ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterBoolean *>( def.get() ) );


  def.reset( new QgsProcessingParameterBoolean( "optional_default_true", QString(), true, true ) );

  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( "false" ) );
  QVERIFY( def->checkValueIsAcceptable( "true" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional_default_true",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  params.insert( "optional_default_true",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "optional_default_true",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "optional_default_true",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  //optional - should be default
  params.insert( "optional_default_true",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.remove( "optional_default_true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional_default_true=optional boolean true" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterBoolean * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional default true" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toBool(), true );

  def.reset( new QgsProcessingParameterBoolean( "optional_default_false", QString(), false, true ) );

  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( "false" ) );
  QVERIFY( def->checkValueIsAcceptable( "true" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( "false" ) );
  QVERIFY( def->checkValueIsAcceptable( "true" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional_default_false",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  params.insert( "optional_default_false",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "optional_default_false",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "optional_default_false",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  //optional - should be default
  params.insert( "optional_default_false",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  params.remove( "optional_default_false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );

  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterBoolean * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional default false" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toBool(), false );

  def.reset( new QgsProcessingParameterBoolean( "non_optional_default_true", QString(), true, false ) );

  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( "false" ) );
  QVERIFY( def->checkValueIsAcceptable( "true" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be acceptable, because it falls back to default value

  params.insert( "non_optional_default_true",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  params.insert( "non_optional_default_true",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "non_optional_default_true",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.insert( "non_optional_default_true",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), false );
  //non-optional - behavior is undefined, but internally fallback to default
  params.insert( "non_optional_default_true",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );
  params.remove( "non_optional_default_true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def.get(), params, context ), true );

  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterBoolean * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional default true" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toBool(), true );

  def.reset( new QgsProcessingParameterBoolean( "non_optional_no_default", QString(),  QVariant(), false ) );

  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( "false" ) );
  QVERIFY( def->checkValueIsAcceptable( "true" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) ); // should NOT be acceptable, because it falls back to invalid default value
}

void TestQgsProcessing::parameterCrs()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterCrs > def( new QgsProcessingParameterCrs( "non_optional", QString(), QString( "EPSG:3113" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "EPSG:12003" ) );
  QVERIFY( def->checkValueIsAcceptable( "EPSG:3111" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsCoordinateReferenceSystem() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( r1->id() ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( QgsProperty::fromValue( QVariant::fromValue( r1 ) ) ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( r1->id() ) ) );

  // using map layer
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:3111" ) );
  QVERIFY( def->checkValueIsAcceptable( v1->id() ) );
  params.insert( "non_optional",  QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:3111" ) );

  // using QgsCoordinateReferenceSystem
  params.insert( "non_optional",  QgsCoordinateReferenceSystem( "EPSG:28356" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:28356" ) );
  params.insert( "non_optional",  QgsCoordinateReferenceSystem() );
  QVERIFY( !QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).isValid() );

  // special ProjectCrs string
  params.insert( "non_optional",  QStringLiteral( "ProjectCrs" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:28353" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringLiteral( "ProjectCrs" ) ) );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:4326" ) );
  QVERIFY( def->checkValueIsAcceptable( raster1 ) );

  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:32633" ) );
  QVERIFY( def->checkValueIsAcceptable( raster2 ) );

  // string representation of a crs
  params.insert( "non_optional", QString( "EPSG:28355" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:28355" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringLiteral( "EPSG:28355" ) ) );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a crs, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).isValid() );

  // using feature source definition
  params.insert( "non_optional",  QgsProcessingFeatureSourceDefinition( v1->id() ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:3111" ) );
  params.insert( "non_optional",  QgsProcessingFeatureSourceDefinition( QgsProperty::fromValue( QVariant::fromValue( v1 ) ) ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:3111" ) );
  params.insert( "non_optional",  QgsProcessingOutputLayerDefinition( v1->id() ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:3111" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QgsCoordinateReferenceSystem( "EPSG:3111" ), context ), QStringLiteral( "QgsCoordinateReferenceSystem('EPSG:3111')" ) );
  QCOMPARE( def->valueAsPythonString( QgsCoordinateReferenceSystem(), context ), QStringLiteral( "QgsCoordinateReferenceSystem()" ) );
  QCOMPARE( def->valueAsPythonString( "EPSG:12003", context ), QStringLiteral( "'EPSG:12003'" ) );
  QCOMPARE( def->valueAsPythonString( "ProjectCrs", context ), QStringLiteral( "'ProjectCrs'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );
  QCOMPARE( def->valueAsPythonString( raster1, context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( r1->id(), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterCrs fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterCrs *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterCrs *>( def.get() ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=crs EPSG:3113" ) );
  std::unique_ptr< QgsProcessingParameterCrs > fromCode( dynamic_cast< QgsProcessingParameterCrs * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // optional
  def.reset( new QgsProcessingParameterCrs( "optional", QString(), QString( "EPSG:3113" ), true ) );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def.get(), params, context ).authid(), QString( "EPSG:3113" ) );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "EPSG:12003" ) );
  QVERIFY( def->checkValueIsAcceptable( "EPSG:3111" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional crs EPSG:3113" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterCrs * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  code = QStringLiteral( "##optional=optional crs None" );
  fromCode.reset( dynamic_cast< QgsProcessingParameterCrs * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );
}

void TestQgsProcessing::parameterLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterMapLayer > def( new QgsProcessingParameterMapLayer( "non_optional", QString(), QString(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context )->id(), v1->id() );
  QVERIFY( def->checkValueIsAcceptable( v1->id() ) );
  QVERIFY( def->checkValueIsAcceptable( v1->id(), &context ) );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QVERIFY( def->checkValueIsAcceptable( raster1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context )->id(), r1->id() );
  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QVERIFY( def->checkValueIsAcceptable( raster2 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context )->publicSource(), raster2 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def.get(), params, context ) );

  // layer
  params.insert( "non_optional", QVariant::fromValue( r1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context ), r1 );
  params.insert( "non_optional", QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context ), v1 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( raster1, context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( r1->id(), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( r1 ), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=layer" ) );
  std::unique_ptr< QgsProcessingParameterMapLayer > fromCode( dynamic_cast< QgsProcessingParameterMapLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterMapLayer fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterMapLayer *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterMapLayer *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterMapLayer( "optional", QString(), v1->id(), true ) );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context )->id(), v1->id() );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional layer " ) + v1->id() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMapLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // check if can manage QgsProcessingOutputLayerDefinition
  // as QVariat value in parameters (e.g. coming from an input of
  // another algorithm)

  // all ok
  def.reset( new QgsProcessingParameterMapLayer( "non_optional", QString(), r1->id(), true ) );
  QString sink_name( r1->id() );
  QgsProcessingOutputLayerDefinition val( sink_name );
  params.insert( "non_optional", QVariant::fromValue( val ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def.get(), params, context )->id(), r1->id() );

  // not ok, e.g. source name is not a layer and it's not possible to generate a layer from it source
  def.reset( new QgsProcessingParameterMapLayer( "non_optional", QString(), r1->id(), true ) );
  sink_name = QString( "i'm not a layer, and nothing you can do will make me one" );
  QgsProcessingOutputLayerDefinition val2( sink_name );
  params.insert( "non_optional", QVariant::fromValue( val2 ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def.get(), params, context ) );
}

void TestQgsProcessing::parameterExtent()
{
  // setup a context
  QgsProject p;
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  p.addMapLayers( QList<QgsMapLayer *>() << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterExtent > def( new QgsProcessingParameterExtent( "non_optional", QString(), QString( "1,2,3,4" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2,3,4" ) );
  QVERIFY( def->checkValueIsAcceptable( "    1, 2   ,3  , 4  " ) );
  QVERIFY( def->checkValueIsAcceptable( "    1, 2   ,3  , 4  ", &context ) );
  QVERIFY( def->checkValueIsAcceptable( "-1.1,2,-3,-4" ) );
  QVERIFY( def->checkValueIsAcceptable( "-1.1,2,-3,-4", &context ) );
  QVERIFY( def->checkValueIsAcceptable( "-1.1,-2.2,-3.3,-4.4" ) );
  QVERIFY( def->checkValueIsAcceptable( "-1.1,-2.2,-3.3,-4.4", &context ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2,3,4.4[EPSG:4326]" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2,3,4.4[EPSG:4326]", &context ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2,3,4.4 [EPSG:4326]" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2,3,4.4 [EPSG:4326]", &context ) );
  QVERIFY( def->checkValueIsAcceptable( "  -1.1,   -2,    -3,   -4.4   [EPSG:4326]    " ) );
  QVERIFY( def->checkValueIsAcceptable( "  -1.1,   -2,    -3,   -4.4   [EPSG:4326]    ", &context ) );
  QVERIFY( def->checkValueIsAcceptable( "121774.38859446358,948723.6921024882,-264546.200347173,492749.6672022904 [EPSG:3785]" ) );
  QVERIFY( def->checkValueIsAcceptable( "121774.38859446358,948723.6921024882,-264546.200347173,492749.6672022904 [EPSG:3785]", &context ) );

  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsRectangle( 1, 2, 3, 4 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsRectangle() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsReferencedRectangle( QgsRectangle( 1, 2, 3, 4 ), QgsCoordinateReferenceSystem( "EPSG:4326" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsReferencedRectangle( QgsRectangle(), QgsCoordinateReferenceSystem( "EPSG:4326" ) ) ) );

  // these checks require a context - otherwise we could potentially be referring to a layer source
  QVERIFY( def->checkValueIsAcceptable( "1,2,3" ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2,3,a" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1,2,3", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( "1,2,3,a", &context ) );

  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( r1->id() ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( r1->id() ) ) );

  // using map layer
  QVariantMap params;
  params.insert( "non_optional",  r1->id() );
  QVERIFY( def->checkValueIsAcceptable( r1->id() ) );
  QgsRectangle ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QCOMPARE( ext, r1->extent() );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QVERIFY( def->checkValueIsAcceptable( raster1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtent( def.get(), params, context ),  r1->extent() );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );

  // layer as parameter
  params.insert( "non_optional", QVariant::fromValue( r1 ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtent( def.get(), params, context ),  r1->extent() );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );
  QgsGeometry gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 5 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );

  // using feature source definition
  params.insert( "non_optional",  QgsProcessingFeatureSourceDefinition( r1->id() ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );
  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 5 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );
  params.insert( "non_optional",  QgsProcessingFeatureSourceDefinition( QgsProperty::fromValue( QVariant::fromValue( r1 ) ) ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );
  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 5 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );

  // using output layer definition, e.g. from a previous model child algorithm
  params.insert( "non_optional",  QgsProcessingOutputLayerDefinition( r1->id() ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );
  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 5 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 1535375, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  5083255, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 100 );

  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QVERIFY( def->checkValueIsAcceptable( raster2 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:32633" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QGSCOMPARENEAR( ext.xMinimum(), 781662.375000, 10 );
  QGSCOMPARENEAR( ext.xMaximum(), 793062.375000, 10 );
  QGSCOMPARENEAR( ext.yMinimum(),  3339523.125000, 10 );
  QGSCOMPARENEAR( ext.yMaximum(), 3350923.125000, 10 );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 17.924273, 0.01 );
  QGSCOMPARENEAR( ext.xMaximum(), 18.045658, 0.01 );
  QGSCOMPARENEAR( ext.yMinimum(),  30.151856, 0.01 );
  QGSCOMPARENEAR( ext.yMaximum(), 30.257289, 0.01 );
  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 85 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 17.924273, 0.01 );
  QGSCOMPARENEAR( ext.xMaximum(), 18.045658, 0.01 );
  QGSCOMPARENEAR( ext.yMinimum(),  30.151856, 0.01 );
  QGSCOMPARENEAR( ext.yMaximum(), 30.257289, 0.01 );

  // string representation of an extent
  params.insert( "non_optional", QString( "1.1,2.2,3.3,4.4" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringLiteral( "1.1,2.2,3.3,4.4" ) ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QGSCOMPARENEAR( ext.xMinimum(), 1.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 2.2, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  3.3, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 4.4, 0.001 );

  // with target CRS - should make no difference, because source CRS is unknown
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 2.2, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  3.3, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 4.4, 0.001 );

  // with crs in string
  params.insert( "non_optional", QString( "1.1,3.3,2.2,4.4 [EPSG:4326]" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QGSCOMPARENEAR( ext.xMinimum(), 1.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 3.3, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  2.2, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 4.4, 0.001 );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 122451, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 367354, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  244963, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 490287, 100 );
  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 85 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 122451, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 367354, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  244963, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 490287, 100 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a crs, and nothing you can do will make me one" ) );
  QVERIFY( QgsProcessingParameters::parameterAsExtent( def.get(), params, context ).isNull() );

  // QgsRectangle
  params.insert( "non_optional", QgsRectangle( 11.1, 12.2, 13.3, 14.4 ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QGSCOMPARENEAR( ext.xMinimum(), 11.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 13.3, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  12.2, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 14.4, 0.001 );

  // with target CRS - should make no difference, because source CRS is unknown
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 11.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 13.3, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  12.2, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 14.4, 0.001 );

  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QCOMPARE( gExt.asWkt( 1 ), QStringLiteral( "Polygon ((11.1 12.2, 13.3 12.2, 13.3 14.4, 11.1 14.4, 11.1 12.2))" ) );

  p.setCrs( QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:3785" ) );

  // QgsReferencedRectangle
  params.insert( "non_optional", QgsReferencedRectangle( QgsRectangle( 1.1, 2.2, 3.3, 4.4 ), QgsCoordinateReferenceSystem( "EPSG:4326" ) ) );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QGSCOMPARENEAR( ext.xMinimum(), 1.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 3.3, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  2.2, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 4.4, 0.001 );
  QCOMPARE( QgsProcessingParameters::parameterAsExtentCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );

  // with target CRS
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( ext.xMinimum(), 122451, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 367354, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  244963, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 490287, 100 );

  // as reprojected geometry
  gExt = QgsProcessingParameters::parameterAsExtentGeometry( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QCOMPARE( gExt.constGet()->vertexCount(), 85 );
  ext = gExt.boundingBox();
  QGSCOMPARENEAR( ext.xMinimum(), 122451, 100 );
  QGSCOMPARENEAR( ext.xMaximum(), 367354, 100 );
  QGSCOMPARENEAR( ext.yMinimum(),  244963, 100 );
  QGSCOMPARENEAR( ext.yMaximum(), 490287, 100 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( "1,2,3,4", context ), QStringLiteral( "'1,2,3,4'" ) );
  QCOMPARE( def->valueAsPythonString( r1->id(), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( r1 ), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( raster2, context ), QString( "'" ) + testDataDir + QStringLiteral( "landsat.tif'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( QgsRectangle( 11, 12, 13, 14 ), context ), QStringLiteral( "'11, 13, 12, 14'" ) );
  QCOMPARE( def->valueAsPythonString( QgsReferencedRectangle( QgsRectangle( 11, 12, 13, 14 ), QgsCoordinateReferenceSystem( "epsg:4326" ) ), context ), QStringLiteral( "'11, 13, 12, 14 [EPSG:4326]'" ) );
  QCOMPARE( def->valueAsPythonString( "1,2,3,4 [EPSG:4326]", context ), QStringLiteral( "'1,2,3,4 [EPSG:4326]'" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=extent 1,2,3,4" ) );
  std::unique_ptr< QgsProcessingParameterExtent > fromCode( dynamic_cast< QgsProcessingParameterExtent * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterExtent fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterExtent *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterExtent *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterExtent( "optional", QString(), QString( "5,6,7,8" ), true ) );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2,3,4" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  // Extent is unique in that it will let you set invalid, whereas other
  // optional parameters become "default" when assigning invalid.
  params.insert( "optional",  QVariant() );
  ext = QgsProcessingParameters::parameterAsExtent( def.get(), params, context );
  QVERIFY( ext.isNull() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional extent 5,6,7,8" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterExtent * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
}

void TestQgsProcessing::parameterPoint()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterPoint > def( new QgsProcessingParameterPoint( "non_optional", QString(), QString( "1,2" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( "(1.1,2)" ) );
  QVERIFY( def->checkValueIsAcceptable( "    1.1,  2  " ) );
  QVERIFY( def->checkValueIsAcceptable( " (    1.1,  2 ) " ) );
  QVERIFY( def->checkValueIsAcceptable( "-1.1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,-2" ) );
  QVERIFY( def->checkValueIsAcceptable( "-1.1,-2" ) );
  QVERIFY( def->checkValueIsAcceptable( "(-1.1,-2)" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2[EPSG:4326]" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2 [EPSG:4326]" ) );
  QVERIFY( def->checkValueIsAcceptable( "(1.1,2 [EPSG:4326] )" ) );
  QVERIFY( def->checkValueIsAcceptable( "  -1.1,   -2   [EPSG:4326]    " ) );
  QVERIFY( def->checkValueIsAcceptable( "  (  -1.1,   -2   [EPSG:4326]  )  " ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,a" ) );
  QVERIFY( !def->checkValueIsAcceptable( "(1.1,a)" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "(layer12312312)" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( "()" ) );
  QVERIFY( !def->checkValueIsAcceptable( " (  ) " ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsPointXY( 1, 2 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsReferencedPointXY( QgsPointXY( 1, 2 ), QgsCoordinateReferenceSystem( "EPSG:4326" ) ) ) );

  // string representing a point
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1,2.2" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2.2" ) );
  QgsPointXY point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QGSCOMPARENEAR( point.x(), 1.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 2.2, 0.001 );

  // with target CRS - should make no difference, because source CRS is unknown
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( point.x(), 1.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 2.2, 0.001 );

  // with optional brackets
  params.insert( "non_optional", QString( "(1.1,2.2)" ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QGSCOMPARENEAR( point.x(), 1.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 2.2, 0.001 );

  params.insert( "non_optional", QString( "  (   -1.1  ,-2.2  )  " ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QGSCOMPARENEAR( point.x(), -1.1, 0.001 );
  QGSCOMPARENEAR( point.y(), -2.2, 0.001 );

  // with CRS as string
  params.insert( "non_optional", QString( "1.1,2.2[EPSG:4326]" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsPointCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( point.x(), 122451, 100 );
  QGSCOMPARENEAR( point.y(), 244963, 100 );
  params.insert( "non_optional", QString( "1.1,2.2 [EPSG:4326]" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsPointCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( point.x(), 122451, 100 );
  QGSCOMPARENEAR( point.y(), 244963, 100 );

  params.insert( "non_optional", QString( "  ( 1.1,2.2   [EPSG:4326]   ) " ) );
  QCOMPARE( QgsProcessingParameters::parameterAsPointCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( point.x(), 122451, 100 );
  QGSCOMPARENEAR( point.y(), 244963, 100 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a crs, and nothing you can do will make me one" ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QCOMPARE( point.x(), 0.0 );
  QCOMPARE( point.y(), 0.0 );

  params.insert( "non_optional", QString( "   (   )  " ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QCOMPARE( point.x(), 0.0 );
  QCOMPARE( point.y(), 0.0 );

  // QgsPointXY
  params.insert( "non_optional", QgsPointXY( 11.1, 12.2 ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QGSCOMPARENEAR( point.x(), 11.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 12.2, 0.001 );

  // with target CRS - should make no difference, because source CRS is unknown
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( point.x(), 11.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 12.2, 0.001 );

  // QgsReferencedPointXY
  params.insert( "non_optional", QgsReferencedPointXY( QgsPointXY( 1.1, 2.2 ), QgsCoordinateReferenceSystem( "EPSG:4326" ) ) );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QGSCOMPARENEAR( point.x(), 1.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 2.2, 0.001 );
  QCOMPARE( QgsProcessingParameters::parameterAsPointCrs( def.get(), params, context ).authid(), QStringLiteral( "EPSG:4326" ) );

  // with target CRS
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context, QgsCoordinateReferenceSystem( "EPSG:3785" ) );
  QGSCOMPARENEAR( point.x(), 122451, 100 );
  QGSCOMPARENEAR( point.y(), 244963, 100 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( "1,2", context ), QStringLiteral( "'1,2'" ) );
  QCOMPARE( def->valueAsPythonString( "1,2 [EPSG:4326]", context ), QStringLiteral( "'1,2 [EPSG:4326]'" ) );
  QCOMPARE( def->valueAsPythonString( QgsPointXY( 11, 12 ), context ), QStringLiteral( "'11,12'" ) );
  QCOMPARE( def->valueAsPythonString( QgsReferencedPointXY( QgsPointXY( 11, 12 ), QgsCoordinateReferenceSystem( "epsg:4326" ) ), context ), QStringLiteral( "'11,12 [EPSG:4326]'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=point 1,2" ) );
  std::unique_ptr< QgsProcessingParameterPoint > fromCode( dynamic_cast< QgsProcessingParameterPoint * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterPoint fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterPoint *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterPoint *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterPoint( "optional", QString(), QString( "5.1,6.2" ), true ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,a" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  point = QgsProcessingParameters::parameterAsPoint( def.get(), params, context );
  QGSCOMPARENEAR( point.x(), 5.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 6.2, 0.001 );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional point 5.1,6.2" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterPoint * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
}

void TestQgsProcessing::parameterFile()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterFile > def( new QgsProcessingParameterFile( "non_optional", QString(), QgsProcessingParameterFile::File, QString(), QString( "abc.bmp" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "bricks.bmp" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( "  " ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // string representing a file
  QVariantMap params;
  params.insert( "non_optional", QString( "def.bmp" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsFile( def.get(), params, context ), QString( "def.bmp" ) );

  // with extension
  def.reset( new QgsProcessingParameterFile( "non_optional", QString(), QgsProcessingParameterFile::File, QStringLiteral( ".bmp" ), QString( "abc.bmp" ), false ) );
  QVERIFY( def->checkValueIsAcceptable( "bricks.bmp" ) );
  QVERIFY( def->checkValueIsAcceptable( "bricks.BMP" ) );
  QVERIFY( !def->checkValueIsAcceptable( "bricks.pcx" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( "bricks.bmp", context ), QStringLiteral( "'bricks.bmp'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=file abc.bmp" ) );
  std::unique_ptr< QgsProcessingParameterFile > fromCode( dynamic_cast< QgsProcessingParameterFile * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->behavior(), def->behavior() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterFile fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.extension(), def->extension() );
  QCOMPARE( fromMap.behavior(), def->behavior() );
  def.reset( dynamic_cast< QgsProcessingParameterFile *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterFile *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterFile( "optional", QString(), QgsProcessingParameterFile::File, QString(), QString( "gef.bmp" ),  true ) );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "bricks.bmp" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( "  " ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsFile( def.get(), params, context ), QString( "gef.bmp" ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional file gef.bmp" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFile * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->behavior(), def->behavior() );

  // folder
  def.reset( new QgsProcessingParameterFile( "optional", QString(), QgsProcessingParameterFile::Folder, QString(), QString( "/home/me" ),  true ) );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional folder /home/me" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFile * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->behavior(), def->behavior() );
}

void TestQgsProcessing::parameterMatrix()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterMatrix > def( new QgsProcessingParameterMatrix( "non_optional", QString(), 3, false, QStringList(), QString(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2,3" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1 << 2 << 3 ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << ( QVariantList() << 1 << 2 << 3 ) << ( QVariantList() << 1 << 2 << 3 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // list
  QVariantMap params;
  params.insert( "non_optional", QVariantList() << 1 << 2 << 3 );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def.get(), params, context ), QVariantList() << 1 << 2 << 3 );

  //string
  params.insert( "non_optional", QString( "4,5,6" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def.get(), params, context ), QVariantList() << 4 << 5 << 6 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "[5]" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << 1 << 2 << 3, context ), QStringLiteral( "[1,2,3]" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << ( QVariantList() << 1 << 2 << 3 ) << ( QVariantList() << 1 << 2 << 3 ), context ), QStringLiteral( "[1,2,3,1,2,3]" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << ( QVariantList() << 1 << QVariant() << 3 ) << ( QVariantList() << QVariant() << 2 << 3 ), context ), QStringLiteral( "[1,None,3,None,2,3]" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << ( QVariantList() << 1 << QString( "" ) << 3 ) << ( QVariantList() << 1 << 2 << QString( "" ) ), context ), QStringLiteral( "[1,'',3,1,2,'']" ) );
  QCOMPARE( def->valueAsPythonString( "1,2,3", context ), QStringLiteral( "[1,2,3]" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=matrix" ) );
  std::unique_ptr< QgsProcessingParameterMatrix > fromCode( dynamic_cast< QgsProcessingParameterMatrix * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterMatrix fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.headers(), def->headers() );
  QCOMPARE( fromMap.numberRows(), def->numberRows() );
  QCOMPARE( fromMap.hasFixedNumberRows(), def->hasFixedNumberRows() );
  def.reset( dynamic_cast< QgsProcessingParameterMatrix *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterMatrix *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterMatrix( "optional", QString(), 3, false, QStringList(), QVariantList() << 4 << 5 << 6,  true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2,3" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1 << 2 << 3 ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional matrix" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMatrix * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def.get(), params, context ), QVariantList() << 4 << 5 << 6 );
  def.reset( new QgsProcessingParameterMatrix( "optional", QString(), 3, false, QStringList(), QString( "1,2,3" ),  true ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional matrix 1,2,3" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMatrix * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def.get(), params, context ), QVariantList() << 1 << 2 << 3 );
}

void TestQgsProcessing::parameterLayerList()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V5", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << v2 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterMultipleLayers > def( new QgsProcessingParameterMultipleLayers( "non_optional", QString(), QgsProcessing::TypeMapLayer, QString(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );

  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );


  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 );
  // using existing map layer
  params.insert( "non_optional",  QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 );

  // using two existing map layer ID
  params.insert( "non_optional",  QVariantList() << v1->id() << r1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << r1 );

  // using two existing map layers
  params.insert( "non_optional",  QVariantList() << QVariant::fromValue( v1 ) << QVariant::fromValue( r1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << r1 );

  // mix of list and single layers (happens from models)
  params.insert( "non_optional",  QVariantList() << QVariant( QVariantList() << QVariant::fromValue( v1 ) << QVariant::fromValue( v2 ) ) << QVariant::fromValue( r1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << v2 << r1 );

  // mix of two lists (happens from models)
  params.insert( "non_optional",  QVariantList() << QVariant( QVariantList() << QVariant::fromValue( v1 ) << QVariant::fromValue( v2 ) ) << QVariant( QVariantList() << QVariant::fromValue( r1 ) ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << v2 << r1 );

  // mix of existing layers and non project layer string
  params.insert( "non_optional",  QVariantList() << v1->id() << raster2 );
  QList< QgsMapLayer *> layers = QgsProcessingParameters::parameterAsLayerList( def.get(), params, context );
  QCOMPARE( layers.at( 0 ), v1 );
  QCOMPARE( layers.at( 1 )->publicSource(), raster2 );

  // mix of existing layer and ID (and check we keep order)
  params.insert( "non_optional",  QVariantList() << QVariant::fromValue( v1 ) << v2->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << v2 );

  params.insert( "non_optional",  QVariantList() << v1->id() << QVariant::fromValue( v2 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << v2 );

  // empty string
  params.insert( "non_optional",  QString( "" ) );
  QVERIFY( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ).isEmpty() );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ).isEmpty() );

  // with 2 min inputs
  def->setMinimumNumberInputs( 2 );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() ) );
  QVERIFY( def->checkValueIsAcceptable( QStringList() << "layer12312312" << "layerB" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << "layer12312312" << "layerB" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  def->setMinimumNumberInputs( 3 );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "layer12312312" << "layerB" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "layer12312312" << "layerB" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( "layer12312312", context ), QStringLiteral( "'layer12312312'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( r1 ), context ), QStringLiteral( "['" ) + testDataDir + QStringLiteral( "tenbytenraster.asc']" ) );
  QCOMPARE( def->valueAsPythonString( r1->id(), context ), QStringLiteral( "['" ) + testDataDir + QStringLiteral( "tenbytenraster.asc']" ) );
  QCOMPARE( def->valueAsPythonString( QStringList() << r1->id() << raster2, context ), QStringLiteral( "['" ) + testDataDir + QStringLiteral( "tenbytenraster.asc','" ) + testDataDir + QStringLiteral( "landsat.tif']" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=multiple vector" ) );
  std::unique_ptr< QgsProcessingParameterMultipleLayers > fromCode( dynamic_cast< QgsProcessingParameterMultipleLayers * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );
  QCOMPARE( fromCode->layerType(), QgsProcessing::TypeVectorAnyGeometry );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterMultipleLayers fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.layerType(), def->layerType() );
  QCOMPARE( fromMap.minimumNumberInputs(), def->minimumNumberInputs() );
  def.reset( dynamic_cast< QgsProcessingParameterMultipleLayers *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterMultipleLayers *>( def.get() ) );

  // optional with one default layer
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeMapLayer, v1->id(), true ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );

  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 );

  params.insert( "optional",  QVariantList() << QVariant::fromValue( r1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << r1 );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional multiple vector " ) + v1->id() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMultipleLayers * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->layerType(), QgsProcessing::TypeVectorAnyGeometry );

  // optional with two default layers
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeMapLayer, QVariantList() << v1->id() << r1->publicSource(), true ) );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << r1 );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional multiple vector " ) + v1->id() + "," + r1->publicSource() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMultipleLayers * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), v1->id() + "," + r1->publicSource() );
  QCOMPARE( fromCode->layerType(), QgsProcessing::TypeVectorAnyGeometry );

  // optional with one default direct layer
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeMapLayer, QVariant::fromValue( v1 ), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 );

  // optional with two default direct layers
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeMapLayer, QVariantList() << QVariant::fromValue( v1 ) << QVariant::fromValue( r1 ), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << v1 << r1 );

  def.reset( new QgsProcessingParameterMultipleLayers( "type", QString(), QgsProcessing::TypeRaster ) );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##type=multiple raster" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMultipleLayers * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "type" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );
  QCOMPARE( fromCode->layerType(), QgsProcessing::TypeRaster );

  def.reset( new QgsProcessingParameterMultipleLayers( "type", QString(), QgsProcessing::TypeFile ) );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##type=multiple file" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMultipleLayers * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "type" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );
  QCOMPARE( fromCode->layerType(), def->layerType() );

  // manage QgsProcessingOutputLayerDefinition as parameter value

  // optional with sink to a QgsMapLayer.id()
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeFile ) );
  params.insert( QString( "optional" ), QgsProcessingOutputLayerDefinition( r1->publicSource() ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() << r1 );

  // optional with sink to an empty string
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeFile ) );
  params.insert( QString( "optional" ), QgsProcessingOutputLayerDefinition( QString() ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() );

  // optional with sink to an nonsense string
  def.reset( new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessing::TypeFile ) );
  params.insert( QString( "optional" ), QgsProcessingOutputLayerDefinition( QString( "i'm not a layer, and nothing you can do will make me one" ) ) );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def.get(), params, context ), QList< QgsMapLayer *>() );
}

void TestQgsProcessing::parameterDistance()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterDistance > def( new QgsProcessingParameterDistance( "non_optional", QString(), 5, QStringLiteral( "parent" ), false ) );
  QCOMPARE( def->parentParameterName(), QStringLiteral( "parent" ) );
  def->setParentParameterName( QStringLiteral( "parent2" ) );
  QCOMPARE( def->defaultUnit(), QgsUnitTypes::DistanceUnknownUnit );
  def->setDefaultUnit( QgsUnitTypes::DistanceFeet );
  QCOMPARE( def->defaultUnit(), QgsUnitTypes::DistanceFeet );
  std::unique_ptr< QgsProcessingParameterDistance > clone( def->clone() );
  QCOMPARE( clone->parentParameterName(), QStringLiteral( "parent2" ) );
  QCOMPARE( clone->defaultUnit(), QgsUnitTypes::DistanceFeet );

  QCOMPARE( def->parentParameterName(), QStringLiteral( "parent2" ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be acceptable, falls back to default value

  // string representing a number
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1" ) );
  double number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 1.1, 0.001 );

  // double
  params.insert( "non_optional", 1.1 );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 1.1, 0.001 );
  // int
  params.insert( "non_optional", 1 );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 1, 0.001 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a number, and nothing you can do will make me one" ) );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QCOMPARE( number, 5.0 );

  // with min value
  def->setMinimum( 11 );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( def->checkValueIsAcceptable( 25 ) );
  QVERIFY( def->checkValueIsAcceptable( "21.1" ) );
  // with max value
  def->setMaximum( 21 );
  QVERIFY( !def->checkValueIsAcceptable( 35 ) );
  QVERIFY( !def->checkValueIsAcceptable( "31.1" ) );
  QVERIFY( def->checkValueIsAcceptable( 15 ) );
  QVERIFY( def->checkValueIsAcceptable( "11.1" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "5" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "1.1" ), context ), QStringLiteral( "1.1" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterDistance fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.minimum(), def->minimum() );
  QCOMPARE( fromMap.maximum(), def->maximum() );
  QCOMPARE( fromMap.dataType(), def->dataType() );
  QCOMPARE( fromMap.parentParameterName(), QStringLiteral( "parent2" ) );
  QCOMPARE( fromMap.defaultUnit(), QgsUnitTypes::DistanceFeet );
  def.reset( dynamic_cast< QgsProcessingParameterDistance *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterDistance *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterDistance( "optional", QString(), 5.4, QStringLiteral( "parent" ), true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 5.4, 0.001 );
  // unconvertible string
  params.insert( "optional",  QVariant( "aaaa" ) );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 5.4, 0.001 );

  // non-optional, invalid default
  def.reset( new QgsProcessingParameterDistance( "non_optional", QString(), QVariant(), QStringLiteral( "parent" ), false ) );
  QCOMPARE( def->parentParameterName(), QStringLiteral( "parent" ) );
  def->setParentParameterName( QStringLiteral( "parent2" ) );
  QCOMPARE( def->parentParameterName(), QStringLiteral( "parent2" ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) ); // should NOT be acceptable, falls back to invalid default value
}

void TestQgsProcessing::parameterNumber()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterNumber > def( new QgsProcessingParameterNumber( "non_optional", QString(), QgsProcessingParameterNumber::Double, 5, false ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be acceptable, falls back to default value

  // string representing a number
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1" ) );
  double number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 1.1, 0.001 );
  int iNumber = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( iNumber, 1 );

  // double
  params.insert( "non_optional", 1.1 );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 1.1, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( iNumber, 1 );

  // int
  params.insert( "non_optional", 1 );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 1, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( iNumber, 1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a number, and nothing you can do will make me one" ) );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QCOMPARE( number, 5.0 );
  iNumber = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( iNumber, 5 );

  // with min value
  def->setMinimum( 11 );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( def->checkValueIsAcceptable( 25 ) );
  QVERIFY( def->checkValueIsAcceptable( "21.1" ) );
  // with max value
  def->setMaximum( 21 );
  QVERIFY( !def->checkValueIsAcceptable( 35 ) );
  QVERIFY( !def->checkValueIsAcceptable( "31.1" ) );
  QVERIFY( def->checkValueIsAcceptable( 15 ) );
  QVERIFY( def->checkValueIsAcceptable( "11.1" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "5" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "1.1" ), context ), QStringLiteral( "1.1" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=number 5" ) );
  std::unique_ptr< QgsProcessingParameterNumber > fromCode( dynamic_cast< QgsProcessingParameterNumber * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );


  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterNumber fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.minimum(), def->minimum() );
  QCOMPARE( fromMap.maximum(), def->maximum() );
  QCOMPARE( fromMap.dataType(), def->dataType() );
  def.reset( dynamic_cast< QgsProcessingParameterNumber *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterNumber *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterNumber( "optional", QString(), QgsProcessingParameterNumber::Double, 5.4, true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 5.4, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( iNumber, 5 );
  // unconvertible string
  params.insert( "optional",  QVariant( "aaaa" ) );
  number = QgsProcessingParameters::parameterAsDouble( def.get(), params, context );
  QGSCOMPARENEAR( number, 5.4, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( iNumber, 5 );

  code = def->asScriptCode();
  QCOMPARE( code.left( 30 ), QStringLiteral( "##optional=optional number 5.4" ) ); // truncate code to 30, to avoid Qt 5.6 rounding issues
  fromCode.reset( dynamic_cast< QgsProcessingParameterNumber * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  fromCode.reset( dynamic_cast< QgsProcessingParameterNumber * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##optional=optional number None" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );

  // non-optional, invalid default
  def.reset( new QgsProcessingParameterNumber( "non_optional", QString(), QgsProcessingParameterNumber::Double, QVariant(), false ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) ); // should NOT be acceptable, falls back to invalid default value
}

void TestQgsProcessing::parameterRange()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterRange > def( new QgsProcessingParameterRange( "non_optional", QString(), QgsProcessingParameterNumber::Double, QString( "5,6" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1" ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1.1,2,3" ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1,a" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1.1 << 2 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << 1.1 << 2 << 3 ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // string representing a range of numbers
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1,1.2" ) );
  QList< double > range = QgsProcessingParameters::parameterAsRange( def.get(), params, context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );

  // list
  params.insert( "non_optional", QVariantList() << 1.1 << 1.2 );
  range = QgsProcessingParameters::parameterAsRange( def.get(), params, context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );

  // too many elements:
  params.insert( "non_optional", QString( "1.1,1.2,1.3" ) );
  range = QgsProcessingParameters::parameterAsRange( def.get(), params, context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );
  params.insert( "non_optional", QVariantList() << 1.1 << 1.2 << 1.3 );
  range = QgsProcessingParameters::parameterAsRange( def.get(), params,  context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );

  // not enough elements - don't care about the result, just don't crash!
  params.insert( "non_optional", QString( "1.1" ) );
  range = QgsProcessingParameters::parameterAsRange( def.get(), params, context );
  params.insert( "non_optional", QVariantList() << 1.1 );
  range = QgsProcessingParameters::parameterAsRange( def.get(), params, context );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( "1.1,2", context ), QStringLiteral( "[1.1,2]" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << 1.1 << 2, context ), QStringLiteral( "[1.1,2]" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=range 5,6" ) );
  std::unique_ptr< QgsProcessingParameterRange > fromCode( dynamic_cast< QgsProcessingParameterRange * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );


  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterRange fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.dataType(), def->dataType() );
  def.reset( dynamic_cast< QgsProcessingParameterRange *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterRange *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterRange( "optional", QString(), QgsProcessingParameterNumber::Double, QString( "5.4,7.4" ), true ) );
  QVERIFY( def->checkValueIsAcceptable( "1.1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1.1 << 2 ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  range = QgsProcessingParameters::parameterAsRange( def.get(), params, context );
  QGSCOMPARENEAR( range.at( 0 ), 5.4, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 7.4, 0.001 );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional range 5.4,7.4" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterRange * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
}

void TestQgsProcessing::parameterRasterLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterRasterLayer > def( new QgsProcessingParameterRasterLayer( "non_optional", QString(), QVariant(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.tif" ) );
  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.tif", &context ) );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  r1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->id(), r1->id() );

  // using existing map layer
  params.insert( "non_optional",  QVariant::fromValue( r1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->id(), r1->id() );

  // not raster layer
  params.insert( "non_optional",  v1->id() );
  QVERIFY( !QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context ) );

  // using existing vector layer
  params.insert( "non_optional",  QVariant::fromValue( v1 ) );
  QVERIFY( !QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context ) );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->id(), r1->id() );
  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->publicSource(), raster2 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( raster1, context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( r1->id(), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( r1 ), context ), QString( "'" ) + testDataDir + QStringLiteral( "tenbytenraster.asc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=raster" ) );
  std::unique_ptr< QgsProcessingParameterRasterLayer > fromCode( dynamic_cast< QgsProcessingParameterRasterLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // optional
  def.reset( new QgsProcessingParameterRasterLayer( "optional", QString(), r1->id(), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->id(), r1->id() );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.tif" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->id(), r1->id() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional raster " ) + r1->id() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterRasterLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterRasterLayer fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterRasterLayer *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterRasterLayer *>( def.get() ) );

  // optional with direct layer
  def.reset( new QgsProcessingParameterRasterLayer( "optional", QString(), QVariant::fromValue( r1 ), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def.get(), params, context )->id(), r1->id() );

  // invalidRasterError
  params.clear();
  QCOMPARE( QgsProcessingAlgorithm::invalidRasterError( params, QStringLiteral( "MISSING" ) ), QStringLiteral( "Could not load source layer for MISSING: no value specified for parameter" ) );
  params.insert( QStringLiteral( "INPUT" ), QStringLiteral( "my layer" ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidRasterError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: my layer not found" ) );
  params.insert( QStringLiteral( "INPUT" ), QgsProperty::fromValue( "my prop layer" ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidRasterError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: my prop layer not found" ) );
  params.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidRasterError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: invalid value" ) );
}

void TestQgsProcessing::parameterEnum()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterEnum > def( new QgsProcessingParameterEnum( "non_optional", QString(), QStringList() << "A" << "B" << "C", false, 2, false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( 0 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << 1 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" ) );
  QVERIFY( !def->checkValueIsAcceptable( 15 ) );
  QVERIFY( !def->checkValueIsAcceptable( -1 ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be acceptable, because falls back to default value

  // string representing a number
  QVariantMap params;
  params.insert( "non_optional", QString( "1" ) );
  int iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 1 );

  // double
  params.insert( "non_optional", 2.2 );
  iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 2 );

  // int
  params.insert( "non_optional", 1 );
  iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a number, and nothing you can do will make me one" ) );
  iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 2 );

  // out of range
  params.insert( "non_optional", 4 );
  iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 2 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "5" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "1.1" ), context ), QStringLiteral( "1" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=enum A;B;C 2" ) );
  std::unique_ptr< QgsProcessingParameterEnum > fromCode( dynamic_cast< QgsProcessingParameterEnum * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->options(), def->options() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterEnum fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.options(), def->options() );
  QCOMPARE( fromMap.allowMultiple(), def->allowMultiple() );
  def.reset( dynamic_cast< QgsProcessingParameterEnum *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterEnum *>( def.get() ) );

  // multiple
  def.reset( new QgsProcessingParameterEnum( "non_optional", QString(), QStringList() << "A" << "B" << "C", true, 5, false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( 0 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) ); // since non-optional, empty list not allowed
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" ) );
  QVERIFY( !def->checkValueIsAcceptable( 15 ) );
  QVERIFY( !def->checkValueIsAcceptable( -1 ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "non_optional", QString( "1,2" ) );
  QList< int > iNumbers = QgsProcessingParameters::parameterAsEnums( def.get(), params, context );
  QCOMPARE( iNumbers, QList<int>() << 1 << 2 );
  params.insert( "non_optional", QVariantList() << 0 << 2 );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def.get(), params, context );
  QCOMPARE( iNumbers, QList<int>() << 0 << 2 );

  // empty list
  params.insert( "non_optional", QVariantList() );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def.get(), params, context );
  QCOMPARE( iNumbers, QList<int>() );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << 1 << 2, context ), QStringLiteral( "[1,2]" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "1,2" ), context ), QStringLiteral( "[1,2]" ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=enum multiple A;B;C 5" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterEnum * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->options(), def->options() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  // optional
  def.reset( new QgsProcessingParameterEnum( "optional", QString(), QStringList() << "a" << "b", false, 5, true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( 0 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << 1 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" ) );
  QVERIFY( !def->checkValueIsAcceptable( 15 ) );
  QVERIFY( !def->checkValueIsAcceptable( -1 ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional enum a;b 5" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterEnum * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->options(), def->options() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  params.insert( "optional",  QVariant() );
  iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 5 );
  // unconvertible string
  params.insert( "optional",  QVariant( "aaaa" ) );
  iNumber = QgsProcessingParameters::parameterAsEnum( def.get(), params, context );
  QCOMPARE( iNumber, 5 );
  //optional with multiples
  def.reset( new QgsProcessingParameterEnum( "optional", QString(), QStringList() << "A" << "B" << "C", true, QVariantList() << 1 << 2, true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( def->checkValueIsAcceptable( "1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( 0 ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" ) );
  QVERIFY( !def->checkValueIsAcceptable( 15 ) );
  QVERIFY( !def->checkValueIsAcceptable( -1 ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def.get(), params, context );
  QCOMPARE( iNumbers, QList<int>() << 1 << 2 );
  def.reset( new QgsProcessingParameterEnum( "optional", QString(), QStringList() << "A" << "B" << "C", true, "1,2", true ) );
  params.insert( "optional",  QVariant() );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def.get(), params, context );
  QCOMPARE( iNumbers, QList<int>() << 1 << 2 );
  // empty list
  params.insert( "optional", QVariantList() );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def.get(), params, context );
  QCOMPARE( iNumbers, QList<int>() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional enum multiple A;B;C 1,2" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterEnum * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->options(), def->options() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  // non optional, no default
  def.reset( new QgsProcessingParameterEnum( "non_optional", QString(), QStringList() << "A" << "B" << "C", false, QVariant(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "1,2" ) );
  QVERIFY( def->checkValueIsAcceptable( 0 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << 1 ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" ) );
  QVERIFY( !def->checkValueIsAcceptable( 15 ) );
  QVERIFY( !def->checkValueIsAcceptable( -1 ) );
  QVERIFY( !def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) ); // should NOT be acceptable, because falls back to invalid default value
}

void TestQgsProcessing::parameterString()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterString > def( new QgsProcessingParameterString( "non_optional", QString(), QString(), false, false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "abcdef" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString( "abcdef" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "'5'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc\ndef" ), context ), QStringLiteral( "'abc\\ndef'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=string" ) );
  std::unique_ptr< QgsProcessingParameterString > fromCode( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterString fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.multiLine(), def->multiLine() );
  def.reset( dynamic_cast< QgsProcessingParameterString *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterString *>( def.get() ) );

  def->setMultiLine( true );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=string long" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );
  def->setMultiLine( false );

  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=string None" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=string it's mario" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), QStringLiteral( "it's mario" ) );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  def->setDefaultValue( QStringLiteral( "it's mario" ) );
  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=string 'my val'" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), QStringLiteral( "my val" ) );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=string \"my val\"" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), QStringLiteral( "my val" ) );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  // optional
  def.reset( new QgsProcessingParameterString( "optional", QString(), QString( "default" ), false, true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString( "default" ) );
  params.insert( "optional",  QString() ); //empty string should not result in default value
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional string default" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  def->setMultiLine( true );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional string long default" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterString * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->multiLine(), def->multiLine() );

  // not optional, valid default!
  def.reset( new QgsProcessingParameterString( "non_optional", QString(), QString( "def" ), false, false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be valid, falls back to valid default
}


void TestQgsProcessing::parameterAuthConfig()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterAuthConfig > def( new QgsProcessingParameterAuthConfig( "non_optional", QString(), QString(), false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "abcdef" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString( "abcdef" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "'5'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc\ndef" ), context ), QStringLiteral( "'abc\\ndef'" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=authcfg" ) );
  std::unique_ptr< QgsProcessingParameterAuthConfig > fromCode( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterAuthConfig fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterAuthConfig *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterAuthConfig *>( def.get() ) );

  fromCode.reset( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=authcfg None" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QVERIFY( !fromCode->defaultValue().isValid() );

  fromCode.reset( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=authcfg it's mario" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), QStringLiteral( "it's mario" ) );

  def->setDefaultValue( QStringLiteral( "it's mario" ) );
  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  fromCode.reset( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=authcfg 'my val'" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), QStringLiteral( "my val" ) );

  fromCode.reset( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( QStringLiteral( "##non_optional=authcfg \"my val\"" ) ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue().toString(), QStringLiteral( "my val" ) );

  // optional
  def.reset( new QgsProcessingParameterAuthConfig( "optional", QString(), QString( "default" ), true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString( "default" ) );
  params.insert( "optional",  QString() ); //empty string should not result in default value
  QCOMPARE( QgsProcessingParameters::parameterAsString( def.get(), params, context ), QString() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional authcfg default" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterAuthConfig * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // not optional, valid default!
  def.reset( new QgsProcessingParameterAuthConfig( "non_optional", QString(), QString( "def" ), false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be valid, falls back to valid default
}

void TestQgsProcessing::parameterExpression()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterExpression > def( new QgsProcessingParameterExpression( "non_optional", QString(), QString( "1+1" ), QString(), false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) ); // should be acceptable, because it will fallback to default value

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "abcdef" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def.get(), params, context ), QString( "abcdef" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "'5'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc\ndef" ), context ), QStringLiteral( "'abc\\ndef'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=expression 1+1" ) );
  std::unique_ptr< QgsProcessingParameterExpression > fromCode( dynamic_cast< QgsProcessingParameterExpression * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterExpression fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.parentLayerParameterName(), def->parentLayerParameterName() );
  def.reset( dynamic_cast< QgsProcessingParameterExpression *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterExpression *>( def.get() ) );

  QVERIFY( def->dependsOnOtherParameters().isEmpty() );
  def->setParentLayerParameterName( QStringLiteral( "test_layer" ) );
  QCOMPARE( def->dependsOnOtherParameters(), QStringList() << QStringLiteral( "test_layer" ) );

  // optional
  def.reset( new QgsProcessingParameterExpression( "optional", QString(), QString( "default" ), QString(), true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def.get(), params, context ), QString( "default" ) );
  // valid expression, should not fallback
  params.insert( "optional",  QVariant( "1+2" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def.get(), params, context ), QString( "1+2" ) );
  // invalid expression, should fallback
  params.insert( "optional",  QVariant( "1+" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def.get(), params, context ), QString( "default" ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional expression default" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterExpression * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // non optional, no default
  def.reset( new QgsProcessingParameterExpression( "non_optional", QString(), QString(), QString(), false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) ); // should NOT be acceptable, because it will fallback to invalid default value

}

void TestQgsProcessing::parameterField()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterField > def( new QgsProcessingParameterField( "non_optional", QString(), QVariant(), QString(), QgsProcessingParameterField::Any, false, false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "a" << "b" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" << "b" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "a" ) );
  QStringList fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QCOMPARE( fields, QStringList() << "a" );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "probably\'invalid\"field", context ), QStringLiteral( "'probably\\'invalid\\\"field'" ) );


  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=field" ) );
  std::unique_ptr< QgsProcessingParameterField > fromCode( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  QVERIFY( def->dependsOnOtherParameters().isEmpty() );
  def->setParentLayerParameterName( "my_parent" );
  QCOMPARE( def->dependsOnOtherParameters(), QStringList() << QStringLiteral( "my_parent" ) );

  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  def->setDataType( QgsProcessingParameterField::Numeric );
  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  def->setDataType( QgsProcessingParameterField::String );
  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  def->setDataType( QgsProcessingParameterField::DateTime );
  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  // multiple
  def.reset( new QgsProcessingParameterField( "non_optional", QString(), QVariant(), QString(), QgsProcessingParameterField::Any, true, false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringList() << "a" << "b" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << "a" << "b" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );

  params.insert( "non_optional", QString( "a;b" ) );
  fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QCOMPARE( fields, QStringList() << "a" << "b" );
  params.insert( "non_optional", QVariantList() << "a" << "b" );
  fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QCOMPARE( fields, QStringList() << "a" << "b" );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringList() << "a" << "b", context ), QStringLiteral( "['a','b']" ) );
  QCOMPARE( def->valueAsPythonString( QStringList() << "a" << "b", context ), QStringLiteral( "['a','b']" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterField fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromMap.dataType(), def->dataType() );
  QCOMPARE( fromMap.allowMultiple(), def->allowMultiple() );
  def.reset( dynamic_cast< QgsProcessingParameterField *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterField *>( def.get() ) );

  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  // optional
  def.reset( new QgsProcessingParameterField( "optional", QString(), QString( "def" ), QString(), QgsProcessingParameterField::Any, false, true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() << "a" << "b" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() << "a" << "b" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QCOMPARE( fields, QStringList() << "def" );

  // optional, no default
  def.reset( new QgsProcessingParameterField( "optional", QString(), QVariant(), QString(), QgsProcessingParameterField::Any, false, true ) );
  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QVERIFY( fields.isEmpty() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional field" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterField * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  //optional with multiples
  def.reset( new QgsProcessingParameterField( "optional", QString(), QString( "abc;def" ), QString(), QgsProcessingParameterField::Any, true, true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "test" ) );
  QVERIFY( def->checkValueIsAcceptable( QStringList() << "a" << "b" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << "a" << "b" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QCOMPARE( fields, QStringList() << "abc" << "def" );
  def.reset( new QgsProcessingParameterField( "optional", QString(), QVariantList() << "abc" << "def", QString(), QgsProcessingParameterField::Any, true, true ) );
  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def.get(), params, context );
  QCOMPARE( fields, QStringList() << "abc" << "def" );
}

void TestQgsProcessing::parameterVectorLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString vector1 = testDataDir + "multipoint.shp";
  QString raster = testDataDir + "landsat.tif";
  QFileInfo fi1( raster );
  QFileInfo fi2( vector1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( fi2.filePath(), "V4", "ogr" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterVectorLayer > def( new QgsProcessingParameterVectorLayer( "non_optional", QString(), QList< int >(), QString( "somelayer" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "layer1231123" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer12312312" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context )->id(), v1->id() );

  // using existing layer
  params.insert( "non_optional",  QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context )->id(), v1->id() );

  // not vector layer
  params.insert( "non_optional",  r1->id() );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  // using existing non-vector layer
  params.insert( "non_optional",  QVariant::fromValue( r1 ) );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  // string representing a layer source
  params.insert( "non_optional", vector1 );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context )->publicSource(), vector1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( vector1, context ), QString( "'" ) + testDataDir + QStringLiteral( "multipoint.shp'" ) );
  QCOMPARE( def->valueAsPythonString( v1->id(), context ), QString( "'" ) + testDataDir + QStringLiteral( "multipoint.shp'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( v1 ), context ), QString( "'" ) + testDataDir + QStringLiteral( "multipoint.shp'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=vector somelayer" ) );
  std::unique_ptr< QgsProcessingParameterVectorLayer > fromCode( dynamic_cast< QgsProcessingParameterVectorLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterVectorLayer fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterVectorLayer *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterVectorLayer *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterVectorLayer( "optional", QString(), QList< int >(), v1->id(), true ) );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params,  context )->id(), v1->id() );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "layer1231123" ) ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional vector " ) + v1->id() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterVectorLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  //optional with direct layer default
  def.reset( new QgsProcessingParameterVectorLayer( "optional", QString(), QList< int >(), QVariant::fromValue( v1 ), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params,  context )->id(), v1->id() );
}

void TestQgsProcessing::parameterMeshLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString vector1 = testDataDir + "multipoint.shp";
  QString raster = testDataDir + "landsat.tif";
  QString mesh = testDataDir + "mesh/quad_and_triangle.2dm";
  QFileInfo fi1( raster );
  QFileInfo fi2( vector1 );
  QFileInfo fi3( mesh );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( fi2.filePath(), "V4", "ogr" );
  QgsMeshLayer *m1 = new QgsMeshLayer( fi3.filePath(), "M1", "mdal" );
  Q_ASSERT( m1 );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 << m1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterMeshLayer > def( new QgsProcessingParameterMeshLayer( "non_optional", QString(), QString( "somelayer" ), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "layer1231123" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( m1 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer12312312" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.2dm" ) );
  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.2dm", &context ) );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  m1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsMeshLayer( def.get(), params, context )->id(), m1->id() );

  // using existing layer
  params.insert( "non_optional",  QVariant::fromValue( m1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsMeshLayer( def.get(), params, context )->id(), m1->id() );

  // not mesh layer
  params.insert( "non_optional",  r1->id() );
  QVERIFY( !QgsProcessingParameters::parameterAsMeshLayer( def.get(), params, context ) );

  // using existing non-mesh layer
  params.insert( "non_optional",  QVariant::fromValue( r1 ) );
  QVERIFY( !QgsProcessingParameters::parameterAsMeshLayer( def.get(), params, context ) );

  // string representing a layer source
  params.insert( "non_optional", mesh );
  QCOMPARE( QgsProcessingParameters::parameterAsMeshLayer( def.get(), params, context )->publicSource(), mesh );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( mesh, context ), QString( "'" ) + testDataDir + QStringLiteral( "mesh/quad_and_triangle.2dm'" ) );
  QCOMPARE( def->valueAsPythonString( m1->id(), context ), QString( "'" ) + testDataDir + QStringLiteral( "mesh/quad_and_triangle.2dm'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( m1 ), context ), QString( "'" ) + testDataDir + QStringLiteral( "mesh/quad_and_triangle.2dm'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.2dm" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.2dm'" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=mesh somelayer" ) );
  std::unique_ptr< QgsProcessingParameterMeshLayer > fromCode( dynamic_cast< QgsProcessingParameterMeshLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterMeshLayer fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  def.reset( dynamic_cast< QgsProcessingParameterMeshLayer *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterMeshLayer *>( def.get() ) );

  // optional
  def.reset( new QgsProcessingParameterMeshLayer( "optional", QString(), m1->id(), true ) );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsMeshLayer( def.get(), params,  context )->id(), m1->id() );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.2dm" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "layer1231123" ) ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional mesh " ) + m1->id() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterMeshLayer * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  //optional with direct layer default
  def.reset( new QgsProcessingParameterMeshLayer( "optional", QString(), QVariant::fromValue( m1 ), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsMeshLayer( def.get(), params,  context )->id(), m1->id() );
}

void TestQgsProcessing::parameterFeatureSource()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString vector1 = testDataDir + "multipoint.shp";
  QString vector2 = testDataDir + "lines.shp";
  QString raster = testDataDir + "landsat.tif";
  QFileInfo fi1( raster );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( vector2, "V5", "ogr" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 << v2 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterFeatureSource > def( new QgsProcessingParameterFeatureSource( "non_optional", QString(), QList< int >() << QgsProcessing::TypeVectorAnyGeometry, QString(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "layer1231123" ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant::fromValue( v1 ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant::fromValue( r1 ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer12312312" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  // ... unless we use context, when the check that the layer actually exists is performed
  QVERIFY( !def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context )->id(), v1->id() );

  // using existing layer
  params.insert( "non_optional",  QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context )->id(), v1->id() );

  // not vector layer
  params.insert( "non_optional",  r1->id() );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  // using existing non-vector layer
  params.insert( "non_optional",  QVariant::fromValue( r1 ) );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  // string representing a layer source
  params.insert( "non_optional", vector1 );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context )->publicSource(), vector1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def.get(), params, context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingFeatureSourceDefinition( "abc" ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingFeatureSourceDefinition( v2->id() ) ), context ), QStringLiteral( "'%1'" ).arg( vector2 ) );
  QCOMPARE( def->valueAsPythonString( v2->id(), context ), QStringLiteral( "'%1'" ).arg( vector2 ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingFeatureSourceDefinition( QgsProperty::fromValue( "abc" ), true ) ), context ), QStringLiteral( "QgsProcessingFeatureSourceDefinition('abc', True)" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingFeatureSourceDefinition( QgsProperty::fromExpression( "\"abc\" || \"def\"" ) ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"abc\" || \"def\"')" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( v2 ), context ), QStringLiteral( "'%1'" ).arg( vector2 ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterFeatureSource fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.dataTypes(), def->dataTypes() );
  def.reset( dynamic_cast< QgsProcessingParameterFeatureSource *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterFeatureSource *>( def.get() ) );


  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=source" ) );
  std::unique_ptr< QgsProcessingParameterFeatureSource > fromCode( dynamic_cast< QgsProcessingParameterFeatureSource * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  def->setDataTypes( QList< int >() << QgsProcessing::TypeVectorPoint );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=source point" ) );
  def->setDataTypes( QList< int >() << QgsProcessing::TypeVectorLine );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=source line" ) );
  def->setDataTypes( QList< int >() << QgsProcessing::TypeVectorPolygon );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=source polygon" ) );
  def->setDataTypes( QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=source point line" ) );
  def->setDataTypes( QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorPolygon );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=source point polygon" ) );


  // optional
  def.reset( new QgsProcessingParameterFeatureSource( "optional", QString(), QList< int >() << QgsProcessing::TypeVectorAnyGeometry, v1->id(), true ) );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params,  context )->id(), v1->id() );
  QVERIFY( def->checkValueIsAcceptable( false ) );
  QVERIFY( def->checkValueIsAcceptable( true ) );
  QVERIFY( def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingFeatureSourceDefinition( "layer1231123" ) ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional source " ) + v1->id() );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFeatureSource * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );


  //optional with direct layer default
  def.reset( new QgsProcessingParameterFeatureSource( "optional", QString(), QList< int >() << QgsProcessing::TypeVectorAnyGeometry, QVariant::fromValue( v1 ), true ) );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def.get(), params,  context )->id(), v1->id() );

  // invalidSourceError
  params.clear();
  QCOMPARE( QgsProcessingAlgorithm::invalidSourceError( params, QStringLiteral( "MISSING" ) ), QStringLiteral( "Could not load source layer for MISSING: no value specified for parameter" ) );
  params.insert( QStringLiteral( "INPUT" ), QStringLiteral( "my layer" ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSourceError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: my layer not found" ) );
  params.insert( QStringLiteral( "INPUT" ), QgsProperty::fromValue( "my prop layer" ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSourceError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: my prop layer not found" ) );
  params.insert( QStringLiteral( "INPUT" ), QgsProcessingFeatureSourceDefinition( QStringLiteral( "my prop layer" ) ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSourceError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: my prop layer not found" ) );
  params.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( v1 ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSourceError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: invalid value" ) );
  params.insert( QStringLiteral( "INPUT" ), QgsProcessingOutputLayerDefinition( QStringLiteral( "my prop layer" ) ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSourceError( params, QStringLiteral( "INPUT" ) ), QStringLiteral( "Could not load source layer for INPUT: my prop layer not found" ) );
}

void TestQgsProcessing::parameterFeatureSink()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterFeatureSink > def( new QgsProcessingParameterFeatureSink( "non_optional", QString(), QgsProcessing::TypeVectorAnyGeometry, QString(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer12312312" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK with or without context - it's an output layer!
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( "abc" ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromValue( "abc" ) ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromExpression( "\"abc\" || \"def\"" ) ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"abc\" || \"def\"')" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "shp" ) );
  QCOMPARE( def->generateTemporaryDestination(), QStringLiteral( "memory:" ) );
  def->setSupportsNonFileBasedOutput( false );
  QVERIFY( def->generateTemporaryDestination().endsWith( QLatin1String( ".shp" ) ) );
  QVERIFY( def->generateTemporaryDestination().startsWith( QgsProcessingUtils::tempFolder() ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterFeatureSink fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.dataType(), def->dataType() );
  QCOMPARE( fromMap.supportsNonFileBasedOutput(), def->supportsNonFileBasedOutput() );
  def.reset( dynamic_cast< QgsProcessingParameterFeatureSink *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterFeatureSink *>( def.get() ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=sink" ) );
  std::unique_ptr< QgsProcessingParameterFeatureSink > fromCode( dynamic_cast< QgsProcessingParameterFeatureSink * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->dataType(), def->dataType() );

  def->setDataType( QgsProcessing::TypeVectorPoint );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=sink point" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFeatureSink * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  def->setDataType( QgsProcessing::TypeVectorLine );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=sink line" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFeatureSink * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  def->setDataType( QgsProcessing::TypeVectorPolygon );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=sink polygon" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFeatureSink * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  def->setDataType( QgsProcessing::TypeVector );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=sink table" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFeatureSink * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );

  // optional
  def.reset( new QgsProcessingParameterFeatureSink( "optional", QString(), QgsProcessing::TypeVectorAnyGeometry, QString(), true ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );
  def->setCreateByDefault( false );
  QVERIFY( !def->createByDefault() );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional sink" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFeatureSink * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  QVERIFY( !def->createByDefault() );

  // test hasGeometry
  QVERIFY( QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeMapLayer ).hasGeometry() );
  QVERIFY( QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeVectorAnyGeometry ).hasGeometry() );
  QVERIFY( QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeVectorPoint ).hasGeometry() );
  QVERIFY( QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeVectorLine ).hasGeometry() );
  QVERIFY( QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeVectorPolygon ).hasGeometry() );
  QVERIFY( !QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeRaster ).hasGeometry() );
  QVERIFY( !QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeFile ).hasGeometry() );
  QVERIFY( !QgsProcessingParameterFeatureSink( "test", QString(), QgsProcessing::TypeVector ).hasGeometry() );

  // invalidSinkError
  QVariantMap params;
  QCOMPARE( QgsProcessingAlgorithm::invalidSinkError( params, QStringLiteral( "MISSING" ) ), QStringLiteral( "Could not create destination layer for MISSING: no value specified for parameter" ) );
  params.insert( QStringLiteral( "OUTPUT" ), QStringLiteral( "d:/test.shp" ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSinkError( params, QStringLiteral( "OUTPUT" ) ), QStringLiteral( "Could not create destination layer for OUTPUT: d:/test.shp" ) );
  params.insert( QStringLiteral( "OUTPUT" ), QgsProperty::fromValue( QStringLiteral( "d:/test2.shp" ) ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSinkError( params, QStringLiteral( "OUTPUT" ) ), QStringLiteral( "Could not create destination layer for OUTPUT: d:/test2.shp" ) );
  params.insert( QStringLiteral( "OUTPUT" ), QgsProcessingOutputLayerDefinition( QStringLiteral( "d:/test3.shp" ) ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSinkError( params, QStringLiteral( "OUTPUT" ) ), QStringLiteral( "Could not create destination layer for OUTPUT: d:/test3.shp" ) );
  params.insert( QStringLiteral( "OUTPUT" ), QgsProcessingFeatureSourceDefinition( QStringLiteral( "source" ) ) );
  QCOMPARE( QgsProcessingAlgorithm::invalidSinkError( params, QStringLiteral( "OUTPUT" ) ), QStringLiteral( "Could not create destination layer for OUTPUT: invalid value" ) );

  // test supported output vector layer extensions

  def.reset( new QgsProcessingParameterFeatureSink( "with_geom", QString(), QgsProcessing::TypeVectorAnyGeometry, QString(), true ) );
  DummyProvider3 provider;
  provider.loadAlgorithms();
  def->mOriginalProvider = &provider;
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 2 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "mif" ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 1 ), QStringLiteral( "tab" ) );
  def->mOriginalProvider = nullptr;
  def->mAlgorithm = const_cast< QgsProcessingAlgorithm * >( provider.algorithms().at( 0 ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 2 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "mif" ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 1 ), QStringLiteral( "tab" ) );

  def.reset( new QgsProcessingParameterFeatureSink( "no_geom", QString(), QgsProcessing::TypeVector, QString(), true ) );
  def->mOriginalProvider = &provider;
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 1 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "dbf" ) );
  def->mOriginalProvider = nullptr;
  def->mAlgorithm = const_cast< QgsProcessingAlgorithm * >( provider.algorithms().at( 0 ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 1 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "dbf" ) );
}

void TestQgsProcessing::parameterVectorOut()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterVectorDestination > def( new QgsProcessingParameterVectorDestination( "non_optional", QString(), QgsProcessing::TypeVectorAnyGeometry, QString(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer1231123" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK with or without context - it's an output layer!
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp", &context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( "abc" ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromValue( "abc" ) ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromExpression( "\"abc\" || \"def\"" ) ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"abc\" || \"def\"')" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "shp" ) );
  QVERIFY( def->generateTemporaryDestination().endsWith( QLatin1String( ".shp" ) ) );
  QVERIFY( def->generateTemporaryDestination().startsWith( QgsProcessingUtils::tempFolder() ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterVectorDestination fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.dataType(), def->dataType() );
  def.reset( dynamic_cast< QgsProcessingParameterVectorDestination *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterVectorDestination *>( def.get() ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=vectorDestination" ) );
  std::unique_ptr< QgsProcessingParameterVectorDestination > fromCode( dynamic_cast< QgsProcessingParameterVectorDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->dataType(), def->dataType() );

  def->setDataType( QgsProcessing::TypeVectorPoint );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=vectorDestination point" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterVectorDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  def->setDataType( QgsProcessing::TypeVectorLine );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=vectorDestination line" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterVectorDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );
  def->setDataType( QgsProcessing::TypeVectorPolygon );
  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=vectorDestination polygon" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterVectorDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->dataType(), def->dataType() );

  // optional
  def.reset( new QgsProcessingParameterVectorDestination( "optional", QString(), QgsProcessing::TypeVectorAnyGeometry, QString(), true ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.shp" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional vectorDestination" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterVectorDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->dataType(), def->dataType() );

  // test hasGeometry
  QVERIFY( QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeMapLayer ).hasGeometry() );
  QVERIFY( QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeVectorAnyGeometry ).hasGeometry() );
  QVERIFY( QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeVectorPoint ).hasGeometry() );
  QVERIFY( QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeVectorLine ).hasGeometry() );
  QVERIFY( QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeVectorPolygon ).hasGeometry() );
  QVERIFY( !QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeRaster ).hasGeometry() );
  QVERIFY( !QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeFile ).hasGeometry() );
  QVERIFY( !QgsProcessingParameterVectorDestination( "test", QString(), QgsProcessing::TypeVector ).hasGeometry() );

  // test layers to load on completion
  def.reset( new QgsProcessingParameterVectorDestination( "x", QStringLiteral( "desc" ), QgsProcessing::TypeVectorAnyGeometry, QString(), true ) );
  QgsProcessingOutputLayerDefinition fs = QgsProcessingOutputLayerDefinition( QStringLiteral( "test.shp" ) );
  fs.destinationProject = &p;
  QVariantMap params;
  params.insert( QStringLiteral( "x" ), QVariant::fromValue( fs ) );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context ), QStringLiteral( "test.shp" ) );

  // make sure layer was automatically added to list to load on completion
  QCOMPARE( context.layersToLoadOnCompletion().size(), 1 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), QStringLiteral( "test.shp" ) );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "desc" ) );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).layerTypeHint, QgsProcessingUtils::Vector );

  // with name overloading
  QgsProcessingContext context2;
  fs = QgsProcessingOutputLayerDefinition( QStringLiteral( "test.shp" ) );
  fs.destinationProject = &p;
  fs.destinationName = QStringLiteral( "my_dest" );
  params.insert( QStringLiteral( "x" ), QVariant::fromValue( fs ) );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context2 ), QStringLiteral( "test.shp" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().size(), 1 );
  QCOMPARE( context2.layersToLoadOnCompletion().keys().at( 0 ), QStringLiteral( "test.shp" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "my_dest" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).outputName, QStringLiteral( "x" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).layerTypeHint, QgsProcessingUtils::Vector );

  QgsProcessingContext context3;
  params.insert( QStringLiteral( "x" ), QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context3 ).right( 6 ), QStringLiteral( "/x.shp" ) );

  QgsProcessingContext context4;
  fs.sink = QgsProperty::fromValue( QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "x" ), QVariant::fromValue( fs ) );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context4 ).right( 6 ), QStringLiteral( "/x.shp" ) );

  // test supported output vector layer extensions

  def.reset( new QgsProcessingParameterVectorDestination( "with_geom", QString(), QgsProcessing::TypeVectorAnyGeometry, QString(), true ) );
  DummyProvider3 provider;
  provider.loadAlgorithms();
  def->mOriginalProvider = &provider;
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 2 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "mif" ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 1 ), QStringLiteral( "tab" ) );
  def->mOriginalProvider = nullptr;
  def->mAlgorithm = const_cast< QgsProcessingAlgorithm * >( provider.algorithms().at( 0 ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 2 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "mif" ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 1 ), QStringLiteral( "tab" ) );

  def.reset( new QgsProcessingParameterVectorDestination( "no_geom", QString(), QgsProcessing::TypeVector, QString(), true ) );
  def->mOriginalProvider = &provider;
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 1 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "dbf" ) );
  def->mOriginalProvider = nullptr;
  def->mAlgorithm = const_cast< QgsProcessingAlgorithm * >( provider.algorithms().at( 0 ) );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().count(), 1 );
  QCOMPARE( def->supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "dbf" ) );
}

void TestQgsProcessing::parameterRasterOut()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterRasterDestination > def( new QgsProcessingParameterRasterDestination( "non_optional", QString(), QVariant(), false ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer12312312" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK with or without context - it's an output layer!
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.tif" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.tif", &context ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "tif" ) );
  QVERIFY( def->generateTemporaryDestination().endsWith( QLatin1String( ".tif" ) ) );
  QVERIFY( def->generateTemporaryDestination().startsWith( QgsProcessingUtils::tempFolder() ) );

  QVariantMap params;
  params.insert( "non_optional", "test.tif" );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context ), QStringLiteral( "test.tif" ) );
  params.insert( "non_optional", QgsProcessingOutputLayerDefinition( "test.tif" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context ), QStringLiteral( "test.tif" ) );

  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( "abc" ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromValue( "abc" ) ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromExpression( "\"abc\" || \"def\"" ) ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"abc\" || \"def\"')" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterRasterDestination fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.supportsNonFileBasedOutput(), def->supportsNonFileBasedOutput() );
  def.reset( dynamic_cast< QgsProcessingParameterRasterDestination *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterRasterDestination *>( def.get() ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=rasterDestination" ) );
  std::unique_ptr< QgsProcessingParameterRasterDestination > fromCode( dynamic_cast< QgsProcessingParameterRasterDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // optional
  def.reset( new QgsProcessingParameterRasterDestination( "optional", QString(), QString( "default.tif" ), true ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.tif" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );

  params.insert( "optional", QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context ), QStringLiteral( "default.tif" ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional rasterDestination default.tif" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterRasterDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // test layers to load on completion
  def.reset( new QgsProcessingParameterRasterDestination( "x", QStringLiteral( "desc" ), QStringLiteral( "default.tif" ), true ) );
  QgsProcessingOutputLayerDefinition fs = QgsProcessingOutputLayerDefinition( QStringLiteral( "test.tif" ) );
  fs.destinationProject = &p;
  params.insert( QStringLiteral( "x" ), QVariant::fromValue( fs ) );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context ), QStringLiteral( "test.tif" ) );

  // make sure layer was automatically added to list to load on completion
  QCOMPARE( context.layersToLoadOnCompletion().size(), 1 );
  QCOMPARE( context.layersToLoadOnCompletion().keys().at( 0 ), QStringLiteral( "test.tif" ) );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "desc" ) );
  QCOMPARE( context.layersToLoadOnCompletion().values().at( 0 ).layerTypeHint, QgsProcessingUtils::Raster );

  // with name overloading
  QgsProcessingContext context2;
  fs = QgsProcessingOutputLayerDefinition( QStringLiteral( "test.tif" ) );
  fs.destinationProject = &p;
  fs.destinationName = QStringLiteral( "my_dest" );
  params.insert( QStringLiteral( "x" ), QVariant::fromValue( fs ) );
  QCOMPARE( QgsProcessingParameters::parameterAsOutputLayer( def.get(), params, context2 ), QStringLiteral( "test.tif" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().size(), 1 );
  QCOMPARE( context2.layersToLoadOnCompletion().keys().at( 0 ), QStringLiteral( "test.tif" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).name, QStringLiteral( "my_dest" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).outputName, QStringLiteral( "x" ) );
  QCOMPARE( context2.layersToLoadOnCompletion().values().at( 0 ).layerTypeHint, QgsProcessingUtils::Raster );
}

void TestQgsProcessing::parameterFileOut()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterFileDestination > def( new QgsProcessingParameterFileDestination( "non_optional", QString(), QStringLiteral( "BMP files (*.bmp)" ), QVariant(), false ) );
  QCOMPARE( def->fileFilter(), QStringLiteral( "BMP files (*.bmp)" ) );
  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "bmp" ) );
  QVERIFY( def->generateTemporaryDestination().endsWith( QLatin1String( ".bmp" ) ) );
  QVERIFY( def->generateTemporaryDestination().startsWith( QgsProcessingUtils::tempFolder() ) );
  def->setFileFilter( QStringLiteral( "PCX files (*.pcx)" ) );
  QCOMPARE( def->fileFilter(), QStringLiteral( "PCX files (*.pcx)" ) );
  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "pcx" ) );
  def->setFileFilter( QStringLiteral( "PCX files (*.pcx *.picx)" ) );
  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "pcx" ) );
  def->setFileFilter( QStringLiteral( "PCX files (*.pcx *.picx);;BMP files (*.bmp)" ) );
  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "pcx" ) );
  def->setFileFilter( QString() );
  QCOMPARE( def->defaultFileExtension(), QStringLiteral( "file" ) );
  QVERIFY( def->generateTemporaryDestination().endsWith( QLatin1String( ".file" ) ) );
  QVERIFY( def->generateTemporaryDestination().startsWith( QgsProcessingUtils::tempFolder() ) );

  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "layer12312312" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK with or without context - it's an output file!
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.txt" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.txt", &context ) );

  QVariantMap params;
  params.insert( "non_optional", "test.txt" );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ), QStringLiteral( "test.txt" ) );
  params.insert( "non_optional", QgsProcessingOutputLayerDefinition( "test.txt" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ), QStringLiteral( "test.txt" ) );

  params.insert( QStringLiteral( "non_optional" ), QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ).right( 7 ), QStringLiteral( "_OUTPUT" ) );

  QgsProcessingOutputLayerDefinition fs;
  fs.sink = QgsProperty::fromValue( QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "non_optional" ), QVariant::fromValue( fs ) );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ).right( 7 ), QStringLiteral( "_OUTPUT" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( "abc" ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromValue( "abc" ) ) ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProperty::fromExpression( "\"abc\" || \"def\"" ) ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"abc\" || \"def\"')" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\test.dat" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\test.dat'" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterFileDestination fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.fileFilter(), def->fileFilter() );
  QCOMPARE( fromMap.supportsNonFileBasedOutput(), def->supportsNonFileBasedOutput() );
  def.reset( dynamic_cast< QgsProcessingParameterFileDestination *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterFileDestination *>( def.get() ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=fileDestination" ) );
  std::unique_ptr< QgsProcessingParameterFileDestination > fromCode( dynamic_cast< QgsProcessingParameterFileDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // optional
  def.reset( new QgsProcessingParameterFileDestination( "optional", QString(), QString(), QString( "default.txt" ), true ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/roads_clipped_transformed_v1_reprojected_final_clipped_aAAA.txt" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "layer1231123" ) ) );

  params.insert( "optional", QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ), QStringLiteral( "default.txt" ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional fileDestination default.txt" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFileDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // outputs definitio test
  def.reset( new QgsProcessingParameterFileDestination( "html", QString(), QString( "HTML files" ), QString(), false ) );
  std::unique_ptr< QgsProcessingOutputDefinition > outputDef( def->toOutputDefinition() );
  QVERIFY( dynamic_cast< QgsProcessingOutputHtml *>( outputDef.get() ) );
  def.reset( new QgsProcessingParameterFileDestination( "html", QString(), QString( "Text files (*.htm)" ), QString(), false ) );
  outputDef.reset( def->toOutputDefinition() );
  QVERIFY( dynamic_cast< QgsProcessingOutputHtml *>( outputDef.get() ) );
  def.reset( new QgsProcessingParameterFileDestination( "file", QString(), QString( "Text files (*.txt)" ), QString(), false ) );
  outputDef.reset( def->toOutputDefinition() );
  QVERIFY( dynamic_cast< QgsProcessingOutputFile *>( outputDef.get() ) );
  def.reset( new QgsProcessingParameterFileDestination( "file", QString(), QString(), QString(), false ) );
  outputDef.reset( def->toOutputDefinition() );
  QVERIFY( dynamic_cast< QgsProcessingOutputFile *>( outputDef.get() ) );
}

void TestQgsProcessing::parameterFolderOut()
{
  // setup a context
  QgsProject p;
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  std::unique_ptr< QgsProcessingParameterFolderDestination > def( new QgsProcessingParameterFolderDestination( "non_optional", QString(), QVariant(), false ) );

  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "asdasd" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( QStringLiteral( "asdasdas" ) ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QgsProperty::fromValue( QString() ) ) );

  // should be OK with or without context - it's an output folder!
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/", &context ) );

  // check that temporary destination does not have dot at the end when there is no extension
  QVERIFY( !def->generateTemporaryDestination().endsWith( QLatin1String( "." ) ) );
  QVERIFY( def->generateTemporaryDestination().startsWith( QgsProcessingUtils::tempFolder() ) );

  QVariantMap params;
  params.insert( "non_optional", "c:/mine" );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ), QStringLiteral( "c:/mine" ) );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "abc" ), context ), QStringLiteral( "'abc'" ) );
  QCOMPARE( def->valueAsPythonString( QVariant::fromValue( QgsProperty::fromExpression( "\"a\"=1" ) ), context ), QStringLiteral( "QgsProperty.fromExpression('\"a\"=1')" ) );
  QCOMPARE( def->valueAsPythonString( "uri='complex' username=\"complex\"", context ), QStringLiteral( "'uri=\\'complex\\' username=\\\"complex\\\"'" ) );
  QCOMPARE( def->valueAsPythonString( QStringLiteral( "c:\\test\\new data\\" ), context ), QStringLiteral( "'c:\\\\test\\\\new data\\\\'" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterFolderDestination fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.flags(), def->flags() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.supportsNonFileBasedOutput(), def->supportsNonFileBasedOutput() );
  def.reset( dynamic_cast< QgsProcessingParameterFolderDestination *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterFolderDestination *>( def.get() ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=folderDestination" ) );
  std::unique_ptr< QgsProcessingParameterFolderDestination > fromCode( dynamic_cast< QgsProcessingParameterFolderDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );

  // optional
  def.reset( new QgsProcessingParameterFolderDestination( "optional", QString(), QString( "c:/junk" ), true ) );
  QVERIFY( !def->checkValueIsAcceptable( false ) );
  QVERIFY( !def->checkValueIsAcceptable( true ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  QVERIFY( def->checkValueIsAcceptable( "layer12312312" ) );
  QVERIFY( def->checkValueIsAcceptable( "c:/Users/admin/Desktop/" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional", QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsFileOutput( def.get(), params, context ), QStringLiteral( "c:/junk" ) );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional folderDestination c:/junk" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterFolderDestination * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
}

void TestQgsProcessing::parameterBand()
{
  QgsProcessingContext context;

  // not optional!
  std::unique_ptr< QgsProcessingParameterBand > def( new QgsProcessingParameterBand( "non_optional", QString(), QVariant(), QString(), false ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );

  // string representing a band
  QVariantMap params;
  params.insert( "non_optional", "1" );
  int band = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( band, 1 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( 5, context ), QStringLiteral( "5" ) );

  QString code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##non_optional=band" ) );
  std::unique_ptr< QgsProcessingParameterBand > fromCode( dynamic_cast< QgsProcessingParameterBand * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );

  QVERIFY( def->dependsOnOtherParameters().isEmpty() );
  def->setParentLayerParameterName( "my_parent" );
  QCOMPARE( def->dependsOnOtherParameters(), QStringList() << QStringLiteral( "my_parent" ) );

  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterBand * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );

  // multiple
  def.reset( new QgsProcessingParameterBand( "non_optional", QString(), QVariant(), QString(), false, true ) );
  QVERIFY( def->checkValueIsAcceptable( QStringList() << "1" << "2" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariantList() << 1 << 2 ) );
  QVERIFY( !def->checkValueIsAcceptable( "" ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( QStringList() ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariantList() ) );

  params.insert( "non_optional", QString( "1;2" ) );
  QList<int> bands = QgsProcessingParameters::parameterAsInts( def.get(), params, context );
  QCOMPARE( bands, QList<int>() << 1 << 2 );
  params.insert( "non_optional", QVariantList() << 1 << 2 );
  bands = QgsProcessingParameters::parameterAsInts( def.get(), params, context );
  QCOMPARE( bands, QList<int>() << 1 << 2 );

  QCOMPARE( def->valueAsPythonString( QVariant(), context ), QStringLiteral( "None" ) );
  QCOMPARE( def->valueAsPythonString( QStringList() << "1" << "2", context ), QStringLiteral( "[1,2]" ) );
  QCOMPARE( def->valueAsPythonString( QVariantList() << 1 << 2, context ), QStringLiteral( "[1,2]" ) );

  QVariantMap map = def->toVariantMap();
  QgsProcessingParameterBand fromMap( "x" );
  QVERIFY( fromMap.fromVariantMap( map ) );
  QCOMPARE( fromMap.name(), def->name() );
  QCOMPARE( fromMap.description(), def->description() );
  QCOMPARE( fromMap.defaultValue(), def->defaultValue() );
  QCOMPARE( fromMap.parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromMap.allowMultiple(), def->allowMultiple() );
  def.reset( dynamic_cast< QgsProcessingParameterBand *>( QgsProcessingParameters::parameterFromVariantMap( map ) ) );
  QVERIFY( dynamic_cast< QgsProcessingParameterBand *>( def.get() ) );

  code = def->asScriptCode();
  fromCode.reset( dynamic_cast< QgsProcessingParameterBand * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "non optional" ) );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
  QCOMPARE( fromCode->allowMultiple(), def->allowMultiple() );

  // optional
  def.reset( new QgsProcessingParameterBand( "optional", QString(), 1, QString(), true ) );
  QVERIFY( def->checkValueIsAcceptable( 1 ) );
  QVERIFY( def->checkValueIsAcceptable( "1" ) );
  QVERIFY( def->checkValueIsAcceptable( "" ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );

  params.insert( "optional",  QVariant() );
  band = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( band, 1 );

  // optional, no default
  def.reset( new QgsProcessingParameterBand( "optional", QString(), QVariant(), QString(), true ) );
  params.insert( "optional",  QVariant() );
  band = QgsProcessingParameters::parameterAsInt( def.get(), params, context );
  QCOMPARE( band, 0 );

  code = def->asScriptCode();
  QCOMPARE( code, QStringLiteral( "##optional=optional band" ) );
  fromCode.reset( dynamic_cast< QgsProcessingParameterBand * >( QgsProcessingParameters::parameterFromScriptCode( code ) ) );
  QVERIFY( fromCode.get() );
  QCOMPARE( fromCode->name(), def->name() );
  QCOMPARE( fromCode->description(), QStringLiteral( "optional" ) );
  QCOMPARE( fromCode->flags(), def->flags() );
  QCOMPARE( fromCode->defaultValue(), def->defaultValue() );
  QCOMPARE( fromCode->parentLayerParameterName(), def->parentLayerParameterName() );
}

void TestQgsProcessing::checkParamValues()
{
  DummyAlgorithm a( "asd" );
  a.checkParameterVals();
}

void TestQgsProcessing::combineLayerExtent()
{
  QgsRectangle ext = QgsProcessingUtils::combineLayerExtents( QList< QgsMapLayer *>() );
  QVERIFY( ext.isNull() );

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  std::unique_ptr< QgsRasterLayer > r1( new QgsRasterLayer( fi1.filePath(), "R1" ) );
  QFileInfo fi2( raster2 );
  std::unique_ptr< QgsRasterLayer > r2( new QgsRasterLayer( fi2.filePath(), "R2" ) );

  ext = QgsProcessingUtils::combineLayerExtents( QList< QgsMapLayer *>() << r1.get() );
  QGSCOMPARENEAR( ext.xMinimum(), 1535375.000000, 10 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 10 );
  QGSCOMPARENEAR( ext.yMinimum(), 5083255, 10 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 10 );

  ext = QgsProcessingUtils::combineLayerExtents( QList< QgsMapLayer *>() << r1.get() << r2.get() );
  QGSCOMPARENEAR( ext.xMinimum(), 781662, 10 );
  QGSCOMPARENEAR( ext.xMaximum(), 1535475, 10 );
  QGSCOMPARENEAR( ext.yMinimum(), 3339523, 10 );
  QGSCOMPARENEAR( ext.yMaximum(), 5083355, 10 );

  // with reprojection
  ext = QgsProcessingUtils::combineLayerExtents( QList< QgsMapLayer *>() << r1.get() << r2.get(), QgsCoordinateReferenceSystem::fromEpsgId( 3785 ) );
  QGSCOMPARENEAR( ext.xMinimum(), 1995320, 10 );
  QGSCOMPARENEAR( ext.xMaximum(), 2008833, 10 );
  QGSCOMPARENEAR( ext.yMinimum(), 3523084, 10 );
  QGSCOMPARENEAR( ext.yMaximum(), 3536664, 10 );
}

void TestQgsProcessing::processingFeatureSource()
{
  QString sourceString = QStringLiteral( "test.shp" );
  QgsProcessingFeatureSourceDefinition fs( sourceString, true );
  QCOMPARE( fs.source.staticValue().toString(), sourceString );
  QVERIFY( fs.selectedFeaturesOnly );

  // test storing QgsProcessingFeatureSource in variant and retrieving
  QVariant fsInVariant = QVariant::fromValue( fs );
  QVERIFY( fsInVariant.isValid() );

  QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( fsInVariant );
  QCOMPARE( fromVar.source.staticValue().toString(), sourceString );
  QVERIFY( fromVar.selectedFeaturesOnly );

  // test evaluating parameter as source
  QgsVectorLayer *layer = new QgsVectorLayer( "Point", "v1", "memory" );
  QgsFeature f( 10001 );
  f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f );

  QgsProject p;
  p.addMapLayer( layer );
  QgsProcessingContext context;
  context.setProject( &p );

  // first using static string definition
  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterString( QStringLiteral( "layer" ) ) );
  QVariantMap params;
  params.insert( QStringLiteral( "layer" ), QgsProcessingFeatureSourceDefinition( layer->id(), false ) );
  std::unique_ptr< QgsFeatureSource > source( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  // can't directly match it to layer, so instead just get the feature and test that it matches what we expect
  QgsFeature f2;
  QVERIFY( source.get() );
  QVERIFY( source->getFeatures().nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt(), f.geometry().asWkt() );

  // direct map layer
  params.insert( QStringLiteral( "layer" ), QVariant::fromValue( layer ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  // can't directly match it to layer, so instead just get the feature and test that it matches what we expect
  QVERIFY( source.get() );
  QVERIFY( source->getFeatures().nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt(), f.geometry().asWkt() );

  // next using property based definition
  params.insert( QStringLiteral( "layer" ), QgsProcessingFeatureSourceDefinition( QgsProperty::fromExpression( QStringLiteral( "trim('%1' + ' ')" ).arg( layer->id() ) ), false ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  // can't directly match it to layer, so instead just get the feature and test that it matches what we expect
  QVERIFY( source.get() );
  QVERIFY( source->getFeatures().nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt(), f.geometry().asWkt() );

  // we also must accept QgsProcessingOutputLayerDefinition - e.g. to allow outputs from earlier child algorithms in models
  params.insert( QStringLiteral( "layer" ), QgsProcessingOutputLayerDefinition( QgsProperty::fromValue( layer->id() ) ) );
  source.reset( QgsProcessingParameters::parameterAsSource( def.get(), params, context ) );
  // can't directly match it to layer, so instead just get the feature and test that it matches what we expect
  QVERIFY( source.get() );
  QVERIFY( source->getFeatures().nextFeature( f2 ) );
  QCOMPARE( f2.geometry().asWkt(), f.geometry().asWkt() );
}

void TestQgsProcessing::processingFeatureSink()
{
  QString sinkString( QStringLiteral( "test.shp" ) );
  QgsProject p;
  QgsProcessingOutputLayerDefinition fs( sinkString, &p );
  QCOMPARE( fs.sink.staticValue().toString(), sinkString );
  QCOMPARE( fs.destinationProject, &p );

  // test storing QgsProcessingFeatureSink in variant and retrieving
  QVariant fsInVariant = QVariant::fromValue( fs );
  QVERIFY( fsInVariant.isValid() );

  QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( fsInVariant );
  QCOMPARE( fromVar.sink.staticValue().toString(), sinkString );
  QCOMPARE( fromVar.destinationProject, &p );

  // test evaluating parameter as sink
  QgsProcessingContext context;
  context.setProject( &p );

  // first using static string definition
  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterFeatureSink( QStringLiteral( "layer" ) ) );
  QVariantMap params;
  params.insert( QStringLiteral( "layer" ), QgsProcessingOutputLayerDefinition( "memory:test", nullptr ) );
  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:3111" ), context, dest ) );
  QVERIFY( sink.get() );
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( dest, context, false ) );
  QVERIFY( layer );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );

  // next using property based definition
  params.insert( QStringLiteral( "layer" ), QgsProcessingOutputLayerDefinition( QgsProperty::fromExpression( QStringLiteral( "trim('memory' + ':test2')" ) ), nullptr ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:3113" ), context, dest ) );
  QVERIFY( sink.get() );
  QgsVectorLayer *layer2 = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( dest, context, false ) );
  QVERIFY( layer2 );
  QCOMPARE( layer2->crs().authid(), QStringLiteral( "EPSG:3113" ) );

  // temporary sink
  params.insert( QStringLiteral( "layer" ), QgsProcessing::TEMPORARY_OUTPUT );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:28356" ), context, dest ) );
  QVERIFY( sink.get() );
  QgsVectorLayer *layer3 = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( dest, context, false ) );
  QVERIFY( layer3 );
  QCOMPARE( layer3->crs().authid(), QStringLiteral( "EPSG:28356" ) );
  QCOMPARE( layer3->dataProvider()->name(), QStringLiteral( "memory" ) );

  params.insert( QStringLiteral( "layer" ), QgsProcessingOutputLayerDefinition( QgsProperty::fromValue( QgsProcessing::TEMPORARY_OUTPUT ), nullptr ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:28354" ), context, dest ) );
  QVERIFY( sink.get() );
  QgsVectorLayer *layer4 = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( dest, context, false ) );
  QVERIFY( layer4 );
  QCOMPARE( layer4->crs().authid(), QStringLiteral( "EPSG:28354" ) );
  QCOMPARE( layer4->dataProvider()->name(), QStringLiteral( "memory" ) );

  // non optional sink
  def.reset( new QgsProcessingParameterFeatureSink( QStringLiteral( "layer" ), QString(), QgsProcessing::TypeMapLayer, QVariant(), false ) );
  QVERIFY( def->checkValueIsAcceptable( QStringLiteral( "memory:test" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "memory:test" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( "memory:test" ) ) );
  QVERIFY( !def->checkValueIsAcceptable( QString() ) );
  QVERIFY( !def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  params.insert( QStringLiteral( "layer" ), QStringLiteral( "memory:test" ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:3113" ), context, dest ) );
  QVERIFY( sink.get() );

  // optional sink
  def.reset( new QgsProcessingParameterFeatureSink( QStringLiteral( "layer" ), QString(), QgsProcessing::TypeMapLayer, QVariant(), true ) );
  QVERIFY( def->checkValueIsAcceptable( QStringLiteral( "memory:test" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProcessingOutputLayerDefinition( "memory:test" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QgsProperty::fromValue( "memory:test" ) ) );
  QVERIFY( def->checkValueIsAcceptable( QString() ) );
  QVERIFY( def->checkValueIsAcceptable( QVariant() ) );
  QVERIFY( !def->checkValueIsAcceptable( 5 ) );
  params.insert( QStringLiteral( "layer" ), QStringLiteral( "memory:test" ) );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:3113" ), context, dest ) );
  QVERIFY( sink.get() );
  // optional sink, not set - should be no sink
  params.insert( QStringLiteral( "layer" ), QVariant() );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:3113" ), context, dest ) );
  QVERIFY( !sink.get() );

  //.... unless there's a default set
  def.reset( new QgsProcessingParameterFeatureSink( QStringLiteral( "layer" ), QString(), QgsProcessing::TypeMapLayer, QStringLiteral( "memory:defaultlayer" ), true ) );
  params.insert( QStringLiteral( "layer" ), QVariant() );
  sink.reset( QgsProcessingParameters::parameterAsSink( def.get(), params, QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem( "EPSG:3113" ), context, dest ) );
  QVERIFY( sink.get() );
}

void TestQgsProcessing::algorithmScope()
{
  QgsProcessingContext pc;

  // no alg
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::processingAlgorithmScope( nullptr, QVariantMap(), pc ) );
  QVERIFY( scope.get() );

  // with alg
  std::unique_ptr< QgsProcessingAlgorithm > alg( new DummyAlgorithm( "alg1" ) );
  QVariantMap params;
  params.insert( QStringLiteral( "a_param" ), 5 );
  scope.reset( QgsExpressionContextUtils::processingAlgorithmScope( alg.get(), params, pc ) );
  QVERIFY( scope.get() );
  QCOMPARE( scope->variable( QStringLiteral( "algorithm_id" ) ).toString(), alg->id() );

  QgsExpressionContext context;
  context.appendScope( scope.release() );
  QgsExpression exp( "parameter('bad')" );
  QVERIFY( !exp.evaluate( &context ).isValid() );
  QgsExpression exp2( "parameter('a_param')" );
  QCOMPARE( exp2.evaluate( &context ).toInt(), 5 );
}

void TestQgsProcessing::validateInputCrs()
{
  DummyAlgorithm alg( "test" );
  alg.runValidateInputCrsChecks();
}

void TestQgsProcessing::generateIteratingDestination()
{
  QgsProcessingContext context;
  QCOMPARE( QgsProcessingUtils::generateIteratingDestination( "memory:x", 1, context ).toString(), QStringLiteral( "memory:x_1" ) );
  QCOMPARE( QgsProcessingUtils::generateIteratingDestination( "memory:x", 2, context ).toString(), QStringLiteral( "memory:x_2" ) );
  QCOMPARE( QgsProcessingUtils::generateIteratingDestination( "ape.shp", 1, context ).toString(), QStringLiteral( "ape_1.shp" ) );
  QCOMPARE( QgsProcessingUtils::generateIteratingDestination( "ape.shp", 2, context ).toString(), QStringLiteral( "ape_2.shp" ) );
  QCOMPARE( QgsProcessingUtils::generateIteratingDestination( "/home/bif.o/ape.shp", 2, context ).toString(), QStringLiteral( "/home/bif.o/ape_2.shp" ) );

  QgsProject p;
  QgsProcessingOutputLayerDefinition def;
  def.sink = QgsProperty::fromValue( "ape.shp" );
  def.destinationProject = &p;
  QVariant res = QgsProcessingUtils::generateIteratingDestination( def, 2, context );
  QVERIFY( res.canConvert<QgsProcessingOutputLayerDefinition>() );
  QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( res );
  QCOMPARE( fromVar.sink.staticValue().toString(), QStringLiteral( "ape_2.shp" ) );
  QCOMPARE( fromVar.destinationProject, &p );

  def.sink = QgsProperty::fromExpression( "'ape' || '.shp'" );
  res = QgsProcessingUtils::generateIteratingDestination( def, 2, context );
  QVERIFY( res.canConvert<QgsProcessingOutputLayerDefinition>() );
  fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( res );
  QCOMPARE( fromVar.sink.staticValue().toString(), QStringLiteral( "ape_2.shp" ) );
  QCOMPARE( fromVar.destinationProject, &p );
}

void TestQgsProcessing::asPythonCommand()
{
  DummyAlgorithm alg( "test" );
  alg.runAsPythonCommandChecks();
}

void TestQgsProcessing::modelerAlgorithm()
{
  //static value source
  QgsProcessingModelChildParameterSource svSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  QCOMPARE( svSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( svSource.staticValue().toInt(), 5 );
  svSource.setStaticValue( 7 );
  QCOMPARE( svSource.staticValue().toInt(), 7 );
  svSource = QgsProcessingModelChildParameterSource::fromModelParameter( "a" );
  // check that calling setStaticValue flips source to StaticValue
  QCOMPARE( svSource.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  svSource.setStaticValue( 7 );
  QCOMPARE( svSource.staticValue().toInt(), 7 );
  QCOMPARE( svSource.source(), QgsProcessingModelChildParameterSource::StaticValue );

  // model parameter source
  QgsProcessingModelChildParameterSource mpSource = QgsProcessingModelChildParameterSource::fromModelParameter( "a" );
  QCOMPARE( mpSource.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( mpSource.parameterName(), QStringLiteral( "a" ) );
  mpSource.setParameterName( "b" );
  QCOMPARE( mpSource.parameterName(), QStringLiteral( "b" ) );
  mpSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setParameterName flips source to ModelParameter
  QCOMPARE( mpSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  mpSource.setParameterName( "c" );
  QCOMPARE( mpSource.parameterName(), QStringLiteral( "c" ) );
  QCOMPARE( mpSource.source(), QgsProcessingModelChildParameterSource::ModelParameter );

  // child alg output source
  QgsProcessingModelChildParameterSource oSource = QgsProcessingModelChildParameterSource::fromChildOutput( "a", "b" );
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( oSource.outputChildId(), QStringLiteral( "a" ) );
  QCOMPARE( oSource.outputName(), QStringLiteral( "b" ) );
  oSource.setOutputChildId( "c" );
  QCOMPARE( oSource.outputChildId(), QStringLiteral( "c" ) );
  oSource.setOutputName( "d" );
  QCOMPARE( oSource.outputName(), QStringLiteral( "d" ) );
  oSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setOutputChildId flips source to ChildOutput
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  oSource.setOutputChildId( "c" );
  QCOMPARE( oSource.outputChildId(), QStringLiteral( "c" ) );
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  oSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setOutputName flips source to ChildOutput
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  oSource.setOutputName( "d" );
  QCOMPARE( oSource.outputName(), QStringLiteral( "d" ) );
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::ChildOutput );

  // expression source
  QgsProcessingModelChildParameterSource expSource = QgsProcessingModelChildParameterSource::fromExpression( "1+2" );
  QCOMPARE( expSource.source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( expSource.expression(), QStringLiteral( "1+2" ) );
  expSource.setExpression( "1+3" );
  QCOMPARE( expSource.expression(), QStringLiteral( "1+3" ) );
  expSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setExpression flips source to Expression
  QCOMPARE( expSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  expSource.setExpression( "1+4" );
  QCOMPARE( expSource.expression(), QStringLiteral( "1+4" ) );
  QCOMPARE( expSource.source(), QgsProcessingModelChildParameterSource::Expression );

  // source equality operator
  QVERIFY( QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) ==
           QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) !=
           QgsProcessingModelChildParameterSource::fromStaticValue( 7 ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) !=
           QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) ) ==
           QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) ) !=
           QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "b" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) ) !=
           QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) ==
           QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) !=
           QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg2" ), QStringLiteral( "out" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) !=
           QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out2" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "a" ) ) ==
           QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "a" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "a" ) ) !=
           QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "b" ) ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "a" ) ) !=
           QgsProcessingModelChildParameterSource::fromStaticValue( QStringLiteral( "b" ) ) );


  QgsProcessingModelChildAlgorithm child( QStringLiteral( "some_id" ) );
  QCOMPARE( child.algorithmId(), QStringLiteral( "some_id" ) );
  QVERIFY( !child.algorithm() );
  QVERIFY( !child.setAlgorithmId( QStringLiteral( "blah" ) ) );
  QVERIFY( !child.reattach() );
  QVERIFY( child.setAlgorithmId( QStringLiteral( "native:centroids" ) ) );
  QVERIFY( child.algorithm() );
  QCOMPARE( child.algorithm()->id(), QStringLiteral( "native:centroids" ) );
  // bit of a hack -- but try to simulate an algorithm not originally available!
  child.mAlgorithm.reset();
  QVERIFY( !child.algorithm() );
  QVERIFY( child.reattach() );
  QVERIFY( child.algorithm() );
  QCOMPARE( child.algorithm()->id(), QStringLiteral( "native:centroids" ) );

  QVariantMap myConfig;
  myConfig.insert( QStringLiteral( "some_key" ), 11 );
  child.setConfiguration( myConfig );
  QCOMPARE( child.configuration(), myConfig );

  child.setDescription( QStringLiteral( "desc" ) );
  QCOMPARE( child.description(), QStringLiteral( "desc" ) );
  QVERIFY( child.isActive() );
  child.setActive( false );
  QVERIFY( !child.isActive() );
  child.setPosition( QPointF( 1, 2 ) );
  QCOMPARE( child.position(), QPointF( 1, 2 ) );
  QVERIFY( child.parametersCollapsed() );
  child.setParametersCollapsed( false );
  QVERIFY( !child.parametersCollapsed() );
  QVERIFY( child.outputsCollapsed() );
  child.setOutputsCollapsed( false );
  QVERIFY( !child.outputsCollapsed() );

  child.setChildId( QStringLiteral( "my_id" ) );
  QCOMPARE( child.childId(), QStringLiteral( "my_id" ) );

  child.setDependencies( QStringList() << "a" << "b" );
  QCOMPARE( child.dependencies(), QStringList() << "a" << "b" );

  QMap< QString, QgsProcessingModelChildParameterSources > sources;
  sources.insert( QStringLiteral( "a" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  child.setParameterSources( sources );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "a" ) ).at( 0 ).staticValue().toInt(), 5 );
  child.addParameterSources( QStringLiteral( "b" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 7 ) << QgsProcessingModelChildParameterSource::fromStaticValue( 9 ) );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "a" ) ).at( 0 ).staticValue().toInt(), 5 );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "b" ) ).count(), 2 );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "b" ) ).at( 0 ).staticValue().toInt(), 7 );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "b" ) ).at( 1 ).staticValue().toInt(), 9 );

  QgsProcessingModelOutput testModelOut;
  testModelOut.setChildId( QStringLiteral( "my_id" ) );
  QCOMPARE( testModelOut.childId(), QStringLiteral( "my_id" ) );
  testModelOut.setChildOutputName( QStringLiteral( "my_output" ) );
  QCOMPARE( testModelOut.childOutputName(), QStringLiteral( "my_output" ) );
  testModelOut.setDefaultValue( QStringLiteral( "my_value" ) );
  QCOMPARE( testModelOut.defaultValue().toString(), QStringLiteral( "my_value" ) );
  testModelOut.setMandatory( true );
  QVERIFY( testModelOut.isMandatory() );

  QgsProcessingOutputLayerDefinition layerDef( QStringLiteral( "my_path" ) );
  layerDef.createOptions["fileEncoding"] = QStringLiteral( "my_encoding" );
  testModelOut.setDefaultValue( layerDef );
  QCOMPARE( testModelOut.defaultValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "my_path" ) );
  QVariantMap map = testModelOut.toVariant().toMap();
  QCOMPARE( map["default_value"].toMap()["sink"].toMap()["val"].toString(), QStringLiteral( "my_path" ) );
  QCOMPARE( map["default_value"].toMap()["create_options"].toMap()["fileEncoding"].toString(), QStringLiteral( "my_encoding" ) );
  QgsProcessingModelOutput out;
  out.loadVariant( map );
  QVERIFY( out.defaultValue().canConvert<QgsProcessingOutputLayerDefinition>() );
  layerDef = out.defaultValue().value<QgsProcessingOutputLayerDefinition>();
  QCOMPARE( layerDef.sink.staticValue().toString(), QStringLiteral( "my_path" ) );
  QCOMPARE( layerDef.createOptions["fileEncoding"].toString(), QStringLiteral( "my_encoding" ) );

  QMap<QString, QgsProcessingModelOutput> outputs;
  QgsProcessingModelOutput out1;
  out1.setDescription( QStringLiteral( "my output" ) );
  outputs.insert( QStringLiteral( "a" ), out1 );
  child.setModelOutputs( outputs );
  QCOMPARE( child.modelOutputs().count(), 1 );
  QCOMPARE( child.modelOutputs().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "my output" ) );
  QCOMPARE( child.modelOutput( "a" ).description(), QStringLiteral( "my output" ) );
  child.modelOutput( "a" ).setDescription( QStringLiteral( "my output 2" ) );
  QCOMPARE( child.modelOutput( "a" ).description(), QStringLiteral( "my output 2" ) );
  // no existent
  child.modelOutput( "b" ).setDescription( QStringLiteral( "my output 3" ) );
  QCOMPARE( child.modelOutput( "b" ).description(), QStringLiteral( "my output 3" ) );
  QCOMPARE( child.modelOutputs().count(), 2 );
  child.removeModelOutput( QStringLiteral( "a" ) );
  QCOMPARE( child.modelOutputs().count(), 1 );


  // model algorithm tests


  QgsProcessingModelAlgorithm alg( "test", "testGroup" );
  QCOMPARE( alg.name(), QStringLiteral( "test" ) );
  QCOMPARE( alg.displayName(), QStringLiteral( "test" ) );
  QCOMPARE( alg.group(), QStringLiteral( "testGroup" ) );
  alg.setName( QStringLiteral( "test2" ) );
  QCOMPARE( alg.name(), QStringLiteral( "test2" ) );
  QCOMPARE( alg.displayName(), QStringLiteral( "test2" ) );
  alg.setGroup( QStringLiteral( "group2" ) );
  QCOMPARE( alg.group(), QStringLiteral( "group2" ) );

  QVariantMap help;
  alg.setHelpContent( help );
  QVERIFY( alg.helpContent().isEmpty() );
  QVERIFY( alg.helpUrl().isEmpty() );
  QVERIFY( alg.shortDescription().isEmpty() );
  help.insert( QStringLiteral( "SHORT_DESCRIPTION" ), QStringLiteral( "short" ) );
  help.insert( QStringLiteral( "HELP_URL" ), QStringLiteral( "url" ) );
  alg.setHelpContent( help );
  QCOMPARE( alg.helpContent(), help );
  QCOMPARE( alg.shortDescription(), QStringLiteral( "short" ) );
  QCOMPARE( alg.helpUrl(), QStringLiteral( "url" ) );

  // child algorithms
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1;
  a1.setDescription( QStringLiteral( "alg1" ) );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( QStringLiteral( "alg2" ) );
  algs.insert( QStringLiteral( "a" ), a1 );
  algs.insert( QStringLiteral( "b" ), a2 );
  alg.setChildAlgorithms( algs );
  QCOMPARE( alg.childAlgorithms().count(), 2 );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "alg1" ) );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "b" ) ).description(), QStringLiteral( "alg2" ) );
  QgsProcessingModelChildAlgorithm a3;
  a3.setChildId( QStringLiteral( "c" ) );
  a3.setDescription( QStringLiteral( "alg3" ) );
  QCOMPARE( alg.addChildAlgorithm( a3 ), QStringLiteral( "c" ) );
  QCOMPARE( alg.childAlgorithms().count(), 3 );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "alg1" ) );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "b" ) ).description(), QStringLiteral( "alg2" ) );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "c" ) ).description(), QStringLiteral( "alg3" ) );
  QCOMPARE( alg.childAlgorithm( "a" ).description(), QStringLiteral( "alg1" ) );
  QCOMPARE( alg.childAlgorithm( "b" ).description(), QStringLiteral( "alg2" ) );
  QCOMPARE( alg.childAlgorithm( "c" ).description(), QStringLiteral( "alg3" ) );
  // initially non-existent
  QVERIFY( alg.childAlgorithm( "d" ).description().isEmpty() );
  alg.childAlgorithm( "d" ).setDescription( QStringLiteral( "alg4" ) );
  QCOMPARE( alg.childAlgorithm( "d" ).description(), QStringLiteral( "alg4" ) );
  // overwrite existing
  QgsProcessingModelChildAlgorithm a4a;
  a4a.setChildId( "d" );
  a4a.setDescription( "new" );
  alg.setChildAlgorithm( a4a );
  QCOMPARE( alg.childAlgorithm( "d" ).description(), QStringLiteral( "new" ) );


  // generating child ids
  QgsProcessingModelChildAlgorithm c1;
  c1.setAlgorithmId( QStringLiteral( "buffer" ) );
  c1.generateChildId( alg );
  QCOMPARE( c1.childId(), QStringLiteral( "buffer_1" ) );
  QCOMPARE( alg.addChildAlgorithm( c1 ), QStringLiteral( "buffer_1" ) );
  QgsProcessingModelChildAlgorithm c2;
  c2.setAlgorithmId( QStringLiteral( "buffer" ) );
  c2.generateChildId( alg );
  QCOMPARE( c2.childId(), QStringLiteral( "buffer_2" ) );
  QCOMPARE( alg.addChildAlgorithm( c2 ), QStringLiteral( "buffer_2" ) );
  QgsProcessingModelChildAlgorithm c3;
  c3.setAlgorithmId( QStringLiteral( "centroid" ) );
  c3.generateChildId( alg );
  QCOMPARE( c3.childId(), QStringLiteral( "centroid_1" ) );
  QCOMPARE( alg.addChildAlgorithm( c3 ), QStringLiteral( "centroid_1" ) );
  QgsProcessingModelChildAlgorithm c4;
  c4.setAlgorithmId( QStringLiteral( "centroid" ) );
  c4.setChildId( QStringLiteral( "centroid_1" ) );// dupe id
  QCOMPARE( alg.addChildAlgorithm( c4 ), QStringLiteral( "centroid_2" ) );
  QCOMPARE( alg.childAlgorithm( QStringLiteral( "centroid_2" ) ).childId(), QStringLiteral( "centroid_2" ) );

  // parameter components
  QMap<QString, QgsProcessingModelParameter> pComponents;
  QgsProcessingModelParameter pc1;
  pc1.setParameterName( QStringLiteral( "my_param" ) );
  QCOMPARE( pc1.parameterName(), QStringLiteral( "my_param" ) );
  pComponents.insert( QStringLiteral( "my_param" ), pc1 );
  alg.setParameterComponents( pComponents );
  QCOMPARE( alg.parameterComponents().count(), 1 );
  QCOMPARE( alg.parameterComponents().value( QStringLiteral( "my_param" ) ).parameterName(), QStringLiteral( "my_param" ) );
  QCOMPARE( alg.parameterComponent( "my_param" ).parameterName(), QStringLiteral( "my_param" ) );
  alg.parameterComponent( "my_param" ).setDescription( QStringLiteral( "my param 2" ) );
  QCOMPARE( alg.parameterComponent( "my_param" ).description(), QStringLiteral( "my param 2" ) );
  // no existent
  alg.parameterComponent( "b" ).setDescription( QStringLiteral( "my param 3" ) );
  QCOMPARE( alg.parameterComponent( "b" ).description(), QStringLiteral( "my param 3" ) );
  QCOMPARE( alg.parameterComponent( "b" ).parameterName(), QStringLiteral( "b" ) );
  QCOMPARE( alg.parameterComponents().count(), 2 );

  // parameter definitions
  QgsProcessingModelAlgorithm alg1a( "test", "testGroup" );
  QgsProcessingModelParameter bool1;
  bool1.setPosition( QPointF( 1, 2 ) );
  alg1a.addModelParameter( new QgsProcessingParameterBoolean( "p1", "desc" ), bool1 );
  QCOMPARE( alg1a.parameterDefinitions().count(), 1 );
  QCOMPARE( alg1a.parameterDefinition( "p1" )->type(), QStringLiteral( "boolean" ) );
  QCOMPARE( alg1a.parameterComponent( "p1" ).position().x(), 1.0 );
  QCOMPARE( alg1a.parameterComponent( "p1" ).position().y(), 2.0 );
  alg1a.updateModelParameter( new QgsProcessingParameterBoolean( "p1", "descx" ) );
  QCOMPARE( alg1a.parameterDefinition( "p1" )->description(), QStringLiteral( "descx" ) );
  alg1a.removeModelParameter( "bad" );
  QCOMPARE( alg1a.parameterDefinitions().count(), 1 );
  alg1a.removeModelParameter( "p1" );
  QVERIFY( alg1a.parameterDefinitions().isEmpty() );
  QVERIFY( alg1a.parameterComponents().isEmpty() );


  // test canExecute
  QgsProcessingModelAlgorithm alg2( "test", "testGroup" );
  QVERIFY( alg2.canExecute() );
  QgsProcessingModelChildAlgorithm c5;
  c5.setAlgorithmId( "native:centroids" );
  alg2.addChildAlgorithm( c5 );
  QVERIFY( alg2.canExecute() );
  // non-existing alg
  QgsProcessingModelChildAlgorithm c6;
  c6.setAlgorithmId( "i'm not an alg" );
  alg2.addChildAlgorithm( c6 );
  QVERIFY( !alg2.canExecute() );

  // test that children are re-attached before testing for canExecute
  QgsProcessingModelAlgorithm alg2a( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm c5a;
  c5a.setAlgorithmId( "native:centroids" );
  alg2a.addChildAlgorithm( c5a );
  // simulate initially missing provider or algorithm (e.g. another model as a child algorithm)
  alg2a.mChildAlgorithms.begin().value().mAlgorithm.reset();
  QVERIFY( alg2a.canExecute() );

  // dependencies
  QgsProcessingModelAlgorithm alg3( "test", "testGroup" );
  QVERIFY( alg3.dependentChildAlgorithms( "notvalid" ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( "notvalid" ).isEmpty() );

  // add a child
  QgsProcessingModelChildAlgorithm c7;
  c7.setChildId( "c7" );
  alg3.addChildAlgorithm( c7 );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c7" ).isEmpty() );

  // direct dependency
  QgsProcessingModelChildAlgorithm c8;
  c8.setChildId( "c8" );
  c8.setDependencies( QStringList() << "c7" );
  alg3.addChildAlgorithm( c8 );
  QVERIFY( alg3.dependentChildAlgorithms( "c8" ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c7" ).isEmpty() );
  QCOMPARE( alg3.dependentChildAlgorithms( "c7" ).count(), 1 );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).contains( "c8" ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c8" ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c8" ).contains( "c7" ) );

  // dependency via parameter source
  QgsProcessingModelChildAlgorithm c9;
  c9.setChildId( "c9" );
  c9.addParameterSources( "x", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "c8", "x" ) );
  alg3.addChildAlgorithm( c9 );
  QVERIFY( alg3.dependentChildAlgorithms( "c9" ).isEmpty() );
  QCOMPARE( alg3.dependentChildAlgorithms( "c8" ).count(), 1 );
  QVERIFY( alg3.dependentChildAlgorithms( "c8" ).contains( "c9" ) );
  QCOMPARE( alg3.dependentChildAlgorithms( "c7" ).count(), 2 );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).contains( "c8" ) );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).contains( "c9" ) );

  QVERIFY( alg3.dependsOnChildAlgorithms( "c7" ).isEmpty() );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c8" ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c8" ).contains( "c7" ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c9" ).count(), 2 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9" ).contains( "c7" ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9" ).contains( "c8" ) );

  QgsProcessingModelChildAlgorithm c9b;
  c9b.setChildId( "c9b" );
  c9b.addParameterSources( "x", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "c9", "x" ) );
  alg3.addChildAlgorithm( c9b );

  QCOMPARE( alg3.dependentChildAlgorithms( "c9" ).count(), 1 );
  QCOMPARE( alg3.dependentChildAlgorithms( "c8" ).count(), 2 );
  QVERIFY( alg3.dependentChildAlgorithms( "c8" ).contains( "c9" ) );
  QVERIFY( alg3.dependentChildAlgorithms( "c8" ).contains( "c9b" ) );
  QCOMPARE( alg3.dependentChildAlgorithms( "c7" ).count(), 3 );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).contains( "c8" ) );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).contains( "c9" ) );
  QVERIFY( alg3.dependentChildAlgorithms( "c7" ).contains( "c9b" ) );

  QVERIFY( alg3.dependsOnChildAlgorithms( "c7" ).isEmpty() );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c8" ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c8" ).contains( "c7" ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c9" ).count(), 2 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9" ).contains( "c7" ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9" ).contains( "c8" ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c9b" ).count(), 3 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( "c7" ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( "c8" ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( "c9" ) );

  alg3.removeChildAlgorithm( "c9b" );


  // (de)activate child algorithm
  alg3.deactivateChildAlgorithm( "c9" );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( "c9" ) );
  QVERIFY( alg3.childAlgorithm( "c9" ).isActive() );
  alg3.deactivateChildAlgorithm( "c8" );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( "c9" ) );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( "c8" ) );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( "c9" ) );
  QVERIFY( alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c8" ).isActive() );
  alg3.deactivateChildAlgorithm( "c7" );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c7" ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( "c9" ) );
  QVERIFY( !alg3.activateChildAlgorithm( "c8" ) );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c7" ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( "c8" ) );
  QVERIFY( alg3.activateChildAlgorithm( "c7" ) );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( !alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c7" ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( "c9" ) );
  QVERIFY( alg3.activateChildAlgorithm( "c8" ) );
  QVERIFY( !alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c7" ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( "c9" ) );
  QVERIFY( alg3.childAlgorithm( "c9" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c8" ).isActive() );
  QVERIFY( alg3.childAlgorithm( "c7" ).isActive() );



  //remove child algorithm
  QVERIFY( !alg3.removeChildAlgorithm( "c7" ) );
  QVERIFY( !alg3.removeChildAlgorithm( "c8" ) );
  QVERIFY( alg3.removeChildAlgorithm( "c9" ) );
  QCOMPARE( alg3.childAlgorithms().count(), 2 );
  QVERIFY( alg3.childAlgorithms().contains( "c7" ) );
  QVERIFY( alg3.childAlgorithms().contains( "c8" ) );
  QVERIFY( !alg3.removeChildAlgorithm( "c7" ) );
  QVERIFY( alg3.removeChildAlgorithm( "c8" ) );
  QCOMPARE( alg3.childAlgorithms().count(), 1 );
  QVERIFY( alg3.childAlgorithms().contains( "c7" ) );
  QVERIFY( alg3.removeChildAlgorithm( "c7" ) );
  QVERIFY( alg3.childAlgorithms().isEmpty() );

  // parameter dependencies
  QgsProcessingModelAlgorithm alg4( "test", "testGroup" );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "not a param" ) );
  QgsProcessingModelChildAlgorithm c10;
  c10.setChildId( "c10" );
  alg4.addChildAlgorithm( c10 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "not a param" ) );
  QgsProcessingModelParameter bool2;
  alg4.addModelParameter( new QgsProcessingParameterBoolean( "p1", "desc" ), bool2 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "p1" ) );
  c10.addParameterSources( "x", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "p2" ) );
  alg4.setChildAlgorithm( c10 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "p1" ) );
  c10.addParameterSources( "y", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "p1" ) );
  alg4.setChildAlgorithm( c10 );
  QVERIFY( alg4.childAlgorithmsDependOnParameter( "p1" ) );

  QgsProcessingModelParameter vlP;
  alg4.addModelParameter( new QgsProcessingParameterVectorLayer( "layer" ), vlP );
  QgsProcessingModelParameter field;
  alg4.addModelParameter( new QgsProcessingParameterField( "field", QString(), QVariant(), QStringLiteral( "layer" ) ), field );
  QVERIFY( !alg4.otherParametersDependOnParameter( "p1" ) );
  QVERIFY( !alg4.otherParametersDependOnParameter( "field" ) );
  QVERIFY( alg4.otherParametersDependOnParameter( "layer" ) );





  // to/from XML
  QgsProcessingModelAlgorithm alg5( "test", "testGroup" );
  alg5.helpContent().insert( "author", "me" );
  alg5.helpContent().insert( "usage", "run" );
  QgsProcessingModelChildAlgorithm alg5c1;
  alg5c1.setChildId( "cx1" );
  alg5c1.setAlgorithmId( "buffer" );
  alg5c1.setConfiguration( myConfig );
  alg5c1.addParameterSources( "x", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "p1" ) );
  alg5c1.addParameterSources( "y", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx2", "out3" ) );
  alg5c1.addParameterSources( "z", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  alg5c1.addParameterSources( "a", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( "2*2" ) );
  alg5c1.addParameterSources( "zm", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 6 )
                              << QgsProcessingModelChildParameterSource::fromModelParameter( "p2" )
                              << QgsProcessingModelChildParameterSource::fromChildOutput( "cx2", "out4" )
                              << QgsProcessingModelChildParameterSource::fromExpression( "1+2" )
                              << QgsProcessingModelChildParameterSource::fromStaticValue( QgsProperty::fromExpression( "1+8" ) ) );
  alg5c1.setActive( true );
  alg5c1.setOutputsCollapsed( true );
  alg5c1.setParametersCollapsed( true );
  alg5c1.setDescription( "child 1" );
  alg5c1.setPosition( QPointF( 1, 2 ) );
  QMap<QString, QgsProcessingModelOutput> alg5c1outputs;
  QgsProcessingModelOutput alg5c1out1;
  alg5c1out1.setDescription( QStringLiteral( "my output" ) );
  alg5c1out1.setPosition( QPointF( 3, 4 ) );
  alg5c1outputs.insert( QStringLiteral( "a" ), alg5c1out1 );
  alg5c1.setModelOutputs( alg5c1outputs );
  alg5.addChildAlgorithm( alg5c1 );

  QgsProcessingModelChildAlgorithm alg5c2;
  alg5c2.setChildId( "cx2" );
  alg5c2.setActive( false );
  alg5c2.setOutputsCollapsed( false );
  alg5c2.setParametersCollapsed( false );
  alg5c2.setDependencies( QStringList() << "a" << "b" );
  alg5.addChildAlgorithm( alg5c2 );

  QgsProcessingModelParameter alg5pc1;
  alg5pc1.setParameterName( QStringLiteral( "my_param" ) );
  alg5pc1.setPosition( QPointF( 11, 12 ) );
  alg5.addModelParameter( new QgsProcessingParameterBoolean( QStringLiteral( "my_param" ) ), alg5pc1 );

  QDomDocument doc = QDomDocument( "model" );
  QDomElement elem = QgsXmlUtils::writeVariant( alg5.toVariant(), doc );
  doc.appendChild( elem );

  QgsProcessingModelAlgorithm alg6;
  QVERIFY( alg6.loadVariant( QgsXmlUtils::readVariant( doc.firstChildElement() ) ) );
  QCOMPARE( alg6.name(), QStringLiteral( "test" ) );
  QCOMPARE( alg6.group(), QStringLiteral( "testGroup" ) );
  QCOMPARE( alg6.helpContent(), alg5.helpContent() );
  QgsProcessingModelChildAlgorithm alg6c1 = alg6.childAlgorithm( "cx1" );
  QCOMPARE( alg6c1.childId(), QStringLiteral( "cx1" ) );
  QCOMPARE( alg6c1.algorithmId(), QStringLiteral( "buffer" ) );
  QCOMPARE( alg6c1.configuration(), myConfig );
  QVERIFY( alg6c1.isActive() );
  QVERIFY( alg6c1.outputsCollapsed() );
  QVERIFY( alg6c1.parametersCollapsed() );
  QCOMPARE( alg6c1.description(), QStringLiteral( "child 1" ) );
  QCOMPARE( alg6c1.position().x(), 1.0 );
  QCOMPARE( alg6c1.position().y(), 2.0 );
  QCOMPARE( alg6c1.parameterSources().count(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( "x" ).at( 0 ).source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( alg6c1.parameterSources().value( "x" ).at( 0 ).parameterName(), QStringLiteral( "p1" ) );
  QCOMPARE( alg6c1.parameterSources().value( "y" ).at( 0 ).source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( alg6c1.parameterSources().value( "y" ).at( 0 ).outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( alg6c1.parameterSources().value( "y" ).at( 0 ).outputName(), QStringLiteral( "out3" ) );
  QCOMPARE( alg6c1.parameterSources().value( "z" ).at( 0 ).source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( alg6c1.parameterSources().value( "z" ).at( 0 ).staticValue().toInt(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( "a" ).at( 0 ).source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( alg6c1.parameterSources().value( "a" ).at( 0 ).expression(), QStringLiteral( "2*2" ) );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).count(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 0 ).source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 0 ).staticValue().toInt(), 6 );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 1 ).source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 1 ).parameterName(), QStringLiteral( "p2" ) );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 2 ).source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 2 ).outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 2 ).outputName(), QStringLiteral( "out4" ) );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 3 ).source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 3 ).expression(), QStringLiteral( "1+2" ) );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 4 ).source(), QgsProcessingModelChildParameterSource::StaticValue );
  QVERIFY( alg6c1.parameterSources().value( "zm" ).at( 4 ).staticValue().canConvert< QgsProperty >() );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 4 ).staticValue().value< QgsProperty >().expressionString(), QStringLiteral( "1+8" ) );

  QCOMPARE( alg6c1.modelOutputs().count(), 1 );
  QCOMPARE( alg6c1.modelOutputs().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "my output" ) );
  QCOMPARE( alg6c1.modelOutput( "a" ).description(), QStringLiteral( "my output" ) );
  QCOMPARE( alg6c1.modelOutput( "a" ).position().x(), 3.0 );
  QCOMPARE( alg6c1.modelOutput( "a" ).position().y(), 4.0 );


  QgsProcessingModelChildAlgorithm alg6c2 = alg6.childAlgorithm( "cx2" );
  QCOMPARE( alg6c2.childId(), QStringLiteral( "cx2" ) );
  QVERIFY( !alg6c2.isActive() );
  QVERIFY( !alg6c2.outputsCollapsed() );
  QVERIFY( !alg6c2.parametersCollapsed() );
  QCOMPARE( alg6c2.dependencies(), QStringList() << "a" << "b" );

  QCOMPARE( alg6.parameterComponents().count(), 1 );
  QCOMPARE( alg6.parameterComponents().value( QStringLiteral( "my_param" ) ).parameterName(), QStringLiteral( "my_param" ) );
  QCOMPARE( alg6.parameterComponent( "my_param" ).parameterName(), QStringLiteral( "my_param" ) );
  QCOMPARE( alg6.parameterComponent( "my_param" ).position().x(), 11.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).position().y(), 12.0 );
  QCOMPARE( alg6.parameterDefinitions().count(), 1 );
  QCOMPARE( alg6.parameterDefinitions().at( 0 )->type(), QStringLiteral( "boolean" ) );

  // destination parameters
  QgsProcessingModelAlgorithm alg7( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm alg7c1;
  alg7c1.setChildId( "cx1" );
  alg7c1.setAlgorithmId( "native:centroids" );
  QMap<QString, QgsProcessingModelOutput> alg7c1outputs;
  QgsProcessingModelOutput alg7c1out1( QStringLiteral( "my_output" ) );
  alg7c1out1.setChildId( "cx1" );
  alg7c1out1.setChildOutputName( "OUTPUT" );
  alg7c1out1.setDescription( QStringLiteral( "my output" ) );
  alg7c1outputs.insert( QStringLiteral( "my_output" ), alg7c1out1 );
  alg7c1.setModelOutputs( alg7c1outputs );
  alg7.addChildAlgorithm( alg7c1 );
  // verify that model has destination parameter created
  QCOMPARE( alg7.destinationParameterDefinitions().count(), 1 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "cx1:my_output" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg7.destinationParameterDefinitions().at( 0 ) )->originalProvider()->id(), QStringLiteral( "native" ) );
  QCOMPARE( alg7.outputDefinitions().count(), 1 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), QStringLiteral( "cx1:my_output" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), QStringLiteral( "outputVector" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );

  QgsProcessingModelChildAlgorithm alg7c2;
  alg7c2.setChildId( "cx2" );
  alg7c2.setAlgorithmId( "native:centroids" );
  QMap<QString, QgsProcessingModelOutput> alg7c2outputs;
  QgsProcessingModelOutput alg7c2out1( QStringLiteral( "my_output2" ) );
  alg7c2out1.setChildId( "cx2" );
  alg7c2out1.setChildOutputName( "OUTPUT" );
  alg7c2out1.setDescription( QStringLiteral( "my output2" ) );
  alg7c2out1.setDefaultValue( QStringLiteral( "my value" ) );
  alg7c2out1.setMandatory( true );
  alg7c2outputs.insert( QStringLiteral( "my_output2" ), alg7c2out1 );
  alg7c2.setModelOutputs( alg7c2outputs );
  alg7.addChildAlgorithm( alg7c2 );

  QCOMPARE( alg7.destinationParameterDefinitions().count(), 2 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "cx1:my_output" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QVERIFY( alg7.destinationParameterDefinitions().at( 0 )->defaultValue().isNull() );
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 0 )->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->name(), QStringLiteral( "cx2:my_output2" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->description(), QStringLiteral( "my output2" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->defaultValue().toString(), QStringLiteral( "my value" ) );
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 1 )->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QCOMPARE( alg7.outputDefinitions().count(), 2 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), QStringLiteral( "cx1:my_output" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), QStringLiteral( "outputVector" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->name(), QStringLiteral( "cx2:my_output2" ) );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->type(), QStringLiteral( "outputVector" ) );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->description(), QStringLiteral( "my output2" ) );

  alg7.removeChildAlgorithm( "cx1" );
  QCOMPARE( alg7.destinationParameterDefinitions().count(), 1 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "cx2:my_output2" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output2" ) );
  QCOMPARE( alg7.outputDefinitions().count(), 1 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), QStringLiteral( "cx2:my_output2" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), QStringLiteral( "outputVector" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), QStringLiteral( "my output2" ) );

  // mandatory model output with optional child algorithm parameter
  QgsProcessingModelChildAlgorithm alg7c3;
  alg7c3.setChildId( "cx3" );
  alg7c3.setAlgorithmId( "native:extractbyexpression" );
  QMap<QString, QgsProcessingModelOutput> alg7c3outputs;
  QgsProcessingModelOutput alg7c3out1;
  alg7c3out1.setChildId( "cx3" );
  alg7c3out1.setChildOutputName( "FAIL_OUTPUT" );
  alg7c3out1.setDescription( QStringLiteral( "my_output3" ) );
  alg7c3outputs.insert( QStringLiteral( "my_output3" ), alg7c3out1 );
  alg7c3.setModelOutputs( alg7c3outputs );
  alg7.addChildAlgorithm( alg7c3 );
  QVERIFY( alg7.destinationParameterDefinitions().at( 1 )->flags() & QgsProcessingParameterDefinition::FlagOptional );
  alg7.childAlgorithm( alg7c3.childId() ).modelOutput( QStringLiteral( "my_output3" ) ).setMandatory( true );
  alg7.updateDestinationParameters();
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 1 )->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
}

void TestQgsProcessing::modelExecution()
{
  // test childOutputIsRequired
  QgsProcessingModelAlgorithm model1;
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "cx1" );
  algc1.setAlgorithmId( "native:centroids" );
  model1.addChildAlgorithm( algc1 );
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( "cx2" );
  algc2.setAlgorithmId( "native:centroids" );
  algc2.addParameterSources( "x", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx1", "p1" ) );
  model1.addChildAlgorithm( algc2 );
  QgsProcessingModelChildAlgorithm algc3;
  algc3.setChildId( "cx3" );
  algc3.setAlgorithmId( "native:centroids" );
  algc3.addParameterSources( "x", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx1", "p2" ) );
  algc3.setActive( false );
  model1.addChildAlgorithm( algc3 );

  QVERIFY( model1.childOutputIsRequired( "cx1", "p1" ) ); // cx2 depends on p1
  QVERIFY( !model1.childOutputIsRequired( "cx1", "p2" ) ); // cx3 depends on p2, but cx3 is not active
  QVERIFY( !model1.childOutputIsRequired( "cx1", "p3" ) ); // nothing requires p3
  QVERIFY( !model1.childOutputIsRequired( "cx2", "p1" ) );
  QVERIFY( !model1.childOutputIsRequired( "cx3", "p1" ) );

  // test parametersForChildAlgorithm
  QgsProcessingModelAlgorithm model2;
  model2.addModelParameter( new QgsProcessingParameterFeatureSource( "SOURCE_LAYER" ), QgsProcessingModelParameter( "SOURCE_LAYER" ) );
  model2.addModelParameter( new QgsProcessingParameterNumber( "DIST", QString(), QgsProcessingParameterNumber::Double ), QgsProcessingModelParameter( "DIST" ) );
  QgsProcessingModelChildAlgorithm alg2c1;
  QgsExpressionContext expContext;
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->setVariable( "myvar", 8 );
  expContext.appendScope( scope );
  alg2c1.setChildId( "cx1" );
  alg2c1.setAlgorithmId( "native:buffer" );
  alg2c1.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "SOURCE_LAYER" ) );
  alg2c1.addParameterSources( "DISTANCE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "DIST" ) );
  alg2c1.addParameterSources( "SEGMENTS", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "@myvar*2" ) ) );
  alg2c1.addParameterSources( "END_CAP_STYLE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 1 ) );
  alg2c1.addParameterSources( "JOIN_STYLE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 2 ) );
  alg2c1.addParameterSources( "DISSOLVE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( false ) );
  QMap<QString, QgsProcessingModelOutput> outputs1;
  QgsProcessingModelOutput out1( "MODEL_OUT_LAYER" );
  out1.setChildOutputName( "OUTPUT" );
  outputs1.insert( QStringLiteral( "MODEL_OUT_LAYER" ), out1 );
  alg2c1.setModelOutputs( outputs1 );
  model2.addChildAlgorithm( alg2c1 );

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString vector = testDataDir + "points.shp";

  QVariantMap modelInputs;
  modelInputs.insert( "SOURCE_LAYER", vector );
  modelInputs.insert( "DIST", 271 );
  modelInputs.insert( "cx1:MODEL_OUT_LAYER", "dest.shp" );
  QgsProcessingOutputLayerDefinition layerDef( "memory:" );
  layerDef.destinationName = "my_dest";
  modelInputs.insert( "cx3:MY_OUT", QVariant::fromValue( layerDef ) );
  QVariantMap childResults;
  QVariantMap params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx1" ), modelInputs, childResults, expContext );
  QCOMPARE( params.value( "DISSOLVE" ).toBool(), false );
  QCOMPARE( params.value( "DISTANCE" ).toInt(), 271 );
  QCOMPARE( params.value( "SEGMENTS" ).toInt(), 16 );
  QCOMPARE( params.value( "END_CAP_STYLE" ).toInt(), 1 );
  QCOMPARE( params.value( "JOIN_STYLE" ).toInt(), 2 );
  QCOMPARE( params.value( "INPUT" ).toString(), vector );
  QCOMPARE( params.value( "OUTPUT" ).toString(), QStringLiteral( "dest.shp" ) );
  QCOMPARE( params.count(), 7 );

  QgsProcessingContext context;

  // Check variables for child algorithm
  // without values
  QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = model2.variablesForChildAlgorithm( "cx1", context );
  QCOMPARE( variables.count(), 6 );
  QCOMPARE( variables.value( "DIST" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );

  // with values
  variables = model2.variablesForChildAlgorithm( "cx1", context, modelInputs, childResults );
  QCOMPARE( variables.count(), 6 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  std::unique_ptr< QgsExpressionContextScope > childScope( model2.createExpressionContextScopeForChildAlgorithm( "cx1", context, modelInputs, childResults ) );
  QCOMPARE( childScope->name(), QStringLiteral( "algorithm_inputs" ) );
  QCOMPARE( childScope->variableCount(), 6 );
  QCOMPARE( childScope->variable( "DIST" ).toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_minx" ).toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_miny" ).toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_maxx" ).toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_maxy" ).toDouble(), 46.8719, 0.001 );


  QVariantMap results;
  results.insert( "OUTPUT", QStringLiteral( "dest.shp" ) );
  childResults.insert( "cx1", results );

  // a child who uses an output from another alg as a parameter value
  QgsProcessingModelChildAlgorithm alg2c2;
  alg2c2.setChildId( "cx2" );
  alg2c2.setAlgorithmId( "native:centroids" );
  alg2c2.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx1", "OUTPUT" ) );
  model2.addChildAlgorithm( alg2c2 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx2" ), modelInputs, childResults, expContext );
  QCOMPARE( params.value( "INPUT" ).toString(), QStringLiteral( "dest.shp" ) );
  QCOMPARE( params.value( "OUTPUT" ).toString(), QStringLiteral( "memory:Centroids" ) );
  QCOMPARE( params.count(), 2 );

  variables = model2.variablesForChildAlgorithm( "cx2", context );
  QCOMPARE( variables.count(), 11 );
  QCOMPARE( variables.value( "DIST" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.outputChildId(), QStringLiteral( "cx1" ) );

  // with values
  variables = model2.variablesForChildAlgorithm( "cx2", context, modelInputs, childResults );
  QCOMPARE( variables.count(), 11 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QString( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.parameterName(), QString( "" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  // a child with an optional output
  QgsProcessingModelChildAlgorithm alg2c3;
  alg2c3.setChildId( "cx3" );
  alg2c3.setAlgorithmId( "native:extractbyexpression" );
  alg2c3.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx1", "OUTPUT" ) );
  alg2c3.addParameterSources( "EXPRESSION", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "true" ) );
  alg2c3.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "MY_OUT" ) );
  alg2c3.setDependencies( QStringList() << "cx2" );
  QMap<QString, QgsProcessingModelOutput> outputs3;
  QgsProcessingModelOutput out2( "MY_OUT" );
  out2.setChildOutputName( "OUTPUT" );
  outputs3.insert( QStringLiteral( "MY_OUT" ), out2 );
  alg2c3.setModelOutputs( outputs3 );

  model2.addChildAlgorithm( alg2c3 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3" ), modelInputs, childResults, expContext );
  QCOMPARE( params.value( "INPUT" ).toString(), QStringLiteral( "dest.shp" ) );
  QCOMPARE( params.value( "EXPRESSION" ).toString(), QStringLiteral( "true" ) );
  QVERIFY( params.value( "OUTPUT" ).canConvert<QgsProcessingOutputLayerDefinition>() );
  QgsProcessingOutputLayerDefinition outDef = qvariant_cast<QgsProcessingOutputLayerDefinition>( params.value( "OUTPUT" ) );
  QCOMPARE( outDef.destinationName, QStringLiteral( "MY_OUT" ) );
  QCOMPARE( outDef.sink.staticValue().toString(), QStringLiteral( "memory:" ) );
  QCOMPARE( params.count(), 3 ); // don't want FAIL_OUTPUT set!

  variables = model2.variablesForChildAlgorithm( "cx3", context );
  QCOMPARE( variables.count(), 16 );
  QCOMPARE( variables.value( "DIST" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT_minx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_minx" ).source.outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT_miny" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_miny" ).source.outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxx" ).source.outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxy" ).source.outputChildId(), QStringLiteral( "cx2" ) );
  // with values
  variables = model2.variablesForChildAlgorithm( "cx3", context, modelInputs, childResults );
  QCOMPARE( variables.count(), 16 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QString( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.parameterName(), QString( "" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.outputChildId(), QString( "cx2" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.parameterName(), QString( "" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  QStringList actualParts = model2.asPythonCode().split( '\n' );
  QStringList expectedParts = QStringLiteral( "##model=name\n"
                              "##DIST=number\n"
                              "##SOURCE_LAYER=source\n"
                              "##model_out_layer=output outputVector\n"
                              "##my_out=output outputVector\n"
                              "results={}\n"
                              "outputs['cx1']=processing.run('native:buffer', {'DISSOLVE':false,'DISTANCE':parameters['DIST'],'END_CAP_STYLE':1,'INPUT':parameters['SOURCE_LAYER'],'JOIN_STYLE':2,'SEGMENTS':QgsExpression('@myvar*2').evaluate()}, context=context, feedback=feedback)\n"
                              "results['MODEL_OUT_LAYER']=outputs['cx1']['OUTPUT']\n"
                              "outputs['cx2']=processing.run('native:centroids', {'INPUT':outputs['cx1']['OUTPUT']}, context=context, feedback=feedback)\n"
                              "outputs['cx3']=processing.run('native:extractbyexpression', {'EXPRESSION':true,'INPUT':outputs['cx1']['OUTPUT'],'OUTPUT':parameters['MY_OUT']}, context=context, feedback=feedback)\n"
                              "results['MY_OUT']=outputs['cx3']['OUTPUT']\n"
                              "return results" ).split( '\n' );
  QCOMPARE( actualParts, expectedParts );
}

void TestQgsProcessing::modelWithProviderWithLimitedTypes()
{
  QgsApplication::processingRegistry()->addProvider( new DummyProvider4() );

  QgsProcessingModelAlgorithm alg( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "cx1" );
  algc1.setAlgorithmId( "dummy4:alg1" );
  QMap<QString, QgsProcessingModelOutput> algc1outputs;
  QgsProcessingModelOutput algc1out1( QStringLiteral( "my_vector_output" ) );
  algc1out1.setChildId( "cx1" );
  algc1out1.setChildOutputName( "vector_dest" );
  algc1out1.setDescription( QStringLiteral( "my output" ) );
  algc1outputs.insert( QStringLiteral( "my_vector_output" ), algc1out1 );
  QgsProcessingModelOutput algc1out2( QStringLiteral( "my_raster_output" ) );
  algc1out2.setChildId( "cx1" );
  algc1out2.setChildOutputName( "raster_dest" );
  algc1out2.setDescription( QStringLiteral( "my output" ) );
  algc1outputs.insert( QStringLiteral( "my_raster_output" ), algc1out2 );
  QgsProcessingModelOutput algc1out3( QStringLiteral( "my_sink_output" ) );
  algc1out3.setChildId( "cx1" );
  algc1out3.setChildOutputName( "sink" );
  algc1out3.setDescription( QStringLiteral( "my output" ) );
  algc1outputs.insert( QStringLiteral( "my_sink_output" ), algc1out3 );
  algc1.setModelOutputs( algc1outputs );
  alg.addChildAlgorithm( algc1 );
  // verify that model has destination parameter created
  QCOMPARE( alg.destinationParameterDefinitions().count(), 3 );
  QCOMPARE( alg.destinationParameterDefinitions().at( 2 )->name(), QStringLiteral( "cx1:my_vector_output" ) );
  QCOMPARE( alg.destinationParameterDefinitions().at( 2 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 2 ) )->originalProvider()->id(), QStringLiteral( "dummy4" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterVectorDestination * >( alg.destinationParameterDefinitions().at( 2 ) )->supportedOutputVectorLayerExtensions(), QStringList() << QStringLiteral( "mif" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterVectorDestination * >( alg.destinationParameterDefinitions().at( 2 ) )->defaultFileExtension(), QStringLiteral( "mif" ) );
  QVERIFY( static_cast< const QgsProcessingParameterVectorDestination * >( alg.destinationParameterDefinitions().at( 2 ) )->generateTemporaryDestination().endsWith( QLatin1String( ".mif" ) ) );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 2 ) )->supportsNonFileBasedOutput() );

  QCOMPARE( alg.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "cx1:my_raster_output" ) );
  QCOMPARE( alg.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 0 ) )->originalProvider()->id(), QStringLiteral( "dummy4" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterRasterDestination * >( alg.destinationParameterDefinitions().at( 0 ) )->supportedOutputRasterLayerExtensions(), QStringList() << QStringLiteral( "mig" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterRasterDestination * >( alg.destinationParameterDefinitions().at( 0 ) )->defaultFileExtension(), QStringLiteral( "mig" ) );
  QVERIFY( static_cast< const QgsProcessingParameterRasterDestination * >( alg.destinationParameterDefinitions().at( 0 ) )->generateTemporaryDestination().endsWith( QLatin1String( ".mig" ) ) );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 0 ) )->supportsNonFileBasedOutput() );

  QCOMPARE( alg.destinationParameterDefinitions().at( 1 )->name(), QStringLiteral( "cx1:my_sink_output" ) );
  QCOMPARE( alg.destinationParameterDefinitions().at( 1 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 1 ) )->originalProvider()->id(), QStringLiteral( "dummy4" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterFeatureSink * >( alg.destinationParameterDefinitions().at( 1 ) )->supportedOutputVectorLayerExtensions(), QStringList() << QStringLiteral( "mif" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterFeatureSink * >( alg.destinationParameterDefinitions().at( 1 ) )->defaultFileExtension(), QStringLiteral( "mif" ) );
  QVERIFY( static_cast< const QgsProcessingParameterFeatureSink * >( alg.destinationParameterDefinitions().at( 1 ) )->generateTemporaryDestination().endsWith( QLatin1String( ".mif" ) ) );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 1 ) )->supportsNonFileBasedOutput() );
}

void TestQgsProcessing::modelVectorOutputIsCompatibleType()
{
  // IMPORTANT: This method is intended to be "permissive" rather than "restrictive".
  // I.e. we only reject outputs which we know can NEVER be acceptable, but
  // if there's doubt then we default to returning true.

  // empty acceptable type list = all are compatible
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), QgsProcessing::TypeVectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), QgsProcessing::TypeVectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), QgsProcessing::TypeMapLayer ) );

  // accept any vector
  QList< int > dataTypes;
  dataTypes << QgsProcessing::TypeVector;
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeMapLayer ) );

  // accept any vector with geometry
  dataTypes.clear();
  dataTypes << QgsProcessing::TypeVectorAnyGeometry;
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeMapLayer ) );

  // accept any point vector
  dataTypes.clear();
  dataTypes << QgsProcessing::TypeVectorPoint;
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPoint ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorLine ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeMapLayer ) );

  // accept any line vector
  dataTypes.clear();
  dataTypes << QgsProcessing::TypeVectorLine;
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorLine ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeMapLayer ) );

  // accept any polygon vector
  dataTypes.clear();
  dataTypes << QgsProcessing::TypeVectorPolygon;
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPoint ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeMapLayer ) );

  // accept any map layer
  dataTypes.clear();
  dataTypes << QgsProcessing::TypeMapLayer;
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeVectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, QgsProcessing::TypeMapLayer ) );
}

void TestQgsProcessing::modelAcceptableValues()
{
  QgsProcessingModelAlgorithm m;
  QgsProcessingModelParameter stringParam1( "string" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );

  QgsProcessingModelParameter stringParam2( "string2" );
  m.addModelParameter( new QgsProcessingParameterString( "string2" ), stringParam2 );

  QgsProcessingModelParameter numParam( "number" );
  m.addModelParameter( new QgsProcessingParameterNumber( "number" ), numParam );

  QgsProcessingModelParameter tableFieldParam( "field" );
  m.addModelParameter( new QgsProcessingParameterField( "field" ), tableFieldParam );

  QgsProcessingModelParameter fileParam( "file" );
  m.addModelParameter( new QgsProcessingParameterFile( "file" ), fileParam );

  // test single types
  QgsProcessingModelChildParameterSources sources = m.availableSourcesForChild( QString(), QStringList() << "number" );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "number" ) );
  sources = m.availableSourcesForChild( QString(), QStringList() << "field" );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "field" ) );
  sources = m.availableSourcesForChild( QString(), QStringList() << "file" );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "file" ) );

  // test multiple types
  sources = m.availableSourcesForChild( QString(), QStringList() << "string" << "number" << "file" );
  QCOMPARE( sources.count(), 4 );
  QSet< QString > res;
  res << sources.at( 0 ).parameterName();
  res << sources.at( 1 ).parameterName();
  res << sources.at( 2 ).parameterName();
  res << sources.at( 3 ).parameterName();

  QCOMPARE( res, QSet< QString >() << QStringLiteral( "string" )
            << QStringLiteral( "string2" )
            << QStringLiteral( "number" )
            << QStringLiteral( "file" ) );

  // check outputs
  QgsProcessingModelChildAlgorithm alg2c1;
  alg2c1.setChildId( "cx1" );
  alg2c1.setAlgorithmId( "native:centroids" );
  m.addChildAlgorithm( alg2c1 );

  sources = m.availableSourcesForChild( QString(), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 1 );
  res.clear();
  res << sources.at( 0 ).outputChildId() + ':' + sources.at( 0 ).outputName();
  QCOMPARE( res, QSet< QString >() << "cx1:OUTPUT" );

  // with dependencies between child algs
  QgsProcessingModelChildAlgorithm alg2c2;
  alg2c2.setChildId( "cx2" );
  alg2c2.setAlgorithmId( "native:centroids" );
  alg2c2.setDependencies( QStringList() << "cx1" );
  m.addChildAlgorithm( alg2c2 );
  sources = m.availableSourcesForChild( QString(), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 2 );
  res.clear();
  res << sources.at( 0 ).outputChildId() + ':' + sources.at( 0 ).outputName();
  res << sources.at( 1 ).outputChildId() + ':' + sources.at( 1 ).outputName();
  QCOMPARE( res, QSet< QString >() << "cx1:OUTPUT" << "cx2:OUTPUT" );

  sources = m.availableSourcesForChild( QStringLiteral( "cx1" ), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 0 );

  sources = m.availableSourcesForChild( QString( "cx2" ), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 1 );
  res.clear();
  res << sources.at( 0 ).outputChildId() + ':' + sources.at( 0 ).outputName();
  QCOMPARE( res, QSet< QString >() << "cx1:OUTPUT" );

  // test limiting by data types
  QgsProcessingModelAlgorithm m2;
  QgsProcessingModelParameter vlInput( "vl" );
  // with no limit on data types
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl" ), vlInput );
  QgsProcessingModelParameter fsInput( "fs" );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs" ), fsInput );

  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorPoint );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVector );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorAnyGeometry );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeMapLayer );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );

  // inputs are limited to vector layers
  m2.removeModelParameter( vlInput.parameterName() );
  m2.removeModelParameter( fsInput.parameterName() );
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl", QString(), QList<int>() << QgsProcessing::TypeVector ), vlInput );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs", QString(), QList<int>() << QgsProcessing::TypeVector ), fsInput );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorPoint );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVector );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorAnyGeometry );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeMapLayer );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );

  // inputs are limited to vector layers with geometries
  m2.removeModelParameter( vlInput.parameterName() );
  m2.removeModelParameter( fsInput.parameterName() );
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl", QString(), QList<int>() << QgsProcessing::TypeVectorAnyGeometry ), vlInput );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs", QString(), QList<int>() << QgsProcessing::TypeVectorAnyGeometry ), fsInput );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorPoint );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVector );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorAnyGeometry );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeMapLayer );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );

  // inputs are limited to vector layers with lines
  m2.removeModelParameter( vlInput.parameterName() );
  m2.removeModelParameter( fsInput.parameterName() );
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl", QString(), QList<int>() << QgsProcessing::TypeVectorLine ), vlInput );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs", QString(), QList<int>() << QgsProcessing::TypeVectorLine ), fsInput );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorPoint );
  QCOMPARE( sources.count(), 0 );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorPolygon );
  QCOMPARE( sources.count(), 0 );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorLine );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVector );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeVectorAnyGeometry );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << QgsProcessing::TypeMapLayer );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "fs" ) );
  QCOMPARE( sources.at( 1 ).parameterName(), QStringLiteral( "vl" ) );
}

void TestQgsProcessing::tempUtils()
{
  QString tempFolder = QgsProcessingUtils::tempFolder();
  // tempFolder should remain constant for session
  QCOMPARE( QgsProcessingUtils::tempFolder(), tempFolder );

  QString tempFile1 = QgsProcessingUtils::generateTempFilename( "test.txt" );
  QVERIFY( tempFile1.endsWith( "test.txt" ) );
  QVERIFY( tempFile1.startsWith( tempFolder ) );

  // expect a different file
  QString tempFile2 = QgsProcessingUtils::generateTempFilename( "test.txt" );
  QVERIFY( tempFile1 != tempFile2 );
  QVERIFY( tempFile2.endsWith( "test.txt" ) );
  QVERIFY( tempFile2.startsWith( tempFolder ) );

  // invalid characters
  QString tempFile3 = QgsProcessingUtils::generateTempFilename( "mybad:file.txt" );
  QVERIFY( tempFile3.endsWith( "mybad_file.txt" ) );
  QVERIFY( tempFile3.startsWith( tempFolder ) );
}

void TestQgsProcessing::convertCompatible()
{
  // start with a compatible shapefile
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString vector = testDataDir + "points.shp";
  QgsVectorLayer *layer = new QgsVectorLayer( vector, "vl" );
  QgsProject p;
  p.addMapLayer( layer );

  QgsProcessingContext context;
  context.setProject( &p );
  QgsProcessingFeedback feedback;
  QString out = QgsProcessingUtils::convertToCompatibleFormat( layer, false, QStringLiteral( "test" ), QStringList() << "shp", QString( "shp" ), context, &feedback );
  // layer should be returned unchanged - underlying source is compatible
  QCOMPARE( out, layer->source() );

  // don't include shp as compatible type
  out = QgsProcessingUtils::convertToCompatibleFormat( layer, false, QStringLiteral( "test" ), QStringList() << "tab", QString( "tab" ), context, &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".tab" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );

  // make sure all features are copied
  std::unique_ptr< QgsVectorLayer > t = qgis::make_unique< QgsVectorLayer >( out, "vl2" );
  QCOMPARE( layer->featureCount(), t->featureCount() );
  QCOMPARE( layer->crs(), t->crs() );

  // use a selection - this will require translation
  QgsFeatureIds ids;
  QgsFeature f;
  QgsFeatureIterator it = layer->getFeatures();
  it.nextFeature( f );
  ids.insert( f.id() );
  it.nextFeature( f );
  ids.insert( f.id() );

  layer->selectByIds( ids );
  out = QgsProcessingUtils::convertToCompatibleFormat( layer, true, QStringLiteral( "test" ), QStringList() << "tab", QString( "tab" ), context, &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".tab" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );
  t = qgis::make_unique< QgsVectorLayer >( out, "vl2" );
  QCOMPARE( t->featureCount(), static_cast< long >( ids.count() ) );

  // using a selection but existing format - will still require translation
  out = QgsProcessingUtils::convertToCompatibleFormat( layer, true, QStringLiteral( "test" ), QStringList() << "shp", QString( "shp" ), context, &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".shp" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );
  t = qgis::make_unique< QgsVectorLayer >( out, "vl2" );
  QCOMPARE( t->featureCount(), static_cast< long >( ids.count() ) );

  // using a feature filter -- this will require translation
  layer->setSubsetString( "1 or 2" );
  out = QgsProcessingUtils::convertToCompatibleFormat( layer, false, QStringLiteral( "test" ), QStringList() << "shp", QString( "shp" ), context, &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".shp" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );
  t = qgis::make_unique< QgsVectorLayer >( out, "vl2" );
  QCOMPARE( t->featureCount(), layer->featureCount() );
  layer->setSubsetString( QString() );

  // non-OGR source -- must be translated, regardless of extension. (e.g. delimited text provider handles CSV very different to OGR!)
  std::unique_ptr< QgsVectorLayer > memLayer = qgis::make_unique< QgsVectorLayer> ( "Point", "v1", "memory" );
  for ( int i = 1; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setGeometry( QgsGeometry( new QgsPoint( 1, 2 ) ) );
    memLayer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }
  out = QgsProcessingUtils::convertToCompatibleFormat( memLayer.get(), false, QStringLiteral( "test" ), QStringList() << "shp", QString( "shp" ), context, &feedback );
  QVERIFY( out != memLayer->source() );
  QVERIFY( out.endsWith( ".shp" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );
  t = qgis::make_unique< QgsVectorLayer >( out, "vl2" );
  QCOMPARE( t->featureCount(), memLayer->featureCount() );

  //delimited text -- must be translated, regardless of extension. (delimited text provider handles CSV very different to OGR!)
  QString csvPath = "file://" + testDataDir + "delimitedtext/testpt.csv?type=csv&useHeader=No&detectTypes=yes&xyDms=yes&geomType=none&subsetIndex=no&watchFile=no";
  std::unique_ptr< QgsVectorLayer > csvLayer = qgis::make_unique< QgsVectorLayer >( csvPath, "vl", "delimitedtext" );
  QVERIFY( csvLayer->isValid() );
  out = QgsProcessingUtils::convertToCompatibleFormat( csvLayer.get(), false, QStringLiteral( "test" ), QStringList() << "csv", QString( "csv" ), context, &feedback );
  QVERIFY( out != csvLayer->source() );
  QVERIFY( out.endsWith( ".csv" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );
  t = qgis::make_unique< QgsVectorLayer >( out, "vl2" );
  QCOMPARE( t->featureCount(), csvLayer->featureCount() );

  // geopackage with layer
  QString gpkgPath = testDataDir + "points_gpkg.gpkg|layername=points_gpkg";
  std::unique_ptr< QgsVectorLayer > gpkgLayer = qgis::make_unique< QgsVectorLayer >( gpkgPath, "vl" );
  QVERIFY( gpkgLayer->isValid() );
  out = QgsProcessingUtils::convertToCompatibleFormat( gpkgLayer.get(), false, QStringLiteral( "test" ), QStringList() << "gpkg" << "shp", QString( "shp" ), context, &feedback );
  // layer should be returned unchanged - underlying source is compatible
  QCOMPARE( out, gpkgLayer->source() );
  gpkgPath = testDataDir + "points_gpkg.gpkg|layername=points_small";
  gpkgLayer = qgis::make_unique< QgsVectorLayer >( gpkgPath, "vl" );
  QVERIFY( gpkgLayer->isValid() );
  out = QgsProcessingUtils::convertToCompatibleFormat( gpkgLayer.get(), false, QStringLiteral( "test" ), QStringList() << "gpkg" << "shp", QString( "shp" ), context, &feedback );
  QCOMPARE( out, gpkgLayer->source() );

  // also test evaluating parameter to compatible format
  std::unique_ptr< QgsProcessingParameterDefinition > def( new QgsProcessingParameterFeatureSource( QStringLiteral( "source" ) ) );
  QVariantMap params;
  params.insert( QStringLiteral( "source" ), QgsProcessingFeatureSourceDefinition( layer->id(), false ) );
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "shp", QString( "shp" ), &feedback );
  QCOMPARE( out, layer->source() );

  // incompatible format, will be converted
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "tab", QString( "tab" ), &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".tab" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );

  // layer as input
  params.insert( QStringLiteral( "source" ), QVariant::fromValue( layer ) );
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "shp", QString( "shp" ), &feedback );
  QCOMPARE( out, layer->source() );

  // incompatible format, will be converted
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "tab", QString( "tab" ), &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".tab" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );

  // selected only, will force export
  params.insert( QStringLiteral( "source" ), QgsProcessingFeatureSourceDefinition( layer->id(), true ) );
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "shp", QString( "shp" ), &feedback );
  QVERIFY( out != layer->source() );
  QVERIFY( out.endsWith( ".shp" ) );
  QVERIFY( out.startsWith( QgsProcessingUtils::tempFolder() ) );

  // vector layer as default
  def.reset( new QgsProcessingParameterFeatureSource( QStringLiteral( "source" ), QString(), QList<int>(), QVariant::fromValue( layer ) ) );
  params.remove( QStringLiteral( "source" ) );
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "shp", QString( "shp" ), &feedback );
  QCOMPARE( out, layer->source() );

  // output layer as input - e.g. from a previous model child
  params.insert( QStringLiteral( "source" ), QgsProcessingOutputLayerDefinition( layer->id() ) );
  out = QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( def.get(), params, context, QStringList() << "shp", QString( "shp" ), &feedback );
  QCOMPARE( out, layer->source() );
}

void TestQgsProcessing::create()
{
  DummyAlgorithm alg( QStringLiteral( "test" ) );
  DummyProvider p( QStringLiteral( "test_provider" ) );
  alg.setProvider( &p );

  std::unique_ptr< QgsProcessingAlgorithm > newInstance( alg.create() );
  QVERIFY( newInstance.get() );
  QCOMPARE( newInstance->provider(), &p );
}

void TestQgsProcessing::combineFields()
{
  QgsFields a;
  QgsFields b;
  // combine empty fields
  QgsFields res = QgsProcessingUtils::combineFields( a, b );
  QVERIFY( res.isEmpty() );

  // fields in a
  a.append( QgsField( "name" ) );
  res = QgsProcessingUtils::combineFields( a, b );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "name" ) );
  b.append( QgsField( "name" ) );
  res = QgsProcessingUtils::combineFields( a, b );
  QCOMPARE( res.count(), 2 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "name" ) );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "name_2" ) );

  a.append( QgsField( "NEW" ) );
  res = QgsProcessingUtils::combineFields( a, b );
  QCOMPARE( res.count(), 3 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "name" ) );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "NEW" ) );
  QCOMPARE( res.at( 2 ).name(), QStringLiteral( "name_2" ) );

  b.append( QgsField( "new" ) );
  res = QgsProcessingUtils::combineFields( a, b );
  QCOMPARE( res.count(), 4 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "name" ) );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "NEW" ) );
  QCOMPARE( res.at( 2 ).name(), QStringLiteral( "name_2" ) );
  QCOMPARE( res.at( 3 ).name(), QStringLiteral( "new_2" ) );
}

void TestQgsProcessing::fieldNamesToIndices()
{
  QgsFields fields;
  fields.append( QgsField( "name" ) );
  fields.append( QgsField( "address" ) );
  fields.append( QgsField( "age" ) );

  QList<int> indices1 = QgsProcessingUtils::fieldNamesToIndices( QStringList(), fields );
  QCOMPARE( indices1, QList<int>() << 0 << 1 << 2 );

  QList<int> indices2 = QgsProcessingUtils::fieldNamesToIndices( QStringList() << "address" << "age", fields );
  QCOMPARE( indices2, QList<int>() << 1 << 2 );

  QList<int> indices3 = QgsProcessingUtils::fieldNamesToIndices( QStringList() << "address" << "agegege", fields );
  QCOMPARE( indices3, QList<int>() << 1 );
}

void TestQgsProcessing::indicesToFields()
{
  QgsFields fields;
  fields.append( QgsField( "name" ) );
  fields.append( QgsField( "address" ) );
  fields.append( QgsField( "age" ) );

  QList<int> indices1 = QList<int>() << 0 << 1 << 2;
  QgsFields fields1 = QgsProcessingUtils::indicesToFields( indices1, fields );
  QCOMPARE( fields1, fields );

  QList<int> indices2 = QList<int>() << 1;
  QgsFields fields2expected;
  fields2expected.append( QgsField( "address" ) );
  QgsFields fields2 = QgsProcessingUtils::indicesToFields( indices2, fields );
  QCOMPARE( fields2, fields2expected );

  QList<int> indices3;
  QgsFields fields3 = QgsProcessingUtils::indicesToFields( indices3, fields );
  QCOMPARE( fields3, QgsFields() );
}

void TestQgsProcessing::stringToPythonLiteral()
{
  QCOMPARE( QgsProcessingUtils::stringToPythonLiteral( QStringLiteral( "a" ) ), QStringLiteral( "'a'" ) );
  QCOMPARE( QgsProcessingUtils::stringToPythonLiteral( QString() ), QStringLiteral( "''" ) );
  QCOMPARE( QgsProcessingUtils::stringToPythonLiteral( QStringLiteral( "a 'string'" ) ), QStringLiteral( "'a \\'string\\''" ) );
  QCOMPARE( QgsProcessingUtils::stringToPythonLiteral( QStringLiteral( "a \"string\"" ) ), QStringLiteral( "'a \\\"string\\\"'" ) );
  QCOMPARE( QgsProcessingUtils::stringToPythonLiteral( QStringLiteral( "a \n str\tin\\g" ) ), QStringLiteral( "'a \\n str\\tin\\\\g'" ) );
}

void TestQgsProcessing::defaultExtensionsForProvider()
{
  DummyProvider3 provider;
  // default implementation should return first supported format for provider
  QCOMPARE( provider.defaultVectorFileExtension( true ), QStringLiteral( "mif" ) );
  QCOMPARE( provider.defaultRasterFileExtension(), QStringLiteral( "mig" ) );
  // unless the user has set a default format, which IS supported by that provider
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Processing/DefaultOutputVectorLayerExt" ), QStringLiteral( "tab" ), QgsSettings::Core );
  settings.setValue( QStringLiteral( "Processing/DefaultOutputRasterLayerExt" ), QStringLiteral( "asc" ), QgsSettings::Core );
  QCOMPARE( provider.defaultVectorFileExtension( true ), QStringLiteral( "tab" ) );
  QCOMPARE( provider.defaultRasterFileExtension(), QStringLiteral( "asc" ) );

  // but if default is not supported by provider, we use a supported format
  settings.setValue( QStringLiteral( "Processing/DefaultOutputVectorLayerExt" ), QStringLiteral( "gpkg" ), QgsSettings::Core );
  settings.setValue( QStringLiteral( "Processing/DefaultOutputRasterLayerExt" ), QStringLiteral( "ecw" ), QgsSettings::Core );
  QCOMPARE( provider.defaultVectorFileExtension( true ), QStringLiteral( "mif" ) );
  QCOMPARE( provider.defaultRasterFileExtension(), QStringLiteral( "mig" ) );
}

void TestQgsProcessing::supportedExtensions()
{
  DummyProvider4 provider;
  QCOMPARE( provider.supportedOutputVectorLayerExtensions().count(), 1 );
  QCOMPARE( provider.supportedOutputVectorLayerExtensions().at( 0 ), QStringLiteral( "mif" ) );

  // if supportedOutputTableExtensions is not implemented, supportedOutputVectorLayerExtensions should be used instead
  QCOMPARE( provider.supportedOutputTableExtensions().count(), 1 );
  QCOMPARE( provider.supportedOutputTableExtensions().at( 0 ), QStringLiteral( "mif" ) );
}

void TestQgsProcessing::supportsNonFileBasedOutput()
{
  DummyAlgorithm alg( QStringLiteral( "test" ) );
  DummyProvider p( QStringLiteral( "test_provider" ) );
  alg.addDestParams();
  // provider has no support for file based outputs, so both output parameters should deny support
  alg.setProvider( &p );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 0 ) )->supportsNonFileBasedOutput() );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 1 ) )->supportsNonFileBasedOutput() );

  DummyAlgorithm alg2( QStringLiteral( "test" ) );
  DummyProvider p2( QStringLiteral( "test_provider" ) );
  p2.supportsNonFileOutputs = true;
  alg2.addDestParams();
  // provider has support for file based outputs, but only first output parameter should indicate support (since the second has support explicitly denied)
  alg2.setProvider( &p2 );
  QVERIFY( static_cast< const QgsProcessingDestinationParameter * >( alg2.destinationParameterDefinitions().at( 0 ) )->supportsNonFileBasedOutput() );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg2.destinationParameterDefinitions().at( 1 ) )->supportsNonFileBasedOutput() );
}

void TestQgsProcessing::addParameterType()
{
  QgsProcessingRegistry reg;
  QSignalSpy spy( &reg, &QgsProcessingRegistry::parameterTypeAdded );
  DummyParameterType *dpt = new DummyParameterType();
  QVERIFY( reg.addParameterType( dpt ) );
  QCOMPARE( spy.count(), 1 );
  QVERIFY( !reg.addParameterType( dpt ) );
  QCOMPARE( spy.count(), 1 );
  QVERIFY( !reg.addParameterType( new DummyParameterType() ) );
  QCOMPARE( spy.count(), 1 );
}

void TestQgsProcessing::removeParameterType()
{
  QgsProcessingRegistry reg;

  auto paramType = new DummyParameterType();

  reg.addParameterType( paramType );
  QSignalSpy spy( &reg, &QgsProcessingRegistry::parameterTypeRemoved );
  reg.removeParameterType( paramType );
  QCOMPARE( spy.count(), 1 );
}

void TestQgsProcessing::parameterTypes()
{
  QgsProcessingRegistry reg;
  int coreParamCount = reg.parameterTypes().count();
  QVERIFY( coreParamCount > 5 );

  auto paramType = new DummyParameterType();

  reg.addParameterType( paramType );
  QCOMPARE( reg.parameterTypes().count(), coreParamCount + 1 );
  QVERIFY( reg.parameterTypes().contains( paramType ) );
}

void TestQgsProcessing::parameterType()
{
  QgsProcessingRegistry reg;

  QVERIFY( reg.parameterType( QStringLiteral( "string" ) ) );
  QVERIFY( !reg.parameterType( QStringLiteral( "borken" ) ) );

  auto paramType = new DummyParameterType();

  reg.addParameterType( paramType );
  QCOMPARE( reg.parameterType( QStringLiteral( "paramType" ) ), paramType );
}

QGSTEST_MAIN( TestQgsProcessing )
#include "testqgsprocessing.moc"
