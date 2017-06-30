/***************************************************************************
                         testqgslayoutitemregistry.cpp
                         -----------------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemregistry.h"
#include "qgis.h"
#include <QObject>
#include <QtTest>

class TestQgsLayoutItemRegistry : public QObject
{
    Q_OBJECT;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void createInstance(); // create global instance of QgsLayoutItemRegistry

  private:

};

void TestQgsLayoutItemRegistry::initTestCase()
{

}

void TestQgsLayoutItemRegistry::cleanupTestCase()
{

}

void TestQgsLayoutItemRegistry::init()
{

}

void TestQgsLayoutItemRegistry::cleanup()
{

}

void TestQgsLayoutItemRegistry::createInstance()
{
  QgsLayoutItemRegistry *registry = QgsLayoutItemRegistry::instance();
  QVERIFY( registry );
}

QTEST_MAIN( TestQgsLayoutItemRegistry )
#include "testqgslayoutitemregistry.moc"
