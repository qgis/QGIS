/***************************************************************************
    testqgsqueryresultwidget.cpp
     ----------------------
    Date                 : Jan 2021
    Copyright            : (C) 2021 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsqueryresultwidget.h"
#include <QApplication>
#include <QMainWindow>
#include <QAction>

class TestQgsQueryResultWidget: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

  private:

};

void TestQgsQueryResultWidget::initTestCase()
{
}

void TestQgsQueryResultWidget::cleanupTestCase()
{
}

void TestQgsQueryResultWidget::init()
{
}

void TestQgsQueryResultWidget::cleanup()
{
}

QGSTEST_MAIN( TestQgsQueryResultWidget )
#include "testqgsqueryresultwidget.moc"
