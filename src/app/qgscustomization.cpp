/***************************************************************************
               qgscustomization.cpp  - Customization
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscustomization.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsstatusbar.h"
#include "qgsbrowserdockwidget.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsgui.h"

#include <QAction>
#include <QWidgetAction>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QKeySequence>
#include <QToolButton>
#include <QStatusBar>
#include <QMetaObject>
#include <QSettings>

bool isInternalWidget( const QString &name )
{
  static const QStringList internalWidgets = QStringList() << QStringLiteral( "qt_tabwidget_stackedwidget" ) << QStringLiteral( "qt_tabwidget_tabbar" );

  if ( internalWidgets.contains( name ) )
    return true;

  return false;
}

#ifdef Q_OS_MACX
QgsCustomizationDialog::QgsCustomizationDialog( QWidget *parent, QSettings *settings )
  : QMainWindow( parent, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsCustomizationDialog::QgsCustomizationDialog( QWidget * parent, QSettings * settings )
  : QMainWindow( parent )
#endif
{
  mSettings = settings;
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( actionSave, &QAction::triggered, this, &QgsCustomizationDialog::actionSave_triggered );
  connect( actionLoad, &QAction::triggered, this, &QgsCustomizationDialog::actionLoad_triggered );
  connect( actionExpandAll, &QAction::triggered, this, &QgsCustomizationDialog::actionExpandAll_triggered );
  connect( actionCollapseAll, &QAction::triggered, this, &QgsCustomizationDialog::actionCollapseAll_triggered );
  connect( actionSelectAll, &QAction::triggered, this, &QgsCustomizationDialog::actionSelectAll_triggered );
  connect( mCustomizationEnabledCheckBox, &QCheckBox::toggled, this, &QgsCustomizationDialog::enableCustomization );
  connect( mLeFilter, &QgsFilterLineEdit::textChanged, this, &QgsCustomizationDialog::filterItems );

  mLeFilter->setShowSearchIcon( true );

  init();
  QStringList myHeaders;
  myHeaders << tr( "Object name" ) << tr( "Label" );
  treeWidget->setHeaderLabels( myHeaders );

  mLastDirSettingsName  = QStringLiteral( "/UI/lastCustomizationDir" );
  //treeWidget->hideColumn(0)
  connect( buttonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::ok );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::apply );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::cancel );
  connect( buttonBox->button( QDialogButtonBox::Reset ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::reset );
  connect( buttonBox->button( QDialogButtonBox::Help ), &QAbstractButton::clicked, this, &QgsCustomizationDialog::showHelp );

}

QTreeWidgetItem *QgsCustomizationDialog::item( const QString &path, QTreeWidgetItem *widgetItem )
{
  QString pathCopy = path;
  if ( pathCopy.startsWith( '/' ) )
    pathCopy = pathCopy.mid( 1 ); // remove '/'
  QStringList names = pathCopy.split( '/' );
  pathCopy = QStringList( names.mid( 1 ) ).join( QLatin1Char( '/' ) );

  if ( ! widgetItem )
  {
    for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem *myItem = treeWidget->topLevelItem( i );
      QString objectName = myItem->text( 0 );
      if ( objectName == names[0] )
      {
        return item( pathCopy, myItem );
      }
    }
  }
  else
  {
    for ( int i = 0; i < widgetItem->childCount(); ++i )
    {
      QTreeWidgetItem *myItem = widgetItem->child( i );
      QString objectName = myItem->text( 0 );
      if ( objectName == names[0] )
      {
        if ( names.size() == 1 )
        {
          return myItem;
        }
        else
        {
          return item( pathCopy, myItem );
        }
      }
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "not found" ), 2 );
  return nullptr;
}

bool QgsCustomizationDialog::filterItems( const QString &text )
{
  bool success = false;

  mTreeInitialVisible.clear();
  // initially hide everything
  std::function< void( QTreeWidgetItem *, bool ) > setChildrenVisible;
  setChildrenVisible = [this, &setChildrenVisible]( QTreeWidgetItem * item, bool visible )
  {
    for ( int i = 0; i < item->childCount(); ++i )
      setChildrenVisible( item->child( i ), visible );
    mTreeInitialVisible.insert( item, !item->isHidden() );
    item->setHidden( !visible );
  };
  setChildrenVisible( treeWidget->invisibleRootItem(), false );

  QList<QTreeWidgetItem *> items = treeWidget->findItems( text, Qt::MatchContains | Qt::MatchRecursive, 0 );
  items.append( treeWidget->findItems( text, Qt::MatchContains | Qt::MatchRecursive, 1 ) );
  success = !items.empty();
  mTreeInitialExpand.clear();

  for ( QTreeWidgetItem *item : std::as_const( items ) )
  {
    setChildrenVisible( item, true );

    QTreeWidgetItem *parent = item;
    while ( parent )
    {
      if ( mTreeInitialExpand.contains( parent ) )
        break;
      mTreeInitialExpand.insert( parent, parent->isExpanded() );
      parent->setExpanded( true );
      parent->setHidden( false );
      parent = parent->parent();
    }
  }

  return success;
}

bool QgsCustomizationDialog::itemChecked( const QString &path )
{
  QgsDebugMsgLevel( QStringLiteral( "thePath = %1" ).arg( path ), 3 );
  QTreeWidgetItem *myItem = item( path );
  if ( !myItem )
    return true;
  return myItem->checkState( 0 ) == Qt::Checked;
}

void QgsCustomizationDialog::setItemChecked( const QString &path, bool on )
{
  QgsDebugMsgLevel( QStringLiteral( "thePath = %1 on = %2" ).arg( path ).arg( on ), 2 );
  QTreeWidgetItem *myItem = item( path );
  if ( !myItem )
    return;
  myItem->setCheckState( 0, on ? Qt::Checked : Qt::Unchecked );
}

void QgsCustomizationDialog::settingsToItem( const QString &path, QTreeWidgetItem *item, QSettings *settings )
{
  QString objectName = item->text( 0 );
  if ( objectName.isEmpty() )
    return; // object is not identifiable

  QString myPath = path + '/' + objectName;

  bool on = settings->value( myPath, true ).toBool();
  item->setCheckState( 0, on ? Qt::Checked : Qt::Unchecked );

  for ( int i = 0; i < item->childCount(); ++i )
  {
    QTreeWidgetItem *myItem = item->child( i );
    settingsToItem( myPath, myItem, settings );
  }
}

void QgsCustomizationDialog::itemToSettings( const QString &path, QTreeWidgetItem *item, QSettings *settings )
{

  QString objectName = item->text( 0 );
  if ( objectName.isEmpty() )
    return; // object is not identifiable

  QString myPath = path + '/' + objectName;
  bool on = item->checkState( 0 ) == Qt::Checked;
  settings->setValue( myPath, on );

  for ( int i = 0; i < item->childCount(); ++i )
  {
    QTreeWidgetItem *myItem = item->child( i );
    itemToSettings( myPath, myItem, settings );
  }
}

void QgsCustomizationDialog::treeToSettings( QSettings *settings )
{
  for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
  {
    itemToSettings( QStringLiteral( "/Customization" ), treeWidget->topLevelItem( i ), settings );
  }
}

void QgsCustomizationDialog::settingsToTree( QSettings *settings )
{
  for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
  {
    settingsToItem( QStringLiteral( "/Customization" ), treeWidget->topLevelItem( i ), settings );
  }
}

void QgsCustomizationDialog::reset()
{
  mSettings->sync();
  settingsToTree( mSettings );

  QSettings settings;
  bool enabled = settings.value( QStringLiteral( "UI/Customization/enabled" ), "false" ).toString() == QLatin1String( "true" );
  mCustomizationEnabledCheckBox->setChecked( enabled );
  treeWidget->setEnabled( enabled );
  toolBar->setEnabled( enabled );
  mLeFilter->setEnabled( enabled );
  mTreeInitialExpand.clear();
}

void QgsCustomizationDialog::ok()
{
  apply();
  hide();
}
void QgsCustomizationDialog::apply()
{
  QgsDebugMsgLevel( QStringLiteral( "columnCount = %1" ).arg( treeWidget->columnCount() ), 3 );
  treeToSettings( mSettings );
  mSettings->setValue( QgsCustomization::instance()->statusPath(), QgsCustomization::User );
  mSettings->sync();

  QSettings settings;
  settings.setValue( QStringLiteral( "UI/Customization/enabled" ), mCustomizationEnabledCheckBox->isChecked() );
}

void QgsCustomizationDialog::cancel()
{
  hide();
}

void QgsCustomizationDialog::actionSave_triggered( bool checked )
{
  Q_UNUSED( checked )
  QSettings mySettings;
  QString lastDir = mySettings.value( mLastDirSettingsName, QDir::homePath() ).toString();

  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Choose a customization INI file" ),
                     lastDir, tr( "Customization files (*.ini)" ) );

  if ( fileName.isEmpty() )
  {
    return;
  }

  if ( !fileName.endsWith( QLatin1String( ".ini" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".ini" );
  }

  QFileInfo fileInfo( fileName );
  mySettings.setValue( mLastDirSettingsName, fileInfo.absoluteDir().absolutePath() );

  QSettings fileSettings( fileName, QSettings::IniFormat );
  treeToSettings( &fileSettings );
}

void QgsCustomizationDialog::actionLoad_triggered( bool checked )
{
  Q_UNUSED( checked )
  QSettings mySettings;
  QString lastDir = mySettings.value( mLastDirSettingsName, QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName( this,
                     tr( "Choose a customization INI file" ),
                     lastDir, tr( "Customization files (*.ini)" ) );

  if ( fileName.isEmpty() )
    return;
  QFileInfo fileInfo( fileName );
  mySettings.setValue( mLastDirSettingsName, fileInfo.absoluteDir().absolutePath() );

  QSettings fileSettings( fileName, QSettings::IniFormat );
  settingsToTree( &fileSettings );
}

void QgsCustomizationDialog::actionExpandAll_triggered( bool checked )
{
  Q_UNUSED( checked )
  treeWidget->expandAll();
}

void QgsCustomizationDialog::actionCollapseAll_triggered( bool checked )
{
  Q_UNUSED( checked )
  treeWidget->collapseAll();
}

void QgsCustomizationDialog::actionSelectAll_triggered( bool checked )
{
  Q_UNUSED( checked )
  QList<QTreeWidgetItem *> items = treeWidget->findItems( QStringLiteral( "*" ), Qt::MatchWildcard | Qt::MatchRecursive, 0 );

  const auto constItems = items;
  for ( QTreeWidgetItem *item : constItems )
    item->setCheckState( 0, Qt::Checked );
}

void QgsCustomizationDialog::enableCustomization( bool checked )
{
  treeWidget->setEnabled( checked );
  toolBar->setEnabled( checked );
  mLeFilter->setEnabled( checked );
}

void QgsCustomizationDialog::init()
{
  QTreeWidgetItem *wi = createTreeItemWidgets();
  if ( wi )
  {
    treeWidget->insertTopLevelItem( 0, wi );
    treeWidget->expandItem( wi );
  }

  treeWidget->insertTopLevelItems( 0, QgsCustomization::instance()->mMainWindowItems );
  treeWidget->addTopLevelItem( QgsCustomization::instance()->mBrowserItem );

  for ( int i = 0; i < treeWidget->topLevelItemCount(); i++ )
    treeWidget->expandItem( treeWidget->topLevelItem( i ) );

  // load check states from the settings
  reset();

  treeWidget->sortItems( 0, Qt::AscendingOrder );
  treeWidget->resizeColumnToContents( 0 );
}

QTreeWidgetItem *QgsCustomizationDialog::createTreeItemWidgets()
{

  QDomDocument myDoc( QStringLiteral( "QgsWidgets" ) );
  QFile myFile( QgsApplication::pkgDataPath() +  "/resources/customization.xml" );
  if ( !myFile.open( QIODevice::ReadOnly ) )
  {
    return nullptr;
  }
  if ( !myDoc.setContent( &myFile ) )
  {
    myFile.close();
    return nullptr;
  }
  myFile.close();

  QDomElement myRoot = myDoc.documentElement();
  if ( myRoot.tagName() != QLatin1String( "qgiswidgets" ) )
  {
    return nullptr;
  }
  QTreeWidgetItem *myItem = readWidgetsXmlNode( myRoot );
  // Do not translate "Widgets", currently it is also used as path
  myItem->setData( 0, Qt::DisplayRole, "Widgets" );

  return myItem;
}

QTreeWidgetItem *QgsCustomizationDialog::readWidgetsXmlNode( const QDomNode &node )
{
  QDomElement myElement = node.toElement();

  QString name = myElement.attribute( QStringLiteral( "objectName" ), QString() );
  QStringList data( name );

  // remove '&' which are used to mark shortcut key
  data << myElement.attribute( QStringLiteral( "label" ), name ).remove( "&" );

  QTreeWidgetItem *myItem = new QTreeWidgetItem( data );

  // It is nice to have icons for each Qt widget class, is it too heavy?
  // There are 47 png files, total 196K in qt/tools/designer/src/components/formeditor/images/
  QString iconName = myElement.attribute( QStringLiteral( "class" ), QString() ).toLower().mid( 1 ) + ".png";
  QString iconPath = QgsApplication::iconPath( "/customization/" + iconName );
  QgsDebugMsgLevel( "iconPath = " + iconPath, 3 );
  if ( QFile::exists( iconPath ) )
  {
    myItem->setIcon( 0, QIcon( iconPath ) );
  }
  myItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  myItem->setCheckState( 0, Qt::Checked );

  QDomNode n = node.firstChild();
  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() )
    {
      QTreeWidgetItem *wi = readWidgetsXmlNode( n );
      myItem->insertChild( 0, wi );
    }
    n = n.nextSibling();
  }
  return myItem;
}

QAction *QgsCustomizationDialog::findAction( QToolButton *toolbutton )
{
  if ( !toolbutton->parent() )
    return toolbutton->defaultAction();

  // We need to find the QAction that was returned from the call of "QToolBar::addWidget".
  // This is a defaultAction in most cases. But when QToolButton is composed of multiple actions,
  // (e.g. "Select Features by ..." button) we need to go through the parent widget to search for the
  // parent action name.
  const QList<QWidgetAction *> tbWidgetActions = toolbutton->parent()->findChildren<QWidgetAction *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QWidgetAction *act : tbWidgetActions )
  {
    QWidget *widget = act->defaultWidget();
    if ( widget == toolbutton )
      return act;
  }

  return toolbutton->defaultAction();
}

bool QgsCustomizationDialog::switchWidget( QWidget *widget, QMouseEvent *e )
{
  Q_UNUSED( e )
  if ( !actionCatch->isChecked() )
    return false;

  QString path = widgetPath( widget );
  QgsDebugMsgLevel( "path = " + path, 3 );

  if ( path.contains( QLatin1String( "/QgsCustomizationDialogBase" ) ) )
  {
    // do not allow modification of this dialog
    return false;
  }
  else if ( path.startsWith( QLatin1String( "/QgisApp" ) ) )
  {
    // changes to main window
    // (work with toolbars, tool buttons)
    if ( widget->inherits( "QToolBar" ) )
    {
      path = "/Toolbars/" + widget->objectName();
    }
    else if ( widget->inherits( "QToolButton" ) )
    {
      QToolButton *toolbutton = qobject_cast<QToolButton *>( widget );
      QAction *action = findAction( toolbutton );
      if ( !action )
        return false;
      QString toolbarName = widget->parent()->objectName();
      QString actionName = action->objectName();
      path = "/Toolbars/" + toolbarName + '/' + actionName;
    }
    else
    {
      // unsupported widget in main window
      return false;
    }
  }
  else
  {
    // ordinary widget in a dialog
    path = "/Widgets" + path;
  }

  QgsDebugMsgLevel( "path final = " + path, 3 );
  bool on = !itemChecked( path );

  QgsDebugMsgLevel( QStringLiteral( "on = %1" ).arg( on ), 3 );

  setItemChecked( path, on );
  QTreeWidgetItem *myItem = item( path );
  if ( myItem )
  {
    treeWidget->scrollToItem( myItem, QAbstractItemView::PositionAtCenter );
    treeWidget->clearSelection();
    myItem->setSelected( true );

    QString style;
    if ( !on )
    {
      style = QStringLiteral( "background-color: #FFCCCC;" );
    }
    widget->setStyleSheet( style );
  }

  return true;
}

QString QgsCustomizationDialog::widgetPath( QWidget *widget, const QString &path )
{
  // go up until QDialog is reached
  QString name = widget->objectName();

  QString pathCopy = path;

  if ( !isInternalWidget( name ) )
  {
    if ( !pathCopy.isEmpty() )
    {
      pathCopy = name + '/' + pathCopy;
    }
    else
    {
      pathCopy = name;
    }
  }

  QWidget *parent = widget->parentWidget();

  if ( !parent || widget->inherits( "QDialog" ) )
  {
    return '/' + pathCopy;
  }

  return widgetPath( parent, pathCopy );
}

void QgsCustomizationDialog::setCatch( bool on )
{
  actionCatch->setChecked( on );
}
bool QgsCustomizationDialog::catchOn()
{
  return actionCatch->isChecked();
}

void QgsCustomizationDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/qgis_configuration.html#customization" ) );
}


void QgsCustomization::addTreeItemActions( QTreeWidgetItem *parentItem, const QList<QAction *> &actions )
{
  for ( const QAction *action : actions )
  {
    if ( action->isSeparator() )
    {
      continue;
    }
    if ( action->menu() )
    {
      // it is a submenu
      addTreeItemMenu( parentItem, action->menu(), action );
    }
    else
    {
      // it is an ordinary action
      QStringList strs;
      // remove '&' which are used to mark shortcut key
      strs << action->objectName() << action->text().remove( '&' );
      QTreeWidgetItem *item = new QTreeWidgetItem( parentItem, strs );
      item->setIcon( 0, action->icon() );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setCheckState( 0, Qt::Checked );
    }
  }
}

void QgsCustomization::addTreeItemMenu( QTreeWidgetItem *parentItem, const QMenu *menu, const QAction *action )
{
  QStringList menustrs;
  // remove '&' which are used to mark shortcut key
  menustrs << menu->objectName() << menu->title().remove( '&' );
  QTreeWidgetItem *menuItem = new QTreeWidgetItem( parentItem, menustrs );
  if ( action )
    menuItem->setIcon( 0, action->icon() );
  menuItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  menuItem->setCheckState( 0, Qt::Checked );

  addTreeItemActions( menuItem, menu->actions() );
}

void QgsCustomization::createTreeItemMenus()
{
  QStringList data;
  data << QStringLiteral( "Menus" );

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );

  QMenuBar *menubar = QgisApp::instance()->menuBar();
  const auto menus = menubar->findChildren<QMenu *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QMenu *menu : menus )
  {
    addTreeItemMenu( topItem, menu );
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemToolbars()
{
  QStringList data;
  data << QStringLiteral( "Toolbars" );

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );

  const auto toolbars = QgisApp::instance()->findChildren<QToolBar *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QToolBar *tb : toolbars )
  {
    QStringList tbstrs;
    tbstrs << tb->objectName() << tb->windowTitle();
    QTreeWidgetItem *tbItem = new QTreeWidgetItem( topItem, tbstrs );
    tbItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
    tbItem->setCheckState( 0, Qt::Checked );

    const QList<QWidgetAction *> tbWidgetActions = tb->findChildren<QWidgetAction *>( QString(), Qt::FindDirectChildrenOnly );
    QList<QAction *> tbActions = tb->actions();

    for ( QAction *act : tbWidgetActions )
    {
      QStringList actstrs;
      // remove '&' which are used to mark shortcut key
      actstrs << act->objectName() << act->text().remove( "&" );
      QTreeWidgetItem *item = new QTreeWidgetItem( tbItem, actstrs );
      item->setIcon( 0, act->icon() );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setCheckState( 0, Qt::Checked );

      QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>( act );
      QWidget *widget = widgetAction->defaultWidget();
      const QList<QAction *> childActions = widget->actions();
      addTreeItemActions( item, childActions );

      tbActions.removeAll( act );
    }

    addTreeItemActions( tbItem, tbActions );
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemDocks()
{
  QStringList data;
  data << QStringLiteral( "Docks" );

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );

  QMainWindow *mw = QgisApp::instance();
  const auto dockWidgets = mw->findChildren<QDockWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QDockWidget *dw : dockWidgets )
  {
    QStringList dwstrs;
    dwstrs << dw->objectName() << dw->windowTitle();
    QTreeWidgetItem *dwItem = new QTreeWidgetItem( topItem, dwstrs );
    dwItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
    dwItem->setCheckState( 0, Qt::Checked );
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemStatus()
{
  QStringList data;
  data << QStringLiteral( "StatusBar" );

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );
  topItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  topItem->setCheckState( 0, Qt::Checked );

  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  const auto children = sb->findChildren<QWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QWidget *child : children )
  {
    if ( !child->objectName().isEmpty() )
    {
      QStringList strs;
      strs << child->objectName();
      QTreeWidgetItem *item = new QTreeWidgetItem( topItem, strs );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setCheckState( 0, Qt::Checked );
    }
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemBrowser()
{
  if ( mBrowserItem )
    return;

  QStringList data;
  data << QStringLiteral( "Browser" );
  mBrowserItem = new QTreeWidgetItem( data );
  QVector<QStringList> items;

  items << QStringList( {QStringLiteral( "special:Home" ), tr( "Home Folder" )} );
  items << QStringList( {QStringLiteral( "special:ProjectHome" ), tr( "Project Home Folder" )} );
  items << QStringList( {QStringLiteral( "special:Favorites" ), tr( "Favorites Folder" )} );
  items << QStringList( {QStringLiteral( "special:Drives" ), tr( "Drive Folders (e.g. C:\\)" )} );
  items << QStringList( {QStringLiteral( "special:Volumes" ), tr( "Volume Folder (MacOS only)" )} );

  const auto constProviders = QgsApplication::dataItemProviderRegistry()->providers();
  for ( QgsDataItemProvider *pr : constProviders )
  {
    int capabilities = pr->capabilities();
    if ( capabilities != QgsDataProvider::NoDataCapabilities )
    {
      QStringList item;
      item << pr->name() << QObject::tr( "Data Item Provider: %1" ).arg( pr->name() );
      items << item;
    }
  }

  for ( const QStringList &strs : items )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem( mBrowserItem, strs );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
    item->setCheckState( 0, Qt::Checked );
  }
}

QgsCustomization *QgsCustomization::sInstance = nullptr;
QgsCustomization *QgsCustomization::instance()
{
  if ( !sInstance )
  {
    sInstance = new QgsCustomization();
  }
  return sInstance;
}

QgsCustomization::QgsCustomization()
  : mStatusPath( QStringLiteral( "/Customization/status" ) )
{

  QSettings settings;
  mEnabled = settings.value( QStringLiteral( "UI/Customization/enabled" ), "false" ).toString() == QLatin1String( "true" );
}

void QgsCustomization::updateMainWindow( QMenu *toolBarMenu, QMenu *panelMenu )
{
  // collect tree items even if the customization is disabled
  createTreeItemMenus();
  createTreeItemToolbars();
  createTreeItemDocks();
  createTreeItemStatus();

  if ( !mEnabled )
    return;

  QgisApp *mw = QgisApp::instance();
  QMenuBar *menubar = mw->menuBar();

  mSettings->beginGroup( QStringLiteral( "Customization/Menus" ) );

  // hide menus and menu actions

  const auto menus = menubar->findChildren<QMenu *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QMenu *menu : menus )
  {
    if ( !menu->objectName().isEmpty() )
    {
      bool visible = mSettings->value( menu->objectName(), true ).toBool();
      if ( !visible )
      {
        menubar->removeAction( menu->menuAction() );
      }
      else
      {
        updateMenu( menu, mSettings );
      }
    }
  }

  mSettings->endGroup();

  // remove toolbars, toolbar actions

  mSettings->beginGroup( QStringLiteral( "Customization/Toolbars" ) );
  const auto toolbars = mw->findChildren<QToolBar *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QToolBar *tb : toolbars )
  {
    if ( !tb->objectName().isEmpty() )
    {
      bool visible = mSettings->value( tb->objectName(), true ).toBool();
      if ( !visible )
      {
        mw->removeToolBar( tb );
        tb->setParent( nullptr );
        // remove also from menu, because toolbars removed here, switched on later from menu don't work correctly
        toolBarMenu->removeAction( tb->toggleViewAction() );
      }
      else
      {
        mSettings->beginGroup( tb->objectName() );
        // hide individual toolbar actions
        const auto constActions = tb->actions();
        for ( QAction *action : constActions )
        {
          if ( action->objectName().isEmpty() )
            continue;

          visible = mSettings->value( action->objectName(), true ).toBool();
          if ( !visible )
          {
            tb->removeAction( action );
            continue;
          }

          if ( action->metaObject()->className() == QLatin1String( "QWidgetAction" ) )
          {
            mSettings->beginGroup( action->objectName() );
            QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>( action );
            QWidget *widget = widgetAction->defaultWidget();
            const QList<QAction *> childActions = widget->actions();
            for ( QAction *wAction : childActions )
            {
              if ( wAction->objectName().isEmpty() )
                continue;

              visible = mSettings->value( wAction->objectName(), true ).toBool();
              if ( !visible )
                widget->removeAction( wAction );
            }
            mSettings->endGroup();
          }
        }
        mSettings->endGroup();
      }
    }
  }

  mSettings->endGroup();

  // remove dock widgets

  mSettings->beginGroup( QStringLiteral( "Customization/Docks" ) );
  const auto dockWidgets = mw->findChildren<QDockWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QDockWidget *dw : dockWidgets )
  {
    if ( !dw->objectName().isEmpty() )
    {
      bool visible = mSettings->value( dw->objectName(), true ).toBool();
      if ( !visible )
      {
        mw->removeDockWidget( dw );
        dw->setParent( nullptr );
        // remove also from menu, because dock removed here, switched on later from menu don't work correctly
        panelMenu->removeAction( dw->toggleViewAction() );
      }
    }
  }

  mSettings->endGroup();

  // remove status bar widgets

  if ( mSettings->value( QStringLiteral( "Customization/StatusBar" ), true ).toBool() )
  {
    mSettings->beginGroup( QStringLiteral( "Customization/StatusBar" ) );

    QgsStatusBar *sb = mw->statusBarIface();
    const auto children = sb->findChildren<QWidget *>();
    for ( QWidget *child : children )
    {
      if ( !child->objectName().isEmpty() )
      {
        bool visible = mSettings->value( child->objectName(), true ).toBool();
        if ( !visible )
        {
          sb->removeWidget( child );
        }
      }
    }

    mSettings->endGroup();
  }
  else
  {
    mw->statusBar()->hide();
    //mw->setStatusBar( 0 ); // do not delete the status bar: some parts of the app use it
  }
}

void QgsCustomization::updateMenu( QMenu *menu, QSettings *settings )
{
  settings->beginGroup( menu->objectName() );
  // hide individual menu actions and call recursively on visible submenus
  const auto constActions = menu->actions();
  for ( QAction *action : constActions )
  {
    QString objName = ( action->menu() ? action->menu()->objectName() : action->objectName() );
    if ( objName.isEmpty() )
    {
      continue;
    }
    bool visible = settings->value( objName, true ).toBool();
    if ( !visible )
      menu->removeAction( action );
    else if ( action->menu() )
    {
      // it is a submenu - let's look if there isn't something to remove
      updateMenu( action->menu(), settings );
    }
  }
  settings->endGroup();
}

void QgsCustomization::openDialog( QWidget *parent )
{
  if ( !pDialog )
  {
    pDialog = new QgsCustomizationDialog( parent, mSettings );
  }

  // I am trying too enable switching widget status by clicking in main app, so I need non modal
  pDialog->show();
}

void QgsCustomization::customizeWidget( QWidget *widget, QEvent *event, QSettings *settings )
{
  Q_UNUSED( event )
  // Test if the widget is child of QDialog
  if ( !widget->inherits( "QDialog" ) )
    return;

  QgsDebugMsgLevel( QStringLiteral( "objectName = %1 event type = %2" ).arg( widget->objectName() ).arg( event->type() ), 3 );

  QgsDebugMsgLevel( QStringLiteral( "%1 x %2" ).arg( widget->metaObject()->className(), QDialog::staticMetaObject.className() ), 3 );
  QString path = QStringLiteral( "/Customization/Widgets/" );

  QgsCustomization::customizeWidget( path, widget, settings );
}

void QgsCustomization::customizeWidget( const QString &path, QWidget *widget, QSettings *settings )
{
  QString name = widget->objectName();
  QString myPath = path;

  // Qt may insert some internal classes in the tree, e.g. QTabWidgetPrivate inserts
  // qt_tabwidget_stackedwidget, such widgets do not appear in the tree generated
  // from ui files and do not have sense from user point of view -> skip

  if ( !isInternalWidget( name ) )
  {
    myPath = path + '/' + name;
  }

  QObjectList children = widget->children();
  QObjectList::iterator i;
  for ( i = children.begin(); i != children.end(); ++i )
  {
    if ( !( *i )->inherits( "QWidget" ) )
      continue;
    QWidget *w = qobject_cast<QWidget *>( *i );

    QString p = myPath + '/' + w->objectName();

    bool on = settings->value( p, true ).toBool();
    //QgsDebugMsg( QStringLiteral( "p = %1 on = %2" ).arg( p ).arg( on ) );
    if ( on )
    {
      QgsCustomization::customizeWidget( myPath, w, settings );
    }
    else
    {
      QLayout *l = widget->layout();
      if ( l )
      {
        QgsDebugMsgLevel( QStringLiteral( "remove" ), 3 );
        QgsCustomization::removeFromLayout( l, w );
        w->hide();
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "hide" ), 3 );
        w->hide();
      }
    }
  }
}

void QgsCustomization::removeFromLayout( QLayout *layout, QWidget *widget )
{
  if ( layout->indexOf( widget ) >= 0 )
  {
    layout->removeWidget( widget );
    return;
  }
  else
  {
    QObjectList children = layout->children();
    QObjectList::iterator i;
    for ( i = children.begin(); i != children.end(); ++i )
    {
      if ( !( *i )->inherits( "QLayout" ) )
        continue;
      QLayout *l = qobject_cast<QLayout *>( *i );

      QgsCustomization::removeFromLayout( l, widget );
    }
  }
}

void QgsCustomization::updateBrowserWidget( QgsBrowserDockWidget *widget )
{
  createTreeItemBrowser();

  if ( !widget )
    return;

  if ( !mEnabled )
    return;

  if ( !mBrowserItem )
    return;

  QStringList disabledDataItems;
  mSettings->beginGroup( QStringLiteral( "Customization/Browser" ) );
  for ( int i = 0; i < mBrowserItem->childCount(); ++i )
  {
    const QTreeWidgetItem *item = mBrowserItem->child( i );
    if ( item && !mSettings->value( item->text( 0 ), true ).toBool() )
    {
      disabledDataItems << item->text( 0 );
    }
  }
  mSettings->endGroup();

  widget->setDisabledDataItemsKeys( disabledDataItems );
}

void QgsCustomization::preNotify( QObject *receiver, QEvent *event, bool *done )
{
  if ( event->type() == QEvent::Show || event->type() == QEvent::MouseButtonPress )
  {
    QWidget *widget = qobject_cast<QWidget *>( receiver );

    if ( mEnabled && widget && event->type() == QEvent::Show )
    {
      QgsCustomization::customizeWidget( widget, event, mSettings );
    }
    else if ( widget && event->type() == QEvent::MouseButtonPress )
    {
      //QgsDebugMsg( QStringLiteral( "click" ) );
      if ( pDialog && pDialog->isVisible() )
      {
        QMouseEvent *e = static_cast<QMouseEvent *>( event );
        *done = pDialog->switchWidget( widget, e );
      }
    }
  }
  // Shortcut arrives only if it is defined and used in main app
  // This would be also possible without necessity to add shortcut to main app
  // but it is better to have it there to avoid future conflicts
  else if ( event->type() == QEvent::KeyPress )
  {
    if ( pDialog && pDialog->isVisible() )
    {
      QKeyEvent *e = static_cast<QKeyEvent *>( event );
      //QgsDebugMsg( QStringLiteral( "key = %1 modifiers = %2" ).arg( e->key() ).arg( e->modifiers() ) );
      if ( e->key() == Qt::Key_M && e->modifiers() == Qt::ControlModifier )
      {
        pDialog->setCatch( !pDialog->catchOn() );
      }
    }
  }
}

QString QgsCustomization::splashPath() const
{
  if ( isEnabled() )
  {
    QString path = mSettings->value( QStringLiteral( "/Customization/splashpath" ), QgsApplication::splashPath() ).toString();
    return path;
  }
  else
  {
    return QgsApplication::splashPath();
  }
}

void QgsCustomization::loadDefault()
{
  QSettings mySettings;

  // Check customization state
  int status = mySettings.value( mStatusPath, QgsCustomization::NotSet ).toInt();
  QgsDebugMsgLevel( "Status path = " + mStatusPath, 2 );
  QgsDebugMsgLevel( QStringLiteral( "status = %1" ).arg( status ), 2 );
  if ( status == QgsCustomization::User || status == QgsCustomization::Default )
    return;

  // Look for default
  QString path = QgsApplication::pkgDataPath() +  "/resources/customization.ini";
  if ( ! QFile::exists( path ) )
  {
    QgsDebugMsgLevel( "Default customization not found in " + path, 2 );
    return;
  }
  QgsDebugMsgLevel( "Loading default customization from " + path, 2 );

  QSettings fileSettings( path );
  QStringList keys = fileSettings.allKeys();
  QgsDebugMsgLevel( QStringLiteral( "size = %1" ).arg( keys.size() ), 2 );
  QStringList::const_iterator i;
  for ( i = keys.constBegin(); i != keys.constEnd(); ++i )
  {
    QString p( *i );

    bool val = fileSettings.value( p ).toBool();

    mSettings->setValue( p, val );
  }
  mySettings.setValue( mStatusPath, QgsCustomization::Default );
}
