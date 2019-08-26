/***************************************************************************
     testqgsstoredexpressionmanager.cpp
     --------------------------------------
    Date                 : August 2019
    Copyright            : (C) 2019 David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include "qgstest.h"
#include "qgsstoredexpressionmanager.h"

class TestQgsStoredExpressionManager : public QObject
{
    Q_OBJECT
  private:
    QgsStoredExpressionManager *mManager = nullptr;

  private slots:
    void init();
    void cleanup();

    void storeSingleExpression();
    void storeListOfExpressions();
    void editExpressionsByExpression();
    void deleteExpressionByExpression();
};

void TestQgsStoredExpressionManager::init()
{
  mManager = new QgsStoredExpressionManager();

  QList <QgsStoredExpression> newStoredExpressions;

  //fill up
  for ( int i = 0; i < 10; i++ )
  {
    QgsStoredExpression storedExpression( QStringLiteral( "test%1" ).arg( i ), QStringLiteral( "\"age\"=%1" ).arg( i ) );
    newStoredExpressions.append( storedExpression );
  }
  mManager->addStoredExpressions( newStoredExpressions );
}

void TestQgsStoredExpressionManager::cleanup()
{
  delete mManager;
}

void TestQgsStoredExpressionManager::storeSingleExpression()
{
  QString name = QStringLiteral( "test0" );
  QString expression = QStringLiteral( "\"age\"=0" );
  QString id = mManager->addStoredExpression( name, expression );

  //get stored expression by id
  QgsStoredExpression storedExpression = mManager->storedExpression( id );
  QCOMPARE( storedExpression.id, id );
  QCOMPARE( storedExpression.name, name );
  QCOMPARE( storedExpression.expression, expression );

  //get all expressions
  QList <QgsStoredExpression> allStoredExpressions = mManager->storedExpressions();
  QCOMPARE( allStoredExpressions.count(), 11 );
  QCOMPARE( allStoredExpressions.at( 10 ).id, id );
  QCOMPARE( allStoredExpressions.at( 10 ).name, name );
  QCOMPARE( allStoredExpressions.at( 10 ).expression, expression );
}

void TestQgsStoredExpressionManager::storeListOfExpressions()
{
  QList <QgsStoredExpression> newStoredExpressions;

  //fill up
  for ( int i = 10; i < 20; i++ )
  {
    QgsStoredExpression storedExpression( QStringLiteral( "test%1" ).arg( i ), QStringLiteral( "\"age\"=%1" ).arg( i ) );
    newStoredExpressions.append( storedExpression );
  }
  mManager->addStoredExpressions( newStoredExpressions );

  //get all expressions
  QList <QgsStoredExpression> allStoredExpressions = mManager->storedExpressions();
  QCOMPARE( allStoredExpressions.count(), 20 );
  QCOMPARE( allStoredExpressions.at( 0 ).name, QStringLiteral( "test0" ) );
  QCOMPARE( allStoredExpressions.at( 0 ).expression, QStringLiteral( "\"age\"=0" ) );
  QCOMPARE( allStoredExpressions.at( 14 ).name, QStringLiteral( "test14" ) );
  QCOMPARE( allStoredExpressions.at( 14 ).expression, QStringLiteral( "\"age\"=14" ) );
  QCOMPARE( allStoredExpressions.at( 19 ).name, QStringLiteral( "test19" ) );
  QCOMPARE( allStoredExpressions.at( 19 ).expression, QStringLiteral( "\"age\"=19" ) );
}

void TestQgsStoredExpressionManager::editExpressionsByExpression()
{
  QgsStoredExpression storedExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\"=4" ) );
  QCOMPARE( storedExpression.name, QStringLiteral( "test4" ) );

  mManager->updateStoredExpression( storedExpression.id, QStringLiteral( "Much older" ), QStringLiteral( "\"age\">99" ) );

  QCOMPARE( mManager->storedExpression( storedExpression.id ).name, QStringLiteral( "Much older" ) );
  QCOMPARE( mManager->storedExpression( storedExpression.id ).expression, QStringLiteral( "\"age\">99" ) );
}

void TestQgsStoredExpressionManager::deleteExpressionByExpression()
{
  QgsStoredExpression storedExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\"=4" ) );
  QCOMPARE( storedExpression.name, QStringLiteral( "test4" ) );

  mManager->removeStoredExpression( storedExpression.id );

  storedExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\"=4" ) );

  QVERIFY( storedExpression.id.isNull() );
}

QGSTEST_MAIN( TestQgsStoredExpressionManager )
#include "testqgsstoredexpressionmanager.moc"
