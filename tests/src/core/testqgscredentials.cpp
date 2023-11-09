/***************************************************************************
     testqgscredentials.cpp
     --------------------
    Date                 : February 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QtConcurrent>
#include "qgscredentials.h"

class TestCredentials : public QgsCredentials
{

  public:
    TestCredentials( bool isInstance )
      : mExpectedRealm( QStringLiteral( "test_realm" ) )
    {
      if ( isInstance )
        setInstance( this );
    }

  protected:
    bool request( const QString &realm, QString &username, QString &password, const QString & ) override
    {
      Q_ASSERT( realm == mExpectedRealm );

      username = mUsername;
      password = mPassword;
      return true;
    }

    bool requestMasterPassword( QString &, bool ) override
    {
      return true;
    }

  public:
    QString mUsername;
    QString mPassword;
    QString mExpectedRealm;
};


class TestQgsCredentials : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void basic();
    void threadSafety();

};


void TestQgsCredentials::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsCredentials::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCredentials::basic()
{
  TestCredentials *t = new TestCredentials( true );
  QCOMPARE( QgsCredentials::instance(), t );

  QString username;
  QString password;
  QVERIFY( QgsCredentials::instance()->get( QStringLiteral( "test_realm" ), username, password ) );
  QVERIFY( username.isEmpty() );
  QVERIFY( password.isEmpty() );

  t->mUsername = QStringLiteral( "user" );
  t->mPassword = QStringLiteral( "pass" );
  QVERIFY( QgsCredentials::instance()->get( QStringLiteral( "test_realm" ), username, password ) );
  QCOMPARE( username, QStringLiteral( "user" ) );
  QCOMPARE( password, QStringLiteral( "pass" ) );

  // put credential
  QgsCredentials::instance()->put( QStringLiteral( "test_realm" ), QStringLiteral( "user" ), QStringLiteral( "pass" ) );
  // TestCredentials should not be used, put credentials should be used instead
  t->mUsername = QStringLiteral( "user2" );
  t->mPassword = QStringLiteral( "pass2" );
  QVERIFY( QgsCredentials::instance()->get( QStringLiteral( "test_realm" ), username, password ) );
  QCOMPARE( username, QStringLiteral( "user" ) );
  QCOMPARE( password, QStringLiteral( "pass" ) );

  t->mExpectedRealm = QStringLiteral( "test_realm2" );
  // should use TestCredentials again, as now cached credentials for test_realm2
  QVERIFY( QgsCredentials::instance()->get( QStringLiteral( "test_realm2" ), username, password ) );
  QCOMPARE( username, QStringLiteral( "user2" ) );
  QCOMPARE( password, QStringLiteral( "pass2" ) );
}

struct GetPutCredentials
{
  void operator()( int i )
  {
    QgsCredentials::instance()->put( QStringLiteral( "test_realm%1" ).arg( i ), QStringLiteral( "username" ), QStringLiteral( "password" ) );
    QString user;
    QString password;
    QVERIFY( QgsCredentials::instance()->get( QStringLiteral( "test_realm%1" ).arg( i ), user, password ) );
    QCOMPARE( user, QStringLiteral( "username" ) );
    QCOMPARE( password, QStringLiteral( "password" ) );
  }
};

void TestQgsCredentials::threadSafety()
{
  // ensure credentials storage/retrieval is thread safe
  TestCredentials *t = new TestCredentials( true );

  t->mUsername = QStringLiteral( "user" );
  t->mPassword = QStringLiteral( "pass" );

  // smash credentials rendering over multiple threads
  QVector< int > list;
  list.resize( 1000 );
  for ( int i = 0; i < list.size(); ++i )
    list[i] = i;
  QtConcurrent::blockingMap( list, GetPutCredentials() );
}

QGSTEST_MAIN( TestQgsCredentials )
#include "testqgscredentials.moc"
