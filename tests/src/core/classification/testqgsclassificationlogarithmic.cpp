/***************************************************************************
     testqgsclassificationlogarithmic.cpp
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
#include "qgsclassificationlogarithmic.h"
#include "qgsclassificationmethod.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

/**
 * \ingroup UnitTests
 * This is a unit test for the qgsClassificationLogarithmic class.
 */

class TestQgsClassificationLogarithmic : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void labelForRange();
};

void TestQgsClassificationLogarithmic::initTestCase()
{
}

void TestQgsClassificationLogarithmic::cleanupTestCase()
{
}

void TestQgsClassificationLogarithmic::init()
{
}

void TestQgsClassificationLogarithmic::cleanup()
{
}

void TestQgsClassificationLogarithmic::labelForRange()
{
  QgsClassificationLogarithmic method;

  QString label = method.labelForRange( 1.5, 100.0, QgsClassificationMethod::LowerBound );
  QCOMPARE( label, QString::fromUtf8( "1.5 ≤ x ≤ 10^2" ) );

  label = method.labelForRange( 100.0, 1000.0, QgsClassificationMethod::Inner );
  QCOMPARE( label, QString::fromUtf8( "10^2 < x ≤ 10^3" ) );

  label = method.labelForRange( 1000.0, 10000.0, QgsClassificationMethod::UpperBound );
  QCOMPARE( label, QString::fromUtf8( "10^3 < x ≤ 10^4" ) );
}

QGSTEST_MAIN( TestQgsClassificationLogarithmic )
#include "testqgsclassificationlogarithmic.moc"
