/***************************************************************************
                         testqgstemporalnavigationobject.cpp
                         ---------------
    begin                : April 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QSignalSpy>

//qgis includes...
#include <qgstemporalnavigationobject.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsTemporalNavigationObject class.
 */
class TestQgsTemporalNavigationObject : public QObject
{
    Q_OBJECT

  public:
    TestQgsTemporalNavigationObject() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void animationState();
    void temporalExtents();
    void frameSettings();
    void navigationMode();
    void expressionContext();
    void testIrregularStep();

  private:
    QgsTemporalNavigationObject *navigationObject = nullptr;
};

void TestQgsTemporalNavigationObject::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsTemporalNavigationObject::init()
{
  //create some objects that will be used in all tests...
  //create a temporal object that will be used in all tests...

  navigationObject = new QgsTemporalNavigationObject();
  navigationObject->setNavigationMode( QgsTemporalNavigationObject::Animated );
}

void TestQgsTemporalNavigationObject::cleanup()
{
}

void TestQgsTemporalNavigationObject::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTemporalNavigationObject::animationState()
{
  const QgsDateTimeRange range = QgsDateTimeRange(
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                                   QDateTime( QDate( 2020, 10, 1 ), QTime( 8, 0, 0 ) )
                                 );
  navigationObject->setTemporalExtents( range );

  navigationObject->setFrameDuration( QgsInterval( 1, QgsUnitTypes::TemporalMonths ) );

  qRegisterMetaType<QgsTemporalNavigationObject::AnimationState>( "AnimationState" );
  const QSignalSpy stateSignal( navigationObject, &QgsTemporalNavigationObject::stateChanged );

  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Idle );

  navigationObject->setAnimationState( QgsTemporalNavigationObject::Forward );
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Forward );
  QCOMPARE( stateSignal.count(), 1 );

  navigationObject->playBackward();
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Reverse );
  QCOMPARE( stateSignal.count(), 2 );

  navigationObject->playForward();
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Forward );
  QCOMPARE( stateSignal.count(), 3 );

  navigationObject->pause();
  QCOMPARE( navigationObject->animationState(), QgsTemporalNavigationObject::Idle );
  QCOMPARE( stateSignal.count(), 4 );

  navigationObject->next();
  QCOMPARE( navigationObject->currentFrameNumber(), 1 );

  navigationObject->previous();
  QCOMPARE( navigationObject->currentFrameNumber(), 0 );

  navigationObject->skipToEnd();
  QCOMPARE( navigationObject->currentFrameNumber(), 9 );

  navigationObject->rewindToStart();
  QCOMPARE( navigationObject->currentFrameNumber(), 0 );

  QCOMPARE( navigationObject->isLooping(), false );
  navigationObject->setLooping( true );
  QCOMPARE( navigationObject->isLooping(), true );

}

void TestQgsTemporalNavigationObject::temporalExtents()
{
  const QgsDateTimeRange range = QgsDateTimeRange(
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                                   QDateTime( QDate( 2020, 12, 1 ), QTime( 8, 0, 0 ) )
                                 );
  navigationObject->setTemporalExtents( range );
  QCOMPARE( navigationObject->temporalExtents(), range );

  navigationObject->setTemporalExtents( QgsDateTimeRange() );
  QCOMPARE( navigationObject->temporalExtents(), QgsDateTimeRange() );
}

void TestQgsTemporalNavigationObject::navigationMode()
{
  const QgsDateTimeRange range = QgsDateTimeRange(
                                   QDateTime( QDate( 2010, 1, 1 ), QTime( 0, 0, 0 ) ),
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ) );

  const QgsDateTimeRange range2 = QgsDateTimeRange(
                                    QDateTime( QDate( 2015, 1, 1 ), QTime( 0, 0, 0 ) ),
                                    QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ) );

  QgsDateTimeRange check;
  auto checkUpdateTemporalRange = [&check]( const QgsDateTimeRange range )
  {
    QCOMPARE( range, check );
  };
  QObject *context = new QObject( this );
  connect( navigationObject, &QgsTemporalNavigationObject::updateTemporalRange, context, checkUpdateTemporalRange );

  // Changing navigation mode emits an updateTemporalRange, in this case it should be an empty range
  navigationObject->setNavigationMode( QgsTemporalNavigationObject::NavigationOff );
  // Setting temporal extents also triggers an updateTemporalRange with an empty range
  navigationObject->setTemporalExtents( range );

  // Changing navigation mode emits an updateTemporalRange, in this case it should be the last range
  // we used in setTemporalExtents.
  check = range;
  navigationObject->setNavigationMode( QgsTemporalNavigationObject::FixedRange );
  check = range2;
  navigationObject->setTemporalExtents( range2 );

  // Delete context to disconnect the signal to the lambda function
  delete context;
  navigationObject->setNavigationMode( QgsTemporalNavigationObject::Animated );
}

void TestQgsTemporalNavigationObject::frameSettings()
{
  navigationObject->setFrameDuration( QgsInterval( 2, QgsUnitTypes::TemporalHours ) );

  const QSignalSpy temporalRangeSignal( navigationObject, &QgsTemporalNavigationObject::updateTemporalRange );

  const QgsDateTimeRange range = QgsDateTimeRange(
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 0, 0 ) ),
                                   true,
                                   false
                                 );
  navigationObject->setTemporalExtents( range );
  QCOMPARE( temporalRangeSignal.count(), 1 );
  // two frames - 8-10am, 10-12am
  QCOMPARE( navigationObject->totalFrameCount(), 2LL );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 0 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 10, 0, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 1 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 10, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 0, 0 ) ),
              true,
              false
            ) );

  navigationObject->setFrameDuration( QgsInterval( 1, QgsUnitTypes::TemporalHours ) );
  QCOMPARE( navigationObject->frameDuration(), QgsInterval( 1, QgsUnitTypes::TemporalHours ) );
  QCOMPARE( temporalRangeSignal.count(), 2 );

  QCOMPARE( navigationObject->frameDuration().originalDuration(), 1.0 );
  QCOMPARE( navigationObject->frameDuration().originalUnit(), QgsUnitTypes::TemporalHours );

  QCOMPARE( navigationObject->currentFrameNumber(), 0 );
  // four frames - 8-9, 9-10, 10-11, 11-12am
  QCOMPARE( navigationObject->totalFrameCount(), 4LL );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 0 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 9, 0, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 1 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 9, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 10, 0, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 2 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 10, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 11, 0, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 3 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 11, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 0, 0 ) ),
              true,
              false
            ) );

  navigationObject->setCurrentFrameNumber( 1 );
  QCOMPARE( navigationObject->currentFrameNumber(), 1 );
  QCOMPARE( temporalRangeSignal.count(), 3 );

  // Test Overflow
  navigationObject->setCurrentFrameNumber( 100 );
  QCOMPARE( navigationObject->currentFrameNumber(), navigationObject->totalFrameCount() - 1 );
  QCOMPARE( temporalRangeSignal.count(), 4 );

  // Test Underflow
  navigationObject->setCurrentFrameNumber( -100 );
  QCOMPARE( navigationObject->currentFrameNumber(), 0 );
  QCOMPARE( temporalRangeSignal.count(), 5 );

  navigationObject->setFramesPerSecond( 1 );
  QCOMPARE( navigationObject->framesPerSecond(), 1.0 );

  // Test if changing the frame duration 'keeps' the current frameNumber
  navigationObject->setCurrentFrameNumber( 2 ); // 10:00-11
  QCOMPARE( navigationObject->currentFrameNumber(), 2 );
  navigationObject->setFrameDuration( QgsInterval( 2, QgsUnitTypes::TemporalHours ) );
  QCOMPARE( navigationObject->currentFrameNumber(), 1 ); // going from 1 hour to 2 hour frames, but stay on 10:00-...
  QCOMPARE( temporalRangeSignal.count(), 7 );

  // two frames - 8-10, 10-12am
  QCOMPARE( navigationObject->totalFrameCount(), 2LL );

  // Test if, when changing to Cumulative mode, the dateTimeRange for frame 2 (with 2 hours frames) is indeed the full range
  navigationObject->setTemporalRangeCumulative( true );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 1 ), QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 0, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( temporalRangeSignal.count(), 7 );

  navigationObject->setTemporalRangeCumulative( false );
  // interval which doesn't fit exactly into overall range
  navigationObject->setFrameDuration( QgsInterval( 0.75, QgsUnitTypes::TemporalHours ) );
  // six frames - 8-8.45, 8.45-9.30, 9.30-10.15, 10.15-11.00, 11.00-11.45, 11.45-12.30
  QCOMPARE( navigationObject->totalFrameCount(), 6LL );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 0 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 45, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 1 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 45, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 9, 30, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 2 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 9, 30, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 10, 15, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 3 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 10, 15, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 11, 0, 0 ) ),
              true,
              false
            ) );
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 4 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 11, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 11, 45, 0 ) ),
              true,
              false
            ) );
  // yes, this frame goes PAST the end of the overall animation range -- but we need to ensure that
  // every frame has equal length!
  QCOMPARE( navigationObject->dateTimeRangeForFrameNumber( 5 ),  QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 1 ), QTime( 11, 45, 0 ) ),
              QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 30, 0 ) ),
              true,
              false
            ) );
}

void TestQgsTemporalNavigationObject::expressionContext()
{
  QgsTemporalNavigationObject object;
  const QgsDateTimeRange range = QgsDateTimeRange(
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 8, 0, 0 ) ),
                                   QDateTime( QDate( 2020, 1, 1 ), QTime( 12, 0, 0 ) )
                                 );
  object.setTemporalExtents( range );
  object.setFrameDuration( QgsInterval( 1, QgsUnitTypes::TemporalHours ) );
  object.setCurrentFrameNumber( 1 );
  object.setFramesPerSecond( 30 );

  std::unique_ptr< QgsExpressionContextScope > scope( object.createExpressionContextScope() );
  QCOMPARE( scope->variable( QStringLiteral( "frame_rate" ) ).toDouble(), 30.0 );
  QCOMPARE( scope->variable( QStringLiteral( "frame_duration" ) ).value< QgsInterval >().seconds(), 3600.0 );
  QCOMPARE( scope->variable( QStringLiteral( "frame_timestep" ) ).value< double >(), 1.0 );
  QCOMPARE( scope->variable( QStringLiteral( "frame_timestep_unit" ) ).value< QgsUnitTypes::TemporalUnit >(), QgsUnitTypes::TemporalUnit::TemporalHours );
  QCOMPARE( scope->variable( QStringLiteral( "frame_number" ) ).toInt(), 1 );
  QCOMPARE( scope->variable( QStringLiteral( "animation_start_time" ) ).toDateTime(), range.begin() );
  QCOMPARE( scope->variable( QStringLiteral( "animation_end_time" ) ).toDateTime(), range.end() );
  QCOMPARE( scope->variable( QStringLiteral( "animation_interval" ) ).value< QgsInterval >(), range.end() - range.begin() );
}

void TestQgsTemporalNavigationObject::testIrregularStep()
{
  // test using the navigation in irregular step mode
  QgsTemporalNavigationObject object;
  const QList< QgsDateTimeRange > ranges{  QgsDateTimeRange(
        QDateTime( QDate( 2020, 1, 10 ), QTime( 0, 0, 0 ) ),
        QDateTime( QDate( 2020, 1, 11 ), QTime( 0, 0, 0 ) ) ),
      QgsDateTimeRange(
        QDateTime( QDate( 2020, 1, 15 ), QTime( 0, 0, 0 ) ),
        QDateTime( QDate( 2020, 1, 20 ), QTime( 0, 0, 0 ) ) ),
      QgsDateTimeRange(
        QDateTime( QDate( 2020, 3, 1 ), QTime( 0, 0, 0 ) ),
        QDateTime( QDate( 2020, 4, 5 ), QTime( 0, 0, 0 ) ) )
                                        };
  object.setAvailableTemporalRanges( ranges );

  object.setFrameDuration( QgsInterval( 1, QgsUnitTypes::TemporalIrregularStep ) );

  QCOMPARE( object.totalFrameCount(), 3LL );

  QCOMPARE( object.dateTimeRangeForFrameNumber( 0 ), QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 10 ), QTime( 0, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 11 ), QTime( 0, 0, 0 ) ) ) );
  // negative should return first frame range
  QCOMPARE( object.dateTimeRangeForFrameNumber( -1 ), QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 10 ), QTime( 0, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 11 ), QTime( 0, 0, 0 ) ) ) );
  QCOMPARE( object.dateTimeRangeForFrameNumber( 1 ), QgsDateTimeRange(
              QDateTime( QDate( 2020, 1, 15 ), QTime( 0, 0, 0 ) ),
              QDateTime( QDate( 2020, 1, 20 ), QTime( 0, 0, 0 ) ) ) );
  QCOMPARE( object.dateTimeRangeForFrameNumber( 2 ), QgsDateTimeRange(
              QDateTime( QDate( 2020, 3, 1 ), QTime( 0, 0, 0 ) ),
              QDateTime( QDate( 2020, 4, 5 ), QTime( 0, 0, 0 ) ) ) );
  QCOMPARE( object.dateTimeRangeForFrameNumber( 5 ), QgsDateTimeRange(
              QDateTime( QDate( 2020, 3, 1 ), QTime( 0, 0, 0 ) ),
              QDateTime( QDate( 2020, 4, 5 ), QTime( 0, 0, 0 ) ) ) );

  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2019, 1, 1 ), QTime() ) ), 0LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 1, 10 ), QTime( 0, 0, 0 ) ) ), 0LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 1, 11 ), QTime( 0, 0, 0 ) ) ), 0LL );
  // in between available ranges, go back a frame
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 1, 12 ), QTime( 0, 0, 0 ) ) ), 0LL );

  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 1, 15 ), QTime( 0, 0, 0 ) ) ), 1LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 1, 16 ), QTime( 0, 0, 0 ) ) ), 1LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 1, 20 ), QTime( 0, 0, 0 ) ) ), 1LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 2, 15 ), QTime( 0, 0, 0 ) ) ), 1LL );

  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 3, 1 ), QTime( 0, 0, 0 ) ) ), 2LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 3, 2 ), QTime( 0, 0, 0 ) ) ), 2LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 4, 5 ), QTime( 0, 0, 0 ) ) ), 2LL );
  QCOMPARE( object.findBestFrameNumberForFrameStart( QDateTime( QDate( 2020, 5, 6 ), QTime( 0, 0, 0 ) ) ), 2LL );
}

QGSTEST_MAIN( TestQgsTemporalNavigationObject )
#include "testqgstemporalnavigationobject.moc"
