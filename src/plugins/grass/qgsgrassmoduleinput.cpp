/***************************************************************************
                          qgsgrassmoduleinput.cpp
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCompleter>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLatin1String>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>

#include "qgis.h"
#include "qgslogger.h"

#include "qgsgrass.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleparam.h"
#include "qgsgrassplugin.h"
#include "qgsgrassvector.h"

extern "C"
{
#include <grass/vector.h>
}

#include "qgsgrassmoduleinput.h"
#include "moc_qgsgrassmoduleinput.cpp"

/**************************** QgsGrassModuleInputModel ****************************/
QgsGrassModuleInputModel::QgsGrassModuleInputModel( QObject *parent )
  : QStandardItemModel( parent )
{
  setColumnCount( 1 );

  mWatcher = new QFileSystemWatcher( this );
  connect( mWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsGrassModuleInputModel::onDirectoryChanged );
  connect( mWatcher, &QFileSystemWatcher::fileChanged, this, &QgsGrassModuleInputModel::onFileChanged );

  connect( QgsGrass::instance(), &QgsGrass::mapsetChanged, this, &QgsGrassModuleInputModel::onMapsetChanged );

  connect( QgsGrass::instance(), &QgsGrass::mapsetSearchPathChanged, this, &QgsGrassModuleInputModel::onMapsetSearchPathChanged );

  reload();
}

void QgsGrassModuleInputModel::onDirectoryChanged( const QString &path )
{
  QgsDebugMsgLevel( "path = " + path, 2 );

  QString locationPath = QgsGrass::getDefaultLocationPath();
  QDir parentDir( path );
  parentDir.cdUp();
  QString mapset;
  QList<QgsGrassObject::Type> types;
  if ( path == locationPath )
  {
    QgsDebugMsgLevel( "location = " + path, 2 );
    QStringList dirNames = locationDirNames();
    //QStringList mapsets = QgsGrass::mapsets( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );

    for ( int i = rowCount() - 1; i >= 0; i-- )
    {
      QString mapset = item( i )->text();
      if ( !QgsGrass::isMapset( locationPath + "/" + mapset ) )
      {
        QgsDebugMsgLevel( "removed mapset " + mapset, 2 );
        removeRows( i, 1 );
      }
    }

    for ( const QString &dirName : dirNames )
    {
      // Add to watcher in any case, either for WIND, cellhd or vector
      QString dirPath = locationPath + "/" + dirName;
      watch( dirPath );
      if ( QgsGrass::isMapset( dirPath ) && findItems( dirName ).isEmpty() )
      {
        addMapset( dirName );
      }
    }
  }
  else if ( parentDir.canonicalPath() == QDir( locationPath ).canonicalPath() ) // mapset
  {
    QgsDebugMsgLevel( "mapset = " + path, 2 );
    QDir dir( path );
    mapset = dir.dirName();
    for ( const QString &watchedDir : watchedDirs() )
    {
      watch( path + "/" + watchedDir );
    }
    // TODO: use db path defined in mapset VAR
    watch( path + "/tgis/sqlite.db" );
  }
  else // cellhd or vector dir
  {
    QgsDebugMsgLevel( "cellhd/vector = " + path, 2 );
    mapset = parentDir.dirName();
    if ( path.endsWith( QLatin1String( "cellhd" ) ) )
    {
      types << QgsGrassObject::Raster;
    }
    else if ( path.endsWith( QLatin1String( "vector" ) ) )
    {
      types << QgsGrassObject::Vector;
    }
  }
  if ( !mapset.isEmpty() )
  {
    QList<QStandardItem *> items = findItems( mapset );
    if ( items.size() == 1 )
    {
      refreshMapset( items[0], mapset, types );
    }
  }
}

void QgsGrassModuleInputModel::onFileChanged( const QString &path )
{
  QgsDebugMsgLevel( "path = " + path, 2 );
  // when tgis/sqlite.db is changed, this gets called twice, probably the file changes more times when it is modified
  if ( path.endsWith( QLatin1String( "/tgis/sqlite.db" ) ) )
  {
    QDir dir = QFileInfo( path ).dir();
    dir.cdUp();
    QString mapset = dir.dirName();
    QList<QStandardItem *> items = findItems( mapset );
    if ( items.size() == 1 )
    {
      QList<QgsGrassObject::Type> types;
      types << QgsGrassObject::Strds << QgsGrassObject::Stvds << QgsGrassObject::Str3ds;
      refreshMapset( items[0], mapset, types );
    }
  }
}

void QgsGrassModuleInputModel::watch( const QString &path )
{
  if ( QFileInfo( path ).isDir() && !mWatcher->directories().contains( path ) )
  {
    mWatcher->addPath( path );
  }
  else if ( QFileInfo( path ).isFile() && !mWatcher->files().contains( path ) )
  {
    mWatcher->addPath( path );
  }
}

QStringList QgsGrassModuleInputModel::locationDirNames()
{
  QString locationPath = QgsGrass::getDefaultLocationPath();
  QDir locationDir( locationPath );
  return locationDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
}

void QgsGrassModuleInputModel::addMapset( const QString &mapset )
{
  QgsDebugMsgLevel( "mapset = " + mapset, 2 );

  QStandardItem *mapsetItem = new QStandardItem( mapset );
  mapsetItem->setData( mapset, MapsetRole );
  mapsetItem->setData( mapset, Qt::EditRole );
  mapsetItem->setData( QgsGrassObject::Mapset, TypeRole );
  mapsetItem->setSelectable( false );

  refreshMapset( mapsetItem, mapset );

  appendRow( mapsetItem );
}

void QgsGrassModuleInputModel::refreshMapset( QStandardItem *mapsetItem, const QString &mapset, const QList<QgsGrassObject::Type> &types )
{
  QgsDebugMsgLevel( "mapset = " + mapset, 2 );
  if ( !mapsetItem )
  {
    return;
  }

  QList<QgsGrassObject::Type> typesCopy = types;
  if ( typesCopy.isEmpty() )
  {
    typesCopy << QgsGrassObject::Raster << QgsGrassObject::Vector;
    typesCopy << QgsGrassObject::Strds << QgsGrassObject::Stvds << QgsGrassObject::Str3ds;
  }
  for ( QgsGrassObject::Type type : typesCopy )
  {
    QgsGrassObject mapsetObject( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation(), mapset, QString(), QgsGrassObject::Mapset );
    QStringList maps = QgsGrass::grassObjects( mapsetObject, type );
    QStringList mapNames;
    for ( const QString &map : maps )
    {
      if ( map.startsWith( QLatin1String( "qgis_import_tmp_" ) ) )
      {
        continue;
      }
      QString mapName = map;

      bool found = false;
      for ( int i = 0; i < mapsetItem->rowCount(); i++ )
      {
        QStandardItem *item = mapsetItem->child( i );
        if ( item->text() == mapName && item->data( TypeRole ).toInt() == type )
        {
          found = true;
          break;
        }
      }
      if ( !found )
      {
        QgsDebugMsgLevel( "add map : " + mapName, 2 );
        QStandardItem *mapItem = new QStandardItem( mapName );
        mapItem->setData( mapName, Qt::EditRole );
        mapItem->setData( map, MapRole );
        mapItem->setData( mapset, MapsetRole );
        mapItem->setData( type, TypeRole );
        mapsetItem->appendRow( mapItem );
      }
      else
      {
        QgsDebugMsgLevel( "map exists : " + mapName, 2 );
      }
      mapNames << mapName;
    }

    for ( int i = mapsetItem->rowCount() - 1; i >= 0; i-- )
    {
      if ( mapsetItem->child( i )->data( TypeRole ).toInt() != type )
      {
        continue;
      }
      QString mapName = mapsetItem->child( i )->text();
      if ( !mapNames.contains( mapName ) )
      {
        QgsDebugMsgLevel( "remove map : " + mapName, 2 );
        mapsetItem->removeRows( i, 1 );
      }
    }
  }
}

void QgsGrassModuleInputModel::reload()
{
  if ( !mWatcher->files().isEmpty() )
  {
    mWatcher->removePaths( mWatcher->files() );
  }
  if ( !mWatcher->directories().isEmpty() )
  {
    mWatcher->removePaths( mWatcher->directories() );
  }

  clear();

  mLocationPath = QgsGrass::getDefaultLocationPath();

  QStringList mapsets = QgsGrass::mapsets( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
  for ( const QString &mapset : mapsets )
  {
    addMapset( mapset );
  }

  mWatcher->addPath( mLocationPath );

  // Watching all dirs in location because a dir may become a mapset later, when WIND is created
  QStringList dirNames = locationDirNames();
  for ( const QString &dirName : dirNames )
  {
    QString dirPath = mLocationPath + "/" + dirName;
    // Watch the dir in any case, WIND maybe created later
    mWatcher->addPath( dirPath );

    for ( const QString &watchedDir : watchedDirs() )
    {
      watch( dirPath + "/" + watchedDir );
    }
    watch( dirPath + "/tgis/sqlite.db" );
  }
}

void QgsGrassModuleInputModel::onMapsetChanged()
{
  if ( mLocationPath != QgsGrass::getDefaultLocationPath() )
  {
    reload();
  }
}

void QgsGrassModuleInputModel::onMapsetSearchPathChanged()
{
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}


QgsGrassModuleInputModel *QgsGrassModuleInputModel::instance()
{
  static QgsGrassModuleInputModel sInstance;
  return &sInstance;
}

QVariant QgsGrassModuleInputModel::data( const QModelIndex &index, int role ) const
{
  QVariant data = QStandardItemModel::data( index, role );
  if ( role == Qt::DisplayRole || role == Qt::EditRole ) // EditRole for combo
  {
    int type = QStandardItemModel::data( index, QgsGrassModuleInputModel::TypeRole ).toInt();
    if ( type == QgsGrassObject::Raster || type == QgsGrassObject::Vector )
    {
      QString mapset = QStandardItemModel::data( index, QgsGrassModuleInputModel::MapsetRole ).toString();
      if ( mapset != QgsGrass::getDefaultMapset() )
      {
        data = QString( data.toString() + "@" + mapset );
      }
    }
  }
  return data;
}

/**************************** QgsGrassModuleInputProxy ****************************/
QgsGrassModuleInputProxy::QgsGrassModuleInputProxy( QgsGrassModuleInputModel *sourceModel, QgsGrassObject::Type type, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mSourceModel( sourceModel )
  , mType( type )
{
  setSourceModel( mSourceModel );
  setDynamicSortFilter( true );
}

bool QgsGrassModuleInputProxy::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( !sourceModel() )
  {
    return false;
  }
  QModelIndex sourceIndex = sourceModel()->index( sourceRow, 0, sourceParent );

  QgsDebugMsgLevel( QString( "mType = %1 item type = %2" ).arg( mType ).arg( sourceModel()->data( sourceIndex, QgsGrassModuleInputModel::TypeRole ).toInt() ), 2 );
  QgsGrassObject::Type itemType = ( QgsGrassObject::Type )( sourceModel()->data( sourceIndex, QgsGrassModuleInputModel::TypeRole ).toInt() );

  if ( itemType == QgsGrassObject::Mapset )
  {
    // TODO: filter out mapsets which have no map of given type? May be confusin if user does not see existing mapset in the tree.
    QString mapset = sourceModel()->data( sourceIndex, QgsGrassModuleInputModel::MapsetRole ).toString();
    if ( QgsGrass::instance()->isMapsetInSearchPath( mapset ) )
    {
      return true;
    }
    else
    {
      QgsDebugError( "mapset " + mapset + " is not in search path" );
      return false;
    }
  }

  return mType == itemType || ( mType == QgsGrassObject::Stds && ( itemType == QgsGrassObject::Strds || itemType == QgsGrassObject::Stvds || itemType == QgsGrassObject::Str3ds ) );
}

bool QgsGrassModuleInputProxy::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  if ( mSourceModel )
  {
    // keep current mapset on top
    if ( mSourceModel->data( left, QgsGrassModuleInputModel::TypeRole ).toInt() == QgsGrassObject::Mapset )
    {
      if ( mSourceModel->data( left ).toString() == QgsGrass::getDefaultMapset() )
      {
        return true;
      }
      else if ( mSourceModel->data( right ).toString() == QgsGrass::getDefaultMapset() )
      {
        return false;
      }
    }
  }
  return QSortFilterProxyModel::lessThan( left, right );
}

/**************************** QgsGrassModuleInputTreeView ****************************/
QgsGrassModuleInputTreeView::QgsGrassModuleInputTreeView( QWidget *parent )
  : QTreeView( parent )
{
  setHeaderHidden( true );
}

void QgsGrassModuleInputTreeView::resetState()
{
  QAbstractItemView::setState( QAbstractItemView::NoState );
}

/**************************** QgsGrassModuleInputPopup ****************************/
QgsGrassModuleInputPopup::QgsGrassModuleInputPopup( QWidget *parent )
  : QTreeView( parent )
{
  //setMinimumHeight(200);
}

void QgsGrassModuleInputPopup::setModel( QAbstractItemModel *model )
{
  QTreeView::setModel( model );
}

/**************************** QgsGrassModuleInputCompleterProxy ****************************/
// TODO refresh data on sourceModel data change
QgsGrassModuleInputCompleterProxy::QgsGrassModuleInputCompleterProxy( QObject *parent )
  : QAbstractProxyModel( parent )
{
}

int QgsGrassModuleInputCompleterProxy::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mRows.size();
}

QModelIndex QgsGrassModuleInputCompleterProxy::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column );
}

QModelIndex QgsGrassModuleInputCompleterProxy::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )
  return QModelIndex();
}

void QgsGrassModuleInputCompleterProxy::setSourceModel( QAbstractItemModel *sourceModel )
{
  QAbstractProxyModel::setSourceModel( sourceModel );
  refreshMapping();
}

QModelIndex QgsGrassModuleInputCompleterProxy::mapFromSource( const QModelIndex &sourceIndex ) const
{
  if ( !mRows.contains( sourceIndex ) )
  {
    return QModelIndex();
  }
  return createIndex( mRows.value( sourceIndex ), 0 );
}

QModelIndex QgsGrassModuleInputCompleterProxy::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !mIndexes.contains( proxyIndex.row() ) )
  {
    return QModelIndex();
  }
  return mIndexes.value( proxyIndex.row() );
}

void QgsGrassModuleInputCompleterProxy::refreshMapping()
{
  // TODO: emit data changed
  mIndexes.clear();
  mRows.clear();
  map( QModelIndex() );
  QgsDebugMsgLevel( QString( "mRows.size() = %1" ).arg( mRows.size() ), 2 );
}

void QgsGrassModuleInputCompleterProxy::map( const QModelIndex &parent, int level )
{
  if ( !sourceModel() )
  {
    return;
  }
  //QgsDebugMsgLevel( "parent = " + sourceModel()->data(parent).toString(), 2 );
  for ( int i = 0; i < sourceModel()->rowCount( parent ); i++ )
  {
    QModelIndex index = sourceModel()->index( i, 0, parent );
    if ( level == 0 ) // mapset
    {
      map( index, level + 1 );
    }
    else if ( level == 1 ) // map
    {
      int row = mRows.size();
      mIndexes.insert( row, index );
      mRows.insert( index, row );
    }
  }
}

/**************************** QgsGrassModuleInputCompleter ****************************/
// TODO: implement tree view in popup

QgsGrassModuleInputCompleter::QgsGrassModuleInputCompleter( QAbstractItemModel *model, QWidget *parent )
  : QCompleter( model, parent )
{
}

bool QgsGrassModuleInputCompleter::eventFilter( QObject *watched, QEvent *event )
{
  if ( event->type() == QEvent::KeyPress && watched == widget() )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    // Disable Up/Down in edit line (causing selection of previous/next item + activated() signal)
    if ( keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down )
    {
      QgsDebugMsgLevel( "Up/Down", 2 );
      return true;
    }
  }
  return QCompleter::eventFilter( watched, event );
}

/**************************** QgsGrassModuleInputComboBox ****************************/
// Ideas from http://qt.shoutwiki.com/wiki/Implementing_QTreeView_in_QComboBox_using_Qt-_Part_2
// and bug work around https://bugreports.qt.io/browse/QTBUG-11913
QgsGrassModuleInputComboBox::QgsGrassModuleInputComboBox( QgsGrassObject::Type type, QWidget *parent )
  : QComboBox( parent )
  , mType( type )
  , mSkipHide( false )
{
  setEditable( true );
  setInsertPolicy( QComboBox::NoInsert );

  mModel = QgsGrassModuleInputModel::instance();
  mProxy = new QgsGrassModuleInputProxy( mModel, mType, this );
  setModel( mProxy );

  mTreeView = new QgsGrassModuleInputTreeView( this );
  mTreeView->setSortingEnabled( true );
  mTreeView->sortByColumn( 0, Qt::AscendingOrder );
  mTreeView->setSelectionMode( QAbstractItemView::SingleSelection );
  //mTreeView->setSelectionMode(QAbstractItemView::MultiSelection); // does not work
  mTreeView->viewport()->installEventFilter( this );
  setView( mTreeView ); // takes ownership
  mTreeView->expandAll();

  QgsGrassModuleInputCompleterProxy *completerProxy = new QgsGrassModuleInputCompleterProxy( this );
  completerProxy->setSourceModel( mProxy );

  QCompleter *completer = new QgsGrassModuleInputCompleter( completerProxy, this );
  completer->setCompletionRole( Qt::DisplayRole );
  completer->setCaseSensitivity( Qt::CaseInsensitive );
  // TODO: enable when Qt version requirement gets over 5.2
  //setFilterMode( Qt::MatchWildcard );
  completer->setCompletionMode( QCompleter::PopupCompletion );
  completer->setMaxVisibleItems( 20 );
  // TODO: set custom treeview for completer popup to show items in tree structure, if feasible
  //QgsGrassModuleInputPopup *popupView = new QgsGrassModuleInputPopup();
  //completer->setPopup( popupView );
  //popupView->setModel( mModel );
  setCompleter( completer );

  setCurrentIndex( -1 );
}

bool QgsGrassModuleInputComboBox::eventFilter( QObject *watched, QEvent *event )
{
  // mSkipHide does not seem to be necessary anymore, not sure why
  if ( event->type() == QEvent::MouseButtonPress && watched == view()->viewport() )
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
    QModelIndex index = view()->indexAt( mouseEvent->pos() );
    if ( !view()->visualRect( index ).contains( mouseEvent->pos() ) )
    {
      mSkipHide = true;
    }
  }
  return false;
}

void QgsGrassModuleInputComboBox::showPopup()
{
  setRootModelIndex( QModelIndex() );
  QComboBox::showPopup();
}

void QgsGrassModuleInputComboBox::hidePopup()
{
  if ( view()->currentIndex().isValid() )
  {
    QModelIndex sourceIndex = mProxy->mapToSource( view()->currentIndex() );
    QStandardItem *item = mModel->itemFromIndex( sourceIndex );
    if ( item && item->isSelectable() )
    {
      setRootModelIndex( view()->currentIndex().parent() );
      setCurrentIndex( view()->currentIndex().row() );
    }
  }

  if ( mSkipHide )
  {
    mSkipHide = false;
  }
  else
  {
    QComboBox::hidePopup();
  }

  //QComboBox::hidePopup();
  // reset state to fix the bug after drag
  mTreeView->resetState();
}

void QgsGrassModuleInputComboBox::setCurrent( const QModelIndex &proxyIndex )
{
  setRootModelIndex( proxyIndex.parent() );
  setModelColumn( proxyIndex.column() );
  setCurrentIndex( proxyIndex.row() );
  setRootModelIndex( QModelIndex() );
  view()->setCurrentIndex( proxyIndex );
}

bool QgsGrassModuleInputComboBox::setCurrent( const QString &map, const QString &mapset )
{
  QString ms = mapset.isEmpty() ? QgsGrass::getDefaultMapset() : mapset;
  QgsDebugMsgLevel( " map = " + map + " mapset = " + mapset + " ms = " + ms, 2 );
  mTreeView->selectionModel()->clear();
  for ( int i = 0; i < mProxy->rowCount(); i++ )
  {
    QModelIndex mapsetIndex = mProxy->index( i, 0 );
    if ( mProxy->data( mapsetIndex, QgsGrassModuleInputModel::MapsetRole ).toString() == ms )
    {
      for ( int j = 0; j < mProxy->rowCount( mapsetIndex ); j++ )
      {
        QModelIndex mapIndex = mProxy->index( j, 0, mapsetIndex );
        if ( mProxy->data( mapIndex, QgsGrassModuleInputModel::MapRole ).toString() == map )
        {
          mTreeView->scrollTo( mapIndex ); // expand
          setCurrent( mapIndex );
          return true;
        }
      }
      break;
    }
  }
  return false;
}

bool QgsGrassModuleInputComboBox::setFirst()
{
  for ( int i = 0; i < mProxy->rowCount(); i++ )
  {
    QModelIndex mapsetIndex = mProxy->index( i, 0 );
    if ( mProxy->rowCount( mapsetIndex ) > 0 )
    {
      QModelIndex mapIndex = mProxy->index( 0, 0, mapsetIndex );
      mTreeView->scrollTo( mapIndex ); // expand
      setCurrent( mapIndex );
      return true;
    }
  }
  return false;
}

/******************** QgsGrassModuleInputSelectedDelegate *********************/
// Taken from Qt Creator
QgsGrassModuleInputSelectedDelegate::QgsGrassModuleInputSelectedDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

void QgsGrassModuleInputSelectedDelegate::handlePressed( const QModelIndex &index )
{
  if ( index.column() == 1 )
  {
    mPressedIndex = index;
  }
}

void QgsGrassModuleInputSelectedDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( option.state & QStyle::State_MouseOver )
  {
    if ( ( QApplication::mouseButtons() & Qt::LeftButton ) == 0 )
      mPressedIndex = QModelIndex();
    QBrush brush = option.palette.alternateBase();
    if ( index == mPressedIndex )
      brush = option.palette.dark();
    painter->fillRect( option.rect, brush );
  }


  QStyledItemDelegate::paint( painter, option, index );

  if ( index.column() == 1 && option.state & QStyle::State_MouseOver )
  {
    const QIcon icon = ( option.state & QStyle::State_Selected ) ? QgsGrassPlugin::getThemeIcon( "closebutton.png" ) : QgsGrassPlugin::getThemeIcon( "darkclosebutton.png" );


    QRect iconRect( option.rect.right() - option.rect.height(), option.rect.top(), option.rect.height(), option.rect.height() );

    icon.paint( painter, iconRect, Qt::AlignRight | Qt::AlignVCenter );
  }
}

/******************** QgsGrassModuleInputSelectedView *********************/
QgsGrassModuleInputSelectedView::QgsGrassModuleInputSelectedView( QWidget *parent )
  : QTreeView( parent )
{
  mDelegate = new QgsGrassModuleInputSelectedDelegate( this );
  setItemDelegate( mDelegate );
  setIndentation( 0 );
  setUniformRowHeights( true );
  setTextElideMode( Qt::ElideMiddle );
  setFrameStyle( QFrame::NoFrame );
  setAttribute( Qt::WA_MacShowFocusRect, false );
  viewport()->setAttribute( Qt::WA_Hover );

  setSelectionMode( QAbstractItemView::SingleSelection );
  setSelectionBehavior( QAbstractItemView::SelectRows );

  installEventFilter( this );
  viewport()->installEventFilter( this );

  connect( this, &QAbstractItemView::pressed, mDelegate, &QgsGrassModuleInputSelectedDelegate::handlePressed );
}

void QgsGrassModuleInputSelectedView::setModel( QAbstractItemModel *model )
{
  QTreeView::setModel( model );
  header()->hide();
  header()->setStretchLastSection( false );
  header()->setSectionResizeMode( 0, QHeaderView::Stretch );
  header()->setSectionResizeMode( 1, QHeaderView::Fixed );
  header()->resizeSection( 1, 16 );
}

bool QgsGrassModuleInputSelectedView::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == this && event->type() == QEvent::KeyPress && currentIndex().isValid() )
  {
    QgsDebugMsgLevel( "KeyPress", 4 );
    QKeyEvent *ke = static_cast<QKeyEvent *>( event );
    if ( ( ke->key() == Qt::Key_Delete || ke->key() == Qt::Key_Backspace ) && ke->modifiers() == 0 )
    {
      emit deleteItem( currentIndex() );
    }
  }
  else if ( obj == viewport() && event->type() == QEvent::MouseButtonRelease )
  {
    QgsDebugMsgLevel( "MouseButtonRelease", 4 );
    QMouseEvent *me = static_cast<QMouseEvent *>( event );
    if ( me->button() == Qt::LeftButton && me->modifiers() == Qt::NoModifier )
    {
      QModelIndex index = indexAt( me->pos() );
      if ( index.isValid() && index.column() == 1 )
      {
        emit deleteItem( index );
        return true;
      }
    }
  }
  return false;
}

/**************************** QgsGrassModuleInput ****************************/
QgsGrassModuleInput::QgsGrassModuleInput( QgsGrassModule *module, QgsGrassModuleStandardOptions *options, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent )
  : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
  , mType( QgsGrassObject::Vector )
  , mModuleStandardOptions( options )
  , mUpdate( false )
  , mUsesRegion( false )
{
  mGeometryTypeMask = GV_POINT | GV_LINE | GV_AREA;

  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Input" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( QStringLiteral( "gisprompt" ) );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( QStringLiteral( "element" ) );

  QDomNode typeNode;
  if ( element == QLatin1String( "vector" ) )
  {
    mType = QgsGrassObject::Vector;

    // Read type mask if "typeoption" is defined
    QString opt = qdesc.attribute( QStringLiteral( "typeoption" ) );
    if ( !opt.isNull() )
    {
      typeNode = nodeByKey( gdesc, opt );

      if ( typeNode.isNull() )
      {
        mErrors << tr( "Cannot find typeoption %1" ).arg( opt );
      }
      else
      {
        mGeometryTypeOption = opt;

        QDomNode valuesNode = typeNode.namedItem( QStringLiteral( "values" ) );
        if ( valuesNode.isNull() )
        {
          mErrors << tr( "Cannot find values for typeoption %1" ).arg( opt );
        }
        else
        {
          mGeometryTypeMask = 0; //GV_POINT | GV_LINE | GV_AREA;
          QDomElement valuesElem = valuesNode.toElement();
          QDomNode valueNode = valuesElem.firstChild();

          while ( !valueNode.isNull() )
          {
            QDomElement valueElem = valueNode.toElement();
            if ( !valueElem.isNull() && valueElem.tagName() == QLatin1String( "value" ) )
            {
              QDomNode n = valueNode.namedItem( QStringLiteral( "name" ) );
              if ( !n.isNull() )
              {
                QDomElement e = n.toElement();
                QString val = e.text().trimmed();
                mGeometryTypeMask |= QgsGrass::vectorType( val );
              }
            }
            valueNode = valueNode.nextSibling();
          }
          QgsDebugMsgLevel( QString( "mGeometryTypeMask = %1" ).arg( mGeometryTypeMask ), 2 );
        }
      }
    }

    // Read type mask defined in configuration
    opt = qdesc.attribute( QStringLiteral( "typemask" ) );
    if ( !opt.isNull() )
    {
      int mask = 0;

      for ( const QString &typeName : opt.split( ',' ) )
      {
        mask |= QgsGrass::vectorType( typeName );
      }

      mGeometryTypeMask &= mask;
      QgsDebugMsgLevel( QString( "mask = %1 -> mGeometryTypeMask = %2" ).arg( mask ).arg( mGeometryTypeMask ), 2 );
    }

    // Read "layeroption" if defined
    opt = qdesc.attribute( QStringLiteral( "layeroption" ) );
    if ( !opt.isNull() )
    {
      QDomNode optNode = nodeByKey( gdesc, opt );

      if ( optNode.isNull() )
      {
        mErrors << tr( "Cannot find layeroption %1" ).arg( opt );
      }
      else
      {
        mVectorLayerOption = opt;
      }
    }

    // Read "mapid"
    mMapId = qdesc.attribute( QStringLiteral( "mapid" ) );
  }
  else if ( element == QLatin1String( "cell" ) )
  {
    mType = QgsGrassObject::Raster;
  }
  else if ( element == QLatin1String( "strds" ) )
  {
    mType = QgsGrassObject::Strds;
  }
  else if ( element == QLatin1String( "stvds" ) )
  {
    mType = QgsGrassObject::Stvds;
  }
  else if ( element == QLatin1String( "str3ds" ) )
  {
    mType = QgsGrassObject::Str3ds;
  }
  else if ( element == QLatin1String( "stds" ) )
  {
    mType = QgsGrassObject::Stds;
  }
  else
  {
    mErrors << tr( "GRASS element %1 not supported" ).arg( element );
  }

  if ( qdesc.attribute( QStringLiteral( "update" ) ) == QLatin1String( "yes" ) )
  {
    mUpdate = true;
  }

  QVBoxLayout *layout = new QVBoxLayout( this );
  // Map + region
  QHBoxLayout *mapLayout = new QHBoxLayout();
  layout->addLayout( mapLayout );

  // Map input
  mComboBox = new QgsGrassModuleInputComboBox( mType, this );
  mComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  // QComboBox does not emit activated() when item is selected in completer popup
  connect( mComboBox, qOverload<int>( &QComboBox::activated ), this, [=]( int index ) { onActivated( mComboBox->itemText( index ) ); } );
  connect( mComboBox->completer(), static_cast<void ( QCompleter::* )( const QString & )>( &QCompleter::activated ), this, &QgsGrassModuleInput::onActivated );
  connect( mComboBox, &QComboBox::editTextChanged, this, &QgsGrassModuleInput::onChanged );
  mapLayout->addWidget( mComboBox );

  // Region button
  QString region = qdesc.attribute( QStringLiteral( "region" ) );
  // TODO: implement region for multiple
  if ( mType == QgsGrassObject::Raster && region != QLatin1String( "no" ) && !mDirect && !multiple() )
  {
    mRegionButton = new QPushButton( QgsGrassPlugin::getThemeIcon( QStringLiteral( "grass_set_region.png" ) ), QString() );

    mRegionButton->setToolTip( tr( "Use region of this map" ) );
    mRegionButton->setCheckable( true );
    mRegionButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    mapLayout->addWidget( mRegionButton );
  }

  if ( multiple() )
  {
    mSelectedModel = new QStandardItemModel( 0, 2 );
    mSelectedTreeView = new QgsGrassModuleInputSelectedView( this );
    mSelectedTreeView->setModel( mSelectedModel );
    connect( mSelectedTreeView, &QgsGrassModuleInputSelectedView::deleteItem, this, &QgsGrassModuleInput::deleteSelectedItem );
    layout->addWidget( mSelectedTreeView );
  }

  // Vector layer + type
  if ( mType == QgsGrassObject::Vector && !multiple() )
  {
    QHBoxLayout *layerLayout = new QHBoxLayout();
    layout->addLayout( layerLayout );

    mLayerLabel = new QLabel( tr( "Sublayer" ), this );
    layerLayout->addWidget( mLayerLabel );

    mLayerComboBox = new QComboBox();
    connect( mLayerComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGrassModuleInput::onLayerChanged );
    layerLayout->addWidget( mLayerComboBox );

    QHBoxLayout *typeLayout = new QHBoxLayout();
    layerLayout->addLayout( typeLayout );

    // Vector types
    if ( !typeNode.isNull() )
    {
      QList<int> types;
      types << GV_POINT << GV_LINE << GV_BOUNDARY << GV_CENTROID << GV_AREA;
      for ( int type : types )
      {
        if ( !( type & mGeometryTypeMask ) )
        {
          continue;
        }
        QCheckBox *typeCheckBox = new QCheckBox( QgsGrass::vectorTypeName( type ), this );
        typeCheckBox->setChecked( true );
        mTypeCheckBoxes.insert( type, typeCheckBox );
        typeLayout->addWidget( typeCheckBox );
      }
    }

    layerLayout->addItem( new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
  }

  if ( !mMapId.isEmpty() )
  {
    QgsGrassModuleParam *item = mModuleStandardOptions->item( mMapId );
    if ( item )
    {
      QgsGrassModuleInput *mapInput = dynamic_cast<QgsGrassModuleInput *>( item );

      connect( mapInput, SIGNAL( valueChanged() ), this, SLOT( updateQgisLayers() ) );
    }
  }

  mUsesRegion = false;
  if ( region.length() > 0 )
  {
    if ( region == QLatin1String( "yes" ) )
      mUsesRegion = true;
  }
  else
  {
    if ( type() == QgsGrassObject::Raster )
      mUsesRegion = true;
  }
  QgsDebugMsgLevel( QString( "mUsesRegion = %1" ).arg( mUsesRegion ), 2 );
  onChanged( QString() );
}

bool QgsGrassModuleInput::useRegion()
{
  return mUsesRegion && mType == QgsGrassObject::Raster && mRegionButton && mRegionButton->isChecked();
}

QStringList QgsGrassModuleInput::options()
{
  QStringList list;

  if ( multiple() )
  {
    QStringList maps;
    for ( int i = 0; i < mSelectedModel->rowCount(); i++ )
    {
      maps << mSelectedModel->item( i )->text();
    }
    list << mKey + "=" + maps.join( QLatin1Char( ',' ) );
  }
  else
  {
    QgsGrassObject grassObject = currentGrassObject();

    // TODO: this is hack for network nodes, do it somehow better
    if ( mMapId.isEmpty() )
    {
      if ( !grassObject.name().isEmpty() )
      {
        list << mKey + "=" + grassObject.fullName();
      }
    }

    if ( !mVectorLayerOption.isEmpty() && currentLayer() )
    {
      list << mVectorLayerOption + "=" + QString::number( currentLayer()->number() );
    }

    if ( !mGeometryTypeOption.isEmpty() )
    {
      list << mGeometryTypeOption + "=" + currentGeometryTypeNames().join( QLatin1Char( ',' ) );
    }
  }

  return list;
}

QgsFields QgsGrassModuleInput::currentFields()
{
  QgsGrassVectorLayer *layer = currentLayer();
  if ( !layer )
  {
    return QgsFields();
  }
  return layer->fields();
}

QgsGrassObject QgsGrassModuleInput::currentGrassObject()
{
  QgsGrassObject grassObject( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation(), QString(), QString(), mType );
  grassObject.setFullName( mComboBox->currentText() );
  return grassObject;
}

QString QgsGrassModuleInput::currentMap()
{
  return currentGrassObject().fullName();
}

QgsGrassVectorLayer *QgsGrassModuleInput::currentLayer()
{
  if ( mLayers.size() == 1 )
  {
    return mLayers.value( 0 );
  }
  if ( !mLayerComboBox )
  {
    return nullptr;
  }
  return mLayers.value( mLayerComboBox->currentIndex() );
}

QStringList QgsGrassModuleInput::currentGeometryTypeNames()
{
  QStringList typeNames;
  for ( int checkBoxType : mTypeCheckBoxes.keys() )
  {
    QCheckBox *checkBox = mTypeCheckBoxes.value( checkBoxType );
    if ( checkBox->isChecked() )
    {
      typeNames << QgsGrass::vectorTypeName( checkBoxType );
    }
  }
  return typeNames;
}

QStringList QgsGrassModuleInput::currentLayerCodes()
{
  QStringList list;

  if ( auto *lCurrentLayer = currentLayer() )
  {
    for ( QString type : currentGeometryTypeNames() )
    {
      type.replace( QLatin1String( "area" ), QLatin1String( "polygon" ) );
      list << QStringLiteral( "%1_%2" ).arg( lCurrentLayer->number() ).arg( type );
    }
  }
  QgsDebugMsgLevel( "list = " + list.join( "," ), 2 );
  return list;
}

void QgsGrassModuleInput::onChanged( const QString &text )
{
  Q_UNUSED( text ) // silence warning
  QgsDebugMsgLevel( "text = " + text, 2 );

  if ( multiple() )
  {
    return;
  }
  if ( mType == QgsGrassObject::Vector )
  {
    mLayers.clear();
    mLayerComboBox->clear();
    mLayerLabel->hide();
    mLayerComboBox->hide();
    delete mVector;
    mVector = nullptr;

    QgsGrassObject grassObject = currentGrassObject();
    if ( QgsGrass::objectExists( grassObject ) )
    {
      QgsDebugMsgLevel( "map exists", 2 );
      mVector = new QgsGrassVector( grassObject );
      if ( !mVector->openHead() )
      {
        QgsGrass::warning( mVector->error() );
      }
      else
      {
        // Find layers matching type mask
        for ( QgsGrassVectorLayer *layer : mVector->layers() )
        {
          QgsDebugMsgLevel( QString( "layer->number() = %1 layer.type() = %2 mGeometryTypeMask = %3" ).arg( layer->number() ).arg( layer->type() ).arg( mGeometryTypeMask ), 2 );
          // TODO: does it make sense to add layer 0, i.e. no layer?
          if ( layer->number() > 0 && layer->type() & mGeometryTypeMask )
          {
            mLayers.append( layer );
          }
        }
      }
      QgsDebugMsgLevel( QString( "mLayers.size() = %1" ).arg( mLayers.size() ), 2 );

      // Combo is used to get layer even if just one
      for ( QgsGrassVectorLayer *layer : mLayers )
      {
        mLayerComboBox->addItem( QString::number( layer->number() ), layer->number() );
      }
      if ( mLayers.size() > 1 )
      {
        mLayerLabel->show();
        mLayerComboBox->show();
      }
    }
    onLayerChanged(); // emits valueChanged()
  }
  else // Raster
  {
    emit valueChanged();
  }
}

void QgsGrassModuleInput::onLayerChanged()
{
  // TODO(?): support vector sublayers/types for multiple input
  if ( multiple() )
  {
    return;
  }
  for ( int checkBoxType : mTypeCheckBoxes.keys() )
  {
    QCheckBox *checkBox = mTypeCheckBoxes.value( checkBoxType );
    checkBox->setChecked( false );
    checkBox->hide();
  }

  QgsGrassVectorLayer *layer = currentLayer();
  if ( layer )
  {
    // number of types  in the layer matching mGeometryTypeMask
    int typeCount = 0;
    for ( int type : layer->types() )
    {
      if ( type & mGeometryTypeMask )
      {
        typeCount++;
      }
    }
    QgsDebugMsgLevel( QString( "typeCount = %1" ).arg( typeCount ), 2 );

    int layerType = layer->type(); // may be multiple
    for ( int checkBoxType : mTypeCheckBoxes.keys() )
    {
      QCheckBox *checkBox = mTypeCheckBoxes.value( checkBoxType );
      checkBox->hide();
      if ( checkBoxType & layerType )
      {
        checkBox->setChecked( true );
        if ( typeCount > 1 )
        {
          checkBox->show();
        }
      }
    }
  }

  emit valueChanged();
}

QString QgsGrassModuleInput::ready()
{
  QString error;

  QString noInput = tr( "no input" );
  if ( multiple() )
  {
    if ( mSelectedModel->rowCount() == 0 )
    {
      error = noInput;
    }
  }
  else
  {
    QgsDebugMsgLevel( QString( "count = %1" ).arg( mComboBox->count() ), 2 );
    if ( mComboBox->count() == 0 )
    {
      error = noInput;
    }
    else
    {
      if ( !mVectorLayerOption.isEmpty() && currentLayer() && currentLayer()->number() < 1 )
      {
        error = tr( "current map does not contain features of required type" );
      }
      else
      {
        if ( !mGeometryTypeOption.isEmpty() && currentGeometryTypeNames().isEmpty() )
        {
          error = tr( "geometry type not selected" );
        }
      }
    }
  }
  if ( !error.isEmpty() )
  {
    error.prepend( title() + " : " );
  }
  return error;
}

void QgsGrassModuleInput::onActivated( const QString &text )
{
  QgsDebugMsgLevel( "text = " + text, 2 );
  if ( multiple() )
  {
    if ( mSelectedModel->findItems( text ).size() == 0 )
    {
      QStandardItem *item = new QStandardItem( text );
      mSelectedModel->appendRow( item );
      emit valueChanged();
    }
    // QCompleter resets the text after activated() if the text is cleared here,
    // so we have unset/reset the completer to clear the text
    if ( sender() == mComboBox->completer() )
    {
      QCompleter *completer = mComboBox->completer();
      mComboBox->setCompleter( nullptr );
      mComboBox->clearEditText();
      mComboBox->setCompleter( completer );
    }
    else
    {
      mComboBox->clearEditText();
    }
  }
  else
  {
    onChanged( text );
  }
}

void QgsGrassModuleInput::deleteSelectedItem( const QModelIndex &index )
{
  if ( index.isValid() )
  {
    mSelectedModel->removeRow( index.row() );
    emit valueChanged();
  }
}
