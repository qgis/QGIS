/***************************************************************************
     qgsclassificationstandarddeviation.cpp
     --------------------------------------
    Date                 : Jan 2026
    Copyright            : (C) 2026 by Hiroya Hoshihara
    Email                : hiroya.hoshihara at gmail dot com
     ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsclassificationmethod.h"
#include "qgsclassificationstandarddeviation.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

/**
 * \ingroup UnitTests
 * This is a unit test for the qgsClassificationStandardDeviation class.
 */

class TestQgsClassificationStandardDeviation : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void labelForRange();
};

void TestQgsClassificationStandardDeviation::initTestCase()
{
}

void TestQgsClassificationStandardDeviation::cleanupTestCase()
{
}

void TestQgsClassificationStandardDeviation::init()
{
}

void TestQgsClassificationStandardDeviation::cleanup()
{
}

void TestQgsClassificationStandardDeviation::labelForRange()
{
  QgsClassificationStandardDeviation method;

  QList<double> values = { 30.0, 40.0, 50.0, 60.0, 70.0 };
  QString error;
  double minimum = 30.0;
  double maximum = 70.0;
  method.calculateBreaks( minimum, maximum, values, 3, error );

  QString expectedLower = method.valueToLabel( 40.0 );
  QString expectedUpper = method.valueToLabel( 50.0 );
  QString expected = QString::fromUtf8( "%1 < x ≤ %2" ).arg( expectedLower, expectedUpper );

  QString label = method.labelForRange( 40.0, 50.0, QgsClassificationMethod::Inner );
  QCOMPARE( label, expected );
}

QGSTEST_MAIN( TestQgsClassificationStandardDeviation )
#include "testqgsclassificationstandarddeviation.moc"
