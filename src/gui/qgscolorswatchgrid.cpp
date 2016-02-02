/***************************************************************************
    qgscolorswatchgrid.cpp
    ------------------
    Date                 : July 2014
    Copyright            : (C) 2014 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorswatchgrid.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>

#define NUMBER_COLORS_PER_ROW 10 //number of color swatches per row
#define SWATCH_SIZE 14 //width/height of color swatches
#define SWATCH_SPACING 4 //horizontal/vertical gap between swatches
#define LEFT_MARGIN 6 //margin between left edge and first swatch
#define RIGHT_MARGIN 6 //margin between right edge and last swatch
#define TOP_MARGIN 6 //margin between label and first swatch
#define BOTTOM_MARGIN 6 //margin between last swatch row and end of widget
#define LABEL_SIZE 20 //label rect height
#define LABEL_MARGIN 4 //spacing between label box and text

QgsColorSwatchGrid::QgsColorSwatchGrid( QgsColorScheme* scheme, const QString& context, QWidget *parent )
    : QWidget( parent )
    , mScheme( scheme )
    , mContext( context )
    , mDrawBoxDepressed( false )
    , mCurrentHoverBox( -1 )
    , mFocused( false )
    , mCurrentFocusBox( 0 )
    , mPressedOnWidget( false )
{
  //need to receive all mouse over events
  setMouseTracking( true );

  setFocusPolicy( Qt::StrongFocus );
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

  //calculate widget width
  mWidth = NUMBER_COLORS_PER_ROW * SWATCH_SIZE + ( NUMBER_COLORS_PER_ROW - 1 ) * SWATCH_SPACING + LEFT_MARGIN + RIGHT_MARGIN;

  refreshColors();
}

QgsColorSwatchGrid::~QgsColorSwatchGrid()
{

}

QSize QgsColorSwatchGrid::minimumSizeHint() const
{
  return QSize( mWidth, calculateHeight() );
}

QSize QgsColorSwatchGrid::sizeHint() const
{
  return QSize( mWidth, calculateHeight() );
}

void QgsColorSwatchGrid::setContext( const QString &context )
{
  mContext = context;
  refreshColors();
}

void QgsColorSwatchGrid::setBaseColor( const QColor &baseColor )
{
  mBaseColor = baseColor;
  refreshColors();
}

void QgsColorSwatchGrid::refreshColors()
{
  //get colors from scheme
  mColors = mScheme->fetchColors( mContext, mBaseColor );

  //have to update size of widget in case number of colors has changed
  updateGeometry();
  repaint();
}

void QgsColorSwatchGrid::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter painter( this );
  draw( painter );
  painter.end();
}

void QgsColorSwatchGrid::mouseMoveEvent( QMouseEvent *event )
{
  //calculate box mouse cursor is over
  int newBox = swatchForPosition( event->pos() );

  mDrawBoxDepressed = event->buttons() & Qt::LeftButton;
  if ( newBox != mCurrentHoverBox )
  {
    //only repaint if changes are required
    mCurrentHoverBox = newBox;
    repaint();

    updateTooltip( newBox );
  }

  emit hovered();
}

void QgsColorSwatchGrid::updateTooltip( const int colorIdx )
{
  if ( colorIdx >= 0 && colorIdx < mColors.length() )
  {
    //if color has an associated name from the color scheme, use that
    QString colorName = mColors.at( colorIdx ).second;
    if ( colorName.isEmpty() )
    {
      //otherwise, build a default string
      QColor color = mColors.at( colorIdx ).first;
      colorName = QString( tr( "rgb(%1, %2, %3)" ) ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
    }
    setToolTip( colorName );
  }
  else
  {
    //clear tooltip
    setToolTip( QString() );
  }
}

void QgsColorSwatchGrid::mousePressEvent( QMouseEvent *event )
{
  if ( !mDrawBoxDepressed && event->buttons() & Qt::LeftButton )
  {
    mCurrentHoverBox = swatchForPosition( event->pos() );
    mDrawBoxDepressed = true;
    repaint();
  }
  mPressedOnWidget = true;
}

void QgsColorSwatchGrid::mouseReleaseEvent( QMouseEvent *event )
{
  if ( ! mPressedOnWidget )
  {
    return;
  }

  int box = swatchForPosition( event->pos() );
  if ( mDrawBoxDepressed && event->button() == Qt::LeftButton )
  {
    mCurrentHoverBox = box;
    mDrawBoxDepressed = false;
    repaint();
  }

  if ( box >= 0 && box < mColors.length() && event->button() == Qt::LeftButton )
  {
    //color clicked
    emit colorChanged( mColors.at( box ).first );
  }
}

void QgsColorSwatchGrid::keyPressEvent( QKeyEvent *event )
{
  //handle keyboard navigation
  if ( event->key() == Qt::Key_Right )
  {
    mCurrentFocusBox = qMin( mCurrentFocusBox + 1, mColors.length() - 1 );
  }
  else if ( event->key() == Qt::Key_Left )
  {
    mCurrentFocusBox = qMax( mCurrentFocusBox - 1, 0 );
  }
  else if ( event->key() == Qt::Key_Up )
  {
    int currentRow = mCurrentFocusBox / NUMBER_COLORS_PER_ROW;
    int currentColumn = mCurrentFocusBox % NUMBER_COLORS_PER_ROW;
    currentRow--;

    if ( currentRow >= 0 )
    {
      mCurrentFocusBox = currentRow * NUMBER_COLORS_PER_ROW + currentColumn;
    }
    else
    {
      //moved above first row
      focusPreviousChild();
    }
  }
  else if ( event->key() == Qt::Key_Down )
  {
    int currentRow = mCurrentFocusBox / NUMBER_COLORS_PER_ROW;
    int currentColumn = mCurrentFocusBox % NUMBER_COLORS_PER_ROW;
    currentRow++;
    int box = currentRow * NUMBER_COLORS_PER_ROW + currentColumn;

    if ( box < mColors.length() )
    {
      mCurrentFocusBox = box;
    }
    else
    {
      //moved below first row
      focusNextChild();
    }
  }
  else if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space )
  {
    //color clicked
    emit colorChanged( mColors.at( mCurrentFocusBox ).first );
  }
  else
  {
    //some other key, pass it on
    QWidget::keyPressEvent( event );
    return;
  }

  repaint();
}

void QgsColorSwatchGrid::focusInEvent( QFocusEvent *event )
{
  Q_UNUSED( event );
  mFocused = true;
  repaint();
}

void QgsColorSwatchGrid::focusOutEvent( QFocusEvent *event )
{
  Q_UNUSED( event );
  mFocused = false;
  repaint();
}

int QgsColorSwatchGrid::calculateHeight() const
{
  int numberRows = ceil(( double )mColors.length() / NUMBER_COLORS_PER_ROW );
  return numberRows * ( SWATCH_SIZE ) + ( numberRows - 1 ) * SWATCH_SPACING + TOP_MARGIN + LABEL_SIZE + BOTTOM_MARGIN;
}

void QgsColorSwatchGrid::draw( QPainter &painter )
{
  QPalette pal = QPalette( qApp->palette() );
  QColor headerBgColor = pal.color( QPalette::Mid );
  QColor headerTextColor = pal.color( QPalette::BrightText );
  QColor highlight = pal.color( QPalette::Highlight );

  //draw header background
  painter.setBrush( headerBgColor );
  painter.setPen( Qt::NoPen );
  painter.drawRect( QRect( 0, 0, width(), LABEL_SIZE ) );

  //draw header text
  painter.setPen( headerTextColor );
  painter.drawText( QRect( LABEL_MARGIN, 0, width() - 2 * LABEL_MARGIN, LABEL_SIZE ),
                    Qt::AlignLeft | Qt::AlignVCenter, mScheme->schemeName() );

  //draw color swatches
  QgsNamedColorList::const_iterator colorIt = mColors.constBegin();
  int index = 0;
  for ( ; colorIt != mColors.constEnd(); ++colorIt )
  {
    int row = index / NUMBER_COLORS_PER_ROW;
    int column = index % NUMBER_COLORS_PER_ROW;

    QRect swatchRect = QRect( column * ( SWATCH_SIZE + SWATCH_SPACING ) + LEFT_MARGIN,
                              row * ( SWATCH_SIZE + SWATCH_SPACING ) + TOP_MARGIN + LABEL_SIZE,
                              SWATCH_SIZE, SWATCH_SIZE );

    if ( mCurrentHoverBox == index )
    {
      //hovered boxes are slightly larger
      swatchRect.adjust( -1, -1, 1, 1 );
    }

    //start with checkboard pattern for semi-transparent colors
    if (( *colorIt ).first.alpha() != 255 )
    {
      QBrush checkBrush = QBrush( transparentBackground() );
      painter.setPen( Qt::NoPen );
      painter.setBrush( checkBrush );
      painter.drawRect( swatchRect );
    }

    if ( mCurrentHoverBox == index )
    {
      if ( mDrawBoxDepressed )
      {
        painter.setPen( QColor( 100, 100, 100 ) );
      }
      else
      {
        //hover color
        painter.setPen( QColor( 220, 220, 220 ) );
      }
    }
    else if ( mFocused && index == mCurrentFocusBox )
    {
      painter.setPen( highlight );
    }
    else if (( *colorIt ).first.name() == mBaseColor.name() )
    {
      //currently active color
      painter.setPen( QColor( 75, 75, 75 ) );
    }
    else
    {
      painter.setPen( QColor( 197, 197, 197 ) );
    }

    painter.setBrush(( *colorIt ).first );
    painter.drawRect( swatchRect );

    index++;
  }
}

const QPixmap& QgsColorSwatchGrid::transparentBackground()
{
  static QPixmap transpBkgrd;

  if ( transpBkgrd.isNull() )
    transpBkgrd = QgsApplication::getThemePixmap( "/transp-background_8x8.png" );

  return transpBkgrd;
}

int QgsColorSwatchGrid::swatchForPosition( QPoint position ) const
{
  //calculate box for position
  int box = -1;
  int column = ( position.x() - LEFT_MARGIN ) / ( SWATCH_SIZE + SWATCH_SPACING );
  int xRem = ( position.x() - LEFT_MARGIN ) % ( SWATCH_SIZE + SWATCH_SPACING );
  int row = ( position.y() - TOP_MARGIN - LABEL_SIZE ) / ( SWATCH_SIZE + SWATCH_SPACING );
  int yRem = ( position.y() - TOP_MARGIN - LABEL_SIZE ) % ( SWATCH_SIZE + SWATCH_SPACING );

  if ( xRem <= SWATCH_SIZE + 1 && yRem <= SWATCH_SIZE + 1 && column < NUMBER_COLORS_PER_ROW )
  {
    //if pos is actually inside a valid box, calculate which box
    box = column + row * NUMBER_COLORS_PER_ROW;
  }
  return box;
}


//
// QgsColorGridAction
//


QgsColorSwatchGridAction::QgsColorSwatchGridAction( QgsColorScheme* scheme, QMenu *menu, const QString& context, QWidget *parent )
    : QWidgetAction( parent )
    , mMenu( menu )
    , mSuppressRecurse( false )
    , mDismissOnColorSelection( true )
{
  mColorSwatchGrid = new QgsColorSwatchGrid( scheme, context, parent );

  setDefaultWidget( mColorSwatchGrid );
  connect( mColorSwatchGrid, SIGNAL( colorChanged( QColor ) ), this, SLOT( setColor( QColor ) ) );

  connect( this, SIGNAL( hovered() ), this, SLOT( onHover() ) );
  connect( mColorSwatchGrid, SIGNAL( hovered() ), this, SLOT( onHover() ) );

  //hide the action if no colors to be shown
  setVisible( !mColorSwatchGrid->colors()->isEmpty() );
}

QgsColorSwatchGridAction::~QgsColorSwatchGridAction()
{

}

void QgsColorSwatchGridAction::setBaseColor( const QColor &baseColor )
{
  mColorSwatchGrid->setBaseColor( baseColor );
}

QColor QgsColorSwatchGridAction::baseColor() const
{
  return mColorSwatchGrid->baseColor();
}

QString QgsColorSwatchGridAction::context() const
{
  return mColorSwatchGrid->context();
}

void QgsColorSwatchGridAction::setContext( const QString &context )
{
  mColorSwatchGrid->setContext( context );
}

void QgsColorSwatchGridAction::refreshColors()
{
  mColorSwatchGrid->refreshColors();
  //hide the action if no colors shown
  setVisible( !mColorSwatchGrid->colors()->isEmpty() );
}

void QgsColorSwatchGridAction::setColor( const QColor &color )
{
  emit colorChanged( color );
  QAction::trigger();
  if ( mMenu && mDismissOnColorSelection )
  {
    mMenu->hide();
  }
}

void QgsColorSwatchGridAction::onHover()
{
  //see https://bugreports.qt-project.org/browse/QTBUG-10427?focusedCommentId=185610&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-185610

  if ( mSuppressRecurse )
  {
    return;
  }

  if ( mMenu )
  {
    mSuppressRecurse = true;
    mMenu->setActiveAction( this );
    mSuppressRecurse = false;
  }
}
