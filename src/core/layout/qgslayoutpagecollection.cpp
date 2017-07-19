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

QgsLayout *QgsLayoutPageCollection::layout() const
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

void QgsLayoutPageCollection::addPage( QgsLayoutItemPage *page )
{
  mPages.append( page );
  mLayout->addItem( page );
}

void QgsLayoutPageCollection::insertPage( QgsLayoutItemPage *page, int beforePage )
{
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
}

void QgsLayoutPageCollection::deletePage( int pageNumber )
{
  if ( pageNumber < 0 || pageNumber >= mPages.count() )
    return;

  QgsLayoutItemPage *page = mPages.takeAt( pageNumber );
  mLayout->removeItem( page );
  page->deleteLater();
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
