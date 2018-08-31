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

#include "qgstest.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsnativealgorithms.h"
#include "qgsxmlutils.h"

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
                       WidgetType type = Standard )
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

    QVariant value() const override
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
        QgsAbstractProcessingParameterWidgetWrapper::WidgetType type ) override
    {
      return new TestWidgetWrapper( parameter, type );
    }

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

  QVERIFY( !registry.createParameterWidgetWrapper( &numParam, QgsAbstractProcessingParameterWidgetWrapper::Standard ) );

  TestWidgetFactory *factory = new TestWidgetFactory( QStringLiteral( "str" ) );
  QVERIFY( registry.addParameterWidgetFactory( factory ) );

  // duplicate type not allowed
  TestWidgetFactory *factory2 = new TestWidgetFactory( QStringLiteral( "str" ) );
  QVERIFY( !registry.addParameterWidgetFactory( factory2 ) );
  delete factory2;

  QgsAbstractProcessingParameterWidgetWrapper *wrapper = registry.createParameterWidgetWrapper( &numParam, QgsAbstractProcessingParameterWidgetWrapper::Standard );
  QVERIFY( !wrapper );
  wrapper = registry.createParameterWidgetWrapper( &stringParam, QgsAbstractProcessingParameterWidgetWrapper::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), QStringLiteral( "str" ) );
  delete wrapper;

  TestWidgetFactory *factory3 = new TestWidgetFactory( QStringLiteral( "num" ) );
  QVERIFY( registry.addParameterWidgetFactory( factory3 ) );

  wrapper = registry.createParameterWidgetWrapper( &numParam, QgsAbstractProcessingParameterWidgetWrapper::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), QStringLiteral( "num" ) );
  delete wrapper;

  // removing
  registry.removeParameterWidgetFactory( nullptr );
  TestWidgetFactory *factory4 = new TestWidgetFactory( QStringLiteral( "xxxx" ) );
  registry.removeParameterWidgetFactory( factory4 );
  registry.removeParameterWidgetFactory( factory );
  wrapper = registry.createParameterWidgetWrapper( &stringParam, QgsAbstractProcessingParameterWidgetWrapper::Standard );
  QVERIFY( !wrapper );

  wrapper = registry.createParameterWidgetWrapper( &numParam, QgsAbstractProcessingParameterWidgetWrapper::Standard );
  QVERIFY( wrapper );
  QCOMPARE( wrapper->parameterDefinition()->type(), QStringLiteral( "num" ) );
  delete wrapper;
}

void TestProcessingGui::testWrapperGeneral()
{
  TestParamType param( QStringLiteral( "boolean" ), QStringLiteral( "bool" ) );
  QgsProcessingBooleanWidgetWrapper wrapper( &param );
  QCOMPARE( wrapper.type(), QgsAbstractProcessingParameterWidgetWrapper::Standard );
  QgsProcessingBooleanWidgetWrapper wrapper2( &param, QgsAbstractProcessingParameterWidgetWrapper::Batch );
  QCOMPARE( wrapper2.type(), QgsAbstractProcessingParameterWidgetWrapper::Batch );

  QCOMPARE( wrapper2.parameterDefinition()->name(), QStringLiteral( "bool" ) );

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

  // check that created widget starts with default value
  param = TestParamType( QStringLiteral( "boolean" ), QStringLiteral( "bool" ), true );
  QgsProcessingBooleanWidgetWrapper trueDefault( &param );
  w = trueDefault.createWrappedWidget( context );
  QVERIFY( trueDefault.value().toBool() );
  delete w;
  param = TestParamType( QStringLiteral( "boolean" ), QStringLiteral( "bool" ), false );
  QgsProcessingBooleanWidgetWrapper falseDefault( &param );
  w = falseDefault.createWrappedWidget( context );
  QVERIFY( !falseDefault.value().toBool() );
  delete w;
}

void TestProcessingGui::testBooleanWrapper()
{
  TestParamType param( QStringLiteral( "boolean" ), QStringLiteral( "bool" ) );

  // standard wrapper
  QgsProcessingBooleanWidgetWrapper wrapper( &param );

  QgsProcessingContext context;
  QWidget *w = wrapper.createWrappedWidget( context );
  wrapper.setWidgetValue( true, context );
  QVERIFY( wrapper.value().toBool() );
  QVERIFY( static_cast< QCheckBox * >( wrapper.wrappedWidget() )->isChecked() );
  wrapper.setWidgetValue( false, context );
  QVERIFY( !wrapper.value().toBool() );
  QVERIFY( !static_cast< QCheckBox * >( wrapper.wrappedWidget() )->isChecked() );

  // should be no label in standard mode
  QVERIFY( !wrapper.createWrappedLabel() );
  QCOMPARE( static_cast< QCheckBox * >( wrapper.wrappedWidget() )->text(), QStringLiteral( "bool" ) );
  delete w;

  // batch wrapper
  QgsProcessingBooleanWidgetWrapper wrapperB( &param, QgsAbstractProcessingParameterWidgetWrapper::Batch );

  w = wrapperB.createWrappedWidget( context );
  wrapperB.setWidgetValue( true, context );
  QVERIFY( wrapperB.value().toBool() );
  QVERIFY( static_cast< QComboBox * >( wrapperB.wrappedWidget() )->currentData().toBool() );
  wrapperB.setWidgetValue( false, context );
  QVERIFY( !wrapperB.value().toBool() );
  QVERIFY( !static_cast< QComboBox * >( wrapperB.wrappedWidget() )->currentData().toBool() );

  // should be no label in batch mode
  QVERIFY( !wrapperB.createWrappedLabel() );
  delete w;
}

QGSTEST_MAIN( TestProcessingGui )
#include "testprocessinggui.moc"
