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
#include "qgssymbollayerutils.h"
#include "qgslogger.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QBuffer>

#define NUMBER_COLORS_PER_ROW 10 //number of color swatches per row

QgsColorSwatchGrid::QgsColorSwatchGrid( QgsColorScheme *scheme, const QString &context, QWidget *parent )
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

  mLabelHeight = Qgis::UI_SCALE_FACTOR * fontMetrics().height();
  mLabelMargin = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( '.' );
  mSwatchSize = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 1.75;
  mSwatchOutlineSize = std::max( fontMetrics().horizontalAdvance( '.' ) * 0.4, 1.0 );

  mSwatchSpacing = mSwatchSize * 0.3;
  mSwatchMargin = mLabelMargin;

  //calculate widget width
  mWidth = NUMBER_COLORS_PER_ROW * mSwatchSize + ( NUMBER_COLORS_PER_ROW - 1 ) * mSwatchSpacing + mSwatchMargin + mSwatchMargin;

  refreshColors();
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
  Q_UNUSED( event )
  QPainter painter( this );
  draw( painter );
  painter.end();
}

void QgsColorSwatchGrid::mouseMoveEvent( QMouseEvent *event )
{
  //calculate box mouse cursor is over
  const int newBox = swatchForPosition( event->pos() );

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
    const QColor color = mColors.at( colorIdx ).first;

    //if color has an associated name from the color scheme, use that
    const QString colorName = mColors.at( colorIdx ).second;

    // create very large preview swatch, because the grid itself has only tiny preview icons
    const int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 23 );
    const int height = static_cast< int >( width / 1.61803398875 ); // golden ratio
    const int margin = static_cast< int >( height * 0.1 );
    QImage icon = QImage( width + 2 * margin, height + 2 * margin, QImage::Format_ARGB32 );
    icon.fill( Qt::transparent );

    QPainter p;
    p.begin( &icon );

    //start with checkboard pattern
    const QBrush checkBrush = QBrush( transparentBackground() );
    p.setPen( Qt::NoPen );
    p.setBrush( checkBrush );
    p.drawRect( margin, margin, width, height );

    //draw color over pattern
    p.setBrush( QBrush( mColors.at( colorIdx ).first ) );

    //draw border
    p.setPen( QColor( 197, 197, 197 ) );
    p.drawRect( margin, margin, width, height );
    p.end();

    QByteArray data;
    QBuffer buffer( &data );
    icon.save( &buffer, "PNG", 100 );

    QString info;
    if ( !colorName.isEmpty() )
      info += QStringLiteral( "<h3>%1</h3><p>" ).arg( colorName );

    info += QStringLiteral( "<b>HEX</b> %1<br>"
                            "<b>RGB</b> %2<br>"
                            "<b>HSV</b> %3,%4,%5<p>" ).arg( color.name(),
                                QgsSymbolLayerUtils::encodeColor( color ) )
            .arg( color.hue() ).arg( color.saturation() ).arg( color.value() );
    info += QStringLiteral( "<img src='data:image/png;base64, %0'>" ).arg( QString( data.toBase64() ) );

    setToolTip( info );

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

  const int box = swatchForPosition( event->pos() );
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
    mCurrentFocusBox = std::min< int >( mCurrentFocusBox + 1, mColors.length() - 1 );
  }
  else if ( event->key() == Qt::Key_Left )
  {
    mCurrentFocusBox = std::max< int >( mCurrentFocusBox - 1, 0 );
  }
  else if ( event->key() == Qt::Key_Up )
  {
    int currentRow = mCurrentFocusBox / NUMBER_COLORS_PER_ROW;
    const int currentColumn = mCurrentFocusBox % NUMBER_COLORS_PER_ROW;
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
    const int currentColumn = mCurrentFocusBox % NUMBER_COLORS_PER_ROW;
    currentRow++;
    const int box = currentRow * NUMBER_COLORS_PER_ROW + currentColumn;

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
  Q_UNUSED( event )
  mFocused = true;
  repaint();
}

void QgsColorSwatchGrid::focusOutEvent( QFocusEvent *event )
{
  Q_UNUSED( event )
  mFocused = false;
  repaint();
}

int QgsColorSwatchGrid::calculateHeight() const
{
  const int numberRows = std::ceil( static_cast<double>( mColors.length() ) / NUMBER_COLORS_PER_ROW );
  return numberRows * ( mSwatchSize ) + ( numberRows - 1 ) * mSwatchSpacing + mSwatchMargin + mLabelHeight + 0.5 * mLabelMargin + mSwatchMargin;
}

void QgsColorSwatchGrid::draw( QPainter &painter )
{
  const QPalette pal = QPalette( qApp->palette() );
  const QColor headerBgColor = pal.color( QPalette::Mid );
  const QColor headerTextColor = pal.color( QPalette::BrightText );
  const QColor highlight = pal.color( QPalette::Highlight );

  //draw header background
  painter.setBrush( headerBgColor );
  painter.setPen( Qt::NoPen );
  painter.drawRect( QRect( 0, 0, width(), mLabelHeight + 0.5 * mLabelMargin ) );

  //draw header text
  painter.setPen( headerTextColor );
  painter.drawText( QRect( mLabelMargin, 0.25 * mLabelMargin, width() - 2 * mLabelMargin, mLabelHeight ),
                    Qt::AlignLeft | Qt::AlignVCenter, mScheme->schemeName() );

  //draw color swatches
  QgsNamedColorList::const_iterator colorIt = mColors.constBegin();
  int index = 0;
  for ( ; colorIt != mColors.constEnd(); ++colorIt )
  {
    const int row = index / NUMBER_COLORS_PER_ROW;
    const int column = index % NUMBER_COLORS_PER_ROW;

    QRect swatchRect = QRect( column * ( mSwatchSize + mSwatchSpacing ) + mSwatchMargin,
                              row * ( mSwatchSize + mSwatchSpacing ) + mSwatchMargin + mLabelHeight + 0.5 * mLabelMargin,
                              mSwatchSize, mSwatchSize );

    if ( mCurrentHoverBox == index )
    {
      //hovered boxes are slightly larger
      swatchRect.adjust( -1, -1, 1, 1 );
    }

    //start with checkboard pattern for semi-transparent colors
    if ( ( *colorIt ).first.alpha() != 255 )
    {
      const QBrush checkBrush = QBrush( transparentBackground() );
      painter.setPen( Qt::NoPen );
      painter.setBrush( checkBrush );
      painter.drawRect( swatchRect );
    }

    if ( mCurrentHoverBox == index )
    {
      if ( mDrawBoxDepressed )
      {
        painter.setPen( QPen( QColor( 100, 100, 100 ), mSwatchOutlineSize ) );
      }
      else
      {
        //hover color
        painter.setPen( QPen( QColor( 220, 220, 220 ), mSwatchOutlineSize ) );
      }
    }
    else if ( mFocused && index == mCurrentFocusBox )
    {
      painter.setPen( highlight );
    }
    else if ( ( *colorIt ).first.name() == mBaseColor.name() )
    {
      //currently active color
      painter.setPen( QPen( QColor( 75, 75, 75 ), mSwatchOutlineSize ) );
    }
    else
    {
      painter.setPen( QPen( QColor( 197, 197, 197 ), mSwatchOutlineSize ) );
    }

    painter.setBrush( ( *colorIt ).first );
    painter.drawRect( swatchRect );

    index++;
  }
}

QPixmap QgsColorSwatchGrid::transparentBackground()
{
  static QPixmap sTranspBkgrd;

  if ( sTranspBkgrd.isNull() )
    sTranspBkgrd = QgsApplication::getThemePixmap( QStringLiteral( "/transp-background_8x8.png" ) );

  return sTranspBkgrd;
}

int QgsColorSwatchGrid::swatchForPosition( QPoint position ) const
{
  //calculate box for position
  int box = -1;
  const int column = ( position.x() - mSwatchMargin ) / ( mSwatchSize + mSwatchSpacing );
  const int xRem = ( position.x() - mSwatchMargin ) % ( mSwatchSize + mSwatchSpacing );
  const int row = ( position.y() - mSwatchMargin - mLabelHeight ) / ( mSwatchSize + mSwatchSpacing );
  const int yRem = ( position.y() - mSwatchMargin - mLabelHeight ) % ( mSwatchSize + mSwatchSpacing );

  if ( xRem <= mSwatchSize + 1 && yRem <= mSwatchSize + 1 && column < NUMBER_COLORS_PER_ROW )
  {
    //if pos is actually inside a valid box, calculate which box
    box = column + row * NUMBER_COLORS_PER_ROW;
  }
  return box;
}


//
// QgsColorGridAction
//


QgsColorSwatchGridAction::QgsColorSwatchGridAction( QgsColorScheme *scheme, QMenu *menu, const QString &context, QWidget *parent )
  : QWidgetAction( parent )
  , mMenu( menu )
  , mSuppressRecurse( false )
  , mDismissOnColorSelection( true )
{
  mColorSwatchGrid = new QgsColorSwatchGrid( scheme, context, parent );

  setDefaultWidget( mColorSwatchGrid );
  connect( mColorSwatchGrid, &QgsColorSwatchGrid::colorChanged, this, &QgsColorSwatchGridAction::setColor );

  connect( this, &QAction::hovered, this, &QgsColorSwatchGridAction::onHover );
  connect( mColorSwatchGrid, &QgsColorSwatchGrid::hovered, this, &QgsColorSwatchGridAction::onHover );

  //hide the action if no colors to be shown
  setVisible( !mColorSwatchGrid->colors()->isEmpty() );
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
  //see https://bugreports.qt.io/browse/QTBUG-10427?focusedCommentId=185610&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-185610

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
