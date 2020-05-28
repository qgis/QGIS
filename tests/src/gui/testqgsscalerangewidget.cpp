/***************************************************************************
                         testqgsscalerangewidget.cpp
                         ---------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Sandro Santilli
    email                : strk at kbt dot io
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

#include "qgsscalerangewidget.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QObject>
#include <QLineEdit>
#include <QComboBox>
#include <QtTest/QSignalSpy>

#include <memory>

/**
 * @ingroup UnitTests
 * This is a unit test for the scale range widget
 *
 * \see QgsScaleRangeWidget
 */
class TestQgsScaleRangeWidget : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void test_setScaleRange();
  private:
    std::unique_ptr<QgsScaleRangeWidget> widget;
};

void TestQgsScaleRangeWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsScaleRangeWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsScaleRangeWidget::init()
{
  widget.reset( new QgsScaleRangeWidget() );
}

void TestQgsScaleRangeWidget::cleanup()
{
}

void TestQgsScaleRangeWidget::test_setScaleRange()
{
  // Test that setting scale range is always honoured
  // rather than being limited by previously set
  // max or min.
  // See https://github.com/qgis/QGIS/issues/23389

  widget->setScaleRange( 6, 4 );
  QCOMPARE( widget->minimumScale(), 6.0 );
  QCOMPARE( widget->maximumScale(), 4.0 );

  widget->setScaleRange( 10.0, 8 );
  QCOMPARE( widget->minimumScale(), 10.0 );
  QCOMPARE( widget->maximumScale(), 8.0 );

  widget->setScaleRange( 4, 2 );
  QCOMPARE( widget->minimumScale(), 4.0 );
  QCOMPARE( widget->maximumScale(), 2.0 );

  // TODO: test passing min > max

}

QGSTEST_MAIN( TestQgsScaleRangeWidget )
#include "testqgsscalerangewidget.moc"
