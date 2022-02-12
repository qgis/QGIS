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
#include "qgsgrassregion.h"
#include "qgsgrassshell.h"
#include "qgsgrass.h"
#include "qgsconfig.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QCloseEvent>
#include <QDomDocument>
#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>


//
// For experimental model view alternative ui by Tim
//
//
#include "qgsdetaileditemdata.h"
#include "qgsdetaileditemdelegate.h"


#ifdef Q_OS_WIN
#include "qgsgrassutils.h"
#endif


QgsGrassTools::QgsGrassTools( QgisInterface *iface, QWidget *parent, const char *name, Qt::WindowFlags f )
  : QgsDockWidget( parent, f )
{
  Q_UNUSED( name )
  QgsDebugMsg( "QgsGrassTools()" );
  setupUi( this );
  connect( mFilterInput, &QLineEdit::textChanged, this, &QgsGrassTools::mFilterInput_textChanged );
  connect( mDebugButton, &QPushButton::clicked, this, &QgsGrassTools::mDebugButton_clicked );
  connect( mCloseDebugButton, &QPushButton::clicked, this, &QgsGrassTools::mCloseDebugButton_clicked );
  connect( mViewModeButton, &QToolButton::clicked, this, &QgsGrassTools::mViewModeButton_clicked );
  QPushButton *closeMapsetButton = new QPushButton( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileExit.svg" ) ), tr( "Close mapset" ), this );
  mTabWidget->setCornerWidget( closeMapsetButton );
  connect( closeMapsetButton, &QAbstractButton::clicked, this, &QgsGrassTools::closeMapset );

  qRegisterMetaType<QgsDetailedItemData>();

  mIface = iface;
  mCanvas = mIface->mapCanvas();

  //statusBar()->hide();

  // set the dialog title
  resetTitle();

  // Tree view code.
  if ( !QgsGrass::modulesDebug() )
  {
    mDebugWidget->hide();
  }

  // List view
  mTreeModel = new QStandardItemModel( 0, 1 );
  mTreeModelProxy = new QgsGrassToolsTreeFilterProxyModel( this );
  mTreeModelProxy->setSourceModel( mTreeModel );
  mTreeModelProxy->setFilterRole( Qt::UserRole + Search );

  mTreeView->setModel( mTreeModelProxy );

  connect( mTreeView, &QAbstractItemView::clicked,
           this, &QgsGrassTools::itemClicked );

  // List view with filter
  mModulesListModel = new QStandardItemModel( 0, 1 );
  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModulesListModel );
  mModelProxy->setFilterRole( Qt::UserRole + 2 );

  mListView->setModel( mModelProxy );
  connect( mListView, &QAbstractItemView::clicked,
           this, &QgsGrassTools::itemClicked );

  mListView->hide();

  connect( QgsGrass::instance(), &QgsGrass::modulesConfigChanged, this, static_cast<bool ( QgsGrassTools::* )()>( &QgsGrassTools::loadConfig ) );
  connect( QgsGrass::instance(), &QgsGrass::modulesDebugChanged, this, &QgsGrassTools::debugChanged );

  connect( mDebugReloadButton, &QAbstractButton::clicked, this, static_cast<bool ( QgsGrassTools::* )()>( &QgsGrassTools::loadConfig ) );

  // Region widget tab
  mRegion = new QgsGrassRegion( mIface, this );
  mTabWidget->addTab( mRegion, tr( "Region" ) );

  // Show before loadConfig() so that user can see loading
  show();
  showTabs();
}

void QgsGrassTools::resetTitle()
{
  QString title;
  if ( QgsGrass::activeMode() )
  {
    title = tr( "GRASS Tools: %1/%2" ).arg( QgsGrass::getDefaultLocation(), QgsGrass::getDefaultMapset() );
  }
  else
  {
    title = tr( "GRASS Tools" );
  }
  setWindowTitle( title );
}

void QgsGrassTools::showTabs()
{

  resetTitle();

  // Build modules tree if empty
  QgsDebugMsg( QString( "mTreeModel->rowCount() = %1" ).arg( mTreeModel->rowCount() ) );
  if ( mTreeModel->rowCount() == 0 )
  {
    // Load the modules lists
    QApplication::setOverrideCursor( Qt::WaitCursor );
    loadConfig();
    QApplication::restoreOverrideCursor();
    QgsDebugMsg( QString( "mTreeModel->rowCount() = %1" ).arg( mTreeModel->rowCount() ) );
  }

  // we always show tabs but disabled if not active
  if ( QgsGrass::activeMode() )
  {
    mMessageLabel->hide();
    mTabWidget->setEnabled( true );
  }
  else
  {
    mMessageLabel->show();
    mTabWidget->setEnabled( false );
  }
}

void QgsGrassTools::runModule( QString name, bool direct )
{
  if ( name.length() == 0 )
  {
    return;  // Section
  }

#ifdef HAVE_POSIX_OPENPT
  QgsGrassShell *sh = nullptr;
#endif

  QWidget *m = nullptr;
  if ( name == QLatin1String( "shell" ) )
  {
#ifdef Q_OS_WIN
    QgsGrass::putEnv( "GRASS_HTML_BROWSER", QgsGrassUtils::htmlBrowserPath() );
    QStringList env;
    QByteArray origPath = qgetenv( "PATH" );
    QByteArray origPythonPath = qgetenv( "PYTHONPATH" );
    QString path = QString( origPath ) + QgsGrass::pathSeparator() + QgsGrass::grassModulesPaths().join( QgsGrass::pathSeparator() );
    QString pythonPath = QString( origPythonPath ) + QgsGrass::pathSeparator() + QgsGrass::getPythonPath();
    QgsDebugMsg( "path = " + path );
    QgsDebugMsg( "pythonPath = " + pythonPath );
    qputenv( "PATH", path.toLocal8Bit() );
    qputenv( "PYTHONPATH", pythonPath.toLocal8Bit() );
    // QProcess does not support environment for startDetached() -> set/reset to orig
    if ( !QProcess::startDetached( getenv( "COMSPEC" ) ) )
    {
      QMessageBox::warning( 0, "Warning", tr( "Cannot start command shell (%1)" ).arg( getenv( "COMSPEC" ) ) );
    }
    qputenv( "PATH", origPath );
    qputenv( "PYTHONPATH", origPythonPath );
    return;
#else

#ifdef HAVE_POSIX_OPENPT
    sh = new QgsGrassShell( this, mTabWidget );
    m = qobject_cast<QWidget *>( sh );
#else
    QMessageBox::warning( 0, tr( "Warning" ), tr( "GRASS Shell is not compiled." ) );
#endif

#endif // Q_OS_WIN
  }
  else
  {
    // set wait cursor because starting module may be slow because of getting temporal datasets (t.list)
    QApplication::setOverrideCursor( Qt::WaitCursor );
    QgsGrassModule *gmod = new QgsGrassModule( this, name, mIface, direct, mTabWidget );
    QApplication::restoreOverrideCursor();
    if ( !gmod->errors().isEmpty() )
    {
      QgsGrass::warning( gmod->errors().join( QLatin1Char( '\n' ) ) );
    }
    m = qobject_cast<QWidget *>( gmod );
  }

  int height = mTabWidget->iconSize().height();
  QString path = QgsGrass::modulesConfigDirPath() + "/" + name;
  QPixmap pixmap = QgsGrassModule::pixmap( path, height );

  if ( !pixmap.isNull() )
  {
    // Icon size in QT does not seem to be variable
    // -> reset the width to max icon width
    if ( mTabWidget->iconSize().width() < pixmap.width() )
    {
      mTabWidget->setIconSize( QSize( pixmap.width(), mTabWidget->iconSize().height() ) );
    }


    QIcon is;
    is.addPixmap( pixmap );
    mTabWidget->addTab( m, is, QString() );
  }
  else
  {
    mTabWidget->addTab( m, name );
  }


  mTabWidget->setCurrentIndex( mTabWidget->count() - 1 );

  // We must call resize to reset COLUMNS environment variable
  // used by bash !!!
#if 0
  /* TODO: Implement something that resizes the terminal without
   *       crashes.
   */
#ifdef HAVE_POSIX_OPENPT
  if ( sh )
  {
    sh->resizeTerminal();
  }
#endif
#endif
}

bool QgsGrassTools::loadConfig()
{
  QString conf = QgsGrass::modulesConfigDirPath() + "/default.qgc";
  return loadConfig( conf, mTreeModel, mModulesListModel, false );
}

bool QgsGrassTools::loadConfig( QString filePath, QStandardItemModel *treeModel, QStandardItemModel *modulesListModel, bool direct )
{
  QgsDebugMsg( filePath );
  treeModel->clear();
  modulesListModel->clear();

  QFile file( filePath );

  if ( !file.exists() )
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), tr( "The config file (%1) not found." ).arg( filePath ) );
    return false;
  }
  if ( ! file.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), tr( "Cannot open config file (%1)." ).arg( filePath ) );
    return false;
  }

  QDomDocument doc( QStringLiteral( "qgisgrass" ) );
  QString err;
  int line, column;
  if ( !doc.setContent( &file,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read config file (%1):" ).arg( filePath )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
    QMessageBox::warning( nullptr, tr( "Warning" ), errmsg );
    file.close();
    return false;
  }

  QDomElement docElem = doc.documentElement();
  QDomNodeList modulesNodes = docElem.elementsByTagName( QStringLiteral( "modules" ) );

  if ( modulesNodes.count() == 0 )
  {
    file.close();
    return false;
  }

  QDomNode modulesNode = modulesNodes.item( 0 );
  QDomElement modulesElem = modulesNode.toElement();

  // Go through the sections and modules and add them to the list view
  addModules( nullptr, modulesElem, treeModel, modulesListModel, false );
  if ( direct )
  {
    removeEmptyItems( treeModel );
  }
  mTreeView->expandToDepth( 0 );

  file.close();
  return true;
}

void QgsGrassTools::debugChanged()
{
  if ( QgsGrass::modulesDebug() )
  {
    mDebugWidget->show();
  }
  else
  {
    mDebugWidget->hide();
  }
}

void QgsGrassTools::appendItem( QStandardItemModel *treeModel, QStandardItem *parent, QStandardItem *item )
{
  if ( parent )
  {
    parent->appendRow( item );
  }
  else if ( treeModel )
  {
    treeModel->appendRow( item );
  }
}

void QgsGrassTools::addModules( QStandardItem *parent, QDomElement &element, QStandardItemModel *treeModel, QStandardItemModel *modulesListModel, bool direct )
{
  QDomNode n = element.firstChild();
  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() )
    {
// QgsDebugMsg(QString("tag = %1").arg(e.tagName()));

      if ( e.tagName() != QLatin1String( "section" ) && e.tagName() != QLatin1String( "grass" ) )
      {
        QgsDebugMsg( QString( "Unknown tag: %1" ).arg( e.tagName() ) );
        continue;
      }

      // Check GRASS version
      QStringList errors;
      if ( !QgsGrassModuleOption::checkVersion( e.attribute( QStringLiteral( "version_min" ) ), e.attribute( QStringLiteral( "version_max" ) ), errors ) )
      {
        // TODO: show somehow errors only in debug mode, but without reloading tree
        if ( !errors.isEmpty() )
        {
          QString label = e.attribute( QStringLiteral( "label" ) ) + e.attribute( QStringLiteral( "name" ) ); // one should be non empty
          label += "\n  ERROR:\t" + errors.join( QLatin1String( "\n\t" ) );
          QStandardItem *item = new QStandardItem( label );
          item->setData( label, Qt::UserRole + Label );
          item->setData( label, Qt::UserRole + Search );
          item->setData( QgsApplication::getThemeIcon( QStringLiteral( "mIconWarning.svg" ) ), Qt::DecorationRole );
          appendItem( treeModel, parent, item );
        }
        n = n.nextSibling();
        continue;
      }

      if ( e.tagName() == QLatin1String( "section" ) )
      {
        QString label = QApplication::translate( "grasslabel", e.attribute( QStringLiteral( "label" ) ).toUtf8() );
        QgsDebugMsg( QString( "label = %1" ).arg( label ) );
        QStandardItem *item = new QStandardItem( label );
        item->setData( label, Qt::UserRole + Label ); // original label, for debug
        item->setData( label, Qt::UserRole + Search ); // for filtering later

        addModules( item, e, treeModel, modulesListModel, direct );
        appendItem( treeModel, parent, item );
      }
      else if ( e.tagName() == QLatin1String( "grass" ) )
      {
        // GRASS module
        QString name = e.attribute( QStringLiteral( "name" ) );
        QgsDebugMsgLevel( QString( "name = %1" ).arg( name ), 1 );

        //QString path = QgsApplication::pkgDataPath() + "/grass/modules/" + name;
        QString path = QgsGrass::modulesConfigDirPath() + "/" + name;
        QgsGrassModule::Description description = QgsGrassModule::description( path );

        if ( !direct || description.direct )
        {
          QString label = name + " - " + description.label;
          QPixmap pixmap = QgsGrassModule::pixmap( path, 32 );
          QStandardItem *item = new QStandardItem( name + "\n" + description.label );
          item->setData( name, Qt::UserRole + Name ); // for calling runModule later
          item->setData( label, Qt::UserRole + Label ); // original label, for debug
          item->setData( label, Qt::UserRole + Search ); // for filtering later
          item->setData( pixmap, Qt::DecorationRole );
          item->setCheckable( false );
          item->setEditable( false );
          appendItem( treeModel, parent, item );

          bool exists = false;
          for ( int i = 0; i < modulesListModel->rowCount(); i++ )
          {
            if ( modulesListModel->item( i )->data( Qt::UserRole + Name ).toString() == name )
            {
              exists = true;
              break;
            }
          }
          if ( !exists )
          {
            QStandardItem *listItem = item->clone();
            listItem->setText( name + "\n" + description.label );
            // setData in the delegate with a variantised QgsDetailedItemData
            QgsDetailedItemData myData;
            myData.setTitle( name );
            myData.setDetail( label );
            myData.setIcon( pixmap );
            myData.setCheckable( false );
            myData.setRenderAsWidget( false );
            QVariant myVariant = QVariant::fromValue( myData );
            listItem->setData( myVariant, Qt::UserRole );
            modulesListModel->appendRow( listItem );
          }
        }
      }
    }
    n = n.nextSibling();
  }

}

// used for direct
void QgsGrassTools::removeEmptyItems( QStandardItemModel *treeModel )
{
  // TODO: never tested
  if ( !treeModel )
  {
    return;
  }
  // Clean tree nodes without children
  for ( int i = treeModel->rowCount() - 1; i >= 0; i-- )
  {
    QStandardItem *item = treeModel->item( i );
    removeEmptyItems( item );
    if ( item->rowCount() == 0 )
    {
      treeModel->removeRow( i );
    }
  }
}

// used for direct
void QgsGrassTools::removeEmptyItems( QStandardItem *item )
{
  // TODO: never tested
  for ( int i = item->rowCount() - 1; i >= 0; i-- )
  {
    QStandardItem *sub = item->child( i );
    removeEmptyItems( sub );
    if ( sub->rowCount() == 0 )
    {
      item->removeRow( i );
    }
  }
}

void QgsGrassTools::closeMapset()
{
  QgsGrass::instance()->closeMapsetWarn();
  QgsGrass::saveMapset();
}

void QgsGrassTools::mapsetChanged()
{

  mTabWidget->setCurrentIndex( 0 );
  closeTools();
  mRegion->mapsetChanged();
  showTabs();
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
  hide();
}

void QgsGrassTools::closeEvent( QCloseEvent *e )
{
  e->accept();
}

void QgsGrassTools::emitRegionChanged()
{
  emit regionChanged();
}

void QgsGrassTools::closeTools()
{

  for ( int i = mTabWidget->count() - 1; i > 1; i-- ) // first is module tree, second is region
  {
    delete mTabWidget->widget( i );
  }
}

//
// Helper function for Tim's experimental model list
//
void QgsGrassTools::mFilterInput_textChanged( QString text )
{
  QgsDebugMsg( "GRASS modules filter changed to :" + text );
  mTreeModelProxy->setFilter( text );
  if ( text.isEmpty() )
  {
    mTreeView->collapseAll();
    mTreeView->expandToDepth( 0 );
  }
  else
  {
    mTreeView->expandAll();
  }

  // using simple wildcard filter which is probably what users is expecting, at least until
  // there is a filter type switch in UI
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::Wildcard );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
}

void QgsGrassTools::itemClicked( const QModelIndex &index )
{
  QgsDebugMsg( "Entered" );
  if ( index.column() == 0 )
  {
    //
    // If the model has been filtered, the index row in the proxy won't match
    // the index row in the underlying model so we need to jump through this
    // little hoop to get the correct item
    //
    const QSortFilterProxyModel *proxyModel = qobject_cast<const QSortFilterProxyModel *>( index.model() );
    if ( !proxyModel )
    {
      return;
    }
    QModelIndex modelIndex = proxyModel->mapToSource( index );

    QStandardItemModel *model = nullptr;
    if ( proxyModel == mTreeModelProxy )
    {
      model = mTreeModel;
    }
    else
    {
      model = mModulesListModel;
    }

    QStandardItem *mypItem = model->itemFromIndex( modelIndex );
    if ( mypItem )
    {
      QString myModuleName = mypItem->data( Qt::UserRole + Name ).toString();
      runModule( myModuleName, false );
    }
  }
}

void QgsGrassTools::mDebugButton_clicked()
{

  QApplication::setOverrideCursor( Qt::BusyCursor );

  int errors = 0;
  for ( int i = 0; i < mTreeModel->rowCount(); i++ )
  {
    errors += debug( mTreeModel->item( i ) );
  }
  mDebugLabel->setText( tr( "%1 errors found" ).arg( errors ) );

  QApplication::restoreOverrideCursor();
}

int QgsGrassTools::debug( QStandardItem *item )
{
  if ( !item )
  {
    return 0;
  }
  QString name = item->data( Qt::UserRole + Name ).toString();
  QString label = item->data( Qt::UserRole + Label ).toString();
  if ( name.isEmpty() ) // section
  {
    int errors = 0;

    for ( int i = 0; i < item->rowCount(); i++ )
    {
      QStandardItem *sub = item->child( i );
      errors += debug( sub );
    }
    if ( errors > 0 )
    {
      label += " ( " + tr( "%n error(s)", nullptr, errors ) + " )";
      item->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconWarning.svg" ) ) );
    }
    else
    {
      item->setIcon( QIcon() );
    }
    item->setText( label );
    return errors;
  }
  else // module
  {
    if ( name == QLatin1String( "shell" ) )
    {
      return 0;
    }
    QgsGrassModule *module = new QgsGrassModule( this, name, mIface, false );
    QgsDebugMsg( QString( "module: %1 errors: %2" ).arg( name ).arg( module->errors().size() ) );
    for ( QString error : module->errors() )
    {
      // each error may have multiple rows and may be html formatted (<br>)
      label += "\n  ERROR:\t" + error.replace( QLatin1String( "<br>" ), QLatin1String( "\n" ) ).replace( QLatin1String( "\n" ), QLatin1String( "\n\t" ) );
    }
    item->setText( label );
    int nErrors = module->errors().size();
    delete module;
    return nErrors;
  }
}

void QgsGrassTools::mCloseDebugButton_clicked()
{
  QgsGrass::instance()->setModulesDebug( false );
}


void QgsGrassTools::mViewModeButton_clicked()
{
  if ( mTreeView->isHidden() )
  {
    mListView->hide();
    mTreeView->show();
    mViewModeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconListView.svg" ) ) );
  }
  else
  {
    mTreeView->hide();
    mListView->show();
    mViewModeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconTreeView.svg" ) ) );
  }
}

QgsGrassToolsTreeFilterProxyModel::QgsGrassToolsTreeFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  mRegExp.setPatternSyntax( QRegExp::Wildcard );
  mRegExp.setCaseSensitivity( Qt::CaseInsensitive );
}

void QgsGrassToolsTreeFilterProxyModel::setSourceModel( QAbstractItemModel *sourceModel )
{
  mModel = sourceModel;
  QSortFilterProxyModel::setSourceModel( sourceModel );
}

void QgsGrassToolsTreeFilterProxyModel::setFilter( const QString &filter )
{
  QgsDebugMsg( QString( "filter = %1" ).arg( filter ) );
  if ( mFilter == filter )
  {
    return;
  }
  mFilter = filter;
  mRegExp.setPattern( mFilter );

  invalidateFilter();
}

bool QgsGrassToolsTreeFilterProxyModel::filterAcceptsString( const QString &value ) const
{
  return value.contains( mRegExp );
}

bool QgsGrassToolsTreeFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( mFilter.isEmpty() || !mModel )
  {
    return true;
  }

  QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  return filterAcceptsItem( sourceIndex ) || filterAcceptsAncestor( sourceIndex ) || filterAcceptsDescendant( sourceIndex );
}

bool QgsGrassToolsTreeFilterProxyModel::filterAcceptsAncestor( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
  {
    return true;
  }

  QModelIndex sourceParentIndex = mModel->parent( sourceIndex );
  if ( !sourceParentIndex.isValid() )
    return false;
  if ( filterAcceptsItem( sourceParentIndex ) )
    return true;

  return filterAcceptsAncestor( sourceParentIndex );
}

bool QgsGrassToolsTreeFilterProxyModel::filterAcceptsDescendant( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
  {
    return true;
  }

  for ( int i = 0; i < mModel->rowCount( sourceIndex ); i++ )
  {
    QModelIndex sourceChildIndex = mModel->index( i, 0, sourceIndex );
    if ( filterAcceptsItem( sourceChildIndex ) )
      return true;
    if ( filterAcceptsDescendant( sourceChildIndex ) )
      return true;
  }
  return false;
}

bool QgsGrassToolsTreeFilterProxyModel::filterAcceptsItem( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
  {
    return true;
  }
  return filterAcceptsString( mModel->data( sourceIndex, filterRole() ).toString() );
}
