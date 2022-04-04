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

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QStandardItemModel>
#include <QSignalSpy>

#include "qgstest.h"
#include "qgsconfig.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingparameters.h"
#include "qgsmodelundocommand.h"
#include "qgsprocessingmaplayercombobox.h"
#include "qgsnativealgorithms.h"
#include "processing/models/qgsprocessingmodelalgorithm.h"
#include "processing/models/qgsprocessingmodelgroupbox.h"
#include "qgsxmlutils.h"
#include "qgspropertyoverridebutton.h"
#include "qgsprojectionselectionwidget.h"
#include "qgsdoublespinbox.h"
#include "qgsspinbox.h"
#include "qgsmapcanvas.h"
#include "qgsauthconfigselect.h"
#include "qgsauthmanager.h"
#include "qgsprocessingmatrixparameterdialog.h"
#include "models/qgsprocessingmodelalgorithm.h"
#include "qgsfilewidget.h"
#include "qgsexpressionlineedit.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsprintlayout.h"
#include "qgslayoutmanager.h"
#include "qgslayoutcombobox.h"
#include "qgslayoutitemcombobox.h"
#include "qgslayoutitemlabel.h"
#include "qgsscalewidget.h"
#include "mesh/qgsmeshlayer.h"
#include "mesh/qgsmeshdataprovider.h"
#include "qgscolorbutton.h"
#include "qgsprocessingparameterdefinitionwidget.h"
#include "qgscoordinateoperationwidget.h"
#include "qgsmessagebar.h"
#include "qgsfieldcombobox.h"
#include "qgsmapthemecollection.h"
#include "qgsdatetimeedit.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderconnectioncombobox.h"
#include "qgsdatabaseschemacombobox.h"
#include "qgsdatabasetablecombobox.h"
#include "qgsprocessingoutputdestinationwidget.h"
#include "qgssettings.h"
#include "qgsprocessingfeaturesourceoptionswidget.h"
#include "qgsextentwidget.h"
#include "qgsrasterbandcombobox.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsprocessingfieldmapwidgetwrapper.h"
#include "qgsprocessingparameterfieldmap.h"
#include "qgsprocessingaggregatewidgetwrapper.h"
#include "qgsprocessingparameteraggregate.h"
#include "qgsprocessingparametertininputlayers.h"
#include "qgsprocessingtininputlayerswidget.h"
#include "qgsprocessingparameterdxflayers.h"
#include "qgsprocessingdxflayerswidgetwrapper.h"
#include "qgsprocessingmeshdatasetwidget.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgspluginlayer.h"
#include "qgspointcloudlayer.h"
#include "qgsannotationlayer.h"


class TestParamType : public QgsProcessingParameterDefinition
{
  public:

    TestParamType( const QString &type, const QString &name, const QVariant &defaultValue = QVariant() )
      : QgsProcessingParameterDefinition( name, name, defaultValue )
      , mType( type )
    {}

    QString mType;

    QgsProcessingParameterDefinition *clone() const override
    {
      return new TestParamType( mType, name() );
    }

    QString type() const override { return mType; }
    QString valueAsPythonString( const QVariant &, QgsProcessingContext & ) const override { return QString(); }
    QString asScriptCode() const override { return QString(); }

};

class TestWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper // clazy:exclude=missing-qobject-macro
{
  public:

    TestWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                       QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard )
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

    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter,
        QgsProcessingGui::WidgetType type ) override
    {
      return new TestWidgetWrapper( parameter, type );
    }


    QStringList compatibleParameterTypes() const override { return QStringList(); }

    QStringList compatibleOutputTypes() const override { return QStringList(); }

};

class DummyPluginLayer: public QgsPluginLayer
{
  public:

    DummyPluginLayer( const QString &layerType, const QString &layerName ): QgsPluginLayer( layerType, layerName )
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
    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override
    {
      Q_UNUSED( node );
      Q_UNUSED( errorMessage );
      Q_UNUSED( context );
      Q_UNUSED( categories );
      return true;
    };
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                         StyleCategories categories = AllStyleCategories ) const override
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
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
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
  QVERIFY2( !QgsApplication::authManager()->isDisabled(),
            "Authentication system is DISABLED" );

  // verify QGIS_AUTH_DB_DIR_PATH (temp auth db path) worked
  QString db1( QFileInfo( QgsApplication::authManager()->authenticationDatabasePath() ).canonicalFilePath() );
  QString db2( QFileInfo( mTempDir + "/qgis-auth.db" ).canonicalFilePath() );
  QVERIFY2( db1 == db2, "Auth db temp path does not match db path of manager" );

  // verify master pass can be set manually
  // (this also creates a fresh password hash in the new temp database)
  QVERIFY2( QgsApplication::authManager()->setMasterPassword( mPass, true ),
            "Master password could not be set" );
  QVERIFY2( QgsApplication::authManager()->masterPasswordIsSet(),
            "Auth master password not set from passed string" );

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
  QVERIFY2( !QgsApplication::authManager()->isDisabled(),
            "Authentication system is DISABLED" );

  // verify QGIS_AUTH_PASSWORD_FILE worked, when compared against hash in db
  QVERIFY2( QgsApplication::authManager()->masterPasswordIsSet(),
            "Auth master password not set from QGIS_AUTH_PASSWORD_FILE" );

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

}

void TestProcessingGui::testModelUndo()
{
  // make a little model
  QgsProcessingModelAlgorithm model( QStringLiteral( "test" ), QStringLiteral( "testGroup" ) );
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1( "native:buffer" );
  a1.setDescription( QStringLiteral( "alg1" ) );
  a1.setChildId( QStringLiteral( "alg1" ) );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( QStringLiteral( "alg2" ) );
  a2.setChildId( QStringLiteral( "alg2" ) );
  QgsProcessingModelChildAlgorithm a3( QStringLiteral( "native:buffer" ) );
  a3.setDescription( QStringLiteral( "alg3" ) );
  a3.setChildId( QStringLiteral( "alg3" ) );
  algs.insert( QStringLiteral( "alg1" ), a1 );
  algs.insert( QStringLiteral( "alg2" ), a2 );
  algs.insert( QStringLiteral( "alg3" ), a3 );
  model.setChildAlgorithms( algs );

  QgsModelUndoCommand command( &model, QStringLiteral( "undo" ) );
  model.childAlgorithm( QStringLiteral( "alg1" ) ).setDescription( QStringLiteral( "new desc" ) );
  command.saveAfterState();

  QCOMPARE( model.childAlgorithm( QStringLiteral( "alg1" ) ).description(), QStringLiteral( "new desc" ) );
  command.undo();
  QCOMPARE( model.childAlgorithm( QStringLiteral( "alg1" ) ).description(), QStringLiteral( "alg1" ) );

  // first redo should have no effect -- we ignore it, since it's automatically triggered when the
  // command is added to the stack (yet we apply the initial change to the models outside of the undo stack)
  command.redo();
  QCOMPARE( model.childAlgorithm( QStringLiteral( "alg1" ) ).description(), QStringLiteral( "alg1" ) );
  command.redo();
  QCOMPARE( model.childAlgorithm( QStringLiteral( "alg1" ) ).description(), QStringLiteral( "new desc" ) );

  // the last used parameter values setting should not be affected by undo stack changes
  QVariantMap params;
  params.insert( QStringLiteral( "a" ), 1 );
  model.setDesignerParameterValues( params );
  command.undo();
  QCOMPARE( model.designerParameterValues(), params );
  command.redo();
  QCOMPARE( model.designerParameterValues(), params );
}

void TestProcessingGui::testSetGetConfig()
{
  const QList< const QgsProcessingAlgorithm * > algorithms = QgsApplication::processingRegistry()->algorithms();

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
  const QgsProcessingAlgorithm *algorithm = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:filter" ) );
  std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );

  QVariantMap config;
  QVariantList outputs;
  QVariantMap output;
  output.insert( QStringLiteral( "name" ), QStringLiteral( "test" ) );
  output.insert( QStringLiteral( "expression" ), QStringLiteral( "I am an expression" ) );
  output.insert( QStringLiteral( "isModelOutput" ), true );
  outputs.append( output );
  config.insert( QStringLiteral( "outputs" ), outputs );
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
  QgsProcessingGuiRegistry registry;

  TestParamType numParam( QStringLiteral( "num" ), QStringLiteral( "num" ) );
  TestParamType stringParam( QStringLiteral( "str" ), QStringLiteral( "str" ) );

  QVERIFY( !registry.createParameterWidgetWrapper( &numParam, QgsProcessingGui::Standard ) );

  TestWidgetFactory *factory = new TestWidgetFactory( QStringLiteral( "str" ) );
  QVERIFY( registry.addParameterWidgetFactory( factory ) );

  // duplicate type not allowed
  TestWidgetFactory *factory2 = new TestWidgetFactory( QStringLiteral( "str" ) );
  QVERIFY( !registry.addParameterWidgetFactory( factory2 ) );
  delete factory2;

  QgsAbstractProcessingParameterWidgetWrapper *wrapper = registry.createParameterWidgetWrapper( &numParam, QgsProcessingGui::Standard );
  QVERIFY( !wrapper );
  wrapper = registry.createParameterWidgetWrapper( &stringParam, QgsProcessingGui::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), QStringLiteral( "str" ) );
  delete wrapper;

  TestWidgetFactory *factory3 = new TestWidgetFactory( QStringLiteral( "num" ) );
  QVERIFY( registry.addParameterWidgetFactory( factory3 ) );

  wrapper = registry.createParameterWidgetWrapper( &numParam, QgsProcessingGui::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), QStringLiteral( "num" ) );
  delete wrapper;

  // removing
  registry.removeParameterWidgetFactory( nullptr );
  TestWidgetFactory *factory4 = new TestWidgetFactory( QStringLiteral( "xxxx" ) );
  registry.removeParameterWidgetFactory( factory4 );
  registry.removeParameterWidgetFactory( factory );
  wrapper = registry.createParameterWidgetWrapper( &stringParam, QgsProcessingGui::Standard );
  QVERIFY( !wrapper );

  wrapper = registry.createParameterWidgetWrapper( &numParam, QgsProcessingGui::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), QStringLiteral( "num" ) );
  delete wrapper;
}

void TestProcessingGui::testWrapperGeneral()
{
  TestParamType param( QStringLiteral( "boolean" ), QStringLiteral( "bool" ) );
  param.setAdditionalExpressionContextVariables( QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QgsProcessingBooleanWidgetWrapper wrapper( &param );
  QCOMPARE( wrapper.type(), QgsProcessingGui::Standard );

  QgsExpressionContext expContext = wrapper.createExpressionContext();
  QVERIFY( expContext.hasVariable( QStringLiteral( "a" ) ) );
  QVERIFY( expContext.hasVariable( QStringLiteral( "b" ) ) );
  QCOMPARE( expContext.highlightedVariables(), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );

  QgsProcessingBooleanWidgetWrapper wrapper2( &param, QgsProcessingGui::Batch );
  QCOMPARE( wrapper2.type(), QgsProcessingGui::Batch );
  QCOMPARE( wrapper2.parameterDefinition()->name(), QStringLiteral( "bool" ) );

  QgsProcessingBooleanWidgetWrapper wrapperModeler( &param, QgsProcessingGui::Modeler );
  QCOMPARE( wrapperModeler.type(), QgsProcessingGui::Modeler );

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
  param = TestParamType( QStringLiteral( "boolean" ), QStringLiteral( "bool" ), true );
  QgsProcessingBooleanWidgetWrapper trueDefault( &param );
  w = trueDefault.createWrappedWidget( context );
  QVERIFY( trueDefault.widgetValue().toBool() );
  delete w;
  param = TestParamType( QStringLiteral( "boolean" ), QStringLiteral( "bool" ), false );
  QgsProcessingBooleanWidgetWrapper falseDefault( &param );
  w = falseDefault.createWrappedWidget( context );
  QVERIFY( !falseDefault.widgetValue().toBool() );
  delete w;

  std::unique_ptr< QgsMapCanvas > mc = std::make_unique< QgsMapCanvas >();
  QgsProcessingParameterWidgetContext widgetContext;
  widgetContext.setMapCanvas( mc.get() );
  QCOMPARE( widgetContext.mapCanvas(), mc.get() );

  std::unique_ptr< QgsMessageBar > mb = std::make_unique< QgsMessageBar >();
  widgetContext.setMessageBar( mb.get() );
  QCOMPARE( widgetContext.messageBar(), mb.get() );

  QgsProject p;
  widgetContext.setProject( &p );
  QCOMPARE( widgetContext.project(), &p );
  std::unique_ptr< QgsProcessingModelAlgorithm > model = std::make_unique< QgsProcessingModelAlgorithm >();
  widgetContext.setModel( model.get() );
  QCOMPARE( widgetContext.model(), model.get() );
  widgetContext.setModelChildAlgorithmId( QStringLiteral( "xx" ) );
  QCOMPARE( widgetContext.modelChildAlgorithmId(), QStringLiteral( "xx" ) );

  wrapper.setWidgetContext( widgetContext );
  QCOMPARE( wrapper.widgetContext().mapCanvas(), mc.get() );
  QCOMPARE( wrapper.widgetContext().messageBar(), mb.get() );
  QCOMPARE( wrapper.widgetContext().model(), model.get() );
  QCOMPARE( wrapper.widgetContext().modelChildAlgorithmId(), QStringLiteral( "xx" ) );
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
    void setWidgetValue( const QVariant &val, QgsProcessingContext & ) override { v = val;}
    QVariant widgetValue() const override { return v; }

    QVariant v;
};

void TestProcessingGui::testWrapperDynamic()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:centroids" ) );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( QStringLiteral( "INPUT" ) );
  const QgsProcessingParameterDefinition *allPartsDef = centroidAlg->parameterDefinition( QStringLiteral( "ALL_PARTS" ) );

  QgsProcessingBooleanWidgetWrapper inputWrapper( layerDef, QgsProcessingGui::Standard );
  QgsProcessingBooleanWidgetWrapper allPartsWrapper( allPartsDef, QgsProcessingGui::Standard );

  QgsProcessingContext context;

  std::unique_ptr< QWidget > allPartsWidget( allPartsWrapper.createWrappedWidget( context ) );
  // dynamic parameter, so property button should be created
  QVERIFY( allPartsWrapper.mPropertyButton.data() != nullptr );

  std::unique_ptr< QWidget > inputWidget( inputWrapper.createWrappedWidget( context ) );
  // not dynamic parameter, so property button should be NOT created
  QVERIFY( inputWrapper.mPropertyButton.data() == nullptr );

  // set dynamic parameter to dynamic value
  allPartsWrapper.setParameterValue( QgsProperty::fromExpression( QStringLiteral( "1+2" ) ), context );
  QCOMPARE( allPartsWrapper.parameterValue().value< QgsProperty >().expressionString(), QStringLiteral( "1+2" ) );
  // not dynamic value
  allPartsWrapper.setParameterValue( true, context );
  QCOMPARE( allPartsWrapper.parameterValue().toBool(), true );
  allPartsWrapper.setParameterValue( false, context );
  QCOMPARE( allPartsWrapper.parameterValue().toBool(), false );

  // project layer
  QgsProject p;
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
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
  QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
  layerWrapper.setWidgetValue( pointFileName, context );
  allPartsWrapper.setDynamicParentLayerParameter( &layerWrapper );
  QCOMPARE( allPartsWrapper.mPropertyButton->vectorLayer()->publicSource(), pointFileName );
  // must be owned by wrapper, or layer may be deleted while still required by wrapper
  QCOMPARE( allPartsWrapper.mDynamicLayer->publicSource(), pointFileName );
}

void TestProcessingGui::testModelerWrapper()
{
  // make a little model
  QgsProcessingModelAlgorithm model( QStringLiteral( "test" ), QStringLiteral( "testGroup" ) );
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1( "native:buffer" );
  a1.setDescription( QStringLiteral( "alg1" ) );
  a1.setChildId( QStringLiteral( "alg1" ) );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( QStringLiteral( "alg2" ) );
  a2.setChildId( QStringLiteral( "alg2" ) );
  QgsProcessingModelChildAlgorithm a3( QStringLiteral( "native:buffer" ) );
  a3.setDescription( QStringLiteral( "alg3" ) );
  a3.setChildId( QStringLiteral( "alg3" ) );
  QgsProcessingModelChildAlgorithm a4( QStringLiteral( "native:package" ) );
  a4.setDescription( QStringLiteral( "alg4" ) );
  a4.setChildId( QStringLiteral( "alg4" ) );
  algs.insert( QStringLiteral( "alg1" ), a1 );
  algs.insert( QStringLiteral( "alg2" ), a2 );
  algs.insert( QStringLiteral( "alg3" ), a3 );
  algs.insert( QStringLiteral( "alg4" ), a4 );
  model.setChildAlgorithms( algs );

  QMap<QString, QgsProcessingModelParameter> pComponents;
  QgsProcessingModelParameter pc1;
  pc1.setParameterName( QStringLiteral( "my_param" ) );
  pComponents.insert( QStringLiteral( "my_param" ), pc1 );
  model.setParameterComponents( pComponents );

  QgsProcessingModelParameter bool1( "p1" );
  model.addModelParameter( new QgsProcessingParameterBoolean( "p1", "desc" ), bool1 );
  QgsProcessingModelParameter testParam( "p2" );
  model.addModelParameter( new TestParamType( "test_type", "p2" ), testParam );
  QgsProcessingModelParameter testDestParam( "p3" );
  model.addModelParameter( new QgsProcessingParameterFileDestination( "test_dest", "p3" ), testDestParam );
  QgsProcessingModelParameter testLayerParam( "p4" );
  model.addModelParameter( new QgsProcessingParameterMapLayer( "p4", "test_layer" ), testLayerParam );

  // try to create a parameter widget, no factories registered
  QgsProcessingGuiRegistry registry;
  QgsProcessingContext context;
  QVERIFY( !registry.createModelerParameterWidget( &model, QStringLiteral( "a" ), model.parameterDefinition( "p2" ), context ) );

  // register factory
  TestWidgetFactory *factory = new TestWidgetFactory( QStringLiteral( "test_type" ) );
  QVERIFY( registry.addParameterWidgetFactory( factory ) );
  QgsProcessingModelerParameterWidget *w = registry.createModelerParameterWidget( &model, QStringLiteral( "a" ), model.parameterDefinition( "p2" ), context );
  QVERIFY( w );
  delete w;

  w = registry.createModelerParameterWidget( &model, QStringLiteral( "a" ), model.parameterDefinition( "p1" ), context );
  QVERIFY( w );
  // should default to static value
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::StaticValue );
  delete w;

  w = registry.createModelerParameterWidget( &model, QStringLiteral( "a" ), model.parameterDefinition( "p4" ), context );
  QVERIFY( w );
  // a layer parameter should default to "model input" type
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ModelParameter );
  delete w;

  // widget tests
  w = new QgsProcessingModelerParameterWidget( &model, "alg1", model.parameterDefinition( "p1" ), context );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->parameterDefinition()->name(), QStringLiteral( "p1" ) );
  QLabel *l = w->createLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "desc" ) );
  QCOMPARE( l->toolTip(), w->parameterDefinition()->toolTip() );
  delete l;

  // static value
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( true ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().staticValue().toBool(), true );
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( false ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().staticValue().toBool(), false );
  QCOMPARE( w->mStackedWidget->currentIndex(), 0 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Value" ) );

  // expression value
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "1+2" ) ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().expression(), QStringLiteral( "1+2" ) );
  QCOMPARE( w->mStackedWidget->currentIndex(), 1 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Pre-calculated Value" ) );

  // model input - should fail, because we haven't populated sources yet, and so have no compatible sources
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QVERIFY( w->value().value< QgsProcessingModelChildParameterSource>().parameterName().isEmpty() );
  QCOMPARE( w->mStackedWidget->currentIndex(), 2 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Model Input" ) );

  // alg output  - should fail, because we haven't populated sources yet, and so have no compatible sources
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QVERIFY( w->value().value< QgsProcessingModelChildParameterSource>().outputChildId().isEmpty() );
  QCOMPARE( w->mStackedWidget->currentIndex(), 3 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Algorithm Output" ) );

  // populate sources and re-try
  w->populateSources( QStringList() << QStringLiteral( "boolean" ), QStringList() << QStringLiteral( "outputVector" ), QList<int>() );

  // model input
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().parameterName(), QStringLiteral( "p1" ) );

  // alg output
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().outputChildId(), QStringLiteral( "alg3" ) );
  QCOMPARE( w->value().value< QgsProcessingModelChildParameterSource>().outputName(), QStringLiteral( "OUTPUT" ) );

  // model output
  delete w;
  w = new QgsProcessingModelerParameterWidget( &model, "alg1", model.parameterDefinition( "test_dest" ), context );
  QCOMPARE( w->parameterDefinition()->name(), QStringLiteral( "test_dest" ) );
  // should default to being a model output for destination parameters, but with no value
  QVERIFY( w->isModelOutput() );
  QCOMPARE( w->modelOutputName(), QString() );
  // set it to something else
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) );
  QVERIFY( !w->isModelOutput() );
  // and back
  w->setToModelOutput( QStringLiteral( "out" ) );
  QVERIFY( w->isModelOutput() );
  QCOMPARE( w->modelOutputName(), QStringLiteral( "out" ) );
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) );
  w->setToModelOutput( QString() );
  QVERIFY( w->isModelOutput() );
  QCOMPARE( w->modelOutputName(), QString() );

  // multi-source input
  delete w;
  const QgsProcessingAlgorithm *packageAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:package" ) );
  const QgsProcessingParameterDefinition *layerDef = packageAlg->parameterDefinition( QStringLiteral( "LAYERS" ) );

  w = new QgsProcessingModelerParameterWidget( &model, "alg4", layerDef, context );

  w->setWidgetValue( QList< QgsProcessingModelChildParameterSource>()
                     << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) )
                     << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) )
                     << QgsProcessingModelChildParameterSource::fromStaticValue( QStringLiteral( "something" ) ) );
  QCOMPARE( w->value().toList().count(), 3 );

  QCOMPARE( w->value().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().outputChildId(), QStringLiteral( "alg3" ) );
  QCOMPARE( w->value().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().outputName(), QStringLiteral( "OUTPUT" ) );
  QCOMPARE( w->value().toList().at( 1 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( w->value().toList().at( 1 ).value< QgsProcessingModelChildParameterSource>().parameterName(), QStringLiteral( "p1" ) );
  QCOMPARE( w->value().toList().at( 2 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().toList().at( 2 ).value< QgsProcessingModelChildParameterSource>().staticValue().toString(), QStringLiteral( "something" ) );
  delete w;

}

void TestProcessingGui::testHiddenWrapper()
{
  TestParamType param( QStringLiteral( "boolean" ), QStringLiteral( "bool" ) );

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

  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( !wrapper.linkedVectorLayer() );
  wrapper.setLinkedVectorLayer( vl.get() );
  QCOMPARE( wrapper.linkedVectorLayer(), vl.get() );
}

void TestProcessingGui::testBooleanWrapper()
{
  TestParamType param( QStringLiteral( "boolean" ), QStringLiteral( "bool" ) );

  // standard wrapper
  QgsProcessingBooleanWidgetWrapper wrapper( &param );
  QSignalSpy spy( &wrapper, &QgsProcessingBooleanWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );
  wrapper.setWidgetValue( true, context );
  QCOMPARE( spy.count(), 1 );
  QVERIFY( wrapper.widgetValue().toBool() );
  QVERIFY( static_cast< QCheckBox * >( wrapper.wrappedWidget() )->isChecked() );
  wrapper.setWidgetValue( false, context );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( !wrapper.widgetValue().toBool() );
  QVERIFY( !static_cast< QCheckBox * >( wrapper.wrappedWidget() )->isChecked() );

  // should be no label in standard mode
  QVERIFY( !wrapper.createWrappedLabel() );
  QCOMPARE( static_cast< QCheckBox * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "bool" ) );

  // check signal
  static_cast< QCheckBox * >( wrapper.wrappedWidget() )->setChecked( true );
  QCOMPARE( spy.count(), 3 );
  static_cast< QCheckBox * >( wrapper.wrappedWidget() )->setChecked( false );
  QCOMPARE( spy.count(), 4 );

  delete w;

  // batch wrapper
  QgsProcessingBooleanWidgetWrapper wrapperB( &param, QgsProcessingGui::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingBooleanWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( true, context );
  QCOMPARE( spy2.count(), 1 );
  QVERIFY( wrapperB.widgetValue().toBool() );
  QVERIFY( static_cast< QComboBox * >( wrapperB.wrappedWidget() )->currentData().toBool() );
  wrapperB.setWidgetValue( false, context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( !wrapperB.widgetValue().toBool() );
  QVERIFY( !static_cast< QComboBox * >( wrapperB.wrappedWidget() )->currentData().toBool() );

  // check signal
  static_cast< QComboBox * >( w )->setCurrentIndex( 0 );
  QCOMPARE( spy2.count(), 3 );
  static_cast< QComboBox * >( w )->setCurrentIndex( 1 );
  QCOMPARE( spy2.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingBooleanWidgetWrapper wrapperM( &param, QgsProcessingGui::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingBooleanWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( true, context );
  QVERIFY( wrapperM.widgetValue().toBool() );
  QCOMPARE( spy3.count(), 1 );
  QVERIFY( static_cast< QComboBox * >( wrapperM.wrappedWidget() )->currentData().toBool() );
  wrapperM.setWidgetValue( false, context );
  QVERIFY( !wrapperM.widgetValue().toBool() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( !static_cast< QComboBox * >( wrapperM.wrappedWidget() )->currentData().toBool() );

  // check signal
  static_cast< QComboBox * >( w )->setCurrentIndex( 0 );
  QCOMPARE( spy3.count(), 3 );
  static_cast< QComboBox * >( w )->setCurrentIndex( 1 );
  QCOMPARE( spy3.count(), 4 );

  // should be a label in modeler mode
  QLabel *l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "bool" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "boolean" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterBoolean boolParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), true, false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "boolean" ), context, widgetContext, &boolParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( static_cast< QgsProcessingParameterBoolean * >( def.get() )->defaultValue().toBool() );
  boolParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  boolParam.setDefaultValue( false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "boolean" ), context, widgetContext, &boolParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( !static_cast< QgsProcessingParameterBoolean * >( def.get() )->defaultValue().toBool() );
}

void TestProcessingGui::testStringWrapper()
{
  QgsProcessingParameterString param( QStringLiteral( "string" ), QStringLiteral( "string" ) );

  // standard wrapper
  QgsProcessingStringWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( QStringLiteral( "a" ), context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "a" ) );
  wrapper.setWidgetValue( QString(), context );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( wrapper.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text().isEmpty() );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "string" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast< QLineEdit * >( wrapper.wrappedWidget() )->setText( QStringLiteral( "b" ) );
  QCOMPARE( spy.count(), 3 );
  static_cast< QLineEdit * >( wrapper.wrappedWidget() )->clear();
  QCOMPARE( spy.count(), 4 );

  delete w;

  // batch wrapper
  QgsProcessingStringWidgetWrapper wrapperB( &param, QgsProcessingGui::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( QStringLiteral( "a" ), context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QLineEdit * >( wrapperB.wrappedWidget() )->text(), QStringLiteral( "a" ) );
  wrapperB.setWidgetValue( QString(), context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( wrapperB.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast< QLineEdit * >( wrapperB.wrappedWidget() )->text().isEmpty() );

  // check signal
  static_cast< QLineEdit * >( w )->setText( QStringLiteral( "x" ) );
  QCOMPARE( spy2.count(), 3 );
  static_cast< QLineEdit * >( w )->clear();
  QCOMPARE( spy2.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingStringWidgetWrapper wrapperM( &param, QgsProcessingGui::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( QStringLiteral( "a" ), context );
  QCOMPARE( wrapperM.widgetValue().toString(), QStringLiteral( "a" ) );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( static_cast< QLineEdit * >( wrapperM.wrappedWidget() )->text(), QStringLiteral( "a" ) );
  wrapperM.setWidgetValue( QString(), context );
  QVERIFY( wrapperM.widgetValue().toString().isEmpty() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( static_cast< QLineEdit * >( wrapperM.wrappedWidget() )->text().isEmpty() );

  // check signal
  static_cast< QLineEdit * >( w )->setText( QStringLiteral( "x" ) );
  QCOMPARE( spy3.count(), 3 );
  static_cast< QLineEdit * >( w )->clear();
  QCOMPARE( spy3.count(), 4 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "string" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  //
  // multiline parameter
  //
  param = QgsProcessingParameterString( QStringLiteral( "string" ), QStringLiteral( "string" ), QVariant(), true );

  // standard wrapper
  QgsProcessingStringWidgetWrapper wrapperMultiLine( &param );

  w = wrapperMultiLine.createWrappedWidget( context );

  QSignalSpy spy4( &wrapperMultiLine, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperMultiLine.setWidgetValue( QStringLiteral( "a" ), context );
  QCOMPARE( spy4.count(), 1 );
  QCOMPARE( wrapperMultiLine.widgetValue().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QPlainTextEdit * >( wrapperMultiLine.wrappedWidget() )->toPlainText(), QStringLiteral( "a" ) );
  wrapperMultiLine.setWidgetValue( QString(), context );
  QCOMPARE( spy4.count(), 2 );
  QVERIFY( wrapperMultiLine.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast< QPlainTextEdit * >( wrapperMultiLine.wrappedWidget() )->toPlainText().isEmpty() );

  l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "string" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast< QPlainTextEdit * >( wrapperMultiLine.wrappedWidget() )->setPlainText( QStringLiteral( "b" ) );
  QCOMPARE( spy4.count(), 3 );
  static_cast< QPlainTextEdit * >( wrapperMultiLine.wrappedWidget() )->clear();
  QCOMPARE( spy4.count(), 4 );

  delete w;

  // batch wrapper - should still be a line edit
  QgsProcessingStringWidgetWrapper wrapperMultiLineB( &param, QgsProcessingGui::Batch );

  w = wrapperMultiLineB.createWrappedWidget( context );
  QSignalSpy spy5( &wrapperMultiLineB, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperMultiLineB.setWidgetValue( QStringLiteral( "a" ), context );
  QCOMPARE( spy5.count(), 1 );
  QCOMPARE( wrapperMultiLineB.widgetValue().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QLineEdit * >( wrapperMultiLineB.wrappedWidget() )->text(), QStringLiteral( "a" ) );
  wrapperMultiLineB.setWidgetValue( QString(), context );
  QCOMPARE( spy5.count(), 2 );
  QVERIFY( wrapperMultiLineB.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast< QLineEdit * >( wrapperMultiLineB.wrappedWidget() )->text().isEmpty() );

  // check signal
  static_cast< QLineEdit * >( w )->setText( QStringLiteral( "x" ) );
  QCOMPARE( spy5.count(), 3 );
  static_cast< QLineEdit * >( w )->clear();
  QCOMPARE( spy5.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingStringWidgetWrapper wrapperMultiLineM( &param, QgsProcessingGui::Modeler );

  w = wrapperMultiLineM.createWrappedWidget( context );
  QSignalSpy spy6( &wrapperMultiLineM, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperMultiLineM.setWidgetValue( QStringLiteral( "a" ), context );
  QCOMPARE( wrapperMultiLineM.widgetValue().toString(), QStringLiteral( "a" ) );
  QCOMPARE( spy6.count(), 1 );
  QCOMPARE( static_cast< QPlainTextEdit * >( wrapperMultiLineM.wrappedWidget() )->toPlainText(), QStringLiteral( "a" ) );
  wrapperMultiLineM.setWidgetValue( QString(), context );
  QVERIFY( wrapperMultiLineM.widgetValue().toString().isEmpty() );
  QCOMPARE( spy6.count(), 2 );
  QVERIFY( static_cast< QPlainTextEdit * >( wrapperMultiLineM.wrappedWidget() )->toPlainText().isEmpty() );

  // check signal
  static_cast< QPlainTextEdit * >( w )->setPlainText( QStringLiteral( "x" ) );
  QCOMPARE( spy6.count(), 3 );
  static_cast< QPlainTextEdit * >( w )->clear();
  QCOMPARE( spy6.count(), 4 );

  // should be a label in modeler mode
  l = wrapperMultiLineM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "string" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;


  //
  // with value hints
  //
  param = QgsProcessingParameterString( QStringLiteral( "string" ), QStringLiteral( "string" ), QVariant() );
  param.setMetadata( { {
      QStringLiteral( "widget_wrapper" ),
      QVariantMap(
      { {
          QStringLiteral( "value_hints" ),
          QStringList() << "value 1" << "value 2" << "value 3"
        }
      }
      )
    }
  } );

  QgsProcessingStringWidgetWrapper wrapperHints( &param );

  w = wrapperHints.createWrappedWidget( context );

  QSignalSpy spy7( &wrapperHints, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperHints.setWidgetValue( QStringLiteral( "value 2" ), context );
  QCOMPARE( spy7.count(), 1 );
  QCOMPARE( wrapperHints.widgetValue().toString(), QStringLiteral( "value 2" ) );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHints.wrappedWidget() )->currentText(), QStringLiteral( "value 2" ) );
  wrapperHints.setWidgetValue( QStringLiteral( "value 3" ), context );
  QCOMPARE( spy7.count(), 2 );
  QCOMPARE( wrapperHints.widgetValue().toString(), QStringLiteral( "value 3" ) );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHints.wrappedWidget() )->currentText(), QStringLiteral( "value 3" ) );

  // set to value which is not present -- should fallback to first value
  wrapperHints.setWidgetValue( QStringLiteral( "value 4" ), context );
  QCOMPARE( spy7.count(), 3 );
  QCOMPARE( wrapperHints.widgetValue().toString(), QStringLiteral( "value 1" ) );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHints.wrappedWidget() )->currentText(), QStringLiteral( "value 1" ) );

  l = wrapperHints.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "string" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  qgis::down_cast< QComboBox * >( wrapperHints.wrappedWidget() )->setCurrentIndex( 1 );
  QCOMPARE( spy7.count(), 4 );
  qgis::down_cast< QComboBox * >( wrapperHints.wrappedWidget() )->setCurrentIndex( 2 );
  QCOMPARE( spy7.count(), 5 );

  delete w;

  // with value hints, optional param
  param = QgsProcessingParameterString( QStringLiteral( "string" ), QStringLiteral( "string" ), QVariant(), false, true );
  param.setMetadata( { {
      QStringLiteral( "widget_wrapper" ),
      QVariantMap(
      { {
          QStringLiteral( "value_hints" ),
          QStringList() << "value 1" << "value 2" << "value 3"
        }
      }
      )
    }
  } );

  QgsProcessingStringWidgetWrapper wrapperHintsOptional( &param );

  w = wrapperHintsOptional.createWrappedWidget( context );

  QSignalSpy spy8( &wrapperHintsOptional, &QgsProcessingStringWidgetWrapper::widgetValueHasChanged );
  wrapperHintsOptional.setWidgetValue( QStringLiteral( "value 2" ), context );
  QCOMPARE( spy8.count(), 1 );
  QCOMPARE( wrapperHintsOptional.widgetValue().toString(), QStringLiteral( "value 2" ) );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHintsOptional.wrappedWidget() )->currentText(), QStringLiteral( "value 2" ) );
  wrapperHintsOptional.setWidgetValue( QVariant(), context );
  QCOMPARE( spy8.count(), 2 );
  QVERIFY( !wrapperHintsOptional.widgetValue().isValid() );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHintsOptional.wrappedWidget() )->currentText(), QString() );
  wrapperHintsOptional.setWidgetValue( QStringLiteral( "value 3" ), context );
  QCOMPARE( spy8.count(), 3 );
  QCOMPARE( wrapperHintsOptional.widgetValue().toString(), QStringLiteral( "value 3" ) );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHintsOptional.wrappedWidget() )->currentText(), QStringLiteral( "value 3" ) );

  // set to value which is not present -- should fallback to first value ("not set")
  wrapperHintsOptional.setWidgetValue( QStringLiteral( "value 4" ), context );
  QCOMPARE( spy8.count(), 4 );
  QVERIFY( !wrapperHintsOptional.widgetValue().isValid() );
  QCOMPARE( qgis::down_cast< QComboBox * >( wrapperHintsOptional.wrappedWidget() )->currentText(), QString() );

  l = wrapperHintsOptional.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "string [optional]" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  qgis::down_cast< QComboBox * >( wrapperHintsOptional.wrappedWidget() )->setCurrentIndex( 1 );
  QCOMPARE( spy8.count(), 5 );
  qgis::down_cast< QComboBox * >( wrapperHintsOptional.wrappedWidget() )->setCurrentIndex( 2 );
  QCOMPARE( spy8.count(), 6 );

  delete w;


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterString * >( def.get() )->multiLine() );

  // using a parameter definition as initial values
  QgsProcessingParameterString stringParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "aaa" ), true );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, &stringParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( static_cast< QgsProcessingParameterString * >( def.get() )->multiLine() );
  QCOMPARE( static_cast< QgsProcessingParameterString * >( def.get() )->defaultValue().toString(), QStringLiteral( "aaa" ) );
  stringParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  stringParam.setMultiLine( false );
  stringParam.setDefaultValue( QString() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, &stringParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( static_cast< QgsProcessingParameterString * >( def.get() )->defaultValue().toString().isEmpty() );
  QVERIFY( !static_cast< QgsProcessingParameterString * >( def.get() )->multiLine() );
}

void TestProcessingGui::testFileWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterFile param( QStringLiteral( "file" ), QStringLiteral( "file" ) );

    QgsProcessingFileWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFileWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QString( TEST_DATA_DIR + QStringLiteral( "/points.shp" ) ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QString( TEST_DATA_DIR + QStringLiteral( "/points.shp" ) ) );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper.wrappedWidget() )->filePath(), QString( TEST_DATA_DIR + QStringLiteral( "/points.shp" ) ) );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper.wrappedWidget() )->filter(), QStringLiteral( "All files (*.*)" ) );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper.wrappedWidget() )->storageMode(),  QgsFileWidget::GetFile );
    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast< QgsFileWidget * >( wrapper.wrappedWidget() )->filePath().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "file" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsFileWidget * >( wrapper.wrappedWidget() )->setFilePath( TEST_DATA_DIR + QStringLiteral( "/polys.shp" ) );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with extension
    QgsProcessingParameterFile param2( QStringLiteral( "file" ), QStringLiteral( "file" ), QgsProcessingParameterFile::File, QStringLiteral( "qml" ) );

    QgsProcessingFileWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper2.wrappedWidget() )->filter(), QStringLiteral( "QML files (*.qml)" ) );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper2.wrappedWidget() )->storageMode(),  QgsFileWidget::GetFile );

    // with filter
    QgsProcessingParameterFile param3( QStringLiteral( "file" ), QStringLiteral( "file" ), QgsProcessingParameterFile::File, QString(), QVariant(), false, QStringLiteral( "Project files (*.qgs *.qgz)" ) );

    QgsProcessingFileWidgetWrapper wrapper3( & param3, type );
    w = wrapper3.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper3.wrappedWidget() )->filter(), QStringLiteral( "Project files (*.qgs *.qgz)" ) );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper3.wrappedWidget() )->storageMode(),  QgsFileWidget::GetFile );

    // folder mode
    QgsProcessingParameterFile param4( QStringLiteral( "folder" ), QStringLiteral( "folder" ), QgsProcessingParameterFile::Folder );

    QgsProcessingFileWidgetWrapper wrapper4( &param4, type );
    w = wrapper4.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper4.wrappedWidget() )->storageMode(),  QgsFileWidget::GetDirectory );
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFile fileParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterFile::File );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext, &fileParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->behavior(), QgsProcessingParameterFile::File );
  QVERIFY( !static_cast< QgsProcessingParameterFile * >( def.get() )->defaultValue().isValid() );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->fileFilter(), QStringLiteral( "All files (*.*)" ) );
  fileParam.setFileFilter( QStringLiteral( "TAB files (*.tab)" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext, &fileParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->fileFilter(), QStringLiteral( "TAB files (*.tab)" ) );

  fileParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  fileParam.setBehavior( QgsProcessingParameterFile::Folder );
  fileParam.setDefaultValue( QStringLiteral( "my path" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext, &fileParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->behavior(), QgsProcessingParameterFile::Folder );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->defaultValue().toString(), QStringLiteral( "my path" ) );
}

void TestProcessingGui::testAuthCfgWrapper()
{
  QList<QgsAuthMethodConfig> configs;

  // Basic
  QgsAuthMethodConfig b_config;
  b_config.setId( QStringLiteral( "aaaaaaa" ) );
  b_config.setName( QStringLiteral( "Basic" ) );
  b_config.setMethod( QStringLiteral( "Basic" ) );
  b_config.setUri( QStringLiteral( "http://example.com" ) );
  b_config.setConfig( QStringLiteral( "username" ), QStringLiteral( "username" ) );
  b_config.setConfig( QStringLiteral( "password" ), QStringLiteral( "password" ) );
  b_config.setConfig( QStringLiteral( "realm" ), QStringLiteral( "Realm" ) );
  configs << b_config;

  QgsAuthMethodConfig b_config2;
  b_config2.setId( QStringLiteral( "bbbbbbb" ) );
  b_config2.setName( QStringLiteral( "Basic2" ) );
  b_config2.setMethod( QStringLiteral( "Basic" ) );
  b_config2.setUri( QStringLiteral( "http://example.com" ) );
  b_config2.setConfig( QStringLiteral( "username" ), QStringLiteral( "username" ) );
  b_config2.setConfig( QStringLiteral( "password" ), QStringLiteral( "password" ) );
  b_config2.setConfig( QStringLiteral( "realm" ), QStringLiteral( "Realm" ) );
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

  QgsProcessingParameterAuthConfig param( QStringLiteral( "authcfg" ), QStringLiteral( "authcfg" ) );

  // standard wrapper
  QgsProcessingAuthConfigWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingAuthConfigWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( authIds.at( 0 ), context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().toString(), authIds.at( 0 ) );
  QCOMPARE( static_cast< QgsAuthConfigSelect * >( wrapper.wrappedWidget() )->configId(), authIds.at( 0 ) );
  wrapper.setWidgetValue( authIds.at( 1 ), context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().toString(), authIds.at( 1 ) );
  QCOMPARE( static_cast< QgsAuthConfigSelect * >( wrapper.wrappedWidget() )->configId(), authIds.at( 1 ) );
  wrapper.setWidgetValue( QString(), context );
  QCOMPARE( spy.count(), 3 );
  QVERIFY( wrapper.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast< QgsAuthConfigSelect * >( wrapper.wrappedWidget() )->configId().isEmpty() );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "authcfg" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast< QgsAuthConfigSelect * >( wrapper.wrappedWidget() )->setConfigId( authIds.at( 0 ) );
  QCOMPARE( spy.count(), 4 );

  delete w;

  // batch wrapper
  QgsProcessingAuthConfigWidgetWrapper wrapperB( &param, QgsProcessingGui::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingAuthConfigWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( authIds.at( 0 ), context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().toString(), authIds.at( 0 ) );
  QCOMPARE( static_cast< QgsAuthConfigSelect * >( wrapperB.wrappedWidget() )->configId(), authIds.at( 0 ) );
  wrapperB.setWidgetValue( QString(), context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( wrapperB.widgetValue().toString().isEmpty() );
  QVERIFY( static_cast< QgsAuthConfigSelect * >( wrapperB.wrappedWidget() )->configId().isEmpty() );

  // check signal
  static_cast< QgsAuthConfigSelect * >( w )->setConfigId( authIds.at( 0 ) );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingAuthConfigWidgetWrapper wrapperM( &param, QgsProcessingGui::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingAuthConfigWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( authIds.at( 0 ), context );
  QCOMPARE( wrapperM.widgetValue().toString(), authIds.at( 0 ) );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( static_cast< QgsAuthConfigSelect * >( wrapperM.wrappedWidget() )->configId(), authIds.at( 0 ) );
  wrapperM.setWidgetValue( QString(), context );
  QVERIFY( wrapperM.widgetValue().toString().isEmpty() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( static_cast< QgsAuthConfigSelect * >( wrapperM.wrappedWidget() )->configId().isEmpty() );

  // check signal
  static_cast< QgsAuthConfigSelect * >( w )->setConfigId( authIds.at( 0 ) );
  QCOMPARE( spy3.count(), 3 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "authcfg" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;
}

void TestProcessingGui::testCrsWrapper()
{
  QgsProcessingParameterCrs param( QStringLiteral( "crs" ), QStringLiteral( "crs" ) );

  // standard wrapper
  QgsProcessingCrsWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );

  QSignalSpy spy( &wrapper, &QgsProcessingCrsWidgetWrapper::widgetValueHasChanged );
  wrapper.setWidgetValue( QStringLiteral( "epsg:3111" ), context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( wrapper.widgetValue().value< QgsCoordinateReferenceSystem >().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( static_cast< QgsProjectionSelectionWidget * >( wrapper.wrappedWidget() )->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  wrapper.setWidgetValue( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), context );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( wrapper.widgetValue().value< QgsCoordinateReferenceSystem >().authid(), QStringLiteral( "EPSG:28356" ) );
  QCOMPARE( static_cast< QgsProjectionSelectionWidget * >( wrapper.wrappedWidget() )->crs().authid(), QStringLiteral( "EPSG:28356" ) );
  wrapper.setWidgetValue( QString(), context );
  QCOMPARE( spy.count(), 3 );
  QVERIFY( !wrapper.widgetValue().value< QgsCoordinateReferenceSystem >().isValid() );
  QVERIFY( !static_cast< QgsProjectionSelectionWidget * >( wrapper.wrappedWidget() )->crs().isValid() );

  QLabel *l = wrapper.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "crs" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  static_cast< QgsProjectionSelectionWidget * >( wrapper.wrappedWidget() )->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  QCOMPARE( spy.count(), 4 );
  static_cast< QgsProjectionSelectionWidget * >( wrapper.wrappedWidget() )->setCrs( QgsCoordinateReferenceSystem() );
  QCOMPARE( spy.count(), 5 );

  delete w;

  // batch wrapper
  QgsProcessingCrsWidgetWrapper wrapperB( &param, QgsProcessingGui::Batch );

  w = wrapperB.createWrappedWidget( context );
  QSignalSpy spy2( &wrapperB, &QgsProcessingCrsWidgetWrapper::widgetValueHasChanged );
  wrapperB.setWidgetValue( QStringLiteral( "epsg:3111" ), context );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( wrapperB.widgetValue().value< QgsCoordinateReferenceSystem >().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( static_cast< QgsProjectionSelectionWidget * >( wrapperB.wrappedWidget() )->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  wrapperB.setWidgetValue( QgsCoordinateReferenceSystem(), context );
  QCOMPARE( spy2.count(), 2 );
  QVERIFY( !wrapperB.widgetValue().value< QgsCoordinateReferenceSystem >().isValid() );
  QVERIFY( !static_cast< QgsProjectionSelectionWidget * >( wrapperB.wrappedWidget() )->crs().isValid() );

  // check signal
  static_cast< QgsProjectionSelectionWidget * >( w )->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  QCOMPARE( spy2.count(), 3 );
  static_cast< QgsProjectionSelectionWidget * >( w )->setCrs( QgsCoordinateReferenceSystem() );
  QCOMPARE( spy2.count(), 4 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingCrsWidgetWrapper wrapperM( &param, QgsProcessingGui::Modeler );

  w = wrapperM.createWrappedWidget( context );
  QSignalSpy spy3( &wrapperM, &QgsProcessingCrsWidgetWrapper::widgetValueHasChanged );
  wrapperM.setWidgetValue( QStringLiteral( "epsg:3111" ), context );
  QCOMPARE( wrapperM.widgetValue().value< QgsCoordinateReferenceSystem >().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( wrapperM.mProjectionSelectionWidget->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QVERIFY( !wrapperM.mUseProjectCrsCheckBox->isChecked() );
  wrapperM.setWidgetValue( QgsCoordinateReferenceSystem(), context );
  QVERIFY( !wrapperM.widgetValue().value< QgsCoordinateReferenceSystem >().isValid() );
  QCOMPARE( spy3.count(), 2 );
  QVERIFY( !wrapperM.mProjectionSelectionWidget->crs().isValid() );
  QVERIFY( !wrapperM.mUseProjectCrsCheckBox->isChecked() );
  wrapperM.setWidgetValue( QStringLiteral( "ProjectCrs" ), context );
  QCOMPARE( wrapperM.widgetValue().toString(), QStringLiteral( "ProjectCrs" ) );
  QCOMPARE( spy3.count(), 3 );
  QVERIFY( wrapperM.mUseProjectCrsCheckBox->isChecked() );

  // check signal
  wrapperM.mUseProjectCrsCheckBox->setChecked( false );
  QCOMPARE( spy3.count(), 4 );
  wrapperM.mProjectionSelectionWidget->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28355" ) ) );
  QCOMPARE( spy3.count(), 5 );
  wrapperM.mProjectionSelectionWidget->setCrs( QgsCoordinateReferenceSystem() );
  QCOMPARE( spy3.count(), 6 );

  // should be a label in modeler mode
  l = wrapperM.createWrappedLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "crs" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "crs" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterCrs crsParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "EPSG:4326" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "crs" ), context, widgetContext, &crsParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterCrs * >( def.get() )->defaultValue().toString(), QStringLiteral( "EPSG:4326" ) );
  crsParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  crsParam.setDefaultValue( QStringLiteral( "EPSG:3111" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "crs" ), context, widgetContext, &crsParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterCrs * >( def.get() )->defaultValue().toString(), QStringLiteral( "EPSG:3111" ) );
}

void TestProcessingGui::testNumericWrapperDouble()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingContext context;

    QgsProcessingParameterNumber param( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double );
    QgsProcessingNumericWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->expressionsEnabled() );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->decimals(), 6 ); // you can change this, if it's an intentional change!
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->singleStep(), 1.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->minimum(), -999999999.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->maximum(), 999999999.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->clearValue(), 0.0 );

    QSignalSpy spy( &wrapper, &QgsProcessingNumericWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 5, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 5.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->value(), 5.0 );
    wrapper.setWidgetValue( QStringLiteral( "28356" ), context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 28356.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->value(), 28356.0 );
    wrapper.setWidgetValue( QVariant(), context ); // not optional, so shouldn't work
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 0.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->value(), 0.0 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "num" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->setValue( 37.0 );
    QCOMPARE( spy.count(), 4 );
    static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->clear();
    QCOMPARE( spy.count(), 5 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 0.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapper.wrappedWidget() )->value(), 0.0 );

    delete w;

    // with min value
    QgsProcessingParameterNumber paramMin( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double );
    paramMin.setMinimum( -5 );

    QgsProcessingNumericWidgetWrapper wrapperMin( &paramMin, type );

    w = wrapperMin.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMin.wrappedWidget() )->singleStep(), 1.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMin.wrappedWidget() )->minimum(), -5.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMin.wrappedWidget() )->maximum(), 999999999.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMin.wrappedWidget() )->clearValue(), -5.0 );
    QCOMPARE( wrapperMin.parameterValue().toDouble(), 0.0 );
    delete w;

    // with max value
    QgsProcessingParameterNumber paramMax( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double );
    paramMax.setMaximum( 5 );

    QgsProcessingNumericWidgetWrapper wrapperMax( &paramMax, type );

    w = wrapperMax.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMax.wrappedWidget() )->singleStep(), 1.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMax.wrappedWidget() )->minimum(), -999999999.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMax.wrappedWidget() )->maximum(), 5.0 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMax.wrappedWidget() )->clearValue(), 0.0 );
    QCOMPARE( wrapperMax.parameterValue().toDouble(), 0.0 );
    delete w;

    // with min and max value
    QgsProcessingParameterNumber paramMinMax( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double );
    paramMinMax.setMinimum( -.1 );
    paramMinMax.setMaximum( .1 );

    QgsProcessingNumericWidgetWrapper wrapperMinMax( &paramMinMax, type );

    w = wrapperMinMax.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMinMax.wrappedWidget() )->singleStep(), 0.02 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMinMax.wrappedWidget() )->minimum(), -.1 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMinMax.wrappedWidget() )->maximum(), .1 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperMinMax.wrappedWidget() )->clearValue(), -.1 );
    QCOMPARE( wrapperMinMax.parameterValue().toDouble(), 0.0 );
    delete w;

    // with default value
    QgsProcessingParameterNumber paramDefault( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double );
    paramDefault.setDefaultValue( 55 );

    QgsProcessingNumericWidgetWrapper wrapperDefault( &paramDefault, type );

    w = wrapperDefault.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperDefault.wrappedWidget() )->clearValue(), 55.0 );
    QCOMPARE( wrapperDefault.parameterValue().toDouble(), 55.0 );
    delete w;

    // optional, no default
    QgsProcessingParameterNumber paramOptional( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double, QVariant(), true );

    QgsProcessingNumericWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperOptional.wrappedWidget() )->clearValue(), -1000000000.0 );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    QCOMPARE( wrapperOptional.parameterValue().toDouble(), 5.0 );
    wrapperOptional.setParameterValue( QVariant(), context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    static_cast< QgsDoubleSpinBox * >( wrapperOptional.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptional.parameterValue().isValid() );

    // optional, with default
    paramOptional.setDefaultValue( 3 );
    QgsProcessingNumericWidgetWrapper wrapperOptionalDefault( &paramOptional, type );

    w = wrapperOptionalDefault.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperOptionalDefault.wrappedWidget() )->clearValue(), -1000000000.0 );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 3.0 );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    wrapperOptionalDefault.setParameterValue( QVariant(), context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperOptionalDefault.wrappedWidget() )->value(), -1000000000.0 );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    static_cast< QgsDoubleSpinBox * >( wrapperOptionalDefault.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );

    delete w;

    // with decimals
    QgsProcessingParameterNumber paramDecimals( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Double, QVariant(), true, 1, 1.02 );
    QVariantMap metadata;
    QVariantMap wrapperMetadata;
    wrapperMetadata.insert( QStringLiteral( "decimals" ), 2 );
    metadata.insert( QStringLiteral( "widget_wrapper" ), wrapperMetadata );
    paramDecimals.setMetadata( metadata );
    QgsProcessingNumericWidgetWrapper wrapperDecimals( &paramDecimals, type );
    w = wrapperDecimals.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperDecimals.wrappedWidget() )->decimals(), 2 );
    QCOMPARE( static_cast< QgsDoubleSpinBox * >( wrapperDecimals.wrappedWidget() )->singleStep(), 0.01 ); // single step should never be less than set number of decimals
    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterNumber numParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterNumber::Double, 1.0 );
  numParam.setMinimum( 0 );
  numParam.setMaximum( 10 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext, &numParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->dataType(), QgsProcessingParameterNumber::Double );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->minimum(), 0.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->maximum(), 10.0 );
  numParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  numParam.setDataType( QgsProcessingParameterNumber::Integer );
  numParam.setMinimum( -1 );
  numParam.setMaximum( 1 );
  numParam.setDefaultValue( 0 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext, &numParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->defaultValue().toInt(), 0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->dataType(), QgsProcessingParameterNumber::Integer );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->minimum(), -1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->maximum(), 1.0 );
}

void TestProcessingGui::testNumericWrapperInt()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingContext context;

    QgsProcessingParameterNumber param( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Integer );
    QgsProcessingNumericWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->expressionsEnabled() );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->minimum(), -999999999 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->maximum(), 999999999 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->clearValue(), 0 );

    QSignalSpy spy( &wrapper, &QgsProcessingNumericWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 5, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toInt(), 5 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->value(), 5 );
    wrapper.setWidgetValue( QStringLiteral( "28356" ), context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toInt(), 28356 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->value(), 28356 );
    wrapper.setWidgetValue( QVariant(), context ); // not optional, so shouldn't work
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toInt(), 0 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->value(), 0 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "num" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->setValue( 37 );
    QCOMPARE( spy.count(), 4 );
    static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->clear();
    QCOMPARE( spy.count(), 5 );
    QCOMPARE( wrapper.widgetValue().toInt(), 0 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapper.wrappedWidget() )->value(), 0 );

    delete w;

    // with min value
    QgsProcessingParameterNumber paramMin( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Integer );
    paramMin.setMinimum( -5 );

    QgsProcessingNumericWidgetWrapper wrapperMin( &paramMin, type );

    w = wrapperMin.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMin.wrappedWidget() )->minimum(), -5 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMin.wrappedWidget() )->maximum(), 999999999 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMin.wrappedWidget() )->clearValue(), -5 );
    QCOMPARE( wrapperMin.parameterValue().toInt(), 0 );
    delete w;

    // with max value
    QgsProcessingParameterNumber paramMax( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Integer );
    paramMax.setMaximum( 5 );

    QgsProcessingNumericWidgetWrapper wrapperMax( &paramMax, type );

    w = wrapperMax.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMax.wrappedWidget() )->minimum(), -999999999 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMax.wrappedWidget() )->maximum(), 5 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMax.wrappedWidget() )->clearValue(), 0 );
    QCOMPARE( wrapperMax.parameterValue().toInt(), 0 );
    delete w;

    // with min and max value
    QgsProcessingParameterNumber paramMinMax( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Integer );
    paramMinMax.setMinimum( -1 );
    paramMinMax.setMaximum( 1 );

    QgsProcessingNumericWidgetWrapper wrapperMinMax( &paramMinMax, type );

    w = wrapperMinMax.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMinMax.wrappedWidget() )->minimum(), -1 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMinMax.wrappedWidget() )->maximum(), 1 );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperMinMax.wrappedWidget() )->clearValue(), -1 );
    QCOMPARE( wrapperMinMax.parameterValue().toInt(), 0 );
    delete w;

    // with default value
    QgsProcessingParameterNumber paramDefault( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Integer );
    paramDefault.setDefaultValue( 55 );

    QgsProcessingNumericWidgetWrapper wrapperDefault( &paramDefault, type );

    w = wrapperDefault.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperDefault.wrappedWidget() )->clearValue(), 55 );
    QCOMPARE( wrapperDefault.parameterValue().toInt(), 55 );
    delete w;

    // optional, no default
    QgsProcessingParameterNumber paramOptional( QStringLiteral( "num" ), QStringLiteral( "num" ), QgsProcessingParameterNumber::Integer, QVariant(), true );

    QgsProcessingNumericWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperOptional.wrappedWidget() )->clearValue(), -1000000000 );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    QCOMPARE( wrapperOptional.parameterValue().toInt(), 5 );
    wrapperOptional.setParameterValue( QVariant(), context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    static_cast< QgsSpinBox * >( wrapperOptional.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptional.parameterValue().isValid() );

    // optional, with default
    paramOptional.setDefaultValue( 3 );
    QgsProcessingNumericWidgetWrapper wrapperOptionalDefault( &paramOptional, type );

    w = wrapperOptionalDefault.createWrappedWidget( context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperOptionalDefault.wrappedWidget() )->clearValue(), -1000000000 );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 3 );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 5 );
    wrapperOptionalDefault.setParameterValue( QVariant(), context );
    QCOMPARE( static_cast< QgsSpinBox * >( wrapperOptionalDefault.wrappedWidget() )->value(), -1000000000 );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 5 );
    static_cast< QgsSpinBox * >( wrapperOptionalDefault.wrappedWidget() )->clear();
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toInt(), 5 );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterNumber numParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterNumber::Integer, 1 );
  numParam.setMinimum( 0 );
  numParam.setMaximum( 10 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext, &numParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->dataType(), QgsProcessingParameterNumber::Integer );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->minimum(), 0.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->maximum(), 10.0 );
  numParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  numParam.setDataType( QgsProcessingParameterNumber::Double );
  numParam.setMinimum( -2.5 );
  numParam.setMaximum( 2.5 );
  numParam.setDefaultValue( 0.5 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext, &numParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->defaultValue().toDouble(), 0.5 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->dataType(), QgsProcessingParameterNumber::Double );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->minimum(), -2.5 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->maximum(), 2.5 );

  // integer type, no min/max values set
  QgsProcessingParameterNumber numParam2( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterNumber::Integer, 1 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext, &numParam2 );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->dataType(), QgsProcessingParameterNumber::Integer );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->minimum(), numParam2.minimum() );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->maximum(), numParam2.maximum() );

  // double type, no min/max values set
  QgsProcessingParameterNumber numParam3( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterNumber::Double, 1 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "number" ), context, widgetContext, &numParam3 );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->dataType(), QgsProcessingParameterNumber::Double );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->minimum(), numParam3.minimum() );
  QCOMPARE( static_cast< QgsProcessingParameterNumber * >( def.get() )->maximum(), numParam3.maximum() );
}

void TestProcessingGui::testDistanceWrapper()
{
  QgsProcessingParameterDistance param( QStringLiteral( "distance" ), QStringLiteral( "distance" ) );

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
  QCOMPARE( l->text(), QStringLiteral( "distance" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  wrapper.mDoubleSpinBox->setValue( 43.0 );
  QCOMPARE( spy.count(), 3 );

  // test unit handling
  w->show();

  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "<unknown>" ) );

  // crs values
  wrapper.setUnitParameterValue( QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "meters" ) );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast< int >( QgsUnitTypes::DistanceMeters ) );

  wrapper.setUnitParameterValue( QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "degrees" ) );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "meters" ) );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast< int >( QgsUnitTypes::DistanceMeters ) );

  wrapper.setUnitParameterValue( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "degrees" ) );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // layer values
  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl.get() ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "meters" ) );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast< int >( QgsUnitTypes::DistanceMeters ) );

  std::unique_ptr< QgsVectorLayer > vl2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl2.get() ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "degrees" ) );
  QVERIFY( wrapper.mWarningLabel->isVisible() );
  QVERIFY( !wrapper.mUnitsCombo->isVisible() );
  QVERIFY( wrapper.mLabel->isVisible() );

  // unresolvable values
  wrapper.setUnitParameterValue( QStringLiteral( "blah" ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "<unknown>" ) );
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
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "meters" ) );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast< int >( QgsUnitTypes::DistanceMeters ) );

  // using unit choice
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );
  wrapper.mUnitsCombo->setCurrentIndex( wrapper.mUnitsCombo->findData( QgsUnitTypes::DistanceKilometers ) );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5000.0 );
  wrapper.setParameterValue( 2, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2000.0 );

  wrapper.setUnitParameterValue( id );
  QCOMPARE( wrapper.parameterValue().toDouble(), 2.0 );
  wrapper.setParameterValue( 5, context );
  QCOMPARE( wrapper.parameterValue().toDouble(), 5.0 );

  delete w;

  // with default unit
  QgsProcessingParameterDistance paramDefaultUnit( QStringLiteral( "num" ), QStringLiteral( "num" ) );
  paramDefaultUnit.setDefaultUnit( QgsUnitTypes::DistanceFeet );
  QgsProcessingDistanceWidgetWrapper wrapperDefaultUnit( &paramDefaultUnit, QgsProcessingGui::Standard );
  w = wrapperDefaultUnit.createWrappedWidget( context );
  w->show();
  QCOMPARE( wrapperDefaultUnit.mLabel->text(), QStringLiteral( "feet" ) );
  delete w;

  // with decimals
  QgsProcessingParameterDistance paramDecimals( QStringLiteral( "num" ), QStringLiteral( "num" ), QVariant(), QString(), true, 1, 1.02 );
  QVariantMap metadata;
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( QStringLiteral( "decimals" ), 2 );
  metadata.insert( QStringLiteral( "widget_wrapper" ), wrapperMetadata );
  paramDecimals.setMetadata( metadata );
  QgsProcessingDistanceWidgetWrapper wrapperDecimals( &paramDecimals, QgsProcessingGui::Standard );
  w = wrapperDecimals.createWrappedWidget( context );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->decimals(), 2 );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->singleStep(), 0.01 ); // single step should never be less than set number of decimals
  delete w;

  // batch wrapper
  QgsProcessingDistanceWidgetWrapper wrapperB( &param, QgsProcessingGui::Batch );

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
  static_cast< QgsDoubleSpinBox * >( w )->setValue( 29 );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingDistanceWidgetWrapper wrapperM( &param, QgsProcessingGui::Modeler );

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
  QCOMPARE( l->text(), QStringLiteral( "distance" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "distance" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterDistance distParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), 1, QStringLiteral( "parent" ) );
  distParam.setMinimum( 1 );
  distParam.setMaximum( 100 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "distance" ), context, widgetContext, &distParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->minimum(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->maximum(), 100.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->parentParameterName(), QStringLiteral( "parent" ) );
  distParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  distParam.setParentParameterName( QString() );
  distParam.setMinimum( 10 );
  distParam.setMaximum( 12 );
  distParam.setDefaultValue( 11.5 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "distance" ), context, widgetContext, &distParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->defaultValue().toDouble(), 11.5 );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->minimum(), 10.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDistance * >( def.get() )->maximum(), 12.0 );
  QVERIFY( static_cast< QgsProcessingParameterDistance * >( def.get() )->parentParameterName().isEmpty() );
}

void TestProcessingGui::testDurationWrapper()
{
  QgsProcessingParameterDuration param( QStringLiteral( "duration" ), QStringLiteral( "duration" ) );

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
  QCOMPARE( l->text(), QStringLiteral( "duration" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete l;

  // check signal
  wrapper.mDoubleSpinBox->setValue( 43.0 );
  QCOMPARE( spy.count(), 3 );

  // with default unit
  QgsProcessingParameterDuration paramDefaultUnit( QStringLiteral( "dur" ), QStringLiteral( "dur" ) );
  paramDefaultUnit.setDefaultUnit( QgsUnitTypes::TemporalDays );
  QgsProcessingDurationWidgetWrapper wrapperDefaultUnit( &paramDefaultUnit, QgsProcessingGui::Standard );
  w = wrapperDefaultUnit.createWrappedWidget( context );
  w->show();
  QCOMPARE( wrapperDefaultUnit.mUnitsCombo->currentText(), QgsUnitTypes::toString( QgsUnitTypes::TemporalDays ) );
  delete w;

  // with decimals
  QgsProcessingParameterDuration paramDecimals( QStringLiteral( "num" ), QStringLiteral( "num" ), QVariant(), true, 1, 1.02 );
  QVariantMap metadata;
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( QStringLiteral( "decimals" ), 2 );
  metadata.insert( QStringLiteral( "widget_wrapper" ), wrapperMetadata );
  paramDecimals.setMetadata( metadata );
  QgsProcessingDurationWidgetWrapper wrapperDecimals( &paramDecimals, QgsProcessingGui::Standard );
  w = wrapperDecimals.createWrappedWidget( context );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->decimals(), 2 );
  QCOMPARE( wrapperDecimals.mDoubleSpinBox->singleStep(), 0.01 ); // single step should never be less than set number of decimals
  delete w;

  // batch wrapper
  QgsProcessingDurationWidgetWrapper wrapperB( &param, QgsProcessingGui::Batch );

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
  static_cast< QgsDoubleSpinBox * >( w )->setValue( 29 );
  QCOMPARE( spy2.count(), 3 );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;

  // modeler wrapper
  QgsProcessingDurationWidgetWrapper wrapperM( &param, QgsProcessingGui::Modeler );

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
  QCOMPARE( l->text(), QStringLiteral( "duration [milliseconds]" ) );
  QCOMPARE( l->toolTip(), param.toolTip() );
  delete w;
  delete l;

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "duration" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterDuration durParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), 1 );
  durParam.setMinimum( 1 );
  durParam.setMaximum( 100 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "duration" ), context, widgetContext, &durParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterDuration * >( def.get() )->defaultValue().toDouble(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDuration * >( def.get() )->minimum(), 1.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDuration * >( def.get() )->maximum(), 100.0 );
  durParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  durParam.setMinimum( 10 );
  durParam.setMaximum( 12 );
  durParam.setDefaultValue( 11.5 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "duration" ), context, widgetContext, &durParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterDuration * >( def.get() )->defaultValue().toDouble(), 11.5 );
  QCOMPARE( static_cast< QgsProcessingParameterDuration * >( def.get() )->minimum(), 10.0 );
  QCOMPARE( static_cast< QgsProcessingParameterDuration * >( def.get() )->maximum(), 12.0 );
}

void TestProcessingGui::testScaleWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingContext context;

    QgsProcessingParameterScale param( QStringLiteral( "num" ), QStringLiteral( "num" ) );
    QgsProcessingScaleWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QSignalSpy spy( &wrapper, &QgsProcessingNumericWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 5, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 5.0 );
    QCOMPARE( static_cast< QgsScaleWidget * >( wrapper.wrappedWidget() )->scale(), 5.0 );
    wrapper.setWidgetValue( QStringLiteral( "28356" ), context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 28356.0 );
    QCOMPARE( static_cast< QgsScaleWidget * >( wrapper.wrappedWidget() )->scale(), 28356.0 );
    wrapper.setWidgetValue( QVariant(), context ); // not optional, so shouldn't work
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toDouble(), 0.0 );
    QCOMPARE( static_cast< QgsScaleWidget * >( wrapper.wrappedWidget() )->scale(), 0.0 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "num" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsScaleWidget * >( wrapper.wrappedWidget() )->setScale( 37.0 );
    QCOMPARE( spy.count(), 4 );

    delete w;

    // optional, no default
    QgsProcessingParameterScale paramOptional( QStringLiteral( "num" ), QStringLiteral( "num" ), QVariant(), true );

    QgsProcessingScaleWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    QCOMPARE( wrapperOptional.parameterValue().toDouble(), 5.0 );
    wrapperOptional.setParameterValue( QVariant(), context );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );
    wrapperOptional.setParameterValue( 5, context );
    static_cast< QgsScaleWidget * >( wrapperOptional.wrappedWidget() )->setScale( std::numeric_limits< double >::quiet_NaN() );
    QVERIFY( !wrapperOptional.parameterValue().isValid() );

    // optional, with default
    paramOptional.setDefaultValue( 3 );
    QgsProcessingScaleWidgetWrapper wrapperOptionalDefault( &paramOptional, type );

    w = wrapperOptionalDefault.createWrappedWidget( context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 3.0 );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    wrapperOptionalDefault.setParameterValue( QVariant(), context );
    QVERIFY( std::isnan( static_cast< QgsScaleWidget * >( wrapperOptionalDefault.wrappedWidget() )->scale() ) );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );
    static_cast< QgsScaleWidget * >( wrapperOptionalDefault.wrappedWidget() )->setScale( std::numeric_limits< double >::quiet_NaN() );
    QVERIFY( !wrapperOptionalDefault.parameterValue().isValid() );
    wrapperOptionalDefault.setParameterValue( 5, context );
    QCOMPARE( wrapperOptionalDefault.parameterValue().toDouble(), 5.0 );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "scale" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterScale scaleParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), 1000 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "scale" ), context, widgetContext, &scaleParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterScale * >( def.get() )->defaultValue().toDouble(), 1000.0 );
  scaleParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  scaleParam.setDefaultValue( 28356 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "scale" ), context, widgetContext, &scaleParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterScale * >( def.get() )->defaultValue().toDouble(), 28356.0 );
}

void TestProcessingGui::testRangeWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingContext context;

    QgsProcessingParameterRange param( QStringLiteral( "range" ), QStringLiteral( "range" ), QgsProcessingParameterNumber::Double );
    param.setDefaultValue( QStringLiteral( "0.0,100.0" ) );
    QgsProcessingRangeWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( w );

    // initial value
    QCOMPARE( wrapper.parameterValue().toString(), QStringLiteral( "0,100" ) );

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
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "5,7" ) );
    QCOMPARE( wrapper.mMinSpinBox->value(), 5.0 );
    QCOMPARE( wrapper.mMaxSpinBox->value(), 7.0 );
    wrapper.setWidgetValue( QStringLiteral( "28.1,36.5" ), context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "28.1,36.5" ) );
    QCOMPARE( wrapper.mMinSpinBox->value(), 28.1 );
    QCOMPARE( wrapper.mMaxSpinBox->value(), 36.5 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "range" ) );
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
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "7,36.5" ) );
    wrapper.mMaxSpinBox->setValue( 9.0 );
    QCOMPARE( spy.count(), 4 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "7,9" ) );

    // check that min/max are mutually adapted
    wrapper.setParameterValue( QStringLiteral( "200.0,100.0" ), context );
    QCOMPARE( wrapper.parameterValue().toString(), QStringLiteral( "100,100" ) );

    wrapper.mMaxSpinBox->setValue( 50 );
    QCOMPARE( wrapper.parameterValue().toString(), QStringLiteral( "50,50" ) );
    wrapper.mMinSpinBox->setValue( 100 );
    QCOMPARE( wrapper.parameterValue().toString(), QStringLiteral( "100,100" ) );

    delete w;

    // ints
    QgsProcessingParameterRange param2( QStringLiteral( "range" ), QStringLiteral( "range" ), QgsProcessingParameterNumber::Integer );
    param2.setDefaultValue( QStringLiteral( "0.1,100.1" ) );

    QgsProcessingRangeWidgetWrapper wrapper2( &param2, type );

    w = wrapper2.createWrappedWidget( context );
    QVERIFY( w );
    QCOMPARE( wrapper2.mMinSpinBox->decimals(), 0 );
    QCOMPARE( wrapper2.mMaxSpinBox->decimals(), 0 ); // you can't change this, vampire worms will bite you at night if you do

    // check initial value
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "0,100" ) );
    // check rounding
    wrapper2.setParameterValue( QStringLiteral( "100.1,200.1" ), context );
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "100,200" ) );
    wrapper2.setParameterValue( QStringLiteral( "100.6,200.6" ), context );
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "101,201" ) );
    // check set/get
    wrapper2.setParameterValue( QStringLiteral( "100.1,200.1" ), context );
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "100,200" ) );
    // check that min/max are mutually adapted
    wrapper2.setParameterValue( QStringLiteral( "200.1,100.1" ), context );
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "100,100" ) );
    wrapper2.mMaxSpinBox->setValue( 50.1 );
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "50,50" ) );
    wrapper2.mMinSpinBox->setValue( 100.1 );
    QCOMPARE( wrapper2.parameterValue().toString(), QStringLiteral( "100,100" ) );

    delete w;

    // optional
    QgsProcessingParameterRange paramOptional( QStringLiteral( "range" ), QStringLiteral( "range" ), QgsProcessingParameterNumber::Double, QVariant(), true );

    QgsProcessingRangeWidgetWrapper wrapperOptional( &paramOptional, type );

    w = wrapperOptional.createWrappedWidget( context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "None,None" ) );
    wrapperOptional.setParameterValue( QStringLiteral( "1,100" ), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "1,100" ) );
    wrapperOptional.setParameterValue( QStringLiteral( "None,100" ), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "None,100" ) );
    wrapperOptional.setParameterValue( QStringLiteral( "1,None" ), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "1,None" ) );
    wrapperOptional.setParameterValue( QStringLiteral( "None,None" ), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "None,None" ) );
    wrapperOptional.setParameterValue( QStringLiteral( "None" ), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "None,None" ) );
    wrapperOptional.setParameterValue( QVariant(), context );
    QCOMPARE( wrapperOptional.parameterValue().toString(), QStringLiteral( "None,None" ) );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "range" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterRange rangeParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterNumber::Integer, QStringLiteral( "0,255" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "range" ), context, widgetContext, &rangeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterRange * >( def.get() )->defaultValue().toString(), QStringLiteral( "0,255" ) );
  QCOMPARE( static_cast< QgsProcessingParameterRange * >( def.get() )->dataType(), QgsProcessingParameterNumber::Integer );
  rangeParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  rangeParam.setDataType( QgsProcessingParameterNumber::Double );
  rangeParam.setDefaultValue( QStringLiteral( "0,1" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "range" ), context, widgetContext, &rangeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterRange * >( def.get() )->defaultValue().toString(), QStringLiteral( "0,1" ) );
  QCOMPARE( static_cast< QgsProcessingParameterRange * >( def.get() )->dataType(), QgsProcessingParameterNumber::Double );
}

void TestProcessingGui::testMatrixDialog()
{
  QgsProcessingParameterMatrix matrixParam( QString(), QString(), 3, false, QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  std::unique_ptr< QgsProcessingMatrixParameterPanelWidget > dlg = std::make_unique< QgsProcessingMatrixParameterPanelWidget>( nullptr, &matrixParam );
  // variable length table
  QVERIFY( dlg->mButtonAdd->isEnabled() );
  QVERIFY( dlg->mButtonRemove->isEnabled() );
  QVERIFY( dlg->mButtonRemoveAll->isEnabled() );

  QCOMPARE( dlg->table(), QVariantList() );

  dlg = std::make_unique< QgsProcessingMatrixParameterPanelWidget >( nullptr, &matrixParam, QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) );
  QCOMPARE( dlg->table(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) );
  dlg->addRow();
  QCOMPARE( dlg->table(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) << QString() << QString() );
  dlg->deleteAllRows();
  QCOMPARE( dlg->table(), QVariantList() );

  QgsProcessingParameterMatrix matrixParam2( QString(), QString(), 3, true, QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  dlg = std::make_unique< QgsProcessingMatrixParameterPanelWidget >( nullptr, &matrixParam2, QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) );
  QVERIFY( !dlg->mButtonAdd->isEnabled() );
  QVERIFY( !dlg->mButtonRemove->isEnabled() );
  QVERIFY( !dlg->mButtonRemoveAll->isEnabled() );
}

void TestProcessingGui::testMatrixWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingContext context;

    QgsProcessingParameterMatrix param( QStringLiteral( "matrix" ), QStringLiteral( "matrix" ), 3, false, QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
    param.setDefaultValue( QStringLiteral( "0.0,100.0,150.0,250.0" ) );
    QgsProcessingMatrixWidgetWrapper wrapper( &param, type );

    QWidget *w = wrapper.createWrappedWidget( context );
    QVERIFY( w );

    // initial value
    QCOMPARE( wrapper.parameterValue().toList(), QVariantList() << QStringLiteral( "0.0" ) << QStringLiteral( "100.0" ) << QStringLiteral( "150.0" ) << QStringLiteral( "250.0" ) );

    QSignalSpy spy( &wrapper, &QgsProcessingMatrixWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << 5 << 7, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << QStringLiteral( "5" ) << QStringLiteral( "7" ) );
    QCOMPARE( wrapper.mMatrixWidget->value(), QVariantList() << QStringLiteral( "5" ) << QStringLiteral( "7" ) );
    wrapper.setWidgetValue( QStringLiteral( "28.1,36.5,5.5,8.9" ), context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << QStringLiteral( "28.1" ) << QStringLiteral( "36.5" ) << QStringLiteral( "5.5" ) << QStringLiteral( "8.9" ) );
    QCOMPARE( wrapper.mMatrixWidget->value(), QVariantList() << QStringLiteral( "28.1" ) << QStringLiteral( "36.5" ) << QStringLiteral( "5.5" ) << QStringLiteral( "8.9" ) );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "matrix" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    wrapper.mMatrixWidget->setValue( QVariantList() << QStringLiteral( "7" ) << QStringLiteral( "9" ) );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << QStringLiteral( "7" ) << QStringLiteral( "9" ) );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "matrix" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterMatrix matrixParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), 1, false, QStringList() << "A" << "B" << "C", QVariantList() << 0 << 0 << 0 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "matrix" ), context, widgetContext, &matrixParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterMatrix * >( def.get() )->headers(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast< QgsProcessingParameterMatrix * >( def.get() )->defaultValue().toStringList(), QStringList() << "0" << "0" << "0" );
  QVERIFY( !static_cast< QgsProcessingParameterMatrix * >( def.get() )->hasFixedNumberRows() );
  matrixParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  matrixParam.setHasFixedNumberRows( true );
  matrixParam.setDefaultValue( QVariantList() << 1 << 2 << 3 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "matrix" ), context, widgetContext, &matrixParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterMatrix * >( def.get() )->headers(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast< QgsProcessingParameterMatrix * >( def.get() )->defaultValue().toStringList(), QStringList() << "1" << "2" << "3" );
  QVERIFY( static_cast< QgsProcessingParameterMatrix * >( def.get() )->hasFixedNumberRows() );
}

void TestProcessingGui::testExpressionWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:centroids" ) );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( QStringLiteral( "INPUT" ) );

  auto testWrapper = [layerDef]( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterExpression param( QStringLiteral( "expression" ), QStringLiteral( "expression" ) );

    QgsProcessingExpressionWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingExpressionWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "1+2" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(),  QStringLiteral( "1+2" ) );
    QCOMPARE( static_cast< QgsExpressionLineEdit * >( wrapper.wrappedWidget() )->expression(),  QStringLiteral( "1+2" ) );
    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast< QgsExpressionLineEdit * >( wrapper.wrappedWidget() )->expression().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "expression" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsExpressionLineEdit * >( wrapper.wrappedWidget() )->setExpression( QStringLiteral( "3+4" ) );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( QStringLiteral( "other" ) );
    QgsProcessingExpressionWidgetWrapper wrapper2( &param, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingExpressionWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QStringLiteral( "11+12" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(),  QStringLiteral( "11+12" ) );
    QCOMPARE( static_cast< QgsFieldExpressionWidget * >( wrapper2.wrappedWidget() )->expression(),  QStringLiteral( "11+12" ) );

    wrapper2.setWidgetValue( QString(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( wrapper2.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast< QgsFieldExpressionWidget * >( wrapper2.wrappedWidget() )->expression().isEmpty() );

    static_cast< QgsFieldExpressionWidget * >( wrapper2.wrappedWidget() )->setExpression( QStringLiteral( "3+4" ) );
    QCOMPARE( spy2.count(), 3 );

    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
    p.addMapLayer( vl );

    QVERIFY( !wrapper2.mFieldExpWidget->layer() );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mFieldExpWidget->layer(), vl );

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QVERIFY( !wrapper2.mFieldExpWidget->layer() );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QVERIFY( !wrapper2.mFieldExpWidget->layer() );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // with project layer
    context.setProject( &p );
    TestProcessingContextGenerator generator( context );
    wrapper2.registerProcessingContextGenerator( &generator );

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mFieldExpWidget->layer(), vl );
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mFieldExpWidget->layer()->publicSource(), pointFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "expression" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterExpression exprParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QVariant(), QStringLiteral( "parent" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "expression" ), context, widgetContext, &exprParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterExpression * >( def.get() )->parentLayerParameterName(), QStringLiteral( "parent" ) );
  exprParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  exprParam.setParentLayerParameterName( QString() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "expression" ), context, widgetContext, &exprParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( static_cast< QgsProcessingParameterExpression * >( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testFieldSelectionPanel()
{
  QgsProcessingParameterField fieldParam( QString(), QString(), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true );
  QgsProcessingFieldPanelWidget w( nullptr, &fieldParam );
  QSignalSpy spy( &w, &QgsProcessingFieldPanelWidget::changed );

  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 field(s) selected" ) );
  w.setValue( QStringLiteral( "aa" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.value().toList(), QVariantList() << QStringLiteral( "aa" ) );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "aa" ) );

  w.setValue( QVariantList() << QStringLiteral( "bb" ) << QStringLiteral( "aa" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << QStringLiteral( "bb" ) << QStringLiteral( "aa" ) );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "bb,aa" ) );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 field(s) selected" ) );

}

void TestProcessingGui::testFieldWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:centroids" ) );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( QStringLiteral( "INPUT" ) );

  auto testWrapper = [layerDef]( QgsProcessingGui::WidgetType type )
  {
    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString?field=aaa:int&field=bbb:string" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
    p.addMapLayer( vl );

    QgsProcessingParameterField param( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ) );

    QgsProcessingFieldWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void )w;
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper.setParentLayerWrapperValue( &layerWrapper );

    QSignalSpy spy( &wrapper, &QgsProcessingFieldWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bbb" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(),  QStringLiteral( "bbb" ) );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsFieldComboBox * >( wrapper.wrappedWidget() )->currentField(),  QStringLiteral( "bbb" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(),  QStringLiteral( "bbb" ) );
        break;
    }

    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );

    delete w;

    // optional
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true );

    QgsProcessingFieldWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy2( &wrapper2, &QgsProcessingFieldWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QStringLiteral( "aaa" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(),  QStringLiteral( "aaa" ) );

    wrapper2.setWidgetValue( QString(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( wrapper2.widgetValue().toString().isEmpty() );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsFieldComboBox * >( wrapper2.wrappedWidget() )->currentField(), QString() );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(),  QString() );
        break;
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "field [optional]" ) );
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
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        static_cast< QgsFieldComboBox * >( wrapper2.wrappedWidget() )->setField( QStringLiteral( "bbb" ) );
        break;

      case QgsProcessingGui::Modeler:
        static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->setText( QStringLiteral( "bbb" ) );
        break;
    }

    QCOMPARE( spy2.count(), 3 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), vl );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }

    layerWrapper.setWidgetValue( vl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case QgsProcessingGui::Modeler:
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
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), vl );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper2.mComboBox->layer()->publicSource(), pointFileName );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }

    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );

    delete w;

    // multiple
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true );

    QgsProcessingFieldWidgetWrapper wrapper3( &param, type );

    w = wrapper3.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( vl ), context );
    wrapper3.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy3( &wrapper3, &QgsProcessingFieldWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "aaa" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << QStringLiteral( "aaa" ) );

    wrapper3.setWidgetValue( QString(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( wrapper3.widgetValue().toString().isEmpty() );

    wrapper3.setWidgetValue( QStringLiteral( "aaa;bbb" ), context );
    QCOMPARE( spy3.count(), 3 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << QStringLiteral( "aaa" ) << QStringLiteral( "bbb" ) );

    delete w;

    // filtering fields
    QgsFields f;
    f.append( QgsField( QStringLiteral( "string" ), QVariant::String ) );
    f.append( QgsField( QStringLiteral( "double" ), QVariant::Double ) );
    f.append( QgsField( QStringLiteral( "int" ), QVariant::Int ) );
    f.append( QgsField( QStringLiteral( "date" ), QVariant::Date ) );
    f.append( QgsField( QStringLiteral( "time" ), QVariant::Time ) );
    f.append( QgsField( QStringLiteral( "datetime" ), QVariant::DateTime ) );

    QgsFields f2 = wrapper3.filterFields( f );
    QCOMPARE( f2, f );

    // string fields
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::String, false, true );
    QgsProcessingFieldWidgetWrapper wrapper4( &param, type );
    w = wrapper4.createWrappedWidget( context );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsFieldComboBox * >( wrapper4.wrappedWidget() )->filters(), QgsFieldProxyModel::String );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    f2 = wrapper4.filterFields( f );
    QCOMPARE( f2.size(), 1 );
    QCOMPARE( f2.at( 0 ).name(), QStringLiteral( "string" ) );
    delete w;

    // string, multiple
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::String, true, true );
    QgsProcessingFieldWidgetWrapper wrapper4a( &param, type );
    w = wrapper4a.createWrappedWidget( context );
    wrapper4a.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper4a.mPanel->fields().count(), 1 );
        QCOMPARE( wrapper4a.mPanel->fields().at( 0 ).name(), QStringLiteral( "bbb" ) );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    delete w;

    // numeric fields
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Numeric, false, true );
    QgsProcessingFieldWidgetWrapper wrapper5( &param, type );
    w = wrapper5.createWrappedWidget( context );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsFieldComboBox * >( wrapper5.wrappedWidget() )->filters(), QgsFieldProxyModel::Numeric );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    f2 = wrapper5.filterFields( f );
    QCOMPARE( f2.size(), 2 );
    QCOMPARE( f2.at( 0 ).name(), QStringLiteral( "double" ) );
    QCOMPARE( f2.at( 1 ).name(), QStringLiteral( "int" ) );

    delete w;

    // numeric, multiple
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Numeric, true, true );
    QgsProcessingFieldWidgetWrapper wrapper5a( &param, type );
    w = wrapper5a.createWrappedWidget( context );
    wrapper5a.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper5a.mPanel->fields().count(), 1 );
        QCOMPARE( wrapper5a.mPanel->fields().at( 0 ).name(), QStringLiteral( "aaa" ) );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    delete w;

    // datetime fields
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::DateTime, false, true );
    QgsProcessingFieldWidgetWrapper wrapper6( &param, type );
    w = wrapper6.createWrappedWidget( context );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsFieldComboBox * >( wrapper6.wrappedWidget() )->filters(), QgsFieldProxyModel::Date | QgsFieldProxyModel::Time | QgsFieldProxyModel::DateTime );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    f2 = wrapper6.filterFields( f );
    QCOMPARE( f2.size(), 3 );
    QCOMPARE( f2.at( 0 ).name(), QStringLiteral( "date" ) );
    QCOMPARE( f2.at( 1 ).name(), QStringLiteral( "time" ) );
    QCOMPARE( f2.at( 2 ).name(), QStringLiteral( "datetime" ) );


    // default to all fields
    param = QgsProcessingParameterField( QStringLiteral( "field" ), QStringLiteral( "field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true );
    param.setDefaultToAllFields( true );
    QgsProcessingFieldWidgetWrapper wrapper7( &param, type );
    w = wrapper7.createWrappedWidget( context );
    wrapper7.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << QStringLiteral( "aaa" ) << QStringLiteral( "bbb" ) );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    delete w;

    // MultipleLayers as parent layer
    QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "LineString?field=bbb:string" ), QStringLiteral( "y" ), QStringLiteral( "memory" ) );
    p.addMapLayer( vl2 );

    QgsProcessingFieldWidgetWrapper wrapper8( &param, type );
    wrapper8.registerProcessingContextGenerator( &generator );
    w = wrapper8.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariantList() << vl->id() << vl2->id(), context );
    wrapper8.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << QStringLiteral( "bbb" ) );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "field" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !def->defaultValue().isValid() );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterField fieldParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "field_name" ), QStringLiteral( "parent" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "field" ), context, widgetContext, &fieldParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->defaultValue().toString(), QStringLiteral( "field_name" ) );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->parentLayerParameterName(), QStringLiteral( "parent" ) );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->dataType(), QgsProcessingParameterField::Any );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->allowMultiple(), false );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->defaultToAllFields(), false );
  fieldParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  fieldParam.setParentLayerParameterName( QString() );
  fieldParam.setAllowMultiple( true );
  fieldParam.setDefaultToAllFields( true );
  fieldParam.setDataType( QgsProcessingParameterField::String );
  fieldParam.setDefaultValue( QStringLiteral( "field_1;field_2" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "field" ), context, widgetContext, &fieldParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->defaultValue().toString(), QStringLiteral( "field_1;field_2" ) );
  QVERIFY( static_cast< QgsProcessingParameterBand * >( def.get() )->parentLayerParameterName().isEmpty() );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->dataType(), QgsProcessingParameterField::String );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->allowMultiple(), true );
  QCOMPARE( static_cast< QgsProcessingParameterField * >( def.get() )->defaultToAllFields(), true );
}

void TestProcessingGui::testMultipleSelectionDialog()
{
  QVariantList availableOptions;
  QVariantList selectedOptions;
  std::unique_ptr< QgsProcessingMultipleSelectionPanelWidget > dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, selectedOptions );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );

  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  availableOptions << QVariant( "aa" ) << QVariant( 15 ) << QVariant::fromValue( vl.get() );
  dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, selectedOptions );
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
  dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->selectedOptions(), selectedOptions );
  dlg->mModel->item( 1 )->setCheckState( Qt::Unchecked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "bb" );

  // mix of standard and additional options
  availableOptions << QVariant( 6.6 ) << QVariant( "aa" );
  dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->rowCount(), 3 );
  QCOMPARE( dlg->selectedOptions(), selectedOptions ); // order must be maintained!
  dlg->mModel->item( 2 )->setCheckState( Qt::Checked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "bb" << QVariant( 6.6 ) << QVariant( "aa" ) );

  // selection buttons
  selectedOptions.clear();
  availableOptions = QVariantList() << QVariant( "a" ) << QVariant( "b" ) << QVariant( "c" );
  dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, selectedOptions );
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
  dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->item( 0 )->text(), QStringLiteral( "a" ) );
  QCOMPARE( dlg->mModel->item( 1 )->text(), QStringLiteral( "6" ) );
  QCOMPARE( dlg->mModel->item( 2 )->text(), QStringLiteral( "6.2" ) );
  dlg->setValueFormatter( []( const QVariant & v )-> QString
  {
    return v.toString() + '_';
  } );
  QCOMPARE( dlg->mModel->item( 0 )->text(), QStringLiteral( "a_" ) );
  QCOMPARE( dlg->mModel->item( 1 )->text(), QStringLiteral( "6_" ) );
  QCOMPARE( dlg->mModel->item( 2 )->text(), QStringLiteral( "6.2_" ) );

  // mix of fixed + model choices
  availableOptions = QVariantList() << QVariant( "a" ) << 6 << 6.2
                     << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) )
                     << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "input" ) ) );
  dlg = std::make_unique< QgsProcessingMultipleSelectionPanelWidget >( availableOptions, QVariantList() << 6
        << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) )
        << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "input" ) ) ) );

  // when any selected option is a model child parameter source, then we require that all options are upgraded in place to model child parameter sources
  QVariantList res = dlg->selectedOptions();
  QCOMPARE( res.size(), 3 );
  QCOMPARE( res.at( 0 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromStaticValue( 6 ) );
  QCOMPARE( res.at( 1 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) );
  QCOMPARE( res.at( 2 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "input" ) ) );
  dlg->selectAll( true );
  res = dlg->selectedOptions();
  QCOMPARE( res.size(), 5 );
  QCOMPARE( res.at( 0 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromStaticValue( 6 ) );
  QCOMPARE( res.at( 1 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg" ), QStringLiteral( "out" ) ) );
  QCOMPARE( res.at( 2 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "input" ) ) );
  QCOMPARE( res.at( 3 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromStaticValue( QStringLiteral( "a" ) ) );
  QCOMPARE( res.at( 4 ).value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromStaticValue( 6.2 ) );
}

void TestProcessingGui::testMultipleFileSelectionDialog()
{
  std::unique_ptr< QgsProcessingParameterMultipleLayers > param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeRaster );
  QVariantList selectedOptions;
  std::unique_ptr< QgsProcessingMultipleInputPanelWidget > dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), selectedOptions, QList<QgsProcessingModelChildParameterSource >() );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );

  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "point" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "line" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "polygon" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( QStringLiteral( "None" ), QStringLiteral( "nogeom" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( noGeom );
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", QStringLiteral( "mesh" ), QStringLiteral( "mdal" ) );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  QgsProject::instance()->addMapLayer( mesh );
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", QStringLiteral( "raster" ) );
  QgsProject::instance()->addMapLayer( raster );
  DummyPluginLayer *plugin = new DummyPluginLayer( "dummylayer", "plugin" );
  QgsProject::instance()->addMapLayer( plugin );

#ifdef HAVE_EPT
  QgsPointCloudLayer *pointCloud = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", QStringLiteral( "pointcloud" ), QStringLiteral( "ept" ) );
  QgsProject::instance()->addMapLayer( pointCloud );
#endif

  QgsAnnotationLayer *annotationLayer = new QgsAnnotationLayer( QStringLiteral( "secondary annotations" ), QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QgsProject::instance()->addMapLayer( annotationLayer );

  dlg->setProject( QgsProject::instance() );
  // should be filtered to raster layers only
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "raster [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->id() );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  // existing value using layer id should match to project layer
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList() << raster->id(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "raster [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->id() );
  QCOMPARE( dlg->selectedOptions().size(), 1 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), raster->id() );
  // existing value using layer source should also match to project layer
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList() << raster->source(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "raster [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->source() );
  QCOMPARE( dlg->selectedOptions().size(), 1 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), raster->source() );
  // existing value using full layer path not matching a project layer should work
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList() << raster->source() << QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "raster [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), raster->source() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->selectedOptions().size(), 2 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), raster->source() );
  QCOMPARE( dlg->selectedOptions().at( 1 ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );

  // should remember layer order
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList()  << QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) << raster->source(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), QStringLiteral( "raster [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), raster->source() );
  QCOMPARE( dlg->selectedOptions().size(), 2 );
  QCOMPARE( dlg->selectedOptions().at( 0 ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" ) );
  QCOMPARE( dlg->selectedOptions().at( 1 ).toString(), raster->source() );

  // mesh
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeMesh );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "mesh" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), mesh->id() );

  // plugin
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypePlugin );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "plugin" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), plugin->id() );

#ifdef HAVE_EPT
  // point cloud
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypePointCloud );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "pointcloud [EPSG:28356]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), pointCloud->id() );
#endif

  // annotation
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeAnnotation );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "secondary annotations" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), annotationLayer->id() );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ) ).toString(), QStringLiteral( "Annotations" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 1, 0 ), Qt::UserRole ).toString(), QStringLiteral( "main" ) );

  // vector points
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeVectorPoint );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "point [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), point->id() );

  // vector lines
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeVectorLine );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "line [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), line->id() );

  // vector polygons
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeVectorPolygon );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 1 );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ) ).toString(), QStringLiteral( "polygon [EPSG:4326]" ) );
  QCOMPARE( dlg->mModel->data( dlg->mModel->index( 0, 0 ), Qt::UserRole ).toString(), polygon->id() );

  // vector any type
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeVector );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 4 );
  QSet< QString > titles;
  for ( int i = 0; i < dlg->mModel->rowCount(); ++i )
    titles << dlg->mModel->data( dlg->mModel->index( i, 0 ) ).toString();
  QCOMPARE( titles, QSet<QString>() << QStringLiteral( "polygon [EPSG:4326]" ) << QStringLiteral( "point [EPSG:4326]" ) << QStringLiteral( "line [EPSG:4326]" ) << QStringLiteral( "nogeom" ) );

  // any type
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeMapLayer );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
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
  QCOMPARE( titles, QSet<QString>() << QStringLiteral( "polygon [EPSG:4326]" ) << QStringLiteral( "point [EPSG:4326]" ) << QStringLiteral( "line [EPSG:4326]" )
            << QStringLiteral( "nogeom" ) << QStringLiteral( "raster [EPSG:4326]" ) << QStringLiteral( "mesh" ) << QStringLiteral( "plugin" )
            << QStringLiteral( "pointcloud [EPSG:28356]" ) << QStringLiteral( "secondary annotations" ) << QStringLiteral( "Annotations" ) );
#else
  QCOMPARE( titles, QSet<QString>() << QStringLiteral( "polygon [EPSG:4326]" ) << QStringLiteral( "point [EPSG:4326]" ) << QStringLiteral( "line [EPSG:4326]" )
            << QStringLiteral( "nogeom" ) << QStringLiteral( "raster [EPSG:4326]" ) << QStringLiteral( "mesh" ) << QStringLiteral( "plugin" )
            << QStringLiteral( "secondary annotations" ) << QStringLiteral( "Annotations" ) );
#endif

  // files
  param = std::make_unique< QgsProcessingParameterMultipleLayers >( QString(), QString(), QgsProcessing::TypeFile );
  dlg = std::make_unique< QgsProcessingMultipleInputPanelWidget >( param.get(), QVariantList(), QList<QgsProcessingModelChildParameterSource >() );
  dlg->setProject( QgsProject::instance() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );
}

void TestProcessingGui::testRasterBandSelectionPanel()
{
  QgsProcessingParameterBand bandParam( QString(), QString(), QVariant(), QStringLiteral( "INPUT" ), false, true );
  QgsProcessingRasterBandPanelWidget w( nullptr, &bandParam );
  QSignalSpy spy( &w, &QgsProcessingRasterBandPanelWidget::changed );

  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 band(s) selected" ) );
  w.setValue( QStringLiteral( "1" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.value().toList(), QVariantList() << QStringLiteral( "1" ) );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "1 band(s) selected" ) );

  w.setValue( QVariantList() << QStringLiteral( "2" ) << QStringLiteral( "1" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << QStringLiteral( "2" ) << QStringLiteral( "1" ) );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "2 band(s) selected" ) );

  w.setValue( QVariantList() << 3 << 5 << 1 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() << 3 << 5 << 1 );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "3 band(s) selected" ) );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 band(s) selected" ) );
}

void TestProcessingGui::testBandWrapper()
{
  const QgsProcessingAlgorithm *statsAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:rasterlayerstatistics" ) );
  const QgsProcessingParameterDefinition *layerDef = statsAlg->parameterDefinition( QStringLiteral( "INPUT" ) );

  auto testWrapper = [layerDef]( QgsProcessingGui::WidgetType type )
  {
    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsRasterLayer *rl = new QgsRasterLayer( TEST_DATA_DIR + QStringLiteral( "/landsat.tif" ), QStringLiteral( "x" ), QStringLiteral( "gdal" ) );
    p.addMapLayer( rl );

    QgsProcessingParameterBand param( QStringLiteral( "band" ), QStringLiteral( "band" ), QVariant(), QStringLiteral( "INPUT" ) );

    QgsProcessingBandWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void )w;
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper.setParentLayerWrapperValue( &layerWrapper );

    QSignalSpy spy( &wrapper, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 3, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toInt(), 3 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsRasterBandComboBox * >( wrapper.wrappedWidget() )->currentBand(), 3 );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(),  QStringLiteral( "3" ) );
        break;
    }

    wrapper.setWidgetValue( QStringLiteral( "1" ), context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toInt(), 1 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsRasterBandComboBox * >( wrapper.wrappedWidget() )->currentBand(), 1 );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(),  QStringLiteral( "1" ) );
        break;
    }

    delete w;

    // optional
    param = QgsProcessingParameterBand( QStringLiteral( "band" ), QStringLiteral( "band" ), QVariant(), QStringLiteral( "INPUT" ), true, false );

    QgsProcessingBandWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy2( &wrapper2, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QStringLiteral( "4" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toInt(),  4 );

    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( !wrapper2.widgetValue().isValid() );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsRasterBandComboBox * >( wrapper2.wrappedWidget() )->currentBand(), -1 );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(),  QString() );
        break;
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "band [optional]" ) );
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
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        static_cast< QgsRasterBandComboBox * >( wrapper2.wrappedWidget() )->setBand( 6 );
        break;

      case QgsProcessingGui::Modeler:
        static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->setText( QStringLiteral( "6" ) );
        break;
    }

    QCOMPARE( spy2.count(), 3 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), rl );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }

    // should not be owned by wrapper
    QVERIFY( !wrapper2.mParentLayer.get() );
    layerWrapper.setWidgetValue( QVariant(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }

    layerWrapper.setWidgetValue( rl->id(), context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QVERIFY( !wrapper2.mComboBox->layer() );
        break;

      case QgsProcessingGui::Modeler:
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
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper2.mComboBox->layer(), rl );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }
    QVERIFY( !wrapper2.mParentLayer.get() );

    // non-project layer
    QString rasterFileName = TEST_DATA_DIR + QStringLiteral( "/landsat-f32-b1.tif" );
    layerWrapper.setWidgetValue( rasterFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( wrapper2.mComboBox->layer()->publicSource(), rasterFileName );
        break;

      case QgsProcessingGui::Modeler:
        break;
    }

    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), rasterFileName );

    delete w;

    // multiple
    param = QgsProcessingParameterBand( QStringLiteral( "band" ), QStringLiteral( "band" ), QVariant(), QStringLiteral( "INPUT" ), true, true );

    QgsProcessingBandWidgetWrapper wrapper3( &param, type );

    w = wrapper3.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper3.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy3( &wrapper3, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "5" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 5 );

    wrapper3.setWidgetValue( QString(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( wrapper3.widgetValue().toString().isEmpty() );

    wrapper3.setWidgetValue( QStringLiteral( "3;4" ), context );
    QCOMPARE( spy3.count(), 3 );
    QCOMPARE( wrapper3.widgetValue().toStringList(), QStringList() << QStringLiteral( "3" ) << QStringLiteral( "4" ) );

    wrapper3.setWidgetValue( QVariantList() << 5 << 6 << 7, context );
    QCOMPARE( spy3.count(), 4 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 5 << 6 << 7 );

    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 5 );
    QVERIFY( !wrapper3.widgetValue().isValid() );


    // multiple non-optional
    param = QgsProcessingParameterBand( QStringLiteral( "band" ), QStringLiteral( "band" ), QVariant(), QStringLiteral( "INPUT" ), false, true );

    QgsProcessingBandWidgetWrapper wrapper4( &param, type );

    w = wrapper4.createWrappedWidget( context );
    layerWrapper.setWidgetValue( QVariant::fromValue( rl ), context );
    wrapper4.setParentLayerWrapperValue( &layerWrapper );
    QSignalSpy spy4( &wrapper4, &QgsProcessingBandWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( QStringLiteral( "5" ), context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 5 );

    wrapper4.setWidgetValue( QString(), context );
    QCOMPARE( spy4.count(), 2 );
    QVERIFY( wrapper4.widgetValue().toString().isEmpty() );

    wrapper4.setWidgetValue( QStringLiteral( "3;4" ), context );
    QCOMPARE( spy4.count(), 3 );
    QCOMPARE( wrapper4.widgetValue().toStringList(), QStringList() << QStringLiteral( "3" ) << QStringLiteral( "4" ) );

    wrapper4.setWidgetValue( QVariantList() << 5 << 6 << 7, context );
    QCOMPARE( spy4.count(), 4 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 5 << 6 << 7 );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "band" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterBand bandParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), 1, QStringLiteral( "parent" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "band" ), context, widgetContext, &bandParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterBand * >( def.get() )->defaultValue().toString(), QStringLiteral( "1" ) );
  QCOMPARE( static_cast< QgsProcessingParameterBand * >( def.get() )->allowMultiple(), false );
  QCOMPARE( static_cast< QgsProcessingParameterBand * >( def.get() )->parentLayerParameterName(), QStringLiteral( "parent" ) );
  bandParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  bandParam.setParentLayerParameterName( QString() );
  bandParam.setAllowMultiple( true );
  bandParam.setDefaultValue( QVariantList() << 2 << 3 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "band" ), context, widgetContext, &bandParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterBand * >( def.get() )->defaultValue().toStringList(), QStringList() << "2" << "3" );
  QCOMPARE( static_cast< QgsProcessingParameterBand * >( def.get() )->allowMultiple(), true );
  QVERIFY( static_cast< QgsProcessingParameterBand * >( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testMultipleInputWrapper()
{
  QString path1 = TEST_DATA_DIR + QStringLiteral( "/landsat-f32-b1.tif" );
  QString path2 = TEST_DATA_DIR + QStringLiteral( "/landsat.tif" );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterMultipleLayers param( QStringLiteral( "multi" ), QStringLiteral( "multi" ), QgsProcessing::TypeVector, QVariant(), false );

    QgsProcessingMultipleLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;

    QWidget *w = wrapper.createWrappedWidget( context );
    ( void )w;

    QSignalSpy spy( &wrapper, &QgsProcessingMultipleLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << path1 << path2, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList(), QVariantList() << path1 << path2 );
    QCOMPARE( static_cast< QgsProcessingMultipleLayerPanelWidget * >( wrapper.wrappedWidget() )->value().toList(), QVariantList() << path1 << path2 );

    wrapper.setWidgetValue( path1, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toStringList(), QStringList() << path1 );
    QCOMPARE( static_cast< QgsProcessingMultipleLayerPanelWidget * >( wrapper.wrappedWidget() )->value().toList(), QVariantList() << path1 );
    delete w;

    // optional
    param = QgsProcessingParameterMultipleLayers( QStringLiteral( "multi" ), QStringLiteral( "multi" ), QgsProcessing::TypeVector, QVariant(), true );

    QgsProcessingMultipleLayerWidgetWrapper wrapper2( &param, type );

    w = wrapper2.createWrappedWidget( context );
    QSignalSpy spy2( &wrapper2, &QgsProcessingMultipleLayerWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( path2, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList(), QVariantList() << path2 );

    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( static_cast< QgsProcessingMultipleLayerPanelWidget * >( wrapper2.wrappedWidget() )->value().toList().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "multi [optional]" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsProcessingMultipleLayerPanelWidget * >( wrapper2.wrappedWidget() )->setValue( QVariantList() << path1 );
    QCOMPARE( spy2.count(), 3 );


    if ( wrapper.type() == QgsProcessingGui::Modeler )
    {
      // different mix of sources

      wrapper2.setWidgetValue( QVariantList()
                               << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) )
                               << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) )
                               << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromStaticValue( QStringLiteral( "something" ) ) ), context ) ;
      QCOMPARE( wrapper2.widgetValue().toList().count(), 3 );

      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ChildOutput );
      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ChildOutput );
      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().outputChildId(), QStringLiteral( "alg3" ) );
      QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).value< QgsProcessingModelChildParameterSource>().outputName(), QStringLiteral( "OUTPUT" ) );
      QCOMPARE( wrapper2.widgetValue().toList().at( 1 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::ModelParameter );
      QCOMPARE( wrapper2.widgetValue().toList().at( 1 ).value< QgsProcessingModelChildParameterSource>().parameterName(), QStringLiteral( "p1" ) );
      QCOMPARE( wrapper2.widgetValue().toList().at( 2 ).value< QgsProcessingModelChildParameterSource>().source(), QgsProcessingModelChildParameterSource::StaticValue );
      QCOMPARE( wrapper2.widgetValue().toList().at( 2 ).value< QgsProcessingModelChildParameterSource>().staticValue().toString(), QStringLiteral( "something" ) );
      delete w;
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "multilayer" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterMultipleLayers layersParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "multilayer" ), context, widgetContext, &layersParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterMultipleLayers * >( def.get() )->layerType(), QgsProcessing::TypeVectorAnyGeometry );
  layersParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  layersParam.setLayerType( QgsProcessing::TypeRaster );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "multilayer" ), context, widgetContext, &layersParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterMultipleLayers * >( def.get() )->layerType(), QgsProcessing::TypeRaster );
}

void TestProcessingGui::testEnumSelectionPanel()
{
  QgsProcessingParameterEnum enumParam( QString(), QString(), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true );
  QgsProcessingEnumPanelWidget w( nullptr, &enumParam );
  QSignalSpy spy( &w, &QgsProcessingEnumPanelWidget::changed );

  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 options selected" ) );
  w.setValue( 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w.value().toList(), QVariantList() << 1 );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "b" ) );

  w.setValue( QVariantList() << 2 << 0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << 2 << 0 );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "c,a" ) );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 options selected" ) );

  // static strings
  QgsProcessingParameterEnum enumParam2( QString(), QString(), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), false, true );
  QgsProcessingEnumPanelWidget w2( nullptr, &enumParam2 );
  QSignalSpy spy2( &w2, &QgsProcessingEnumPanelWidget::changed );

  QCOMPARE( w2.mLineEdit->text(), QStringLiteral( "0 options selected" ) );
  w2.setValue( QStringLiteral( "a" ) );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( w2.value().toList(), QVariantList() << QStringLiteral( "a" ) );
  QCOMPARE( w2.mLineEdit->text(), QStringLiteral( "a" ) );

  w2.setValue( QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "a" ) );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( w2.value().toList(), QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "a" ) );
  QCOMPARE( w2.mLineEdit->text(), QStringLiteral( "c,a" ) );

  w2.setValue( QVariant() );
  QCOMPARE( spy2.count(), 3 );
  QCOMPARE( w2.value().toList(), QVariantList() );
  QCOMPARE( w2.mLineEdit->text(), QStringLiteral( "0 options selected" ) );
}

void TestProcessingGui::testEnumCheckboxPanel()
{
  //single value
  QgsProcessingParameterEnum param( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), false );
  QgsProcessingEnumCheckboxPanelWidget panel( nullptr, &param );
  QSignalSpy spy( &panel, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel.value(), QVariant() );
  panel.setValue( 2 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( panel.value().toInt(), 2 );
  QVERIFY( !panel.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel.mButtons[ 2 ]->isChecked() );
  panel.setValue( 0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( panel.value().toInt(), 0 );
  QVERIFY( panel.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel.mButtons[ 2 ]->isChecked() );
  panel.mButtons[1]->setChecked( true );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( panel.value().toInt(), 1 );
  panel.setValue( QVariantList() << 2 );
  QCOMPARE( spy.count(), 5 );
  QCOMPARE( panel.value().toInt(), 2 );
  QVERIFY( !panel.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel.mButtons[ 2 ]->isChecked() );

  // multiple value
  QgsProcessingParameterEnum param2( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true );
  QgsProcessingEnumCheckboxPanelWidget panel2( nullptr, &param2 );
  QSignalSpy spy2( &panel2, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel2.value().toList(), QVariantList() );
  panel2.setValue( 2 );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 2 );
  QVERIFY( !panel2.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel2.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel2.mButtons[ 2 ]->isChecked() );
  panel2.setValue( QVariantList() << 0 << 1 );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 0 << 1 );
  QVERIFY( panel2.mButtons[ 0 ]->isChecked() );
  QVERIFY( panel2.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel2.mButtons[ 2 ]->isChecked() );
  panel2.mButtons[0]->setChecked( false );
  QCOMPARE( spy2.count(), 3 );
  QCOMPARE( panel2.value().toList(), QVariantList()  << 1 );
  panel2.mButtons[2]->setChecked( true );
  QCOMPARE( spy2.count(), 4 );
  QCOMPARE( panel2.value().toList(), QVariantList()  << 1 << 2 );
  panel2.deselectAll();
  QCOMPARE( spy2.count(), 5 );
  QCOMPARE( panel2.value().toList(), QVariantList() );
  panel2.selectAll();
  QCOMPARE( spy2.count(), 6 );
  QCOMPARE( panel2.value().toList(), QVariantList() << 0 << 1 << 2 );

  // multiple value optional
  QgsProcessingParameterEnum param3( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), true );
  QgsProcessingEnumCheckboxPanelWidget panel3( nullptr, &param3 );
  QSignalSpy spy3( &panel3, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel3.value().toList(), QVariantList() );
  panel3.setValue( 2 );
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 2 );
  QVERIFY( !panel3.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel3.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel3.mButtons[ 2 ]->isChecked() );
  panel3.setValue( QVariantList() << 0 << 1 );
  QCOMPARE( spy3.count(), 2 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 0 << 1 );
  QVERIFY( panel3.mButtons[ 0 ]->isChecked() );
  QVERIFY( panel3.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel3.mButtons[ 2 ]->isChecked() );
  panel3.mButtons[0]->setChecked( false );
  QCOMPARE( spy3.count(), 3 );
  QCOMPARE( panel3.value().toList(), QVariantList()  << 1 );
  panel3.mButtons[2]->setChecked( true );
  QCOMPARE( spy3.count(), 4 );
  QCOMPARE( panel3.value().toList(), QVariantList()  << 1 << 2 );
  panel3.deselectAll();
  QCOMPARE( spy3.count(), 5 );
  QCOMPARE( panel3.value().toList(), QVariantList() );
  panel3.selectAll();
  QCOMPARE( spy3.count(), 6 );
  QCOMPARE( panel3.value().toList(), QVariantList() << 0 << 1 << 2 );
  panel3.setValue( QVariantList() );
  QCOMPARE( panel3.value().toList(), QVariantList() );
  QVERIFY( !panel3.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel3.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel3.mButtons[ 2 ]->isChecked() );
  QCOMPARE( spy3.count(), 7 );
  panel3.selectAll();
  QCOMPARE( spy3.count(), 8 );
  panel3.setValue( QVariant() );
  QCOMPARE( panel3.value().toList(), QVariantList() );
  QVERIFY( !panel3.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel3.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel3.mButtons[ 2 ]->isChecked() );
  QCOMPARE( spy3.count(), 9 );

  //single value using static strings
  QgsProcessingParameterEnum param4( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), false, QVariant(), false, true );
  QgsProcessingEnumCheckboxPanelWidget panel4( nullptr, &param4 );
  QSignalSpy spy4( &panel4, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel4.value(), QVariant() );
  panel4.setValue( QStringLiteral( "c" ) );
  QCOMPARE( spy4.count(), 1 );
  QCOMPARE( panel4.value().toString(), QStringLiteral( "c" ) );
  QVERIFY( !panel4.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel4.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel4.mButtons[ 2 ]->isChecked() );
  panel4.setValue( QStringLiteral( "a" ) );
  QCOMPARE( spy4.count(), 2 );
  QCOMPARE( panel4.value().toString(), QStringLiteral( "a" ) );
  QVERIFY( panel4.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel4.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel4.mButtons[ 2 ]->isChecked() );
  panel4.mButtons[1]->setChecked( true );
  QCOMPARE( spy4.count(), 4 );
  QCOMPARE( panel4.value().toString(), QStringLiteral( "b" ) );
  panel4.setValue( QVariantList() << QStringLiteral( "c" ) );
  QCOMPARE( spy4.count(), 5 );
  QCOMPARE( panel4.value().toString(), QStringLiteral( "c" ) );
  QVERIFY( !panel4.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel4.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel4.mButtons[ 2 ]->isChecked() );

  // multiple value with static strings
  QgsProcessingParameterEnum param5( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), false, true );
  QgsProcessingEnumCheckboxPanelWidget panel5( nullptr, &param5 );
  QSignalSpy spy5( &panel5, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel5.value().toList(), QVariantList() );
  panel5.setValue( QStringLiteral( "c" ) );
  QCOMPARE( spy5.count(), 1 );
  QCOMPARE( panel5.value().toList(), QVariantList() << QStringLiteral( "c" ) );
  QVERIFY( !panel5.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel5.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel5.mButtons[ 2 ]->isChecked() );
  panel5.setValue( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QCOMPARE( spy5.count(), 2 );
  QCOMPARE( panel5.value().toList(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QVERIFY( panel5.mButtons[ 0 ]->isChecked() );
  QVERIFY( panel5.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel5.mButtons[ 2 ]->isChecked() );
  panel5.mButtons[0]->setChecked( false );
  QCOMPARE( spy5.count(), 3 );
  QCOMPARE( panel5.value().toList(), QVariantList()  << QStringLiteral( "b" ) );
  panel5.mButtons[2]->setChecked( true );
  QCOMPARE( spy5.count(), 4 );
  QCOMPARE( panel5.value().toList(), QVariantList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
  panel5.deselectAll();
  QCOMPARE( spy5.count(), 5 );
  QCOMPARE( panel5.value().toList(), QVariantList() );
  panel5.selectAll();
  QCOMPARE( spy5.count(), 6 );
  QCOMPARE( panel5.value().toList(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) );

  // multiple value optional with statis strings
  QgsProcessingParameterEnum param6( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), true, true );
  QgsProcessingEnumCheckboxPanelWidget panel6( nullptr, &param6 );
  QSignalSpy spy6( &panel6, &QgsProcessingEnumCheckboxPanelWidget::changed );

  QCOMPARE( panel6.value().toList(), QVariantList() );
  panel6.setValue( QStringLiteral( "c" ) );
  QCOMPARE( spy6.count(), 1 );
  QCOMPARE( panel6.value().toList(), QVariantList() << QStringLiteral( "c" ) );
  QVERIFY( !panel6.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel6.mButtons[ 1 ]->isChecked() );
  QVERIFY( panel6.mButtons[ 2 ]->isChecked() );
  panel6.setValue( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QCOMPARE( spy6.count(), 2 );
  QCOMPARE( panel6.value().toList(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QVERIFY( panel6.mButtons[ 0 ]->isChecked() );
  QVERIFY( panel6.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel6.mButtons[ 2 ]->isChecked() );
  panel6.mButtons[0]->setChecked( false );
  QCOMPARE( spy6.count(), 3 );
  QCOMPARE( panel6.value().toList(), QVariantList() << QStringLiteral( "b" ) );
  panel6.mButtons[2]->setChecked( true );
  QCOMPARE( spy6.count(), 4 );
  QCOMPARE( panel6.value().toList(), QVariantList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
  panel6.deselectAll();
  QCOMPARE( spy6.count(), 5 );
  QCOMPARE( panel6.value().toList(), QVariantList() );
  panel6.selectAll();
  QCOMPARE( spy6.count(), 6 );
  QCOMPARE( panel6.value().toList(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
  panel6.setValue( QVariantList() );
  QCOMPARE( panel6.value().toList(), QVariantList() );
  QVERIFY( !panel6.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel6.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel6.mButtons[ 2 ]->isChecked() );
  QCOMPARE( spy6.count(), 7 );
  panel6.selectAll();
  QCOMPARE( spy6.count(), 8 );
  panel6.setValue( QVariant() );
  QCOMPARE( panel6.value().toList(), QVariantList() );
  QVERIFY( !panel6.mButtons[ 0 ]->isChecked() );
  QVERIFY( !panel6.mButtons[ 1 ]->isChecked() );
  QVERIFY( !panel6.mButtons[ 2 ]->isChecked() );
  QCOMPARE( spy6.count(), 9 );
}

void TestProcessingGui::testEnumWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type, bool checkboxStyle = false )
  {
    // non optional, single value
    QgsProcessingParameterEnum param( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), false );
    QVariantMap metadata;
    QVariantMap wrapperMetadata;
    wrapperMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
    metadata.insert( QStringLiteral( "widget_wrapper" ), wrapperMetadata );
    if ( checkboxStyle )
      param.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( 1, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toInt(),  1 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "b" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper.wrappedWidget() )->value().toInt(), 1 );
    }
    wrapper.setWidgetValue( 0, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toInt(),  0 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "a" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper.wrappedWidget() )->value().toInt(), 0 );
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "enum" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast< QComboBox * >( wrapper.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper.wrappedWidget() )->setValue( 2 );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // optional
    QgsProcessingParameterEnum param2( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), false, QVariant(), true );
    if ( checkboxStyle )
      param2.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper2( &param2, type );

    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( 1, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toInt(),  1 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentIndex(), 2 );
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "b" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper2.wrappedWidget() )->value().toInt(), 1 );
    }
    wrapper2.setWidgetValue( 0, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toInt(),  0 );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "a" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper2.wrappedWidget() )->value().toInt(), 0 );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    if ( !checkboxStyle )
    {
      QVERIFY( !wrapper2.widgetValue().isValid() );
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "[Not selected]" ) );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast< QComboBox * >( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper2.wrappedWidget() )->setValue( 1 );
    QCOMPARE( spy2.count(), 4 );

    delete w;

    // allow multiple, non optional
    QgsProcessingParameterEnum param3( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), false );
    if ( checkboxStyle )
      param3.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper3( &param3, type );

    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( 1, context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 1 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    wrapper3.setWidgetValue( 0, context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 0 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    wrapper3.setWidgetValue( QVariantList() << 2 << 1, context );
    QCOMPARE( spy3.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 2 << 1 );
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 2 << 1 );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper3.widgetValue().toList(), QVariantList() << 1 << 2 );
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper3.wrappedWidget() )->value().toList(), QVariantList() << 1 << 2 );
    }
    // check signal
    if ( !checkboxStyle )
      static_cast< QgsProcessingEnumPanelWidget * >( wrapper3.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );
    else
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper3.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );

    QCOMPARE( spy3.count(), 4 );

    delete w;

    // allow multiple, optional
    QgsProcessingParameterEnum param4( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), true );
    if ( checkboxStyle )
      param4.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper4( &param4, type );

    w = wrapper4.createWrappedWidget( context );

    QSignalSpy spy4( &wrapper4, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( 1, context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 1 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 1 );
    wrapper4.setWidgetValue( 0, context );
    QCOMPARE( spy4.count(), 2 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 0 );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 0 );
    wrapper4.setWidgetValue( QVariantList() << 2 << 1, context );
    QCOMPARE( spy4.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 2 << 1 );
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 2 << 1 );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() << 1 << 2 );
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() << 1 << 2 );
    }
    wrapper4.setWidgetValue( QVariantList(), context );
    QCOMPARE( spy4.count(), 4 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );

    wrapper4.setWidgetValue( QVariant(), context );
    QCOMPARE( spy4.count(), 5 );
    QCOMPARE( wrapper4.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->value().toList(), QVariantList() );

    // check signal
    if ( !checkboxStyle )
    {
      static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );
      QCOMPARE( spy4.count(), 6 );
      static_cast< QgsProcessingEnumPanelWidget * >( wrapper4.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy4.count(), 7 );
    }
    else
    {
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->setValue( QVariantList() << 0 << 1 );
      QCOMPARE( spy4.count(), 6 );
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper4.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy4.count(), 7 );
    }

    delete w;

    // non optional, single with static strings
    QgsProcessingParameterEnum param5( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), false, QVariant(), false, true );
    if ( checkboxStyle )
      param5.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper5( &param5, type );

    w = wrapper5.createWrappedWidget( context );

    QSignalSpy spy5( &wrapper5, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper5.setWidgetValue( QStringLiteral( "b" ), context );
    QCOMPARE( spy5.count(), 1 );
    QCOMPARE( wrapper5.widgetValue().toString(), QStringLiteral( "b" ) );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper5.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast< QComboBox * >( wrapper5.wrappedWidget() )->currentText(), QStringLiteral( "b" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper5.wrappedWidget() )->value().toString(), QStringLiteral( "b" ) );
    }
    wrapper5.setWidgetValue( QStringLiteral( "a" ), context );
    QCOMPARE( spy5.count(), 2 );
    QCOMPARE( wrapper5.widgetValue().toString(), QStringLiteral( "a" ) );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper5.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast< QComboBox * >( wrapper5.wrappedWidget() )->currentText(), QStringLiteral( "a" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper5.wrappedWidget() )->value().toString(), QStringLiteral( "a" ) );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast< QComboBox * >( wrapper5.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper5.wrappedWidget() )->setValue( QStringLiteral( "c" ) );
    QCOMPARE( spy5.count(), 3 );

    delete w;

    // single, optional with static strings
    QgsProcessingParameterEnum param6( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), false, QVariant(), true, true );
    if ( checkboxStyle )
      param6.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper6( &param6, type );

    w = wrapper6.createWrappedWidget( context );

    QSignalSpy spy6( &wrapper6, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper6.setWidgetValue( QStringLiteral( "b" ), context );
    QCOMPARE( spy6.count(), 1 );
    QCOMPARE( wrapper6.widgetValue().toString(), QStringLiteral( "b" ) );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper6.wrappedWidget() )->currentIndex(), 2 );
      QCOMPARE( static_cast< QComboBox * >( wrapper6.wrappedWidget() )->currentText(), QStringLiteral( "b" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper6.wrappedWidget() )->value().toString(), QStringLiteral( "b" ) );
    }
    wrapper6.setWidgetValue( QStringLiteral( "a" ), context );
    QCOMPARE( spy6.count(), 2 );
    QCOMPARE( wrapper6.widgetValue().toString(), QStringLiteral( "a" ) );
    if ( !checkboxStyle )
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper6.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast< QComboBox * >( wrapper6.wrappedWidget() )->currentText(), QStringLiteral( "a" ) );
    }
    else
    {
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper6.wrappedWidget() )->value().toString(), QStringLiteral( "a" ) );
    }
    wrapper6.setWidgetValue( QVariant(), context );
    QCOMPARE( spy6.count(), 3 );
    if ( !checkboxStyle )
    {
      QVERIFY( !wrapper6.widgetValue().isValid() );
      QCOMPARE( static_cast< QComboBox * >( wrapper6.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast< QComboBox * >( wrapper6.wrappedWidget() )->currentText(), QStringLiteral( "[Not selected]" ) );
    }

    // check signal
    if ( !checkboxStyle )
      static_cast< QComboBox * >( wrapper6.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper6.wrappedWidget() )->setValue( QStringLiteral( "a" ) );
    QCOMPARE( spy6.count(), 4 );

    delete w;

    // multiple, non optional with static strings
    QgsProcessingParameterEnum param7( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), false, true );
    if ( checkboxStyle )
      param7.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper7( &param7, type );

    w = wrapper7.createWrappedWidget( context );

    QSignalSpy spy7( &wrapper7, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper7.setWidgetValue( QStringLiteral( "b" ), context );
    QCOMPARE( spy7.count(), 1 );
    QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << QStringLiteral( "b" ) );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "b" ) );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "b" ) );
    wrapper7.setWidgetValue( QStringLiteral( "a" ), context );
    QCOMPARE( spy7.count(), 2 );
    QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << QStringLiteral( "a" ) );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "a" ) );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "a" ) );
    wrapper7.setWidgetValue( QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "b" ), context );
    QCOMPARE( spy7.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "b" ) );
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "b" ) );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper7.widgetValue().toList(), QVariantList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper7.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
    }
    // check signal
    if ( !checkboxStyle )
      static_cast< QgsProcessingEnumPanelWidget * >( wrapper7.wrappedWidget() )->setValue( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
    else
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper7.wrappedWidget() )->setValue( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );

    QCOMPARE( spy7.count(), 4 );

    delete w;

    // multiple, optional with static strings
    QgsProcessingParameterEnum param8( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), true, true );
    if ( checkboxStyle )
      param8.setMetadata( metadata );

    QgsProcessingEnumWidgetWrapper wrapper8( &param8, type );

    w = wrapper8.createWrappedWidget( context );

    QSignalSpy spy8( &wrapper8, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper8.setWidgetValue( QStringLiteral( "b" ), context );
    QCOMPARE( spy8.count(), 1 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << QStringLiteral( "b" ) );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "b" ) );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "b" ) );
    wrapper8.setWidgetValue( QStringLiteral( "a" ), context );
    QCOMPARE( spy8.count(), 2 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << QStringLiteral( "a" ) );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "a" ) );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "a" ) );
    wrapper8.setWidgetValue( QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "b" ), context );
    QCOMPARE( spy8.count(), 3 );
    if ( !checkboxStyle )
    {
      QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "b" ) );
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "c" ) << QStringLiteral( "b" ) );
    }
    else
    {
      // checkbox style isn't ordered
      QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
    }
    wrapper8.setWidgetValue( QVariantList(), context );
    QCOMPARE( spy8.count(), 4 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );

    wrapper8.setWidgetValue( QVariant(), context );
    QCOMPARE( spy8.count(), 5 );
    QCOMPARE( wrapper8.widgetValue().toList(), QVariantList() );
    if ( !checkboxStyle )
      QCOMPARE( static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );
    else
      QCOMPARE( static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->value().toList(), QVariantList() );

    // check signal
    if ( !checkboxStyle )
    {
      static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->setValue( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
      QCOMPARE( spy8.count(), 6 );
      static_cast< QgsProcessingEnumPanelWidget * >( wrapper8.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy8.count(), 7 );
    }
    else
    {
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->setValue( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
      QCOMPARE( spy8.count(), 6 );
      static_cast< QgsProcessingEnumCheckboxPanelWidget * >( wrapper8.wrappedWidget() )->setValue( QVariant() );
      QCOMPARE( spy8.count(), 7 );
    }

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // checkbox style (not for batch or model mode!)
  testWrapper( QgsProcessingGui::Standard, true );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "enum" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterEnum enumParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringList() << "A" << "B" << "C", false, 2 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "enum" ), context, widgetContext, &enumParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterEnum * >( def.get() )->options(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast< QgsProcessingParameterEnum * >( def.get() )->defaultValue().toStringList(), QStringList() << "2" );
  QVERIFY( !static_cast< QgsProcessingParameterEnum * >( def.get() )->allowMultiple() );
  enumParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  enumParam.setAllowMultiple( true );
  enumParam.setDefaultValue( QVariantList() << 0 << 1 );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "enum" ), context, widgetContext, &enumParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterEnum * >( def.get() )->options(), QStringList() << "A" << "B" << "C" );
  QCOMPARE( static_cast< QgsProcessingParameterEnum * >( def.get() )->defaultValue().toStringList(), QStringList() << "0" << "1" );
  QVERIFY( static_cast< QgsProcessingParameterEnum * >( def.get() )->allowMultiple() );
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

  auto testWrapper = [&p]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterLayout param( QStringLiteral( "layout" ), QStringLiteral( "layout" ), false );

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
    QCOMPARE( wrapper.widgetValue().toString(),  QStringLiteral( "l2" ) );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "l2" ) );
    }
    else
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "l2" ) );
    }
    wrapper.setWidgetValue( "l1", context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toString(),  QStringLiteral( "l1" ) );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper.wrappedWidget() )->currentIndex(), 0 );
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
    }
    else
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "layout" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    if ( type != QgsProcessingGui::Modeler )
    {
      static_cast< QComboBox * >( wrapper.wrappedWidget() )->setCurrentIndex( 1 );
    }
    else
    {
      static_cast< QComboBox * >( wrapper.wrappedWidget() )->setCurrentText( QStringLiteral( "aaaa" ) );
    }
    QCOMPARE( spy.count(), 3 );

    delete w;

    // optional

    QgsProcessingParameterLayout param2( QStringLiteral( "layout" ), QStringLiteral( "layout" ), QVariant(), true );

    QgsProcessingLayoutWidgetWrapper wrapper2( &param2, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "l2", context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "l2" ) );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper2.wrappedWidget() )->currentIndex(), 2 );
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l2" ) );
    }
    else
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l2" ) );
    }
    wrapper2.setWidgetValue( "l1", context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "l1" ) );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper2.wrappedWidget() )->currentIndex(), 1 );
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
    }
    else
    {
      QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( static_cast< QgsLayoutComboBox * >( wrapper2.wrappedWidget() )->currentIndex(), 0 );
      QVERIFY( static_cast< QgsLayoutComboBox * >( wrapper2.wrappedWidget() )->currentText().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText().isEmpty() );
    }

    // check signal
    if ( type != QgsProcessingGui::Modeler )
      static_cast< QComboBox * >( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
    else
      static_cast< QComboBox * >( wrapper2.wrappedWidget() )->setCurrentText( QStringLiteral( "aaa" ) );
    QCOMPARE( spy2.count(), 4 );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

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

  auto testWrapper = [&p, l1, label1, label2]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterLayoutItem param( QStringLiteral( "layout" ), QStringLiteral( "layout" ), false );

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
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), label2->uuid() );
      QCOMPARE( static_cast< QgsLayoutItemComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "b" ) );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "b" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "b" ) );
    }
    wrapper.setWidgetValue( "a", context );
    QCOMPARE( spy.count(), 2 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), label1->uuid() );
      QCOMPARE( static_cast< QgsLayoutItemComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "a" ) );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "a" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "a" ) );
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "layout" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    if ( type != QgsProcessingGui::Modeler )
    {
      static_cast< QComboBox * >( wrapper.wrappedWidget() )->setCurrentIndex( 1 );
    }
    else
    {
      static_cast< QLineEdit * >( wrapper.wrappedWidget() )->setText( QStringLiteral( "aaaa" ) );
    }
    QCOMPARE( spy.count(), 3 );

    delete w;

    // optional

    QgsProcessingParameterLayoutItem param2( QStringLiteral( "layout" ), QStringLiteral( "layout" ), QVariant(), QString(), -1, true );

    QgsProcessingLayoutItemWidgetWrapper wrapper2( &param2, type );
    wrapper2.setWidgetContext( widgetContext );
    w = wrapper2.createWrappedWidget( context );
    wrapper2.setLayout( l1 );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "b", context );
    QCOMPARE( spy2.count(), 1 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper2.widgetValue().toString(), label2->uuid() );
      QCOMPARE( static_cast< QgsLayoutItemComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "b" ) );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "b" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(), QStringLiteral( "b" ) );
    }
    wrapper2.setWidgetValue( "a", context );
    QCOMPARE( spy2.count(), 2 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper2.widgetValue().toString(), label1->uuid() );
      QCOMPARE( static_cast< QgsLayoutItemComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "a" ) );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "a" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(), QStringLiteral( "a" ) );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type != QgsProcessingGui::Modeler )
    {
      QVERIFY( static_cast< QgsLayoutItemComboBox * >( wrapper2.wrappedWidget() )->currentText().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().isEmpty() );
    }

    // check signal
    if ( type != QgsProcessingGui::Modeler )
      static_cast< QgsLayoutItemComboBox * >( wrapper2.wrappedWidget() )->setCurrentIndex( 1 );
    else
      static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->setText( QStringLiteral( "aaa" ) );
    QCOMPARE( spy2.count(), 4 );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layoutitem" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterLayoutItem itemParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QVariant(), QStringLiteral( "parent" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layoutitem" ), context, widgetContext, &itemParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterLayoutItem * >( def.get() )->parentLayoutParameterName(), QStringLiteral( "parent" ) );
  itemParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  itemParam.setParentLayoutParameterName( QString() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layoutitem" ), context, widgetContext, &itemParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( static_cast< QgsProcessingParameterLayoutItem * >( def.get() )->parentLayoutParameterName().isEmpty() );
}

void TestProcessingGui::testPointPanel()
{
  std::unique_ptr< QgsProcessingPointPanel > panel = std::make_unique< QgsProcessingPointPanel >( nullptr );
  QSignalSpy spy( panel.get(), &QgsProcessingPointPanel::changed );

  panel->setValue( QgsPointXY( 100, 150 ), QgsCoordinateReferenceSystem() );
  QCOMPARE( panel->value().toString(), QStringLiteral( "100.000000,150.000000" ) );
  QCOMPARE( spy.count(), 1 );

  panel->setValue( QgsPointXY( 200, 250 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  QCOMPARE( panel->value().toString(), QStringLiteral( "200.000000,250.000000 [EPSG:3111]" ) );
  QCOMPARE( spy.count(), 2 );

  panel->setValue( QgsPointXY( 123456.123456789, 654321.987654321 ), QgsCoordinateReferenceSystem() );
  QCOMPARE( panel->value().toString(), QStringLiteral( "123456.123457,654321.987654" ) );
  QCOMPARE( spy.count(), 3 );

  QVERIFY( !panel->mLineEdit->showClearButton() );
  panel->setAllowNull( true );
  QVERIFY( panel->mLineEdit->showClearButton() );
  panel->clear();
  QVERIFY( !panel->value().isValid() );
  QCOMPARE( spy.count(), 4 );

  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  panel->setMapCanvas( &canvas );
  panel->updatePoint( QgsPointXY( 1.5, -3.5 ) );
  QCOMPARE( panel->value().toString(), QStringLiteral( "1.500000,-3.500000 [EPSG:28356]" ) );
  QCOMPARE( spy.count(), 5 );

  panel.reset();
}


void TestProcessingGui::testPointWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterPoint param( QStringLiteral( "point" ), QStringLiteral( "point" ), false );

    QgsProcessingPointWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( "1,2", context );
    QCOMPARE( spy.count(), 1 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "1.000000,2.000000" ) );
      QCOMPARE( static_cast< QgsProcessingPointPanel * >( wrapper.wrappedWidget() )->mLineEdit->text(), QStringLiteral( "1.000000,2.000000" ) );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "1,2" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "1,2" ) );
    }
    wrapper.setWidgetValue( "1,2 [EPSG:3111]", context );
    QCOMPARE( spy.count(), 2 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "1.000000,2.000000 [EPSG:3111]" ) );
      QCOMPARE( static_cast< QgsProcessingPointPanel * >( wrapper.wrappedWidget() )->mLineEdit->text(), QStringLiteral( "1.000000,2.000000 [EPSG:3111]" ) );
    }
    else
    {
      QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "1,2 [EPSG:3111]" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "1,2 [EPSG:3111]" ) );
    }

    // check signal
    if ( type != QgsProcessingGui::Modeler )
    {
      static_cast< QgsProcessingPointPanel * >( wrapper.wrappedWidget() )->mLineEdit->setText( QStringLiteral( "b" ) );
    }
    else
    {
      static_cast< QLineEdit * >( wrapper.wrappedWidget() )->setText( QStringLiteral( "aaaa" ) );
    }
    QCOMPARE( spy.count(), 3 );


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "point" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    delete w;

    // optional

    QgsProcessingParameterPoint param2( QStringLiteral( "point" ), QStringLiteral( "point" ), QVariant(), true );

    QgsProcessingPointWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "1,2", context );
    QCOMPARE( spy2.count(), 1 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( static_cast< QgsProcessingPointPanel * >( wrapper2.wrappedWidget() )->mLineEdit->text(), QStringLiteral( "1.000000,2.000000" ) );
      QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "1.000000,2.000000" ) );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "1,2" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(), QStringLiteral( "1,2" ) );
    }

    wrapper2.setWidgetValue( "1,2 [EPSG:3111]", context );
    QCOMPARE( spy2.count(), 2 );
    if ( type != QgsProcessingGui::Modeler )
    {
      QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "1.000000,2.000000 [EPSG:3111]" ) );
      QCOMPARE( static_cast< QgsProcessingPointPanel * >( wrapper2.wrappedWidget() )->mLineEdit->text(), QStringLiteral( "1.000000,2.000000 [EPSG:3111]" ) );
    }
    else
    {
      QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "1,2 [EPSG:3111]" ) );
      QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(), QStringLiteral( "1,2 [EPSG:3111]" ) );
    }
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type == QgsProcessingGui::Modeler )
    {
      QVERIFY( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast< QgsProcessingPointPanel * >( wrapper2.wrappedWidget() )->mLineEdit->text().isEmpty() );
    }
    wrapper2.setWidgetValue( "1,3", context );
    QCOMPARE( spy2.count(), 4 );
    wrapper2.setWidgetValue( "", context );
    QCOMPARE( spy2.count(), 5 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    if ( type == QgsProcessingGui::Modeler )
    {
      QVERIFY( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().isEmpty() );
    }
    else
    {
      QVERIFY( static_cast< QgsProcessingPointPanel * >( wrapper2.wrappedWidget() )->mLineEdit->text().isEmpty() );
    }

    // check signals
    wrapper2.setWidgetValue( "1,3", context );
    QCOMPARE( spy2.count(), 6 );
    if ( type == QgsProcessingGui::Modeler )
    {
      static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->clear();
    }
    else
    {
      static_cast< QgsProcessingPointPanel * >( wrapper2.wrappedWidget() )->mLineEdit->clear();
    }
    QCOMPARE( spy2.count(), 7 );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "point" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterPoint pointParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "1,2" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "point" ), context, widgetContext, &pointParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterPoint * >( def.get() )->defaultValue().toString(), QStringLiteral( "1.000000,2.000000" ) );
  pointParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  pointParam.setDefaultValue( QStringLiteral( "4,7" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "point" ), context, widgetContext, &pointParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterPoint * >( def.get() )->defaultValue().toString(), QStringLiteral( "4.000000,7.000000" ) );

}


void TestProcessingGui::testGeometryWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterGeometry param( QStringLiteral( "geometry" ), QStringLiteral( "geometry" ), false );

    QgsProcessingGeometryWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "POINT (1 2)" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString().toLower(), QStringLiteral( "point (1 2)" ) );
    QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text().toLower(), QStringLiteral( "point (1 2)" ).toLower() );
    wrapper.setWidgetValue( QString(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( wrapper.widgetValue().toString().isEmpty() );
    QVERIFY( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text().isEmpty() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "geometry" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QLineEdit * >( wrapper.wrappedWidget() )->setText( QStringLiteral( "b" ) );
    QCOMPARE( spy.count(), 3 );
    static_cast< QLineEdit * >( wrapper.wrappedWidget() )->clear();
    QCOMPARE( spy.count(), 4 );

    delete w;

    // optional

    QgsProcessingParameterGeometry param2( QStringLiteral( "geometry" ), QStringLiteral( "geometry" ), QVariant(), true );

    QgsProcessingGeometryWidgetWrapper wrapper2( &param2, type );

    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingLayoutItemWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "POINT (1 2)", context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString().toLower(), QStringLiteral( "point (1 2)" ) );
    QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().toLower(), QStringLiteral( "point (1 2)" ) );

    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 2 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().isEmpty() );

    wrapper2.setWidgetValue( "POINT (1 3)", context );
    QCOMPARE( spy2.count(), 3 );
    wrapper2.setWidgetValue( "", context );
    QCOMPARE( spy2.count(), 4 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().isEmpty() );

    delete w;
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );


  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );


  // config widget
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "geometry" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterGeometry geometryParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "POINT (1 2)" ) );

  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "geometry" ), context, widgetContext, &geometryParam );

  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterGeometry * >( def.get() )->defaultValue().toString().toLower(), QStringLiteral( "point (1 2)" ) );
  geometryParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  geometryParam.setDefaultValue( QStringLiteral( "POINT (4 7)" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "geometry" ), context, widgetContext, &geometryParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterGeometry * >( def.get() )->defaultValue().toString().toLower(), QStringLiteral( "point (4 7)" ) );

}




void TestProcessingGui::testExtentWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterExtent param( QStringLiteral( "extent" ), QStringLiteral( "extent" ), false );

    QgsProcessingExtentWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingExtentWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( "1,2,3,4", context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "1.000000000,2.000000000,3.000000000,4.000000000" ) );
    QCOMPARE( static_cast< QgsExtentWidget * >( wrapper.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );

    wrapper.setWidgetValue( "1,2,3,4 [EPSG:3111]", context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "1.000000000,2.000000000,3.000000000,4.000000000 [EPSG:3111]" ) );
    QCOMPARE( static_cast< QgsExtentWidget * >( wrapper.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );
    QCOMPARE( static_cast< QgsExtentWidget * >( wrapper.wrappedWidget() )->outputCrs().authid(), QStringLiteral( "EPSG:3111" ) );

    // check signal
    static_cast< QgsExtentWidget * >( wrapper.wrappedWidget() )->setOutputExtentFromUser( QgsRectangle( 11, 22, 33, 44 ), QgsCoordinateReferenceSystem() );
    QCOMPARE( spy.count(), 3 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "extent" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    delete w;

    // optional

    QgsProcessingParameterExtent param2( QStringLiteral( "extent" ), QStringLiteral( "extent" ), QVariant(), true );

    QgsProcessingExtentWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingExtentWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( "1,2,3,4", context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( static_cast< QgsExtentWidget * >( wrapper2.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "1.000000000,2.000000000,3.000000000,4.000000000" ) );

    wrapper2.setWidgetValue( "1,2,3,4 [EPSG:3111]", context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "1.000000000,2.000000000,3.000000000,4.000000000 [EPSG:3111]" ) );
    QCOMPARE( static_cast< QgsExtentWidget * >( wrapper2.wrappedWidget() )->outputExtent(), QgsRectangle( 1, 3, 2, 4 ) );
    QCOMPARE( static_cast< QgsExtentWidget * >( wrapper2.wrappedWidget() )->outputCrs().authid(), QStringLiteral( "EPSG:3111" ) );
    wrapper2.setWidgetValue( QVariant(), context );
    QCOMPARE( spy2.count(), 3 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( !static_cast< QgsExtentWidget * >( wrapper2.wrappedWidget() )->isValid() );

    wrapper2.setWidgetValue( "1,3,4,7", context );
    QCOMPARE( spy2.count(), 4 );
    wrapper2.setWidgetValue( "", context );
    QCOMPARE( spy2.count(), 5 );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    QVERIFY( !static_cast< QgsExtentWidget * >( wrapper2.wrappedWidget() )->isValid() );

    // check signals
    wrapper2.setWidgetValue( "1,3,9,8", context );
    QCOMPARE( spy2.count(), 6 );
    static_cast< QgsExtentWidget * >( wrapper2.wrappedWidget() )->clear();
    QCOMPARE( spy2.count(), 7 );

    delete w;

  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "extent" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterExtent extentParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "1,2,3,4" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "extent" ), context, widgetContext, &extentParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterExtent * >( def.get() )->defaultValue().toString(), QStringLiteral( "1.000000000,2.000000000,3.000000000,4.000000000" ) );
  extentParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  extentParam.setDefaultValue( QStringLiteral( "4,7,8,9" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "extent" ), context, widgetContext, &extentParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterExtent * >( def.get() )->defaultValue().toString(), QStringLiteral( "4.000000000,7.000000000,8.000000000,9.000000000" ) );
}

void TestProcessingGui::testColorWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterColor param( QStringLiteral( "color" ), QStringLiteral( "color" ) );

    QgsProcessingColorWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingColorWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QColor( 255, 0, 0 ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().value< QColor >().name(),  QStringLiteral( "#ff0000" ) );
    QCOMPARE( static_cast< QgsColorButton * >( wrapper.wrappedWidget() )->color(),  QColor( 255, 0, 0 ) );
    QVERIFY( !static_cast< QgsColorButton * >( wrapper.wrappedWidget() )->showNull() );
    QVERIFY( static_cast< QgsColorButton * >( wrapper.wrappedWidget() )->allowOpacity() );
    wrapper.setWidgetValue( QColor(), context );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( !wrapper.widgetValue().value< QColor >().isValid() );
    QVERIFY( !static_cast< QgsColorButton * >( wrapper.wrappedWidget() )->color().isValid() );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "color" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsColorButton * >( wrapper.wrappedWidget() )->setColor( QColor( 0, 255, 0 ) );
    QCOMPARE( spy.count(), 3 );

    // with opacity
    wrapper.setWidgetValue( QColor( 255, 0, 0, 100 ), context );
    QCOMPARE( wrapper.widgetValue().value< QColor >(), QColor( 255, 0, 0, 100 ) );

    delete w;

    // with null
    QgsProcessingParameterColor param2( QStringLiteral( "c2" ), QStringLiteral( "c2" ), QColor( 10, 20, 30 ), true, true );

    QgsProcessingColorWidgetWrapper wrapper2( &param2, type );
    w = wrapper2.createWrappedWidget( context );
    QVERIFY( static_cast< QgsColorButton * >( wrapper2.wrappedWidget() )->showNull() );
    QCOMPARE( static_cast< QgsColorButton * >( wrapper2.wrappedWidget() )->color().name(),  QStringLiteral( "#0a141e" ) );
    wrapper2.setWidgetValue( QVariant(), context );
    QVERIFY( !wrapper2.widgetValue().isValid() );
    wrapper2.setWidgetValue( QColor( 255, 0, 255 ), context );
    QCOMPARE( wrapper2.widgetValue().value< QColor >().name(), QStringLiteral( "#ff00ff" ) );

    // no opacity
    QgsProcessingParameterColor param3( QStringLiteral( "c2" ), QStringLiteral( "c2" ), QColor( 10, 20, 30 ), false, true );

    QgsProcessingColorWidgetWrapper wrapper3( &param3, type );
    w = wrapper3.createWrappedWidget( context );
    wrapper3.setWidgetValue( QColor( 255, 0, 0, 100 ), context );
    QCOMPARE( wrapper3.widgetValue().value< QColor >(), QColor( 255, 0, 0 ) );
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "color" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( static_cast< QgsProcessingParameterColor * >( def.get() )->opacityEnabled() ); // should default to true

  // using a parameter definition as initial values
  QgsProcessingParameterColor colorParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QColor( 255, 0, 0, 100 ), true );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "color" ), context, widgetContext, &colorParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterColor * >( def.get() )->defaultValue().value< QColor >(), QColor( 255, 0, 0, 100 ) );
  QVERIFY( static_cast< QgsProcessingParameterColor * >( def.get() )->opacityEnabled() );
  colorParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  colorParam.setOpacityEnabled( false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "color" ), context, widgetContext, &colorParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterColor * >( def.get() )->defaultValue().value< QColor >(), QColor( 255, 0, 0 ) ); // (no opacity!)
  QVERIFY( !static_cast< QgsProcessingParameterColor * >( def.get() )->opacityEnabled() );
}

void TestProcessingGui::testCoordinateOperationWrapper()
{
  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterCoordinateOperation param( QStringLiteral( "op" ), QStringLiteral( "op" ) );

    QgsProcessingCoordinateOperationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    wrapper.setSourceCrsParameterValue( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26745" ) ) );
    wrapper.setDestinationCrsParameterValue( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );

    QSignalSpy spy( &wrapper, &QgsProcessingCoordinateOperationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ) );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      {
        QCOMPARE( static_cast< QgsCoordinateOperationWidget * >( wrapper.wrappedWidget() )->selectedOperation().proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ) );
        wrapper.setWidgetValue( QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( static_cast< QgsCoordinateOperationWidget * >( wrapper.wrappedWidget() )->selectedOperation().proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ) );

        // check signal
        QgsCoordinateOperationWidget::OperationDetails deets;
        deets.proj = QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" );
        static_cast< QgsCoordinateOperationWidget * >( wrapper.wrappedWidget() )->setSelectedOperation( deets );
        QCOMPARE( spy.count(), 3 );
        break;
      }

      case QgsProcessingGui::Modeler:
      case QgsProcessingGui::Batch:
      {
        QCOMPARE( wrapper.mLineEdit->text(), QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ) );
        wrapper.setWidgetValue( QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.mLineEdit->text(), QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=159 +z=175 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ) );

        // check signal
        wrapper.mLineEdit->setText( QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=us-ft +xy_out=m +step +inv +proj=lcc +lat_0=33.5 +lon_0=-118 +lat_1=35.4666666666667 +lat_2=34.0333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=push +v_3 +step +proj=cart +ellps=clrk66 +step +proj=helmert +x=-8 +y=160 +z=176 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84" ) );
        QCOMPARE( spy.count(), 3 );
        break;
      }
    }

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "op" ) );
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
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "coordinateoperation" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  QVERIFY( !static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->sourceCrs().isValid() ); // should default to not set
  QVERIFY( !static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->destinationCrs().isValid() ); // should default to not set
  QVERIFY( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->sourceCrsParameterName().isEmpty() ); // should default to not set
  QVERIFY( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->destinationCrsParameterName().isEmpty() ); // should default to not set

  // using a parameter definition as initial values
  QgsProcessingParameterCoordinateOperation coordParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "+proj" ), QStringLiteral( "a" ), QStringLiteral( "b" ), QStringLiteral( "EPSG:26745" ), QStringLiteral( "EPSG:4326" ), false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "coordinateoperation" ), context, widgetContext, &coordParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->defaultValue().toString(), QStringLiteral( "+proj" ) );
  QCOMPARE( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->sourceCrsParameterName(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->destinationCrsParameterName(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->sourceCrs().value< QgsCoordinateReferenceSystem >( ).authid(), QStringLiteral( "EPSG:26745" ) );
  QCOMPARE( static_cast< QgsProcessingParameterCoordinateOperation * >( def.get() )->destinationCrs().value< QgsCoordinateReferenceSystem >( ).authid(), QStringLiteral( "EPSG:4326" ) );
  coordParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "coordinateoperation" ), context, widgetContext, &coordParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
}

void TestProcessingGui::mapLayerComboBox()
{
  QgsProject::instance()->removeAllMapLayers();
  QgsProcessingContext context;
  context.setProject( QgsProject::instance() );

  // feature source param
  std::unique_ptr< QgsProcessingParameterDefinition > param( new QgsProcessingParameterFeatureSource( QStringLiteral( "param" ), QString() ) );
  std::unique_ptr< QgsProcessingMapLayerComboBox> combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );

  QSignalSpy spy( combo.get(), &QgsProcessingMapLayerComboBox::valueChanged );
  QVERIFY( !combo->value().isValid() );
  combo->setValue( QStringLiteral( "file path" ), context );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( combo->value().toString(), QStringLiteral( "file path" ) );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 1 );
  combo->setValue( QVariant(), context ); // not possible, it's not an optional param
  QCOMPARE( combo->value().toString(), QStringLiteral( "file path" ) );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 1 );
  combo->setValue( QStringLiteral( "file path 2" ), context );
  QCOMPARE( combo->value().toString(), QStringLiteral( "file path 2" ) );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 2 );
  combo->setValue( QStringLiteral( "file path" ), context );
  QCOMPARE( combo->value().toString(), QStringLiteral( "file path" ) );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 3 );
  combo->setLayer( nullptr ); // not possible, not optional
  QCOMPARE( combo->value().toString(), QStringLiteral( "file path" ) );
  QVERIFY( !combo->currentLayer() );
  QCOMPARE( spy.count(), 3 );

  // project layers
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsFeature f;
  vl->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayer( vl );
  QVERIFY( vl->isValid() );
  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "l2" ), QStringLiteral( "memory" ) );
  vl2->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayer( vl2 );
  QVERIFY( vl2->isValid() );

  QCOMPARE( combo->value().toString(), QStringLiteral( "file path" ) );
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

  combo->setValue( QStringLiteral( "file path" ), context );
  QCOMPARE( combo->value().toString(), QStringLiteral( "file path" ) );
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
  QVERIFY( combo->value().canConvert< QgsProcessingFeatureSourceDefinition >() );
  QCOMPARE( combo->value().value< QgsProcessingFeatureSourceDefinition >().source.staticValue().toString(), vl2->id() );
  QVERIFY( combo->value().value< QgsProcessingFeatureSourceDefinition >().selectedFeaturesOnly );
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
  QVERIFY( combo->value().canConvert< QgsProcessingFeatureSourceDefinition >() );
  QCOMPARE( spy.count(), 10 );
  combo->setValue( QVariant::fromValue( vl ), context );
  QCOMPARE( combo->value().toString(), vl->id() );
  QVERIFY( combo->currentText().startsWith( vl->name() ) );
  QCOMPARE( spy.count(), 11 );

  // one last variation - selection to selection
  combo->setValue( sourceDef, context );
  QCOMPARE( spy.count(), 12 );
  QVERIFY( combo->value().value< QgsProcessingFeatureSourceDefinition >().selectedFeaturesOnly );
  vl->selectAll();
  sourceDef = QgsProcessingFeatureSourceDefinition( vl->id(), true );
  combo->setValue( sourceDef, context );
  // expect "selected only" state to remain
  QVERIFY( combo->value().canConvert< QgsProcessingFeatureSourceDefinition >() );
  QCOMPARE( combo->value().value< QgsProcessingFeatureSourceDefinition >().source.staticValue().toString(), vl->id() );
  QVERIFY( combo->value().value< QgsProcessingFeatureSourceDefinition >().selectedFeaturesOnly );
  QVERIFY( combo->currentText().startsWith( vl->name() ) );
  QCOMPARE( spy.count(), 13 );

  // iterate over features
  QVERIFY( !( combo->value().value< QgsProcessingFeatureSourceDefinition >().flags & QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature ) );
  sourceDef.flags |= QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature;
  combo->setValue( sourceDef, context );
  QVERIFY( combo->value().value< QgsProcessingFeatureSourceDefinition >().flags & QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature );
  sourceDef.flags = QgsProcessingFeatureSourceDefinition::Flags();
  combo->setValue( sourceDef, context );
  QVERIFY( !( combo->value().value< QgsProcessingFeatureSourceDefinition >().flags & QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature ) );

  // advanced settings
  sourceDef.featureLimit = 67;
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().value< QgsProcessingFeatureSourceDefinition >().featureLimit, 67LL );
  sourceDef.featureLimit = -1;
  combo->setValue( sourceDef, context );
  QCOMPARE( combo->value().value< QgsProcessingFeatureSourceDefinition >().featureLimit, -1LL );
  sourceDef.flags |= QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck;
  sourceDef.geometryCheck = QgsFeatureRequest::GeometrySkipInvalid;
  combo->setValue( sourceDef, context );
  QVERIFY( combo->value().value< QgsProcessingFeatureSourceDefinition >().flags & QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck );
  QCOMPARE( combo->value().value< QgsProcessingFeatureSourceDefinition >().geometryCheck, QgsFeatureRequest::GeometrySkipInvalid );
  sourceDef.flags = QgsProcessingFeatureSourceDefinition::Flags();
  combo->setValue( sourceDef, context );
  QVERIFY( !( combo->value().value< QgsProcessingFeatureSourceDefinition >().flags & QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck ) );

  combo.reset();
  param.reset();

  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( QStringLiteral( "None" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( noGeom );
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", QStringLiteral( "Triangle and Quad Mdal" ), QStringLiteral( "mdal" ) );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  QgsProject::instance()->addMapLayer( mesh );
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", QStringLiteral( "band1_byte" ) );
  QgsProject::instance()->addMapLayer( raster );
  QgsPointCloudLayer *pointCloud = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", QStringLiteral( "Point cloud" ), QStringLiteral( "ept" ) );
  QVERIFY( pointCloud->isValid() );
  QgsProject::instance()->addMapLayer( pointCloud );

  // map layer param, all types are acceptable
  param = std::make_unique< QgsProcessingParameterMapLayer> ( QStringLiteral( "param" ), QString() );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // map layer param, only point vector and raster types are acceptable
  param = std::make_unique< QgsProcessingParameterMapLayer> ( QStringLiteral( "param" ), QString(), QVariant(), false, QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeRaster );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // raster layer param, only raster types are acceptable
  param = std::make_unique< QgsProcessingParameterRasterLayer> ( QStringLiteral( "param" ), QString() );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // mesh layer parm, only mesh types are acceptable
  param = std::make_unique< QgsProcessingParameterMeshLayer> ( QStringLiteral( "param" ), QString() );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // point cloud layer parm, only point cloud types are acceptable
  param = std::make_unique< QgsProcessingParameterPointCloudLayer> ( QStringLiteral( "param" ), QString() );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // feature source and vector layer params
  // if not specified, the default is any vector layer with geometry
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ) );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  auto param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ) );
  auto combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint );
  combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorLine );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorLine );
  combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPolygon );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPolygon );
  combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVector );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVector );
  combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorAnyGeometry );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorAnyGeometry );
  combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = std::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
  combo2 = std::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  param = std::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>(), QVariant(), true );
  combo = std::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  QgsVectorLayer *point = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( QStringLiteral( "None" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( noGeom );
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", QStringLiteral( "Triangle and Quad Mdal" ), QStringLiteral( "mdal" ) );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  QgsProject::instance()->addMapLayer( mesh );
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", QStringLiteral( "band1_byte" ) );
  QgsProject::instance()->addMapLayer( raster );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterMapLayer param( QStringLiteral( "layer" ), QStringLiteral( "layer" ), false );

    QgsProcessingMapLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "band1_byte" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte" ) );
        break;
    }

    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "band1_byte" ) );

    // check signal
    static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( polygon );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), polygon->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1 [EPSG:4326]" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
        break;
    }
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "l1" ) );

    delete w;

    // optional
    QgsProcessingParameterMapLayer param2( QStringLiteral( "layer" ), QStringLiteral( "layer" ), QVariant(), true );
    QgsProcessingMapLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "band1_byte" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "layer" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layer" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterMapLayer layerParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QVariant(), false, QList< int >() << QgsProcessing::TypeVectorAnyGeometry );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layer" ), context, widgetContext, &layerParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterMapLayer * >( def.get() )->dataTypes(), QList< int >() << QgsProcessing::TypeVectorAnyGeometry );
  layerParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  layerParam.setDataTypes( QList< int >() << QgsProcessing::TypeRaster << QgsProcessing::TypeVectorPoint );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layer" ), context, widgetContext, &layerParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterMapLayer * >( def.get() )->dataTypes(), QList< int >()  << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeRaster );
}

void TestProcessingGui::testRasterLayerWrapper()
{
  // setup a project
  QgsProject::instance()->removeAllMapLayers();
  QgsRasterLayer *raster = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", QStringLiteral( "band1_byte" ) );
  QgsProject::instance()->addMapLayer( raster );
  QgsRasterLayer *raster2 = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif", QStringLiteral( "band1_byte2" ) );
  QgsProject::instance()->addMapLayer( raster2 );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterRasterLayer param( QStringLiteral( "raster" ), QStringLiteral( "raster" ), false );

    QgsProcessingMapLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "band1_byte" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte" ) );
        break;
    }

    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "band1_byte" ) );

    // check signal
    static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( raster2 );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), raster2->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte2 [EPSG:4326]" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte2" ) );
        break;
    }
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "band1_byte2" ) );

    delete w;

    // optional
    QgsProcessingParameterRasterLayer param2( QStringLiteral( "raster" ), QStringLiteral( "raster" ), QVariant(), true );
    QgsProcessingMapLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingMapLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "band1_byte" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), raster->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "band1_byte" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "raster" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testVectorLayerWrapper()
{
  // setup a project with a range of vector layers
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "point" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( QStringLiteral( "None" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( noGeom );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterVectorLayer param( QStringLiteral( "vector" ), QStringLiteral( "vector" ), QList<int >() << QgsProcessing::TypeVector, false );

    QgsProcessingVectorLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingVectorLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "point" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "point [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "point" ) );
        break;
    }

    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "point" ) );

    // check signal
    static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( polygon );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), polygon->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1 [EPSG:4326]" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
        break;
    }
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "l1" ) );

    delete w;

    // optional
    QgsProcessingParameterVectorLayer param2( QStringLiteral( "vector" ), QStringLiteral( "vector" ), QList< int >() << QgsProcessing::TypeVector, QVariant(), true );
    QgsProcessingVectorLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingVectorLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "point" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "point [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "point" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "vector" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "vector" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterVectorLayer layerParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QList< int >() << QgsProcessing::TypeVectorAnyGeometry );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "vector" ), context, widgetContext, &layerParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterVectorLayer * >( def.get() )->dataTypes(), QList< int >() << QgsProcessing::TypeVectorAnyGeometry );
  layerParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  layerParam.setDataTypes( QList< int >() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPoint );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "vector" ), context, widgetContext, &layerParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterVectorLayer * >( def.get() )->dataTypes(), QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
}

void TestProcessingGui::testFeatureSourceWrapper()
{
  // setup a project with a range of vector layers
  QgsProject::instance()->removeAllMapLayers();
  QgsVectorLayer *point = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "point" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( point );
  QgsVectorLayer *line = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( line );
  QgsVectorLayer *polygon = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( polygon );
  QgsVectorLayer *noGeom = new QgsVectorLayer( QStringLiteral( "None" ), QStringLiteral( "l1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( noGeom );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterFeatureSource param( QStringLiteral( "source" ), QStringLiteral( "source" ), QList<int >() << QgsProcessing::TypeVector, false );

    QgsProcessingFeatureSourceWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFeatureSourceWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "point" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "point [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "point" ) );
        break;
    }

    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "point" ) );

    // check signal
    static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( polygon );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), polygon->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1 [EPSG:4326]" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "l1" ) );
        break;
    }
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "l1" ) );

    delete w;

    // optional
    QgsProcessingParameterFeatureSource param2( QStringLiteral( "source" ), QStringLiteral( "source" ), QList< int >() << QgsProcessing::TypeVector, QVariant(), true );
    QgsProcessingFeatureSourceWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFeatureSourceWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "point" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), point->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "point [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "point" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "source" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "source" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFeatureSource sourceParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QList< int >() << QgsProcessing::TypeVectorAnyGeometry );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "source" ), context, widgetContext, &sourceParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterFeatureSource * >( def.get() )->dataTypes(), QList< int >() << QgsProcessing::TypeVectorAnyGeometry );
  sourceParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  sourceParam.setDataTypes( QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "source" ), context, widgetContext, &sourceParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterFeatureSource * >( def.get() )->dataTypes(), QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
}

void TestProcessingGui::testMeshLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsMeshLayer *mesh = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", QStringLiteral( "mesh1" ), QStringLiteral( "mdal" ) );
  mesh->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh->isValid() );
  mesh->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QgsProject::instance()->addMapLayer( mesh );
  QgsMeshLayer *mesh2 = new QgsMeshLayer( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm", QStringLiteral( "mesh2" ), QStringLiteral( "mdal" ) );
  mesh2->dataProvider()->addDataset( QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mesh2->isValid() );
  mesh2->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QgsProject::instance()->addMapLayer( mesh2 );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterMeshLayer param( QStringLiteral( "mesh" ), QStringLiteral( "mesh" ), false );

    QgsProcessingMeshLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "mesh2" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), mesh2->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "mesh2 [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "mesh2" ) );
        break;
    }

    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "mesh2" ) );

    // check signal
    static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( mesh );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), mesh->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "mesh1 [EPSG:4326]" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "mesh1" ) );
        break;
    }
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "mesh1" ) );

    delete w;

    // optional
    QgsProcessingParameterMeshLayer param2( QStringLiteral( "mesh" ), QStringLiteral( "mesh" ), QVariant(), true );
    QgsProcessingMeshLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingMeshLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "mesh2" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), mesh2->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "mesh2 [EPSG:4326]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "mesh2" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "mesh" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::paramConfigWidget()
{
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  def->setDescription( QStringLiteral( "test desc" ) );
  def->setFlags( QgsProcessingParameterDefinition::FlagOptional );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, def.get() );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  def->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, def.get() );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
}

void TestProcessingGui::testMapThemeWrapper()
{
  // add some themes to the project
  QgsProject p;
  p.mapThemeCollection()->insert( QStringLiteral( "aa" ), QgsMapThemeCollection::MapThemeRecord() );
  p.mapThemeCollection()->insert( QStringLiteral( "bb" ), QgsMapThemeCollection::MapThemeRecord() );

  QCOMPARE( p.mapThemeCollection()->mapThemes(), QStringList() << QStringLiteral( "aa" ) << QStringLiteral( "bb" ) );

  auto testWrapper = [&p]( QgsProcessingGui::WidgetType type )
  {
    // non optional, no existing themes
    QgsProcessingParameterMapTheme param( QStringLiteral( "theme" ), QStringLiteral( "theme" ), false );

    QgsProcessingMapThemeWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        // batch or standard mode, only valid themes can be set!
        QCOMPARE( spy.count(), 0 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentIndex(), -1 );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 0 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentIndex(), -1 );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "aa" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "aa" ) );
    QCOMPARE( static_cast< QComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );

    // check signal
    static_cast< QComboBox * >( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
    QCOMPARE( spy2.count(), 3 );

    delete w;

    // optional
    QgsProcessingParameterMapTheme param2( QStringLiteral( "theme" ), QStringLiteral( "theme" ), true );
    QgsProcessingMapThemeWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "aa" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "aa" ) );
    QCOMPARE( static_cast< QComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "theme" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  widgetContext.setProject( &p );
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "maptheme" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterMapTheme * >( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterMapTheme themeParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "aaa" ), false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "maptheme" ), context, widgetContext, &themeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterMapTheme * >( def.get() )->defaultValue().toString(), QStringLiteral( "aaa" ) );
  themeParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  themeParam.setDefaultValue( QStringLiteral( "xxx" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "maptheme" ), context, widgetContext, &themeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterMapTheme * >( def.get() )->defaultValue().toString(), QStringLiteral( "xxx" ) );
  themeParam.setDefaultValue( QVariant() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "maptheme" ), context, widgetContext, &themeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QVERIFY( !static_cast< QgsProcessingParameterMapTheme * >( def.get() )->defaultValue().isValid() );
}

void TestProcessingGui::testDateTimeWrapper()
{
  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional, no existing themes
    QgsProcessingParameterDateTime param( QStringLiteral( "datetime" ), QStringLiteral( "datetime" ), QgsProcessingParameterDateTime::DateTime, QVariant(), false );

    QgsProcessingDateTimeWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    // not a date value
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy.count(), 0 );
    // not optional, so an invalid value gets a date anyway...
    QVERIFY( wrapper.widgetValue().isValid() );
    wrapper.setWidgetValue( QStringLiteral( "2019-08-07" ), context );
    QCOMPARE( spy.count(), 1 );
    QVERIFY( wrapper.widgetValue().isValid() );
    QCOMPARE( wrapper.widgetValue().toDateTime(), QDateTime( QDate( 2019, 8, 7 ), QTime( 0, 0, 0 ) ) );
    QCOMPARE( static_cast< QgsDateTimeEdit * >( wrapper.wrappedWidget() )->dateTime(), QDateTime( QDate( 2019, 8, 7 ), QTime( 0, 0, 0 ) ) );
    wrapper.setWidgetValue( QStringLiteral( "2019-08-07" ), context );
    QCOMPARE( spy.count(), 1 );

    // check signal
    static_cast< QgsDateTimeEdit * >( wrapper.wrappedWidget() )->setDateTime( QDateTime( QDate( 2019, 8, 9 ), QTime( 0, 0, 0 ) ) );
    QCOMPARE( spy.count(), 2 );

    delete w;

    // optional
    QgsProcessingParameterDateTime param2( QStringLiteral( "datetime" ), QStringLiteral( "datetime" ), QgsProcessingParameterDateTime::DateTime, QVariant(), true );
    QgsProcessingDateTimeWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    QSignalSpy spy3( &wrapper3, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 0 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    QVERIFY( !static_cast< QgsDateTimeEdit * >( wrapper3.wrappedWidget() )->dateTime().isValid() );
    wrapper3.setWidgetValue( QStringLiteral( "2019-03-20" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toDateTime(), QDateTime( QDate( 2019, 3, 20 ), QTime( 0, 0, 0 ) ) );
    QCOMPARE( static_cast< QgsDateTimeEdit * >( wrapper3.wrappedWidget() )->dateTime(), QDateTime( QDate( 2019, 3, 20 ), QTime( 0, 0, 0 ) ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    // date mode
    QgsProcessingParameterDateTime param3( QStringLiteral( "datetime" ), QStringLiteral( "datetime" ), QgsProcessingParameterDateTime::Date, QVariant(), true );
    QgsProcessingDateTimeWidgetWrapper wrapper4( &param3, type );
    w = wrapper4.createWrappedWidget( context );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    QSignalSpy spy4( &wrapper4, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy4.count(), 0 );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    QVERIFY( !static_cast< QgsDateEdit * >( wrapper4.wrappedWidget() )->date().isValid() );
    wrapper4.setWidgetValue( QStringLiteral( "2019-03-20" ), context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toDate(), QDate( 2019, 3, 20 ) );
    QCOMPARE( static_cast< QgsDateEdit * >( wrapper4.wrappedWidget() )->date(), QDate( 2019, 3, 20 ) );
    wrapper4.setWidgetValue( QDate( 2020, 1, 3 ), context );
    QCOMPARE( spy4.count(), 2 );
    QCOMPARE( wrapper4.widgetValue().toDate(), QDate( 2020, 1, 3 ) );
    QCOMPARE( static_cast< QgsDateEdit * >( wrapper4.wrappedWidget() )->date(), QDate( 2020, 1, 3 ) );
    wrapper4.setWidgetValue( QVariant(), context );
    QCOMPARE( spy4.count(), 3 );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    delete w;

    // time mode
    QgsProcessingParameterDateTime param4( QStringLiteral( "datetime" ), QStringLiteral( "datetime" ), QgsProcessingParameterDateTime::Time, QVariant(), true );
    QgsProcessingDateTimeWidgetWrapper wrapper5( &param4, type );
    w = wrapper5.createWrappedWidget( context );
    QVERIFY( !wrapper5.widgetValue().isValid() );
    QSignalSpy spy5( &wrapper5, &QgsProcessingDateTimeWidgetWrapper::widgetValueHasChanged );
    wrapper5.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy5.count(), 0 );
    QVERIFY( !wrapper5.widgetValue().isValid() );
    QVERIFY( !static_cast< QgsTimeEdit * >( wrapper5.wrappedWidget() )->time().isValid() );
    wrapper5.setWidgetValue( QStringLiteral( "11:34:56" ), context );
    QCOMPARE( spy5.count(), 1 );
    QCOMPARE( wrapper5.widgetValue().toTime(), QTime( 11, 34, 56 ) );
    QCOMPARE( static_cast< QgsTimeEdit * >( wrapper5.wrappedWidget() )->time(), QTime( 11, 34, 56 ) );
    wrapper5.setWidgetValue( QTime( 9, 34, 56 ), context );
    QCOMPARE( spy5.count(), 2 );
    QCOMPARE( wrapper5.widgetValue().toTime(), QTime( 9, 34, 56 ) );
    QCOMPARE( static_cast< QgsTimeEdit * >( wrapper5.wrappedWidget() )->time(), QTime( 9, 34, 56 ) );
    wrapper5.setWidgetValue( QVariant(), context );
    QCOMPARE( spy5.count(), 3 );
    QVERIFY( !wrapper5.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "datetime" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "datetime" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterDateTime * >( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterDateTime datetimeParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterDateTime::Date, QVariant(), false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "datetime" ), context, widgetContext, &datetimeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterDateTime * >( def.get() )->dataType(), QgsProcessingParameterDateTime::Date );
  datetimeParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  datetimeParam.setDefaultValue( QStringLiteral( "xxx" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "datetime" ), context, widgetContext, &datetimeParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterDateTime * >( def.get() )->dataType(), QgsProcessingParameterDateTime::Date );
}

void TestProcessingGui::testProviderConnectionWrapper()
{
  // register some connections
  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) );
  QgsAbstractProviderConnection *conn = md->createConnection( QStringLiteral( "test uri.gpkg" ), QVariantMap() );
  md->saveConnection( conn, QStringLiteral( "aa" ) );
  md->saveConnection( conn, QStringLiteral( "bb" ) );

  auto testWrapper = []( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterProviderConnection param( QStringLiteral( "conn" ), QStringLiteral( "connection" ), QStringLiteral( "ogr" ), false );

    QgsProcessingProviderConnectionWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingProviderConnectionWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProviderConnectionComboBox * >( wrapper.wrappedWidget() )->currentConnection(), QStringLiteral( "bb" ) );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy.count(), 1 );
    wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
    QCOMPARE( spy.count(), 2 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        // batch or standard mode, only valid connections can be set!
        // not valid
        wrapper.setWidgetValue( QStringLiteral( "cc" ), context );
        QCOMPARE( spy.count(), 3 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast< QComboBox * >( wrapper.wrappedWidget() )->currentIndex(), -1 );
        break;

      }
      case QgsProcessingGui::Modeler:
        // invalid connections permitted
        wrapper.setWidgetValue( QStringLiteral( "cc" ), context );
        QCOMPARE( spy.count(), 3 );
        QCOMPARE( static_cast< QgsProviderConnectionComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "cc" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "cc" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 4 );
        QCOMPARE( static_cast< QgsProviderConnectionComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        break;
    }

    delete w;
    // optional
    QgsProcessingParameterProviderConnection param2( QStringLiteral( "conn" ), QStringLiteral( "connection" ), QStringLiteral( "ogr" ), QVariant(), true );
    QgsProcessingProviderConnectionWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingEnumWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "aa" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "aa" ) );
    QCOMPARE( static_cast< QComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;
    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "connection" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "providerconnection" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterProviderConnection * >( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterProviderConnection connParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "spatialite" ), QStringLiteral( "aaa" ), false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "providerconnection" ), context, widgetContext, &connParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterProviderConnection * >( def.get() )->defaultValue().toString(), QStringLiteral( "aaa" ) );
  QCOMPARE( static_cast< QgsProcessingParameterProviderConnection * >( def.get() )->providerId(), QStringLiteral( "spatialite" ) );
  connParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  connParam.setDefaultValue( QStringLiteral( "xxx" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "providerconnection" ), context, widgetContext, &connParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterProviderConnection * >( def.get() )->defaultValue().toString(), QStringLiteral( "xxx" ) );
  connParam.setDefaultValue( QVariant() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "providerconnection" ), context, widgetContext, &connParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QVERIFY( !static_cast< QgsProcessingParameterProviderConnection * >( def.get() )->defaultValue().isValid() );
}

void TestProcessingGui::testDatabaseSchemaWrapper()
{
#ifdef ENABLE_PGTEST
  // register some connections
  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );

  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsAbstractProviderConnection *conn = md->createConnection( QStringLiteral( "%1 sslmode=disable" ).arg( dbConn ), QVariantMap() );
  md->saveConnection( conn, QStringLiteral( "aa" ) );

  const QStringList schemas = dynamic_cast<QgsAbstractDatabaseProviderConnection *>( conn )->schemas();
  QVERIFY( !schemas.isEmpty() );

  auto testWrapper = [&schemas]( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterProviderConnection connParam( QStringLiteral( "conn" ), QStringLiteral( "connection" ), QStringLiteral( "postgres" ), QVariant(), true );
    TestLayerWrapper connWrapper( &connParam );

    QgsProcessingParameterDatabaseSchema param( QStringLiteral( "schema" ), QStringLiteral( "schema" ), QStringLiteral( "conn" ), QVariant(), false );

    QgsProcessingDatabaseSchemaWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    // no connection associated yet
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    // Set the parent widget connection value
    connWrapper.setWidgetValue( QStringLiteral( "aa" ), context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );

    // now we should have schemas available
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->count(), schemas.count() );

    QSignalSpy spy( &wrapper, &QgsProcessingDatabaseSchemaWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "qgis_test" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "qgis_test" ) );
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->currentSchema(), QStringLiteral( "qgis_test" ) );
    wrapper.setWidgetValue( QStringLiteral( "public" ), context );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "public" ) );
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->currentSchema(), QStringLiteral( "public" ) );
    QCOMPARE( spy.count(), 2 );
    wrapper.setWidgetValue( QStringLiteral( "public" ), context );
    QCOMPARE( spy.count(), 2 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        // batch or standard mode, only valid schemas can be set!
        // not valid
        wrapper.setWidgetValue( QStringLiteral( "cc" ), context );
        QCOMPARE( spy.count(), 3 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentIndex(), -1 );
        break;

      }
      case QgsProcessingGui::Modeler:
        // invalid schemas permitted
        wrapper.setWidgetValue( QStringLiteral( "cc" ), context );
        QCOMPARE( spy.count(), 3 );
        QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "cc" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "cc" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 4 );
        QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "aa" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        break;
    }

    // make sure things are ok if connection is changed back to nothing
    connWrapper.setWidgetValue( QVariant(), context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        QCOMPARE( spy.count(), 3 );
        break;
      }

      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 5 );
        break;
    }
    QVERIFY( !wrapper.widgetValue().isValid() );

    wrapper.setWidgetValue( QStringLiteral( "qgis_test" ), context );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        QVERIFY( !wrapper.widgetValue().isValid() );
        break;
      }

      case QgsProcessingGui::Modeler:
        // invalid schemas permitted
        QCOMPARE( spy.count(), 6 );
        QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "qgis_test" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "qgis_test" ) );

        break;
    }
    delete w;

    connWrapper.setWidgetValue( QStringLiteral( "aa" ), context );

    // optional
    QgsProcessingParameterDatabaseSchema param2( QStringLiteral( "schema" ), QStringLiteral( "schema" ), QStringLiteral( "conn" ), QVariant(), true );
    QgsProcessingDatabaseSchemaWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    wrapper3.setParentConnectionWrapperValue( &connWrapper );

    QSignalSpy spy3( &wrapper3, &QgsProcessingDatabaseSchemaWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "qgis_test" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "qgis_test" ) );
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper3.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "qgis_test" ) );
    wrapper3.setWidgetValue( QStringLiteral( "public" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "public" ) );
    QCOMPARE( static_cast< QgsDatabaseSchemaComboBox * >( wrapper3.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "public" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );

    delete w;
    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "schema" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databaseschema" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterDatabaseSchema * >( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterDatabaseSchema schemaParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "connparam" ), QStringLiteral( "aaa" ), false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databaseschema" ), context, widgetContext, &schemaParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseSchema * >( def.get() )->defaultValue().toString(), QStringLiteral( "aaa" ) );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseSchema * >( def.get() )->parentConnectionParameterName(), QStringLiteral( "connparam" ) );
  schemaParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  schemaParam.setDefaultValue( QStringLiteral( "xxx" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databaseschema" ), context, widgetContext, &schemaParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseSchema * >( def.get() )->defaultValue().toString(), QStringLiteral( "xxx" ) );
  schemaParam.setDefaultValue( QVariant() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databaseschema" ), context, widgetContext, &schemaParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QVERIFY( !static_cast< QgsProcessingParameterDatabaseSchema * >( def.get() )->defaultValue().isValid() );
#endif
}

void TestProcessingGui::testDatabaseTableWrapper()
{
#ifdef ENABLE_PGTEST
  // register some connections
  QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );

  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsAbstractProviderConnection *conn = md->createConnection( QStringLiteral( "%1 sslmode=disable" ).arg( dbConn ), QVariantMap() );
  md->saveConnection( conn, QStringLiteral( "aa" ) );

  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables = dynamic_cast<QgsAbstractDatabaseProviderConnection *>( conn )->tables( QStringLiteral( "qgis_test" ) );
  QStringList tableNames;
  for ( const QgsAbstractDatabaseProviderConnection::TableProperty &prop : tables )
    tableNames << prop.tableName();

  QVERIFY( !tableNames.isEmpty() );

  auto testWrapper = [&tableNames]( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterProviderConnection connParam( QStringLiteral( "conn" ), QStringLiteral( "connection" ), QStringLiteral( "postgres" ), QVariant(), true );
    TestLayerWrapper connWrapper( &connParam );
    QgsProcessingParameterDatabaseSchema schemaParam( QStringLiteral( "schema" ), QStringLiteral( "schema" ), QStringLiteral( "connection" ), QVariant(), true );
    TestLayerWrapper schemaWrapper( &schemaParam );

    QgsProcessingParameterDatabaseTable param( QStringLiteral( "table" ), QStringLiteral( "table" ), QStringLiteral( "conn" ), QStringLiteral( "schema" ), QVariant(), false );

    QgsProcessingDatabaseTableWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );
    // no connection associated yet
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    // Set the parent widget connection value
    connWrapper.setWidgetValue( QStringLiteral( "aa" ), context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );
    schemaWrapper.setWidgetValue( QStringLiteral( "qgis_test" ), context );
    wrapper.setParentSchemaWrapperValue( &schemaWrapper );

    // now we should have tables available
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->count(), tableNames.count() );

    QSignalSpy spy( &wrapper, &QgsProcessingDatabaseTableWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "someData" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "someData" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->currentTable(), QStringLiteral( "someData" ) );
    wrapper.setWidgetValue( QStringLiteral( "some_poly_data" ), context );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "some_poly_data" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->currentTable(), QStringLiteral( "some_poly_data" ) );
    QCOMPARE( spy.count(), 2 );
    wrapper.setWidgetValue( QStringLiteral( "some_poly_data" ), context );
    QCOMPARE( spy.count(), 2 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        // batch or standard mode, only valid tables can be set!
        // not valid
        wrapper.setWidgetValue( QStringLiteral( "cc" ), context );
        QCOMPARE( spy.count(), 3 );
        QVERIFY( !wrapper.widgetValue().isValid() );
        QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentIndex(), -1 );
        break;

      }
      case QgsProcessingGui::Modeler:
        // invalid tables permitted
        wrapper.setWidgetValue( QStringLiteral( "cc" ), context );
        QCOMPARE( spy.count(), 3 );
        QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "cc" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "cc" ) );
        wrapper.setWidgetValue( QStringLiteral( "someData" ), context );
        QCOMPARE( spy.count(), 4 );
        QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "someData" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "someData" ) );
        break;
    }

    // make sure things are ok if connection is changed back to nothing
    connWrapper.setWidgetValue( QVariant(), context );
    wrapper.setParentConnectionWrapperValue( &connWrapper );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->count(), 0 );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        QCOMPARE( spy.count(), 3 );
        break;
      }

      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 5 );
        break;
    }
    QVERIFY( !wrapper.widgetValue().isValid() );

    wrapper.setWidgetValue( QStringLiteral( "some_poly_data" ), context );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      {
        QVERIFY( !wrapper.widgetValue().isValid() );
        break;
      }

      case QgsProcessingGui::Modeler:
        // invalid tables permitted
        QCOMPARE( spy.count(), 6 );
        QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "some_poly_data" ) );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "some_poly_data" ) );

        break;
    }
    delete w;

    connWrapper.setWidgetValue( QStringLiteral( "aa" ), context );

    // optional
    QgsProcessingParameterDatabaseTable param2( QStringLiteral( "table" ), QStringLiteral( "table" ), QStringLiteral( "conn" ), QStringLiteral( "schema" ), QVariant(), true );
    QgsProcessingDatabaseTableWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    wrapper3.setParentConnectionWrapperValue( &connWrapper );
    wrapper3.setParentSchemaWrapperValue( &schemaWrapper );

    QSignalSpy spy3( &wrapper3, &QgsProcessingDatabaseTableWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "someData" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "someData" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper3.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "someData" ) );
    wrapper3.setWidgetValue( QStringLiteral( "some_poly_data" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "some_poly_data" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper3.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "some_poly_data" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );

    delete w;

    // allowing new table names
    QgsProcessingParameterDatabaseTable param3( QStringLiteral( "table" ), QStringLiteral( "table" ), QStringLiteral( "conn" ), QStringLiteral( "schema" ), QVariant(), false, true );
    QgsProcessingDatabaseTableWidgetWrapper wrapper4( &param3, type );
    w = wrapper4.createWrappedWidget( context );

    wrapper4.setParentConnectionWrapperValue( &connWrapper );
    wrapper4.setParentSchemaWrapperValue( &schemaWrapper );

    QSignalSpy spy4( &wrapper4, &QgsProcessingDatabaseTableWidgetWrapper::widgetValueHasChanged );
    wrapper4.setWidgetValue( QStringLiteral( "someData" ), context );
    QCOMPARE( spy4.count(), 1 );
    QCOMPARE( wrapper4.widgetValue().toString(), QStringLiteral( "someData" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper4.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "someData" ) );
    wrapper4.setWidgetValue( QStringLiteral( "some_poly_data" ), context );
    QCOMPARE( spy4.count(), 2 );
    QCOMPARE( wrapper4.widgetValue().toString(), QStringLiteral( "some_poly_data" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper4.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "some_poly_data" ) );
    wrapper4.setWidgetValue( QVariant(), context );
    QCOMPARE( spy4.count(), 3 );
    QVERIFY( !wrapper4.widgetValue().isValid() );
    // should always allow non existing table names
    wrapper4.setWidgetValue( QStringLiteral( "someDataxxxxxxxxxxxxxxxxxxxx" ), context );
    QCOMPARE( spy4.count(), 4 );
    QCOMPARE( wrapper4.widgetValue().toString(), QStringLiteral( "someDataxxxxxxxxxxxxxxxxxxxx" ) );
    QCOMPARE( static_cast< QgsDatabaseTableComboBox * >( wrapper4.wrappedWidget() )->comboBox()->currentText(), QStringLiteral( "someDataxxxxxxxxxxxxxxxxxxxx" ) );


    delete w;


    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "table" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databasetable" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterDatabaseTable * >( def.get() )->defaultValue().isValid() );

  // using a parameter definition as initial values
  QgsProcessingParameterDatabaseTable tableParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "connparam" ), QStringLiteral( "schemaparam" ), QStringLiteral( "aaa" ), false );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databasetable" ), context, widgetContext, &tableParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseTable * >( def.get() )->defaultValue().toString(), QStringLiteral( "aaa" ) );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseTable * >( def.get() )->parentConnectionParameterName(), QStringLiteral( "connparam" ) );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseTable * >( def.get() )->parentSchemaParameterName(), QStringLiteral( "schemaparam" ) );
  tableParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  tableParam.setDefaultValue( QStringLiteral( "xxx" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databasetable" ), context, widgetContext, &tableParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterDatabaseTable * >( def.get() )->defaultValue().toString(), QStringLiteral( "xxx" ) );
  tableParam.setDefaultValue( QVariant() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "databasetable" ), context, widgetContext, &tableParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QVERIFY( !static_cast< QgsProcessingParameterDatabaseTable * >( def.get() )->defaultValue().isValid() );
#endif
}

void TestProcessingGui::testFieldMapWidget()
{
  QgsProcessingFieldMapPanelWidget widget;

  QVariantMap map;
  map.insert( QStringLiteral( "name" ), QStringLiteral( "n" ) );
  map.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::Double ) );
  map.insert( QStringLiteral( "length" ), 8 );
  map.insert( QStringLiteral( "precision" ), 5 );
  QVariantMap map2;
  map2.insert( QStringLiteral( "name" ), QStringLiteral( "n2" ) );
  map2.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::String ) );
  map2.insert( QStringLiteral( "expression" ), QStringLiteral( "'abc' || \"def\"" ) );

  QSignalSpy spy( &widget, &QgsProcessingFieldMapPanelWidget::changed );
  widget.setValue( QVariantList() << map << map2 );
  QCOMPARE( spy.size(), 1 );

  QCOMPARE( widget.value().toList().size(), 2 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::Double ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "length" ) ).toInt(), 8 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "precision" ) ).toInt(), 5 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "expression" ) ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::String ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "expression" ) ).toString(), QStringLiteral( "'abc' || \"def\"" ) );
}

void TestProcessingGui::testFieldMapWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:centroids" ) );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( QStringLiteral( "INPUT" ) );

  auto testWrapper = [layerDef]( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterFieldMapping param( QStringLiteral( "mapping" ), QStringLiteral( "mapping" ) );

    QgsProcessingFieldMapWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QVariantMap map;
    map.insert( QStringLiteral( "name" ), QStringLiteral( "n" ) );
    map.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::Double ) );
    map.insert( QStringLiteral( "length" ), 8 );
    map.insert( QStringLiteral( "precision" ), 5 );
    QVariantMap map2;
    map2.insert( QStringLiteral( "name" ), QStringLiteral( "n2" ) );
    map2.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::String ) );
    map2.insert( QStringLiteral( "expression" ), QStringLiteral( "'abc' || \"def\"" ) );

    QSignalSpy spy( &wrapper, &QgsProcessingFieldMapWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << map << map2, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::Double ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "length" ) ).toInt(), 8 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "precision" ) ).toInt(), 5 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "expression" ) ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::String ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "expression" ) ).toString(), QStringLiteral( "'abc' || \"def\"" ) );

    QCOMPARE( static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper.wrappedWidget() )->value().toList().count(), 2 );
    QCOMPARE( static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper.wrappedWidget() )->value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
    QCOMPARE( static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper.wrappedWidget() )->value().toList().at( 1 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
    wrapper.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toList().size(), 1 );
    QCOMPARE( static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper.wrappedWidget() )->value().toList().size(), 1 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "mapping" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper.wrappedWidget() )->setValue( QVariantList() << map << map2 );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( QStringLiteral( "other" ) );
    QgsProcessingFieldMapWidgetWrapper wrapper2( &param, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingFieldMapWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().size(),  1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
    QCOMPARE( static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );

    wrapper2.setWidgetValue( QVariantList() << map2, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toList().size(),  1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
    QCOMPARE( static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );

    static_cast< QgsProcessingFieldMapPanelWidget * >( wrapper2.wrappedWidget() )->setValue( QVariantList() << map );
    QCOMPARE( spy2.count(), 3 );

    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
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
    QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer()->publicSource(), pointFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );

  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "fields_mapping" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFieldMapping mapParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "parent" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "fields_mapping" ), context, widgetContext, &mapParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterFieldMapping * >( def.get() )->parentLayerParameterName(), QStringLiteral( "parent" ) );
  mapParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  mapParam.setParentLayerParameterName( QString() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "fields_mapping" ), context, widgetContext, &mapParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( static_cast< QgsProcessingParameterFieldMapping * >( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testAggregateWidget()
{
  QgsProcessingAggregatePanelWidget widget;

  QVariantMap map;
  map.insert( QStringLiteral( "name" ), QStringLiteral( "n" ) );
  map.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::Double ) );
  map.insert( QStringLiteral( "length" ), 8 );
  map.insert( QStringLiteral( "precision" ), 5 );
  QVariantMap map2;
  map2.insert( QStringLiteral( "name" ), QStringLiteral( "n2" ) );
  map2.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::String ) );
  map2.insert( QStringLiteral( "input" ), QStringLiteral( "'abc' || \"def\"" ) );
  map2.insert( QStringLiteral( "aggregate" ), QStringLiteral( "concatenate" ) );
  map2.insert( QStringLiteral( "delimiter" ), QStringLiteral( "|" ) );

  QSignalSpy spy( &widget, &QgsProcessingAggregatePanelWidget::changed );
  widget.setValue( QVariantList() << map << map2 );
  QCOMPARE( spy.size(), 1 );

  QCOMPARE( widget.value().toList().size(), 2 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::Double ) );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "length" ) ).toInt(), 8 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "precision" ) ).toInt(), 5 );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "input" ) ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "aggregate" ) ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 0 ).toMap().value( QStringLiteral( "delimiter" ) ).toString(), QString() );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::String ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "input" ) ).toString(), QStringLiteral( "'abc' || \"def\"" ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "aggregate" ) ).toString(), QStringLiteral( "concatenate" ) );
  QCOMPARE( widget.value().toList().at( 1 ).toMap().value( QStringLiteral( "delimiter" ) ).toString(), QStringLiteral( "|" ) );
}

void TestProcessingGui::testAggregateWrapper()
{
  const QgsProcessingAlgorithm *centroidAlg = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:centroids" ) );
  const QgsProcessingParameterDefinition *layerDef = centroidAlg->parameterDefinition( QStringLiteral( "INPUT" ) );

  auto testWrapper = [layerDef]( QgsProcessingGui::WidgetType type )
  {
    QgsProcessingParameterAggregate param( QStringLiteral( "mapping" ), QStringLiteral( "mapping" ) );

    QgsProcessingAggregateWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QVariantMap map;
    map.insert( QStringLiteral( "name" ), QStringLiteral( "n" ) );
    map.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::Double ) );
    map.insert( QStringLiteral( "length" ), 8 );
    map.insert( QStringLiteral( "precision" ), 5 );
    QVariantMap map2;
    map2.insert( QStringLiteral( "name" ), QStringLiteral( "n2" ) );
    map2.insert( QStringLiteral( "type" ), static_cast< int >( QVariant::String ) );
    map2.insert( QStringLiteral( "input" ), QStringLiteral( "'abc' || \"def\"" ) );
    map2.insert( QStringLiteral( "aggregate" ), QStringLiteral( "concatenate" ) );
    map2.insert( QStringLiteral( "delimiter" ), QStringLiteral( "|" ) );

    QSignalSpy spy( &wrapper, &QgsProcessingFieldMapWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QVariantList() << map << map2, context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::Double ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "length" ) ).toInt(), 8 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "precision" ) ).toInt(), 5 );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "input" ) ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "aggregate" ) ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "delimiter" ) ).toString(), QString() );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "type" ) ).toInt(), static_cast< int >( QVariant::String ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "input" ) ).toString(), QStringLiteral( "'abc' || \"def\"" ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "aggregate" ) ).toString(), QStringLiteral( "concatenate" ) );
    QCOMPARE( wrapper.widgetValue().toList().at( 1 ).toMap().value( QStringLiteral( "delimiter" ) ).toString(), QStringLiteral( "|" ) );

    QCOMPARE( static_cast< QgsProcessingAggregatePanelWidget * >( wrapper.wrappedWidget() )->value().toList().count(), 2 );
    QCOMPARE( static_cast< QgsProcessingAggregatePanelWidget * >( wrapper.wrappedWidget() )->value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
    QCOMPARE( static_cast< QgsProcessingAggregatePanelWidget * >( wrapper.wrappedWidget() )->value().toList().at( 1 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
    wrapper.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy.count(), 2 );
    QCOMPARE( wrapper.widgetValue().toList().size(), 1 );
    QCOMPARE( static_cast< QgsProcessingAggregatePanelWidget * >( wrapper.wrappedWidget() )->value().toList().size(), 1 );

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "mapping" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }

    // check signal
    static_cast< QgsProcessingAggregatePanelWidget * >( wrapper.wrappedWidget() )->setValue( QVariantList() << map << map2 );
    QCOMPARE( spy.count(), 3 );

    delete w;

    // with layer
    param.setParentLayerParameterName( QStringLiteral( "other" ) );
    QgsProcessingAggregateWidgetWrapper wrapper2( &param, type );
    w = wrapper2.createWrappedWidget( context );

    QSignalSpy spy2( &wrapper2, &QgsProcessingAggregateWidgetWrapper::widgetValueHasChanged );
    wrapper2.setWidgetValue( QVariantList() << map, context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toList().size(),  1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );
    QCOMPARE( static_cast< QgsProcessingAggregatePanelWidget * >( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n" ) );

    wrapper2.setWidgetValue( QVariantList() << map2, context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toList().size(),  1 );
    QCOMPARE( wrapper2.widgetValue().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );
    QCOMPARE( static_cast< QgsProcessingAggregatePanelWidget * >( wrapper2.wrappedWidget() )->value().toList().at( 0 ).toMap().value( QStringLiteral( "name" ) ).toString(), QStringLiteral( "n2" ) );

    static_cast< QgsProcessingAggregatePanelWidget * >( wrapper2.wrappedWidget() )->setValue( QVariantList() << map );
    QCOMPARE( spy2.count(), 3 );


    TestLayerWrapper layerWrapper( layerDef );
    QgsProject p;
    QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
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
    QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
    layerWrapper.setWidgetValue( pointFileName, context );
    wrapper2.setParentLayerWrapperValue( &layerWrapper );
    QCOMPARE( wrapper2.mPanel->layer()->publicSource(), pointFileName );
    // must be owned by wrapper, or layer may be deleted while still required by wrapper
    QCOMPARE( wrapper2.mParentLayer->publicSource(), pointFileName );

  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  QgsProcessingContext context;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "aggregates" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterAggregate mapParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "parent" ) );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "aggregates" ), context, widgetContext, &mapParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterAggregate * >( def.get() )->parentLayerParameterName(), QStringLiteral( "parent" ) );
  mapParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  mapParam.setParentLayerParameterName( QString() );
  widget = std::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "aggregates" ), context, widgetContext, &mapParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( static_cast< QgsProcessingParameterAggregate * >( def.get() )->parentLayerParameterName().isEmpty() );
}

void TestProcessingGui::testOutputDefinitionWidget()
{
  QgsProcessingParameterFeatureSink sink( QStringLiteral( "test" ) );
  QgsProcessingLayerOutputDestinationWidget panel( &sink, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  QgsProcessingOutputLayerDefinition def;
  def.sink.setStaticValue( QgsProcessing::TEMPORARY_OUTPUT );
  def.createOptions.insert( QStringLiteral( "fileEncoding" ), QStringLiteral( "utf8" ) );
  panel.setValue( def );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );

  panel.setValue( QStringLiteral( "memory:" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  def.sink.setStaticValue( QStringLiteral( "memory:" ) );
  panel.setValue( def );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );

  panel.setValue( QStringLiteral( "ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''" ) );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( QStringLiteral( "ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  panel.setValue( QStringLiteral( "postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql=" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql=" ) );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );
  panel.setValue( QStringLiteral( "postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql=" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );

  panel.setValue( QStringLiteral( "/home/me/test.shp" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "/home/me/test.shp" ) );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( QStringLiteral( "/home/me/test.shp" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( QStringLiteral( "/home/me/test2.shp" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 4 );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), TEST_DATA_DIR );
  panel.setValue( QStringLiteral( "test.shp" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "utf8" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + QStringLiteral( "/test.shp" ) ) );

  // optional, test skipping
  sink.setFlags( sink.flags() | QgsProcessingParameterDefinition::FlagOptional );
  sink.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &sink, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  def = QgsProcessingOutputLayerDefinition( QStringLiteral( "test.shp" ) );
  QgsRemappingSinkDefinition remap;
  QMap< QString, QgsProperty > fieldMap;
  fieldMap.insert( QStringLiteral( "field1" ), QgsProperty::fromField( QStringLiteral( "source1" ) ) );
  fieldMap.insert( QStringLiteral( "field2" ), QgsProperty::fromExpression( QStringLiteral( "source || source2" ) ) );
  remap.setFieldMap( fieldMap );
  def.setRemappingDefinition( remap );

  panel3.setValue( def );
  v = panel3.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QVERIFY( v.value< QgsProcessingOutputLayerDefinition>().useRemapping() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().remappingDefinition().fieldMap().size(), 2 );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().remappingDefinition().fieldMap().value( QStringLiteral( "field1" ) ), QgsProperty::fromField( QStringLiteral( "source1" ) ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().remappingDefinition().fieldMap().value( QStringLiteral( "field2" ) ), QgsProperty::fromExpression( QStringLiteral( "source || source2" ) ) );

  panel3.setValue( QStringLiteral( "other.shp" ) );
  v = panel3.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QVERIFY( !v.value< QgsProcessingOutputLayerDefinition>().useRemapping() );
}

void TestProcessingGui::testOutputDefinitionWidgetVectorOut()
{
  QgsProcessingParameterVectorDestination vector( QStringLiteral( "test" ) );
  QgsProcessingLayerOutputDestinationWidget panel( &vector, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( QStringLiteral( "memory:" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );


  panel.setValue( QStringLiteral( "ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''" ) );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( QStringLiteral( "ogr:dbname='/me/a.gpkg' table=\"d\" (geom) sql=''" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  panel.setValue( QStringLiteral( "postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql=" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql=" ) );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );
  panel.setValue( QStringLiteral( "postgis:dbname='oraclesux' host=10.1.1.221 port=5432 user='qgis' password='qgis' table=\"stufff\".\"output\" (the_geom) sql=" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 2 );

  panel.setValue( QStringLiteral( "/home/me/test.shp" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "/home/me/test.shp" ) );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( QStringLiteral( "/home/me/test.shp" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 3 );
  panel.setValue( QStringLiteral( "/home/me/test2.shp" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 4 );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), TEST_DATA_DIR );
  panel.setValue( QStringLiteral( "test.shp" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + QStringLiteral( "/test.shp" ) ) );

  // optional, test skipping
  vector.setFlags( vector.flags() | QgsProcessingParameterDefinition::FlagOptional );
  vector.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &vector, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QgsProcessingParameterRasterDestination raster( QStringLiteral( "test" ) );
  QgsProcessingLayerOutputDestinationWidget panel( &raster, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( QStringLiteral( "/home/me/test.tif" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "/home/me/test.tif" ) );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( QStringLiteral( "/home/me/test.tif" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), TEST_DATA_DIR );
  panel.setValue( QStringLiteral( "test.tif" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + QStringLiteral( "/test.tif" ) ) );

  // optional, test skipping
  raster.setFlags( raster.flags() | QgsProcessingParameterDefinition::FlagOptional );
  raster.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &raster, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QgsProcessingParameterPointCloudDestination pointCloud( QStringLiteral( "test" ) );
  QgsProcessingLayerOutputDestinationWidget panel( &pointCloud, false );

  QSignalSpy skipSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy( &panel, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  QVariant v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );

  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel.outputIsSkipped() );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );
  panel.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 0 );

  panel.setValue( QStringLiteral( "/home/me/test.las" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "/home/me/test.las" ) );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( QStringLiteral( "/home/me/test.las" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), TEST_DATA_DIR );
  panel.setValue( QStringLiteral( "test.las" ) );
  v = panel.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QString( TEST_DATA_DIR + QStringLiteral( "/test.las" ) ) );

  // optional, test skipping
  pointCloud.setFlags( pointCloud.flags() | QgsProcessingParameterDefinition::FlagOptional );
  pointCloud.setCreateByDefault( true );
  QgsProcessingLayerOutputDestinationWidget panel2( &pointCloud, false );

  QSignalSpy skipSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged );
  QSignalSpy changedSpy2( &panel2, &QgsProcessingLayerOutputDestinationWidget::destinationChanged );

  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
  QVERIFY( !panel2.outputIsSkipped() );

  panel2.setValue( QgsProcessing::TEMPORARY_OUTPUT );
  v = panel2.value();
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QVERIFY( v.canConvert< QgsProcessingOutputLayerDefinition>() );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().createOptions.value( QStringLiteral( "fileEncoding" ) ).toString(), QStringLiteral( "System" ) );
  QCOMPARE( v.value< QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QgsProcessing::TEMPORARY_OUTPUT );
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
  QgsProcessingParameterFolderDestination folder( QStringLiteral( "test" ) );
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

  panel.setValue( QStringLiteral( "/home/me/" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.toString(), QStringLiteral( "/home/me/" ) );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( QStringLiteral( "/home/me/" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), TEST_DATA_DIR );
  panel.setValue( QStringLiteral( "mystuff" ) );
  v = panel.value();
  QCOMPARE( v.toString(), QString( TEST_DATA_DIR + QStringLiteral( "/mystuff" ) ) );

  // optional, test skipping
  folder.setFlags( folder.flags() | QgsProcessingParameterDefinition::FlagOptional );
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
  QgsProcessingParameterFileDestination file( QStringLiteral( "test" ) );
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

  panel.setValue( QStringLiteral( "/home/me/test.tif" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );
  v = panel.value();
  QCOMPARE( v.toString(), QStringLiteral( "/home/me/test.tif" ) );
  QVERIFY( !panel.outputIsSkipped() );
  panel.setValue( QStringLiteral( "/home/me/test.tif" ) );
  QCOMPARE( skipSpy.count(), 0 );
  QCOMPARE( changedSpy.count(), 1 );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), TEST_DATA_DIR );
  panel.setValue( QStringLiteral( "test.tif" ) );
  v = panel.value();
  QCOMPARE( v.toString(), QString( TEST_DATA_DIR + QStringLiteral( "/test.tif" ) ) );

  // optional, test skipping
  file.setFlags( file.flags() | QgsProcessingParameterDefinition::FlagOptional );
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

  w.setGeometryCheckMethod( false, QgsFeatureRequest::GeometrySkipInvalid );
  QCOMPARE( spy.count(), 2 );
  QVERIFY( !w.isOverridingInvalidGeometryCheck() );
  w.setGeometryCheckMethod( true, QgsFeatureRequest::GeometrySkipInvalid );
  QCOMPARE( spy.count(), 3 );
  QVERIFY( w.isOverridingInvalidGeometryCheck() );
  QCOMPARE( w.geometryCheckMethod(), QgsFeatureRequest::GeometrySkipInvalid );
  w.setGeometryCheckMethod( true, QgsFeatureRequest::GeometrySkipInvalid );
  QCOMPARE( spy.count(), 3 );
  w.setGeometryCheckMethod( true, QgsFeatureRequest::GeometryAbortOnInvalid );
  QCOMPARE( spy.count(), 4 );
  QVERIFY( w.isOverridingInvalidGeometryCheck() );
  QCOMPARE( w.geometryCheckMethod(), QgsFeatureRequest::GeometryAbortOnInvalid );
  w.setGeometryCheckMethod( false, QgsFeatureRequest::GeometryAbortOnInvalid );
  QVERIFY( !w.isOverridingInvalidGeometryCheck() );
  QCOMPARE( spy.count(), 5 );
}

void TestProcessingGui::testVectorOutWrapper()
{
  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterVectorDestination param( QStringLiteral( "vector" ), QStringLiteral( "vector" ) );

    QgsProcessingVectorDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingVectorDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "/bb.shp" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
        wrapper.setWidgetValue( QStringLiteral( "/aa.shp" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/aa.shp" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/aa.shp" ) );
        break;
    }

    // check signal
    static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->setValue( QStringLiteral( "/cc.shp" ) );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/cc.shp" ) );
    delete w;

    // optional
    QgsProcessingParameterVectorDestination param2( QStringLiteral( "vector" ), QStringLiteral( "vector" ), QgsProcessing::TypeVector, QVariant(), true );
    QgsProcessingVectorDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingVectorDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "/bb.shp" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
    QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper3.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "vector" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  // testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testSinkWrapper()
{
  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterFeatureSink param( QStringLiteral( "sink" ), QStringLiteral( "sink" ) );

    QgsProcessingFeatureSinkWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFeatureSinkWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "/bb.shp" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
        wrapper.setWidgetValue( QStringLiteral( "/aa.shp" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/aa.shp" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/aa.shp" ) );
        break;
    }

    // check signal
    static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->setValue( QStringLiteral( "/cc.shp" ) );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/cc.shp" ) );
    delete w;

    // optional
    QgsProcessingParameterFeatureSink param2( QStringLiteral( "sink" ), QStringLiteral( "sink" ), QgsProcessing::TypeVector, QVariant(), true );
    QgsProcessingFeatureSinkWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFeatureSinkWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "/bb.shp" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
    QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper3.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.shp" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "sink" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  // testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testRasterOutWrapper()
{
  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterRasterDestination param( QStringLiteral( "raster" ), QStringLiteral( "raster" ) );

    QgsProcessingRasterDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingRasterDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "/bb.tif" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.tif" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.tif" ) );
        wrapper.setWidgetValue( QStringLiteral( "/aa.tif" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/aa.tif" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/aa.tif" ) );
        break;
    }

    // check signal
    static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->setValue( QStringLiteral( "/cc.tif" ) );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/cc.tif" ) );
    delete w;

    // optional
    QgsProcessingParameterRasterDestination param2( QStringLiteral( "raster" ), QStringLiteral( "raster" ), QVariant(), true );
    QgsProcessingRasterDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingRasterDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "/bb.tif" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.tif" ) );
    QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper3.wrappedWidget() )->value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), QStringLiteral( "/bb.tif" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "raster" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  // testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testFileOutWrapper()
{
  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterFileDestination param( QStringLiteral( "file" ), QStringLiteral( "file" ) );

    QgsProcessingFileDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFileDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "/bb.tif" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "/bb.tif" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().toString(), QStringLiteral( "/bb.tif" ) );
        wrapper.setWidgetValue( QStringLiteral( "/aa.tif" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "/aa.tif" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().toString(), QStringLiteral( "/aa.tif" ) );
        break;
    }

    // check signal
    static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->setValue( QStringLiteral( "/cc.tif" ) );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "/cc.tif" ) );
    delete w;

    // optional
    QgsProcessingParameterFileDestination param2( QStringLiteral( "file" ), QStringLiteral( "file" ), QString(), QVariant(), true );
    QgsProcessingFileDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFileDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "/bb.tif" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "/bb.tif" ) );
    QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper3.wrappedWidget() )->value().toString(), QStringLiteral( "/bb.tif" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "file" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  // testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testFolderOutWrapper()
{
  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterFolderDestination param( QStringLiteral( "folder" ), QStringLiteral( "folder" ) );

    QgsProcessingFolderDestinationWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingFolderDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "/bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "/bb" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().toString(), QStringLiteral( "/bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "/aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "/aa" ) );
        QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->value().toString(), QStringLiteral( "/aa" ) );
        break;
    }

    // check signal
    static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper.wrappedWidget() )->setValue( QStringLiteral( "/cc" ) );
    QCOMPARE( spy.count(), 3 );
    QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "/cc" ) );
    delete w;

    // optional
    QgsProcessingParameterFolderDestination param2( QStringLiteral( "folder" ), QStringLiteral( "folder" ), QVariant(), true );
    QgsProcessingFolderDestinationWidgetWrapper wrapper3( &param2, type );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingFolderDestinationWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "/bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "/bb" ) );
    QCOMPARE( static_cast< QgsProcessingLayerOutputDestinationWidget * >( wrapper3.wrappedWidget() )->value().toString(), QStringLiteral( "/bb" ) );
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 2 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "folder" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  // testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testTinInputLayerWrapper()
{
  QgsProcessingParameterTinInputLayers definition( QStringLiteral( "TIN input layers" ) ) ;
  QgsProcessingTinInputLayersWidgetWrapper wrapper;

  std::unique_ptr<QWidget> w( wrapper.createWidget() );
  QVERIFY( w );

  QSignalSpy spy( &wrapper, &QgsProcessingTinInputLayersWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QgsProject project;
  context.setProject( &project );
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( QStringLiteral( "Point" ),
      QStringLiteral( "PointLayerForTin" ),
      QStringLiteral( "memory" ) );
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
  QCOMPARE( valueAsPythonString, QStringLiteral( "[{'source': 'PointLayerForTin','type': 0,'attributeIndex': -1}]" ) );
}

void TestProcessingGui::testDxfLayersWrapper()
{
  QgsProcessingParameterDxfLayers definition( QStringLiteral( "DXF layers" ) ) ;
  QgsProcessingDxfLayersWidgetWrapper wrapper;

  std::unique_ptr<QWidget> w( wrapper.createWidget() );
  QVERIFY( w );

  QSignalSpy spy( &wrapper, &QgsProcessingTinInputLayersWidgetWrapper::widgetValueHasChanged );

  QgsProcessingContext context;
  QgsProject project;
  context.setProject( &project );
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( QStringLiteral( "Point" ),
      QStringLiteral( "PointLayer" ),
      QStringLiteral( "memory" ) );
  project.addMapLayer( vectorLayer );

  QVariantList layerList;
  QVariantMap layerMap;
  layerMap["layer"] = "PointLayer";
  layerMap["attributeIndex"] = -1;
  layerList.append( layerMap );

  QVERIFY( definition.checkValueIsAcceptable( layerList, &context ) );
  wrapper.setWidgetValue( layerList, context );
  QCOMPARE( spy.count(), 1 );

  QVariant value = wrapper.widgetValue();

  QVERIFY( definition.checkValueIsAcceptable( value, &context ) );
  QString valueAsPythonString = definition.valueAsPythonString( value, context );
  QCOMPARE( valueAsPythonString, QStringLiteral( "[{'layer': '%1','attributeIndex': -1}]" ).arg( vectorLayer->source() ) );
}

void TestProcessingGui::testMeshDatasetWrapperLayerInProject()
{
  QgsProcessingParameterMeshLayer layerDefinition( QStringLiteral( "layer" ), QStringLiteral( "layer" ) );
  QgsProcessingMeshLayerWidgetWrapper layerWrapper( &layerDefinition );

  QSet<int> supportedDataType( {QgsMeshDatasetGroupMetadata::DataOnVertices} );
  QgsProcessingParameterMeshDatasetGroups groupsDefinition( QStringLiteral( "groups" ),
      QStringLiteral( "groups" ),
      QStringLiteral( "layer" ),
      supportedDataType );
  QgsProcessingMeshDatasetGroupsWidgetWrapper groupsWrapper( &groupsDefinition );

  QgsProcessingParameterMeshDatasetTime timeDefinition( QStringLiteral( "time" ), QStringLiteral( "time" ), QStringLiteral( "layer" ), QStringLiteral( "groups" ) );
  QgsProcessingMeshDatasetTimeWidgetWrapper timeWrapper( &timeDefinition );

  QList<QgsAbstractProcessingParameterWidgetWrapper *> wrappers;
  wrappers << &layerWrapper << &groupsWrapper << &timeWrapper;

  QgsProject project;
  QgsProcessingContext context;
  context.setProject( &project );
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr<QgsMapCanvas> mapCanvas = std::make_unique<QgsMapCanvas>();
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
  QString meshLayerName = QStringLiteral( "mesh layer" );
  QgsMeshLayer *layer = new QgsMeshLayer( uri, meshLayerName, QStringLiteral( "mdal" ) );
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
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "static" ) );

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 2 );
  QCOMPARE( timeSpy.count(), 3 );

  // with layer in the project
  layerSpy.clear();
  groupsSpy.clear();
  timeSpy.clear();

  project.addMapLayer( layer );
  static_cast<QgsMeshLayerTemporalProperties *>( layer->temporalProperties() )->setReferenceTime(
    QDateTime( QDate( 2020, 01, 01 ), QTime( 0, 0, 0 ), Qt::UTC ), layer->dataProvider()->temporalCapabilities() );
  layerWrapper.setWidgetValue( meshLayerName, context );

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 1 );
  QCOMPARE( timeSpy.count(), 2 );

  datasetGroupWidget->selectCurrentActiveDatasetGroup();

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 2 );
  QCOMPARE( timeSpy.count(), 3 );

  QVariant groupsValue = groupsWrapper.widgetValue();
  QVERIFY( groupsValue.type() == QVariant::List );
  QVariantList groupsList = groupsValue.toList();
  QCOMPARE( groupsList.count(), 1 );
  QCOMPARE( groupsList.at( 0 ).toInt(), 1 );
  QString pythonString = groupsDefinition.valueAsPythonString( groupsValue, context );
  QCOMPARE( pythonString, QStringLiteral( "[1]" ) );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsValue ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsValue ), QList<int>( {1} ) );

  // 2 datasets on vertices
  settings = layer->rendererSettings();
  settings.setActiveVectorDatasetGroup( 2 );
  layer->setRendererSettings( settings );
  datasetGroupWidget->selectCurrentActiveDatasetGroup();

  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 4 );

  pythonString = groupsDefinition.valueAsPythonString( groupsWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "[1,2]" ) );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsWrapper.widgetValue() ), QList<int>() << 1 << 2 );

  datasetTimeWidget->radioButtonDatasetGroupTimeStep->setChecked( true );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 4 ); //radioButtonDatasetGroupTimeStep already checked

  QVariant timeValue = timeWrapper.widgetValue();
  QVERIFY( timeValue.type() == QVariant::Map );
  QVariantMap timeValueMap = timeValue.toMap();
  QCOMPARE( timeValueMap[QStringLiteral( "type" )].toString(), QStringLiteral( "dataset-time-step" ) );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'dataset-time-step','value': [1,0]}" ) );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeValue ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeValue ), QStringLiteral( "dataset-time-step" ) );
  QVERIFY( QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( timeValue ) == QgsMeshDatasetIndex( 1, 0 ) );

  datasetTimeWidget->radioButtonDefinedDateTime->setChecked( true );
  QDateTime dateTime = QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 1, 0 ), Qt::UTC );
  datasetTimeWidget->dateTimeEdit->setDateTime( dateTime );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 6 );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'defined-date-time','value': QDateTime(QDate(2020, 1, 1), QTime(0, 1, 0))}" ) );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "defined-date-time" ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( timeWrapper.widgetValue() ), dateTime );

  QVERIFY( !datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );
  mapCanvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2021, 1, 1 ), QTime( 0, 3, 0 ), Qt::UTC ), QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 5, 0 ), Qt::UTC ) ) );
  QVERIFY( datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );

  datasetTimeWidget->radioButtonCurrentCanvasTime->setChecked( true );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 8 );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'current-context-time'}" ) );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "current-context-time" ) );

  // 0 dataset on vertices
  settings = layer->rendererSettings();
  settings.setActiveScalarDatasetGroup( -1 );
  settings.setActiveVectorDatasetGroup( -1 );
  layer->setRendererSettings( settings );
  datasetGroupWidget->selectCurrentActiveDatasetGroup();
  QVERIFY( !datasetTimeWidget->isEnabled() );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'static'}" ) );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "static" ) );

  // 1 static dataset on vertices
  settings = layer->rendererSettings();
  settings.setActiveScalarDatasetGroup( 0 );
  settings.setActiveVectorDatasetGroup( -1 );
  layer->setRendererSettings( settings );
  datasetGroupWidget->selectCurrentActiveDatasetGroup();
  QVERIFY( !datasetTimeWidget->isEnabled() );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'static'}" ) );
  QVERIFY( timeDefinition.checkValueIsAcceptable( timeWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "static" ) );

  groupsWrapper.setWidgetValue( 3, context );
  QCOMPARE( datasetGroupWidget->value(), QVariantList( {3} ) );
  groupsWrapper.setWidgetValue( QVariantList( {1, 2, 3} ), context );
  QCOMPARE( datasetGroupWidget->value().toList(), QVariantList( {1, 2, 3} ) );
  groupsWrapper.setWidgetValue( QVariantList( {"1", "2", "3"} ), context );
  QCOMPARE( datasetGroupWidget->value().toList(), QVariantList( {1, 2, 3} ) );
  groupsWrapper.setWidgetValue( QgsProperty::fromExpression( QStringLiteral( "1+3" ) ), context );
  QCOMPARE( datasetGroupWidget->value().toList(), QVariantList( {4} ) );

  timeWrapper.setWidgetValue( QDateTime( QDate( 2020, 01, 02 ), QTime( 1, 2, 3 ) ), context );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'defined-date-time','value': QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3))}" ) );
  timeWrapper.setWidgetValue( QVariant::fromValue( QDateTime( QDate( 2020, 02, 01 ), QTime( 3, 2, 1 ) ) ).toString(), context );
  pythonString = timeDefinition.valueAsPythonString( timeWrapper.widgetValue(), context );
  QCOMPARE( pythonString, QStringLiteral( "{'type': 'defined-date-time','value': QDateTime(QDate(2020, 2, 1), QTime(3, 2, 1))}" ) );
}

void TestProcessingGui::testMeshDatasetWrapperLayerOutsideProject()
{
  QgsProcessingParameterMeshLayer layerDefinition( QStringLiteral( "layer" ), QStringLiteral( "layer" ) );
  QgsProcessingMeshLayerWidgetWrapper layerWrapper( &layerDefinition );

  QSet<int> supportedDataType( {QgsMeshDatasetGroupMetadata::DataOnFaces} );
  QgsProcessingParameterMeshDatasetGroups groupsDefinition( QStringLiteral( "groups" ),
      QStringLiteral( "groups" ),
      QStringLiteral( "layer" ),
      supportedDataType );
  QgsProcessingMeshDatasetGroupsWidgetWrapper groupsWrapper( &groupsDefinition );

  QgsProcessingParameterMeshDatasetTime timeDefinition( QStringLiteral( "time" ), QStringLiteral( "time" ), QStringLiteral( "layer" ), QStringLiteral( "groups" ) );
  QgsProcessingMeshDatasetTimeWidgetWrapper timeWrapper( &timeDefinition );

  QList<QgsAbstractProcessingParameterWidgetWrapper *> wrappers;
  wrappers << &layerWrapper << &groupsWrapper << &timeWrapper;

  QgsProject project;
  QgsProcessingContext context;
  context.setProject( &project );
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr<QgsMapCanvas> mapCanvas = std::make_unique<QgsMapCanvas>();
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
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "static" ) );
  QVERIFY( !datasetTimeWidget->isEnabled() );

  groups << 11;
  groupsWrapper.setWidgetValue( groups, context );
  QCOMPARE( layerSpy.count(), 1 );
  QCOMPARE( groupsSpy.count(), 3 );
  QCOMPARE( timeSpy.count(), 5 );
  QVERIFY( datasetTimeWidget->isEnabled() );
  QVERIFY( groupsDefinition.checkValueIsAcceptable( groupsWrapper.widgetValue() ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( groupsWrapper.widgetValue() ), QList<int>() << 0 << 11 );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "dataset-time-step" ) );
  QVERIFY( QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( timeWrapper.widgetValue() ) == QgsMeshDatasetIndex( 11, 0 ) );

  QVERIFY( datasetTimeWidget->radioButtonDefinedDateTime->isEnabled() );
  QVERIFY( !datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );

  datasetTimeWidget->radioButtonDefinedDateTime->setChecked( true );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::valueAsTimeType( timeWrapper.widgetValue() ), QStringLiteral( "defined-date-time" ) );
  QCOMPARE( QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( timeWrapper.widgetValue() ),
            QDateTime( QDate( 1990, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );


  mapCanvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2021, 1, 1 ), QTime( 0, 3, 0 ), Qt::UTC ), QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 5, 0 ), Qt::UTC ) ) );
  QVERIFY( datasetTimeWidget->radioButtonCurrentCanvasTime->isEnabled() );

}

void TestProcessingGui::testPointCloudLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsPointCloudLayer *cloud1 = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", QStringLiteral( "cloud1" ), QStringLiteral( "ept" ) );
  QVERIFY( cloud1->isValid() );
  QgsProject::instance()->addMapLayer( cloud1 );
  QgsPointCloudLayer *cloud2 = new QgsPointCloudLayer( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/sunshine-coast/ept.json", QStringLiteral( "cloud2" ), QStringLiteral( "ept" ) );
  QVERIFY( cloud2->isValid() );
  QgsProject::instance()->addMapLayer( cloud2 );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterPointCloudLayer param( QStringLiteral( "cloud" ), QStringLiteral( "cloud" ), false );

    QgsProcessingPointCloudLayerWidgetWrapper wrapper( &param, type );

    QgsProcessingContext context;
    QWidget *w = wrapper.createWrappedWidget( context );

    QSignalSpy spy( &wrapper, &QgsProcessingPointCloudLayerWidgetWrapper::widgetValueHasChanged );
    wrapper.setWidgetValue( QStringLiteral( "bb" ), context );

    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
      case QgsProcessingGui::Modeler:
        QCOMPARE( spy.count(), 1 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "bb" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
        wrapper.setWidgetValue( QStringLiteral( "aa" ), context );
        QCOMPARE( spy.count(), 2 );
        QCOMPARE( wrapper.widgetValue().toString(), QStringLiteral( "aa" ) );
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper.wrappedWidget() )->currentText(), QStringLiteral( "aa" ) );
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
    wrapper2.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper2.setWidgetValue( QStringLiteral( "cloud2" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), cloud2->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "cloud2 [EPSG:28356]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "cloud2" ) );
        break;
    }

    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "cloud2" ) );

    // check signal
    static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( cloud1 );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), cloud1->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "cloud1 [EPSG:28356]" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "cloud1" ) );
        break;
    }
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "cloud1" ) );

    delete w;

    // optional
    QgsProcessingParameterPoint param2( QStringLiteral( "cloud" ), QStringLiteral( "cloud" ), QVariant(), true );
    QgsProcessingPointCloudLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingPointCloudLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "bb" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "bb" ) );
    QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "bb" ) );
    wrapper3.setWidgetValue( QStringLiteral( "cloud2" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), cloud2->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "cloud2 [EPSG:28356]" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( static_cast< QgsProcessingMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "cloud2" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "cloud" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testAnnotationLayerWrapper()
{
  // setup a project with a range of layer types
  QgsProject::instance()->removeAllMapLayers();
  QgsAnnotationLayer *layer1 = new QgsAnnotationLayer( QStringLiteral( "secondary annotations" ), QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer1->isValid() );
  QgsProject::instance()->addMapLayer( layer1 );

  auto testWrapper = [ = ]( QgsProcessingGui::WidgetType type )
  {
    // non optional
    QgsProcessingParameterAnnotationLayer param( QStringLiteral( "annotation" ), QStringLiteral( "annotation" ), false );

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
    wrapper2.setWidgetValue( QStringLiteral( "main" ), context );
    QCOMPARE( spy2.count(), 1 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "main" ) );
    QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "Annotations" ) );
    wrapper2.setWidgetValue( QStringLiteral( "secondary annotations" ), context );
    QCOMPARE( spy2.count(), 2 );
    QCOMPARE( wrapper2.widgetValue().toString(), layer1->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "secondary annotations" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "secondary annotations" ) );
        break;
    }

    QCOMPARE( static_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "secondary annotations" ) );

    // check signal
    static_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->setLayer( QgsProject::instance()->mainAnnotationLayer() );
    QCOMPARE( spy2.count(), 3 );
    QCOMPARE( wrapper2.widgetValue().toString(), QStringLiteral( "main" ) );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "Annotations" ) );
        break;

      case QgsProcessingGui::Modeler:
        QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentText(), QStringLiteral( "Annotations" ) );
        break;
    }
    QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper2.wrappedWidget() )->currentLayer()->name(), QStringLiteral( "Annotations" ) );

    delete w;

    // optional
    QgsProcessingParameterAnnotationLayer param2( QStringLiteral( "annotation" ), QStringLiteral( "annotation" ), QVariant(), true );
    QgsProcessingAnnotationLayerWidgetWrapper wrapper3( &param2, type );
    wrapper3.setWidgetContext( widgetContext );
    w = wrapper3.createWrappedWidget( context );

    QSignalSpy spy3( &wrapper3, &QgsProcessingAnnotationLayerWidgetWrapper::widgetValueHasChanged );
    wrapper3.setWidgetValue( QStringLiteral( "main" ), context );
    QCOMPARE( spy3.count(), 1 );
    QCOMPARE( wrapper3.widgetValue().toString(), QStringLiteral( "main" ) );
    QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "Annotations" ) );
    wrapper3.setWidgetValue( QStringLiteral( "secondary annotations" ), context );
    QCOMPARE( spy3.count(), 2 );
    QCOMPARE( wrapper3.widgetValue().toString(), layer1->id() );
    switch ( type )
    {
      case QgsProcessingGui::Standard:
      case QgsProcessingGui::Batch:
        QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "secondary annotations" ) );
        break;
      case QgsProcessingGui::Modeler:
        QCOMPARE( qgis::down_cast< QgsMapLayerComboBox * >( wrapper3.wrappedWidget() )->currentText(), QStringLiteral( "secondary annotations" ) );
        break;
    }
    wrapper3.setWidgetValue( QVariant(), context );
    QCOMPARE( spy3.count(), 3 );
    QVERIFY( !wrapper3.widgetValue().isValid() );
    delete w;

    QLabel *l = wrapper.createWrappedLabel();
    if ( wrapper.type() != QgsProcessingGui::Batch )
    {
      QVERIFY( l );
      QCOMPARE( l->text(), QStringLiteral( "annotation" ) );
      QCOMPARE( l->toolTip(), param.toolTip() );
      delete l;
    }
    else
    {
      QVERIFY( !l );
    }
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testModelGraphicsView()
{
  // test model
  QgsProcessingModelAlgorithm model1;

  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "buffer" );
  algc1.setAlgorithmId( "native:buffer" );
  QgsProcessingModelParameter param;
  param.setParameterName( QStringLiteral( "LAYER" ) );
  param.setSize( QSizeF( 500, 400 ) );
  param.setPosition( QPointF( 101, 102 ) );
  param.comment()->setDescription( QStringLiteral( "input comment" ) );
  model1.addModelParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "LAYER" ) ), param );
  algc1.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "LAYER" ) ) );
  algc1.comment()->setDescription( QStringLiteral( "alg comment" ) );
  algc1.comment()->setSize( QSizeF( 300, 200 ) );
  algc1.comment()->setPosition( QPointF( 201, 202 ) );

  QgsProcessingModelOutput modelOut;
  modelOut.setChildId( algc1.childId() );
  modelOut.setChildOutputName( QStringLiteral( "my_output" ) );
  modelOut.comment()->setDescription( QStringLiteral( "output comm" ) );
  QMap< QString, QgsProcessingModelOutput > outs;
  outs.insert( QStringLiteral( "OUTPUT" ), modelOut );
  algc1.setModelOutputs( outs );
  model1.addChildAlgorithm( algc1 );

  QgsProcessingModelGroupBox groupBox;
  groupBox.setDescription( QStringLiteral( "group" ) );
  model1.addGroupBox( groupBox );

  // hiding comments
  QgsProcessingContext context;
  QgsModelGraphicsScene scene2;
  scene2.setModel( &model1 );
  scene2.setFlags( QgsModelGraphicsScene::FlagHideComments );
  scene2.createItems( &model1, context );
  QList< QGraphicsItem * > items = scene2.items();
  QgsModelParameterGraphicItem *layerItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelParameterGraphicItem *param = dynamic_cast< QgsModelParameterGraphicItem * >( item ) )
    {
      layerItem = param;
      break;
    }
  }
  QVERIFY( layerItem );
  QgsModelCommentGraphicItem *layerCommentItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelCommentGraphicItem *comment = dynamic_cast< QgsModelCommentGraphicItem * >( item ) )
    {
      layerCommentItem = comment;
      break;
    }
  }
  // should not exist
  QVERIFY( !layerCommentItem );


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
    if ( QgsModelParameterGraphicItem *param = dynamic_cast< QgsModelParameterGraphicItem * >( item ) )
    {
      layerItem = param;

    }
  }
  QVERIFY( layerItem );
  QCOMPARE( dynamic_cast<  QgsProcessingModelParameter * >( layerItem->component() )->parameterName(), QStringLiteral( "LAYER" ) );
  QCOMPARE( layerItem->itemRect().size(), QSizeF( 500, 400 ) );
  QCOMPARE( layerItem->scenePos(), QPointF( 101, 102 ) );

  QgsModelChildAlgorithmGraphicItem *algItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelChildAlgorithmGraphicItem *param = dynamic_cast< QgsModelChildAlgorithmGraphicItem * >( item ) )
    {
      algItem = param;
      break;
    }
  }
  QVERIFY( algItem );
  QCOMPARE( dynamic_cast<  QgsProcessingModelChildAlgorithm * >( algItem->component() )->algorithmId(), QStringLiteral( "native:buffer" ) );

  QgsModelOutputGraphicItem *outputItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelOutputGraphicItem *comment = dynamic_cast< QgsModelOutputGraphicItem * >( item ) )
    {
      outputItem = comment;
      break;
    }
  }
  QVERIFY( outputItem );
  QCOMPARE( dynamic_cast< QgsProcessingModelOutput * >( outputItem->component() )->childOutputName(), QStringLiteral( "my_output" ) );


  layerCommentItem = nullptr;
  QgsModelCommentGraphicItem *algCommentItem = nullptr;
  QgsModelCommentGraphicItem *outputCommentItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelCommentGraphicItem *comment = dynamic_cast< QgsModelCommentGraphicItem * >( item ) )
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
  QCOMPARE( algCommentItem->component()->description(), QStringLiteral( "alg comment" ) );
  QCOMPARE( algCommentItem->itemRect().size(), QSizeF( 300, 200 ) );
  QCOMPARE( algCommentItem->scenePos(), QPointF( 201, 202 ) );

  QVERIFY( layerCommentItem );
  QCOMPARE( layerCommentItem->component()->description(), QStringLiteral( "input comment" ) );

  QVERIFY( outputCommentItem );
  QCOMPARE( outputCommentItem->component()->description(), QStringLiteral( "output comm" ) );

  QgsModelGroupBoxGraphicItem *groupItem = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelGroupBoxGraphicItem *comment = dynamic_cast< QgsModelGroupBoxGraphicItem * >( item ) )
    {
      groupItem = comment;
      break;
    }
  }
  QVERIFY( groupItem );
  QCOMPARE( dynamic_cast< QgsProcessingModelGroupBox * >( groupItem->component() )->description(), QStringLiteral( "group" ) );


  QgsModelGraphicsView view;
  view.setModelScene( &scene );

  // copy some items
  view.copyItems( QList< QgsModelComponentGraphicItem * >() << layerItem << algItem << groupItem, QgsModelGraphicsView::ClipboardCopy );


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
  QCOMPARE( algDest.parameterComponents().value( QStringLiteral( "LAYER" ) ).parameterName(), QStringLiteral( "LAYER" ) );
  // comment should not be copied, was not selected
  QCOMPARE( algDest.parameterComponents().value( QStringLiteral( "LAYER" ) ).comment()->description(), QString() );
  QCOMPARE( algDest.childAlgorithms().size(), 1 );
  // the child algorithm is already unique, so should not be changed
  QCOMPARE( algDest.childAlgorithms().keys().at( 0 ), QStringLiteral( "buffer" ) );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "buffer" ) ).algorithmId(), QStringLiteral( "native:buffer" ) );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "buffer" ) ).comment()->description(), QString() );
  // output was not selected
  QVERIFY( algDest.childAlgorithms().value( QStringLiteral( "buffer" ) ).modelOutputs().empty() );
  QCOMPARE( algDest.groupBoxes().size(), 1 );
  QCOMPARE( algDest.groupBoxes().at( 0 ).description(), QStringLiteral( "group" ) );

  // copy comments and output (not output comment though!)
  view.copyItems( QList< QgsModelComponentGraphicItem * >() << layerItem << layerCommentItem << algItem << algCommentItem << outputItem << groupItem, QgsModelGraphicsView::ClipboardCopy );
  viewDest.pasteItems( QgsModelGraphicsView::PasteModeInPlace );

  QCOMPARE( algDest.parameterComponents().size(), 2 );
  QCOMPARE( algDest.parameterComponents().value( QStringLiteral( "LAYER" ) ).parameterName(), QStringLiteral( "LAYER" ) );
  QCOMPARE( algDest.parameterComponents().value( QStringLiteral( "LAYER (2)" ) ).parameterName(), QStringLiteral( "LAYER (2)" ) );
  QCOMPARE( algDest.parameterComponents().value( QStringLiteral( "LAYER" ) ).comment()->description(), QString() );
  QCOMPARE( algDest.parameterComponents().value( QStringLiteral( "LAYER (2)" ) ).comment()->description(), QStringLiteral( "input comment" ) );
  QCOMPARE( algDest.childAlgorithms().size(), 2 );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "buffer" ) ).algorithmId(), QStringLiteral( "native:buffer" ) );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "buffer" ) ).comment()->description(), QString() );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_1" ) ).algorithmId(), QStringLiteral( "native:buffer" ) );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_1" ) ).comment()->description(), QStringLiteral( "alg comment" ) );
  QVERIFY( algDest.childAlgorithms().value( QStringLiteral( "buffer" ) ).modelOutputs().empty() );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_1" ) ).modelOutputs().size(), 1 );
  // output comment wasn't selected
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_1" ) ).modelOutputs().value( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_1" ) ).modelOutputs().keys().at( 0 ) ).comment()->description(), QString() );
  QCOMPARE( algDest.groupBoxes().size(), 2 );
  QCOMPARE( algDest.groupBoxes().at( 0 ).description(), QStringLiteral( "group" ) );
  QCOMPARE( algDest.groupBoxes().at( 1 ).description(), QStringLiteral( "group" ) );

  // output and output comment
  view.copyItems( QList< QgsModelComponentGraphicItem * >() << algItem << outputItem << outputCommentItem, QgsModelGraphicsView::ClipboardCopy );
  viewDest.pasteItems( QgsModelGraphicsView::PasteModeInPlace );
  QCOMPARE( algDest.childAlgorithms().size(), 3 );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_2" ) ).modelOutputs().size(), 1 );
  QCOMPARE( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_2" ) ).modelOutputs().value( algDest.childAlgorithms().value( QStringLiteral( "native:buffer_2" ) ).modelOutputs().keys().at( 0 ) ).comment()->description(), QStringLiteral( "output comm" ) );
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
