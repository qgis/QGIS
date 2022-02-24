/***************************************************************************
    testqgsadvanceddigitizingdockwidget.cpp
     --------------------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCoreApplication>
#include <QLocale>

#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsadvanceddigitizingdockwidget.h"

class TestQgsAdvancedDigitizingDockWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsAdvancedDigitizingDockWidget() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void parseUserInput();

};

void TestQgsAdvancedDigitizingDockWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsAdvancedDigitizingDockWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAdvancedDigitizingDockWidget::init()
{
}

void TestQgsAdvancedDigitizingDockWidget::cleanup()
{
}

void TestQgsAdvancedDigitizingDockWidget::parseUserInput()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  QgsAdvancedDigitizingDockWidget widget{ &canvas };

  bool ok;
  double result;

  QLocale::setDefault( QLocale::English );

  result = widget.parseUserInput( QStringLiteral( "1.2345" ), ok );
  QCOMPARE( result, 1.2345 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1,234.5" ), ok );
  QCOMPARE( result, 1234.5 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1,200.6/2" ), ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1200.6/2" ), ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  QLocale::setDefault( QLocale::Italian );

  result = widget.parseUserInput( QStringLiteral( "1,2345" ), ok );
  QCOMPARE( result, 1.2345 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1.234,5" ), ok );
  QCOMPARE( result, 1234.5 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1.200,6/2" ), ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1200,6/2" ), ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

}

QGSTEST_MAIN( TestQgsAdvancedDigitizingDockWidget )
#include "testqgsadvanceddigitizingdockwidget.moc"
