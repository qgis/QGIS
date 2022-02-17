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
#include "qgsexpressioncontextutils.h"

#include <QObject>
#include "qgstest.h"

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
    void scopeByName();
    void contextCopy();
    void contextStackFunctions();
    void evaluate();
    void setFeature();
    void setGeometry();
    void setFields();
    void takeScopes();
    void highlighted();

    void globalScope();
    void projectScope();
    void layerScope();
    void featureBasedContext();

    void cache();

    void valuesAsMap();
    void description();
    void readWriteScope();

  private:

    class GetTestValueFunction : public QgsScopedExpressionFunction
    {
      public:
        GetTestValueFunction()
          : QgsScopedExpressionFunction( QStringLiteral( "get_test_value" ), 1, QStringLiteral( "test" ) ) {}

        QVariant func( const QVariantList &, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override
        {
          return 42;
        }

        QgsScopedExpressionFunction *clone() const override
        {
          return new GetTestValueFunction();
        }

    };

    class GetTestValueFunction2 : public QgsScopedExpressionFunction
    {
      public:
        GetTestValueFunction2()
          : QgsScopedExpressionFunction( QStringLiteral( "get_test_value" ), 1, QStringLiteral( "test" ) ) {}

        QVariant func( const QVariantList &, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override
        {
          return 43;
        }

        QgsScopedExpressionFunction *clone() const override
        {
          return new GetTestValueFunction2();
        }
    };

    class ModifiableFunction : public QgsScopedExpressionFunction
    {
      public:
        explicit ModifiableFunction( int *v )
          : QgsScopedExpressionFunction( QStringLiteral( "test_function" ), 1, QStringLiteral( "test" ) )
          , mVal( v )
        {}

        QVariant func( const QVariantList &, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override
        {
          if ( !mVal )
            return QVariant();

          return ++( *mVal );
        }

        QgsScopedExpressionFunction *clone() const override
        {
          return new ModifiableFunction( mVal );
        }

        /**
         * This function is not static, it's value changes with every invocation.
         */
        bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override
        {
          Q_UNUSED( node )
          Q_UNUSED( parent )
          Q_UNUSED( context )
          return false;
        }

      private:

        int *mVal = nullptr;
    };
};

void TestQgsExpressionContext::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
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
  QgsExpressionContextScope scope( QStringLiteral( "scope name" ) );
  QCOMPARE( scope.name(), QString( "scope name" ) );

  QVERIFY( !scope.hasVariable( "test" ) );
  QVERIFY( !scope.variable( "test" ).isValid() );
  QCOMPARE( scope.variableNames().length(), 0 );
  QCOMPARE( scope.variableCount(), 0 );
  QVERIFY( scope.description( "test" ).isEmpty() );

  scope.setVariable( QStringLiteral( "test" ), 5 );
  QVERIFY( scope.hasVariable( "test" ) );
  QVERIFY( scope.variable( "test" ).isValid() );
  QCOMPARE( scope.variable( "test" ).toInt(), 5 );
  QCOMPARE( scope.variableNames().length(), 1 );
  QCOMPARE( scope.variableCount(), 1 );
  QCOMPARE( scope.variableNames().at( 0 ), QString( "test" ) );
  QVERIFY( scope.description( "test" ).isEmpty() );

  scope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "readonly" ), QStringLiteral( "readonly_test" ), true, false, QStringLiteral( "readonly variable" ) ) );
  QVERIFY( scope.isReadOnly( "readonly" ) );
  QCOMPARE( scope.description( "readonly" ), QStringLiteral( "readonly variable" ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "notreadonly" ), QStringLiteral( "not_readonly_test" ), false ) );
  QVERIFY( !scope.isReadOnly( "notreadonly" ) );

  //updating a read only variable should remain read only
  scope.setVariable( QStringLiteral( "readonly" ), "newvalue" );
  QVERIFY( scope.isReadOnly( "readonly" ) );
  // and keep its description
  QCOMPARE( scope.description( "readonly" ), QStringLiteral( "readonly variable" ) );

  //test retrieving filtered variable names
  scope.setVariable( QStringLiteral( "_hidden_" ), "hidden" );
  QCOMPARE( scope.filteredVariableNames(), QStringList() << "readonly" << "notreadonly" << "test" );

  //removal
  scope.setVariable( QStringLiteral( "toremove" ), 5 );
  QVERIFY( scope.hasVariable( "toremove" ) );
  QVERIFY( !scope.removeVariable( "missing" ) );
  QVERIFY( scope.removeVariable( "toremove" ) );
  QVERIFY( !scope.hasVariable( "toremove" ) );
}

void TestQgsExpressionContext::contextScopeCopy()
{
  QgsExpressionContextScope scope( QStringLiteral( "scope name" ) );
  scope.setVariable( QStringLiteral( "test" ), 5 );
  scope.addFunction( QStringLiteral( "get_test_value" ), new GetTestValueFunction() );

  //copy constructor
  const QgsExpressionContextScope copy( scope );
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

  scope.addFunction( QStringLiteral( "get_test_value" ), new GetTestValueFunction() );
  QVERIFY( scope.hasFunction( "get_test_value" ) );
  QVERIFY( scope.function( "get_test_value" ) );
  const QgsExpressionContext temp;
  QCOMPARE( scope.function( "get_test_value" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 42 );

  //test functionNames
  scope.addFunction( QStringLiteral( "get_test_value2" ), new GetTestValueFunction() );
  const QStringList functionNames = scope.functionNames();
  QCOMPARE( functionNames.count(), 2 );
  QVERIFY( functionNames.contains( "get_test_value" ) );
  QVERIFY( functionNames.contains( "get_test_value2" ) );
}

void TestQgsExpressionContext::contextStack()
{
  QgsExpressionContext context;

  context.popScope();

  //test retrieving from empty context
  QVERIFY( !context.hasVariable( "test" ) );
  QVERIFY( !context.variable( "test" ).isValid() );
  QCOMPARE( context.variableNames().length(), 0 );
  QCOMPARE( context.scopeCount(), 0 );
  QVERIFY( !context.scope( 0 ) );
  QVERIFY( !context.lastScope() );
  QVERIFY( context.description( "test" ).isEmpty() );

  //add a scope to the context
  QgsExpressionContextScope *testScope = new QgsExpressionContextScope();
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
  QgsExpressionContextScope *scope1 = context.scope( 0 );
  QCOMPARE( context.indexOfScope( scope1 ), 0 );
  QgsExpressionContextScope scopeNotInContext;
  QCOMPARE( context.indexOfScope( &scopeNotInContext ), -1 );

  //now add a variable to the first scope
  scope1->setVariable( QStringLiteral( "test" ), 1 );
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 1 );
  QCOMPARE( context.variableNames().length(), 1 );

  //add a second scope, should override the first
  QgsExpressionContextScope *scope2 = new QgsExpressionContextScope();
  context << scope2;
  QCOMPARE( context.scopeCount(), 2 );
  QCOMPARE( context.scope( 1 ), scope2 );
  QCOMPARE( context.lastScope(), scope2 );
  QCOMPARE( context.indexOfScope( scope2 ), 1 );
  //test without setting variable first...
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 1 );
  QCOMPARE( context.variableNames().length(), 1 );
  scope2->setVariable( QStringLiteral( "test" ), 2 );
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 2 );
  QCOMPARE( context.variableNames().length(), 1 );

  //make sure context falls back to earlier scopes
  scope1->setVariable( QStringLiteral( "test2" ), 11 );
  QVERIFY( context.hasVariable( "test2" ) );
  QCOMPARE( context.variable( "test2" ).toInt(), 11 );
  QCOMPARE( context.variableNames().length(), 2 );

  //check filteredVariableNames method
  scope2->setVariable( QStringLiteral( "_hidden" ), 5 );
  const QStringList filteredNames = context.filteredVariableNames();
  QCOMPARE( filteredNames.count(), 2 );
  QCOMPARE( filteredNames.at( 0 ), QString( "test" ) );
  QCOMPARE( filteredNames.at( 1 ), QString( "test2" ) );

  //test scopes method
  const QList< QgsExpressionContextScope *> scopes = context.scopes();
  QCOMPARE( scopes.length(), 2 );
  QCOMPARE( scopes.at( 0 ), scope1 );
  QCOMPARE( scopes.at( 1 ), scope2 );

  QVERIFY( context.description( "readonly" ).isEmpty() );

  //check isReadOnly
  scope2->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "readonly" ), 5, true, false, QStringLiteral( "readonly variable" ) ) );
  QVERIFY( context.isReadOnly( "readonly" ) );
  QVERIFY( !context.isReadOnly( "test" ) );
  QCOMPARE( context.description( "readonly" ), QStringLiteral( "readonly variable" ) );

  // Check scopes can be popped
  delete context.popScope();
  QCOMPARE( scopes.length(), 2 );
  QCOMPARE( scopes.at( 0 ), scope1 );
}

void TestQgsExpressionContext::scopeByName()
{
  QgsExpressionContext context;
  QCOMPARE( context.indexOfScope( "test1" ), -1 );
  context << new QgsExpressionContextScope( QStringLiteral( "test1" ) );
  context << new QgsExpressionContextScope( QStringLiteral( "test2" ) );
  QCOMPARE( context.indexOfScope( "test1" ), 0 );
  QCOMPARE( context.indexOfScope( "test2" ), 1 );
  QCOMPARE( context.indexOfScope( "not in context" ), -1 );
}

void TestQgsExpressionContext::contextCopy()
{
  QgsExpressionContext context;
  context << new QgsExpressionContextScope();
  context.scope( 0 )->setVariable( QStringLiteral( "test" ), 1 );

  //copy constructor
  const QgsExpressionContext copy( context );
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
  QgsExpressionContextScope *scope1 = context.scope( 0 );
  scope1->addFunction( QStringLiteral( "get_test_value" ), new GetTestValueFunction() );
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  const QgsExpressionContext temp;
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 42 );

  //add a second scope, should override the first
  context << new QgsExpressionContextScope();
  //test without setting function first...
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 42 );

  //then set the variable so it overrides
  QgsExpressionContextScope *scope2 = context.scope( 1 );
  scope2->addFunction( QStringLiteral( "get_test_value" ), new GetTestValueFunction2() );
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 43 );

  //make sure stack falls back to earlier contexts
  scope2->addFunction( QStringLiteral( "get_test_value2" ), new GetTestValueFunction() );
  QVERIFY( context.hasFunction( "get_test_value2" ) );
  QVERIFY( context.function( "get_test_value2" ) );
  QCOMPARE( context.function( "get_test_value2" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 42 );

  //test functionNames
  const QStringList names = context.functionNames();
  QCOMPARE( names.count(), 2 );
  QCOMPARE( names.at( 0 ), QString( "get_test_value" ) );
  QCOMPARE( names.at( 1 ), QString( "get_test_value2" ) );
}

void TestQgsExpressionContext::evaluate()
{
  QgsExpression exp( QStringLiteral( "1 + 2" ) );
  QCOMPARE( exp.evaluate().toInt(), 3 );

  QgsExpressionContext context;
  context << new QgsExpressionContextScope();

  QgsExpressionContextScope *s = context.scope( 0 );
  s->setVariable( QStringLiteral( "test" ), 5 );
  QCOMPARE( exp.evaluate( &context ).toInt(), 3 );
  QgsExpression expWithVariable( QStringLiteral( "var('test')" ) );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 5 );
  s->setVariable( QStringLiteral( "test" ), 7 );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 7 );
  QgsExpression expWithVariable2( QStringLiteral( "var('test') + var('test2')" ) );
  s->setVariable( QStringLiteral( "test2" ), 9 );
  QCOMPARE( expWithVariable2.evaluate( &context ).toInt(), 16 );

  QgsExpression expWithVariableBad( QStringLiteral( "var('bad')" ) );
  QVERIFY( !expWithVariableBad.evaluate( &context ).isValid() );

  //test shorthand variables
  QgsExpression expShorthand( QStringLiteral( "@test" ) );
  QCOMPARE( expShorthand.evaluate( &context ).toInt(), 7 );
  QgsExpression expShorthandBad( QStringLiteral( "@bad" ) );
  QVERIFY( !expShorthandBad.evaluate( &context ).isValid() );

  //test with a function provided by a context
  QgsExpression::registerFunction( new ModifiableFunction( nullptr ), true );
  QgsExpression testExpWContextFunction( QStringLiteral( "test_function(1)" ) );
  QVERIFY( !testExpWContextFunction.evaluate().isValid() );

  int val1 = 5;
  s->addFunction( QStringLiteral( "test_function" ), new ModifiableFunction( &val1 ) );
  testExpWContextFunction.prepare( &context );
  QCOMPARE( testExpWContextFunction.evaluate( &context ).toInt(), 6 );
  QCOMPARE( testExpWContextFunction.evaluate( &context ).toInt(), 7 );
  QCOMPARE( val1, 7 );

  //test with another context to ensure that expressions are evaluated against correct context
  QgsExpressionContext context2;
  context2 << new QgsExpressionContextScope();
  QgsExpressionContextScope *s2 = context2.scope( 0 );
  int val2 = 50;
  s2->addFunction( QStringLiteral( "test_function" ), new ModifiableFunction( &val2 ) );
  QCOMPARE( testExpWContextFunction.evaluate( &context2 ).toInt(), 51 );
  QCOMPARE( testExpWContextFunction.evaluate( &context2 ).toInt(), 52 );
}

void TestQgsExpressionContext::setFeature()
{
  QgsFeature feature( 50LL );
  feature.setValid( true );
  QgsExpressionContextScope scope;
  scope.setFeature( feature );
  QVERIFY( scope.hasFeature() );
  QCOMPARE( scope.feature().id(), 50LL );
  scope.removeFeature();
  QVERIFY( !scope.hasFeature() );
  QVERIFY( !scope.feature().isValid() );

  //test setting a feature in a context with no scopes
  QgsExpressionContext emptyContext;
  QVERIFY( !emptyContext.feature().isValid() );
  emptyContext.setFeature( feature );
  //setFeature should have created a scope
  QCOMPARE( emptyContext.scopeCount(), 1 );
  QVERIFY( emptyContext.feature().isValid() );
  QCOMPARE( emptyContext.feature().id(), 50LL );
  QCOMPARE( emptyContext.feature().id(), 50LL );

  QgsExpressionContext contextWithScope;
  contextWithScope << new QgsExpressionContextScope();
  contextWithScope.setFeature( feature );
  QCOMPARE( contextWithScope.scopeCount(), 1 );
  QVERIFY( contextWithScope.feature().isValid() );
  QCOMPARE( contextWithScope.feature().id(), 50LL );
  QCOMPARE( contextWithScope.feature().id(), 50LL );
}

void TestQgsExpressionContext::setGeometry()
{
  QgsGeometry g( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ) );
  QgsExpressionContextScope scope;
  scope.setGeometry( g );
  QVERIFY( scope.hasGeometry() );
  QCOMPARE( scope.geometry().asWkt(), QStringLiteral( "Point (1 2)" ) );
  scope.removeGeometry();
  QVERIFY( !scope.hasGeometry() );
  QVERIFY( scope.geometry().isNull() );

  //test setting a geometry in a context with no scopes
  QgsExpressionContext emptyContext;
  QVERIFY( !emptyContext.hasGeometry() );
  QVERIFY( emptyContext.geometry().isNull() );
  emptyContext.setGeometry( g );
  //setGeometry should have created a scope
  QCOMPARE( emptyContext.scopeCount(), 1 );
  QVERIFY( emptyContext.hasGeometry() );
  QCOMPARE( emptyContext.geometry().asWkt(), QStringLiteral( "Point (1 2)" ) );

  QgsExpressionContext contextWithScope;
  contextWithScope << new QgsExpressionContextScope();
  contextWithScope.setGeometry( g );
  QCOMPARE( contextWithScope.scopeCount(), 1 );
  QVERIFY( contextWithScope.hasGeometry() );
  QCOMPARE( contextWithScope.geometry().asWkt(), QStringLiteral( "Point (1 2)" ) );
}

void TestQgsExpressionContext::setFields()
{
  QgsFields fields;
  const QgsField field( QStringLiteral( "testfield" ) );
  fields.append( field );

  QgsExpressionContextScope scope;
  scope.setFields( fields );
  QVERIFY( scope.hasVariable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE( ( qvariant_cast<QgsFields>( scope.variable( QgsExpressionContext::EXPR_FIELDS ) ) ).at( 0 ).name(), QString( "testfield" ) );

  //test setting a fields in a context with no scopes
  QgsExpressionContext emptyContext;
  QVERIFY( emptyContext.fields().isEmpty() );
  emptyContext.setFields( fields );
  //setFeature should have created a scope
  QCOMPARE( emptyContext.scopeCount(), 1 );
  QVERIFY( emptyContext.hasVariable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE( ( qvariant_cast<QgsFields>( emptyContext.variable( QgsExpressionContext::EXPR_FIELDS ) ) ).at( 0 ).name(), QString( "testfield" ) );
  QCOMPARE( emptyContext.fields().at( 0 ).name(), QString( "testfield" ) );

  QgsExpressionContext contextWithScope;
  contextWithScope << new QgsExpressionContextScope();
  contextWithScope.setFields( fields );
  QCOMPARE( contextWithScope.scopeCount(), 1 );
  QVERIFY( contextWithScope.hasVariable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE( ( qvariant_cast<QgsFields>( contextWithScope.variable( QgsExpressionContext::EXPR_FIELDS ) ) ).at( 0 ).name(), QString( "testfield" ) );
  QCOMPARE( contextWithScope.fields().at( 0 ).name(), QString( "testfield" ) );
}

void TestQgsExpressionContext::takeScopes()
{
  QgsExpressionContextUtils::setGlobalVariable( QStringLiteral( "test_global" ), "testval" );

  QgsProject *project = QgsProject::instance();
  QgsExpressionContextUtils::setProjectVariable( project, QStringLiteral( "test_project" ), "testval" );

  QgsExpressionContext context;

  QgsExpressionContextScope *projectScope = QgsExpressionContextUtils::projectScope( project );

  QgsExpressionContextScope *globalScope = QgsExpressionContextUtils::globalScope();
  context << globalScope
          << projectScope;

  QCOMPARE( context.variable( "test_global" ).toString(), QString( "testval" ) );
  QCOMPARE( context.variable( "test_project" ).toString(), QString( "testval" ) );

  const auto scopes = context.takeScopes();

  QCOMPARE( scopes.length(), 2 );
  QVERIFY( scopes.at( 0 )->hasVariable( "test_global" ) );
  QVERIFY( scopes.at( 1 )->hasVariable( "test_project" ) );

  qDeleteAll( scopes );

  QVERIFY( !context.variable( "test_global" ).isValid() );
  QVERIFY( !context.variable( "test_project" ).isValid() );
}

void TestQgsExpressionContext::highlighted()
{
  QgsExpressionContext context;
  QVERIFY( !context.isHighlightedFunction( QStringLiteral( "x" ) ) );
  QVERIFY( !context.isHighlightedVariable( QStringLiteral( "x" ) ) );
  QVERIFY( context.highlightedVariables().isEmpty() );
  context.setHighlightedFunctions( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) );
  QVERIFY( context.isHighlightedFunction( QStringLiteral( "x" ) ) );
  QVERIFY( context.isHighlightedFunction( QStringLiteral( "y" ) ) );
  QVERIFY( !context.isHighlightedFunction( QStringLiteral( "z" ) ) );
  QVERIFY( !context.isHighlightedVariable( QStringLiteral( "x" ) ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QCOMPARE( context.highlightedVariables(), QStringList() << QStringLiteral( "a" ) << QStringLiteral( "b" ) );
  QVERIFY( context.isHighlightedVariable( QStringLiteral( "a" ) ) );
  QVERIFY( context.isHighlightedVariable( QStringLiteral( "b" ) ) );
  QVERIFY( !context.isHighlightedVariable( QStringLiteral( "c" ) ) );
  QVERIFY( !context.isHighlightedFunction( QStringLiteral( "a" ) ) );
  context.setHighlightedFunctions( QStringList() );
  context.setHighlightedVariables( QStringList() );
  QVERIFY( !context.isHighlightedFunction( QStringLiteral( "x" ) ) );
  QVERIFY( !context.isHighlightedVariable( QStringLiteral( "a" ) ) );
  QVERIFY( context.highlightedVariables().isEmpty() );
}

void TestQgsExpressionContext::globalScope()
{
  QgsExpressionContextUtils::setGlobalVariable( QStringLiteral( "test" ), "testval" );

  QgsExpressionContext context;
  QgsExpressionContextScope *globalScope = QgsExpressionContextUtils::globalScope();
  context << globalScope;
  QCOMPARE( globalScope->name(), tr( "Global" ) );

  QCOMPARE( context.variable( "test" ).toString(), QString( "testval" ) );

  QgsExpression expGlobal( QStringLiteral( "var('test')" ) );
  QCOMPARE( expGlobal.evaluate( &context ).toString(), QString( "testval" ) );

  //test some other recognized global variables
  QgsExpression expVersion( QStringLiteral( "var('qgis_version')" ) );
  QgsExpression expVersionNo( QStringLiteral( "var('qgis_version_no')" ) );
  QgsExpression expReleaseName( QStringLiteral( "var('qgis_release_name')" ) );
  QgsExpression expAccountName( QStringLiteral( "var('user_account_name')" ) );
  QgsExpression expUserFullName( QStringLiteral( "var('user_full_name')" ) );
  QgsExpression expOsName( QStringLiteral( "var('qgis_os_name')" ) );
  QgsExpression expPlatform( QStringLiteral( "var('qgis_platform')" ) );

  QCOMPARE( expVersion.evaluate( &context ).toString(), Qgis::version() );
  QCOMPARE( expVersionNo.evaluate( &context ).toInt(), Qgis::versionInt() );
  QCOMPARE( expReleaseName.evaluate( &context ).toString(), Qgis::releaseName() );
  QCOMPARE( expAccountName.evaluate( &context ).toString(), QgsApplication::userLoginName() );
  QCOMPARE( expUserFullName.evaluate( &context ).toString(), QgsApplication::userFullName() );
  QCOMPARE( expOsName.evaluate( &context ).toString(), QgsApplication::osName() );
  QCOMPARE( expPlatform.evaluate( &context ).toString(), QgsApplication::platform() );

  //test setGlobalVariables
  QVariantMap vars;
  vars.insert( QStringLiteral( "newvar1" ), QStringLiteral( "val1" ) );
  vars.insert( QStringLiteral( "newvar2" ), QStringLiteral( "val2" ) );
  QgsExpressionContextUtils::setGlobalVariables( vars );
  QgsExpressionContextScope *globalScope2 = QgsExpressionContextUtils::globalScope();

  QVERIFY( !globalScope2->hasVariable( "test" ) );
  QCOMPARE( globalScope2->variable( "newvar1" ).toString(), QString( "val1" ) );
  QCOMPARE( globalScope2->variable( "newvar2" ).toString(), QString( "val2" ) );

  delete globalScope2;

  //test removeGlobalVariables
  QgsExpressionContextUtils::setGlobalVariable( QStringLiteral( "key" ), "value" );
  std::unique_ptr< QgsExpressionContextScope > globalScope3( QgsExpressionContextUtils::globalScope() );
  QVERIFY( globalScope3->hasVariable( "key" ) );
  QgsExpressionContextUtils::removeGlobalVariable( QStringLiteral( "key" ) );
  globalScope3.reset( QgsExpressionContextUtils::globalScope() );
  QVERIFY( !globalScope3->hasVariable( "key" ) );
}

void TestQgsExpressionContext::projectScope()
{
  QgsProject project;
  QgsProjectMetadata md;
  md.setTitle( QStringLiteral( "project title" ) );
  md.setAuthor( QStringLiteral( "project author" ) );
  md.setAbstract( QStringLiteral( "project abstract" ) );
  md.setCreationDateTime( QDateTime( QDate( 2011, 3, 5 ), QTime( 9, 5, 4 ) ) );
  md.setIdentifier( QStringLiteral( "project identifier" ) );
  QgsAbstractMetadataBase::KeywordMap keywords;
  keywords.insert( QStringLiteral( "voc1" ), QStringList() << "a" << "b" );
  keywords.insert( QStringLiteral( "voc2" ), QStringList() << "c" << "d" );
  md.setKeywords( keywords );
  project.setMetadata( md );

  QgsExpressionContextUtils::setProjectVariable( &project, QStringLiteral( "test" ), "testval" );
  QgsExpressionContextUtils::setProjectVariable( &project, QStringLiteral( "testdouble" ), 5.2 );

  QgsExpressionContext context;
  QgsExpressionContextScope *scope = QgsExpressionContextUtils::projectScope( &project );
  context << scope;
  QCOMPARE( scope->name(), tr( "Project" ) );

  // metadata variables
  QCOMPARE( context.variable( "project_title" ).toString(), QStringLiteral( "project title" ) );
  QCOMPARE( context.variable( "project_author" ).toString(), QStringLiteral( "project author" ) );
  QCOMPARE( context.variable( "project_abstract" ).toString(), QStringLiteral( "project abstract" ) );
  QCOMPARE( context.variable( "project_creation_date" ).toDateTime(),  QDateTime( QDate( 2011, 3, 5 ), QTime( 9, 5, 4 ) ) );
  QCOMPARE( context.variable( "project_identifier" ).toString(), QStringLiteral( "project identifier" ) );
  QVariantMap keywordsExpected;
  keywordsExpected.insert( QStringLiteral( "voc1" ), QStringList() << "a" << "b" );
  keywordsExpected.insert( QStringLiteral( "voc2" ), QStringList() << "c" << "d" );
  const QVariantMap keywordsResult = context.variable( "project_keywords" ).toMap();
  QCOMPARE( keywordsResult, keywordsExpected );

  QCOMPARE( context.variable( "test" ).toString(), QString( "testval" ) );
  QCOMPARE( context.variable( "testdouble" ).toDouble(), 5.2 );

  QgsExpression expProject( QStringLiteral( "var('test')" ) );
  QCOMPARE( expProject.evaluate( &context ).toString(), QString( "testval" ) );

  // layers
  QVERIFY( context.variable( "layers" ).isValid() );
  QVERIFY( context.variable( "layer_ids" ).isValid() );
  QVERIFY( context.variable( "layers" ).toList().isEmpty() );
  QVERIFY( context.variable( "layer_ids" ).toList().isEmpty() );

  // add layer
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "test layer" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vectorLayer2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "test layer" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vectorLayer3 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "test layer" ), QStringLiteral( "memory" ) );
  project.addMapLayer( vectorLayer );
  QgsExpressionContextScope *projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 1 );
  QCOMPARE( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 0 ) ), vectorLayer );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 1 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer->id() ) );
  delete projectScope;
  project.addMapLayer( vectorLayer2 );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 2 );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 0 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 1 ) ) ) ) );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 2 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer2->id() ) );
  delete projectScope;
  project.addMapLayers( QList< QgsMapLayer * >() << vectorLayer3 );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 3 );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 0 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 1 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 2 ) ) ) ) );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 3 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer2->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer3->id() ) );
  delete projectScope;
  project.removeMapLayer( vectorLayer );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 2 );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 0 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast< QgsMapLayer * >( qvariant_cast< QObject * >( projectScope->variable( "layers" ).toList().at( 1 ) ) ) ) );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 2 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer2->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer3->id() ) );
  delete projectScope;
  project.removeMapLayers( QList< QgsMapLayer * >() << vectorLayer2 << vectorLayer3 );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( projectScope->variable( "layers" ).toList().isEmpty() );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().isEmpty() );
  delete projectScope;

  //test clearing project variables
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( projectScope->hasVariable( "test" ) );
  project.clear();
  delete projectScope;
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( !projectScope->hasVariable( "test" ) );

  //test a preset project variable
  project.setTitle( QStringLiteral( "test project" ) );
  delete projectScope;
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "project_title" ).toString(), QString( "test project" ) );
  delete projectScope;

  //test setProjectVariables
  QVariantMap vars;
  vars.insert( QStringLiteral( "newvar1" ), QStringLiteral( "val1" ) );
  vars.insert( QStringLiteral( "newvar2" ), QStringLiteral( "val2" ) );
  QgsExpressionContextUtils::setProjectVariables( &project, vars );
  projectScope = QgsExpressionContextUtils::projectScope( &project );

  QVERIFY( !projectScope->hasVariable( "test" ) );
  QCOMPARE( projectScope->variable( "newvar1" ).toString(), QString( "val1" ) );
  QCOMPARE( projectScope->variable( "newvar2" ).toString(), QString( "val2" ) );
  delete projectScope;

  //test removeProjectVariable
  QgsExpressionContextUtils::setProjectVariable( &project, QStringLiteral( "key" ), "value" );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( projectScope->hasVariable( "key" ) );
  QgsExpressionContextUtils::removeProjectVariable( &project, QStringLiteral( "key" ) );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( !projectScope->hasVariable( "key" ) );
  delete projectScope;
  projectScope = nullptr;

  //test project scope functions

  //project_color function
  QgsProjectColorScheme s;
  QgsNamedColorList colorList;
  colorList << qMakePair( QColor( 200, 255, 0 ), QStringLiteral( "vomit yellow" ) );
  colorList << qMakePair( QColor( 30, 60, 20 ), QStringLiteral( "murky depths of hades" ) );
  s.setColors( colorList );
  QgsExpressionContext contextColors;
  contextColors << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  QgsExpression expProjectColor( QStringLiteral( "project_color('murky depths of hades')" ) );
  QCOMPARE( expProjectColor.evaluate( &contextColors ).toString(), QString( "30,60,20" ) );
  //matching color names should be case insensitive
  QgsExpression expProjectColorCaseInsensitive( QStringLiteral( "project_color('Murky Depths of hades')" ) );
  QCOMPARE( expProjectColorCaseInsensitive.evaluate( &contextColors ).toString(), QString( "30,60,20" ) );
  QgsExpression badProjectColor( QStringLiteral( "project_color('dusk falls in san juan del sur')" ) );
  QCOMPARE( badProjectColor.evaluate( &contextColors ), QVariant() );
}

void TestQgsExpressionContext::layerScope()
{
  //test passing no layer - should be no crash
  QgsExpressionContextScope *layerScope = QgsExpressionContextUtils::layerScope( nullptr );
  QCOMPARE( layerScope->name(), tr( "Layer" ) );
  QCOMPARE( layerScope->variableCount(), 0 );
  delete layerScope;
  layerScope = nullptr;

  //create a map layer
  std::unique_ptr<QgsVectorLayer> vectorLayer( new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "test layer" ), QStringLiteral( "memory" ) ) );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::layerScope( vectorLayer.get() );

  QCOMPARE( context.variable( "layer_name" ).toString(), vectorLayer->name() );
  QCOMPARE( context.variable( "layer_id" ).toString(), vectorLayer->id() );
  QCOMPARE( context.variable( "layer_crs" ).toString(), vectorLayer->sourceCrs().authid() );

  QgsExpression expProject( QStringLiteral( "var('layer_name')" ) );
  QCOMPARE( expProject.evaluate( &context ).toString(), vectorLayer->name() );

  //check that fields were set
  const QgsFields fromVar = qvariant_cast<QgsFields>( context.variable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE( fromVar, vectorLayer->fields() );

  //test setting layer variables
  QgsExpressionContextUtils::setLayerVariable( vectorLayer.get(), QStringLiteral( "testvar" ), "testval" );
  delete layerScope;
  layerScope = QgsExpressionContextUtils::layerScope( vectorLayer.get() );
  QCOMPARE( layerScope->variable( "testvar" ).toString(), QString( "testval" ) );

  QVariantMap variables;
  variables.insert( QStringLiteral( "var1" ), QStringLiteral( "val1" ) );
  variables.insert( QStringLiteral( "var2" ), QStringLiteral( "val2" ) );
  QgsExpressionContextUtils::setLayerVariables( vectorLayer.get(), variables );
  delete layerScope;
  layerScope = QgsExpressionContextUtils::layerScope( vectorLayer.get() );
  QCOMPARE( layerScope->variable( "testvar" ), QVariant() );
  QCOMPARE( layerScope->variable( "var1" ).toString(), QString( "val1" ) );
  QCOMPARE( layerScope->variable( "var2" ).toString(), QString( "val2" ) );
  delete layerScope;
}

void TestQgsExpressionContext::featureBasedContext()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "x1" ) ) );
  fields.append( QgsField( QStringLiteral( "x2" ) ) );
  fields.append( QgsField( QStringLiteral( "foo" ), QVariant::Int ) );

  QgsFeature f;
  f.initAttributes( 3 );
  f.setAttribute( 2, QVariant( 20 ) );

  const QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );

  const QgsFeature evalFeature = context.feature();
  const QgsFields evalFields = qvariant_cast<QgsFields>( context.variable( QStringLiteral( "_fields_" ) ) );
  QCOMPARE( evalFeature.attributes(), f.attributes() );
  QCOMPARE( evalFields, fields );
}

void TestQgsExpressionContext::cache()
{
  //test setting and retrieving cached values
  const QgsExpressionContext context;

  //use a const reference to ensure that cache is usable from const QgsExpressionContexts
  const QgsExpressionContext &c = context;

  QVERIFY( !c.hasCachedValue( "test" ) );
  QVERIFY( !c.cachedValue( "test" ).isValid() );

  c.setCachedValue( QStringLiteral( "test" ), "my value" );
  QVERIFY( c.hasCachedValue( "test" ) );
  QCOMPARE( c.cachedValue( "test" ), QVariant( "my value" ) );

  // copy should copy cache
  const QgsExpressionContext context2( c );
  QVERIFY( context2.hasCachedValue( "test" ) );
  QCOMPARE( context2.cachedValue( "test" ), QVariant( "my value" ) );

  // assignment should copy cache
  QgsExpressionContext context3;
  context3 = c;
  QVERIFY( context3.hasCachedValue( "test" ) );
  QCOMPARE( context3.cachedValue( "test" ), QVariant( "my value" ) );

  // clear cache
  c.clearCachedValues();
  QVERIFY( !c.hasCachedValue( "test" ) );
  QVERIFY( !c.cachedValue( "test" ).isValid() );
}

void TestQgsExpressionContext::valuesAsMap()
{
  QgsExpressionContext context;

  //test retrieving from empty context
  QVERIFY( context.variablesToMap().isEmpty() );

  //add a scope to the context
  QgsExpressionContextScope *s1 = new QgsExpressionContextScope();
  s1->setVariable( QStringLiteral( "v1" ), "t1" );
  s1->setVariable( QStringLiteral( "v2" ), "t2" );
  context << s1;

  QVariantMap m = context.variablesToMap();
  QCOMPARE( m.size(), 2 );
  QCOMPARE( m.value( "v1" ).toString(), QString( "t1" ) );
  QCOMPARE( m.value( "v2" ).toString(), QString( "t2" ) );

  QgsExpressionContextScope *s2 = new QgsExpressionContextScope();
  s2->setVariable( QStringLiteral( "v2" ), "t2a" );
  s2->setVariable( QStringLiteral( "v3" ), "t3" );
  context << s2;

  m = context.variablesToMap();
  QCOMPARE( m.size(), 3 );
  QCOMPARE( m.value( "v1" ).toString(), QString( "t1" ) );
  QCOMPARE( m.value( "v2" ).toString(), QString( "t2a" ) );
  QCOMPARE( m.value( "v3" ).toString(), QString( "t3" ) );
}

void TestQgsExpressionContext::description()
{
  // test that description falls back to default values if not set
  QgsExpressionContext context;
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_title" ) ) );
  context << scope;
  QCOMPARE( context.description( "project_title" ), QgsExpression::variableHelpText( "project_title" ) );
  // but if set, use that
  scope = new QgsExpressionContextScope();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_title" ), QVariant(), true, true, QStringLiteral( "my desc" ) ) );
  context << scope;
  QCOMPARE( context.description( "project_title" ), QStringLiteral( "my desc" ) );
}

void TestQgsExpressionContext::readWriteScope()
{
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  const QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );

  QgsExpressionContextScope s1;
  s1.setVariable( QStringLiteral( "v1" ), "t1" );
  s1.setVariable( QStringLiteral( "v2" ), 55 );
  QDomElement e = doc.createElement( QStringLiteral( "scope_test" ) );
  s1.writeXml( e, doc, QgsReadWriteContext() );
  doc.appendChild( e );

  QgsExpressionContextScope s2;
  QCOMPARE( s2.variableCount(), 0 );

  // invalid xml element
  const QDomElement e2 = doc.createElement( QStringLiteral( "empty" ) );
  s2.readXml( e2, QgsReadWriteContext() );
  QCOMPARE( s2.variableCount(), 0 );

  // valid element
  s2.readXml( e, QgsReadWriteContext() );
  QCOMPARE( s2.variableCount(), 2 );
  QCOMPARE( qgis::listToSet( s2.variableNames() ), QSet< QString >() << QStringLiteral( "v1" ) << QStringLiteral( "v2" ) );
  QCOMPARE( s2.variable( QStringLiteral( "v1" ) ).toString(), QStringLiteral( "t1" ) );
  QCOMPARE( s2.variable( QStringLiteral( "v2" ) ).toInt(), 55 );
}

QGSTEST_MAIN( TestQgsExpressionContext )
#include "testqgsexpressioncontext.moc"
