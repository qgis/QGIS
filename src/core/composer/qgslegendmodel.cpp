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
#include "qgsfield.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QDomDocument>
#include <QDomElement>
#include <QSettings>

QgsLegendModel::QgsLegendModel(): QStandardItemModel()
{
  if ( QgsMapLayerRegistry::instance() )
  {
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( removeLayer( const QString& ) ) );
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( addLayer( QgsMapLayer* ) ) );
  }
}

QgsLegendModel::~QgsLegendModel()
{
  removeAllSymbols();
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
    QStandardItem* layerItem = new QStandardItem( currentLayer->name() );
    //set layer id as user data into the item
    layerItem->setData( QVariant( currentLayer->getLayerID() ) );
    layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), layerItem );

    switch ( currentLayer->type() )
    {
      case QgsMapLayer::VECTOR:
        addVectorLayerItems( layerItem, currentLayer );
        break;
      case QgsMapLayer::RASTER:
        addRasterLayerItem( layerItem, currentLayer );
        break;
      default:
        break;
    }
  }

}

int QgsLegendModel::addVectorLayerItems( QStandardItem* layerItem, QgsMapLayer* vlayer )
{
  if ( !layerItem || !vlayer )
  {
    return 1;
  }

  QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( vlayer );
  if ( !vectorLayer )
  {
    return 2;
  }

  const QgsRenderer* vectorRenderer = vectorLayer->renderer();
  if ( !vectorRenderer )
  {
    return 3;
  }

  //text field that describes classification attribute?
  QSettings settings;
  if ( settings.value( "/qgis/showLegendClassifiers", false ).toBool() )
  {
    QgsVectorDataProvider* provider = vectorLayer->dataProvider();

    if ( provider )
    {
      QgsFieldMap providerFields = provider->fields();
      QgsAttributeList attributes = vectorRenderer->classificationAttributes();
      QgsAttributeList::const_iterator att_it = attributes.constBegin();
      for ( ; att_it != attributes.constEnd(); ++att_it )
      {
        QgsFieldMap::const_iterator fieldIt = providerFields.find( *att_it );
        if ( fieldIt != providerFields.constEnd() )
        {
          QString attributeName = fieldIt.value().name();
          QStandardItem* attributeItem = new QStandardItem( attributeName );
          layerItem->setChild( layerItem->rowCount(), 0, attributeItem );
        }
      }
    }
  }

  const QList<QgsSymbol*> vectorSymbols = vectorRenderer->symbols();
  QList<QgsSymbol*>::const_iterator symbolIt = vectorSymbols.constBegin();

  QStandardItem* currentSymbolItem = 0;

  for ( ; symbolIt != vectorSymbols.constEnd(); ++symbolIt )
  {
    if ( !( *symbolIt ) )
    {
      continue;
    }

    //label
    QString label;
    QString lowerValue = ( *symbolIt )->lowerValue();
    QString upperValue = ( *symbolIt )->upperValue();

    if ( lowerValue == upperValue || upperValue.isEmpty() )
    {
      label = lowerValue;
    }
    else
    {
      label = lowerValue + " - " + upperValue;
    }

    //icon item
    switch (( *symbolIt )->type() )
    {
      case QGis::Point:
        currentSymbolItem = new QStandardItem( QIcon( QPixmap::fromImage(( *symbolIt )->getPointSymbolAsImage() ) ), label );
        break;
      case QGis::Line:
        currentSymbolItem = new QStandardItem( QIcon( QPixmap::fromImage(( *symbolIt )->getLineSymbolAsImage() ) ), label );
        break;
      case QGis::Polygon:
        currentSymbolItem = new QStandardItem( QIcon( QPixmap::fromImage(( *symbolIt )->getPolygonSymbolAsImage() ) ), label );
        break;
      default:
        currentSymbolItem = 0;
        break;
    }

    //Pass deep copy of QgsSymbol as user data. Cast to void* necessary such that QMetaType handles it
    QgsSymbol* symbolCopy = new QgsSymbol( **symbolIt );
    currentSymbolItem->setData( QVariant::fromValue(( void* )symbolCopy ) );
    insertSymbol( symbolCopy );

    if ( !currentSymbolItem )
    {
      continue;
    }

    currentSymbolItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

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

  QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( rlayer );
  if ( !rasterLayer )
  {
    return 2;
  }

  QStandardItem* currentSymbolItem = new QStandardItem( QIcon( rasterLayer->getLegendQPixmap( true ) ), "" );

  int currentRowCount = layerItem->rowCount();
  layerItem->setChild( currentRowCount, 0, currentSymbolItem );

  return 0;
}

void QgsLegendModel::insertSymbol( QgsSymbol* s )
{
  QSet<QgsSymbol*>::iterator it = mSymbols.find( s );
  if ( it != mSymbols.end() )
  {
    delete( *it ); //very unlikely
  }
  mSymbols.insert( s );
}

void QgsLegendModel::removeSymbol( QgsSymbol* s )
{
  mSymbols.remove( s );
}

void QgsLegendModel::removeAllSymbols()
{
  QSet<QgsSymbol*>::iterator it = mSymbols.begin();
  for ( ; it != mSymbols.end(); ++it )
  {
    delete *it;
  }
  mSymbols.clear();
}

#if 0
void QgsLegendModel::updateLayerEntries( const QStringList& newLayerIds )
{
  if ( !invisibleRootItem() )
  {
    return;
  }

  //check for layers to remove
  QStandardItem* currentLayerItem = 0;
  QSet<int> rowsToRemove;

  int numRootItems = rowCount();
  for ( int i = 0; i < numRootItems ; ++i )
  {
    currentLayerItem = item( i );
    if ( !currentLayerItem )
    {
      continue;
    }

    QString layerId = currentLayerItem->data().toString();

    if ( !newLayerIds.contains( layerId ) ) //layer has been removed
    {
      rowsToRemove.insert( i );
    }
  }

  //remove layers in reverse order
  if ( rowsToRemove.size() > 0 )
  {
    QSet<int>::const_iterator delete_it = --rowsToRemove.constEnd();
    for ( ;; --delete_it )
    {
      removeRow( *delete_it );
      if ( delete_it == rowsToRemove.constBegin() )
      {
        break;
      }
    }

  }


  mLayerIds = newLayerIds;
}
#endif //0

void QgsLegendModel::updateLayer( const QString& layerId )
{
  //find layer item
  QStandardItem* layerItem = 0;
  QStandardItem* currentLayerItem = 0;

  int numRootItems = rowCount();
  for ( int i = 0; i < numRootItems ; ++i )
  {
    currentLayerItem = item( i );
    if ( !currentLayerItem )
    {
      continue;
    }

    QString currentId = currentLayerItem->data().toString();
    if ( currentId == layerId )
    {
      layerItem = currentLayerItem;
      break;
    }
  }

  if ( layerItem )
  {
    QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    if ( mapLayer )
    {
      //delete all the entries under layer item
      for ( int i = rowCount() - 1; i >= 0; --i )
      {
        layerItem->removeRow( i );
      }

      //and add the new ones...
      switch ( mapLayer->type() )
      {
        case QgsMapLayer::VECTOR:
          addVectorLayerItems( layerItem, mapLayer );
          break;
        case QgsMapLayer::RASTER:
          addRasterLayerItem( layerItem, mapLayer );
          break;
        default:
          break;
      }
    }
  }
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

    QString currentId = currentLayerItem->data().toString();
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
  //set layer id as user data into the item
  layerItem->setData( QVariant( theMapLayer->getLayerID() ) );
  layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), layerItem );

  //and child items of layer
  switch ( theMapLayer->type() )
  {
    case QgsMapLayer::VECTOR:
      addVectorLayerItems( layerItem, theMapLayer );
      break;
    case QgsMapLayer::RASTER:
      addRasterLayerItem( layerItem, theMapLayer );
      break;
    default:
      break;
  }
  emit layersChanged();
}

bool QgsLegendModel::writeXML( QDomElement& composerLegendElem, QDomDocument& doc )
{
  if ( composerLegendElem.isNull() )
  {
    return false;
  }

  QDomElement legendModelElem = doc.createElement( "Model" );

  //iterate over all items...
  QStandardItem* currentLayerItem = 0;
  QStandardItem* currentClassificationItem = 0;
  int numRootItems = rowCount();

  for ( int i = 0; i < numRootItems; ++i )
  {
    currentLayerItem = item( i );
    QDomElement newLayerItem = doc.createElement( "LayerItem" );
    newLayerItem.setAttribute( "layerId", currentLayerItem->data().toString() );
    newLayerItem.setAttribute( "text", currentLayerItem->text() );

    //add layer/classification items
    int numClassItems = currentLayerItem->rowCount();
    for ( int j = 0; j < numClassItems; ++j )
    {
      currentClassificationItem = currentLayerItem->child( j );

      //store text and QgsSymbol for vector classification items
      QVariant symbolVariant = currentClassificationItem->data();
      QgsSymbol* symbol = 0;
      if ( symbolVariant.canConvert<void*>() )
      {
        void* symbolData = symbolVariant.value<void*>();
        symbol = ( QgsSymbol* )( symbolData );
      }
      if ( symbol )
      {
        QDomElement vectorClassElem = doc.createElement( "VectorClassificationItem" );
        vectorClassElem.setAttribute( "text", currentClassificationItem->text() );
        symbol->writeXML( vectorClassElem, doc );
        newLayerItem.appendChild( vectorClassElem );
        continue;
      }

      //a text item
      if ( currentClassificationItem->icon().isNull() )
      {
        QDomElement textItemElem = doc.createElement( "TextItem" );
        textItemElem.setAttribute( "text", currentClassificationItem->text() );
        newLayerItem.appendChild( textItemElem );
      }
      else //else it can only be a raster item
      {
        QDomElement rasterClassElem = doc.createElement( "RasterItem" );
        rasterClassElem.setAttribute( "text", currentClassificationItem->text() );
        //storing the layer id also in the raster item makes parsing easier
        rasterClassElem.setAttribute( "layerId", currentLayerItem->data().toString() );
        newLayerItem.appendChild( rasterClassElem );
      }
    }

    legendModelElem.appendChild( newLayerItem );
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

  //delete all stored symbols first
  removeAllSymbols();

  //iterate over layer items
  QDomNodeList layerItemList = legendModelElem.elementsByTagName( "LayerItem" );
  QgsMapLayer* currentLayer = 0; //store current layer to get

  for ( int i = 0; i < layerItemList.size(); ++i )
  {
    QDomElement layerItemElem = layerItemList.at( i ).toElement();
    QString layerId = layerItemElem.attribute( "layerId" );

    QStandardItem* layerItem = new QStandardItem( layerItemElem.attribute( "text" ) );

    //set layer id as user data into the item
    layerItem->setData( QVariant( layerId ) );
    layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

    currentLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    //go through all children of layerItemElem
    QDomElement currentChildElement = layerItemElem.firstChildElement();
    while ( !currentChildElement.isNull() )
    {
      QStandardItem* childItem = new QStandardItem( currentChildElement.attribute( "text" ) );
      if ( currentChildElement.tagName() == "RasterItem" )
      {
        //get icon from current layer
        QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( currentLayer );
        if ( rasterLayer )
        {
          childItem->setIcon( QIcon( rasterLayer->getLegendQPixmap( true ) ) );
        }
        layerItem->setChild( layerItem->rowCount(), 0, childItem );
      }
      else if ( currentChildElement.tagName() == "VectorClassificationItem" )
      {
        //read QgsSymbol from xml and get icon
        QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
        if ( vectorLayer )
        {
          //look for symbol
          QDomNodeList symbolNodeList = currentChildElement.elementsByTagName( "symbol" );
          if ( symbolNodeList.size() > 0 )
          {
            QgsSymbol* symbol = new QgsSymbol( vectorLayer->vectorType() );
            QDomNode symbolNode = symbolNodeList.at( 0 );
            symbol->readXML( symbolNode );
            childItem->setData( QVariant::fromValue(( void* )symbol ) );

            //add icon
            switch ( symbol->type() )
            {
              case QGis::Point:
                childItem->setIcon( QIcon( QPixmap::fromImage( symbol->getPointSymbolAsImage() ) ) );
                break;
              case QGis::Line:
                childItem->setIcon( QIcon( QPixmap::fromImage( symbol->getLineSymbolAsImage() ) ) );
                break;
              case QGis::Polygon:
                childItem->setIcon( QIcon( QPixmap::fromImage( symbol->getPolygonSymbolAsImage() ) ) );
                break;
              case QGis::Unknown:
                // should not occur
                break;
            }
            insertSymbol( symbol );
          }
        }
        layerItem->setChild( layerItem->rowCount(), 0, childItem );
      }
      else if ( currentChildElement.tagName() == "TextItem" )
      {
        layerItem->setChild( layerItem->rowCount(), 0, childItem );
      }
      else //unknown tag name, don't add item
      {
        delete childItem;
      }

      currentChildElement = currentChildElement.nextSiblingElement();
    }

    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), layerItem );
  }

  return true;
}
