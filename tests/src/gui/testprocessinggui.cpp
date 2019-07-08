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

#include "qgstest.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingmaplayercombobox.h"
#include "qgsnativealgorithms.h"
#include "processing/models/qgsprocessingmodelalgorithm.h"
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

class TestWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper
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

    QList< int > compatibleDataTypes() const override { return QList<int >(); }

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
    void testSetGetConfig();
    void testFilterAlgorithmConfig();
    void testWrapperFactoryRegistry();
    void testWrapperGeneral();
    void testWrapperDynamic();
    void testModelerWrapper();
    void testBooleanWrapper();
    void testStringWrapper();
    void testFileWrapper();
    void testAuthCfgWrapper();
    void testCrsWrapper();
    void testNumericWrapperDouble();
    void testNumericWrapperInt();
    void testDistanceWrapper();
    void testScaleWrapper();
    void testRangeWrapper();
    void testMatrixDialog();
    void testMatrixWrapper();
    void testExpressionWrapper();
    void testMultipleSelectionDialog();
    void testEnumSelectionPanel();
    void testEnumCheckboxPanel();
    void testEnumWrapper();
    void testLayoutWrapper();
    void testLayoutItemWrapper();
    void testPointPanel();
    void testPointWrapper();
    void testColorWrapper();
    void mapLayerComboBox();
    void paramConfigWidget();

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
  qputenv( "QGIS_AUTH_DB_DIR_PATH", mTempDir.toAscii() );

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
    qputenv( "QGIS_AUTH_PASSWORD_FILE", passfilepath.toAscii() );
  }
  // qDebug( "QGIS_AUTH_PASSWORD_FILE=%s", qgetenv( "QGIS_AUTH_PASSWORD_FILE" ).constData() );

  // re-init app and auth manager
  QgsApplication::quit();
  // QTest::qSleep( 3000 );
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

void TestProcessingGui::testSetGetConfig()
{
  const QList< const QgsProcessingAlgorithm * > algorithms = QgsApplication::instance()->processingRegistry()->algorithms();

  // Find all defined widgets for native algorithms
  // and get the default configuration (that is, we create a widget
  // and get the configuration it returns without being modified in any way)
  // We then set and get this configuration and validate that it matches the original one.
  for ( const QgsProcessingAlgorithm *algorithm : algorithms )
  {
    std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::instance()->processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );
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
  const QgsProcessingAlgorithm *algorithm = QgsApplication::instance()->processingRegistry()->algorithmById( QStringLiteral( "native:filter" ) );
  std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::instance()->processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );

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

  std::unique_ptr< QgsMapCanvas > mc = qgis::make_unique< QgsMapCanvas >();
  QgsProcessingParameterWidgetContext widgetContext;
  widgetContext.setMapCanvas( mc.get() );
  QCOMPARE( widgetContext.mapCanvas(), mc.get() );
  QgsProject p;
  widgetContext.setProject( &p );
  QCOMPARE( widgetContext.project(), &p );
  std::unique_ptr< QgsProcessingModelAlgorithm > model = qgis::make_unique< QgsProcessingModelAlgorithm >();
  widgetContext.setModel( model.get() );
  QCOMPARE( widgetContext.model(), model.get() );
  widgetContext.setModelChildAlgorithmId( QStringLiteral( "xx" ) );
  QCOMPARE( widgetContext.modelChildAlgorithmId(), QStringLiteral( "xx" ) );

  wrapper.setWidgetContext( widgetContext );
  QCOMPARE( wrapper.widgetContext().mapCanvas(), mc.get() );
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


class TestLayerWrapper : public QgsAbstractProcessingParameterWidgetWrapper
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
  algs.insert( QStringLiteral( "alg1" ), a1 );
  algs.insert( QStringLiteral( "alg2" ), a2 );
  algs.insert( QStringLiteral( "alg3" ), a3 );
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


  // widget tests
  w = new QgsProcessingModelerParameterWidget( &model, "alg1", model.parameterDefinition( "p1" ), context );
  QCOMPARE( w->parameterDefinition()->name(), QStringLiteral( "p1" ) );
  QLabel *l = w->createLabel();
  QVERIFY( l );
  QCOMPARE( l->text(), QStringLiteral( "desc" ) );
  QCOMPARE( l->toolTip(), w->parameterDefinition()->toolTip() );
  delete l;

  // static value
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( true ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().staticValue().toBool(), true );
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( false ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( w->value().staticValue().toBool(), false );
  QCOMPARE( w->mStackedWidget->currentIndex(), 0 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Value" ) );

  // expression value
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "1+2" ) ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( w->value().expression(), QStringLiteral( "1+2" ) );
  QCOMPARE( w->mStackedWidget->currentIndex(), 1 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Pre-calculated Value" ) );

  // model input - should fail, because we haven't populated sources yet, and so have no compatible sources
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QVERIFY( w->value().parameterName().isEmpty() );
  QCOMPARE( w->mStackedWidget->currentIndex(), 2 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Model Input" ) );

  // alg output  - should fail, because we haven't populated sources yet, and so have no compatible sources
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QVERIFY( w->value().outputChildId().isEmpty() );
  QCOMPARE( w->mStackedWidget->currentIndex(), 3 );
  QCOMPARE( w->mSourceButton->toolTip(), QStringLiteral( "Algorithm Output" ) );

  // populate sources and re-try
  w->populateSources( QStringList() << QStringLiteral( "boolean" ), QStringList() << QStringLiteral( "outputVector" ), QList<int>() );

  // model input
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( w->value().parameterName(), QStringLiteral( "p1" ) );

  // alg output
  w->setWidgetValue( QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "alg3" ), QStringLiteral( "OUTPUT" ) ) );
  QCOMPARE( w->value().source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( w->value().outputChildId(), QStringLiteral( "alg3" ) );
  QCOMPARE( w->value().outputName(), QStringLiteral( "OUTPUT" ) );

  delete w;

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
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "boolean" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterBoolean boolParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), true, false );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "boolean" ), context, widgetContext, &boolParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( static_cast< QgsProcessingParameterBoolean * >( def.get() )->defaultValue().toBool() );
  boolParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  boolParam.setDefaultValue( false );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "boolean" ), context, widgetContext, &boolParam );
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


  // config widget
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( !static_cast< QgsProcessingParameterString * >( def.get() )->multiLine() );

  // using a parameter definition as initial values
  QgsProcessingParameterString stringParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QStringLiteral( "aaa" ), true );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, &stringParam );
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
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, &stringParam );
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
    wrapper.setWidgetValue( TEST_DATA_DIR + QStringLiteral( "/points.shp" ), context );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( wrapper.widgetValue().toString(),  TEST_DATA_DIR + QStringLiteral( "/points.shp" ) );
    QCOMPARE( static_cast< QgsFileWidget * >( wrapper.wrappedWidget() )->filePath(),  TEST_DATA_DIR + QStringLiteral( "/points.shp" ) );
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
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterFile fileParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QgsProcessingParameterFile::File );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext, &fileParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->behavior(), QgsProcessingParameterFile::File );
  QVERIFY( !static_cast< QgsProcessingParameterFile * >( def.get() )->defaultValue().isValid() );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->fileFilter(), QStringLiteral( "All files (*.*)" ) );
  fileParam.setFileFilter( QStringLiteral( "TAB files (*.tab)" ) );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext, &fileParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( static_cast< QgsProcessingParameterFile * >( def.get() )->fileFilter(), QStringLiteral( "TAB files (*.tab)" ) );

  fileParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  fileParam.setBehavior( QgsProcessingParameterFile::Folder );
  fileParam.setDefaultValue( QStringLiteral( "my path" ) );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "file" ), context, widgetContext, &fileParam );
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
  for ( QgsAuthMethodConfig config : qgis::as_const( configs ) )
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
  std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  wrapper.setUnitParameterValue( QVariant::fromValue( vl.get() ) );
  QCOMPARE( wrapper.mLabel->text(), QStringLiteral( "meters" ) );
  QVERIFY( !wrapper.mWarningLabel->isVisible() );
  QVERIFY( wrapper.mUnitsCombo->isVisible() );
  QVERIFY( !wrapper.mLabel->isVisible() );
  QCOMPARE( wrapper.mUnitsCombo->currentData().toInt(), static_cast< int >( QgsUnitTypes::DistanceMeters ) );

  std::unique_ptr< QgsVectorLayer > vl2 = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
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
  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );
}

void TestProcessingGui::testMatrixDialog()
{
  QgsProcessingParameterMatrix matrixParam( QString(), QString(), 3, false, QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  std::unique_ptr< QgsProcessingMatrixParameterDialog > dlg = qgis::make_unique< QgsProcessingMatrixParameterDialog>( nullptr, nullptr, &matrixParam );
  // variable length table
  QVERIFY( dlg->mButtonAdd->isEnabled() );
  QVERIFY( dlg->mButtonRemove->isEnabled() );
  QVERIFY( dlg->mButtonRemoveAll->isEnabled() );

  QCOMPARE( dlg->table(), QVariantList() );

  dlg = qgis::make_unique< QgsProcessingMatrixParameterDialog >( nullptr, nullptr, &matrixParam, QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) );
  QCOMPARE( dlg->table(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) );
  dlg->addRow();
  QCOMPARE( dlg->table(), QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) << QString() << QString() );
  dlg->deleteAllRows();
  QCOMPARE( dlg->table(), QVariantList() );

  QgsProcessingParameterMatrix matrixParam2( QString(), QString(), 3, true, QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  dlg = qgis::make_unique< QgsProcessingMatrixParameterDialog >( nullptr, nullptr, &matrixParam2, QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ) << QStringLiteral( "d" ) << QStringLiteral( "e" ) << QStringLiteral( "f" ) );
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
}

void TestProcessingGui::testMultipleSelectionDialog()
{
  QVariantList availableOptions;
  QVariantList selectedOptions;
  std::unique_ptr< QgsProcessingMultipleSelectionDialog > dlg = qgis::make_unique< QgsProcessingMultipleSelectionDialog >( availableOptions, selectedOptions );
  QVERIFY( dlg->selectedOptions().isEmpty() );
  QCOMPARE( dlg->mModel->rowCount(), 0 );

  std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  availableOptions << QVariant( "aa" ) << QVariant( 15 ) << QVariant::fromValue( vl.get() );
  dlg = qgis::make_unique< QgsProcessingMultipleSelectionDialog >( availableOptions, selectedOptions );
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
  dlg = qgis::make_unique< QgsProcessingMultipleSelectionDialog >( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->rowCount(), 2 );
  QCOMPARE( dlg->selectedOptions(), selectedOptions );
  dlg->mModel->item( 1 )->setCheckState( Qt::Unchecked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "bb" );

  // mix of standard and additional options
  availableOptions << QVariant( 6.6 ) << QVariant( "aa" );
  dlg = qgis::make_unique< QgsProcessingMultipleSelectionDialog >( availableOptions, selectedOptions );
  QCOMPARE( dlg->mModel->rowCount(), 3 );
  QCOMPARE( dlg->selectedOptions(), selectedOptions ); // order must be maintained!
  dlg->mModel->item( 2 )->setCheckState( Qt::Checked );
  QCOMPARE( dlg->selectedOptions(), QVariantList() << "bb" << QVariant( 6.6 ) << QVariant( "aa" ) );

  // selection buttons
  selectedOptions.clear();
  availableOptions = QVariantList() << QVariant( "a" ) << QVariant( "b" ) << QVariant( "c" );
  dlg = qgis::make_unique< QgsProcessingMultipleSelectionDialog >( availableOptions, selectedOptions );
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
  dlg = qgis::make_unique< QgsProcessingMultipleSelectionDialog >( availableOptions, selectedOptions );
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
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "1 options selected" ) );

  w.setValue( QVariantList() << 2 << 0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( w.value().toList(), QVariantList() << 2 << 0 );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "2 options selected" ) );

  w.setValue( QVariant() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( w.value().toList(), QVariantList() );
  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "0 options selected" ) );
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

    // allow multiple
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
    QgsProcessingParameterEnum param4( QStringLiteral( "enum" ), QStringLiteral( "enum" ), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" ), true, QVariant(), false );
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

  };

  // standard wrapper
  testWrapper( QgsProcessingGui::Standard );

  // batch wrapper
  testWrapper( QgsProcessingGui::Batch );

  // modeler wrapper
  testWrapper( QgsProcessingGui::Modeler );

  // checkbox style (not for batch or model mode!)
  testWrapper( QgsProcessingGui::Standard, true );

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
      QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "l2" ) );
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
      QCOMPARE( static_cast< QLineEdit * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "l1" ) );
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
      QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(), QStringLiteral( "l2" ) );
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
      QCOMPARE( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text(), QStringLiteral( "l1" ) );
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
      QVERIFY( static_cast< QLineEdit * >( wrapper2.wrappedWidget() )->text().isEmpty() );
    }

    // check signal
    if ( type != QgsProcessingGui::Modeler )
      static_cast< QComboBox * >( wrapper2.wrappedWidget() )->setCurrentIndex( 2 );
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
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layoutitem" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  QgsProcessingParameterLayoutItem itemParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QVariant(), QStringLiteral( "parent" ) );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layoutitem" ), context, widgetContext, &itemParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterLayoutItem * >( def.get() )->parentLayoutParameterName(), QStringLiteral( "parent" ) );
  itemParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  itemParam.setParentLayoutParameterName( QString() );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "layoutitem" ), context, widgetContext, &itemParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QVERIFY( static_cast< QgsProcessingParameterLayoutItem * >( def.get() )->parentLayoutParameterName().isEmpty() );
}

void TestProcessingGui::testPointPanel()
{
  std::unique_ptr< QgsProcessingPointPanel > panel = qgis::make_unique< QgsProcessingPointPanel >( nullptr );
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
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "color" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QVERIFY( static_cast< QgsProcessingParameterColor * >( def.get() )->opacityEnabled() ); // should default to true

  // using a parameter definition as initial values
  QgsProcessingParameterColor colorParam( QStringLiteral( "n" ), QStringLiteral( "test desc" ), QColor( 255, 0, 0, 100 ), true );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "color" ), context, widgetContext, &colorParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  QCOMPARE( static_cast< QgsProcessingParameterColor * >( def.get() )->defaultValue().value< QColor >(), QColor( 255, 0, 0, 100 ) );
  QVERIFY( static_cast< QgsProcessingParameterColor * >( def.get() )->opacityEnabled() );
  colorParam.setFlags( QgsProcessingParameterDefinition::FlagAdvanced | QgsProcessingParameterDefinition::FlagOptional );
  colorParam.setOpacityEnabled( false );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "color" ), context, widgetContext, &colorParam );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  QCOMPARE( static_cast< QgsProcessingParameterColor * >( def.get() )->defaultValue().value< QColor >(), QColor( 255, 0, 0 ) ); // (no opacity!)
  QVERIFY( !static_cast< QgsProcessingParameterColor * >( def.get() )->opacityEnabled() );
}

void TestProcessingGui::mapLayerComboBox()
{
  QgsProject::instance()->removeAllMapLayers();
  QgsProcessingContext context;
  context.setProject( QgsProject::instance() );

  // feature source param
  std::unique_ptr< QgsProcessingParameterDefinition > param( new QgsProcessingParameterFeatureSource( QStringLiteral( "param" ), QString() ) );
  std::unique_ptr< QgsProcessingMapLayerComboBox> combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );

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
  // except "selected only" state to remain
  QVERIFY( combo->value().canConvert< QgsProcessingFeatureSourceDefinition >() );
  QCOMPARE( combo->value().value< QgsProcessingFeatureSourceDefinition >().source.staticValue().toString(), vl->id() );
  QVERIFY( combo->value().value< QgsProcessingFeatureSourceDefinition >().selectedFeaturesOnly );
  QVERIFY( combo->currentText().startsWith( vl->name() ) );
  QCOMPARE( spy.count(), 13 );
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

  // map layer param, all types are acceptable
  param = qgis::make_unique< QgsProcessingParameterMapLayer> ( QStringLiteral( "param" ), QString() );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // raster layer param, only raster types are acceptable
  param = qgis::make_unique< QgsProcessingParameterRasterLayer> ( QStringLiteral( "param" ), QString() );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // mesh layer parm, only mesh types are acceptable
  param = qgis::make_unique< QgsProcessingParameterMeshLayer> ( QStringLiteral( "param" ), QString() );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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
  combo.reset();
  param.reset();

  // feature source and vector layer params
  // if not specified, the default is any vector layer with geometry
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ) );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  auto param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ) );
  auto combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // point layer
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint );
  combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // line layer
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorLine );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorLine );
  combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // polygon
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPolygon );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPolygon );
  combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // no geom
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVector );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVector );
  combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // any geom
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorAnyGeometry );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorAnyGeometry );
  combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // combination point and line only
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
  param2 = qgis::make_unique< QgsProcessingParameterFeatureSource> ( QStringLiteral( "param" ), QString(), QList< int>() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine );
  combo2 = qgis::make_unique< QgsProcessingMapLayerComboBox >( param2.get() );
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
  combo2.reset();
  param2.reset();
  combo.reset();
  param.reset();

  // optional
  param = qgis::make_unique< QgsProcessingParameterVectorLayer> ( QStringLiteral( "param" ), QString(), QList< int>(), QVariant(), true );
  combo = qgis::make_unique< QgsProcessingMapLayerComboBox >( param.get() );
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

void TestProcessingGui::paramConfigWidget()
{
  QgsProcessingContext context;
  QgsProcessingParameterWidgetContext widgetContext;
  std::unique_ptr< QgsProcessingParameterDefinitionWidget > widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext );
  std::unique_ptr< QgsProcessingParameterDefinition > def( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) ); // should default to mandatory
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

  // using a parameter definition as initial values
  def->setDescription( QStringLiteral( "test desc" ) );
  def->setFlags( QgsProcessingParameterDefinition::FlagOptional );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, def.get() );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagOptional );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );
  def->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  widget = qgis::make_unique< QgsProcessingParameterDefinitionWidget >( QStringLiteral( "string" ), context, widgetContext, def.get() );
  def.reset( widget->createParameter( QStringLiteral( "param_name" ) ) );
  QCOMPARE( def->name(), QStringLiteral( "param_name" ) );
  QCOMPARE( def->description(), QStringLiteral( "test desc" ) );
  QVERIFY( !( def->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QVERIFY( def->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
}

void TestProcessingGui::cleanupTempDir()
{
  QDir tmpDir = QDir( mTempDir );
  if ( tmpDir.exists() )
  {
    Q_FOREACH ( const QString &tf, tmpDir.entryList( QDir::NoDotAndDotDot | QDir::Files ) )
    {
      QVERIFY2( tmpDir.remove( mTempDir + '/' + tf ), qPrintable( "Could not remove " + mTempDir + '/' + tf ) );
    }
    QVERIFY2( tmpDir.rmdir( mTempDir ), qPrintable( "Could not remove directory " + mTempDir ) );
  }
}

QGSTEST_MAIN( TestProcessingGui )
#include "testprocessinggui.moc"
