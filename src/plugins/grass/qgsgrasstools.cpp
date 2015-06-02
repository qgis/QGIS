/***************************************************************************
                              qgsgrasstools.cpp
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrasstools.h"
#include "qgsgrassmodule.h"
#include "qgsgrassshell.h"
#include "qgsgrass.h"
#include "qgsconfig.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QCloseEvent>
#include <QDomDocument>
#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>


//
// For experimental model view alternative ui by Tim
//
//
#include "qgsdetaileditemdata.h"
#include "qgsdetaileditemdelegate.h"
#include <QSortFilterProxyModel>
#include <QStandardItem>

#ifdef Q_OS_WIN
#include "qgsgrassutils.h"
#endif


QgsGrassTools::QgsGrassTools( QgisInterface *iface, QWidget * parent, const char * name, Qt::WindowFlags f )
    : QDockWidget( parent, f )
    , mModulesListModel( 0 )
    , mModelProxy( 0 )
    , mDirectModulesListModel( 0 )
    , mDirectModelProxy( 0 )
{
  Q_UNUSED( name );
  setupUi( this );
  QgsDebugMsg( "QgsGrassTools()" );
  qRegisterMetaType<QgsDetailedItemData>();

  setWindowTitle( tr( "GRASS Tools" ) );
  //    setupUi(this);

  mIface = iface;
  mCanvas = mIface->mapCanvas();

  //statusBar()->hide();

  // set the dialog title
  QString title = tr( "GRASS Tools: %1/%2" ).arg( QgsGrass::getDefaultLocation() ).arg( QgsGrass::getDefaultMapset() );
  setWindowTitle( title );

  // Tree view code.
  mModulesTree->header()->hide();
  connect( mModulesTree, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
           this, SLOT( moduleClicked( QTreeWidgetItem *, int ) ) );

  mDirectModulesTree->header()->hide();
  connect( mDirectModulesTree, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
           this, SLOT( directModuleClicked( QTreeWidgetItem *, int ) ) );

  // List view with filter
  mModulesListModel = new QStandardItemModel( 0, 1 );
  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModulesListModel );
  mModelProxy->setFilterRole( Qt::UserRole + 2 );

  mListView->setModel( mModelProxy );
  connect( mListView, SIGNAL( clicked( const QModelIndex ) ),
           this, SLOT( listItemClicked( const QModelIndex ) ) );

  mDirectModulesListModel = new QStandardItemModel( 0, 1 );
  mDirectModelProxy = new QSortFilterProxyModel( this );
  mDirectModelProxy->setSourceModel( mDirectModulesListModel );
  mDirectModelProxy->setFilterRole( Qt::UserRole + 2 );

  mDirectListView->setModel( mDirectModelProxy );
  connect( mDirectListView, SIGNAL( clicked( const QModelIndex ) ),
           this, SLOT( directListItemClicked( const QModelIndex ) ) );

  // Show before loadConfig() so that user can see loading
  restorePosition();
  showTabs();
}

void QgsGrassTools::showTabs()
{
  QgsDebugMsg( "entered." );

  QString title;
  if ( QgsGrass::activeMode() )
  {
    title = tr( "GRASS Tools: %1/%2" ).arg( QgsGrass::getDefaultLocation() ).arg( QgsGrass::getDefaultMapset() );
  }
  else
  {
#ifdef GRASS_DIRECT
    title = tr( "GRASS Direct Tools" );
#else
    title = tr( "GRASS Tools" );
#endif
  }
  setWindowTitle( title );

  // we always show tabs but disabled if not active
  // direct mode currently disabled
  mTabWidget->removeTab( mTabWidget->indexOf( mDirectModulesTreeTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mDirectModulesListTab ) );
#if 0
  mTabWidget->removeTab( mTabWidget->indexOf( mModulesListTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mModulesTreeTab ) );

  mTabWidget->insertTab( 0, mModulesTreeTab, tr( "Modules Tree" ) );
  mTabWidget->insertTab( 1, mModulesListTab, tr( "Modules List" ) );

  repaint();
#endif

  QString conf = QgsApplication::pkgDataPath() + "/grass/config/default.qgc";
  if ( QgsGrass::activeMode() )
  {
    QgsDebugMsg( QString( "topLevelItemCount = %1" ).arg( mModulesTree->topLevelItemCount() ) );
    if ( mModulesTree->topLevelItemCount() == 0 )
    {
      // Load the modules lists
      QApplication::setOverrideCursor( Qt::WaitCursor );
      loadConfig( conf, mModulesTree, mModulesListModel, false );
      QApplication::restoreOverrideCursor();
    }
    QgsDebugMsg( QString( "topLevelItemCount = %1" ).arg( mModulesTree->topLevelItemCount() ) );
    mTabWidget->setEnabled( true );
  }
  else
  {
#ifdef GRASS_DIRECT
    // Remove open indirect modules tabs
    for ( int i = mTabWidget->count() - 1; i >= 0; i-- )
    {
      QgsGrassModule *module = qobject_cast<QgsGrassModule *>( mTabWidget->widget( i ) );
      if ( module && !module->isDirect() )
      {
        mTabWidget->removeTab( i );
        delete module;
      }
    }

    mTabWidget->insertTab( 0, mDirectModulesTreeTab, tr( "Direct Modules Tree" ) );
    mTabWidget->insertTab( 1, mDirectModulesListTab, tr( "Direct Modules List" ) );
    repaint();
    if ( mDirectModulesTree->topLevelItemCount() == 0 )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      loadConfig( conf, mDirectModulesTree, mDirectModulesListModel, true );
      QApplication::restoreOverrideCursor();
    }
#else
    mTabWidget->setEnabled( false );
#endif
  }
}

void QgsGrassTools::moduleClicked( QTreeWidgetItem * item, int column )
{
  Q_UNUSED( column );
  QgsDebugMsg( "entered." );
  if ( !item )
    return;

  QString name = item->text( 1 );
  QgsDebugMsg( QString( "name = %1" ).arg( name ) );
  runModule( name, false );
}

void QgsGrassTools::directModuleClicked( QTreeWidgetItem * item, int column )
{
  Q_UNUSED( column );
  QgsDebugMsg( "entered." );
  if ( !item )
    return;

  QString name = item->text( 1 );
  QgsDebugMsg( QString( "name = %1" ).arg( name ) );
  runModule( name, true );
}

void QgsGrassTools::runModule( QString name, bool direct )
{
  if ( name.length() == 0 )
    return;  // Section

#if defined(HAVE_OPENPTY) && !defined(Q_OS_WIN)
  QgsGrassShell* sh = 0;
#endif

  QString path = QgsApplication::pkgDataPath() + "/grass/modules/" + name;
  QgsDebugMsg( QString( "path = %1" ).arg( path ) );
  QWidget *m;
  if ( name == "shell" )
  {
#ifdef Q_OS_WIN
    QgsGrass::putEnv( "GRASS_HTML_BROWSER", QgsGrassUtils::htmlBrowserPath() );
    if ( !QProcess::startDetached( getenv( "COMSPEC" ) ) )
    {
      QMessageBox::warning( 0, "Warning", tr( "Cannot start command shell (%1)" ).arg( getenv( "COMSPEC" ) ) );
    }
    return;
#else

#ifdef HAVE_OPENPTY
    sh = new QgsGrassShell( this, mTabWidget );
    m = qobject_cast<QWidget *>( sh );
#else
    QMessageBox::warning( 0, tr( "Warning" ), tr( "GRASS Shell is not compiled." ) );
#endif // HAVE_OPENPTY

#endif // ! Q_OS_WIN
  }
  else
  {
    QgsGrassModule *gmod = new QgsGrassModule( this, name, mIface, path, direct, mTabWidget );
    m = qobject_cast<QWidget *>( gmod );
  }

  int height = mTabWidget->iconSize().height();
  QPixmap pixmap = QgsGrassModule::pixmap( path, height );

  // Icon size in QT4 does not seem to be variable
  // -> reset the width to max icon width
  if ( mTabWidget->iconSize().width() < pixmap.width() )
  {
    mTabWidget->setIconSize( QSize( pixmap.width(), mTabWidget->iconSize().height() ) );
  }

  QIcon is;
  is.addPixmap( pixmap );
  mTabWidget->addTab( m, is, "" );


  mTabWidget->setCurrentIndex( mTabWidget->count() - 1 );

  // We must call resize to reset COLUMNS environment variable
  // used by bash !!!

#if 0
  /* TODO: Implement something that resizes the terminal without
   *       crashes.
   */
#ifdef HAVE_OPENPTY
  if ( sh )
    sh->resizeTerminal();
#endif
#endif
}

bool QgsGrassTools::loadConfig( QString filePath, QTreeWidget *modulesTreeWidget, QStandardItemModel * modulesListModel, bool direct )
{
  QgsDebugMsg( filePath );
  modulesTreeWidget->clear();
  modulesTreeWidget->setIconSize( QSize( 80, 22 ) );

  QFile file( filePath );

  if ( !file.exists() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "The config file (%1) not found." ).arg( filePath ) );
    return false;
  }
  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open config file (%1)." ).arg( filePath ) );
    return false;
  }

  QDomDocument doc( "qgisgrass" );
  QString err;
  int line, column;
  if ( !doc.setContent( &file,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read config file (%1):" ).arg( filePath )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    file.close();
    return false;
  }

  QDomElement docElem = doc.documentElement();
  QDomNodeList modulesNodes = docElem.elementsByTagName( "modules" );

  if ( modulesNodes.count() == 0 )
  {
    file.close();
    return false;
  }

  QDomNode modulesNode = modulesNodes.item( 0 );
  QDomElement modulesElem = modulesNode.toElement();

  // Go through the sections and modules and add them to the list view
  addModules( 0, modulesElem, modulesTreeWidget, modulesListModel, direct );
  if ( direct )
  {
    removeEmptyItems( modulesTreeWidget );
  }
  modulesTreeWidget->topLevelItem( 0 )->setExpanded( true );

  file.close();
  return true;
}

void QgsGrassTools::addModules( QTreeWidgetItem *parent, QDomElement &element, QTreeWidget *modulesTreeWidget, QStandardItemModel * modulesListModel, bool direct )
{
  QDomNode n = element.firstChild();

  QTreeWidgetItem *item;
  QTreeWidgetItem *lastItem = 0;
  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() )
    {
// QgsDebugMsg(QString("tag = %1").arg(e.tagName()));

      if ( e.tagName() != "section" && e.tagName() != "grass" )
      {
        QgsDebugMsg( QString( "Unknown tag: %1" ).arg( e.tagName() ) );
        continue;
      }

      // Check GRASS version
      QString version_min = e.attribute( "version_min" );
      QString version_max = e.attribute( "version_max" );

      if ( !QgsGrassModuleOption::checkVersion( e.attribute( "version_min" ), e.attribute( "version_max" ) ) )
      {
        n = n.nextSibling();
        continue;
      }

      if ( parent )
      {
        item = new QTreeWidgetItem( parent, lastItem );
      }
      else
      {
        item = new QTreeWidgetItem( modulesTreeWidget, lastItem );
      }

      if ( e.tagName() == "section" )
      {
        QString label = QApplication::translate( "grasslabel", e.attribute( "label" ).toUtf8() );
        QgsDebugMsg( QString( "label = %1" ).arg( label ) );
        item->setText( 0, label );
        item->setExpanded( false );

        addModules( item, e, modulesTreeWidget, modulesListModel, direct );

        lastItem = item;
      }
      else if ( e.tagName() == "grass" )
      { // GRASS module
        QString name = e.attribute( "name" );
        QgsDebugMsg( QString( "name = %1" ).arg( name ) );

        QString path = QgsApplication::pkgDataPath() + "/grass/modules/" + name;
        QgsGrassModule::Description description = QgsGrassModule::description( path );

        if ( !direct || description.direct )
        {
          QString label = description.label;
          QPixmap pixmap = QgsGrassModule::pixmap( path, 32 );

          item->setText( 0, name + " - " + label );
          item->setIcon( 0, QIcon( pixmap ) );
          item->setText( 1, name );
          lastItem = item;

          // Add this item to our list model
          QStandardItem * mypDetailItem = new QStandardItem( name + "\n" + label );
          mypDetailItem->setData( name, Qt::UserRole + 1 ); //for calling runModule later
          QString mySearchText = name + " - " + label;
          mypDetailItem->setData( mySearchText, Qt::UserRole + 2 ); //for filtering later
          mypDetailItem->setData( pixmap, Qt::DecorationRole );
          mypDetailItem->setCheckable( false );
          mypDetailItem->setEditable( false );
          // setData in the delegate with a variantised QgsDetailedItemData
          QgsDetailedItemData myData;
          myData.setTitle( name );
          myData.setDetail( label );
          myData.setIcon( pixmap );
          myData.setCheckable( false );
          myData.setRenderAsWidget( false );
          QVariant myVariant = qVariantFromValue( myData );
          mypDetailItem->setData( myVariant, Qt::UserRole );
          modulesListModel->appendRow( mypDetailItem );
        }
        else
        {
          delete item;
        }
      }
    }
    n = n.nextSibling();
  }

}

void QgsGrassTools::removeEmptyItems( QTreeWidget *tree )
{
  // Clean tree nodes without children
  for ( int i = tree->topLevelItemCount() - 1; i >= 0; i-- )
  {
    QTreeWidgetItem *sub = tree->topLevelItem( i );
    removeEmptyItems( sub );
    if ( sub->childCount() == 0 )
    {
      tree->removeItemWidget( sub, 0 );
      tree->takeTopLevelItem( i );
      delete sub;
    }
  }
}

void QgsGrassTools::removeEmptyItems( QTreeWidgetItem *item )
{
  for ( int i = item->childCount() - 1; i >= 0; i-- )
  {

    QTreeWidgetItem *sub = item->child( i );
    QString name = sub->text( 1 ); //module name
    if ( !name.isEmpty() ) continue; // module
    removeEmptyItems( sub );
    if ( sub->childCount() == 0 )
    {
      item->removeChild( sub );
      delete sub;
    }
  }
}

void QgsGrassTools::mapsetChanged()
{
  QgsDebugMsg( "entered." );

  closeTools();
  showTabs();
}

QgsGrassTools::~QgsGrassTools()
{
  QgsDebugMsg( "entered." );
  saveWindowLocation();
}

QString QgsGrassTools::appDir( void )
{
#if defined(Q_OS_WIN)
  return QgsGrass::shortPath( QgsApplication::applicationDirPath() );
#else
  return QgsApplication::applicationDirPath();
#endif
}

void QgsGrassTools::close( void )
{
  saveWindowLocation();
  hide();
}

void QgsGrassTools::closeEvent( QCloseEvent *e )
{
  saveWindowLocation();
  e->accept();
}

void QgsGrassTools::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/GRASS/windows/tools/geometry" ).toByteArray() );
  show();
}

void QgsGrassTools::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/GRASS/windows/tools/geometry", saveGeometry() );
}

void QgsGrassTools::emitRegionChanged()
{
  QgsDebugMsg( "entered." );
  emit regionChanged();
}

void QgsGrassTools::closeTools()
{
  QgsDebugMsg( "entered." );

  for ( int i = mTabWidget->count() - 1; i > 1; i-- ) // first two are module tree and module list
  {
    delete mTabWidget->widget( i );
  }
}

//
// Helper function for Tim's experimental model list
//
void QgsGrassTools::on_mFilterInput_textChanged( QString theText )
{
  QgsDebugMsg( "GRASS modules filter changed to :" + theText );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( theText, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
}

void QgsGrassTools::on_mDirectFilterInput_textChanged( QString theText )
{
  QgsDebugMsg( "GRASS direct modules filter changed to :" + theText );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( theText, myCaseSensitivity, mySyntax );
  mDirectModelProxy->setFilterRegExp( myRegExp );
}

void QgsGrassTools::listItemClicked( const QModelIndex &theIndex )
{
  if ( theIndex.column() == 0 )
  {
    //
    // If the model has been filtered, the index row in the proxy wont match
    // the index row in the underlying model so we need to jump through this
    // little hoop to get the correct item
    //
    QStandardItem * mypItem =
      mModulesListModel->findItems( theIndex.data( Qt::DisplayRole ).toString() ).first();
    QString myModuleName = mypItem->data( Qt::UserRole + 1 ).toString();
    runModule( myModuleName, false );
  }
}

void QgsGrassTools::directListItemClicked( const QModelIndex &theIndex )
{
  if ( theIndex.column() == 0 )
  {
    //
    // If the model has been filtered, the index row in the proxy wont match
    // the index row in the underlying model so we need to jump through this
    // little hoop to get the correct item
    //
    QStandardItem * mypItem =
      mDirectModulesListModel->findItems( theIndex.data( Qt::DisplayRole ).toString() ).first();
    QString myModuleName = mypItem->data( Qt::UserRole + 1 ).toString();
    runModule( myModuleName, true );
  }
}
