/***************************************************************************
                         testqgsexpressioncontext.cpp
                         ----------------------------
    begin                : April 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#include "qgsexpressioncontext.h"
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgscolorscheme.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsExpressionContext : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void contextScope();
    void contextScopeCopy();
    void contextScopeFunctions();
    void contextStack();
    void contextCopy();
    void contextStackFunctions();
    void evaluate();
    void setFeature();
    void setFields();

    void globalScope();
    void projectScope();
    void layerScope();
    void featureBasedContext();

  private:

    class GetTestValueFunction : public QgsScopedExpressionFunction
    {
      public:
        GetTestValueFunction()
            : QgsScopedExpressionFunction( "get_test_value", 1, "test" ) {}

        virtual QVariant func( const QVariantList&, const QgsExpressionContext*, QgsExpression* ) override
        {
          return 42;
        }

        QgsScopedExpressionFunction* clone() const override
        {
          return new GetTestValueFunction();
        }

    };

    class GetTestValueFunction2 : public QgsScopedExpressionFunction
    {
      public:
        GetTestValueFunction2()
            : QgsScopedExpressionFunction( "get_test_value", 1, "test" ) {}

        virtual QVariant func( const QVariantList&, const QgsExpressionContext*, QgsExpression* ) override
        {
          return 43;
        }

        QgsScopedExpressionFunction* clone() const override
        {
          return new GetTestValueFunction2();
        }
    };

    class ModifiableFunction : public QgsScopedExpressionFunction
    {
      public:
        ModifiableFunction( int* v )
            : QgsScopedExpressionFunction( "test_function", 1, "test" )
            , mVal( v )
        {}

        virtual QVariant func( const QVariantList&, const QgsExpressionContext*, QgsExpression* ) override
        {
          if ( !mVal )
            return QVariant();

          return ++( *mVal );
        }

        QgsScopedExpressionFunction* clone() const override
        {
          return new ModifiableFunction( mVal );
        }

      private:

        int* mVal;
    };
};

void TestQgsExpressionContext::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );
}

void TestQgsExpressionContext::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsExpressionContext::init()
{

}

void TestQgsExpressionContext::cleanup()
{

}

void TestQgsExpressionContext::contextScope()
{
  QgsExpressionContextScope scope( "scope name" );
  QCOMPARE( scope.name(), QString( "scope name" ) );

  QVERIFY( !scope.hasVariable( "test" ) );
  QVERIFY( !scope.variable( "test" ).isValid() );
  QCOMPARE( scope.variableNames().length(), 0 );
  QCOMPARE( scope.variableCount(), 0 );

  scope.setVariable( "test", 5 );
  QVERIFY( scope.hasVariable( "test" ) );
  QVERIFY( scope.variable( "test" ).isValid() );
  QCOMPARE( scope.variable( "test" ).toInt(), 5 );
  QCOMPARE( scope.variableNames().length(), 1 );
  QCOMPARE( scope.variableCount(), 1 );
  QCOMPARE( scope.variableNames().at( 0 ), QString( "test" ) );

  scope.addVariable( QgsExpressionContextScope::StaticVariable( "readonly", QString( "readonly_test" ), true ) );
  QVERIFY( scope.isReadOnly( "readonly" ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "notreadonly", QString( "not_readonly_test" ), false ) );
  QVERIFY( !scope.isReadOnly( "notreadonly" ) );

  //updating a read only variable should remain read only
  scope.setVariable( "readonly", "newvalue" );
  QVERIFY( scope.isReadOnly( "readonly" ) );

  //removal
  scope.setVariable( "toremove", 5 );
  QVERIFY( scope.hasVariable( "toremove" ) );
  QVERIFY( !scope.removeVariable( "missing" ) );
  QVERIFY( scope.removeVariable( "toremove" ) );
  QVERIFY( !scope.hasVariable( "toremove" ) );
}

void TestQgsExpressionContext::contextScopeCopy()
{
  QgsExpressionContextScope scope( "scope name" );
  scope.setVariable( "test", 5 );
  scope.addFunction( "get_test_value", new GetTestValueFunction() );

  //copy constructor
  QgsExpressionContextScope copy( scope );
  QCOMPARE( copy.name(), QString( "scope name" ) );
  QVERIFY( copy.hasVariable( "test" ) );
  QCOMPARE( copy.variable( "test" ).toInt(), 5 );
  QCOMPARE( copy.variableNames().length(), 1 );
  QCOMPARE( copy.variableCount(), 1 );
  QCOMPARE( copy.variableNames().at( 0 ), QString( "test" ) );
  QVERIFY( copy.hasFunction( "get_test_value" ) );
  QVERIFY( copy.function( "get_test_value" ) );
}

void TestQgsExpressionContext::contextScopeFunctions()
{
  QgsExpressionContextScope scope;

  QVERIFY( !scope.hasFunction( "get_test_value" ) );
  QVERIFY( !scope.function( "get_test_value" ) );

  scope.addFunction( "get_test_value", new GetTestValueFunction() );
  QVERIFY( scope.hasFunction( "get_test_value" ) );
  QVERIFY( scope.function( "get_test_value" ) );
  QgsExpressionContext temp;
  QCOMPARE( scope.function( "get_test_value" )->func( QVariantList(), &temp, 0 ).toInt(), 42 );

  //test functionNames
  scope.addFunction( "get_test_value2", new GetTestValueFunction() );
  QStringList functionNames = scope.functionNames();
  QCOMPARE( functionNames.count(), 2 );
  QVERIFY( functionNames.contains( "get_test_value" ) );
  QVERIFY( functionNames.contains( "get_test_value2" ) );
}

void TestQgsExpressionContext::contextStack()
{
  QgsExpressionContext context;
  //test retrieving from empty context
  QVERIFY( !context.hasVariable( "test" ) );
  QVERIFY( !context.variable( "test" ).isValid() );
  QCOMPARE( context.variableNames().length(), 0 );
  QCOMPARE( context.scopeCount(), 0 );
  QVERIFY( !context.scope( 0 ) );
  QVERIFY( !context.lastScope() );

  //add a scope to the context
  QgsExpressionContextScope* testScope = new QgsExpressionContextScope();
  context << testScope;
  QVERIFY( !context.hasVariable( "test" ) );
  QVERIFY( !context.variable( "test" ).isValid() );
  QCOMPARE( context.variableNames().length(), 0 );
  QCOMPARE( context.scopeCount(), 1 );
  QCOMPARE( context.scope( 0 ), testScope );
  QCOMPARE( context.lastScope(), testScope );

  //some general context scope tests
  QVERIFY( !context.scope( -1 ) );
  QVERIFY( !context.scope( 5 ) );
  QgsExpressionContextScope* scope1 = context.scope( 0 );
  QCOMPARE( context.indexOfScope( scope1 ), 0 );
  QgsExpressionContextScope scopeNotInContext;
  QCOMPARE( context.indexOfScope( &scopeNotInContext ), -1 );

  //now add a variable to the first scope
  scope1->setVariable( "test", 1 );
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 1 );
  QCOMPARE( context.variableNames().length(), 1 );

  //add a second scope, should override the first
  QgsExpressionContextScope* scope2 = new QgsExpressionContextScope();
  context << scope2;
  QCOMPARE( context.scopeCount(), 2 );
  QCOMPARE( context.scope( 1 ), scope2 );
  QCOMPARE( context.lastScope(), scope2 );
  QCOMPARE( context.indexOfScope( scope2 ), 1 );
  //test without setting variable first...
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 1 );
  QCOMPARE( context.variableNames().length(), 1 );
  scope2->setVariable( "test", 2 );
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 2 );
  QCOMPARE( context.variableNames().length(), 1 );

  //make sure context falls back to earlier scopes
  scope1->setVariable( "test2", 11 );
  QVERIFY( context.hasVariable( "test2" ) );
  QCOMPARE( context.variable( "test2" ).toInt(), 11 );
  QCOMPARE( context.variableNames().length(), 2 );

  //check filteredVariableNames method
  scope2->setVariable( "_hidden", 5 );
  QStringList filteredNames = context.filteredVariableNames();
  QCOMPARE( filteredNames.count(), 2 );
  QCOMPARE( filteredNames.at( 0 ), QString( "test" ) );
  QCOMPARE( filteredNames.at( 1 ), QString( "test2" ) );

  //test scopes method
  QList< QgsExpressionContextScope*> scopes = context.scopes();
  QCOMPARE( scopes.length(), 2 );
  QCOMPARE( scopes.at( 0 ), scope1 );
  QCOMPARE( scopes.at( 1 ), scope2 );

  //check isReadOnly
  scope2->addVariable( QgsExpressionContextScope::StaticVariable( "readonly", 5, true ) );
  QVERIFY( context.isReadOnly( "readonly" ) );
  QVERIFY( !context.isReadOnly( "test" ) );
}

void TestQgsExpressionContext::contextCopy()
{
  QgsExpressionContext context;
  context << new QgsExpressionContextScope();
  context.scope( 0 )->setVariable( "test", 1 );

  //copy constructor
  QgsExpressionContext copy( context );
  QCOMPARE( copy.scopeCount(), 1 );
  QVERIFY( copy.hasVariable( "test" ) );
  QCOMPARE( copy.variable( "test" ).toInt(), 1 );
  QCOMPARE( copy.variableNames().length(), 1 );
}

void TestQgsExpressionContext::contextStackFunctions()
{
  QgsExpression::registerFunction( new GetTestValueFunction(), true );

  QgsExpressionContext context;
  //test retrieving from empty stack
  QVERIFY( !context.hasFunction( "get_test_value" ) );
  QVERIFY( !context.function( "get_test_value" ) );

  //add a scope to the context
  context << new QgsExpressionContextScope();
  QVERIFY( !context.hasFunction( "get_test_value" ) );
  QVERIFY( !context.function( "get_test_value" ) );

  //now add a function to the first scope
  QgsExpressionContextScope* scope1 = context.scope( 0 );
  scope1->addFunction( "get_test_value", new GetTestValueFunction() );
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  QgsExpressionContext temp;
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, 0 ).toInt(), 42 );

  //add a second scope, should override the first
  context << new QgsExpressionContextScope();
  //test without setting function first...
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, 0 ).toInt(), 42 );

  //then set the variable so it overrides
  QgsExpressionContextScope* scope2 = context.scope( 1 );
  scope2->addFunction( "get_test_value", new GetTestValueFunction2() );
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, 0 ).toInt(), 43 );

  //make sure stack falls back to earlier contexts
  scope2->addFunction( "get_test_value2", new GetTestValueFunction() );
  QVERIFY( context.hasFunction( "get_test_value2" ) );
  QVERIFY( context.function( "get_test_value2" ) );
  QCOMPARE( context.function( "get_test_value2" )->func( QVariantList(), &temp, 0 ).toInt(), 42 );

  //test functionNames
  QStringList names = context.functionNames();
  QCOMPARE( names.count(), 2 );
  QCOMPARE( names.at( 0 ), QString( "get_test_value" ) );
  QCOMPARE( names.at( 1 ), QString( "get_test_value2" ) );
}

void TestQgsExpressionContext::evaluate()
{
  QgsExpression exp( "1 + 2" );
  QCOMPARE( exp.evaluate().toInt(), 3 );

  QgsExpressionContext context;
  context << new QgsExpressionContextScope();

  QgsExpressionContextScope* s = context.scope( 0 );
  s->setVariable( "test", 5 );
  QCOMPARE( exp.evaluate( &context ).toInt(), 3 );
  QgsExpression expWithVariable( "var('test')" );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 5 );
  s->setVariable( "test", 7 );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 7 );
  QgsExpression expWithVariable2( "var('test') + var('test2')" );
  s->setVariable( "test2", 9 );
  QCOMPARE( expWithVariable2.evaluate( &context ).toInt(), 16 );

  QgsExpression expWithVariableBad( "var('bad')" );
  QVERIFY( !expWithVariableBad.evaluate( &context ).isValid() );

  //test shorthand variables
  QgsExpression expShorthand( "@test" );
  QCOMPARE( expShorthand.evaluate( &context ).toInt(), 7 );
  QgsExpression expShorthandBad( "@bad" );
  QVERIFY( !expShorthandBad.evaluate( &context ).isValid() );

  //test with a function provided by a context
  QgsExpression::registerFunction( new ModifiableFunction( 0 ), true );
  QgsExpression testExpWContextFunction( "test_function(1)" );
  QVERIFY( !testExpWContextFunction.evaluate( ).isValid() );

  int val1 = 5;
  s->addFunction( "test_function", new ModifiableFunction( &val1 ) );
  testExpWContextFunction.prepare( &context );
  QCOMPARE( testExpWContextFunction.evaluate( &context ).toInt(), 6 );
  QCOMPARE( testExpWContextFunction.evaluate( &context ).toInt(), 7 );
  QCOMPARE( val1, 7 );

  //test with another context to ensure that expressions are evaulated against correct context
  QgsExpressionContext context2;
  context2 << new QgsExpressionContextScope();
  QgsExpressionContextScope* s2 = context2.scope( 0 );
  int val2 = 50;
  s2->addFunction( "test_function", new ModifiableFunction( &val2 ) );
  QCOMPARE( testExpWContextFunction.evaluate( &context2 ).toInt(), 51 );
  QCOMPARE( testExpWContextFunction.evaluate( &context2 ).toInt(), 52 );
}

void TestQgsExpressionContext::setFeature()
{
  QgsFeature feature( 50LL );
  QgsExpressionContextScope scope;
  scope.setFeature( feature );
  QVERIFY( scope.hasVariable( QgsExpressionContext::EXPR_FEATURE ) );
  QCOMPARE(( qvariant_cast<QgsFeature>( scope.variable( QgsExpressionContext::EXPR_FEATURE ) ) ).id(), 50LL );

  //test setting a feature in a context with no scopes
  QgsExpressionContext emptyContext;
  emptyContext.setFeature( feature );
  //setFeature should have created a scope
  QCOMPARE( emptyContext.scopeCount(), 1 );
  QVERIFY( emptyContext.hasVariable( QgsExpressionContext::EXPR_FEATURE ) );
  QCOMPARE(( qvariant_cast<QgsFeature>( emptyContext.variable( QgsExpressionContext::EXPR_FEATURE ) ) ).id(), 50LL );

  QgsExpressionContext contextWithScope;
  contextWithScope << new QgsExpressionContextScope();
  contextWithScope.setFeature( feature );
  QCOMPARE( contextWithScope.scopeCount(), 1 );
  QVERIFY( contextWithScope.hasVariable( QgsExpressionContext::EXPR_FEATURE ) );
  QCOMPARE(( qvariant_cast<QgsFeature>( contextWithScope.variable( QgsExpressionContext::EXPR_FEATURE ) ) ).id(), 50LL );
}

void TestQgsExpressionContext::setFields()
{
  QgsFields fields;
  QgsField field( "testfield" );
  fields.append( field );

  QgsExpressionContextScope scope;
  scope.setFields( fields );
  QVERIFY( scope.hasVariable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE(( qvariant_cast<QgsFields>( scope.variable( QgsExpressionContext::EXPR_FIELDS ) ) ).at( 0 ).name(), QString( "testfield" ) );

  //test setting a fields in a context with no scopes
  QgsExpressionContext emptyContext;
  emptyContext.setFields( fields );
  //setFeature should have created a scope
  QCOMPARE( emptyContext.scopeCount(), 1 );
  QVERIFY( emptyContext.hasVariable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE(( qvariant_cast<QgsFields>( emptyContext.variable( QgsExpressionContext::EXPR_FIELDS ) ) ).at( 0 ).name(), QString( "testfield" ) );

  QgsExpressionContext contextWithScope;
  contextWithScope << new QgsExpressionContextScope();
  contextWithScope.setFields( fields );
  QCOMPARE( contextWithScope.scopeCount(), 1 );
  QVERIFY( contextWithScope.hasVariable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE(( qvariant_cast<QgsFields>( contextWithScope.variable( QgsExpressionContext::EXPR_FIELDS ) ) ).at( 0 ).name(), QString( "testfield" ) );
}

void TestQgsExpressionContext::globalScope()
{
  QgsExpressionContextUtils::setGlobalVariable( "test", "testval" );

  QgsExpressionContext context;
  QgsExpressionContextScope* globalScope = QgsExpressionContextUtils::globalScope();
  context << globalScope;
  QCOMPARE( globalScope->name(), tr( "Global" ) );

  QCOMPARE( context.variable( "test" ).toString(), QString( "testval" ) );

  QgsExpression expGlobal( "var('test')" );
  QCOMPARE( expGlobal.evaluate( &context ).toString(), QString( "testval" ) );

  //test some other recognized global variables
  QgsExpression expVersion( "var('qgis_version')" );
  QgsExpression expVersionNo( "var('qgis_version_no')" );
  QgsExpression expReleaseName( "var('qgis_release_name')" );

  QCOMPARE( expVersion.evaluate( &context ).toString(), QString( QGis::QGIS_VERSION ) );
  QCOMPARE( expVersionNo.evaluate( &context ).toInt(), QGis::QGIS_VERSION_INT );
  QCOMPARE( expReleaseName.evaluate( &context ).toString(), QString( QGis::QGIS_RELEASE_NAME ) );

  //test setGlobalVariables
  QgsStringMap vars;
  vars.insert( "newvar1", "val1" );
  vars.insert( "newvar2", "val2" );
  QgsExpressionContextUtils::setGlobalVariables( vars );
  QgsExpressionContextScope* globalScope2 = QgsExpressionContextUtils::globalScope();

  QVERIFY( !globalScope2->hasVariable( "test" ) );
  QCOMPARE( globalScope2->variable( "newvar1" ).toString(), QString( "val1" ) );
  QCOMPARE( globalScope2->variable( "newvar2" ).toString(), QString( "val2" ) );

  delete globalScope2;
}

void TestQgsExpressionContext::projectScope()
{
  QgsExpressionContextUtils::setProjectVariable( "test", "testval" );
  QgsExpressionContextUtils::setProjectVariable( "testdouble", 5.2 );

  QgsExpressionContext context;
  QgsExpressionContextScope* scope = QgsExpressionContextUtils::projectScope();
  context << scope;
  QCOMPARE( scope->name(), tr( "Project" ) );

  QCOMPARE( context.variable( "test" ).toString(), QString( "testval" ) );
  QCOMPARE( context.variable( "testdouble" ).toDouble(), 5.2 );

  QgsExpression expProject( "var('test')" );
  QCOMPARE( expProject.evaluate( &context ).toString(), QString( "testval" ) );

  //test clearing project variables
  QgsExpressionContextScope* projectScope = QgsExpressionContextUtils::projectScope();
  QVERIFY( projectScope->hasVariable( "test" ) );
  QgsProject::instance()->clear();
  delete projectScope;
  projectScope = QgsExpressionContextUtils::projectScope();
  QVERIFY( !projectScope->hasVariable( "test" ) );

  //test a preset project variable
  QgsProject::instance()->setTitle( "test project" );
  delete projectScope;
  projectScope = QgsExpressionContextUtils::projectScope();
  QCOMPARE( projectScope->variable( "project_title" ).toString(), QString( "test project" ) );
  delete projectScope;
  projectScope = 0;

  //test setProjectVariables
  QgsStringMap vars;
  vars.insert( "newvar1", "val1" );
  vars.insert( "newvar2", "val2" );
  QgsExpressionContextUtils::setProjectVariables( vars );
  projectScope = QgsExpressionContextUtils::projectScope();

  QVERIFY( !projectScope->hasVariable( "test" ) );
  QCOMPARE( projectScope->variable( "newvar1" ).toString(), QString( "val1" ) );
  QCOMPARE( projectScope->variable( "newvar2" ).toString(), QString( "val2" ) );
  delete projectScope;
  projectScope = 0;

  //test project scope functions

  //project_color function
  QgsProjectColorScheme s;
  QgsNamedColorList colorList;
  colorList << qMakePair( QColor( 200, 255, 0 ), QString( "vomit yellow" ) );
  colorList << qMakePair( QColor( 30, 60, 20 ), QString( "murky depths of hades" ) );
  s.setColors( colorList );
  QgsExpressionContext contextColors;
  contextColors << QgsExpressionContextUtils::projectScope();

  QgsExpression expProjectColor( "project_color('murky depths of hades')" );
  QCOMPARE( expProjectColor.evaluate( &contextColors ).toString(), QString( "30,60,20" ) );
  //matching color names should be case insensitive
  QgsExpression expProjectColorCaseInsensitive( "project_color('Murky Depths of hades')" );
  QCOMPARE( expProjectColorCaseInsensitive.evaluate( &contextColors ).toString(), QString( "30,60,20" ) );
  QgsExpression badProjectColor( "project_color('dusk falls in san juan del sur')" );
  QCOMPARE( badProjectColor.evaluate( &contextColors ), QVariant() );
}

void TestQgsExpressionContext::layerScope()
{
  //test passing no layer - should be no crash
  QgsExpressionContextScope* layerScope = QgsExpressionContextUtils::layerScope( 0 );
  QCOMPARE( layerScope->name(), tr( "Layer" ) );
  QCOMPARE( layerScope->variableCount(), 0 );
  delete layerScope;
  layerScope = 0;

  //create a map layer
  QScopedPointer<QgsVectorLayer> vectorLayer( new QgsVectorLayer( "Point?field=col1:integer&field=col2:integer&field=col3:integer", "test layer", "memory" ) );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::layerScope( vectorLayer.data() );

  QCOMPARE( context.variable( "layer_name" ).toString(), vectorLayer->name() );
  QCOMPARE( context.variable( "layer_id" ).toString(), vectorLayer->id() );

  QgsExpression expProject( "var('layer_name')" );
  QCOMPARE( expProject.evaluate( &context ).toString(), vectorLayer->name() );

  //check that fields were set
  QgsFields fromVar = qvariant_cast<QgsFields>( context.variable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE( fromVar, vectorLayer->pendingFields() );

  //test setting layer variables
  QgsExpressionContextUtils::setLayerVariable( vectorLayer.data(), "testvar", "testval" );
  delete layerScope;
  layerScope = QgsExpressionContextUtils::layerScope( vectorLayer.data() );
  QCOMPARE( layerScope->variable( "testvar" ).toString(), QString( "testval" ) );

  QgsStringMap variables;
  variables.insert( "var1", "val1" );
  variables.insert( "var2", "val2" );
  QgsExpressionContextUtils::setLayerVariables( vectorLayer.data(), variables );
  delete layerScope;
  layerScope = QgsExpressionContextUtils::layerScope( vectorLayer.data() );
  QCOMPARE( layerScope->variable( "testvar" ), QVariant() );
  QCOMPARE( layerScope->variable( "var1" ).toString(), QString( "val1" ) );
  QCOMPARE( layerScope->variable( "var2" ).toString(), QString( "val2" ) );
  delete layerScope;
}

void TestQgsExpressionContext::featureBasedContext()
{
  QgsFields fields;
  fields.append( QgsField( "x1" ) );
  fields.append( QgsField( "x2" ) );
  fields.append( QgsField( "foo", QVariant::Int ) );

  QgsFeature f;
  f.initAttributes( 3 );
  f.setAttribute( 2, QVariant( 20 ) );

  QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );

  QgsFeature evalFeature = qvariant_cast<QgsFeature>( context.variable( "_feature_" ) );
  QgsFields evalFields = qvariant_cast<QgsFields>( context.variable( "_fields_" ) );
  QCOMPARE( evalFeature.attributes(), f.attributes() );
  QCOMPARE( evalFields, fields );
}

QTEST_MAIN( TestQgsExpressionContext )
#include "testqgsexpressioncontext.moc"
