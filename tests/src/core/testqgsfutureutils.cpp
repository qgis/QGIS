/***************************************************************************
     testqgsfutureutils.cpp
     --------------------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 by David Koňařík
    Email                : dvdkon at konarici dot cz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsexception.h"
#include "qgstest.h"

#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <qassert.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <qtestcase.h>

using namespace Qt::StringLiterals;

#include <qgsfutureutils.h>

class TestQgsFutureUtils : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsFutureUtils()
      : QgsTest( u"QgsFutureUtils tests"_s )
    {}

  private slots:
    void waitForFinished();
    void clone();
    void combineOk();
    void combineNoResult();
    void combineException();
};

void TestQgsFutureUtils::waitForFinished()
{
  QPromise<void> promise;
  QFuture<void> future = QtConcurrent::run( [] { QThread::msleep( 10 ); } ).then( this, [&promise] { promise.finish(); } );
  // promise.future().waitForFinished() would hang here
  QgsFutureUtils::waitForFinished( promise.future() );
  QVERIFY( promise.future().isFinished() );
}

void TestQgsFutureUtils::clone()
{
  QPromise<void> promise;
  QFuture<bool> clonedFuture = QgsFutureUtils::clone( promise.future() ).then( [&promise]() -> bool { return promise.future().isFinished(); } );
  promise.finish();
  QgsFutureUtils::waitForFinished( clonedFuture );
  QVERIFY( clonedFuture.isFinished() );
  QCOMPARE( clonedFuture.result(), true );
}

void TestQgsFutureUtils::combineOk()
{
  QFuture<int> future1 = QtFuture::makeReadyValueFuture( 1 );
  QFuture<int> future2 = QtFuture::makeReadyValueFuture( 2 );
  QFuture<std::tuple<int, int>> combined = QgsFutureUtils::combine( future1, future2 );
  QgsFutureUtils::waitForFinished( combined );
  QVERIFY( combined.isValid() );
  QCOMPARE( combined.resultCount(), 1 );
  QCOMPARE( combined.result(), ( std::tuple<int, int> { 1, 2 } ) );
}

void TestQgsFutureUtils::combineNoResult()
{
  QFuture<int> future1 = QtFuture::makeReadyValueFuture( 1 );
  QPromise<int> promise2;
  QFuture<std::tuple<int, int>> combined = QgsFutureUtils::combine( future1, promise2.future() );
  promise2.finish();
  QgsFutureUtils::waitForFinished( combined );
  QVERIFY( combined.isValid() );
  QCOMPARE( combined.resultCount(), 0 );
}

void TestQgsFutureUtils::combineException()
{
  QFuture<int> future1 = QtFuture::makeReadyValueFuture( 1 );
  QFuture<int> future2 = QtConcurrent::run( []() {
    QThread::msleep( 10 );
    throw QException();
    return 2;
  } );
  QFuture<std::tuple<int, int>> combined = QgsFutureUtils::combine( future1, future2 );
  QVERIFY_THROWS_EXCEPTION( QException, QgsFutureUtils::waitForFinished( combined ) );
  QVERIFY( !combined.isValid() );
  QCOMPARE( combined.resultCount(), 0 );
}

QGSTEST_MAIN( TestQgsFutureUtils )
#include "testqgsfutureutils.moc"
