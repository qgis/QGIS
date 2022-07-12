/***************************************************************************
  testqgsruntimeprofiler.cpp
  --------------------------------------
Date                 : June 2020
Copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsapplication.h"
#include "qgsruntimeprofiler.h"

#include <QSignalSpy>

class TestQgsRuntimeProfiler: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGroups();
    void threading();

};


void TestQgsRuntimeProfiler::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsRuntimeProfiler::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRuntimeProfiler::testGroups()
{
  QgsRuntimeProfiler profiler;

  QVERIFY( profiler.groups().isEmpty() );
  QVERIFY( !profiler.groupIsActive( QStringLiteral( "xxx" ) ) );

  const QSignalSpy spy( &profiler, &QgsRuntimeProfiler::groupAdded );
  profiler.start( QStringLiteral( "task 1" ), QStringLiteral( "group 1" ) );

  QCOMPARE( profiler.groups().count(), 1 );
  QVERIFY( profiler.groups().contains( QStringLiteral( "group 1" ) ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toString(), QStringLiteral( "group 1" ) );
  QVERIFY( profiler.groupIsActive( QStringLiteral( "group 1" ) ) );

  profiler.start( QStringLiteral( "task 2" ), QStringLiteral( "group 2" ) );

  QCOMPARE( profiler.groups().count(), 2 );
  QVERIFY( profiler.groups().contains( QStringLiteral( "group 1" ) ) );
  QVERIFY( profiler.groups().contains( QStringLiteral( "group 2" ) ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).toString(), QStringLiteral( "group 2" ) );
  QVERIFY( profiler.groupIsActive( QStringLiteral( "group 2" ) ) );
  QVERIFY( profiler.groupIsActive( QStringLiteral( "group 1" ) ) );

  // sub task
  profiler.start( QStringLiteral( "task 1a" ), QStringLiteral( "group 1" ) );
  QCOMPARE( profiler.groups().count(), 2 );
  QCOMPARE( spy.count(), 2 );

  profiler.end( QStringLiteral( "group 1" ) );
  QVERIFY( profiler.groupIsActive( QStringLiteral( "group 1" ) ) );
  profiler.end( QStringLiteral( "group 2" ) );
  QVERIFY( !profiler.groupIsActive( QStringLiteral( "group 2" ) ) );
  profiler.end( QStringLiteral( "group 1" ) );
  QVERIFY( !profiler.groupIsActive( QStringLiteral( "group 1" ) ) );

  QCOMPARE( profiler.childGroups( QString(), QStringLiteral( "group 1" ) ), QStringList() << QStringLiteral( "task 1" ) );
  QCOMPARE( profiler.childGroups( QStringLiteral( "task 1" ), QStringLiteral( "group 1" ) ), QStringList() << QStringLiteral( "task 1a" ) );
  QCOMPARE( profiler.childGroups( QString(), QStringLiteral( "group 2" ) ), QStringList() << QStringLiteral( "task 2" ) );
}



class ProfileInThread : public QThread
{
    Q_OBJECT

  public :
    ProfileInThread( QgsRuntimeProfiler *mainProfiler )
      : mMainProfiler( mainProfiler )
    {}

    void run() override
    {
      const QgsScopedRuntimeProfile profile( QStringLiteral( "in thread" ), QStringLiteral( "bg" ) );
      QVERIFY( mMainProfiler != QgsApplication::profiler() );
    }

  private:
    QgsRuntimeProfiler *mMainProfiler = nullptr;


};

void TestQgsRuntimeProfiler::threading()
{
  // test that profiling which occurs in a background thread is bubbled up to the main thread runtime profiler
  QgsApplication::profiler()->clear();
  QCOMPARE( QgsApplication::profiler()->rowCount(), 0 );

  QThread *thread = new ProfileInThread( QgsApplication::profiler() );
  {
    const QgsScopedRuntimeProfile profile( QStringLiteral( "launch thread" ), QStringLiteral( "main" ) );

    QSignalSpy  spy( QgsApplication::profiler(), &QgsRuntimeProfiler::groupAdded );
    thread->start();
    thread->exit();

    spy.wait();
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( spy.at( 0 ).at( 0 ).toString(), QStringLiteral( "bg" ) );
  }

  QCOMPARE( QgsApplication::profiler()->rowCount(), 2 );
  const int row1 = QgsApplication::profiler()->data( QgsApplication::profiler()->index( 0, 0 ) ).toString() == QLatin1String( "launch thread" ) ? 0 : 1;
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1, 0 ) ).toString(), QStringLiteral( "launch thread" ) );
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1, 0 ), QgsRuntimeProfilerNode::Group ).toString(), QStringLiteral( "main" ) );
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1 == 0 ? 1 : 0, 0 ) ).toString(), QStringLiteral( "in thread" ) );
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1 == 0 ? 1 : 0, 0 ), QgsRuntimeProfilerNode::Group ).toString(), QStringLiteral( "bg" ) );
}


QGSTEST_MAIN( TestQgsRuntimeProfiler )
#include "testqgsruntimeprofiler.moc"
