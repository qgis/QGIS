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
#include "qgsnativealgorithms.h"
#include "qgsprocessingregistry.h"
#include "qgstest.h"

#include <QAbstractItemModelTester>
#include <QDockWidget>
#include <QMenu>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsCustomization : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCustomization()
      : QgsTest( u"Customization Tests"_s )
    {}

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
    void testModelProcessing();
    void testModelUserAction_data();
    void testModelUserAction();
    void testToolBarPosition();
    void testMenuOrder();

  private:
    template<class T> T *getItem( QgsCustomization *customization, const QString &path ) const { return dynamic_cast<T *>( getItem( customization, path ) ); }
    QgsCustomization::QgsItem *getItem( QgsCustomization *customization, const QString &path ) const;

    template<class T> T *getItem( const QString &path ) const { return dynamic_cast<T *>( getItem( path ) ); }
    QgsCustomization::QgsItem *getItem( const QString &path ) const { return getItem( mQgisApp->customization(), path ); }

    static QWidget *findQWidget( const QString &path );

    template<class T> static T *findQWidget( const QString &path ) { return dynamic_cast<T *>( findQWidget( path ) ); }

    static QAction *findQAction( const QString &path );
    static long long qactionPosition( const QString &path );
    static QList<QAction *> findQActions( const QWidget *widget, const QString &actionText );

    std::unique_ptr<QgisApp> mQgisApp;
    std::unique_ptr<QTemporaryFile> mCustomizationFile;
};

QgsCustomization::QgsItem *TestQgsCustomization::getItem( QgsCustomization *customization, const QString &path ) const
{
  return customization->getItem( path );
}

QWidget *TestQgsCustomization::findQWidget( const QString &path )
{
  return QgsCustomization::findQWidget( path );
}

QAction *TestQgsCustomization::findQAction( const QString &path )
{
  return QgsCustomization::findQAction( path );
}

QList<QAction *> TestQgsCustomization::findQActions( const QWidget *widget, const QString &actionText )
{
  QList<QAction *> actions;
  const QList<QAction *> allActions = widget->actions();
  for ( QAction *action : std::as_const( allActions ) )
  {
    if ( action->text() == actionText )
      actions << action;
  }
  return actions;
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
  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestQgsCustomization::cleanupTestCase()
{}

void TestQgsCustomization::init()
{
  mQgisApp = std::make_unique<QgisApp>();
  mQgisApp->createStatusBar();

  mQgisApp->mToolbarMenu = new QMenu( u"Toolbars"_s, mQgisApp.get() );
  mQgisApp->mToolbarMenu->setObjectName( u"mToolbarMenu"_s );

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
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" ) );

  mCustomizationFile = std::make_unique<QTemporaryFile>();
  QVERIFY( mCustomizationFile->open() ); // fileName is not available until open
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );
  mQgisApp->setCustomization( std::move( customization ) );
}

void TestQgsCustomization::cleanup()
{
  mCustomizationFile->close();
}

void TestQgsCustomization::testLoadApply()
{
  QVERIFY( !mQgisApp->customization()->splashPath().isEmpty() );

  mQgisApp->customization()->setEnabled( true );

  QVERIFY( !mQgisApp->customization()->splashPath().isEmpty() );

  // Make some modifications to the current customization
  auto setAllVisible = [this]( bool visible ) {
    QVERIFY( getItem<QgsCustomization::QgsMenuItem>( "Menus/mHelpMenu" ) );
    getItem<QgsCustomization::QgsMenuItem>( "Menus/mHelpMenu" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" ) );
    getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( "ToolBars/mFileToolBar" ) );
    getItem<QgsCustomization::QgsToolBarItem>( "ToolBars/mFileToolBar" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
    getItem<QgsCustomization::QgsActionItem>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( "BrowserItems/special:Home" ) );
    getItem<QgsCustomization::QgsBrowserElementItem>( "BrowserItems/special:Home" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsDockItem>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
    getItem<QgsCustomization::QgsDockItem>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( "StatusBarWidgets/LocatorWidget" ) );
    getItem<QgsCustomization::QgsStatusBarWidgetItem>( "StatusBarWidgets/LocatorWidget" )->setVisible( visible );

    QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
    getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->setVisible( visible );
  };

  setAllVisible( false );

  mQgisApp->customization()->write();

  // re-read written customization
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );

  // test item visiblity
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mProjectMenu" ) );
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mProjectMenu" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionRedo" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mFileToolBar" ) );
  QVERIFY( !getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mAttributesToolBar" ) );
  QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mAttributesToolBar" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddVirtualLayer" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddVirtualLayer" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/special:Home" ) );
  QVERIFY( !getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/special:Home" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/GPKG" ) );
  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/GPKG" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/mProgressBar" ) );
  QVERIFY( !getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/mProgressBar" )->isVisible() ); // not visible by default
  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/LocatorWidget" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" )->isVisible() );

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

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" ) );
  getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" )->setVisible( false );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRotatePointSymbols" )->setVisible( false );

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
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" ) );
  getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" )->setVisible( true );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRotatePointSymbols" )->setVisible( true );

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
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" ) );
  getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" )->setVisible( false );

  mQgisApp->customization()->apply();

  QVERIFY( !findQAction( "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" ) );
  QVERIFY( findQAction( "Menus/mEditMenu/mActionRotatePointSymbols" )->isVisible() );
  QCOMPARE( qactionPosition( "Menus/mEditMenu/mActionRotatePointSymbols" ), rotatePosition - 1 );
}

void TestQgsCustomization::testEnabled()
{
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( "Menus/mHelpMenu" ) );
  getItem<QgsCustomization::QgsMenuItem>( "Menus/mHelpMenu" )->setVisible( false );

  mQgisApp->customization()->write();

  // re-read written customization
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" )->isVisible() );
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
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" )->isVisible() );
  QVERIFY( customization->isEnabled() );
}

void TestQgsCustomization::testBackwardCompatibility()
{
  QSettings().setValue( "UI/Customization/enabled", true );

  QTemporaryFile iniFile;
  QVERIFY( iniFile.open() ); // fileName is not available until open
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

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAnnotationsToolBar/mActionAnnotationdeligne" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAnnotationsToolBar/mActionAnnotationdeligne" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAnnotationsToolBar/mActionAnnotationdepolygone" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAnnotationsToolBar/mActionAnnotationdepolygone" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAttributesToolBar/ActionMeasure" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAttributesToolBar/ActionMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasure" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasureAngle" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mAttributesToolBar/ActionMeasure/mActionMeasureAngle" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionCopyFeatures" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionCopyFeatures" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionCutFeatures" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mViewMenu/mMenuMeasure" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mViewMenu/mMenuMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mViewMenu/mMenuMeasure/mActionMeasure" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mViewMenu/mMenuMeasure/mActionMeasure" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mViewMenu/mMenuMeasure/mActionMeasureAngle" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mViewMenu/mMenuMeasure/mActionMeasureAngle" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/AdvancedDigitizingTools" ) );
  QVERIFY( getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/AdvancedDigitizingTools" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/BookmarksDockWidget" ) );
  QVERIFY( !getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/BookmarksDockWidget" )->isVisible() );

  QVERIFY( !customization->statusBarWidgetsItem()->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/LocatorWidget" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/mCoordsEdit" ) );
  QVERIFY( !getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/mCoordsEdit" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/GPKG" ) );
  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/GPKG" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/MSSQL" ) );
  QVERIFY( !getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/MSSQL" )->isVisible() );
}

void TestQgsCustomization::testClone()
{
  mQgisApp->customization()->setEnabled( true );

  // Make some modifications to the current customization
  auto setAllVisible = [this]( bool visible ) {
    QVERIFY( getItem<QgsCustomization::QgsMenuItem>( "Menus/mHelpMenu" ) );
    getItem<QgsCustomization::QgsMenuItem>( "Menus/mHelpMenu" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" ) );
    getItem<QgsCustomization::QgsActionItem>( "Menus/mEditMenu/mActionRedo" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( "ToolBars/mFileToolBar" ) );
    getItem<QgsCustomization::QgsToolBarItem>( "ToolBars/mFileToolBar" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
    getItem<QgsCustomization::QgsActionItem>( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( "BrowserItems/special:Home" ) );
    getItem<QgsCustomization::QgsBrowserElementItem>( "BrowserItems/special:Home" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsDockItem>( "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
    getItem<QgsCustomization::QgsDockItem>( "Docks/QgsAdvancedDigitizingDockWidgetBase" )->setVisible( visible );
    QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( "StatusBarWidgets/LocatorWidget" ) );
    getItem<QgsCustomization::QgsStatusBarWidgetItem>( "StatusBarWidgets/LocatorWidget" )->setVisible( visible );

    QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
    getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->setVisible( visible );
  };

  setAllVisible( false );

  // copy customization
  auto customization = std::make_unique<QgsCustomization>( *mQgisApp->customization() );

  // test item visiblity
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mProjectMenu" ) );
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mProjectMenu" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" ) );
  QVERIFY( !getItem<QgsCustomization::QgsMenuItem>( customization.get(), "Menus/mHelpMenu" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionUndo" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionUndo" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionRedo" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "Menus/mEditMenu/mActionRedo" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mFileToolBar" ) );
  QVERIFY( !getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mFileToolBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mAttributesToolBar" ) );
  QVERIFY( getItem<QgsCustomization::QgsToolBarItem>( customization.get(), "ToolBars/mAttributesToolBar" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddVirtualLayer" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddVirtualLayer" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( customization.get(), "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/special:Home" ) );
  QVERIFY( !getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/special:Home" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/GPKG" ) );
  QVERIFY( getItem<QgsCustomization::QgsBrowserElementItem>( customization.get(), "BrowserItems/GPKG" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/QgsAdvancedDigitizingDockWidgetBase" ) );
  QVERIFY( !getItem<QgsCustomization::QgsDockItem>( customization.get(), "Docks/QgsAdvancedDigitizingDockWidgetBase" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/mProgressBar" ) );
  QVERIFY( !getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/mProgressBar" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/LocatorWidget" ) );
  QVERIFY( !getItem<QgsCustomization::QgsStatusBarWidgetItem>( customization.get(), "StatusBarWidgets/LocatorWidget" )->isVisible() );

  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" ) );
  QVERIFY( !getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction1" )->isVisible() );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" ) );
  QVERIFY( getItem<QgsCustomization::QgsActionItem>( "ToolBars/testToolBar/testToolBarToolButton/testToolBarMenuAction2" )->isVisible() );

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

  // We change visibility of help toolbar, it has to stay invisible all the time we use the model
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" ) );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" )->isVisible() );
  findQWidget<QToolBar>( "ToolBars/mHelpToolBar" )->setVisible( false );

  QgsCustomizationDialog::QgsCustomizationModel model( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ItemVisibility );
  QAbstractItemModelTester modelTester( &model, QAbstractItemModelTester::FailureReportingMode::Fatal );

  QCOMPARE( model.rowCount(), 5 );

  QgsCustomizationDialog::QgsCustomizationModel modelActionSelector( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ActionSelector );
  QAbstractItemModelTester modelActionSelectorTester( &modelActionSelector, QAbstractItemModelTester::FailureReportingMode::Fatal );

  const QModelIndexList addPartActionIndexes
    = modelActionSelector.match( modelActionSelector.index( 0, 0 ), Qt::ItemDataRole::DisplayRole, u"mActionAddPart"_s, -1, Qt::MatchRecursive | Qt::MatchFixedString );
  QCOMPARE( addPartActionIndexes.count(), 2 );

  std::unique_ptr<QMimeData> mimeData( modelActionSelector.mimeData( QModelIndexList() << addPartActionIndexes.at( 0 ) ) );
  QVERIFY( mimeData );

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
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mHelpToolBar" )->isVisible() );

  // revert values
  model.reset();

  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" ) );
  QVERIFY( findQAction( "ToolBars/mLayerToolBar/mActionAddRasterLayer" )->isVisible() );
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mHelpToolBar" )->isVisible() );

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
  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mHelpToolBar" )->isVisible() );

  // apply values
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

  QVERIFY( findQWidget<QToolBar>( "ToolBars/mHelpToolBar" ) );
  QVERIFY( !findQWidget<QToolBar>( "ToolBars/mHelpToolBar" )->isVisible() );

  // test add/setVisible/setHidden/delete for user menu
  {
    const QModelIndex menusIndex = model.index( 2, 0 );
    QCOMPARE( model.data( menusIndex, Qt::ItemDataRole::DisplayRole ), u"Menus"_s );

    const QModelIndex newItemIndex = model.addUserItem( menusIndex );
    QCOMPARE( model.data( newItemIndex, Qt::ItemDataRole::DisplayRole ), u"UserMenu_1"_s );

    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" ) );
    QVERIFY( findQWidget( "Menus/UserMenu_1" ) );

    QVERIFY( model.setData( newItemIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole ) );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" ) );
    QVERIFY( !findQWidget( "Menus/UserMenu_1" ) );

    QVERIFY( model.setData( newItemIndex, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole ) );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" ) );
    QVERIFY( findQWidget( "Menus/UserMenu_1" ) );

    model.deleteUserItems( QList<QModelIndex>() << newItemIndex );

    model.apply();
    QVERIFY( !getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" ) );
    QVERIFY( !findQWidget( "Menus/UserMenu_1" ) );
  }

  // test add/setVisible/setHidden/delete for user toolbar
  {
    const QModelIndex toolbarsIndex = model.index( 4, 0 );
    QCOMPARE( model.data( toolbarsIndex, Qt::ItemDataRole::DisplayRole ), u"ToolBars"_s );

    const QModelIndex newItemIndex = model.addUserItem( toolbarsIndex );
    QCOMPARE( model.data( newItemIndex, Qt::ItemDataRole::DisplayRole ), u"UserToolBar_1"_s );

    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QVERIFY( findQWidget( "ToolBars/UserToolBar_1" ) );

    QList<QAction *> actions = findQActions( mQgisApp->toolBarMenu(), u"UserToolBar_1"_s );
    QCOMPARE( actions.count(), 1 );

    QVERIFY( model.setData( newItemIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole ) );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QVERIFY( !findQWidget( "ToolBars/UserToolBar_1" ) );
    actions = findQActions( mQgisApp->toolBarMenu(), u"UserToolBar_1"_s );
    QCOMPARE( actions.count(), 0 );

    QVERIFY( model.setData( newItemIndex, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole ) );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QVERIFY( findQWidget( "ToolBars/UserToolBar_1" ) );
    actions = findQActions( mQgisApp->toolBarMenu(), u"UserToolBar_1"_s );
    QCOMPARE( actions.count(), 1 );

    // drop an action ref in the toolbar

    QVERIFY( model.canDropMimeData( mimeData.get(), Qt::DropAction::LinkAction, 0, 0, newItemIndex ) );
    QCOMPARE( model.rowCount( newItemIndex ), 0 );
    QVERIFY( modelActionSelector.dropMimeData( mimeData.get(), Qt::DropAction::LinkAction, 0, 0, newItemIndex ) );
    QCOMPARE( model.rowCount( newItemIndex ), 1 );

    QModelIndex actionIndex = model.index( 0, 0, newItemIndex );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::DisplayRole ), u"ActionRef_mActionAddPart_1"_s );
    QCOMPARE( model.data( model.index( 0, 1, newItemIndex ), Qt::ItemDataRole::DisplayRole ), u"Add Part"_s );
    QVERIFY( !model.data( actionIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );

    QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QCOMPARE( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" )->childrenCount(), 0 );

    model.apply();

    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "ToolBars/UserToolBar_1/ActionRef_mActionAddPart_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "ToolBars/UserToolBar_1/ActionRef_mActionAddPart_1" )->isVisible() );
    QVERIFY( findQAction( u"ToolBars/UserToolBar_1/mActionAddPart"_s ) );

    // hide new added action ref
    model.setData( actionIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Unchecked );
    model.apply();

    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "ToolBars/UserToolBar_1/ActionRef_mActionAddPart_1" ) );
    QVERIFY( !getItem<QgsCustomization::QgsActionRefItem>( "ToolBars/UserToolBar_1/ActionRef_mActionAddPart_1" )->isVisible() );
    QVERIFY( !findQAction( u"ToolBars/UserToolBar_1/mActionAddPart"_s ) );

    // delete tool bar

    model.deleteUserItems( QList<QModelIndex>() << newItemIndex );

    model.apply();
    QVERIFY( !getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QVERIFY( !findQWidget( "ToolBars/UserToolBar_1" ) );
    actions = findQActions( mQgisApp->toolBarMenu(), u"UserToolBar_1"_s );
    QCOMPARE( actions.count(), 0 );
  }
}

void TestQgsCustomization::testModelProcessing()
{
  mQgisApp->customization()->setEnabled( true );

  QgsCustomizationDialog::QgsCustomizationModel model( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ItemVisibility );
  QAbstractItemModelTester modelTester( &model, QAbstractItemModelTester::FailureReportingMode::Fatal );

  QCOMPARE( model.rowCount(), 5 );

  QgsCustomizationDialog::QgsCustomizationModel modelActionSelector( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ActionSelector );
  QAbstractItemModelTester modelActionSelectorTester( &modelActionSelector, QAbstractItemModelTester::FailureReportingMode::Fatal );

  const QModelIndexList bufferActionIndexes = modelActionSelector.match( modelActionSelector.index( 0, 0 ), Qt::ItemDataRole::DisplayRole, u"native:buffer"_s, -1, Qt::MatchRecursive | Qt::MatchFixedString );
  QCOMPARE( bufferActionIndexes.count(), 1 );

  std::unique_ptr<QMimeData> mimeData( modelActionSelector.mimeData( bufferActionIndexes ) );
  QVERIFY( mimeData );

  // add user menu and drop a processing algorithm action
  {
    const QModelIndex menusIndex = model.index( 2, 0 );
    QCOMPARE( model.data( menusIndex, Qt::ItemDataRole::DisplayRole ), u"Menus"_s );

    QModelIndex newMenuItemIndex = model.addUserItem( menusIndex );
    QCOMPARE( model.data( newMenuItemIndex, Qt::ItemDataRole::DisplayRole ), u"UserMenu_1"_s );

    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" ) );
    QVERIFY( findQWidget( "Menus/UserMenu_1" ) );

    QVERIFY( model.canDropMimeData( mimeData.get(), Qt::DropAction::LinkAction, 0, 0, newMenuItemIndex ) );

    QCOMPARE( model.rowCount( newMenuItemIndex ), 0 );
    QVERIFY( model.dropMimeData( mimeData.get(), Qt::DropAction::LinkAction, 0, 0, newMenuItemIndex ) );
    QCOMPARE( model.rowCount( newMenuItemIndex ), 1 );

    QModelIndex actionIndex = model.index( 0, 0, newMenuItemIndex );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::DisplayRole ), u"ProcessingAlgorithmRef_buffer_1"_s );
    QCOMPARE( model.data( model.index( 0, 1, newMenuItemIndex ), Qt::ItemDataRole::DisplayRole ), u"Buffer"_s );
    QVERIFY( !model.data( actionIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );

    QVERIFY( getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" ) );
    QCOMPARE( getItem<QgsCustomization::QgsUserMenuItem>( "Menus/UserMenu_1" )->childrenCount(), 0 );

    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" )->isVisible() );
    QVERIFY( findQAction( u"Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1"_s ) );

    model.setData( actionIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Unchecked );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( !getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" )->isVisible() );
    QVERIFY( !findQAction( u"Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1"_s ) );

    model.setData( actionIndex, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Checked );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" )->isVisible() );
    QVERIFY( findQAction( u"Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1"_s ) );

    model.deleteUserItems( QList<QModelIndex>() << actionIndex );
    model.apply();
    QVERIFY( !getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( !findQAction( u"Menus/UserMenu_1/ProcessingAlgorithmRef_buffer_1"_s ) );
  }

  // add user ToolBar and drop a processing algorithm action
  {
    const QModelIndex toolBarsIndex = model.index( 4, 0 );
    QCOMPARE( model.data( toolBarsIndex, Qt::ItemDataRole::DisplayRole ), u"ToolBars"_s );

    QModelIndex newToolBarItemIndex = model.addUserItem( toolBarsIndex );
    QCOMPARE( model.data( newToolBarItemIndex, Qt::ItemDataRole::DisplayRole ), u"UserToolBar_1"_s );

    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QVERIFY( findQWidget( "ToolBars/UserToolBar_1" ) );

    QVERIFY( model.canDropMimeData( mimeData.get(), Qt::DropAction::LinkAction, 0, 0, newToolBarItemIndex ) );

    QCOMPARE( model.rowCount( newToolBarItemIndex ), 0 );
    QVERIFY( model.dropMimeData( mimeData.get(), Qt::DropAction::LinkAction, 0, 0, newToolBarItemIndex ) );
    QCOMPARE( model.rowCount( newToolBarItemIndex ), 1 );

    QModelIndex actionIndex = model.index( 0, 0, newToolBarItemIndex );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::DisplayRole ), u"ProcessingAlgorithmRef_buffer_1"_s );
    QCOMPARE( model.data( model.index( 0, 1, newToolBarItemIndex ), Qt::ItemDataRole::DisplayRole ), u"Buffer"_s );
    QVERIFY( !model.data( actionIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );

    QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" ) );
    QCOMPARE( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/UserToolBar_1" )->childrenCount(), 0 );

    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" )->isVisible() );
    QVERIFY( findQAction( u"ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1"_s ) );

    model.setData( actionIndex, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Unchecked );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( !getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" )->isVisible() );
    QVERIFY( !findQAction( u"ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1"_s ) );

    model.setData( actionIndex, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::CheckStateRole ), Qt::CheckState::Checked );
    model.apply();
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" )->isVisible() );
    QVERIFY( findQAction( u"ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1"_s ) );

    model.deleteUserItems( QList<QModelIndex>() << actionIndex );
    model.apply();
    QVERIFY( !getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( "ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1" ) );
    QVERIFY( !findQAction( u"ToolBars/UserToolBar_1/ProcessingAlgorithmRef_buffer_1"_s ) );
  }
}

void TestQgsCustomization::testModelUserAction_data()
{
  QTest::addColumn<QString>( "rootName" );
  QTest::addColumn<QString>( "itemName" );
  QTest::addColumn<int>( "rootRow" );

  QTest::newRow( "ToolBars" ) << u"ToolBars"_s << "UserToolBar_1" << 4;
  QTest::newRow( "Menus" ) << u"Menus"_s << "UserMenu_1" << 2;
}

void TestQgsCustomization::testModelUserAction()
{
  // test that we reload correctly action icon and title

  QFETCH( QString, rootName );
  QFETCH( QString, itemName );
  QFETCH( int, rootRow );

  mQgisApp->customization()->setEnabled( true );

  QgsCustomizationDialog::QgsCustomizationModel model( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ItemVisibility );
  QAbstractItemModelTester modelTester( &model, QAbstractItemModelTester::FailureReportingMode::Fatal );

  QCOMPARE( model.rowCount(), 5 );

  QgsCustomizationDialog::QgsCustomizationModel modelActionSelector( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ActionSelector );
  QAbstractItemModelTester modelActionSelectorTester( &modelActionSelector, QAbstractItemModelTester::FailureReportingMode::Fatal );

  // create a user tool bar
  const QModelIndex rootItemIndex = model.index( rootRow, 0 );
  QCOMPARE( model.data( rootItemIndex, Qt::ItemDataRole::DisplayRole ), rootName );

  const QModelIndex newItemIndex = model.addUserItem( rootItemIndex );
  QCOMPARE( model.data( newItemIndex, Qt::ItemDataRole::DisplayRole ), itemName );

  // drop an action ref in the toolbar
  {
    const QModelIndexList addPartActionIndexes
      = modelActionSelector.match( modelActionSelector.index( 0, 0 ), Qt::ItemDataRole::DisplayRole, u"mActionAddPart"_s, -1, Qt::MatchRecursive | Qt::MatchFixedString );
    QCOMPARE( addPartActionIndexes.count(), 2 );

    std::unique_ptr<QMimeData> mimeData( modelActionSelector.mimeData( QModelIndexList() << addPartActionIndexes.at( 0 ) ) );
    QVERIFY( mimeData );

    QVERIFY( model.canDropMimeData( mimeData.get(), Qt::DropAction::LinkAction, -1, 0, newItemIndex ) );
    QCOMPARE( model.rowCount( newItemIndex ), 0 );
    QVERIFY( model.dropMimeData( mimeData.get(), Qt::DropAction::LinkAction, -1, 0, newItemIndex ) );
    QCOMPARE( model.rowCount( newItemIndex ), 1 );

    QModelIndex actionIndex = model.index( 0, 0, newItemIndex );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::DisplayRole ), u"ActionRef_mActionAddPart_1"_s );
    QCOMPARE( model.data( model.index( 0, 1, newItemIndex ), Qt::ItemDataRole::DisplayRole ), u"Add Part"_s );
    QVERIFY( !model.data( actionIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );
  }

  {
    const QModelIndexList bufferActionIndexes
      = modelActionSelector.match( modelActionSelector.index( 0, 0 ), Qt::ItemDataRole::DisplayRole, u"native:buffer"_s, -1, Qt::MatchRecursive | Qt::MatchFixedString );
    QCOMPARE( bufferActionIndexes.count(), 1 );

    std::unique_ptr<QMimeData> mimeData( modelActionSelector.mimeData( QModelIndexList() << bufferActionIndexes.at( 0 ) ) );
    QVERIFY( mimeData );

    // drop a processing action ref
    QVERIFY( model.canDropMimeData( mimeData.get(), Qt::DropAction::LinkAction, -1, 0, newItemIndex ) );

    QCOMPARE( model.rowCount( newItemIndex ), 1 );
    QVERIFY( model.dropMimeData( mimeData.get(), Qt::DropAction::LinkAction, -1, 0, newItemIndex ) );
    QCOMPARE( model.rowCount( newItemIndex ), 2 );

    QModelIndex actionIndex = model.index( 1, 0, newItemIndex );
    QCOMPARE( model.data( actionIndex, Qt::ItemDataRole::DisplayRole ), u"ProcessingAlgorithmRef_buffer_1"_s );
    QCOMPARE( model.data( model.index( 1, 1, newItemIndex ), Qt::ItemDataRole::DisplayRole ), u"Buffer"_s );
    QVERIFY( !model.data( actionIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );
  }

  model.apply();

  QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( u"%1/%2/ActionRef_mActionAddPart_1"_s.arg( rootName, itemName ) ) );
  QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( u"%1/%2/ActionRef_mActionAddPart_1"_s.arg( rootName, itemName ) )->isVisible() );
  QVERIFY( findQAction( u"%1/%2/mActionAddPart"_s.arg( rootName, itemName ) ) );
  QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( u"%1/%2/ProcessingAlgorithmRef_buffer_1"_s.arg( rootName, itemName ) ) );
  QVERIFY( getItem<QgsCustomization::QgsProcessingAlgorithmRefItem>( u"%1/%2/ProcessingAlgorithmRef_buffer_1"_s.arg( rootName, itemName ) )->isVisible() );
  QVERIFY( findQAction( u"%1/%2/ProcessingAlgorithmRef_buffer_1"_s.arg( rootName, itemName ) ) );

  mQgisApp->customization()->write();

  // re-read written customization
  auto customization = std::make_unique<QgsCustomization>( mCustomizationFile->fileName() );

  mQgisApp->setCustomization( std::move( customization ) );

  QgsCustomizationDialog::QgsCustomizationModel otherModel( mQgisApp.get(), QgsCustomizationDialog::QgsCustomizationModel::Mode::ItemVisibility );
  QAbstractItemModelTester otherModelTester( &otherModel, QAbstractItemModelTester::FailureReportingMode::Fatal );

  const QModelIndexList userRootItemIndexes = otherModel.match( otherModel.index( 0, 0 ), Qt::ItemDataRole::DisplayRole, itemName, -1, Qt::MatchRecursive | Qt::MatchFixedString );
  QCOMPARE( userRootItemIndexes.count(), 1 );
  QModelIndex userRootItemIndex = userRootItemIndexes.at( 0 );

  QModelIndex addPartActionIndex = otherModel.index( 0, 0, userRootItemIndex );
  QCOMPARE( otherModel.data( addPartActionIndex, Qt::ItemDataRole::DisplayRole ), u"ActionRef_mActionAddPart_1"_s );
  QCOMPARE( otherModel.data( otherModel.index( addPartActionIndex.row(), 1, addPartActionIndex.parent() ), Qt::ItemDataRole::DisplayRole ), u"Add Part"_s );
  QVERIFY( !otherModel.data( addPartActionIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );

  QModelIndex actionBufferIndex = otherModel.index( 1, 0, userRootItemIndex );
  QCOMPARE( otherModel.data( actionBufferIndex, Qt::ItemDataRole::DisplayRole ), u"ProcessingAlgorithmRef_buffer_1"_s );
  QCOMPARE( otherModel.data( otherModel.index( actionBufferIndex.row(), 1, actionBufferIndex.parent() ), Qt::ItemDataRole::DisplayRole ), u"Buffer"_s );
  QVERIFY( !otherModel.data( actionBufferIndex, Qt::ItemDataRole::DecorationRole ).value<QIcon>().isNull() );
}

void TestQgsCustomization::testToolBarPosition()
{
  // check that we keep toolbar position when we call apply()

  mQgisApp->customization()->setEnabled( true );

  const QString name = "my_super_toolbar";
  mQgisApp->customization()->toolBarsItem()->addChild( std::make_unique<QgsCustomization::QgsUserToolBarItem>( name, name, mQgisApp->customization()->toolBarsItem() ) );

  QVERIFY( getItem<QgsCustomization::QgsUserToolBarItem>( "ToolBars/my_super_toolbar" ) );

  mQgisApp->customization()->apply();

  QWidget *mySuperToolBar = findQWidget( "ToolBars/my_super_toolbar" );
  QVERIFY( mySuperToolBar );

  QToolBar *newToolBar = new QToolBar( "another_toolbar", QgisApp::instance() );
  newToolBar->addAction( new QAction( "new_action" ) );
  QgisApp::instance()->addToolBar( newToolBar );

  QApplication::processEvents();
  QPoint mySuperToolBarPos = mySuperToolBar->mapToGlobal( QPoint( 0, 0 ) );

  mQgisApp->customization()->apply();

  QApplication::processEvents();

  mySuperToolBar = findQWidget( "ToolBars/my_super_toolbar" );
  QVERIFY( mySuperToolBar );

  QCOMPARE( mySuperToolBarPos, mySuperToolBar->mapToGlobal( QPoint( 0, 0 ) ) );
}

void TestQgsCustomization::testMenuOrder()
{
  // mix action ref and sub menu and check that everything is in the appropriate order

  mQgisApp->customization()->setEnabled( true );

  mQgisApp->customization()->menusItem()->addChild( std::make_unique<QgsCustomization::QgsUserMenuItem>( "MyMenu", "My menu", mQgisApp->customization()->menusItem() ) );
  QgsCustomization::QgsMenuItem *menuItem = getItem<QgsCustomization::QgsMenuItem>( "Menus/MyMenu" );
  QVERIFY( menuItem );

  {
    const QString actionPath = "Menus/mEditMenu/mActionRedo";
    QgsCustomization::QgsActionItem *actionItem = getItem<QgsCustomization::QgsActionItem>( actionPath );
    QVERIFY( actionItem );
    QVERIFY( getItem<QgsCustomization::QgsActionItem>( actionPath )->isVisible() );

    menuItem->addChild( std::make_unique<QgsCustomization::QgsActionRefItem>( mQgisApp->customization()->uniqueActionName( actionItem->name() ), actionItem->title(), actionPath, menuItem ) );
    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "Menus/MyMenu/ActionRef_mActionRedo_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "Menus/MyMenu/ActionRef_mActionRedo_1" )->isVisible() );
  }

  menuItem->addChild( std::make_unique<QgsCustomization::QgsUserMenuItem>( "MySubMenu", "My sub menu", menuItem ) );
  getItem<QgsCustomization::QgsMenuItem>( "Menus/MyMenu/MySubMenu" );

  {
    const QString actionPath = "Menus/mEditMenu/mActionUndo";
    QgsCustomization::QgsActionItem *actionItem = getItem<QgsCustomization::QgsActionItem>( actionPath );
    QVERIFY( actionItem );
    QVERIFY( getItem<QgsCustomization::QgsActionItem>( actionPath )->isVisible() );

    menuItem->addChild( std::make_unique<QgsCustomization::QgsActionRefItem>( mQgisApp->customization()->uniqueActionName( actionItem->name() ), actionItem->title(), actionPath, menuItem ) );
    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "Menus/MyMenu/ActionRef_mActionUndo_1" ) );
    QVERIFY( getItem<QgsCustomization::QgsActionRefItem>( "Menus/MyMenu/ActionRef_mActionUndo_1" )->isVisible() );
  }

  mQgisApp->customization()->apply();

  QMenu *menu = findQWidget<QMenu>( "Menus/MyMenu" );
  QVERIFY( menu );
  QCOMPARE( menu->actions().count(), 3 );

  QVERIFY( !menu->actions().at( 0 )->menu() );
  QCOMPARE( menu->actions().at( 0 )->text(), "&Redo" );
  QVERIFY( menu->actions().at( 1 )->menu() );
  QCOMPARE( menu->actions().at( 1 )->text(), "My sub menu" );
  QVERIFY( !menu->actions().at( 2 )->menu() );
  QCOMPARE( menu->actions().at( 2 )->text(), "&Undo" );
}

QGSTEST_MAIN( TestQgsCustomization )
#include "testqgscustomization.moc"
