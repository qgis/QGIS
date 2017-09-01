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

QgsLayoutPageCollection::QgsLayoutPageCollection( QgsLayout *layout )
  : QObject( layout )
  , mLayout( layout )
{
  createDefaultPageStyleSymbol();
}

QgsLayoutPageCollection::~QgsLayoutPageCollection()
{
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
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
}

void QgsLayoutPageCollection::reflow()
{
  double currentY = 0;
  QgsLayoutPoint p( 0, 0, mLayout->units() );
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
  {
    page->attemptMove( p );
    currentY += mLayout->convertToLayoutUnits( page->pageSize() ).height() + spaceBetweenPages();
    p.setY( currentY );
  }
  mLayout->updateBounds();
  emit changed();
}

double QgsLayoutPageCollection::maximumPageWidth() const
{
  double maxWidth = 0;
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
  {
    maxWidth = std::max( maxWidth, mLayout->convertToLayoutUnits( page->pageSize() ).width() );
  }
  return maxWidth;
}

int QgsLayoutPageCollection::pageNumberForPoint( QPointF point ) const
{
  int pageNumber = 0;
  double startNextPageY = 0;
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
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

QgsLayoutItemPage *QgsLayoutPageCollection::pageAtPoint( QPointF point ) const
{
  Q_FOREACH ( QGraphicsItem *item, mLayout->items( point ) )
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

QPointF QgsLayoutPageCollection::positionOnPage( QPointF position ) const
{
  double startCurrentPageY = 0;
  double startNextPageY = 0;
  int pageNumber = 0;
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
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

bool QgsLayoutPageCollection::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "PageCollection" ) );

  QDomElement pageStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mPageStyleSymbol.get(), document, context );
  element.appendChild( pageStyleElem );

  for ( const QgsLayoutItemPage *page : mPages )
  {
    page->writeXml( element, document, context );
  }

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutPageCollection::readXml( const QDomElement &e, const QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = e;
  if ( element.nodeName() != QStringLiteral( "PageCollection" ) )
  {
    element = element.firstChildElement( QStringLiteral( "PageCollection" ) );
  }

  if ( element.nodeName() != QStringLiteral( "PageCollection" ) )
  {
    return false;
  }

  int i = 0;
  for ( QgsLayoutItemPage *page : qgsAsConst( mPages ) )
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
    page->readXml( pageElement, document, context );
    mPages.append( page.get() );
    mLayout->addItem( page.release() );
  }

  reflow();
  return true;
}

void QgsLayoutPageCollection::redraw()
{
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
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

int QgsLayoutPageCollection::pageNumber( QgsLayoutItemPage *page ) const
{
  return mPages.indexOf( page );
}

QList<QgsLayoutItemPage *> QgsLayoutPageCollection::visiblePages( QRectF region ) const
{
  QList<QgsLayoutItemPage *> pages;
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
  {
    if ( page->mapToScene( page->rect() ).boundingRect().intersects( region ) )
      pages << page;
  }
  return pages;
}

QList<int> QgsLayoutPageCollection::visiblePageNumbers( QRectF region ) const
{
  QList< int > pages;
  int p = 0;
  Q_FOREACH ( QgsLayoutItemPage *page, mPages )
  {
    if ( page->mapToScene( page->rect() ).boundingRect().intersects( region ) )
      pages << p;
    p++;
  }
  return pages;
}

void QgsLayoutPageCollection::addPage( QgsLayoutItemPage *page )
{
  mLayout->undoStack()->beginCommand( this, tr( "Add page" ) );
  mPages.append( page );
  mLayout->addItem( page );
  reflow();
  mLayout->undoStack()->endCommand();
}

void QgsLayoutPageCollection::insertPage( QgsLayoutItemPage *page, int beforePage )
{
  mLayout->undoStack()->beginCommand( this, tr( "Add page" ) );

  if ( beforePage < 0 )
    beforePage = 0;

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
  mLayout->undoStack()->endCommand();
}

///@cond PRIVATE

class QgsLayoutItemDeletePageUndoCommand: public QgsLayoutItemDeleteUndoCommand
{
  public:

    QgsLayoutItemDeletePageUndoCommand( QgsLayoutItemPage *page, const QString &text, int id = 0, QUndoCommand *parent SIP_TRANSFERTHIS = nullptr )
      : QgsLayoutItemDeleteUndoCommand( page, text, id, parent )
    {}

    void redo() override
    {
      if ( mFirstRun )
      {
        mFirstRun = false;
        return;
      }

      // remove from page collection
      QgsLayoutItemPage *page = dynamic_cast< QgsLayoutItemPage *>( layout()->itemByUuid( itemUuid() ) );
      layout()->pageCollection()->takePage( page );

      QgsLayoutItemDeleteUndoCommand::redo();
      layout()->pageCollection()->reflow();
    }

    void undo() override
    {
      QgsLayoutItemDeleteUndoCommand::undo();
      layout()->pageCollection()->reflow();
    }

    QgsLayoutItem *recreateItem( int, QgsLayout *layout ) override
    {
      QgsLayoutItemPage *page = new QgsLayoutItemPage( layout );
      layout->pageCollection()->addPage( page );
      return page;
    }

};

///@endcond

void QgsLayoutPageCollection::deletePage( int pageNumber )
{
  if ( pageNumber < 0 || pageNumber >= mPages.count() )
    return;

  mLayout->undoStack()->beginMacro( tr( "Remove page" ) );
  mLayout->undoStack()->beginCommand( this, tr( "Remove page" ) );
  emit pageAboutToBeRemoved( pageNumber );
  QgsLayoutItemPage *page = mPages.takeAt( pageNumber );
  //mLayout->undoStack()->stack()->push( new QgsLayoutItemDeletePageUndoCommand( page, tr( "Remove page" ) ) );
  mLayout->removeItem( page );
  page->deleteLater();
  reflow();
  mLayout->undoStack()->endCommand();
  mLayout->undoStack()->endMacro();
}

void QgsLayoutPageCollection::deletePage( QgsLayoutItemPage *page )
{
  if ( !mPages.contains( page ) )
    return;

  mLayout->undoStack()->beginMacro( tr( "Remove page" ) );
  mLayout->undoStack()->beginCommand( this, tr( "Remove page" ) );
  emit pageAboutToBeRemoved( mPages.indexOf( page ) );
  //mLayout->undoStack()->stack()->push( new QgsLayoutItemDeletePageUndoCommand( page, tr( "Remove page" ) ) );
  mPages.removeAll( page );
  page->deleteLater();
  reflow();
  mLayout->undoStack()->endCommand();
  mLayout->undoStack()->endMacro();
}

QgsLayoutItemPage *QgsLayoutPageCollection::takePage( QgsLayoutItemPage *page )
{
  mPages.removeAll( page );
  return page;
}

void QgsLayoutPageCollection::createDefaultPageStyleSymbol()
{
  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "no" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mPageStyleSymbol.reset( QgsFillSymbol::createSimple( properties ) );
}
