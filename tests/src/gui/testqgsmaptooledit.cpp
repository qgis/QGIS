/***************************************************************************
    testqgsmaptooledit.cpp
     --------------------------------------
    Date                 : 6.2.2017
    Copyright            : (C) 2017 Alexander Lisovenko
    Email                : alexander.lisovenko@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCoreApplication>

#include "qgstest.h"
#include "qgsguiutils.h"
#include "qgsmaptooledit.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgssettings.h"

class TestQgsMapToolEdit : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolEdit() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void checkDefaultZValue();

  private:
    QgsMapCanvas *mCanvas = nullptr;

};

void TestQgsMapToolEdit::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapToolEdit::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolEdit::init()
{
  mCanvas = new QgsMapCanvas();
}

void TestQgsMapToolEdit::cleanup()
{
  delete mCanvas;
}

void TestQgsMapToolEdit::checkDefaultZValue()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "/qgis/digitizing/default_z_value" ) );

  QgsMapToolEdit *tool = new QgsMapToolEdit( mCanvas );
  QCOMPARE( tool->defaultZValue(), Qgis::DEFAULT_Z_COORDINATE );

  double z_value_for_test = Qgis::DEFAULT_Z_COORDINATE + 1;
  settings.setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), z_value_for_test );

  QCOMPARE( tool->defaultZValue(), z_value_for_test );
}

QGSTEST_MAIN( TestQgsMapToolEdit )
#include "testqgsmaptooledit.moc"
