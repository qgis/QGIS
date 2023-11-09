/***************************************************************************
     testqgsappdockwidgets.cpp
     --------------------------------------
    Date                 : April 2020
    Copyright            : (C) 2020 by Germán Carrillo
    Email                : gcarrillo@linuxmail.org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QApplication>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDockWidget>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsdockwidget.h"
#include "qgsmessagelog.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgisApp dock widget methods.
 */
class TestQgisAppDockWidgets : public QObject
{
    Q_OBJECT

  public:
    TestQgisAppDockWidgets();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void tabifiedQDockWidgetEmptyArea();
    void tabifiedQgsDockWidgetEmptyArea();
    void tabifiedQDockWidgetOneExisting();
    void tabifiedQDockWidgetOneExistingRaiseTab();
    void tabifiedQgsDockWidgetOneExisting();
    void tabifiedQgsDockWidgetOneExistingRaiseTab();
    void tabifiedQDockWidgetTwoExisting();
    void tabifiedQDockWidgetTwoExistingRaiseTab();
    void tabifiedQDockWidgetTwoExistingOneHidden();
    void tabifiedQDockWidgetTwoExistingOneHiddenRaiseTab();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgisAppDockWidgets::TestQgisAppDockWidgets() = default;

//runs before all tests
void TestQgisAppDockWidgets::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  qDebug() << "TestQgisAppDockWidgets::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  mQgisApp->show();
}

//runs after all tests
void TestQgisAppDockWidgets::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgisAppDockWidgets::init()
{
  // Clear left area
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( mQgisApp->dockWidgetArea( dock ) == Qt::LeftDockWidgetArea )
    {
      mQgisApp->removeDockWidget( dock );
    }
  }
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetEmptyArea()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetEmptyArea()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  QDockWidget *dw = new QDockWidget();
  mQgisApp->addTabifiedDockWidget( area, dw );
  QVERIFY( dw->isVisible() );

  int count = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 1 );
}

void TestQgisAppDockWidgets::tabifiedQgsDockWidgetEmptyArea()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQgsDockWidgetEmptyArea()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  QgsDockWidget *dw = new QgsDockWidget();
  mQgisApp->addTabifiedDockWidget( area, dw );
  QVERIFY( dw->isUserVisible() );
  QVERIFY( dw->isVisible() );

  int count = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 1 );
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetOneExisting()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetOneExisting()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add a base dock widget to the area
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  QVERIFY( mLayerTreeDock->isVisible() );

  // Tabify our dock widget
  const QString dockName = QStringLiteral( "QDockWidget 1" );
  QDockWidget *dw = new QDockWidget( dockName );
  mQgisApp->addTabifiedDockWidget( area, dw );
  QVERIFY( dw->isVisible() );

  // Check our dock widget is tabified
  const QList< QDockWidget *> tdw = mQgisApp->tabifiedDockWidgets( mLayerTreeDock );
  QCOMPARE( tdw.length(), 1 );
  QCOMPARE( tdw.at( 0 ), dw );

  // Check there are 2 visible dock widgets in that area
  int count = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 2 );

  // Check our dock widget's tab is NOT the one that is active
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName )
      {
        dwFound = true;
        QVERIFY( tabBar->currentIndex() != i );
        break;
      }
    }
    if ( dwFound )
    {
      break;
    }
  }
  QVERIFY( dwFound );
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetOneExistingRaiseTab()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetOneExistingRaiseTab()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add a base dock widget to the area
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  QVERIFY( mLayerTreeDock->isVisible() );

  // Tabify our dock widget
  const QString dockName = QStringLiteral( "QDockWidget 1" );
  QDockWidget *dw = new QDockWidget( dockName );
  mQgisApp->addTabifiedDockWidget( area, dw, QStringList(), true );
  QVERIFY( dw->isVisible() );

  // Check our dock widget is tabified
  const QList< QDockWidget *> tdw = mQgisApp->tabifiedDockWidgets( mLayerTreeDock );
  QCOMPARE( tdw.length(), 1 );
  QCOMPARE( tdw.at( 0 ), dw );

  // Check there are 2 visible dock widgets in that area
  int count = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 2 );

  // Check our dock widget's tab is the one that is active
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName )
      {
        dwFound = true;
        QCOMPARE( tabBar->currentIndex(), i );
        break;
      }
    }
    if ( dwFound )
    {
      break;
    }
  }
  QVERIFY( dwFound );
}

void TestQgisAppDockWidgets::tabifiedQgsDockWidgetOneExisting()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQgsDockWidgetOneExisting()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add a base dock widget to the area
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  QVERIFY( mLayerTreeDock->isVisible() );

  // Tabify our dock widget
  const QString dockName = QStringLiteral( "QgsDockWidget 1" );
  QgsDockWidget *dw = new QgsDockWidget( dockName );
  mQgisApp->addTabifiedDockWidget( area, dw );
  QVERIFY( dw->isVisible() );

  // Check our dock widget is tabified
  const QList< QDockWidget *> tdw = mQgisApp->tabifiedDockWidgets( mLayerTreeDock );
  QCOMPARE( tdw.length(), 1 );
  QCOMPARE( tdw.at( 0 ), dw );

  // Check there are 2 visible dock widgets in that area
  int count = 0;
  const QList<QgsDockWidget *> docks = mQgisApp->findChildren<QgsDockWidget *>();
  for ( QgsDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 2 );

  // Check our dock widget's tab is NOT the one that is active
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName )
      {
        dwFound = true;
        QVERIFY( tabBar->currentIndex() != i );
        break;
      }
    }
    if ( dwFound )
    {
      break;
    }
  }
  QVERIFY( dwFound );
}

void TestQgisAppDockWidgets::tabifiedQgsDockWidgetOneExistingRaiseTab()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQgsDockWidgetOneExistingRaiseTab()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add a base dock widget to the area
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  QVERIFY( mLayerTreeDock->isVisible() );

  // Tabify our dock widget
  const QString dockName = QStringLiteral( "QgsDockWidget 1" );
  QgsDockWidget *dw = new QgsDockWidget( dockName );
  mQgisApp->addTabifiedDockWidget( area, dw, QStringList(), true );
  QVERIFY( dw->isVisible() );

  // Check our dock widget is tabified
  const QList< QDockWidget *> tdw = mQgisApp->tabifiedDockWidgets( mLayerTreeDock );
  QCOMPARE( tdw.length(), 1 );
  QCOMPARE( tdw.at( 0 ), dw );

  // Check there are 2 visible dock widgets in that area
  int count = 0;
  const QList<QgsDockWidget *> docks = mQgisApp->findChildren<QgsDockWidget *>();
  for ( QgsDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 2 );

  // Check our dock widget's tab is the one that is active
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName )
      {
        dwFound = true;
        QCOMPARE( tabBar->currentIndex(), i );
        break;
      }
    }
    if ( dwFound )
    {
      break;
    }
  }
  QVERIFY( dwFound );
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExisting()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExisting()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add 2 base dock widgets to the area
  const QString objectName1 = QStringLiteral( "Layers" );
  const QString objectName2 = QStringLiteral( "LayerOrder" );
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mLayerTreeDock->setObjectName( objectName1 );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  QVERIFY( mLayerTreeDock->isVisible() );
  QgsDockWidget *mLayerOrderDock = new QgsDockWidget( tr( "Layer Order" ), mQgisApp );
  mLayerOrderDock->setObjectName( objectName2 );
  mQgisApp->addDockWidget( area, mLayerOrderDock );
  QVERIFY( mLayerOrderDock->isVisible() );

  // Check that they are not tabified
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).isEmpty() );
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).isEmpty() );

  // Tabify our 1st dock widget (picking 1st priority)
  const QString dockName1 = QStringLiteral( "QDockWidget 1" );
  QDockWidget *dw1 = new QDockWidget( dockName1 );
  mQgisApp->addTabifiedDockWidget( area, dw1, QStringList() << objectName1 );
  QVERIFY( dw1->isVisible() );

  // Which one is tabified now?
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).length(), 1 );
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).isEmpty() );

  // Tabify our 2nd dock widget (picking 1st priority)
  const QString dockName2 = QStringLiteral( "QDockWidget 2" );
  QDockWidget *dw2 = new QDockWidget( dockName2 );
  mQgisApp->addTabifiedDockWidget( area, dw2, QStringList() << objectName2 );
  QVERIFY( dw2->isVisible() );

  // Check tabified docks
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).length(), 1 );
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).length(), 1 );

  // Tabify our 3rd dock widget (picking 2nd priority)
  const QString dockName3 = QStringLiteral( "QDockWidget 3" );
  QDockWidget *dw3 = new QDockWidget( dockName3 );
  mQgisApp->addTabifiedDockWidget( area, dw3, QStringList() << QStringLiteral( "Foo" ) << objectName2 );
  QVERIFY( dw3->isVisible() );

  // Check tabified docks
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).length(), 1 );
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).length(), 2 );

  // Check that we have 5 dock widgets now in that area
  int count = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 5 );

  // Check active tabs: should be Layers and Layer Order dock widgets
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound1 = false;
  bool dwFound2 = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == "Layers" )
      {
        dwFound1 = true;
        QCOMPARE( tabBar->currentIndex(), i );
      }
      else if ( tabBar->tabText( i ) == "Layer Order" )
      {
        dwFound2 = true;
        QCOMPARE( tabBar->currentIndex(), i );
      }
    }
    if ( dwFound1 && dwFound2 )
    {
      break;
    }
  }
  QVERIFY( dwFound1 );
  QVERIFY( dwFound2 );
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExistingRaiseTab()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExistingRaiseTab()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add 2 base dock widgets to the area
  const QString objectName1 = QStringLiteral( "Layers" );
  const QString objectName2 = QStringLiteral( "LayerOrder" );
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mLayerTreeDock->setObjectName( objectName1 );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  QVERIFY( mLayerTreeDock->isVisible() );
  QgsDockWidget *mLayerOrderDock = new QgsDockWidget( tr( "Layer Order" ), mQgisApp );
  mLayerOrderDock->setObjectName( objectName2 );
  mQgisApp->addDockWidget( area, mLayerOrderDock );
  QVERIFY( mLayerOrderDock->isVisible() );

  // Check that they are not tabified
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).isEmpty() );
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).isEmpty() );

  // Tabify our 1st dock widget (picking 1st priority)
  const QString dockName1 = QStringLiteral( "QDockWidget 1" );
  QDockWidget *dw1 = new QDockWidget( dockName1 );
  mQgisApp->addTabifiedDockWidget( area, dw1, QStringList() << objectName1, true );
  QVERIFY( dw1->isVisible() );

  // Which one is tabified now?
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).length(), 1 );
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).isEmpty() );

  // Tabify our 2nd dock widget (picking 1st priority)
  const QString dockName2 = QStringLiteral( "QDockWidget 2" );
  QDockWidget *dw2 = new QDockWidget( dockName2 );
  mQgisApp->addTabifiedDockWidget( area, dw2, QStringList() << objectName2, true );
  QVERIFY( dw2->isVisible() );

  // Check tabified docks
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).length(), 1 );
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).length(), 1 );

  // Tabify our 3rd dock widget (picking 2nd priority)
  const QString dockName3 = QStringLiteral( "QDockWidget 3" );
  QDockWidget *dw3 = new QDockWidget( dockName3 );
  mQgisApp->addTabifiedDockWidget( area, dw3, QStringList() << QStringLiteral( "Foo" ) << objectName2, true );
  QVERIFY( dw3->isVisible() );

  // Check tabified docks
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).length(), 1 );
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).length(), 2 );

  // Check that we have 5 dock widgets now in that area
  int count = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( dock->isVisible() && mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
    }
  }
  QCOMPARE( count, 5 );

  // Check active tabs: should be our 1st and 3rd dock widgets
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound1 = false;
  bool dwFound2 = false;
  bool dwFound3 = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName1 )
      {
        dwFound1 = true;
        QCOMPARE( tabBar->currentIndex(), i );
      }
      else if ( tabBar->tabText( i ) == dockName2 )
      {
        dwFound2 = true;
        QVERIFY( tabBar->currentIndex() != i );
      }
      else if ( tabBar->tabText( i ) == dockName3 )
      {
        dwFound3 = true;
        QCOMPARE( tabBar->currentIndex(), i );
      }
    }
    if ( dwFound1 && dwFound2 && dwFound3 )
    {
      break;
    }
  }
  QVERIFY( dwFound1 );
  QVERIFY( dwFound2 );
  QVERIFY( dwFound3 );
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExistingOneHidden()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExistingOneHidden()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add 2 base dock widgets to the area
  const QString objectName1 = QStringLiteral( "Layers" );
  const QString objectName2 = QStringLiteral( "LayerOrder" );
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mLayerTreeDock->setObjectName( objectName1 );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  mLayerTreeDock->hide();  // This one will be hidden
  QVERIFY( !mLayerTreeDock->isVisible() );
  QgsDockWidget *mLayerOrderDock = new QgsDockWidget( tr( "Layer Order" ), mQgisApp );
  mLayerOrderDock->setObjectName( objectName2 );
  mQgisApp->addDockWidget( area, mLayerOrderDock );
  QVERIFY( mLayerOrderDock->isVisible() );

  // Check that they are not tabified
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).isEmpty() );
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).isEmpty() );

  // Tabify our dock widget (picking 2nd priority)
  const QString dockName1 = QStringLiteral( "QDockWidget 1" );
  QDockWidget *dw1 = new QDockWidget( dockName1 );
  mQgisApp->addTabifiedDockWidget( area, dw1, QStringList() << objectName1 << objectName2 );
  QVERIFY( dw1->isVisible() );

  // Which one is tabified now?
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).isEmpty() );
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).length(), 1 );

  // Check that we have 3 dock widgets now in that area, only 2 visible
  int count = 0;
  int visibleCount = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
      if ( dock->isVisible() )
      {
        visibleCount++;
      }
    }
  }
  QCOMPARE( count, 3 );
  QCOMPARE( visibleCount, 2 );

  // Check that our dock widget is NOT active
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName1 )
      {
        dwFound = true;
        QVERIFY( tabBar->currentIndex() != i );
        break;
      }
    }
    if ( dwFound )
    {
      break;
    }
  }
  QVERIFY( dwFound );
}

void TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExistingOneHiddenRaiseTab()
{
  qDebug() << "TestQgisAppDockWidgets::tabifiedQDockWidgetTwoExistingOneHiddenRaiseTab()";
  const Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

  // Add 2 base dock widgets to the area
  const QString objectName1 = QStringLiteral( "Layers" );
  const QString objectName2 = QStringLiteral( "LayerOrder" );
  QgsDockWidget *mLayerTreeDock = new QgsDockWidget( tr( "Layers" ), mQgisApp );
  mLayerTreeDock->setObjectName( objectName1 );
  mQgisApp->addDockWidget( area, mLayerTreeDock );
  mLayerTreeDock->hide();  // This one will be hidden
  QVERIFY( !mLayerTreeDock->isVisible() );
  QgsDockWidget *mLayerOrderDock = new QgsDockWidget( tr( "Layer Order" ), mQgisApp );
  mLayerOrderDock->setObjectName( objectName2 );
  mQgisApp->addDockWidget( area, mLayerOrderDock );
  QVERIFY( mLayerOrderDock->isVisible() );

  // Check that they are not tabified
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).isEmpty() );
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).isEmpty() );

  // Tabify our dock widget (picking 2nd priority)
  const QString dockName1 = QStringLiteral( "QDockWidget 1" );
  QDockWidget *dw1 = new QDockWidget( dockName1 );
  mQgisApp->addTabifiedDockWidget( area, dw1, QStringList() << objectName1 << objectName2, true );
  QVERIFY( dw1->isVisible() );

  // Which one is tabified now?
  QVERIFY( mQgisApp->tabifiedDockWidgets( mLayerTreeDock ).isEmpty() );
  QCOMPARE( mQgisApp->tabifiedDockWidgets( mLayerOrderDock ).length(), 1 );

  // Check that we have 3 dock widgets now in that area, only 2 visible
  int count = 0;
  int visibleCount = 0;
  const QList<QDockWidget *> docks = mQgisApp->findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    if ( mQgisApp->dockWidgetArea( dock ) == area )
    {
      count++;
      if ( dock->isVisible() )
      {
        visibleCount++;
      }
    }
  }
  QCOMPARE( count, 3 );
  QCOMPARE( visibleCount, 2 );

  // Check that our dock widget is NOT active
  const QList<QTabBar *> tabBars = mQgisApp->findChildren<QTabBar *>( QString(), Qt::FindDirectChildrenOnly );
  bool dwFound = false;
  for ( QTabBar *tabBar : tabBars )
  {
    for ( int i = 0; i < tabBar->count(); i++ )
    {
      if ( tabBar->tabText( i ) == dockName1 )
      {
        dwFound = true;
        QCOMPARE( tabBar->currentIndex(), i );
        break;
      }
    }
    if ( dwFound )
    {
      break;
    }
  }
  QVERIFY( dwFound );
}

QGSTEST_MAIN( TestQgisAppDockWidgets )
#include "testqgisappdockwidgets.moc"
