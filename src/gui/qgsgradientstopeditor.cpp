/***************************************************************************
    qgsgradientstopeditor.cpp
    -------------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgradientstopeditor.h"
#include "qgsapplication.h"
#include <QPainter>
#include <QStyleOptionFrameV3>
#include <QMouseEvent>

#define MARKER_WIDTH 11
#define MARKER_HEIGHT 14
#define MARKER_GAP 1.5
#define MARGIN_BOTTOM ( MARKER_HEIGHT + 2 )
#define MARGIN_X ( MARKER_WIDTH / 2 )
#define FRAME_MARGIN 2
#define CLICK_THRESHOLD ( MARKER_WIDTH / 2 + 3 )

QgsGradientStopEditor::QgsGradientStopEditor( QWidget *parent, QgsVectorGradientColorRampV2 *ramp )
    : QWidget( parent )
    , mSelectedStop( 0 )
{
  if ( ramp )
    mGradient = *ramp;
  mStops = mGradient.stops();

  if ( sOuterTriangle.isEmpty() )
  {
    sOuterTriangle << QPointF( 0, MARKER_HEIGHT ) << QPointF( MARKER_WIDTH, MARKER_HEIGHT )
    << QPointF( MARKER_WIDTH, MARKER_WIDTH / 2.0 )
    << QPointF( MARKER_WIDTH / 2.0, 0 )
    << QPointF( 0, MARKER_WIDTH / 2.0 )
    << QPointF( 0, MARKER_HEIGHT );
  }
  if ( sInnerTriangle.isEmpty() )
  {
    sInnerTriangle << QPointF( MARKER_GAP, MARKER_HEIGHT - MARKER_GAP ) << QPointF( MARKER_WIDTH - MARKER_GAP, MARKER_HEIGHT - MARKER_GAP )
    << QPointF( MARKER_WIDTH - MARKER_GAP, MARKER_WIDTH / 2.0 + 1 )
    << QPointF( MARKER_WIDTH / 2.0, MARKER_GAP )
    << QPointF( MARKER_GAP, MARKER_WIDTH / 2.0 + 1 )
    << QPointF( MARKER_GAP, MARKER_HEIGHT - MARKER_GAP );
  }

  setFocusPolicy( Qt::StrongFocus );
  setAcceptDrops( true );
}

void QgsGradientStopEditor::setGradientRamp( const QgsVectorGradientColorRampV2 &ramp )
{
  mGradient = ramp;
  mStops = mGradient.stops();
  mSelectedStop = 0;
  update();
  emit changed();
}

QSize QgsGradientStopEditor::sizeHint() const
{
  //horizontal
  return QSize( 200, 80 );
}

void QgsGradientStopEditor::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter painter( this );

  QRect frameRect( rect().x() + MARGIN_X, rect().y(),
                   rect().width() - 2 * MARGIN_X,
                   rect().height() - MARGIN_BOTTOM );

  //draw frame
  QStyleOptionFrameV3 option;
  option.initFrom( this );
  option.state = hasFocus() ? QStyle::State_KeyboardFocusChange : QStyle::State_None;
  option.rect = frameRect;
  style()->drawPrimitive( QStyle::PE_Frame, &option, &painter );

  if ( hasFocus() )
  {
    //draw focus rect
    QStyleOptionFocusRect option;
    option.initFrom( this );
    option.state = QStyle::State_KeyboardFocusChange;
    option.rect = frameRect;
    style()->drawPrimitive( QStyle::PE_FrameFocusRect, &option, &painter );
  }

  //start with the checkboard pattern
  QBrush checkBrush = QBrush( transparentBackground() );
  painter.setBrush( checkBrush );
  painter.setPen( Qt::NoPen );

  QRect box( frameRect.x() + FRAME_MARGIN, frameRect.y() + FRAME_MARGIN,
             frameRect.width() - 2 * FRAME_MARGIN,
             frameRect.height() - 2 * FRAME_MARGIN );

  painter.drawRect( box );

  // draw gradient preview on top of checkerboard
  for ( int i = 0; i < box.width() + 1; ++i )
  {
    QPen pen( mGradient.color( static_cast< double >( i ) / box.width() ) );
    painter.setPen( pen );
    painter.drawLine( box.left() + i, box.top(), box.left() + i, box.height() + 1 );
  }

  // draw stop markers
  int markerTop = frameRect.bottom() + 1;
  drawStopMarker( painter, QPoint( box.left(), markerTop ), mGradient.color1(), mSelectedStop == 0 );
  drawStopMarker( painter, QPoint( box.right(), markerTop ), mGradient.color2(), mSelectedStop == mGradient.count() - 1 );
  int i = 1;
  Q_FOREACH ( const QgsGradientStop& stop, mStops )
  {
    int x = stop.offset * box.width() + box.left();
    drawStopMarker( painter, QPoint( x, markerTop ), stop.color, mSelectedStop == i );
    ++i;
  }

  painter.end();
}

void QgsGradientStopEditor::selectStop( int index )
{
  if ( index > 0 && index < mGradient.count() - 1 )
  {
    // need to map original stop index across to cached, possibly out of order stop index
    QgsGradientStop selectedStop = mGradient.stops().at( index - 1 );
    index = 1;
    Q_FOREACH ( const QgsGradientStop& stop, mStops )
    {
      if ( stop == selectedStop )
      {
        break;
      }
      index++;
    }
  }

  mSelectedStop = index;
  emit selectedStopChanged( selectedStop() );
  update();
}

QgsGradientStop QgsGradientStopEditor::selectedStop() const
{
  if ( mSelectedStop > 0 && mSelectedStop < mGradient.count() - 1 )
  {
    return mStops.at( mSelectedStop - 1 );
  }
  else if ( mSelectedStop == 0 )
  {
    return QgsGradientStop( 0.0, mGradient.color1() );
  }
  else
  {
    return QgsGradientStop( 1.0, mGradient.color2() );
  }
}

void QgsGradientStopEditor::setSelectedStopColor( const QColor &color )
{
  if ( mSelectedStop > 0 && mSelectedStop < mGradient.count() - 1 )
  {
    mStops[ mSelectedStop - 1 ].color = color;
    mGradient.setStops( mStops );
  }
  else if ( mSelectedStop == 0 )
  {
    mGradient.setColor1( color );
  }
  else
  {
    mGradient.setColor2( color );
  }
  update();
  emit changed();
}

void QgsGradientStopEditor::setSelectedStopOffset( double offset )
{
  if ( mSelectedStop > 0 && mSelectedStop < mGradient.count() - 1 )
  {
    mStops[ mSelectedStop - 1 ].offset = offset;
    mGradient.setStops( mStops );
    update();
    emit changed();
  }
}

void QgsGradientStopEditor::setSelectedStopDetails( const QColor &color, double offset )
{
  if ( mSelectedStop > 0 && mSelectedStop < mGradient.count() - 1 )
  {
    mStops[ mSelectedStop - 1 ].color = color;
    mStops[ mSelectedStop - 1 ].offset = offset;
    mGradient.setStops( mStops );
  }
  else if ( mSelectedStop == 0 )
  {
    mGradient.setColor1( color );
  }
  else
  {
    mGradient.setColor2( color );
  }

  update();
  emit changed();
}

void QgsGradientStopEditor::deleteSelectedStop()
{
  if ( selectedStopIsMovable() )
  {
    //delete stop
    double stopOffset = mStops.at( mSelectedStop - 1 ).offset;
    mStops.removeAt( mSelectedStop - 1 );
    mGradient.setStops( mStops );

    int closest = findClosestStop( relativePositionToPoint( stopOffset ) );
    if ( closest >= 0 )
      selectStop( closest );
    update();
    emit changed();
  }
}

void QgsGradientStopEditor::setColor1( const QColor &color )
{
  mGradient.setColor1( color );
  update();
  emit changed();
}

void QgsGradientStopEditor::setColor2( const QColor &color )
{
  mGradient.setColor2( color );
  update();
  emit changed();
}

void QgsGradientStopEditor::mouseMoveEvent( QMouseEvent *e )
{
  if ( e->buttons() & Qt::LeftButton )
  {
    if ( selectedStopIsMovable() )
    {
      double offset = pointToRelativePosition( e->pos().x() );

      // have to edit the temporary stop list, as setting stops on the gradient will reorder them
      // and change which stop corresponds to the selected one;
      mStops[ mSelectedStop - 1 ].offset = offset;

      mGradient.setStops( mStops );
      update();
      emit changed();
    }
  }
  e->accept();
}

int QgsGradientStopEditor::findClosestStop( int x, int threshold ) const
{
  int closestStop = -1;
  int closestDiff = INT_MAX;
  int currentDiff = INT_MAX;

  // check for matching stops first, so that they take precedence
  // otherwise it's impossible to select a stop which sits above the first/last stop, making
  // it impossible to move or delete these
  int i = 1;
  Q_FOREACH ( const QgsGradientStop& stop, mGradient.stops() )
  {
    currentDiff = qAbs( relativePositionToPoint( stop.offset ) + 1 - x );
    if (( threshold < 0 || currentDiff < threshold ) && currentDiff < closestDiff )
    {
      closestStop = i;
      closestDiff = currentDiff;
    }
    i++;
  }

  //first stop
  currentDiff = qAbs( relativePositionToPoint( 0.0 ) + 1 - x );
  if (( threshold < 0 || currentDiff < threshold ) && currentDiff < closestDiff )
  {
    closestStop = 0;
    closestDiff = currentDiff;
  }

  //last stop
  currentDiff = qAbs( relativePositionToPoint( 1.0 ) + 1 - x );
  if (( threshold < 0 || currentDiff < threshold ) && currentDiff < closestDiff )
  {
    closestStop = mGradient.count() - 1;
    closestDiff = currentDiff;
  }

  return closestStop;
}

void QgsGradientStopEditor::mousePressEvent( QMouseEvent *e )
{
  if ( e->pos().y() >= rect().height() - MARGIN_BOTTOM - 1 )
  {
    // find closest point
    int closestStop = findClosestStop( e->pos().x(), CLICK_THRESHOLD );
    if ( closestStop >= 0 )
    {
      selectStop( closestStop );
    }
    update();
  }
  e->accept();
}

void QgsGradientStopEditor::mouseDoubleClickEvent( QMouseEvent *e )
{
  if ( e->buttons() & Qt::LeftButton )
  {
    // add a new stop
    double offset = pointToRelativePosition( e->pos().x() );
    mStops << QgsGradientStop( offset, mGradient.color( offset ) );
    mSelectedStop = mStops.length();
    mGradient.setStops( mStops );
    update();
    emit changed();
  }
  e->accept();
}

void QgsGradientStopEditor::keyPressEvent( QKeyEvent *e )
{
  if (( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    deleteSelectedStop();
    e->accept();
    return;
  }
  else if ( e->key() == Qt::Key_Left ||  e->key() == Qt::Key_Right )
  {
    if ( selectedStopIsMovable() )
    {
      // calculate offset corresponding to 1 px
      double offsetDiff = pointToRelativePosition( rect().x() + MARGIN_X + FRAME_MARGIN + 2 ) - pointToRelativePosition( rect().x() + MARGIN_X + FRAME_MARGIN + 1 );

      if ( e->modifiers() & Qt::ShiftModifier )
        offsetDiff *= 10.0;

      if ( e->key() == Qt::Key_Left )
        offsetDiff *= -1;

      mStops[ mSelectedStop - 1 ].offset = qBound( 0.0, mStops[ mSelectedStop - 1 ].offset + offsetDiff, 1.0 );
      mGradient.setStops( mStops );
      update();
      e->accept();
      emit changed();
      return;
    }
  }

  QWidget::keyPressEvent( e );
}

const QPixmap& QgsGradientStopEditor::transparentBackground()
{
  static QPixmap transpBkgrd;

  if ( transpBkgrd.isNull() )
    transpBkgrd = QgsApplication::getThemePixmap( "/transp-background_8x8.png" );

  return transpBkgrd;
}

void QgsGradientStopEditor::drawStopMarker( QPainter& painter, QPoint topMiddle, const QColor &color, bool selected )
{
  painter.save();
  painter.setRenderHint( QPainter::Antialiasing );
  painter.setBrush( selected ?  QColor( 150, 150, 150 ) : Qt::white );
  painter.setPen( selected ? Qt::black : QColor( 150, 150, 150 ) );
  // 0.5 offsets to make edges pixel grid aligned
  painter.translate( qRound( topMiddle.x() - MARKER_WIDTH / 2.0 ) + 0.5, topMiddle.y() + 0.5 );
  painter.drawPolygon( sOuterTriangle );

  // draw the checkerboard background for marker
  painter.setBrush( QBrush( transparentBackground() ) );
  painter.setPen( Qt::NoPen );
  painter.drawPolygon( sInnerTriangle );

  // draw color on top
  painter.setBrush( color );
  painter.drawPolygon( sInnerTriangle );
  painter.restore();
}

double QgsGradientStopEditor::pointToRelativePosition( int x ) const
{
  int left = rect().x() + MARGIN_X + FRAME_MARGIN;
  int right = left + rect().width() - 2 * MARGIN_X - 2 * FRAME_MARGIN;

  if ( x <= left )
    return 0;
  else if ( x >= right )
    return 1.0;

  return static_cast< double >( x - left ) / ( right - left );
}

int QgsGradientStopEditor::relativePositionToPoint( double position ) const
{
  int left = rect().x() + MARGIN_X + FRAME_MARGIN;
  int right = left + rect().width() - 2 * MARGIN_X - 2 * FRAME_MARGIN;

  if ( position <= 0 )
    return left;
  else if ( position >= 1.0 )
    return right;

  return left + ( right - left ) * position;
}

bool QgsGradientStopEditor::selectedStopIsMovable() const
{
  // first and last stop can't be moved or deleted
  return mSelectedStop > 0 && mSelectedStop < mGradient.count() - 1;
}


void QgsGradientStopEditor::dragEnterEvent( QDragEnterEvent *e )
{
  //is dragged data valid color data?
  bool hasAlpha;
  QColor mimeColor = QgsSymbolLayerV2Utils::colorFromMimeData( e->mimeData(), hasAlpha );

  if ( mimeColor.isValid() )
  {
    //if so, we accept the drag
    e->acceptProposedAction();
  }
}

void QgsGradientStopEditor::dropEvent( QDropEvent *e )
{
  //is dropped data valid color data?
  bool hasAlpha = false;
  QColor mimeColor = QgsSymbolLayerV2Utils::colorFromMimeData( e->mimeData(), hasAlpha );

  if ( mimeColor.isValid() )
  {
    //accept drop and set new color
    e->acceptProposedAction();

    // add a new stop here
    double offset = pointToRelativePosition( e->pos().x() );
    mStops << QgsGradientStop( offset, mimeColor );
    mSelectedStop = mStops.length();
    mGradient.setStops( mStops );
    update();
    emit changed();
  }

  //could not get color from mime data
}


