/***************************************************************************
                              qgslayout.cpp
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayout.h"
#include "qgslayoutitem.h"
#include "qgslayoutmodel.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutguidecollection.h"
#include "qgsreadwritecontext.h"
#include "qgsproject.h"
#include "qgslayoutitemundocommand.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemgroupundocommand.h"
#include "qgslayoutmultiframe.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutundostack.h"
#include "qgscompositionconverter.h"

QgsLayout::QgsLayout( QgsProject *project )
  : mProject( project )
  , mRenderContext( new QgsLayoutRenderContext( this ) )
  , mReportContext( new QgsLayoutReportContext( this ) )
  , mSnapper( QgsLayoutSnapper( this ) )
  , mGridSettings( this )
  , mPageCollection( new QgsLayoutPageCollection( this ) )
  , mUndoStack( new QgsLayoutUndoStack( this ) )
{
  // just to make sure - this should be the default, but maybe it'll change in some future Qt version...
  setBackgroundBrush( Qt::NoBrush );
  mItemsModel.reset( new QgsLayoutModel( this ) );
}

QgsLayout::~QgsLayout()
{
  // no need for undo commands when we're destroying the layout
  mUndoStack->blockCommands( true );

  deleteAndRemoveMultiFrames();

  // make sure that all layout items are removed before
  // this class is deconstructed - to avoid segfaults
  // when layout items access in destructor layout that isn't valid anymore

  // since deletion of some item types (e.g. groups) trigger deletion
  // of other items, we have to do this careful, one at a time...
  QList<QGraphicsItem *> itemList = items();
  bool deleted = true;
  while ( deleted )
  {
    deleted = false;
    for ( QGraphicsItem *item : qgis::as_const( itemList ) )
    {
      if ( dynamic_cast< QgsLayoutItem * >( item ) && !dynamic_cast< QgsLayoutItemPage *>( item ) )
      {
        delete item;
        deleted = true;
        break;
      }
    }
    itemList = items();
  }

  mItemsModel.reset(); // manually delete, so we can control order of destruction
}

QgsLayout *QgsLayout::clone() const
{
  QDomDocument currentDoc;

  QgsReadWriteContext context;
  QDomElement elem = writeXml( currentDoc, context );
  currentDoc.appendChild( elem );

  std::unique_ptr< QgsLayout > newLayout = qgis::make_unique< QgsLayout >( mProject );
  bool ok = false;
  newLayout->loadFromTemplate( currentDoc, context, true, &ok );
  if ( !ok )
  {
    return nullptr;
  }

  return newLayout.release();
}

void QgsLayout::initializeDefaults()
{
  // default to a A4 landscape page
  QgsLayoutItemPage *page = new QgsLayoutItemPage( this );
  page->setPageSize( QgsLayoutSize( 297, 210, QgsUnitTypes::LayoutMillimeters ) );
  mPageCollection->addPage( page );
  mUndoStack->stack()->clear();
}

void QgsLayout::clear()
{
  deleteAndRemoveMultiFrames();

  //delete all non paper items
  const QList<QGraphicsItem *> itemList = items();
  for ( QGraphicsItem *item : itemList )
  {
    QgsLayoutItem *cItem = dynamic_cast<QgsLayoutItem *>( item );
    QgsLayoutItemPage *pItem = dynamic_cast<QgsLayoutItemPage *>( item );
    if ( cItem && !pItem )
    {
      removeLayoutItemPrivate( cItem );
    }
  }
  mItemsModel->clear();

  mPageCollection->clear();
  mUndoStack->stack()->clear();
}

QgsProject *QgsLayout::project() const
{
  return mProject;
}

QgsLayoutModel *QgsLayout::itemsModel()
{
  return mItemsModel.get();
}

QList<QgsLayoutItem *> QgsLayout::selectedLayoutItems( const bool includeLockedItems )
{
  QList<QgsLayoutItem *> layoutItemList;

  const QList<QGraphicsItem *> graphicsItemList = selectedItems();
  for ( QGraphicsItem *item : graphicsItemList )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item );
    if ( layoutItem && ( includeLockedItems || !layoutItem->isLocked() ) )
    {
      layoutItemList.push_back( layoutItem );
    }
  }

  return layoutItemList;
}

void QgsLayout::setSelectedItem( QgsLayoutItem *item )
{
  whileBlocking( this )->deselectAll();
  if ( item )
  {
    item->setSelected( true );
  }
  emit selectedItemChanged( item );
}

void QgsLayout::deselectAll()
{
  //we can't use QGraphicsScene::clearSelection, as that emits no signals
  //and we don't know which items are being deselected
  //accordingly, we can't inform the layout model of selection changes
  //instead, do the clear selection manually...
  const QList<QGraphicsItem *> selectedItemList = selectedItems();
  for ( QGraphicsItem *item : selectedItemList )
  {
    if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
    {
      layoutItem->setSelected( false );
    }
  }
  emit selectedItemChanged( nullptr );
}

bool QgsLayout::raiseItem( QgsLayoutItem *item, bool deferUpdate )
{
  //model handles reordering items
  bool result = mItemsModel->reorderItemUp( item );
  if ( result && !deferUpdate )
  {
    //update all positions
    updateZValues();
    update();
  }
  return result;
}

bool QgsLayout::lowerItem( QgsLayoutItem *item, bool deferUpdate )
{
  //model handles reordering items
  bool result = mItemsModel->reorderItemDown( item );
  if ( result && !deferUpdate )
  {
    //update all positions
    updateZValues();
    update();
  }
  return result;
}

bool QgsLayout::moveItemToTop( QgsLayoutItem *item, bool deferUpdate )
{
  //model handles reordering items
  bool result = mItemsModel->reorderItemToTop( item );
  if ( result && !deferUpdate )
  {
    //update all positions
    updateZValues();
    update();
  }
  return result;
}

bool QgsLayout::moveItemToBottom( QgsLayoutItem *item, bool deferUpdate )
{
  //model handles reordering items
  bool result = mItemsModel->reorderItemToBottom( item );
  if ( result && !deferUpdate )
  {
    //update all positions
    updateZValues();
    update();
  }
  return result;
}

QgsLayoutItem *QgsLayout::itemByUuid( const QString &uuid, bool includeTemplateUuids ) const
{
  QList<QgsLayoutItem *> itemList;
  layoutItems( itemList );
  for ( QgsLayoutItem *item : qgis::as_const( itemList ) )
  {
    if ( item->uuid() == uuid )
      return item;
    else if ( includeTemplateUuids && item->mTemplateUuid == uuid )
      return item;
  }

  return nullptr;
}

QgsLayoutItem *QgsLayout::itemByTemplateUuid( const QString &uuid ) const
{
  QList<QgsLayoutItem *> itemList;
  layoutItems( itemList );
  for ( QgsLayoutItem *item : qgis::as_const( itemList ) )
  {
    if ( item->mTemplateUuid == uuid )
      return item;
  }

  return nullptr;
}

QgsLayoutItem *QgsLayout::itemById( const QString &id ) const
{
  const QList<QGraphicsItem *> itemList = items();
  for ( QGraphicsItem *item : itemList )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item );
    if ( layoutItem && layoutItem->id() == id )
    {
      return layoutItem;
    }
  }
  return nullptr;
}

QgsLayoutMultiFrame *QgsLayout::multiFrameByUuid( const QString &uuid, bool includeTemplateUuids ) const
{
  for ( QgsLayoutMultiFrame *mf : mMultiFrames )
  {
    if ( mf->uuid() == uuid )
      return mf;
    else if ( includeTemplateUuids && mf->mTemplateUuid == uuid )
      return mf;
  }

  return nullptr;
}

QgsLayoutItem *QgsLayout::layoutItemAt( QPointF position, const bool ignoreLocked ) const
{
  return layoutItemAt( position, nullptr, ignoreLocked );
}

QgsLayoutItem *QgsLayout::layoutItemAt( QPointF position, const QgsLayoutItem *belowItem, const bool ignoreLocked ) const
{
  //get a list of items which intersect the specified position, in descending z order
  const QList<QGraphicsItem *> itemList = items( position, Qt::IntersectsItemShape, Qt::DescendingOrder );

  bool foundBelowItem = false;
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    QgsLayoutItemPage *paperItem = dynamic_cast<QgsLayoutItemPage *>( layoutItem );
    if ( layoutItem && !paperItem )
    {
      // If we are not checking for a an item below a specified item, or if we've
      // already found that item, then we've found our target
      if ( ( ! belowItem || foundBelowItem ) && ( !ignoreLocked || !layoutItem->isLocked() ) )
      {
        return layoutItem;
      }
      else
      {
        if ( layoutItem == belowItem )
        {
          //Target item is next in list
          foundBelowItem = true;
        }
      }
    }
  }
  return nullptr;
}

double QgsLayout::convertToLayoutUnits( QgsLayoutMeasurement measurement ) const
{
  return mRenderContext->measurementConverter().convert( measurement, mUnits ).length();
}

QSizeF QgsLayout::convertToLayoutUnits( const QgsLayoutSize &size ) const
{
  return mRenderContext->measurementConverter().convert( size, mUnits ).toQSizeF();
}

QPointF QgsLayout::convertToLayoutUnits( const QgsLayoutPoint &point ) const
{
  return mRenderContext->measurementConverter().convert( point, mUnits ).toQPointF();
}

QgsLayoutMeasurement QgsLayout::convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mRenderContext->measurementConverter().convert( QgsLayoutMeasurement( length, mUnits ), unit );
}

QgsLayoutSize QgsLayout::convertFromLayoutUnits( QSizeF size, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mRenderContext->measurementConverter().convert( QgsLayoutSize( size.width(), size.height(), mUnits ), unit );
}

QgsLayoutPoint QgsLayout::convertFromLayoutUnits( QPointF point, const QgsUnitTypes::LayoutUnit unit ) const
{
  return mRenderContext->measurementConverter().convert( QgsLayoutPoint( point.x(), point.y(), mUnits ), unit );
}

QgsLayoutRenderContext &QgsLayout::renderContext()
{
  return *mRenderContext;
}

const QgsLayoutRenderContext &QgsLayout::renderContext() const
{
  return *mRenderContext;
}

QgsLayoutReportContext &QgsLayout::reportContext()
{
  return *mReportContext;
}

const QgsLayoutReportContext &QgsLayout::reportContext() const
{
  return *mReportContext;
}

void QgsLayout::reloadSettings()
{
  mGridSettings.loadFromSettings();
  mPageCollection->redraw();
}

QgsLayoutGuideCollection &QgsLayout::guides()
{
  return mPageCollection->guides();
}

const QgsLayoutGuideCollection &QgsLayout::guides() const
{
  return mPageCollection->guides();
}

QgsExpressionContext QgsLayout::createExpressionContext() const
{
  QgsExpressionContext context = QgsExpressionContext();
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( mProject ) );
  if ( mReportContext->layer() )
    context.appendScope( QgsExpressionContextUtils::layerScope( mReportContext->layer() ) );

  context.appendScope( QgsExpressionContextUtils::layoutScope( this ) );
  return context;
}

void QgsLayout::setCustomProperty( const QString &key, const QVariant &value )
{
  mCustomProperties.setValue( key, value );

  if ( key.startsWith( QLatin1String( "variable" ) ) )
    emit variablesChanged();
}

QVariant QgsLayout::customProperty( const QString &key, const QVariant &defaultValue ) const
{
  return mCustomProperties.value( key, defaultValue );
}

void QgsLayout::removeCustomProperty( const QString &key )
{
  mCustomProperties.remove( key );
}

QStringList QgsLayout::customProperties() const
{
  return mCustomProperties.keys();
}

QgsLayoutItemMap *QgsLayout::referenceMap() const
{
  // prefer explicitly set reference map
  if ( QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( itemByUuid( mWorldFileMapId ) ) )
    return map;

  // else try to find largest map
  QList< QgsLayoutItemMap * > maps;
  layoutItems( maps );
  QgsLayoutItemMap *largestMap = nullptr;
  double largestMapArea = 0;
  for ( QgsLayoutItemMap *map : qgis::as_const( maps ) )
  {
    double area = map->rect().width() * map->rect().height();
    if ( area > largestMapArea )
    {
      largestMapArea = area;
      largestMap = map;
    }
  }
  return largestMap;
}

void QgsLayout::setReferenceMap( QgsLayoutItemMap *map )
{
  mWorldFileMapId = map ? map->uuid() : QString();
  mProject->setDirty( true );
}

QgsLayoutPageCollection *QgsLayout::pageCollection()
{
  return mPageCollection.get();
}

const QgsLayoutPageCollection *QgsLayout::pageCollection() const
{
  return mPageCollection.get();
}

QRectF QgsLayout::layoutBounds( bool ignorePages, double margin ) const
{
  //start with an empty rectangle
  QRectF bounds;

  //add all layout items and pages which are in the layout
  Q_FOREACH ( const QGraphicsItem *item, items() )
  {
    const QgsLayoutItem *layoutItem = dynamic_cast<const QgsLayoutItem *>( item );
    if ( !layoutItem )
      continue;

    bool isPage = layoutItem->type() == QgsLayoutItemRegistry::LayoutPage;
    if ( !isPage || !ignorePages )
    {
      //expand bounds with current item's bounds
      QRectF itemBounds;
      if ( isPage )
      {
        // for pages we only consider the item's rect - not the bounding rect
        // as the bounding rect contains extra padding
        itemBounds = layoutItem->mapToScene( layoutItem->rect() ).boundingRect();
      }
      else
        itemBounds = item->sceneBoundingRect();

      if ( bounds.isValid() )
        bounds = bounds.united( itemBounds );
      else
        bounds = itemBounds;
    }
  }

  if ( bounds.isValid() && margin > 0.0 )
  {
    //finally, expand bounds out by specified margin of page size
    double maxWidth = mPageCollection->maximumPageWidth();
    bounds.adjust( -maxWidth * margin, -maxWidth * margin, maxWidth * margin, maxWidth * margin );
  }

  return bounds;

}

QRectF QgsLayout::pageItemBounds( int page, bool visibleOnly ) const
{
  //start with an empty rectangle
  QRectF bounds;

  //add all QgsLayoutItems on page
  const QList<QGraphicsItem *> itemList = items();
  for ( QGraphicsItem *item : itemList )
  {
    const QgsLayoutItem *layoutItem = dynamic_cast<const QgsLayoutItem *>( item );
    if ( layoutItem && layoutItem->type() != QgsLayoutItemRegistry::LayoutPage && layoutItem->page() == page )
    {
      if ( visibleOnly && !layoutItem->isVisible() )
        continue;

      //expand bounds with current item's bounds
      if ( bounds.isValid() )
        bounds = bounds.united( item->sceneBoundingRect() );
      else
        bounds = item->sceneBoundingRect();
    }
  }

  return bounds;
}

void QgsLayout::addLayoutItem( QgsLayoutItem *item )
{
  addLayoutItemPrivate( item );
  QString undoText;
  if ( QgsLayoutItemAbstractMetadata *metadata = QgsApplication::layoutItemRegistry()->itemMetadata( item->type() ) )
  {
    undoText = tr( "Create %1" ).arg( metadata->visibleName() );
  }
  else
  {
    undoText = tr( "Create Item" );
  }
  if ( !mUndoStack->isBlocked() )
    mUndoStack->push( new QgsLayoutItemAddItemCommand( item, undoText ) );
}

void QgsLayout::removeLayoutItem( QgsLayoutItem *item )
{
  std::unique_ptr< QgsLayoutItemDeleteUndoCommand > deleteCommand;
  if ( !mUndoStack->isBlocked() )
  {
    mUndoStack->beginMacro( tr( "Delete Items" ) );
    deleteCommand.reset( new QgsLayoutItemDeleteUndoCommand( item, tr( "Delete Item" ) ) );
  }
  removeLayoutItemPrivate( item );
  if ( deleteCommand )
  {
    mUndoStack->push( deleteCommand.release() );
    mUndoStack->endMacro();
  }
}

void QgsLayout::addMultiFrame( QgsLayoutMultiFrame *multiFrame )
{
  if ( !multiFrame )
    return;

  if ( !mMultiFrames.contains( multiFrame ) )
    mMultiFrames << multiFrame;
}

void QgsLayout::removeMultiFrame( QgsLayoutMultiFrame *multiFrame )
{
  mMultiFrames.removeAll( multiFrame );
}

QList<QgsLayoutMultiFrame *> QgsLayout::multiFrames() const
{
  return mMultiFrames;
}

bool QgsLayout::saveAsTemplate( const QString &path, const QgsReadWriteContext &context ) const
{
  QFile templateFile( path );
  if ( !templateFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return false;
  }

  QDomDocument saveDocument;
  QDomElement elem = writeXml( saveDocument, context );
  saveDocument.appendChild( elem );

  if ( templateFile.write( saveDocument.toByteArray() ) == -1 )
    return false;

  return true;
}

QList< QgsLayoutItem * > QgsLayout::loadFromTemplate( const QDomDocument &document, const QgsReadWriteContext &context, bool clearExisting, bool *ok )
{
  if ( ok )
    *ok = false;

  QList< QgsLayoutItem * > result;

  if ( clearExisting )
  {
    clear();
  }

  QDomDocument doc;

  // If this is a 2.x composition template, convert it to a layout template
  if ( QgsCompositionConverter::isCompositionTemplate( document ) )
  {
    doc = QgsCompositionConverter::convertCompositionTemplate( document, mProject );
  }
  else
  {
    doc = document;
  }

  // remove all uuid attributes since we don't want duplicates UUIDS
  QDomNodeList itemsNodes = doc.elementsByTagName( QStringLiteral( "LayoutItem" ) );
  for ( int i = 0; i < itemsNodes.count(); ++i )
  {
    QDomNode itemNode = itemsNodes.at( i );
    if ( itemNode.isElement() )
    {
      itemNode.toElement().removeAttribute( QStringLiteral( "uuid" ) );
    }
  }
  QDomNodeList multiFrameNodes = doc.elementsByTagName( QStringLiteral( "LayoutMultiFrame" ) );
  for ( int i = 0; i < multiFrameNodes.count(); ++i )
  {
    QDomNode multiFrameNode = multiFrameNodes.at( i );
    if ( multiFrameNode.isElement() )
    {
      multiFrameNode.toElement().removeAttribute( QStringLiteral( "uuid" ) );
      QDomNodeList frameNodes = multiFrameNode.toElement().elementsByTagName( QStringLiteral( "childFrame" ) );
      QDomNode itemNode = frameNodes.at( i );
      if ( itemNode.isElement() )
      {
        itemNode.toElement().removeAttribute( QStringLiteral( "uuid" ) );
      }
    }
  }

  //read general settings
  if ( clearExisting )
  {
    QDomElement layoutElem = doc.documentElement();
    if ( layoutElem.isNull() )
    {
      return result;
    }

    bool loadOk = readXml( layoutElem, doc, context );
    if ( !loadOk )
    {
      return result;
    }
    layoutItems( result );
  }
  else
  {
    result = addItemsFromXml( doc.documentElement(), doc, context );
  }

  if ( ok )
    *ok = true;

  return result;
}

QgsLayoutUndoStack *QgsLayout::undoStack()
{
  return mUndoStack.get();
}

const QgsLayoutUndoStack *QgsLayout::undoStack() const
{
  return mUndoStack.get();
}

///@cond PRIVATE
class QgsLayoutUndoCommand: public QgsAbstractLayoutUndoCommand
{
  public:

    QgsLayoutUndoCommand( QgsLayout *layout, const QString &text, int id, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr )
      : QgsAbstractLayoutUndoCommand( text, id, parent )
      , mLayout( layout )
    {}

  protected:

    void saveState( QDomDocument &stateDoc ) const override
    {
      stateDoc.clear();
      QDomElement documentElement = stateDoc.createElement( QStringLiteral( "UndoState" ) );
      mLayout->writeXmlLayoutSettings( documentElement, stateDoc, QgsReadWriteContext() );
      stateDoc.appendChild( documentElement );
    }

    void restoreState( QDomDocument &stateDoc ) override
    {
      if ( !mLayout )
      {
        return;
      }

      mLayout->readXmlLayoutSettings( stateDoc.documentElement(), stateDoc, QgsReadWriteContext() );
      mLayout->project()->setDirty( true );
    }

  private:

    QgsLayout *mLayout = nullptr;
};
///@endcond

QgsAbstractLayoutUndoCommand *QgsLayout::createCommand( const QString &text, int id, QUndoCommand *parent )
{
  return new QgsLayoutUndoCommand( this, text, id, parent );
}

QgsLayoutItemGroup *QgsLayout::groupItems( const QList<QgsLayoutItem *> &items )
{
  if ( items.size() < 2 )
  {
    //not enough items for a group
    return nullptr;
  }

  mUndoStack->beginMacro( tr( "Group Items" ) );
  std::unique_ptr< QgsLayoutItemGroup > itemGroup( new QgsLayoutItemGroup( this ) );
  for ( QgsLayoutItem *item : items )
  {
    itemGroup->addItem( item );
  }
  QgsLayoutItemGroup *returnGroup = itemGroup.get();
  addLayoutItem( itemGroup.release() );

  std::unique_ptr< QgsLayoutItemGroupUndoCommand > c( new QgsLayoutItemGroupUndoCommand( QgsLayoutItemGroupUndoCommand::Grouped, returnGroup, this, tr( "Group Items" ) ) );
  mUndoStack->push( c.release() );
  mProject->setDirty( true );

  mUndoStack->endMacro();

  return returnGroup;
}

QList<QgsLayoutItem *> QgsLayout::ungroupItems( QgsLayoutItemGroup *group )
{
  QList<QgsLayoutItem *> ungroupedItems;
  if ( !group )
  {
    return ungroupedItems;
  }

  mUndoStack->beginMacro( tr( "Ungroup Items" ) );
  // Call this before removing group items so it can keep note
  // of contents
  std::unique_ptr< QgsLayoutItemGroupUndoCommand > c( new QgsLayoutItemGroupUndoCommand( QgsLayoutItemGroupUndoCommand::Ungrouped, group, this, tr( "Ungroup Items" ) ) );
  mUndoStack->push( c.release() );

  mProject->setDirty( true );

  ungroupedItems = group->items();
  group->removeItems();

  removeLayoutItem( group );
  mUndoStack->endMacro();

  return ungroupedItems;
}

void QgsLayout::refresh()
{
  mUndoStack->blockCommands( true );
  mPageCollection->beginPageSizeChange();
  emit refreshed();
  mPageCollection->reflow();
  mPageCollection->endPageSizeChange();
  mUndoStack->blockCommands( false );
  update();
}

void QgsLayout::writeXmlLayoutSettings( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  mCustomProperties.writeXml( element, document );
  element.setAttribute( QStringLiteral( "units" ), QgsUnitTypes::encodeUnit( mUnits ) );
  element.setAttribute( QStringLiteral( "worldFileMap" ), mWorldFileMapId );
  element.setAttribute( QStringLiteral( "printResolution" ), mRenderContext->dpi() );
}

QDomElement QgsLayout::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "Layout" ) );
  auto save = [&]( const QgsLayoutSerializableObject * object )->bool
  {
    return object->writeXml( element, document, context );
  };
  save( &mSnapper );
  save( &mGridSettings );
  save( mPageCollection.get() );

  //save items except paper items and frame items (they are saved with the corresponding multiframe)
  const QList<QGraphicsItem *> itemList = items();
  for ( const QGraphicsItem *graphicsItem : itemList )
  {
    if ( const QgsLayoutItem *item = dynamic_cast< const QgsLayoutItem *>( graphicsItem ) )
    {
      if ( item->type() == QgsLayoutItemRegistry::LayoutPage )
        continue;

      item->writeXml( element, document, context );
    }
  }

  //save multiframes
  for ( QgsLayoutMultiFrame *mf : mMultiFrames )
  {
    mf->writeXml( element, document, context );
  }

  writeXmlLayoutSettings( element, document, context );
  return element;
}

bool QgsLayout::readXmlLayoutSettings( const QDomElement &layoutElement, const QDomDocument &, const QgsReadWriteContext & )
{
  mCustomProperties.readXml( layoutElement );
  setUnits( QgsUnitTypes::decodeLayoutUnit( layoutElement.attribute( QStringLiteral( "units" ) ) ) );
  mWorldFileMapId = layoutElement.attribute( QStringLiteral( "worldFileMap" ) );
  mRenderContext->setDpi( layoutElement.attribute( QStringLiteral( "printResolution" ), QStringLiteral( "300" ) ).toDouble() );
  emit changed();

  return true;
}

void QgsLayout::addLayoutItemPrivate( QgsLayoutItem *item )
{
  addItem( item );
  updateBounds();
  mItemsModel->rebuildZList();
}

void QgsLayout::removeLayoutItemPrivate( QgsLayoutItem *item )
{
  mItemsModel->setItemRemoved( item );
  // small chance that item is still in a scene - the model may have
  // rejected the removal for some reason. This is probably not necessary,
  // but can't hurt...
  if ( item->scene() )
    removeItem( item );
#if 0 //TODO
  emit itemRemoved( item );
#endif
  item->cleanup();
  item->deleteLater();
}

void QgsLayout::deleteAndRemoveMultiFrames()
{
  qDeleteAll( mMultiFrames );
  mMultiFrames.clear();
}

QPointF QgsLayout::minPointFromXml( const QDomElement &elem ) const
{
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  const QDomNodeList itemList = elem.elementsByTagName( QStringLiteral( "LayoutItem" ) );
  bool found = false;
  for ( int i = 0; i < itemList.size(); ++i )
  {
    const QDomElement currentItemElem = itemList.at( i ).toElement();

    QgsLayoutPoint pos = QgsLayoutPoint::decodePoint( currentItemElem.attribute( QStringLiteral( "position" ) ) );
    QPointF layoutPoint = convertToLayoutUnits( pos );

    minX = std::min( minX, layoutPoint.x() );
    minY = std::min( minY, layoutPoint.y() );
    found = true;
  }
  return found ? QPointF( minX, minY ) : QPointF( 0, 0 );
}

void QgsLayout::updateZValues( const bool addUndoCommands )
{
  int counter = mItemsModel->zOrderListSize();
  const QList<QgsLayoutItem *> zOrderList = mItemsModel->zOrderList();

  if ( addUndoCommands )
  {
    mUndoStack->beginMacro( tr( "Change Item Stacking" ) );
  }
  for ( QgsLayoutItem *currentItem : zOrderList )
  {
    if ( currentItem )
    {
      if ( addUndoCommands )
      {
        mUndoStack->beginCommand( currentItem, QString() );
      }
      currentItem->setZValue( counter );
      if ( addUndoCommands )
      {
        mUndoStack->endCommand();
      }
    }
    --counter;
  }
  if ( addUndoCommands )
  {
    mUndoStack->endMacro();
  }
}

bool QgsLayout::readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context )
{
  if ( layoutElement.nodeName() != QStringLiteral( "Layout" ) )
  {
    return false;
  }

  auto restore = [&]( QgsLayoutSerializableObject * object )->bool
  {
    return object->readXml( layoutElement, document, context );
  };

  blockSignals( true ); // defer changed signal to end
  readXmlLayoutSettings( layoutElement, document, context );
  blockSignals( false );

  restore( mPageCollection.get() );
  restore( &mSnapper );
  restore( &mGridSettings );
  addItemsFromXml( layoutElement, document, context );

  emit changed();

  return true;
}

QList< QgsLayoutItem * > QgsLayout::addItemsFromXml( const QDomElement &parentElement, const QDomDocument &document, const QgsReadWriteContext &context, QPointF *position, bool pasteInPlace )
{
  QList< QgsLayoutItem * > newItems;
  QList< QgsLayoutMultiFrame * > newMultiFrames;

  //if we are adding items to a layout which already contains items, we need to make sure
  //these items are placed at the top of the layout and that zValues are not duplicated
  //so, calculate an offset which needs to be added to the zValue of created items
  int zOrderOffset = mItemsModel->zOrderListSize();

  QPointF pasteShiftPos;
  int pageNumber = -1;
  if ( position )
  {
    //If we are placing items relative to a certain point, then calculate how much we need
    //to shift the items by so that they are placed at this point
    //First, calculate the minimum position from the xml
    QPointF minItemPos = minPointFromXml( parentElement );
    //next, calculate how much each item needs to be shifted from its original position
    //so that it's placed at the correct relative position
    pasteShiftPos = *position - minItemPos;
    if ( pasteInPlace )
    {
      pageNumber = mPageCollection->pageNumberForPoint( *position );
    }
  }
  // multiframes

  //TODO - fix this. pasting multiframe frame items has no effect
  const QDomNodeList multiFrameList = parentElement.elementsByTagName( QStringLiteral( "LayoutMultiFrame" ) );
  for ( int i = 0; i < multiFrameList.size(); ++i )
  {
    const QDomElement multiFrameElem = multiFrameList.at( i ).toElement();
    const int itemType = multiFrameElem.attribute( QStringLiteral( "type" ) ).toInt();
    std::unique_ptr< QgsLayoutMultiFrame > mf( QgsApplication::layoutItemRegistry()->createMultiFrame( itemType, this ) );
    if ( !mf )
    {
      // e.g. plugin based item which is no longer available
      continue;
    }
    mf->readXml( multiFrameElem, document, context );

#if 0 //TODO?
    mf->setCreateUndoCommands( true );
#endif

    QgsLayoutMultiFrame *m = mf.get();
    this->addMultiFrame( mf.release() );

    //offset z values for frames
    //TODO - fix this after fixing multiframe item paste
    /*for ( int frameIdx = 0; frameIdx < mf->frameCount(); ++frameIdx )
    {
      QgsLayoutItemFrame * frame = mf->frame( frameIdx );
      frame->setZValue( frame->zValue() + zOrderOffset );

      // also need to shift frames according to position/pasteInPlacePt
    }*/
    newMultiFrames << m;
  }

  const QDomNodeList layoutItemList = parentElement.childNodes();
  for ( int i = 0; i < layoutItemList.size(); ++i )
  {
    const QDomElement currentItemElem = layoutItemList.at( i ).toElement();
    if ( currentItemElem.nodeName() != QStringLiteral( "LayoutItem" ) )
      continue;

    const int itemType = currentItemElem.attribute( QStringLiteral( "type" ) ).toInt();
    std::unique_ptr< QgsLayoutItem > item( QgsApplication::layoutItemRegistry()->createItem( itemType, this ) );
    if ( !item )
    {
      // e.g. plugin based item which is no longer available
      continue;
    }

    item->readXml( currentItemElem, document, context );
    if ( position )
    {
      if ( pasteInPlace )
      {
        QgsLayoutPoint posOnPage = QgsLayoutPoint::decodePoint( currentItemElem.attribute( QStringLiteral( "positionOnPage" ) ) );
        item->attemptMove( posOnPage, true, false, pageNumber );
      }
      else
      {
        item->attemptMoveBy( pasteShiftPos.x(), pasteShiftPos.y() );
      }
    }

    QgsLayoutItem *layoutItem = item.get();
    addLayoutItem( item.release() );
    layoutItem->setZValue( layoutItem->zValue() + zOrderOffset );
    newItems << layoutItem;
  }

  // we now allow items to "post-process", e.g. if they need to setup connections
  // to other items in the layout, which may not have existed at the time the
  // item's state was restored. E.g. a scalebar may have been restored before the map
  // it is linked to
  for ( QgsLayoutItem *item : qgis::as_const( newItems ) )
  {
    item->finalizeRestoreFromXml();
  }
  for ( QgsLayoutMultiFrame *mf : qgis::as_const( newMultiFrames ) )
  {
    mf->finalizeRestoreFromXml();
  }

  for ( QgsLayoutItem *item : qgis::as_const( newItems ) )
  {
    item->mTemplateUuid.clear();
  }
  for ( QgsLayoutMultiFrame *mf : qgis::as_const( newMultiFrames ) )
  {
    mf->mTemplateUuid.clear();
  }

  //Since this function adds items in an order which isn't the z-order, and each item is added to end of
  //z order list in turn, it will now be inconsistent with the actual order of items in the scene.
  //Make sure z order list matches the actual order of items in the scene.
  mItemsModel->rebuildZList();

  return newItems;
}

void QgsLayout::updateBounds()
{
  setSceneRect( layoutBounds( false, 0.05 ) );
}
