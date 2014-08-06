/***************************************************************************
                         qgslegendmodel.cpp  -  description
                         ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendmodel.h"
#include "qgscomposerlegenditem.h"
#include "qgsfield.h"
#include "qgslayertree.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QSettings>
#include <QMessageBox>

QgsLegendModel::QgsLegendModel()
  : QStandardItemModel()
  , mScaleDenominator( -1 )
  , mAutoUpdate( true )
{
  setColumnCount( 2 );

  if ( QgsMapLayerRegistry::instance() )
  {
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );
  }

  QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
  mHasTopLevelWindow = ( topLevelWidgets.size() > 0 );
}

QgsLegendModel::~QgsLegendModel()
{
}

void QgsLegendModel::setLayerSetAndGroups( QgsLayerTreeGroup* rootGroup )
{
  clear();
  addGroupFromLayerTree( rootGroup, invisibleRootItem() );
}

void QgsLegendModel::addGroupFromLayerTree( QgsLayerTreeGroup* parentGroup, QStandardItem* parentItem )
{
  foreach ( QgsLayerTreeNode* node, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup* nodeGroup = QgsLayerTree::toGroup( node );
      QStandardItem* groupItem = addGroup( nodeGroup->name(), -1, parentItem );
      addGroupFromLayerTree( nodeGroup, groupItem );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() )
        addLayer( nodeLayer->layer(), -1, QString(), parentItem );
    }
  }
}

void QgsLegendModel::setLayerSetAndGroups( const QStringList& layerIds, const QList< GroupLayerInfo >& groupInfo )
{
  setLayerSet( layerIds );

  QStandardItem* currentItem = 0;
  QStandardItem* currentGroupItem = 0;
  int i = 0;

  QList< GroupLayerInfo >::const_iterator infoIt = groupInfo.constBegin();
  for ( ; infoIt != groupInfo.constEnd() && i < invisibleRootItem()->rowCount(); )
  {
    currentItem = invisibleRootItem()->child( i, 0 );
    QString infoKey = infoIt->first;
    if ( infoKey.isNull() ) //a toplevel layer
    {
      ++i;
    }
    else //a group
    {
      currentGroupItem = addGroup( infoKey, i );
      ++i;
      QList<QString> layerList = infoIt->second;
      QList<QString>::const_iterator groupLayerIt = layerList.constBegin();
      for ( ; currentItem && ( groupLayerIt != layerList.constEnd() ); ++groupLayerIt )
      {
        //check if current item is contained in this group
        QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( currentItem );
        if ( !layerItem )
        {
          return; //should never happen
        }
        QString layerID = layerItem->layerID();
        if ( layerList.contains( layerID ) )
        {
          takeRow( i );
          currentGroupItem->setChild( currentGroupItem->rowCount(), 0, currentItem );
        }
        else
        {
          ++i;
        }
        currentItem = invisibleRootItem()->child( i, 0 );
      }
    }
    ++infoIt;
  }
}

void QgsLegendModel::setLayerSet( const QStringList& layerIds, double scaleDenominator, QString rule )
{
  mLayerIds = layerIds;
  mScaleDenominator = scaleDenominator;
  mRule = rule;

  //for now clear the model and add the new entries
  clear();

  QStringList::const_iterator idIter = mLayerIds.constBegin();
  QgsMapLayer* currentLayer = 0;

  for ( ; idIter != mLayerIds.constEnd(); ++idIter )
  {
    currentLayer = QgsMapLayerRegistry::instance()->mapLayer( *idIter );
    addLayer( currentLayer, scaleDenominator, rule );
  }
}

QStandardItem* QgsLegendModel::addGroup( QString text, int position, QStandardItem* parentItem )
{
  if ( text.isNull() )
    text = tr( "Group" );

  if ( !parentItem )
    parentItem = invisibleRootItem();

  QgsComposerGroupItem* groupItem = new QgsComposerGroupItem( text );
  groupItem->setUserText( text );

  if ( position == -1 )
  {
    position = parentItem->rowCount();
  }
  QList<QStandardItem *> itemsList;
  itemsList << groupItem << new QgsComposerStyleItem( groupItem );
  parentItem->insertRow( position, itemsList );

  emit layersChanged();
  return groupItem;
}


void QgsLegendModel::updateItem( QStandardItem* item )
{
  if ( !item )
  {
    return;
  }

  //only layer items are supported for update
  QgsComposerLegendItem* cItem = dynamic_cast<QgsComposerLegendItem*>( item );
  if ( ! cItem )
  {
    return;
  }

  QgsComposerLegendItem::ItemType type = cItem->itemType();
  if ( type == QgsComposerLegendItem::LayerItem )
  {
    updateLayer( cItem );
  }
}

void QgsLegendModel::updateItemText( QStandardItem* item )
{
  if ( !item )
  {
    return;
  }

  //only layer items are supported for update
  QgsComposerLegendItem* cItem = dynamic_cast<QgsComposerLegendItem*>( item );
  if ( ! cItem )
  {
    return;
  }

  emit dataChanged( cItem->index(), cItem->index() );
}

void QgsLegendModel::updateLayer( QStandardItem* layerItem )
{
  QgsDebugMsg( "Entered." );
  QgsComposerLayerItem* lItem = dynamic_cast<QgsComposerLayerItem*>( layerItem );
  if ( lItem )
  {
    emit dataChanged( lItem->index(), lItem->index() );

    QgsMapLayer* mapLayer = lItem->mapLayer();
    if ( mapLayer && mapLayer->legend() )
    {
      mapLayer->legend()->createLegendModelItems( lItem );
    }
  }
}


void QgsLegendModel::removeLayer( const QString& layerId )
{
  int numRootItems = rowCount();
  for ( int i = 0; i < numRootItems ; ++i )
  {
    QgsComposerLayerItem* lItem = dynamic_cast<QgsComposerLayerItem*>( item( i ) );
    if ( !lItem )
    {
      continue;
    }

    if ( layerId == lItem->layerID() )
    {
      if ( QgsMapLayerRegistry::instance() )
      {
        QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( lItem->layerID() );
        if ( layer )
        {
          disconnect( layer, SIGNAL( rendererChanged() ), this, SLOT( updateLayer() ) );
        }
      }
      removeRow( i ); //todo: also remove the subitems and their symbols...
      emit layersChanged();
      return;
    }
  }
}

void QgsLegendModel::addLayer( QgsMapLayer* theMapLayer, double scaleDenominator, QString rule, QStandardItem* parentItem )
{
  Q_UNUSED( scaleDenominator );
  Q_UNUSED( rule );

  if ( !theMapLayer )
  {
    return;
  }

  if ( !parentItem )
    parentItem = invisibleRootItem();

  QgsComposerLayerItem* layerItem = new QgsComposerLayerItem( theMapLayer->name() );
  if ( theMapLayer->title() != "" )
  {
    layerItem->setText( theMapLayer->title() );
    layerItem->setUserText( theMapLayer->title() );
  }
  layerItem->setLayerID( theMapLayer->id() );
  layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  QList<QStandardItem *> itemsList;
  itemsList << layerItem << new QgsComposerStyleItem( layerItem );
  parentItem->appendRow( itemsList );

  theMapLayer->legend()->createLegendModelItems( layerItem );

  if ( theMapLayer->type() == QgsMapLayer::VectorLayer )
  {
    // hide title by default for single symbol
    layerItem->setStyle( layerItem->rowCount() > 1 ? QgsComposerLegendStyle::Subgroup : QgsComposerLegendStyle::Hidden );
  }

  if ( mAutoUpdate )
  {
    connect( theMapLayer, SIGNAL( rendererChanged() ), this, SLOT( updateLayer() ) );
  }

  emit layersChanged();
}

void QgsLegendModel::updateLayer()
{
  QString layerId = qobject_cast<QgsMapLayer*>( QObject::sender() )->id();

  for ( int i = 0, n = rowCount(); i < n ; ++i )
  {
    QgsComposerLayerItem* lItem = dynamic_cast<QgsComposerLayerItem*>( item( i ) );
    if ( lItem && layerId == lItem->layerID() )
    {
      updateLayer( lItem );
      emit layersChanged();
      return;
    }
  }
}

bool QgsLegendModel::writeXML( QDomElement& composerLegendElem, QDomDocument& doc ) const
{
  if ( composerLegendElem.isNull() )
  {
    return false;
  }

  QDomElement legendModelElem = doc.createElement( "Model" );
  legendModelElem.setAttribute( "autoUpdate", mAutoUpdate );
  int nTopLevelItems = invisibleRootItem()->rowCount();
  QStandardItem* currentItem = 0;
  QgsComposerLegendItem* currentLegendItem = 0;

  for ( int i = 0; i < nTopLevelItems; ++i )
  {
    currentItem = invisibleRootItem()->child( i, 0 );
    currentLegendItem = dynamic_cast<QgsComposerLegendItem*>( currentItem );
    if ( currentLegendItem )
    {
      currentLegendItem->writeXML( legendModelElem, doc );
    }
  }

  composerLegendElem.appendChild( legendModelElem );
  return true;
}

bool QgsLegendModel::readXML( const QDomElement& legendModelElem, const QDomDocument& doc )
{
  Q_UNUSED( doc );

  if ( legendModelElem.isNull() )
  {
    return false;
  }

  clear();
  //disable autoupdates here in order to have a setAutoUpdate(true)
  //below connect the rendererChanged signals to the layers
  setAutoUpdate( false );

  QDomNodeList topLevelItemList = legendModelElem.childNodes();
  QDomElement currentElem;
  QgsComposerLegendItem* currentItem = 0;

  int nTopLevelItems =  topLevelItemList.size();
  for ( int i = 0; i < nTopLevelItems; ++i )
  {
    currentElem = topLevelItemList.at( i ).toElement();
    if ( currentElem.isNull() )
    {
      continue;
    }

    //toplevel items can be groups or layers
    if ( currentElem.tagName() == "LayerItem" )
    {
      currentItem = new QgsComposerLayerItem();
    }
    else if ( currentElem.tagName() == "GroupItem" )
    {
      currentItem = new QgsComposerGroupItem();
    }
    currentItem->readXML( currentElem, mHasTopLevelWindow );

    QList<QStandardItem *> itemsList;
    itemsList << currentItem << new QgsComposerStyleItem( currentItem );
    appendRow( itemsList );
  }

  setAutoUpdate( legendModelElem.attribute( "autoUpdate", "1" ).toInt() );
  return true;
}

Qt::DropActions QgsLegendModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

Qt::ItemFlags QgsLegendModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if ( !index.isValid() )
  {
    flags |= Qt::ItemIsDropEnabled;
    return flags;
  }

  QStandardItem* item = itemFromIndex( index );
  QgsComposerLegendItem* cItem = dynamic_cast<QgsComposerLegendItem*>( item );

  if ( cItem )
  {
    QgsComposerLegendItem::ItemType type = cItem->itemType();
    if ( type == QgsComposerLegendItem::GroupItem )
    {
      flags |= Qt::ItemIsDragEnabled;
      flags |= Qt::ItemIsDropEnabled;
    }
    else if ( type == QgsComposerLegendItem::LayerItem )
    {
      flags |= Qt::ItemIsDragEnabled;
    }
  }
  if ( index.column() == 1 && item )
  {
    // Style
    QStandardItem* firstColumnItem = 0;
    if ( item->parent() )
    {
      firstColumnItem = item->parent()->child( index.row(), 0 );
    }
    else
    {
      firstColumnItem = QgsLegendModel::item( index.row(), 0 );
    }
    cItem = dynamic_cast<QgsComposerLegendItem*>( firstColumnItem );

    if ( cItem )
    {
      if ( cItem->itemType() == QgsComposerLegendItem::GroupItem ||
           cItem->itemType() == QgsComposerLegendItem::LayerItem )
      {
        flags |= Qt::ItemIsEditable;
      }
    }
  }
  return flags;
}

bool QgsLegendModel::removeRows( int row, int count, const QModelIndex & parent )
{
  if ( count < 1 )
  {
    return false;
  }

  if ( parent.isValid() )
  {
    for ( int i = row + count - 1; i >= row; --i )
    {
      QStandardItem* item = itemFromIndex( parent );
      if ( item )
      {
        item->takeRow( i );
      }
    }
  }
  else
  {
    for ( int i = row + count - 1; i >= row; --i )
    {
      takeRow( i );
    }
  }
  return true;
}

QMimeData* QgsLegendModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;
  QDomDocument xmlDoc;
  QDomElement xmlRootElement = xmlDoc.createElement( "LegendModelDragData" );
  xmlDoc.appendChild( xmlRootElement );

  QModelIndexList::const_iterator indexIt = indexes.constBegin();
  for ( ; indexIt != indexes.constEnd(); ++indexIt )
  {
    QStandardItem* sItem = itemFromIndex( *indexIt );
    if ( sItem )
    {
      QgsComposerLegendItem* mItem = dynamic_cast<QgsComposerLegendItem*>( sItem );
      if ( mItem )
      {
        mItem->writeXML( xmlRootElement, xmlDoc );
      }
    }
  }
  mimeData->setData( "text/xml", xmlDoc.toByteArray() );
  return mimeData;
}

QStringList QgsLegendModel::mimeTypes() const
{
  QStringList types;
  types << "text/xml";
  return types;
}

bool QgsLegendModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( action );
  Q_UNUSED( column );

  if ( !data->hasFormat( "text/xml" ) )
  {
    return false;
  }

  QStandardItem* dropIntoItem = 0;
  if ( parent.isValid() )
  {
    dropIntoItem = itemFromIndex( parent );
  }
  else
  {
    dropIntoItem = invisibleRootItem();
  }

  //get XML doc
  QByteArray encodedData = data->data( "text/xml" );
  QDomDocument xmlDoc;
  xmlDoc.setContent( encodedData );

  QDomElement dragDataElem = xmlDoc.documentElement();
  if ( dragDataElem.tagName() != "LegendModelDragData" )
  {
    return false;
  }

  QDomNodeList nodeList = dragDataElem.childNodes();
  int nChildNodes = nodeList.size();
  QDomElement currentElem;
  QString currentTagName;
  QgsComposerLegendItem* currentItem = 0;

  for ( int i = 0; i < nChildNodes; ++i )
  {
    currentElem = nodeList.at( i ).toElement();
    if ( currentElem.isNull() )
    {
      continue;
    }
    currentTagName = currentElem.tagName();
    if ( currentTagName == "LayerItem" )
    {
      currentItem = new QgsComposerLayerItem();
    }
    else if ( currentTagName == "GroupItem" )
    {
      currentItem = new QgsComposerGroupItem();
    }
    else
    {
      continue;
    }
    currentItem->readXML( currentElem );
    int index;
    if ( row < 0 )
    {
      index = dropIntoItem->rowCount();
    }
    else
    {
      index = row + i;
    }
    QList<QStandardItem *> itemsList;
    itemsList << currentItem << new QgsComposerStyleItem( currentItem );
    dropIntoItem->insertRow( index, itemsList );
  }
  emit layersChanged();
  return true;
}

void QgsLegendModel::setAutoUpdate( bool autoUpdate )
{
  if ( mAutoUpdate == autoUpdate ) //prevent multiple signal/slot connections
  {
    return;
  }

  mAutoUpdate = autoUpdate;
  if ( autoUpdate )
  {
    if ( QgsMapLayerRegistry::instance() )
    {
      connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
      connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );

      for ( int i = 0, n = rowCount(); i < n ; ++i )
      {
        QgsComposerLayerItem* lItem = dynamic_cast<QgsComposerLayerItem*>( item( i ) );
        if ( lItem )
        {
          QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( lItem->layerID() );
          if ( layer )
          {
            connect( layer, SIGNAL( rendererChanged() ), this, SLOT( updateLayer() ) );
          }
        }
      }
    }
  }
  else
  {
    if ( QgsMapLayerRegistry::instance() )
    {
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );

      for ( int i = 0, n = rowCount(); i < n ; ++i )
      {
        QgsComposerLayerItem* lItem = dynamic_cast<QgsComposerLayerItem*>( item( i ) );
        if ( lItem )
        {
          QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( lItem->layerID() );
          if ( layer )
          {
            disconnect( layer, SIGNAL( rendererChanged() ), this, SLOT( updateLayer() ) );
          }
        }
      }
    }
  }
}
