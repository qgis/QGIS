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
    void staticValueContext(); //test for QgsStaticValueExpressionContext
    void contextStack();
    void evaluate();

    void globalVariables();
    void projectVariables();

  private:

    class TestContext : public QgsExpressionContext
    {
      public:

        TestContext( int value )
            : QgsExpressionContext()
            , mValue( value )
        {
          QgsExpression::registerFunction( new GetTestValueFunction( 0 ) );
          mFunctions.insert( "get_test_value", new GetTestValueFunction( this ) );
        }

      private:

        class GetTestValueFunction : public QgsExpression::Function
        {
          public:
            GetTestValueFunction( TestContext* context )
                : QgsExpression::Function( "get_test_value", 1, "test" ), mContext( context ) {}

            virtual QVariant func( const QVariantList&, const QgsExpressionContext*, QgsExpression* ) override
            {
              mContext->mValue++;
              return mContext->mValue;
            }

          private:
            TestContext* mContext;
        };

        int mValue;
        friend class GetTestValueFunction;
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

void TestQgsExpressionContext::staticValueContext()
{
  QgsStaticValueExpressionContext context;

  QVERIFY( !context.hasVariable( "test" ) );
  QVERIFY( !context.variable( "test" ).isValid() );
  QCOMPARE( context.variableNames().length(), 0 );

  context.setVariable( "test", 5 );
  QVERIFY( context.hasVariable( "test" ) );
  QVERIFY( context.variable( "test" ).isValid() );
  QCOMPARE( context.variable( "test" ).toInt(), 5 );
  QCOMPARE( context.variableNames().length(), 1 );
  QCOMPARE( context.variableNames().at( 0 ), QString( "test" ) );

  context.insertVariable( QgsStaticValueExpressionContext::StaticVariable( "readonly", QString( "readonly_test" ), true ) );
  QVERIFY( context.isReadOnly( "readonly" ) );
  context.insertVariable( QgsStaticValueExpressionContext::StaticVariable( "notreadonly", QString( "not_readonly_test" ), false ) );
  QVERIFY( !context.isReadOnly( "notreadonly" ) );

  //updating a read only variable should remain read only
  context.setVariable( "readonly", "newvalue" );
  QVERIFY( context.isReadOnly( "readonly" ) );
}

void TestQgsExpressionContext::contextStack()
{
  QgsExpressionContextStack stack;
  //test retrieving from empty stack
  QVERIFY( !stack.hasVariable( "test" ) );
  QVERIFY( !stack.variable( "test" ).isValid() );
  QCOMPARE( stack.variableNames().length(), 0 );

  //add a context to the stack
  QgsStaticValueExpressionContext* context1 = new QgsStaticValueExpressionContext();
  stack.appendContext( context1 );
  QVERIFY( !stack.hasVariable( "test" ) );
  QVERIFY( !stack.variable( "test" ).isValid() );
  QCOMPARE( stack.variableNames().length(), 0 );

  //now add a variable to the first context
  context1->setVariable( "test", 1 );
  QVERIFY( stack.hasVariable( "test" ) );
  QCOMPARE( stack.variable( "test" ).toInt(), 1 );
  QCOMPARE( stack.variableNames().length(), 1 );

  //add a second context, should override the first
  QgsStaticValueExpressionContext* context2 = new QgsStaticValueExpressionContext();
  stack.appendContext( context2 );
  //test without setting variable first...
  QVERIFY( stack.hasVariable( "test" ) );
  QCOMPARE( stack.variable( "test" ).toInt(), 1 );
  QCOMPARE( stack.variableNames().length(), 1 );
  //then set the variable so it overrides
  context2->setVariable( "test", 2 );
  QVERIFY( stack.hasVariable( "test" ) );
  QCOMPARE( stack.variable( "test" ).toInt(), 2 );
  QCOMPARE( stack.variableNames().length(), 1 );

  //make sure stack falls back to earlier contexts
  context1->setVariable( "test2", 11 );
  QVERIFY( stack.hasVariable( "test2" ) );
  QCOMPARE( stack.variable( "test2" ).toInt(), 11 );
  QCOMPARE( stack.variableNames().length(), 2 );
}

void TestQgsExpressionContext::evaluate()
{
  QgsExpression exp( "1 + 2" );
  QCOMPARE( exp.evaluate().toInt(), 3 );

  QgsStaticValueExpressionContext context;
  context.setVariable( "test", 5 );
  QCOMPARE( exp.evaluate( &context ).toInt(), 3 );
  QgsExpression expWithVariable( "var('test')" );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 5 );
  context.setVariable( "test", 7 );
  QCOMPARE( expWithVariable.evaluate( &context ).toInt(), 7 );
  QgsExpression expWithVariable2( "var('test') + var('test2')" );
  context.setVariable( "test2", 9 );
  QCOMPARE( expWithVariable2.evaluate( &context ).toInt(), 16 );

  QgsExpression expWithVariableBad( "var('bad')" );
  QVERIFY( !expWithVariableBad.evaluate( &context ).isValid() );

  //test with a function provided by a context
  QgsExpression testExpWContextFunction( "get_test_value(1)" );
  QVERIFY( !testExpWContextFunction.evaluate( ).isValid() );

  TestContext testContext( 0 );
  testExpWContextFunction.prepare( &testContext );
  QCOMPARE( testExpWContextFunction.evaluate( &testContext ).toInt(), 1 );
  QCOMPARE( testExpWContextFunction.evaluate( &testContext ).toInt(), 2 );

  //test with another context to ensure that expressions are evaulated against correct context
  TestContext testContext2( 5 );
  QgsExpression testExpWContextFunction2( "get_test_value(1)" );
  testExpWContextFunction2.prepare( &testContext2 );
  QCOMPARE( testExpWContextFunction2.evaluate( &testContext2 ).toInt(), 6 );
  QCOMPARE( testExpWContextFunction2.evaluate( &testContext2 ).toInt(), 7 );
  QCOMPARE( testExpWContextFunction2.evaluate( &testContext ).toInt(), 3 );
}

void TestQgsExpressionContext::globalVariables()
{
  QgsGlobalExpressionContext* globalContext = new QgsGlobalExpressionContext();
  globalContext->setVariable( "test", "testval" );
  delete globalContext;

  QgsGlobalExpressionContext* globalContext2 = new QgsGlobalExpressionContext();
  QCOMPARE( globalContext2->variable( "test" ).toString(), QString( "testval" ) );

  QgsExpression expGlobal( "var('test')" );
  QCOMPARE( expGlobal.evaluate( globalContext2 ).toString(), QString( "testval" ) );
  delete globalContext2;
}

void TestQgsExpressionContext::projectVariables()
{
  QgsProjectExpressionContext* projectContext = new QgsProjectExpressionContext();
  projectContext->setVariable( "test", "testval" );
  projectContext->setVariable( "testdouble", 5.2 );
  delete projectContext;

  QgsProjectExpressionContext* projectContext2 = new QgsProjectExpressionContext();
  QCOMPARE( projectContext2->variable( "test" ).toString(), QString( "testval" ) );
  QCOMPARE( projectContext2->variable( "testdouble" ).toDouble(), 5.2 );

  QgsExpression expProject( "var('test')" );
  QCOMPARE( expProject.evaluate( projectContext2 ).toString(), QString( "testval" ) );

  //check that project expression context stack consists of both project and global variables
  QgsGlobalExpressionContext* globalContext = QgsGlobalExpressionContext::instance();
  globalContext->setVariable( "stackvar", "global" );
  QCOMPARE( QgsProject::instance()->expressionContextStack()->variable( "stackvar" ).toString(), QString( "global" ) );
  QgsProject::instance()->projectExpressionContext()->setVariable( "stackvar", "project" );
  //'stackvar' variable should be overwritten with project expression context's value
  QCOMPARE( QgsProject::instance()->expressionContextStack()->variable( "stackvar" ).toString(), QString( "project" ) );

  //test clearing project variables
  projectContext2->setVariable( "project_var", "val" );
  projectContext2->clear();
  QVERIFY( !projectContext2->hasVariable( "project_var" ) );

  QgsProject::instance()->projectExpressionContext()->setVariable( "project_var", "val" );
  QVERIFY( QgsProject::instance()->projectExpressionContext()->hasVariable( "project_var" ) );
  QgsProject::instance()->clear();
  QVERIFY( !QgsProject::instance()->projectExpressionContext()->hasVariable( "project_var" ) );
}

QTEST_MAIN( TestQgsExpressionContext )
#include "testqgsexpressioncontext.moc"
