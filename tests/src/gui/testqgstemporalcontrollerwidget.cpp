/***************************************************************************
    qgstemporalcontrollerwidget.cpp
    ---------------------
    begin                : 2026/10/24
    copyright            : (C) 2026 by Till Frankenbach
    email                : till at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalcontrollerwidget.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

class TestQgsTemporalControllerWidget : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsTemporalControllerWidget()
      : QgsTest( u"Temporal Controller Widget Tests"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testDatePersists();
};

void TestQgsTemporalControllerWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsTemporalControllerWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTemporalControllerWidget::init()
{}

void TestQgsTemporalControllerWidget::cleanup()
{}

void TestQgsTemporalControllerWidget::testDatePersists()
{
  QgsTemporalControllerWidget *mTemporalControllerWidget = new QgsTemporalControllerWidget();
  mTemporalControllerWidget->mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::Animated );


  QDateTime start = QDateTime( QDate( 2006, 1, 1 ), QTime( 0, 0 ) );
  QDateTime end = QDateTime( QDate( 2006, 1, 2 ), QTime( 0, 0 ) );

  QVERIFY( start.isValid() );
  QVERIFY( end.isValid() );

  mTemporalControllerWidget->mNavigationObject->setTemporalExtents( QgsDateTimeRange( start, end ) );


  QVERIFY( mTemporalControllerWidget->mStartDateTime );
  QCOMPARE( mTemporalControllerWidget->mStartDateTime->date(), start.date() );
}


QGSTEST_MAIN( TestQgsTemporalControllerWidget )
#include "testqgstemporalcontrollerwidget.moc"
