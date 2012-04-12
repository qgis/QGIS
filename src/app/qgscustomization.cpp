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

#include <QAction>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>
#include <QKeySequence>
#include <QToolButton>
#include <QStatusBar>
#include <QMetaObject>

#ifdef Q_OS_MACX
QgsCustomizationDialog::QgsCustomizationDialog( QWidget *parent )
    : QMainWindow( parent, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
    , mSettings( "QuantumGIS", "QGISCUSTOMIZATION" )
#else
QgsCustomizationDialog::QgsCustomizationDialog( QWidget *parent )
    : QMainWindow( parent )
    , mSettings( "QuantumGIS", "QGISCUSTOMIZATION" )
#endif
{
  setupUi( this );
  init();
  QStringList myHeaders;
  myHeaders << tr( "Object name" ) << tr( "Label" ) << tr( "Description" );
  treeWidget->setHeaderLabels( myHeaders );

  mLastDirSettingsName  = QString( "/UI/lastCustomizationDir" );
  //treeWidget->hideColumn(0)
  connect( buttonBox->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( ok() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), this, SLOT( cancel() ) );
  connect( buttonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ), this, SLOT( reset() ) );

}

QgsCustomizationDialog::~QgsCustomizationDialog()
{
}

QTreeWidgetItem * QgsCustomizationDialog::item( QString thePath, QTreeWidgetItem *theItem )
{
  QString path = thePath;
  if ( path.startsWith( "/" ) )
    path = path.mid( 1 ); // remove '/'
  QStringList names = path.split( '/' );
  path = QStringList( names.mid( 1 ) ).join( "/" );

  if ( ! theItem )
  {
    for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem *myItem = treeWidget->topLevelItem( i );
      QString objectName = myItem->text( 0 );
      if ( objectName == names[0] )
      {
        return item( path, myItem );
      }
    }
  }
  else
  {
    for ( int i = 0; i < theItem->childCount(); ++i )
    {
      QTreeWidgetItem *myItem = theItem->child( i );
      QString objectName = myItem->text( 0 );
      if ( objectName == names[0] )
      {
        if ( names.size() == 1 )
        {
          return myItem;
        }
        else
        {
          return item( path, myItem );
        }
      }
    }
  }
  QgsDebugMsg( "not found" ) ;
  return 0;
}

bool QgsCustomizationDialog::itemChecked( QString thePath )
{
  QgsDebugMsg( QString( "thePath = %1" ).arg( thePath ) );
  QTreeWidgetItem *myItem = item( thePath );
  if ( !myItem )
    return true;
  return myItem->checkState( 0 ) == Qt::Checked ? true : false;
}

void QgsCustomizationDialog::setItemChecked( QString thePath, bool on )
{
  QgsDebugMsg( QString( "thePath = %1 on = %2" ).arg( thePath ).arg( on ) );
  QTreeWidgetItem *myItem = item( thePath );
  if ( !myItem )
    return;
  myItem->setCheckState( 0, on ? Qt::Checked : Qt::Unchecked );
}

void QgsCustomizationDialog::settingsToItem( QString thePath, QTreeWidgetItem *theItem, QSettings *theSettings )
{
  QString objectName = theItem->text( 0 );
  if ( objectName.isEmpty() )
    return; // object is not identifiable

  QString myPath = thePath + "/" + objectName;

  bool on = theSettings->value( myPath, true ).toBool();
  theItem->setCheckState( 0, on ? Qt::Checked : Qt::Unchecked );

  for ( int i = 0; i < theItem->childCount(); ++i )
  {
    QTreeWidgetItem *myItem = theItem->child( i );
    settingsToItem( myPath, myItem, theSettings );
  }
}

void QgsCustomizationDialog::itemToSettings( QString thePath, QTreeWidgetItem *theItem, QSettings *theSettings )
{

  QString objectName = theItem->text( 0 );
  if ( objectName.isEmpty() )
    return; // object is not identifiable

  QString myPath = thePath + "/" + objectName;
  bool on = theItem->checkState( 0 ) == Qt::Checked ? true : false;
  theSettings->setValue( myPath, on );

  for ( int i = 0; i < theItem->childCount(); ++i )
  {
    QTreeWidgetItem *myItem = theItem->child( i );
    itemToSettings( myPath, myItem, theSettings );
  }
}

void QgsCustomizationDialog::treeToSettings( QSettings *theSettings )
{
  for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
  {
    itemToSettings( QString( "/Customization" ), treeWidget->topLevelItem( i ), theSettings );
  }
}

void QgsCustomizationDialog::settingsToTree( QSettings *theSettings )
{
  for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
  {
    settingsToItem( QString( "/Customization" ), treeWidget->topLevelItem( i ), theSettings );
  }
}

void QgsCustomizationDialog::reset()
{
  mSettings.sync();
  settingsToTree( &mSettings );
}

void QgsCustomizationDialog::ok()
{
  apply();
  hide();
}
void QgsCustomizationDialog::apply()
{
  QgsDebugMsg( QString( "columnCount = %1" ).arg( treeWidget->columnCount() ) );
  treeToSettings( &mSettings );
  mSettings.setValue( QgsCustomization::instance()->statusPath(), QgsCustomization::User );
  mSettings.sync();
}

void QgsCustomizationDialog::cancel()
{
  hide();
}

void QgsCustomizationDialog::on_actionSave_triggered( bool checked )
{
  Q_UNUSED( checked );
  QSettings mySettings;
  QString lastDir = mySettings.value( mLastDirSettingsName, "." ).toString();

  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Choose a customization INI file" ),
                     lastDir,  tr( "Customization files (*.ini)" ) );

  if ( fileName.isEmpty() )
    return;
  QFileInfo fileInfo( fileName );
  mySettings.setValue( mLastDirSettingsName, fileInfo.absoluteDir().absolutePath() );

  QSettings fileSettings( fileName, QSettings::IniFormat );
  treeToSettings( &fileSettings );
}

void QgsCustomizationDialog::on_actionLoad_triggered( bool checked )
{
  Q_UNUSED( checked );
  QSettings mySettings;
  QString lastDir = mySettings.value( mLastDirSettingsName, "." ).toString();

  QString fileName = QFileDialog::getOpenFileName( this,
                     tr( "Choose a customization INI file" ),
                     lastDir,  tr( "Customization files (*.ini)" ) );

  if ( fileName.isEmpty() )
    return;
  QFileInfo fileInfo( fileName );
  mySettings.setValue( mLastDirSettingsName, fileInfo.absoluteDir().absolutePath() );

  QSettings fileSettings( fileName, QSettings::IniFormat );
  settingsToTree( &fileSettings );
}

void QgsCustomizationDialog::on_actionExpandAll_triggered( bool checked )
{
  Q_UNUSED( checked );
  treeWidget->expandAll();
}

void QgsCustomizationDialog::on_actionCollapseAll_triggered( bool checked )
{
  Q_UNUSED( checked );
  treeWidget->collapseAll();
}

void QgsCustomizationDialog::on_actionSelectAll_triggered( bool checked )
{
  Q_UNUSED( checked );
  QList<QTreeWidgetItem*> items = treeWidget->findItems( "*", Qt::MatchWildcard | Qt::MatchRecursive, 0 );

  QList<QTreeWidgetItem*>::iterator i;
  for ( i = items.begin(); i != items.end(); ++i )
  {
    ( *i )->setCheckState( 0, Qt::Checked );
  }
}

void QgsCustomizationDialog::init()
{
  QgsDebugMsg( "Entered" );
  QTreeWidgetItem * wi = createTreeItemWidgets();
  if ( wi )
  {
    treeWidget->insertTopLevelItem( 0, wi );
    treeWidget->expandItem( wi );
  }

  treeWidget->insertTopLevelItems( 0, QgsCustomization::instance()->mMainWindowItems );

  for ( int i = 0; i < treeWidget->topLevelItemCount(); i++ )
    treeWidget->expandItem( treeWidget->topLevelItem( i ) );

  // load check states from the settings
  reset();

  treeWidget->sortItems( 0, Qt::AscendingOrder );
  treeWidget->resizeColumnToContents( 0 );
}

QTreeWidgetItem * QgsCustomizationDialog::createTreeItemWidgets()
{
  QgsDebugMsg( "Entered" );

  QDomDocument myDoc( "QgsWidgets" );
  QFile myFile( QgsApplication::pkgDataPath() +  "/resources/customization.xml" );
  if ( !myFile.open( QIODevice::ReadOnly ) )
  {
    return NULL;
  }
  if ( !myDoc.setContent( &myFile ) )
  {
    myFile.close();
    return NULL;
  }
  myFile.close();

  QDomElement myRoot = myDoc.documentElement();
  if ( myRoot.tagName() != "qgiswidgets" )
  {
    return NULL;
  }
  QTreeWidgetItem *myItem = readWidgetsXmlNode( myRoot );
  myItem->setData( 0, Qt::DisplayRole, tr( "Widgets" ) );

  return myItem;
}

QTreeWidgetItem * QgsCustomizationDialog::readWidgetsXmlNode( QDomNode theNode )
{
  QgsDebugMsg( "Entered" );
  QDomElement myElement = theNode.toElement();

  QString name = myElement.attribute( "objectName", "" );
  QStringList data( name );

  data << myElement.attribute( "label", name );
  data << myElement.attribute( "description", "" );

  QTreeWidgetItem *myItem = new QTreeWidgetItem( data );

  // It is nice to have icons for each Qt widget class, is it too heavy?
  // There are 47 png files, total 196K in qt/tools/designer/src/components/formeditor/images/
  QString iconName = myElement.attribute( "class", "" ).toLower().mid( 1 ) + ".png";
  QString iconPath = QgsApplication::iconPath( "/customization/" + iconName );
  QgsDebugMsg( "iconPath = " + iconPath );
  if ( QFile::exists( iconPath ) )
  {
    myItem->setIcon( 0, QIcon( iconPath ) );
  }
  myItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  myItem->setCheckState( 0, Qt::Checked );

  QDomNode n = theNode.firstChild();
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

bool QgsCustomizationDialog::switchWidget( QWidget *widget, QMouseEvent *e )
{
  Q_UNUSED( e );
  QgsDebugMsg( "Entered" );
  if ( !actionCatch->isChecked() )
    return false;
  QString path = widgetPath( widget );
  QgsDebugMsg( "path = " + path );

  if ( path.startsWith( "/QgsCustomizationDialogBase" ) )
  {
    // do not allow modification of this dialog
    return false;
  }
  else if ( path.startsWith( "/QgisApp" ) )
  {
    // changes to main window
    // (work with toolbars, tool buttons)
    if ( widget->inherits( "QToolBar" ) )
    {
      path = "/Toolbars/" + widget->objectName();
    }
    else if ( widget->inherits( "QToolButton" ) )
    {
      QToolButton* toolbutton = qobject_cast<QToolButton*>( widget );
      QAction* action = toolbutton->defaultAction();
      QString toolbarName = widget->parent()->objectName();
      QString actionName = action->objectName();
      path = "/Toolbars/" + toolbarName + "/" + actionName;
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

  QgsDebugMsg( "path final = " + path );
  bool on = !itemChecked( path );

  QgsDebugMsg( QString( "on = %1" ).arg( on ) );

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
      style = "background-color: #FFCCCC;";
    }
    widget->setStyleSheet( style );
  }

  return true;
}

QString QgsCustomizationDialog::widgetPath( QWidget * theWidget, QString thePath )
{
  // go up until QDialog is reached
  QString name = theWidget->objectName();

  QString path = thePath;

  if ( !QgsCustomization::mInternalWidgets.contains( name ) )
  {
    if ( !path.isEmpty() )
    {
      path = name + "/" + path;
    }
    else
    {
      path = name;
    }
  }

  QWidget * parent = theWidget->parentWidget();

  if ( !parent || theWidget->inherits( "QDialog" ) )
  {
    return "/" + path;
  }

  return widgetPath( parent, path );
}

void QgsCustomizationDialog::setCatch( bool on )
{
  actionCatch->setChecked( on );
}
bool QgsCustomizationDialog::catchOn( )
{
  return actionCatch->isChecked( );
}

void QgsCustomization::addTreeItemActions( QTreeWidgetItem* parentItem, const QList<QAction*>& actions )
{
  foreach( QAction* action, actions )
  {
    if ( action->menu() )
    {
      // it is a submenu
      addTreeItemMenu( parentItem, action->menu() );
    }
    else
    {
      // it is an ordinary action
      QStringList strs;
      strs << action->objectName() << action->text();
      QTreeWidgetItem* myItem = new QTreeWidgetItem( parentItem, strs );
      myItem->setIcon( 0, action->icon() );
      myItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      myItem->setCheckState( 0, Qt::Checked );
    }
  }
}

void QgsCustomization::addTreeItemMenu( QTreeWidgetItem* parentItem, QMenu* menu )
{
  QStringList menustrs;
  menustrs << menu->objectName() << menu->title();
  QTreeWidgetItem* menuItem = new QTreeWidgetItem( parentItem, menustrs );
  menuItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  menuItem->setCheckState( 0, Qt::Checked );

  addTreeItemActions( menuItem, menu->actions() );
}

void QgsCustomization::createTreeItemMenus( )
{
  QStringList data;
  data << "Menus";

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );

  QMenuBar* menubar = QgisApp::instance()->menuBar();
  foreach( QObject* obj, menubar->children() )
  {
    if ( obj->inherits( "QMenu" ) )
    {
      QMenu* menu = qobject_cast<QMenu*>( obj );
      addTreeItemMenu( topItem, menu );
    }
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemToolbars( )
{
  QStringList data;
  data << "Toolbars";

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );

  QMainWindow* mw = QgisApp::instance();
  foreach( QObject* obj, mw->children() )
  {
    if ( obj->inherits( "QToolBar" ) )
    {
      QToolBar* tb = qobject_cast<QToolBar*>( obj );
      QStringList tbstrs;
      tbstrs << tb->objectName() << tb->windowTitle();
      QTreeWidgetItem* tbItem = new QTreeWidgetItem( topItem, tbstrs );
      tbItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      tbItem->setCheckState( 0, Qt::Checked );

      addTreeItemActions( tbItem, tb->actions() );
    }
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemDocks( )
{
  QStringList data;
  data << "Docks";

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );

  QMainWindow* mw = QgisApp::instance();
  foreach( QObject* obj, mw->children() )
  {
    if ( obj->inherits( "QDockWidget" ) )
    {
      QDockWidget* dw = qobject_cast<QDockWidget*> ( obj );
      QStringList dwstrs;
      dwstrs << dw->objectName() << dw->windowTitle();
      QTreeWidgetItem* dwItem = new QTreeWidgetItem( topItem, dwstrs );
      dwItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      dwItem->setCheckState( 0, Qt::Checked );
    }
  }

  mMainWindowItems << topItem;
}

void QgsCustomization::createTreeItemStatus( )
{
  QStringList data;
  data << "StatusBar";

  QTreeWidgetItem *topItem = new QTreeWidgetItem( data );
  topItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  topItem->setCheckState( 0, Qt::Checked );

  QStatusBar* sb = QgisApp::instance()->statusBar();
  foreach( QObject* obj, sb->children() )
  {
    if ( obj->inherits( "QWidget" ) && !obj->objectName().isEmpty() )
    {
      QStringList strs;
      strs << obj->objectName();
      QTreeWidgetItem* item = new QTreeWidgetItem( topItem, strs );
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setCheckState( 0, Qt::Checked );
    }
  }

  mMainWindowItems << topItem;
}

QStringList QgsCustomization::mInternalWidgets = QStringList() <<  "qt_tabwidget_stackedwidget" << "qt_tabwidget_tabbar";

QgsCustomization *QgsCustomization::pinstance = 0;
QgsCustomization *QgsCustomization::instance()
{
  if ( pinstance == 0 )
  {
    pinstance = new QgsCustomization();
  }
  return pinstance;
}

QgsCustomization::QgsCustomization()
    : pDialog( 0 )
    , mEnabled( true )
    , mStatusPath( "/Customization/status" )
    , mSettings( "QuantumGIS", "QGISCUSTOMIZATION" )
{
  QgsDebugMsg( "Entered" );
}

QgsCustomization::~QgsCustomization()
{
}

void QgsCustomization::updateMainWindow( QMenu * theToolBarMenu )
{
  // collect tree items even if the customization is disabled
  createTreeItemMenus();
  createTreeItemToolbars();
  createTreeItemDocks();
  createTreeItemStatus();

  if ( !mEnabled )
    return;

  QMainWindow* mw = QgisApp::instance();
  QMenuBar* menubar = mw->menuBar();

  mSettings.beginGroup( "Customization/Menus" );

  // hide menus and menu actions

  foreach( QObject* obj, menubar->children() )
  {
    if ( obj->inherits( "QMenu" ) )
    {
      QMenu* menu = qobject_cast<QMenu*>( obj );
      bool visible = mSettings.value( menu->objectName(), true ).toBool();
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

  mSettings.endGroup();

  // remove toolbars, toolbar actions

  mSettings.beginGroup( "Customization/Toolbars" );
  foreach( QObject* obj, mw->children() )
  {
    if ( obj->inherits( "QToolBar" ) )
    {
      QToolBar* tb = qobject_cast<QToolBar*>( obj );
      bool visible = mSettings.value( tb->objectName(), true ).toBool();
      if ( !visible )
      {
        mw->removeToolBar( tb );
        // remove also from menu, because toolbars removed here, switched on later from menu don't work correctly
        theToolBarMenu->removeAction( tb->toggleViewAction() );
      }
      else
      {
        mSettings.beginGroup( tb->objectName() );
        // hide individual toolbar actions
        foreach( QAction* action, tb->actions() )
        {
          if ( action->objectName().isEmpty() )
          {
            continue;
          }
          visible = mSettings.value( action->objectName(), true ).toBool();
          if ( !visible )
            tb->removeAction( action );
        }
        mSettings.endGroup();
      }
    }
  }

  mSettings.endGroup();

  // remove dock widgets

  mSettings.beginGroup( "Customization/Docks" );
  foreach( QObject* obj, mw->children() )
  {
    if ( obj->inherits( "QDockWidget" ) )
    {
      bool visible = mSettings.value( obj->objectName(), true ).toBool();
      if ( !visible )
      {
        mw->removeDockWidget( qobject_cast<QDockWidget*>( obj ) );
      }
    }
  }

  mSettings.endGroup();

  // remove status bar widgets

  if ( mSettings.value( "Customization/StatusBar", true ).toBool() )
  {
    mSettings.beginGroup( "Customization/StatusBar" );

    QStatusBar* sb = mw->statusBar();
    foreach( QObject* obj, sb->children() )
    {
      if ( obj->inherits( "QWidget" ) )
      {
        QWidget* widget = qobject_cast<QWidget*>( obj );
        if ( widget->objectName().isEmpty() )
        {
          continue;
        }
        bool visible = mSettings.value( widget->objectName(), true ).toBool();
        if ( !visible )
        {
          sb->removeWidget( widget );
        }
      }
    }

    mSettings.endGroup();
  }
  else
  {
    mw->statusBar()->hide();
    //mw->setStatusBar( 0 ); // do not delete the status bar: some parts of the app use it
  }
}

void QgsCustomization::updateMenu( QMenu* menu, QSettings& settings )
{
  settings.beginGroup( menu->objectName() );
  // hide individual menu actions and call recursively on visible submenus
  foreach( QAction* action, menu->actions() )
  {
    QString objName = ( action->menu() ? action->menu()->objectName() : action->objectName() );
    if ( objName.isEmpty() )
    {
      continue;
    }
    bool visible = settings.value( objName, true ).toBool();
    if ( !visible )
      menu->removeAction( action );
    else if ( action->menu() )
    {
      // it is a submenu - let's look if there isn't something to remove
      updateMenu( action->menu(), settings );
    }
  }
  settings.endGroup();
}

void QgsCustomization::openDialog( QWidget *parent )
{
  QgsDebugMsg( "Entered" );
  if ( !pDialog )
  {
    pDialog = new QgsCustomizationDialog( parent );
  }

  // I am trying too enable switching widget status by clicking in main app, so I need non modal
  pDialog->show();
}

void QgsCustomization::customizeWidget( QWidget * widget, QEvent * event )
{
  Q_UNUSED( event );
  // Test if the widget is child of QDialog
  if ( !widget->inherits( "QDialog" ) )
    return;

  QgsDebugMsg( QString( "objectName = %1 event type = %2" ).arg( widget->objectName() ).arg( event->type() ) );

  QgsDebugMsg( QString( "%1 x %2" ).arg( widget->metaObject()->className() ).arg( QDialog::staticMetaObject.className() ) );
  QString path = "/Customization/Widgets/";

  QgsCustomization::customizeWidget( path, widget );
}

void QgsCustomization::customizeWidget( QString thePath, QWidget * theWidget )
{
  QSettings mySettings( "QuantumGIS", "QGISCUSTOMIZATION" );
  QString name = theWidget->objectName();
  QString myPath = thePath;

  // Qt may insert some internal classes in the tree, e.g. QTabWidgetPrivate inserts
  // qt_tabwidget_stackedwidget, such widgets do not appear in the tree generated
  // from ui files and do not have sense from user poin of view -> skip

  if ( !QgsCustomization::mInternalWidgets.contains( name ) )
  {
    myPath = thePath + "/" + name;
  }

  QObjectList children = theWidget->children();
  QObjectList::iterator i;
  for ( i = children.begin(); i != children.end(); ++i )
  {
    if ( !( *i )->inherits( "QWidget" ) )
      continue;
    QWidget * w = qobject_cast<QWidget*>( *i );

    QString p = myPath + "/" + w->objectName();

    bool on = mySettings.value( p, true ).toBool();
    //QgsDebugMsg( QString( "p = %1 on = %2" ).arg( p ).arg( on ) );
    if ( on )
    {
      QgsCustomization::customizeWidget( myPath, w );
    }
    else
    {
      QLayout *l = theWidget->layout();
      if ( l )
      {
        QgsDebugMsg( "remove" );
        QgsCustomization::removeFromLayout( l, w );
        w->hide();
      }
      else
      {
        QgsDebugMsg( "hide" );
        w->hide();
      }
    }
  }
}

void QgsCustomization::removeFromLayout( QLayout *theLayout, QWidget * theWidget )
{
  if ( theLayout->indexOf( theWidget ) >= 0 )
  {
    theLayout->removeWidget( theWidget );
    return;
  }
  else
  {
    QObjectList children = theLayout->children();
    QObjectList::iterator i;
    for ( i = children.begin(); i != children.end(); ++i )
    {
      if ( !( *i )->inherits( "QLayout" ) )
        continue;
      QLayout *l = qobject_cast<QLayout*>( *i );

      QgsCustomization::removeFromLayout( l, theWidget );
    }
  }
}

void QgsCustomization::preNotify( QObject * receiver, QEvent * event, bool * done )
{
  if ( event->type() == QEvent::Show || event->type() == QEvent::MouseButtonPress )
  {
    QWidget *widget = qobject_cast<QWidget*>( receiver );

    if ( mEnabled && widget && event->type() == QEvent::Show )
    {
      QgsCustomization::customizeWidget( widget, event );
    }
    else if ( widget && event->type() == QEvent::MouseButtonPress )
    {
      //QgsDebugMsg( "click" );
      if ( pDialog && pDialog->isVisible() )
      {
        QMouseEvent *e = static_cast<QMouseEvent*>( event );
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
      QKeyEvent *e = static_cast<QKeyEvent*>( event );
      //QgsDebugMsg( QString( "key = %1 modifiers = %2" ).arg( e->key() ).arg( e->modifiers() ) ) ;
      if ( e->key() == Qt::Key_M && e->modifiers() == Qt::ControlModifier )
      {
        pDialog->setCatch( !pDialog->catchOn() );
      }
    }
  }
}

void QgsCustomization::loadDefault()
{
  QSettings mySettings;

  // Check customization state
  int status = mySettings.value( mStatusPath, QgsCustomization::NotSet ).toInt();
  QgsDebugMsg( "Status path = " + mStatusPath );
  QgsDebugMsg( QString( "status = %1" ).arg( status ) );
  if ( status == QgsCustomization::User || status == QgsCustomization::Default )
    return;

  // Look for default
  QString path =  QgsApplication::pkgDataPath() +  "/resources/customization.ini";
  if ( ! QFile::exists( path ) )
  {
    QgsDebugMsg( "Default customization not found in " + path );
    return;
  }
  QgsDebugMsg( "Loading default customization from " + path );

  QSettings fileSettings( path, QSettings::IniFormat );
  QStringList keys = fileSettings.allKeys();
  QgsDebugMsg( QString( "size = %1" ).arg( keys.size() ) );
  QStringList::const_iterator i;
  for ( i = keys.begin(); i != keys.end(); ++i )
  {
    QString p( *i );

    bool val = fileSettings.value( p ).toBool();

    mSettings.setValue( p, val );
  }
  mySettings.setValue( mStatusPath, QgsCustomization::Default );
}
