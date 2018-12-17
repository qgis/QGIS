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
#include <QPlainTextEdit>

#include "qgstest.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingmodelerparameterwidget.h"
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
#include "models/qgsprocessingmodelalgorithm.h"

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
    void testAuthCfgWrapper();
    void testCrsWrapper();
    void testNumericWrapperDouble();
    void testNumericWrapperInt();
    void testDistanceWrapper();
    void testRangeWrapper();

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
  QgsProcessingBooleanWidgetWrapper wrapper( &param );
  QCOMPARE( wrapper.type(), QgsProcessingGui::Standard );

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

  QVERIFY( !allPartsWrapper.mPropertyButton->vectorLayer() );
  allPartsWrapper.setDynamicParentLayerParameter( QVariant::fromValue( vl ) );
  QCOMPARE( allPartsWrapper.mPropertyButton->vectorLayer(), vl );
  // should not be owned by wrapper
  QVERIFY( !allPartsWrapper.mDynamicLayer.get() );
  allPartsWrapper.setDynamicParentLayerParameter( QVariant() );
  QVERIFY( !allPartsWrapper.mPropertyButton->vectorLayer() );

  allPartsWrapper.setDynamicParentLayerParameter( vl->id() );
  QVERIFY( !allPartsWrapper.mPropertyButton->vectorLayer() );
  QVERIFY( !allPartsWrapper.mDynamicLayer.get() );

  // with project layer
  context.setProject( &p );
  TestProcessingContextGenerator generator( context );
  allPartsWrapper.registerProcessingContextGenerator( &generator );

  allPartsWrapper.setDynamicParentLayerParameter( vl->id() );
  QCOMPARE( allPartsWrapper.mPropertyButton->vectorLayer(), vl );
  QVERIFY( !allPartsWrapper.mDynamicLayer.get() );

  // non-project layer
  QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
  allPartsWrapper.setDynamicParentLayerParameter( pointFileName );
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
