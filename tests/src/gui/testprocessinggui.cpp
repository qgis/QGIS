/***************************************************************************
                         testprocesinggui.cpp
                         ---------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"

#include "mesh/qgsmeshdataprovider.h"
#include "mesh/qgsmeshlayer.h"
#include "models/qgsprocessingmodelalgorithm.h"
#include "processing/models/qgsprocessingmodelalgorithm.h"
#include "processing/models/qgsprocessingmodelgroupbox.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsannotationlayer.h"
#include "qgsauthconfigselect.h"
#include "qgsauthmanager.h"
#include "qgscolorbutton.h"
#include "qgscoordinateoperationwidget.h"
#include "qgsdatabaseschemacombobox.h"
#include "qgsdatabasetablecombobox.h"
#include "qgsdatetimeedit.h"
#include "qgsdoublespinbox.h"
#include "qgsexpressionlineedit.h"
#include "qgsextentwidget.h"
#include "qgsfieldcombobox.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsfilewidget.h"
#include "qgsgeometrywidget.h"
#include "qgsgui.h"
#include "qgslayoutcombobox.h"
#include "qgslayoutitemcombobox.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutmanager.h"
#include "qgsmapcanvas.h"
#include "qgsmapthemecollection.h"
#include "qgsmemoryproviderutils.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsmessagebar.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelundocommand.h"
#include "qgsnativealgorithms.h"
#include "qgspluginlayer.h"
#include "qgspointcloudattributecombobox.h"
#include "qgspointcloudlayer.h"
#include "qgsprintlayout.h"
#include "qgsprocessingaggregatewidgetwrapper.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingalignrasterlayerswidgetwrapper.h"
#include "qgsprocessingdxflayerswidgetwrapper.h"
#include "qgsprocessingfeaturesourceoptionswidget.h"
#include "qgsprocessingfieldmapwidgetwrapper.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingmaplayercombobox.h"
#include "qgsprocessingmatrixparameterdialog.h"
#include "qgsprocessingmeshdatasetwidget.h"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsprocessingoutputdestinationwidget.h"
#include "qgsprocessingparameteraggregate.h"
#include "qgsprocessingparameteralignrasterlayers.h"
#include "qgsprocessingparameterdefinitionwidget.h"
#include "qgsprocessingparameterdxflayers.h"
#include "qgsprocessingparameterfieldmap.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertininputlayers.h"
#include "qgsprocessingpointcloudexpressionlineedit.h"
#include "qgsprocessingrasteroptionswidgetwrapper.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingtininputlayerswidget.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsproject.h"
#include "qgsprojectionselectionwidget.h"
#include "qgspropertyoverridebutton.h"
#include "qgsproviderconnectioncombobox.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsrasterbandcombobox.h"
#include "qgsrasterformatsaveoptionswidget.h"
#include "qgsscalewidget.h"
#include "qgssettings.h"
#include "qgsspinbox.h"
#include "qgstest.h"
#include "qgstiledscenelayer.h"
#include "qgsxmlutils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QToolButton>

class TestParamDefinition : public QgsProcessingParameterDefinition
{
  public:
    TestParamDefinition( const QString &type, const QString &name, const QVariant &defaultValue = QVariant() )
      : QgsProcessingParameterDefinition( name, name, defaultValue )
      , mType( type )
    {}
    QString mType;

    QgsProcessingParameterDefinition *clone() const override
    {
      return new TestParamDefinition( mType, name() );
    }

    QString type() const override { return mType; }
    QString valueAsPythonString( const QVariant &, QgsProcessingContext & ) const override { return QString(); }
    QString asScriptCode() const override { return QString(); }
};


class TestParameterType : public QgsProcessingParameterType
{
    // QgsProcessingParameterType interface
  public:
    TestParameterType( const QString &type )
      : mType( type )
    {}
    QString mType;

    QgsProcessingParameterDefinition *create( const QString &name ) const override
    {
      return new QgsProcessingParameterString( name );
    }

    QString description() const override
    {
      return u"Dummy Parameter Description"_s;
    }

    QString name() const override
    {
      return u"Dummy Parameter Type"_s;
    }

    QString id() const override
    {
      return mType;
    }

    QStringList acceptedParameterTypes() const override
    {
      return QStringList();
    }
    QStringList acceptedOutputTypes() const override
    {
      return QStringList();
    }
};


class TestWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper // clazy:exclude=missing-qobject-macro
{
  public:
    TestWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr, Qgis::ProcessingMode type = Qgis::ProcessingMode::Standard )
      : QgsAbstractProcessingParameterWidgetWrapper( parameter, type )
    {}

    QWidget *createWidget() override
    {
      return nullptr;
    }

    QLabel *createLabel() override
    {
      return nullptr;
    }

    void setWidgetValue( const QVariant &, QgsProcessingContext & ) override
    {
    }

    QVariant widgetValue() const override
    {
      return QVariant();
    }
};

class TestWidgetFactory : public QgsProcessingParameterWidgetFactoryInterface
{
  public:
    TestWidgetFactory( const QString &type )
      : type( type )
    {}

    QString type;

    QString parameterType() const override
    {
      return type;
    }

    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, Qgis::ProcessingMode type ) override
    {
      return new TestWidgetWrapper( parameter, type );
    }
};

class DummyPluginLayer : public QgsPluginLayer
{
    Q_OBJECT
  public:
    DummyPluginLayer( const QString &layerType, const QString &layerName )
      : QgsPluginLayer( layerType, layerName )
    {
      mValid = true;
    };

    DummyPluginLayer *clone() const override { return new DummyPluginLayer( "dummylayer", "test" ); };

    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override
    {
      Q_UNUSED( rendererContext );
      return nullptr;
    };

    bool writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const override
    {
      Q_UNUSED( layerNode );
      Q_UNUSED( doc );
      Q_UNUSED( context );
      return true;
    };
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override
    {
      Q_UNUSED( node );
      Q_UNUSED( errorMessage );
      Q_UNUSED( context );
      Q_UNUSED( categories );
      return true;
    };
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const override
    {
      Q_UNUSED( node );
      Q_UNUSED( doc );
      Q_UNUSED( errorMessage );
      Q_UNUSED( context );
      Q_UNUSED( categories );
      return true;
    };

    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override { Q_UNUSED( transformContext ); };
};

class TestProcessingGui : public QObject
{
    Q_OBJECT
  public:
    TestProcessingGui() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testModelUndo();
    void testSetGetConfig();
    void testFilterAlgorithmConfig();
    void testWrapperFactoryRegistry();
    void testWrapperGeneral();
    void testWrapperDynamic();
    void testModelerWrapper();
    void testHiddenWrapper();
    void testBooleanWrapper();
    void testStringWrapper();
    void testFileWrapper();
    void testAuthCfgWrapper();
    void testCrsWrapper();
    void testNumericWrapperDouble();
    void testNumericWrapperInt();
    void testDistanceWrapper();
    void testAreaWrapper();
    void testVolumeWrapper();
    void testDurationWrapper();
    void testScaleWrapper();
    void testRangeWrapper();
    void testMatrixDialog();
    void testMatrixWrapper();
    void testExpressionWrapper();
    void testFieldSelectionPanel();
    void testFieldWrapper();
    void testMultipleSelectionDialog();
    void testMultipleFileSelectionDialog();
    void testRasterBandSelectionPanel();
    void testBandWrapper();
    void testMultipleInputWrapper();
    void testEnumSelectionPanel();
    void testEnumCheckboxPanel();
    void testEnumWrapper();
    void testLayoutWrapper();
    void testLayoutItemWrapper();
    void testPointPanel();
    void testPointWrapper();
    void testGeometryWrapper();
    void testExtentWrapper();
    void testColorWrapper();
    void testCoordinateOperationWrapper();
    void mapLayerComboBox();
    void testMapLayerWrapper();
    void testRasterLayerWrapper();
    void testVectorLayerWrapper();
    void testFeatureSourceWrapper();
    void testMeshLayerWrapper();
    void paramConfigWidget();
    void testMapThemeWrapper();
    void testDateTimeWrapper();
    void testProviderConnectionWrapper();
    void testDatabaseSchemaWrapper();
    void testDatabaseTableWrapper();
    void testPointCloudLayerWrapper();
    void testAnnotationLayerWrapper();
    void testPointCloudAttributeWrapper();
    void testFieldMapWidget();
    void testFieldMapWrapper();
    void testAggregateWidget();
    void testAggregateWrapper();
    void testOutputDefinitionWidget();
    void testOutputDefinitionWidgetVectorOut();
    void testOutputDefinitionWidgetRasterOut();
    void testOutputDefinitionWidgetPointCloudOut();
    void testOutputDefinitionWidgetFolder();
    void testOutputDefinitionWidgetFileOut();
    void testFeatureSourceOptionsWidget();
    void testVectorOutWrapper();
    void testSinkWrapper();
    void testRasterOutWrapper();
    void testFileOutWrapper();
    void testFolderOutWrapper();
    void testTinInputLayerWrapper();
    void testDxfLayersWrapper();
    void testAlignRasterLayersWrapper();
    void testRasterOptionsWrapper();
    void testMeshDatasetWrapperLayerInProject();
    void testMeshDatasetWrapperLayerOutsideProject();
    void testModelGraphicsView();

  private:
    QString mTempDir;
    const char *mPass = "pass";

    void cleanupTempDir();
};

void TestProcessingGui::initTestCase()
{
  mTempDir = QDir::tempPath() + "/auth_proc";
  // setup a temporary auth db:
  cleanupTempDir();

  // make QGIS_AUTH_DB_DIR_PATH temp dir for qgis - auth.db and master password file
  QDir tmpDir = QDir::temp();
  QVERIFY2( tmpDir.mkpath( mTempDir ), "Couldn't make temp directory" );
  qputenv( "QGIS_AUTH_DB_DIR_PATH", mTempDir.toLatin1() );

  // init app and auth manager
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsApplication::authManager()->isDisabled(), "Authentication system is DISABLED" );

  // verify QGIS_AUTH_DB_DIR_PATH (temp auth db path) worked
  Q_NOWARN_DEPRECATED_PUSH
  QString db1( QFileInfo( QgsApplication::authManager()->authenticationDatabasePath() ).canonicalFilePath() );
  Q_NOWARN_DEPRECATED_POP
  QString db2( QFileInfo( mTempDir + "/qgis-auth.db" ).canonicalFilePath() );
  QVERIFY2( db1 == db2, "Auth db temp path does not match db path of manager" );

  // verify master pass can be set manually
  // (this also creates a fresh password hash in the new temp database)
  QVERIFY2( QgsApplication::authManager()->setMasterPassword( mPass, true ), "Master password could not be set" );
  QVERIFY2( QgsApplication::authManager()->masterPasswordIsSet(), "Auth master password not set from passed string" );

  // create QGIS_AUTH_PASSWORD_FILE file
  QString passfilepath = mTempDir + "/passfile";
  QFile passfile( passfilepath );
  if ( passfile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QTextStream fout( &passfile );
    fout << QString( mPass ) << "\r\n";
    passfile.close();
    qputenv( "QGIS_AUTH_PASSWORD_FILE", passfilepath.toLatin1() );
  }

  // re-init app and auth manager
  QgsApplication::quit();
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsApplication::authManager()->isDisabled(), "Authentication system is DISABLED" );

  // verify QGIS_AUTH_PASSWORD_FILE worked, when compared against hash in db
  QVERIFY2( QgsApplication::authManager()->masterPasswordIsSet(), "Auth master password not set from QGIS_AUTH_PASSWORD_FILE" );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestProcessingGui::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestProcessingGui::init()
{
}

void TestProcessingGui::cleanup()
{
  QgsProject::instance()->removeAllMapLayers();
}

void TestProcessingGui::testModelUndo()
{
  // make a little model
  QgsProcessingModelAlgorithm model( u"test"_s, u"testGroup"_s );
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1( "native:buffer" );
  a1.setDescription( u"alg1"_s );
  a1.setChildId( u"alg1"_s );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( u"alg2"_s );
  a2.setChildId( u"alg2"_s );
  QgsProcessingModelChildAlgorithm a3( u"native:buffer"_s );
  a3.setDescription( u"alg3"_s );
  a3.setChildId( u"alg3"_s );
  algs.insert( u"alg1"_s, a1 );
  algs.insert( u"alg2"_s, a2 );
  algs.insert( u"alg3"_s, a3 );
  model.setChildAlgorithms( algs );

  QgsModelUndoCommand command( &model, u"undo"_s );
  model.childAlgorithm( u"alg1"_s ).setDescription( u"new desc"_s );
  command.saveAfterState();

  QCOMPARE( model.childAlgorithm( u"alg1"_s ).description(), u"new desc"_s );
  command.undo();
  QCOMPARE( model.childAlgorithm( u"alg1"_s ).description(), u"alg1"_s );

  // first redo should have no effect -- we ignore it, since it's automatically triggered when the
  // command is added to the stack (yet we apply the initial change to the models outside of the undo stack)
  command.redo();
  QCOMPARE( model.childAlgorithm( u"alg1"_s ).description(), u"alg1"_s );
  command.redo();
  QCOMPARE( model.childAlgorithm( u"alg1"_s ).description(), u"new desc"_s );

  // the last used parameter values setting should not be affected by undo stack changes
  QVariantMap params;
  params.insert( u"a"_s, 1 );
  model.setDesignerParameterValues( params );
  command.undo();
  QCOMPARE( model.designerParameterValues(), params );
  command.redo();
  QCOMPARE( model.designerParameterValues(), params );
}

void TestProcessingGui::testSetGetConfig()
{
  const QList<const QgsProcessingAlgorithm *> algorithms = QgsApplication::processingRegistry()->algorithms();

  // Find all defined widgets for native algorithms
  // and get the default configuration (that is, we create a widget
  // and get the configuration it returns without being modified in any way)
  // We then set and get this configuration and validate that it matches the original one.
  for ( const QgsProcessingAlgorithm *algorithm : algorithms )
  {
    std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );
    if ( configWidget )
    {
      QCOMPARE( configWidget->algorithm(), algorithm );
      const QVariantMap defaultConfig = configWidget->configuration();
      configWidget->setConfiguration( defaultConfig );
      const QVariantMap defaultControlConfig = configWidget->configuration();
      QDomDocument defaultConfigDoc;
      QDomDocument defaultConfigControlDoc;
      QgsXmlUtils::writeVariant( defaultConfig, defaultConfigDoc );
      QgsXmlUtils::writeVariant( defaultControlConfig, defaultConfigControlDoc );
      QCOMPARE( defaultConfigDoc.toString(), defaultConfigControlDoc.toString() );
    }
  }
}

void TestProcessingGui::testFilterAlgorithmConfig()
{
  const QgsProcessingAlgorithm *algorithm = QgsApplication::processingRegistry()->algorithmById( u"native:filter"_s );
  std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );

  QVariantMap config;
  QVariantList outputs;
  QVariantMap output;
  output.insert( u"name"_s, u"test"_s );
  output.insert( u"expression"_s, u"I am an expression"_s );
  output.insert( u"isModelOutput"_s, true );
  outputs.append( output );
  config.insert( u"outputs"_s, outputs );
  configWidget->setConfiguration( config );

  QVariantMap configControl = configWidget->configuration();

  QDomDocument configDoc;
  QDomDocument configControlDoc;
  QgsXmlUtils::writeVariant( config, configDoc );
  QgsXmlUtils::writeVariant( configControl, configControlDoc );
  QCOMPARE( configDoc.toString(), configControlDoc.toString() );
}

void TestProcessingGui::testWrapperFactoryRegistry()
{
  QgsProcessingGuiRegistry guiRegistry;

  TestParamDefinition numParam( u"num"_s, u"num"_s );
  TestParamDefinition stringParam( u"str"_s, u"str"_s );

  TestParameterType *numParamType = new TestParameterType( u"num"_s );
  TestParameterType *stringParamType = new TestParameterType( u"str"_s );

  QVERIFY( !guiRegistry.createParameterWidgetWrapper( &numParam, Qgis::ProcessingMode::Standard ) );

  TestWidgetFactory *factory = new TestWidgetFactory( u"str"_s );
  QVERIFY( guiRegistry.addParameterWidgetFactory( factory ) );

  // duplicate type not allowed
  TestWidgetFactory *factory2 = new TestWidgetFactory( u"str"_s );
  QVERIFY( !guiRegistry.addParameterWidgetFactory( factory2 ) );
  delete factory2;

  // Register parameter type for createParameterWidgetWrapper
  QVERIFY( QgsApplication::processingRegistry()->addParameterType( numParamType ) );
  QVERIFY( QgsApplication::processingRegistry()->addParameterType( stringParamType ) );

  QgsAbstractProcessingParameterWidgetWrapper *wrapper = guiRegistry.createParameterWidgetWrapper( &numParam, Qgis::ProcessingMode::Standard );
  QVERIFY( !wrapper );
  wrapper = guiRegistry.createParameterWidgetWrapper( &stringParam, Qgis::ProcessingMode::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), u"str"_s );
  delete wrapper;

  TestWidgetFactory *factory3 = new TestWidgetFactory( u"num"_s );
  QVERIFY( guiRegistry.addParameterWidgetFactory( factory3 ) );

  wrapper = guiRegistry.createParameterWidgetWrapper( &numParam, Qgis::ProcessingMode::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), u"num"_s );
  delete wrapper;

  // creating wrapper using metadata
  TestParamDefinition customParam( u"custom"_s, u"custom"_s );
  wrapper = guiRegistry.createParameterWidgetWrapper( &customParam, Qgis::ProcessingMode::Standard );
  QVERIFY( !wrapper );
  customParam.setMetadata( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"str"_s } } ) }
  } );
  wrapper = guiRegistry.createParameterWidgetWrapper( &customParam, Qgis::ProcessingMode::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), u"custom"_s );
  delete wrapper;

  // removing
  guiRegistry.removeParameterWidgetFactory( nullptr );
  TestWidgetFactory *factory4 = new TestWidgetFactory( u"xxxx"_s );
  guiRegistry.removeParameterWidgetFactory( factory4 );
  guiRegistry.removeParameterWidgetFactory( factory );
  QgsApplication::processingRegistry()->removeParameterType( numParamType );
  QgsApplication::processingRegistry()->removeParameterType( stringParamType );

  wrapper = guiRegistry.createParameterWidgetWrapper( &stringParam, Qgis::ProcessingMode::Standard );
  QVERIFY( !wrapper );

  wrapper = guiRegistry.createParameterWidgetWrapper( &numParam, Qgis::ProcessingMode::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), u"num"_s );
  delete wrapper;
}

void TestProcessingGui::testWrapperGeneral()
{
  TestParamDefinition param( u"boolean"_s, u"bool"_s );
  param.setAdditionalExpressionContextVariables( QStringList() << u"a"_s << u"b"_s );
  QgsProcessingBooleanWidgetWrapper wrapper( &param );
  QCOMPARE( wrapper.type(), Qgis::ProcessingMode::Standard );

  QgsExpressionContext expContext = wrapper.createExpressionContext();
  QVERIFY( expContext.hasVariable( u"a"_s ) );
  QVERIFY( expContext.hasVariable( u"b"_s ) );
  QCOMPARE( expContext.highlightedVariables(), QStringList() << u"a"_s << u"b"_s );

  QgsProcessingBooleanWidgetWrapper wrapper2( &param, Qgis::ProcessingMode::Batch );
  QCOMPARE( wrapper2.type(), Qgis::ProcessingMode::Batch );
  QCOMPARE( wrapper2.parameterDefinition()->name(), u"bool"_s );

  QgsProcessingBooleanWidgetWrapper wrapperModeler( &param, Qgis::ProcessingMode::Modeler );
  QCOMPARE( wrapperModeler.type(), Qgis::ProcessingMode::Modeler );

  QgsProcessingContext context;
  QVERIFY( !wrapper2.wrappedWidget() );
  QWidget *w = wrapper2.createWrappedWidget( context );
  QVERIFY( w );
  QCOMPARE( wrapper2.wrappedWidget(), w );
  delete w;
  QVERIFY( !wrapper2.wrappedLabel() );
  QLabel *l = wrapper2.createWrappedLabel();
  QCOMPARE( wrapper2.wrappedLabel(), l );
  delete l;
  l = wrapperModeler.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( wrapperModeler.wrappedLabel(), l );
  delete l;

  // check that created widget starts with default value
  param = TestParamDefinition( u"boolean"_s, u"bool"_s, true );
  QgsProcessingBooleanWidgetWrapper trueDefault( &param );
  w = trueDefault.createWrappedWidget( context );
  QVERIFY( trueDefault.widgetValue().toBool() );
  delete w;
  param = TestParamDefinition( u"boolean"_s, u"bool"_s, false );
  QgsProcessingBooleanWidgetWrapper falseDefault( &param );
  w = falseDefault.createWrappedWidget( context );
  QVERIFY( !falseDefault.widgetValue().toBool() );
  delete w;

  auto mc = std::make_unique<QgsMapCanvas>();
  QgsProcessingParameterWidgetContext widgetContext;
  widgetContext.setMapCanvas( mc.get() );
  QCOMPARE( widgetContext.mapCanvas(), mc.get() );

  auto mb = std::make_unique<QgsMessageBar>();
  widgetContext.setMessageBar( mb.get() );
  QCOMPARE( widgetContext.messageBar(), mb.get() );

  QgsProject p;
  widgetContext.setProject( &p );
  QCOMPARE( widgetContext.project(), &p );
  auto model = std::make_unique<QgsProcessingModelAlgorithm>();
  widgetContext.setModel( model.get() );
  QCOMPARE( widgetContext.model(), model.get() );
  widgetContext.setModelChildAlgorithmId( u"xx"_s );
  QCOMPARE( widgetContext.modelChildAlgorithmId(), u"xx"_s );

  wrapper.setWidgetContext( widgetContext );
  QCOMPARE( wrapper.widgetContext().mapCanvas(), mc.get() );
  QCOMPARE( wrapper.widgetContext().messageBar(), mb.get() );
  QCOMPARE( wrapper.widgetContext().model(), model.get() );
  QCOMPARE( wrapper.widgetContext().modelChildAlgorithmId(), u"xx"_s );
}

class TestProcessingContextGenerator : public QgsProcessingContextGenerator
{
  public:
    TestProcessingContextGenerator( QgsProcessingContext &context )
      : mContext( context )
    {}

    QgsProcessingContext *processingContext() override
    {
      return &mContext;
    }

    QgsProcessingContext &mContext;
};

class TestLayerWrapper : public QgsAbstractProcessingParameterWidgetWrapper // clazy:exclude=missing-qobject-macro
{
  public:
    TestLayerWrapper( const QgsProcessingParameterDefinition *parameter = nullptr )
      : QgsAbstractProcessingParameterWidgetWrapper( parameter )
    {}
    QWidget *createWidget() override { return nullptr; }
    void setWidgetValue( const QVariant &val, QgsProcessingContext & ) override { v = val; }
    QVariant widgetValue() const override { return v; }

    QVariant v;
};

void TestProcessingGui::testWrapperDynamic()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( u"native:centroids"_s );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( u"INPUT"_s );
  const QgsProcessingParameterDefinition *allPartsDef = centroidAlg->parameterDefinition( u"ALL_PARTS"_s );

  QgsProcessingBooleanWidgetWrapper inputWrapper( layerDef, Qgis::ProcessingMode::Standard );
  QgsProcessingBooleanWidgetWrapper allPartsWrapper( allPartsDef, Qgis::ProcessingMode::Standard );

  QgsProcessingContext context;

  std::unique_ptr<QWidget> allPartsWidget( allPartsWrapper.createWrappedWidget( context ) );
  // dynamic parameter, so property button should be created
  QVERIFY( allPartsWrapper.mPropertyButton.data() != nullptr );

  std::unique_ptr<QWidget> inputWidget( inputWrapper.createWrappedWidget( context ) );
  // not dynamic parameter, so property button should be NOT created
  QVERIFY( inputWrapper.mPropertyButton.data() == nullptr );

  // set dynamic parameter to dynamic value
  allPartsWrapper.setParameterValue( QgsProperty::fromExpression( u"1+2"_s ), context );
  QCOMPARE( allPartsWrapper.parameterValue().value<QgsProperty>().expressionString(), u"1+2"_s );
  // not dynamic value
  allPartsWrapper.setParameterValue( true, context );
  QCOMPARE( allPartsWrapper.parameterValue().toBool(), true );
  allPartsWrapper.setParameterValue( false, context );
  QCOMPARE( allPartsWrapper.parameterValue().toBool(), false );

  // project layer
  QgsProject p;
  QgsVectorLayer *vl = new QgsVectorLayer( u"LineString"_s, u"x"_s, u"memory"_s );
  p.addMapLayer( vl );

  TestLayerWrapper layerWrapper( layerDef );

  QVERIFY( !allPartsWrapper.mPropertyButton->vectorLayer() );
  layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
  allPartsWrapper.setDynamicParentLayerParameter( &layerWrapper );
  QCOMPARE( allPartsWrapper.mPropertyButton->vectorLayer(), vl );
  // should not be owned by wrapper
  QVERIFY( !allPartsWrapper.mDynamicLayer.get() );
  layerWrapper.setWidgetValue( QVariant(), context );
  allPartsWrapper.setDynamicParentLayerParameter( &layerWrapper );
  QVERIFY( !allPartsWrapper.mPropertyButton->vectorLayer() );

  layerWrapper.setWidgetValue( vl->id(), context );
  allPartsWrapper.setDynamicParentLayerParameter( &layerWrapper );
  QVERIFY( !allPartsWrapper.mPropertyButton->vectorLayer() );
  QVERIFY( !allPartsWrapper.mDynamicLayer.get() );

  // with project layer
  context.setProject( &p );
  TestProcessingContextGenerator generator( context );
  allPartsWrapper.registerProcessingContextGenerator( &generator );

  layerWrapper.setWidgetValue( vl->id(), context );
  allPartsWrapper.setDynamicParentLayerParameter( &layerWrapper );
  QCOMPARE( allPartsWrapper.mPropertyButton->vectorLayer(), vl );
  QVERIFY( !allPartsWrapper.mDynamicLayer.get() );

  // non-project layer
  QString pointFileName = TEST_DATA_DIR + u"/points.shp"_s;
  layerWrapper.setWidgetValue( pointFileName, context );
  allPartsWrapper.setDynamicParentLayerParameter( &layerWrapper );
  QCOMPARE( allPartsWrapper.mPropertyButton->vectorLayer()->publicSource(), pointFileName );
  // must be owned by wrapper, or layer may be deleted while still required by wrapper
  QCOMPARE( allPartsWrapper.mDynamicLayer->publicSource(), pointFileName );
}

void TestProcessingGui::testModelerWrapper()
{
  // make a little model
  QgsProcessingModelAlgorithm model( u"test"_s, u"testGroup"_s );
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1( "native:buffer" );
  a1.setDescription( u"alg1"_s );
  a1.setChildId( u"alg1"_s );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( u"alg2"_s );
  a2.setChildId( u"alg2"_s );
  QgsProcessingModelChildAlgorithm a3( u"native:buffer"_s );
  a3.setDescription( u"alg3"_s );
  a3.setChildId( u"alg3"_s );
  QgsProcessingModelChildAlgorithm a4( u"native:package"_s );
  a4.setDescription( u"alg4"_s );
  a4.setChildId( u"alg4"_s );
  algs.insert( u"alg1"_s, a1 );
  algs.insert( u"alg2"_s, a2 );
  algs.insert( u"alg3"_s, a3 );
  algs.insert( u"alg4"_s, a4 );
  model.setChildAlgorithms( algs );

  QMap<QString, QgsProcessingModelParameter> pComponents;
  QgsProcessingModelParameter pc1;
  pc1.setParameterName( u"my_param"_s );
  pComponents.insert( u"my_param"_s, pc1 );
  model.setParameterComponents( pComponents );

  QgsProcessingModelParameter bool1( "p1" );
  model.addModelParameter( new QgsProcessingParameterBoolean( "p1", "desc" ), bool1 );
  QgsProcessingModelParameter testParam( "p2" );
  model.addModelParameter( new TestParamDefinition( "test_type", "p2" ), testParam );
  QgsProcessingModelParameter testDestParam( "p3" );
  model.addModelParameter( new QgsProcessingParameterFileDestination( "test_dest", "p3" ), testDestParam );
  QgsProcessingModelParameter testLayerParam( "p4" );
  model.addModelParameter( new QgsProcessingParameterMapLayer( "p4", "test_layer" ), testLayerParam );
  QgsProcessingModelParameter testOptionalLayerParam( "p5" );
  model.addModelParameter( new QgsProcessingParameterMapLayer( "p5", "test_layer2", QVariant(), true ), testLayerParam );

  // try to create a parameter widget, no factories registered
  QgsProcessingGuiRegistry guiRegistry;

  QgsProcessingContext context;
  QVERIFY( !guiRegistry.createModelerParameterWidget( &model, u"a"_s, model.parameterDefinition( "p2" ), context ) );

  // register parameter type for createModelerParameterWidget
  QgsApplication::processingRegistry()->addParameterType( new TestParameterType( "test_type" ) );

  // register factory
  TestWidgetFactory *factory = new TestWidgetFactory( u"test_type"_s );
  QVERIFY( guiRegistry.addParameterWidgetFactory( factory ) );
  QgsProcessingModelerParameterWidget *w = guiRegistry.createModelerParameterWidget( &model, u"a"_s, model.parameterDefinition( "p2" ), context );
  QVERIFY( w );
  delete w;

  w = guiRegistry.createModelerParameterWidget( &model, u"a"_s, model.parameterDefinition( "p1" ), context );
  QVERIFY( w );
  // should default to static value
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  delete w;

  w = guiRegistry.createModelerParameterWidget( &model, u"a"_s, model.parameterDefinition( "p4" ), context );
  QVERIFY( w );
  // a layer parameter should default to "model input" type
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  delete w;
  // but an optionl layer parameter should NOT -- we don't want to autofill values for optional layers by default
  w = guiRegistry.createModelerParameterWidget( &model, u"a"_s, model.parameterDefinition( "p5" ), context );
  QVERIFY( w );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  delete w;

  // widget tests
  w = new QgsProcessingModelerParameterWidget( &model, "alg1", model.parameterDefinition( "p1" ), context );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->parameterDefinition()->name(), u"p1"_s );
  QLabel *l = w->createLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"desc"_s );
  QCOMPARE( l->toolTip(), w->parameterDefinition()->toolTip() );
  delete l;

  // static value
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( true ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().staticValue().toBool(), true );
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( false ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().staticValue().toBool(), false );
  QCOMPARE( w->mStackedWidget->currentIndex(), 0 );
  QCOMPARE( w->mSourceButton->toolTip(), u"Value"_s );

  // expression value
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromExpression( u"1+2"_s ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::Expression );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().expression(), u"1+2"_s );
  QCOMPARE( w->mStackedWidget->currentIndex(), 1 );
  QCOMPARE( w->mSourceButton->toolTip(), u"Pre-calculated Value"_s );

  // model input - should fail, because we haven't populated sources yet, and so have no compatible sources
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromModelParameter( u"p1"_s ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QVERIFY( w->value().value<QgsProcessingModelChildParameterSource>().parameterName().isEmpty() );
  QCOMPARE( w->mStackedWidget->currentIndex(), 2 );
  QCOMPARE( w->mSourceButton->toolTip(), u"Model Input"_s );

  // alg output  - should fail, because we haven't populated sources yet, and so have no compatible sources
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg3"_s, u"OUTPUT"_s ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QVERIFY( w->value().value<QgsProcessingModelChildParameterSource>().outputChildId().isEmpty() );
  QCOMPARE( w->mStackedWidget->currentIndex(), 3 );
  QCOMPARE( w->mSourceButton->toolTip(), u"Algorithm Output"_s );

  // populate sources and re-try
  w->populateSources( QStringList() << u"boolean"_s, QStringList() << u"outputVector"_s, QList<int>() );

  // model input
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromModelParameter( u"p1"_s ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().parameterName(), u"p1"_s );


  // alg output
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg3"_s, u"OUTPUT"_s ) );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().outputChildId(), u"alg3"_s );
  QCOMPARE( w->value().value<QgsProcessingModelChildParameterSource>().outputName(), u"OUTPUT"_s );

  // model output
  delete w;
  w = new QgsProcessingModelerParameterWidget( &model, "alg1", model.parameterDefinition( "test_dest" ), context );
  QCOMPARE( w->parameterDefinition()->name(), u"test_dest"_s );
  // should default to being a model output for destination parameters, but with no value
  QVERIFY( w->isModelOutput() );
  QCOMPARE( w->modelOutputName(), QString() );
  // set it to something else
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg3"_s, u"OUTPUT"_s ) );
  QVERIFY( !w->isModelOutput() );
  // and back
  w->setToModelOutput( u"out"_s );
  QVERIFY( w->isModelOutput() );
  QCOMPARE( w->modelOutputName(), u"out"_s );
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg3"_s, u"OUTPUT"_s ) );
  w->setToModelOutput( QString() );
  QVERIFY( w->isModelOutput() );
  QCOMPARE( w->modelOutputName(), QString() );

  // multi-source input
  delete w;
  const QgsProcessingAlgorithm *packageAlg = QgsApplication::processingRegistry()->algorithmById( u"native:package"_s );
  const QgsProcessingParameterDefinition *layerDef = packageAlg->parameterDefinition( u"LAYERS"_s );

  w = new QgsProcessingModelerParameterWidget( &model, "alg4", layerDef, context );

  w->setWidgetValue( QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"alg3"_s, u"OUTPUT"_s ) << QgsProcessingModelChildParameterSource::fromModelParameter( u"p1"_s ) << QgsProcessingModelChildParameterSource::fromStaticValue( u"something"_s ) );
  QCOMPARE( w->value().toList().count(), 3 );

  QCOMPARE( w->value().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().outputChildId(), u"alg3"_s );
  QCOMPARE( w->value().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().outputName(), u"OUTPUT"_s );
  QCOMPARE( w->value().toList().at( 1 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( w->value().toList().at( 1 ).value<QgsProcessingModelChildParameterSource>().parameterName(), u"p1"_s );
  QCOMPARE( w->value().toList().at( 2 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().toList().at( 2 ).value<QgsProcessingModelChildParameterSource>().staticValue().toString(), u"something"_s );
  delete w;
}

void TestProcessingGui::testHiddenWrapper()
{
  TestParamDefinition param( u"boolean"_s, u"bool"_s );

  QgsProcessingHiddenWidgetWrapper wrapper( &param );
  QSignalSpy spy( &wrapper, &QgsProcessingHiddenWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  wrapper.setWidgetValue( 1, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toInt(), 1 );
  wrapper.setWidgetValue( 1, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toInt(), 1 );
  wrapper.setWidgetValue( 2, context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toInt(), 2 );

  QVERIFY( !wrapper.createWrappedWidget( context ) );
  QVERIFY( !wrapper.createWrappedLabel() );

  auto vl = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:3111&field=pk:int"_s, u"vl"_s, u"memory"_s );
  QVERIFY( !wrapper.linkedVectorLayer() );
  wrapper.setLinkedVectorLayer( vl.get() );
  QCOMPARE( wrapper.linkedVectorLayer(), vl.get() );
}

void TestProcessingGui::testBooleanWrapper()
{
  TestParamDefinition param( u"boolean"_s, u"bool"_s );

  // standard wrapper
  QgsProcessingBooleanWidgetWrapper wrapper( &param );
  QSignalSpy spy( &wrapper, &QgsProcessingBooleanWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );
  wrapper.setWidgetValue( true, context );
  QCOMPARE( spy.count(), 1 );
  QVERIFY( wrapper.widgetValue().toBool() );
  QVERIFY( static_cast<QCheckBox *>( wrapper.wrappedWidget() )->isChecked() );
  wrapper.setWidgetValue( false, context );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( !wrapper.widgetValue().toBool() );
  QVERIFY( !static_cast<QCheckBox *>( wrapper.wrappedWidget() )->isChecked() );

  // should be no label in standard mode
  QVERIFY( !wrapper.createWrappedLabel() );
  QCOMPARE( static_cast<QCheckBox *>( wrapper.wrappedWidget() )->text(), u"bool"_s );

  // check signal
  static_cast<QCheckBox *>( wrapper.wrappedWidget() )->setChecked( true );
  QCOMPARE( spy.count(), 3 );
  static_cast<QCheckBox *>( wrapper.wrappedWidget() )->setChecked( false );
  QCOMPARE( spy.count(), 4 );

  delete w;

  // batch wrapper
  QgsProcessingBooleanWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingBooleanWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( true, context );
  QCOMPARE( spy2.count(), 1 );
  QVERIFY( wrapperB.widgetValue().toBool() );
  QVERIFY( static_cast<QComboBox *>( wrapperB.wrappedWidget() )->currentData().toBool() );
  wrapperB.setWidgetValue( false, context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( !wrapperB.widgetValue().toBool() );
  QVERIFY( !static_cast<QComboBox *>( wrapperB.wrappedWidget() )->currentData().toBool() );

  // check signal
  static_cast<QComboBox *>( w )->setCurrentIndex( 0 );
  QCOMPARE( spy2.count(), 3 );
  static_cast<QComboBox *>( w )->setCurrentIndex( 1 );
  QCOMPARE( spy2.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingBooleanWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingBooleanWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( true, context );
  QVERIFY( wrapperM.widgetValue().toBool() );
  QCOMPARE( spy3.count(), 1 );
  QVERIFY( static_cast<QComboBox *>( wrapperM.wrappedWidget() )->currentData().toBool() );
  wrapperM.setWidgetValue( false, context );
  QVERIFY( !wrapperM.widgetValue().toBool() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( !static_cast<QComboBox *>( wrapperM.wrappedWidget() )->currentData().toBool() );

  // check signal
  static_cast<QComboBox *>( w )->setCurrentIndex( 0 );
  QCOMPARE( spy3.count(), 3 );
  static_cast<QComboBox *>( w )->setCurrentIndex( 1 );
  QCOMPARE( spy3.count(), 4 );

  // should be a label in modeler mode
  QLabel *l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"bool"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"boolean"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterBoolean boolParam( u"n"_s, u"test desc"_s, true, false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"boolean"_s, context, widgetContext, &boolParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( static_cast<QgsProcessingParameterBoolean *>( def.get() )->defaultValue().toBool() );
  boolParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  boolParam.setDefaultValue( false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"boolean"_s, context, widgetContext, &boolParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QVERIFY( !static_cast<QgsProcessingParameterBoolean *>( def.get() )->defaultValue().toBool() );
}

void TestProcessingGui::testStringWrapper()
{
  QgsProcessingParameterString param( u"string"_s, u"string"_s );

  // standard wrapper
  QgsProcessingStringWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( u"a"_s, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toString(), u"a"_s );
  QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"a"_s );
  wrapper.setWidgetValue( QString(), context );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( wrapper.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text().isEmpty() );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"string"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast<QLineEdit *>( wrapper.wrappedWidget() )->setText( u"b"_s );
  QCOMPARE( spy.count(), 3 );
  static_cast<QLineEdit *>( wrapper.wrappedWidget() )->clear();
  QCOMPARE( spy.count(), 4 );

  delete w;

  // batch wrapper
  QgsProcessingStringWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( u"a"_s, context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toString(), u"a"_s );
  QCOMPARE( static_cast<QLineEdit *>( wrapperB.wrappedWidget() )->text(), u"a"_s );
  wrapperB.setWidgetValue( QString(), context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( wrapperB.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast<QLineEdit *>( wrapperB.wrappedWidget() )->text().isEmpty() );

  // check signal
  static_cast<QLineEdit *>( w )->setText( u"x"_s );
  QCOMPARE( spy2.count(), 3 );
  static_cast<QLineEdit *>( w )->clear();
  QCOMPARE( spy2.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingStringWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( u"a"_s, context );
  QCOMPARE( wrapperM.widgetValue().toString(), u"a"_s );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( static_cast<QLineEdit *>( wrapperM.wrappedWidget() )->text(), u"a"_s );
  wrapperM.setWidgetValue( QString(), context );
  QVERIFY( wrapperM.widgetValue().toString().isEmpty() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( static_cast<QLineEdit *>( wrapperM.wrappedWidget() )->text().isEmpty() );

  // check signal
  static_cast<QLineEdit *>( w )->setText( u"x"_s );
  QCOMPARE( spy3.count(), 3 );
  static_cast<QLineEdit *>( w )->clear();
  QCOMPARE( spy3.count(), 4 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"string"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  //
  // multiline parameter
  //
  param = QgsProcessingParameterString( u"string"_s, u"string"_s, QVariant(), true );

  // standard wrapper
  QgsProcessingStringWidgetWrapper wrapperMultiLine( &param );

  w = wrapperMultiLine.createWrappedWidget( context );

  QSignalSpy spy4( &wrapperMultiLine, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperMultiLine.setWidgetValue( u"a"_s, context );
  QCOMPARE( spy4.count(), 1 );
  QCOMPARE( wrapperMultiLine.widgetValue().toString(), u"a"_s );
  QCOMPARE( static_cast<QPlainTextEdit *>( wrapperMultiLine.wrappedWidget() )->toPlainText(), u"a"_s );
  wrapperMultiLine.setWidgetValue( QString(), context );
  QCOMPARE( spy4.count(), 2 );
  QVERIFY( wrapperMultiLine.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast<QPlainTextEdit *>( wrapperMultiLine.wrappedWidget() )->toPlainText().isEmpty() );

  l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"string"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast<QPlainTextEdit *>( wrapperMultiLine.wrappedWidget() )->setPlainText( u"b"_s );
  QCOMPARE( spy4.count(), 3 );
  static_cast<QPlainTextEdit *>( wrapperMultiLine.wrappedWidget() )->clear();
  QCOMPARE( spy4.count(), 4 );

  delete w;

  // batch wrapper - should still be a line edit
  QgsProcessingStringWidgetWrapper wrapperMultiLineB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperMultiLineB.createWrappedWidget( context );
  QSignalSpy spy5( &wrapperMultiLineB, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperMultiLineB.setWidgetValue( u"a"_s, context );
  QCOMPARE( spy5.count(), 1 );
  QCOMPARE( wrapperMultiLineB.widgetValue().toString(), u"a"_s );
  QCOMPARE( static_cast<QLineEdit *>( wrapperMultiLineB.wrappedWidget() )->text(), u"a"_s );
  wrapperMultiLineB.setWidgetValue( QString(), context );
  QCOMPARE( spy5.count(), 2 );
  QVERIFY( wrapperMultiLineB.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast<QLineEdit *>( wrapperMultiLineB.wrappedWidget() )->text().isEmpty() );

  // check signal
  static_cast<QLineEdit *>( w )->setText( u"x"_s );
  QCOMPARE( spy5.count(), 3 );
  static_cast<QLineEdit *>( w )->clear();
  QCOMPARE( spy5.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingStringWidgetWrapper wrapperMultiLineM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperMultiLineM.createWrappedWidget( context );
  QSignalSpy spy6( &wrapperMultiLineM, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperMultiLineM.setWidgetValue( u"a"_s, context );
  QCOMPARE( wrapperMultiLineM.widgetValue().toString(), u"a"_s );
  QCOMPARE( spy6.count(), 1 );
  QCOMPARE( static_cast<QPlainTextEdit *>( wrapperMultiLineM.wrappedWidget() )->toPlainText(), u"a"_s );
  wrapperMultiLineM.setWidgetValue( QString(), context );
  QVERIFY( wrapperMultiLineM.widgetValue().toString().isEmpty() );
  QCOMPARE( spy6.count(), 2 );
  QVERIFY( static_cast<QPlainTextEdit *>( wrapperMultiLineM.wrappedWidget() )->toPlainText().isEmpty() );

  // check signal
  static_cast<QPlainTextEdit *>( w )->setPlainText( u"x"_s );
  QCOMPARE( spy6.count(), 3 );
  static_cast<QPlainTextEdit *>( w )->clear();
  QCOMPARE( spy6.count(), 4 );

  // should be a label in modeler mode
  l = wrapperMultiLineM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"string"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;


  //
  // with value hints
  //
  param = QgsProcessingParameterString( u"string"_s, u"string"_s, QVariant() );
  param.setMetadata( { { u"widget_wrapper"_s, QVariantMap( { { u"value_hints"_s, QStringList() << "value 1" << "value 2" << "value 3" } } ) }
  } );

  QgsProcessingStringWidgetWrapper wrapperHints( &param );

  w = wrapperHints.createWrappedWidget( context );

  QSignalSpy spy7( &wrapperHints, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperHints.setWidgetValue( u"value 2"_s, context );
  QCOMPARE( spy7.count(), 1 );
  QCOMPARE( wrapperHints.widgetValue().toString(), u"value 2"_s );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHints.wrappedWidget() )->currentText(), u"value 2"_s );
  wrapperHints.setWidgetValue( u"value 3"_s, context );
  QCOMPARE( spy7.count(), 2 );
  QCOMPARE( wrapperHints.widgetValue().toString(), u"value 3"_s );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHints.wrappedWidget() )->currentText(), u"value 3"_s );

  // set to value which is not present -- should fallback to first value
  wrapperHints.setWidgetValue( u"value 4"_s, context );
  QCOMPARE( spy7.count(), 3 );
  QCOMPARE( wrapperHints.widgetValue().toString(), u"value 1"_s );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHints.wrappedWidget() )->currentText(), u"value 1"_s );

  l = wrapperHints.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"string"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  qgis::down_cast<QComboBox *>( wrapperHints.wrappedWidget() )->setCurrentIndex( 1 );
  QCOMPARE( spy7.count(), 4 );
  qgis::down_cast<QComboBox *>( wrapperHints.wrappedWidget() )->setCurrentIndex( 2 );
  QCOMPARE( spy7.count(), 5 );

  delete w;

  // with value hints, optional param
  param = QgsProcessingParameterString( u"string"_s, u"string"_s, QVariant(), false, true );
  param.setMetadata( { { u"widget_wrapper"_s, QVariantMap( { { u"value_hints"_s, QStringList() << "value 1" << "value 2" << "value 3" } } ) }
  } );

  QgsProcessingStringWidgetWrapper wrapperHintsOptional( &param );

  w = wrapperHintsOptional.createWrappedWidget( context );

  QSignalSpy spy8( &wrapperHintsOptional, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperHintsOptional.setWidgetValue( u"value 2"_s, context );
  QCOMPARE( spy8.count(), 1 );
  QCOMPARE( wrapperHintsOptional.widgetValue().toString(), u"value 2"_s );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHintsOptional.wrappedWidget() )->currentText(), u"value 2"_s );
  wrapperHintsOptional.setWidgetValue( QVariant(), context );
  QCOMPARE( spy8.count(), 2 );
  QVERIFY( !wrapperHintsOptional.widgetValue().isValid() );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHintsOptional.wrappedWidget() )->currentText(), QString() );
  wrapperHintsOptional.setWidgetValue( u"value 3"_s, context );
  QCOMPARE( spy8.count(), 3 );
  QCOMPARE( wrapperHintsOptional.widgetValue().toString(), u"value 3"_s );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHintsOptional.wrappedWidget() )->currentText(), u"value 3"_s );

  // set to value which is not present -- should fallback to first value ("not set")
  wrapperHintsOptional.setWidgetValue( u"value 4"_s, context );
  QCOMPARE( spy8.count(), 4 );
  QVERIFY( !wrapperHintsOptional.widgetValue().isValid() );
  QCOMPARE( qgis::down_cast<QComboBox *>( wrapperHintsOptional.wrappedWidget() )->currentText(), QString() );

  l = wrapperHintsOptional.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"string [optional]"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  qgis::down_cast<QComboBox *>( wrapperHintsOptional.wrappedWidget() )->setCurrentIndex( 1 );
  QCOMPARE( spy8.count(), 5 );
  qgis::down_cast<QComboBox *>( wrapperHintsOptional.wrappedWidget() )->setCurrentIndex( 2 );
  QCOMPARE( spy8.count(), 6 );

  delete w;


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"string"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( !static_cast<QgsProcessingParameterString *>( def.get() )->multiLine() );

  // using a parameter definition as initial values
  QgsProcessingParameterString stringParam( u"n"_s, u"test desc"_s, u"aaa"_s, true );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"string"_s, context, widgetContext, &stringParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( static_cast<QgsProcessingParameterString *>( def.get() )->multiLine() );
  QCOMPARE( static_cast<QgsProcessingParameterString *>( def.get() )->defaultValue().toString(), u"aaa"_s );
  stringParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  stringParam.setMultiLine( false );
  stringParam.setDefaultValue( QString() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"string"_s, context, widgetContext, &stringParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QVERIFY( static_cast<QgsProcessingParameterString *>( def.get() )->defaultValue().toString().isEmpty() );
  QVERIFY( !static_cast<QgsProcessingParameterString *>( def.get() )->multiLine() );
}

void TestProcessingGui::testFileWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingParameterFile param( u"file"_s, u"file"_s );

    QgsProcessingFileWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFileWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QString( TEST_DATA_DIR + u"/points.shp"_s ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QString( TEST_DATA_DIR + u"/points.shp"_s ) );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper.wrappedWidget() )->filePath(), QString( TEST_DATA_DIR + u"/points.shp"_s ) );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper.wrappedWidget() )->filter(), u"All files (*.*)"_s );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper.wrappedWidget() )->storageMode(), QgsFileWidget::GetFile );
    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast<QgsFileWidget *>( wrapper.wrappedWidget() )->filePath().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"file"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsFileWidget *>( wrapper.wrappedWidget() )->setFilePath( TEST_DATA_DIR + u"/polys.shp"_s );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with extension
    QgsProcessingParameterFile param2( u"file"_s, u"file"_s, Qgis::ProcessingFileParameterBehavior::File, u"qml"_s );

    QgsProcessingFileWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper2.wrappedWidget() )->filter(), u"QML files (*.qml)"_s );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper2.wrappedWidget() )->storageMode(), QgsFileWidget::GetFile );

    // with filter
    QgsProcessingParameterFile param3( u"file"_s, u"file"_s, Qgis::ProcessingFileParameterBehavior::File, QString(), QVariant(), false, u"Project files (*.qgs *.qgz)"_s );

    QgsProcessingFileWidgetWrapper wrapper3( &param3, type );
    w = wrapper3.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper3.wrappedWidget() )->filter(), u"Project files (*.qgs *.qgz)"_s );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper3.wrappedWidget() )->storageMode(), QgsFileWidget::GetFile );

    // folder mode
    QgsProcessingParameterFile param4( u"folder"_s, u"folder"_s, Qgis::ProcessingFileParameterBehavior::Folder );

    QgsProcessingFileWidgetWrapper wrapper4( &param4, type );
    w = wrapper4.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsFileWidget *>( wrapper4.wrappedWidget() )->storageMode(), QgsFileWidget::GetDirectory );
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"file"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFile fileParam( u"n"_s, u"test desc"_s, Qgis::ProcessingFileParameterBehavior::File );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"file"_s, context, widgetContext, &fileParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterFile *>( def.get() )->behavior(), Qgis::ProcessingFileParameterBehavior::File );
  QVERIFY( !static_cast<QgsProcessingParameterFile *>( def.get() )->defaultValue().isValid() );
  QCOMPARE( static_cast<QgsProcessingParameterFile *>( def.get() )->fileFilter(), u"All files (*.*)"_s );
  fileParam.setFileFilter( u"TAB files (*.tab)"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"file"_s, context, widgetContext, &fileParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( static_cast<QgsProcessingParameterFile *>( def.get() )->fileFilter(), u"TAB files (*.tab)"_s );

  fileParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  fileParam.setBehavior( Qgis::ProcessingFileParameterBehavior::Folder );
  fileParam.setDefaultValue( u"my path"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"file"_s, context, widgetContext, &fileParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterFile *>( def.get() )->behavior(), Qgis::ProcessingFileParameterBehavior::Folder );
  QCOMPARE( static_cast<QgsProcessingParameterFile *>( def.get() )->defaultValue().toString(), u"my path"_s );
}

void TestProcessingGui::testAuthCfgWrapper()
{
  QList<QgsAuthMethodConfig> configs;

  // Basic
  QgsAuthMethodConfig b_config;
  b_config.setId( u"aaaaaaa"_s );
  b_config.setName( u"Basic"_s );
  b_config.setMethod( u"Basic"_s );
  b_config.setUri( u"http://example.com"_s );
  b_config.setConfig( u"username"_s, u"username"_s );
  b_config.setConfig( u"password"_s, u"password"_s );
  b_config.setConfig( u"realm"_s, u"Realm"_s );
  configs << b_config;

  QgsAuthMethodConfig b_config2;
  b_config2.setId( u"bbbbbbb"_s );
  b_config2.setName( u"Basic2"_s );
  b_config2.setMethod( u"Basic"_s );
  b_config2.setUri( u"http://example.com"_s );
  b_config2.setConfig( u"username"_s, u"username"_s );
  b_config2.setConfig( u"password"_s, u"password"_s );
  b_config2.setConfig( u"realm"_s, u"Realm"_s );
  configs << b_config2;

  QgsAuthManager *authm = QgsApplication::authManager();
  QStringList authIds;
  for ( QgsAuthMethodConfig config : std::as_const( configs ) )
  {
    QVERIFY( config.isValid() );

    QVERIFY( authm->storeAuthenticationConfig( config ) );

    // config should now have a valid, unique ID
    authIds << config.id();
  }

  QCOMPARE( authIds.count(), 2 );

  QgsProcessingParameterAuthConfig param( u"authcfg"_s, u"authcfg"_s );

  // standard wrapper
  QgsProcessingAuthConfigWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingAuthConfigWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( authIds.at( 0 ), context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toString(), authIds.at( 0 ) );
  QCOMPARE( static_cast<QgsAuthConfigSelect *>( wrapper.wrappedWidget() )->configId(), authIds.at( 0 ) );
  wrapper.setWidgetValue( authIds.at( 1 ), context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toString(), authIds.at( 1 ) );
  QCOMPARE( static_cast<QgsAuthConfigSelect *>( wrapper.wrappedWidget() )->configId(), authIds.at( 1 ) );
  wrapper.setWidgetValue( QString(), context );
  QCOMPARE( spy.count(), 3 );
  QVERIFY( wrapper.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast<QgsAuthConfigSelect *>( wrapper.wrappedWidget() )->configId().isEmpty() );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"authcfg"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast<QgsAuthConfigSelect *>( wrapper.wrappedWidget() )->setConfigId( authIds.at( 0 ) );
  QCOMPARE( spy.count(), 4 );

  delete w;

  // batch wrapper
  QgsProcessingAuthConfigWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingAuthConfigWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( authIds.at( 0 ), context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toString(), authIds.at( 0 ) );
  QCOMPARE( static_cast<QgsAuthConfigSelect *>( wrapperB.wrappedWidget() )->configId(), authIds.at( 0 ) );
  wrapperB.setWidgetValue( QString(), context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( wrapperB.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast<QgsAuthConfigSelect *>( wrapperB.wrappedWidget() )->configId().isEmpty() );

  // check signal
  static_cast<QgsAuthConfigSelect *>( w )->setConfigId( authIds.at( 0 ) );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingAuthConfigWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingAuthConfigWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( authIds.at( 0 ), context );
  QCOMPARE( wrapperM.widgetValue().toString(), authIds.at( 0 ) );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( static_cast<QgsAuthConfigSelect *>( wrapperM.wrappedWidget() )->configId(), authIds.at( 0 ) );
  wrapperM.setWidgetValue( QString(), context );
  QVERIFY( wrapperM.widgetValue().toString().isEmpty() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( static_cast<QgsAuthConfigSelect *>( wrapperM.wrappedWidget() )->configId().isEmpty() );

  // check signal
  static_cast<QgsAuthConfigSelect *>( w )->setConfigId( authIds.at( 0 ) );
  QCOMPARE( spy3.count(), 3 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"authcfg"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;
}

void TestProcessingGui::testCrsWrapper()
{
  QgsProcessingParameterCrs param( u"crs"_s, u"crs"_s );

  // standard wrapper
  QgsProcessingCrsWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingCrsWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( u"epsg:3111"_s, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().value<QgsCoordinateReferenceSystem>().authid(), u"EPSG:3111"_s );
  QCOMPARE( static_cast<QgsProjectionSelectionWidget *>( wrapper.wrappedWidget() )->crs().authid(), u"EPSG:3111"_s );
  wrapper.setWidgetValue( QgsCoordinateReferenceSystem( u"EPSG:28356"_s ), context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().value<QgsCoordinateReferenceSystem>().authid(), u"EPSG:28356"_s );
  QCOMPARE( static_cast<QgsProjectionSelectionWidget *>( wrapper.wrappedWidget() )->crs().authid(), u"EPSG:28356"_s );
  wrapper.setWidgetValue( QString(), context );
  QCOMPARE( spy.count(), 3 );
  QVERIFY( !wrapper.widgetValue().value<QgsCoordinateReferenceSystem>().isValid() );
  QVERIFY( !static_cast<QgsProjectionSelectionWidget *>( wrapper.wrappedWidget() )->crs().isValid() );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"crs"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast<QgsProjectionSelectionWidget *>( wrapper.wrappedWidget() )->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  QCOMPARE( spy.count(), 4 );
  static_cast<QgsProjectionSelectionWidget *>( wrapper.wrappedWidget() )->setCrs( QgsCoordinateReferenceSystem() );
  QCOMPARE( spy.count(), 5 );

  delete w;

  // batch wrapper
  QgsProcessingCrsWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingCrsWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( u"epsg:3111"_s, context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().value<QgsCoordinateReferenceSystem>().authid(), u"EPSG:3111"_s );
  QCOMPARE( static_cast<QgsProjectionSelectionWidget *>( wrapperB.wrappedWidget() )->crs().authid(), u"EPSG:3111"_s );
  wrapperB.setWidgetValue( QgsCoordinateReferenceSystem(), context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( !wrapperB.widgetValue().value<QgsCoordinateReferenceSystem>().isValid() );
  QVERIFY( !static_cast<QgsProjectionSelectionWidget *>( wrapperB.wrappedWidget() )->crs().isValid() );

  // check signal
  static_cast<QgsProjectionSelectionWidget *>( w )->setCrs( QgsCoordinateReferenceSystem( u"EPSG:28356"_s ) );
  QCOMPARE( spy2.count(), 3 );
  static_cast<QgsProjectionSelectionWidget *>( w )->setCrs( QgsCoordinateReferenceSystem() );
  QCOMPARE( spy2.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingCrsWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingCrsWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( u"epsg:3111"_s, context );
  QCOMPARE( wrapperM.widgetValue().value<QgsCoordinateReferenceSystem>().authid(), u"EPSG:3111"_s );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( wrapperM.mProjectionSelectionWidget->crs().authid(), u"EPSG:3111"_s );
  QVERIFY( !wrapperM.mUseProjectCrsCheckBox->isChecked() );
  wrapperM.setWidgetValue( QgsCoordinateReferenceSystem(), context );
  QVERIFY( !wrapperM.widgetValue().value<QgsCoordinateReferenceSystem>().isValid() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( !wrapperM.mProjectionSelectionWidget->crs().isValid() );
  QVERIFY( !wrapperM.mUseProjectCrsCheckBox->isChecked() );
  wrapperM.setWidgetValue( u"ProjectCrs"_s, context );
  QCOMPARE( wrapperM.widgetValue().toString(), u"ProjectCrs"_s );
  QCOMPARE( spy3.count(), 3 );
  QVERIFY( wrapperM.mUseProjectCrsCheckBox->isChecked() );

  // check signal
  wrapperM.mUseProjectCrsCheckBox->setChecked( false );
  QCOMPARE( spy3.count(), 4 );
  wrapperM.mProjectionSelectionWidget->setCrs( QgsCoordinateReferenceSystem( u"EPSG:28355"_s ) );
  QCOMPARE( spy3.count(), 5 );
  wrapperM.mProjectionSelectionWidget->setCrs( QgsCoordinateReferenceSystem() );
  QCOMPARE( spy3.count(), 6 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"crs"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"crs"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterCrs crsParam( u"n"_s, u"test desc"_s, u"EPSG:4326"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"crs"_s, context, widgetContext, &crsParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterCrs *>( def.get() )->defaultValue().toString(), u"EPSG:4326"_s );
  crsParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  crsParam.setDefaultValue( u"EPSG:3111"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"crs"_s, context, widgetContext, &crsParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterCrs *>( def.get() )->defaultValue().toString(), u"EPSG:3111"_s );
}

void TestProcessingGui::testNumericWrapperDouble()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingContext context;

    QgsProcessingParameterNumber param( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double );
    QgsProcessingNumericWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->expressionsEnabled() );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->decimals(), 6 ); // you can change this, if it's an intentional change!
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->singleStep(), 1.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->minimum(), -999999999.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->maximum(), 999999999.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->clearValue(), 0.0 );

    QSignalSpy spy( &wrapper, &QgsProcessingNumericWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 5, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 5.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->value(), 5.0 );
    wrapper.setWidgetValue( u"28356"_s, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 28356.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->value(), 28356.0 );
    wrapper.setWidgetValue( QVariant(), context ); // not optional, so shouldn't work
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 0.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->value(), 0.0 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"num"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->setValue( 37.0 );
    QCOMPARE( spy.count(), 4 );
    static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->clear();
    QCOMPARE( spy.count(), 5 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 0.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapper.wrappedWidget() )->value(), 0.0 );

    delete w;

    // with min value
    QgsProcessingParameterNumber paramMin( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double );
    paramMin.setMinimum( -5 );

    QgsProcessingNumericWidgetWrapper wrapperMin( &paramMin, type );

    w = wrapperMin.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMin.wrappedWidget() )->singleStep(), 1.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMin.wrappedWidget() )->minimum(), -5.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMin.wrappedWidget() )->maximum(), 999999999.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMin.wrappedWidget() )->clearValue(), -5.0 );
    QCOMPARE( wrapperMin.parameterValue().toDouble(), 0.0 );
    delete w;

    // with max value
    QgsProcessingParameterNumber paramMax( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double );
    paramMax.setMaximum( 5 );

    QgsProcessingNumericWidgetWrapper wrapperMax( &paramMax, type );

    w = wrapperMax.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMax.wrappedWidget() )->singleStep(), 1.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMax.wrappedWidget() )->minimum(), -999999999.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMax.wrappedWidget() )->maximum(), 5.0 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMax.wrappedWidget() )->clearValue(), 0.0 );
    QCOMPARE( wrapperMax.parameterValue().toDouble(), 0.0 );
    delete w;

    // with min and max value
    QgsProcessingParameterNumber paramMinMax( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double );
    paramMinMax.setMinimum( -.1 );
    paramMinMax.setMaximum( .1 );

    QgsProcessingNumericWidgetWrapper wrapperMinMax( &paramMinMax, type );

    w = wrapperMinMax.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMinMax.wrappedWidget() )->singleStep(), 0.02 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMinMax.wrappedWidget() )->minimum(), -.1 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMinMax.wrappedWidget() )->maximum(), .1 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperMinMax.wrappedWidget() )->clearValue(), -.1 );
    QCOMPARE( wrapperMinMax.parameterValue().toDouble(), 0.0 );
    delete w;

    // with default value
    QgsProcessingParameterNumber paramDefault( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double );
    paramDefault.setDefaultValue( 55 );

    QgsProcessingNumericWidgetWrapper wrapperDefault( &paramDefault, type );

    w = wrapperDefault.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperDefault.wrappedWidget() )->clearValue(), 55.0 );
    QCOMPARE( wrapperDefault.parameterValue().toDouble(), 55.0 );
    delete w;

    // optional, no default
    QgsProcessingParameterNumber paramOptional( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double, QVariant(), true );

    QgsProcessingNumericWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperOptional.wrappedWidget() )->clearValue(), -1000000000.0 );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    QCOMPARE( wrapperOptional.parameterValue().toDouble(), 5.0 );
    wrapperOptional.setParameterValue( QVariant(), context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    static_cast<QgsDoubleSpinBox *>( wrapperOptional.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptional.parameterValue().isValid() );

    // optional, with default
    paramOptional.setDefaultValue( 3 );
    QgsProcessingNumericWidgetWrapper wrapperOptionalDefault( &paramOptional, type );

    w = wrapperOptionalDefault.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperOptionalDefault.wrappedWidget() )->clearValue(), -1000000000.0 );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 3.0 );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    wrapperOptionalDefault.setParameterValue( QVariant(), context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperOptionalDefault.wrappedWidget() )->value(), -1000000000.0 );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    static_cast<QgsDoubleSpinBox *>( wrapperOptionalDefault.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );

    delete w;

    // with decimals
    QgsProcessingParameterNumber paramDecimals( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1, 1.02 );
    QVariantMap metadata;
    QVariantMap wrapperMetadata;
    wrapperMetadata.insert( u"decimals"_s, 2 );
    metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
    paramDecimals.setMetadata( metadata );
    QgsProcessingNumericWidgetWrapper wrapperDecimals( &paramDecimals, type );
    w = wrapperDecimals.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperDecimals.wrappedWidget() )->decimals(), 2 );
    QCOMPARE( static_cast<QgsDoubleSpinBox *>( wrapperDecimals.wrappedWidget() )->singleStep(), 0.01 ); // single step should never be less than set number of decimals
    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterNumber numParam( u"n"_s, u"test desc"_s, Qgis::ProcessingNumberParameterType::Double, 1.0 );
  numParam.setMinimum( 0 );
  numParam.setMaximum( 10 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext, &numParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Double );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->minimum(), 0.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->maximum(), 10.0 );
  numParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  numParam.setDataType( Qgis::ProcessingNumberParameterType::Integer );
  numParam.setMinimum( -1 );
  numParam.setMaximum( 1 );
  numParam.setDefaultValue( 0 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext, &numParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->defaultValue().toInt(), 0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Integer );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->minimum(), -1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->maximum(), 1.0 );
}

void TestProcessingGui::testNumericWrapperInt()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingContext context;

    QgsProcessingParameterNumber param( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Integer );
    QgsProcessingNumericWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->expressionsEnabled() );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->minimum(), -999999999 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->maximum(), 999999999 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->clearValue(), 0 );

    QSignalSpy spy( &wrapper, &QgsProcessingNumericWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 5, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toInt(), 5 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->value(), 5 );
    wrapper.setWidgetValue( u"28356"_s, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toInt(), 28356 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->value(), 28356 );
    wrapper.setWidgetValue( QVariant(), context ); // not optional, so shouldn't work
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toInt(), 0 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->value(), 0 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"num"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->setValue( 37 );
    QCOMPARE( spy.count(), 4 );
    static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->clear();
    QCOMPARE( spy.count(), 5 );
    QCOMPARE( wrapper.widgetValue().toInt(), 0 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapper.wrappedWidget() )->value(), 0 );

    delete w;

    // with min value
    QgsProcessingParameterNumber paramMin( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Integer );
    paramMin.setMinimum( -5 );

    QgsProcessingNumericWidgetWrapper wrapperMin( &paramMin, type );

    w = wrapperMin.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMin.wrappedWidget() )->minimum(), -5 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMin.wrappedWidget() )->maximum(), 999999999 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMin.wrappedWidget() )->clearValue(), -5 );
    QCOMPARE( wrapperMin.parameterValue().toInt(), 0 );
    delete w;

    // with max value
    QgsProcessingParameterNumber paramMax( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Integer );
    paramMax.setMaximum( 5 );

    QgsProcessingNumericWidgetWrapper wrapperMax( &paramMax, type );

    w = wrapperMax.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMax.wrappedWidget() )->minimum(), -999999999 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMax.wrappedWidget() )->maximum(), 5 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMax.wrappedWidget() )->clearValue(), 0 );
    QCOMPARE( wrapperMax.parameterValue().toInt(), 0 );
    delete w;

    // with min and max value
    QgsProcessingParameterNumber paramMinMax( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Integer );
    paramMinMax.setMinimum( -1 );
    paramMinMax.setMaximum( 1 );

    QgsProcessingNumericWidgetWrapper wrapperMinMax( &paramMinMax, type );

    w = wrapperMinMax.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMinMax.wrappedWidget() )->minimum(), -1 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMinMax.wrappedWidget() )->maximum(), 1 );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperMinMax.wrappedWidget() )->clearValue(), -1 );
    QCOMPARE( wrapperMinMax.parameterValue().toInt(), 0 );
    delete w;

    // with default value
    QgsProcessingParameterNumber paramDefault( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Integer );
    paramDefault.setDefaultValue( 55 );

    QgsProcessingNumericWidgetWrapper wrapperDefault( &paramDefault, type );

    w = wrapperDefault.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperDefault.wrappedWidget() )->clearValue(), 55 );
    QCOMPARE( wrapperDefault.parameterValue().toInt(), 55 );
    delete w;

    // optional, no default
    QgsProcessingParameterNumber paramOptional( u"num"_s, u"num"_s, Qgis::ProcessingNumberParameterType::Integer, QVariant(), true );

    QgsProcessingNumericWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperOptional.wrappedWidget() )->clearValue(), -1000000000 );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    QCOMPARE( wrapperOptional.parameterValue().toInt(), 5 );
    wrapperOptional.setParameterValue( QVariant(), context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    static_cast<QgsSpinBox *>( wrapperOptional.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptional.parameterValue().isValid() );

    // optional, with default
    paramOptional.setDefaultValue( 3 );
    QgsProcessingNumericWidgetWrapper wrapperOptionalDefault( &paramOptional, type );

    w = wrapperOptionalDefault.createWrappedWidget( context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperOptionalDefault.wrappedWidget() )->clearValue(), -1000000000 );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 3 );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 5 );
    wrapperOptionalDefault.setParameterValue( QVariant(), context );
    QCOMPARE( static_cast<QgsSpinBox *>( wrapperOptionalDefault.wrappedWidget() )->value(), -1000000000 );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 5 );
    static_cast<QgsSpinBox *>( wrapperOptionalDefault.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 5 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterNumber numParam( u"n"_s, u"test desc"_s, Qgis::ProcessingNumberParameterType::Integer, 1 );
  numParam.setMinimum( 0 );
  numParam.setMaximum( 10 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext, &numParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Integer );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->minimum(), 0.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->maximum(), 10.0 );
  numParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  numParam.setDataType( Qgis::ProcessingNumberParameterType::Double );
  numParam.setMinimum( -2.5 );
  numParam.setMaximum( 2.5 );
  numParam.setDefaultValue( 0.5 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext, &numParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->defaultValue().toDouble(), 0.5 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Double );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->minimum(), -2.5 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->maximum(), 2.5 );

  // integer type, no min/max values set
  QgsProcessingParameterNumber numParam2( u"n"_s, u"test desc"_s, Qgis::ProcessingNumberParameterType::Integer, 1 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext, &numParam2 );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Integer );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->minimum(), numParam2.minimum() );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->maximum(), numParam2.maximum() );

  // double type, no min/max values set
  QgsProcessingParameterNumber numParam3( u"n"_s, u"test desc"_s, Qgis::ProcessingNumberParameterType::Double, 1 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"number"_s, context, widgetContext, &numParam3 );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Double );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->minimum(), numParam3.minimum() );
  QCOMPARE( static_cast<QgsProcessingParameterNumber *>( def.get() )->maximum(), numParam3.maximum() );
}

void TestProcessingGui::testDistanceWrapper()
{
  QgsProcessingParameterDistance param( u"distance"_s, u"distance"_s );

  // standard wrapper
  QgsProcessingDistanceWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingDistanceWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( 55.5, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toDouble(), 55.5 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), 55.5 );
  wrapper.setWidgetValue( -34.0, context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toDouble(), -34.0 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), -34.0 );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"distance"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  wrapper.mDoubleSpinBox->setValue( 43.0 );
  QCOMPARE( spy.count(), 3 );

  // test unit handling
  w->show();

  QCOMPARE( wrapper.mLabel->text(), u"<unknown>"_s );

  // crs values
  wrapper.setUnitParameterValue( u"EPSG:3111"_s );
  QCOMPARE( wrapper.mLabel->text(), u"meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast<int>( Qgis::DistanceUnit::Meters ) );

  wrapper.setUnitParameterValue( u"EPSG:4326"_s );
  QCOMPARE( wrapper.mLabel->text(), u"degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  QCOMPARE( wrapper.mLabel->text(), u"meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast<int>( Qgis::DistanceUnit::Meters ) );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QCOMPARE( wrapper.mLabel->text(), u"degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // layer values
  auto vl = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:3111&field=pk:int"_s, u"vl"_s, u"memory"_s );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl.get() ) );
  QCOMPARE( wrapper.mLabel->text(), u"meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast<int>( Qgis::DistanceUnit::Meters ) );

  auto vl2 = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:4326&field=pk:int"_s, u"vl"_s, u"memory"_s );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl2.get() ) );
  QCOMPARE( wrapper.mLabel->text(), u"degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // unresolvable values
  wrapper.setUnitParameterValue( u"blah"_s );
  QCOMPARE( wrapper.mLabel->text(), u"<unknown>"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // resolvable text value
  const QString id = vl->id();
  QgsProject::instance()->addMapLayer( vl.release() );
  context.setProject( QgsProject::instance() );

  TestProcessingContextGenerator generator( context );
  wrapper.registerProcessingContextGenerator( &generator );
  wrapper.setUnitParameterValue( QVariant::fromValue( id ) );
  QCOMPARE( wrapper.mLabel->text(), u"meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast<int>( Qgis::DistanceUnit::Meters ) );

  // using unit choice
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );
  wrapper.mUnitsCombo->setCurrentIndex( wrapper.mUnitsCombo->findData( static_cast<int>( Qgis::DistanceUnit::Kilometers ) ) );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5000.0 );
  wrapper.setParameterValue( 2, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2000.0 );

  // Changing to a layer with a CRS that has compatible units won't change the units
  wrapper.setUnitParameterValue( QVariant::fromValue( id ) );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2000.0 );
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5000.0 );
  QCOMPARE( wrapper.mUnitsCombo->currentIndex(), wrapper.mUnitsCombo->findData( static_cast<int>( Qgis::DistanceUnit::Kilometers ) ) );

  // Changing to a layer with 4326 projection will reset the units
  const QString id2 = vl2->id();
  QgsProject::instance()->addMapLayer( vl2.release() );
  wrapper.setUnitParameterValue( QVariant::fromValue( id2 ) );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );
  wrapper.setParameterValue( 2, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2.0 );

  delete w;

  // with default unit
  QgsProcessingParameterDistance paramDefaultUnit( u"num"_s, u"num"_s );
  paramDefaultUnit.setDefaultUnit( Qgis::DistanceUnit::Feet );
  QgsProcessingDistanceWidgetWrapper wrapperDefaultUnit( &paramDefaultUnit, Qgis::ProcessingMode::Standard );
  w = wrapperDefaultUnit.createWrappedWidget( context );
  w->show();
  QCOMPARE( wrapperDefaultUnit.mLabel->text(), u"feet"_s );
  delete w;

  // with decimals
  QgsProcessingParameterDistance paramDecimals( u"num"_s, u"num"_s, QVariant(), QString(), true, 1, 1.02 );
  QVariantMap metadata;
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( u"decimals"_s, 2 );
  metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
  paramDecimals.setMetadata( metadata );
  QgsProcessingDistanceWidgetWrapper wrapperDecimals( &paramDecimals, Qgis::ProcessingMode::Standard );
  w = wrapperDecimals.createWrappedWidget( context );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->decimals(), 2 );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->singleStep(), 0.01 ); // single step should never be less than set number of decimals
  delete w;

  // batch wrapper
  QgsProcessingDistanceWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingDistanceWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( 34, context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), 34.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), 34.0 );
  wrapperB.setWidgetValue( -57, context );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), -57.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), -57.0 );

  // check signal
  static_cast<QgsDoubleSpinBox *>( w )->setValue( 29 );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingDistanceWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingDistanceWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( 29, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), 29.0 );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), 29.0 );
  wrapperM.setWidgetValue( -29, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), -29.0 );
  QCOMPARE( spy3.count(), 2 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), -29.0 );

  // check signal
  wrapperM.mDoubleSpinBox->setValue( 33 );
  QCOMPARE( spy3.count(), 3 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"distance"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"distance"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterDistance distParam( u"n"_s, u"test desc"_s, 1, u"parent"_s );
  distParam.setMinimum( 1 );
  distParam.setMaximum( 100 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"distance"_s, context, widgetContext, &distParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->minimum(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->maximum(), 100.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->parentParameterName(), u"parent"_s );
  distParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  distParam.setParentParameterName( QString() );
  distParam.setMinimum( 10 );
  distParam.setMaximum( 12 );
  distParam.setDefaultValue( 11.5 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"distance"_s, context, widgetContext, &distParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->defaultValue().toDouble(), 11.5 );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->minimum(), 10.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDistance *>( def.get() )->maximum(), 12.0 );
  QVERIFY( static_cast<QgsProcessingParameterDistance *>( def.get() )->parentParameterName().isEmpty() );
}

void TestProcessingGui::testAreaWrapper()
{
  QgsProcessingParameterArea param( u"area"_s, u"area"_s );

  // standard wrapper
  QgsProcessingAreaWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingAreaWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( 55.5, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toDouble(), 55.5 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), 55.5 );
  wrapper.setWidgetValue( 3.0, context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toDouble(), 3.0 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), 3.0 );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"area"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  wrapper.mDoubleSpinBox->setValue( 43.0 );
  QCOMPARE( spy.count(), 3 );

  // test unit handling
  w->show();

  QCOMPARE( wrapper.mLabel->text(), u"<unknown>"_s );

  // crs values
  wrapper.setUnitParameterValue( u"EPSG:3111"_s );
  QCOMPARE( wrapper.mLabel->text(), u"square meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::AreaUnit>(), Qgis::AreaUnit::SquareMeters );

  wrapper.setUnitParameterValue( u"EPSG:4326"_s );
  QCOMPARE( wrapper.mLabel->text(), u"square degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  QCOMPARE( wrapper.mLabel->text(), u"square meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::AreaUnit>(), Qgis::AreaUnit::SquareMeters );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QCOMPARE( wrapper.mLabel->text(), u"square degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // layer values
  auto vl = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:3111&field=pk:int"_s, u"vl"_s, u"memory"_s );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl.get() ) );
  QCOMPARE( wrapper.mLabel->text(), u"square meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::AreaUnit>(), Qgis::AreaUnit::SquareMeters );

  auto vl2 = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:4326&field=pk:int"_s, u"vl"_s, u"memory"_s );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl2.get() ) );
  QCOMPARE( wrapper.mLabel->text(), u"square degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // unresolvable values
  wrapper.setUnitParameterValue( u"blah"_s );
  QCOMPARE( wrapper.mLabel->text(), u"<unknown>"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // resolvable text value
  const QString id = vl->id();
  QgsProject::instance()->addMapLayer( vl.release() );
  context.setProject( QgsProject::instance() );

  TestProcessingContextGenerator generator( context );
  wrapper.registerProcessingContextGenerator( &generator );
  wrapper.setUnitParameterValue( id );
  QCOMPARE( wrapper.mLabel->text(), u"square meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::AreaUnit>(), Qgis::AreaUnit::SquareMeters );

  // using unit choice
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );
  wrapper.mUnitsCombo->setCurrentIndex( wrapper.mUnitsCombo->findData( QVariant::fromValue( Qgis::AreaUnit::Hectares ) ) );
  QCOMPARE( wrapper.parameterValue().toDouble(), 50000.0 );
  wrapper.setParameterValue( 2, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 20000.0 );

  wrapper.setUnitParameterValue( id );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2.0 );
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );

  delete w;

  // with default unit
  QgsProcessingParameterArea paramDefaultUnit( u"num"_s, u"num"_s );
  paramDefaultUnit.setDefaultUnit( Qgis::AreaUnit::SquareFeet );
  QgsProcessingAreaWidgetWrapper wrapperDefaultUnit( &paramDefaultUnit, Qgis::ProcessingMode::Standard );
  w = wrapperDefaultUnit.createWrappedWidget( context );
  w->show();
  QCOMPARE( wrapperDefaultUnit.mLabel->text(), u"square feet"_s );
  delete w;

  // with decimals
  QgsProcessingParameterArea paramDecimals( u"num"_s, u"num"_s, QVariant(), QString(), true, 1, 1.02 );
  QVariantMap metadata;
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( u"decimals"_s, 2 );
  metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
  paramDecimals.setMetadata( metadata );
  QgsProcessingAreaWidgetWrapper wrapperDecimals( &paramDecimals, Qgis::ProcessingMode::Standard );
  w = wrapperDecimals.createWrappedWidget( context );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->decimals(), 2 );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->singleStep(), 0.01 ); // single step should never be less than set number of decimals
  delete w;

  // batch wrapper
  QgsProcessingAreaWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingAreaWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( 34, context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), 34.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), 34.0 );
  wrapperB.setWidgetValue( 5, context );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), 5.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), 5.0 );

  // check signal
  static_cast<QgsDoubleSpinBox *>( w )->setValue( 29 );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingAreaWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingAreaWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( 29, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), 29.0 );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), 29.0 );
  wrapperM.setWidgetValue( 4, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), 4.0 );
  QCOMPARE( spy3.count(), 2 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), 4.0 );

  // check signal
  wrapperM.mDoubleSpinBox->setValue( 33 );
  QCOMPARE( spy3.count(), 3 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"area"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"area"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterArea distParam( u"n"_s, u"test desc"_s, 1, u"parent"_s );
  distParam.setMinimum( 1 );
  distParam.setMaximum( 100 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"area"_s, context, widgetContext, &distParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->minimum(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->maximum(), 100.0 );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->parentParameterName(), u"parent"_s );
  distParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  distParam.setParentParameterName( QString() );
  distParam.setMinimum( 10 );
  distParam.setMaximum( 12 );
  distParam.setDefaultValue( 11.5 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"area"_s, context, widgetContext, &distParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->defaultValue().toDouble(), 11.5 );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->minimum(), 10.0 );
  QCOMPARE( static_cast<QgsProcessingParameterArea *>( def.get() )->maximum(), 12.0 );
  QVERIFY( static_cast<QgsProcessingParameterArea *>( def.get() )->parentParameterName().isEmpty() );
}

void TestProcessingGui::testVolumeWrapper()
{
  QgsProcessingParameterVolume param( u"volume"_s, u"volume"_s );

  // standard wrapper
  QgsProcessingVolumeWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingVolumeWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( 55.5, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toDouble(), 55.5 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), 55.5 );
  wrapper.setWidgetValue( 3.0, context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toDouble(), 3.0 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), 3.0 );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"volume"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  wrapper.mDoubleSpinBox->setValue( 43.0 );
  QCOMPARE( spy.count(), 3 );

  // test unit handling
  w->show();

  QCOMPARE( wrapper.mLabel->text(), u"<unknown>"_s );

  // crs values
  wrapper.setUnitParameterValue( u"EPSG:3111"_s );
  QCOMPARE( wrapper.mLabel->text(), u"cubic meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::VolumeUnit>(), Qgis::VolumeUnit::CubicMeters );

  wrapper.setUnitParameterValue( u"EPSG:4326"_s );
  QCOMPARE( wrapper.mLabel->text(), u"cubic degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  QCOMPARE( wrapper.mLabel->text(), u"cubic meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::VolumeUnit>(), Qgis::VolumeUnit::CubicMeters );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QCOMPARE( wrapper.mLabel->text(), u"cubic degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // layer values
  auto vl = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:3111&field=pk:int"_s, u"vl"_s, u"memory"_s );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl.get() ) );
  QCOMPARE( wrapper.mLabel->text(), u"cubic meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::VolumeUnit>(), Qgis::VolumeUnit::CubicMeters );

  auto vl2 = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:4326&field=pk:int"_s, u"vl"_s, u"memory"_s );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl2.get() ) );
  QCOMPARE( wrapper.mLabel->text(), u"cubic degrees"_s );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // unresolvable values
  wrapper.setUnitParameterValue( u"blah"_s );
  QCOMPARE( wrapper.mLabel->text(), u"<unknown>"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // resolvable text value
  const QString id = vl->id();
  QgsProject::instance()->addMapLayer( vl.release() );
  context.setProject( QgsProject::instance() );

  TestProcessingContextGenerator generator( context );
  wrapper.registerProcessingContextGenerator( &generator );
  wrapper.setUnitParameterValue( id );
  QCOMPARE( wrapper.mLabel->text(), u"cubic meters"_s );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().value<Qgis::VolumeUnit>(), Qgis::VolumeUnit::CubicMeters );

  // using unit choice
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );
  wrapper.mUnitsCombo->setCurrentIndex( wrapper.mUnitsCombo->findData( QVariant::fromValue( Qgis::VolumeUnit::Liters ) ) );
  QCOMPARE( wrapper.parameterValue().toDouble(), 0.005 );
  wrapper.setParameterValue( 2, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 0.002 );

  wrapper.setUnitParameterValue( id );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2.0 );
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );

  delete w;

  // with default unit
  QgsProcessingParameterVolume paramDefaultUnit( u"num"_s, u"num"_s );
  paramDefaultUnit.setDefaultUnit( Qgis::VolumeUnit::CubicFeet );
  QgsProcessingVolumeWidgetWrapper wrapperDefaultUnit( &paramDefaultUnit, Qgis::ProcessingMode::Standard );
  w = wrapperDefaultUnit.createWrappedWidget( context );
  w->show();
  QCOMPARE( wrapperDefaultUnit.mLabel->text(), u"cubic feet"_s );
  delete w;

  // with decimals
  QgsProcessingParameterVolume paramDecimals( u"num"_s, u"num"_s, QVariant(), QString(), true, 1, 1.02 );
  QVariantMap metadata;
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( u"decimals"_s, 2 );
  metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
  paramDecimals.setMetadata( metadata );
  QgsProcessingVolumeWidgetWrapper wrapperDecimals( &paramDecimals, Qgis::ProcessingMode::Standard );
  w = wrapperDecimals.createWrappedWidget( context );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->decimals(), 2 );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->singleStep(), 0.01 ); // single step should never be less than set number of decimals
  delete w;

  // batch wrapper
  QgsProcessingVolumeWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingVolumeWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( 34, context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), 34.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), 34.0 );
  wrapperB.setWidgetValue( 5, context );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), 5.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), 5.0 );

  // check signal
  static_cast<QgsDoubleSpinBox *>( w )->setValue( 29 );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingVolumeWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingVolumeWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( 29, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), 29.0 );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), 29.0 );
  wrapperM.setWidgetValue( 3, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), 3.0 );
  QCOMPARE( spy3.count(), 2 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), 3.0 );

  // check signal
  wrapperM.mDoubleSpinBox->setValue( 33 );
  QCOMPARE( spy3.count(), 3 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"volume"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"volume"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterVolume distParam( u"n"_s, u"test desc"_s, 1, u"parent"_s );
  distParam.setMinimum( 1 );
  distParam.setMaximum( 100 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"volume"_s, context, widgetContext, &distParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->minimum(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->maximum(), 100.0 );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->parentParameterName(), u"parent"_s );
  distParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  distParam.setParentParameterName( QString() );
  distParam.setMinimum( 10 );
  distParam.setMaximum( 12 );
  distParam.setDefaultValue( 11.5 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"volume"_s, context, widgetContext, &distParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->defaultValue().toDouble(), 11.5 );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->minimum(), 10.0 );
  QCOMPARE( static_cast<QgsProcessingParameterVolume *>( def.get() )->maximum(), 12.0 );
  QVERIFY( static_cast<QgsProcessingParameterVolume *>( def.get() )->parentParameterName().isEmpty() );
}

void TestProcessingGui::testDurationWrapper()
{
  QgsProcessingParameterDuration param( u"duration"_s, u"duration"_s );

  // standard wrapper
  QgsProcessingDurationWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingDurationWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( 55.5, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toDouble(), 55.5 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), 55.5 );
  wrapper.setWidgetValue( -34.0, context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toDouble(), -34.0 );
  QCOMPARE( wrapper.mDoubleSpinBox->value(), -34.0 );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"duration"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  wrapper.mDoubleSpinBox->setValue( 43.0 );
  QCOMPARE( spy.count(), 3 );

  // with default unit
  QgsProcessingParameterDuration paramDefaultUnit( u"dur"_s, u"dur"_s );
  paramDefaultUnit.setDefaultUnit( Qgis::TemporalUnit::Days );
  QgsProcessingDurationWidgetWrapper wrapperDefaultUnit( &paramDefaultUnit, Qgis::ProcessingMode::Standard );
  w = wrapperDefaultUnit.createWrappedWidget( context );
  w->show();
  QCOMPARE( wrapperDefaultUnit.mUnitsCombo->currentText(), QgsUnitTypes::toString( Qgis::TemporalUnit::Days ) );
  delete w;

  // with decimals
  QgsProcessingParameterDuration paramDecimals( u"num"_s, u"num"_s, QVariant(), true, 1, 1.02 );
  QVariantMap metadata;
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( u"decimals"_s, 2 );
  metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
  paramDecimals.setMetadata( metadata );
  QgsProcessingDurationWidgetWrapper wrapperDecimals( &paramDecimals, Qgis::ProcessingMode::Standard );
  w = wrapperDecimals.createWrappedWidget( context );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->decimals(), 2 );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->singleStep(), 0.01 ); // single step should never be less than set number of decimals
  delete w;

  // batch wrapper
  QgsProcessingDurationWidgetWrapper wrapperB( &param, Qgis::ProcessingMode::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingDurationWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( 34, context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), 34.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), 34.0 );
  wrapperB.setWidgetValue( -57, context );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( wrapperB.widgetValue().toDouble(), -57.0 );
  QCOMPARE( wrapperB.mDoubleSpinBox->value(), -57.0 );

  // check signal
  static_cast<QgsDoubleSpinBox *>( w )->setValue( 29 );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingDurationWidgetWrapper wrapperM( &param, Qgis::ProcessingMode::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingDurationWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( 29, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), 29.0 );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), 29.0 );
  wrapperM.setWidgetValue( -29, context );
  QCOMPARE( wrapperM.widgetValue().toDouble(), -29.0 );
  QCOMPARE( spy3.count(), 2 );
  QCOMPARE( wrapperM.mDoubleSpinBox->value(), -29.0 );

  // check signal
  wrapperM.mDoubleSpinBox->setValue( 33 );
  QCOMPARE( spy3.count(), 3 );

  // should be a label in modeler mode which includes a unit type string
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), u"duration [milliseconds]"_s );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"duration"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterDuration durParam( u"n"_s, u"test desc"_s, 1 );
  durParam.setMinimum( 1 );
  durParam.setMaximum( 100 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"duration"_s, context, widgetContext, &durParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterDuration *>( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDuration *>( def.get() )->minimum(), 1.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDuration *>( def.get() )->maximum(), 100.0 );
  durParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  durParam.setMinimum( 10 );
  durParam.setMaximum( 12 );
  durParam.setDefaultValue( 11.5 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"duration"_s, context, widgetContext, &durParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterDuration *>( def.get() )->defaultValue().toDouble(), 11.5 );
  QCOMPARE( static_cast<QgsProcessingParameterDuration *>( def.get() )->minimum(), 10.0 );
  QCOMPARE( static_cast<QgsProcessingParameterDuration *>( def.get() )->maximum(), 12.0 );
}

void TestProcessingGui::testScaleWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingContext context;

    QgsProcessingParameterScale param( u"num"_s, u"num"_s );
    QgsProcessingScaleWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QSignalSpy spy( &wrapper, &QgsProcessingNumericWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 5, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 5.0 );
    QCOMPARE( static_cast<QgsScaleWidget *>( wrapper.wrappedWidget() )->scale(), 5.0 );
    wrapper.setWidgetValue( u"28356"_s, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 28356.0 );
    QCOMPARE( static_cast<QgsScaleWidget *>( wrapper.wrappedWidget() )->scale(), 28356.0 );
    wrapper.setWidgetValue( QVariant(), context ); // not optional, so shouldn't work
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 0.0 );
    QCOMPARE( static_cast<QgsScaleWidget *>( wrapper.wrappedWidget() )->scale(), 0.0 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"num"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsScaleWidget *>( wrapper.wrappedWidget() )->setScale( 37.0 );
    QCOMPARE( spy.count(), 4 );

    delete w;

    // optional, no default
    QgsProcessingParameterScale paramOptional( u"num"_s, u"num"_s, QVariant(), true );

    QgsProcessingScaleWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    QCOMPARE( wrapperOptional.parameterValue().toDouble(), 5.0 );
    wrapperOptional.setParameterValue( QVariant(), context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    static_cast<QgsScaleWidget *>( wrapperOptional.wrappedWidget() )->setScale( std::numeric_limits<double>::quiet_NaN() );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );

    // optional, with default
    paramOptional.setDefaultValue( 3 );
    QgsProcessingScaleWidgetWrapper wrapperOptionalDefault( &paramOptional, type );

    w = wrapperOptionalDefault.createWrappedWidget( context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 3.0 );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    wrapperOptionalDefault.setParameterValue( QVariant(), context );
    QVERIFY( std::isnan( static_cast<QgsScaleWidget *>( wrapperOptionalDefault.wrappedWidget() )->scale() ) );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    static_cast<QgsScaleWidget *>( wrapperOptionalDefault.wrappedWidget() )->setScale( std::numeric_limits<double>::quiet_NaN() );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"scale"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterScale scaleParam( u"n"_s, u"test desc"_s, 1000 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"scale"_s, context, widgetContext, &scaleParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterScale *>( def.get() )->defaultValue().toDouble(), 1000.0 );
  scaleParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  scaleParam.setDefaultValue( 28356 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"scale"_s, context, widgetContext, &scaleParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterScale *>( def.get() )->defaultValue().toDouble(), 28356.0 );
}

void TestProcessingGui::testRangeWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingContext context;

    QgsProcessingParameterRange param( u"range"_s, u"range"_s, Qgis::ProcessingNumberParameterType::Double );
    param.setDefaultValue( u"0.0,100.0"_s );
    QgsProcessingRangeWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( w );

    // initial value
    QCOMPARE( wrapper.parameterValue().toString(), u"0,100"_s );

    QVERIFY( wrapper.mMinSpinBox->expressionsEnabled() );
    QVERIFY( wrapper.mMaxSpinBox->expressionsEnabled() );
    QCOMPARE( wrapper.mMinSpinBox->decimals(), 6 ); // you can change this, if it's an intentional change!
    QCOMPARE( wrapper.mMaxSpinBox->decimals(), 6 ); // you can change this, if it's an intentional change!
    QGSCOMPARENEAR( wrapper.mMinSpinBox->minimum(), -99999999.999999, 1 );
    QGSCOMPARENEAR( wrapper.mMaxSpinBox->minimum(), -99999999.999999, 1 );
    QGSCOMPARENEAR( wrapper.mMinSpinBox->maximum(), 99999999.999999, 1 );
    QGSCOMPARENEAR( wrapper.mMaxSpinBox->maximum(), 99999999.999999, 1 );

    QSignalSpy spy( &wrapper, &QgsProcessingRangeWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << 5 << 7, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"5,7"_s );
    QCOMPARE( wrapper.mMinSpinBox->value(), 5.0 );
    QCOMPARE( wrapper.mMaxSpinBox->value(), 7.0 );
    wrapper.setWidgetValue( u"28.1,36.5"_s, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toString(), u"28.1,36.5"_s );
    QCOMPARE( wrapper.mMinSpinBox->value(), 28.1 );
    QCOMPARE( wrapper.mMaxSpinBox->value(), 36.5 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"range"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    wrapper.mMinSpinBox->setValue( 7.0 );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toString(), u"7,36.5"_s );
    wrapper.mMaxSpinBox->setValue( 9.0 );
    QCOMPARE( spy.count(), 4 );
    QCOMPARE( wrapper.widgetValue().toString(), u"7,9"_s );

    // check that min/max are mutually adapted
    wrapper.setParameterValue( u"200.0,100.0"_s, context );
    QCOMPARE( wrapper.parameterValue().toString(), u"100,100"_s );

    wrapper.mMaxSpinBox->setValue( 50 );
    QCOMPARE( wrapper.parameterValue().toString(), u"50,50"_s );
    wrapper.mMinSpinBox->setValue( 100 );
    QCOMPARE( wrapper.parameterValue().toString(), u"100,100"_s );

    delete w;

    // ints
    QgsProcessingParameterRange param2( u"range"_s, u"range"_s, Qgis::ProcessingNumberParameterType::Integer );
    param2.setDefaultValue( u"0.1,100.1"_s );

    QgsProcessingRangeWidgetWrapper wrapper2( &param2, type );

    w = wrapper2.createWrappedWidget( context );
    QVERIFY( w );
    QCOMPARE( wrapper2.mMinSpinBox->decimals(), 0 );
    QCOMPARE( wrapper2.mMaxSpinBox->decimals(), 0 ); // you can't change this, vampire worms will bite you at night if you do

    // check initial value
    QCOMPARE( wrapper2.parameterValue().toString(), u"0,100"_s );
    // check rounding
    wrapper2.setParameterValue( u"100.1,200.1"_s, context );
    QCOMPARE( wrapper2.parameterValue().toString(), u"100,200"_s );
    wrapper2.setParameterValue( u"100.6,200.6"_s, context );
    QCOMPARE( wrapper2.parameterValue().toString(), u"101,201"_s );
    // check set/get
    wrapper2.setParameterValue( u"100.1,200.1"_s, context );
    QCOMPARE( wrapper2.parameterValue().toString(), u"100,200"_s );
    // check that min/max are mutually adapted
    wrapper2.setParameterValue( u"200.1,100.1"_s, context );
    QCOMPARE( wrapper2.parameterValue().toString(), u"100,100"_s );
    wrapper2.mMaxSpinBox->setValue( 50.1 );
    QCOMPARE( wrapper2.parameterValue().toString(), u"50,50"_s );
    wrapper2.mMinSpinBox->setValue( 100.1 );
    QCOMPARE( wrapper2.parameterValue().toString(), u"100,100"_s );

    delete w;

    // optional
    QgsProcessingParameterRange paramOptional( u"range"_s, u"range"_s, Qgis::ProcessingNumberParameterType::Double, QVariant(), true );

    QgsProcessingRangeWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"None,None"_s );
    wrapperOptional.setParameterValue( u"1,100"_s, context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"1,100"_s );
    wrapperOptional.setParameterValue( u"None,100"_s, context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"None,100"_s );
    wrapperOptional.setParameterValue( u"1,None"_s, context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"1,None"_s );
    wrapperOptional.setParameterValue( u"None,None"_s, context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"None,None"_s );
    wrapperOptional.setParameterValue( u"None"_s, context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"None,None"_s );
    wrapperOptional.setParameterValue( QVariant(), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), u"None,None"_s );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"range"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterRange rangeParam( u"n"_s, u"test desc"_s, Qgis::ProcessingNumberParameterType::Integer, u"0,255"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"range"_s, context, widgetContext, &rangeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterRange *>( def.get() )->defaultValue().toString(), u"0,255"_s );
  QCOMPARE( static_cast<QgsProcessingParameterRange *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Integer );
  rangeParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  rangeParam.setDataType( Qgis::ProcessingNumberParameterType::Double );
  rangeParam.setDefaultValue( u"0,1"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"range"_s, context, widgetContext, &rangeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterRange *>( def.get() )->defaultValue().toString(), u"0,1"_s );
  QCOMPARE( static_cast<QgsProcessingParameterRange *>( def.get() )->dataType(), Qgis::ProcessingNumberParameterType::Double );
}

void TestProcessingGui::testMatrixDialog()
{
  QgsProcessingParameterMatrix matrixParam( QString(), QString(), 3, false, QStringList() << u"a"_s << u"b"_s );
  auto dlg = std::make_unique<QgsProcessingMatrixParameterPanelWidget>( nullptr, &matrixParam );
  // variable length table
  QVERIFY( dlg->mButtonAdd->isEnabled() );
  QVERIFY( dlg->mButtonRemove->isEnabled() );
  QVERIFY( dlg->mButtonRemoveAll->isEnabled() );

  QCOMPARE( dlg->table(), QVariantList() );

  dlg = std::make_unique<QgsProcessingMatrixParameterPanelWidget>( nullptr, &matrixParam, QVariantList() << u"a"_s << u"b"_s << u"c"_s << u"d"_s << u"e"_s << u"f"_s );
  QCOMPARE( dlg->table(), QVariantList() << u"a"_s << u"b"_s << u"c"_s << u"d"_s << u"e"_s << u"f"_s );
  dlg->addRow();
  QCOMPARE( dlg->table(), QVariantList() << u"a"_s << u"b"_s << u"c"_s << u"d"_s << u"e"_s << u"f"_s << QString() << QString() );
  dlg->deleteAllRows();
  QCOMPARE( dlg->table(), QVariantList() );

  QgsProcessingParameterMatrix matrixParam2( QString(), QString(), 3, true, QStringList() << u"a"_s << u"b"_s );
  dlg = std::make_unique<QgsProcessingMatrixParameterPanelWidget>( nullptr, &matrixParam2, QVariantList() << u"a"_s << u"b"_s << u"c"_s << u"d"_s << u"e"_s << u"f"_s );
  QVERIFY( !dlg->mButtonAdd->isEnabled() );
  QVERIFY( !dlg->mButtonRemove->isEnabled() );
  QVERIFY( !dlg->mButtonRemoveAll->isEnabled() );
}

void TestProcessingGui::testMatrixWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingContext context;

    QgsProcessingParameterMatrix param( u"matrix"_s, u"matrix"_s, 3, false, QStringList() << u"a"_s << u"b"_s );
    param.setDefaultValue( u"0.0,100.0,150.0,250.0"_s );
    QgsProcessingMatrixWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( w );

    // initial value
    QCOMPARE( wrapper.parameterValue().toList(), QVariantList() << u"0.0"_s << u"100.0"_s << u"150.0"_s << u"250.0"_s );

    QSignalSpy spy( &wrapper, &QgsProcessingMatrixWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << 5 << 7, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << u"5"_s << u"7"_s );
    QCOMPARE( wrapper.mMatrixWidget->value(), QVariantList() << u"5"_s << u"7"_s );
    wrapper.setWidgetValue( u"28.1,36.5,5.5,8.9"_s, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << u"28.1"_s << u"36.5"_s << u"5.5"_s << u"8.9"_s );
    QCOMPARE( wrapper.mMatrixWidget->value(), QVariantList() << u"28.1"_s << u"36.5"_s << u"5.5"_s << u"8.9"_s );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"matrix"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    wrapper.mMatrixWidget->setValue( QVariantList() << u"7"_s << u"9"_s );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << u"7"_s << u"9"_s );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"matrix"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterMatrix matrixParam( u"n"_s, u"test desc"_s, 1, false, QStringList() << "A" << "B" << "C", QVariantList() << 0 << 0 << 0 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"matrix"_s, context, widgetContext, &matrixParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterMatrix *>( def.get() )->headers(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast<QgsProcessingParameterMatrix *>( def.get() )->defaultValue().toStringList(), QStringList() << "0" << "0" << "0" );
  QVERIFY( !static_cast<QgsProcessingParameterMatrix *>( def.get() )->hasFixedNumberRows() );
  matrixParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  matrixParam.setHasFixedNumberRows( true );
  matrixParam.setDefaultValue( QVariantList() << 1 << 2 << 3 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"matrix"_s, context, widgetContext, &matrixParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterMatrix *>( def.get() )->headers(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast<QgsProcessingParameterMatrix *>( def.get() )->defaultValue().toStringList(), QStringList() << "1" << "2" << "3" );
  QVERIFY( static_cast<QgsProcessingParameterMatrix *>( def.get() )->hasFixedNumberRows() );
}

void TestProcessingGui::testExpressionWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( u"native:centroids"_s );
  const QgsProcessingParameterDefinition *vLayerDef = centroidAlg->parameterDefinition( u"INPUT"_s );
  const QgsProcessingParameterDefinition *pcLayerDef = new QgsProcessingParameterPointCloudLayer( "INPUT", u"input"_s, QVariant(), false );

  auto testWrapper = [vLayerDef, pcLayerDef]( Qgis::ProcessingMode type ) {
    QgsProcessingParameterExpression param( u"expression"_s, u"expression"_s );

    QgsProcessingExpressionWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingExpressionWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"1+2"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"1+2"_s );
    QCOMPARE( static_cast<QgsExpressionLineEdit *>( wrapper.wrappedWidget() )->expression(), u"1+2"_s );
    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast<QgsExpressionLineEdit *>( wrapper.wrappedWidget() )->expression().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"expression"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsExpressionLineEdit *>( wrapper.wrappedWidget() )->setExpression( u"3+4"_s );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( u"other"_s );
    QgsProcessingExpressionWidgetWrapper wrapper2( &param, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingExpressionWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"11+12"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"11+12"_s );
    QCOMPARE( static_cast<QgsFieldExpressionWidget *>( wrapper2.wrappedWidget() )->expression(), u"11+12"_s );

    wrapper2.setWidgetValue( QString(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( wrapper2.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast<QgsFieldExpressionWidget *>( wrapper2.wrappedWidget() )->expression().isEmpty() );

    static_cast<QgsFieldExpressionWidget *>( wrapper2.wrappedWidget() )->setExpression( u"3+4"_s );
    QCOMPARE( spy2.count(), 3 );

    TestLayerWrapper vLayerWrapper( vLayerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( u"LineString"_s, u"x"_s, u"memory"_s );
    p.addMapLayer( vl );

    QVERIFY( !wrapper2.mFieldExpWidget->layer() );
    vLayerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper2.setParentLayerWrapperValue( &vLayerWrapper );
    QCOMPARE( wrapper2.mFieldExpWidget->layer(), vl );

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    vLayerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &vLayerWrapper );
    QVERIFY( !wrapper2.mFieldExpWidget->layer() );

    vLayerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &vLayerWrapper );
    QVERIFY( !wrapper2.mFieldExpWidget->layer() );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    vLayerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &vLayerWrapper );
    QCOMPARE( wrapper2.mFieldExpWidget->layer(), vl );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointFileName = TEST_DATA_DIR + u"/points.shp"_s;
    vLayerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &vLayerWrapper );
    QCOMPARE( wrapper2.mFieldExpWidget->layer()->publicSource(), pointFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );

    //
    // point cloud expression
    //
    param.setExpressionType( Qgis::ExpressionType::PointCloud );
    param.setParentLayerParameterName( u"pointcloud"_s );
    QgsProcessingExpressionWidgetWrapper wrapper3( &param, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingExpressionWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"Intensity+100"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"Intensity+100"_s );
    QCOMPARE( static_cast<QgsProcessingPointCloudExpressionLineEdit *>( wrapper3.wrappedWidget() )->expression(), u"Intensity+100"_s );
    wrapper3.setWidgetValue( QString(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( wrapper3.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast<QgsProcessingPointCloudExpressionLineEdit *>( wrapper3.wrappedWidget() )->expression().isEmpty() );

    // check signal
    static_cast<QgsProcessingPointCloudExpressionLineEdit *>( wrapper3.wrappedWidget() )->setExpression( u"Red+4"_s );
    QCOMPARE( spy3.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( u"other"_s );
    QgsProcessingExpressionWidgetWrapper wrapper4( &param, type );
    w = wrapper4.createWrappedWidget( context );

    QSignalSpy spy4( &wrapper4, &QgsProcessingExpressionWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( u"Intensity+100"_s, context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toString(), u"Intensity+100"_s );
    QCOMPARE( static_cast<QgsProcessingPointCloudExpressionLineEdit *>( wrapper4.wrappedWidget() )->expression(), u"Intensity+100"_s );

    wrapper4.setWidgetValue( QString(), context );
    QCOMPARE( spy4.count(), 2 );
    QVERIFY( wrapper4.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast<QgsProcessingPointCloudExpressionLineEdit *>( wrapper4.wrappedWidget() )->expression().isEmpty() );

    static_cast<QgsProcessingPointCloudExpressionLineEdit *>( wrapper4.wrappedWidget() )->setExpression( u"Red+4"_s );
    QCOMPARE( spy4.count(), 3 );

    TestLayerWrapper pcLayerWrapper( pcLayerDef );
    QgsPointCloudLayer *pcl = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/copc/rgb.copc.laz", u"x"_s, u"copc"_s );
    p.addMapLayer( pcl );

    QVERIFY( !wrapper4.mPointCloudExpLineEdit->layer() );
    pcLayerWrapper.setWidgetValue( QVariant::fromValue( pcl ), context );
    wrapper4.setParentLayerWrapperValue( &pcLayerWrapper );
    QCOMPARE( wrapper4.mPointCloudExpLineEdit->layer(), pcl );

    // should not be owned by wrapper
    QVERIFY( !wrapper4.mParentLayer.get() );
    pcLayerWrapper.setWidgetValue( QVariant(), context );
    wrapper4.setParentLayerWrapperValue( &pcLayerWrapper );
    QVERIFY( !wrapper4.mPointCloudExpLineEdit->layer() );

    pcLayerWrapper.setWidgetValue( pcl->id(), context );
    wrapper4.setParentLayerWrapperValue( &pcLayerWrapper );
    QVERIFY( !wrapper4.mPointCloudExpLineEdit->layer() );
    QVERIFY( !wrapper4.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator2( context );
    wrapper4.registerProcessingContextGenerator( &generator2 );

    pcLayerWrapper.setWidgetValue( pcl->id(), context );
    wrapper4.setParentLayerWrapperValue( &pcLayerWrapper );
    QCOMPARE( wrapper4.mPointCloudExpLineEdit->layer(), pcl );
    QVERIFY( !wrapper4.mParentLayer.get() );

    // non-project layer
    QString pointCloudFileName = TEST_DATA_DIR + u"/point_clouds/copc/sunshine-coast.copc.laz"_s;
    pcLayerWrapper.setWidgetValue( pointCloudFileName, context );
    wrapper4.setParentLayerWrapperValue( &pcLayerWrapper );
    QCOMPARE( wrapper4.mPointCloudExpLineEdit->layer()->publicSource(), pointCloudFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper4.mParentLayer->publicSource(), pointCloudFileName );
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"expression"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterExpression *>( def.get() )->expressionType(), Qgis::ExpressionType::Qgis );

  // using a parameter definition as initial values
  QgsProcessingParameterExpression exprParam( u"n"_s, u"test desc"_s, QVariant(), u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"expression"_s, context, widgetContext, &exprParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterExpression *>( def.get() )->parentLayerParameterName(), u"parent"_s );
  QCOMPARE( static_cast<QgsProcessingParameterExpression *>( def.get() )->expressionType(), Qgis::ExpressionType::Qgis );
  exprParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  exprParam.setExpressionType( Qgis::ExpressionType::PointCloud );
  exprParam.setParentLayerParameterName( QString() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"expression"_s, context, widgetContext, &exprParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QVERIFY( static_cast<QgsProcessingParameterExpression *>( def.get() )->parentLayerParameterName().isEmpty() );
  QCOMPARE( static_cast<QgsProcessingParameterExpression *>( def.get() )->expressionType(), Qgis::ExpressionType::PointCloud );
}

void TestProcessingGui::testFieldSelectionPanel()
{
  QgsProcessingParameterField fieldParam( QString(), QString(), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true );
  QgsProcessingFieldPanelWidget w( nullptr, &fieldParam );
  QSignalSpy spy( &w, &QgsProcessingFieldPanelWidget::changed );

  QCOMPARE( w.mLineEdit->text(), u"0 field(s) selected"_s );
  w.setValue( u"aa"_s );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.value().toList(), QVariantList() << u"aa"_s );
  QCOMPARE( w.mLineEdit->text(), u"aa"_s );

  w.setValue( QVariantList() << u"bb"_s << u"aa"_s );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << u"bb"_s << u"aa"_s );
  QCOMPARE( w.mLineEdit->text(), u"bb,aa"_s );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), u"0 field(s) selected"_s );

  // ensure that settings fields invalidates value and removes values that don't
  // exists in the fields, see https://github.com/qgis/QGIS/issues/39351
  w.setValue( QVariantList() << u"bb"_s << u"aa"_s << u"cc"_s );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( w.value().toList(), QVariantList() << u"bb"_s << u"aa"_s << u"cc"_s );
  QCOMPARE( w.mLineEdit->text(), u"bb,aa,cc"_s );

  QgsFields fields;
  fields.append( QgsField( u"aa"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"cc"_s, QMetaType::Type::Int ) );
  w.setFields( fields );
  QCOMPARE( spy.count(), 5 );
  QCOMPARE( w.value().toList(), QVariantList() << u"aa"_s << u"cc"_s );
  QCOMPARE( w.mLineEdit->text(), u"aa,cc"_s );
}

void TestProcessingGui::testFieldWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( u"native:centroids"_s );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( u"INPUT"_s );

  auto testWrapper = [layerDef]( Qgis::ProcessingMode type ) {
    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( u"LineString?field=aaa:int&field=bbb:string"_s, u"x"_s, u"memory"_s );
    p.addMapLayer( vl );

    QgsProcessingParameterField param( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s );

    QgsProcessingFieldWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void ) w;
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper.setParentLayerWrapperValue( &layerWrapper );

    QSignalSpy spy( &wrapper, &QgsProcessingFieldWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bbb"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"bbb"_s );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper.wrappedWidget() )->currentField(), u"bbb"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"bbb"_s );
        break;
    }

    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );

    delete w;

    // optional
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true );

    QgsProcessingFieldWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy2( &wrapper2, &QgsProcessingFieldWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"aaa"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"aaa"_s );

    wrapper2.setWidgetValue( QString(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( wrapper2.widgetValue().toString().isEmpty() );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper2.wrappedWidget() )->currentField(), QString() );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), QString() );
        break;
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"field [optional]"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        static_cast<QgsFieldComboBox *>( wrapper2.wrappedWidget() )->setField( u"bbb"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->setText( u"bbb"_s );
        break;
    }

    QCOMPARE( spy2.count(), 3 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), vl );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), vl );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointFileName = TEST_DATA_DIR + u"/points.shp"_s;
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer()->publicSource(), pointFileName );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );

    delete w;

    // multiple
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true );

    QgsProcessingFieldWidgetWrapper wrapper3( &param, type );

    w = wrapper3.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper3.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy3( &wrapper3, &QgsProcessingFieldWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"aaa"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << u"aaa"_s );

    wrapper3.setWidgetValue( QString(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( wrapper3.widgetValue().toString().isEmpty() );

    wrapper3.setWidgetValue( u"aaa;bbb"_s, context );
    QCOMPARE( spy3.count(), 3 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << u"aaa"_s << u"bbb"_s );

    delete w;

    // filtering fields
    QgsFields f;
    f.append( QgsField( u"string"_s, QMetaType::Type::QString ) );
    f.append( QgsField( u"double"_s, QMetaType::Type::Double ) );
    f.append( QgsField( u"int"_s, QMetaType::Type::Int ) );
    f.append( QgsField( u"date"_s, QMetaType::Type::QDate ) );
    f.append( QgsField( u"time"_s, QMetaType::Type::QTime ) );
    f.append( QgsField( u"datetime"_s, QMetaType::Type::QDateTime ) );
    f.append( QgsField( u"binary"_s, QMetaType::Type::QByteArray ) );
    f.append( QgsField( u"boolean"_s, QMetaType::Type::Bool ) );

    QgsFields f2 = wrapper3.filterFields( f );
    QCOMPARE( f2, f );

    // string fields
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::String, false, true );
    QgsProcessingFieldWidgetWrapper wrapper4( &param, type );
    w = wrapper4.createWrappedWidget( context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper4.wrappedWidget() )->filters(), QgsFieldProxyModel::String );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    f2 = wrapper4.filterFields( f );
    QCOMPARE( f2.size(), 1 );
    QCOMPARE( f2.at( 0 ).name(), u"string"_s );
    delete w;

    // string, multiple
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::String, true, true );
    QgsProcessingFieldWidgetWrapper wrapper4a( &param, type );
    w = wrapper4a.createWrappedWidget( context );
    wrapper4a.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper4a.mPanel->fields().count(), 1 );
        QCOMPARE( wrapper4a.mPanel->fields().at( 0 ).name(), u"bbb"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    delete w;

    // numeric fields
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Numeric, false, true );
    QgsProcessingFieldWidgetWrapper wrapper5( &param, type );
    w = wrapper5.createWrappedWidget( context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper5.wrappedWidget() )->filters(), QgsFieldProxyModel::Numeric );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    f2 = wrapper5.filterFields( f );
    QCOMPARE( f2.size(), 2 );
    QCOMPARE( f2.at( 0 ).name(), u"double"_s );
    QCOMPARE( f2.at( 1 ).name(), u"int"_s );

    delete w;

    // numeric, multiple
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Numeric, true, true );
    QgsProcessingFieldWidgetWrapper wrapper5a( &param, type );
    w = wrapper5a.createWrappedWidget( context );
    wrapper5a.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper5a.mPanel->fields().count(), 1 );
        QCOMPARE( wrapper5a.mPanel->fields().at( 0 ).name(), u"aaa"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    delete w;

    // datetime fields
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::DateTime, false, true );
    QgsProcessingFieldWidgetWrapper wrapper6( &param, type );
    w = wrapper6.createWrappedWidget( context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper6.wrappedWidget() )->filters(), QgsFieldProxyModel::Date | QgsFieldProxyModel::Time | QgsFieldProxyModel::DateTime );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    f2 = wrapper6.filterFields( f );
    QCOMPARE( f2.size(), 3 );
    QCOMPARE( f2.at( 0 ).name(), u"date"_s );
    QCOMPARE( f2.at( 1 ).name(), u"time"_s );
    QCOMPARE( f2.at( 2 ).name(), u"datetime"_s );

    // binary fields
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Binary, false, true );
    QgsProcessingFieldWidgetWrapper wrapper7( &param, type );
    w = wrapper7.createWrappedWidget( context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper7.wrappedWidget() )->filters(), QgsFieldProxyModel::Binary );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    f2 = wrapper7.filterFields( f );
    QCOMPARE( f2.size(), 1 );
    QCOMPARE( f2.at( 0 ).name(), u"binary"_s );
    delete w;

    // boolean fields
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Boolean, false, true );
    QgsProcessingFieldWidgetWrapper wrapper8( &param, type );
    w = wrapper8.createWrappedWidget( context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsFieldComboBox *>( wrapper8.wrappedWidget() )->filters(), QgsFieldProxyModel::Boolean );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    f2 = wrapper8.filterFields( f );
    QCOMPARE( f2.size(), 1 );
    QCOMPARE( f2.at( 0 ).name(), u"boolean"_s );
    delete w;

    // default to all fields
    param = QgsProcessingParameterField( u"field"_s, u"field"_s, QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true );
    param.setDefaultToAllFields( true );
    QgsProcessingFieldWidgetWrapper wrapper9( &param, type );
    w = wrapper9.createWrappedWidget( context );
    wrapper9.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper9.widgetValue().toList(), QVariantList() << u"aaa"_s << u"bbb"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    delete w;

    // MultipleLayers as parent layer
    QgsVectorLayer *vl2 = new QgsVectorLayer( u"LineString?field=bbb:string"_s, u"y"_s, u"memory"_s );
    p.addMapLayer( vl2 );

    QgsProcessingFieldWidgetWrapper wrapper10( &param, type );
    wrapper10.registerProcessingContextGenerator( &generator );
    w = wrapper10.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariantList() << vl->id() << vl2->id(), context );
    wrapper10.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper10.widgetValue().toList(), QVariantList() << u"bbb"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"field"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !def->defaultValue().isValid() );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterField fieldParam( u"n"_s, u"test desc"_s, u"field_name"_s, u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"field"_s, context, widgetContext, &fieldParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->defaultValue().toString(), u"field_name"_s );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->parentLayerParameterName(), u"parent"_s );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->dataType(), Qgis::ProcessingFieldParameterDataType::Any );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->allowMultiple(), false );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->defaultToAllFields(), false );
  fieldParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  fieldParam.setParentLayerParameterName( QString() );
  fieldParam.setAllowMultiple( true );
  fieldParam.setDefaultToAllFields( true );
  fieldParam.setDataType( Qgis::ProcessingFieldParameterDataType::String );
  fieldParam.setDefaultValue( u"field_1;field_2"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"field"_s, context, widgetContext, &fieldParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->defaultValue().toString(), u"field_1;field_2"_s );
  QVERIFY( static_cast<QgsProcessingParameterBand *>( def.get() )->parentLayerParameterName().isEmpty() );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->dataType(), Qgis::ProcessingFieldParameterDataType::String );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->allowMultiple(), true );
  QCOMPARE( static_cast<QgsProcessingParameterField *>( def.get() )->defaultToAllFields(), true );
}

void TestProcessingGui::testMultipleSelectionDialog()
{
  QVariantList availableOptions;
  QVariantList selectedOptions;
  auto dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, selectedOptions );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );

  auto vl = std::make_unique<QgsVectorLayer>( u"LineString"_s, u"x"_s, u"memory"_s );
  availableOptions << QVariant( "aa" ) << QVariant( 15 ) << QVariant::fromValue( vl.get() );
  dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, selectedOptions );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  QCOMPARE( dlg->mModel->rowCount(), 3 );
  dlg->selectAll( true );
  QCOMPARE( dlg->selectedOptions(), availableOptions );
  dlg->mModel->item( 1 )->setCheckState( Qt::Unchecked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "aa" << QVariant::fromValue( vl.get() ) );

  // reorder rows
  QList<QStandardItem *> itemList = dlg->mModel->takeRow( 2 );
  dlg->mModel->insertRow( 0, itemList );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << QVariant::fromValue( vl.get() ) << "aa" );

  // additional options
  availableOptions.clear();
  selectedOptions << QVariant( "bb" ) << QVariant( 6.6 );
  dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->selectedOptions(), selectedOptions );
  dlg->mModel->item( 1 )->setCheckState( Qt::Unchecked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "bb" );

  // mix of standard and additional options
  availableOptions << QVariant( 6.6 ) << QVariant( "aa" );
  dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->rowCount(), 3 );
  QCOMPARE( dlg->selectedOptions(), selectedOptions ); // order must be maintained!
  dlg->mModel->item( 2 )->setCheckState( Qt::Checked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "bb" << QVariant( 6.6 ) << QVariant( "aa" ) );

  // selection buttons
  selectedOptions.clear();
  availableOptions = QVariantList() << QVariant( "a" ) << QVariant( "b" ) << QVariant( "c" );
  dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, selectedOptions );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  dlg->mSelectionList->selectionModel()->select( dlg->mModel->index( 1, 0 ), QItemSelectionModel::ClearAndSelect );
  // without a multi-selection, select all/toggle options should affect all items
  dlg->selectAll( true );
  QCOMPARE( dlg->selectedOptions(), availableOptions );
  dlg->selectAll( false );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  dlg->toggleSelection();
  QCOMPARE( dlg->selectedOptions(), availableOptions );
  dlg->toggleSelection();
  QVERIFY( dlg->selectedOptions().isEmpty() );
  // with multi-selection, actions should only affected selected rows
  dlg->mSelectionList->selectionModel()->select( dlg->mModel->index( 2, 0 ), QItemSelectionModel::Select );
  dlg->selectAll( true );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "b" << "c" );
  dlg->selectAll( false );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  dlg->selectAll( true );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "b" << "c" );
  dlg->mModel->item( 0 )->setCheckState( Qt::Checked );
  dlg->selectAll( false );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "a" );
  dlg->toggleSelection();
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "a" << "b" << "c" );
  dlg->toggleSelection();
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "a" );

  // text format
  availableOptions = QVariantList() << QVariant( "a" ) << 6 << 6.2;
  dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->item( 0 )->text(), u"a"_s );
  QCOMPARE( dlg->mModel->item( 1 )->text(), u"6"_s );
  QCOMPARE( dlg->mModel->item( 2 )->text(), u"6.2"_s );
  dlg->setValueFormatter( []( const QVariant &v ) -> QString {
    return v.toString() + '_';
  } );
  QCOMPARE( dlg->mModel->item( 0 )->text(), u"a_"_s );
  QCOMPARE( dlg->mModel->item( 1 )->text(), u"6_"_s );
  QCOMPARE( dlg->mModel->item( 2 )->text(), u"6.2_"_s );

  // mix of fixed + model choices
  availableOptions = QVariantList() << QVariant( "a" ) << 6 << 6.2
                                    << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) )
                                    << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromModelParameter( u"input"_s ) );
  dlg = std::make_unique<QgsProcessingMultipleSelectionPanelWidget>( availableOptions, QVariantList() << 6 << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) ) << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromModelParameter( u"input"_s ) ) );

  // when any selected option is a model child parameter source, then we require that all options are upgraded in place to model child parameter sources
  QVariantList res = dlg->selectedOptions();
  QCOMPARE( res.size(), 3 );
  QCOMPARE( res.at( 0 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromStaticValue( 6 ) );
  QCOMPARE( res.at( 1 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) );
  QCOMPARE( res.at( 2 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromModelParameter( u"input"_s ) );
  dlg->selectAll( true );
  res = dlg->selectedOptions();
  QCOMPARE( res.size(), 5 );
  QCOMPARE( res.at( 0 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromStaticValue( 6 ) );
  QCOMPARE( res.at( 1 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) );
  QCOMPARE( res.at( 2 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromModelParameter( u"input"_s ) );
  QCOMPARE( res.at( 3 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromStaticValue( u"a"_s ) );
  QCOMPARE( res.at( 4 ).value<QgsProcessingModelChildParameterSource>(), QgsProcessingModelChildParameterSource::fromStaticValue( 6.2 ) );
}

void TestProcessingGui::testMultipleFileSelectionDialog()
{
  auto param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::Raster );
  QVariantList selectedOptions;
  auto dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), selectedOptions, QList<QgsProcessingModelChildParameterSource>() );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );

  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( u"Point"_s, u"point"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( u"LineString"_s, u"line"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( u"Polygon"_s, u"polygon"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( u"None"_s, u"nogeom"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( noGeom );
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", u"mesh"_s, u"mdal"_s );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  QgsProject::instance()->addMapLayer( mesh );
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", u"raster"_s );
  QgsProject::instance()->addMapLayer( raster );
  DummyPluginLayer *plugin = new DummyPluginLayer( "dummylayer", "plugin" );
  QgsProject::instance()->addMapLayer( plugin );

#ifdef HAVE_EPT
  QgsPointCloudLayer *pointCloud = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", u"pointcloud"_s, u"ept"_s );
  QgsProject::instance()->addMapLayer( pointCloud );
#endif

  QgsAnnotationLayer *annotationLayer = new QgsAnnotationLayer( u"secondary annotations"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QgsProject::instance()->addMapLayer( annotationLayer );

  dlg->setProject( QgsProject::instance() );
  // should be filtered to raster layers only
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"raster [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->id() );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  // existing value using layer id should match to project layer
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList() << raster->id(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"raster [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->id() );
  QCOMPARE( dlg->selectedOptions().size(), 1 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), raster->id() );
  // existing value using layer source should also match to project layer
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList() << raster->source(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"raster [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->source() );
  QCOMPARE( dlg->selectedOptions().size(), 1 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), raster->source() );
  // existing value using full layer path not matching a project layer should work
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList() << raster->source() << QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"raster [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->source() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->selectedOptions().size(), 2 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), raster->source() );
  QCOMPARE( dlg->selectedOptions().at( 1 ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );

  // should remember layer order
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList() << QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) << raster->source(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), u"raster [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), raster->source() );
  QCOMPARE( dlg->selectedOptions().size(), 2 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->selectedOptions().at( 1 ).toString(), raster->source() );

  // mesh
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::Mesh );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"mesh"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), mesh->id() );

  // plugin
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::Plugin );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"plugin"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), plugin->id() );

#ifdef HAVE_EPT
  // point cloud
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::PointCloud );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"pointcloud [EPSG:28356]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), pointCloud->id() );
#endif

  // annotation
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::Annotation );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"secondary annotations"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), annotationLayer->id() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), u"Annotations"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), u"main"_s );

  // vector points
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::VectorPoint );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"point [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), point->id() );

  // vector lines
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::VectorLine );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"line [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), line->id() );

  // vector polygons
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::VectorPolygon );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"polygon [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), polygon->id() );

  // vector any geometry type
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::VectorAnyGeometry );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 3 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), u"line [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), line->id() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), u"point [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), point->id() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 2, 0 ) ).toString(), u"polygon [EPSG:4326]"_s );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 2, 0 ), Qt::UserRole ).toString(), polygon->id() );

  // vector any type
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::Vector );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 4 );
  QSet<QString> titles;
  for ( int i = 0; i < dlg->mModel->rowCount(); ++i )
    titles << dlg->mModel->data( dlg->mModel->index( i, 0 ) ).toString();
  QCOMPARE( titles, QSet<QString>() << u"polygon [EPSG:4326]"_s << u"point [EPSG:4326]"_s << u"line [EPSG:4326]"_s << u"nogeom"_s );

  // any type
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::MapLayer );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
#ifdef HAVE_EPT
  QCOMPARE( dlg->mModel->rowCount(), 10 );
#else
  QCOMPARE( dlg->mModel->rowCount(), 9 );
#endif

  titles.clear();
  for ( int i = 0; i < dlg->mModel->rowCount(); ++i )
    titles << dlg->mModel->data( dlg->mModel->index( i, 0 ) ).toString();
#ifdef HAVE_EPT
  QCOMPARE( titles, QSet<QString>() << u"polygon [EPSG:4326]"_s << u"point [EPSG:4326]"_s << u"line [EPSG:4326]"_s << u"nogeom"_s << u"raster [EPSG:4326]"_s << u"mesh"_s << u"plugin"_s << u"pointcloud [EPSG:28356]"_s << u"secondary annotations"_s << u"Annotations"_s );
#else
  QCOMPARE( titles, QSet<QString>() << u"polygon [EPSG:4326]"_s << u"point [EPSG:4326]"_s << u"line [EPSG:4326]"_s << u"nogeom"_s << u"raster [EPSG:4326]"_s << u"mesh"_s << u"plugin"_s << u"secondary annotations"_s << u"Annotations"_s );
#endif

  // files
  param = std::make_unique<QgsProcessingParameterMultipleLayers>( QString(), QString(), Qgis::ProcessingSourceType::File );
  dlg = std::make_unique<QgsProcessingMultipleInputPanelWidget>( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource>() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );
}

void TestProcessingGui::testRasterBandSelectionPanel()
{
  QgsProcessingParameterBand bandParam( QString(), QString(), QVariant(), u"INPUT"_s, false, true );
  QgsProcessingRasterBandPanelWidget w( nullptr, &bandParam );
  QSignalSpy spy( &w, &QgsProcessingRasterBandPanelWidget::changed );

  QCOMPARE( w.mLineEdit->text(), u"0 band(s) selected"_s );
  w.setValue( u"1"_s );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.value().toList(), QVariantList() << u"1"_s );
  QCOMPARE( w.mLineEdit->text(), u"1 band(s) selected"_s );

  w.setValue( QVariantList() << u"2"_s << u"1"_s );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << u"2"_s << u"1"_s );
  QCOMPARE( w.mLineEdit->text(), u"2 band(s) selected"_s );

  w.setValue( QVariantList() << 3 << 5 << 1 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() << 3 << 5 << 1 );
  QCOMPARE( w.mLineEdit->text(), u"3 band(s) selected"_s );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), u"0 band(s) selected"_s );
}

void TestProcessingGui::testBandWrapper()
{
  const QgsProcessingAlgorithm *statsAlg = QgsApplication::processingRegistry()->algorithmById( u"native:rasterlayerstatistics"_s );
  const QgsProcessingParameterDefinition *layerDef = statsAlg->parameterDefinition( u"INPUT"_s );

  auto testWrapper = [layerDef]( Qgis::ProcessingMode type ) {
    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsRasterLayer *rl = new QgsRasterLayer( TEST_DATA_DIR + u"/landsat.tif"_s, u"x"_s, u"gdal"_s );
    p.addMapLayer( rl );

    QgsProcessingParameterBand param( u"band"_s, u"band"_s, QVariant(), u"INPUT"_s );

    QgsProcessingBandWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void ) w;
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper.setParentLayerWrapperValue( &layerWrapper );

    QSignalSpy spy( &wrapper, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 3, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toInt(), 3 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsRasterBandComboBox *>( wrapper.wrappedWidget() )->currentBand(), 3 );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"3"_s );
        break;
    }

    wrapper.setWidgetValue( u"1"_s, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toInt(), 1 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsRasterBandComboBox *>( wrapper.wrappedWidget() )->currentBand(), 1 );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"1"_s );
        break;
    }

    delete w;

    // optional
    param = QgsProcessingParameterBand( u"band"_s, u"band"_s, QVariant(), u"INPUT"_s, true, false );

    QgsProcessingBandWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy2( &wrapper2, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"4"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toInt(), 4 );

    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( !wrapper2.widgetValue().isValid() );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsRasterBandComboBox *>( wrapper2.wrappedWidget() )->currentBand(), -1 );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), QString() );
        break;
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"band [optional]"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        static_cast<QgsRasterBandComboBox *>( wrapper2.wrappedWidget() )->setBand( 6 );
        break;

      case Qgis::ProcessingMode::Modeler:
        static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->setText( u"6"_s );
        break;
    }

    QCOMPARE( spy2.count(), 3 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), rl );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    layerWrapper.setWidgetValue( rl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    layerWrapper.setWidgetValue( rl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), rl );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString rasterFileName = TEST_DATA_DIR + u"/landsat-f32-b1.tif"_s;
    layerWrapper.setWidgetValue( rasterFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer()->publicSource(), rasterFileName );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), rasterFileName );

    delete w;

    // multiple
    param = QgsProcessingParameterBand( u"band"_s, u"band"_s, QVariant(), u"INPUT"_s, true, true );

    QgsProcessingBandWidgetWrapper wrapper3( &param, type );

    w = wrapper3.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper3.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy3( &wrapper3, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"5"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 5 );

    wrapper3.setWidgetValue( QString(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( wrapper3.widgetValue().toString().isEmpty() );

    wrapper3.setWidgetValue( u"3;4"_s, context );
    QCOMPARE( spy3.count(), 3 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << u"3"_s << u"4"_s );

    wrapper3.setWidgetValue( QVariantList() << 5 << 6 << 7, context );
    QCOMPARE( spy3.count(), 4 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 5 << 6 << 7 );

    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 5 );
    QVERIFY( !wrapper3.widgetValue().isValid() );


    // multiple non-optional
    param = QgsProcessingParameterBand( u"band"_s, u"band"_s, QVariant(), u"INPUT"_s, false, true );

    QgsProcessingBandWidgetWrapper wrapper4( &param, type );

    w = wrapper4.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper4.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy4( &wrapper4, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( u"5"_s, context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 5 );

    wrapper4.setWidgetValue( QString(), context );
    QCOMPARE( spy4.count(), 2 );
    QVERIFY( wrapper4.widgetValue().toString().isEmpty() );

    wrapper4.setWidgetValue( u"3;4"_s, context );
    QCOMPARE( spy4.count(), 3 );
    QCOMPARE( wrapper4.widgetValue().toStringList(), QStringList() << u"3"_s << u"4"_s );

    wrapper4.setWidgetValue( QVariantList() << 5 << 6 << 7, context );
    QCOMPARE( spy4.count(), 4 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 5 << 6 << 7 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"band"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterBand bandParam( u"n"_s, u"test desc"_s, 1, u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"band"_s, context, widgetContext, &bandParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterBand *>( def.get() )->defaultValue().toString(), u"1"_s );
  QCOMPARE( static_cast<QgsProcessingParameterBand *>( def.get() )->allowMultiple(), false );
  QCOMPARE( static_cast<QgsProcessingParameterBand *>( def.get() )->parentLayerParameterName(), u"parent"_s );
  bandParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  bandParam.setParentLayerParameterName( QString() );
  bandParam.setAllowMultiple( true );
  bandParam.setDefaultValue( QVariantList() << 2 << 3 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"band"_s, context, widgetContext, &bandParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterBand *>( def.get() )->defaultValue().toStringList(), QStringList() << "2" << "3" );
  QCOMPARE( static_cast<QgsProcessingParameterBand *>( def.get() )->allowMultiple(), true );
  QVERIFY( static_cast<QgsProcessingParameterBand *>( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testMultipleInputWrapper()
{
  QString path1 = TEST_DATA_DIR + u"/landsat-f32-b1.tif"_s;
  QString path2 = TEST_DATA_DIR + u"/landsat.tif"_s;

  auto testWrapper = [path1, path2]( Qgis::ProcessingMode type ) {
    QgsProcessingParameterMultipleLayers param( u"multi"_s, u"multi"_s, Qgis::ProcessingSourceType::Vector, QVariant(), false );

    QgsProcessingMultipleLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void ) w;

    QSignalSpy spy( &wrapper, &QgsProcessingMultipleLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << path1 << path2, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << path1 << path2 );
    QCOMPARE( static_cast<QgsProcessingMultipleLayerPanelWidget *>( wrapper.wrappedWidget() )->value().toList(), QVariantList() << path1 << path2 );

    wrapper.setWidgetValue( path1, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toStringList(), QStringList() << path1 );
    QCOMPARE( static_cast<QgsProcessingMultipleLayerPanelWidget *>( wrapper.wrappedWidget() )->value().toList(), QVariantList() << path1 );
    delete w;

    // optional
    param = QgsProcessingParameterMultipleLayers( u"multi"_s, u"multi"_s, Qgis::ProcessingSourceType::Vector, QVariant(), true );

    QgsProcessingMultipleLayerWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    QSignalSpy spy2( &wrapper2, &QgsProcessingMultipleLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( path2, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList(), QVariantList() << path2 );

    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( static_cast<QgsProcessingMultipleLayerPanelWidget *>( wrapper2.wrappedWidget() )->value().toList().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"multi [optional]"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsProcessingMultipleLayerPanelWidget *>( wrapper2.wrappedWidget() )->setValue( QVariantList() << path1 );
    QCOMPARE( spy2.count(), 3 );


    if ( wrapper.type() == Qgis::ProcessingMode::Modeler )
    {
      // different mix of sources

      wrapper2.setWidgetValue( QVariantList() << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg3"_s, u"OUTPUT"_s ) ) << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromModelParameter( u"p1"_s ) ) << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromStaticValue( u"something"_s ) ), context );
      QCOMPARE( wrapper2.widgetValue().toList().count(), 3 );

      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().outputChildId(), u"alg3"_s );
      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value<QgsProcessingModelChildParameterSource>().outputName(), u"OUTPUT"_s );
      QCOMPARE( wrapper2.widgetValue().toList().at( 1 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
      QCOMPARE( wrapper2.widgetValue().toList().at( 1 ).value<QgsProcessingModelChildParameterSource>().parameterName(), u"p1"_s );
      QCOMPARE( wrapper2.widgetValue().toList().at( 2 ).value<QgsProcessingModelChildParameterSource>().source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
      QCOMPARE( wrapper2.widgetValue().toList().at( 2 ).value<QgsProcessingModelChildParameterSource>().staticValue().toString(), u"something"_s );
      delete w;
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"multilayer"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterMultipleLayers layersParam( u"n"_s, u"test desc"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"multilayer"_s, context, widgetContext, &layersParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterMultipleLayers *>( def.get() )->layerType(), Qgis::ProcessingSourceType::VectorAnyGeometry );
  layersParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  layersParam.setLayerType( Qgis::ProcessingSourceType::Raster );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"multilayer"_s, context, widgetContext, &layersParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterMultipleLayers *>( def.get() )->layerType(), Qgis::ProcessingSourceType::Raster );
}

void TestProcessingGui::testEnumSelectionPanel()
{
  QgsProcessingParameterEnum enumParam( QString(), QString(), QStringList() << u"a"_s << u"b"_s << u"c"_s, true );
  QgsProcessingEnumPanelWidget w( nullptr, &enumParam );
  QSignalSpy spy( &w, &QgsProcessingEnumPanelWidget::changed );

  QCOMPARE( w.mLineEdit->text(), u"0 options selected"_s );
  w.setValue( 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.value().toList(), QVariantList() << 1 );
  QCOMPARE( w.mLineEdit->text(), u"b"_s );

  w.setValue( QVariantList() << 2 << 0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << 2 << 0 );
  QCOMPARE( w.mLineEdit->text(), u"c,a"_s );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), u"0 options selected"_s );

  // static strings
  QgsProcessingParameterEnum enumParam2( QString(), QString(), QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), false, true );
  QgsProcessingEnumPanelWidget w2( nullptr, &enumParam2 );
  QSignalSpy spy2( &w2, &QgsProcessingEnumPanelWidget::changed );

  QCOMPARE( w2.mLineEdit->text(), u"0 options selected"_s );
  w2.setValue( u"a"_s );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( w2.value().toList(), QVariantList() << u"a"_s );
  QCOMPARE( w2.mLineEdit->text(), u"a"_s );

  w2.setValue( QVariantList() << u"c"_s << u"a"_s );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( w2.value().toList(), QVariantList() << u"c"_s << u"a"_s );
  QCOMPARE( w2.mLineEdit->text(), u"c,a"_s );

  w2.setValue( QVariant() );
  QCOMPARE( spy2.count(), 3 );
  QCOMPARE( w2.value().toList(), QVariantList() );
  QCOMPARE( w2.mLineEdit->text(), u"0 options selected"_s );
}

void TestProcessingGui::testEnumCheckboxPanel()
{
  //single value
  QgsProcessingParameterEnum param( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, false );
  QgsProcessingEnumCheckboxPanelWidget panel( nullptr, &param );
  QSignalSpy spy( &panel, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel.value(), QVariant() );
  panel.setValue( 2 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( panel.value().toInt(), 2 );
  QVERIFY( !panel.mButtons[0]->isChecked() );
  QVERIFY( !panel.mButtons[1]->isChecked() );
  QVERIFY( panel.mButtons[2]->isChecked() );
  panel.setValue( 0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( panel.value().toInt(), 0 );
  QVERIFY( panel.mButtons[0]->isChecked() );
  QVERIFY( !panel.mButtons[1]->isChecked() );
  QVERIFY( !panel.mButtons[2]->isChecked() );
  panel.mButtons[1]->setChecked( true );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( panel.value().toInt(), 1 );
  panel.setValue( QVariantList() << 2 );
  QCOMPARE( spy.count(), 5 );
  QCOMPARE( panel.value().toInt(), 2 );
  QVERIFY( !panel.mButtons[0]->isChecked() );
  QVERIFY( !panel.mButtons[1]->isChecked() );
  QVERIFY( panel.mButtons[2]->isChecked() );

  // multiple value
  QgsProcessingParameterEnum param2( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true );
  QgsProcessingEnumCheckboxPanelWidget panel2( nullptr, &param2 );
  QSignalSpy spy2( &panel2, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel2.value().toList(), QVariantList() );
  panel2.setValue( 2 );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 2 );
  QVERIFY( !panel2.mButtons[0]->isChecked() );
  QVERIFY( !panel2.mButtons[1]->isChecked() );
  QVERIFY( panel2.mButtons[2]->isChecked() );
  panel2.setValue( QVariantList() << 0 << 1 );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 0 << 1 );
  QVERIFY( panel2.mButtons[0]->isChecked() );
  QVERIFY( panel2.mButtons[1]->isChecked() );
  QVERIFY( !panel2.mButtons[2]->isChecked() );
  panel2.mButtons[0]->setChecked( false );
  QCOMPARE( spy2.count(), 3 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 1 );
  panel2.mButtons[2]->setChecked( true );
  QCOMPARE( spy2.count(), 4 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 1 << 2 );
  panel2.deselectAll();
  QCOMPARE( spy2.count(), 5 );
  QCOMPARE( panel2.value().toList(), QVariantList() );
  panel2.selectAll();
  QCOMPARE( spy2.count(), 6 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 0 << 1 << 2 );

  // multiple value optional
  QgsProcessingParameterEnum param3( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), true );
  QgsProcessingEnumCheckboxPanelWidget panel3( nullptr, &param3 );
  QSignalSpy spy3( &panel3, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel3.value().toList(), QVariantList() );
  panel3.setValue( 2 );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 2 );
  QVERIFY( !panel3.mButtons[0]->isChecked() );
  QVERIFY( !panel3.mButtons[1]->isChecked() );
  QVERIFY( panel3.mButtons[2]->isChecked() );
  panel3.setValue( QVariantList() << 0 << 1 );
  QCOMPARE( spy3.count(), 2 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 0 << 1 );
  QVERIFY( panel3.mButtons[0]->isChecked() );
  QVERIFY( panel3.mButtons[1]->isChecked() );
  QVERIFY( !panel3.mButtons[2]->isChecked() );
  panel3.mButtons[0]->setChecked( false );
  QCOMPARE( spy3.count(), 3 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 1 );
  panel3.mButtons[2]->setChecked( true );
  QCOMPARE( spy3.count(), 4 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 1 << 2 );
  panel3.deselectAll();
  QCOMPARE( spy3.count(), 5 );
  QCOMPARE( panel3.value().toList(), QVariantList() );
  panel3.selectAll();
  QCOMPARE( spy3.count(), 6 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 0 << 1 << 2 );
  panel3.setValue( QVariantList() );
  QCOMPARE( panel3.value().toList(), QVariantList() );
  QVERIFY( !panel3.mButtons[0]->isChecked() );
  QVERIFY( !panel3.mButtons[1]->isChecked() );
  QVERIFY( !panel3.mButtons[2]->isChecked() );
  QCOMPARE( spy3.count(), 7 );
  panel3.selectAll();
  QCOMPARE( spy3.count(), 8 );
  panel3.setValue( QVariant() );
  QCOMPARE( panel3.value().toList(), QVariantList() );
  QVERIFY( !panel3.mButtons[0]->isChecked() );
  QVERIFY( !panel3.mButtons[1]->isChecked() );
  QVERIFY( !panel3.mButtons[2]->isChecked() );
  QCOMPARE( spy3.count(), 9 );

  //single value using static strings
  QgsProcessingParameterEnum param4( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, false, QVariant(), false, true );
  QgsProcessingEnumCheckboxPanelWidget panel4( nullptr, &param4 );
  QSignalSpy spy4( &panel4, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel4.value(), QVariant() );
  panel4.setValue( u"c"_s );
  QCOMPARE( spy4.count(), 1 );
  QCOMPARE( panel4.value().toString(), u"c"_s );
  QVERIFY( !panel4.mButtons[0]->isChecked() );
  QVERIFY( !panel4.mButtons[1]->isChecked() );
  QVERIFY( panel4.mButtons[2]->isChecked() );
  panel4.setValue( u"a"_s );
  QCOMPARE( spy4.count(), 2 );
  QCOMPARE( panel4.value().toString(), u"a"_s );
  QVERIFY( panel4.mButtons[0]->isChecked() );
  QVERIFY( !panel4.mButtons[1]->isChecked() );
  QVERIFY( !panel4.mButtons[2]->isChecked() );
  panel4.mButtons[1]->setChecked( true );
  QCOMPARE( spy4.count(), 4 );
  QCOMPARE( panel4.value().toString(), u"b"_s );
  panel4.setValue( QVariantList() << u"c"_s );
  QCOMPARE( spy4.count(), 5 );
  QCOMPARE( panel4.value().toString(), u"c"_s );
  QVERIFY( !panel4.mButtons[0]->isChecked() );
  QVERIFY( !panel4.mButtons[1]->isChecked() );
  QVERIFY( panel4.mButtons[2]->isChecked() );

  // multiple value with static strings
  QgsProcessingParameterEnum param5( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), false, true );
  QgsProcessingEnumCheckboxPanelWidget panel5( nullptr, &param5 );
  QSignalSpy spy5( &panel5, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel5.value().toList(), QVariantList() );
  panel5.setValue( u"c"_s );
  QCOMPARE( spy5.count(), 1 );
  QCOMPARE( panel5.value().toList(), QVariantList() << u"c"_s );
  QVERIFY( !panel5.mButtons[0]->isChecked() );
  QVERIFY( !panel5.mButtons[1]->isChecked() );
  QVERIFY( panel5.mButtons[2]->isChecked() );
  panel5.setValue( QVariantList() << u"a"_s << u"b"_s );
  QCOMPARE( spy5.count(), 2 );
  QCOMPARE( panel5.value().toList(), QVariantList() << u"a"_s << u"b"_s );
  QVERIFY( panel5.mButtons[0]->isChecked() );
  QVERIFY( panel5.mButtons[1]->isChecked() );
  QVERIFY( !panel5.mButtons[2]->isChecked() );
  panel5.mButtons[0]->setChecked( false );
  QCOMPARE( spy5.count(), 3 );
  QCOMPARE( panel5.value().toList(), QVariantList() << u"b"_s );
  panel5.mButtons[2]->setChecked( true );
  QCOMPARE( spy5.count(), 4 );
  QCOMPARE( panel5.value().toList(), QVariantList() << u"b"_s << u"c"_s );
  panel5.deselectAll();
  QCOMPARE( spy5.count(), 5 );
  QCOMPARE( panel5.value().toList(), QVariantList() );
  panel5.selectAll();
  QCOMPARE( spy5.count(), 6 );
  QCOMPARE( panel5.value().toList(), QVariantList() << u"a"_s << u"b"_s << u"c"_s );

  // multiple value optional with statis strings
  QgsProcessingParameterEnum param6( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), true, true );
  QgsProcessingEnumCheckboxPanelWidget panel6( nullptr, &param6 );
  QSignalSpy spy6( &panel6, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel6.value().toList(), QVariantList() );
  panel6.setValue( u"c"_s );
  QCOMPARE( spy6.count(), 1 );
  QCOMPARE( panel6.value().toList(), QVariantList() << u"c"_s );
  QVERIFY( !panel6.mButtons[0]->isChecked() );
  QVERIFY( !panel6.mButtons[1]->isChecked() );
  QVERIFY( panel6.mButtons[2]->isChecked() );
  panel6.setValue( QVariantList() << u"a"_s << u"b"_s );
  QCOMPARE( spy6.count(), 2 );
  QCOMPARE( panel6.value().toList(), QVariantList() << u"a"_s << u"b"_s );
  QVERIFY( panel6.mButtons[0]->isChecked() );
  QVERIFY( panel6.mButtons[1]->isChecked() );
  QVERIFY( !panel6.mButtons[2]->isChecked() );
  panel6.mButtons[0]->setChecked( false );
  QCOMPARE( spy6.count(), 3 );
  QCOMPARE( panel6.value().toList(), QVariantList() << u"b"_s );
  panel6.mButtons[2]->setChecked( true );
  QCOMPARE( spy6.count(), 4 );
  QCOMPARE( panel6.value().toList(), QVariantList() << u"b"_s << u"c"_s );
  panel6.deselectAll();
  QCOMPARE( spy6.count(), 5 );
  QCOMPARE( panel6.value().toList(), QVariantList() );
  panel6.selectAll();
  QCOMPARE( spy6.count(), 6 );
  QCOMPARE( panel6.value().toList(), QVariantList() << u"a"_s << u"b"_s << u"c"_s );
  panel6.setValue( QVariantList() );
  QCOMPARE( panel6.value().toList(), QVariantList() );
  QVERIFY( !panel6.mButtons[0]->isChecked() );
  QVERIFY( !panel6.mButtons[1]->isChecked() );
  QVERIFY( !panel6.mButtons[2]->isChecked() );
  QCOMPARE( spy6.count(), 7 );
  panel6.selectAll();
  QCOMPARE( spy6.count(), 8 );
  panel6.setValue( QVariant() );
  QCOMPARE( panel6.value().toList(), QVariantList() );
  QVERIFY( !panel6.mButtons[0]->isChecked() );
  QVERIFY( !panel6.mButtons[1]->isChecked() );
  QVERIFY( !panel6.mButtons[2]->isChecked() );
  QCOMPARE( spy6.count(), 9 );
}

void TestProcessingGui::testEnumWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type, bool checkboxStyle = false ) {
    // non optional, single value
    QgsProcessingParameterEnum param( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, false );
    QVariantMap metadata;
    QVariantMap wrapperMetadata;
    wrapperMetadata.insert( u"useCheckBoxes"_s, true );
    metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
    if ( checkboxStyle )
      param.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 1, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toInt(), 1 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentText(), u"b"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper.wrappedWidget() )->value().toInt(), 1 );
    }
    wrapper.setWidgetValue( 0, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toInt(), 0 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentText(), u"a"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper.wrappedWidget() )->value().toInt(), 0 );
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"enum"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast<QComboBox *>( wrapper.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper.wrappedWidget() )->setValue( 2 );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // optional
    QgsProcessingParameterEnum param2( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, false, QVariant(), true );
    if ( checkboxStyle )
      param2.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper2( &param2, type );

    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( 1, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toInt(), 1 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentIndex(), 2 );
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"b"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper2.wrappedWidget() )->value().toInt(), 1 );
    }
    wrapper2.setWidgetValue( 0, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toInt(), 0 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"a"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper2.wrappedWidget() )->value().toInt(), 0 );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    if ( !checkboxStyle )
    {
      QVERIFY( !wrapper2.widgetValue().isValid() );
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"[Not selected]"_s );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast<QComboBox *>( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper2.wrappedWidget() )->setValue( 1 );
    QCOMPARE( spy2.count(), 4 );

    delete w;

    // allow multiple, non optional
    QgsProcessingParameterEnum param3( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), false );
    if ( checkboxStyle )
      param3.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper3( &param3, type );

    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( 1, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 1 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    wrapper3.setWidgetValue( 0, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 0 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    wrapper3.setWidgetValue( QVariantList() << 2 << 1, context );
    QCOMPARE( spy3.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 2 << 1 );
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 2 << 1 );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 1 << 2 );
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 1 << 2 );
    }
    // check signal
    if ( !checkboxStyle )
      static_cast<QgsProcessingEnumPanelWidget *>( wrapper3.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );
    else
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper3.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );

    QCOMPARE( spy3.count(), 4 );

    delete w;

    // allow multiple, optional
    QgsProcessingParameterEnum param4( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), true );
    if ( checkboxStyle )
      param4.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper4( &param4, type );

    w = wrapper4.createWrappedWidget( context );

    QSignalSpy spy4( &wrapper4, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( 1, context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 1 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    wrapper4.setWidgetValue( 0, context );
    QCOMPARE( spy4.count(), 2 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 0 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    wrapper4.setWidgetValue( QVariantList() << 2 << 1, context );
    QCOMPARE( spy4.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 2 << 1 );
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 2 << 1 );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 1 << 2 );
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 1 << 2 );
    }
    wrapper4.setWidgetValue( QVariantList(), context );
    QCOMPARE( spy4.count(), 4 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );

    wrapper4.setWidgetValue( QVariant(), context );
    QCOMPARE( spy4.count(), 5 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );

    // check signal
    if ( !checkboxStyle )
    {
      static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );
      QCOMPARE( spy4.count(), 6 );
      static_cast<QgsProcessingEnumPanelWidget *>( wrapper4.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy4.count(), 7 );
    }
    else
    {
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );
      QCOMPARE( spy4.count(), 6 );
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper4.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy4.count(), 7 );
    }

    delete w;

    // non optional, single with static strings
    QgsProcessingParameterEnum param5( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, false, QVariant(), false, true );
    if ( checkboxStyle )
      param5.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper5( &param5, type );

    w = wrapper5.createWrappedWidget( context );

    QSignalSpy spy5( &wrapper5, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper5.setWidgetValue( u"b"_s, context );
    QCOMPARE( spy5.count(), 1 );
    QCOMPARE( wrapper5.widgetValue().toString(), u"b"_s );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper5.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast<QComboBox *>( wrapper5.wrappedWidget() )->currentText(), u"b"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper5.wrappedWidget() )->value().toString(), u"b"_s );
    }
    wrapper5.setWidgetValue( u"a"_s, context );
    QCOMPARE( spy5.count(), 2 );
    QCOMPARE( wrapper5.widgetValue().toString(), u"a"_s );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper5.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast<QComboBox *>( wrapper5.wrappedWidget() )->currentText(), u"a"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper5.wrappedWidget() )->value().toString(), u"a"_s );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast<QComboBox *>( wrapper5.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper5.wrappedWidget() )->setValue( u"c"_s );
    QCOMPARE( spy5.count(), 3 );

    delete w;

    // single, optional with static strings
    QgsProcessingParameterEnum param6( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, false, QVariant(), true, true );
    if ( checkboxStyle )
      param6.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper6( &param6, type );

    w = wrapper6.createWrappedWidget( context );

    QSignalSpy spy6( &wrapper6, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper6.setWidgetValue( u"b"_s, context );
    QCOMPARE( spy6.count(), 1 );
    QCOMPARE( wrapper6.widgetValue().toString(), u"b"_s );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper6.wrappedWidget() )->currentIndex(), 2 );
      QCOMPARE( static_cast<QComboBox *>( wrapper6.wrappedWidget() )->currentText(), u"b"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper6.wrappedWidget() )->value().toString(), u"b"_s );
    }
    wrapper6.setWidgetValue( u"a"_s, context );
    QCOMPARE( spy6.count(), 2 );
    QCOMPARE( wrapper6.widgetValue().toString(), u"a"_s );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper6.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast<QComboBox *>( wrapper6.wrappedWidget() )->currentText(), u"a"_s );
    }
    else
    {
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper6.wrappedWidget() )->value().toString(), u"a"_s );
    }
    wrapper6.setWidgetValue( QVariant(), context );
    QCOMPARE( spy6.count(), 3 );
    if ( !checkboxStyle )
    {
      QVERIFY( !wrapper6.widgetValue().isValid() );
      QCOMPARE( static_cast<QComboBox *>( wrapper6.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast<QComboBox *>( wrapper6.wrappedWidget() )->currentText(), u"[Not selected]"_s );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast<QComboBox *>( wrapper6.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper6.wrappedWidget() )->setValue( u"a"_s );
    QCOMPARE( spy6.count(), 4 );

    delete w;

    // multiple, non optional with static strings
    QgsProcessingParameterEnum param7( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), false, true );
    if ( checkboxStyle )
      param7.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper7( &param7, type );

    w = wrapper7.createWrappedWidget( context );

    QSignalSpy spy7( &wrapper7, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper7.setWidgetValue( u"b"_s, context );
    QCOMPARE( spy7.count(), 1 );
    QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << u"b"_s );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << u"b"_s );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << u"b"_s );
    wrapper7.setWidgetValue( u"a"_s, context );
    QCOMPARE( spy7.count(), 2 );
    QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << u"a"_s );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << u"a"_s );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << u"a"_s );
    wrapper7.setWidgetValue( QVariantList() << u"c"_s << u"b"_s, context );
    QCOMPARE( spy7.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << u"c"_s << u"b"_s );
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << u"c"_s << u"b"_s );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << u"b"_s << u"c"_s );
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << u"b"_s << u"c"_s );
    }
    // check signal
    if ( !checkboxStyle )
      static_cast<QgsProcessingEnumPanelWidget *>( wrapper7.wrappedWidget() )->setValue( QVariantList() << u"a"_s << u"b"_s );
    else
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper7.wrappedWidget() )->setValue( QVariantList() << u"a"_s << u"b"_s );

    QCOMPARE( spy7.count(), 4 );

    delete w;

    // multiple, optional with static strings
    QgsProcessingParameterEnum param8( u"enum"_s, u"enum"_s, QStringList() << u"a"_s << u"b"_s << u"c"_s, true, QVariant(), true, true );
    if ( checkboxStyle )
      param8.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper8( &param8, type );

    w = wrapper8.createWrappedWidget( context );

    QSignalSpy spy8( &wrapper8, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper8.setWidgetValue( u"b"_s, context );
    QCOMPARE( spy8.count(), 1 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << u"b"_s );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << u"b"_s );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << u"b"_s );
    wrapper8.setWidgetValue( u"a"_s, context );
    QCOMPARE( spy8.count(), 2 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << u"a"_s );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << u"a"_s );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << u"a"_s );
    wrapper8.setWidgetValue( QVariantList() << u"c"_s << u"b"_s, context );
    QCOMPARE( spy8.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << u"c"_s << u"b"_s );
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << u"c"_s << u"b"_s );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << u"b"_s << u"c"_s );
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << u"b"_s << u"c"_s );
    }
    wrapper8.setWidgetValue( QVariantList(), context );
    QCOMPARE( spy8.count(), 4 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );

    wrapper8.setWidgetValue( QVariant(), context );
    QCOMPARE( spy8.count(), 5 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );

    // check signal
    if ( !checkboxStyle )
    {
      static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->setValue( QVariantList() << u"a"_s << u"b"_s );
      QCOMPARE( spy8.count(), 6 );
      static_cast<QgsProcessingEnumPanelWidget *>( wrapper8.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy8.count(), 7 );
    }
    else
    {
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->setValue( QVariantList() << u"a"_s << u"b"_s );
      QCOMPARE( spy8.count(), 6 );
      static_cast<QgsProcessingEnumCheckboxPanelWidget *>( wrapper8.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy8.count(), 7 );
    }

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // checkbox style (not for batch or model mode!)
  testWrapper( Qgis::ProcessingMode::Standard, true );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"enum"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterEnum enumParam( u"n"_s, u"test desc"_s, QStringList() << "A" << "B" << "C", false, 2 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"enum"_s, context, widgetContext, &enumParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterEnum *>( def.get() )->options(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast<QgsProcessingParameterEnum *>( def.get() )->defaultValue().toStringList(), QStringList() << "2" );
  QVERIFY( !static_cast<QgsProcessingParameterEnum *>( def.get() )->allowMultiple() );
  enumParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  enumParam.setAllowMultiple( true );
  enumParam.setDefaultValue( QVariantList() << 0 << 1 );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"enum"_s, context, widgetContext, &enumParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterEnum *>( def.get() )->options(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast<QgsProcessingParameterEnum *>( def.get() )->defaultValue().toStringList(), QStringList() << "0" << "1" );
  QVERIFY( static_cast<QgsProcessingParameterEnum *>( def.get() )->allowMultiple() );
}

void TestProcessingGui::testLayoutWrapper()
{
  QgsProject p;
  QgsPrintLayout *l1 = new QgsPrintLayout( &p );
  l1->setName( "l1" );
  p.layoutManager()->addLayout( l1 );
  QgsPrintLayout *l2 = new QgsPrintLayout( &p );
  l2->setName( "l2" );
  p.layoutManager()->addLayout( l2 );

  auto testWrapper = [&p]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterLayout param( u"layout"_s, u"layout"_s, false );

    QgsProcessingLayoutWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    context.setProject( &p );
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( &p );
    wrapper.setWidgetContext( widgetContext );
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingLayoutWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( "l2", context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"l2"_s );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper.wrappedWidget() )->currentText(), u"l2"_s );
    }
    else
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentText(), u"l2"_s );
    }
    wrapper.setWidgetValue( "l1", context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toString(), u"l1"_s );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper.wrappedWidget() )->currentText(), u"l1"_s );
    }
    else
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentText(), u"l1"_s );
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"layout"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      static_cast<QComboBox *>( wrapper.wrappedWidget() )->setCurrentIndex( 1 );
    }
    else
    {
      static_cast<QComboBox *>( wrapper.wrappedWidget() )->setCurrentText( u"aaaa"_s );
    }
    QCOMPARE( spy.count(), 3 );

    delete w;

    // optional

    QgsProcessingParameterLayout param2( u"layout"_s, u"layout"_s, QVariant(), true );

    QgsProcessingLayoutWidgetWrapper wrapper2( &param2, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "l2", context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"l2"_s );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper2.wrappedWidget() )->currentIndex(), 2 );
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l2"_s );
    }
    else
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l2"_s );
    }
    wrapper2.setWidgetValue( "l1", context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"l1"_s );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper2.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1"_s );
    }
    else
    {
      QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1"_s );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( static_cast<QgsLayoutComboBox *>( wrapper2.wrappedWidget() )->currentIndex(), 0 );
      QVERIFY( static_cast<QgsLayoutComboBox *>( wrapper2.wrappedWidget() )->currentText().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText().isEmpty() );
    }

    // check signal
    if ( type != Qgis::ProcessingMode::Modeler )
      static_cast<QComboBox *>( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast<QComboBox *>( wrapper2.wrappedWidget() )->setCurrentText( u"aaa"_s );
    QCOMPARE( spy2.count(), 4 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testLayoutItemWrapper()
{
  QgsProject p;
  QgsPrintLayout *l1 = new QgsPrintLayout( &p );
  l1->setName( "l1" );
  p.layoutManager()->addLayout( l1 );
  QgsLayoutItemLabel *label1 = new QgsLayoutItemLabel( l1 );
  label1->setId( "a" );
  l1->addLayoutItem( label1 );
  QgsLayoutItemLabel *label2 = new QgsLayoutItemLabel( l1 );
  label2->setId( "b" );
  l1->addLayoutItem( label2 );

  auto testWrapper = [&p, l1, label1, label2]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterLayoutItem param( u"layout"_s, u"layout"_s, false );

    QgsProcessingLayoutItemWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    context.setProject( &p );
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( &p );
    wrapper.setWidgetContext( widgetContext );
    QWidget *w = wrapper.createWrappedWidget( context );

    wrapper.setLayout( l1 );

    QSignalSpy spy( &wrapper, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( "b", context );
    QCOMPARE( spy.count(), 1 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), label2->uuid() );
      QCOMPARE( static_cast<QgsLayoutItemComboBox *>( wrapper.wrappedWidget() )->currentText(), u"b"_s );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), u"b"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"b"_s );
    }
    wrapper.setWidgetValue( "a", context );
    QCOMPARE( spy.count(), 2 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), label1->uuid() );
      QCOMPARE( static_cast<QgsLayoutItemComboBox *>( wrapper.wrappedWidget() )->currentText(), u"a"_s );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), u"a"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"a"_s );
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"layout"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      static_cast<QComboBox *>( wrapper.wrappedWidget() )->setCurrentIndex( 1 );
    }
    else
    {
      static_cast<QLineEdit *>( wrapper.wrappedWidget() )->setText( u"aaaa"_s );
    }
    QCOMPARE( spy.count(), 3 );

    delete w;

    // optional

    QgsProcessingParameterLayoutItem param2( u"layout"_s, u"layout"_s, QVariant(), QString(), -1, true );

    QgsProcessingLayoutItemWidgetWrapper wrapper2( &param2, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );
    wrapper2.setLayout( l1 );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "b", context );
    QCOMPARE( spy2.count(), 1 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper2.widgetValue().toString(), label2->uuid() );
      QCOMPARE( static_cast<QgsLayoutItemComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"b"_s );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), u"b"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), u"b"_s );
    }
    wrapper2.setWidgetValue( "a", context );
    QCOMPARE( spy2.count(), 2 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper2.widgetValue().toString(), label1->uuid() );
      QCOMPARE( static_cast<QgsLayoutItemComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"a"_s );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), u"a"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), u"a"_s );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QVERIFY( static_cast<QgsLayoutItemComboBox *>( wrapper2.wrappedWidget() )->currentText().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text().isEmpty() );
    }

    // check signal
    if ( type != Qgis::ProcessingMode::Modeler )
      static_cast<QgsLayoutItemComboBox *>( wrapper2.wrappedWidget() )->setCurrentIndex( 1 );
    else
      static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->setText( u"aaa"_s );
    QCOMPARE( spy2.count(), 4 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"layoutitem"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterLayoutItem itemParam( u"n"_s, u"test desc"_s, QVariant(), u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"layoutitem"_s, context, widgetContext, &itemParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterLayoutItem *>( def.get() )->parentLayoutParameterName(), u"parent"_s );
  itemParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  itemParam.setParentLayoutParameterName( QString() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"layoutitem"_s, context, widgetContext, &itemParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QVERIFY( static_cast<QgsProcessingParameterLayoutItem *>( def.get() )->parentLayoutParameterName().isEmpty() );
}

void TestProcessingGui::testPointPanel()
{
  auto panel = std::make_unique<QgsProcessingPointPanel>( nullptr );
  QSignalSpy spy( panel.get(), &QgsProcessingPointPanel::changed );

  panel->setValue( QgsPointXY( 100, 150 ), QgsCoordinateReferenceSystem() );
  QCOMPARE( panel->value().toString(), u"100.000000,150.000000"_s );
  QCOMPARE( spy.count(), 1 );

  panel->setValue( QgsPointXY( 200, 250 ), QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  QCOMPARE( panel->value().toString(), u"200.000000,250.000000 [EPSG:3111]"_s );
  QCOMPARE( spy.count(), 2 );

  panel->setValue( QgsPointXY( 123456.123456789, 654321.987654321 ), QgsCoordinateReferenceSystem() );
  QCOMPARE( panel->value().toString(), u"123456.123457,654321.987654"_s );
  QCOMPARE( spy.count(), 3 );

  QVERIFY( !panel->mLineEdit->showClearButton() );
  panel->setAllowNull( true );
  QVERIFY( panel->mLineEdit->showClearButton() );
  panel->clear();
  QVERIFY( !panel->value().isValid() );
  QCOMPARE( spy.count(), 4 );

  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:28356"_s ) );
  panel->setMapCanvas( &canvas );
  panel->updatePoint( QgsPointXY( 1.5, -3.5 ) );
  QCOMPARE( panel->value().toString(), u"1.500000,-3.500000 [EPSG:28356]"_s );
  QCOMPARE( spy.count(), 5 );

  panel.reset();
}


void TestProcessingGui::testPointWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterPoint param( u"point"_s, u"point"_s, false );

    QgsProcessingPointWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( "1,2", context );
    QCOMPARE( spy.count(), 1 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), u"1.000000,2.000000"_s );
      QCOMPARE( static_cast<QgsProcessingPointPanel *>( wrapper.wrappedWidget() )->mLineEdit->text(), u"1.000000,2.000000"_s );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), u"1,2"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"1,2"_s );
    }
    wrapper.setWidgetValue( "1,2 [EPSG:3111]", context );
    QCOMPARE( spy.count(), 2 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), u"1.000000,2.000000 [EPSG:3111]"_s );
      QCOMPARE( static_cast<QgsProcessingPointPanel *>( wrapper.wrappedWidget() )->mLineEdit->text(), u"1.000000,2.000000 [EPSG:3111]"_s );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), u"1,2 [EPSG:3111]"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"1,2 [EPSG:3111]"_s );
    }

    // check signal
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      static_cast<QgsProcessingPointPanel *>( wrapper.wrappedWidget() )->mLineEdit->setText( u"b"_s );
    }
    else
    {
      static_cast<QLineEdit *>( wrapper.wrappedWidget() )->setText( u"aaaa"_s );
    }
    QCOMPARE( spy.count(), 3 );


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"point"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    delete w;

    // optional

    QgsProcessingParameterPoint param2( u"point"_s, u"point"_s, QVariant(), true );

    QgsProcessingPointWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "1,2", context );
    QCOMPARE( spy2.count(), 1 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( static_cast<QgsProcessingPointPanel *>( wrapper2.wrappedWidget() )->mLineEdit->text(), u"1.000000,2.000000"_s );
      QCOMPARE( wrapper2.widgetValue().toString(), u"1.000000,2.000000"_s );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), u"1,2"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), u"1,2"_s );
    }

    wrapper2.setWidgetValue( "1,2 [EPSG:3111]", context );
    QCOMPARE( spy2.count(), 2 );
    if ( type != Qgis::ProcessingMode::Modeler )
    {
      QCOMPARE( wrapper2.widgetValue().toString(), u"1.000000,2.000000 [EPSG:3111]"_s );
      QCOMPARE( static_cast<QgsProcessingPointPanel *>( wrapper2.wrappedWidget() )->mLineEdit->text(), u"1.000000,2.000000 [EPSG:3111]"_s );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), u"1,2 [EPSG:3111]"_s );
      QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), u"1,2 [EPSG:3111]"_s );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type == Qgis::ProcessingMode::Modeler )
    {
      QVERIFY( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast<QgsProcessingPointPanel *>( wrapper2.wrappedWidget() )->mLineEdit->text().isEmpty() );
    }
    wrapper2.setWidgetValue( "1,3", context );
    QCOMPARE( spy2.count(), 4 );
    wrapper2.setWidgetValue( "", context );
    QCOMPARE( spy2.count(), 5 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type == Qgis::ProcessingMode::Modeler )
    {
      QVERIFY( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast<QgsProcessingPointPanel *>( wrapper2.wrappedWidget() )->mLineEdit->text().isEmpty() );
    }

    // check signals
    wrapper2.setWidgetValue( "1,3", context );
    QCOMPARE( spy2.count(), 6 );
    if ( type == Qgis::ProcessingMode::Modeler )
    {
      static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->clear();
    }
    else
    {
      static_cast<QgsProcessingPointPanel *>( wrapper2.wrappedWidget() )->mLineEdit->clear();
    }
    QCOMPARE( spy2.count(), 7 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"point"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterPoint pointParam( u"n"_s, u"test desc"_s, u"1,2"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"point"_s, context, widgetContext, &pointParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterPoint *>( def.get() )->defaultValue().toString(), u"1.000000,2.000000"_s );
  pointParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  pointParam.setDefaultValue( u"4,7"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"point"_s, context, widgetContext, &pointParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterPoint *>( def.get() )->defaultValue().toString(), u"4.000000,7.000000"_s );
}


void TestProcessingGui::testGeometryWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterGeometry param( u"geometry"_s, u"geometry"_s, false );

    QgsProcessingGeometryWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingGeometryWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"POINT (1 2)"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString().toLower(), u"point (1 2)"_s );
    QCOMPARE( static_cast<QgsGeometryWidget *>( wrapper.wrappedWidget() )->geometryValue().asWkt().toLower(), u"point (1 2)"_s.toLower() );
    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast<QgsGeometryWidget *>( wrapper.wrappedWidget() )->geometryValue().asWkt().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"geometry"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsGeometryWidget *>( wrapper.wrappedWidget() )->setGeometryValue( QgsReferencedGeometry( QgsGeometry::fromWkt( "point(0 0)" ), QgsCoordinateReferenceSystem() ) );
    QCOMPARE( spy.count(), 3 );
    static_cast<QgsGeometryWidget *>( wrapper.wrappedWidget() )->clearGeometry();
    QCOMPARE( spy.count(), 4 );

    delete w;

    // optional

    QgsProcessingParameterGeometry param2( u"geometry"_s, u"geometry"_s, QVariant(), true );

    QgsProcessingGeometryWidgetWrapper wrapper2( &param2, type );

    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "POINT (1 2)", context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString().toLower(), u"point (1 2)"_s );
    QCOMPARE( static_cast<QgsGeometryWidget *>( wrapper2.wrappedWidget() )->geometryValue().asWkt().toLower(), u"point (1 2)"_s );

    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( static_cast<QgsGeometryWidget *>( wrapper2.wrappedWidget() )->geometryValue().asWkt().isEmpty() );

    wrapper2.setWidgetValue( "POINT (1 3)", context );
    QCOMPARE( spy2.count(), 3 );
    wrapper2.setWidgetValue( "", context );
    QCOMPARE( spy2.count(), 4 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( static_cast<QgsGeometryWidget *>( wrapper2.wrappedWidget() )->geometryValue().asWkt().isEmpty() );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );


  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );


  // config widget
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"geometry"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterGeometry geometryParam( u"n"_s, u"test desc"_s, u"POINT (1 2)"_s );

  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"geometry"_s, context, widgetContext, &geometryParam );

  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterGeometry *>( def.get() )->defaultValue().toString().toLower(), u"point (1 2)"_s );
  geometryParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  geometryParam.setDefaultValue( u"POINT (4 7)"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"geometry"_s, context, widgetContext, &geometryParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterGeometry *>( def.get() )->defaultValue().toString().toLower(), u"point (4 7)"_s );
}


void TestProcessingGui::testExtentWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterExtent param( u"extent"_s, u"extent"_s, false );

    QgsProcessingExtentWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingExtentWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( "1,2,3,4", context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"1.000000000,2.000000000,3.000000000,4.000000000"_s );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );

    wrapper.setWidgetValue( "1,2,3,4 [EPSG:3111]", context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toString(), u"1.000000000,2.000000000,3.000000000,4.000000000 [EPSG:3111]"_s );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper.wrappedWidget() )->outputCrs().authid(), u"EPSG:3111"_s );

    // check signal
    static_cast<QgsExtentWidget *>( wrapper.wrappedWidget() )->setOutputExtentFromUser( QgsRectangle( 11, 22, 33, 44 ), QgsCoordinateReferenceSystem() );
    QCOMPARE( spy.count(), 3 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"extent"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    delete w;

    // optional

    QgsProcessingParameterExtent param2( u"extent"_s, u"extent"_s, QVariant(), true );

    QgsProcessingExtentWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingExtentWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "1,2,3,4", context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );
    QCOMPARE( wrapper2.widgetValue().toString(), u"1.000000000,2.000000000,3.000000000,4.000000000"_s );

    wrapper2.setWidgetValue( "1,2,3,4 [EPSG:3111]", context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"1.000000000,2.000000000,3.000000000,4.000000000 [EPSG:3111]"_s );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->outputCrs().authid(), u"EPSG:3111"_s );
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( !static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->isValid() );

    // simulate a user manually entering an extent by hand
    qgis::down_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->mCondensedLineEdit->setText( "372830.001,373830.001,372830.001,373830.001" );
    qgis::down_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->setOutputExtentFromCondensedLineEdit();
    QCOMPARE( spy2.count(), 4 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"372830.001000000,373830.001000000,372830.001000000,373830.001000000 [EPSG:3111]"_s );
    QCOMPARE( qgis::down_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->outputExtent(), QgsRectangle( 372830.001, 372830.001, 373830.001, 373830.001 ) );
    QCOMPARE( static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->outputCrs().authid(), u"EPSG:3111"_s );

    wrapper2.setWidgetValue( "", context );
    QCOMPARE( spy2.count(), 5 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( !static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->isValid() );

    // check signals
    wrapper2.setWidgetValue( "1,3,9,8", context );
    QCOMPARE( spy2.count(), 6 );
    static_cast<QgsExtentWidget *>( wrapper2.wrappedWidget() )->clear();
    QCOMPARE( spy2.count(), 7 );

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"extent"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterExtent extentParam( u"n"_s, u"test desc"_s, u"1,2,3,4"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"extent"_s, context, widgetContext, &extentParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterExtent *>( def.get() )->defaultValue().toString(), u"1.000000000,2.000000000,3.000000000,4.000000000"_s );
  extentParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  extentParam.setDefaultValue( u"4,7,8,9"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"extent"_s, context, widgetContext, &extentParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterExtent *>( def.get() )->defaultValue().toString(), u"4.000000000,7.000000000,8.000000000,9.000000000"_s );
}

void TestProcessingGui::testColorWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingParameterColor param( u"color"_s, u"color"_s );

    QgsProcessingColorWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingColorWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QColor( 255, 0, 0 ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().value<QColor>().name(), u"#ff0000"_s );
    QCOMPARE( static_cast<QgsColorButton *>( wrapper.wrappedWidget() )->color(), QColor( 255, 0, 0 ) );
    QVERIFY( !static_cast<QgsColorButton *>( wrapper.wrappedWidget() )->showNull() );
    QVERIFY( static_cast<QgsColorButton *>( wrapper.wrappedWidget() )->allowOpacity() );
    wrapper.setWidgetValue( QColor(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( !wrapper.widgetValue().value<QColor>().isValid() );
    QVERIFY( !static_cast<QgsColorButton *>( wrapper.wrappedWidget() )->color().isValid() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"color"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsColorButton *>( wrapper.wrappedWidget() )->setColor( QColor( 0, 255, 0 ) );
    QCOMPARE( spy.count(), 3 );

    // with opacity
    wrapper.setWidgetValue( QColor( 255, 0, 0, 100 ), context );
    QCOMPARE( wrapper.widgetValue().value<QColor>(), QColor( 255, 0, 0, 100 ) );

    delete w;

    // with null
    QgsProcessingParameterColor param2( u"c2"_s, u"c2"_s, QColor( 10, 20, 30 ), true, true );

    QgsProcessingColorWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );
    QVERIFY( static_cast<QgsColorButton *>( wrapper2.wrappedWidget() )->showNull() );
    QCOMPARE( static_cast<QgsColorButton *>( wrapper2.wrappedWidget() )->color().name(), u"#0a141e"_s );
    wrapper2.setWidgetValue( QVariant(), context );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    wrapper2.setWidgetValue( QColor( 255, 0, 255 ), context );
    QCOMPARE( wrapper2.widgetValue().value<QColor>().name(), u"#ff00ff"_s );

    // no opacity
    QgsProcessingParameterColor param3( u"c2"_s, u"c2"_s, QColor( 10, 20, 30 ), false, true );

    QgsProcessingColorWidgetWrapper wrapper3( &param3, type );
    w = wrapper3.createWrappedWidget( context );
    wrapper3.setWidgetValue( QColor( 255, 0, 0, 100 ), context );
    QCOMPARE( wrapper3.widgetValue().value<QColor>(), QColor( 255, 0, 0 ) );
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"color"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( static_cast<QgsProcessingParameterColor *>( def.get() )->opacityEnabled() ); // should default to true

  // using a parameter definition as initial values
  QgsProcessingParameterColor colorParam( u"n"_s, u"test desc"_s, QColor( 255, 0, 0, 100 ), true );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"color"_s, context, widgetContext, &colorParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterColor *>( def.get() )->defaultValue().value<QColor>(), QColor( 255, 0, 0, 100 ) );
  QVERIFY( static_cast<QgsProcessingParameterColor *>( def.get() )->opacityEnabled() );
  colorParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  colorParam.setOpacityEnabled( false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"color"_s, context, widgetContext, &colorParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterColor *>( def.get() )->defaultValue().value<QColor>(), QColor( 255, 0, 0 ) ); // (no opacity!)
  QVERIFY( !static_cast<QgsProcessingParameterColor *>( def.get() )->opacityEnabled() );
}

void TestProcessingGui::testCoordinateOperationWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingParameterCoordinateOperation param( u"op"_s, u"op"_s );

    QgsProcessingCoordinateOperationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    wrapper.setSourceCrsParameterValue( QgsCoordinateReferenceSystem( u"EPSG:26745"_s ) );
    wrapper.setDestinationCrsParameterValue( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );

    QSignalSpy spy( &wrapper, &QgsProcessingCoordinateOperationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      {
        QCOMPARE( static_cast<QgsCoordinateOperationWidget *>( wrapper.wrappedWidget() )->selectedOperation().proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s );
        wrapper.setWidgetValue( u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( static_cast<QgsCoordinateOperationWidget *>( wrapper.wrappedWidget() )->selectedOperation().proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s );

        // check signal
        QgsCoordinateOperationWidget::OperationDetails deets;
        deets.proj = u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s;
        static_cast<QgsCoordinateOperationWidget *>( wrapper.wrappedWidget() )->setSelectedOperation( deets );
        QCOMPARE( spy.count(), 3 );
        break;
      }

      case Qgis::ProcessingMode::Modeler:
      case Qgis::ProcessingMode::Batch:
      {
        QCOMPARE( wrapper.mLineEdit->text(), u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s );
        wrapper.setWidgetValue( u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.mLineEdit->text(), u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s );

        // check signal
        wrapper.mLineEdit->setText( u"+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"_s );
        QCOMPARE( spy.count(), 3 );
        break;
      }
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"op"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"coordinateoperation"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  QVERIFY( !static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->sourceCrs().isValid() );                  // should default to not set
  QVERIFY( !static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->destinationCrs().isValid() );             // should default to not set
  QVERIFY( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->sourceCrsParameterName().isEmpty() );      // should default to not set
  QVERIFY( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->destinationCrsParameterName().isEmpty() ); // should default to not set

  // using a parameter definition as initial values
  QgsProcessingParameterCoordinateOperation coordParam( u"n"_s, u"test desc"_s, u"+proj"_s, u"a"_s, u"b"_s, u"EPSG:26745"_s, u"EPSG:4326"_s, false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"coordinateoperation"_s, context, widgetContext, &coordParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->defaultValue().toString(), u"+proj"_s );
  QCOMPARE( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->sourceCrsParameterName(), u"a"_s );
  QCOMPARE( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->destinationCrsParameterName(), u"b"_s );
  QCOMPARE( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->sourceCrs().value<QgsCoordinateReferenceSystem>().authid(), u"EPSG:26745"_s );
  QCOMPARE( static_cast<QgsProcessingParameterCoordinateOperation *>( def.get() )->destinationCrs().value<QgsCoordinateReferenceSystem>().authid(), u"EPSG:4326"_s );
  coordParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"coordinateoperation"_s, context, widgetContext, &coordParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
}

void TestProcessingGui::mapLayerComboBox()
{
  QgsProject::instance()->removeAllMapLayers();
  QgsProcessingContext context;
  context.setProject( QgsProject::instance() );

  // feature source param
  std::unique_ptr<QgsProcessingParameterDefinition> param( new QgsProcessingParameterFeatureSource( u"param"_s, QString() ) );
  auto combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );

  QSignalSpy spy( combo.get(), &QgsProcessingMapLayerComboBox::valueChanged );
  QVERIFY( !combo->value().isValid() );
  combo->setValue( u"file path"_s, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( combo->value().toString(), u"file path"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 1 );
  combo->setValue( QVariant(), context ); // not possible, it's not an optional param
  QCOMPARE( combo->value().toString(), u"file path"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 1 );
  combo->setValue( u"file path 2"_s, context );
  QCOMPARE( combo->value().toString(), u"file path 2"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 2 );
  combo->setValue( u"file path"_s, context );
  QCOMPARE( combo->value().toString(), u"file path"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 3 );
  combo->setLayer( nullptr ); // not possible, not optional
  QCOMPARE( combo->value().toString(), u"file path"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 3 );

  // project layers
  QgsVectorLayer *vl = new QgsVectorLayer( u"LineString"_s, u"l1"_s, u"memory"_s );
  QgsFeature f;
  vl->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayer( vl );
  QVERIFY( vl->isValid() );
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"LineString"_s, u"l2"_s, u"memory"_s );
  vl2->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayer( vl2 );
  QVERIFY( vl2->isValid() );

  QCOMPARE( combo->value().toString(), u"file path"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 3 );

  combo->setLayer( vl );
  QCOMPARE( combo->currentLayer(), vl );
  QCOMPARE( combo->value().toString(), vl->id() );
  QVERIFY( combo->currentText().startsWith( vl->name() ) );
  QCOMPARE( spy.count(), 4 );
  combo->setLayer( vl );
  QCOMPARE( spy.count(), 4 );

  combo->setLayer( vl2 );
  QCOMPARE( combo->value().toString(), vl2->id() );
  QVERIFY( combo->currentText().startsWith( vl2->name() ) );
  QCOMPARE( spy.count(), 5 );

  combo->setValue( u"file path"_s, context );
  QCOMPARE( combo->value().toString(), u"file path"_s );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 6 );

  // setting feature source def, i.e. with selection
  QgsProcessingFeatureSourceDefinition sourceDef( vl2->id(), false );
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().toString(), vl2->id() );
  QVERIFY( combo->currentText().startsWith( vl2->name() ) );
  QCOMPARE( spy.count(), 7 );
  // asking for selected features only, but no selection in layer, won't be allowed
  sourceDef = QgsProcessingFeatureSourceDefinition( vl2->id(), true );
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().toString(), vl2->id() );
  QVERIFY( combo->currentText().startsWith( vl2->name() ) );
  QCOMPARE( spy.count(), 7 ); // no change

  // now make a selection in the layer, and repeat
  vl2->selectAll();
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().userType(), qMetaTypeId<QgsProcessingFeatureSourceDefinition>() );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().source.staticValue().toString(), vl2->id() );
  QVERIFY( combo->value().value<QgsProcessingFeatureSourceDefinition>().selectedFeaturesOnly );
  QVERIFY( combo->currentText().startsWith( vl2->name() ) );
  QCOMPARE( spy.count(), 8 );

  // remove layer selection, and check result...
  vl2->removeSelection();
  QCOMPARE( combo->value().toString(), vl2->id() );
  QVERIFY( combo->currentText().startsWith( vl2->name() ) );
  QCOMPARE( spy.count(), 9 );

  // phew, nearly there. Let's check another variation
  vl2->selectAll();
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().userType(), qMetaTypeId<QgsProcessingFeatureSourceDefinition>() );
  QCOMPARE( spy.count(), 10 );
  combo->setValue( QVariant::fromValue( vl ), context );
  QCOMPARE( combo->value().toString(), vl->id() );
  QVERIFY( combo->currentText().startsWith( vl->name() ) );
  QCOMPARE( spy.count(), 11 );

  // one last variation - selection to selection
  combo->setValue( sourceDef, context );
  QCOMPARE( spy.count(), 12 );
  QVERIFY( combo->value().value<QgsProcessingFeatureSourceDefinition>().selectedFeaturesOnly );
  vl->selectAll();
  sourceDef = QgsProcessingFeatureSourceDefinition( vl->id(), true );
  combo->setValue( sourceDef, context );
  // expect "selected only" state to remain
  QCOMPARE( combo->value().userType(), qMetaTypeId<QgsProcessingFeatureSourceDefinition>() );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().source.staticValue().toString(), vl->id() );
  QVERIFY( combo->value().value<QgsProcessingFeatureSourceDefinition>().selectedFeaturesOnly );
  QVERIFY( combo->currentText().startsWith( vl->name() ) );
  QCOMPARE( spy.count(), 13 );

  // iterate over features
  QVERIFY( !( combo->value().value<QgsProcessingFeatureSourceDefinition>().flags & Qgis::ProcessingFeatureSourceDefinitionFlag::CreateIndividualOutputPerInputFeature ) );
  sourceDef.flags |= Qgis::ProcessingFeatureSourceDefinitionFlag::CreateIndividualOutputPerInputFeature;
  combo->setValue( sourceDef, context );
  QVERIFY( combo->value().value<QgsProcessingFeatureSourceDefinition>().flags & Qgis::ProcessingFeatureSourceDefinitionFlag::CreateIndividualOutputPerInputFeature );
  sourceDef.flags = Qgis::ProcessingFeatureSourceDefinitionFlags();
  combo->setValue( sourceDef, context );
  QVERIFY( !( combo->value().value<QgsProcessingFeatureSourceDefinition>().flags & Qgis::ProcessingFeatureSourceDefinitionFlag::CreateIndividualOutputPerInputFeature ) );

  // advanced settings
  sourceDef.featureLimit = 67;
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().featureLimit, 67LL );
  sourceDef.featureLimit = -1;
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().featureLimit, -1LL );
  sourceDef.flags |= Qgis::ProcessingFeatureSourceDefinitionFlag::OverrideDefaultGeometryCheck;
  sourceDef.geometryCheck = Qgis::InvalidGeometryCheck::SkipInvalid;
  combo->setValue( sourceDef, context );
  QVERIFY( combo->value().value<QgsProcessingFeatureSourceDefinition>().flags & Qgis::ProcessingFeatureSourceDefinitionFlag::OverrideDefaultGeometryCheck );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().geometryCheck, Qgis::InvalidGeometryCheck::SkipInvalid );
  sourceDef.flags = Qgis::ProcessingFeatureSourceDefinitionFlags();
  combo->setValue( sourceDef, context );
  QVERIFY( !( combo->value().value<QgsProcessingFeatureSourceDefinition>().flags & Qgis::ProcessingFeatureSourceDefinitionFlag::OverrideDefaultGeometryCheck ) );
  sourceDef.filterExpression = u"name='test'"_s;
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().filterExpression, u"name='test'"_s );
  sourceDef.filterExpression = QString();
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().value<QgsProcessingFeatureSourceDefinition>().filterExpression, QString() );

  combo.reset();
  param.reset();

  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( u"Point"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( u"LineString"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( u"Polygon"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( u"None"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( noGeom );
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", u"Triangle and Quad Mdal"_s, u"mdal"_s );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  QgsProject::instance()->addMapLayer( mesh );
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", u"band1_byte"_s );
  QgsProject::instance()->addMapLayer( raster );
  QgsPointCloudLayer *pointCloud = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", u"Point cloud"_s, u"ept"_s );
  QVERIFY( pointCloud->isValid() );
  QgsProject::instance()->addMapLayer( pointCloud );
  QgsTiledSceneLayer *tiledScene = new QgsTiledSceneLayer( "tiled_scene_source", u"tiled scene"_s, u"test_tiled_scene_provider"_s );
  QVERIFY( tiledScene->isValid() );
  QgsProject::instance()->addMapLayer( tiledScene );

  // map layer param, all types are acceptable
  param = std::make_unique<QgsProcessingParameterMapLayer>( u"param"_s, QString() );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo->setLayer( line );
  QCOMPARE( combo->currentLayer(), line );
  combo->setLayer( polygon );
  QCOMPARE( combo->currentLayer(), polygon );
  combo->setLayer( noGeom );
  QCOMPARE( combo->currentLayer(), noGeom );
  combo->setLayer( mesh );
  QCOMPARE( combo->currentLayer(), mesh );
  combo->setLayer( raster );
  QCOMPARE( combo->currentLayer(), raster );
  combo->setLayer( pointCloud );
  QCOMPARE( combo->currentLayer(), pointCloud );
  combo->setLayer( tiledScene );
  QCOMPARE( combo->currentLayer(), tiledScene );
  combo.reset();
  param.reset();

  // map layer param, only point vector and raster types are acceptable
  param = std::make_unique<QgsProcessingParameterMapLayer>( u"param"_s, QString(), QVariant(), false, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::Raster ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( raster );
  QCOMPARE( combo->currentLayer(), raster );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( tiledScene );
  QVERIFY( !combo->currentLayer() );
  combo.reset();
  param.reset();

  // map layer param, only tiled scene layers are acceptable
  param = std::make_unique<QgsProcessingParameterMapLayer>( u"param"_s, QString(), QVariant(), false, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::TiledScene ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( tiledScene );
  QCOMPARE( combo->currentLayer(), tiledScene );
  combo.reset();
  param.reset();

  // raster layer param, only raster types are acceptable
  param = std::make_unique<QgsProcessingParameterRasterLayer>( u"param"_s, QString() );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( raster );
  QCOMPARE( combo->currentLayer(), raster );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( tiledScene );
  QVERIFY( !combo->currentLayer() );
  combo.reset();
  param.reset();

  // mesh layer parm, only mesh types are acceptable
  param = std::make_unique<QgsProcessingParameterMeshLayer>( u"param"_s, QString() );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( mesh );
  QCOMPARE( combo->currentLayer(), mesh );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( tiledScene );
  QVERIFY( !combo->currentLayer() );
  combo.reset();
  param.reset();

  // point cloud layer parm, only point cloud types are acceptable
  param = std::make_unique<QgsProcessingParameterPointCloudLayer>( u"param"_s, QString() );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo->setLayer( pointCloud );
  QCOMPARE( combo->currentLayer(), pointCloud );
  combo->setLayer( tiledScene );
  QVERIFY( !combo->currentLayer() );
  combo.reset();
  param.reset();

  // feature source and vector layer params
  // if not specified, the default is any vector layer with geometry
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  auto param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s );
  auto combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo2->setLayer( point );
  QCOMPARE( combo2->currentLayer(), point );
  combo->setLayer( line );
  QCOMPARE( combo->currentLayer(), line );
  combo2->setLayer( line );
  QCOMPARE( combo2->currentLayer(), line );
  combo->setLayer( polygon );
  QCOMPARE( combo->currentLayer(), polygon );
  combo2->setLayer( polygon );
  QCOMPARE( combo2->currentLayer(), polygon );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( noGeom );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // point layer
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo2->setLayer( point );
  QCOMPARE( combo2->currentLayer(), point );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( line );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( polygon );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( noGeom );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // line layer
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
  combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( point );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( line );
  QCOMPARE( combo->currentLayer(), line );
  combo2->setLayer( line );
  QCOMPARE( combo2->currentLayer(), line );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( polygon );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( noGeom );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // polygon
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) );
  combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( point );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( line );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( line );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( polygon );
  QCOMPARE( combo->currentLayer(), polygon );
  combo2->setLayer( polygon );
  QCOMPARE( combo2->currentLayer(), polygon );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( noGeom );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // no geom
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo2->setLayer( point );
  QCOMPARE( combo2->currentLayer(), point );
  combo->setLayer( line );
  QCOMPARE( combo->currentLayer(), line );
  combo2->setLayer( line );
  QCOMPARE( combo2->currentLayer(), line );
  combo->setLayer( polygon );
  QCOMPARE( combo->currentLayer(), polygon );
  combo2->setLayer( polygon );
  QCOMPARE( combo2->currentLayer(), polygon );
  combo->setLayer( noGeom );
  QCOMPARE( combo->currentLayer(), noGeom );
  combo2->setLayer( noGeom );
  QCOMPARE( combo2->currentLayer(), noGeom );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // any geom
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo2->setLayer( point );
  QCOMPARE( combo2->currentLayer(), point );
  combo->setLayer( line );
  QCOMPARE( combo->currentLayer(), line );
  combo2->setLayer( line );
  QCOMPARE( combo2->currentLayer(), line );
  combo->setLayer( polygon );
  QCOMPARE( combo->currentLayer(), polygon );
  combo2->setLayer( polygon );
  QCOMPARE( combo2->currentLayer(), polygon );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( noGeom );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // combination point and line only
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  param2 = std::make_unique<QgsProcessingParameterFeatureSource>( u"param"_s, QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
  combo2 = std::make_unique<QgsProcessingMapLayerComboBox>( param2.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo2->setLayer( point );
  QCOMPARE( combo2->currentLayer(), point );
  combo->setLayer( line );
  QCOMPARE( combo->currentLayer(), line );
  combo2->setLayer( line );
  QCOMPARE( combo2->currentLayer(), line );
  combo->setLayer( polygon );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( polygon );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( noGeom );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( noGeom );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( mesh );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( mesh );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( raster );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( raster );
  QVERIFY( !combo2->currentLayer() );
  combo->setLayer( pointCloud );
  QVERIFY( !combo->currentLayer() );
  combo2->setLayer( pointCloud );
  QVERIFY( !combo2->currentLayer() );
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // optional
  param = std::make_unique<QgsProcessingParameterVectorLayer>( u"param"_s, QString(), QList<int>(), QVariant(), true );
  combo = std::make_unique<QgsProcessingMapLayerComboBox>( param.get() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo->setLayer( nullptr );
  QVERIFY( !combo->currentLayer() );
  QVERIFY( !combo->value().isValid() );
  combo->setLayer( point );
  QCOMPARE( combo->currentLayer(), point );
  combo->setValue( QVariant(), context );
  QVERIFY( !combo->currentLayer() );
  QVERIFY( !combo->value().isValid() );

  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();
  QgsProject::instance()->removeAllMapLayers();
}

void TestProcessingGui::testMapLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( u"Point"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( u"LineString"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( u"Polygon"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( u"None"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( noGeom );
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", u"Triangle and Quad Mdal"_s, u"mdal"_s );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  QgsProject::instance()->addMapLayer( mesh );
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", u"band1_byte"_s );
  QgsProject::instance()->addMapLayer( raster );

  auto testWrapper = [raster, polygon]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterMapLayer param( u"layer"_s, u"layer"_s, false );

    QgsProcessingMapLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingMapLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"band1_byte"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"band1_byte [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"band1_byte"_s );
        break;
    }

    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"band1_byte"_s );

    // check signal
    static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( polygon );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), polygon->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1 [EPSG:4326]"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1"_s );
        break;
    }
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"l1"_s );

    delete w;

    // optional
    QgsProcessingParameterMapLayer param2( u"layer"_s, u"layer"_s, QVariant(), true );
    QgsProcessingMapLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"band1_byte"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"band1_byte [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"band1_byte"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"layer"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"layer"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterMapLayer layerParam( u"n"_s, u"test desc"_s, QVariant(), false, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"layer"_s, context, widgetContext, &layerParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterMapLayer *>( def.get() )->dataTypes(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  layerParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  layerParam.setDataTypes( QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Raster ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"layer"_s, context, widgetContext, &layerParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterMapLayer *>( def.get() )->dataTypes(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::Raster ) );
}

void TestProcessingGui::testRasterLayerWrapper()
{
  // setup a project
  QgsProject::instance()->removeAllMapLayers();
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", u"band1_byte"_s );
  QgsProject::instance()->addMapLayer( raster );
  QgsRasterLayer *raster2 = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", u"band1_byte2"_s );
  QgsProject::instance()->addMapLayer( raster2 );

  auto testWrapper = [raster, raster2]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterRasterLayer param( u"raster"_s, u"raster"_s, false );

    QgsProcessingMapLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingMapLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingRasterLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"band1_byte"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"band1_byte [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"band1_byte"_s );
        break;
    }

    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"band1_byte"_s );

    // check signal
    static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( raster2 );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), raster2->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"band1_byte2 [EPSG:4326]"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"band1_byte2"_s );
        break;
    }
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"band1_byte2"_s );

    delete w;

    // optional
    QgsProcessingParameterRasterLayer param2( u"raster"_s, u"raster"_s, QVariant(), true );
    QgsProcessingMapLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"band1_byte"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"band1_byte [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"band1_byte"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"raster"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testVectorLayerWrapper()
{
  // setup a project with a range of vector layers
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( u"Point"_s, u"point"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( u"LineString"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( u"Polygon"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( u"None"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( noGeom );

  auto testWrapper = [point, polygon]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterVectorLayer param( u"vector"_s, u"vector"_s, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ), false );

    QgsProcessingVectorLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingVectorLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingMapLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"point"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"point [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"point"_s );
        break;
    }

    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"point"_s );

    // check signal
    static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( polygon );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), polygon->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1 [EPSG:4326]"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1"_s );
        break;
    }
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"l1"_s );

    delete w;

    // optional
    QgsProcessingParameterVectorLayer param2( u"vector"_s, u"vector"_s, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ), QVariant(), true );
    QgsProcessingVectorLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingVectorLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"point"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"point [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"point"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"vector"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"vector"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterVectorLayer layerParam( u"n"_s, u"test desc"_s, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"vector"_s, context, widgetContext, &layerParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterVectorLayer *>( def.get() )->dataTypes(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  layerParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  layerParam.setDataTypes( QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"vector"_s, context, widgetContext, &layerParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterVectorLayer *>( def.get() )->dataTypes(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
}

void TestProcessingGui::testFeatureSourceWrapper()
{
  // setup a project with a range of vector layers
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( u"Point"_s, u"point"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( u"LineString"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( u"Polygon"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( u"None"_s, u"l1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( noGeom );

  auto testWrapper = [point, polygon]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterFeatureSource param( u"source"_s, u"source"_s, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ), false );

    QgsProcessingFeatureSourceWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFeatureSourceWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingMapLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"point"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"point [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"point"_s );
        break;
    }

    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"point"_s );

    // check signal
    static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( polygon );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), polygon->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1 [EPSG:4326]"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"l1"_s );
        break;
    }
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"l1"_s );

    delete w;

    // optional
    QgsProcessingParameterFeatureSource param2( u"source"_s, u"source"_s, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ), QVariant(), true );
    QgsProcessingFeatureSourceWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFeatureSourceWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"point"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"point [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"point"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"source"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"source"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFeatureSource sourceParam( u"n"_s, u"test desc"_s, QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"source"_s, context, widgetContext, &sourceParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterFeatureSource *>( def.get() )->dataTypes(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  sourceParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  sourceParam.setDataTypes( QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"source"_s, context, widgetContext, &sourceParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterFeatureSource *>( def.get() )->dataTypes(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
}

void TestProcessingGui::testMeshLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", u"mesh1"_s, u"mdal"_s );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  mesh->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QgsProject::instance()->addMapLayer( mesh );
  QgsMeshLayer *mesh2 = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", u"mesh2"_s, u"mdal"_s );
  mesh2->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh2->isValid() );
  mesh2->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QgsProject::instance()->addMapLayer( mesh2 );

  auto testWrapper = [mesh, mesh2]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterMeshLayer param( u"mesh"_s, u"mesh"_s, false );

    QgsProcessingMeshLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingMapLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"mesh2"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), mesh2->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"mesh2 [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"mesh2"_s );
        break;
    }

    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"mesh2"_s );

    // check signal
    static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( mesh );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), mesh->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"mesh1 [EPSG:4326]"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"mesh1"_s );
        break;
    }
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"mesh1"_s );

    delete w;

    // optional
    QgsProcessingParameterMeshLayer param2( u"mesh"_s, u"mesh"_s, QVariant(), true );
    QgsProcessingMeshLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"mesh2"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), mesh2->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"mesh2 [EPSG:4326]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"mesh2"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"mesh"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::paramConfigWidget()
{
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"string"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  def->setDescription( u"test desc"_s );
  def->setFlags( Qgis::ProcessingParameterFlag::Optional );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"string"_s, context, widgetContext, def.get() );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  def->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"string"_s, context, widgetContext, def.get() );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
}

void TestProcessingGui::testMapThemeWrapper()
{
  // add some themes to the project
  QgsProject p;
  p.mapThemeCollection()->insert( u"aa"_s, QgsMapThemeCollection::MapThemeRecord() );
  p.mapThemeCollection()->insert( u"bb"_s, QgsMapThemeCollection::MapThemeRecord() );

  QCOMPARE( p.mapThemeCollection()->mapThemes(), QStringList() << u"aa"_s << u"bb"_s );

  auto testWrapper = [&p]( Qgis::ProcessingMode type ) {
    // non optional, no existing themes
    QgsProcessingParameterMapTheme param( u"theme"_s, u"theme"_s, false );

    QgsProcessingMapThemeWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        // batch or standard mode, only valid themes can be set!
        QCOMPARE( spy.count(), 0 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentIndex(), -1 );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 0 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentIndex(), -1 );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( &p );

    QgsProcessingMapThemeWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"aa"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"aa"_s );
    QCOMPARE( static_cast<QComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"aa"_s );

    // check signal
    static_cast<QComboBox *>( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
    QCOMPARE( spy2.count(), 3 );

    delete w;

    // optional
    QgsProcessingParameterMapTheme param2( u"theme"_s, u"theme"_s, true );
    QgsProcessingMapThemeWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"aa"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"aa"_s );
    QCOMPARE( static_cast<QComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"aa"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"theme"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  widgetContext.setProject( &p );
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"maptheme"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( !static_cast<QgsProcessingParameterMapTheme *>( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterMapTheme themeParam( u"n"_s, u"test desc"_s, u"aaa"_s, false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"maptheme"_s, context, widgetContext, &themeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterMapTheme *>( def.get() )->defaultValue().toString(), u"aaa"_s );
  themeParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  themeParam.setDefaultValue( u"xxx"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"maptheme"_s, context, widgetContext, &themeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterMapTheme *>( def.get() )->defaultValue().toString(), u"xxx"_s );
  themeParam.setDefaultValue( QVariant() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"maptheme"_s, context, widgetContext, &themeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QVERIFY( !static_cast<QgsProcessingParameterMapTheme *>( def.get() )->defaultValue().isValid() );
}

void TestProcessingGui::testDateTimeWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional, no existing themes
    QgsProcessingParameterDateTime param( u"datetime"_s, u"datetime"_s, Qgis::ProcessingDateTimeParameterDataType::DateTime, QVariant(), false );

    QgsProcessingDateTimeWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    // not a date value
    wrapper.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy.count(), 0 );
    // not optional, so an invalid value gets a date anyway...
    QVERIFY( wrapper.widgetValue().isValid() );
    wrapper.setWidgetValue( u"2019-08-07"_s, context );
    QCOMPARE( spy.count(), 1 );
    QVERIFY( wrapper.widgetValue().isValid() );
    QCOMPARE( wrapper.widgetValue().toDateTime(), QDateTime( QDate( 2019, 8, 7 ), QTime( 0, 0, 0 ) ) );
    QCOMPARE( static_cast<QgsDateTimeEdit *>( wrapper.wrappedWidget() )->dateTime(), QDateTime( QDate( 2019, 8, 7 ), QTime( 0, 0, 0 ) ) );
    wrapper.setWidgetValue( u"2019-08-07"_s, context );
    QCOMPARE( spy.count(), 1 );

    // check signal
    static_cast<QgsDateTimeEdit *>( wrapper.wrappedWidget() )->setDateTime( QDateTime( QDate( 2019, 8, 9 ), QTime( 0, 0, 0 ) ) );
    QCOMPARE( spy.count(), 2 );

    delete w;

    // optional
    QgsProcessingParameterDateTime param2( u"datetime"_s, u"datetime"_s, Qgis::ProcessingDateTimeParameterDataType::DateTime, QVariant(), true );
    QgsProcessingDateTimeWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    QSignalSpy spy3( &wrapper3, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 0 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    QVERIFY( !static_cast<QgsDateTimeEdit *>( wrapper3.wrappedWidget() )->dateTime().isValid() );
    wrapper3.setWidgetValue( u"2019-03-20"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toDateTime(), QDateTime( QDate( 2019, 3, 20 ), QTime( 0, 0, 0 ) ) );
    QCOMPARE( static_cast<QgsDateTimeEdit *>( wrapper3.wrappedWidget() )->dateTime(), QDateTime( QDate( 2019, 3, 20 ), QTime( 0, 0, 0 ) ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    // date mode
    QgsProcessingParameterDateTime param3( u"datetime"_s, u"datetime"_s, Qgis::ProcessingDateTimeParameterDataType::Date, QVariant(), true );
    QgsProcessingDateTimeWidgetWrapper wrapper4( &param3, type );
    w = wrapper4.createWrappedWidget( context );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    QSignalSpy spy4( &wrapper4, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy4.count(), 0 );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    QVERIFY( !static_cast<QgsDateEdit *>( wrapper4.wrappedWidget() )->date().isValid() );
    wrapper4.setWidgetValue( u"2019-03-20"_s, context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toDate(), QDate( 2019, 3, 20 ) );
    QCOMPARE( static_cast<QgsDateEdit *>( wrapper4.wrappedWidget() )->date(), QDate( 2019, 3, 20 ) );
    wrapper4.setWidgetValue( QDate( 2020, 1, 3 ), context );
    QCOMPARE( spy4.count(), 2 );
    QCOMPARE( wrapper4.widgetValue().toDate(), QDate( 2020, 1, 3 ) );
    QCOMPARE( static_cast<QgsDateEdit *>( wrapper4.wrappedWidget() )->date(), QDate( 2020, 1, 3 ) );
    wrapper4.setWidgetValue( QVariant(), context );
    QCOMPARE( spy4.count(), 3 );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    delete w;

    // time mode
    QgsProcessingParameterDateTime param4( u"datetime"_s, u"datetime"_s, Qgis::ProcessingDateTimeParameterDataType::Time, QVariant(), true );
    QgsProcessingDateTimeWidgetWrapper wrapper5( &param4, type );
    w = wrapper5.createWrappedWidget( context );
    QVERIFY( !wrapper5.widgetValue().isValid() );
    QSignalSpy spy5( &wrapper5, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    wrapper5.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy5.count(), 0 );
    QVERIFY( !wrapper5.widgetValue().isValid() );
    QVERIFY( !static_cast<QgsTimeEdit *>( wrapper5.wrappedWidget() )->time().isValid() );
    wrapper5.setWidgetValue( u"11:34:56"_s, context );
    QCOMPARE( spy5.count(), 1 );
    QCOMPARE( wrapper5.widgetValue().toTime(), QTime( 11, 34, 56 ) );
    QCOMPARE( static_cast<QgsTimeEdit *>( wrapper5.wrappedWidget() )->time(), QTime( 11, 34, 56 ) );
    wrapper5.setWidgetValue( QTime( 9, 34, 56 ), context );
    QCOMPARE( spy5.count(), 2 );
    QCOMPARE( wrapper5.widgetValue().toTime(), QTime( 9, 34, 56 ) );
    QCOMPARE( static_cast<QgsTimeEdit *>( wrapper5.wrappedWidget() )->time(), QTime( 9, 34, 56 ) );
    wrapper5.setWidgetValue( QVariant(), context );
    QCOMPARE( spy5.count(), 3 );
    QVERIFY( !wrapper5.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"datetime"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"datetime"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( !static_cast<QgsProcessingParameterDateTime *>( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterDateTime datetimeParam( u"n"_s, u"test desc"_s, Qgis::ProcessingDateTimeParameterDataType::Date, QVariant(), false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"datetime"_s, context, widgetContext, &datetimeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterDateTime *>( def.get() )->dataType(), Qgis::ProcessingDateTimeParameterDataType::Date );
  datetimeParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  datetimeParam.setDefaultValue( u"xxx"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"datetime"_s, context, widgetContext, &datetimeParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterDateTime *>( def.get() )->dataType(), Qgis::ProcessingDateTimeParameterDataType::Date );
}

void TestProcessingGui::testProviderConnectionWrapper()
{
  // register some connections
  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s );
  QgsAbstractProviderConnection *conn = md->createConnection( u"test uri.gpkg"_s, QVariantMap() );
  md->saveConnection( conn, u"aa"_s );
  md->saveConnection( conn, u"bb"_s );

  auto testWrapper = []( Qgis::ProcessingMode type ) {
    QgsProcessingParameterProviderConnection param( u"conn"_s, u"connection"_s, u"ogr"_s, false );

    QgsProcessingProviderConnectionWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingProviderConnectionWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProviderConnectionComboBox *>( wrapper.wrappedWidget() )->currentConnection(), u"bb"_s );
    wrapper.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy.count(), 1 );
    wrapper.setWidgetValue( u"aa"_s, context );
    QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
    QCOMPARE( spy.count(), 2 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        // batch or standard mode, only valid connections can be set!
        // not valid
        wrapper.setWidgetValue( u"cc"_s, context );
        QCOMPARE( spy.count(), 3 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast<QComboBox *>( wrapper.wrappedWidget() )->currentIndex(), -1 );
        break;
      }
      case Qgis::ProcessingMode::Modeler:
        // invalid connections permitted
        wrapper.setWidgetValue( u"cc"_s, context );
        QCOMPARE( spy.count(), 3 );
        QCOMPARE( static_cast<QgsProviderConnectionComboBox *>( wrapper.wrappedWidget() )->currentText(), u"cc"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"cc"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 4 );
        QCOMPARE( static_cast<QgsProviderConnectionComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        break;
    }

    delete w;
    // optional
    QgsProcessingParameterProviderConnection param2( u"conn"_s, u"connection"_s, u"ogr"_s, QVariant(), true );
    QgsProcessingProviderConnectionWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"aa"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"aa"_s );
    QCOMPARE( static_cast<QComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"aa"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;
    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"connection"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"providerconnection"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( !static_cast<QgsProcessingParameterProviderConnection *>( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterProviderConnection connParam( u"n"_s, u"test desc"_s, u"spatialite"_s, u"aaa"_s, false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"providerconnection"_s, context, widgetContext, &connParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterProviderConnection *>( def.get() )->defaultValue().toString(), u"aaa"_s );
  QCOMPARE( static_cast<QgsProcessingParameterProviderConnection *>( def.get() )->providerId(), u"spatialite"_s );
  connParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  connParam.setDefaultValue( u"xxx"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"providerconnection"_s, context, widgetContext, &connParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterProviderConnection *>( def.get() )->defaultValue().toString(), u"xxx"_s );
  connParam.setDefaultValue( QVariant() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"providerconnection"_s, context, widgetContext, &connParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QVERIFY( !static_cast<QgsProcessingParameterProviderConnection *>( def.get() )->defaultValue().isValid() );
}

void TestProcessingGui::testDatabaseSchemaWrapper()
{
#ifdef ENABLE_PGTEST
  // register some connections
  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"postgres"_s );

  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsAbstractProviderConnection *conn = md->createConnection( u"%1 sslmode=disable"_s.arg( dbConn ), QVariantMap() );
  md->saveConnection( conn, u"aa"_s );

  const QStringList schemas = dynamic_cast<QgsAbstractDatabaseProviderConnection *>( conn )->schemas();
  QVERIFY( !schemas.isEmpty() );

  auto testWrapper = [&schemas]( Qgis::ProcessingMode type ) {
    QgsProcessingParameterProviderConnection connParam( u"conn"_s, u"connection"_s, u"postgres"_s, QVariant(), true );
    TestLayerWrapper connWrapper( &connParam );

    QgsProcessingParameterDatabaseSchema param( u"schema"_s, u"schema"_s, u"conn"_s, QVariant(), false );

    QgsProcessingDatabaseSchemaWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    // no connection associated yet
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    // Set the parent widget connection value
    connWrapper.setWidgetValue( u"aa"_s, context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );

    // now we should have schemas available
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->count(), schemas.count() );

    QSignalSpy spy( &wrapper, &QgsProcessingDatabaseSchemaWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"qgis_test"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"qgis_test"_s );
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->currentSchema(), u"qgis_test"_s );
    wrapper.setWidgetValue( u"public"_s, context );
    QCOMPARE( wrapper.widgetValue().toString(), u"public"_s );
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->currentSchema(), u"public"_s );
    QCOMPARE( spy.count(), 2 );
    wrapper.setWidgetValue( u"public"_s, context );
    QCOMPARE( spy.count(), 2 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        // batch or standard mode, only valid schemas can be set!
        // not valid
        wrapper.setWidgetValue( u"cc"_s, context );
        QCOMPARE( spy.count(), 3 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentIndex(), -1 );
        break;
      }
      case Qgis::ProcessingMode::Modeler:
        // invalid schemas permitted
        wrapper.setWidgetValue( u"cc"_s, context );
        QCOMPARE( spy.count(), 3 );
        QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentText(), u"cc"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"cc"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 4 );
        QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentText(), u"aa"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        break;
    }

    // make sure things are ok if connection is changed back to nothing
    connWrapper.setWidgetValue( QVariant(), context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        QCOMPARE( spy.count(), 3 );
        break;
      }

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 5 );
        break;
    }
    QVERIFY( !wrapper.widgetValue().isValid() );

    wrapper.setWidgetValue( u"qgis_test"_s, context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        QVERIFY( !wrapper.widgetValue().isValid() );
        break;
      }

      case Qgis::ProcessingMode::Modeler:
        // invalid schemas permitted
        QCOMPARE( spy.count(), 6 );
        QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentText(), u"qgis_test"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"qgis_test"_s );

        break;
    }
    delete w;

    connWrapper.setWidgetValue( u"aa"_s, context );

    // optional
    QgsProcessingParameterDatabaseSchema param2( u"schema"_s, u"schema"_s, u"conn"_s, QVariant(), true );
    QgsProcessingDatabaseSchemaWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    wrapper3.setParentConnectionWrapperValue( &connWrapper );

    QSignalSpy spy3( &wrapper3, &QgsProcessingDatabaseSchemaWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"qgis_test"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"qgis_test"_s );
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper3.wrappedWidget() )->comboBox()->currentText(), u"qgis_test"_s );
    wrapper3.setWidgetValue( u"public"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"public"_s );
    QCOMPARE( static_cast<QgsDatabaseSchemaComboBox *>( wrapper3.wrappedWidget() )->comboBox()->currentText(), u"public"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );

    delete w;
    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"schema"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databaseschema"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( !static_cast<QgsProcessingParameterDatabaseSchema *>( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterDatabaseSchema schemaParam( u"n"_s, u"test desc"_s, u"connparam"_s, u"aaa"_s, false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databaseschema"_s, context, widgetContext, &schemaParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseSchema *>( def.get() )->defaultValue().toString(), u"aaa"_s );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseSchema *>( def.get() )->parentConnectionParameterName(), u"connparam"_s );
  schemaParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  schemaParam.setDefaultValue( u"xxx"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databaseschema"_s, context, widgetContext, &schemaParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseSchema *>( def.get() )->defaultValue().toString(), u"xxx"_s );
  schemaParam.setDefaultValue( QVariant() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databaseschema"_s, context, widgetContext, &schemaParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QVERIFY( !static_cast<QgsProcessingParameterDatabaseSchema *>( def.get() )->defaultValue().isValid() );
#endif
}

void TestProcessingGui::testDatabaseTableWrapper()
{
#ifdef ENABLE_PGTEST
  // register some connections
  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"postgres"_s );

  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsAbstractProviderConnection *conn = md->createConnection( u"%1 sslmode=disable"_s.arg( dbConn ), QVariantMap() );
  md->saveConnection( conn, u"aa"_s );

  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables = dynamic_cast<QgsAbstractDatabaseProviderConnection *>( conn )->tables( u"qgis_test"_s );
  QStringList tableNames;
  for ( const QgsAbstractDatabaseProviderConnection::TableProperty &prop : tables )
    tableNames << prop.tableName();

  QVERIFY( !tableNames.isEmpty() );

  auto testWrapper = [&tableNames]( Qgis::ProcessingMode type ) {
    QgsProcessingParameterProviderConnection connParam( u"conn"_s, u"connection"_s, u"postgres"_s, QVariant(), true );
    TestLayerWrapper connWrapper( &connParam );
    QgsProcessingParameterDatabaseSchema schemaParam( u"schema"_s, u"schema"_s, u"connection"_s, QVariant(), true );
    TestLayerWrapper schemaWrapper( &schemaParam );

    QgsProcessingParameterDatabaseTable param( u"table"_s, u"table"_s, u"conn"_s, u"schema"_s, QVariant(), false );

    QgsProcessingDatabaseTableWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    // no connection associated yet
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    // Set the parent widget connection value
    connWrapper.setWidgetValue( u"aa"_s, context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );
    schemaWrapper.setWidgetValue( u"qgis_test"_s, context );
    wrapper.setParentSchemaWrapperValue( &schemaWrapper );

    // now we should have tables available
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->count(), tableNames.count() );

    QSignalSpy spy( &wrapper, &QgsProcessingDatabaseTableWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"someData"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"someData"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->currentTable(), u"someData"_s );
    wrapper.setWidgetValue( u"some_poly_data"_s, context );
    QCOMPARE( wrapper.widgetValue().toString(), u"some_poly_data"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->currentTable(), u"some_poly_data"_s );
    QCOMPARE( spy.count(), 2 );
    wrapper.setWidgetValue( u"some_poly_data"_s, context );
    QCOMPARE( spy.count(), 2 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        // batch or standard mode, only valid tables can be set!
        // not valid
        wrapper.setWidgetValue( u"cc"_s, context );
        QCOMPARE( spy.count(), 3 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentIndex(), -1 );
        break;
      }
      case Qgis::ProcessingMode::Modeler:
        // invalid tables permitted
        wrapper.setWidgetValue( u"cc"_s, context );
        QCOMPARE( spy.count(), 3 );
        QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentText(), u"cc"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"cc"_s );
        wrapper.setWidgetValue( u"someData"_s, context );
        QCOMPARE( spy.count(), 4 );
        QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentText(), u"someData"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"someData"_s );
        break;
    }

    // make sure things are ok if connection is changed back to nothing
    connWrapper.setWidgetValue( QVariant(), context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        QCOMPARE( spy.count(), 3 );
        break;
      }

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 5 );
        break;
    }
    QVERIFY( !wrapper.widgetValue().isValid() );

    wrapper.setWidgetValue( u"some_poly_data"_s, context );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      {
        QVERIFY( !wrapper.widgetValue().isValid() );
        break;
      }

      case Qgis::ProcessingMode::Modeler:
        // invalid tables permitted
        QCOMPARE( spy.count(), 6 );
        QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper.wrappedWidget() )->comboBox()->currentText(), u"some_poly_data"_s );
        QCOMPARE( wrapper.widgetValue().toString(), u"some_poly_data"_s );

        break;
    }
    delete w;

    connWrapper.setWidgetValue( u"aa"_s, context );

    // optional
    QgsProcessingParameterDatabaseTable param2( u"table"_s, u"table"_s, u"conn"_s, u"schema"_s, QVariant(), true );
    QgsProcessingDatabaseTableWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    wrapper3.setParentConnectionWrapperValue( &connWrapper );
    wrapper3.setParentSchemaWrapperValue( &schemaWrapper );

    QSignalSpy spy3( &wrapper3, &QgsProcessingDatabaseTableWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"someData"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"someData"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper3.wrappedWidget() )->comboBox()->currentText(), u"someData"_s );
    wrapper3.setWidgetValue( u"some_poly_data"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"some_poly_data"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper3.wrappedWidget() )->comboBox()->currentText(), u"some_poly_data"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );

    delete w;

    // allowing new table names
    QgsProcessingParameterDatabaseTable param3( u"table"_s, u"table"_s, u"conn"_s, u"schema"_s, QVariant(), false, true );
    QgsProcessingDatabaseTableWidgetWrapper wrapper4( &param3, type );
    w = wrapper4.createWrappedWidget( context );

    wrapper4.setParentConnectionWrapperValue( &connWrapper );
    wrapper4.setParentSchemaWrapperValue( &schemaWrapper );

    QSignalSpy spy4( &wrapper4, &QgsProcessingDatabaseTableWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( u"someData"_s, context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toString(), u"someData"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper4.wrappedWidget() )->comboBox()->currentText(), u"someData"_s );
    wrapper4.setWidgetValue( u"some_poly_data"_s, context );
    QCOMPARE( spy4.count(), 2 );
    QCOMPARE( wrapper4.widgetValue().toString(), u"some_poly_data"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper4.wrappedWidget() )->comboBox()->currentText(), u"some_poly_data"_s );
    wrapper4.setWidgetValue( QVariant(), context );
    QCOMPARE( spy4.count(), 3 );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    // should always allow non existing table names
    wrapper4.setWidgetValue( u"someDataxxxxxxxxxxxxxxxxxxxx"_s, context );
    QCOMPARE( spy4.count(), 4 );
    QCOMPARE( wrapper4.widgetValue().toString(), u"someDataxxxxxxxxxxxxxxxxxxxx"_s );
    QCOMPARE( static_cast<QgsDatabaseTableComboBox *>( wrapper4.wrappedWidget() )->comboBox()->currentText(), u"someDataxxxxxxxxxxxxxxxxxxxx"_s );


    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"table"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databasetable"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QVERIFY( !static_cast<QgsProcessingParameterDatabaseTable *>( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterDatabaseTable tableParam( u"n"_s, u"test desc"_s, u"connparam"_s, u"schemaparam"_s, u"aaa"_s, false );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databasetable"_s, context, widgetContext, &tableParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseTable *>( def.get() )->defaultValue().toString(), u"aaa"_s );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseTable *>( def.get() )->parentConnectionParameterName(), u"connparam"_s );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseTable *>( def.get() )->parentSchemaParameterName(), u"schemaparam"_s );
  tableParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  tableParam.setDefaultValue( u"xxx"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databasetable"_s, context, widgetContext, &tableParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterDatabaseTable *>( def.get() )->defaultValue().toString(), u"xxx"_s );
  tableParam.setDefaultValue( QVariant() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"databasetable"_s, context, widgetContext, &tableParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QVERIFY( !static_cast<QgsProcessingParameterDatabaseTable *>( def.get() )->defaultValue().isValid() );
#endif
}

void TestProcessingGui::testFieldMapWidget()
{
  QgsProcessingFieldMapPanelWidget widget;
  widget.mSkipConfirmDialog = true;

  QVariantMap map;
  map.insert( u"name"_s, u"n"_s );
  map.insert( u"type"_s, static_cast<int>( QMetaType::Type::Double ) );
  map.insert( u"length"_s, 8 );
  map.insert( u"precision"_s, 5 );
  QVariantMap map2;
  map2.insert( u"name"_s, u"n2"_s );
  map2.insert( u"type"_s, static_cast<int>( QMetaType::Type::QString ) );
  map2.insert( u"expression"_s, u"'abc' || \"def\""_s );
  map2.insert( u"alias"_s, u"my alias"_s );
  map2.insert( u"comment"_s, u"my comment"_s );

  QSignalSpy spy( &widget, &QgsProcessingFieldMapPanelWidget::changed );
  widget.setValue( QVariantList() << map << map2 );
  QCOMPARE( spy.size(), 1 );

  QCOMPARE( widget.value().toList().size(), 2 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::Double ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"length"_s ).toInt(), 8 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"precision"_s ).toInt(), 5 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"expression"_s ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::QString ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"expression"_s ).toString(), u"'abc' || \"def\""_s );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"alias"_s ).toString(), u"my alias"_s );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"comment"_s ).toString(), u"my comment"_s );

  // Test load fields from memory layer, see issue GH #62019
  QgsFields templateFields;
  templateFields.append( QgsField( u"template_field_1"_s, QMetaType::Type::QString ) );
  std::unique_ptr<QgsVectorLayer> templateLayer( QgsMemoryProviderUtils::createMemoryLayer( u"source"_s, templateFields, Qgis::WkbType::Point ) );

  QgsFields sourceFields;
  sourceFields.append( QgsField( u"source_field_1"_s, QMetaType::Type::QString ) );
  std::unique_ptr<QgsVectorLayer> sourceLayer( QgsMemoryProviderUtils::createMemoryLayer( u"template"_s, sourceFields, Qgis::WkbType::Point ) );

  widget.setLayer( sourceLayer.get() );
  widget.loadFieldsFromLayer();

  // Check fields
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"source_field_1"_s );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::QString ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"expression"_s ).toString(), QStringLiteral( R"("source_field_1")" ) );

  QgsProject project;
  project.addMapLayer( sourceLayer.get(), false, false );
  project.addMapLayer( templateLayer.get(), false, false );
  widget.mLayerCombo->setProject( &project );

  widget.mLayerCombo->setLayer( templateLayer.get() );
  widget.loadLayerFields();

  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"template_field_1"_s );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::QString ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"expression"_s ).toString(), QStringLiteral( R"("source_field_1")" ) );
}

void TestProcessingGui::testFieldMapWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( u"native:centroids"_s );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( u"INPUT"_s );

  auto testWrapper = [layerDef]( Qgis::ProcessingMode type ) {
    QgsProcessingParameterFieldMapping param( u"mapping"_s, u"mapping"_s );

    QgsProcessingFieldMapWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    qobject_cast<QgsProcessingFieldMapPanelWidget *>( w )->mSkipConfirmDialog = true;

    QVariantMap map;
    map.insert( u"name"_s, u"n"_s );
    map.insert( u"type"_s, static_cast<int>( QMetaType::Type::Double ) );
    map.insert( u"length"_s, 8 );
    map.insert( u"precision"_s, 5 );
    QVariantMap map2;
    map2.insert( u"name"_s, u"n2"_s );
    map2.insert( u"type"_s, static_cast<int>( QMetaType::Type::QString ) );
    map2.insert( u"expression"_s, u"'abc' || \"def\""_s );
    map2.insert( u"alias"_s, u"my alias"_s );
    map2.insert( u"comment"_s, u"my comment"_s );

    QSignalSpy spy( &wrapper, &QgsProcessingFieldMapWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << map << map2, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::Double ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"length"_s ).toInt(), 8 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"precision"_s ).toInt(), 5 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"expression"_s ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::QString ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"expression"_s ).toString(), u"'abc' || \"def\""_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"alias"_s ).toString(), u"my alias"_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"comment"_s ).toString(), u"my comment"_s );

    QCOMPARE( static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper.wrappedWidget() )->value().toList().count(), 2 );
    QCOMPARE( static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper.wrappedWidget() )->value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
    QCOMPARE( static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper.wrappedWidget() )->value().toList().at( 1 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
    wrapper.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toList().size(), 1 );
    QCOMPARE( static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper.wrappedWidget() )->value().toList().size(), 1 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"mapping"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper.wrappedWidget() )->setValue( QVariantList() << map << map2 );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( u"other"_s );
    QgsProcessingFieldMapWidgetWrapper wrapper2( &param, type );
    w = wrapper2.createWrappedWidget( context );
    qobject_cast<QgsProcessingFieldMapPanelWidget *>( w )->mSkipConfirmDialog = true;

    QSignalSpy spy2( &wrapper2, &QgsProcessingFieldMapWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().size(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
    QCOMPARE( static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );

    wrapper2.setWidgetValue( QVariantList() << map2, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toList().size(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
    QCOMPARE( static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n2"_s );

    static_cast<QgsProcessingFieldMapPanelWidget *>( wrapper2.wrappedWidget() )->setValue( QVariantList() << map );
    QCOMPARE( spy2.count(), 3 );

    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( u"LineString"_s, u"x"_s, u"memory"_s );
    p.addMapLayer( vl );

    QVERIFY( !wrapper2.mPanel->layer() );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer(), vl );

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QVERIFY( !wrapper2.mPanel->layer() );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QVERIFY( !wrapper2.mPanel->layer() );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer(), vl );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointFileName = TEST_DATA_DIR + u"/points.shp"_s;
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer()->publicSource(), pointFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"fields_mapping"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFieldMapping mapParam( u"n"_s, u"test desc"_s, u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"fields_mapping"_s, context, widgetContext, &mapParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterFieldMapping *>( def.get() )->parentLayerParameterName(), u"parent"_s );
  mapParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  mapParam.setParentLayerParameterName( QString() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"fields_mapping"_s, context, widgetContext, &mapParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QVERIFY( static_cast<QgsProcessingParameterFieldMapping *>( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testAggregateWidget()
{
  QgsProcessingAggregatePanelWidget widget;

  QVariantMap map;
  map.insert( u"name"_s, u"n"_s );
  map.insert( u"type"_s, static_cast<int>( QMetaType::Type::Double ) );
  map.insert( u"length"_s, 8 );
  map.insert( u"precision"_s, 5 );
  QVariantMap map2;
  map2.insert( u"name"_s, u"n2"_s );
  map2.insert( u"type"_s, static_cast<int>( QMetaType::Type::QString ) );
  map2.insert( u"input"_s, u"'abc' || \"def\""_s );
  map2.insert( u"aggregate"_s, u"concatenate"_s );
  map2.insert( u"delimiter"_s, u"|"_s );

  QSignalSpy spy( &widget, &QgsProcessingAggregatePanelWidget::changed );
  widget.setValue( QVariantList() << map << map2 );
  QCOMPARE( spy.size(), 1 );

  QCOMPARE( widget.value().toList().size(), 2 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::Double ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"length"_s ).toInt(), 8 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"precision"_s ).toInt(), 5 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"input"_s ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"aggregate"_s ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( u"delimiter"_s ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::QString ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"input"_s ).toString(), u"'abc' || \"def\""_s );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"aggregate"_s ).toString(), u"concatenate"_s );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( u"delimiter"_s ).toString(), u"|"_s );
}

void TestProcessingGui::testAggregateWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( u"native:centroids"_s );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( u"INPUT"_s );

  auto testWrapper = [layerDef]( Qgis::ProcessingMode type ) {
    QgsProcessingParameterAggregate param( u"mapping"_s, u"mapping"_s );

    QgsProcessingAggregateWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    qobject_cast<QgsProcessingAggregatePanelWidget *>( w )->mSkipConfirmDialog = true;

    QVariantMap map;
    map.insert( u"name"_s, u"n"_s );
    map.insert( u"type"_s, static_cast<int>( QMetaType::Type::Double ) );
    map.insert( u"length"_s, 8 );
    map.insert( u"precision"_s, 5 );
    QVariantMap map2;
    map2.insert( u"name"_s, u"n2"_s );
    map2.insert( u"type"_s, static_cast<int>( QMetaType::Type::QString ) );
    map2.insert( u"input"_s, u"'abc' || \"def\""_s );
    map2.insert( u"aggregate"_s, u"concatenate"_s );
    map2.insert( u"delimiter"_s, u"|"_s );

    QSignalSpy spy( &wrapper, &QgsProcessingFieldMapWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << map << map2, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::Double ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"length"_s ).toInt(), 8 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"precision"_s ).toInt(), 5 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"input"_s ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"aggregate"_s ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( u"delimiter"_s ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"type"_s ).toInt(), static_cast<int>( QMetaType::Type::QString ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"input"_s ).toString(), u"'abc' || \"def\""_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"aggregate"_s ).toString(), u"concatenate"_s );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( u"delimiter"_s ).toString(), u"|"_s );

    QCOMPARE( static_cast<QgsProcessingAggregatePanelWidget *>( wrapper.wrappedWidget() )->value().toList().count(), 2 );
    QCOMPARE( static_cast<QgsProcessingAggregatePanelWidget *>( wrapper.wrappedWidget() )->value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
    QCOMPARE( static_cast<QgsProcessingAggregatePanelWidget *>( wrapper.wrappedWidget() )->value().toList().at( 1 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
    wrapper.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toList().size(), 1 );
    QCOMPARE( static_cast<QgsProcessingAggregatePanelWidget *>( wrapper.wrappedWidget() )->value().toList().size(), 1 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"mapping"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast<QgsProcessingAggregatePanelWidget *>( wrapper.wrappedWidget() )->setValue( QVariantList() << map << map2 );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( u"other"_s );
    QgsProcessingAggregateWidgetWrapper wrapper2( &param, type );
    w = wrapper2.createWrappedWidget( context );
    qobject_cast<QgsProcessingAggregatePanelWidget *>( w )->mSkipConfirmDialog = true;

    QSignalSpy spy2( &wrapper2, &QgsProcessingAggregateWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().size(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );
    QCOMPARE( static_cast<QgsProcessingAggregatePanelWidget *>( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n"_s );

    wrapper2.setWidgetValue( QVariantList() << map2, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toList().size(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n2"_s );
    QCOMPARE( static_cast<QgsProcessingAggregatePanelWidget *>( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( u"name"_s ).toString(), u"n2"_s );

    static_cast<QgsProcessingAggregatePanelWidget *>( wrapper2.wrappedWidget() )->setValue( QVariantList() << map );
    QCOMPARE( spy2.count(), 3 );


    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( u"LineString"_s, u"x"_s, u"memory"_s );
    p.addMapLayer( vl );

    QVERIFY( !wrapper2.mPanel->layer() );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer(), vl );

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QVERIFY( !wrapper2.mPanel->layer() );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QVERIFY( !wrapper2.mPanel->layer() );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer(), vl );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointFileName = TEST_DATA_DIR + u"/points.shp"_s;
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer()->publicSource(), pointFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"aggregates"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterAggregate mapParam( u"n"_s, u"test desc"_s, u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"aggregates"_s, context, widgetContext, &mapParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterAggregate *>( def.get() )->parentLayerParameterName(), u"parent"_s );
  mapParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  mapParam.setParentLayerParameterName( QString() );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"aggregates"_s, context, widgetContext, &mapParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QVERIFY( static_cast<QgsProcessingParameterAggregate *>( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testOutputDefinitionWidget()
{
  QgsProcessingParameterFeatureSink sink( u"test"_s );
  QgsProcessingLayerOutputDestinationWidget panel( &sink, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  QgsProcessingOutputLayerDefinition def;
  def.sink.setStaticValue( QgsProcessing::TEMPORARY_OUTPUT );
  def.createOptions.insert( u"fileEncoding"_s, u"utf8"_s );
  panel.setValue( def );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );

  panel.setValue( u"memory:"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  def.sink.setStaticValue( u"memory:"_s );
  panel.setValue( def );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );

  panel.setValue( u"ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''"_s );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( u"ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  panel.setValue( u"postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql="_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql="_s );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );
  panel.setValue( u"postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql="_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );

  panel.setValue( u"/home/me/test.shp"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/home/me/test.shp"_s );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( u"/home/me/test.shp"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( u"/home/me/test2.shp"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 4 );

  QgsSettings settings;
  settings.setValue( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, TEST_DATA_DIR );
  panel.setValue( u"test.shp"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"utf8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + u"/test.shp"_s ) );

  // optional, test skipping
  sink.setFlags( sink.flags() | Qgis::ProcessingParameterFlag::Optional );
  sink.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &sink, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );

  panel2.setValue( QVariant() );
  v = panel2.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 1 );
  QCOMPARE( changedSpy2.count(), 1 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 2 );
  QCOMPARE( changedSpy2.count(), 2 );

  sink.setCreateByDefault( false );
  QgsProcessingLayerOutputDestinationWidget panel3( &sink, false );

  QSignalSpy skipSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );

  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel3.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );

  panel3.setValue( QVariant() );
  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 2 );
  QCOMPARE( changedSpy3.count(), 2 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 3 );
  QCOMPARE( changedSpy3.count(), 3 );

  // with remapping
  def = QgsProcessingOutputLayerDefinition( u"test.shp"_s );
  QgsRemappingSinkDefinition remap;
  QMap<QString, QgsProperty> fieldMap;
  fieldMap.insert( u"field1"_s, QgsProperty::fromField( u"source1"_s ) );
  fieldMap.insert( u"field2"_s, QgsProperty::fromExpression( u"source || source2"_s ) );
  remap.setFieldMap( fieldMap );
  def.setRemappingDefinition( remap );

  panel3.setValue( def );
  v = panel3.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QVERIFY( v.value<QgsProcessingOutputLayerDefinition>().useRemapping() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().remappingDefinition().fieldMap().size(), 2 );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().remappingDefinition().fieldMap().value( u"field1"_s ), QgsProperty::fromField( u"source1"_s ) );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().remappingDefinition().fieldMap().value( u"field2"_s ), QgsProperty::fromExpression( u"source || source2"_s ) );

  panel3.setValue( u"other.shp"_s );
  v = panel3.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QVERIFY( !v.value<QgsProcessingOutputLayerDefinition>().useRemapping() );
}

void TestProcessingGui::testOutputDefinitionWidgetVectorOut()
{
  QgsProcessingParameterVectorDestination vector( u"test"_s );
  QgsProcessingLayerOutputDestinationWidget panel( &vector, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( u"memory:"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );


  panel.setValue( u"ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''"_s );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( u"ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  panel.setValue( u"postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql="_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql="_s );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );
  panel.setValue( u"postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql="_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );

  panel.setValue( u"/home/me/test.shp"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/home/me/test.shp"_s );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( u"/home/me/test.shp"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( u"/home/me/test2.shp"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 4 );

  QgsSettings settings;
  settings.setValue( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, TEST_DATA_DIR );
  panel.setValue( u"test.shp"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + u"/test.shp"_s ) );

  // optional, test skipping
  vector.setFlags( vector.flags() | Qgis::ProcessingParameterFlag::Optional );
  vector.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &vector, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );

  panel2.setValue( QVariant() );
  v = panel2.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 1 );
  QCOMPARE( changedSpy2.count(), 1 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 2 );
  QCOMPARE( changedSpy2.count(), 2 );

  vector.setCreateByDefault( false );
  QgsProcessingLayerOutputDestinationWidget panel3( &vector, false );

  QSignalSpy skipSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );

  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel3.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );

  panel3.setValue( QVariant() );
  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 2 );
  QCOMPARE( changedSpy3.count(), 2 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 3 );
  QCOMPARE( changedSpy3.count(), 3 );
}

void TestProcessingGui::testOutputDefinitionWidgetRasterOut()
{
  QgsProcessingParameterRasterDestination raster( u"test"_s );
  QgsProcessingLayerOutputDestinationWidget panel( &raster, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( u"/home/me/test.tif"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/home/me/test.tif"_s );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( u"/home/me/test.tif"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, TEST_DATA_DIR );
  panel.setValue( u"test.tif"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + u"/test.tif"_s ) );

  // optional, test skipping
  raster.setFlags( raster.flags() | Qgis::ProcessingParameterFlag::Optional );
  raster.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &raster, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );

  panel2.setValue( QVariant() );
  v = panel2.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 1 );
  QCOMPARE( changedSpy2.count(), 1 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 2 );
  QCOMPARE( changedSpy2.count(), 2 );

  raster.setCreateByDefault( false );
  QgsProcessingLayerOutputDestinationWidget panel3( &raster, false );

  QSignalSpy skipSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );

  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel3.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );

  panel3.setValue( QVariant() );
  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 2 );
  QCOMPARE( changedSpy3.count(), 2 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 3 );
  QCOMPARE( changedSpy3.count(), 3 );
}

void TestProcessingGui::testOutputDefinitionWidgetPointCloudOut()
{
  QgsProcessingParameterPointCloudDestination pointCloud( u"test"_s );
  QgsProcessingLayerOutputDestinationWidget panel( &pointCloud, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( u"/home/me/test.las"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/home/me/test.las"_s );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( u"/home/me/test.las"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, TEST_DATA_DIR );
  panel.setValue( u"test.las"_s );
  v = panel.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + u"/test.las"_s ) );

  // optional, test skipping
  pointCloud.setFlags( pointCloud.flags() | Qgis::ProcessingParameterFlag::Optional );
  pointCloud.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &pointCloud, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );

  panel2.setValue( QVariant() );
  v = panel2.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 1 );
  QCOMPARE( changedSpy2.count(), 1 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 2 );
  QCOMPARE( changedSpy2.count(), 2 );

  pointCloud.setCreateByDefault( false );
  QgsProcessingLayerOutputDestinationWidget panel3( &pointCloud, false );

  QSignalSpy skipSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );

  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel3.value();
  QCOMPARE( v.userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().createOptions.value( u"fileEncoding"_s ).toString(), u"UTF-8"_s );
  QCOMPARE( v.value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );

  panel3.setValue( QVariant() );
  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 2 );
  QCOMPARE( changedSpy3.count(), 2 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 3 );
  QCOMPARE( changedSpy3.count(), 3 );
}

void TestProcessingGui::testOutputDefinitionWidgetFolder()
{
  QgsProcessingParameterFolderDestination folder( u"test"_s );
  QgsProcessingLayerOutputDestinationWidget panel( &folder, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( u"/home/me/"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.toString(), u"/home/me/"_s );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( u"/home/me/"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, TEST_DATA_DIR );
  panel.setValue( u"mystuff"_s );
  v = panel.value();
  QCOMPARE( v.toString(), QString( TEST_DATA_DIR + u"/mystuff"_s ) );

  // optional, test skipping
  folder.setFlags( folder.flags() | Qgis::ProcessingParameterFlag::Optional );
  folder.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &folder, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );

  panel2.setValue( QVariant() );
  v = panel2.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 1 );
  QCOMPARE( changedSpy2.count(), 1 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 2 );
  QCOMPARE( changedSpy2.count(), 2 );

  folder.setCreateByDefault( false );
  QgsProcessingLayerOutputDestinationWidget panel3( &folder, false );

  QSignalSpy skipSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );

  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel3.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );

  panel3.setValue( QVariant() );
  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 2 );
  QCOMPARE( changedSpy3.count(), 2 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 3 );
  QCOMPARE( changedSpy3.count(), 3 );
}

void TestProcessingGui::testOutputDefinitionWidgetFileOut()
{
  QgsProcessingParameterFileDestination file( u"test"_s );
  QgsProcessingLayerOutputDestinationWidget panel( &file, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( u"/home/me/test.tif"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.toString(), u"/home/me/test.tif"_s );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( u"/home/me/test.tif"_s );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, TEST_DATA_DIR );
  panel.setValue( u"test.tif"_s );
  v = panel.value();
  QCOMPARE( v.toString(), QString( TEST_DATA_DIR + u"/test.tif"_s ) );

  // optional, test skipping
  file.setFlags( file.flags() | Qgis::ProcessingParameterFlag::Optional );
  file.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &file, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 0 );
  QCOMPARE( changedSpy2.count(), 0 );

  panel2.setValue( QVariant() );
  v = panel2.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel2.outputIsSkipped() );
  QCOMPARE( skipSpy2.count(), 1 );
  QCOMPARE( changedSpy2.count(), 1 );
  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy2.count(), 2 );
  QCOMPARE( changedSpy2.count(), 2 );

  file.setCreateByDefault( false );
  QgsProcessingLayerOutputDestinationWidget panel3( &file, false );

  QSignalSpy skipSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy3( &panel3, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );

  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel3.value();
  QCOMPARE( v.toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 1 );
  QCOMPARE( changedSpy3.count(), 1 );

  panel3.setValue( QVariant() );
  v = panel3.value();
  QVERIFY( !v.isValid() );
  QVERIFY( panel3.outputIsSkipped() );
  QCOMPARE( skipSpy3.count(), 2 );
  QCOMPARE( changedSpy3.count(), 2 );
  panel3.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy3.count(), 3 );
  QCOMPARE( changedSpy3.count(), 3 );
}

void TestProcessingGui::testFeatureSourceOptionsWidget()
{
  QgsProcessingFeatureSourceOptionsWidget w;
  QSignalSpy spy( &w, &QgsProcessingFeatureSourceOptionsWidget::widgetChanged );

  w.setFeatureLimit( 66 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.featureLimit(), 66 );
  w.setFeatureLimit( 66 );
  QCOMPARE( spy.count(), 1 );
  w.setFeatureLimit( -1 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.featureLimit(), -1 );

  w.setGeometryCheckMethod( false, Qgis::InvalidGeometryCheck::SkipInvalid );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( !w.isOverridingInvalidGeometryCheck() );
  w.setGeometryCheckMethod( true, Qgis::InvalidGeometryCheck::SkipInvalid );
  QCOMPARE( spy.count(), 3 );
  QVERIFY( w.isOverridingInvalidGeometryCheck() );
  QCOMPARE( w.geometryCheckMethod(), Qgis::InvalidGeometryCheck::SkipInvalid );
  w.setGeometryCheckMethod( true, Qgis::InvalidGeometryCheck::SkipInvalid );
  QCOMPARE( spy.count(), 3 );
  w.setGeometryCheckMethod( true, Qgis::InvalidGeometryCheck::AbortOnInvalid );
  QCOMPARE( spy.count(), 4 );
  QVERIFY( w.isOverridingInvalidGeometryCheck() );
  QCOMPARE( w.geometryCheckMethod(), Qgis::InvalidGeometryCheck::AbortOnInvalid );
  w.setGeometryCheckMethod( false, Qgis::InvalidGeometryCheck::AbortOnInvalid );
  QVERIFY( !w.isOverridingInvalidGeometryCheck() );
  QCOMPARE( spy.count(), 5 );

  w.setFilterExpression( u"name='test'"_s );
  QCOMPARE( spy.count(), 6 );
  QCOMPARE( w.filterExpression(), u"name='test'"_s );
  w.setFilterExpression( u"name='test'"_s );
  QCOMPARE( spy.count(), 6 );
  w.setFilterExpression( QString() );
  QCOMPARE( spy.count(), 7 );
  QCOMPARE( w.filterExpression(), QString() );
}

void TestProcessingGui::testVectorOutWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterVectorDestination param( u"vector"_s, u"vector"_s );

    QgsProcessingVectorDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingVectorDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"/bb.shp"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
        wrapper.setWidgetValue( u"/aa.shp"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/aa.shp"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/aa.shp"_s );
        break;
    }

    // check signal
    static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->setValue( u"/cc.shp"_s );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/cc.shp"_s );
    delete w;

    // optional
    QgsProcessingParameterVectorDestination param2( u"vector"_s, u"vector"_s, Qgis::ProcessingSourceType::Vector, QVariant(), true );
    QgsProcessingVectorDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingVectorDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"/bb.shp"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
    QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper3.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"vector"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  // testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testSinkWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterFeatureSink param( u"sink"_s, u"sink"_s );

    QgsProcessingFeatureSinkWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFeatureSinkWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"/bb.shp"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
        wrapper.setWidgetValue( u"/aa.shp"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/aa.shp"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/aa.shp"_s );
        break;
    }

    // check signal
    static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->setValue( u"/cc.shp"_s );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/cc.shp"_s );
    delete w;

    // optional
    QgsProcessingParameterFeatureSink param2( u"sink"_s, u"sink"_s, Qgis::ProcessingSourceType::Vector, QVariant(), true );
    QgsProcessingFeatureSinkWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFeatureSinkWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"/bb.shp"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
    QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper3.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.shp"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"sink"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  // testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testRasterOutWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterRasterDestination param( u"raster"_s, u"raster"_s );

    QgsProcessingRasterDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingRasterDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"/bb.tif"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.tif"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.tif"_s );
        wrapper.setWidgetValue( u"/aa.tif"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/aa.tif"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/aa.tif"_s );
        break;
    }

    // check signal
    static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->setValue( u"/cc.tif"_s );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/cc.tif"_s );
    delete w;

    // optional
    QgsProcessingParameterRasterDestination param2( u"raster"_s, u"raster"_s, QVariant(), true );
    QgsProcessingRasterDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingRasterDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"/bb.tif"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.tif"_s );
    QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper3.wrappedWidget() )->value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"/bb.tif"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"raster"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  // testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testFileOutWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterFileDestination param( u"file"_s, u"file"_s );

    QgsProcessingFileDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFileDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"/bb.tif"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"/bb.tif"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().toString(), u"/bb.tif"_s );
        wrapper.setWidgetValue( u"/aa.tif"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"/aa.tif"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().toString(), u"/aa.tif"_s );
        break;
    }

    // check signal
    static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->setValue( u"/cc.tif"_s );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toString(), u"/cc.tif"_s );
    delete w;

    // optional
    QgsProcessingParameterFileDestination param2( u"file"_s, u"file"_s, QString(), QVariant(), true );
    QgsProcessingFileDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFileDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"/bb.tif"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"/bb.tif"_s );
    QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper3.wrappedWidget() )->value().toString(), u"/bb.tif"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"file"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  // testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testFolderOutWrapper()
{
  auto testWrapper = []( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterFolderDestination param( u"folder"_s, u"folder"_s );

    QgsProcessingFolderDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFolderDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"/bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"/bb"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().toString(), u"/bb"_s );
        wrapper.setWidgetValue( u"/aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"/aa"_s );
        QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->value().toString(), u"/aa"_s );
        break;
    }

    // check signal
    static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper.wrappedWidget() )->setValue( u"/cc"_s );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toString(), u"/cc"_s );
    delete w;

    // optional
    QgsProcessingParameterFolderDestination param2( u"folder"_s, u"folder"_s, QVariant(), true );
    QgsProcessingFolderDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFolderDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"/bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"/bb"_s );
    QCOMPARE( static_cast<QgsProcessingLayerOutputDestinationWidget *>( wrapper3.wrappedWidget() )->value().toString(), u"/bb"_s );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"folder"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  // testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testTinInputLayerWrapper()
{
  QgsProcessingParameterTinInputLayers definition( u"TIN input layers"_s );
  QgsProcessingTinInputLayersWidgetWrapper wrapper;

  std::unique_ptr<QWidget> w( wrapper.createWidget() );
  QVERIFY( w );

  QSignalSpy spy( &wrapper, &QgsProcessingTinInputLayersWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QgsProject project;
  context.setProject( &project );
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( u"Point"_s, u"PointLayerForTin"_s, u"memory"_s );
  project.addMapLayer( vectorLayer );

  QVariantList layerList;
  QVariantMap layerMap;
  layerMap["source"] = "PointLayerForTin";
  layerMap["type"] = 0;
  layerMap["attributeIndex"] = -1;
  layerList.append( layerMap );

  QVERIFY( definition.checkValueIsAcceptable( layerList, &context ) );
  wrapper.setWidgetValue( layerList, context );
  QCOMPARE( spy.count(), 1 );

  QVariant value = wrapper.widgetValue();

  QVERIFY( definition.checkValueIsAcceptable( value, &context ) );
  QString valueAsPythonString = definition.valueAsPythonString( value, context );
  QCOMPARE( valueAsPythonString, u"[{'source': 'PointLayerForTin','type': 0,'attributeIndex': -1}]"_s );
}

void TestProcessingGui::testDxfLayersWrapper()
{
  QgsProcessingParameterDxfLayers definition( u"DXF layers"_s );
  QgsProcessingDxfLayersWidgetWrapper wrapper;

  std::unique_ptr<QWidget> w( wrapper.createWidget() );
  QVERIFY( w );

  QSignalSpy spy( &wrapper, &QgsProcessingTinInputLayersWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QgsProject project;
  context.setProject( &project );
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( u"Point"_s, u"PointLayer"_s, u"memory"_s );
  project.addMapLayer( vectorLayer );

  QVariantList layerList;
  QVariantMap layerMap;
  layerMap["layer"] = "PointLayer";
  layerMap["attributeIndex"] = -1;
  layerMap["overriddenLayerName"] = QString();
  layerMap["buildDataDefinedBlocks"] = DEFAULT_DXF_DATA_DEFINED_BLOCKS;
  layerMap["dataDefinedBlocksMaximumNumberOfClasses"] = -1;
  layerList.append( layerMap );

  QVERIFY( definition.checkValueIsAcceptable( layerList, &context ) );
  wrapper.setWidgetValue( layerList, context );
  QCOMPARE( spy.count(), 1 );

  QVariant value = wrapper.widgetValue();

  QVERIFY( definition.checkValueIsAcceptable( value, &context ) );
  QString valueAsPythonString = definition.valueAsPythonString( value, context );
  QCOMPARE( valueAsPythonString, u"[{'layer': '%1','attributeIndex': -1,'overriddenLayerName': '','buildDataDefinedBlocks': True,'dataDefinedBlocksMaximumNumberOfClasses': -1}]"_s.arg( vectorLayer->source() ) );
}

void TestProcessingGui::testAlignRasterLayersWrapper()
{
  QgsProcessingParameterAlignRasterLayers definition( u"Raster layers"_s );
  QgsProcessingAlignRasterLayersWidgetWrapper wrapper;

  std::unique_ptr<QWidget> w( wrapper.createWidget() );
  QVERIFY( w );

  QSignalSpy spy( &wrapper, &QgsProcessingTinInputLayersWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QgsProject project;
  context.setProject( &project );
  QgsRasterLayer *rasterLayer = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", u"raster"_s );
  project.addMapLayer( rasterLayer );

  QVariantList layerList;
  QVariantMap layerMap;
  layerMap["inputFile"] = rasterLayer->source();
  layerMap["outputFile"] = "";
  layerMap["resampleMethod"] = 1;
  layerMap["rescale"] = false;
  layerList.append( layerMap );

  QVERIFY( definition.checkValueIsAcceptable( layerList, &context ) );
  wrapper.setWidgetValue( layerList, context );
  QCOMPARE( spy.count(), 1 );

  QVariant value = wrapper.widgetValue();

  QVERIFY( definition.checkValueIsAcceptable( value, &context ) );
  QString valueAsPythonString = definition.valueAsPythonString( value, context );
  QCOMPARE( valueAsPythonString, u"[{'inputFile': '%1','outputFile': '%2','resampleMethod': 1,'rescale': False}]"_s.arg( rasterLayer->source() ).arg( layerMap["outputFile"].toString() ) );
}

void TestProcessingGui::testRasterOptionsWrapper()
{
  QgsProcessingParameterString param( u"string"_s, u"string"_s );
  param.setMetadata( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) }
  } );

  QgsProcessingContext context;
  QgsProcessingRasterOptionsWidgetWrapper wrapper( &param );

  std::unique_ptr<QWidget> w( wrapper.createWidget() );
  QVERIFY( w );

  QSignalSpy spy( &wrapper, &QgsProcessingRasterOptionsWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( u"TFW=YES"_s, context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toString(), u"TFW=YES"_s );
  wrapper.setWidgetValue( u"TFW=YES TILED=YES"_s, context );
  QCOMPARE( wrapper.widgetValue().toString(), u"TFW=YES|TILED=YES"_s );
  wrapper.setWidgetValue( u"TFW=YES|TILED=NO"_s, context );
  QCOMPARE( wrapper.widgetValue().toString(), u"TFW=YES|TILED=NO"_s );
}

void TestProcessingGui::testMeshDatasetWrapperLayerInProject()
{
  QgsProcessingParameterMeshLayer layerDefinition( u"layer"_s, u"layer"_s );
  QgsProcessingMeshLayerWidgetWrapper layerWrapper( &layerDefinition );

  QSet<int> supportedDataType( { QgsMeshDatasetGroupMetadata::DataOnVertices } );
  QgsProcessingParameterMeshDatasetGroups groupsDefinition( u"groups"_s, u"groups"_s, u"layer"_s, supportedDataType );
  QgsProcessingMeshDatasetGroupsWidgetWrapper groupsWrapper( &groupsDefinition );

  QgsProcessingParameterMeshDatasetTime timeDefinition( u"time"_s, u"time"_s, u"layer"_s, u"groups"_s );
  QgsProcessingMeshDatasetTimeWidgetWrapper timeWrapper( &timeDefinition );

  QList<QgsAbstractProcessingParameterWidgetWrapper *> wrappers;
  wrappers << &layerWrapper << &groupsWrapper << &timeWrapper;

  QgsProject project;
  QgsProcessingContext context;
  context.setProject( &project );
  QgsProcessingParameterWidgetContext widgetContext;
  auto mapCanvas = std::make_unique<QgsMapCanvas>();
  widgetContext.setMapCanvas( mapCanvas.get() );

  widgetContext.setProject( &project );
  layerWrapper.setWidgetContext( widgetContext );
  groupsWrapper.setWidgetContext( widgetContext );
  timeWrapper.setWidgetContext( widgetContext );

  TestProcessingContextGenerator generator( context );
  layerWrapper.registerProcessingContextGenerator( &generator );
  groupsWrapper.registerProcessingContextGenerator( &generator );
  timeWrapper.registerProcessingContextGenerator( &generator );

  QSignalSpy layerSpy( &layerWrapper, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
  QSignalSpy groupsSpy( &groupsWrapper, &QgsProcessingMeshDatasetGroupsWidgetWrapper::widgetValueHasChanged );
  QSignalSpy timeSpy( &timeWrapper, &QgsProcessingMeshDatasetTimeWidgetWrapper::widgetValueHasChanged );

  std::unique_ptr<QWidget> layerWidget( layerWrapper.createWrappedWidget( context ) );
  std::unique_ptr<QWidget> groupWidget( groupsWrapper.createWrappedWidget( context ) );
  std::unique_ptr<QWidget> timeWidget( timeWrapper.createWrappedWidget( context ) );
  QgsProcessingMeshDatasetGroupsWidget *datasetGroupWidget = qobject_cast<QgsProcessingMeshDatasetGroupsWidget *>( groupWidget.get() );
  QgsProcessingMeshDatasetTimeWidget *datasetTimeWidget = qobject_cast<QgsProcessingMeshDatasetTimeWidget *>( timeWidget.get() );

  QVERIFY( layerWidget );
  QVERIFY( groupWidget );
  QVERIFY( datasetGroupWidget );
  QVERIFY( timeWidget );

  groupsWrapper.postInitialize( wrappers );
  timeWrapper.postInitialize( wrappers );

  QString dataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  dataDir += "/mesh";
  QString uri( dataDir + "/quad_and_triangle.2dm" );
  QString meshLayerName = u"mesh layer"_s;
  QgsMeshLayer *layer = new QgsMeshLayer( uri, meshLayerName, u"mdal"_s );
  QVERIFY( layer->isValid() );
  layer->addDatasets( dataDir + "/quad_and_triangle_vertex_scalar.dat" );
  layer->addDatasets( dataDir + "/quad_and_triangle_vertex_vector.dat" );
  layer->addDatasets( dataDir + "/quad_and_triangle_els_face_scalar.dat" );
  layer->addDatasets( dataDir + "/quad_and_triangle_els_face_vector.dat" );
  QgsMeshRendererSettings settings = layer->rendererSettings();
  // 1 dataset on vertices and 1 dataset on faces
  settings.setActiveScalarDatasetGroup( 1 );
  settings.setActiveVectorDatasetGroup( 4 );
  layer->setRendererSettings( settings );
  QCOMPARE( layer->datasetGroupCount(), 5 );

  layerSpy.clear();
  groupsSpy.clear();
  timeSpy.clear();

  // without layer in the project
  QString meshOutOfProject( dataDir + "/trap_steady_05_3D.nc" );
  layerWrapper.setWidgetValue( meshOutOfProject, context );

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 1 );
  QCOMPARE( timeSpy.count(), 1 );

  QVERIFY( datasetTimeWidget->radioButtonDatasetGroupTimeStep->isChecked() );

  QVariantList groups;
  groups << 0;
  groupsWrapper.setWidgetValue( groups, context );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsWrapper.widgetValue() ), QList<int>() << 0 );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"static"_s );

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 2 );
  QCOMPARE( timeSpy.count(), 3 );

  // with layer in the project
  layerSpy.clear();
  groupsSpy.clear();
  timeSpy.clear();

  project.addMapLayer( layer );
  static_cast<QgsMeshLayerTemporalProperties *>( layer->temporalProperties() )->setReferenceTime( QDateTime( QDate( 2020, 01, 01 ), QTime( 0, 0, 0 ), Qt::UTC ), layer->dataProvider()->temporalCapabilities() );
  layerWrapper.setWidgetValue( meshLayerName, context );

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 1 );
  QCOMPARE( timeSpy.count(), 2 );

  datasetGroupWidget->selectCurrentActiveDatasetGroup();

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 2 );
  QCOMPARE( timeSpy.count(), 3 );

  QVariant groupsValue = groupsWrapper.widgetValue();
  QVERIFY( groupsValue.userType() == QMetaType::Type::QVariantList );
  QVariantList groupsList = groupsValue.toList();
  QCOMPARE( groupsList.count(), 1 );
  QCOMPARE( groupsList.at( 0 ).toInt(), 1 );
  QString pythonString = groupsDefinition.valueAsPythonString( groupsValue, context );
  QCOMPARE( pythonString, u"[1]"_s );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsValue ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsValue ), QList<int>( { 1 } ) );

  // 2 datasets on vertices
  settings = layer->rendererSettings();
  settings.setActiveVectorDatasetGroup( 2 );
  layer->setRendererSettings( settings );
  datasetGroupWidget->selectCurrentActiveDatasetGroup();

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 4 );

  pythonString = groupsDefinition.valueAsPythonString( groupsWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"[1,2]"_s );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsWrapper.widgetValue() ), QList<int>() << 1 << 2 );

  datasetTimeWidget->radioButtonDatasetGroupTimeStep->setChecked( true );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 4 ); //radioButtonDatasetGroupTimeStep already checked

  QVariant timeValue = timeWrapper.widgetValue();
  QVERIFY( timeValue.userType() == QMetaType::Type::QVariantMap );
  QVariantMap timeValueMap = timeValue.toMap();
  QCOMPARE( timeValueMap[u"type"_s].toString(), u"dataset-time-step"_s );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'dataset-time-step','value': [1,0]}"_s );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeValue ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeValue ), u"dataset-time-step"_s );
  QVERIFY( QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( timeValue ) == QgsMeshDatasetIndex( 1, 0 ) );

  datasetTimeWidget->radioButtonDefinedDateTime->setChecked( true );
  QDateTime dateTime = QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 1, 0 ), Qt::UTC );
  datasetTimeWidget->dateTimeEdit->setDateTime( dateTime );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 6 );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'defined-date-time','value': QDateTime(QDate(2020, 1, 1), QTime(0, 1, 0))}"_s );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"defined-date-time"_s );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( timeWrapper.widgetValue() ), dateTime );

  QVERIFY( !datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );
  mapCanvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2021, 1, 1 ), QTime( 0, 3, 0 ), Qt::UTC ), QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 5, 0 ), Qt::UTC ) ) );
  QVERIFY( datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );

  datasetTimeWidget->radioButtonCurrentCanvasTime->setChecked( true );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 8 );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'current-context-time'}"_s );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"current-context-time"_s );

  // 0 dataset on vertices
  settings = layer->rendererSettings();
  settings.setActiveScalarDatasetGroup( -1 );
  settings.setActiveVectorDatasetGroup( -1 );
  layer->setRendererSettings( settings );
  datasetGroupWidget->selectCurrentActiveDatasetGroup();
  QVERIFY( !datasetTimeWidget->isEnabled() );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'static'}"_s );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"static"_s );

  // 1 static dataset on vertices
  settings = layer->rendererSettings();
  settings.setActiveScalarDatasetGroup( 0 );
  settings.setActiveVectorDatasetGroup( -1 );
  layer->setRendererSettings( settings );
  datasetGroupWidget->selectCurrentActiveDatasetGroup();
  QVERIFY( !datasetTimeWidget->isEnabled() );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'static'}"_s );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"static"_s );

  groupsWrapper.setWidgetValue( 3, context );
  QCOMPARE( datasetGroupWidget->value(), QVariantList() << 3 );
  groupsWrapper.setWidgetValue( QVariantList( { 1, 2, 3 } ), context );
  QCOMPARE( datasetGroupWidget->value().toList(), QVariantList( { 1, 2, 3 } ) );
  groupsWrapper.setWidgetValue( QVariantList( { "1", "2", "3" } ), context );
  QCOMPARE( datasetGroupWidget->value().toList(), QVariantList( { 1, 2, 3 } ) );
  groupsWrapper.setWidgetValue( QgsProperty::fromExpression( u"1+3"_s ), context );
  QCOMPARE( datasetGroupWidget->value().toList(), QVariantList() << 4 );

  timeWrapper.setWidgetValue( QDateTime( QDate( 2020, 01, 02 ), QTime( 1, 2, 3 ) ), context );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'defined-date-time','value': QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3))}"_s );
  timeWrapper.setWidgetValue( QVariant::fromValue( QDateTime( QDate( 2020, 02, 01 ), QTime( 3, 2, 1 ) ) ).toString(), context );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, u"{'type': 'defined-date-time','value': QDateTime(QDate(2020, 2, 1), QTime(3, 2, 1))}"_s );

  // parameter definition widget
  std::unique_ptr<QgsProcessingAbstractParameterDefinitionWidget> paramWidgetGroup( groupsWrapper.createParameterDefinitionWidget( context, widgetContext, &groupsDefinition, nullptr ) );
  Qgis::ProcessingParameterFlags flags;
  std::unique_ptr<QgsProcessingParameterDefinition> paramDefGroup( paramWidgetGroup->createParameter( u"my_param_name"_s, u"my_param_descr"_s, flags ) );
  QVERIFY( paramDefGroup );
  QCOMPARE( paramDefGroup->name(), u"my_param_name"_s );
  QCOMPARE( paramDefGroup->description(), u"my_param_descr"_s );
  QCOMPARE( paramDefGroup->type(), QgsProcessingParameterMeshDatasetGroups::typeName() );
  QCOMPARE( static_cast<QgsProcessingParameterMeshDatasetGroups *>( paramDefGroup.get() )->meshLayerParameterName(), u"layer"_s );

  std::unique_ptr<QgsProcessingAbstractParameterDefinitionWidget> paramWidgetTime( timeWrapper.createParameterDefinitionWidget( context, widgetContext, &timeDefinition, nullptr ) );
  std::unique_ptr<QgsProcessingParameterDefinition> paramDefTime( paramWidgetTime->createParameter( u"my_param_name"_s, u"my_param_descr"_s, flags ) );
  QVERIFY( paramDefTime );
  QCOMPARE( paramDefTime->name(), u"my_param_name"_s );
  QCOMPARE( paramDefTime->description(), u"my_param_descr"_s );
  QCOMPARE( paramDefTime->type(), QgsProcessingParameterMeshDatasetTime::typeName() );
  QCOMPARE( static_cast<QgsProcessingParameterMeshDatasetTime *>( paramDefTime.get() )->meshLayerParameterName(), u"layer"_s );
  QCOMPARE( static_cast<QgsProcessingParameterMeshDatasetTime *>( paramDefTime.get() )->datasetGroupParameterName(), u"groups"_s );
}

void TestProcessingGui::testMeshDatasetWrapperLayerOutsideProject()
{
  QgsProcessingParameterMeshLayer layerDefinition( u"layer"_s, u"layer"_s );
  QgsProcessingMeshLayerWidgetWrapper layerWrapper( &layerDefinition );

  QSet<int> supportedDataType( { QgsMeshDatasetGroupMetadata::DataOnFaces } );
  QgsProcessingParameterMeshDatasetGroups groupsDefinition( u"groups"_s, u"groups"_s, u"layer"_s, supportedDataType );
  QgsProcessingMeshDatasetGroupsWidgetWrapper groupsWrapper( &groupsDefinition );

  QgsProcessingParameterMeshDatasetTime timeDefinition( u"time"_s, u"time"_s, u"layer"_s, u"groups"_s );
  QgsProcessingMeshDatasetTimeWidgetWrapper timeWrapper( &timeDefinition );

  QList<QgsAbstractProcessingParameterWidgetWrapper *> wrappers;
  wrappers << &layerWrapper << &groupsWrapper << &timeWrapper;

  QgsProject project;
  QgsProcessingContext context;
  context.setProject( &project );
  QgsProcessingParameterWidgetContext widgetContext;
  auto mapCanvas = std::make_unique<QgsMapCanvas>();
  widgetContext.setMapCanvas( mapCanvas.get() );

  widgetContext.setProject( &project );
  layerWrapper.setWidgetContext( widgetContext );
  groupsWrapper.setWidgetContext( widgetContext );
  timeWrapper.setWidgetContext( widgetContext );

  TestProcessingContextGenerator generator( context );
  layerWrapper.registerProcessingContextGenerator( &generator );
  groupsWrapper.registerProcessingContextGenerator( &generator );
  timeWrapper.registerProcessingContextGenerator( &generator );

  QSignalSpy layerSpy( &layerWrapper, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
  QSignalSpy groupsSpy( &groupsWrapper, &QgsProcessingMeshDatasetGroupsWidgetWrapper::widgetValueHasChanged );
  QSignalSpy timeSpy( &timeWrapper, &QgsProcessingMeshDatasetTimeWidgetWrapper::widgetValueHasChanged );

  std::unique_ptr<QWidget> layerWidget( layerWrapper.createWrappedWidget( context ) );
  std::unique_ptr<QWidget> groupWidget( groupsWrapper.createWrappedWidget( context ) );
  std::unique_ptr<QWidget> timeWidget( timeWrapper.createWrappedWidget( context ) );
  QgsProcessingMeshDatasetGroupsWidget *datasetGroupWidget = qobject_cast<QgsProcessingMeshDatasetGroupsWidget *>( groupWidget.get() );
  QgsProcessingMeshDatasetTimeWidget *datasetTimeWidget = qobject_cast<QgsProcessingMeshDatasetTimeWidget *>( timeWidget.get() );

  QVERIFY( layerWidget );
  QVERIFY( groupWidget );
  QVERIFY( datasetGroupWidget );
  QVERIFY( timeWidget );

  groupsWrapper.postInitialize( wrappers );
  timeWrapper.postInitialize( wrappers );

  layerSpy.clear();
  groupsSpy.clear();
  timeSpy.clear();

  QString dataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString meshOutOfProject( dataDir + "/mesh/trap_steady_05_3D.nc" );
  layerWrapper.setWidgetValue( meshOutOfProject, context );

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 1 );
  QCOMPARE( timeSpy.count(), 1 );

  QVariantList groups;
  groups << 0;
  groupsWrapper.setWidgetValue( groups, context );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 2 );
  QCOMPARE( timeSpy.count(), 3 );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsWrapper.widgetValue() ), QList<int>() << 0 );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"static"_s );
  QVERIFY( !datasetTimeWidget->isEnabled() );

  groups << 11;
  groupsWrapper.setWidgetValue( groups, context );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 5 );
  QVERIFY( datasetTimeWidget->isEnabled() );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsWrapper.widgetValue() ), QList<int>() << 0 << 11 );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"dataset-time-step"_s );
  QVERIFY( QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( timeWrapper.widgetValue() ) == QgsMeshDatasetIndex( 11, 0 ) );

  QVERIFY( datasetTimeWidget->radioButtonDefinedDateTime->isEnabled() );
  QVERIFY( !datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );

  datasetTimeWidget->radioButtonDefinedDateTime->setChecked( true );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), u"defined-date-time"_s );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( timeWrapper.widgetValue() ), QDateTime( QDate( 1990, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );


  mapCanvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2021, 1, 1 ), QTime( 0, 3, 0 ), Qt::UTC ), QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 5, 0 ), Qt::UTC ) ) );
  QVERIFY( datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );
}

void TestProcessingGui::testPointCloudLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsPointCloudLayer *cloud1 = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", u"cloud1"_s, u"ept"_s );
  QVERIFY( cloud1->isValid() );
  QgsProject::instance()->addMapLayer( cloud1 );
  QgsPointCloudLayer *cloud2 = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", u"cloud2"_s, u"ept"_s );
  QVERIFY( cloud2->isValid() );
  QgsProject::instance()->addMapLayer( cloud2 );

  auto testWrapper = [cloud1, cloud2]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterPointCloudLayer param( u"cloud"_s, u"cloud"_s, false );

    QgsProcessingPointCloudLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingPointCloudLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"bb"_s, context );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), u"bb"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"bb"_s );
        wrapper.setWidgetValue( u"aa"_s, context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), u"aa"_s );
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper.wrappedWidget() )->currentText(), u"aa"_s );
        break;
    }

    delete w;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingMapLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingPointCloudLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper2.setWidgetValue( u"cloud2"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), cloud2->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"cloud2 [EPSG:28356]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"cloud2"_s );
        break;
    }

    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"cloud2"_s );

    // check signal
    static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( cloud1 );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), cloud1->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"cloud1 [EPSG:28356]"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"cloud1"_s );
        break;
    }
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"cloud1"_s );

    delete w;

    // optional
    QgsProcessingParameterPoint param2( u"cloud"_s, u"cloud"_s, QVariant(), true );
    QgsProcessingPointCloudLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingPointCloudLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"bb"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"bb"_s );
    QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"bb"_s );
    wrapper3.setWidgetValue( u"cloud2"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), cloud2->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"cloud2 [EPSG:28356]"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QgsProcessingMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"cloud2"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"cloud"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testAnnotationLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsAnnotationLayer *layer1 = new QgsAnnotationLayer( u"secondary annotations"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer1->isValid() );
  QgsProject::instance()->addMapLayer( layer1 );

  auto testWrapper = [layer1]( Qgis::ProcessingMode type ) {
    // non optional
    QgsProcessingParameterAnnotationLayer param( u"annotation"_s, u"annotation"_s, false );

    QgsProcessingAnnotationLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    // with project
    QgsProcessingParameterWidgetContext widgetContext;
    widgetContext.setProject( QgsProject::instance() );
    context.setProject( QgsProject::instance() );

    QgsProcessingAnnotationLayerWidgetWrapper wrapper2( &param, type );
    wrapper2.setWidgetContext( widgetContext );
    QWidget *w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingAnnotationLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"main"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"main"_s );
    QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"Annotations"_s );
    wrapper2.setWidgetValue( u"secondary annotations"_s, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), layer1->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"secondary annotations"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"secondary annotations"_s );
        break;
    }

    QCOMPARE( static_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"secondary annotations"_s );

    // check signal
    static_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->setLayer( QgsProject::instance()->mainAnnotationLayer() );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"main"_s );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"Annotations"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentText(), u"Annotations"_s );
        break;
    }
    QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper2.wrappedWidget() )->currentLayer()->name(), u"Annotations"_s );

    delete w;

    // optional
    QgsProcessingParameterAnnotationLayer param2( u"annotation"_s, u"annotation"_s, QVariant(), true );
    QgsProcessingAnnotationLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingAnnotationLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"main"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), u"main"_s );
    QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"Annotations"_s );
    wrapper3.setWidgetValue( u"secondary annotations"_s, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), layer1->id() );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"secondary annotations"_s );
        break;
      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( qgis::down_cast<QgsMapLayerComboBox *>( wrapper3.wrappedWidget() )->currentText(), u"secondary annotations"_s );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"annotation"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );
}

void TestProcessingGui::testPointCloudAttributeWrapper()
{
  const QgsProcessingParameterDefinition *layerDef = new QgsProcessingParameterPointCloudLayer( "INPUT", u"input"_s, QVariant(), false );

  auto testWrapper = [layerDef]( Qgis::ProcessingMode type ) {
    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsPointCloudLayer *pcl = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/copc/rgb.copc.laz", u"x"_s, u"copc"_s );
    p.addMapLayer( pcl );

    QgsProcessingParameterPointCloudAttribute param( u"attribute"_s, u"attribute"_s, QVariant(), u"INPUT"_s );

    QgsProcessingPointCloudAttributeWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void ) w;
    layerWrapper.setWidgetValue( QVariant::fromValue( pcl ), context );
    wrapper.setParentLayerWrapperValue( &layerWrapper );

    QSignalSpy spy( &wrapper, &QgsProcessingPointCloudAttributeWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( u"Red"_s, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), u"Red"_s );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsPointCloudAttributeComboBox *>( wrapper.wrappedWidget() )->currentAttribute(), u"Red"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper.wrappedWidget() )->text(), u"Red"_s );
        break;
    }

    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );

    delete w;

    // optional
    param = QgsProcessingParameterPointCloudAttribute( u"attribute"_s, u"attribute"_s, QVariant(), u"INPUT"_s, false, true );

    QgsProcessingPointCloudAttributeWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( pcl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy2( &wrapper2, &QgsProcessingPointCloudAttributeWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( u"Intensity"_s, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), u"Intensity"_s );

    wrapper2.setWidgetValue( QString(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( wrapper2.widgetValue().toString().isEmpty() );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( static_cast<QgsPointCloudAttributeComboBox *>( wrapper2.wrappedWidget() )->currentAttribute(), QString() );
        break;

      case Qgis::ProcessingMode::Modeler:
        QCOMPARE( static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->text(), QString() );
        break;
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != Qgis::ProcessingMode::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), u"attribute [optional]"_s );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        static_cast<QgsPointCloudAttributeComboBox *>( wrapper2.wrappedWidget() )->setAttribute( u"Red"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        static_cast<QLineEdit *>( wrapper2.wrappedWidget() )->setText( u"Red"_s );
        break;
    }

    QCOMPARE( spy2.count(), 3 );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), pcl );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    layerWrapper.setWidgetValue( pcl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    layerWrapper.setWidgetValue( pcl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), pcl );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointCloudFileName = TEST_DATA_DIR + u"/point_clouds/copc/sunshine-coast.copc.laz"_s;
    layerWrapper.setWidgetValue( pointCloudFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper2.mComboBox->layer()->publicSource(), pointCloudFileName );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }

    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointCloudFileName );

    delete w;

    // multiple
    param = QgsProcessingParameterPointCloudAttribute( u"attribute"_s, u"attribute"_s, QVariant(), u"INPUT"_s, true, true );

    QgsProcessingPointCloudAttributeWidgetWrapper wrapper3( &param, type );

    w = wrapper3.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( pcl ), context );
    wrapper3.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy3( &wrapper3, &QgsProcessingPointCloudAttributeWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( u"Intensity"_s, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << u"Intensity"_s );

    wrapper3.setWidgetValue( QString(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( wrapper3.widgetValue().toString().isEmpty() );

    wrapper3.setWidgetValue( u"Intensity;Red"_s, context );
    QCOMPARE( spy3.count(), 3 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << u"Intensity"_s << u"Red"_s );

    delete w;

    // default to all fields
    param = QgsProcessingParameterPointCloudAttribute( u"attribute"_s, u"attribute"_s, QVariant(), u"INPUT"_s, true, true );
    param.setDefaultToAllAttributes( true );
    QgsProcessingPointCloudAttributeWidgetWrapper wrapper4( &param, type );
    w = wrapper4.createWrappedWidget( context );
    wrapper4.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case Qgis::ProcessingMode::Standard:
      case Qgis::ProcessingMode::Batch:
        QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << u"X"_s << u"Y"_s << u"Z"_s << u"Intensity"_s << u"ReturnNumber"_s << u"NumberOfReturns"_s << u"ScanDirectionFlag"_s << u"EdgeOfFlightLine"_s << u"Classification"_s << u"ScanAngleRank"_s << u"UserData"_s << u"PointSourceId"_s << u"Synthetic"_s << u"KeyPoint"_s << u"Withheld"_s << u"Overlap"_s << u"ScannerChannel"_s << u"GpsTime"_s << u"Red"_s << u"Green"_s << u"Blue"_s );
        break;

      case Qgis::ProcessingMode::Modeler:
        break;
    }
    delete w;
  };

  // standard wrapper
  testWrapper( Qgis::ProcessingMode::Standard );

  // batch wrapper
  testWrapper( Qgis::ProcessingMode::Batch );

  // modeler wrapper
  testWrapper( Qgis::ProcessingMode::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  auto widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"attribute"_s, context, widgetContext );
  std::unique_ptr<QgsProcessingParameterDefinition> def( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QVERIFY( !def->defaultValue().isValid() );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterPointCloudAttribute attrParam( u"n"_s, u"test desc"_s, u"attribute_name"_s, u"parent"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"attribute"_s, context, widgetContext, &attrParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QVERIFY( !( def->flags() & Qgis::ProcessingParameterFlag::Advanced ) );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->defaultValue().toString(), u"attribute_name"_s );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->parentLayerParameterName(), u"parent"_s );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->allowMultiple(), false );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->defaultToAllAttributes(), false );
  attrParam.setFlags( Qgis::ProcessingParameterFlag::Advanced | Qgis::ProcessingParameterFlag::Optional );
  attrParam.setParentLayerParameterName( QString() );
  attrParam.setAllowMultiple( true );
  attrParam.setDefaultToAllAttributes( true );
  attrParam.setDefaultValue( u"Intensity;Red"_s );
  widget = std::make_unique<QgsProcessingParameterDefinitionWidget>( u"attribute"_s, context, widgetContext, &attrParam );
  def.reset( widget->createParameter( u"param_name"_s ) );
  QCOMPARE( def->name(), u"param_name"_s );
  QCOMPARE( def->description(), u"test desc"_s );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Optional );
  QVERIFY( def->flags() & Qgis::ProcessingParameterFlag::Advanced );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->defaultValue().toString(), u"Intensity;Red"_s );
  QVERIFY( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->parentLayerParameterName().isEmpty() );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->allowMultiple(), true );
  QCOMPARE( static_cast<QgsProcessingParameterPointCloudAttribute *>( def.get() )->defaultToAllAttributes(), true );
}

void TestProcessingGui::testModelGraphicsView()
{
  // test model
  QgsProcessingModelAlgorithm model1;

  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "buffer" );
  algc1.setAlgorithmId( "native:buffer" );
  QgsProcessingModelParameter param;
  param.setParameterName( u"LAYER"_s );
  param.setSize( QSizeF( 500, 400 ) );
  param.setPosition( QPointF( 101, 102 ) );
  param.comment()->setDescription( u"input comment"_s );
  model1.addModelParameter( new QgsProcessingParameterMapLayer( u"LAYER"_s ), param );
  algc1.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromModelParameter( u"LAYER"_s ) );
  algc1.comment()->setDescription( u"alg comment"_s );
  algc1.comment()->setSize( QSizeF( 300, 200 ) );
  algc1.comment()->setPosition( QPointF( 201, 202 ) );

  QgsProcessingModelOutput modelOut;
  modelOut.setChildId( algc1.childId() );
  modelOut.setChildOutputName( u"my_output"_s );
  modelOut.comment()->setDescription( u"output comm"_s );
  QMap<QString, QgsProcessingModelOutput> outs;
  outs.insert( u"OUTPUT"_s, modelOut );
  algc1.setModelOutputs( outs );
  model1.addChildAlgorithm( algc1 );

  QgsProcessingModelGroupBox groupBox;
  groupBox.setDescription( u"group"_s );
  model1.addGroupBox( groupBox );

  // hiding comments
  QgsProcessingContext context;
  QgsModelGraphicsScene scene2;
  scene2.setModel( &model1 );
  scene2.setFlags( QgsModelGraphicsScene::FlagHideComments );
  scene2.createItems( &model1, context );
  QList<QGraphicsItem *> items = scene2.items();
  QgsModelParameterGraphicItem *layerItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelParameterGraphicItem *param = dynamic_cast<QgsModelParameterGraphicItem *>( item ) )
    {
      layerItem = param;
      break;
    }
  }
  QVERIFY( layerItem );
  QgsModelCommentGraphicItem *layerCommentItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelCommentGraphicItem *comment = dynamic_cast<QgsModelCommentGraphicItem *>( item ) )
    {
      layerCommentItem = comment;
      break;
    }
  }
  // should not exist
  QVERIFY( !layerCommentItem );

  //check model bounds
  scene2.updateBounds();
  QRectF modelRect = scene2.sceneRect();
  QGSCOMPARENEAR( modelRect.height(), 624.4, 5 ); // Slightly higher threeshold because of various font size can marginally change the bounding rect
  QGSCOMPARENEAR( modelRect.width(), 655.00, 0.01 );
  QGSCOMPARENEAR( modelRect.left(), -252.0, 0.01 );
  QGSCOMPARENEAR( modelRect.top(), -232.0, 0.01 );


  // test model large modelRect
  QgsProcessingModelAlgorithm model2;

  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( "buffer" );
  algc2.setAlgorithmId( "native:buffer" );
  algc2.setPosition( QPointF( 4250, 4250 ) );
  QgsProcessingModelParameter param1;
  param1.setParameterName( u"LAYER"_s );
  param1.setSize( QSizeF( 500, 400 ) );
  param1.setPosition( QPointF( -250, -250 ) );
  model2.addModelParameter( new QgsProcessingParameterMapLayer( u"LAYER"_s ), param );
  algc2.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromModelParameter( u"LAYER"_s ) );

  model2.addChildAlgorithm( algc2 );

  QgsModelGraphicsScene scene3;
  scene3.setModel( &model2 );
  scene3.createItems( &model2, context );

  scene3.updateBounds();
  QRectF modelRect2 = scene3.sceneRect();
  QGSCOMPARENEAR( modelRect2.height(), 4505.4, 5 ); // Slightly higher threeshold because of various font size can marginally change the bounding rect
  QGSCOMPARENEAR( modelRect2.width(), 4603.0, 0.01 );
  QGSCOMPARENEAR( modelRect2.left(), -201.0, 0.01 );
  QGSCOMPARENEAR( modelRect2.top(), -150.0, 0.01 );

  QgsModelGraphicsScene scene;
  QVERIFY( !scene.model() );
  scene.setModel( &model1 );
  QCOMPARE( scene.model(), &model1 );

  QVERIFY( scene.items().empty() );
  scene.createItems( &model1, context );
  items = scene.items();
  layerItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelParameterGraphicItem *param = dynamic_cast<QgsModelParameterGraphicItem *>( item ) )
    {
      layerItem = param;
    }
  }
  QVERIFY( layerItem );
  QCOMPARE( dynamic_cast<QgsProcessingModelParameter *>( layerItem->component() )->parameterName(), u"LAYER"_s );
  QCOMPARE( layerItem->itemRect().size(), QSizeF( 500, 400 ) );
  QCOMPARE( layerItem->scenePos(), QPointF( 101, 102 ) );

  QgsModelChildAlgorithmGraphicItem *algItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelChildAlgorithmGraphicItem *param = dynamic_cast<QgsModelChildAlgorithmGraphicItem *>( item ) )
    {
      algItem = param;
      break;
    }
  }
  QVERIFY( algItem );
  QCOMPARE( dynamic_cast<QgsProcessingModelChildAlgorithm *>( algItem->component() )->algorithmId(), u"native:buffer"_s );

  QgsModelOutputGraphicItem *outputItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelOutputGraphicItem *comment = dynamic_cast<QgsModelOutputGraphicItem *>( item ) )
    {
      outputItem = comment;
      break;
    }
  }
  QVERIFY( outputItem );
  QCOMPARE( dynamic_cast<QgsProcessingModelOutput *>( outputItem->component() )->childOutputName(), u"my_output"_s );


  layerCommentItem = nullptr;
  QgsModelCommentGraphicItem *algCommentItem = nullptr;
  QgsModelCommentGraphicItem *outputCommentItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelCommentGraphicItem *comment = dynamic_cast<QgsModelCommentGraphicItem *>( item ) )
    {
      if ( comment->parentComponentItem() == layerItem )
      {
        layerCommentItem = comment;
      }
      else if ( comment->parentComponentItem() == algItem )
      {
        algCommentItem = comment;
      }
      else if ( comment->parentComponentItem() == outputItem )
      {
        outputCommentItem = comment;
      }
    }
  }

  QVERIFY( algCommentItem );
  QCOMPARE( algCommentItem->component()->description(), u"alg comment"_s );
  QCOMPARE( algCommentItem->itemRect().size(), QSizeF( 300, 200 ) );
  QCOMPARE( algCommentItem->scenePos(), QPointF( 201, 202 ) );

  QVERIFY( layerCommentItem );
  QCOMPARE( layerCommentItem->component()->description(), u"input comment"_s );

  QVERIFY( outputCommentItem );
  QCOMPARE( outputCommentItem->component()->description(), u"output comm"_s );

  QgsModelGroupBoxGraphicItem *groupItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelGroupBoxGraphicItem *comment = dynamic_cast<QgsModelGroupBoxGraphicItem *>( item ) )
    {
      groupItem = comment;
      break;
    }
  }
  QVERIFY( groupItem );
  QCOMPARE( dynamic_cast<QgsProcessingModelGroupBox *>( groupItem->component() )->description(), u"group"_s );


  QgsModelGraphicsView view;
  view.setModelScene( &scene );

  // copy some items
  view.copyItems( QList<QgsModelComponentGraphicItem *>() << layerItem << algItem << groupItem, QgsModelGraphicsView::ClipboardCopy );


  // second view to paste into
  QgsProcessingModelAlgorithm algDest;
  QVERIFY( algDest.childAlgorithms().empty() );
  QVERIFY( algDest.parameterComponents().empty() );
  QVERIFY( algDest.groupBoxes().empty() );
  QgsModelGraphicsScene sceneDest;
  sceneDest.setModel( &algDest );
  QgsModelGraphicsView viewDest;
  viewDest.setModelScene( &sceneDest );
  viewDest.pasteItems( QgsModelGraphicsView::PasteModeInPlace );

  QCOMPARE( algDest.parameterComponents().size(), 1 );
  QCOMPARE( algDest.parameterComponents().value( u"LAYER"_s ).parameterName(), u"LAYER"_s );
  // comment should not be copied, was not selected
  QCOMPARE( algDest.parameterComponents().value( u"LAYER"_s ).comment()->description(), QString() );
  QCOMPARE( algDest.childAlgorithms().size(), 1 );
  // the child algorithm is already unique, so should not be changed
  QCOMPARE( algDest.childAlgorithms().keys().at( 0 ), u"buffer"_s );
  QCOMPARE( algDest.childAlgorithms().value( u"buffer"_s ).algorithmId(), u"native:buffer"_s );
  QCOMPARE( algDest.childAlgorithms().value( u"buffer"_s ).comment()->description(), QString() );
  // output was not selected
  QVERIFY( algDest.childAlgorithms().value( u"buffer"_s ).modelOutputs().empty() );
  QCOMPARE( algDest.groupBoxes().size(), 1 );
  QCOMPARE( algDest.groupBoxes().at( 0 ).description(), u"group"_s );

  // copy comments and output (not output comment though!)
  view.copyItems( QList<QgsModelComponentGraphicItem *>() << layerItem << layerCommentItem << algItem << algCommentItem << outputItem << groupItem, QgsModelGraphicsView::ClipboardCopy );
  viewDest.pasteItems( QgsModelGraphicsView::PasteModeInPlace );

  QCOMPARE( algDest.parameterComponents().size(), 2 );
  QCOMPARE( algDest.parameterComponents().value( u"LAYER"_s ).parameterName(), u"LAYER"_s );
  QCOMPARE( algDest.parameterComponents().value( u"LAYER (2)"_s ).parameterName(), u"LAYER (2)"_s );
  QCOMPARE( algDest.parameterComponents().value( u"LAYER"_s ).comment()->description(), QString() );
  QCOMPARE( algDest.parameterComponents().value( u"LAYER (2)"_s ).comment()->description(), u"input comment"_s );
  QCOMPARE( algDest.childAlgorithms().size(), 2 );
  QCOMPARE( algDest.childAlgorithms().value( u"buffer"_s ).algorithmId(), u"native:buffer"_s );
  QCOMPARE( algDest.childAlgorithms().value( u"buffer"_s ).comment()->description(), QString() );
  QCOMPARE( algDest.childAlgorithms().value( u"native:buffer_1"_s ).algorithmId(), u"native:buffer"_s );
  QCOMPARE( algDest.childAlgorithms().value( u"native:buffer_1"_s ).comment()->description(), u"alg comment"_s );
  QVERIFY( algDest.childAlgorithms().value( u"buffer"_s ).modelOutputs().empty() );
  QCOMPARE( algDest.childAlgorithms().value( u"native:buffer_1"_s ).modelOutputs().size(), 1 );
  // output comment wasn't selected
  QCOMPARE( algDest.childAlgorithms().value( u"native:buffer_1"_s ).modelOutputs().value( algDest.childAlgorithms().value( u"native:buffer_1"_s ).modelOutputs().keys().at( 0 ) ).comment()->description(), QString() );
  QCOMPARE( algDest.groupBoxes().size(), 2 );
  QCOMPARE( algDest.groupBoxes().at( 0 ).description(), u"group"_s );
  QCOMPARE( algDest.groupBoxes().at( 1 ).description(), u"group"_s );

  // output and output comment
  view.copyItems( QList<QgsModelComponentGraphicItem *>() << algItem << outputItem << outputCommentItem, QgsModelGraphicsView::ClipboardCopy );
  viewDest.pasteItems( QgsModelGraphicsView::PasteModeInPlace );
  QCOMPARE( algDest.childAlgorithms().size(), 3 );
  QCOMPARE( algDest.childAlgorithms().value( u"native:buffer_2"_s ).modelOutputs().size(), 1 );
  QCOMPARE( algDest.childAlgorithms().value( u"native:buffer_2"_s ).modelOutputs().value( algDest.childAlgorithms().value( u"native:buffer_2"_s ).modelOutputs().keys().at( 0 ) ).comment()->description(), u"output comm"_s );
}

void TestProcessingGui::cleanupTempDir()
{
  QDir tmpDir = QDir( mTempDir );
  if ( tmpDir.exists() )
  {
    for ( const QString &tf : tmpDir.entryList( QDir::NoDotAndDotDot | QDir::Files ) )
    {
      QVERIFY2( tmpDir.remove( mTempDir + '/' + tf ), qPrintable( "Could not remove " + mTempDir + '/' + tf ) );
    }
    QVERIFY2( tmpDir.rmdir( mTempDir ), qPrintable( "Could not remove directory " + mTempDir ) );
  }
}

QGSTEST_MAIN( TestProcessingGui )
#include "testprocessinggui.moc"
