/***************************************************************************
                              qgsfilterlineedit.cpp
                              ------------------------
  begin                : October 27, 2012
  copyright            : (C) 2012 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilterlineedit.h"
#include "qgsapplication.h"

#include <QToolButton>
#include <QStyle>
#include <QFocusEvent>
#include <QPainter>

QgsFilterLineEdit::QgsFilterLineEdit( QWidget* parent, const QString& nullValue )
    : QLineEdit( parent )
    , mClearButtonVisible( true )
    , mClearMode( ClearToNull )
    , mNullValue( nullValue )
    , mFocusInEvent( false )
    , mClearHover( false )
{
  // need mouse tracking to handle cursor changes
  setMouseTracking( true );

  QIcon clearIcon = QgsApplication::getThemeIcon( "/mIconClearText.svg" );
  mClearIconSize = QSize( 16, 16 );
  mClearIconPixmap = clearIcon.pixmap( mClearIconSize );
  QIcon hoverIcon = QgsApplication::getThemeIcon( "/mIconClearTextHover.svg" );
  mClearHoverPixmap = hoverIcon.pixmap( mClearIconSize );

  connect( this, SIGNAL( textChanged( const QString& ) ), this,
           SLOT( onTextChanged( const QString& ) ) );
}

void QgsFilterLineEdit::setShowClearButton( bool visible )
{
  bool changed = mClearButtonVisible != visible;
  mClearButtonVisible = visible;
  if ( !visible )
    mClearHover = false;

  if ( changed )
    update();
}

void QgsFilterLineEdit::mousePressEvent( QMouseEvent* e )
{
  if ( !mFocusInEvent )
    QLineEdit::mousePressEvent( e );
  else
    mFocusInEvent = false;

  if ( shouldShowClear() && clearRect().contains( e->pos() ) )
  {
    clearValue();
  }
}

void QgsFilterLineEdit::mouseMoveEvent( QMouseEvent* e )
{
  QLineEdit::mouseMoveEvent( e );
  if ( shouldShowClear() && clearRect().contains( e->pos() ) )
  {
    if ( !mClearHover )
    {
      setCursor( Qt::ArrowCursor );
      mClearHover = true;
      update();
    }
  }
  else if ( mClearHover )
  {
    setCursor( Qt::IBeamCursor );
    mClearHover = false;
    update();
  }
}

void QgsFilterLineEdit::focusInEvent( QFocusEvent* e )
{
  QLineEdit::focusInEvent( e );
  if ( e->reason() == Qt::MouseFocusReason && isNull() )
  {
    mFocusInEvent = true;
    selectAll();
  }
}

void QgsFilterLineEdit::clearValue()
{
  switch ( mClearMode )
  {
    case ClearToNull:
      setText( mNullValue );
      break;

    case ClearToDefault:
      setText( mDefaultValue );
      break;
  }

  if ( mClearHover )
  {
    setCursor( Qt::IBeamCursor );
    mClearHover = false;
  }

  setModified( true );
  emit cleared();
}

void QgsFilterLineEdit::paintEvent( QPaintEvent* e )
{
  QLineEdit::paintEvent( e );
  if ( shouldShowClear() )
  {
    QRect r = clearRect();
    QPainter p( this );
    if ( mClearHover )
      p.drawPixmap( r.left() , r.top() , mClearHoverPixmap );
    else
      p.drawPixmap( r.left() , r.top() , mClearIconPixmap );
  }
}

void QgsFilterLineEdit::leaveEvent( QEvent* e )
{
  if ( mClearHover )
  {
    mClearHover = false;
    update();
  }

  QLineEdit::leaveEvent( e );
}

void QgsFilterLineEdit::onTextChanged( const QString &text )
{
  if ( isNull() )
  {
    setStyleSheet( QString( "QLineEdit { font: italic; color: gray; } %1" ).arg( mStyleSheet ) );
    emit valueChanged( QString::null );
  }
  else
  {
    setStyleSheet( mStyleSheet );
    emit valueChanged( text );
  }

  if ( mClearHover && !shouldShowClear() )
  {
    setCursor( Qt::IBeamCursor );
    mClearHover = false;
  }
}

bool QgsFilterLineEdit::shouldShowClear() const
{
  if ( !isEnabled() || isReadOnly() || !mClearButtonVisible )
    return false;

  switch ( mClearMode )
  {
    case ClearToNull:
      return !isNull();

    case ClearToDefault:
      return value() != mDefaultValue;
  }
  return false; //avoid warnings
}

QRect QgsFilterLineEdit::clearRect() const
{
  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  return QRect( rect().right() - frameWidth * 2 - mClearIconSize.width(),
                ( rect().bottom() + 1 - mClearIconSize.height() ) / 2,
                mClearIconSize.width(),
                mClearIconSize.height() );
}
