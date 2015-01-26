/***************************************************************************
    testqgsgui.cpp
     --------------------------------------
    Date                 : 26.1.2015
    Copyright            : (C) 2015 Michael Kirk
    Email                : michael at jackpine dot me
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include <qgisgui.h>

class TestQgsGui : public QObject
{
    Q_OBJECT
  private slots:
    void createFileFilter();

};

void TestQgsGui::createFileFilter()
{
  QString expected = "FOO format (*.foo *.FOO)";
  QString actual = QgisGui::createFileFilter_("foo");;

  QCOMPARE( actual, expected );
}

QTEST_MAIN( TestQgsGui )
#include "testqgsgui.moc"
