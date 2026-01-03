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
#include "qgsapplication.h"
#include "qgsruntimeprofiler.h"
#include "qgstest.h"

#include <QSignalSpy>

class TestQgsRuntimeProfiler : public QObject
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
  QVERIFY( !profiler.groupIsActive( u"xxx"_s ) );

  const QSignalSpy spy( &profiler, &QgsRuntimeProfiler::groupAdded );
  profiler.start( u"task 1"_s, u"group 1"_s );

  QCOMPARE( profiler.groups().count(), 1 );
  QVERIFY( profiler.groups().contains( u"group 1"_s ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).toString(), u"group 1"_s );
  QVERIFY( profiler.groupIsActive( u"group 1"_s ) );

  profiler.start( u"task 2"_s, u"group 2"_s );

  QCOMPARE( profiler.groups().count(), 2 );
  QVERIFY( profiler.groups().contains( u"group 1"_s ) );
  QVERIFY( profiler.groups().contains( u"group 2"_s ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).toString(), u"group 2"_s );
  QVERIFY( profiler.groupIsActive( u"group 2"_s ) );
  QVERIFY( profiler.groupIsActive( u"group 1"_s ) );

  // sub task
  profiler.start( u"task 1a"_s, u"group 1"_s );
  QCOMPARE( profiler.groups().count(), 2 );
  QCOMPARE( spy.count(), 2 );

  profiler.end( u"group 1"_s );
  QVERIFY( profiler.groupIsActive( u"group 1"_s ) );
  profiler.end( u"group 2"_s );
  QVERIFY( !profiler.groupIsActive( u"group 2"_s ) );
  profiler.end( u"group 1"_s );
  QVERIFY( !profiler.groupIsActive( u"group 1"_s ) );

  QCOMPARE( profiler.childGroups( QString(), u"group 1"_s ), QStringList() << u"task 1"_s );
  QCOMPARE( profiler.childGroups( u"task 1"_s, u"group 1"_s ), QStringList() << u"task 1a"_s );
  QCOMPARE( profiler.childGroups( QString(), u"group 2"_s ), QStringList() << u"task 2"_s );

  QString profilerAsText = profiler.asText();
  // verify individual chunks as the ordering of individual model items can vary
  QVERIFY( profilerAsText.contains( u"group 2\r\n- task 2: 0"_s ) );
  QVERIFY( profilerAsText.contains( u"group 1\r\n"_s ) );
  QVERIFY( profilerAsText.contains( u"\r\n- task 1: 0"_s ) );
  QVERIFY( profilerAsText.contains( u"\r\n-- task 1a: 0"_s ) );

  profilerAsText = profiler.asText( u"group 2"_s );
  // verify individual chunks as the ordering of individual model items can vary
  QCOMPARE( profilerAsText, u"group 2\r\n- task 2: 0"_s );
}


class ProfileInThread : public QThread
{
    Q_OBJECT

  public:
    ProfileInThread( QgsRuntimeProfiler *mainProfiler )
      : mMainProfiler( mainProfiler )
    {}

    void run() override
    {
      const QgsScopedRuntimeProfile profile( u"in thread"_s, u"bg"_s );
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
    const QgsScopedRuntimeProfile profile( u"launch thread"_s, u"main"_s );

    QSignalSpy spy( QgsApplication::profiler(), &QgsRuntimeProfiler::groupAdded );
    thread->start();
    thread->exit();

    spy.wait();
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( spy.at( 0 ).at( 0 ).toString(), u"bg"_s );
  }

  QCOMPARE( QgsApplication::profiler()->rowCount(), 2 );
  const int row1 = QgsApplication::profiler()->data( QgsApplication::profiler()->index( 0, 0 ) ).toString() == "launch thread"_L1 ? 0 : 1;
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1, 0 ) ).toString(), u"launch thread"_s );
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1, 0 ), static_cast<int>( QgsRuntimeProfilerNode::CustomRole::Group ) ).toString(), u"main"_s );
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1 == 0 ? 1 : 0, 0 ) ).toString(), u"in thread"_s );
  QCOMPARE( QgsApplication::profiler()->data( QgsApplication::profiler()->index( row1 == 0 ? 1 : 0, 0 ), static_cast<int>( QgsRuntimeProfilerNode::CustomRole::Group ) ).toString(), u"bg"_s );
}


QGSTEST_MAIN( TestQgsRuntimeProfiler )
#include "testqgsruntimeprofiler.moc"
