/***************************************************************************
    qgsfloatingwidget.cpp
    ---------------------
    begin                : April 2016
    copyright            : (C) Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfloatingwidget.h"
#include <QEvent>
#include <QStyleOption>
#include <QPainter>

//
// QgsFloatingWidget
//

QgsFloatingWidget::QgsFloatingWidget( QWidget *parent )
    : QWidget( parent )
    , mAnchorWidget( nullptr )
    , mParentEventFilter( nullptr )
    , mAnchorEventFilter( nullptr )
    , mFloatAnchorPoint( BottomMiddle )
    , mAnchorWidgetAnchorPoint( TopMiddle )
{
  if ( parent )
  {
    mParentEventFilter = new QgsFloatingWidgetEventFilter( parent );
    parent->installEventFilter( mParentEventFilter );
    connect( mParentEventFilter, SIGNAL( anchorPointChanged() ), this, SLOT( anchorPointChanged() ) );
  }
}

void QgsFloatingWidget::setAnchorWidget( QWidget *widget )
{
  // remove existing event filter
  if ( mAnchorWidget )
  {
    mAnchorWidget->removeEventFilter( mAnchorEventFilter );
    delete mAnchorEventFilter;
    mAnchorEventFilter = nullptr;
  }

  mAnchorWidget = widget;
  if ( mAnchorWidget )
  {
    mAnchorEventFilter = new QgsFloatingWidgetEventFilter( mAnchorWidget );
    mAnchorWidget->installEventFilter( mAnchorEventFilter );
    connect( mAnchorEventFilter, SIGNAL( anchorPointChanged() ), this, SLOT( anchorPointChanged() ) );
  }

  anchorPointChanged();
}

QWidget *QgsFloatingWidget::anchorWidget()
{
  return mAnchorWidget;
}

void QgsFloatingWidget::showEvent( QShowEvent *e )
{
  anchorPointChanged();
  QWidget::showEvent( e );
}

void QgsFloatingWidget::paintEvent( QPaintEvent* e )
{
  Q_UNUSED( e );
  QStyleOption opt;
  opt.init( this );
  QPainter p( this );
  style()->drawPrimitive( QStyle::PE_Widget, &opt, &p, this );
}

void QgsFloatingWidget::anchorPointChanged()
{
  if ( mAnchorWidget )
  {
    QPoint anchorWidgetOrigin;

    switch ( mAnchorWidgetAnchorPoint )
    {
      case TopLeft:
        anchorWidgetOrigin = QPoint( 0, 0 );
        break;
      case TopMiddle:
        anchorWidgetOrigin = QPoint( mAnchorWidget->width() / 2, 0 );
        break;
      case TopRight:
        anchorWidgetOrigin = QPoint( mAnchorWidget->width(), 0 );
        break;
      case MiddleLeft:
        anchorWidgetOrigin = QPoint( 0, mAnchorWidget->height() / 2 );
        break;
      case Middle:
        anchorWidgetOrigin = QPoint( mAnchorWidget->width() / 2, mAnchorWidget->height() / 2 );
        break;
      case MiddleRight:
        anchorWidgetOrigin = QPoint( mAnchorWidget->width(), mAnchorWidget->height() / 2 );
        break;
      case BottomLeft:
        anchorWidgetOrigin = QPoint( 0, mAnchorWidget->height() );
        break;
      case BottomMiddle:
        anchorWidgetOrigin = QPoint( mAnchorWidget->width() / 2, mAnchorWidget->height() );
        break;
      case BottomRight:
        anchorWidgetOrigin = QPoint( mAnchorWidget->width(), mAnchorWidget->height() );
        break;
    }

    anchorWidgetOrigin = mAnchorWidget->mapTo( parentWidget(), anchorWidgetOrigin );
    int anchorX = anchorWidgetOrigin.x();
    int anchorY = anchorWidgetOrigin.y();

    switch ( mFloatAnchorPoint )
    {
      case TopLeft:
        break;
      case TopMiddle:
        anchorX = anchorX - width() / 2;
        break;
      case TopRight:
        anchorX = anchorX - width();
        break;
      case MiddleLeft:
        anchorY = anchorY - height() / 2;
        break;
      case Middle:
        anchorY = anchorY - height() / 2;
        anchorX = anchorX - width() / 2;
        break;
      case MiddleRight:
        anchorX = anchorX - width();
        anchorY = anchorY - height() / 2;
        break;
      case BottomLeft:
        anchorY = anchorY - height();
        break;
      case BottomMiddle:
        anchorX = anchorX - width() / 2;
        anchorY = anchorY - height();
        break;
      case BottomRight:
        anchorX = anchorX - width();
        anchorY = anchorY - height();
        break;
    }

    // constrain x so that widget floats within parent widget
    anchorX = qBound( 0, anchorX, parentWidget()->width() - width() );

    move( anchorX, anchorY );
  }
}

//
// QgsFloatingWidgetEventFilter
//

/// @cond PRIVATE
QgsFloatingWidgetEventFilter::QgsFloatingWidgetEventFilter( QWidget *parent )
    : QObject( parent )
{

}

bool QgsFloatingWidgetEventFilter::eventFilter( QObject *object, QEvent *event )
{
  Q_UNUSED( object );
  switch ( event->type() )
  {
    case QEvent::Move:
    case QEvent::Resize:
      emit anchorPointChanged();
      return true;
    default:
      return false;
  }
}

///@endcond
