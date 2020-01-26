/***************************************************************************
                         qgsabstracttemporal.h
                         ---------------
    begin                : January 2020
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
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgsabstracttemporal.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>

class TestSignalReceiver : public QObject
{
    Q_OBJECT

  public:
    TestSignalReceiver()
      : QObject( nullptr )
    {}
    QPainter::CompositionMode blendMode =  QPainter::CompositionMode_SourceOver ;
  public slots:
    void onBlendModeChanged( const QPainter::CompositionMode blendMode )
    {
      this->blendMode = blendMode;
    }
};

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsAbstractTemporal class.
 */
class TestQgsAbstractTemporal : public QObject
{
    Q_OBJECT

  public:
    TestQgsAbstractTemporal() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void isTemporal();
    void checkSettingTemporal();
    void checkSettingTemporalRange();
    void checkSettingCurrentDateTime();

  private:
    QgsAbstractTemporal *temporalObject = nullptr;
};

void TestQgsAbstractTemporal::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsAbstractTemporal::init()
{
  //create some objects that will be used in all tests...
  //create a temporal object that will be used in all tests...

  temporalObject = new QgsAbstractTemporal( true );
}

void TestQgsAbstractTemporal::cleanup()
{
}

void TestQgsAbstractTemporal::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAbstractTemporal::isTemporal()
{
  QVERIFY( !temporalObject->isTemporal() );
}

void TestQgsAbstractTemporal::checkSettingTemporal()
{
  temporalObject->setIsTemporal( false );

  QCOMPARE( temporalObject->isTemporal(), false );
}

void TestQgsAbstractTemporal::checkSettingTemporalRange()
{
  QgsDateTimeRange dateTimeRange = new QgsDateTimeRange();

  temporalObject->setTemporalRange( dateTimeRange );

  QCOMPARE( temporalObject->temporalRange(), dateTimeRange );
}

void TestQgsAbstractTemporal::checkSettingCurrentDateTime()
{
  QDateTime dateTime = new QDateTime();

  temporalObject->setCurrentDateTime( &dateTime );

  QCOMPARE( temporalObject->currentDateTime(), &dateTime );
}

QGSTEST_MAIN( TestQgsAbstractTemporal )
#include "testqgsabstracttemporal.moc"
