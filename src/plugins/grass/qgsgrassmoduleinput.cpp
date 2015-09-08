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
{
  setColumnCount( 1 );
  reload();
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
    bool currentMapset = mapset == QgsGrass::getDefaultMapset();
    QStandardItem *mapsetItem = new QStandardItem( mapset );
    mapsetItem->setData( mapset, MapsetRole );
    mapsetItem->setData( mapset, Qt::EditRole );
    mapsetItem->setData( QgsGrassObject::None, TypeRole );
    mapsetItem->setSelectable( false );

    QList<QgsGrassObject::Type> types;
    types << QgsGrassObject::Raster << QgsGrassObject::Vector;
    foreach ( QgsGrassObject::Type type, types )
    {
      QStringList maps = QgsGrass::grassObjects( QgsGrass::getDefaultGisdbase() + "/" + QgsGrass::getDefaultLocation() + "/" + mapset, type );
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
        QStandardItem *mapItem = new QStandardItem( mapName );
        mapItem->setData( mapName, Qt::EditRole );
        mapItem->setData( map, MapRole );
        mapItem->setData( mapset, MapsetRole );
        mapItem->setData( type, TypeRole );
        mapsetItem->appendRow( mapItem );
      }
    }
    appendRow( mapsetItem );
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
QgsGrassModuleInputCompleter::QgsGrassModuleInputCompleter( QWidget * parent )
    : QCompleter( parent )
    , mSeparator( ":" )
{
}

QgsGrassModuleInputCompleter::QgsGrassModuleInputCompleter( QAbstractItemModel * model, QWidget * parent )
    : QCompleter( model, parent )
    , mSeparator( ":" )
{
}

QString QgsGrassModuleInputCompleter::pathFromIndex( const QModelIndex& index ) const
{
  return QCompleter::pathFromIndex( index );
}

QStringList QgsGrassModuleInputCompleter::splitPath( const QString& path ) const
{
  return QCompleter::splitPath( path );
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
  //setModel ( mModel );
  setModel( mProxy );

  mTreeView = new QgsGrassModuleInputTreeView( this );
  mTreeView->setSelectionMode( QAbstractItemView::SingleSelection );
  //view->setSelectionMode(QAbstractItemView::MultiSelection);
  mTreeView->viewport()->installEventFilter( this );
  setView( mTreeView );
  mTreeView->expandAll();

  QgsGrassModuleInputCompleterProxy *completerProxy = new QgsGrassModuleInputCompleterProxy( this );
  completerProxy->setSourceModel( mProxy );

  QCompleter *completer = new QgsGrassModuleInputCompleter( completerProxy );
  completer->setCompletionRole( Qt::DisplayRole );
  completer->setCaseSensitivity( Qt::CaseInsensitive );
  completer->setCompletionMode( QCompleter::PopupCompletion );
  //completer->setCompletionMode( QCompleter::UnfilteredPopupCompletion );
  completer->setMaxVisibleItems( 20 );

  // TODO: set custom treeview for popup to show items in tree structure, if possible
  //QgsGrassModuleInputPopup *popupView = new QgsGrassModuleInputPopup();
  //completer->setPopup( popupView );
  //popupView->setModel( mModel );

  setCompleter( completer );
  setCurrentIndex( -1 );
}

bool QgsGrassModuleInputComboBox::eventFilter( QObject * watched, QEvent * event )
{
  //QgsDebugMsg( QString( "event type = %1" ).arg( event->type() ) );
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

  // reset state to fix the bug after drag
  mTreeView->resetState();
}

QgsGrassModuleInputComboBox::~QgsGrassModuleInputComboBox()
{

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
    , mComboBox( 0 )
    , mRegionButton( 0 )
    , mLayerLabel( 0 )
    , mLayerComboBox( 0 )
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
  //mModel = new QgsGrassModuleInputModel(this);
  // Map + region
  QHBoxLayout *mapLayout = new QHBoxLayout( this );
  layout->addLayout( mapLayout );

  // Map input
  mComboBox = new QgsGrassModuleInputComboBox( mType, this );
  mComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
  //connect( mComboBox, SIGNAL( activated( int ) ), this, SLOT( changed( int ) ) );
  connect( mComboBox, SIGNAL( activated( const QString & ) ), this, SLOT( onChanged( const QString & ) ) );
  connect( mComboBox, SIGNAL( editTextChanged( const QString & ) ), this, SLOT( onChanged( const QString & ) ) );
  mapLayout->addWidget( mComboBox );

  // Region button
  QString region = qdesc.attribute( "region" );
  if ( mType == QgsGrassObject::Raster && region != "no" && !mDirect )
  {
    mRegionButton = new QPushButton( QgsGrassPlugin::getThemeIcon( "grass_set_region.png" ), "" );

    mRegionButton->setToolTip( tr( "Use region of this map" ) );
    mRegionButton->setCheckable( true );
    mRegionButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy:: Preferred );
    mapLayout->addWidget( mRegionButton );
  }

  // Vector layer + type
  if ( mType == QgsGrassObject::Vector )
  {
    QHBoxLayout *layerLayout = new QHBoxLayout( this );
    layout->addLayout( layerLayout );

    mLayerLabel = new QLabel( tr( "Sublayer" ), this );
    layerLayout->addWidget( mLayerLabel );

    mLayerComboBox = new QComboBox();
    connect( mLayerComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onLayerChanged() ) );
    layerLayout->addWidget( mLayerComboBox );

    QHBoxLayout *typeLayout = new QHBoxLayout( this );
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


  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer *> ) ),
           this, SLOT( updateQgisLayers() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( updateQgisLayers() ) );


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

#if 0
QgsMapLayer * QgsGrassModuleInput::currentLayer()
{
  QgsDebugMsg( "entered" );

  int limit = 0;
  if ( !mRequired )
    limit = 1;

  int current = mComboBox->currentIndex();
  if ( current < limit )
    return 0;

  if ( current >= limit && current <  mMapLayers.size() )
  {
    return mMapLayers[current];
  }

  return 0;
}
#endif

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
    QgsDebugMsg( "map does not exist" );
    emit valueChanged();
  }
}

void QgsGrassModuleInput::onLayerChanged()
{
  QgsDebugMsg( "entered" );

  foreach ( int checkBoxType, mTypeCheckBoxes.keys() )
  {
    QCheckBox *checkBox = mTypeCheckBoxes.value( checkBoxType );
    checkBox->setChecked( false );
    checkBox->hide();
  }

  QgsGrassVectorLayer * layer = currentLayer();

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

  QgsDebugMsg( QString( "count = %1" ).arg( mComboBox->count() ) );
  if ( mComboBox->count() == 0 )
  {
    error = tr( "no input" );
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
  if ( !error.isEmpty() )
  {
    error.prepend( title() + " : " );
  }
  return error;
}


