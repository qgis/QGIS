/***************************************************************************
    testqgshtmlwidgetwrapper.cpp
     --------------------------------------
    Date                 : September 2020
    Copyright            : (C) 2020 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editorwidgets/core/qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshtmlwidgetwrapper.h"
#include "qgstest.h"

class TestQgsHtmlWidgetWrapper : public QObject
{
    Q_OBJECT
  public:
    TestQgsHtmlWidgetWrapper() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
};

void TestQgsHtmlWidgetWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsHtmlWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsHtmlWidgetWrapper::init()
{
}

void TestQgsHtmlWidgetWrapper::cleanup()
{
}

QGSTEST_MAIN( TestQgsHtmlWidgetWrapper )
#include "testqgshtmlwidgetwrapper.moc"
