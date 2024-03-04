/***************************************************************************
    qgsoverlaywidgetlayout.cpp
    ---------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoverlaywidgetlayout.h"
#include "qgis.h"
#include <QWidget>

QgsOverlayWidgetLayout::QgsOverlayWidgetLayout( QWidget *parent )
  : QLayout( parent )
{

}

QgsOverlayWidgetLayout::~QgsOverlayWidgetLayout()
{
  QLayoutItem *item;
  while ( ( item = takeAt( 0 ) ) )
    delete item;
}

int QgsOverlayWidgetLayout::count() const
{
  return mLeftItems.size() + mRightItems.size() + mTopItems.size() + mBottomItems.size();
}

void QgsOverlayWidgetLayout::addItem( QLayoutItem *item )
{
  if ( !mLeftItems.contains( item ) && !mRightItems.contains( item ) && !mTopItems.contains( item ) && !mBottomItems.contains( item ) )
    mLeftItems.append( item );
}

QLayoutItem *QgsOverlayWidgetLayout::itemAt( int index ) const
{
  if ( index < 0 )
    return nullptr;

  if ( index < mLeftItems.size() )
    return mLeftItems.at( index );
  index -= mLeftItems.size();

  if ( index < mRightItems.size() )
    return mRightItems.at( index );
  index -= mRightItems.size();

  if ( index < mTopItems.size() )
    return mTopItems.at( index );
  index -= mTopItems.size();

  if ( index < mBottomItems.size() )
    return mBottomItems.at( index );

  return nullptr;
}

QLayoutItem *QgsOverlayWidgetLayout::takeAt( int index )
{
  if ( index < 0 )
    return nullptr;

  if ( index < mLeftItems.size() )
    return mLeftItems.takeAt( index );
  index -= mLeftItems.size();

  if ( index < mRightItems.size() )
    return mRightItems.takeAt( index );
  index -= mRightItems.size();

  if ( index < mTopItems.size() )
    return mTopItems.takeAt( index );
  index -= mTopItems.size();

  if ( index < mBottomItems.size() )
    return mBottomItems.takeAt( index );

  return nullptr;
}

QSize QgsOverlayWidgetLayout::sizeHint() const
{
  if ( QWidget *parent = parentWidget() )
  {
    return parent->sizeHint();
  }
  return QSize();
}

QSize QgsOverlayWidgetLayout::minimumSize() const
{
  if ( QWidget *parent = parentWidget() )
  {
    return parent->minimumSize();
  }
  return QSize();
}

void QgsOverlayWidgetLayout::setGeometry( const QRect &rect )
{
  QLayout::setGeometry( rect );

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  getContentsMargins( &leftMargin, &topMargin, &rightMargin, &bottomMargin );

  // adjust available rect to account for margins
  const int innerLeft = rect.left() + leftMargin;
  const int innerRight = rect.right() - rightMargin;
  const int innerTop = rect.top() + topMargin;
  const int innerBottom = rect.bottom() - bottomMargin;
  const int innerHeight = innerBottom - innerTop;

  int left = innerLeft;
  for ( QLayoutItem *item : std::as_const( mLeftItems ) )
  {
    const QSize sizeHint = item->sizeHint();
    item->setGeometry( QRect( left, innerTop, sizeHint.width(), innerHeight ) );
    left += sizeHint.width() + mHorizontalSpacing;
  }

  int right = innerRight;
  for ( QLayoutItem *item : std::as_const( mRightItems ) )
  {
    const QSize sizeHint = item->sizeHint();
    item->setGeometry( QRect( right - sizeHint.width(), innerTop, sizeHint.width(), innerHeight ) );
    right -= sizeHint.width() + mHorizontalSpacing;
  }

  int top = innerTop;
  for ( QLayoutItem *item : std::as_const( mTopItems ) )
  {
    const QSize sizeHint = item->sizeHint();
    item->setGeometry( QRect( left, top, right - left, sizeHint.height() ) );
    top += sizeHint.height() + mVerticalSpacing;
  }

  int bottom = innerBottom;
  for ( QLayoutItem *item : std::as_const( mBottomItems ) )
  {
    const QSize sizeHint = item->sizeHint();
    item->setGeometry( QRect( left, bottom - sizeHint.height(), right - left, sizeHint.height() ) );
    bottom -= sizeHint.height() + mVerticalSpacing;
  }
}

void QgsOverlayWidgetLayout::addWidget( QWidget *widget, Qt::Edge edge )
{
  QWidgetItem *widgetItem = new QWidgetItem( widget );
  switch ( edge )
  {
    case Qt::LeftEdge:
      mLeftItems.append( widgetItem );
      break;
    case Qt::RightEdge:
      mRightItems.append( widgetItem );
      break;
    case Qt::TopEdge:
      mTopItems.append( widgetItem );
      break;
    case Qt::BottomEdge:
      mBottomItems.append( widgetItem );
      break;
  }

  addChildWidget( widget );
  invalidate();
}

void QgsOverlayWidgetLayout::setHorizontalSpacing( int spacing )
{
  mHorizontalSpacing = spacing;
  invalidate();
}

void QgsOverlayWidgetLayout::setVerticalSpacing( int spacing )
{
  mVerticalSpacing = spacing;
  invalidate();
}
