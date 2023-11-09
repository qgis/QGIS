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

  //fill up some for the FilterExpression
  for ( int i = 0; i < 10; i++ )
  {
    const QgsStoredExpression storedExpression( QStringLiteral( "filter%1" ).arg( i ), QStringLiteral( "\"age\"=%1" ).arg( i ), QgsStoredExpression::Category::FilterExpression );
    newStoredExpressions.append( storedExpression );
  }
  //fill up some for the DefaultValues
  for ( int i = 10; i < 20; i++ )
  {
    const QgsStoredExpression storedExpression( QStringLiteral( "default%1" ).arg( i ), QStringLiteral( "'ID_'+%1" ).arg( i ), QgsStoredExpression::Category::DefaultValueExpression );
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
  //add single stored filter expression
  const QString name = QStringLiteral( "test20" );
  const QString expression = QStringLiteral( "\"age\"=20" );
  const QString id = mManager->addStoredExpression( name, expression );

  //get stored expression by id
  const QgsStoredExpression storedExpression = mManager->storedExpression( id );
  QCOMPARE( storedExpression.id, id );
  QCOMPARE( storedExpression.name, name );
  QCOMPARE( storedExpression.expression, expression );
  QCOMPARE( storedExpression.tag, QgsStoredExpression::Category::FilterExpression );

  //get all expressions
  const QList <QgsStoredExpression> allStoredExpressions = mManager->storedExpressions();
  QCOMPARE( allStoredExpressions.count(), 21 );

  //get all expressions for Category::FilterExpression
  const QList <QgsStoredExpression> allStoredFilterExpressions = mManager->storedExpressions( QgsStoredExpression::Category::FilterExpression );
  QCOMPARE( allStoredFilterExpressions.count(), 11 );

  QCOMPARE( allStoredFilterExpressions.at( 10 ).id, id );
  QCOMPARE( allStoredFilterExpressions.at( 10 ).name, name );
  QCOMPARE( allStoredFilterExpressions.at( 10 ).expression, expression );
  QCOMPARE( allStoredFilterExpressions.at( 10 ).tag, QgsStoredExpression::Category::FilterExpression );
}

void TestQgsStoredExpressionManager::storeListOfExpressions()
{
  QList <QgsStoredExpression> newStoredExpressions;

  //fill up
  for ( int i = 20; i < 30; i++ )
  {
    const QgsStoredExpression storedExpression( QStringLiteral( "test%1" ).arg( i ), QStringLiteral( "\"age\"=%1" ).arg( i ) );
    newStoredExpressions.append( storedExpression );
  }
  mManager->addStoredExpressions( newStoredExpressions );

  //get all expressions
  const QList <QgsStoredExpression> allStoredExpressions = mManager->storedExpressions();
  QCOMPARE( allStoredExpressions.count(), 30 );
  QCOMPARE( allStoredExpressions.at( 0 ).name, QStringLiteral( "filter0" ) );
  QCOMPARE( allStoredExpressions.at( 0 ).expression, QStringLiteral( "\"age\"=0" ) );
  QCOMPARE( allStoredExpressions.at( 14 ).name, QStringLiteral( "default14" ) );
  QCOMPARE( allStoredExpressions.at( 14 ).expression, QStringLiteral( "'ID_'+14" ) );
  QCOMPARE( allStoredExpressions.at( 25 ).name, QStringLiteral( "test25" ) );
  QCOMPARE( allStoredExpressions.at( 25 ).expression, QStringLiteral( "\"age\"=25" ) );
}

void TestQgsStoredExpressionManager::editExpressionsByExpression()
{
  const QgsStoredExpression storedExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\"=4" ) );
  QCOMPARE( storedExpression.name, QStringLiteral( "filter4" ) );

  mManager->updateStoredExpression( storedExpression.id, QStringLiteral( "Much older" ), QStringLiteral( "\"age\">99" ), QgsStoredExpression::Category::FilterExpression );

  QCOMPARE( mManager->storedExpression( storedExpression.id ).name, QStringLiteral( "Much older" ) );
  QCOMPARE( mManager->storedExpression( storedExpression.id ).expression, QStringLiteral( "\"age\">99" ) );
  QCOMPARE( mManager->storedExpression( storedExpression.id ).tag, QgsStoredExpression::Category::FilterExpression );

  const QgsStoredExpression newStoredExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\">99" ) );
  QCOMPARE( newStoredExpression.name, QStringLiteral( "Much older" ) );
}

void TestQgsStoredExpressionManager::deleteExpressionByExpression()
{
  QgsStoredExpression storedExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\"=4" ) );
  QCOMPARE( storedExpression.name, QStringLiteral( "filter4" ) );

  mManager->removeStoredExpression( storedExpression.id );

  storedExpression = mManager->findStoredExpressionByExpression( QStringLiteral( "\"age\"=4" ) );
  QVERIFY( storedExpression.id.isNull() );
}

QGSTEST_MAIN( TestQgsStoredExpressionManager )
#include "testqgsstoredexpressionmanager.moc"
