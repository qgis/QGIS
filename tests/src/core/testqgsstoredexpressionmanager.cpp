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

#include "qgsstoredexpressionmanager.h"
#include "qgstest.h"

#include <QString>

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

  QList<QgsStoredExpression> newStoredExpressions;

  //fill up some for the FilterExpression
  for ( int i = 0; i < 10; i++ )
  {
    const QgsStoredExpression storedExpression( u"filter%1"_s.arg( i ), u"\"age\"=%1"_s.arg( i ), QgsStoredExpression::Category::FilterExpression );
    newStoredExpressions.append( storedExpression );
  }
  //fill up some for the DefaultValues
  for ( int i = 10; i < 20; i++ )
  {
    const QgsStoredExpression storedExpression( u"default%1"_s.arg( i ), u"'ID_'+%1"_s.arg( i ), QgsStoredExpression::Category::DefaultValueExpression );
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
  const QString name = u"test20"_s;
  const QString expression = u"\"age\"=20"_s;
  const QString id = mManager->addStoredExpression( name, expression );

  //get stored expression by id
  const QgsStoredExpression storedExpression = mManager->storedExpression( id );
  QCOMPARE( storedExpression.id, id );
  QCOMPARE( storedExpression.name, name );
  QCOMPARE( storedExpression.expression, expression );
  QCOMPARE( storedExpression.tag, QgsStoredExpression::Category::FilterExpression );

  //get all expressions
  const QList<QgsStoredExpression> allStoredExpressions = mManager->storedExpressions();
  QCOMPARE( allStoredExpressions.count(), 21 );

  //get all expressions for Category::FilterExpression
  const QList<QgsStoredExpression> allStoredFilterExpressions = mManager->storedExpressions( QgsStoredExpression::Category::FilterExpression );
  QCOMPARE( allStoredFilterExpressions.count(), 11 );

  QCOMPARE( allStoredFilterExpressions.at( 10 ).id, id );
  QCOMPARE( allStoredFilterExpressions.at( 10 ).name, name );
  QCOMPARE( allStoredFilterExpressions.at( 10 ).expression, expression );
  QCOMPARE( allStoredFilterExpressions.at( 10 ).tag, QgsStoredExpression::Category::FilterExpression );
}

void TestQgsStoredExpressionManager::storeListOfExpressions()
{
  QList<QgsStoredExpression> newStoredExpressions;

  //fill up
  for ( int i = 20; i < 30; i++ )
  {
    const QgsStoredExpression storedExpression( u"test%1"_s.arg( i ), u"\"age\"=%1"_s.arg( i ) );
    newStoredExpressions.append( storedExpression );
  }
  mManager->addStoredExpressions( newStoredExpressions );

  //get all expressions
  const QList<QgsStoredExpression> allStoredExpressions = mManager->storedExpressions();
  QCOMPARE( allStoredExpressions.count(), 30 );
  QCOMPARE( allStoredExpressions.at( 0 ).name, u"filter0"_s );
  QCOMPARE( allStoredExpressions.at( 0 ).expression, u"\"age\"=0"_s );
  QCOMPARE( allStoredExpressions.at( 14 ).name, u"default14"_s );
  QCOMPARE( allStoredExpressions.at( 14 ).expression, u"'ID_'+14"_s );
  QCOMPARE( allStoredExpressions.at( 25 ).name, u"test25"_s );
  QCOMPARE( allStoredExpressions.at( 25 ).expression, u"\"age\"=25"_s );
}

void TestQgsStoredExpressionManager::editExpressionsByExpression()
{
  const QgsStoredExpression storedExpression = mManager->findStoredExpressionByExpression( u"\"age\"=4"_s );
  QCOMPARE( storedExpression.name, u"filter4"_s );

  mManager->updateStoredExpression( storedExpression.id, u"Much older"_s, u"\"age\">99"_s, QgsStoredExpression::Category::FilterExpression );

  QCOMPARE( mManager->storedExpression( storedExpression.id ).name, u"Much older"_s );
  QCOMPARE( mManager->storedExpression( storedExpression.id ).expression, u"\"age\">99"_s );
  QCOMPARE( mManager->storedExpression( storedExpression.id ).tag, QgsStoredExpression::Category::FilterExpression );

  const QgsStoredExpression newStoredExpression = mManager->findStoredExpressionByExpression( u"\"age\">99"_s );
  QCOMPARE( newStoredExpression.name, u"Much older"_s );
}

void TestQgsStoredExpressionManager::deleteExpressionByExpression()
{
  QgsStoredExpression storedExpression = mManager->findStoredExpressionByExpression( u"\"age\"=4"_s );
  QCOMPARE( storedExpression.name, u"filter4"_s );

  mManager->removeStoredExpression( storedExpression.id );

  storedExpression = mManager->findStoredExpressionByExpression( u"\"age\"=4"_s );
  QVERIFY( storedExpression.id.isNull() );
}

QGSTEST_MAIN( TestQgsStoredExpressionManager )
#include "testqgsstoredexpressionmanager.moc"
