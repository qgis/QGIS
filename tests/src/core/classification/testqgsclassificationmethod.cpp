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
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationmethod.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * This is a unit test for the qgsClassificationMethod class.
 */

class TestQgsClassificationMethod : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void labelForRange();
};

void TestQgsClassificationMethod::initTestCase()
{
}

void TestQgsClassificationMethod::cleanupTestCase()
{
}

void TestQgsClassificationMethod::init()
{
}

void TestQgsClassificationMethod::cleanup()
{
}

void TestQgsClassificationMethod::labelForRange()
{
  QgsClassificationEqualInterval method;

  QString label = method.labelForRange( 0.0, 10.0, QgsClassificationMethod::LowerBound );
  QCOMPARE( label, QString( u"0 ≤ x ≤ 10"_s ) );

  label = method.labelForRange( 0.0, 10.0, QgsClassificationMethod::Inner );
  QCOMPARE( label, QString( u"0 < x ≤ 10"_s ) );

  label = method.labelForRange( 0.0, 10.0, QgsClassificationMethod::UpperBound );
  QCOMPARE( label, QString( u"0 < x ≤ 10"_s ) );
}

QGSTEST_MAIN( TestQgsClassificationMethod )
#include "testqgsclassificationmethod.moc"
