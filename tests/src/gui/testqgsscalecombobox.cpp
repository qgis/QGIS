/***************************************************************************
                         testqgsscalecombobox.cpp
                         ---------------------------
    begin                : September 2012
    copyright            : (C) 2012 by Magnus Homann
    email                : magnus at homann dot se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsscalecombobox.h"
#include <QObject>
#include <QLineEdit>
#include <QComboBox>
#include <QtTest>

class TestQgsScaleComboBox: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void basic();
  private:
    QgsScaleComboBox *mScaleEdit;
};

void TestQgsScaleComboBox::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Create a combobox, and init with predefined scales.
  mScaleEdit = new QgsScaleComboBox();
};

void TestQgsScaleComboBox::cleanupTestCase()
{
  delete mScaleEdit;
};

void TestQgsScaleComboBox::init()
{
};

void TestQgsScaleComboBox::basic()
{
  mScaleEdit->lineEdit()->setText( "" );
  QTest::keyClicks( mScaleEdit->lineEdit(), "1:2345");
  QCOMPARE( mScaleEdit->lineEdit()->text(), QString("1:2345"));
};
 
void TestQgsScaleComboBox::cleanup()
{
};

QTEST_MAIN( TestQgsScaleComboBox )
#include "moc_testqgsscalecombobox.cxx"
