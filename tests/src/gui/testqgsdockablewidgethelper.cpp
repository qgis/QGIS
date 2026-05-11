/***************************************************************************
    testqgsdockablewidgethelper.cpp
     ----------------------
    Date                 : May 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdockablewidgethelper.h"
#include "qgsdockwidget.h"
#include "qgstest.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsDockableWidgetHelper : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testBasicCreation();
    void testToggleDockMode();
    void testTitleAndName();
    void testUserVisible();
    void testSignals();
    void testActionAndToolButton();
    void testXmlSerialization();

  private:
};

void TestQgsDockableWidgetHelper::initTestCase()
{}

void TestQgsDockableWidgetHelper::cleanupTestCase()
{}

void TestQgsDockableWidgetHelper::init()
{}

void TestQgsDockableWidgetHelper::cleanup()
{}

void TestQgsDockableWidgetHelper::testBasicCreation()
{
  QMainWindow mw;
  QWidget w;

  // test forcing docked mode
  {
    QgsDockableWidgetHelper helperDocked( u"Test Dock"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );

    QVERIFY( helperDocked.isDocked() );
    QVERIFY( helperDocked.dockWidget() );
    QVERIFY( !helperDocked.dialog() );
    QCOMPARE( helperDocked.widget(), &w );
  }

  // test forcing dialog mode
  {
    QgsDockableWidgetHelper helperDialog( u"Test Dialog"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDialog );

    QVERIFY( !helperDialog.isDocked() );
    QVERIFY( !helperDialog.dockWidget() );
    QVERIFY( helperDialog.dialog() );
    QCOMPARE( helperDialog.widget(), &w );
  }
}

void TestQgsDockableWidgetHelper::testToggleDockMode()
{
  QMainWindow mw;
  QWidget w;

  QgsDockableWidgetHelper helper( u"Test"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );
  QVERIFY( helper.isDocked() );
  QVERIFY( helper.dockWidget() );

  // switch to dialog
  helper.toggleDockMode( false );
  QVERIFY( !helper.isDocked() );
  QVERIFY( !helper.dockWidget() );
  QVERIFY( helper.dialog() );

  // back to docked
  helper.toggleDockMode( true );
  QVERIFY( helper.isDocked() );
  QVERIFY( helper.dockWidget() );
  QVERIFY( !helper.dialog() );
}

void TestQgsDockableWidgetHelper::testTitleAndName()
{
  QMainWindow mw;
  QWidget w;

  QgsDockableWidgetHelper helper( u"Initial Title"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );

  QCOMPARE( helper.windowTitle(), u"Initial Title"_s );
  QCOMPARE( helper.dockWidget()->windowTitle(), u"Initial Title"_s );

  helper.setWindowTitle( u"New Title"_s );
  QCOMPARE( helper.windowTitle(), u"New Title"_s );
  QCOMPARE( helper.dockWidget()->windowTitle(), u"New Title"_s );

  helper.setDockObjectName( u"MyDock"_s );
  QCOMPARE( helper.dockObjectName(), u"MyDock"_s );
  QCOMPARE( helper.dockWidget()->objectName(), u"MyDock"_s );

  helper.toggleDockMode( false );
  QCOMPARE( helper.dialog()->windowTitle(), u"New Title"_s );
  QCOMPARE( helper.dialog()->objectName(), u"MyDock"_s );

  helper.toggleDockMode( true );
  QCOMPARE( helper.dockWidget()->windowTitle(), u"New Title"_s );
  QCOMPARE( helper.dockWidget()->objectName(), u"MyDock"_s );
}

void TestQgsDockableWidgetHelper::testUserVisible()
{
  QMainWindow mw;
  QWidget w;

  QgsDockableWidgetHelper helper( u"Test"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );
  mw.show();

  // set user visible in dock mode
  helper.setUserVisible( true );
  QVERIFY( helper.isUserVisible() );
  helper.setUserVisible( false );
  QVERIFY( !helper.isUserVisible() );

  // test in dialog mode
  helper.toggleDockMode( false );
  helper.setUserVisible( true );
  QVERIFY( helper.isUserVisible() );
  helper.setUserVisible( false );
  QVERIFY( !helper.isUserVisible() );
}

void TestQgsDockableWidgetHelper::testSignals()
{
  QMainWindow mw;
  QWidget w;

  QgsDockableWidgetHelper helper( u"Test"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );
  mw.show();

  QSignalSpy spyDockMode( &helper, &QgsDockableWidgetHelper::dockModeToggled );
  QSignalSpy spyVisibility( &helper, &QgsDockableWidgetHelper::visibilityChanged );
  QSignalSpy spyClosed( &helper, &QgsDockableWidgetHelper::closed );

  // test in dock mode first
  QVERIFY( helper.isUserVisible() );
  helper.setUserVisible( true );
  QCOMPARE( spyVisibility.count(), 0 );
  helper.setUserVisible( false );
  QCOMPARE( spyVisibility.count(), 1 );
  QCOMPARE( spyVisibility.last().at( 0 ).toBool(), false );
  helper.setUserVisible( true );
  QCOMPARE( spyVisibility.count(), 2 );
  QCOMPARE( spyVisibility.last().at( 0 ).toBool(), true );

  helper.toggleDockMode( false );
  QCOMPARE( spyDockMode.count(), 1 );
  QCOMPARE( spyDockMode.last().at( 0 ).toBool(), false );
  QVERIFY( helper.isUserVisible() );
  spyVisibility.clear();
  helper.setUserVisible( false );
  QCOMPARE( spyVisibility.count(), 1 );
  QCOMPARE( spyVisibility.last().at( 0 ).toBool(), false );
  helper.setUserVisible( true );
  QCOMPARE( spyVisibility.count(), 2 );
  QCOMPARE( spyVisibility.last().at( 0 ).toBool(), true );

  helper.reject();
  QCOMPARE( spyClosed.count(), 1 );
}

void TestQgsDockableWidgetHelper::testActionAndToolButton()
{
  QMainWindow mw;
  QWidget w;

  QgsDockableWidgetHelper helper( u"Test"_s, &w, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );

  // test action
  QAction *toggleAction = helper.createDockUndockAction( u"Toggle Dock"_s, &mw );
  QVERIFY( toggleAction->isCheckable() );
  QVERIFY( toggleAction->isChecked() );

  toggleAction->trigger();
  QVERIFY( !helper.isDocked() );
  QVERIFY( !toggleAction->isChecked() );

  // test toolbutton
  QToolButton *toolButton = helper.createDockUndockToolButton();
  toolButton->setParent( &mw );
  QVERIFY( toolButton->isCheckable() );
  QVERIFY( !toolButton->isChecked() );

  // clicking the button should re-dock it
  toolButton->click();
  QVERIFY( helper.isDocked() );
  QVERIFY( toolButton->isChecked() );
}

void TestQgsDockableWidgetHelper::testXmlSerialization()
{
  QMainWindow mw;
  QWidget w1;
  QWidget w2;

  QgsDockableWidgetHelper sourceHelper( u"Source"_s, &w1, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );

  QDomDocument doc( u"test"_s );

  // test round trip in dock mode
  sourceHelper.toggleDockMode( true );

  QDomElement dockedRoot = doc.createElement( u"docked_state"_s );
  sourceHelper.writeXml( dockedRoot );

  // starting in dialog mode!
  QgsDockableWidgetHelper restoreHelperDocked( u"RestoreDocked"_s, &w2, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDocked );
  restoreHelperDocked.readXml( dockedRoot );
  QVERIFY( restoreHelperDocked.isDocked() );
  QVERIFY( restoreHelperDocked.dockWidget() );
  QCOMPARE( restoreHelperDocked.dockWidget()->geometry(), sourceHelper.dockWidget()->geometry() );

  // test round trip in dialog mode
  sourceHelper.toggleDockMode( false );

  QDomElement dialogRoot = doc.createElement( u"dialog_state"_s );
  sourceHelper.writeXml( dialogRoot );

  QgsDockableWidgetHelper restoreHelperDialog( u"RestoreDialog"_s, &w2, &mw, QString(), QStringList(), QgsDockableWidgetHelper::OpeningMode::ForceDialog );
  restoreHelperDialog.readXml( dialogRoot );

  QVERIFY( !restoreHelperDialog.isDocked() );
  QVERIFY( restoreHelperDialog.dialog() );
  QCOMPARE( restoreHelperDialog.dialog()->geometry(), sourceHelper.dialog()->geometry() );
}

QGSTEST_MAIN( TestQgsDockableWidgetHelper )
#include "testqgsdockablewidgethelper.moc"
