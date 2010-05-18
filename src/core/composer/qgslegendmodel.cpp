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
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QSettings>

QgsLegendModel::QgsLegendModel(): QStandardItemModel()
{
  if ( QgsMapLayerRegistry::instance() )
  {
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );
  }
  setItemPrototype( new QgsComposerSymbolItem() );
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

    //addItem for layer
    QgsComposerLayerItem* layerItem = new QgsComposerLayerItem( currentLayer->name() );
    layerItem->setLayerID( currentLayer->getLayerID() );
    layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), layerItem );

    switch ( currentLayer->type() )
    {
      case QgsMapLayer::VectorLayer:
      {
        QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( currentLayer );
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
        addRasterLayerItem( layerItem, currentLayer );
        break;
      default:
        break;
    }
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
      currentSymbolItem->setIcon( QgsSymbolLayerV2Utils::symbolPreviewIcon( symbolIt->second, QSize( 30, 30 ) ) );
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

  //use a vector symbol item without symbol
  QgsComposerRasterSymbolItem* currentSymbolItem = new QgsComposerRasterSymbolItem( QIcon( rasterLayer->legendAsPixmap( true ) ), "" );
  currentSymbolItem->setLayerID( rasterLayer->getLayerID() );
  int currentRowCount = layerItem->rowCount();
  layerItem->setChild( currentRowCount, 0, currentSymbolItem );

  return 0;
}

void QgsLegendModel::updateItem( QStandardItem* item )
{
#if 0
  if ( !item )
  {
    return;
  }

  //is it a toplevel layer item?
  QModelIndex itemIndex = indexFromItem( item );
  QModelIndex parentIndex = itemIndex.parent();
  if ( !parentIndex.isValid() ) // a layer item?
  {
    updateLayer( item );
  }

  //take QgsSymbol* from user data
  QVariant symbolVariant = item->data( Qt::UserRole + 2 );
  QgsSymbol* symbol = 0;
  if ( symbolVariant.canConvert<void*>() )
  {
    void* symbolData = symbolVariant.value<void*>();
    symbol = ( QgsSymbol* )( symbolData );
  }

  QVariant symbolNgVariant = item->data( Qt::UserRole + 3 );
  QgsSymbolV2* symbolNg = 0;
  if ( symbolNgVariant.canConvert<void*>() )
  {
    void* symbolNgData = symbolVariant.value<void*>();
    symbolNg = ( QgsSymbolV2* )symbolNgData;
  }

  if ( symbol )  //vector classification item
  {
    updateVectorClassificationItem( item, symbol, item->text() );
  }
  else if ( symbolNg )
  {
    updateVectorV2ClassificationItem( item, symbolNg, item->text() );
  }
  else if ( !item->icon().isNull() ) //raster classification item
  {
    updateRasterClassificationItem( item );
  }
#endif //0
}

void QgsLegendModel::updateLayer( QStandardItem* layerItem )
{
#if 0
  if ( !layerItem )
  {
    return;
  }

  QString layerId = layerItem->data( Qt::UserRole + 2 ).toString();
  QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if ( mapLayer )
  {
    //delete all the entries under layer item
    int currentRowCount = layerItem->rowCount();
    for ( int i = currentRowCount - 1; i >= 0; --i )
    {
      layerItem->removeRow( i );
    }

    //and add the new ones...
    switch ( mapLayer->type() )
    {
      case QgsMapLayer::VectorLayer:
      {
        QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( mapLayer );
        if ( vLayer )
        {
          if ( vLayer->isUsingRendererV2() )
          {
            addVectorLayerItemsV2( layerItem, vLayer );
          }
          else
          {
            addVectorLayerItems( layerItem, vLayer );
          }
        }
      }
      break;
      case QgsMapLayer::RasterLayer:
        addRasterLayerItem( layerItem, mapLayer );
        break;
      default:
        break;
    }
  }
#endif //0
}

void QgsLegendModel::updateVectorClassificationItem( QStandardItem* classificationItem, QgsSymbol* symbol, QString itemText )
{
#if 0
  //this function uses the following logic to find a classification match:
  //first test if there is a symbol where lowerbound - upperbound equels itemText
  //if no match found, test if there is a symbol where label equals itemText
  //still no match found. Test, if there is a symbol with same pen/brush/point symbol

  //get parent item
  QStandardItem* parentItem = classificationItem->parent();
  if ( !parentItem )
  {
    return;
  }

  //get maplayer object from parent item
  QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( parentItem->data( Qt::UserRole + 2 ).toString() );
  if ( !ml )
  {
    return;
  }
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
  if ( !vl )
  {
    return;
  }
  int opacity = vl->getTransparency();

  const QgsRenderer* layerRenderer = vl->renderer();
  if ( !layerRenderer )
  {
    return;
  }

  QList<QgsSymbol*> symbolList = layerRenderer->symbols();
  QList<QgsSymbol*>::iterator symbolIt;
  QgsSymbol* currentSymbol = 0;

  //try to find a symbol where lowerbound - upperbound matches item text
  symbolIt = symbolList.begin();
  for ( ; symbolIt != symbolList.end(); ++symbolIt )
  {
    currentSymbol = *symbolIt;
    if ( currentSymbol->lowerValue() + " - " + currentSymbol->upperValue() == itemText )
    {
      parentItem->insertRow( classificationItem->row(), itemFromSymbol( currentSymbol, opacity, vl->getLayerID() ) );
      parentItem->removeRow( classificationItem->row() );
      return;
    }
  }

  //try to find a symbol where lower value matches item text (non-numeric classifications)
  symbolIt = symbolList.begin();
  for ( ; symbolIt != symbolList.end(); ++symbolIt )
  {
    currentSymbol = *symbolIt;
    if ( currentSymbol->lowerValue() == itemText )
    {
      parentItem->insertRow( classificationItem->row(), itemFromSymbol( currentSymbol, opacity, vl->getLayerID() ) );
      parentItem->removeRow( classificationItem->row() );
      return;
    }
  }

  //try to find a symbol where label matches item text
  symbolIt = symbolList.begin();
  for ( ; symbolIt != symbolList.end(); ++symbolIt )
  {
    currentSymbol = *symbolIt;
    if ( currentSymbol->label() == itemText )
    {
      removeSymbol( symbol );
      parentItem->insertRow( classificationItem->row(), itemFromSymbol( currentSymbol, opacity, vl->getLayerID() ) );
      parentItem->removeRow( classificationItem->row() );
      return;
    }
  }
#endif //0
}

void QgsLegendModel::updateVectorV2ClassificationItem( QStandardItem* classificationItem, QgsSymbolV2* symbol, QString itemText )
{
  //todo...
}


void QgsLegendModel::updateRasterClassificationItem( QStandardItem* classificationItem )
{
#if 0
  if ( !classificationItem )
  {
    return;
  }

  QStandardItem* parentItem = classificationItem->parent();
  if ( !parentItem )
  {
    return;
  }

  QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( parentItem->data( Qt::UserRole + 2 ).toString() );
  if ( !ml )
  {
    return;
  }

  QgsRasterLayer* rl = qobject_cast<QgsRasterLayer *>( ml );
  if ( !rl )
  {
    return;
  }

  QStandardItem* currentSymbolItem = new QStandardItem( QIcon( rl->legendAsPixmap( true ) ), "" );

  currentSymbolItem->setData( QgsLegendModel::ClassificationItem, Qt::UserRole + 1 ); //first user data stores the item type
  parentItem->insertRow( 0, currentSymbolItem );
  parentItem->removeRow( 1 );
#endif //0
}

void QgsLegendModel::removeLayer( const QString& layerId )
{
  QStandardItem* currentLayerItem = 0;

  int numRootItems = rowCount();
  for ( int i = 0; i < numRootItems ; ++i )
  {
    currentLayerItem = item( i );
    if ( !currentLayerItem )
    {
      continue;
    }

    QString currentId = currentLayerItem->data( Qt::UserRole + 2 ).toString();
    if ( currentId == layerId )
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

  //append new layer item
  QStandardItem* layerItem = new QStandardItem( theMapLayer->name() );
  layerItem->setData( QgsLegendModel::LayerItem, Qt::UserRole + 1 ); //first user data stores the item type
  layerItem->setData( QVariant( theMapLayer->getLayerID() ), Qt::UserRole + 2 );
  layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), layerItem );

  //and child items of layer
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

  currentSymbolItem = new QgsComposerSymbolItem( QIcon( QPixmap::fromImage( symbolImage ) ), itemText );

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
  int nTopLevelItems = invisibleRootItem()->rowCount();
  QStandardItem* currentItem = 0;
  QgsComposerLegendItem* currentLegendItem = 0;

  for ( int i = 0; i < nTopLevelItems; ++i )
  {
    currentItem = invisibleRootItem()->child( i, 0 );
    currentLegendItem = dynamic_cast<QgsComposerLegendItem*>( currentItem );
    if ( currentItem )
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
    currentItem->readXML( currentElem );
    appendRow( currentItem );
  }
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
