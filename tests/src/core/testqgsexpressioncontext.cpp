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

#include "qgsapplication.h"
#include "qgscolorscheme.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaplayerstore.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QObject>

class TestQgsExpressionContext : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void contextScope();
    void contextScopeCopy();
    void contextScopeFunctions();
    void contextStack();
    void scopeByName();
    void contextCopy();
    void contextMove();
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
    void layerStores();
    void uniqueHash();

  private:
    class GetTestValueFunction : public QgsScopedExpressionFunction
    {
      public:
        GetTestValueFunction()
          : QgsScopedExpressionFunction( u"get_test_value"_s, 1, u"test"_s ) {}

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
          : QgsScopedExpressionFunction( u"get_test_value"_s, 1, u"test"_s ) {}

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
          : QgsScopedExpressionFunction( u"test_function"_s, 1, u"test"_s )
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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
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
  QgsExpressionContextScope scope( u"scope name"_s );
  QCOMPARE( scope.name(), QString( "scope name" ) );

  QVERIFY( !scope.hasVariable( "test" ) );
  QVERIFY( !scope.variable( "test" ).isValid() );
  QCOMPARE( scope.variableNames().length(), 0 );
  QCOMPARE( scope.variableCount(), 0 );
  QVERIFY( scope.description( "test" ).isEmpty() );

  scope.setVariable( u"test"_s, 5 );
  QVERIFY( scope.hasVariable( "test" ) );
  QVERIFY( scope.variable( "test" ).isValid() );
  QCOMPARE( scope.variable( "test" ).toInt(), 5 );
  QCOMPARE( scope.variableNames().length(), 1 );
  QCOMPARE( scope.variableCount(), 1 );
  QCOMPARE( scope.variableNames().at( 0 ), QString( "test" ) );
  QVERIFY( scope.description( "test" ).isEmpty() );

  scope.addVariable( QgsExpressionContextScope::StaticVariable( u"readonly"_s, u"readonly_test"_s, true, false, u"readonly variable"_s ) );
  QVERIFY( scope.isReadOnly( "readonly" ) );
  QCOMPARE( scope.description( "readonly" ), u"readonly variable"_s );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( u"notreadonly"_s, u"not_readonly_test"_s, false ) );
  QVERIFY( !scope.isReadOnly( "notreadonly" ) );

  //updating a read only variable should remain read only
  scope.setVariable( u"readonly"_s, "newvalue" );
  QVERIFY( scope.isReadOnly( "readonly" ) );
  // and keep its description
  QCOMPARE( scope.description( "readonly" ), u"readonly variable"_s );

  //test retrieving filtered variable names
  scope.setVariable( u"_hidden_"_s, "hidden" );
  QCOMPARE( scope.filteredVariableNames(), QStringList() << "readonly" << "notreadonly" << "test" );

  //removal
  scope.setVariable( u"toremove"_s, 5 );
  QVERIFY( scope.hasVariable( "toremove" ) );
  QVERIFY( !scope.removeVariable( "missing" ) );
  QVERIFY( scope.removeVariable( "toremove" ) );
  QVERIFY( !scope.hasVariable( "toremove" ) );

  // checks for variables visibility updates
  QCOMPARE( scope.hiddenVariables().length(), 0 );

  scope.addHiddenVariable( u"visibilitytest"_s );
  QCOMPARE( scope.hiddenVariables().length(), 1 );
  QCOMPARE( scope.hiddenVariables().at( 0 ), u"visibilitytest"_s );

  scope.removeHiddenVariable( u"visibilitytest"_s );
  QCOMPARE( scope.hiddenVariables().length(), 0 );

  QStringList hiddenVariables;
  hiddenVariables << u"visibilitytest1"_s;
  hiddenVariables << u"visibilitytest2"_s;
  hiddenVariables << u"visibilitytest3"_s;

  scope.setHiddenVariables( hiddenVariables );

  QCOMPARE( scope.hiddenVariables().length(), 3 );
}

void TestQgsExpressionContext::contextScopeCopy()
{
  QgsExpressionContextScope scope( u"scope name"_s );
  scope.setVariable( u"test"_s, 5 );
  scope.addFunction( u"get_test_value"_s, new GetTestValueFunction() );

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

  scope.addFunction( u"get_test_value"_s, new GetTestValueFunction() );
  QVERIFY( scope.hasFunction( "get_test_value" ) );
  QVERIFY( scope.function( "get_test_value" ) );
  const QgsExpressionContext temp;
  QCOMPARE( scope.function( "get_test_value" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 42 );

  //test functionNames
  scope.addFunction( u"get_test_value2"_s, new GetTestValueFunction() );
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
  scope1->setVariable( u"test"_s, 1 );
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
  scope2->setVariable( u"test"_s, 2 );
  QVERIFY( context.hasVariable( "test" ) );
  QCOMPARE( context.variable( "test" ).toInt(), 2 );
  QCOMPARE( context.variableNames().length(), 1 );

  //make sure context falls back to earlier scopes
  scope1->setVariable( u"test2"_s, 11 );
  QVERIFY( context.hasVariable( "test2" ) );
  QCOMPARE( context.variable( "test2" ).toInt(), 11 );
  QCOMPARE( context.variableNames().length(), 2 );

  //check filteredVariableNames method
  scope2->setVariable( u"_hidden"_s, 5 );
  const QStringList filteredNames = context.filteredVariableNames();
  QCOMPARE( filteredNames.count(), 2 );
  QCOMPARE( filteredNames.at( 0 ), QString( "test" ) );
  QCOMPARE( filteredNames.at( 1 ), QString( "test2" ) );

  //test scopes method
  const QList<QgsExpressionContextScope *> scopes = context.scopes();
  QCOMPARE( scopes.length(), 2 );
  QCOMPARE( scopes.at( 0 ), scope1 );
  QCOMPARE( scopes.at( 1 ), scope2 );

  QVERIFY( context.description( "readonly" ).isEmpty() );

  //check isReadOnly
  scope2->addVariable( QgsExpressionContextScope::StaticVariable( u"readonly"_s, 5, true, false, u"readonly variable"_s ) );
  QVERIFY( context.isReadOnly( "readonly" ) );
  QVERIFY( !context.isReadOnly( "test" ) );
  QCOMPARE( context.description( "readonly" ), u"readonly variable"_s );

  // Check scopes can be popped
  delete context.popScope();
  QCOMPARE( scopes.length(), 2 );
  QCOMPARE( scopes.at( 0 ), scope1 );
}

void TestQgsExpressionContext::scopeByName()
{
  QgsExpressionContext context;
  QCOMPARE( context.indexOfScope( "test1" ), -1 );
  context << new QgsExpressionContextScope( u"test1"_s );
  context << new QgsExpressionContextScope( u"test2"_s );
  QCOMPARE( context.indexOfScope( "test1" ), 0 );
  QCOMPARE( context.indexOfScope( "test2" ), 1 );
  QCOMPARE( context.indexOfScope( "not in context" ), -1 );
}

void TestQgsExpressionContext::contextCopy()
{
  QgsExpressionContext context;
  context << new QgsExpressionContextScope();
  context.scope( 0 )->setVariable( u"test"_s, 1 );

  //copy constructor
  const QgsExpressionContext copy( context );
  QCOMPARE( copy.scopeCount(), 1 );
  QVERIFY( copy.hasVariable( "test" ) );
  QCOMPARE( copy.variable( "test" ).toInt(), 1 );
  QCOMPARE( copy.variableNames().length(), 1 );
}

void TestQgsExpressionContext::contextMove()
{
  QgsExpressionContext context;
  context << new QgsExpressionContextScope();
  context.scope( 0 )->setVariable( u"test"_s, 1 );

  // NOLINTBEGIN(bugprone-use-after-move)
  // NOLINTBEGIN(clang-analyzer-cplusplus.Move)
  //move constructor
  QgsExpressionContext newContext( std::move( context ) );
  QCOMPARE( newContext.scopeCount(), 1 );
  QCOMPARE( context.scopeCount(), 0 );
  QVERIFY( newContext.hasVariable( "test" ) );
  QCOMPARE( newContext.variable( "test" ).toInt(), 1 );
  QCOMPARE( newContext.variableNames().length(), 1 );

  QgsExpressionContext newContext2;
  newContext2 = std::move( newContext );
  QCOMPARE( newContext2.scopeCount(), 1 );
  QCOMPARE( newContext.scopeCount(), 0 );
  QVERIFY( newContext2.hasVariable( "test" ) );
  QCOMPARE( newContext2.variable( "test" ).toInt(), 1 );
  QCOMPARE( newContext2.variableNames().length(), 1 );
  // NOLINTEND(clang-analyzer-cplusplus.Move)
  // NOLINTEND(bugprone-use-after-move)
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
  scope1->addFunction( u"get_test_value"_s, new GetTestValueFunction() );
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
  scope2->addFunction( u"get_test_value"_s, new GetTestValueFunction2() );
  QVERIFY( context.hasFunction( "get_test_value" ) );
  QVERIFY( context.function( "get_test_value" ) );
  QCOMPARE( context.function( "get_test_value" )->func( QVariantList(), &temp, nullptr, nullptr ).toInt(), 43 );

  //make sure stack falls back to earlier contexts
  scope2->addFunction( u"get_test_value2"_s, new GetTestValueFunction() );
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
  QgsExpression exp( u"1 + 2"_s );
  QCOMPARE( exp.evaluate().toInt(), 3 );

  QgsExpressionContext context;
  context << new QgsExpressionContextScope();

  QgsExpressionContextScope *s = context.scope( 0 );
  s->setVariable( u"test"_s, 5 );
  QCOMPARE( exp.evaluate( &context ).toInt(), 3 );
  QgsExpression expWithVariable( u"var('test')"_s );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 5 );
  s->setVariable( u"test"_s, 7 );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 7 );
  QgsExpression expWithVariable2( u"var('test') + var('test2')"_s );
  s->setVariable( u"test2"_s, 9 );
  QCOMPARE( expWithVariable2.evaluate( &context ).toInt(), 16 );

  QgsExpression expWithVariableBad( u"var('bad')"_s );
  QVERIFY( !expWithVariableBad.evaluate( &context ).isValid() );

  //test shorthand variables
  QgsExpression expShorthand( u"@test"_s );
  QCOMPARE( expShorthand.evaluate( &context ).toInt(), 7 );
  QgsExpression expShorthandBad( u"@bad"_s );
  QVERIFY( !expShorthandBad.evaluate( &context ).isValid() );

  //test with a function provided by a context
  QgsExpression::registerFunction( new ModifiableFunction( nullptr ), true );
  QgsExpression testExpWContextFunction( u"test_function(1)"_s );
  QVERIFY( !testExpWContextFunction.evaluate().isValid() );

  int val1 = 5;
  s->addFunction( u"test_function"_s, new ModifiableFunction( &val1 ) );
  testExpWContextFunction.prepare( &context );
  QCOMPARE( testExpWContextFunction.evaluate( &context ).toInt(), 6 );
  QCOMPARE( testExpWContextFunction.evaluate( &context ).toInt(), 7 );
  QCOMPARE( val1, 7 );

  //test with another context to ensure that expressions are evaluated against correct context
  QgsExpressionContext context2;
  context2 << new QgsExpressionContextScope();
  QgsExpressionContextScope *s2 = context2.scope( 0 );
  int val2 = 50;
  s2->addFunction( u"test_function"_s, new ModifiableFunction( &val2 ) );
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
  QCOMPARE( scope.geometry().asWkt(), u"Point (1 2)"_s );
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
  QCOMPARE( emptyContext.geometry().asWkt(), u"Point (1 2)"_s );

  QgsExpressionContext contextWithScope;
  contextWithScope << new QgsExpressionContextScope();
  contextWithScope.setGeometry( g );
  QCOMPARE( contextWithScope.scopeCount(), 1 );
  QVERIFY( contextWithScope.hasGeometry() );
  QCOMPARE( contextWithScope.geometry().asWkt(), u"Point (1 2)"_s );
}

void TestQgsExpressionContext::setFields()
{
  QgsFields fields;
  const QgsField field( u"testfield"_s );
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
  QgsExpressionContextUtils::setGlobalVariable( u"test_global"_s, "testval" );

  QgsProject *project = QgsProject::instance();
  QgsExpressionContextUtils::setProjectVariable( project, u"test_project"_s, "testval" );

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
  QVERIFY( !context.isHighlightedFunction( u"x"_s ) );
  QVERIFY( !context.isHighlightedVariable( u"x"_s ) );
  QVERIFY( context.highlightedVariables().isEmpty() );
  context.setHighlightedFunctions( QStringList() << u"x"_s << u"y"_s );
  QVERIFY( context.isHighlightedFunction( u"x"_s ) );
  QVERIFY( context.isHighlightedFunction( u"y"_s ) );
  QVERIFY( !context.isHighlightedFunction( u"z"_s ) );
  QVERIFY( !context.isHighlightedVariable( u"x"_s ) );
  context.setHighlightedVariables( QStringList() << u"a"_s << u"b"_s );
  QCOMPARE( context.highlightedVariables(), QStringList() << u"a"_s << u"b"_s );
  QVERIFY( context.isHighlightedVariable( u"a"_s ) );
  QVERIFY( context.isHighlightedVariable( u"b"_s ) );
  QVERIFY( !context.isHighlightedVariable( u"c"_s ) );
  QVERIFY( !context.isHighlightedFunction( u"a"_s ) );
  context.setHighlightedFunctions( QStringList() );
  context.setHighlightedVariables( QStringList() );
  QVERIFY( !context.isHighlightedFunction( u"x"_s ) );
  QVERIFY( !context.isHighlightedVariable( u"a"_s ) );
  QVERIFY( context.highlightedVariables().isEmpty() );
}

void TestQgsExpressionContext::globalScope()
{
  QgsExpressionContextUtils::setGlobalVariable( u"test"_s, "testval" );

  QgsExpressionContext context;
  QgsExpressionContextScope *globalScope = QgsExpressionContextUtils::globalScope();
  context << globalScope;
  QCOMPARE( globalScope->name(), tr( "Global" ) );

  QCOMPARE( context.variable( "test" ).toString(), QString( "testval" ) );

  QgsExpression expGlobal( u"var('test')"_s );
  QCOMPARE( expGlobal.evaluate( &context ).toString(), QString( "testval" ) );

  //test some other recognized global variables
  QgsExpression expVersion( u"var('qgis_version')"_s );
  QgsExpression expVersionNo( u"var('qgis_version_no')"_s );
  QgsExpression expReleaseName( u"var('qgis_release_name')"_s );
  QgsExpression expAccountName( u"var('user_account_name')"_s );
  QgsExpression expUserFullName( u"var('user_full_name')"_s );
  QgsExpression expOsName( u"var('qgis_os_name')"_s );
  QgsExpression expPlatform( u"var('qgis_platform')"_s );

  QCOMPARE( expVersion.evaluate( &context ).toString(), Qgis::version() );
  QCOMPARE( expVersionNo.evaluate( &context ).toInt(), Qgis::versionInt() );
  QCOMPARE( expReleaseName.evaluate( &context ).toString(), Qgis::releaseName() );
  QCOMPARE( expAccountName.evaluate( &context ).toString(), QgsApplication::userLoginName() );
  QCOMPARE( expUserFullName.evaluate( &context ).toString(), QgsApplication::userFullName() );
  QCOMPARE( expOsName.evaluate( &context ).toString(), QgsApplication::osName() );
  QCOMPARE( expPlatform.evaluate( &context ).toString(), QgsApplication::platform() );

  //test setGlobalVariables
  QVariantMap vars;
  vars.insert( u"newvar1"_s, u"val1"_s );
  vars.insert( u"newvar2"_s, u"val2"_s );
  QgsExpressionContextUtils::setGlobalVariables( vars );
  QgsExpressionContextScope *globalScope2 = QgsExpressionContextUtils::globalScope();

  QVERIFY( !globalScope2->hasVariable( "test" ) );
  QCOMPARE( globalScope2->variable( "newvar1" ).toString(), QString( "val1" ) );
  QCOMPARE( globalScope2->variable( "newvar2" ).toString(), QString( "val2" ) );

  delete globalScope2;

  //test removeGlobalVariables
  QgsExpressionContextUtils::setGlobalVariable( u"key"_s, "value" );
  std::unique_ptr<QgsExpressionContextScope> globalScope3( QgsExpressionContextUtils::globalScope() );
  QVERIFY( globalScope3->hasVariable( "key" ) );
  QgsExpressionContextUtils::removeGlobalVariable( u"key"_s );
  globalScope3.reset( QgsExpressionContextUtils::globalScope() );
  QVERIFY( !globalScope3->hasVariable( "key" ) );
}

void TestQgsExpressionContext::projectScope()
{
  QgsProject project;
  QgsProjectMetadata md;
  md.setTitle( u"project title"_s );
  md.setAuthor( u"project author"_s );
  md.setAbstract( u"project abstract"_s );
  md.setCreationDateTime( QDateTime( QDate( 2011, 3, 5 ), QTime( 9, 5, 4 ) ) );
  md.setIdentifier( u"project identifier"_s );
  QgsAbstractMetadataBase::KeywordMap keywords;
  keywords.insert( u"voc1"_s, QStringList() << "a" << "b" );
  keywords.insert( u"voc2"_s, QStringList() << "c" << "d" );
  md.setKeywords( keywords );
  project.setMetadata( md );

  QgsExpressionContextUtils::setProjectVariable( &project, u"test"_s, "testval" );
  QgsExpressionContextUtils::setProjectVariable( &project, u"testdouble"_s, 5.2 );

  QgsExpressionContext context;
  QgsExpressionContextScope *scope = QgsExpressionContextUtils::projectScope( &project );
  context << scope;
  QCOMPARE( scope->name(), tr( "Project" ) );

  // metadata variables
  QCOMPARE( context.variable( "project_title" ).toString(), u"project title"_s );
  QCOMPARE( context.variable( "project_author" ).toString(), u"project author"_s );
  QCOMPARE( context.variable( "project_abstract" ).toString(), u"project abstract"_s );
  QCOMPARE( context.variable( "project_creation_date" ).toDateTime(), QDateTime( QDate( 2011, 3, 5 ), QTime( 9, 5, 4 ) ) );
  QCOMPARE( context.variable( "project_identifier" ).toString(), u"project identifier"_s );
  QVariantMap keywordsExpected;
  keywordsExpected.insert( u"voc1"_s, QStringList() << "a" << "b" );
  keywordsExpected.insert( u"voc2"_s, QStringList() << "c" << "d" );
  const QVariantMap keywordsResult = context.variable( "project_keywords" ).toMap();
  QCOMPARE( keywordsResult, keywordsExpected );

  QCOMPARE( context.variable( "test" ).toString(), QString( "testval" ) );
  QCOMPARE( context.variable( "testdouble" ).toDouble(), 5.2 );

  QgsExpression expProject( u"var('test')"_s );
  QCOMPARE( expProject.evaluate( &context ).toString(), QString( "testval" ) );

  // layers
  QVERIFY( context.variable( "layers" ).isValid() );
  QVERIFY( context.variable( "layer_ids" ).isValid() );
  QVERIFY( context.variable( "layers" ).toList().isEmpty() );
  QVERIFY( context.variable( "layer_ids" ).toList().isEmpty() );

  // add layer
  QgsVectorLayer *vectorLayer = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:integer&field=col3:integer"_s, u"test layer"_s, u"memory"_s );
  QgsVectorLayer *vectorLayer2 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:integer&field=col3:integer"_s, u"test layer"_s, u"memory"_s );
  QgsVectorLayer *vectorLayer3 = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:integer&field=col3:integer"_s, u"test layer"_s, u"memory"_s );
  project.addMapLayer( vectorLayer );
  QgsExpressionContextScope *projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 1 );
  QCOMPARE( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 0 ) ), vectorLayer );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 1 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer->id() ) );
  delete projectScope;
  project.addMapLayer( vectorLayer2 );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 2 );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 0 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 1 ) ) ) ) );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 2 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer2->id() ) );
  delete projectScope;
  project.addMapLayers( QList<QgsMapLayer *>() << vectorLayer3 );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 3 );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 0 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 1 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 2 ) ) ) ) );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 3 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer2->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer3->id() ) );
  delete projectScope;
  project.removeMapLayer( vectorLayer );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "layers" ).toList().size(), 2 );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 0 ) ) ) ) );
  QVERIFY( project.mapLayers().values().contains( qobject_cast<QgsMapLayer *>( qvariant_cast<QObject *>( projectScope->variable( "layers" ).toList().at( 1 ) ) ) ) );
  QCOMPARE( projectScope->variable( "layer_ids" ).toList().size(), 2 );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer2->id() ) );
  QVERIFY( projectScope->variable( "layer_ids" ).toList().contains( vectorLayer3->id() ) );
  delete projectScope;
  project.removeMapLayers( QList<QgsMapLayer *>() << vectorLayer2 << vectorLayer3 );
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
  project.setTitle( u"test project"_s );
  delete projectScope;
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QCOMPARE( projectScope->variable( "project_title" ).toString(), QString( "test project" ) );
  delete projectScope;

  //test setProjectVariables
  QVariantMap vars;
  vars.insert( u"newvar1"_s, u"val1"_s );
  vars.insert( u"newvar2"_s, u"val2"_s );
  QgsExpressionContextUtils::setProjectVariables( &project, vars );
  projectScope = QgsExpressionContextUtils::projectScope( &project );

  QVERIFY( !projectScope->hasVariable( "test" ) );
  QCOMPARE( projectScope->variable( "newvar1" ).toString(), QString( "val1" ) );
  QCOMPARE( projectScope->variable( "newvar2" ).toString(), QString( "val2" ) );
  delete projectScope;

  //test removeProjectVariable
  QgsExpressionContextUtils::setProjectVariable( &project, u"key"_s, "value" );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( projectScope->hasVariable( "key" ) );
  QgsExpressionContextUtils::removeProjectVariable( &project, u"key"_s );
  projectScope = QgsExpressionContextUtils::projectScope( &project );
  QVERIFY( !projectScope->hasVariable( "key" ) );
  delete projectScope;
  projectScope = nullptr;

  //test project scope functions

  //project_color function
  QgsProjectColorScheme s;
  QgsNamedColorList colorList;
  colorList << qMakePair( QColor( 200, 255, 0 ), u"vomit yellow"_s );
  colorList << qMakePair( QColor( 30, 60, 20 ), u"murky depths of hades"_s );
  colorList << qMakePair( QColor::fromCmykF( 1., 0.9, 0.8, 0.7 ), u"cmyk colors"_s );
  s.setColors( colorList );
  QgsExpressionContext contextColors;
  contextColors << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  QgsExpression expProjectColor( u"project_color('murky depths of hades')"_s );
  QCOMPARE( expProjectColor.evaluate( &contextColors ).toString(), QString( "30,60,20" ) );
  //matching color names should be case insensitive
  QgsExpression expProjectColorCaseInsensitive( u"project_color('Murky Depths of hades')"_s );
  QCOMPARE( expProjectColorCaseInsensitive.evaluate( &contextColors ).toString(), QString( "30,60,20" ) );
  QgsExpression badProjectColor( u"project_color('dusk falls in san juan del sur')"_s );
  QCOMPARE( badProjectColor.evaluate( &contextColors ), QVariant() );

  QgsExpression expProjectColorObject( u"project_color_object('murky depths of hades')"_s );
  QCOMPARE( expProjectColorObject.evaluate( &contextColors ), QVariant( QColor::fromRgb( 30, 60, 20 ) ) );
  //matching color names should be case insensitive
  QgsExpression expProjectColorObjectCaseInsensitive( u"project_color_object('Murky Depths of hades')"_s );
  QCOMPARE( expProjectColorObjectCaseInsensitive.evaluate( &contextColors ), QVariant( QColor::fromRgb( 30, 60, 20 ) ) );
  QgsExpression expProjectColorCmyk( u"project_color_object('cmyk colors')"_s );
  QCOMPARE( expProjectColorCmyk.evaluate( &contextColors ), QVariant( QColor::fromCmykF( 1., 0.9, 0.8, 0.7 ) ) );
  QgsExpression badProjectColorObject( u"project_color_object('dusk falls in san juan del sur')"_s );
  QCOMPARE( badProjectColorObject.evaluate( &contextColors ), QVariant() );
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
  auto vectorLayer = std::make_unique<QgsVectorLayer>( u"Point?field=col1:integer&field=col2:integer&field=col3:integer"_s, u"test layer"_s, u"memory"_s );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::layerScope( vectorLayer.get() );

  QCOMPARE( context.variable( "layer_name" ).toString(), vectorLayer->name() );
  QCOMPARE( context.variable( "layer_id" ).toString(), vectorLayer->id() );
  QCOMPARE( context.variable( "layer_crs" ).toString(), vectorLayer->sourceCrs().authid() );
  QCOMPARE( context.variable( "layer_crs_ellipsoid" ).toString(), vectorLayer->sourceCrs().ellipsoidAcronym() );

  QgsExpression expProject( u"var('layer_name')"_s );
  QCOMPARE( expProject.evaluate( &context ).toString(), vectorLayer->name() );

  //check that fields were set
  const QgsFields fromVar = qvariant_cast<QgsFields>( context.variable( QgsExpressionContext::EXPR_FIELDS ) );
  QCOMPARE( fromVar, vectorLayer->fields() );

  //test setting layer variables
  QgsExpressionContextUtils::setLayerVariable( vectorLayer.get(), u"testvar1"_s, "testval1" );
  QgsExpressionContextUtils::setLayerVariable( vectorLayer.get(), u"testvar2"_s, "testval2" );
  QgsExpressionContextUtils::setLayerVariable( vectorLayer.get(), u"testvar2"_s, "testval2override" );
  delete layerScope;
  layerScope = QgsExpressionContextUtils::layerScope( vectorLayer.get() );
  QCOMPARE( layerScope->variable( "testvar1" ).toString(), QString( "testval1" ) );

  // Check that values were correctly overridden, with no duplicated entries
  QCOMPARE( layerScope->variable( "testvar2" ).toString(), QString( "testval2override" ) );
  QCOMPARE( vectorLayer->customProperty( u"variableNames"_s ).toStringList(), QStringList() << "testvar1" << "testvar2" );
  QCOMPARE( vectorLayer->customProperty( u"variableValues"_s ).toStringList(), QStringList() << "testval1" << "testval2override" );

  QVariantMap variables;
  variables.insert( u"var1"_s, u"val1"_s );
  variables.insert( u"var2"_s, u"val2"_s );
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
  fields.append( QgsField( u"x1"_s ) );
  fields.append( QgsField( u"x2"_s ) );
  fields.append( QgsField( u"foo"_s, QMetaType::Type::Int ) );

  QgsFeature f;
  f.initAttributes( 3 );
  f.setAttribute( 2, QVariant( 20 ) );

  const QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f, fields );

  const QgsFeature evalFeature = context.feature();
  const QgsFields evalFields = qvariant_cast<QgsFields>( context.variable( u"_fields_"_s ) );
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

  c.setCachedValue( u"test"_s, "my value" );
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
  s1->setVariable( u"v1"_s, "t1" );
  s1->setVariable( u"v2"_s, "t2" );
  context << s1;

  QVariantMap m = context.variablesToMap();
  QCOMPARE( m.size(), 2 );
  QCOMPARE( m.value( "v1" ).toString(), QString( "t1" ) );
  QCOMPARE( m.value( "v2" ).toString(), QString( "t2" ) );

  QgsExpressionContextScope *s2 = new QgsExpressionContextScope();
  s2->setVariable( u"v2"_s, "t2a" );
  s2->setVariable( u"v3"_s, "t3" );
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
  scope->addVariable( QgsExpressionContextScope::StaticVariable( u"project_title"_s ) );
  context << scope;
  QCOMPARE( context.description( "project_title" ), QgsExpression::variableHelpText( "project_title" ) );
  // but if set, use that
  scope = new QgsExpressionContextScope();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( u"project_title"_s, QVariant(), true, true, u"my desc"_s ) );
  context << scope;
  QCOMPARE( context.description( "project_title" ), u"my desc"_s );
}

void TestQgsExpressionContext::readWriteScope()
{
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );
  const QDomElement rootNode = doc.createElement( u"qgis"_s );

  QgsExpressionContextScope s1;
  s1.setVariable( u"v1"_s, "t1" );
  s1.setVariable( u"v2"_s, 55 );
  QDomElement e = doc.createElement( u"scope_test"_s );
  s1.writeXml( e, doc, QgsReadWriteContext() );
  doc.appendChild( e );

  QgsExpressionContextScope s2;
  QCOMPARE( s2.variableCount(), 0 );

  // invalid xml element
  const QDomElement e2 = doc.createElement( u"empty"_s );
  s2.readXml( e2, QgsReadWriteContext() );
  QCOMPARE( s2.variableCount(), 0 );

  // valid element
  s2.readXml( e, QgsReadWriteContext() );
  QCOMPARE( s2.variableCount(), 2 );
  QCOMPARE( qgis::listToSet( s2.variableNames() ), QSet<QString>() << u"v1"_s << u"v2"_s );
  QCOMPARE( s2.variable( u"v1"_s ).toString(), u"t1"_s );
  QCOMPARE( s2.variable( u"v2"_s ).toInt(), 55 );
}

void TestQgsExpressionContext::layerStores()
{
  QgsExpressionContextScope *scope1 = new QgsExpressionContextScope();
  QVERIFY( scope1->layerStores().isEmpty() );

  QgsMapLayerStore store1;
  auto store2 = std::make_unique<QgsMapLayerStore>();
  scope1->addLayerStore( &store1 );
  scope1->addLayerStore( store2.get() );
  QCOMPARE( scope1->layerStores(), QList<QgsMapLayerStore *>( { &store1, store2.get() } ) );

  QgsExpressionContextScope *scope3 = new QgsExpressionContextScope();
  QgsMapLayerStore store3;
  scope3->addLayerStore( &store3 );

  QCOMPARE( scope3->layerStores(), QList<QgsMapLayerStore *>( { &store3 } ) );

  QgsExpressionContext context;
  QVERIFY( context.layerStores().isEmpty() );
  QVERIFY( !context.loadedLayerStore() );

  QgsExpressionContextScope *scope2 = new QgsExpressionContextScope();
  context.appendScopes( { scope1, scope2, scope3 } );

  // stores from scope 3 should take precedence
  QCOMPARE( context.layerStores(), QList<QgsMapLayerStore *>( { &store3, &store1, store2.get() } ) );

  store2.reset();
  QCOMPARE( context.layerStores(), QList<QgsMapLayerStore *>( { &store3, &store1 } ) );

  QVERIFY( !context.loadedLayerStore() );
  QgsMapLayerStore store4;
  context.setLoadedLayerStore( &store4 );
  QCOMPARE( context.loadedLayerStore(), &store4 );
  // store4 must also be present in layerStores()
  QCOMPARE( context.layerStores(), QList<QgsMapLayerStore *>( { &store3, &store1, &store4 } ) );

  QgsExpressionContext c2( context );
  QCOMPARE( c2.loadedLayerStore(), &store4 );

  QgsExpressionContext c3;
  c3 = context;
  QCOMPARE( c3.loadedLayerStore(), &store4 );
}

void TestQgsExpressionContext::uniqueHash()
{
  QgsExpressionContext context;
  bool ok = true;
  // the actual hash values aren't important, just that they are unique. Feel free to change the expected results accordingly
  QSet<QString> vars;
  vars.insert( u"var1"_s );
  vars.insert( u"var2"_s );
  QCOMPARE( context.uniqueHash( ok, vars ), u"var1=||~~||var2=||~~||"_s );
  QVERIFY( ok );

  QgsExpressionContextScope *scope1 = new QgsExpressionContextScope();
  context.appendScope( scope1 );
  scope1->setVariable( u"var1"_s, u"a string"_s );
  scope1->setVariable( u"var2"_s, 5 );
  QCOMPARE( context.uniqueHash( ok, vars ), u"var1=a string||~~||var2=5||~~||"_s );
  QVERIFY( ok );

  QgsExpressionContextScope *scope2 = new QgsExpressionContextScope();
  context.appendScope( scope2 );
  scope2->setVariable( u"var1"_s, u"b string"_s );
  QCOMPARE( context.uniqueHash( ok, vars ), u"var1=b string||~~||var2=5||~~||"_s );
  QVERIFY( ok );

  QgsFeature feature;
  feature.setId( 11 );
  feature.setAttributes( QgsAttributes() << 5 << 11 );
  context.setFeature( feature );

  QCOMPARE( context.uniqueHash( ok, vars ), u"11||~~||18646899||~~||var1=b string||~~||var2=5||~~||"_s );
  QVERIFY( ok );

  // a value which can't be converted to string
  scope2->setVariable( u"var1"_s, QVariant::fromValue( QgsCoordinateReferenceSystem( "EPSG:3857" ) ) );
  context.uniqueHash( ok, vars );
  QVERIFY( !ok );
}

QGSTEST_MAIN( TestQgsExpressionContext )
#include "testqgsexpressioncontext.moc"
