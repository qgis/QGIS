/***************************************************************************
    testqgscustomization.cpp
    ---------------------
    begin                : 2025/12/10
    copyright            : (C) 2025 by Julien Cabieces
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
#include "qgsbrowserdockwidget.h"
#include "qgsbrowserguimodel.h"
#include "qgsbrowserwidget.h"
#include "qgscustomization.h"
#include "qgscustomizationdialog.h"
#include "qgslayertreeview.h"
#include "qgsstatusbar.h"
#include "qgstest.h"

#include <QAbstractItemModelTester>
#include <QDockWidget>

class TestQgsCustomization : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCustomization()
      : QgsTest( u"Customization Tests"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testLoadApply();
    void testActionPosition();
    void testEnabled();
    void testBackwardCompatibility();
    void testClone();
    void testModel();

  private:
    template<class T>
    T *getItem( const std::unique_ptr<QgsCustomization> &customization, const QString &path ) const
    {
      return dynamic_cast<T *>( getItem( customization, path ) );
    }
    QgsCustomization::Item *getItem( const std::unique_ptr<QgsCustomization> &customization, const QString &path ) const;

    template<class T>
    T *getItem( const QString &path ) const { return dynamic_cast<T *>( getItem( path ) ); }
    QgsCustomization::Item *getItem( const QString &path ) const
    {
      return getItem( mQgisApp->customization(), path );
    }

    static QWidget *findQWidget( const QString &path );

    template<class T>
    static T *findQWidget( const QString &path )
    {
      return dynamic_cast<T *>( findQWidget( path ) );
    }

    static QAction *findQAction( const QString &path );
    static long long qactionPosition( const QString &path );

    std::unique_ptr<QgisApp> mQgisApp;
    std::unique_ptr<QTemporaryFile> mCustomizationFile;
};

QgsCustomization::Item *TestQgsCustomization::getItem( const std::unique_ptr<QgsCustomization> &customization, const QString &path ) const
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  const QHash<QString, QgsCustomization::Item *> rootItems = {
    { "Menus", customization->menus().get() },
    { "ToolBars", customization->toolBars().get() },
    { "Docks", customization->docks().get() },
    { "BrowserItems", customization->browserItems().get() },
    { "StatusBarWidgets", customization->statusBarWidgets().get() }
  };

  QgsCustomization::Item *currentItem = nullptr;
  for ( const QString &pathElem : pathElems )
  {
    if ( currentItem )
    {
      currentItem = currentItem->getChild( pathElem );
    }
    else
    {
      currentItem = rootItems.value( pathElem );
    }

    if ( !currentItem )
      return nullptr;
  }

  return currentItem;
}

QWidget *TestQgsCustomization::findQWidget( const QString &path )
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  const QHash<QString, QWidget *> rootWidgets = { { "Menus", QgisApp::instance()->menuBar() }, { "ToolBars", QgisApp::instance() }, { "Docks", QgisApp::instance() }, { "StatusBarWidgets", QgisApp::instance()->statusBarIface() } };

  const QString rootElem = pathElems.takeFirst();
  QWidget *currentWidget = rootWidgets.value( rootElem );
  if ( !currentWidget )
    return nullptr;

  for ( const QString &pathElem : pathElems )
  {
    if ( dynamic_cast<QToolBar *>( currentWidget )
         || dynamic_cast<QMenu *>( currentWidget )
         || dynamic_cast<QMenuBar *>( currentWidget ) )
    {
      for ( QgsCustomization::QWidgetIterator::Info it : QgsCustomization::QWidgetIterator( currentWidget ) )
      {
        if ( it.name == pathElem )
        {
          currentWidget = it.widget;
          break;
        }
      }
    }
    else
    {
      QList<QObject *> children = currentWidget->children();
      QList<QObject *>::const_iterator it = std::find_if( children.cbegin(), children.cend(), [&pathElem]( QObject *obj ) { return dynamic_cast<QWidget *>( obj ) && obj->objectName() == pathElem; } );
      currentWidget = dynamic_cast<QWidget *>( *it );
    }

    if ( !currentWidget )
      return nullptr;
  }

  return currentWidget;
}

QAction *TestQgsCustomization::findQAction( const QString &path )
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  qsizetype lastSlashIndex = path.lastIndexOf( "/" );
  if ( lastSlashIndex < 0 )
    return nullptr;

  QWidget *currentWidget = findQWidget( path.first( lastSlashIndex ) );
  if ( !currentWidget )
    return nullptr;

  const QString actionName = path.mid( lastSlashIndex + 1 );

  const QList<QAction *> actions = currentWidget->actions();
  const QList<QAction *>::const_iterator actionIt = std::find_if( actions.cbegin(), actions.cend(), [&actionName]( QAction *action ) { return action->objectName() == actionName; } );
  return actionIt != actions.cend() ? *actionIt : nullptr;
}

long long TestQgsCustomization::qactionPosition( const QString &path )
{
  qsizetype lastSlashIndex = path.lastIndexOf( "/" );
  QWidget *currentWidget = findQWidget( path.first( lastSlashIndex ) );
  if ( !currentWidget )
    return -1;

  const QString actionName = path.mid( lastSlashIndex + 1 );

  const QList<QAction *> actions = currentWidget->actions();
  const QList<QAction *>::const_iterator actionIt = std::find_if( actions.cbegin(), actions.cend(), [&actionName]( QAction *action ) { return action->objectName() == actionName; } );
  return actionIt == actions.cend() ? -1 : std::distance( actions.cbegin(), actionIt );
}

void TestQgsCustomization::initTestCase()
{
}

void TestQgsCustomization::cleanupTestCase()
{
}

void TestQgsCustomization::init()
{
  mQgisApp = std::make_unique<QgisApp>();
  mQgisApp->createStatusBar();

  QgsBrowserGuiModel *browserModel = new QgsBrowserGuiModel( this );
  mQgisApp->mBrowserWidget = new QgsBrowserDockWidget( tr( "Browser" ), browserModel, mQgisApp.get() );
  mQgisApp->mBrowserWidget->setObjectName( u"Browser"_s );


  // add a test tool bar to test action with menu
  QToolBar *toolBar = new QToolBar( "testToolBar", mQgisApp.get() );
  toolBar->setObjectName( "testToolBar" );
  QToolButton *tb = new QToolButton( toolBar );
  tb->setPopupMode( QToolButton::MenuButtonPopup );
  QAction *action = new QAction( "testToolBarMenuAction1" );
  action->setObjectName( "testToolBarMenuAction1" );
  tb->addAction( action );
  tb->setDefaultAction( action );
  action = new QAction( "testToolBarMenuAction2" );
  action->setObjectName( "testToolBarMenuAction2" );
  tb->addAction( action );
  QAction *tbAction = toolBar->addWidget( tb );
  tbAction->setObjectName( "testToolBarToolButton" );
  mQgisApp->addToolBar( toolBar );

  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->setVisible( true );

  // Random crashes if this is is visible
  mQgisApp->mLayerTreeView->setVisible( false );

  mQgisApp->show();

  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );

  mCustomizationFile = std::make_unique<QTemporaryFile>();
  mCustomizationFile->open(); // fileName is not available until open
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );
  mQgisApp->setCustomization( std::move( customization ) );
}

void TestQgsCustomization::cleanup()
{
  mCustomizationFile->close();
}

void TestQgsCustomization::testLoadApply()
{
  mQgisApp->customization()->setEnabled( true );

  // Make some modifications to the current customization
  auto setAllVisible = [this]( bool visible ) {
    QVERIFY( getItem<QgsCustomization::Menu>( "Menus/mHelpMenu" ) );
    getItem<QgsCustomization::Menu>( "Menus/mHelpMenu" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" ) );
    getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::ToolBar>( "ToolBars/mFileToolBar" ) );
    getItem<QgsCustomization::ToolBar>( "ToolBars/mFileToolBar" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
    getItem<QgsCustomization::Action>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::BrowserItem>( "BrowserItems/special:Home" ) );
    getItem<QgsCustomization::BrowserItem>( "BrowserItems/special:Home" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::Dock>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
    getItem<QgsCustomization::Dock>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::StatusBarWidget>( "StatusBarWidgets/LocatorWidget" ) );
    getItem<QgsCustomization::StatusBarWidget>( "StatusBarWidgets/LocatorWidget" )->setVisible( visible );

    QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
    getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->setVisible( visible );
  };

  setAllVisible( false );

  mQgisApp->customization()->write();

  // re-read written customization
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );

  // test item visiblity
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mProjectMenu" ) );
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mProjectMenu" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionRedo" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mFileToolBar" ) );
  QVERIFY( !getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mAttributesToolBar" ) );
  QVERIFY( getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mAttributesToolBar" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddVirtualLayer" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddVirtualLayer" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/special:Home" ) );
  QVERIFY( !getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/special:Home" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/GPKG" ) );
  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/GPKG" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Dock>( customization, "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !getItem<QgsCustomization::Dock>( customization, "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/mProgressBar" ) );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/mProgressBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/LocatorWidget" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" ) );
  QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" )->isVisible() );

  // test initial situation
  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );
  QVERIFY( !mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );

  // apply modification
  mQgisApp->customization()->apply();

  QVERIFY( !findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( !findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( !findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );

  // go back to initial situation
  setAllVisible( true );

  mQgisApp->customization()->apply();

  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( !findQWidget<QMenu>( "Menus/mHelpMenu" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );
  QVERIFY( !mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );
}

void TestQgsCustomization::testActionPosition()
{
  mQgisApp->customization()->setEnabled( true );

  // test initial situation
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  long long undoPosition = qactionPosition( "Menus/mEditMenu/mActionUndo" );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" )->isVisible() );
  long long redoPosition = qactionPosition( "Menus/mEditMenu/mActionRedo" );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" )->isVisible() );
  long long rotatePosition = qactionPosition( "Menus/mEditMenu/mActionRotatePointSymbols" );

  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" ) );
  getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" )->setVisible( false );
  QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRotatePointSymbols" )->setVisible( false );

  // apply modification
  mQgisApp->customization()->apply();

  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  // same mActionRedo action than edit menu one must be existing in toolbar
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QCOMPARE( qactionPosition( "Menus/mEditMenu/mActionUndo" ), undoPosition );

  // go back to initial situation
  QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" ) );
  getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" )->setVisible( true );
  QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRotatePointSymbols" )->setVisible( true );

  mQgisApp->customization()->apply();

  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QCOMPARE( qactionPosition( "Menus/mEditMenu/mActionUndo" ), undoPosition );
  QCOMPARE( qactionPosition( "Menus/mEditMenu/mActionRedo" ), redoPosition );
  QCOMPARE( qactionPosition( "Menus/mEditMenu/mActionRotatePointSymbols" ), rotatePosition );

  // remove mActionRedo and check mActionRotatePointSymbols position is just same as before minus one
  // (because mActionRedo is missing)
  QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" ) );
  getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" )->setVisible( false );

  mQgisApp->customization()->apply();

  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" )->isVisible() );
  QCOMPARE( qactionPosition( "Menus/mEditMenu/mActionRotatePointSymbols" ), rotatePosition - 1 );
}

void TestQgsCustomization::testEnabled()
{
  QVERIFY( getItem<QgsCustomization::Menu>( "Menus/mHelpMenu" ) );
  getItem<QgsCustomization::Menu>( "Menus/mHelpMenu" )->setVisible( false );

  mQgisApp->customization()->write();

  // re-read written customization
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" )->isVisible() );
  QVERIFY( !customization->isEnabled() );

  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );

  mQgisApp->customization()->apply();

  // still visible because we didn't  enabled customization
  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );

  mQgisApp->customization()->setEnabled( true );

  mQgisApp->customization()->apply();
  QVERIFY( !findQWidget<QMenu>( "Menus/mHelpMenu" ) );

  mQgisApp->customization()->write();

  // re-read written customization
  customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" )->isVisible() );
  QVERIFY( customization->isEnabled() );
}

void TestQgsCustomization::testBackwardCompatibility()
{
  QSettings().setValue( "UI/Customization/enabled", true );

  QTemporaryFile iniFile;
  iniFile.open(); // fileName is not available until open
  const QString iniFileName = iniFile.fileName();

  {
    QSettings settings( iniFileName, QSettings::IniFormat );

    settings.setValue( "/Customization/splashpath", "/tmp/splashPath.png" );

    settings.setValue( "Customization/Toolbars/mAnnotationsToolBar/mActionAnnotationdeligne", true );
    settings.setValue( "Customization/Toolbars/mAnnotationsToolBar/mActionAnnotationdepolygone", false );

    settings.setValue( "Customization/Toolbars/mAttributesToolBar/ActionMeasure", false );
    settings.setValue( "Customization/Toolbars/mAttributesToolBar/ActionMeasure/mActionMeasure", true );
    settings.setValue( "Customization/Toolbars/mAttributesToolBar/ActionMeasure/mActionMeasureAngle", false );

    settings.setValue( "Customization/Menus/mEditMenu/mActionCopyFeatures", true );
    settings.setValue( "Customization/Menus/mEditMenu/mActionCutFeatures", false );
    settings.setValue( "Customization/Menus/mViewMenu/mMenuMeasure", false );
    settings.setValue( "Customization/Menus/mViewMenu/mMenuMeasure/mActionMeasure", true );
    settings.setValue( "Customization/Menus/mViewMenu/mMenuMeasure/mActionMeasureAngle", false );

    settings.setValue( "Customization/Docks/AdvancedDigitizingTools", true );
    settings.setValue( "Customization/Docks/BookmarksDockWidget", false );

    settings.setValue( "Customization/StatusBar", false );
    settings.setValue( "Customization/StatusBar/LocatorWidget", true );
    settings.setValue( "Customization/StatusBar/mCoordsEdit", false );

    settings.setValue( "Customization/Browser/GPKG", true );
    settings.setValue( "Customization/Browser/MSSQL", false );
  }

  auto customization = std::make_unique<QgsCustomization>( ( QString() ) );
  customization->loadOldIniFile( iniFileName );

  QVERIFY( customization->isEnabled() );
  QCOMPARE( customization->splashPath(), "/tmp/splashPath.png" );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAnnotationsToolBar/mActionAnnotationdeligne" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAnnotationsToolBar/mActionAnnotationdeligne" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAnnotationsToolBar/mActionAnnotationdepolygone" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "ToolBars/mAnnotationsToolBar/mActionAnnotationdepolygone" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAttributesToolBar/ActionMeasure" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "ToolBars/mAttributesToolBar/ActionMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasure" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasureAngle" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasureAngle" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionCopyFeatures" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionCopyFeatures" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionCutFeatures" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mViewMenu/mMenuMeasure" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "Menus/mViewMenu/mMenuMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mViewMenu/mMenuMeasure/mActionMeasure" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mViewMenu/mMenuMeasure/mActionMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mViewMenu/mMenuMeasure/mActionMeasureAngle" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "Menus/mViewMenu/mMenuMeasure/mActionMeasureAngle" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Dock>( customization, "Docks/AdvancedDigitizingTools" ) );
  QVERIFY( getItem<QgsCustomization::Dock>( customization, "Docks/AdvancedDigitizingTools" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Dock>( customization, "Docks/BookmarksDockWidget" ) );
  QVERIFY( !getItem<QgsCustomization::Dock>( customization, "Docks/BookmarksDockWidget" )->isVisible() );

  QVERIFY( !customization->statusBarWidgets()->isVisible() );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/mCoordsEdit" ) );
  QVERIFY( !getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/mCoordsEdit" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/GPKG" ) );
  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/GPKG" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/MSSQL" ) );
  QVERIFY( !getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/MSSQL" )->isVisible() );
}

void TestQgsCustomization::testClone()
{
  mQgisApp->customization()->setEnabled( true );

  // Make some modifications to the current customization
  auto setAllVisible = [this]( bool visible ) {
    QVERIFY( getItem<QgsCustomization::Menu>( "Menus/mHelpMenu" ) );
    getItem<QgsCustomization::Menu>( "Menus/mHelpMenu" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" ) );
    getItem<QgsCustomization::Action>( "Menus/mEditMenu/mActionRedo" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::ToolBar>( "ToolBars/mFileToolBar" ) );
    getItem<QgsCustomization::ToolBar>( "ToolBars/mFileToolBar" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
    getItem<QgsCustomization::Action>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::BrowserItem>( "BrowserItems/special:Home" ) );
    getItem<QgsCustomization::BrowserItem>( "BrowserItems/special:Home" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::Dock>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
    getItem<QgsCustomization::Dock>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::StatusBarWidget>( "StatusBarWidgets/LocatorWidget" ) );
    getItem<QgsCustomization::StatusBarWidget>( "StatusBarWidgets/LocatorWidget" )->setVisible( visible );

    QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
    getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->setVisible( visible );
  };

  setAllVisible( false );

  // copy customization
  auto customization = std::make_unique<QgsCustomization>( *mQgisApp->customization() );

  // test item visiblity
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mProjectMenu" ) );
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mProjectMenu" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::Menu>( customization, "Menus/mHelpMenu" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "Menus/mEditMenu/mActionRedo" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mFileToolBar" ) );
  QVERIFY( !getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mAttributesToolBar" ) );
  QVERIFY( getItem<QgsCustomization::ToolBar>( customization, "ToolBars/mAttributesToolBar" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddVirtualLayer" ) );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddVirtualLayer" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( customization, "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/special:Home" ) );
  QVERIFY( !getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/special:Home" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/GPKG" ) );
  QVERIFY( getItem<QgsCustomization::BrowserItem>( customization, "BrowserItems/GPKG" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Dock>( customization, "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !getItem<QgsCustomization::Dock>( customization, "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/mProgressBar" ) );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/mProgressBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !getItem<QgsCustomization::StatusBarWidget>( customization, "StatusBarWidgets/LocatorWidget" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( !getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" ) );
  QVERIFY( getItem<QgsCustomization::Action>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" )->isVisible() );

  // test initial situation
  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );
  QVERIFY( !mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );

  // apply modification
  mQgisApp->customization()->apply();

  QVERIFY( !findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( !findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( !findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );

  // go back to initial situation
  setAllVisible( true );

  mQgisApp->customization()->apply();

  QVERIFY( findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( !findQWidget<QMenu>( "Menus/mHelpMenu" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRedo" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );
  QVERIFY( !mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );

  // restore clone
  mQgisApp->setCustomization( std::move( customization ) );

  // check our modifications are here
  QVERIFY( !findQWidget<QMenu>( "Menus/mHelpMenu" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" ) );
  QVERIFY( findQAction( "ToolBars/mDigitizeToolBar/mActionRedo" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mFileToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( !findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !findQWidget<QDockWidget>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );
  QVERIFY( findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !findQWidget<QWidget>( "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( !findQAction( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( mQgisApp->browserWidget()->browserWidget()->mDisabledDataItemsKeys.contains( "special:Home" ) );
}

void TestQgsCustomization::testModel()
{
  mQgisApp->customization()->setEnabled( true );

  QgsCustomizationDialog::QgsCustomizationModel model( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ItemVisibility );
  QAbstractItemModelTester modelTester( &model, QAbstractItemModelTester::FailureReportingMode::Fatal );

  QCOMPARE( model.rowCount(), 5 );

  // Uncheck ToolBars/mLayerToolBar/mActionAddRasterLayer item
  {
    QModelIndex toolBarsIndex = model.index( 4, 0 );
    QCOMPARE( model.data( toolBarsIndex, Qt::ItemDataRole::DisplayRole ), u"ToolBars"_s );
    QModelIndexList items = model.match( model.index( 0, 0, toolBarsIndex ), Qt::DisplayRole, "mLayerToolBar", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex layerToolBarIndex = items.first();
    QCOMPARE( model.data( layerToolBarIndex, Qt::ItemDataRole::DisplayRole ), u"mLayerToolBar"_s );
    items = model.match( model.index( 0, 0, layerToolBarIndex ), Qt::DisplayRole, "mActionAddRasterLayer", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex addRasterLayerIndex = items.first();
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::DisplayRole ), u"mActionAddRasterLayer"_s );

    model.setData( addRasterLayerIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Unchecked );
  }

  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  // revert values
  model.reset();

  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  {
    QModelIndex toolBarsIndex = model.index( 4, 0 );
    QCOMPARE( model.data( toolBarsIndex, Qt::ItemDataRole::DisplayRole ), u"ToolBars"_s );
    QModelIndexList items = model.match( model.index( 0, 0, toolBarsIndex ), Qt::DisplayRole, "mLayerToolBar", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex layerToolBarIndex = items.first();
    QCOMPARE( model.data( layerToolBarIndex, Qt::ItemDataRole::DisplayRole ), u"mLayerToolBar"_s );
    items = model.match( model.index( 0, 0, layerToolBarIndex ), Qt::DisplayRole, "mActionAddRasterLayer", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex addRasterLayerIndex = items.first();
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::DisplayRole ), u"mActionAddRasterLayer"_s );

    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Checked );
  }

  // Uncheck ToolBars/mLayerToolBar/mActionAddRasterLayer item
  {
    QModelIndex toolBarsIndex = model.index( 4, 0 );
    QCOMPARE( model.data( toolBarsIndex, Qt::ItemDataRole::DisplayRole ), u"ToolBars"_s );
    QModelIndexList items = model.match( model.index( 0, 0, toolBarsIndex ), Qt::DisplayRole, "mLayerToolBar", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex layerToolBarIndex = items.first();
    QCOMPARE( model.data( layerToolBarIndex, Qt::ItemDataRole::DisplayRole ), u"mLayerToolBar"_s );
    items = model.match( model.index( 0, 0, layerToolBarIndex ), Qt::DisplayRole, "mActionAddRasterLayer", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex addRasterLayerIndex = items.first();
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::DisplayRole ), u"mActionAddRasterLayer"_s );

    model.setData( addRasterLayerIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Unchecked );
  }

  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  // revert values
  model.apply();

  {
    QModelIndex toolBarsIndex = model.index( 4, 0 );
    QCOMPARE( model.data( toolBarsIndex, Qt::ItemDataRole::DisplayRole ), u"ToolBars"_s );
    QModelIndexList items = model.match( model.index( 0, 0, toolBarsIndex ), Qt::DisplayRole, "mLayerToolBar", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex layerToolBarIndex = items.first();
    QCOMPARE( model.data( layerToolBarIndex, Qt::ItemDataRole::DisplayRole ), u"mLayerToolBar"_s );
    items = model.match( model.index( 0, 0, layerToolBarIndex ), Qt::DisplayRole, "mActionAddRasterLayer", 1 );
    QCOMPARE( items.count(), 1 );
    QModelIndex addRasterLayerIndex = items.first();
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::DisplayRole ), u"mActionAddRasterLayer"_s );

    model.setData( addRasterLayerIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( addRasterLayerIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Unchecked );
  }

  // the action is no longer visible
  QVERIFY( !findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
}


QGSTEST_MAIN( TestQgsCustomization )
#include "testqgscustomization.moc"
