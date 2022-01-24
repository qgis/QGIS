/***************************************************************************
                              qgslayoutpagecollection.cpp
                             ----------------------------
    begin                : July 2017
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

#include "qgslayoutpagecollection.h"
#include "qgslayout.h"
#include "qgsreadwritecontext.h"
#include "qgsproject.h"
#include "qgslayoutitemundocommand.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutframe.h"
#include "qgslayoutundostack.h"
#include "qgsfillsymbol.h"

QgsLayoutPageCollection::QgsLayoutPageCollection( QgsLayout *layout )
  : QObject( layout )
  , mLayout( layout )
  , mGuideCollection( new QgsLayoutGuideCollection( layout, this ) )
{
  createDefaultPageStyleSymbol();
}

QgsLayoutPageCollection::~QgsLayoutPageCollection()
{
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    mLayout->removeItem( page );
    page->deleteLater();
  }
}

void QgsLayoutPageCollection::setPageStyleSymbol( QgsFillSymbol *symbol )
{
  if ( !symbol )
    return;

  mPageStyleSymbol.reset( static_cast<QgsFillSymbol *>( symbol->clone() ) );

  for ( QgsLayoutItemPage *page : std::as_const( mPages ) )
  {
    page->setPageStyleSymbol( symbol->clone() );
    page->update();
  }
}

const QgsFillSymbol *QgsLayoutPageCollection::pageStyleSymbol() const
{
  return mPageStyleSymbol.get();
}

void QgsLayoutPageCollection::beginPageSizeChange()
{
  mPreviousItemPositions.clear();
  QList< QgsLayoutItem * > items;
  mLayout->layoutItems( items );

  for ( QgsLayoutItem *item : std::as_const( items ) )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutPage )
      continue;

    mPreviousItemPositions.insert( item->uuid(), qMakePair( item->page(), item->pagePositionWithUnits() ) );
  }
}

void QgsLayoutPageCollection::endPageSizeChange()
{
  for ( auto it = mPreviousItemPositions.constBegin(); it != mPreviousItemPositions.constEnd(); ++it )
  {
    const QString key { it.key() };
    if ( QgsLayoutItem *item = mLayout->itemByUuid( key ) )
    {
      if ( !mBlockUndoCommands )
        item->beginCommand( QString() );

      item->attemptMove( it.value().second, true, false, it.value().first );

      // Item might have been deleted
      if ( mLayout->itemByUuid( key ) )
      {
        if ( !mBlockUndoCommands )
          item->endCommand();
      }
      else
      {
        item->cancelCommand();
      }
    }
  }
  mPreviousItemPositions.clear();
}

void QgsLayoutPageCollection::reflow()
{
  double currentY = 0;
  QgsLayoutPoint p( 0, 0, mLayout->units() );
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    page->attemptMove( p );
    currentY += mLayout->convertToLayoutUnits( page->pageSize() ).height() + spaceBetweenPages();
    p.setY( currentY );
  }
  mLayout->guides().update();
  mLayout->updateBounds();
  emit changed();
}

double QgsLayoutPageCollection::maximumPageWidth() const
{
  double maxWidth = 0;
  for ( QgsLayoutItemPage *page : mPages )
  {
    maxWidth = std::max( maxWidth, mLayout->convertToLayoutUnits( page->pageSize() ).width() );
  }
  return maxWidth;
}

QSizeF QgsLayoutPageCollection::maximumPageSize() const
{
  double maxArea = 0;
  QSizeF maxSize;
  for ( QgsLayoutItemPage *page : mPages )
  {
    QSizeF pageSize = mLayout->convertToLayoutUnits( page->pageSize() );
    double area = pageSize.width() * pageSize.height();
    if ( area > maxArea )
    {
      maxArea = area;
      maxSize = pageSize;
    }
  }
  return maxSize;
}

bool QgsLayoutPageCollection::hasUniformPageSizes() const
{
  QSizeF size;
  for ( QgsLayoutItemPage *page : mPages )
  {
    QSizeF pageSize = mLayout->convertToLayoutUnits( page->pageSize() );
    if ( !size.isValid() )
      size = pageSize;
    else
    {
      if ( !qgsDoubleNear( pageSize.width(), size.width(), 0.01 )
           || !qgsDoubleNear( pageSize.height(), size.height(), 0.01 ) )
        return false;
    }
  }
  return true;
}

int QgsLayoutPageCollection::pageNumberForPoint( QPointF point ) const
{
  int pageNumber = 0;
  double startNextPageY = 0;
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    startNextPageY += page->rect().height() + spaceBetweenPages();
    if ( startNextPageY > point.y() )
      break;
    pageNumber++;
  }

  if ( pageNumber > mPages.count() - 1 )
    pageNumber = mPages.count() - 1;
  return pageNumber;
}

int QgsLayoutPageCollection::predictPageNumberForPoint( QPointF point ) const
{
  if ( mPages.empty() )
    return 0;

  int pageNumber = 0;
  double startNextPageY = 0;
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    startNextPageY += page->rect().height() + spaceBetweenPages();
    if ( startNextPageY >= point.y() )
      break;
    pageNumber++;
  }

  if ( startNextPageY >= point.y() )
  {
    // found an existing page
    return pageNumber;
  }

  double lastPageHeight = mPages.last()->rect().height();
  while ( startNextPageY < point.y() )
  {
    startNextPageY += lastPageHeight + spaceBetweenPages();
    if ( startNextPageY >= point.y() )
      break;
    pageNumber++;
  }

  return pageNumber;
}

QgsLayoutItemPage *QgsLayoutPageCollection::pageAtPoint( QPointF point ) const
{
  const QList< QGraphicsItem * > items = mLayout->items( point );
  for ( QGraphicsItem *item : items )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutPage )
    {
      QgsLayoutItemPage *page = static_cast< QgsLayoutItemPage * >( item );
      if ( page->mapToScene( page->rect() ).boundingRect().contains( point ) )
        return page;
    }
  }
  return nullptr;
}

QPointF QgsLayoutPageCollection::pagePositionToLayoutPosition( int page, const QgsLayoutPoint &position ) const
{
  QPointF layoutUnitsPos = mLayout->convertToLayoutUnits( position );
  if ( page > 0 && page < mPages.count() )
  {
    layoutUnitsPos.ry() += mPages.at( page )->pos().y();
  }
  return layoutUnitsPos;
}

QgsLayoutPoint QgsLayoutPageCollection::pagePositionToAbsolute( int page, const QgsLayoutPoint &position ) const
{
  double vDelta = 0.0;
  if ( page > 0 && page < mPages.count() )
  {
    vDelta = mLayout->convertFromLayoutUnits( mPages.at( page )->pos().y(), position.units() ).length();
  }

  return QgsLayoutPoint( position.x(), position.y() + vDelta, position.units() );
}

QPointF QgsLayoutPageCollection::positionOnPage( QPointF position ) const
{
  double startCurrentPageY = 0;
  double startNextPageY = 0;
  int pageNumber = 0;
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    startCurrentPageY = startNextPageY;
    startNextPageY += page->rect().height() + spaceBetweenPages();
    if ( startNextPageY > position.y() )
      break;
    pageNumber++;
  }

  double y;
  if ( pageNumber == mPages.size() )
  {
    //y coordinate is greater then the end of the last page, so return distance between
    //top of last page and y coordinate
    y = position.y() - ( startNextPageY - spaceBetweenPages() );
  }
  else
  {
    //y coordinate is less then the end of the last page
    y = position.y() - startCurrentPageY;
  }
  return QPointF( position.x(), y );
}

double QgsLayoutPageCollection::spaceBetweenPages() const
{
  return mLayout->convertToLayoutUnits( QgsLayoutMeasurement( 10 ) );
}

double QgsLayoutPageCollection::pageShadowWidth() const
{
  return spaceBetweenPages() / 2;
}

void QgsLayoutPageCollection::resizeToContents( const QgsMargins &margins, QgsUnitTypes::LayoutUnit marginUnits )
{
  //calculate current bounds
  QRectF bounds = mLayout->layoutBounds( true, 0.0 );
  if ( bounds.isEmpty() )
    return;

  if ( !mBlockUndoCommands )
    mLayout->undoStack()->beginCommand( this, tr( "Resize to Contents" ) );

  for ( int page = mPages.count() - 1; page > 0; page-- )
  {
    deletePage( page );
  }

  if ( mPages.empty() )
  {
    std::unique_ptr< QgsLayoutItemPage > page = std::make_unique< QgsLayoutItemPage >( mLayout );
    addPage( page.release() );
  }

  QgsLayoutItemPage *page = mPages.at( 0 );

  double marginLeft = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( margins.left(), marginUnits ) );
  double marginTop = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( margins.top(), marginUnits ) );
  double marginBottom = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( margins.bottom(), marginUnits ) );
  double marginRight = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( margins.right(), marginUnits ) );

  bounds.setWidth( bounds.width() + marginLeft + marginRight );
  bounds.setHeight( bounds.height() + marginTop + marginBottom );

  QgsLayoutSize newPageSize = mLayout->convertFromLayoutUnits( bounds.size(), mLayout->units() );
  page->setPageSize( newPageSize );

  reflow();

  //also move all items so that top-left of bounds is at marginLeft, marginTop
  double diffX = marginLeft - bounds.left();
  double diffY = marginTop - bounds.top();

  const QList<QGraphicsItem *> itemList = mLayout->items();
  for ( QGraphicsItem *item : itemList )
  {
    if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
    {
      QgsLayoutItemPage *pageItem = dynamic_cast<QgsLayoutItemPage *>( layoutItem );
      if ( !pageItem )
      {
        layoutItem->beginCommand( tr( "Move Item" ) );
        layoutItem->attemptMoveBy( diffX, diffY );
        layoutItem->endCommand();
      }
    }
  }

  //also move guides
  mLayout->undoStack()->beginCommand( &mLayout->guides(), tr( "Move Guides" ) );
  const QList< QgsLayoutGuide * > verticalGuides = mLayout->guides().guides( Qt::Vertical );
  for ( QgsLayoutGuide *guide : verticalGuides )
  {
    guide->setLayoutPosition( guide->layoutPosition() + diffX );
  }
  const QList< QgsLayoutGuide * > horizontalGuides = mLayout->guides().guides( Qt::Horizontal );
  for ( QgsLayoutGuide *guide : horizontalGuides )
  {
    guide->setLayoutPosition( guide->layoutPosition() + diffY );
  }
  mLayout->undoStack()->endCommand();

  if ( !mBlockUndoCommands )
    mLayout->undoStack()->endCommand();
}

bool QgsLayoutPageCollection::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "PageCollection" ) );

  QDomElement pageStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mPageStyleSymbol.get(), document, context );
  element.appendChild( pageStyleElem );

  for ( const QgsLayoutItemPage *page : mPages )
  {
    page->writeXml( element, document, context );
  }

  mGuideCollection->writeXml( element, document, context );

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutPageCollection::readXml( const QDomElement &e, const QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = e;
  if ( element.nodeName() != QLatin1String( "PageCollection" ) )
  {
    element = element.firstChildElement( QStringLiteral( "PageCollection" ) );
  }

  if ( element.nodeName() != QLatin1String( "PageCollection" ) )
  {
    return false;
  }

  mBlockUndoCommands = true;

  int i = 0;
  for ( QgsLayoutItemPage *page : std::as_const( mPages ) )
  {
    emit pageAboutToBeRemoved( i );
    mLayout->removeItem( page );
    page->deleteLater();
    ++i;
  }
  mPages.clear();

  QDomElement pageStyleSymbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !pageStyleSymbolElem.isNull() )
  {
    mPageStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( pageStyleSymbolElem, context ) );
  }

  QDomNodeList pageList = element.elementsByTagName( QStringLiteral( "LayoutItem" ) );
  for ( int i = 0; i < pageList.size(); ++i )
  {
    QDomElement pageElement = pageList.at( i ).toElement();
    std::unique_ptr< QgsLayoutItemPage > page( new QgsLayoutItemPage( mLayout ) );
    if ( mPageStyleSymbol )
      page->setPageStyleSymbol( mPageStyleSymbol->clone() );
    page->readXml( pageElement, document, context );
    page->finalizeRestoreFromXml();
    mPages.append( page.get() );
    mLayout->addItem( page.release() );
  }

  reflow();

  mGuideCollection->readXml( element, document, context );

  mBlockUndoCommands = false;
  return true;
}

QgsLayoutGuideCollection &QgsLayoutPageCollection::guides()
{
  return *mGuideCollection;
}

const QgsLayoutGuideCollection &QgsLayoutPageCollection::guides() const
{
  return *mGuideCollection;
}

void QgsLayoutPageCollection::redraw()
{
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    page->redraw();
  }
}

QgsLayout *QgsLayoutPageCollection::layout()
{
  return mLayout;
}

QList<QgsLayoutItemPage *> QgsLayoutPageCollection::pages()
{
  return mPages;
}

int QgsLayoutPageCollection::pageCount() const
{
  return mPages.count();
}

QgsLayoutItemPage *QgsLayoutPageCollection::page( int pageNumber )
{
  return mPages.value( pageNumber );
}

const QgsLayoutItemPage *QgsLayoutPageCollection::page( int pageNumber ) const
{
  return mPages.value( pageNumber );
}

int QgsLayoutPageCollection::pageNumber( QgsLayoutItemPage *page ) const
{
  return mPages.indexOf( page );
}

QList<QgsLayoutItemPage *> QgsLayoutPageCollection::visiblePages( const QRectF &region ) const
{
  QList<QgsLayoutItemPage *> pages;
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    if ( page->mapToScene( page->rect() ).boundingRect().intersects( region ) )
      pages << page;
  }
  return pages;
}

QList<int> QgsLayoutPageCollection::visiblePageNumbers( const QRectF &region ) const
{
  QList< int > pages;
  int p = 0;
  const auto constMPages = mPages;
  for ( QgsLayoutItemPage *page : constMPages )
  {
    if ( page->mapToScene( page->rect() ).boundingRect().intersects( region ) )
      pages << p;
    p++;
  }
  return pages;
}

bool QgsLayoutPageCollection::pageIsEmpty( int page ) const
{
  //get all items on page
  const QList<QgsLayoutItem *> items = mLayout->pageCollection()->itemsOnPage( page );

  //loop through and check for non-paper items
  for ( QgsLayoutItem *item : items )
  {
    //is item a paper item?
    if ( item->type() != QgsLayoutItemRegistry::LayoutPage )
    {
      //item is not a paper item, so we have other items on the page
      return false;
    }
  }
  //no non-paper items
  return true;
}

QList<QgsLayoutItem *> QgsLayoutPageCollection::itemsOnPage( int page ) const
{
  QList<QgsLayoutItem *> itemList;
  const QList<QGraphicsItem *> graphicsItemList = mLayout->items();
  itemList.reserve( graphicsItemList.size() );
  for ( QGraphicsItem *graphicsItem : graphicsItemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    if ( item && item->page() == page )
    {
      itemList.push_back( item );
    }
  }
  return itemList;
}

bool QgsLayoutPageCollection::shouldExportPage( int page ) const
{
  if ( page >= mPages.count() || page < 0 )
  {
    //page number out of range, of course we shouldn't export it - stop smoking crack!
    return false;
  }

  QgsLayoutItemPage *pageItem = mPages.at( page );
  if ( !pageItem->shouldDrawItem() )
    return false;

  //check all frame items on page
  QList<QgsLayoutFrame *> frames;
  itemsOnPage( frames, page );
  for ( QgsLayoutFrame *frame : std::as_const( frames ) )
  {
    if ( frame->hidePageIfEmpty() && frame->isEmpty() )
    {
      //frame is set to hide page if empty, and frame is empty, so we don't want to export this page
      return false;
    }
  }
  return true;
}

void QgsLayoutPageCollection::addPage( QgsLayoutItemPage *page )
{
  if ( !mBlockUndoCommands )
    mLayout->undoStack()->beginCommand( this, tr( "Add Page" ) );
  mPages.append( page );
  mLayout->addItem( page );
  reflow();
  if ( !mBlockUndoCommands )
    mLayout->undoStack()->endCommand();
}

QgsLayoutItemPage *QgsLayoutPageCollection::extendByNewPage()
{
  if ( mPages.empty() )
    return nullptr;

  QgsLayoutItemPage *lastPage = mPages.at( mPages.count() - 1 );
  std::unique_ptr< QgsLayoutItemPage > newPage = std::make_unique< QgsLayoutItemPage >( mLayout );
  newPage->attemptResize( lastPage->sizeWithUnits() );
  addPage( newPage.release() );
  return mPages.at( mPages.count() - 1 );
}

void QgsLayoutPageCollection::insertPage( QgsLayoutItemPage *page, int beforePage )
{
  if ( !mBlockUndoCommands )
  {
    mLayout->undoStack()->beginMacro( tr( "Add Page" ) );
    mLayout->undoStack()->beginCommand( this, tr( "Add Page" ) );
  }

  if ( beforePage < 0 )
    beforePage = 0;

  beginPageSizeChange();
  if ( beforePage >= mPages.count() )
  {
    mPages.append( page );
  }
  else
  {
    mPages.insert( beforePage, page );
  }
  mLayout->addItem( page );
  reflow();

  // bump up stored page numbers to account
  for ( auto it = mPreviousItemPositions.begin(); it != mPreviousItemPositions.end(); ++it ) // clazy:exclude=detaching-member
  {
    if ( it.value().first < beforePage )
      continue;

    it.value().first = it.value().first + 1;
  }

  endPageSizeChange();
  if ( ! mBlockUndoCommands )
  {
    mLayout->undoStack()->endCommand();
    mLayout->undoStack()->endMacro();
  }
}

void QgsLayoutPageCollection::deletePage( int pageNumber )
{
  if ( pageNumber < 0 || pageNumber >= mPages.count() )
    return;

  if ( !mBlockUndoCommands )
  {
    mLayout->undoStack()->beginMacro( tr( "Remove Page" ) );
    mLayout->undoStack()->beginCommand( this, tr( "Remove Page" ) );
  }
  emit pageAboutToBeRemoved( pageNumber );
  beginPageSizeChange();
  QgsLayoutItemPage *page = mPages.takeAt( pageNumber );
  mLayout->removeItem( page );
  page->deleteLater();
  reflow();

  // bump stored page numbers to account
  for ( auto it = mPreviousItemPositions.begin(); it != mPreviousItemPositions.end(); ++it ) // clazy:exclude=detaching-member
  {
    if ( it.value().first <= pageNumber )
      continue;

    it.value().first = it.value().first - 1;
  }

  endPageSizeChange();
  if ( ! mBlockUndoCommands )
  {
    mLayout->undoStack()->endCommand();
    mLayout->undoStack()->endMacro();
  }
}

void QgsLayoutPageCollection::deletePage( QgsLayoutItemPage *page )
{
  if ( !mPages.contains( page ) )
    return;

  if ( !mBlockUndoCommands )
  {
    mLayout->undoStack()->beginMacro( tr( "Remove Page" ) );
    mLayout->undoStack()->beginCommand( this, tr( "Remove Page" ) );
  }
  int pageIndex = mPages.indexOf( page );
  emit pageAboutToBeRemoved( pageIndex );
  beginPageSizeChange();
  mPages.removeAll( page );
  page->deleteLater();
  // remove immediately from scene -- otherwise immediately calculation of layout bounds (such as is done
  // in reflow) will still consider the page, at least until it's actually deleted at the next event loop
  mLayout->removeItem( page );
  reflow();

  // bump stored page numbers to account
  for ( auto it = mPreviousItemPositions.begin(); it != mPreviousItemPositions.end(); ++it ) // clazy:exclude=detaching-member
  {
    if ( it.value().first <= pageIndex )
      continue;

    it.value().first = it.value().first - 1;
  }

  endPageSizeChange();
  if ( !mBlockUndoCommands )
  {
    mLayout->undoStack()->endCommand();
    mLayout->undoStack()->endMacro();
  }
}

void QgsLayoutPageCollection::clear()
{
  if ( !mBlockUndoCommands )
  {
    mLayout->undoStack()->beginMacro( tr( "Remove Pages" ) );
    mLayout->undoStack()->beginCommand( this, tr( "Remove Pages" ) );
  }
  for ( int i = mPages.count() - 1;  i >= 0; --i )
  {
    emit pageAboutToBeRemoved( i );
    mPages.takeAt( i )->deleteLater();
  }
  reflow();
  if ( !mBlockUndoCommands )
  {
    mLayout->undoStack()->endCommand();
    mLayout->undoStack()->endMacro();
  }
}

QgsLayoutItemPage *QgsLayoutPageCollection::takePage( QgsLayoutItemPage *page )
{
  mPages.removeAll( page );
  return page;
}

void QgsLayoutPageCollection::createDefaultPageStyleSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "no" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mPageStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );
}

