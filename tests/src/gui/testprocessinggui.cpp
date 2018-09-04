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

    void setWidgetValue( const QVariant &, const QgsProcessingContext & ) override
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
};

void TestProcessingGui::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
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

QGSTEST_MAIN( TestProcessingGui )
#include "testprocessinggui.moc"
