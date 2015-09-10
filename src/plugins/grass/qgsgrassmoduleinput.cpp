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

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include "qgsgrass.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleparam.h"
#include "qgsgrassplugin.h"
#include "qgsgrassprovider.h"
#include "qgsgrassvector.h"

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#endif
}

#include "qgsgrassmoduleinput.h"

/**************************** QgsGrassModuleInputModel ****************************/
QgsGrassModuleInputModel::QgsGrassModuleInputModel( QObject *parent )
    : QStandardItemModel( parent )
    , mWatcher( 0 )
{
  setColumnCount( 1 );
  reload();

  QString locationPath = QgsGrass::getDefaultLocationPath();
  mWatcher = new QFileSystemWatcher( this );
  mWatcher->addPath( locationPath );

  // Watching all dirs in loacation because a dir may become a mapset later, when WIND is created

  //QStringList mapsets = QgsGrass::mapsets( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
  QStringList dirNames = locationDirNames();
  foreach ( QString dirName, dirNames )
  {
    QString dirPath = locationPath + "/" + dirName;
    // Watch the dir in any case, WIND mabe created later
    mWatcher->addPath( dirPath );

    foreach ( QString watchedDir, watchedDirs() )
    {
      watch( dirPath + "/" + watchedDir );
    }
  }
  connect( mWatcher, SIGNAL( directoryChanged( const QString & ) ), SLOT( onDirectoryChanged( const QString & ) ) );
}

void QgsGrassModuleInputModel::onDirectoryChanged( const QString & path )
{
  QgsDebugMsg( "path = " + path );

  QString locationPath = QgsGrass::getDefaultLocationPath();
  QDir parentDir( path );
  parentDir.cdUp();
  QString mapset;

  if ( path == locationPath )
  {
    QgsDebugMsg( "location = " + path );
    QStringList dirNames = locationDirNames();
    //QStringList mapsets = QgsGrass::mapsets( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );

    for ( int i = rowCount() - 1; i >= 0; i-- )
    {
      QString mapset = item( i )->text();
      if ( !QgsGrass::isMapset( locationPath + "/" + mapset ) )
      {
        QgsDebugMsg( "removed mapset " + mapset );
        removeRows( i, 1 );
      }
    }

    foreach ( QString dirName, dirNames )
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
    QgsDebugMsg( "mapset = " + path );
    QDir dir( path );
    mapset = dir.dirName();
    foreach ( QString watchedDir, watchedDirs() )
    {
      watch( path + "/" + watchedDir );
    }
  }
  else // cellhd or vector dir
  {
    QgsDebugMsg( "cellhd/vector = " + path );
    mapset = parentDir.dirName();
  }
  if ( !mapset.isEmpty() )
  {
    QList<QStandardItem *> items = findItems( mapset );
    if ( items.size() == 1 )
    {
      refreshMapset( items[0], mapset );
    }
  }
}

void QgsGrassModuleInputModel::watch( const QString & path )
{
  if ( !mWatcher->directories().contains( path ) && QFileInfo( path ).exists() )
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

void QgsGrassModuleInputModel::addMapset( const QString & mapset )
{
  QgsDebugMsg( "mapset = " + mapset );


  QStandardItem *mapsetItem = new QStandardItem( mapset );
  mapsetItem->setData( mapset, MapsetRole );
  mapsetItem->setData( mapset, Qt::EditRole );
  mapsetItem->setData( QgsGrassObject::None, TypeRole );
  mapsetItem->setSelectable( false );

  refreshMapset( mapsetItem, mapset );

  appendRow( mapsetItem );
}

void QgsGrassModuleInputModel::refreshMapset( QStandardItem *mapsetItem, const QString & mapset )
{
  QgsDebugMsg( "mapset = " + mapset );
  if ( !mapsetItem )
  {
    return;
  }
  bool currentMapset = mapset == QgsGrass::getDefaultMapset();
  QList<QgsGrassObject::Type> types;
  types << QgsGrassObject::Raster << QgsGrassObject::Vector;
  foreach ( QgsGrassObject::Type type, types )
  {
    QStringList maps = QgsGrass::grassObjects( QgsGrass::getDefaultGisdbase() + "/" + QgsGrass::getDefaultLocation() + "/" + mapset, type );
    QStringList mapNames;
    foreach ( QString map, maps )
    {
      if ( map.startsWith( "qgis_import_tmp_" ) )
      {
        continue;
      }
      QString mapName = map;
      // For now, for completer popup simplicity
      // TODO: implement tree view in popup
      if ( !currentMapset )
      {
        mapName += "@" + mapset;
      }

      bool found = false;
      for ( int i = 0; i < mapsetItem->rowCount(); i++ )
      {
        QStandardItem * item = mapsetItem->child( i );
        if ( item->text() == mapName && item->data( TypeRole ).toInt() == type )
        {
          found = true;
          break;
        }
      }
      if ( !found )
      {
        QgsDebugMsg( "add map : " + mapName );
        QStandardItem *mapItem = new QStandardItem( mapName );
        mapItem->setData( mapName, Qt::EditRole );
        mapItem->setData( map, MapRole );
        mapItem->setData( mapset, MapsetRole );
        mapItem->setData( type, TypeRole );
        mapsetItem->appendRow( mapItem );
      }
      else
      {
        QgsDebugMsg( "map exists : " + mapName );
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
        QgsDebugMsg( "remove map : " + mapName );
        mapsetItem->removeRows( i, 1 );
      }
    }
  }
}

void QgsGrassModuleInputModel::reload()
{
  clear();
  QStringList mapsets = QgsGrass::mapsets( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
  // Put current mapset on top
  mapsets.removeOne( QgsGrass::getDefaultMapset() );
  mapsets.prepend( QgsGrass::getDefaultMapset() );

  foreach ( QString mapset, mapsets )
  {
    addMapset( mapset );
  }
}

QgsGrassModuleInputModel::~QgsGrassModuleInputModel()
{

}

QgsGrassModuleInputModel *QgsGrassModuleInputModel::instance()
{
  static QgsGrassModuleInputModel sInstance;
  return &sInstance;
}

/**************************** QgsGrassModuleInputProxy ****************************/
QgsGrassModuleInputProxy::QgsGrassModuleInputProxy( QgsGrassObject::Type type, QObject *parent )
    : QSortFilterProxyModel( parent )
    , mType( type )
{
  setDynamicSortFilter( true );
}

bool QgsGrassModuleInputProxy::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( !sourceModel() )
  {
    return false;
  }
  QModelIndex sourceIndex = sourceModel()->index( sourceRow, 0, sourceParent );

  QgsDebugMsg( QString( "mType = %1 item type = %2" ).arg( mType ).arg( sourceModel()->data( sourceIndex, QgsGrassModuleInputModel::TypeRole ).toInt() ) );
  //return true;
  QgsGrassObject::Type itemType = ( QgsGrassObject::Type )( sourceModel()->data( sourceIndex, QgsGrassModuleInputModel::TypeRole ).toInt() );
  // TODO: filter out mapsets without given type? May be confusing.
  return itemType == QgsGrassObject::None || mType == itemType; // None for mapsets
}

/**************************** QgsGrassModuleInputTreeView ****************************/
QgsGrassModuleInputTreeView::QgsGrassModuleInputTreeView( QWidget * parent )
    : QTreeView( parent )
{
  setHeaderHidden( true );
}

void QgsGrassModuleInputTreeView::resetState()
{
  QAbstractItemView::setState( QAbstractItemView::NoState );
}

/**************************** QgsGrassModuleInputPopup ****************************/
QgsGrassModuleInputPopup::QgsGrassModuleInputPopup( QWidget * parent )
    : QTreeView( parent )
{
  //setMinimumHeight(200);
}

void QgsGrassModuleInputPopup::setModel( QAbstractItemModel * model )
{
  QgsDebugMsg( "entered" );
  QTreeView::setModel( model );
}

/**************************** QgsGrassModuleInputCompleterProxy ****************************/
// TODO refresh data on sourceModel data change
QgsGrassModuleInputCompleterProxy::QgsGrassModuleInputCompleterProxy( QObject * parent )
    : QAbstractProxyModel( parent )
{
}

int QgsGrassModuleInputCompleterProxy::rowCount( const QModelIndex & parent ) const
{
  Q_UNUSED( parent );
  return mRows.size();
}

QModelIndex QgsGrassModuleInputCompleterProxy::index( int row, int column, const QModelIndex & parent ) const
{
  Q_UNUSED( parent );
  return createIndex( row, column );
}

QModelIndex QgsGrassModuleInputCompleterProxy::parent( const QModelIndex & index ) const
{
  Q_UNUSED( index );
  return QModelIndex();
}

void QgsGrassModuleInputCompleterProxy::setSourceModel( QAbstractItemModel * sourceModel )
{
  QAbstractProxyModel::setSourceModel( sourceModel );
  refreshMapping();
}

QModelIndex QgsGrassModuleInputCompleterProxy::mapFromSource( const QModelIndex & sourceIndex ) const
{
  if ( !mRows.contains( sourceIndex ) )
  {
    return QModelIndex();
  }
  return createIndex( mRows.value( sourceIndex ), 0 );
}

QModelIndex QgsGrassModuleInputCompleterProxy::mapToSource( const QModelIndex & proxyIndex ) const
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
  QgsDebugMsg( "entered" );
  mIndexes.clear();
  mRows.clear();
  map( QModelIndex() );
  QgsDebugMsg( QString( "mRows.size() = %1" ).arg( mRows.size() ) );
}

void QgsGrassModuleInputCompleterProxy::map( const QModelIndex & parent, int level )
{
  //QgsDebugMsg( "entered" );
  if ( !sourceModel() )
  {
    return;
  }
  //QgsDebugMsg( "parent = " + sourceModel()->data(parent).toString() );
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
QgsGrassModuleInputCompleter::QgsGrassModuleInputCompleter( QAbstractItemModel * model, QWidget * parent )
    : QCompleter( model, parent )
{
}

bool QgsGrassModuleInputCompleter::eventFilter( QObject * watched, QEvent * event )
{
  if ( event->type() == QEvent::KeyPress && watched == widget() )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    // Disable Up/Down in edit line (causing selection of previous/next item + activated() signal)
    if ( keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down )
    {
      QgsDebugMsg( "Up/Down" );
      return true;
    }
  }
  return QCompleter::eventFilter( watched, event );
}

/**************************** QgsGrassModuleInputComboBox ****************************/
// Ideas from http://qt.shoutwiki.com/wiki/Implementing_QTreeView_in_QComboBox_using_Qt-_Part_2
// and bug work around https://bugreports.qt.io/browse/QTBUG-11913
QgsGrassModuleInputComboBox::QgsGrassModuleInputComboBox( QgsGrassObject::Type type, QWidget * parent )
    : QComboBox( parent )
    , mType( type )
    , mModel( 0 )
    , mProxy( 0 )
    , mTreeView( 0 )
    , mSkipHide( false )
{
  setEditable( true );
  setInsertPolicy( QComboBox::NoInsert );

  mModel = QgsGrassModuleInputModel::instance();
  mProxy = new QgsGrassModuleInputProxy( mType, this );
  mProxy->setSourceModel( mModel );
  setModel( mProxy );

  mTreeView = new QgsGrassModuleInputTreeView( this );
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

bool QgsGrassModuleInputComboBox::eventFilter( QObject * watched, QEvent * event )
{
  // mSkipHide does not seem to be necessary anymore, not sure why
  if ( event->type() == QEvent::MouseButtonPress && watched == view()->viewport() )
  {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );
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

QgsGrassModuleInputComboBox::~QgsGrassModuleInputComboBox()
{

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

void QgsGrassModuleInputSelectedDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index ) const
{
  if ( option.state & QStyle::State_MouseOver )
  {
    if (( QApplication::mouseButtons() & Qt::LeftButton ) == 0 )
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


    QRect iconRect( option.rect.right() - option.rect.height(),
                    option.rect.top(),
                    option.rect.height(),
                    option.rect.height() );

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

  connect( this, SIGNAL( pressed( const QModelIndex & ) ),
           mDelegate, SLOT( handlePressed( const QModelIndex & ) ) );
}

void QgsGrassModuleInputSelectedView::setModel( QAbstractItemModel *model )
{
  QTreeView::setModel( model );
  header()->hide();
  header()->setStretchLastSection( false );
  header()->setResizeMode( 0, QHeaderView::Stretch );
  header()->setResizeMode( 1, QHeaderView::Fixed );
  header()->resizeSection( 1, 16 );
}

bool QgsGrassModuleInputSelectedView::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == this && event->type() == QEvent::KeyPress && currentIndex().isValid() )
  {
    QgsDebugMsg( "KeyPress" );
    QKeyEvent *ke = static_cast<QKeyEvent*>( event );
    if (( ke->key() == Qt::Key_Delete || ke->key() == Qt::Key_Backspace ) && ke->modifiers() == 0 )
    {
      emit deleteItem( currentIndex() );
    }
  }
  else if ( obj == viewport() && event->type() == QEvent::MouseButtonRelease )
  {
    QgsDebugMsg( "MouseButtonRelease" );
    QMouseEvent * me = static_cast<QMouseEvent*>( event );
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
QgsGrassModuleInput::QgsGrassModuleInput( QgsGrassModule *module,
    QgsGrassModuleStandardOptions *options, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mType( QgsGrassObject::Vector )
    , mModuleStandardOptions( options )
    , mModel( 0 )
    , mSelectedModel( 0 )
    , mComboBox( 0 )
    , mRegionButton( 0 )
    , mLayerLabel( 0 )
    , mLayerComboBox( 0 )
    , mSelectedTreeView( 0 )
    , mVector( 0 )
    , mUpdate( false )
    , mUsesRegion( false )
    , mRequired( false )
{
  QgsDebugMsg( "entered" );
  mGeometryTypeMask = GV_POINT | GV_LINE | GV_AREA;

  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Input" );
  }
  adjustTitle();

  // Check if this parameter is required
  mRequired = gnode.toElement().attribute( "required" ) == "yes";

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  QDomNode typeNode;
  if ( element == "vector" )
  {
    mType = QgsGrassObject::Vector;

    // Read type mask if "typeoption" is defined
    QString opt = qdesc.attribute( "typeoption" );
    if ( ! opt.isNull() )
    {
      typeNode = nodeByKey( gdesc, opt );

      if ( typeNode.isNull() )
      {
        mErrors << tr( "Cannot find typeoption %1" ).arg( opt );
      }
      else
      {
        mGeometryTypeOption = opt;

        QDomNode valuesNode = typeNode.namedItem( "values" );
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
            if ( !valueElem.isNull() && valueElem.tagName() == "value" )
            {
              QDomNode n = valueNode.namedItem( "name" );
              if ( !n.isNull() )
              {
                QDomElement e = n.toElement();
                QString val = e.text().trimmed();
                mGeometryTypeMask |= QgsGrass::vectorType( val );
              }
            }
            valueNode = valueNode.nextSibling();
          }
          QgsDebugMsg( QString( "mGeometryTypeMask = %1" ).arg( mGeometryTypeMask ) );
        }
      }
    }

    // Read type mask defined in configuration
    opt = qdesc.attribute( "typemask" );
    if ( ! opt.isNull() )
    {
      int mask = 0;

      foreach ( QString typeName, opt.split( "," ) )
      {
        mask |= QgsGrass::vectorType( typeName );
      }

      mGeometryTypeMask &= mask;
      QgsDebugMsg( QString( "mask = %1 -> mGeometryTypeMask = %2" ).arg( mask ).arg( mGeometryTypeMask ) );
    }

    // Read "layeroption" if defined
    opt = qdesc.attribute( "layeroption" );
    if ( ! opt.isNull() )
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
    mMapId = qdesc.attribute( "mapid" );
  }
  else if ( element == "cell" )
  {
    mType = QgsGrassObject::Raster;
  }
  else
  {
    mErrors << tr( "GRASS element %1 not supported" ).arg( element );
  }

  if ( qdesc.attribute( "update" ) == "yes" )
  {
    mUpdate = true;
  }

  QVBoxLayout *layout = new QVBoxLayout( this );
  // Map + region
  QHBoxLayout *mapLayout = new QHBoxLayout();
  layout->addLayout( mapLayout );

  // Map input
  mComboBox = new QgsGrassModuleInputComboBox( mType, this );
  mComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
  // QComboBox does not emit activated() when item is selected in completer popup
  connect( mComboBox, SIGNAL( activated( const QString & ) ), this, SLOT( onActivated( const QString & ) ) );
  connect( mComboBox->completer(), SIGNAL( activated( const QString & ) ), this, SLOT( onActivated( const QString & ) ) );
  connect( mComboBox, SIGNAL( editTextChanged( const QString & ) ), this, SLOT( onChanged( const QString & ) ) );
  mapLayout->addWidget( mComboBox );

  // Region button
  QString region = qdesc.attribute( "region" );
  // TODO: implement region for multiple
  if ( mType == QgsGrassObject::Raster && region != "no" && !mDirect && !multiple() )
  {
    mRegionButton = new QPushButton( QgsGrassPlugin::getThemeIcon( "grass_set_region.png" ), "" );

    mRegionButton->setToolTip( tr( "Use region of this map" ) );
    mRegionButton->setCheckable( true );
    mRegionButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy:: Preferred );
    mapLayout->addWidget( mRegionButton );
  }

  if ( multiple() )
  {
    mSelectedModel = new QStandardItemModel( 0, 2 );
    mSelectedTreeView = new QgsGrassModuleInputSelectedView( this );
    mSelectedTreeView->setModel( mSelectedModel );
    connect( mSelectedTreeView, SIGNAL( deleteItem( const QModelIndex & ) ), this, SLOT( deleteSelectedItem( const QModelIndex & ) ) );
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
    connect( mLayerComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onLayerChanged() ) );
    layerLayout->addWidget( mLayerComboBox );

    QHBoxLayout *typeLayout = new QHBoxLayout();
    layerLayout->addLayout( typeLayout );

    // Vector types
    if ( !typeNode.isNull() )
    {

      QList<int> types;
      types << GV_POINT << GV_LINE << GV_BOUNDARY << GV_CENTROID << GV_AREA;
      foreach ( int type, types )
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
    if ( region == "yes" )
      mUsesRegion = true;
  }
  else
  {
    if ( type() == QgsGrassObject::Raster )
      mUsesRegion = true;
  }
  onChanged( "" );
}

QgsGrassModuleInput::~QgsGrassModuleInput()
{
}

bool QgsGrassModuleInput::useRegion()
{
  QgsDebugMsg( "entered" );

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
    list << mKey + "=" + maps.join( "," );
  }
  else
  {
    QgsGrassObject grassObject = currentGrassObject();

    // TODO: this is hack for network nodes, do it somehow better
    if ( mMapId.isEmpty() )
    {
      if ( !grassObject.name().isEmpty() )
      {
        list << mKey + "=" + grassObject.fullName() ;
      }
    }

    if ( !mVectorLayerOption.isEmpty() && currentLayer() )
    {
      list << mVectorLayerOption + "=" + QString::number( currentLayer()->number() );
    }

    if ( !mGeometryTypeOption.isEmpty() )
    {

      list << mGeometryTypeOption + "=" + currentGeometryTypeNames().join( "," );
    }
  }

  return list;
}

QgsFields QgsGrassModuleInput::currentFields()
{
  QgsDebugMsg( "entered" );

  QgsGrassVectorLayer * layer = currentLayer();
  if ( !layer )
  {
    return QgsFields();
  }
  return layer->fields();
}

QgsGrassObject QgsGrassModuleInput::currentGrassObject()
{
  QgsDebugMsg( "entered" );

  QgsGrassObject grassObject( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation(), "", "", mType );
  grassObject.setFullName( mComboBox->currentText() );
  return grassObject;
}

QString QgsGrassModuleInput::currentMap()
{
  return currentGrassObject().fullName();
}

QgsGrassVectorLayer * QgsGrassModuleInput::currentLayer()
{
  if ( mLayers.size() == 1 )
  {
    return mLayers.value( 0 );
  }
  if ( !mLayerComboBox )
  {
    return 0;
  }
  return mLayers.value( mLayerComboBox->currentIndex() );
}

QStringList QgsGrassModuleInput::currentGeometryTypeNames()
{
  QStringList typeNames;
  foreach ( int checkBoxType, mTypeCheckBoxes.keys() )
  {
    QCheckBox *checkBox = mTypeCheckBoxes.value( checkBoxType );
    if ( checkBox->isChecked() )
    {
      typeNames << QgsGrass::vectorTypeName( checkBoxType );
    }
  }
  return typeNames;
}

void QgsGrassModuleInput::onChanged( const QString & text )
{
  QgsDebugMsg( "text = " + text );

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
    mVector = 0;

    QgsGrassObject grassObject = currentGrassObject();
    if ( QgsGrass::objectExists( grassObject ) )
    {
      QgsDebugMsg( "map exists" );
      mVector = new QgsGrassVector( grassObject );
      if ( !mVector->openHead() )
      {
        QgsGrass::warning( mVector->error() );
      }
      else
      {
        // Find layers matching type mask
        foreach ( QgsGrassVectorLayer *layer, mVector->layers() )
        {
          QgsDebugMsg( QString( "layer->number() = %1 layer.type() = %2 mGeometryTypeMask = %3" ).arg( layer->number() ).arg( layer->type() ).arg( mGeometryTypeMask ) );
          if ( layer->type() & mGeometryTypeMask )
          {
            mLayers.append( layer );
          }
        }
      }
      QgsDebugMsg( QString( "mLayers.size() = %1" ).arg( mLayers.size() ) );

      // Combo is used to get layer even if just one
      foreach ( QgsGrassVectorLayer * layer, mLayers )
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
  QgsDebugMsg( "entered" );

  // TODO(?): support vector sublayers/types for multiple input
  if ( multiple() )
  {
    return;
  }
  foreach ( int checkBoxType, mTypeCheckBoxes.keys() )
  {
    QCheckBox *checkBox = mTypeCheckBoxes.value( checkBoxType );
    checkBox->setChecked( false );
    checkBox->hide();
  }

  QgsGrassVectorLayer * layer = currentLayer();
  if ( multiple() )
    if ( layer )
    {
      // number of types  in the layer matching mGeometryTypeMask
      int typeCount = 0;
      foreach ( int type, layer->types() )
      {
        if ( type & mGeometryTypeMask )
        {
          typeCount++;
        }
      }
      QgsDebugMsg( QString( "typeCount = %1" ).arg( typeCount ) );

      int layerType = layer->type(); // may be multiple
      foreach ( int checkBoxType, mTypeCheckBoxes.keys() )
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
  QgsDebugMsg( "entered" );

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
    QgsDebugMsg( QString( "count = %1" ).arg( mComboBox->count() ) );
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

void QgsGrassModuleInput::onActivated( const QString & text )
{
  QgsDebugMsg( "text = " + text );
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
      mComboBox->setCompleter( 0 );
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
  QgsDebugMsg( "entered" );
  if ( index.isValid() )
  {
    mSelectedModel->removeRow( index.row() );
    emit valueChanged();
  }
}
