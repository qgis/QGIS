/***************************************************************************
                             qgslayoutview.cpp
                             -----------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutviewtooltemporarykeypan.h"
#include "qgslayoutviewtooltemporarykeyzoom.h"
#include "qgslayoutviewtooltemporarymousepan.h"
#include "qgslayoutruler.h"
#include "qgssettings.h"
#include "qgsrectangle.h"
#include "qgsapplication.h"
#include <memory>
#include <QDesktopWidget>
#include <QMenu>

#define MIN_VIEW_SCALE 0.05
#define MAX_VIEW_SCALE 1000.0

QgsLayoutView::QgsLayoutView( QWidget *parent )
  : QGraphicsView( parent )
{
  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );

  // set the "scene" background on the view using stylesheets
  // we don't want to use QGraphicsScene::setBackgroundBrush because we want to keep
  // a transparent background for exports, and it's only a cosmetic thing for the view only
  // ALSO - only set it on the viewport - we don't want scrollbars/etc affected by this
  viewport()->setStyleSheet( QStringLiteral( "background-color:#d7d7d7;" ) );

  mSpacePanTool = new QgsLayoutViewToolTemporaryKeyPan( this );
  mMidMouseButtonPanTool = new QgsLayoutViewToolTemporaryMousePan( this );
  mSpaceZoomTool = new QgsLayoutViewToolTemporaryKeyZoom( this );
  mSnapMarker.reset( new QgsLayoutViewSnapMarker() );
}

QgsLayout *QgsLayoutView::currentLayout()
{
  return qobject_cast<QgsLayout *>( scene() );
}

const QgsLayout *QgsLayoutView::currentLayout() const
{
  return qobject_cast<const QgsLayout *>( scene() );
}

void QgsLayoutView::setCurrentLayout( QgsLayout *layout )
{
  setScene( layout );

  connect( layout->pageCollection(), &QgsLayoutPageCollection::changed, this, &QgsLayoutView::viewChanged );
  viewChanged();

  mSnapMarker.reset( new QgsLayoutViewSnapMarker() );
  mSnapMarker->hide();
  layout->addItem( mSnapMarker.get() );

  if ( mHorizontalRuler )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
  }
  if ( mVerticalRuler )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
  }

  //emit layoutSet, so that designer dialogs can update for the new layout
  emit layoutSet( layout );
}

QgsLayoutViewTool *QgsLayoutView::tool()
{
  return mTool;
}

void QgsLayoutView::setTool( QgsLayoutViewTool *tool )
{
  if ( !tool )
    return;

  if ( mTool )
  {
    mTool->deactivate();
  }

  mSnapMarker->setVisible( false );

  // activate new tool before setting it - gives tools a chance
  // to respond to whatever the current tool is
  tool->activate();
  mTool = tool;

  emit toolSet( mTool );
}

void QgsLayoutView::unsetTool( QgsLayoutViewTool *tool )
{
  if ( mTool && mTool == tool )
  {
    mTool->deactivate();
    emit toolSet( nullptr );
    setCursor( Qt::ArrowCursor );
  }
}

void QgsLayoutView::scaleSafe( double scale )
{
  double currentScale = transform().m11();
  scale *= currentScale;
  scale = qBound( MIN_VIEW_SCALE, scale, MAX_VIEW_SCALE );
  setTransform( QTransform::fromScale( scale, scale ) );
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::setZoomLevel( double level )
{
  if ( currentLayout()->units() == QgsUnitTypes::LayoutPixels )
  {
    setTransform( QTransform::fromScale( level, level ) );
  }
  else
  {
    double dpi = QgsApplication::desktop()->logicalDpiX();
    //monitor dpi is not always correct - so make sure the value is sane
    if ( ( dpi < 60 ) || ( dpi > 1200 ) )
      dpi = 72;

    //desired pixel width for 1mm on screen
    level = qBound( MIN_VIEW_SCALE, level, MAX_VIEW_SCALE );
    double mmLevel = currentLayout()->convertFromLayoutUnits( level, QgsUnitTypes::LayoutMillimeters ).length() * dpi / 25.4;
    setTransform( QTransform::fromScale( mmLevel, mmLevel ) );
  }
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::setHorizontalRuler( QgsLayoutRuler *ruler )
{
  mHorizontalRuler = ruler;
  ruler->setLayoutView( this );
  if ( QgsLayout *layout = currentLayout() )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, ruler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, ruler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, ruler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, ruler, [ = ] { mHorizontalRuler->update(); } );
  }
  viewChanged();
}

void QgsLayoutView::setVerticalRuler( QgsLayoutRuler *ruler )
{
  mVerticalRuler = ruler;
  ruler->setLayoutView( this );
  if ( QgsLayout *layout = currentLayout() )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, ruler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, ruler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, ruler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, ruler, [ = ] { mVerticalRuler->update(); } );
  }
  viewChanged();
}

void QgsLayoutView::setMenuProvider( QgsLayoutViewMenuProvider *provider )
{
  mMenuProvider.reset( provider );
}

QgsLayoutViewMenuProvider *QgsLayoutView::menuProvider() const
{
  return mMenuProvider.get();
}

QList<QgsLayoutItemPage *> QgsLayoutView::visiblePages() const
{
  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();
  return currentLayout()->pageCollection()->visiblePages( visibleRect );
}

QList<int> QgsLayoutView::visiblePageNumbers() const
{
  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();
  return currentLayout()->pageCollection()->visiblePageNumbers( visibleRect );
}

void QgsLayoutView::zoomFull()
{
  fitInView( scene()->sceneRect(), Qt::KeepAspectRatio );
  viewChanged();
  emit zoomLevelChanged();
}

void QgsLayoutView::zoomWidth()
{
  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();

  double verticalCenter = ( visibleRect.top() + visibleRect.bottom() ) / 2.0;
  // expand out visible rect to include left/right edges of scene
  // centered on current visible vertical center
  // note that we can't have a 0 height rect - fitInView doesn't handle that
  // so we just set a very small height instead.
  const double tinyHeight = 0.01;
  QRectF targetRect( scene()->sceneRect().left(),
                     verticalCenter - tinyHeight,
                     scene()->sceneRect().width(),
                     tinyHeight * 2 );

  fitInView( targetRect, Qt::KeepAspectRatio );
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::zoomIn()
{
  scaleSafe( 2 );
}

void QgsLayoutView::zoomOut()
{
  scaleSafe( 0.5 );
}

void QgsLayoutView::zoomActual()
{
  setZoomLevel( 1.0 );
}

void QgsLayoutView::emitZoomLevelChanged()
{
  emit zoomLevelChanged();
}

void QgsLayoutView::mousePressEvent( QMouseEvent *event )
{
  mSnapMarker->setVisible( false );

  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    mTool->layoutPressEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
  {
    if ( event->button() == Qt::MidButton )
    {
      // Pan layout with middle mouse button
      setTool( mMidMouseButtonPanTool );
      event->accept();
    }
    else if ( event->button() == Qt::RightButton && mMenuProvider )
    {
      QMenu *menu = mMenuProvider->createContextMenu( this, currentLayout(), mapToScene( event->pos() ) );
      if ( menu )
      {
        menu->exec( event->globalPos() );
        delete menu;
      }
    }
    else
    {
      QGraphicsView::mousePressEvent( event );
    }
  }
  else
  {
    QGraphicsView::mousePressEvent( event );
  }
}

void QgsLayoutView::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    mTool->layoutReleaseEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsLayoutView::mouseMoveEvent( QMouseEvent *event )
{
  mMouseCurrentXY = event->pos();

  QPointF cursorPos = mapToScene( mMouseCurrentXY );
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    if ( mTool->flags() & QgsLayoutViewTool::FlagSnaps )
    {
      //draw snapping point indicator
      if ( me->isSnapped() )
      {
        cursorPos = me->snappedPoint();
        mSnapMarker->setPos( me->snappedPoint() );
        mSnapMarker->setVisible( true );
      }
      else
        mSnapMarker->setVisible( false );
    }
    mTool->layoutMoveEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  //update cursor position in status bar
  emit cursorPosChanged( cursorPos );

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseMoveEvent( event );
}

void QgsLayoutView::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    mTool->layoutDoubleClickEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsLayoutView::wheelEvent( QWheelEvent *event )
{
  if ( mTool )
  {
    mTool->wheelEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
  {
    event->accept();
    wheelZoom( event );
  }
}

void QgsLayoutView::keyPressEvent( QKeyEvent *event )
{
  if ( mTool )
  {
    mTool->keyPressEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
  {
    if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() )
    {
      if ( !( event->modifiers() & Qt::ControlModifier ) )
      {
        // Pan layout with space bar
        setTool( mSpacePanTool );
      }
      else
      {
        //ctrl+space pressed, so switch to temporary keyboard based zoom tool
        setTool( mSpaceZoomTool );
      }
      event->accept();
    }
  }
}

void QgsLayoutView::keyReleaseEvent( QKeyEvent *event )
{
  if ( mTool )
  {
    mTool->keyReleaseEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyReleaseEvent( event );
}

void QgsLayoutView::resizeEvent( QResizeEvent *event )
{
  QGraphicsView::resizeEvent( event );
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::scrollContentsBy( int dx, int dy )
{
  QGraphicsView::scrollContentsBy( dx, dy );
  viewChanged();
}

void QgsLayoutView::viewChanged()
{
  if ( mHorizontalRuler )
  {
    mHorizontalRuler->setSceneTransform( viewportTransform() );
  }
  if ( mVerticalRuler )
  {
    mVerticalRuler->setSceneTransform( viewportTransform() );
  }

  // determine page at center of view
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();
  QPointF centerVisible = visibleRect.center();

  if ( currentLayout() && currentLayout()->pageCollection() )
  {
    int newPage = currentLayout()->pageCollection()->pageNumberForPoint( centerVisible );
    if ( newPage != mCurrentPage )
    {
      mCurrentPage = newPage;
      emit pageChanged( mCurrentPage );
    }
  }
}

void QgsLayoutView::wheelZoom( QWheelEvent *event )
{
  //get mouse wheel zoom behavior settings
  QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  bool zoomIn = event->angleDelta().y() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
  QPointF scenePoint = mapToScene( event->pos() );

  //adjust view center
  QgsPointXY oldCenter( visibleRect.center() );
  QgsPointXY newCenter( scenePoint.x() + ( ( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                        scenePoint.y() + ( ( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );
  centerOn( newCenter.x(), newCenter.y() );

  //zoom layout
  if ( zoomIn )
  {
    scaleSafe( zoomFactor );
  }
  else
  {
    scaleSafe( 1 / zoomFactor );
  }
}


//
// QgsLayoutViewSnapMarker
//

///@cond PRIVATE
QgsLayoutViewSnapMarker::QgsLayoutViewSnapMarker()
  : QGraphicsRectItem( QRectF( 0, 0, 0, 0 ) )
{
  QFont f;
  QFontMetrics fm( f );
  mSize = fm.width( "X" );
  setPen( QPen( Qt::transparent, mSize ) );

  setFlags( flags() | QGraphicsItem::ItemIgnoresTransformations );
  setZValue( QgsLayout::ZSnapIndicator );
}

void QgsLayoutViewSnapMarker::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * )
{
  QPen pen( QColor( 255, 0, 0 ) );
  pen.setWidth( 0 );
  p->setPen( pen );
  p->setBrush( Qt::NoBrush );

  double halfSize = mSize / 2.0;
  p->drawLine( QLineF( -halfSize, -halfSize, halfSize, halfSize ) );
  p->drawLine( QLineF( -halfSize, halfSize, halfSize, -halfSize ) );
}

///@endcond
