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
#include "qgsprocessingmodelalgorithm.h"

#include <QObject>
#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsnativealgorithms.h"
#include "qgsprocessingregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsxmlutils.h"
#include "qgsprocessingprovider.h"


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

class DummyProvider4 : public QgsProcessingProvider // clazy:exclude=missing-qobject-macro
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
      QVERIFY( addAlgorithm( new DummyAlgorithm2( QStringLiteral( "alg1" ) ) ) );
    }

};


class TestQgsProcessingModelAlgorithm: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
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
    void modelDependencies();
    void modelSource();
    void modelNameMatchesFileName();
    void renameModelParameter();
    void internalVersion();

  private:

};

void TestQgsProcessingModelAlgorithm::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsSettings settings;
  settings.clear();

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestQgsProcessingModelAlgorithm::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingModelAlgorithm::modelScope()
{
  QgsProcessingContext pc;

  QgsProcessingModelAlgorithm alg( QStringLiteral( "test" ), QStringLiteral( "testGroup" ) );

  QVERIFY( !alg.fromFile( QStringLiteral( "not a file" ) ) );

  QVariantMap variables;
  variables.insert( QStringLiteral( "v1" ), 5 );
  variables.insert( QStringLiteral( "v2" ), QStringLiteral( "aabbccd" ) );
  alg.setVariables( variables );

  QVariantMap params;
  params.insert( QStringLiteral( "a_param" ), 5 );
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::processingModelAlgorithmScope( &alg, params, pc ) );
  QVERIFY( scope.get() );
  QCOMPARE( scope->variable( QStringLiteral( "model_name" ) ).toString(), QStringLiteral( "test" ) );
  QCOMPARE( scope->variable( QStringLiteral( "model_group" ) ).toString(), QStringLiteral( "testGroup" ) );
  QVERIFY( scope->hasVariable( QStringLiteral( "model_path" ) ) );
  QVERIFY( scope->hasVariable( QStringLiteral( "model_folder" ) ) );
  QCOMPARE( scope->variable( QStringLiteral( "model_path" ) ).toString(), QString() );
  QCOMPARE( scope->variable( QStringLiteral( "model_folder" ) ).toString(), QString() );
  QCOMPARE( scope->variable( QStringLiteral( "v1" ) ).toInt(), 5 );
  QCOMPARE( scope->variable( QStringLiteral( "v2" ) ).toString(), QStringLiteral( "aabbccd" ) );

  QgsProject p;
  pc.setProject( &p );
  p.setFileName( TEST_DATA_DIR + QStringLiteral( "/test_file.qgs" ) );
  scope.reset( QgsExpressionContextUtils::processingModelAlgorithmScope( &alg, params, pc ) );
  QCOMPARE( scope->variable( QStringLiteral( "model_path" ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/test_file.qgs" ) ) );
  QCOMPARE( scope->variable( QStringLiteral( "model_folder" ) ).toString(), QStringLiteral( TEST_DATA_DIR ) );

  alg.setSourceFilePath( TEST_DATA_DIR + QStringLiteral( "/processing/my_model.model3" ) );
  scope.reset( QgsExpressionContextUtils::processingModelAlgorithmScope( &alg, params, pc ) );
  QCOMPARE( scope->variable( QStringLiteral( "model_path" ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/processing/my_model.model3" ) ) );
  QCOMPARE( scope->variable( QStringLiteral( "model_folder" ) ).toString(), QString( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/processing" ) ) );

  const QgsExpressionContext ctx = alg.createExpressionContext( QVariantMap(), pc );
  QVERIFY( scope->hasVariable( QStringLiteral( "model_path" ) ) );
  QVERIFY( scope->hasVariable( QStringLiteral( "model_folder" ) ) );
}

void TestQgsProcessingModelAlgorithm::modelerAlgorithm()
{
  //static value source
  QgsProcessingModelChildParameterSource svSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  QCOMPARE( svSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( svSource.staticValue().toInt(), 5 );
  svSource.setStaticValue( 7 );
  QCOMPARE( svSource.staticValue().toInt(), 7 );
  QMap< QString, QString > friendlyNames;
  QCOMPARE( svSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "7" ) );
  svSource = QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) );
  // check that calling setStaticValue flips source to StaticValue
  QCOMPARE( svSource.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( svSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "parameters['a']" ) );
  svSource.setStaticValue( 7 );
  QCOMPARE( svSource.staticValue().toInt(), 7 );
  QCOMPARE( svSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( svSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "7" ) );

  // model parameter source
  QgsProcessingModelChildParameterSource mpSource = QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "a" ) );
  QCOMPARE( mpSource.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( mpSource.parameterName(), QStringLiteral( "a" ) );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "parameters['a']" ) );
  mpSource.setParameterName( QStringLiteral( "b" ) );
  QCOMPARE( mpSource.parameterName(), QStringLiteral( "b" ) );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "parameters['b']" ) );
  mpSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setParameterName flips source to ModelParameter
  QCOMPARE( mpSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "5" ) );
  mpSource.setParameterName( QStringLiteral( "c" ) );
  QCOMPARE( mpSource.parameterName(), QStringLiteral( "c" ) );
  QCOMPARE( mpSource.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( mpSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "parameters['c']" ) );

  // child alg output source
  QgsProcessingModelChildParameterSource oSource = QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "a" ), QStringLiteral( "b" ) );
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( oSource.outputChildId(), QStringLiteral( "a" ) );
  QCOMPARE( oSource.outputName(), QStringLiteral( "b" ) );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "outputs['a']['b']" ) );
  // with friendly name
  friendlyNames.insert( QStringLiteral( "a" ), QStringLiteral( "alga" ) );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "outputs['alga']['b']" ) );
  oSource.setOutputChildId( QStringLiteral( "c" ) );
  QCOMPARE( oSource.outputChildId(), QStringLiteral( "c" ) );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "outputs['c']['b']" ) );
  oSource.setOutputName( QStringLiteral( "d" ) );
  QCOMPARE( oSource.outputName(), QStringLiteral( "d" ) );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "outputs['c']['d']" ) );
  oSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setOutputChildId flips source to ChildOutput
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "5" ) );
  oSource.setOutputChildId( QStringLiteral( "c" ) );
  QCOMPARE( oSource.outputChildId(), QStringLiteral( "c" ) );
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  oSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setOutputName flips source to ChildOutput
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( oSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "5" ) );
  oSource.setOutputName( QStringLiteral( "d" ) );
  QCOMPARE( oSource.outputName(), QStringLiteral( "d" ) );
  QCOMPARE( oSource.source(), QgsProcessingModelChildParameterSource::ChildOutput );

  // expression source
  QgsProcessingModelChildParameterSource expSource = QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "1+2" ) );
  QCOMPARE( expSource.source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( expSource.expression(), QStringLiteral( "1+2" ) );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "QgsExpression('1+2').evaluate()" ) );
  expSource.setExpression( QStringLiteral( "1+3" ) );
  QCOMPARE( expSource.expression(), QStringLiteral( "1+3" ) );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "QgsExpression('1+3').evaluate()" ) );
  expSource.setExpression( QStringLiteral( "'a' || 'b\\'c'" ) );
  QCOMPARE( expSource.expression(), QStringLiteral( "'a' || 'b\\'c'" ) );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "QgsExpression(\"'a' || 'b\\\\'c'\").evaluate()" ) );
  expSource = QgsProcessingModelChildParameterSource::fromStaticValue( 5 );
  // check that calling setExpression flips source to Expression
  QCOMPARE( expSource.source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "5" ) );
  expSource.setExpression( QStringLiteral( "1+4" ) );
  QCOMPARE( expSource.expression(), QStringLiteral( "1+4" ) );
  QCOMPARE( expSource.source(), QgsProcessingModelChildParameterSource::Expression );
  QCOMPARE( expSource.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, nullptr, friendlyNames ), QStringLiteral( "QgsExpression('1+4').evaluate()" ) );

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

  // a comment
  QgsProcessingModelComment comment;
  comment.setSize( QSizeF( 9, 8 ) );
  QCOMPARE( comment.size(), QSizeF( 9, 8 ) );
  comment.setPosition( QPointF( 11, 14 ) );
  QCOMPARE( comment.position(), QPointF( 11, 14 ) );
  comment.setDescription( QStringLiteral( "a comment" ) );
  QCOMPARE( comment.description(), QStringLiteral( "a comment" ) );
  comment.setColor( QColor( 123, 45, 67 ) );
  QCOMPARE( comment.color(), QColor( 123, 45, 67 ) );
  std::unique_ptr< QgsProcessingModelComment > commentClone( comment.clone() );
  QCOMPARE( commentClone->toVariant(), comment.toVariant() );
  QCOMPARE( commentClone->size(), QSizeF( 9, 8 ) );
  QCOMPARE( commentClone->position(), QPointF( 11, 14 ) );
  QCOMPARE( commentClone->description(), QStringLiteral( "a comment" ) );
  QCOMPARE( commentClone->color(), QColor( 123, 45, 67 ) );
  QgsProcessingModelComment comment2;
  comment2.loadVariant( comment.toVariant().toMap() );
  QCOMPARE( comment2.size(), QSizeF( 9, 8 ) );
  QCOMPARE( comment2.position(), QPointF( 11, 14 ) );
  QCOMPARE( comment2.description(), QStringLiteral( "a comment" ) );
  QCOMPARE( comment2.color(), QColor( 123, 45, 67 ) );

  // group boxes
  QgsProcessingModelGroupBox groupBox;
  groupBox.setSize( QSizeF( 9, 8 ) );
  QCOMPARE( groupBox.size(), QSizeF( 9, 8 ) );
  groupBox.setPosition( QPointF( 11, 14 ) );
  QCOMPARE( groupBox.position(), QPointF( 11, 14 ) );
  groupBox.setDescription( QStringLiteral( "a comment" ) );
  QCOMPARE( groupBox.description(), QStringLiteral( "a comment" ) );
  groupBox.setColor( QColor( 123, 45, 67 ) );
  QCOMPARE( groupBox.color(), QColor( 123, 45, 67 ) );
  std::unique_ptr< QgsProcessingModelGroupBox > groupClone( groupBox.clone() );
  QCOMPARE( groupClone->toVariant(), groupBox.toVariant() );
  QCOMPARE( groupClone->size(), QSizeF( 9, 8 ) );
  QCOMPARE( groupClone->position(), QPointF( 11, 14 ) );
  QCOMPARE( groupClone->description(), QStringLiteral( "a comment" ) );
  QCOMPARE( groupClone->color(), QColor( 123, 45, 67 ) );
  QCOMPARE( groupClone->uuid(), groupBox.uuid() );
  QgsProcessingModelGroupBox groupBox2;
  groupBox2.loadVariant( groupBox.toVariant().toMap() );
  QCOMPARE( groupBox2.size(), QSizeF( 9, 8 ) );
  QCOMPARE( groupBox2.position(), QPointF( 11, 14 ) );
  QCOMPARE( groupBox2.description(), QStringLiteral( "a comment" ) );
  QCOMPARE( groupBox2.color(), QColor( 123, 45, 67 ) );
  QCOMPARE( groupBox2.uuid(), groupBox.uuid() );

  const QMap< QString, QString > friendlyOutputNames;
  QgsProcessingModelChildAlgorithm child( QStringLiteral( "some_id" ) );
  QCOMPARE( child.algorithmId(), QStringLiteral( "some_id" ) );
  QVERIFY( !child.algorithm() );
  QVERIFY( !child.setAlgorithmId( QStringLiteral( "blah" ) ) );
  QVERIFY( !child.reattach() );
  QVERIFY( child.setAlgorithmId( QStringLiteral( "native:centroids" ) ) );
  QVERIFY( child.algorithm() );
  QCOMPARE( child.algorithm()->id(), QStringLiteral( "native:centroids" ) );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, QgsStringMap(), 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    alg_params = {\n    }\n    outputs[''] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)" ) );
  QgsStringMap extraParams;
  extraParams[QStringLiteral( "SOMETHING" )] = QStringLiteral( "SOMETHING_ELSE" );
  extraParams[QStringLiteral( "SOMETHING2" )] = QStringLiteral( "SOMETHING_ELSE2" );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    alg_params = {\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs[''] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)" ) );
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
  child.setSize( QSizeF( 3, 4 ) );
  QCOMPARE( child.size(), QSizeF( 3, 4 ) );
  QVERIFY( child.linksCollapsed( Qt::TopEdge ) );
  child.setLinksCollapsed( Qt::TopEdge, false );
  QVERIFY( !child.linksCollapsed( Qt::TopEdge ) );
  QVERIFY( child.linksCollapsed( Qt::BottomEdge ) );
  child.setLinksCollapsed( Qt::BottomEdge, false );
  QVERIFY( !child.linksCollapsed( Qt::BottomEdge ) );
  child.comment()->setDescription( QStringLiteral( "com" ) );
  QCOMPARE( child.comment()->description(), QStringLiteral( "com" ) );
  child.comment()->setSize( QSizeF( 56, 78 ) );
  child.comment()->setPosition( QPointF( 111, 122 ) );

  QgsProcessingModelChildAlgorithm other;
  other.setChildId( QStringLiteral( "diff" ) );
  other.setDescription( QStringLiteral( "d2" ) );
  other.setAlgorithmId( QStringLiteral( "alg33" ) );
  other.setLinksCollapsed( Qt::BottomEdge, true );
  other.setLinksCollapsed( Qt::TopEdge, true );
  other.comment()->setDescription( QStringLiteral( "other comment" ) );
  other.copyNonDefinitionProperties( child );
  // only subset of properties should have been copied!
  QCOMPARE( other.description(), QStringLiteral( "d2" ) );
  QCOMPARE( other.position(), QPointF( 1, 2 ) );
  QCOMPARE( other.size(), QSizeF( 3, 4 ) );
  QVERIFY( !other.linksCollapsed( Qt::TopEdge ) );
  QVERIFY( !other.linksCollapsed( Qt::BottomEdge ) );
  QCOMPARE( other.comment()->description(), QStringLiteral( "other comment" ) );
  QCOMPARE( other.comment()->position(), QPointF( 111, 122 ) );
  QCOMPARE( other.comment()->size(), QSizeF( 56, 78 ) );

  child.comment()->setDescription( QString() );

  child.setChildId( QStringLiteral( "my_id" ) );
  QCOMPARE( child.childId(), QStringLiteral( "my_id" ) );

  child.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( QStringLiteral( "a" ) ) << QgsProcessingModelChildDependency( QStringLiteral( "b" ) ) );
  QCOMPARE( child.dependencies(), QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( QStringLiteral( "a" ) ) << QgsProcessingModelChildDependency( QStringLiteral( "b" ) ) );

  QMap< QString, QgsProcessingModelChildParameterSources > sources;
  sources.insert( QStringLiteral( "a" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  child.setParameterSources( sources );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "a" ) ).at( 0 ).staticValue().toInt(), 5 );
  child.addParameterSources( QStringLiteral( "b" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 7 ) << QgsProcessingModelChildParameterSource::fromStaticValue( 9 ) );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "a" ) ).at( 0 ).staticValue().toInt(), 5 );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "b" ) ).count(), 2 );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "b" ) ).at( 0 ).staticValue().toInt(), 7 );
  QCOMPARE( child.parameterSources().value( QStringLiteral( "b" ) ).at( 1 ).staticValue().toInt(), 9 );

  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    # desc\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)" ) );
  child.comment()->setDescription( QStringLiteral( "do something useful" ) );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)" ) );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > childClone( child.clone() );
  QCOMPARE( childClone->toVariant(), child.toVariant() );
  QCOMPARE( childClone->comment()->description(), QStringLiteral( "do something useful" ) );

  QgsProcessingModelOutput testModelOut;
  testModelOut.setChildId( QStringLiteral( "my_id" ) );
  QCOMPARE( testModelOut.childId(), QStringLiteral( "my_id" ) );
  testModelOut.setChildOutputName( QStringLiteral( "my_output" ) );
  QCOMPARE( testModelOut.childOutputName(), QStringLiteral( "my_output" ) );
  testModelOut.setDefaultValue( QStringLiteral( "my_value" ) );
  QCOMPARE( testModelOut.defaultValue().toString(), QStringLiteral( "my_value" ) );
  testModelOut.setMandatory( true );
  QVERIFY( testModelOut.isMandatory() );
  testModelOut.comment()->setDescription( QStringLiteral( "my comm" ) );
  QCOMPARE( testModelOut.comment()->description(), QStringLiteral( "my comm" ) );
  std::unique_ptr< QgsProcessingModelOutput > outputClone( testModelOut.clone() );
  QCOMPARE( outputClone->toVariant(), testModelOut.toVariant() );
  QCOMPARE( outputClone->comment()->description(), QStringLiteral( "my comm" ) );
  QgsProcessingModelOutput testModelOutV;
  testModelOutV.loadVariant( testModelOut.toVariant().toMap() );
  QCOMPARE( testModelOutV.comment()->description(), QStringLiteral( "my comm" ) );

  QgsProcessingOutputLayerDefinition layerDef( QStringLiteral( "my_path" ) );
  layerDef.createOptions[QStringLiteral( "fileEncoding" )] = QStringLiteral( "my_encoding" );
  testModelOut.setDefaultValue( layerDef );
  QCOMPARE( testModelOut.defaultValue().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), QStringLiteral( "my_path" ) );
  QVariantMap map = testModelOut.toVariant().toMap();
  QCOMPARE( map[QStringLiteral( "default_value" )].toMap()["sink"].toMap()["val"].toString(), QStringLiteral( "my_path" ) );
  QCOMPARE( map["default_value"].toMap()["create_options"].toMap()[QStringLiteral( "fileEncoding" )].toString(), QStringLiteral( "my_encoding" ) );
  QgsProcessingModelOutput out;
  out.loadVariant( map );
  QVERIFY( out.defaultValue().canConvert<QgsProcessingOutputLayerDefinition>() );
  layerDef = out.defaultValue().value<QgsProcessingOutputLayerDefinition>();
  QCOMPARE( layerDef.sink.staticValue().toString(), QStringLiteral( "my_path" ) );
  QCOMPARE( layerDef.createOptions[QStringLiteral( "fileEncoding" )].toString(), QStringLiteral( "my_encoding" ) );

  QMap<QString, QgsProcessingModelOutput> outputs;
  QgsProcessingModelOutput out1;
  out1.setDescription( QStringLiteral( "my output" ) );
  outputs.insert( QStringLiteral( "a" ), out1 );
  child.setModelOutputs( outputs );
  QCOMPARE( child.modelOutputs().count(), 1 );
  QCOMPARE( child.modelOutputs().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "my output" ) );
  QCOMPARE( child.modelOutput( QStringLiteral( "a" ) ).description(), QStringLiteral( "my output" ) );
  child.modelOutput( "a" ).setDescription( QStringLiteral( "my output 2" ) );
  QCOMPARE( child.modelOutput( "a" ).description(), QStringLiteral( "my output 2" ) );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': [7,9],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n    results['my_id:a'] = outputs['my_id']['']" ) );

  // ensure friendly name is used if present
  child.addParameterSources( QStringLiteral( "b" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "a", "out" ) );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': outputs['alga']['out'],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n    results['my_id:a'] = outputs['my_id']['']" ) );
  friendlyNames.remove( QStringLiteral( "a" ) );
  QCOMPARE( child.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, extraParams, 4, 2, friendlyNames, friendlyOutputNames ).join( '\n' ), QStringLiteral( "    # desc\n    # do something useful\n    alg_params = {\n      'a': 5,\n      'b': outputs['a']['out'],\n      'SOMETHING': SOMETHING_ELSE,\n      'SOMETHING2': SOMETHING_ELSE2\n    }\n    outputs['my_id'] = processing.run('native:centroids', alg_params, context=context, feedback=feedback, is_child_algorithm=True)\n    results['my_id:a'] = outputs['my_id']['']" ) );

  // no existent
  child.modelOutput( QStringLiteral( "b" ) ).setDescription( QStringLiteral( "my output 3" ) );
  QCOMPARE( child.modelOutput( QStringLiteral( "b" ) ).description(), QStringLiteral( "my output 3" ) );
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

  QVERIFY( alg.groupBoxes().isEmpty() );
  alg.addGroupBox( groupBox );
  QCOMPARE( alg.groupBoxes().size(), 1 );
  QCOMPARE( alg.groupBoxes().at( 0 ).uuid(), groupBox.uuid() );
  QCOMPARE( alg.groupBoxes().at( 0 ).uuid(), groupBox.uuid() );
  alg.removeGroupBox( QStringLiteral( "a" ) );
  QCOMPARE( alg.groupBoxes().size(), 1 );
  alg.removeGroupBox( groupBox.uuid() );
  QVERIFY( alg.groupBoxes().isEmpty() );


  QVariantMap lastParams;
  lastParams.insert( QStringLiteral( "a" ), 2 );
  lastParams.insert( QStringLiteral( "b" ), 4 );
  alg.setDesignerParameterValues( lastParams );
  QCOMPARE( alg.designerParameterValues(), lastParams );

  // child algorithms
  QMap<QString, QgsProcessingModelChildAlgorithm> algs;
  QgsProcessingModelChildAlgorithm a1;
  a1.setDescription( QStringLiteral( "alg1" ) );
  QgsProcessingModelChildAlgorithm a2;
  a2.setDescription( QStringLiteral( "alg2" ) );
  a2.setPosition( QPointF( 112, 131 ) );
  a2.setSize( QSizeF( 44, 55 ) );
  a2.comment()->setSize( QSizeF( 111, 222 ) );
  a2.comment()->setPosition( QPointF( 113, 114 ) );
  a2.comment()->setDescription( QStringLiteral( "c" ) );
  a2.comment()->setColor( QColor( 255, 254, 253 ) );
  QgsProcessingModelOutput oo;
  oo.setPosition( QPointF( 312, 331 ) );
  oo.setSize( QSizeF( 344, 355 ) );
  oo.comment()->setSize( QSizeF( 311, 322 ) );
  oo.comment()->setPosition( QPointF( 313, 314 ) );
  oo.comment()->setDescription( QStringLiteral( "c3" ) );
  oo.comment()->setColor( QColor( 155, 14, 353 ) );
  QMap< QString, QgsProcessingModelOutput > a2Outs;
  a2Outs.insert( QStringLiteral( "out1" ), oo );
  a2.setModelOutputs( a2Outs );

  algs.insert( QStringLiteral( "a" ), a1 );
  algs.insert( QStringLiteral( "b" ), a2 );
  alg.setChildAlgorithms( algs );
  QCOMPARE( alg.childAlgorithms().count(), 2 );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "alg1" ) );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "b" ) ).description(), QStringLiteral( "alg2" ) );

  QgsProcessingModelChildAlgorithm a2other;
  a2other.setChildId( QStringLiteral( "b" ) );
  a2other.setDescription( QStringLiteral( "alg2 other" ) );
  const QgsProcessingModelOutput oo2;
  QMap< QString, QgsProcessingModelOutput > a2Outs2;
  a2Outs2.insert( QStringLiteral( "out1" ), oo2 );
  // this one didn't already exist in the algorithm
  QgsProcessingModelOutput oo3;
  oo3.comment()->setDescription( QStringLiteral( "my comment" ) );
  a2Outs2.insert( QStringLiteral( "out3" ), oo3 );

  a2other.setModelOutputs( a2Outs2 );

  a2other.copyNonDefinitionPropertiesFromModel( &alg );
  QCOMPARE( a2other.description(), QStringLiteral( "alg2 other" ) );
  QCOMPARE( a2other.position(), QPointF( 112, 131 ) );
  QCOMPARE( a2other.size(), QSizeF( 44, 55 ) );
  QCOMPARE( a2other.comment()->size(), QSizeF( 111, 222 ) );
  QCOMPARE( a2other.comment()->position(), QPointF( 113, 114 ) );
  // should not be copied
  QCOMPARE( a2other.comment()->description(), QString() );
  QVERIFY( !a2other.comment()->color().isValid() );

  QCOMPARE( a2other.modelOutput( QStringLiteral( "out1" ) ).position(), QPointF( 312, 331 ) );
  QCOMPARE( a2other.modelOutput( QStringLiteral( "out1" ) ).size(), QSizeF( 344, 355 ) );
  QCOMPARE( a2other.modelOutput( QStringLiteral( "out1" ) ).comment()->size(), QSizeF( 311, 322 ) );
  QCOMPARE( a2other.modelOutput( QStringLiteral( "out1" ) ).comment()->position(), QPointF( 313, 314 ) );
  // should be copied for outputs
  QCOMPARE( a2other.modelOutput( QStringLiteral( "out1" ) ).comment()->description(), QStringLiteral( "c3" ) );
  QCOMPARE( a2other.modelOutput( QStringLiteral( "out1" ) ).comment()->color(), QColor( 155, 14, 353 ) );
  // new outputs should not be affected
  QCOMPARE( a2other.modelOutput( QStringLiteral( "out3" ) ).comment()->description(), QStringLiteral( "my comment" ) );

  QgsProcessingModelChildAlgorithm a3;
  a3.setChildId( QStringLiteral( "c" ) );
  a3.setDescription( QStringLiteral( "alg3" ) );
  QCOMPARE( alg.addChildAlgorithm( a3 ), QStringLiteral( "c" ) );
  QCOMPARE( alg.childAlgorithms().count(), 3 );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "a" ) ).description(), QStringLiteral( "alg1" ) );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "b" ) ).description(), QStringLiteral( "alg2" ) );
  QCOMPARE( alg.childAlgorithms().value( QStringLiteral( "c" ) ).description(), QStringLiteral( "alg3" ) );
  QCOMPARE( alg.childAlgorithm( "a" ).description(), QStringLiteral( "alg1" ) );
  QCOMPARE( alg.childAlgorithm( QStringLiteral( "b" ) ).description(), QStringLiteral( "alg2" ) );
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
  pc1.comment()->setDescription( QStringLiteral( "my comment" ) );
  QCOMPARE( pc1.comment()->description(), QStringLiteral( "my comment" ) );
  std::unique_ptr< QgsProcessingModelParameter > paramClone( pc1.clone() );
  QCOMPARE( paramClone->toVariant(), pc1.toVariant() );
  QCOMPARE( paramClone->comment()->description(), QStringLiteral( "my comment" ) );
  QgsProcessingModelParameter pcc1;
  pcc1.loadVariant( pc1.toVariant().toMap() );
  QCOMPARE( pcc1.comment()->description(), QStringLiteral( "my comment" ) );
  pComponents.insert( QStringLiteral( "my_param" ), pc1 );
  alg.setParameterComponents( pComponents );
  QCOMPARE( alg.parameterComponents().count(), 1 );
  QCOMPARE( alg.parameterComponents().value( QStringLiteral( "my_param" ) ).parameterName(), QStringLiteral( "my_param" ) );
  QCOMPARE( alg.parameterComponent( "my_param" ).parameterName(), QStringLiteral( "my_param" ) );
  alg.parameterComponent( "my_param" ).setDescription( QStringLiteral( "my param 2" ) );
  QCOMPARE( alg.parameterComponent( "my_param" ).description(), QStringLiteral( "my param 2" ) );
  // no existent
  alg.parameterComponent( QStringLiteral( "b" ) ).setDescription( QStringLiteral( "my param 3" ) );
  QCOMPARE( alg.parameterComponent( QStringLiteral( "b" ) ).description(), QStringLiteral( "my param 3" ) );
  QCOMPARE( alg.parameterComponent( QStringLiteral( "b" ) ).parameterName(), QStringLiteral( "b" ) );
  QCOMPARE( alg.parameterComponents().count(), 2 );

  // parameter definitions
  QgsProcessingModelAlgorithm alg1a( "test", "testGroup" );
  QgsProcessingModelParameter bool1;
  bool1.setPosition( QPointF( 1, 2 ) );
  bool1.setSize( QSizeF( 11, 12 ) );
  alg1a.addModelParameter( new QgsProcessingParameterBoolean( QStringLiteral( "p1" ), "desc" ), bool1 );
  QCOMPARE( alg1a.parameterDefinitions().count(), 1 );
  QCOMPARE( alg1a.parameterDefinition( QStringLiteral( "p1" ) )->type(), QStringLiteral( "boolean" ) );
  QCOMPARE( alg1a.parameterComponent( QStringLiteral( "p1" ) ).position().x(), 1.0 );
  QCOMPARE( alg1a.parameterComponent( QStringLiteral( "p1" ) ).position().y(), 2.0 );
  QCOMPARE( alg1a.parameterComponent( QStringLiteral( "p1" ) ).size().width(), 11.0 );
  QCOMPARE( alg1a.parameterComponent( QStringLiteral( "p1" ) ).size().height(), 12.0 );
  alg1a.updateModelParameter( new QgsProcessingParameterBoolean( QStringLiteral( "p1" ), "descx" ) );
  QCOMPARE( alg1a.parameterDefinition( QStringLiteral( "p1" ) )->description(), QStringLiteral( "descx" ) );
  alg1a.removeModelParameter( "bad" );
  QCOMPARE( alg1a.parameterDefinitions().count(), 1 );
  alg1a.removeModelParameter( QStringLiteral( "p1" ) );
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
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "notvalid" ) ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "notvalid" ) ).isEmpty() );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "notvalid" ) ).isEmpty() );

  // add a child
  QgsProcessingModelChildAlgorithm c7;
  c7.setChildId( QStringLiteral( "c7" ) );
  alg3.addChildAlgorithm( c7 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c7" ) ).isEmpty() );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c7" ) ).isEmpty() );

  // direct dependency
  QgsProcessingModelChildAlgorithm c8;
  c8.setChildId( QStringLiteral( "c8" ) );
  c8.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( QStringLiteral( "c7" ) ) );
  alg3.addChildAlgorithm( c8 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c8" ) ).isEmpty() );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c7" ) ).isEmpty() );
  QCOMPARE( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).count(), 1 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).contains( QStringLiteral( "c8" ) ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( QStringLiteral( "c8" ) ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c8" ) ).contains( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c7" ) ).isEmpty() );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c8" ) ).size(), 1 );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c8" ) ).at( 0 ).childId, QStringLiteral( "c7" ) );

  // dependency via parameter source
  QgsProcessingModelChildAlgorithm c9;
  c9.setChildId( QStringLiteral( "c9" ) );
  c9.addParameterSources( QStringLiteral( "x" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "c8" ), QStringLiteral( "x" ) ) );
  alg3.addChildAlgorithm( c9 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c9" ) ).isEmpty() );
  QCOMPARE( alg3.dependentChildAlgorithms( QStringLiteral( "c8" ) ).count(), 1 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c8" ) ).contains( QStringLiteral( "c9" ) ) );
  QCOMPARE( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).count(), 2 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).contains( QStringLiteral( "c8" ) ) );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).contains( QStringLiteral( "c9" ) ) );

  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c7" ) ).isEmpty() );
  QCOMPARE( alg3.dependsOnChildAlgorithms( QStringLiteral( "c8" ) ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c8" ) ).contains( QStringLiteral( "c7" ) ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( QStringLiteral( "c9" ) ).count(), 2 );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c9" ) ).contains( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c9" ) ).contains( QStringLiteral( "c8" ) ) );

  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c7" ) ).isEmpty() );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c8" ) ).size(), 1 );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c8" ) ).at( 0 ).childId, QStringLiteral( "c7" ) );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9" ) ).size(), 2 );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c7" ) ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c8" ) ) ) );

  QgsProcessingModelChildAlgorithm c9b;
  c9b.setChildId( QStringLiteral( "c9b" ) );
  c9b.addParameterSources( QStringLiteral( "x" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "c9" ), QStringLiteral( "x" ) ) );
  alg3.addChildAlgorithm( c9b );

  QCOMPARE( alg3.dependentChildAlgorithms( QStringLiteral( "c9" ) ).count(), 1 );
  QCOMPARE( alg3.dependentChildAlgorithms( QStringLiteral( "c8" ) ).count(), 2 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c8" ) ).contains( QStringLiteral( "c9" ) ) );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c8" ) ).contains( "c9b" ) );
  QCOMPARE( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).count(), 3 );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).contains( QStringLiteral( "c8" ) ) );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).contains( QStringLiteral( "c9" ) ) );
  QVERIFY( alg3.dependentChildAlgorithms( QStringLiteral( "c7" ) ).contains( "c9b" ) );

  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c7" ) ).isEmpty() );
  QCOMPARE( alg3.dependsOnChildAlgorithms( QStringLiteral( "c8" ) ).count(), 1 );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c8" ) ).contains( QStringLiteral( "c7" ) ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( QStringLiteral( "c9" ) ).count(), 2 );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c9" ) ).contains( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( QStringLiteral( "c9" ) ).contains( QStringLiteral( "c8" ) ) );
  QCOMPARE( alg3.dependsOnChildAlgorithms( "c9b" ).count(), 3 );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( QStringLiteral( "c8" ) ) );
  QVERIFY( alg3.dependsOnChildAlgorithms( "c9b" ).contains( QStringLiteral( "c9" ) ) );

  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c7" ) ).isEmpty() );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c8" ) ).size(), 1 );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c8" ) ).at( 0 ).childId, QStringLiteral( "c7" ) );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9" ) ).size(), 2 );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c7" ) ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c8" ) ) ) );
  QCOMPARE( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9b" ) ).size(), 3 );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9b" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c7" ) ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9b" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c8" ) ) ) );
  QVERIFY( alg3.availableDependenciesForChildAlgorithm( QStringLiteral( "c9b" ) ).contains( QgsProcessingModelChildDependency( QStringLiteral( "c9" ) ) ) );

  alg3.removeChildAlgorithm( "c9b" );


  // (de)activate child algorithm
  alg3.deactivateChildAlgorithm( QStringLiteral( "c9" ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( QStringLiteral( "c9" ) ) );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  alg3.deactivateChildAlgorithm( QStringLiteral( "c8" ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( QStringLiteral( "c9" ) ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( QStringLiteral( "c8" ) ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( QStringLiteral( "c9" ) ) );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  alg3.deactivateChildAlgorithm( QStringLiteral( "c7" ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c7" ) ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( QStringLiteral( "c9" ) ) );
  QVERIFY( !alg3.activateChildAlgorithm( QStringLiteral( "c8" ) ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c7" ) ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( QStringLiteral( "c8" ) ) );
  QVERIFY( alg3.activateChildAlgorithm( QStringLiteral( "c7" ) ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c7" ) ).isActive() );
  QVERIFY( !alg3.activateChildAlgorithm( QStringLiteral( "c9" ) ) );
  QVERIFY( alg3.activateChildAlgorithm( QStringLiteral( "c8" ) ) );
  QVERIFY( !alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c7" ) ).isActive() );
  QVERIFY( alg3.activateChildAlgorithm( QStringLiteral( "c9" ) ) );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c9" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c8" ) ).isActive() );
  QVERIFY( alg3.childAlgorithm( QStringLiteral( "c7" ) ).isActive() );



  //remove child algorithm
  QVERIFY( !alg3.removeChildAlgorithm( QStringLiteral( "c7" ) ) );
  QVERIFY( !alg3.removeChildAlgorithm( QStringLiteral( "c8" ) ) );
  QVERIFY( alg3.removeChildAlgorithm( QStringLiteral( "c9" ) ) );
  QCOMPARE( alg3.childAlgorithms().count(), 2 );
  QVERIFY( alg3.childAlgorithms().contains( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.childAlgorithms().contains( QStringLiteral( "c8" ) ) );
  QVERIFY( !alg3.removeChildAlgorithm( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.removeChildAlgorithm( QStringLiteral( "c8" ) ) );
  QCOMPARE( alg3.childAlgorithms().count(), 1 );
  QVERIFY( alg3.childAlgorithms().contains( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.removeChildAlgorithm( QStringLiteral( "c7" ) ) );
  QVERIFY( alg3.childAlgorithms().isEmpty() );

  // parameter dependencies
  QgsProcessingModelAlgorithm alg4( "test", "testGroup" );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "not a param" ) );
  QgsProcessingModelChildAlgorithm c10;
  c10.setChildId( "c10" );
  alg4.addChildAlgorithm( c10 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( "not a param" ) );
  const QgsProcessingModelParameter bool2;
  alg4.addModelParameter( new QgsProcessingParameterBoolean( QStringLiteral( "p1" ), "desc" ), bool2 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( QStringLiteral( "p1" ) ) );
  c10.addParameterSources( QStringLiteral( "x" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "p2" ) );
  alg4.setChildAlgorithm( c10 );
  QVERIFY( !alg4.childAlgorithmsDependOnParameter( QStringLiteral( "p1" ) ) );
  c10.addParameterSources( QStringLiteral( "y" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) );
  alg4.setChildAlgorithm( c10 );
  QVERIFY( alg4.childAlgorithmsDependOnParameter( QStringLiteral( "p1" ) ) );

  const QgsProcessingModelParameter vlP;
  alg4.addModelParameter( new QgsProcessingParameterVectorLayer( "layer" ), vlP );
  const QgsProcessingModelParameter field;
  alg4.addModelParameter( new QgsProcessingParameterField( QStringLiteral( "field" ), QString(), QVariant(), QStringLiteral( "layer" ) ), field );
  QVERIFY( !alg4.otherParametersDependOnParameter( QStringLiteral( "p1" ) ) );
  QVERIFY( !alg4.otherParametersDependOnParameter( QStringLiteral( "field" ) ) );
  QVERIFY( alg4.otherParametersDependOnParameter( "layer" ) );





  // to/from XML
  QgsProcessingModelAlgorithm alg5( QStringLiteral( "test" ), QStringLiteral( "testGroup" ) );
  alg5.helpContent().insert( QStringLiteral( "author" ), QStringLiteral( "me" ) );
  alg5.helpContent().insert( QStringLiteral( "usage" ), QStringLiteral( "run" ) );
  alg5.addGroupBox( groupBox );
  QVariantMap variables;
  variables.insert( QStringLiteral( "v1" ), 5 );
  variables.insert( QStringLiteral( "v2" ), QStringLiteral( "aabbccd" ) );
  alg5.setVariables( variables );
  QCOMPARE( alg5.variables(), variables );
  QgsProcessingModelChildAlgorithm alg5c1;
  alg5c1.setChildId( QStringLiteral( "cx1" ) );
  alg5c1.setAlgorithmId( QStringLiteral( "buffer" ) );
  alg5c1.setConfiguration( myConfig );
  alg5c1.addParameterSources( QStringLiteral( "x" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "p1" ) ) );
  alg5c1.addParameterSources( QStringLiteral( "y" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( "cx2", "out3" ) );
  alg5c1.addParameterSources( QStringLiteral( "z" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  alg5c1.addParameterSources( "a", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( "2*2" ) );
  alg5c1.addParameterSources( "zm", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( 6 )
                              << QgsProcessingModelChildParameterSource::fromModelParameter( "p2" )
                              << QgsProcessingModelChildParameterSource::fromChildOutput( "cx2", "out4" )
                              << QgsProcessingModelChildParameterSource::fromExpression( "1+2" )
                              << QgsProcessingModelChildParameterSource::fromStaticValue( QgsProperty::fromExpression( "1+8" ) ) );
  alg5c1.setActive( true );
  alg5c1.setLinksCollapsed( Qt::BottomEdge, true );
  alg5c1.setLinksCollapsed( Qt::TopEdge, true );
  alg5c1.setDescription( "child 1" );
  alg5c1.setPosition( QPointF( 1, 2 ) );
  alg5c1.setSize( QSizeF( 11, 21 ) );
  QMap<QString, QgsProcessingModelOutput> alg5c1outputs;
  QgsProcessingModelOutput alg5c1out1;
  alg5c1out1.setDescription( QStringLiteral( "my output" ) );
  alg5c1out1.setPosition( QPointF( 3, 4 ) );
  alg5c1out1.setSize( QSizeF( 31, 41 ) );
  alg5c1outputs.insert( QStringLiteral( "a" ), alg5c1out1 );
  alg5c1.setModelOutputs( alg5c1outputs );
  alg5.addChildAlgorithm( alg5c1 );

  QgsProcessingModelChildAlgorithm alg5c2;
  alg5c2.setChildId( "cx2" );
  alg5c2.setAlgorithmId( QStringLiteral( "native:centroids" ) );
  alg5c2.setActive( false );
  alg5c2.setLinksCollapsed( Qt::BottomEdge, false );
  alg5c2.setLinksCollapsed( Qt::TopEdge, false );
  alg5c2.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( "a" ) << QgsProcessingModelChildDependency( QStringLiteral( "b" ) ) );
  alg5.addChildAlgorithm( alg5c2 );

  QgsProcessingModelParameter alg5pc1;
  alg5pc1.setParameterName( QStringLiteral( "my_param" ) );
  alg5pc1.setPosition( QPointF( 11, 12 ) );
  alg5pc1.setSize( QSizeF( 21, 22 ) );
  alg5.addModelParameter( new QgsProcessingParameterBoolean( QStringLiteral( "my_param" ) ), alg5pc1 );
  alg5.setDesignerParameterValues( lastParams );

  QDomDocument doc = QDomDocument( "model" );
  alg5.initAlgorithm();
  const QVariant v = alg5.toVariant();
  // make sure private parameters weren't included in the definition
  QVERIFY( !v.toMap().value( QStringLiteral( "parameterDefinitions" ) ).toMap().contains( QStringLiteral( "VERBOSE_LOG" ) ) );

  const QDomElement elem = QgsXmlUtils::writeVariant( v, doc );
  doc.appendChild( elem );

  QgsProcessingModelAlgorithm alg6;
  QVERIFY( alg6.loadVariant( QgsXmlUtils::readVariant( doc.firstChildElement() ) ) );
  QCOMPARE( alg6.name(), QStringLiteral( "test" ) );
  QCOMPARE( alg6.group(), QStringLiteral( "testGroup" ) );
  QCOMPARE( alg6.helpContent(), alg5.helpContent() );
  QCOMPARE( alg6.variables(), variables );
  QCOMPARE( alg6.designerParameterValues(), lastParams );

  QCOMPARE( alg6.groupBoxes().size(), 1 );
  QCOMPARE( alg6.groupBoxes().at( 0 ).size(), QSizeF( 9, 8 ) );
  QCOMPARE( alg6.groupBoxes().at( 0 ).position(), QPointF( 11, 14 ) );
  QCOMPARE( alg6.groupBoxes().at( 0 ).description(), QStringLiteral( "a comment" ) );
  QCOMPARE( alg6.groupBoxes().at( 0 ).color(), QColor( 123, 45, 67 ) );

  QgsProcessingModelChildAlgorithm alg6c1 = alg6.childAlgorithm( QStringLiteral( "cx1" ) );
  QCOMPARE( alg6c1.childId(), QStringLiteral( "cx1" ) );
  QCOMPARE( alg6c1.algorithmId(), QStringLiteral( "buffer" ) );
  QCOMPARE( alg6c1.configuration(), myConfig );
  QVERIFY( alg6c1.isActive() );
  QVERIFY( alg6c1.linksCollapsed( Qt::BottomEdge ) );
  QVERIFY( alg6c1.linksCollapsed( Qt::TopEdge ) );
  QCOMPARE( alg6c1.description(), QStringLiteral( "child 1" ) );
  QCOMPARE( alg6c1.position().x(), 1.0 );
  QCOMPARE( alg6c1.position().y(), 2.0 );
  QCOMPARE( alg6c1.size().width(), 11.0 );
  QCOMPARE( alg6c1.size().height(), 21.0 );
  QCOMPARE( alg6c1.parameterSources().count(), 5 );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "x" ) ).at( 0 ).source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "x" ) ).at( 0 ).parameterName(), QStringLiteral( "p1" ) );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "y" ) ).at( 0 ).source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "y" ) ).at( 0 ).outputChildId(), QStringLiteral( "cx2" ) );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "y" ) ).at( 0 ).outputName(), QStringLiteral( "out3" ) );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "z" ) ).at( 0 ).source(), QgsProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( alg6c1.parameterSources().value( QStringLiteral( "z" ) ).at( 0 ).staticValue().toInt(), 5 );
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
  QCOMPARE( alg6c1.modelOutput( "a" ).size().width(), 31.0 );
  QCOMPARE( alg6c1.modelOutput( "a" ).size().height(), 41.0 );

  const QgsProcessingModelChildAlgorithm alg6c2 = alg6.childAlgorithm( "cx2" );
  QCOMPARE( alg6c2.childId(), QStringLiteral( "cx2" ) );
  QVERIFY( !alg6c2.isActive() );
  QVERIFY( !alg6c2.linksCollapsed( Qt::BottomEdge ) );
  QVERIFY( !alg6c2.linksCollapsed( Qt::TopEdge ) );
  QCOMPARE( alg6c2.dependencies(), QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( "a" ) << QgsProcessingModelChildDependency( QStringLiteral( "b" ) ) );

  QCOMPARE( alg6.parameterComponents().count(), 1 );
  QCOMPARE( alg6.parameterComponents().value( QStringLiteral( "my_param" ) ).parameterName(), QStringLiteral( "my_param" ) );
  QCOMPARE( alg6.parameterComponent( "my_param" ).parameterName(), QStringLiteral( "my_param" ) );
  QCOMPARE( alg6.parameterComponent( "my_param" ).position().x(), 11.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).position().y(), 12.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).size().width(), 21.0 );
  QCOMPARE( alg6.parameterComponent( "my_param" ).size().height(), 22.0 );
  QCOMPARE( alg6.parameterDefinitions().count(), 1 );
  QCOMPARE( alg6.parameterDefinitions().at( 0 )->type(), QStringLiteral( "boolean" ) );

  // destination parameters
  QgsProcessingModelAlgorithm alg7( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm alg7c1;
  alg7c1.setChildId( QStringLiteral( "cx1" ) );
  alg7c1.setAlgorithmId( "native:centroids" );
  QMap<QString, QgsProcessingModelOutput> alg7c1outputs;
  QgsProcessingModelOutput alg7c1out1( QStringLiteral( "my_output" ) );
  alg7c1out1.setChildId( QStringLiteral( "cx1" ) );
  alg7c1out1.setChildOutputName( "OUTPUT" );
  alg7c1out1.setDescription( QStringLiteral( "my output" ) );
  alg7c1outputs.insert( QStringLiteral( "my_output" ), alg7c1out1 );
  alg7c1.setModelOutputs( alg7c1outputs );
  alg7.addChildAlgorithm( alg7c1 );
  // verify that model has destination parameter created
  QCOMPARE( alg7.destinationParameterDefinitions().count(), 1 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "my_output" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg7.destinationParameterDefinitions().at( 0 ) )->originalProvider()->id(), QStringLiteral( "native" ) );
  QCOMPARE( alg7.outputDefinitions().count(), 1 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), QStringLiteral( "my_output" ) );
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
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "my_output" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QVERIFY( alg7.destinationParameterDefinitions().at( 0 )->defaultValue().isNull() );
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 0 )->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->name(), QStringLiteral( "my_output2" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->description(), QStringLiteral( "my output2" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 1 )->defaultValue().toString(), QStringLiteral( "my value" ) );
  QVERIFY( !( alg7.destinationParameterDefinitions().at( 1 )->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  QCOMPARE( alg7.outputDefinitions().count(), 2 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), QStringLiteral( "my_output" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->type(), QStringLiteral( "outputVector" ) );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->name(), QStringLiteral( "my_output2" ) );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->type(), QStringLiteral( "outputVector" ) );
  QCOMPARE( alg7.outputDefinitions().at( 1 )->description(), QStringLiteral( "my output2" ) );

  alg7.removeChildAlgorithm( QStringLiteral( "cx1" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().count(), 1 );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "my_output2" ) );
  QCOMPARE( alg7.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output2" ) );
  QCOMPARE( alg7.outputDefinitions().count(), 1 );
  QCOMPARE( alg7.outputDefinitions().at( 0 )->name(), QStringLiteral( "my_output2" ) );
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

void TestQgsProcessingModelAlgorithm::modelExecution()
{
  // test childOutputIsRequired
  QgsProcessingModelAlgorithm model1;
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( QStringLiteral( "cx1" ) );
  algc1.setAlgorithmId( "native:centroids" );
  model1.addChildAlgorithm( algc1 );
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( "cx2" );
  algc2.setAlgorithmId( "native:centroids" );
  algc2.addParameterSources( QStringLiteral( "x" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "cx1" ), QStringLiteral( "p1" ) ) );
  model1.addChildAlgorithm( algc2 );
  QgsProcessingModelChildAlgorithm algc3;
  algc3.setChildId( "cx3" );
  algc3.setAlgorithmId( "native:centroids" );
  algc3.addParameterSources( QStringLiteral( "x" ), QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "cx1" ), "p2" ) );
  algc3.setActive( false );
  model1.addChildAlgorithm( algc3 );

  QVERIFY( model1.childOutputIsRequired( QStringLiteral( "cx1" ), QStringLiteral( "p1" ) ) ); // cx2 depends on p1
  QVERIFY( !model1.childOutputIsRequired( QStringLiteral( "cx1" ), "p2" ) ); // cx3 depends on p2, but cx3 is not active
  QVERIFY( !model1.childOutputIsRequired( QStringLiteral( "cx1" ), "p3" ) ); // nothing requires p3
  QVERIFY( !model1.childOutputIsRequired( "cx2", QStringLiteral( "p1" ) ) );
  QVERIFY( !model1.childOutputIsRequired( "cx3", QStringLiteral( "p1" ) ) );

  // test parametersForChildAlgorithm
  QgsProcessingModelAlgorithm model2;
  QgsProcessingModelParameter sourceParam( "SOURCE_LAYER" );
  sourceParam.comment()->setDescription( QStringLiteral( "an input" ) );
  model2.addModelParameter( new QgsProcessingParameterFeatureSource( "SOURCE_LAYER" ), sourceParam );
  model2.addModelParameter( new QgsProcessingParameterNumber( "DIST", QString(), QgsProcessingParameterNumber::Double ), QgsProcessingModelParameter( "DIST" ) );
  QgsProcessingParameterCrs *p = new QgsProcessingParameterCrs( "CRS", QString(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28355" ) ) );
  p->setFlags( p->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  model2.addModelParameter( p, QgsProcessingModelParameter( "CRS" ) );
  QgsProcessingModelChildAlgorithm alg2c1;
  QgsExpressionContext expContext;
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->setVariable( "myvar", 8 );
  expContext.appendScope( scope );
  alg2c1.setChildId( QStringLiteral( "cx1" ) );
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
  QVariantMap params = model2.parametersForChildAlgorithm( model2.childAlgorithm( QStringLiteral( "cx1" ) ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
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
  QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = model2.variablesForChildAlgorithm( QStringLiteral( "cx1" ), context );
  QCOMPARE( variables.count(), 7 );
  QCOMPARE( variables.value( "DIST" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "CRS" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );

  // with values
  variables = model2.variablesForChildAlgorithm( QStringLiteral( "cx1" ), context, modelInputs, childResults );
  QCOMPARE( variables.count(), 7 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  std::unique_ptr< QgsExpressionContextScope > childScope( model2.createExpressionContextScopeForChildAlgorithm( QStringLiteral( "cx1" ), context, modelInputs, childResults ) );
  QCOMPARE( childScope->name(), QStringLiteral( "algorithm_inputs" ) );
  QCOMPARE( childScope->variableCount(), 7 );
  QCOMPARE( childScope->variable( "DIST" ).toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_minx" ).toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_miny" ).toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_maxx" ).toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( childScope->variable( "SOURCE_LAYER_maxy" ).toDouble(), 46.8719, 0.001 );


  QVariantMap results;
  results.insert( "OUTPUT", QStringLiteral( "dest.shp" ) );
  childResults.insert( QStringLiteral( "cx1" ), results );

  // a child who uses an output from another alg as a parameter value
  QgsProcessingModelChildAlgorithm alg2c2;
  alg2c2.setChildId( "cx2" );
  alg2c2.setAlgorithmId( "native:centroids" );
  alg2c2.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "cx1" ), "OUTPUT" ) );
  model2.addChildAlgorithm( alg2c2 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx2" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "INPUT" ).toString(), QStringLiteral( "dest.shp" ) );
  QCOMPARE( params.value( "OUTPUT" ).toString(), QStringLiteral( "memory:Centroids" ) );
  QCOMPARE( params.count(), 2 );

  variables = model2.variablesForChildAlgorithm( "cx2", context );
  QCOMPARE( variables.count(), 12 );
  QCOMPARE( variables.value( "DIST" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_minx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_miny" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "SOURCE_LAYER_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_minx" ).source.outputChildId(),  QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_miny" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxx" ).source.outputChildId(), QStringLiteral( "cx1" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.source(), QgsProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( variables.value( "cx1_OUTPUT_maxy" ).source.outputChildId(), QStringLiteral( "cx1" ) );

  // with values
  variables = model2.variablesForChildAlgorithm( "cx2", context, modelInputs, childResults );
  QCOMPARE( variables.count(), 12 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QString( QStringLiteral( "cx1" ) ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.parameterName(), QString( "" ) );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_minx" ).value.toDouble(), -118.8888, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_miny" ).value.toDouble(), 22.8002, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxx" ).value.toDouble(), -83.3333, 0.001 );
  QGSCOMPARENEAR( variables.value( "SOURCE_LAYER_maxy" ).value.toDouble(), 46.8719, 0.001 );

  // a child with an optional output
  QgsProcessingModelChildAlgorithm alg2c3;
  alg2c3.setChildId( "cx3" );
  alg2c3.setAlgorithmId( "native:extractbyexpression" );
  alg2c3.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "cx1" ), "OUTPUT" ) );
  alg2c3.addParameterSources( "EXPRESSION", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "true" ) );
  alg2c3.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "MY_OUT" ) );
  alg2c3.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( "cx2" ) );
  QMap<QString, QgsProcessingModelOutput> outputs3;
  QgsProcessingModelOutput out2( "MY_OUT" );
  out2.setChildOutputName( "OUTPUT" );
  out2.setDescription( QStringLiteral( "My output" ) );
  outputs3.insert( QStringLiteral( "MY_OUT" ), out2 );
  alg2c3.setModelOutputs( outputs3 );

  model2.addChildAlgorithm( alg2c3 );
  QVERIFY( model2.parameterDefinition( QStringLiteral( "My_output" ) ) );

  // using older version compatibility
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "INPUT" ).toString(), QStringLiteral( "dest.shp" ) );
  QCOMPARE( params.value( "EXPRESSION" ).toString(), QStringLiteral( "true" ) );
  QVERIFY( params.value( "OUTPUT" ).canConvert<QgsProcessingOutputLayerDefinition>() );
  const QgsProcessingOutputLayerDefinition outDef = qvariant_cast<QgsProcessingOutputLayerDefinition>( params.value( "OUTPUT" ) );
  QCOMPARE( outDef.destinationName, QStringLiteral( "MY_OUT" ) );
  QCOMPARE( outDef.sink.staticValue().toString(), QStringLiteral( "memory:" ) );
  QCOMPARE( params.count(), 3 ); // don't want FAIL_OUTPUT set!

  // using newer version naming
  modelInputs.remove( QStringLiteral( "cx3:MY_OUT" ) );
  modelInputs.insert( "my_output", QVariant::fromValue( layerDef ) );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "INPUT" ).toString(), QStringLiteral( "dest.shp" ) );
  QCOMPARE( params.value( "EXPRESSION" ).toString(), QStringLiteral( "true" ) );
  QVERIFY( params.value( "OUTPUT" ).canConvert<QgsProcessingOutputLayerDefinition>() );
  const QgsProcessingOutputLayerDefinition outDef2 = qvariant_cast<QgsProcessingOutputLayerDefinition>( params.value( "OUTPUT" ) );
  QCOMPARE( outDef2.destinationName, QStringLiteral( "MY_OUT" ) );
  QCOMPARE( outDef2.sink.staticValue().toString(), QStringLiteral( "memory:" ) );
  QCOMPARE( params.count(), 3 ); // don't want FAIL_OUTPUT set!

  // a child with an expression which is invalid
  QgsProcessingModelChildAlgorithm alg2c3a;
  alg2c3a.setChildId( "cx3a" );
  alg2c3a.setAlgorithmId( "native:extractbyexpression" );
  alg2c3a.setDescription( "Extract by expression" );
  alg2c3a.addParameterSources( "INPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "1/'a'" ) ) );
  alg2c3a.addParameterSources( "EXPRESSION", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "true" ) );
  alg2c3a.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromModelParameter( "MY_OUT" ) );

  model2.addChildAlgorithm( alg2c3a );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx3a" ), modelInputs, childResults, expContext, error );
  QCOMPARE( error, QStringLiteral( "Could not evaluate expression for parameter INPUT for Extract by expression: Cannot convert 'a' to double" ) );
  model2.removeChildAlgorithm( "cx3a" );

  // a child with an static output value
  QgsProcessingModelChildAlgorithm alg2c4;
  alg2c4.setChildId( "cx4" );
  alg2c4.setAlgorithmId( "native:extractbyexpression" );
  alg2c4.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromStaticValue( "STATIC" ) );
  model2.addChildAlgorithm( alg2c4 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx4" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "OUTPUT" ).toString(), QStringLiteral( "STATIC" ) );
  model2.removeChildAlgorithm( "cx4" );
  // expression based output value
  alg2c4.addParameterSources( "OUTPUT", QgsProcessingModelChildParameterSources() << QgsProcessingModelChildParameterSource::fromExpression( "'A' || 'B'" ) );
  model2.addChildAlgorithm( alg2c4 );
  params = model2.parametersForChildAlgorithm( model2.childAlgorithm( "cx4" ), modelInputs, childResults, expContext, error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( params.value( "OUTPUT" ).toString(), QStringLiteral( "AB" ) );
  model2.removeChildAlgorithm( "cx4" );

  variables = model2.variablesForChildAlgorithm( "cx3", context );
  QCOMPARE( variables.count(), 17 );
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
  QCOMPARE( variables.count(), 17 );
  QCOMPARE( variables.value( "DIST" ).value.toInt(), 271 );
  QCOMPARE( variables.value( "SOURCE_LAYER" ).source.parameterName(), QString( "SOURCE_LAYER" ) );
  QCOMPARE( variables.value( "cx1_OUTPUT" ).source.outputChildId(), QString( QStringLiteral( "cx1" ) ) );
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
  QgsProcessingModelChildAlgorithm &cx1 = model2.childAlgorithm( QStringLiteral( "cx1" ) );
  const QString oldDescription = cx1.description();
  cx1.setDescription( "cx '():.1" );
  variables = model2.variablesForChildAlgorithm( "cx3", context );
  QVERIFY( !variables.contains( "cx1_OUTPUT" ) );
  QVERIFY( !variables.contains( "cx '():.1_OUTPUT" ) );
  QVERIFY( variables.contains( "cx______1_OUTPUT" ) );
  cx1.setDescription( oldDescription ); // set descrin back to avoid fail of following tests

  // test model to python conversion
  model2.setName( QStringLiteral( "2my model" ) );
  model2.childAlgorithm( QStringLiteral( "cx1" ) ).modelOutput( QStringLiteral( "MODEL_OUT_LAYER" ) ).setDescription( "my model output" );
  model2.updateDestinationParameters();
  model2.childAlgorithm( QStringLiteral( "cx1" ) ).setDescription( "first step in my model" );
  const QStringList actualParts = model2.asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, 2 );
  QgsDebugMsg( actualParts.join( '\n' ) );
  const QStringList expectedParts = QStringLiteral( "\"\"\"\n"
                                    "Model exported as python.\n"
                                    "Name : 2my model\n"
                                    "Group : \n"
                                    "With QGIS : %1\n"
                                    "\"\"\"\n\n"
                                    "from qgis.core import QgsProcessing\n"
                                    "from qgis.core import QgsProcessingAlgorithm\n"
                                    "from qgis.core import QgsProcessingMultiStepFeedback\n"
                                    "from qgis.core import QgsProcessingParameterFeatureSource\n"
                                    "from qgis.core import QgsProcessingParameterNumber\n"
                                    "from qgis.core import QgsProcessingParameterCrs\n"
                                    "from qgis.core import QgsProcessingParameterFeatureSink\n"
                                    "from qgis.core import QgsProcessingParameterDefinition\n"
                                    "from qgis.core import QgsCoordinateReferenceSystem\n"
                                    "from qgis.core import QgsExpression\n"
                                    "import processing\n"
                                    "\n"
                                    "\n"
                                    "class MyModel(QgsProcessingAlgorithm):\n"
                                    "\n"
                                    "  def initAlgorithm(self, config=None):\n"
                                    "    # an input\n"
                                    "    self.addParameter(QgsProcessingParameterFeatureSource('SOURCE_LAYER', '', defaultValue=None))\n"
                                    "    self.addParameter(QgsProcessingParameterNumber('DIST', '', type=QgsProcessingParameterNumber.Double, defaultValue=None))\n"
                                    "    param = QgsProcessingParameterCrs('CRS', '', defaultValue=QgsCoordinateReferenceSystem('EPSG:28355'))\n"
                                    "    param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)\n"
                                    "    self.addParameter(param)\n"
                                    "    self.addParameter(QgsProcessingParameterFeatureSink('MyModelOutput', 'my model output', type=QgsProcessing.TypeVectorPolygon, createByDefault=True, supportsAppend=True, defaultValue=None))\n"
                                    "    self.addParameter(QgsProcessingParameterFeatureSink('MyOutput', 'My output', type=QgsProcessing.TypeVectorAnyGeometry, createByDefault=True, defaultValue=None))\n"
                                    "\n"
                                    "  def processAlgorithm(self, parameters, context, model_feedback):\n"
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
                                    "  def name(self):\n"
                                    "    return '2my model'\n"
                                    "\n"
                                    "  def displayName(self):\n"
                                    "    return '2my model'\n"
                                    "\n"
                                    "  def group(self):\n"
                                    "    return ''\n"
                                    "\n"
                                    "  def groupId(self):\n"
                                    "    return ''\n"
                                    "\n"
                                    "  def createInstance(self):\n"
                                    "    return MyModel()\n" ).arg( Qgis::versionInt() ).split( '\n' );
  QCOMPARE( actualParts, expectedParts );
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
  param.setParameterName( QStringLiteral( "LAYER" ) );
  model1.addModelParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "LAYER" ) ), param );
  algc1.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "LAYER" ) ) );
  model1.addChildAlgorithm( algc1 );

  //then create some branches which come off this, depending on the layer type
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( QStringLiteral( "buffer" ) );
  algc2.setAlgorithmId( "native:buffer" );
  algc2.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "filter" ), QStringLiteral( "VECTOR" ) ) );
  QMap<QString, QgsProcessingModelOutput> outputsc2;
  QgsProcessingModelOutput outc2( "BUFFER_OUTPUT" );
  outc2.setChildOutputName( "OUTPUT" );
  outputsc2.insert( QStringLiteral( "BUFFER_OUTPUT" ), outc2 );
  algc2.setModelOutputs( outputsc2 );
  model1.addChildAlgorithm( algc2 );
  // ...we want a complex branch, so add some more bits to the branch
  QgsProcessingModelChildAlgorithm algc3;
  algc3.setChildId( "buffer2" );
  algc3.setAlgorithmId( "native:buffer" );
  algc3.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "buffer" ), QStringLiteral( "OUTPUT" ) ) );
  QMap<QString, QgsProcessingModelOutput> outputsc3;
  QgsProcessingModelOutput outc3( "BUFFER2_OUTPUT" );
  outc3.setChildOutputName( "OUTPUT" );
  outputsc3.insert( QStringLiteral( "BUFFER2_OUTPUT" ), outc3 );
  algc3.setModelOutputs( outputsc3 );
  model1.addChildAlgorithm( algc3 );
  QgsProcessingModelChildAlgorithm algc4;
  algc4.setChildId( "buffer3" );
  algc4.setAlgorithmId( "native:buffer" );
  algc4.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "buffer" ), QStringLiteral( "OUTPUT" ) ) );
  QMap<QString, QgsProcessingModelOutput> outputsc4;
  QgsProcessingModelOutput outc4( "BUFFER3_OUTPUT" );
  outc4.setChildOutputName( "OUTPUT" );
  outputsc4.insert( QStringLiteral( "BUFFER3_OUTPUT" ), outc4 );
  algc4.setModelOutputs( outputsc4 );
  model1.addChildAlgorithm( algc4 );

  // now add some bits to the raster branch
  QgsProcessingModelChildAlgorithm algr2;
  algr2.setChildId( "fill2" );
  algr2.setAlgorithmId( "native:fillnodata" );
  algr2.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "filter" ), QStringLiteral( "RASTER" ) ) );
  QMap<QString, QgsProcessingModelOutput> outputsr2;
  QgsProcessingModelOutput outr2( "RASTER_OUTPUT" );
  outr2.setChildOutputName( "OUTPUT" );
  outputsr2.insert( QStringLiteral( "RASTER_OUTPUT" ), outr2 );
  algr2.setModelOutputs( outputsr2 );
  model1.addChildAlgorithm( algr2 );

  // some more bits on the raster branch
  QgsProcessingModelChildAlgorithm algr3;
  algr3.setChildId( "fill3" );
  algr3.setAlgorithmId( "native:fillnodata" );
  algr3.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "fill2" ), QStringLiteral( "OUTPUT" ) ) );
  QMap<QString, QgsProcessingModelOutput> outputsr3;
  QgsProcessingModelOutput outr3( "RASTER_OUTPUT2" );
  outr3.setChildOutputName( "OUTPUT" );
  outputsr3.insert( QStringLiteral( "RASTER_OUTPUT2" ), outr3 );
  algr3.setModelOutputs( outputsr3 );
  model1.addChildAlgorithm( algr3 );

  QgsProcessingModelChildAlgorithm algr4;
  algr4.setChildId( "fill4" );
  algr4.setAlgorithmId( "native:fillnodata" );
  algr4.addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "fill2" ), QStringLiteral( "OUTPUT" ) ) );
  QMap<QString, QgsProcessingModelOutput> outputsr4;
  QgsProcessingModelOutput outr4( "RASTER_OUTPUT3" );
  outr4.setChildOutputName( "OUTPUT" );
  outputsr4.insert( QStringLiteral( "RASTER_OUTPUT3" ), outr4 );
  algr4.setModelOutputs( outputsr4 );
  model1.addChildAlgorithm( algr4 );

  QgsProcessingFeedback feedback;
  QVariantMap params;
  // vector input
  params.insert( QStringLiteral( "LAYER" ), QStringLiteral( "v1" ) );
  params.insert( QStringLiteral( "buffer:BUFFER_OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "buffer2:BUFFER2_OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "buffer3:BUFFER3_OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "fill2:RASTER_OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "fill3:RASTER_OUTPUT2" ), QgsProcessing::TEMPORARY_OUTPUT );
  params.insert( QStringLiteral( "fill4:RASTER_OUTPUT3" ), QgsProcessing::TEMPORARY_OUTPUT );
  QVariantMap results = model1.run( params, context, &feedback );
  // we should get the vector branch outputs only
  QVERIFY( !results.value( QStringLiteral( "buffer:BUFFER_OUTPUT" ) ).toString().isEmpty() );
  QVERIFY( !results.value( QStringLiteral( "buffer2:BUFFER2_OUTPUT" ) ).toString().isEmpty() );
  QVERIFY( !results.value( QStringLiteral( "buffer3:BUFFER3_OUTPUT" ) ).toString().isEmpty() );
  QVERIFY( !results.contains( QStringLiteral( "fill2:RASTER_OUTPUT" ) ) );
  QVERIFY( !results.contains( QStringLiteral( "fill3:RASTER_OUTPUT2" ) ) );
  QVERIFY( !results.contains( QStringLiteral( "fill4:RASTER_OUTPUT3" ) ) );

  // raster input
  params.insert( QStringLiteral( "LAYER" ), QStringLiteral( "R1" ) );
  results = model1.run( params, context, &feedback );
  // we should get the raster branch outputs only
  QVERIFY( !results.value( QStringLiteral( "fill2:RASTER_OUTPUT" ) ).toString().isEmpty() );
  QVERIFY( !results.value( QStringLiteral( "fill3:RASTER_OUTPUT2" ) ).toString().isEmpty() );
  QVERIFY( !results.value( QStringLiteral( "fill4:RASTER_OUTPUT3" ) ).toString().isEmpty() );
  QVERIFY( !results.contains( QStringLiteral( "buffer:BUFFER_OUTPUT" ) ) );
  QVERIFY( !results.contains( QStringLiteral( "buffer2:BUFFER2_OUTPUT" ) ) );
  QVERIFY( !results.contains( QStringLiteral( "buffer3:BUFFER3_OUTPUT" ) ) );
}

void TestQgsProcessingModelAlgorithm::modelBranchPruningConditional()
{
  QgsProcessingContext context;

  context.expressionContext().appendScope( new QgsExpressionContextScope() );
  context.expressionContext().scope( 0 )->setVariable( QStringLiteral( "var1" ), 1 );
  context.expressionContext().scope( 0 )->setVariable( QStringLiteral( "var2" ), 0 );

  // test that model branches are trimmed for algorithms which depend on conditional branches
  QgsProcessingModelAlgorithm model1;

  // first add the filter by layer type alg
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( "branch" );
  algc1.setAlgorithmId( "native:condition" );
  QVariantMap config;
  QVariantList conditions;
  QVariantMap cond1;
  cond1.insert( QStringLiteral( "name" ), QStringLiteral( "name1" ) );
  cond1.insert( QStringLiteral( "expression" ), QStringLiteral( "@var1" ) );
  conditions << cond1;
  QVariantMap cond2;
  cond2.insert( QStringLiteral( "name" ), QStringLiteral( "name2" ) );
  cond2.insert( QStringLiteral( "expression" ), QStringLiteral( "@var2" ) );
  conditions << cond2;
  config.insert( QStringLiteral( "conditions" ), conditions );
  algc1.setConfiguration( config );
  model1.addChildAlgorithm( algc1 );

  //then create some branches which come off this
  QgsProcessingModelChildAlgorithm algc2;
  algc2.setChildId( "exception" );
  algc2.setAlgorithmId( "native:raiseexception" );
  algc2.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( QStringLiteral( "branch" ), QStringLiteral( "name1" ) ) );
  model1.addChildAlgorithm( algc2 );

  QgsProcessingModelChildAlgorithm algc3;
  algc2.setChildId( "exception" );
  algc3.setAlgorithmId( "native:raisewarning" );
  algc3.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( QStringLiteral( "branch" ), QStringLiteral( "name2" ) ) );
  model1.addChildAlgorithm( algc3 );

  QgsProcessingFeedback feedback;
  const QVariantMap params;
  bool ok = false;
  QVariantMap results = model1.run( params, context, &feedback, &ok );
  QVERIFY( !ok ); // the branch with the exception should be hit

  // flip the condition results
  context.expressionContext().scope( 0 )->setVariable( QStringLiteral( "var1" ), 0 );
  context.expressionContext().scope( 0 )->setVariable( QStringLiteral( "var2" ), 1 );

  results = model1.run( params, context, &feedback, &ok );
  QVERIFY( ok ); // the branch with the exception should NOT be hit
}

void TestQgsProcessingModelAlgorithm::modelWithProviderWithLimitedTypes()
{
  QgsApplication::processingRegistry()->addProvider( new DummyProvider4() );

  QgsProcessingModelAlgorithm alg( "test", "testGroup" );
  QgsProcessingModelChildAlgorithm algc1;
  algc1.setChildId( QStringLiteral( "cx1" ) );
  algc1.setAlgorithmId( "dummy4:alg1" );
  QMap<QString, QgsProcessingModelOutput> algc1outputs;
  QgsProcessingModelOutput algc1out1( QStringLiteral( "my_vector_output" ) );
  algc1out1.setChildId( QStringLiteral( "cx1" ) );
  algc1out1.setChildOutputName( "vector_dest" );
  algc1out1.setDescription( QStringLiteral( "my output" ) );
  algc1outputs.insert( QStringLiteral( "my_vector_output" ), algc1out1 );
  QgsProcessingModelOutput algc1out2( QStringLiteral( "my_raster_output" ) );
  algc1out2.setChildId( QStringLiteral( "cx1" ) );
  algc1out2.setChildOutputName( "raster_dest" );
  algc1out2.setDescription( QStringLiteral( "my output" ) );
  algc1outputs.insert( QStringLiteral( "my_raster_output" ), algc1out2 );
  QgsProcessingModelOutput algc1out3( QStringLiteral( "my_sink_output" ) );
  algc1out3.setChildId( QStringLiteral( "cx1" ) );
  algc1out3.setChildOutputName( "sink" );
  algc1out3.setDescription( QStringLiteral( "my output" ) );
  algc1outputs.insert( QStringLiteral( "my_sink_output" ), algc1out3 );
  algc1.setModelOutputs( algc1outputs );
  alg.addChildAlgorithm( algc1 );
  // verify that model has destination parameter created
  QCOMPARE( alg.destinationParameterDefinitions().count(), 3 );
  QCOMPARE( alg.destinationParameterDefinitions().at( 2 )->name(), QStringLiteral( "my_output_3" ) );
  QCOMPARE( alg.destinationParameterDefinitions().at( 2 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 2 ) )->originalProvider()->id(), QStringLiteral( "dummy4" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterVectorDestination * >( alg.destinationParameterDefinitions().at( 2 ) )->supportedOutputVectorLayerExtensions(), QStringList() << QStringLiteral( "mif" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterVectorDestination * >( alg.destinationParameterDefinitions().at( 2 ) )->defaultFileExtension(), QStringLiteral( "mif" ) );
  QVERIFY( static_cast< const QgsProcessingParameterVectorDestination * >( alg.destinationParameterDefinitions().at( 2 ) )->generateTemporaryDestination().endsWith( QLatin1String( ".mif" ) ) );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 2 ) )->supportsNonFileBasedOutput() );

  QCOMPARE( alg.destinationParameterDefinitions().at( 0 )->name(), QStringLiteral( "my_output" ) );
  QCOMPARE( alg.destinationParameterDefinitions().at( 0 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 0 ) )->originalProvider()->id(), QStringLiteral( "dummy4" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterRasterDestination * >( alg.destinationParameterDefinitions().at( 0 ) )->supportedOutputRasterLayerExtensions(), QStringList() << QStringLiteral( "mig" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterRasterDestination * >( alg.destinationParameterDefinitions().at( 0 ) )->defaultFileExtension(), QStringLiteral( "mig" ) );
  QVERIFY( static_cast< const QgsProcessingParameterRasterDestination * >( alg.destinationParameterDefinitions().at( 0 ) )->generateTemporaryDestination().endsWith( QLatin1String( ".mig" ) ) );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 0 ) )->supportsNonFileBasedOutput() );

  QCOMPARE( alg.destinationParameterDefinitions().at( 1 )->name(), QStringLiteral( "my_output_2" ) );
  QCOMPARE( alg.destinationParameterDefinitions().at( 1 )->description(), QStringLiteral( "my output" ) );
  QCOMPARE( static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 1 ) )->originalProvider()->id(), QStringLiteral( "dummy4" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterFeatureSink * >( alg.destinationParameterDefinitions().at( 1 ) )->supportedOutputVectorLayerExtensions(), QStringList() << QStringLiteral( "mif" ) );
  QCOMPARE( static_cast< const QgsProcessingParameterFeatureSink * >( alg.destinationParameterDefinitions().at( 1 ) )->defaultFileExtension(), QStringLiteral( "mif" ) );
  QVERIFY( static_cast< const QgsProcessingParameterFeatureSink * >( alg.destinationParameterDefinitions().at( 1 ) )->generateTemporaryDestination().endsWith( QLatin1String( ".mif" ) ) );
  QVERIFY( !static_cast< const QgsProcessingDestinationParameter * >( alg.destinationParameterDefinitions().at( 1 ) )->supportsNonFileBasedOutput() );
}

void TestQgsProcessingModelAlgorithm::modelVectorOutputIsCompatibleType()
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

void TestQgsProcessingModelAlgorithm::modelAcceptableValues()
{
  QgsProcessingModelAlgorithm m;
  const QgsProcessingModelParameter stringParam1( "string" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );

  const QgsProcessingModelParameter stringParam2( "string2" );
  m.addModelParameter( new QgsProcessingParameterString( "string2" ), stringParam2 );

  const QgsProcessingModelParameter numParam( "number" );
  m.addModelParameter( new QgsProcessingParameterNumber( "number" ), numParam );

  const QgsProcessingModelParameter tableFieldParam( QStringLiteral( "field" ) );
  m.addModelParameter( new QgsProcessingParameterField( QStringLiteral( "field" ) ), tableFieldParam );

  const QgsProcessingModelParameter fileParam( "file" );
  m.addModelParameter( new QgsProcessingParameterFile( "file" ), fileParam );

  // test single types
  QgsProcessingModelChildParameterSources sources = m.availableSourcesForChild( QString(), QStringList() << "number" );
  QCOMPARE( sources.count(), 1 );
  QCOMPARE( sources.at( 0 ).parameterName(), QStringLiteral( "number" ) );
  sources = m.availableSourcesForChild( QString(), QStringList() << QStringLiteral( "field" ) );
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
  alg2c1.setChildId( QStringLiteral( "cx1" ) );
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
  alg2c2.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( QStringLiteral( "cx1" ) ) );
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
  const QgsProcessingModelParameter vlInput( "vl" );
  // with no limit on data types
  m2.addModelParameter( new QgsProcessingParameterVectorLayer( "vl" ), vlInput );
  const QgsProcessingModelParameter fsInput( "fs" );
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

void TestQgsProcessingModelAlgorithm::modelValidate()
{
  QgsProcessingModelAlgorithm m;
  QStringList errors;
  QVERIFY( !m.validate( errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "Model does not contain any algorithms" ) );

  const QgsProcessingModelParameter stringParam1( "string" );
  m.addModelParameter( new QgsProcessingParameterString( "string" ), stringParam1 );
  QgsProcessingModelChildAlgorithm alg2c1;
  alg2c1.setChildId( QStringLiteral( "cx1" ) );
  alg2c1.setAlgorithmId( "native:centroids" );
  alg2c1.setDescription( QStringLiteral( "centroids" ) );
  m.addChildAlgorithm( alg2c1 );

  QVERIFY( !m.validateChildAlgorithm( QStringLiteral( "cx1" ), errors ) );
  QCOMPARE( errors.size(), 2 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "Parameter <i>INPUT</i> is mandatory" ) );
  QCOMPARE( errors.at( 1 ), QStringLiteral( "Parameter <i>ALL_PARTS</i> is mandatory" ) );

  QVERIFY( !m.validate( errors ) );
  QCOMPARE( errors.size(), 2 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "<b>centroids</b>: Parameter <i>INPUT</i> is mandatory" ) );
  QCOMPARE( errors.at( 1 ), QStringLiteral( "<b>centroids</b>: Parameter <i>ALL_PARTS</i> is mandatory" ) );

  QgsProcessingModelChildParameterSource badSource;
  badSource.setSource( QgsProcessingModelChildParameterSource::StaticValue );
  badSource.setStaticValue( 56 );
  m.childAlgorithm( QStringLiteral( "cx1" ) ).addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << badSource );

  QVERIFY( !m.validateChildAlgorithm( QStringLiteral( "cx1" ), errors ) );
  QCOMPARE( errors.size(), 2 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "Value for <i>INPUT</i> is not acceptable for this parameter" ) );
  QCOMPARE( errors.at( 1 ), QStringLiteral( "Parameter <i>ALL_PARTS</i> is mandatory" ) );

  QgsProcessingModelChildParameterSource goodSource;
  goodSource.setSource( QgsProcessingModelChildParameterSource::Expression );
  m.childAlgorithm( QStringLiteral( "cx1" ) ).addParameterSources( QStringLiteral( "ALL_PARTS" ), QList< QgsProcessingModelChildParameterSource >() << goodSource );

  QVERIFY( !m.validateChildAlgorithm( QStringLiteral( "cx1" ), errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "Value for <i>INPUT</i> is not acceptable for this parameter" ) );

  badSource.setSource( QgsProcessingModelChildParameterSource::ChildOutput );
  badSource.setOutputChildId( QStringLiteral( "cc" ) );
  m.childAlgorithm( QStringLiteral( "cx1" ) ).addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << badSource );

  QVERIFY( !m.validateChildAlgorithm( QStringLiteral( "cx1" ), errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "Child algorithm <i>cc</i> used for parameter <i>INPUT</i> does not exist" ) );

  badSource.setSource( QgsProcessingModelChildParameterSource::ModelParameter );
  badSource.setParameterName( QStringLiteral( "cc" ) );
  m.childAlgorithm( QStringLiteral( "cx1" ) ).addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << badSource );

  QVERIFY( !m.validateChildAlgorithm( QStringLiteral( "cx1" ), errors ) );
  QCOMPARE( errors.size(), 1 );
  QCOMPARE( errors.at( 0 ), QStringLiteral( "Model input <i>cc</i> used for parameter <i>INPUT</i> does not exist" ) );

  goodSource.setSource( QgsProcessingModelChildParameterSource::StaticValue );
  goodSource.setStaticValue( QString( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" ) );
  m.childAlgorithm( QStringLiteral( "cx1" ) ).addParameterSources( QStringLiteral( "INPUT" ), QList< QgsProcessingModelChildParameterSource >() << goodSource );

  QVERIFY( m.validateChildAlgorithm( QStringLiteral( "cx1" ), errors ) );
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
  QCOMPARE( m2.orderedParameters().at( 0 ).parameterName(), QStringLiteral( "cc string" ) );
  QCOMPARE( m2.orderedParameters().at( 1 ).parameterName(), QStringLiteral( "a string" ) );
  QCOMPARE( m2.orderedParameters().at( 2 ).parameterName(), QStringLiteral( "string" ) );

  QCOMPARE( m2.parameterDefinitions().at( 0 )->name(), QStringLiteral( "cc string" ) );
  QCOMPARE( m2.parameterDefinitions().at( 1 )->name(), QStringLiteral( "a string" ) );
  QCOMPARE( m2.parameterDefinitions().at( 2 )->name(), QStringLiteral( "string" ) );
}

void TestQgsProcessingModelAlgorithm::modelDependencies()
{
  const QgsProcessingModelChildDependency dep( QStringLiteral( "childId" ), QStringLiteral( "branch" ) );

  QCOMPARE( dep.childId, QStringLiteral( "childId" ) );
  QCOMPARE( dep.conditionalBranch, QStringLiteral( "branch" ) );

  const QVariant v = dep.toVariant();
  QgsProcessingModelChildDependency dep2;
  QVERIFY( dep2.loadVariant( v.toMap() ) );

  QCOMPARE( dep2.childId, QStringLiteral( "childId" ) );
  QCOMPARE( dep2.conditionalBranch, QStringLiteral( "branch" ) );

  QVERIFY( dep == dep2 );
  QVERIFY( !( dep != dep2 ) );
  dep2.conditionalBranch = QStringLiteral( "b" );

  QVERIFY( !( dep == dep2 ) );
  QVERIFY( dep != dep2 );
  dep2.conditionalBranch = QStringLiteral( "branch" );
  dep2.childId = QStringLiteral( "c" );
  QVERIFY( !( dep == dep2 ) );
  QVERIFY( dep != dep2 );
  dep2.childId = QStringLiteral( "childId" );
  QVERIFY( dep == dep2 );
  QVERIFY( !( dep != dep2 ) );
}

void TestQgsProcessingModelAlgorithm::modelSource()
{
  QgsProcessingModelChildParameterSource source;
  source.setExpression( QStringLiteral( "expression" ) );
  source.setExpressionText( QStringLiteral( "expression string" ) );
  source.setOutputName( QStringLiteral( "output name " ) );
  source.setStaticValue( QString( "value" ) );
  source.setOutputChildId( QStringLiteral( "output child id" ) );
  source.setParameterName( QStringLiteral( "parameter name" ) );
  source.setSource( QgsProcessingModelChildParameterSource::ChildOutput );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << source;

  ds.device()->seek( 0 );

  QgsProcessingModelChildParameterSource res;
  ds >> res;

  QCOMPARE( res.expression(), QStringLiteral( "expression" ) );
  QCOMPARE( res.expressionText(), QStringLiteral( "expression string" ) );
  QCOMPARE( res.outputName(), QStringLiteral( "output name " ) );
  QCOMPARE( res.staticValue().toString(), QStringLiteral( "value" ) );
  QCOMPARE( res.outputChildId(), QStringLiteral( "output child id" ) );
  QCOMPARE( res.parameterName(), QStringLiteral( "parameter name" ) );
  QCOMPARE( res.source(), QgsProcessingModelChildParameterSource::ChildOutput );
}

void TestQgsProcessingModelAlgorithm::modelNameMatchesFileName()
{
  QgsProcessingModelAlgorithm model;
  model.setName( QStringLiteral( "my name" ) );
  QVERIFY( !model.modelNameMatchesFilePath() );

  model.setSourceFilePath( QStringLiteral( "/home/me/my name.something.model3" ) );
  QVERIFY( !model.modelNameMatchesFilePath() );
  model.setSourceFilePath( QStringLiteral( "/home/me/my name.model3" ) );
  QVERIFY( model.modelNameMatchesFilePath() );
  model.setSourceFilePath( QStringLiteral( "/home/me/MY NAME.model3" ) );
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
  algc1.setChildId( QStringLiteral( "cx1" ) );
  algc1.setAlgorithmId( "native:buffer" );

  algc1.addParameterSources( QStringLiteral( "CHILD_OUTPUT" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromChildOutput( QStringLiteral( "filter" ), QStringLiteral( "VECTOR" ) ) );
  algc1.addParameterSources( QStringLiteral( "STATIC_VALUE" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromStaticValue( 5 ) );
  algc1.addParameterSources( QStringLiteral( "STATIC_VALUE_DD_EXPERESSION" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromStaticValue( QgsProperty::fromExpression( QStringLiteral( "@oldName * 2 + @string2" ) ) ) );
  algc1.addParameterSources( QStringLiteral( "MODEL_PARAM_1" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "oldName" ) ) );
  algc1.addParameterSources( QStringLiteral( "MODEL_PARAM_2" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromModelParameter( QStringLiteral( "string2" ) ) );
  algc1.addParameterSources( QStringLiteral( "EXPRESSION" ), QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "@oldName * 2 + @string2" ) ) );

  m.addChildAlgorithm( algc1 );

  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputChildId(), QStringLiteral( "filter" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputName(), QStringLiteral( "VECTOR" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE" ) ].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE_DD_EXPERESSION" ) ].constFirst().staticValue().value< QgsProperty >().expressionString(), QStringLiteral( "@oldName * 2 + @string2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_1" ) ].constFirst().parameterName(), QStringLiteral( "oldName" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_2" ) ].constFirst().parameterName(), QStringLiteral( "string2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "EXPRESSION" ) ].constFirst().expression(), QStringLiteral( "@oldName * 2 + @string2" ) );

  m.changeParameterName( QStringLiteral( "x" ), QStringLiteral( "y" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputChildId(), QStringLiteral( "filter" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputName(), QStringLiteral( "VECTOR" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE" ) ].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE_DD_EXPERESSION" ) ].constFirst().staticValue().value< QgsProperty >().expressionString(), QStringLiteral( "@oldName * 2 + @string2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_1" ) ].constFirst().parameterName(), QStringLiteral( "oldName" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_2" ) ].constFirst().parameterName(), QStringLiteral( "string2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "EXPRESSION" ) ].constFirst().expression(), QStringLiteral( "@oldName * 2 + @string2" ) );

  m.changeParameterName( QStringLiteral( "oldName" ), QStringLiteral( "apricot" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputChildId(), QStringLiteral( "filter" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputName(), QStringLiteral( "VECTOR" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE" ) ].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE_DD_EXPERESSION" ) ].constFirst().staticValue().value< QgsProperty >().expressionString(), QStringLiteral( "@apricot * 2 + @string2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_1" ) ].constFirst().parameterName(), QStringLiteral( "apricot" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_2" ) ].constFirst().parameterName(), QStringLiteral( "string2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "EXPRESSION" ) ].constFirst().expression(), QStringLiteral( "@apricot * 2 + @string2" ) );

  m.changeParameterName( QStringLiteral( "string2" ), QStringLiteral( "int2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputChildId(), QStringLiteral( "filter" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "CHILD_OUTPUT" ) ].constFirst().outputName(), QStringLiteral( "VECTOR" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE" ) ].constFirst().staticValue(), QVariant( 5 ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "STATIC_VALUE_DD_EXPERESSION" ) ].constFirst().staticValue().value< QgsProperty >().expressionString(), QStringLiteral( "@apricot * 2 + @int2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_1" ) ].constFirst().parameterName(), QStringLiteral( "apricot" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "MODEL_PARAM_2" ) ].constFirst().parameterName(), QStringLiteral( "int2" ) );
  QCOMPARE( m.childAlgorithm( QStringLiteral( "cx1" ) ).parameterSources()[ QStringLiteral( "EXPRESSION" ) ].constFirst().expression(), QStringLiteral( "@apricot * 2 + @int2" ) );
}

void TestQgsProcessingModelAlgorithm::internalVersion()
{
  // test internal version handling
  QgsProcessingModelAlgorithm model;

  // load older model, should be version 1
  QVERIFY( model.fromFile( TEST_DATA_DIR + QStringLiteral( "/test_model.model3" ) ) );
  QCOMPARE( model.mInternalVersion, QgsProcessingModelAlgorithm::InternalVersion::Version1 );

  // create new model and save/restore, should be version 2
  QgsProcessingModelAlgorithm model2;
  QgsProcessingModelAlgorithm model3;
  QVERIFY( model3.loadVariant( model2.toVariant() ) );
  QCOMPARE( model3.mInternalVersion, QgsProcessingModelAlgorithm::InternalVersion::Version2 );
}

QGSTEST_MAIN( TestQgsProcessingModelAlgorithm )
#include "testqgsprocessingmodelalgorithm.moc"
