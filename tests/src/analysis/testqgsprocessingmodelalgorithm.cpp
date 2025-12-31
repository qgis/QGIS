/***************************************************************************
                         testqgsprocessingmodelalgorithm.cpp
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
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsnativealgorithms.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingregistry.h"
#include "qgstest.h"
#include "qgsxmlutils.h"

#include <QObject>

class DummyAlgorithm2 : public QgsProcessingAlgorithm
{
  public:
    DummyAlgorithm2( const QString &name )
      : mName( name ) { mFlags = QgsProcessingAlgorithm::flags(); }

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override
    {
      addParameter( new QgsProcessingParameterVectorDestination( u"vector_dest"_s ) );
      addParameter( new QgsProcessingParameterRasterDestination( u"raster_dest"_s ) );
      addParameter( new QgsProcessingParameterFeatureSink( u"sink"_s ) );
    }
    QString name() const override { return mName; }
    QString displayName() const override { return mName; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override { return QVariantMap(); }

    Qgis::ProcessingAlgorithmFlags flags() const override { return mFlags; }
    DummyAlgorithm2 *createInstance() const override { return new DummyAlgorithm2( name() ); }

    QString mName;

    Qgis::ProcessingAlgorithmFlags mFlags;
};


class DummySecurityRiskAlgorithm : public QgsProcessingAlgorithm
{
  public:
    DummySecurityRiskAlgorithm( const QString &name )
      : mName( name ) {}

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override
    {
      addParameter( new QgsProcessingParameterVectorDestination( u"vector_dest"_s ) );
    }
    QString name() const override { return mName; }
    QString displayName() const override { return mName; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override { return QVariantMap(); }

    Qgis::ProcessingAlgorithmFlags flags() const override { return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::SecurityRisk; }
    DummySecurityRiskAlgorithm *createInstance() const override { return new DummySecurityRiskAlgorithm( name() ); }

    QString mName;
};

class DummyRaiseExceptionAlgorithm : public QgsProcessingAlgorithm
{
  public:
    DummyRaiseExceptionAlgorithm( const QString &name )
      : mName( name )
    {
      mFlags = QgsProcessingAlgorithm::flags();
      hasPostProcessed = false;
    }
    static bool hasPostProcessed;
    ~DummyRaiseExceptionAlgorithm() override
    {
      hasPostProcessed |= mHasPostProcessed;
    }

    void initAlgorithm( const QVariantMap & = QVariantMap() ) override
    {
    }
    QString name() const override { return mName; }
    QString displayName() const override { return mName; }
    QVariantMap processAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      throw QgsProcessingException( u"something bad happened"_s );
    }
    static bool postProcessAlgorithmCalled;
    QVariantMap postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * ) final
    {
      postProcessAlgorithmCalled = true;
      return QVariantMap();
    }

    Qgis::ProcessingAlgorithmFlags flags() const override { return mFlags; }
    DummyRaiseExceptionAlgorithm *createInstance() const override { return new DummyRaiseExceptionAlgorithm( name() ); }

    QString mName;

    Qgis::ProcessingAlgorithmFlags mFlags;
};
bool DummyRaiseExceptionAlgorithm::hasPostProcessed = false;
bool DummyRaiseExceptionAlgorithm::postProcessAlgorithmCalled = false;

class DummyProvider4 : public QgsProcessingProvider // clazy:exclude=missing-qobject-macro
{
  public:
    DummyProvider4() = default;
    QString id() const override { return u"dummy4"_s; }
    QString name() const override { return u"dummy4"_s; }

    bool supportsNonFileBasedOutput() const override
    {
      return false;
    }

    QStringList supportedOutputVectorLayerExtensions() const override
    {
      return QStringList() << u"mif"_s;
    }

    QList<QPair<QString, QString>> supportedOutputRasterLayerFormatAndExtensions() const override
    {
      return QList<QPair<QString, QString>>() << QPair<QString, QString>( u"XYZ"_s, u"xyz"_s );
    }

    void loadAlgorithms() override
    {
      QVERIFY( addAlgorithm( new DummyAlgorithm2( u"alg1"_s ) ) );
      QVERIFY( addAlgorithm( new DummyRaiseExceptionAlgorithm( u"raise"_s ) ) );
      QVERIFY( addAlgorithm( new DummySecurityRiskAlgorithm( u"risky"_s ) ) );

      QgsProcessingModelAlgorithm model;
      model.setName( "dummymodel" );

      const QgsProcessingModelParameter sourceParam( "test" );
      model.addModelParameter( new QgsProcessingParameterString( "test" ), sourceParam );

      QgsProcessingModelChildAlgorithm childAlgorithm;
      childAlgorithm.setChildId( u"calculate"_s );
      childAlgorithm.setAlgorithmId( "native:calculateexpression" );
      childAlgorithm.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromExpression( "'value_from_child'" ) } );
      QgsProcessingModelOutput childOutput( "OUTPUT" );
      childOutput.setChildId( u"calculate"_s );
      childOutput.setChildOutputName( "OUTPUT" );
      childOutput.setDescription( u"my output"_s );
      childAlgorithm.setModelOutputs( { { u"OUTPUT"_s, childOutput } } );
      model.addChildAlgorithm( childAlgorithm );

      QVERIFY( addAlgorithm( model.create() ) );
    }
};


class TestQgsProcessingModelAlgorithm : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingModelAlgorithm()
      : QgsTest( u"Processing Model Test"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void modelScope();
    void modelerAlgorithm();
    void modelExecution();
    void modelBranchPruning();
    void modelBranchPruningConditional();
    void modelWithProviderWithLimitedTypes();
    void modelVectorOutputIsCompatibleType();
    void modelAcceptableValues();
    void modelValidate();
    void modelInputs();
    void modelOutputs();
    void modelWithChildException();
    void modelExecuteWithPreviousState();
    void modelExecuteWithPreviousStateNoLeak();
    void modelDependencies();
    void modelSource();
    void modelNameMatchesFileName();
    void renameModelParameter();
    void internalVersion();
    void modelChildOrderWithVariables();
    void flags();
    void modelWithDuplicateNames();

  private:
};

void TestQgsProcessingModelAlgorithm::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QgsSettings settings;
  settings.clear();

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
  QgsApplication::processingRegistry()->addProvider( new DummyProvider4() );
}

void TestQgsProcessingModelAlgorithm::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingModelAlgorithm::modelScope()
{
  QgsProcessingContext pc;

  QgsProcessingModelAlgorithm alg( u"test"_s, u"testGroup"_s );

  QVERIFY( !alg.fromFile( u"not a file"_s ) );

  QVariantMap variables;
  variables.insert( u"v1"_s, 5 );
  variables.insert( u"v2"_s, u"aabbccd"_s );
  alg.setVariables( variables );

  QVariantMap params;
  params.insert( u"a_param"_s, 5 );
  std::unique_ptr<QgsExpressionContextScope> scope( QgsExpressionContextUtils::processingModelAlgorithmScope( &alg, params, pc ) );
  QVERIFY( scope.get() );
  QCOMPARE( scope->variable( u"model_name"_s ).toString(), u"test"_s );
  QCOMPARE( scope->variable( u"model_group"_s ).toString(), u"testGroup"_s );
  QVERIFY( scope->hasVariable( u"model_path"_s ) );
  QVERIFY( scope->hasVariable( u"model_folder"_s ) );
  QCOMPARE( scope->variable( u"model_path"_s ).toString(), QString() );
  QCOMPARE( scope->variable( u"model_folder"_s ).toString(), QString() );
  QCOMPARE( scope->variable( u"v1"_s ).toInt(), 5 );
  QCOMPARE( scope->variable( u"v2"_s ).toString(), u"aabbccd"_s );

  QgsProject p;
  pc.setProject( &p );
  p.setFileName( TEST_DATA_DIR + u"/test_file.qgs"_s );
  scope.reset( QgsExpressionContextUtils::processingModelAlgorithmScope( &alg, params, pc ) );
  QCOMPARE( scope->variable( u"model_path"_s ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + u"/test_file.qgs"_s ) );
  QCOMPARE( scope->variable( u"model_folder"_s ).toString(), QStringLiteral( TEST_DATA_DIR ) );

  alg.setSourceFilePath( TEST_DATA_DIR + u"/processing/my_model.model3"_s );
  scope.reset( QgsExpressionContextUtils::processingModelAlgorithmScope( &alg, params, pc ) );
  QCOMPARE( scope->variable( u"model_path"_s ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + u"/processing/my_model.model3"_s ) );
  QCOMPARE( scope->variable( u"model_folder"_s ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + u"/processing"_s ) );

  const QgsExpressionContext ctx = alg.createExpressionContext( QVariantMap(), pc );
  QVERIFY( scope->hasVariable( u"model_path"_s ) );
  QVERIFY( scope->hasVariable( u"model_folder"_s ) );
}

void TestQgsProcessingModelAlgorithm::modelerAlgorithm()
{
  //static value source
  QgsProcessingModelChildParameterSource svSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  QCOMPARE( svSource.source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( svSource.staticValue().toInt(), 5 );
  svSource.setStaticValue( 7 );
  QCOMPARE( svSource.staticValue().toInt(), 7 );
  QMap<QString, QString> friendlyNames;
  QCOMPARE( svSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"7"_s );
  svSource = QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s );
  // check that calling setStaticValue flips source to StaticValue
  QCOMPARE( svSource.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( svSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"parameters['a']"_s );
  svSource.setStaticValue( 7 );
  QCOMPARE( svSource.staticValue().toInt(), 7 );
  QCOMPARE( svSource.source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( svSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"7"_s );

  // model parameter source
  QgsProcessingModelChildParameterSource mpSource = QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s );
  QCOMPARE( mpSource.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( mpSource.parameterName(), u"a"_s );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"parameters['a']"_s );
  mpSource.setParameterName( u"b"_s );
  QCOMPARE( mpSource.parameterName(), u"b"_s );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"parameters['b']"_s );
  mpSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setParameterName flips source to ModelParameter
  QCOMPARE( mpSource.source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"5"_s );
  mpSource.setParameterName( u"c"_s );
  QCOMPARE( mpSource.parameterName(), u"c"_s );
  QCOMPARE( mpSource.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"parameters['c']"_s );

  // child alg output source
  QgsProcessingModelChildParameterSource oSource = QgsProcessingModelChildParameterSource::fromChildOutput( u"a"_s, u"b"_s );
  QCOMPARE( oSource.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( oSource.outputChildId(), u"a"_s );
  QCOMPARE( oSource.outputName(), u"b"_s );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"outputs['a']['b']"_s );
  // with friendly name
  friendlyNames.insert( u"a"_s, u"alga"_s );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"outputs['alga']['b']"_s );
  oSource.setOutputChildId( u"c"_s );
  QCOMPARE( oSource.outputChildId(), u"c"_s );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"outputs['c']['b']"_s );
  oSource.setOutputName( u"d"_s );
  QCOMPARE( oSource.outputName(), u"d"_s );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"outputs['c']['d']"_s );
  oSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setOutputChildId flips source to ChildOutput
  QCOMPARE( oSource.source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"5"_s );
  oSource.setOutputChildId( u"c"_s );
  QCOMPARE( oSource.outputChildId(), u"c"_s );
  QCOMPARE( oSource.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  oSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setOutputName flips source to ChildOutput
  QCOMPARE( oSource.source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"5"_s );
  oSource.setOutputName( u"d"_s );
  QCOMPARE( oSource.outputName(), u"d"_s );
  QCOMPARE( oSource.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );

  // expression source
  QgsProcessingModelChildParameterSource expSource = QgsProcessingModelChildParameterSource::fromExpression( u"1+2"_s );
  QCOMPARE( expSource.source(), Qgis::ProcessingModelChildParameterSource::Expression );
  QCOMPARE( expSource.expression(), u"1+2"_s );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"QgsExpression('1+2').evaluate()"_s );
  expSource.setExpression( u"1+3"_s );
  QCOMPARE( expSource.expression(), u"1+3"_s );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"QgsExpression('1+3').evaluate()"_s );
  expSource.setExpression( u"'a' || 'b\\'c'"_s );
  QCOMPARE( expSource.expression(), u"'a' || 'b\\'c'"_s );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"QgsExpression(\"'a' || 'b\\\\'c'\").evaluate()"_s );
  expSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setExpression flips source to Expression
  QCOMPARE( expSource.source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"5"_s );
  expSource.setExpression( u"1+4"_s );
  QCOMPARE( expSource.expression(), u"1+4"_s );
  QCOMPARE( expSource.source(), Qgis::ProcessingModelChildParameterSource::Expression );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), u"QgsExpression('1+4').evaluate()"_s );

  // source equality operator
  QVERIFY( QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) == QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) != QgsProcessingModelChildParameterSource::fromStaticValue( 7 ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) != QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s ) == QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s ) != QgsProcessingModelChildParameterSource::fromModelParameter( u"b"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromModelParameter( u"a"_s ) != QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) == QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) != QgsProcessingModelChildParameterSource::fromChildOutput( u"alg2"_s, u"out"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out"_s ) != QgsProcessingModelChildParameterSource::fromChildOutput( u"alg"_s, u"out2"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromExpression( u"a"_s ) == QgsProcessingModelChildParameterSource::fromExpression( u"a"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromExpression( u"a"_s ) != QgsProcessingModelChildParameterSource::fromExpression( u"b"_s ) );
  QVERIFY( QgsProcessingModelChildParameterSource::fromExpression( u"a"_s ) != QgsProcessingModelChildParameterSource::fromStaticValue( u"b"_s ) );

  // a comment
  QgsProcessingModelComment comment;
  comment.setSize( QSizeF( 9, 8 ) );
  QCOMPARE( comment.size(), QSizeF( 9, 8 ) );
  comment.setPosition( QPointF( 11, 14 ) );
  QCOMPARE( comment.position(), QPointF( 11, 14 ) );
  comment.setDescription( u"a comment"_s );
  QCOMPARE( comment.description(), u"a comment"_s );
  comment.setColor( QColor( 123, 45, 67 ) );
  QCOMPARE( comment.color(), QColor( 123, 45, 67 ) );
  std::unique_ptr<QgsProcessingModelComment> commentClone( comment.clone() );
  QCOMPARE( commentClone->toVariant(), comment.toVariant() );
  QCOMPARE( commentClone->size(), QSizeF( 9, 8 ) );
  QCOMPARE( commentClone->position(), QPointF( 11, 14 ) );
  QCOMPARE( commentClone->description(), u"a comment"_s );
  QCOMPARE( commentClone->color(), QColor( 123, 45, 67 ) );
  QgsProcessingModelComment comment2;
  comment2.loadVariant( comment.toVariant().toMap() );
  QCOMPARE( comment2.size(), QSizeF( 9, 8 ) );
  QCOMPARE( comment2.position(), QPointF( 11, 14 ) );
  QCOMPARE( comment2.description(), u"a comment"_s );
  QCOMPARE( comment2.color(), QColor( 123, 45, 67 ) );

  // group boxes
  QgsProcessingModelGroupBox groupBox;
  groupBox.setSize( QSizeF( 9, 8 ) );
  QCOMPARE( groupBox.size(), QSizeF( 9, 8 ) );
  groupBox.setPosition( QPointF( 11, 14 ) );
  QCOMPARE( groupBox.position(), QPointF( 11, 14 ) );
  groupBox.setDescription( u"a comment"_s );
  QCOMPARE( groupBox.description(), u"a comment"_s );
  groupBox.setColor( QColor( 123, 45, 67 ) );
  QCOMPARE( groupBox.color(), QColor( 123, 45, 67 ) );
  std::unique_ptr<QgsProcessingModelGroupBox> groupClone( groupBox.clone() );
  QCOMPARE( groupClone->toVariant(), groupBox.toVariant() );
  QCOMPARE( groupClone->size(), QSizeF( 9, 8 ) );
  QCOMPARE( groupClone->position(), QPointF( 11, 14 ) );
  QCOMPARE( groupClone->description(), u"a comment"_s );
  QCOMPARE( groupClone->color(), QColor( 123, 45, 67 ) );
  QCOMPARE( groupClone->uuid(), groupBox.uuid() );
  QgsProcessingModelGroupBox groupBox2;
  groupBox2.loadVariant( groupBox.toVariant().toMap() );
  QCOMPARE( groupBox2.size(), QSizeF( 9, 8 ) );
  QCOMPARE( groupBox2.position(), QPointF( 11, 14 ) );
  QCOMPARE( groupBox2.description(), u"a comment"_s );
  QCOMPARE( groupBox2.color(), QColor( 123, 45, 67 ) );
  QCOMPARE( groupBox2.uuid(), groupBox.uuid() );

  const QMap<QString, QString> friendlyOutputNames;
  QgsProcessingModelChildAlgorithm child( u"some_id"_s );
  QCOMPARE( child.algorithmId(), u"some_id"_s );
  QVERIFY( !child.algorithm() );
  QVERIFY( !child.setAlgorithmId( u"blah"_s ) );
  QVERIFY( !child.reattach() );
  QVERIFY( child.setAlgorithmId( u"native:centroids"_s ) );
  QVERIFY( child.algorithm() );
  QCOMPARE( child.algorithm()->id(), u"native:centroids"_s );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, QgsStringMap(), 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    alg_params = {\n    }\n    outputs[''] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)"_s );
  QgsStringMap extraParams;
  extraParams[u"SOMETHING"_s] = u"SOMETHING_ELSE"_s;
  extraParams[u"SOMETHING2"_s] = u"SOMETHING_ELSE2"_s;
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    alg_params = {\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs[''] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)"_s );
  // bit of a hack -- but try to simulate an algorithm not originally available!
  child.mAlgorithm.reset();
  QVERIFY( !child.algorithm() );
  QVERIFY( child.reattach() );
  QVERIFY( child.algorithm() );
  QCOMPARE( child.algorithm()->id(), u"native:centroids"_s );

  QVariantMap myConfig;
  myConfig.insert( u"some_key"_s, 11 );
  child.setConfiguration( myConfig );
  QCOMPARE( child.configuration(), myConfig );

  child.setDescription( u"desc"_s );
  QCOMPARE( child.description(), u"desc"_s );
  QVERIFY( child.isActive() );
  child.setActive( false );
  QVERIFY( !child.isActive() );
  child.setPosition( QPointF( 1, 2 ) );
  QCOMPARE( child.position(), QPointF( 1, 2 ) );
  child.setSize( QSizeF( 3, 4 ) );
  QCOMPARE( child.size(), QSizeF( 3, 4 ) );
  QVERIFY( child.linksCollapsed( Qt::TopEdge ) );
  child.setLinksCollapsed( Qt::TopEdge, false );
  QVERIFY( !child.linksCollapsed( Qt::TopEdge ) );
  QVERIFY( child.linksCollapsed( Qt::BottomEdge ) );
  child.setLinksCollapsed( Qt::BottomEdge, false );
  QVERIFY( !child.linksCollapsed( Qt::BottomEdge ) );
  child.comment()->setDescription( u"com"_s );
  QCOMPARE( child.comment()->description(), u"com"_s );
  child.comment()->setSize( QSizeF( 56, 78 ) );
  child.comment()->setPosition( QPointF( 111, 122 ) );

  QgsProcessingModelChildAlgorithm other;
  other.setChildId( u"diff"_s );
  other.setDescription( u"d2"_s );
  other.setAlgorithmId( u"alg33"_s );
  other.setLinksCollapsed( Qt::BottomEdge, true );
  other.setLinksCollapsed( Qt::TopEdge, true );
  other.comment()->setDescription( u"other comment"_s );
  other.copyNonDefinitionProperties( child );
  // only subset of properties should have been copied!
  QCOMPARE( other.description(), u"d2"_s );
  QCOMPARE( other.position(), QPointF( 1, 2 ) );
  QCOMPARE( other.size(), QSizeF( 3, 4 ) );
  QVERIFY( !other.linksCollapsed( Qt::TopEdge ) );
  QVERIFY( !other.linksCollapsed( Qt::BottomEdge ) );
  QCOMPARE( other.comment()->description(), u"other comment"_s );
  QCOMPARE( other.comment()->position(), QPointF( 111, 122 ) );
  QCOMPARE( other.comment()->size(), QSizeF( 56, 78 ) );

  child.comment()->setDescription( QString() );

  child.setChildId( u"my_id"_s );
  QCOMPARE( child.childId(), u"my_id"_s );

  child.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( u"a"_s ) << QgsProcessingModelChildDependency( u"b"_s ) );
  QCOMPARE( child.dependencies(), QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( u"a"_s ) << QgsProcessingModelChildDependency( u"b"_s ) );

  QMap<QString, QgsProcessingModelChildParameterSources> sources;
  sources.insert( u"a"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  child.setParameterSources( sources );
  QCOMPARE( child.parameterSources().value( u"a"_s ).at( 0 ).staticValue().toInt(), 5 );
  child.addParameterSources( u"b"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 7 ) << QgsProcessingModelChildParameterSource::fromStaticValue( 9 ) );
  QCOMPARE( child.parameterSources().value( u"a"_s ).at( 0 ).staticValue().toInt(), 5 );
  QCOMPARE( child.parameterSources().value( u"b"_s ).count(), 2 );
  QCOMPARE( child.parameterSources().value( u"b"_s ).at( 0 ).staticValue().toInt(), 7 );
  QCOMPARE( child.parameterSources().value( u"b"_s ).at( 1 ).staticValue().toInt(), 9 );

  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    # desc\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)"_s );
  child.comment()->setDescription( u"do\nsomething\n\nuseful"_s );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    # desc\n    # do\n    # something\n    # \n    # useful\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)"_s );
  child.comment()->setDescription( u"do something useful"_s );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)"_s );

  std::unique_ptr<QgsProcessingModelChildAlgorithm> childClone( child.clone() );
  QCOMPARE( childClone->toVariant(), child.toVariant() );
  QCOMPARE( childClone->comment()->description(), u"do something useful"_s );

  QgsProcessingModelOutput testModelOut;
  testModelOut.setChildId( u"my_id"_s );
  QCOMPARE( testModelOut.childId(), u"my_id"_s );
  testModelOut.setChildOutputName( u"my_output"_s );
  QCOMPARE( testModelOut.childOutputName(), u"my_output"_s );
  testModelOut.setDefaultValue( u"my_value"_s );
  QCOMPARE( testModelOut.defaultValue().toString(), u"my_value"_s );
  testModelOut.setMandatory( true );
  QVERIFY( testModelOut.isMandatory() );
  testModelOut.comment()->setDescription( u"my comm"_s );
  QCOMPARE( testModelOut.comment()->description(), u"my comm"_s );
  std::unique_ptr<QgsProcessingModelOutput> outputClone( testModelOut.clone() );
  QCOMPARE( outputClone->toVariant(), testModelOut.toVariant() );
  QCOMPARE( outputClone->comment()->description(), u"my comm"_s );
  QgsProcessingModelOutput testModelOutV;
  testModelOutV.loadVariant( testModelOut.toVariant().toMap() );
  QCOMPARE( testModelOutV.comment()->description(), u"my comm"_s );

  QgsProcessingOutputLayerDefinition layerDef( u"my_path"_s );
  layerDef.createOptions[u"fileEncoding"_s] = u"my_encoding"_s;
  testModelOut.setDefaultValue( layerDef );
  QCOMPARE( testModelOut.defaultValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), u"my_path"_s );
  QVariantMap map = testModelOut.toVariant().toMap();
  QCOMPARE( map[u"default_value"_s].toMap()["sink"].toMap()["val"].toString(), u"my_path"_s );
  QCOMPARE( map["default_value"].toMap()["create_options"].toMap()[u"fileEncoding"_s].toString(), u"my_encoding"_s );
  QgsProcessingModelOutput out;
  out.loadVariant( map );
  QCOMPARE( out.defaultValue().userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  layerDef = out.defaultValue().value<QgsProcessingOutputLayerDefinition>();
  QCOMPARE( layerDef.sink.staticValue().toString(), u"my_path"_s );
  QCOMPARE( layerDef.createOptions[u"fileEncoding"_s].toString(), u"my_encoding"_s );

  QMap<QString, QgsProcessingModelOutput> outputs;
  QgsProcessingModelOutput out1;
  out1.setDescription( u"my output"_s );
  outputs.insert( u"a"_s, out1 );
  child.setModelOutputs( outputs );
  QCOMPARE( child.modelOutputs().count(), 1 );
  QCOMPARE( child.modelOutputs().value( u"a"_s ).description(), u"my output"_s );
  QCOMPARE( child.modelOutput( u"a"_s ).description(), u"my output"_s );
  child.modelOutput( "a" ).setDescription( u"my output 2"_s );
  QCOMPARE( child.modelOutput( "a" ).description(), u"my output 2"_s );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n    results['my_id:a'] = outputs['my_id']['']"_s );

  // ensure friendly name is used if present
  child.addParameterSources( u"b"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "a", "out" ) );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': outputs['alga']['out'],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n    results['my_id:a'] = outputs['my_id']['']"_s );
  friendlyNames.remove( u"a"_s );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), u"    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': outputs['a']['out'],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n    results['my_id:a'] = outputs['my_id']['']"_s );

  // no existent
  child.modelOutput( u"b"_s ).setDescription( u"my output 3"_s );
  QCOMPARE( child.modelOutput( u"b"_s ).description(), u"my output 3"_s );
  QCOMPARE( child.modelOutputs().count(), 2 );
  child.removeModelOutput( u"a"_s );
  QCOMPARE( child.modelOutputs().count(), 1 );

  // model algorithm tests

  QgsProcessingModelAlgorithm alg( "test", "testGroup" );
  QCOMPARE( alg.name(), u"test"_s );
  QCOMPARE( alg.displayName(), u"test"_s );
  QCOMPARE( alg.group(), u"testGroup"_s );
  alg.setName( u"test2"_s );
  QCOMPARE( alg.name(), u"test2"_s );
  QCOMPARE( alg.displayName(), u"test2"_s );
  alg.setGroup( u"group2"_s );
  QCOMPARE( alg.group(), u"group2"_s );

  QVariantMap help;
  alg.setHelpContent( help );
  QVERIFY( alg.helpContent().isEmpty() );
  QVERIFY( alg.helpUrl().isEmpty() );
  QVERIFY( alg.shortDescription().isEmpty() );
  help.insert( u"SHORT_DESCRIPTION"_s, u"short"_s );
  help.insert( u"HELP_URL"_s, u"url"_s );
  alg.setHelpContent( help );
  QCOMPARE( alg.helpContent(), help );
  QCOMPARE( alg.shortDescription(), u"short"_s );
  QCOMPARE( alg.helpUrl(), u"url"_s );

  QVERIFY( alg.groupBoxes().isEmpty() );
  alg.addGroupBox( groupBox );
  QCOMPARE( alg.groupBoxes().size(), 1 );
  QCOMPARE( alg.groupBoxes().at( 0 ).uuid(), groupBox.uuid() );
  QCOMPARE( alg.groupBoxes().at( 0 ).uuid(), groupBox.uuid() );
  alg.removeGroupBox( u"a"_s );
  QCOMPARE( alg.groupBoxes().size(), 1 );
  alg.removeGroupBox( groupBox.uuid() );
  QVERIFY( alg.groupBoxes().isEmpty() );


  QVariantMap lastParams;
  lastParams.insert( u"a"_s, 2 );
  lastParams.insert( u"b"_s, 4 );
  alg.setDesignerParameterValues( lastParams );
  QCOMPARE( alg.designerParameterValues(), lastParams );

  // child algorithms
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1;
  a1.setDescription( u"alg1"_s );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( u"alg2"_s );
  a2.setPosition( QPointF( 112, 131 ) );
  a2.setSize( QSizeF( 44, 55 ) );
  a2.comment()->setSize( QSizeF( 111, 222 ) );
  a2.comment()->setPosition( QPointF( 113, 114 ) );
  a2.comment()->setDescription( u"c"_s );
  a2.comment()->setColor( QColor( 255, 254, 253 ) );
  QgsProcessingModelOutput oo;
  oo.setPosition( QPointF( 312, 331 ) );
  oo.setSize( QSizeF( 344, 355 ) );
  oo.comment()->setSize( QSizeF( 311, 322 ) );
  oo.comment()->setPosition( QPointF( 313, 314 ) );
  oo.comment()->setDescription( u"c3"_s );
  oo.comment()->setColor( QColor( 155, 14, 353 ) );
  QMap<QString, QgsProcessingModelOutput> a2Outs;
  a2Outs.insert( u"out1"_s, oo );
  a2.setModelOutputs( a2Outs );

  algs.insert( u"a"_s, a1 );
  algs.insert( u"b"_s, a2 );
  alg.setChildAlgorithms( algs );
  QCOMPARE( alg.childAlgorithms().count(), 2 );
  QCOMPARE( alg.childAlgorithms().value( u"a"_s ).description(), u"alg1"_s );
  QCOMPARE( alg.childAlgorithms().value( u"b"_s ).description(), u"alg2"_s );

  QgsProcessingModelChildAlgorithm a2other;
  a2other.setChildId( u"b"_s );
  a2other.setDescription( u"alg2 other"_s );
  const QgsProcessingModelOutput oo2;
  QMap<QString, QgsProcessingModelOutput> a2Outs2;
  a2Outs2.insert( u"out1"_s, oo2 );
  // this one didn't already exist in the algorithm
  QgsProcessingModelOutput oo3;
  oo3.comment()->setDescription( u"my comment"_s );
  a2Outs2.insert( u"out3"_s, oo3 );

  a2other.setModelOutputs( a2Outs2 );

  a2other.copyNonDefinitionPropertiesFromModel( &alg );
  QCOMPARE( a2other.description(), u"alg2 other"_s );
  QCOMPARE( a2other.position(), QPointF( 112, 131 ) );
  QCOMPARE( a2other.size(), QSizeF( 44, 55 ) );
  QCOMPARE( a2other.comment()->size(), QSizeF( 111, 222 ) );
  QCOMPARE( a2other.comment()->position(), QPointF( 113, 114 ) );
  // should not be copied
  QCOMPARE( a2other.comment()->description(), QString() );
  QVERIFY( !a2other.comment()->color().isValid() );

  QCOMPARE( a2other.modelOutput( u"out1"_s ).position(), QPointF( 312, 331 ) );
  QCOMPARE( a2other.modelOutput( u"out1"_s ).size(), QSizeF( 344, 355 ) );
  QCOMPARE( a2other.modelOutput( u"out1"_s ).comment()->size(), QSizeF( 311, 322 ) );
  QCOMPARE( a2other.modelOutput( u"out1"_s ).comment()->position(), QPointF( 313, 314 ) );
  // should be copied for outputs
  QCOMPARE( a2other.modelOutput( u"out1"_s ).comment()->description(), u"c3"_s );
  QCOMPARE( a2other.modelOutput( u"out1"_s ).comment()->color(), QColor( 155, 14, 353 ) );
  // new outputs should not be affected
  QCOMPARE( a2other.modelOutput( u"out3"_s ).comment()->description(), u"my comment"_s );

  QgsProcessingModelChildAlgorithm a3;
  a3.setChildId( u"c"_s );
  a3.setDescription( u"alg3"_s );
  QCOMPARE( alg.addChildAlgorithm( a3 ), u"c"_s );
  QCOMPARE( alg.childAlgorithms().count(), 3 );
  QCOMPARE( alg.childAlgorithms().value( u"a"_s ).description(), u"alg1"_s );
  QCOMPARE( alg.childAlgorithms().value( u"b"_s ).description(), u"alg2"_s );
  QCOMPARE( alg.childAlgorithms().value( u"c"_s ).description(), u"alg3"_s );
  QCOMPARE( alg.childAlgorithm( "a" ).description(), u"alg1"_s );
  QCOMPARE( alg.childAlgorithm( u"b"_s ).description(), u"alg2"_s );
  QCOMPARE( alg.childAlgorithm( "c" ).description(), u"alg3"_s );
  // initially non-existent
  QVERIFY( alg.childAlgorithm( "d" ).description().isEmpty() );
  alg.childAlgorithm( "d" ).setDescription( u"alg4"_s );
  QCOMPARE( alg.childAlgorithm( "d" ).description(), u"alg4"_s );
  // overwrite existing
  QgsProcessingModelChildAlgorithm a4a;
  a4a.setChildId( "d" );
  a4a.setDescription( "new" );
  alg.setChildAlgorithm( a4a );
  QCOMPARE( alg.childAlgorithm( "d" ).description(), u"new"_s );


  // generating child ids
  QgsProcessingModelChildAlgorithm c1;
  c1.setAlgorithmId( u"buffer"_s );
  c1.generateChildId( alg );
  QCOMPARE( c1.childId(), u"buffer_1"_s );
  QCOMPARE( alg.addChildAlgorithm( c1 ), u"buffer_1"_s );
  QgsProcessingModelChildAlgorithm c2;
  c2.setAlgorithmId( u"buffer"_s );
  c2.generateChildId( alg );
  QCOMPARE( c2.childId(), u"buffer_2"_s );
  QCOMPARE( alg.addChildAlgorithm( c2 ), u"buffer_2"_s );
  QgsProcessingModelChildAlgorithm c3;
  c3.setAlgorithmId( u"centroid"_s );
  c3.generateChildId( alg );
  QCOMPARE( c3.childId(), u"centroid_1"_s );
  QCOMPARE( alg.addChildAlgorithm( c3 ), u"centroid_1"_s );
  QgsProcessingModelChildAlgorithm c4;
  c4.setAlgorithmId( u"centroid"_s );
  c4.setChildId( u"centroid_1"_s ); // dupe id
  QCOMPARE( alg.addChildAlgorithm( c4 ), u"centroid_2"_s );
  QCOMPARE( alg.childAlgorithm( u"centroid_2"_s ).childId(), u"centroid_2"_s );

  // parameter components
  QMap<QString, QgsProcessingModelParameter> pComponents;
  QgsProcessingModelParameter pc1;
  pc1.setParameterName( u"my_param"_s );
  QCOMPARE( pc1.parameterName(), u"my_param"_s );
  pc1.comment()->setDescription( u"my comment"_s );
  QCOMPARE( pc1.comment()->description(), u"my comment"_s );
  std::unique_ptr<QgsProcessingModelParameter> paramClone( pc1.clone() );
  QCOMPARE( paramClone->toVariant(), pc1.toVariant() );
  QCOMPARE( paramClone->comment()->description(), u"my comment"_s );
  QgsProcessingModelParameter pcc1;
  pcc1.loadVariant( pc1.toVariant().toMap() );
  QCOMPARE( pcc1.comment()->description(), u"my comment"_s );
  pComponents.insert( u"my_param"_s, pc1 );
  alg.setParameterComponents( pComponents );
  QCOMPARE( alg.parameterComponents().count(), 1 );
  QCOMPARE( alg.parameterComponents().value( u"my_param"_s ).parameterName(), u"my_param"_s );
  QCOMPARE( alg.parameterComponent( "my_param" ).parameterName(), u"my_param"_s );
  alg.parameterComponent( "my_param" ).setDescription( u"my param 2"_s );
  QCOMPARE( alg.parameterComponent( "my_param" ).description(), u"my param 2"_s );
  // no existent
  alg.parameterComponent( u"b"_s ).setDescription( u"my param 3"_s );
  QCOMPARE( alg.parameterComponent( u"b"_s ).description(), u"my param 3"_s );
  QCOMPARE( alg.parameterComponent( u"b"_s ).parameterName(), u"b"_s );
  QCOMPARE( alg.parameterComponents().count(), 2 );

  // parameter definitions
  QgsProcessingModelAlgorithm alg1a( "test", "testGroup" );
  QgsProcessingModelParameter bool1;
  bool1.setPosition( QPointF( 1, 2 ) );
  bool1.setSize( QSizeF( 11, 12 ) );
  alg1a.addModelParameter( new QgsProcessingParameterBoolean( u"p1"_s, "desc" ), bool1 );
  QCOMPARE( alg1a.parameterDefinitions().count(), 1 );
  QCOMPARE( alg1a.parameterDefinition( u"p1"_s )->type(), u"boolean"_s );
  QCOMPARE( alg1a.parameterComponent( u"p1"_s ).position().x(), 1.0 );
  QCOMPARE( alg1a.parameterComponent( u"p1"_s ).position().y(), 2.0 );
  QCOMPARE( alg1a.parameterComponent( u"p1"_s ).size().width(), 11.0 );
  QCOMPARE( alg1a.parameterComponent( u"p1"_s ).size().height(), 12.0 );
  alg1a.updateModelParameter( new QgsProcessingParameterBoolean( u"p1"_s, "descx" ) );
  QCOMPARE( alg1a.parameterDefinition( u"p1"_s )->description(), u"descx"_s );
  alg1a.removeModelParameter( "bad" );
  QCOMPARE( alg1a.parameterDefinitions().count(), 1 );
  alg1a.removeModelParameter( u"p1"_s );
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
  QVERIFY( alg3.dependentChildAlgorithms( u"notvalid"_s ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"notvalid"_s ).isEmpty() );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"notvalid"_s ).isEmpty() );

  // add a child
  QgsProcessingModelChildAlgorithm c7;
  c7.setChildId( u"c7"_s );
  alg3.addChildAlgorithm( c7 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c7"_s ).isEmpty() );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c7"_s ).isEmpty() );

  // direct dependency
  QgsProcessingModelChildAlgorithm c8;
  c8.setChildId( u"c8"_s );
  c8.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( u"c7"_s ) );
  alg3.addChildAlgorithm( c8 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c8"_s ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c7"_s ).isEmpty() );
  QCOMPARE( alg3.dependentChildAlgorithms( u"c7"_s ).count(), 1 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).contains( u"c8"_s ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( u"c8"_s ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c8"_s ).contains( u"c7"_s ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c7"_s ).isEmpty() );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c8"_s ).size(), 1 );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c8"_s ).at( 0 ).childId, u"c7"_s );

  // dependency via parameter source
  QgsProcessingModelChildAlgorithm c9;
  c9.setChildId( u"c9"_s );
  c9.addParameterSources( u"x"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( u"c8"_s, u"x"_s ) );
  alg3.addChildAlgorithm( c9 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c9"_s ).isEmpty() );
  QCOMPARE( alg3.dependentChildAlgorithms( u"c8"_s ).count(), 1 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c8"_s ).contains( u"c9"_s ) );
  QCOMPARE( alg3.dependentChildAlgorithms( u"c7"_s ).count(), 2 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).contains( u"c8"_s ) );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).contains( u"c9"_s ) );

  QVERIFY( alg3.dependsOnChildAlgorithms( u"c7"_s ).isEmpty() );
  QCOMPARE( alg3.dependsOnChildAlgorithms( u"c8"_s ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c8"_s ).contains( u"c7"_s ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( u"c9"_s ).count(), 2 );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c9"_s ).contains( u"c7"_s ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c9"_s ).contains( u"c8"_s ) );

  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c7"_s ).isEmpty() );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c8"_s ).size(), 1 );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c8"_s ).at( 0 ).childId, u"c7"_s );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c9"_s ).size(), 2 );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9"_s ).contains( QgsProcessingModelChildDependency( u"c7"_s ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9"_s ).contains( QgsProcessingModelChildDependency( u"c8"_s ) ) );

  QgsProcessingModelChildAlgorithm c9b;
  c9b.setChildId( u"c9b"_s );
  c9b.addParameterSources( u"x"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( u"c9"_s, u"x"_s ) );
  alg3.addChildAlgorithm( c9b );

  QCOMPARE( alg3.dependentChildAlgorithms( u"c9"_s ).count(), 1 );
  QCOMPARE( alg3.dependentChildAlgorithms( u"c8"_s ).count(), 2 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c8"_s ).contains( u"c9"_s ) );
  QVERIFY( alg3.dependentChildAlgorithms( u"c8"_s ).contains( "c9b" ) );
  QCOMPARE( alg3.dependentChildAlgorithms( u"c7"_s ).count(), 3 );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).contains( u"c8"_s ) );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).contains( u"c9"_s ) );
  QVERIFY( alg3.dependentChildAlgorithms( u"c7"_s ).contains( "c9b" ) );

  QVERIFY( alg3.dependsOnChildAlgorithms( u"c7"_s ).isEmpty() );
  QCOMPARE( alg3.dependsOnChildAlgorithms( u"c8"_s ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c8"_s ).contains( u"c7"_s ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( u"c9"_s ).count(), 2 );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c9"_s ).contains( u"c7"_s ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( u"c9"_s ).contains( u"c8"_s ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c9b" ).count(), 3 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( u"c7"_s ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( u"c8"_s ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( u"c9"_s ) );

  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c7"_s ).isEmpty() );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c8"_s ).size(), 1 );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c8"_s ).at( 0 ).childId, u"c7"_s );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c9"_s ).size(), 2 );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9"_s ).contains( QgsProcessingModelChildDependency( u"c7"_s ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9"_s ).contains( QgsProcessingModelChildDependency( u"c8"_s ) ) );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( u"c9b"_s ).size(), 3 );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9b"_s ).contains( QgsProcessingModelChildDependency( u"c7"_s ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9b"_s ).contains( QgsProcessingModelChildDependency( u"c8"_s ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( u"c9b"_s ).contains( QgsProcessingModelChildDependency( u"c9"_s ) ) );

  alg3.removeChildAlgorithm( "c9b" );


  // (de)activate child algorithm
  alg3.deactivateChildAlgorithm( u"c9"_s );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( u"c9"_s ) );
  QVERIFY( alg3.childAlgorithm( u"c9"_s ).isActive() );
  alg3.deactivateChildAlgorithm( u"c8"_s );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( u"c9"_s ) );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( u"c8"_s ) );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( u"c9"_s ) );
  QVERIFY( alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c8"_s ).isActive() );
  alg3.deactivateChildAlgorithm( u"c7"_s );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c7"_s ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( u"c9"_s ) );
  QVERIFY( !alg3.activateChildAlgorithm( u"c8"_s ) );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c7"_s ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( u"c8"_s ) );
  QVERIFY( alg3.activateChildAlgorithm( u"c7"_s ) );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( !alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c7"_s ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( u"c9"_s ) );
  QVERIFY( alg3.activateChildAlgorithm( u"c8"_s ) );
  QVERIFY( !alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c7"_s ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( u"c9"_s ) );
  QVERIFY( alg3.childAlgorithm( u"c9"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c8"_s ).isActive() );
  QVERIFY( alg3.childAlgorithm( u"c7"_s ).isActive() );


  //remove child algorithm
  QVERIFY( !alg3.removeChildAlgorithm( u"c7"_s ) );
  QVERIFY( !alg3.removeChildAlgorithm( u"c8"_s ) );
  QVERIFY( alg3.removeChildAlgorithm( u"c9"_s ) );
  QCOMPARE( alg3.childAlgorithms().count(), 2 );
  QVERIFY( alg3.childAlgorithms().contains( u"c7"_s ) );
  QVERIFY( alg3.childAlgorithms().contains( u"c8"_s ) );
  QVERIFY( !alg3.removeChildAlgorithm( u"c7"_s ) );
  QVERIFY( alg3.removeChildAlgorithm( u"c8"_s ) );
  QCOMPARE( alg3.childAlgorithms().count(), 1 );
  QVERIFY( alg3.childAlgorithms().contains( u"c7"_s ) );
  QVERIFY( alg3.removeChildAlgorithm( u"c7"_s ) );
  QVERIFY( alg3.childAlgorithms().isEmpty() );

  // parameter dependencies
  QgsProcessingModelAlgorithm alg4( "test", "testGroup" );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "not a param" ) );
  QgsProcessingModelChildAlgorithm c10;
  c10.setChildId( "c10" );
  alg4.addChildAlgorithm( c10 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "not a param" ) );
  const QgsProcessingModelParameter bool2;
  alg4.addModelParameter( new QgsProcessingParameterBoolean( u"p1"_s, "desc" ), bool2 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( u"p1"_s ) );
  c10.addParameterSources( u"x"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "p2" ) );
  alg4.setChildAlgorithm( c10 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( u"p1"_s ) );
  c10.addParameterSources( u"y"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( u"p1"_s ) );
  alg4.setChildAlgorithm( c10 );
  QVERIFY( alg4.childAlgorithmsDependOnParameter( u"p1"_s ) );

  const QgsProcessingModelParameter vlP;
  alg4.addModelParameter( new QgsProcessingParameterVectorLayer( "layer" ), vlP );
  const QgsProcessingModelParameter field;
  alg4.addModelParameter( new QgsProcessingParameterField( u"field"_s, QString(), QVariant(), u"layer"_s ), field );
  QVERIFY( !alg4.otherParametersDependOnParameter( u"p1"_s ) );
  QVERIFY( !alg4.otherParametersDependOnParameter( u"field"_s ) );
  QVERIFY( alg4.otherParametersDependOnParameter( "layer" ) );


  // to/from XML
  QgsProcessingModelAlgorithm alg5( u"test"_s, u"testGroup"_s );
  alg5.helpContent().insert( u"author"_s, u"me"_s );
  alg5.helpContent().insert( u"usage"_s, u"run"_s );
  alg5.addGroupBox( groupBox );
  QVariantMap variables;
  variables.insert( u"v1"_s, 5 );
  variables.insert( u"v2"_s, u"aabbccd"_s );
  alg5.setVariables( variables );
  QCOMPARE( alg5.variables(), variables );
  QgsProcessingModelChildAlgorithm alg5c1;
  alg5c1.setChildId( u"cx1"_s );
  alg5c1.setAlgorithmId( u"buffer"_s );
  alg5c1.setConfiguration( myConfig );
  alg5c1.addParameterSources( u"x"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( u"p1"_s ) );
  alg5c1.addParameterSources( u"y"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx2", "out3" ) );
  alg5c1.addParameterSources( u"z"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  alg5c1.addParameterSources( "a", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( "2*2" ) );
  alg5c1.addParameterSources( "zm", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 6 ) << QgsProcessingModelChildParameterSource::fromModelParameter( "p2" ) << QgsProcessingModelChildParameterSource::fromChildOutput( "cx2", "out4" ) << QgsProcessingModelChildParameterSource::fromExpression( "1+2" ) << QgsProcessingModelChildParameterSource::fromStaticValue( QgsProperty::fromExpression( "1+8" ) ) );
  alg5c1.setActive( true );
  alg5c1.setLinksCollapsed( Qt::BottomEdge, true );
  alg5c1.setLinksCollapsed( Qt::TopEdge, true );
  alg5c1.setDescription( "child 1" );
  alg5c1.setPosition( QPointF( 1, 2 ) );
  alg5c1.setSize( QSizeF( 11, 21 ) );
  QMap<QString, QgsProcessingModelOutput> alg5c1outputs;
  QgsProcessingModelOutput alg5c1out1;
  alg5c1out1.setDescription( u"my output"_s );
  alg5c1out1.setPosition( QPointF( 3, 4 ) );
  alg5c1out1.setSize( QSizeF( 31, 41 ) );
  alg5c1outputs.insert( u"a"_s, alg5c1out1 );
  alg5c1.setModelOutputs( alg5c1outputs );
  alg5.addChildAlgorithm( alg5c1 );

  QgsProcessingModelChildAlgorithm alg5c2;
  alg5c2.setChildId( "cx2" );
  alg5c2.setAlgorithmId( u"native:centroids"_s );
  alg5c2.setActive( false );
  alg5c2.setLinksCollapsed( Qt::BottomEdge, false );
  alg5c2.setLinksCollapsed( Qt::TopEdge, false );
  alg5c2.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( "a" ) << QgsProcessingModelChildDependency( u"b"_s ) );
  alg5.addChildAlgorithm( alg5c2 );

  QgsProcessingModelParameter alg5pc1;
  alg5pc1.setParameterName( u"my_param"_s );
  alg5pc1.setPosition( QPointF( 11, 12 ) );
  alg5pc1.setSize( QSizeF( 21, 22 ) );
  alg5.addModelParameter( new QgsProcessingParameterBoolean( u"my_param"_s ), alg5pc1 );
  alg5.setDesignerParameterValues( lastParams );

  QDomDocument doc = QDomDocument( "model" );
  alg5.initAlgorithm();
  const QVariant v = alg5.toVariant();
  // make sure private parameters weren't included in the definition
  QVERIFY( !v.toMap().value( u"parameterDefinitions"_s ).toMap().contains( u"VERBOSE_LOG"_s ) );

  const QDomElement elem = QgsXmlUtils::writeVariant( v, doc );
  doc.appendChild( elem );

  QgsProcessingModelAlgorithm alg6;
  QVERIFY( alg6.loadVariant( QgsXmlUtils::readVariant( doc.firstChildElement() ) ) );
  QCOMPARE( alg6.name(), u"test"_s );
  QCOMPARE( alg6.group(), u"testGroup"_s );
  QCOMPARE( alg6.helpContent(), alg5.helpContent() );
  QCOMPARE( alg6.variables(), variables );
  QCOMPARE( alg6.designerParameterValues(), lastParams );

  QCOMPARE( alg6.groupBoxes().size(), 1 );
  QCOMPARE( alg6.groupBoxes().at( 0 ).size(), QSizeF( 9, 8 ) );
  QCOMPARE( alg6.groupBoxes().at( 0 ).position(), QPointF( 11, 14 ) );
  QCOMPARE( alg6.groupBoxes().at( 0 ).description(), u"a comment"_s );
  QCOMPARE( alg6.groupBoxes().at( 0 ).color(), QColor( 123, 45, 67 ) );

  QgsProcessingModelChildAlgorithm alg6c1 = alg6.childAlgorithm( u"cx1"_s );
  QCOMPARE( alg6c1.childId(), u"cx1"_s );
  QCOMPARE( alg6c1.algorithmId(), u"buffer"_s );
  QCOMPARE( alg6c1.configuration(), myConfig );
  QVERIFY( alg6c1.isActive() );
  QVERIFY( alg6c1.linksCollapsed( Qt::BottomEdge ) );
  QVERIFY( alg6c1.linksCollapsed( Qt::TopEdge ) );
  QCOMPARE( alg6c1.description(), u"child 1"_s );
  QCOMPARE( alg6c1.position().x(), 1.0 );
  QCOMPARE( alg6c1.position().y(), 2.0 );
  QCOMPARE( alg6c1.size().width(), 11.0 );
  QCOMPARE( alg6c1.size().height(), 21.0 );
  QCOMPARE( alg6c1.parameterSources().count(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( u"x"_s ).at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( alg6c1.parameterSources().value( u"x"_s ).at( 0 ).parameterName(), u"p1"_s );
  QCOMPARE( alg6c1.parameterSources().value( u"y"_s ).at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( alg6c1.parameterSources().value( u"y"_s ).at( 0 ).outputChildId(), u"cx2"_s );
  QCOMPARE( alg6c1.parameterSources().value( u"y"_s ).at( 0 ).outputName(), u"out3"_s );
  QCOMPARE( alg6c1.parameterSources().value( u"z"_s ).at( 0 ).source(), Qgis::Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( alg6c1.parameterSources().value( u"z"_s ).at( 0 ).staticValue().toInt(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( "a" ).at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::Expression );
  QCOMPARE( alg6c1.parameterSources().value( "a" ).at( 0 ).expression(), u"2*2"_s );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).count(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 0 ).staticValue().toInt(), 6 );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 1 ).source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 1 ).parameterName(), u"p2"_s );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 2 ).source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 2 ).outputChildId(), u"cx2"_s );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 2 ).outputName(), u"out4"_s );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 3 ).source(), Qgis::ProcessingModelChildParameterSource::Expression );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 3 ).expression(), u"1+2"_s );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 4 ).source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 4 ).staticValue().userType(), qMetaTypeId<QgsProperty>() );
  QCOMPARE( alg6c1.parameterSources().value( "zm" ).at( 4 ).staticValue().value<QgsProperty>().expressionString(), u"1+8"_s );

  QCOMPARE( alg6c1.modelOutputs().count(), 1 );
  QCOMPARE( alg6c1.modelOutputs().value( u"a"_s ).description(), u"my output"_s );
  QCOMPARE( alg6c1.modelOutput( "a" ).description(), u"my output"_s );
  QCOMPARE( alg6c1.modelOutput( "a" ).position().x(), 3.0 );
  QCOMPARE( alg6c1.modelOutput( "a" ).position().y(), 4.0 );
  QCOMPARE( alg6c1.modelOutput( "a" ).size().width(), 31.0 );
  QCOMPARE( alg6c1.modelOutput( "a" ).size().height(), 41.0 );

  const QgsProcessingModelChildAlgorithm alg6c2 = alg6.childAlgorithm( "cx2" );
  QCOMPARE( alg6c2.childId(), u"cx2"_s );
  QVERIFY( !alg6c2.isActive() );
  QVERIFY( !alg6c2.linksCollapsed( Qt::BottomEdge ) );
  QVERIFY( !alg6c2.linksCollapsed( Qt::TopEdge ) );
  QCOMPARE( alg6c2.dependencies(), QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( "a" ) << QgsProcessingModelChildDependency( u"b"_s ) );

  QCOMPARE( alg6.parameterComponents().count(), 1 );
  QCOMPARE( alg6.parameterComponents().value( u"my_param"_s ).parameterName(), u"my_param"_s );
  QCOMPARE( alg6.parameterComponent( "my_param" ).parameterName(), u"my_param"_s );
  QCOMPARE( alg6.parameterComponent( "my_param" ).position().x(), 11.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).position().y(), 12.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).size().width(), 21.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).size().height(), 22.0 );
  QCOMPARE( alg6.parameterDefinitions().count(), 1 );
  QCOMPARE( alg6.parameterDefinitions().at( 0 )->type(), u"boolean"_s );

  // destination parameters
  QgsProcessingModelAlgorithm alg7( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm alg7c1;
  alg7c1.setChildId( u"cx1"_s );
  alg7c1.setAlgorithmId( "native:centroids" );
  QMap<QString, QgsProcessingModelOutput> alg7c1outputs;
  QgsProcessingModelOutput alg7c1out1( u"my_output"_s );
  alg7c1out1.setChildId( u"cx1"_s );
  alg7c1out1.setChildOutputName( "OUTPUT" );
  alg7c1out1.setDescription( u"my output"_s );
  alg7c1outputs.insert( u"my_output"_s, alg7c1out1 );
  alg7c1.setModelOutputs( alg7c1outputs );
  alg7.addChildAlgorithm( alg7c1 );
  // verify that model has destination parameter created
  QCOMPARE( alg7.destinationParameterDefinitions().count(), 1 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), u"my_output"_s );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), u"my output"_s );
  QCOMPARE( static_cast<const QgsProcessingDestinationParameter *>( alg7.destinationParameterDefinitions().at( 0 ) )->originalProvider()->id(), u"native"_s );
  QCOMPARE( alg7.outputDefinitions().count(), 1 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), u"my_output"_s );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), u"outputVector"_s );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), u"my output"_s );

  QgsProcessingModelChildAlgorithm alg7c2;
  alg7c2.setChildId( "cx2" );
  alg7c2.setAlgorithmId( "native:centroids" );
  QMap<QString, QgsProcessingModelOutput> alg7c2outputs;
  QgsProcessingModelOutput alg7c2out1( u"my_output2"_s );
  alg7c2out1.setChildId( "cx2" );
  alg7c2out1.setChildOutputName( "OUTPUT" );
  alg7c2out1.setDescription( u"my output2"_s );
  alg7c2out1.setDefaultValue( u"my value"_s );
  alg7c2out1.setMandatory( true );
  alg7c2outputs.insert( u"my_output2"_s, alg7c2out1 );
  alg7c2.setModelOutputs( alg7c2outputs );
  alg7.addChildAlgorithm( alg7c2 );

  QCOMPARE( alg7.destinationParameterDefinitions().count(), 2 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), u"my_output"_s );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), u"my output"_s );
  QVERIFY( alg7.destinationParameterDefinitions().at( 0 )->defaultValue().isNull() );
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 0 )->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->name(), u"my_output2"_s );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->description(), u"my output2"_s );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->defaultValue().toString(), u"my value"_s );
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 1 )->flags() & Qgis::ProcessingParameterFlag::Optional ) );
  QCOMPARE( alg7.outputDefinitions().count(), 2 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), u"my_output"_s );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), u"outputVector"_s );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), u"my output"_s );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->name(), u"my_output2"_s );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->type(), u"outputVector"_s );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->description(), u"my output2"_s );

  alg7.removeChildAlgorithm( u"cx1"_s );
  QCOMPARE( alg7.destinationParameterDefinitions().count(), 1 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), u"my_output2"_s );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), u"my output2"_s );
  QCOMPARE( alg7.outputDefinitions().count(), 1 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), u"my_output2"_s );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), u"outputVector"_s );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), u"my output2"_s );

  // mandatory model output with optional child algorithm parameter
  QgsProcessingModelChildAlgorithm alg7c3;
  alg7c3.setChildId( "cx3" );
  alg7c3.setAlgorithmId( "native:extractbyexpression" );
  QMap<QString, QgsProcessingModelOutput> alg7c3outputs;
  QgsProcessingModelOutput alg7c3out1;
  alg7c3out1.setChildId( "cx3" );
  alg7c3out1.setChildOutputName( "FAIL_OUTPUT" );
  alg7c3out1.setDescription( u"my_output3"_s );
  alg7c3outputs.insert( u"my_output3"_s, alg7c3out1 );
  alg7c3.setModelOutputs( alg7c3outputs );
  alg7.addChildAlgorithm( alg7c3 );
  QVERIFY( alg7.destinationParameterDefinitions().at( 1 )->flags() & Qgis::ProcessingParameterFlag::Optional );
  alg7.childAlgorithm( alg7c3.childId() ).modelOutput( u"my_output3"_s ).setMandatory( true );
  alg7.updateDestinationParameters();
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 1 )->flags() & Qgis::ProcessingParameterFlag::Optional ) );
}

void TestQgsProcessingModelAlgorithm::modelExecution()
{
  // test childOutputIsRequired
  QgsProcessingModelAlgorithm model1;
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( u"cx1"_s );
  algc1.setAlgorithmId( "native:centroids" );
  model1.addChildAlgorithm( algc1 );
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( "cx2" );
  algc2.setAlgorithmId( "native:centroids" );
  algc2.addParameterSources( u"x"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( u"cx1"_s, u"p1"_s ) );
  model1.addChildAlgorithm( algc2 );
  QgsProcessingModelChildAlgorithm algc3;
  algc3.setChildId( "cx3" );
  algc3.setAlgorithmId( "native:centroids" );
  algc3.addParameterSources( u"x"_s, QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( u"cx1"_s, "p2" ) );
  algc3.setActive( false );
  model1.addChildAlgorithm( algc3 );

  QVERIFY( model1.childOutputIsRequired( u"cx1"_s, u"p1"_s ) ); // cx2 depends on p1
  QVERIFY( !model1.childOutputIsRequired( u"cx1"_s, "p2" ) );   // cx3 depends on p2, but cx3 is not active
  QVERIFY( !model1.childOutputIsRequired( u"cx1"_s, "p3" ) );   // nothing requires p3
  QVERIFY( !model1.childOutputIsRequired( "cx2", u"p1"_s ) );
  QVERIFY( !model1.childOutputIsRequired( "cx3", u"p1"_s ) );

  // test parametersForChildAlgorithm
  QgsProcessingModelAlgorithm model2;
  QgsProcessingModelParameter sourceParam( "SOURCE_LAYER" );
  sourceParam.comment()->setDescription( u"an input"_s );
  model2.addModelParameter( new QgsProcessingParameterFeatureSource( "SOURCE_LAYER" ), sourceParam );
  model2.addModelParameter( new QgsProcessingParameterNumber( "DIST", QString(), Qgis::ProcessingNumberParameterType::Double ), QgsProcessingModelParameter( "DIST" ) );
  QgsProcessingParameterCrs *p = new QgsProcessingParameterCrs( "CRS", QString(), QgsCoordinateReferenceSystem( u"EPSG:28355"_s ) );
  p->setFlags( p->flags() | Qgis::ProcessingParameterFlag::Advanced );
  model2.addModelParameter( p, QgsProcessingModelParameter( "CRS" ) );
  QgsProcessingModelChildAlgorithm alg2c1;
  QgsExpressionContext expContext;
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->setVariable( "myvar", 8 );
  expContext.appendScope( scope );
  alg2c1.setChildId( u"cx1"_s );
  alg2c1.setAlgorithmId( "native:buffer" );
  alg2c1.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "SOURCE_LAYER" ) );
  alg2c1.addParameterSources( "DISTANCE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "DIST" ) );
  alg2c1.addParameterSources( "SEGMENTS", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( u"@myvar*2"_s ) );
  alg2c1.addParameterSources( "END_CAP_STYLE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 1 ) );
  alg2c1.addParameterSources( "JOIN_STYLE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 2 ) );
  alg2c1.addParameterSources( "DISSOLVE", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( false ) );
  QMap<QString, QgsProcessingModelOutput> outputs1;
  QgsProcessingModelOutput out1( "MODEL_OUT_LAYER" );
  out1.setChildOutputName( "OUTPUT" );
  outputs1.insert( u"MODEL_OUT_LAYER"_s, out1 );
  alg2c1.setModelOutputs( outputs1 );
  model2.addChildAlgorithm( alg2c1 );

  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  const QString vector = testDataDir + "points.shp";

  QVariantMap modelInputs;
  modelInputs.insert( "SOURCE_LAYER", vector );
  modelInputs.insert( "DIST", 271 );
  modelInputs.insert( "cx1:MODEL_OUT_LAYER", "dest.shp" );
  QgsProcessingOutputLayerDefinition layerDef( "memory:" );
  layerDef.destinationName = "my_dest";
  modelInputs.insert( "cx3:MY_OUT", QVariant::fromValue( layerDef ) );
  QVariantMap childResults;
  QString error;
  QVariantMap params = model2.parametersForChildAlgorithm( model2.childAlgorithm( u"cx1"_s ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "DISSOLVE" ).toBool(), false );
  QCOMPARE( params.value( "DISTANCE" ).toInt(), 271 );
  QCOMPARE( params.value( "SEGMENTS" ).toInt(), 16 );
  QCOMPARE( params.value( "END_CAP_STYLE" ).toInt(), 1 );
  QCOMPARE( params.value( "JOIN_STYLE" ).toInt(), 2 );
  QCOMPARE( params.value( "INPUT" ).toString(), vector );
  QCOMPARE( params.value( "OUTPUT" ).toString(), u"dest.shp"_s );
  QCOMPARE( params.count(), 7 );

  QgsProcessingContext context;

  // Check variables for child algorithm
  // without values
  QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = model2.variablesForChildAlgorithm( u"cx1"_s, &context );
  QCOMPARE( variables.count(), 7 );
  QCOMPARE( variables.value( "DIST" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "CRS" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );

  // with values
  variables = model2.variablesForChildAlgorithm( u"cx1"_s, &context, modelInputs, childResults );
  QCOMPARE( variables.count(), 7 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  std::unique_ptr<QgsExpressionContextScope> childScope( model2.createExpressionContextScopeForChildAlgorithm( u"cx1"_s, context, modelInputs, childResults ) );
  QCOMPARE( childScope->name(), u"algorithm_inputs"_s );
  QCOMPARE( childScope->variableCount(), 7 );
  QCOMPARE( childScope->variable( "DIST" ).toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_minx" ).toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_miny" ).toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_maxx" ).toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_maxy" ).toDouble(), 46.8719, 0.001 );


  QVariantMap results;
  results.insert( "OUTPUT", u"dest.shp"_s );
  childResults.insert( u"cx1"_s, results );

  // a child who uses an output from another alg as a parameter value
  QgsProcessingModelChildAlgorithm alg2c2;
  alg2c2.setChildId( "cx2" );
  alg2c2.setAlgorithmId( "native:centroids" );
  alg2c2.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( u"cx1"_s, "OUTPUT" ) );
  model2.addChildAlgorithm( alg2c2 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx2" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "INPUT" ).toString(), u"dest.shp"_s );
  QCOMPARE( params.value( "OUTPUT" ).toString(), u"memory:Centroids"_s );
  QCOMPARE( params.count(), 2 );

  variables = model2.variablesForChildAlgorithm( "cx2", &context );
  QCOMPARE( variables.count(), 12 );
  QCOMPARE( variables.value( "DIST" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.outputChildId(), u"cx1"_s );

  // with values
  variables = model2.variablesForChildAlgorithm( "cx2", &context, modelInputs, childResults );
  QCOMPARE( variables.count(), 12 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QString( u"cx1"_s ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.parameterName(), QString( "" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  // a child with an optional output
  QgsProcessingModelChildAlgorithm alg2c3;
  alg2c3.setChildId( "cx3" );
  alg2c3.setAlgorithmId( "native:extractbyexpression" );
  alg2c3.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( u"cx1"_s, "OUTPUT" ) );
  alg2c3.addParameterSources( "EXPRESSION", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "true" ) );
  alg2c3.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "MY_OUT" ) );
  alg2c3.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( "cx2" ) );
  QMap<QString, QgsProcessingModelOutput> outputs3;
  QgsProcessingModelOutput out2( "MY_OUT" );
  out2.setChildOutputName( "OUTPUT" );
  out2.setDescription( u"My output"_s );
  outputs3.insert( u"MY_OUT"_s, out2 );
  alg2c3.setModelOutputs( outputs3 );

  model2.addChildAlgorithm( alg2c3 );
  QVERIFY( model2.parameterDefinition( u"My_output"_s ) );

  // using older version compatibility
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "INPUT" ).toString(), u"dest.shp"_s );
  QCOMPARE( params.value( "EXPRESSION" ).toString(), u"true"_s );
  QCOMPARE( params.value( "OUTPUT" ).userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  const QgsProcessingOutputLayerDefinition outDef = qvariant_cast<QgsProcessingOutputLayerDefinition>( params.value( "OUTPUT" ) );
  QCOMPARE( outDef.destinationName, u"MY_OUT"_s );
  QCOMPARE( outDef.sink.staticValue().toString(), u"memory:"_s );
  QCOMPARE( params.count(), 3 ); // don't want FAIL_OUTPUT set!

  // using newer version naming
  modelInputs.remove( u"cx3:MY_OUT"_s );
  modelInputs.insert( "my_output", QVariant::fromValue( layerDef ) );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "INPUT" ).toString(), u"dest.shp"_s );
  QCOMPARE( params.value( "EXPRESSION" ).toString(), u"true"_s );
  QCOMPARE( params.value( "OUTPUT" ).userType(), qMetaTypeId<QgsProcessingOutputLayerDefinition>() );
  const QgsProcessingOutputLayerDefinition outDef2 = qvariant_cast<QgsProcessingOutputLayerDefinition>( params.value( "OUTPUT" ) );
  QCOMPARE( outDef2.destinationName, u"MY_OUT"_s );
  QCOMPARE( outDef2.sink.staticValue().toString(), u"memory:"_s );
  QCOMPARE( params.count(), 3 ); // don't want FAIL_OUTPUT set!

  // a child with an expression which is invalid
  QgsProcessingModelChildAlgorithm alg2c3a;
  alg2c3a.setChildId( "cx3a" );
  alg2c3a.setAlgorithmId( "native:extractbyexpression" );
  alg2c3a.setDescription( "Extract by expression" );
  alg2c3a.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( u"1/'a'"_s ) );
  alg2c3a.addParameterSources( "EXPRESSION", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "true" ) );
  alg2c3a.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "MY_OUT" ) );

  model2.addChildAlgorithm( alg2c3a );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3a" ), modelInputs, childResults, expContext, error );
  QCOMPARE( error, u"Could not evaluate expression for parameter INPUT for Extract by expression: Cannot convert 'a' to double"_s );
  model2.removeChildAlgorithm( "cx3a" );

  // a child with an static output value
  QgsProcessingModelChildAlgorithm alg2c4;
  alg2c4.setChildId( "cx4" );
  alg2c4.setAlgorithmId( "native:extractbyexpression" );
  alg2c4.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "STATIC" ) );
  model2.addChildAlgorithm( alg2c4 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx4" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "OUTPUT" ).toString(), u"STATIC"_s );
  model2.removeChildAlgorithm( "cx4" );
  // expression based output value
  alg2c4.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( "'A' || 'B'" ) );
  model2.addChildAlgorithm( alg2c4 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx4" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "OUTPUT" ).toString(), u"AB"_s );
  model2.removeChildAlgorithm( "cx4" );

  variables = model2.variablesForChildAlgorithm( "cx3", &context );
  QCOMPARE( variables.count(), 17 );
  QCOMPARE( variables.value( "DIST" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.outputChildId(), u"cx1"_s );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.outputChildId(), u"cx2"_s );
  QCOMPARE( variables.value( "cx2_OUTPUT_minx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_minx" ).source.outputChildId(), u"cx2"_s );
  QCOMPARE( variables.value( "cx2_OUTPUT_miny" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_miny" ).source.outputChildId(), u"cx2"_s );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxx" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxx" ).source.outputChildId(), u"cx2"_s );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxy" ).source.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx2_OUTPUT_maxy" ).source.outputChildId(), u"cx2"_s );
  // with values
  variables = model2.variablesForChildAlgorithm( "cx3", &context, modelInputs, childResults );
  QCOMPARE( variables.count(), 17 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QString( u"cx1"_s ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.parameterName(), QString( "" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.outputChildId(), QString( "cx2" ) );
  QCOMPARE( variables.value( "cx2_OUTPUT" ).source.parameterName(), QString( "" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  // test safe name of the child alg parameter as source to another algorithm
  // parameter name should have [\s ' ( ) : .] chars changed to "_" (regexp [\\s'\"\\(\\):\.])
  // this case is esecially important in case of grass algs where name algorithm contains "."
  // name of the variable is get from childDescription or childId. Refs https://github.com/qgis/QGIS/issues/36377
  QgsProcessingModelChildAlgorithm &cx1 = model2.childAlgorithm( u"cx1"_s );
  const QString oldDescription = cx1.description();
  cx1.setDescription( "cx '():.1" );
  variables = model2.variablesForChildAlgorithm( "cx3", &context );
  QVERIFY( !variables.contains( "cx1_OUTPUT" ) );
  QVERIFY( !variables.contains( "cx '():.1_OUTPUT" ) );
  QVERIFY( variables.contains( "cx______1_OUTPUT" ) );
  cx1.setDescription( oldDescription ); // set descrin back to avoid fail of following tests

  // test model to python conversion
  model2.setName( u"2my model"_s );
  model2.childAlgorithm( u"cx1"_s ).modelOutput( u"MODEL_OUT_LAYER"_s ).setDescription( "my model output" );
  model2.updateDestinationParameters();
  model2.childAlgorithm( u"cx1"_s ).setDescription( "first step in my model" );
  const QStringList actualParts = model2.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, 2 );
  QgsDebugMsgLevel( actualParts.join( '\n' ), 1 );
  const QStringList expectedParts = QStringLiteral( "\"\"\"\n"
                                                    "Model exported as python.\n"
                                                    "Name : 2my model\n"
                                                    "Group : \n"
                                                    "With QGIS : %1\n"
                                                    "\"\"\"\n\n"
                                                    "from typing import Any, Optional\n"
                                                    "\n"
                                                    "from qgis.core import QgsProcessing\n"
                                                    "from qgis.core import QgsProcessingAlgorithm\n"
                                                    "from qgis.core import QgsProcessingContext\n"
                                                    "from qgis.core import QgsProcessingFeedback, QgsProcessingMultiStepFeedback\n"
                                                    "from qgis.core import QgsProcessingParameterFeatureSource\n"
                                                    "from qgis.core import QgsProcessingParameterNumber\n"
                                                    "from qgis.core import QgsProcessingParameterCrs\n"
                                                    "from qgis.core import QgsProcessingParameterFeatureSink\n"
                                                    "from qgis.core import QgsProcessingParameterDefinition\n"
                                                    "from qgis.core import QgsCoordinateReferenceSystem\n"
                                                    "from qgis.core import QgsExpression\n"
                                                    "from qgis import processing\n"
                                                    "\n"
                                                    "\n"
                                                    "class MyModel(QgsProcessingAlgorithm):\n"
                                                    "\n"
                                                    "  def initAlgorithm(self, config: Optional[dict[str, Any]] = None):\n"
                                                    "    # an input\n"
                                                    "    self.addParameter(QgsProcessingParameterFeatureSource('SOURCE_LAYER', '', defaultValue=None))\n"
                                                    "    self.addParameter(QgsProcessingParameterNumber('DIST', '', type=QgsProcessingParameterNumber.Double, defaultValue=None))\n"
                                                    "    param = QgsProcessingParameterCrs('CRS', '', defaultValue=QgsCoordinateReferenceSystem('EPSG:28355'))\n"
                                                    "    param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)\n"
                                                    "    self.addParameter(param)\n"
                                                    "    self.addParameter(QgsProcessingParameterFeatureSink('MyModelOutput', 'my model output', type=QgsProcessing.TypeVectorPolygon, createByDefault=True, supportsAppend=True, defaultValue=None))\n"
                                                    "    self.addParameter(QgsProcessingParameterFeatureSink('MyOutput', 'My output', type=QgsProcessing.TypeVectorAnyGeometry, createByDefault=True, defaultValue=None))\n"
                                                    "\n"
                                                    "  def processAlgorithm(self, parameters: dict[str, Any], context: QgsProcessingContext, model_feedback: QgsProcessingFeedback) -> dict[str, Any]:\n"
                                                    "    # Use a multi-step feedback, so that individual child algorithm progress reports are adjusted for the\n"
                                                    "    # overall progress through the model\n"
                                                    "    feedback = QgsProcessingMultiStepFeedback(3, model_feedback)\n"
                                                    "    results = {}\n"
                                                    "    outputs = {}\n"
                                                    "\n"
                                                    "    # first step in my model\n"
                                                    "    alg_params = {\n"
                                                    "      'DISSOLVE': False,\n"
                                                    "      'DISTANCE': parameters['DIST'],\n"
                                                    "      'END_CAP_STYLE': 1,  # Flat\n"
                                                    "      'INPUT': parameters['SOURCE_LAYER'],\n"
                                                    "      'JOIN_STYLE': 2,  # Bevel\n"
                                                    "      'SEGMENTS': QgsExpression('@myvar*2').evaluate(),\n"
                                                    "      'OUTPUT': parameters['MyModelOutput']\n"
                                                    "    }\n"
                                                    "    outputs['FirstStepInMyModel'] = processing.run('native:buffer', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n"
                                                    "    results['MyModelOutput'] = outputs['FirstStepInMyModel']['OUTPUT']\n"
                                                    "\n"
                                                    "    feedback.setCurrentStep(1)\n"
                                                    "    if feedback.isCanceled():\n"
                                                    "      return {}\n"
                                                    "\n"
                                                    "    alg_params = {\n"
                                                    "      'INPUT': outputs['FirstStepInMyModel']['OUTPUT'],\n"
                                                    "      'OUTPUT': QgsProcessing.TEMPORARY_OUTPUT\n"
                                                    "    }\n"
                                                    "    outputs['cx2'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n"
                                                    "\n"
                                                    "    feedback.setCurrentStep(2)\n"
                                                    "    if feedback.isCanceled():\n"
                                                    "      return {}\n"
                                                    "\n"
                                                    "    alg_params = {\n"
                                                    "      'EXPRESSION': 'true',\n"
                                                    "      'INPUT': outputs['FirstStepInMyModel']['OUTPUT'],\n"
                                                    "      'OUTPUT': parameters['MY_OUT'],\n"
                                                    "      'OUTPUT': parameters['MyOutput']\n"
                                                    "    }\n"
                                                    "    outputs['cx3'] = processing.run('native:extractbyexpression', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n"
                                                    "    results['MyOutput'] = outputs['cx3']['OUTPUT']\n"
                                                    "    return results\n"
                                                    "\n"
                                                    "  def name(self) -> str:\n"
                                                    "    return '2my model'\n"
                                                    "\n"
                                                    "  def displayName(self) -> str:\n"
                                                    "    return '2my model'\n"
                                                    "\n"
                                                    "  def group(self) -> str:\n"
                                                    "    return ''\n"
                                                    "\n"
                                                    "  def groupId(self) -> str:\n"
                                                    "    return ''\n"
                                                    "\n"
                                                    "  def createInstance(self):\n"
                                                    "    return self.__class__()\n" )
                                      .arg( Qgis::versionInt() )
                                      .split( '\n' );
  QCOMPARE( actualParts, expectedParts );
}

void TestQgsProcessingModelAlgorithm::modelWithDuplicateNames()
{
  // test that same name are correctly made unique when exporting in python
  QgsProcessingModelAlgorithm model;

  // load model with duplicate names
  QVERIFY( model.fromFile( TEST_DATA_DIR + u"/duplicate_names.model3"_s ) );

  const QStringList actualParts = model.asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, 2 );

  const QRegularExpression re( "outputs\\['([^']*)'\\] = processing.run\\('native:buffer'" );
  QVERIFY( re.isValid() );
  QStringList names;
  for ( QString part : actualParts )
  {
    const QRegularExpressionMatch match = re.match( part );
    if ( match.hasMatch() )
    {
      names << match.captured( 1 );
    }
  }

  names.sort();
  QCOMPARE( names, QStringList() << "Tampon" << "Tampon_2" );
}


void TestQgsProcessingModelAlgorithm::modelBranchPruning()
{
  QgsVectorLayer *layer3111 = new QgsVectorLayer( "Point?crs=epsg:3111", "v1", "memory" );
  QgsProject p;
  p.addMapLayer( layer3111 );

  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  const QString raster1 = testDataDir + "landsat_4326.tif";
  const QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  p.addMapLayer( r1 );

  QgsProcessingContext context;
  context.setProject( &p );

  // test that model branches are trimmed for algorithms which return the FlagPruneModelBranchesBasedOnAlgorithmResults flag
  QgsProcessingModelAlgorithm model1;

  // first add the filter by layer type alg
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "filter" );
  algc1.setAlgorithmId( "native:filterlayersbytype" );
  QgsProcessingModelParameter param;
  param.setParameterName( u"LAYER"_s );
  model1.addModelParameter( new QgsProcessingParameterMapLayer( u"LAYER"_s ), param );
  algc1.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromModelParameter( u"LAYER"_s ) );
  model1.addChildAlgorithm( algc1 );

  //then create some branches which come off this, depending on the layer type
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( u"buffer"_s );
  algc2.setAlgorithmId( "native:buffer" );
  algc2.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"filter"_s, u"VECTOR"_s ) );
  QMap<QString, QgsProcessingModelOutput> outputsc2;
  QgsProcessingModelOutput outc2( "BUFFER_OUTPUT" );
  outc2.setChildOutputName( "OUTPUT" );
  outputsc2.insert( u"BUFFER_OUTPUT"_s, outc2 );
  algc2.setModelOutputs( outputsc2 );
  model1.addChildAlgorithm( algc2 );
  // ...we want a complex branch, so add some more bits to the branch
  QgsProcessingModelChildAlgorithm algc3;
  algc3.setChildId( "buffer2" );
  algc3.setAlgorithmId( "native:buffer" );
  algc3.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"buffer"_s, u"OUTPUT"_s ) );
  QMap<QString, QgsProcessingModelOutput> outputsc3;
  QgsProcessingModelOutput outc3( "BUFFER2_OUTPUT" );
  outc3.setChildOutputName( "OUTPUT" );
  outputsc3.insert( u"BUFFER2_OUTPUT"_s, outc3 );
  algc3.setModelOutputs( outputsc3 );
  model1.addChildAlgorithm( algc3 );
  QgsProcessingModelChildAlgorithm algc4;
  algc4.setChildId( "buffer3" );
  algc4.setAlgorithmId( "native:buffer" );
  algc4.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"buffer"_s, u"OUTPUT"_s ) );
  QMap<QString, QgsProcessingModelOutput> outputsc4;
  QgsProcessingModelOutput outc4( "BUFFER3_OUTPUT" );
  outc4.setChildOutputName( "OUTPUT" );
  outputsc4.insert( u"BUFFER3_OUTPUT"_s, outc4 );
  algc4.setModelOutputs( outputsc4 );
  model1.addChildAlgorithm( algc4 );

  // now add some bits to the raster branch
  QgsProcessingModelChildAlgorithm algr2;
  algr2.setChildId( "fill2" );
  algr2.setAlgorithmId( "native:fillnodata" );
  algr2.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"filter"_s, u"RASTER"_s ) );
  QMap<QString, QgsProcessingModelOutput> outputsr2;
  QgsProcessingModelOutput outr2( "RASTER_OUTPUT" );
  outr2.setChildOutputName( "OUTPUT" );
  outputsr2.insert( u"RASTER_OUTPUT"_s, outr2 );
  algr2.setModelOutputs( outputsr2 );
  model1.addChildAlgorithm( algr2 );

  // some more bits on the raster branch
  QgsProcessingModelChildAlgorithm algr3;
  algr3.setChildId( "fill3" );
  algr3.setAlgorithmId( "native:fillnodata" );
  algr3.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"fill2"_s, u"OUTPUT"_s ) );
  QMap<QString, QgsProcessingModelOutput> outputsr3;
  QgsProcessingModelOutput outr3( "RASTER_OUTPUT2" );
  outr3.setChildOutputName( "OUTPUT" );
  outputsr3.insert( u"RASTER_OUTPUT2"_s, outr3 );
  algr3.setModelOutputs( outputsr3 );
  model1.addChildAlgorithm( algr3 );

  QgsProcessingModelChildAlgorithm algr4;
  algr4.setChildId( "fill4" );
  algr4.setAlgorithmId( "native:fillnodata" );
  algr4.addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"fill2"_s, u"OUTPUT"_s ) );
  QMap<QString, QgsProcessingModelOutput> outputsr4;
  QgsProcessingModelOutput outr4( "RASTER_OUTPUT3" );
  outr4.setChildOutputName( "OUTPUT" );
  outputsr4.insert( u"RASTER_OUTPUT3"_s, outr4 );
  algr4.setModelOutputs( outputsr4 );
  model1.addChildAlgorithm( algr4 );

  QgsProcessingFeedback feedback;
  QVariantMap params;
  // vector input
  params.insert( u"LAYER"_s, u"v1"_s );
  params.insert( u"buffer:BUFFER_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( u"buffer2:BUFFER2_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( u"buffer3:BUFFER3_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( u"fill2:RASTER_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( u"fill3:RASTER_OUTPUT2"_s, QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( u"fill4:RASTER_OUTPUT3"_s, QgsProcessing::TEMPORARY_OUTPUT );
  QVariantMap results = model1.run( params, context, &feedback );
  // we should get the vector branch outputs only
  QVERIFY( !results.value( u"buffer:BUFFER_OUTPUT"_s ).toString().isEmpty() );
  QVERIFY( !results.value( u"buffer2:BUFFER2_OUTPUT"_s ).toString().isEmpty() );
  QVERIFY( !results.value( u"buffer3:BUFFER3_OUTPUT"_s ).toString().isEmpty() );
  QVERIFY( !results.contains( u"fill2:RASTER_OUTPUT"_s ) );
  QVERIFY( !results.contains( u"fill3:RASTER_OUTPUT2"_s ) );
  QVERIFY( !results.contains( u"fill4:RASTER_OUTPUT3"_s ) );

  // raster input
  params.insert( u"LAYER"_s, u"R1"_s );
  context.modelResult().clear();
  results = model1.run( params, context, &feedback );
  // we should get the raster branch outputs only
  QVERIFY( !results.value( u"fill2:RASTER_OUTPUT"_s ).toString().isEmpty() );
  QVERIFY( !results.value( u"fill3:RASTER_OUTPUT2"_s ).toString().isEmpty() );
  QVERIFY( !results.value( u"fill4:RASTER_OUTPUT3"_s ).toString().isEmpty() );
  QVERIFY( !results.contains( u"buffer:BUFFER_OUTPUT"_s ) );
  QVERIFY( !results.contains( u"buffer2:BUFFER2_OUTPUT"_s ) );
  QVERIFY( !results.contains( u"buffer3:BUFFER3_OUTPUT"_s ) );
}

void TestQgsProcessingModelAlgorithm::modelBranchPruningConditional()
{
  QgsProcessingContext context;

  context.expressionContext().appendScope( new QgsExpressionContextScope() );
  context.expressionContext().scope( 0 )->setVariable( u"var1"_s, 1 );
  context.expressionContext().scope( 0 )->setVariable( u"var2"_s, 0 );

  // test that model branches are trimmed for algorithms which depend on conditional branches
  QgsProcessingModelAlgorithm model1;

  // first add the filter by layer type alg
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "branch" );
  algc1.setAlgorithmId( "native:condition" );
  QVariantMap config;
  QVariantList conditions;
  QVariantMap cond1;
  cond1.insert( u"name"_s, u"name1"_s );
  cond1.insert( u"expression"_s, u"@var1"_s );
  conditions << cond1;
  QVariantMap cond2;
  cond2.insert( u"name"_s, u"name2"_s );
  cond2.insert( u"expression"_s, u"@var2"_s );
  conditions << cond2;
  config.insert( u"conditions"_s, conditions );
  algc1.setConfiguration( config );
  model1.addChildAlgorithm( algc1 );

  //then create some branches which come off this
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( "exception" );
  algc2.setAlgorithmId( "native:raiseexception" );
  algc2.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( u"branch"_s, u"name1"_s ) );
  model1.addChildAlgorithm( algc2 );

  QgsProcessingModelChildAlgorithm algc3;
  algc2.setChildId( "exception" );
  algc3.setAlgorithmId( "native:raisewarning" );
  algc3.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( u"branch"_s, u"name2"_s ) );
  model1.addChildAlgorithm( algc3 );

  QgsProcessingFeedback feedback;
  const QVariantMap params;
  bool ok = false;
  QVariantMap results = model1.run( params, context, &feedback, &ok );
  QVERIFY( !ok ); // the branch with the exception should be hit

  // flip the condition results
  context.expressionContext().scope( 0 )->setVariable( u"var1"_s, 0 );
  context.expressionContext().scope( 0 )->setVariable( u"var2"_s, 1 );

  context.modelResult().clear();
  results = model1.run( params, context, &feedback, &ok );
  QVERIFY( ok ); // the branch with the exception should NOT be hit
}

void TestQgsProcessingModelAlgorithm::modelWithProviderWithLimitedTypes()
{
  QgsProcessingModelAlgorithm alg( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( u"cx1"_s );
  algc1.setAlgorithmId( "dummy4:alg1" );
  QMap<QString, QgsProcessingModelOutput> algc1outputs;
  QgsProcessingModelOutput algc1out1( u"my_vector_output"_s );
  algc1out1.setChildId( u"cx1"_s );
  algc1out1.setChildOutputName( "vector_dest" );
  algc1out1.setDescription( u"my output"_s );
  algc1outputs.insert( u"my_vector_output"_s, algc1out1 );
  QgsProcessingModelOutput algc1out2( u"my_raster_output"_s );
  algc1out2.setChildId( u"cx1"_s );
  algc1out2.setChildOutputName( "raster_dest" );
  algc1out2.setDescription( u"my output"_s );
  algc1outputs.insert( u"my_raster_output"_s, algc1out2 );
  QgsProcessingModelOutput algc1out3( u"my_sink_output"_s );
  algc1out3.setChildId( u"cx1"_s );
  algc1out3.setChildOutputName( "sink" );
  algc1out3.setDescription( u"my output"_s );
  algc1outputs.insert( u"my_sink_output"_s, algc1out3 );
  algc1.setModelOutputs( algc1outputs );
  alg.addChildAlgorithm( algc1 );
  // verify that model has destination parameter created
  QCOMPARE( alg.destinationParameterDefinitions().count(), 3 );
  QCOMPARE( alg.destinationParameterDefinitions().at( 2 )->name(), u"my_output_3"_s );
  QCOMPARE( alg.destinationParameterDefinitions().at( 2 )->description(), u"my output"_s );
  QCOMPARE( static_cast<const QgsProcessingDestinationParameter *>( alg.destinationParameterDefinitions().at( 2 ) )->originalProvider()->id(), u"dummy4"_s );
  QCOMPARE( static_cast<const QgsProcessingParameterVectorDestination *>( alg.destinationParameterDefinitions().at( 2 ) )->supportedOutputVectorLayerExtensions(), QStringList() << u"mif"_s );
  QCOMPARE( static_cast<const QgsProcessingParameterVectorDestination *>( alg.destinationParameterDefinitions().at( 2 ) )->defaultFileExtension(), u"mif"_s );
  QVERIFY( static_cast<const QgsProcessingParameterVectorDestination *>( alg.destinationParameterDefinitions().at( 2 ) )->generateTemporaryDestination().endsWith( ".mif"_L1 ) );
  QVERIFY( !static_cast<const QgsProcessingDestinationParameter *>( alg.destinationParameterDefinitions().at( 2 ) )->supportsNonFileBasedOutput() );

  QCOMPARE( alg.destinationParameterDefinitions().at( 0 )->name(), u"my_output"_s );
  QCOMPARE( alg.destinationParameterDefinitions().at( 0 )->description(), u"my output"_s );
  QCOMPARE( static_cast<const QgsProcessingDestinationParameter *>( alg.destinationParameterDefinitions().at( 0 ) )->originalProvider()->id(), u"dummy4"_s );
  Q_NOWARN_DEPRECATED_PUSH
  QCOMPARE( static_cast<const QgsProcessingParameterRasterDestination *>( alg.destinationParameterDefinitions().at( 0 ) )->supportedOutputRasterLayerExtensions(), QStringList() << u"xyz"_s );
  Q_NOWARN_DEPRECATED_POP
  QCOMPARE( static_cast<const QgsProcessingParameterRasterDestination *>( alg.destinationParameterDefinitions().at( 0 ) )->defaultFileExtension(), u"xyz"_s );
  QVERIFY( static_cast<const QgsProcessingParameterRasterDestination *>( alg.destinationParameterDefinitions().at( 0 ) )->generateTemporaryDestination().endsWith( ".xyz"_L1 ) );
  QVERIFY( !static_cast<const QgsProcessingDestinationParameter *>( alg.destinationParameterDefinitions().at( 0 ) )->supportsNonFileBasedOutput() );

  QCOMPARE( alg.destinationParameterDefinitions().at( 1 )->name(), u"my_output_2"_s );
  QCOMPARE( alg.destinationParameterDefinitions().at( 1 )->description(), u"my output"_s );
  QCOMPARE( static_cast<const QgsProcessingDestinationParameter *>( alg.destinationParameterDefinitions().at( 1 ) )->originalProvider()->id(), u"dummy4"_s );
  QCOMPARE( static_cast<const QgsProcessingParameterFeatureSink *>( alg.destinationParameterDefinitions().at( 1 ) )->supportedOutputVectorLayerExtensions(), QStringList() << u"mif"_s );
  QCOMPARE( static_cast<const QgsProcessingParameterFeatureSink *>( alg.destinationParameterDefinitions().at( 1 ) )->defaultFileExtension(), u"mif"_s );
  QVERIFY( static_cast<const QgsProcessingParameterFeatureSink *>( alg.destinationParameterDefinitions().at( 1 ) )->generateTemporaryDestination().endsWith( ".mif"_L1 ) );
  QVERIFY( !static_cast<const QgsProcessingDestinationParameter *>( alg.destinationParameterDefinitions().at( 1 ) )->supportsNonFileBasedOutput() );
}

void TestQgsProcessingModelAlgorithm::modelVectorOutputIsCompatibleType()
{
  // IMPORTANT: This method is intended to be "permissive" rather than "restrictive".
  // I.e. we only reject outputs which we know can NEVER be acceptable, but
  // if there's doubt then we default to returning true.

  // empty acceptable type list = all are compatible
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( QList<int>(), Qgis::ProcessingSourceType::MapLayer ) );

  // accept any vector
  QList<int> dataTypes;
  dataTypes << static_cast<int>( Qgis::ProcessingSourceType::Vector );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::MapLayer ) );

  // accept any vector with geometry
  dataTypes.clear();
  dataTypes << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::MapLayer ) );

  // accept any point vector
  dataTypes.clear();
  dataTypes << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::MapLayer ) );

  // accept any line vector
  dataTypes.clear();
  dataTypes << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::MapLayer ) );

  // accept any polygon vector
  dataTypes.clear();
  dataTypes << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( !QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::MapLayer ) );

  // accept any map layer
  dataTypes.clear();
  dataTypes << static_cast<int>( Qgis::ProcessingSourceType::MapLayer );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::Vector ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPoint ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorLine ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::VectorPolygon ) );
  QVERIFY( QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( dataTypes, Qgis::ProcessingSourceType::MapLayer ) );
}

void TestQgsProcessingModelAlgorithm::modelAcceptableValues()
{
  QgsProcessingModelAlgorithm m;
  const QgsProcessingModelParameter stringParam1( "string" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );

  const QgsProcessingModelParameter stringParam2( "string2" );
  m.addModelParameter( new QgsProcessingParameterString( "string2" ), stringParam2 );

  const QgsProcessingModelParameter numParam( "number" );
  m.addModelParameter( new QgsProcessingParameterNumber( "number" ), numParam );

  const QgsProcessingModelParameter tableFieldParam( u"field"_s );
  m.addModelParameter( new QgsProcessingParameterField( u"field"_s ), tableFieldParam );

  const QgsProcessingModelParameter fileParam( "file" );
  m.addModelParameter( new QgsProcessingParameterFile( "file" ), fileParam );

  // test single types
  QgsProcessingModelChildParameterSources sources = m.availableSourcesForChild( QString(), QStringList() << "number" );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"number"_s );
  sources = m.availableSourcesForChild( QString(), QStringList() << u"field"_s );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"field"_s );
  sources = m.availableSourcesForChild( QString(), QStringList() << "file" );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"file"_s );

  // test multiple types
  sources = m.availableSourcesForChild( QString(), QStringList() << "string" << "number" << "file" );
  QCOMPARE( sources.count(), 4 );
  QSet<QString> res;
  res << sources.at( 0 ).parameterName();
  res << sources.at( 1 ).parameterName();
  res << sources.at( 2 ).parameterName();
  res << sources.at( 3 ).parameterName();

  QCOMPARE( res, QSet<QString>() << u"string"_s << u"string2"_s << u"number"_s << u"file"_s );

  // check outputs
  QgsProcessingModelChildAlgorithm alg2c1;
  alg2c1.setChildId( u"cx1"_s );
  alg2c1.setAlgorithmId( "native:centroids" );
  m.addChildAlgorithm( alg2c1 );

  sources = m.availableSourcesForChild( QString(), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 1 );
  res.clear();
  res << sources.at( 0 ).outputChildId() + ':' + sources.at( 0 ).outputName();
  QCOMPARE( res, QSet<QString>() << "cx1:OUTPUT" );

  // with dependencies between child algs
  QgsProcessingModelChildAlgorithm alg2c2;
  alg2c2.setChildId( "cx2" );
  alg2c2.setAlgorithmId( "native:centroids" );
  alg2c2.setDependencies( QList<QgsProcessingModelChildDependency>() << QgsProcessingModelChildDependency( u"cx1"_s ) );
  m.addChildAlgorithm( alg2c2 );
  sources = m.availableSourcesForChild( QString(), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 2 );
  res.clear();
  res << sources.at( 0 ).outputChildId() + ':' + sources.at( 0 ).outputName();
  res << sources.at( 1 ).outputChildId() + ':' + sources.at( 1 ).outputName();
  QCOMPARE( res, QSet<QString>() << "cx1:OUTPUT" << "cx2:OUTPUT" );

  sources = m.availableSourcesForChild( u"cx1"_s, QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 0 );

  sources = m.availableSourcesForChild( QString( "cx2" ), QStringList(), QStringList() << "string" << "outputVector" );
  QCOMPARE( sources.count(), 1 );
  res.clear();
  res << sources.at( 0 ).outputChildId() + ':' + sources.at( 0 ).outputName();
  QCOMPARE( res, QSet<QString>() << "cx1:OUTPUT" );

  // test limiting by data types
  QgsProcessingModelAlgorithm m2;
  const QgsProcessingModelParameter vlInput( "vl" );
  // with no limit on data types
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl" ), vlInput );
  const QgsProcessingModelParameter fsInput( "fs" );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs" ), fsInput );

  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::MapLayer ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );

  // inputs are limited to vector layers
  m2.removeModelParameter( vlInput.parameterName() );
  m2.removeModelParameter( fsInput.parameterName() );
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl", QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ), vlInput );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs", QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ), fsInput );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::MapLayer ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );

  // inputs are limited to vector layers with geometries
  m2.removeModelParameter( vlInput.parameterName() );
  m2.removeModelParameter( fsInput.parameterName() );
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl", QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ), vlInput );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs", QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ), fsInput );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::MapLayer ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );

  // inputs are limited to vector layers with lines
  m2.removeModelParameter( vlInput.parameterName() );
  m2.removeModelParameter( fsInput.parameterName() );
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl", QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ), vlInput );
  m2.addModelParameter( new QgsProcessingParameterFeatureSource( "fs", QString(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ), fsInput );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source" );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  QCOMPARE( sources.count(), 0 );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) );
  QCOMPARE( sources.count(), 0 );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
  sources = m2.availableSourcesForChild( QString(), QStringList() << "vector" << "source", QStringList(), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::MapLayer ) );
  QCOMPARE( sources.count(), 2 );
  QCOMPARE( sources.at( 0 ).parameterName(), u"fs"_s );
  QCOMPARE( sources.at( 1 ).parameterName(), u"vl"_s );
}

void TestQgsProcessingModelAlgorithm::modelValidate()
{
  QgsProcessingModelAlgorithm m;
  QStringList errors;
  QVERIFY( !m.validate( errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Model does not contain any algorithms"_s );

  const QgsProcessingModelParameter stringParam1( "string" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );
  QgsProcessingModelChildAlgorithm alg2c1;
  alg2c1.setChildId( u"cx1"_s );
  alg2c1.setAlgorithmId( "native:centroids" );
  alg2c1.setDescription( u"centroids"_s );
  m.addChildAlgorithm( alg2c1 );

  QVERIFY( !m.validateChildAlgorithm( u"cx1"_s, errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Parameter <i>INPUT</i> is mandatory"_s );

  QVERIFY( !m.validate( errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"<b>centroids</b>: Parameter <i>INPUT</i> is mandatory"_s );

  QgsProcessingModelChildParameterSource badSource;
  badSource.setSource( Qgis::ProcessingModelChildParameterSource::StaticValue );
  badSource.setStaticValue( 56 );
  m.childAlgorithm( u"cx1"_s ).addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << badSource );

  QVERIFY( !m.validateChildAlgorithm( u"cx1"_s, errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Value for <i>INPUT</i> is not acceptable for this parameter"_s );

  QgsProcessingModelChildParameterSource goodSource;
  goodSource.setSource( Qgis::ProcessingModelChildParameterSource::Expression );
  m.childAlgorithm( u"cx1"_s ).addParameterSources( u"ALL_PARTS"_s, QList<QgsProcessingModelChildParameterSource>() << goodSource );

  QVERIFY( !m.validateChildAlgorithm( u"cx1"_s, errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Value for <i>INPUT</i> is not acceptable for this parameter"_s );

  QgsProcessingModelChildAlgorithm alg3c1;
  alg3c1.setChildId( u"cx3"_s );
  alg3c1.setAlgorithmId( "native:stringconcatenation" );
  alg3c1.setDescription( u"string concat"_s );
  m.addChildAlgorithm( alg3c1 );
  QVERIFY( !m.validateChildAlgorithm( u"cx3"_s, errors ) );
  QCOMPARE( errors.size(), 2 );
  QCOMPARE( errors.at( 0 ), u"Parameter <i>INPUT_1</i> is mandatory"_s );
  QCOMPARE( errors.at( 1 ), u"Parameter <i>INPUT_2</i> is mandatory"_s );
  goodSource.setSource( Qgis::ProcessingModelChildParameterSource::StaticValue );
  goodSource.setStaticValue( 56 );
  m.childAlgorithm( u"cx3"_s ).addParameterSources( u"INPUT_1"_s, QList<QgsProcessingModelChildParameterSource>() << goodSource );
  QVERIFY( !m.validateChildAlgorithm( u"cx3"_s, errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Parameter <i>INPUT_2</i> is mandatory"_s );
  badSource.setSource( Qgis::ProcessingModelChildParameterSource::StaticValue );
  badSource.setStaticValue( "" );
  m.childAlgorithm( u"cx3"_s ).addParameterSources( u"INPUT_1"_s, QList<QgsProcessingModelChildParameterSource>() << badSource );
  QVERIFY( !m.validateChildAlgorithm( u"cx3"_s, errors ) );
  QCOMPARE( errors.size(), 2 );
  QCOMPARE( errors.at( 0 ), u"Value for <i>INPUT_1</i> is not acceptable for this parameter"_s );
  QCOMPARE( errors.at( 1 ), u"Parameter <i>INPUT_2</i> is mandatory"_s );

  m.removeChildAlgorithm( u"cx3"_s );

  badSource.setSource( Qgis::ProcessingModelChildParameterSource::ChildOutput );
  badSource.setOutputChildId( u"cc"_s );
  m.childAlgorithm( u"cx1"_s ).addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << badSource );

  QVERIFY( !m.validateChildAlgorithm( u"cx1"_s, errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Child algorithm <i>cc</i> used for parameter <i>INPUT</i> does not exist"_s );

  badSource.setSource( Qgis::ProcessingModelChildParameterSource::ModelParameter );
  badSource.setParameterName( u"cc"_s );
  m.childAlgorithm( u"cx1"_s ).addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << badSource );

  QVERIFY( !m.validateChildAlgorithm( u"cx1"_s, errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), u"Model input <i>cc</i> used for parameter <i>INPUT</i> does not exist"_s );

  goodSource.setSource( Qgis::ProcessingModelChildParameterSource::StaticValue );
  goodSource.setStaticValue( QString( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" ) );
  m.childAlgorithm( u"cx1"_s ).addParameterSources( u"INPUT"_s, QList<QgsProcessingModelChildParameterSource>() << goodSource );

  QVERIFY( m.validateChildAlgorithm( u"cx1"_s, errors ) );
  QCOMPARE( errors.size(), 0 );

  QVERIFY( m.validate( errors ) );
  QCOMPARE( errors.size(), 0 );
}

void TestQgsProcessingModelAlgorithm::modelInputs()
{
  QgsProcessingModelAlgorithm m;

  // add a bunch of inputs
  const QgsProcessingModelParameter stringParam1( "string" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );

  const QgsProcessingModelParameter stringParam2( "a string" );
  m.addModelParameter( new QgsProcessingParameterString( "a string" ), stringParam2 );

  const QgsProcessingModelParameter stringParam3( "cc string" );
  m.addModelParameter( new QgsProcessingParameterString( "cc string" ), stringParam3 );

  // set specific input order for parameters
  m.setParameterOrder( QStringList() << "cc string" << "a string" );

  QgsProcessingModelAlgorithm m2;
  m2.loadVariant( m.toVariant() );
  QCOMPARE( m2.orderedParameters().count(), 3 );
  QCOMPARE( m2.orderedParameters().at( 0 ).parameterName(), u"cc string"_s );
  QCOMPARE( m2.orderedParameters().at( 1 ).parameterName(), u"a string"_s );
  QCOMPARE( m2.orderedParameters().at( 2 ).parameterName(), u"string"_s );

  QCOMPARE( m2.parameterDefinitions().at( 0 )->name(), u"cc string"_s );
  QCOMPARE( m2.parameterDefinitions().at( 1 )->name(), u"a string"_s );
  QCOMPARE( m2.parameterDefinitions().at( 2 )->name(), u"string"_s );
}

void TestQgsProcessingModelAlgorithm::modelOutputs()
{
  QgsProcessingModelAlgorithm m;
  QVERIFY( m.orderedOutputs().isEmpty() );
  QVERIFY( m.outputGroup().isEmpty() );
  m.setOutputGroup( u"output group"_s );
  QCOMPARE( m.outputGroup(), u"output group"_s );

  const QgsProcessingModelParameter sourceParam( "INPUT" );
  m.addModelParameter( new QgsProcessingParameterFeatureSource( "INPUT" ), sourceParam );

  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( u"cx1"_s );
  algc1.setAlgorithmId( "native:buffer" );
  algc1.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromModelParameter( "INPUT" ) } );

  m.addChildAlgorithm( algc1 );
  QVERIFY( m.orderedOutputs().isEmpty() );

  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( u"cx2"_s );
  algc2.setAlgorithmId( "native:buffer" );
  algc2.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromModelParameter( "INPUT" ) } );

  QMap<QString, QgsProcessingModelOutput> outputs;
  QgsProcessingModelOutput out1;
  out1.setChildOutputName( u"OUTPUT"_s );
  outputs.insert( u"a"_s, out1 );
  algc2.setModelOutputs( outputs );

  m.addChildAlgorithm( algc2 );

  QCOMPARE( m.orderedOutputs().size(), 1 );
  QCOMPARE( m.orderedOutputs().at( 0 ).childId(), u"cx2"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).name(), u"a"_s );

  QgsProcessingModelChildAlgorithm algc3;
  algc3.setChildId( u"cx3"_s );
  algc3.setAlgorithmId( "native:buffer" );
  algc3.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromModelParameter( "INPUT" ) } );

  outputs.clear();
  QgsProcessingModelOutput out2;
  out2.setChildOutputName( u"OUTPUT"_s );
  outputs.insert( u"b"_s, out2 );
  algc3.setModelOutputs( outputs );

  m.addChildAlgorithm( algc3 );

  QCOMPARE( m.orderedOutputs().size(), 2 );
  QCOMPARE( m.orderedOutputs().at( 0 ).childId(), u"cx2"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).name(), u"a"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childId(), u"cx3"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).name(), u"b"_s );

  QgsProcessingModelChildAlgorithm algc4;
  algc4.setChildId( u"cx4"_s );
  algc4.setAlgorithmId( "native:buffer" );
  algc4.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromModelParameter( "INPUT" ) } );

  outputs.clear();
  QgsProcessingModelOutput out3;
  out3.setChildOutputName( u"OUTPUT"_s );
  outputs.insert( u"c"_s, out2 );
  algc4.setModelOutputs( outputs );

  m.addChildAlgorithm( algc4 );

  QCOMPARE( m.orderedOutputs().size(), 3 );
  QCOMPARE( m.orderedOutputs().at( 0 ).childId(), u"cx2"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).name(), u"a"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childId(), u"cx3"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).name(), u"b"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).childId(), u"cx4"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).name(), u"c"_s );

  // set specific output order (incomplete, and with some non-matching values)
  m.setOutputOrder( { u"cx3:OUTPUT"_s, u"cx2:OUTPUT"_s, u"cx1:OUTPUT"_s } );
  QCOMPARE( m.orderedOutputs().size(), 3 );
  QCOMPARE( m.orderedOutputs().at( 0 ).childId(), u"cx3"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).name(), u"b"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childId(), u"cx2"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).name(), u"a"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).childId(), u"cx4"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).name(), u"c"_s );

  // set specific output order, complete
  m.setOutputOrder( { u"cx3:OUTPUT"_s, u"cx4:OUTPUT"_s, u"cx2:OUTPUT"_s } );
  QCOMPARE( m.orderedOutputs().size(), 3 );
  QCOMPARE( m.orderedOutputs().at( 0 ).childId(), u"cx3"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 0 ).name(), u"b"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childId(), u"cx4"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 1 ).name(), u"c"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).childId(), u"cx2"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m.orderedOutputs().at( 2 ).name(), u"a"_s );

  // save/restore
  QgsProcessingModelAlgorithm m2;
  m2.loadVariant( m.toVariant() );
  QCOMPARE( m2.outputGroup(), u"output group"_s );
  QCOMPARE( m2.orderedOutputs().size(), 3 );
  QCOMPARE( m2.orderedOutputs().at( 0 ).childId(), u"cx3"_s );
  QCOMPARE( m2.orderedOutputs().at( 0 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m2.orderedOutputs().at( 0 ).name(), u"b"_s );
  QCOMPARE( m2.orderedOutputs().at( 1 ).childId(), u"cx4"_s );
  QCOMPARE( m2.orderedOutputs().at( 1 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m2.orderedOutputs().at( 1 ).name(), u"c"_s );
  QCOMPARE( m2.orderedOutputs().at( 2 ).childId(), u"cx2"_s );
  QCOMPARE( m2.orderedOutputs().at( 2 ).childOutputName(), u"OUTPUT"_s );
  QCOMPARE( m2.orderedOutputs().at( 2 ).name(), u"a"_s );

  // also run and check context details
  QgsProcessingContext context;
  QgsProcessingFeedback feedback;
  QVariantMap params;
  QgsVectorLayer *layer3111 = new QgsVectorLayer( "Point?crs=epsg:3111", "v1", "memory" );
  QgsProject p;
  p.addMapLayer( layer3111 );
  context.setProject( &p );
  params.insert( u"INPUT"_s, u"v1"_s );
  params.insert( u"cx2:a"_s, QgsProcessingOutputLayerDefinition( QgsProcessing::TEMPORARY_OUTPUT, &p ) );
  params.insert( u"cx3:b"_s, QgsProcessingOutputLayerDefinition( QgsProcessing::TEMPORARY_OUTPUT, &p ) );
  params.insert( u"cx4:c"_s, QgsProcessingOutputLayerDefinition( QgsProcessing::TEMPORARY_OUTPUT, &p ) );

  QVariantMap results = m.run( params, context, &feedback );
  const QString destA = results.value( u"cx2:a"_s ).toString();
  QVERIFY( !destA.isEmpty() );
  QCOMPARE( context.layerToLoadOnCompletionDetails( destA ).groupName, u"output group"_s );
  QCOMPARE( context.layerToLoadOnCompletionDetails( destA ).layerSortKey, 2 );

  const QString destB = results.value( u"cx3:b"_s ).toString();
  QVERIFY( !destB.isEmpty() );
  QCOMPARE( context.layerToLoadOnCompletionDetails( destB ).groupName, u"output group"_s );
  QCOMPARE( context.layerToLoadOnCompletionDetails( destB ).layerSortKey, 0 );

  const QString destC = results.value( u"cx4:c"_s ).toString();
  QVERIFY( !destC.isEmpty() );
  QCOMPARE( context.layerToLoadOnCompletionDetails( destC ).groupName, u"output group"_s );
  QCOMPARE( context.layerToLoadOnCompletionDetails( destC ).layerSortKey, 1 );

  // not all layers are set to load in project
  QgsProcessingContext context2;
  context2.setProject( &p );
  params.clear();
  params.insert( u"INPUT"_s, u"v1"_s );
  // should not be loaded on completion:
  params.insert( u"cx2:a"_s, QgsProcessing::TEMPORARY_OUTPUT );
  // should be loaded on completion:
  params.insert( u"cx3:b"_s, QgsProcessingOutputLayerDefinition( QgsProcessing::TEMPORARY_OUTPUT, &p ) );
  params.insert( u"cx4:c"_s, QgsProcessingOutputLayerDefinition( QgsProcessing::TEMPORARY_OUTPUT, &p ) );

  QVariantMap results2 = m.run( params, context2, &feedback );
  const QString destA2 = results2.value( u"cx2:a"_s ).toString();
  QVERIFY( !destA2.isEmpty() );
  QVERIFY( !context2.willLoadLayerOnCompletion( destA2 ) );

  const QString destB2 = results2.value( u"cx3:b"_s ).toString();
  QVERIFY( !destB2.isEmpty() );
  QCOMPARE( context2.layerToLoadOnCompletionDetails( destB2 ).groupName, u"output group"_s );
  QCOMPARE( context2.layerToLoadOnCompletionDetails( destB2 ).layerSortKey, 0 );

  const QString destC2 = results2.value( u"cx4:c"_s ).toString();
  QVERIFY( !destC2.isEmpty() );
  QCOMPARE( context2.layerToLoadOnCompletionDetails( destC2 ).groupName, u"output group"_s );
  QCOMPARE( context2.layerToLoadOnCompletionDetails( destC2 ).layerSortKey, 1 );
}


void TestQgsProcessingModelAlgorithm::modelWithChildException()
{
  QgsProcessingModelAlgorithm m;

  const QgsProcessingModelParameter sourceParam( "INPUT" );
  m.addModelParameter( new QgsProcessingParameterFeatureSource( "INPUT" ), sourceParam );

  QgsProcessingModelChildAlgorithm algWhichCreatesLayer;
  algWhichCreatesLayer.setChildId( u"buffer"_s );
  algWhichCreatesLayer.setAlgorithmId( "native:buffer" );
  algWhichCreatesLayer.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromModelParameter( "INPUT" ) } );

  m.addChildAlgorithm( algWhichCreatesLayer );

  QgsProcessingModelChildAlgorithm algWhichRaisesException;
  algWhichRaisesException.setChildId( u"raise"_s );
  algWhichRaisesException.setDescription( u"my second step"_s );
  algWhichRaisesException.setAlgorithmId( "dummy4:raise" );
  algWhichRaisesException.setDependencies( { QgsProcessingModelChildDependency( u"buffer"_s ) } );
  m.addChildAlgorithm( algWhichRaisesException );

  // run and check context details
  QgsProcessingContext context;
  context.setLogLevel( Qgis::ProcessingLogLevel::ModelDebug );
  QgsProcessingFeedback feedback;
  QVariantMap params;
  QgsVectorLayer *layer3111 = new QgsVectorLayer( "Point?crs=epsg:3111", "v1", "memory" );
  QgsProject p;
  p.addMapLayer( layer3111 );
  context.setProject( &p );
  params.insert( u"INPUT"_s, u"v1"_s );

  bool ok = false;
  m.run( params, context, &feedback, &ok );
  // model should fail, exception was raised
  QVERIFY( !ok );
  // but result from successful buffer step should still be available in the context
  QCOMPARE( context.temporaryLayerStore()->count(), 1 );
  // confirm that QgsProcessingAlgorithm::postProcess was called for failing DummyRaiseExceptionAlgorithm step
  QVERIFY( DummyRaiseExceptionAlgorithm::hasPostProcessed );
  // but not DummyRaiseExceptionAlgorithm::postProcessAlgorithm
  QVERIFY( !DummyRaiseExceptionAlgorithm::postProcessAlgorithmCalled );

  // results and inputs from buffer child should be available through the context
  QCOMPARE( context.modelResult().childResults().value( "buffer" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( context.modelResult().childResults().value( "buffer" ).inputs().value( "INPUT" ).toString(), u"v1"_s );
  QCOMPARE( context.modelResult().childResults().value( "buffer" ).inputs().value( "OUTPUT" ).toString(), u"memory:Buffered"_s );
  QCOMPARE( context.modelResult().childResults().value( "buffer" ).htmlLog().left( 50 ), u"<span style=\"color:#777\">Prepare algorithm: buffer"_s );
  QCOMPARE( context.modelResult().childResults().value( "buffer" ).htmlLog().right( 21 ), u"s (1 output(s)).<br/>"_s );
  QVERIFY( context.temporaryLayerStore()->mapLayer( context.modelResult().childResults().value( "buffer" ).outputs().value( "OUTPUT" ).toString() ) );
  QCOMPARE( context.modelResult().rawChildInputs().value( "buffer" ).toMap().value( "INPUT" ).toString(), u"v1"_s );
  QCOMPARE( context.modelResult().rawChildInputs().value( "buffer" ).toMap().value( "OUTPUT" ).toString(), u"memory:Buffered"_s );
  QCOMPARE( context.modelResult().rawChildOutputs().value( "buffer" ).toMap().value( "OUTPUT" ).toString(), context.modelResult().childResults().value( "buffer" ).outputs().value( "OUTPUT" ).toString() );

  QCOMPARE( context.modelResult().childResults().value( "raise" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Failed );
  QCOMPARE( context.modelResult().childResults().value( "raise" ).htmlLog().left( 49 ), u"<span style=\"color:#777\">Prepare algorithm: raise"_s );
  QVERIFY( context.modelResult().childResults().value( "raise" ).htmlLog().contains( u"Error encountered while running my second step: something bad happened"_s ) );

  QSet<QString> expected { u"buffer"_s };
  QCOMPARE( context.modelResult().executedChildIds(), expected );
}

void TestQgsProcessingModelAlgorithm::modelExecuteWithPreviousState()
{
  QgsProcessingModelAlgorithm m;

  const QgsProcessingModelParameter sourceParam( "test" );
  m.addModelParameter( new QgsProcessingParameterString( "test" ), sourceParam );

  QgsProcessingModelChildAlgorithm childAlgorithm;
  childAlgorithm.setChildId( u"calculate"_s );
  childAlgorithm.setAlgorithmId( "native:calculateexpression" );
  childAlgorithm.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromExpression( " @test || '_1'" ) } );
  m.addChildAlgorithm( childAlgorithm );

  QgsProcessingModelChildAlgorithm childAlgorithm2;
  childAlgorithm2.setChildId( u"calculate2"_s );
  childAlgorithm2.setAlgorithmId( "native:calculateexpression" );
  childAlgorithm2.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromExpression( " @calculate_OUTPUT  || '_2'" ) } );
  childAlgorithm2.setDependencies( { QgsProcessingModelChildDependency( u"calculate"_s ) } );
  m.addChildAlgorithm( childAlgorithm2 );

  // run and check context details
  QgsProcessingContext context;
  context.setLogLevel( Qgis::ProcessingLogLevel::ModelDebug );
  QgsProcessingFeedback feedback;
  QVariantMap params;
  params.insert( u"test"_s, u"my string"_s );

  // start with no initial state
  bool ok = false;
  m.run( params, context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( context.modelResult().childResults().value( "calculate" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( context.modelResult().childResults().value( "calculate" ).inputs().value( "INPUT" ).toString(), u"my string_1"_s );
  QCOMPARE( context.modelResult().childResults().value( "calculate" ).outputs().value( "OUTPUT" ).toString(), u"my string_1"_s );
  QCOMPARE( context.modelResult().rawChildInputs().value( "calculate" ).toMap().value( "INPUT" ).toString(), u"my string_1"_s );
  QCOMPARE( context.modelResult().rawChildOutputs().value( "calculate" ).toMap().value( "OUTPUT" ).toString(), u"my string_1"_s );

  QCOMPARE( context.modelResult().childResults().value( "calculate2" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( context.modelResult().childResults().value( "calculate2" ).inputs().value( "INPUT" ).toString(), u"my string_1_2"_s );
  QCOMPARE( context.modelResult().childResults().value( "calculate2" ).outputs().value( "OUTPUT" ).toString(), u"my string_1_2"_s );
  QCOMPARE( context.modelResult().rawChildInputs().value( "calculate2" ).toMap().value( "INPUT" ).toString(), u"my string_1_2"_s );
  QCOMPARE( context.modelResult().rawChildOutputs().value( "calculate2" ).toMap().value( "OUTPUT" ).toString(), u"my string_1_2"_s );

  QSet<QString> expected { u"calculate"_s, u"calculate2"_s };
  QCOMPARE( context.modelResult().executedChildIds(), expected );
  QgsProcessingModelResult firstResult = context.modelResult();

  context.modelResult().clear();
  // start with an initial state

  auto modelConfig = std::make_unique<QgsProcessingModelInitialRunConfig>();
  modelConfig->setPreviouslyExecutedChildAlgorithms( { u"calculate"_s } );
  modelConfig->setInitialChildInputs( QVariantMap { { u"calculate"_s, QVariantMap { { u"INPUT"_s, u"a different string"_s } } } } );
  modelConfig->setInitialChildOutputs( QVariantMap { { u"calculate"_s, QVariantMap { { u"OUTPUT"_s, u"a different string"_s } } } } );
  context.setModelInitialRunConfig( std::move( modelConfig ) );

  m.run( params, context, &feedback, &ok );
  QVERIFY( ok );
  // "calculate" should not be re-executed
  QCOMPARE( context.modelResult().childResults().value( "calculate" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::NotExecuted );
  QVERIFY( context.modelResult().childResults().value( "calculate" ).inputs().isEmpty() );
  QVERIFY( context.modelResult().childResults().value( "calculate" ).outputs().isEmpty() );

  // the second child algorithm should be re-run
  QCOMPARE( context.modelResult().childResults().value( "calculate2" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( context.modelResult().childResults().value( "calculate2" ).inputs().value( "INPUT" ).toString(), u"a different string_2"_s );
  QCOMPARE( context.modelResult().childResults().value( "calculate2" ).outputs().value( "OUTPUT" ).toString(), u"a different string_2"_s );
  QCOMPARE( context.modelResult().rawChildInputs().value( "calculate2" ).toMap().value( "INPUT" ).toString(), u"a different string_2"_s );
  QCOMPARE( context.modelResult().rawChildOutputs().value( "calculate2" ).toMap().value( "OUTPUT" ).toString(), u"a different string_2"_s );

  expected = QSet<QString> { u"calculate"_s, u"calculate2"_s };
  QCOMPARE( context.modelResult().executedChildIds(), expected );

  // config should be discarded, it should never be re-used or passed on to non top-level models
  QVERIFY( !context.modelInitialRunConfig() );

  // merge with first result, to get complete set of results across both executions
  firstResult.mergeWith( context.modelResult() );
  QCOMPARE( firstResult.childResults().value( "calculate" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( firstResult.childResults().value( "calculate" ).inputs().value( "INPUT" ).toString(), u"my string_1"_s );
  QCOMPARE( firstResult.childResults().value( "calculate" ).outputs().value( "OUTPUT" ).toString(), u"my string_1"_s );
  QCOMPARE( firstResult.rawChildInputs().value( "calculate" ).toMap().value( "INPUT" ).toString(), u"a different string"_s );
  QCOMPARE( firstResult.rawChildOutputs().value( "calculate" ).toMap().value( "OUTPUT" ).toString(), u"a different string"_s );
  QCOMPARE( firstResult.childResults().value( "calculate2" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( firstResult.childResults().value( "calculate2" ).inputs().value( "INPUT" ).toString(), u"a different string_2"_s );
  QCOMPARE( firstResult.childResults().value( "calculate2" ).outputs().value( "OUTPUT" ).toString(), u"a different string_2"_s );
  QCOMPARE( firstResult.rawChildInputs().value( "calculate2" ).toMap().value( "INPUT" ).toString(), u"a different string_2"_s );
  QCOMPARE( firstResult.rawChildOutputs().value( "calculate2" ).toMap().value( "OUTPUT" ).toString(), u"a different string_2"_s );

  QCOMPARE( context.temporaryLayerStore()->count(), 0 );

  // test handling of temporary layers generated during earlier runs
  modelConfig = std::make_unique<QgsProcessingModelInitialRunConfig>();

  auto previousStore = std::make_unique<QgsMapLayerStore>();
  QgsVectorLayer *layer = new QgsVectorLayer( "Point?crs=epsg:3111", "v1", "memory" );
  previousStore->addMapLayer( layer );
  previousStore->moveToThread( nullptr );
  modelConfig->setPreviousLayerStore( std::move( previousStore ) );

  context.setModelInitialRunConfig( std::move( modelConfig ) );
  m.run( params, context, &feedback, &ok );
  QVERIFY( ok );
  // layer should have been transferred to context's temporary layer store as part of model execution
  QCOMPARE( context.temporaryLayerStore()->count(), 1 );
  QCOMPARE( context.temporaryLayerStore()->mapLayersByName( u"v1"_s ).at( 0 ), layer );
}

void TestQgsProcessingModelAlgorithm::modelExecuteWithPreviousStateNoLeak()
{
  QgsProcessingModelAlgorithm m;

  QgsProcessingModelChildAlgorithm childAlgorithm;
  childAlgorithm.setChildId( u"calculate"_s );
  childAlgorithm.setAlgorithmId( "native:calculateexpression" );
  childAlgorithm.addParameterSources( "INPUT", { QgsProcessingModelChildParameterSource::fromExpression( " 'from outer'" ) } );
  QgsProcessingModelOutput childOutput1( "OUTPUT" );
  childOutput1.setChildId( u"calculate"_s );
  childOutput1.setChildOutputName( "OUTPUT" );
  childOutput1.setDescription( u"my output"_s );
  childAlgorithm.setModelOutputs( { { u"OUTPUT"_s, childOutput1 } } );
  m.addChildAlgorithm( childAlgorithm );

  QgsProcessingModelChildAlgorithm nestedModel;
  nestedModel.setChildId( u"childmodel"_s );
  nestedModel.setAlgorithmId( "dummy4:dummymodel" );
  // want outer scope to run first
  nestedModel.setDependencies( { QgsProcessingModelChildDependency( u"calculate"_s ) } );
  QgsProcessingModelOutput childOutput( "nestedout" );
  childOutput.setChildId( u"childmodel"_s );
  childOutput.setChildOutputName( "OUTPUT" );
  childOutput.setDescription( u"my output2"_s );
  nestedModel.setModelOutputs( { { u"nestedout"_s, childOutput } } );
  m.addChildAlgorithm( nestedModel );

  // run and check context details
  QgsProcessingContext context;
  context.setLogLevel( Qgis::ProcessingLogLevel::ModelDebug );
  QgsProcessingFeedback feedback;
  QVariantMap params;

  // start with no initial state
  bool ok = false;
  m.run( params, context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( context.modelResult().childResults().value( "calculate" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QCOMPARE( context.modelResult().childResults().value( "childmodel" ).executionStatus(), Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
  QSet<QString> expected { u"calculate"_s, u"childmodel"_s };
  QCOMPARE( context.modelResult().executedChildIds(), expected );
  QgsProcessingModelResult firstResult = context.modelResult();
  QCOMPARE( context.modelResult().childResults().value( "childmodel" ).outputs().value( u"CHILD_RESULTS"_s ).toMap().value( u"calculate"_s ).toMap().value( u"OUTPUT"_s ).toString(), u"value_from_child"_s );
  QCOMPARE( context.modelResult().childResults().value( u"calculate"_s ).outputs().value( u"OUTPUT"_s ).toString(), u"from outer"_s );
}

void TestQgsProcessingModelAlgorithm::modelDependencies()
{
  const QgsProcessingModelChildDependency dep( u"childId"_s, u"branch"_s );

  QCOMPARE( dep.childId, u"childId"_s );
  QCOMPARE( dep.conditionalBranch, u"branch"_s );

  const QVariant v = dep.toVariant();
  QgsProcessingModelChildDependency dep2;
  QVERIFY( dep2.loadVariant( v.toMap() ) );

  QCOMPARE( dep2.childId, u"childId"_s );
  QCOMPARE( dep2.conditionalBranch, u"branch"_s );

  QVERIFY( dep == dep2 );
  QVERIFY( !( dep != dep2 ) );
  dep2.conditionalBranch = u"b"_s;

  QVERIFY( !( dep == dep2 ) );
  QVERIFY( dep != dep2 );
  dep2.conditionalBranch = u"branch"_s;
  dep2.childId = u"c"_s;
  QVERIFY( !( dep == dep2 ) );
  QVERIFY( dep != dep2 );
  dep2.childId = u"childId"_s;
  QVERIFY( dep == dep2 );
  QVERIFY( !( dep != dep2 ) );
}

void TestQgsProcessingModelAlgorithm::modelSource()
{
  QgsProcessingModelChildParameterSource source;
  source.setExpression( u"expression"_s );
  source.setExpressionText( u"expression string"_s );
  source.setOutputName( u"output name "_s );
  source.setStaticValue( QString( "value" ) );
  source.setOutputChildId( u"output child id"_s );
  source.setParameterName( u"parameter name"_s );
  source.setSource( Qgis::ProcessingModelChildParameterSource::ChildOutput );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << source;

  ds.device()->seek( 0 );

  QgsProcessingModelChildParameterSource res;
  ds >> res;

  QCOMPARE( res.expression(), u"expression"_s );
  QCOMPARE( res.expressionText(), u"expression string"_s );
  QCOMPARE( res.outputName(), u"output name "_s );
  QCOMPARE( res.staticValue().toString(), u"value"_s );
  QCOMPARE( res.outputChildId(), u"output child id"_s );
  QCOMPARE( res.parameterName(), u"parameter name"_s );
  QCOMPARE( res.source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
}

void TestQgsProcessingModelAlgorithm::modelNameMatchesFileName()
{
  QgsProcessingModelAlgorithm model;
  model.setName( u"my name"_s );
  QVERIFY( !model.modelNameMatchesFilePath() );

  model.setSourceFilePath( u"/home/me/my name.something.model3"_s );
  QVERIFY( !model.modelNameMatchesFilePath() );
  model.setSourceFilePath( u"/home/me/my name.model3"_s );
  QVERIFY( model.modelNameMatchesFilePath() );
  model.setSourceFilePath( u"/home/me/MY NAME.model3"_s );
  QVERIFY( model.modelNameMatchesFilePath() );
}

void TestQgsProcessingModelAlgorithm::renameModelParameter()
{
  // test renaming a model parameter correctly updates all child algorithm sources
  // to match

  QgsProcessingModelAlgorithm m;
  const QgsProcessingModelParameter stringParam1( "oldName" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );

  const QgsProcessingModelParameter stringParam2( "string2" );
  m.addModelParameter( new QgsProcessingParameterString( "string2" ), stringParam2 );

  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( u"cx1"_s );
  algc1.setAlgorithmId( "native:buffer" );

  algc1.addParameterSources( u"CHILD_OUTPUT"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromChildOutput( u"filter"_s, u"VECTOR"_s ) );
  algc1.addParameterSources( u"STATIC_VALUE"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  algc1.addParameterSources( u"STATIC_VALUE_DD_EXPERESSION"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromStaticValue( QgsProperty::fromExpression( u"@oldName * 2 + @string2"_s ) ) );
  algc1.addParameterSources( u"MODEL_PARAM_1"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromModelParameter( u"oldName"_s ) );
  algc1.addParameterSources( u"MODEL_PARAM_2"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromModelParameter( u"string2"_s ) );
  algc1.addParameterSources( u"EXPRESSION"_s, QList<QgsProcessingModelChildParameterSource>() << QgsProcessingModelChildParameterSource::fromExpression( u"@oldName * 2 + @string2"_s ) );

  m.addChildAlgorithm( algc1 );

  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputChildId(), u"filter"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputName(), u"VECTOR"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE"_s].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE_DD_EXPERESSION"_s].constFirst().staticValue().value<QgsProperty>().expressionString(), u"@oldName * 2 + @string2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_1"_s].constFirst().parameterName(), u"oldName"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_2"_s].constFirst().parameterName(), u"string2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"EXPRESSION"_s].constFirst().expression(), u"@oldName * 2 + @string2"_s );

  m.changeParameterName( u"x"_s, u"y"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputChildId(), u"filter"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputName(), u"VECTOR"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE"_s].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE_DD_EXPERESSION"_s].constFirst().staticValue().value<QgsProperty>().expressionString(), u"@oldName * 2 + @string2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_1"_s].constFirst().parameterName(), u"oldName"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_2"_s].constFirst().parameterName(), u"string2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"EXPRESSION"_s].constFirst().expression(), u"@oldName * 2 + @string2"_s );

  m.changeParameterName( u"oldName"_s, u"apricot"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputChildId(), u"filter"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputName(), u"VECTOR"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE"_s].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE_DD_EXPERESSION"_s].constFirst().staticValue().value<QgsProperty>().expressionString(), u"@apricot * 2 + @string2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_1"_s].constFirst().parameterName(), u"apricot"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_2"_s].constFirst().parameterName(), u"string2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"EXPRESSION"_s].constFirst().expression(), u"@apricot * 2 + @string2"_s );

  m.changeParameterName( u"string2"_s, u"int2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputChildId(), u"filter"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"CHILD_OUTPUT"_s].constFirst().outputName(), u"VECTOR"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE"_s].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"STATIC_VALUE_DD_EXPERESSION"_s].constFirst().staticValue().value<QgsProperty>().expressionString(), u"@apricot * 2 + @int2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_1"_s].constFirst().parameterName(), u"apricot"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"MODEL_PARAM_2"_s].constFirst().parameterName(), u"int2"_s );
  QCOMPARE( m.childAlgorithm( u"cx1"_s ).parameterSources()[u"EXPRESSION"_s].constFirst().expression(), u"@apricot * 2 + @int2"_s );
}

void TestQgsProcessingModelAlgorithm::internalVersion()
{
  // test internal version handling
  QgsProcessingModelAlgorithm model;

  // load older model, should be version 1
  QVERIFY( model.fromFile( TEST_DATA_DIR + u"/test_model.model3"_s ) );
  QCOMPARE( model.mInternalVersion, QgsProcessingModelAlgorithm::InternalVersion::Version1 );

  // create new model and save/restore, should be version 2
  QgsProcessingModelAlgorithm model2;
  QgsProcessingModelAlgorithm model3;
  QVERIFY( model3.loadVariant( model2.toVariant() ) );
  QCOMPARE( model3.mInternalVersion, QgsProcessingModelAlgorithm::InternalVersion::Version2 );
}

void TestQgsProcessingModelAlgorithm::modelChildOrderWithVariables()
{
  // dependencies
  QgsProcessingModelAlgorithm model( "test", "testGroup" );

  const QgsProcessingModelParameter stringParam( "a_parameter" );
  model.addModelParameter( new QgsProcessingParameterString( "a_parameter" ), stringParam );

  QgsProcessingModelChildAlgorithm c1;
  c1.setChildId( u"c1"_s );
  c1.setAlgorithmId( u"native:stringconcatenation"_s );
  // a parameter source from an expression which isn't coming from another child algorithm
  c1.setParameterSources(
    { { u"INPUT_2"_s, { QgsProcessingModelChildParameterSource::fromExpression( u"@a_parameter || 'x'"_s ) } }
    }
  );
  model.addChildAlgorithm( c1 );

  QgsProcessingModelChildAlgorithm c2;
  c2.setChildId( u"c2"_s );
  c2.setAlgorithmId( u"native:stringconcatenation"_s );
  c2.setParameterSources(
    { { u"INPUT_1"_s, { QgsProcessingModelChildParameterSource::fromExpression( u"@c1_CONCATENATION || 'x'"_s ) } }
    }
  );
  model.addChildAlgorithm( c2 );

  QgsProcessingContext context;
  QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = model.variablesForChildAlgorithm( u"c2"_s, &context );
  QCOMPARE( variables.size(), 2 );
  QCOMPARE( variables.firstKey(), u"a_parameter"_s );
  QCOMPARE( variables.lastKey(), u"c1_CONCATENATION"_s );

  QVERIFY( model.dependsOnChildAlgorithms( u"c1"_s ).isEmpty() );
  QCOMPARE( model.dependsOnChildAlgorithms( u"c2"_s ).size(), 1 );
  QCOMPARE( *model.dependsOnChildAlgorithms( u"c2"_s ).constBegin(), u"c1"_s );

  QgsProcessingModelChildAlgorithm c3;
  c3.setChildId( u"c3"_s );
  c3.setAlgorithmId( u"native:stringconcatenation"_s );

  // make c1 dependent on c3's output via a variable
  model.childAlgorithm( u"c1"_s ).addParameterSources( u"INPUT_1"_s, QList<QgsProcessingModelChildParameterSource> { QgsProcessingModelChildParameterSource::fromExpression( u"@c3_CONCATENATION || 'x'"_s ) } );
  model.addChildAlgorithm( c3 );

  QCOMPARE( model.dependsOnChildAlgorithms( u"c3"_s ).size(), 0 );
  QCOMPARE( model.dependsOnChildAlgorithms( u"c1"_s ).size(), 1 );
  QCOMPARE( *model.dependsOnChildAlgorithms( u"c1"_s ).constBegin(), u"c3"_s );
  QCOMPARE( model.dependsOnChildAlgorithms( u"c2"_s ).size(), 2 );
  QVERIFY( model.dependsOnChildAlgorithms( u"c2"_s ).contains( u"c1"_s ) );
  QVERIFY( model.dependsOnChildAlgorithms( u"c2"_s ).contains( u"c3"_s ) );

  // circular dependency -- this is ok, we just don't want to hang
  model.childAlgorithm( u"c3"_s ).setParameterSources( { { u"INPUT_1"_s, { QgsProcessingModelChildParameterSource::fromExpression( u"@c2_CONCATENATION || 'x'"_s ) } } } );

  QCOMPARE( model.dependsOnChildAlgorithms( u"c3"_s ).size(), 2 );
  QVERIFY( model.dependsOnChildAlgorithms( u"c3"_s ).contains( u"c1"_s ) );
  QVERIFY( model.dependsOnChildAlgorithms( u"c3"_s ).contains( u"c2"_s ) );
  QCOMPARE( model.dependsOnChildAlgorithms( u"c1"_s ).size(), 2 );
  QVERIFY( model.dependsOnChildAlgorithms( u"c1"_s ).contains( u"c2"_s ) );
  QVERIFY( model.dependsOnChildAlgorithms( u"c1"_s ).contains( u"c3"_s ) );
  QCOMPARE( model.dependsOnChildAlgorithms( u"c2"_s ).size(), 2 );
  QVERIFY( model.dependsOnChildAlgorithms( u"c2"_s ).contains( u"c1"_s ) );
  QVERIFY( model.dependsOnChildAlgorithms( u"c2"_s ).contains( u"c3"_s ) );
}

void TestQgsProcessingModelAlgorithm::flags()
{
  QgsProcessingModelAlgorithm model( "test", "testGroup" );

  const QgsProcessingModelParameter stringParam( "a_parameter" );
  model.addModelParameter( new QgsProcessingParameterString( "a_parameter" ), stringParam );

  QgsProcessingModelChildAlgorithm c1;
  c1.setChildId( u"c1"_s );
  c1.setAlgorithmId( u"native:stringconcatenation"_s );
  model.addChildAlgorithm( c1 );
  QVERIFY( !model.flags().testFlag( Qgis::ProcessingAlgorithmFlag::SecurityRisk ) );

  // add algorithm with security risk
  QgsProcessingModelChildAlgorithm c2;
  c2.setChildId( u"c2"_s );
  c2.setAlgorithmId( u"dummy4:risky"_s );
  model.addChildAlgorithm( c2 );
  QVERIFY( model.flags().testFlag( Qgis::ProcessingAlgorithmFlag::SecurityRisk ) );
}

QGSTEST_MAIN( TestQgsProcessingModelAlgorithm )
#include "testqgsprocessingmodelalgorithm.moc"
