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

#include "qgsconfig.h"

#include "qgisapp.h"
#include "qgsprocessingregistry.h"
#include "qgstest.h"

#include <QString>

#ifdef HAVE_3D
#include "qgs3dmapcanvaswidget.h"
#include "qgsproject.h"
#include "qgsprojectviewsettings.h"
#include "qgsreferencedgeometry.h"
#endif

#include <QApplication>
#include <QSplashScreen>
#include <QString>
#include <QToolBar>

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
#ifdef HAVE_3D
    void valid3DWidgetNames();
#endif

  private:
    //! Walks \a widgets and their menus, checking every action carries an objectName
    void checkWidgetNames( QList<QPair<QString, const QWidget *>> widgets );

    QgisApp *mQgisApp = nullptr;
};


TestQgisAppWidgetNames::TestQgisAppWidgetNames()
  : QgsTest( u"Test QGIS application menus and actions have valid objectName"_s )
{}

//runs before all tests
void TestQgisAppWidgetNames::initTestCase()
{
  mQgisApp = new QgisApp( new QSplashScreen() );
}

//runs after all tests
void TestQgisAppWidgetNames::cleanupTestCase()
{
  // Hack to avoid an issue when deleting the application
  QgsApplication::processingRegistry()->blockSignals( true );
}

void TestQgisAppWidgetNames::init()
{}

void TestQgisAppWidgetNames::cleanup()
{}

void TestQgisAppWidgetNames::validWidgetNames()
{
  QList<QPair<QString, const QWidget *>> widgets = { { "/MainWindow/menuBar", mQgisApp->menuBar() } };

  const QList<QToolBar *> toolBars = mQgisApp->findChildren<QToolBar *>( Qt::FindDirectChildrenOnly );
  for ( const QToolBar *toolBar : toolBars )
  {
    QVERIFY2( !toolBar->objectName().isEmpty(), qPrintable( u"QToolBar has no objectName"_s ) );
    widgets << QPair<QString, const QWidget *> { "/MainWindow/" + toolBar->objectName(), toolBar };
  }

  checkWidgetNames( widgets );
}

#ifdef HAVE_3D

void TestQgisAppWidgetNames::valid3DWidgetNames()
{
  // a 3D map view refuses to open without a valid project extent
  QgsProject::instance()->viewSettings()->setPresetFullExtent( QgsReferencedRectangle( QgsRectangle( 0, 0, 100, 100 ), QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) ) );

  QVERIFY( mQgisApp->createNewMapCanvas3D( u"Test 3D"_s, Qgis::SceneMode::Local ) );

  Qgs3DMapCanvasWidget *canvas3D = nullptr;
  const QList<Qgs3DMapCanvasWidget *> canvases3D = mQgisApp->findChildren<Qgs3DMapCanvasWidget *>();
  for ( Qgs3DMapCanvasWidget *candidate : canvases3D )
  {
    if ( candidate->canvasName() == "Test 3D"_L1 )
      canvas3D = candidate;
  }
  // the widget is not necessarily parented to the main window -- it lives in a dock or its own dialog
  if ( !canvas3D )
  {
    const QList<QWidget *> allWidgets = QApplication::allWidgets();
    for ( QWidget *widget : allWidgets )
    {
      if ( Qgs3DMapCanvasWidget *candidate = qobject_cast<Qgs3DMapCanvasWidget *>( widget ) )
      {
        if ( candidate->canvasName() == "Test 3D"_L1 )
          canvas3D = candidate;
      }
    }
  }
  QVERIFY2( canvas3D, "could not find the newly created Qgs3DMapCanvasWidget" );

  QList<QPair<QString, const QWidget *>> widgets;
  const QList<QToolBar *> toolBars = canvas3D->findChildren<QToolBar *>();
  QVERIFY( !toolBars.isEmpty() );
  for ( const QToolBar *toolBar : toolBars )
  {
    QVERIFY2( !toolBar->objectName().isEmpty(), qPrintable( u"Qgs3DMapCanvasWidget QToolBar has no objectName"_s ) );
    widgets << QPair<QString, const QWidget *> { "/3DMapView/" + toolBar->objectName(), toolBar };
  }

  checkWidgetNames( widgets );

  mQgisApp->close3DMapView( u"Test 3D"_s );
}

#endif

void TestQgisAppWidgetNames::checkWidgetNames( QList<QPair<QString, const QWidget *>> widgets )
{
  while ( !widgets.isEmpty() )
  {
    QPair<QString, const QWidget *> pathAndwidget = widgets.takeFirst();
    const QString path = pathAndwidget.first;
    const QWidget *widget = pathAndwidget.second;
    const QList<QAction *> &actions = widget->actions();
    for ( QAction *action : actions )
    {
      if ( QMenu *menu = action->menu() )
      {
        // We don't care that actions from this menu have correct objectName because they are
        // already removed from the menu when the associated dock widget is hidden
        if ( menu == mQgisApp->panelMenu() || menu == mQgisApp->toolBarMenu() )
          continue;

        // Dump parent object tree with visual missing label to help in finding the culprite:
        if ( menu->objectName().isEmpty() && menu->parent() != nullptr )
        {
          menu->setObjectName( "===========THIS_OBJECT_HAS_NO_NAME============" );
          menu->parent()->dumpObjectTree();
        }
        QVERIFY2( !menu->objectName().isEmpty(), qPrintable( u"'%1' %2 has a QMenu with no objectName"_s.arg( path ).arg( menu->parent()->metaObject()->className() ) ) );
        widgets << QPair<QString, const QWidget *> { path + "/" + menu->objectName(), menu };
      }
      else
      {
        // Dump parent object tree with visual missing label to help in finding the culprite:
        if ( !action->isSeparator() && action->objectName().isEmpty() && action->parent() != nullptr )
        {
          action->setObjectName( "===========THIS_OBJECT_HAS_NO_NAME============" );
          action->parent()->dumpObjectTree();
        }
        QVERIFY2(
          action->isSeparator() || !action->objectName().isEmpty(),
          qPrintable( u"'%1' %2 has a %3 with no objectName"_s.arg( path ).arg( action->parent()->metaObject()->className() ).arg( action->metaObject()->className() ) )
        );
      }
    }
  }
}

QGSTEST_MAIN( TestQgisAppWidgetNames )
#include "testqgisappwidgetnames.moc"
