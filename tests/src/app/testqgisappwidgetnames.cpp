/***************************************************************************
    testqgisappwidgetnames.cpp
    ---------------------
    begin                : 2026/01/21
    copyright            : (C) 2026 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsprocessingregistry.h"
#include "qgstest.h"

#include <QApplication>
#include <QSplashScreen>
#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * Test QGIS application menus and actions have valid (non empty) objectName
 * so they can then be used in customization
 */
class TestQgisAppWidgetNames : public QgsTest
{
    Q_OBJECT

  public:
    TestQgisAppWidgetNames();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void validWidgetNames();

  private:
    QgisApp *mQgisApp = nullptr;
};


TestQgisAppWidgetNames::TestQgisAppWidgetNames()
  : QgsTest( u"Test QGIS application menus and actions have valid objectName"_s )
{}

//runs before all tests
void TestQgisAppWidgetNames::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  mQgisApp = new QgisApp( new QSplashScreen() );
}

//runs after all tests
void TestQgisAppWidgetNames::cleanupTestCase()
{
  // Hack to avoid an issue when deleting the application
  QgsApplication::processingRegistry()->blockSignals( true );
}

void TestQgisAppWidgetNames::init()
{
}

void TestQgisAppWidgetNames::cleanup()
{
}

void TestQgisAppWidgetNames::validWidgetNames()
{
  QList<QPair<QString, const QWidget *>> widgets = { { "/MainWindow/menuBar", mQgisApp->menuBar() } };

  const QList<QToolBar *> toolBars = mQgisApp->findChildren<QToolBar *>( Qt::FindDirectChildrenOnly );
  for ( const QToolBar *toolBar : toolBars )
  {
    QVERIFY2( !toolBar->objectName().isEmpty(), qPrintable( u"QToolBar has no objectName"_s ) );
    widgets << QPair<QString, const QWidget *> { "/MainWindow/" + toolBar->objectName(), toolBar };
  }

  while ( !widgets.isEmpty() )
  {
    QPair<QString, const QWidget *> pathAndwidget = widgets.takeFirst();
    const QString path = pathAndwidget.first;
    const QWidget *widget = pathAndwidget.second;
    const QList<QAction *> &actions = widget->actions();
    for ( const QAction *action : actions )
    {
      if ( QMenu *menu = action->menu() )
      {
        // We don't care that actions from this menu have correct objectName because they are
        // already removed from the menu when the associated dock widget is hidden
        if ( menu == mQgisApp->panelMenu() || menu == mQgisApp->toolBarMenu() )
          continue;

        QVERIFY2( !menu->objectName().isEmpty(), qPrintable( u"'%1' %2 has a QMenu with no objectName"_s.arg( path ).arg( menu->parent()->metaObject()->className() ) ) );
        widgets << QPair<QString, const QWidget *> { path + "/" + menu->objectName(), menu };
      }
      else
      {
        QVERIFY2( action->isSeparator() || !action->objectName().isEmpty(), qPrintable( u"'%1' %2 has a %3 with no objectName"_s.arg( path ).arg( action->parent()->metaObject()->className() ).arg( action->metaObject()->className() ) ) );
      }
    }
  }
}

QGSTEST_MAIN( TestQgisAppWidgetNames )
#include "testqgisappwidgetnames.moc"
