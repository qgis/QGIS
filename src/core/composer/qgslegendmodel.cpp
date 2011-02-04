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
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbol.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QSettings>
#include <QMessageBox>

QgsLegendModel::QgsLegendModel(): QStandardItemModel(), mAutoUpdate( true )
{
  if ( QgsMapLayerRegistry::instance() )
  {
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );
  }
  setItemPrototype( new QgsComposerSymbolItem() );

  QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
  mHasTopLevelWindow = ( topLevelWidgets.size() > 0 );
}

QgsLegendModel::~QgsLegendModel()
{
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
        //QString layerID = currentItem->data(Qt::UserRole + 2).toString();
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

void QgsLegendModel::setLayerSet( const QStringList& layerIds )
{
  mLayerIds = layerIds;

  //for now clear the model and add the new entries
  clear();

  QStringList::const_iterator idIter = mLayerIds.constBegin();
  QgsMapLayer* currentLayer = 0;

  for ( ; idIter != mLayerIds.constEnd(); ++idIter )
  {
    currentLayer = QgsMapLayerRegistry::instance()->mapLayer( *idIter );
    addLayer( currentLayer );
  }
}

QStandardItem* QgsLegendModel::addGroup( QString text, int position )
{
  QgsComposerGroupItem* groupItem = new QgsComposerGroupItem( text );
  if ( position == -1 )
  {
    invisibleRootItem()->insertRow( invisibleRootItem()->rowCount(), groupItem );
  }
  else
  {
    invisibleRootItem()->insertRow( position, groupItem );
  }
  return groupItem;
}

int QgsLegendModel::addVectorLayerItemsV2( QStandardItem* layerItem, QgsVectorLayer* vlayer )
{
  if ( !layerItem || !vlayer )
  {
    return 1;
  }

  QgsFeatureRendererV2* renderer = vlayer->rendererV2();
  if ( !renderer )
  {
    return 2;
  }

  QgsLegendSymbolList lst = renderer->legendSymbolItems();
  QgsLegendSymbolList::const_iterator symbolIt = lst.constBegin();
  for ( ; symbolIt != lst.constEnd(); ++symbolIt )
  {
    QgsComposerSymbolV2Item* currentSymbolItem = new QgsComposerSymbolV2Item( symbolIt->first );
    currentSymbolItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    if ( symbolIt->second )
    {
      if ( mHasTopLevelWindow ) //only use QIcon / QPixmap if we have a running x-server
      {
        currentSymbolItem->setIcon( QgsSymbolLayerV2Utils::symbolPreviewIcon( symbolIt->second, QSize( 30, 30 ) ) );
      }
      currentSymbolItem->setSymbolV2( symbolIt->second->clone() );
    }
    layerItem->setChild( layerItem->rowCount(), 0, currentSymbolItem );
  }

  return 0;
}

int QgsLegendModel::addVectorLayerItems( QStandardItem* layerItem, QgsVectorLayer* vlayer )
{
  if ( !layerItem || !vlayer )
  {
    return 1;
  }

  int opacity = vlayer->getTransparency();

  const QgsRenderer* vectorRenderer = vlayer->renderer();
  if ( !vectorRenderer )
  {
    return 3;
  }

  //text field that describes classification attribute?
  QSettings settings;
  if ( settings.value( "/qgis/showLegendClassifiers", false ).toBool() )
  {
    QgsFieldMap layerFields = vlayer->pendingFields();
    QgsAttributeList attributes = vectorRenderer->classificationAttributes();
    QgsAttributeList::const_iterator att_it = attributes.constBegin();
    for ( ; att_it != attributes.constEnd(); ++att_it )
    {
      QgsFieldMap::const_iterator fieldIt = layerFields.find( *att_it );
      if ( fieldIt != layerFields.constEnd() )
      {
        QString attributeName = vlayer->attributeDisplayName( fieldIt.key() );
        QStandardItem* attributeItem = new QStandardItem( attributeName );
        attributeItem->setData( QgsLegendModel::ClassificationItem, Qt::UserRole + 1 ); //first user data stores the item type
        attributeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        layerItem->setChild( layerItem->rowCount(), 0, attributeItem );
      }
    }
  }

  const QList<QgsSymbol*> vectorSymbols = vectorRenderer->symbols();
  QList<QgsSymbol*>::const_iterator symbolIt = vectorSymbols.constBegin();

  for ( ; symbolIt != vectorSymbols.constEnd(); ++symbolIt )
  {
    if ( !( *symbolIt ) )
    {
      continue;
    }

    QStandardItem* currentSymbolItem = itemFromSymbol( *symbolIt, opacity, vlayer->getLayerID() );
    if ( !currentSymbolItem )
    {
      continue;
    }

    layerItem->setChild( layerItem->rowCount(), 0, currentSymbolItem );

  }

  return 0;
}

int QgsLegendModel::addRasterLayerItem( QStandardItem* layerItem, QgsMapLayer* rlayer )
{
  if ( !layerItem || !rlayer )
  {
    return 1;
  }

  QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer *>( rlayer );
  if ( !rasterLayer )
  {
    return 2;
  }

  QgsComposerRasterSymbolItem* currentSymbolItem = new QgsComposerRasterSymbolItem();
  //use a vector symbol item without symbol
  if ( mHasTopLevelWindow ) //only use QIcon / QPixmap if we have a running x-server
  {
    currentSymbolItem->setIcon( QIcon( rasterLayer->legendAsPixmap( true ) ) );
  }
  currentSymbolItem->setLayerID( rasterLayer->getLayerID() );
  int currentRowCount = layerItem->rowCount();
  layerItem->setChild( currentRowCount, 0, currentSymbolItem );

  return 0;
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

void QgsLegendModel::updateLayer( QStandardItem* layerItem )
{
  QgsComposerLayerItem* lItem = dynamic_cast<QgsComposerLayerItem*>( layerItem );
  if ( lItem )
  {
    QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( lItem->layerID() );
    if ( mapLayer )
    {
      //delete all the entries under layer item
      int currentRowCount = lItem->rowCount();
      for ( int i = currentRowCount - 1; i >= 0; --i )
      {
        lItem->removeRow( i );
      }

      //set layer name as item text
      layerItem->setText( mapLayer->name() );

      QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>( mapLayer );
      if ( vLayer )
      {
        if ( vLayer->isUsingRendererV2() )
        {
          addVectorLayerItemsV2( lItem, vLayer );
        }
        else
        {
          addVectorLayerItems( lItem, vLayer );
        }
      }

      QgsRasterLayer* rLayer = qobject_cast<QgsRasterLayer*>( mapLayer );
      if ( rLayer )
      {
        addRasterLayerItem( lItem, rLayer );
      }
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
      removeRow( i ); //todo: also remove the subitems and their symbols...
      emit layersChanged();
      return;
    }
  }
}

void QgsLegendModel::addLayer( QgsMapLayer* theMapLayer )
{
  if ( !theMapLayer )
  {
    return;
  }

  QgsComposerLayerItem* layerItem = new QgsComposerLayerItem( theMapLayer->name() );
  layerItem->setLayerID( theMapLayer->getLayerID() );
  layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), layerItem );

  switch ( theMapLayer->type() )
  {
    case QgsMapLayer::VectorLayer:
    {
      QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( theMapLayer );
      if ( vl )
      {
        if ( vl->isUsingRendererV2() )
        {
          addVectorLayerItemsV2( layerItem, vl );
        }
        else
        {
          addVectorLayerItems( layerItem, vl );
        }
      }
      break;
    }
    case QgsMapLayer::RasterLayer:
      addRasterLayerItem( layerItem, theMapLayer );
      break;
    default:
      break;
  }
  emit layersChanged();
}

QStandardItem* QgsLegendModel::itemFromSymbol( QgsSymbol* s, int opacity, const QString& layerID )
{
  QgsComposerSymbolItem* currentSymbolItem = 0;

  //label
  QString itemText;
  QString label;

  QString lowerValue = s->lowerValue();
  QString upperValue = s->upperValue();

  label = s->label();

  //Take the label as item text if it is there
  if ( !label.isEmpty() )
  {
    itemText = label;
  }
  //take single value
  else if ( lowerValue == upperValue || upperValue.isEmpty() )
  {
    itemText = lowerValue;
  }
  else //or value range
  {
    itemText = lowerValue + " - " + upperValue;
  }

  //icon item
  QImage symbolImage;
  switch ( s->type() )
  {
    case QGis::Point:
      symbolImage =  s->getPointSymbolAsImage();
      break;
    case QGis::Line:
      symbolImage = s->getLineSymbolAsImage();
      break;
    case QGis::Polygon:
      symbolImage = s->getPolygonSymbolAsImage();
      break;
    default:
      return 0;
  }

  if ( opacity != 255 )
  {
    //todo: manipulate image pixel by pixel...
    QRgb oldColor;
    for ( int i = 0; i < symbolImage.height(); ++i )
    {
      QRgb* scanLineBuffer = ( QRgb* ) symbolImage.scanLine( i );
      for ( int j = 0; j < symbolImage.width(); ++j )
      {
        oldColor = symbolImage.pixel( j, i );
        scanLineBuffer[j] = qRgba( qRed( oldColor ), qGreen( oldColor ), qBlue( oldColor ), opacity );
      }
    }
  }

  currentSymbolItem = new QgsComposerSymbolItem( itemText );
  if ( mHasTopLevelWindow )//only use QIcon / QPixmap if we have a running x-server
  {
    currentSymbolItem->setIcon( QIcon( QPixmap::fromImage( symbolImage ) ) );
  }

  if ( !currentSymbolItem )
  {
    return 0;
  }

  //Pass deep copy of QgsSymbol as user data. Cast to void* necessary such that QMetaType handles it
  QgsSymbol* symbolCopy = new QgsSymbol( *s );
  currentSymbolItem->setSymbol( symbolCopy );
  currentSymbolItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  currentSymbolItem ->setLayerID( layerID );
  return currentSymbolItem;
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
  if ( legendModelElem.isNull() )
  {
    return false;
  }

  clear();

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
    appendRow( currentItem );
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
    if ( row < 0 )
    {
      dropIntoItem->insertRow( dropIntoItem->rowCount(), currentItem );
    }
    else
    {
      dropIntoItem->insertRow( row + i, currentItem );
    }
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
    }
  }
  else
  {
    if ( QgsMapLayerRegistry::instance() )
    {
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
      disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );
    }
  }
}
