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
#include "qgsanimatedicon.h"

#include <QToolButton>
#include <QStyle>
#include <QFocusEvent>
#include <QPainter>

QgsFilterLineEdit::QgsFilterLineEdit( QWidget *parent, const QString &nullValue )
  : QLineEdit( parent )
  , mNullValue( nullValue )
{
  // need mouse tracking to handle cursor changes
  setMouseTracking( true );

  QIcon clearIcon = QgsApplication::getThemeIcon( "/mIconClearText.svg" );
  mClearIconSize = QSize( 16, 16 );
  mClearIconPixmap = clearIcon.pixmap( mClearIconSize );
  QIcon hoverIcon = QgsApplication::getThemeIcon( "/mIconClearTextHover.svg" );
  mClearHoverPixmap = hoverIcon.pixmap( mClearIconSize );

  QIcon searchIcon = QgsApplication::getThemeIcon( "/search.svg" );
  mSearchIconSize = QSize( 16, 16 );
  mSearchIconPixmap = searchIcon.pixmap( mSearchIconSize );

  connect( this, &QLineEdit::textChanged, this,
           &QgsFilterLineEdit::onTextChanged );
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

void QgsFilterLineEdit::setShowSearchIcon( bool visible )
{
  bool changed = mSearchIconVisible != visible;
  if ( changed )
  {
    mSearchIconVisible = visible;
    update();
  }
}

void QgsFilterLineEdit::mousePressEvent( QMouseEvent *e )
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

void QgsFilterLineEdit::mouseMoveEvent( QMouseEvent *e )
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

void QgsFilterLineEdit::focusInEvent( QFocusEvent *e )
{
  QLineEdit::focusInEvent( e );
  if ( e->reason() == Qt::MouseFocusReason && ( isNull() || mSelectOnFocus ) )
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
      selectAll();
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

void QgsFilterLineEdit::paintEvent( QPaintEvent *e )
{
  QLineEdit::paintEvent( e );
  if ( shouldShowClear() )
  {
    QRect r = clearRect();
    QPainter p( this );
    if ( mClearHover )
      p.drawPixmap( r.left(), r.top(), mClearHoverPixmap );
    else
      p.drawPixmap( r.left(), r.top(), mClearIconPixmap );
  }

  if ( mSearchIconVisible && !shouldShowClear() )
  {
    QRect r = searchRect();
    QPainter p( this );
    p.drawPixmap( r.left(), r.top(), mSearchIconPixmap );
  }

  if ( mShowSpinner )
  {
    QRect r = busySpinnerRect();
    QPainter p( this );
    p.drawPixmap( r.left(), r.top(), mBusySpinner->icon().pixmap( r.size() ) );
  }
}

void QgsFilterLineEdit::leaveEvent( QEvent *e )
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
    setStyleSheet( QStringLiteral( "QLineEdit { font: italic; color: gray; } %1" ).arg( mStyleSheet ) );
    emit valueChanged( QString() );
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

void QgsFilterLineEdit::updateBusySpinner()
{
  update();
}

bool QgsFilterLineEdit::selectOnFocus() const
{
  return mSelectOnFocus;
}

void QgsFilterLineEdit::setSelectOnFocus( bool selectOnFocus )
{
  if ( mSelectOnFocus == selectOnFocus )
    return;

  mSelectOnFocus = selectOnFocus;
  emit selectOnFocusChanged();
}

bool QgsFilterLineEdit::showSpinner() const
{
  return mShowSpinner;
}

void QgsFilterLineEdit::setShowSpinner( bool showSpinner )
{
  if ( showSpinner == mShowSpinner )
    return;

  if ( showSpinner )
  {
    if ( !mBusySpinner )
      mBusySpinner = new QgsAnimatedIcon( QgsApplication::iconPath( QStringLiteral( "/mIconLoading.gif" ) ), this );

    mBusySpinner->connectFrameChanged( this, &QgsFilterLineEdit::updateBusySpinner );
  }
  else
  {
    mBusySpinner->disconnectFrameChanged( this, &QgsFilterLineEdit::updateBusySpinner );
    update();
  }

  mShowSpinner = showSpinner;
  emit showSpinnerChanged();
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

QRect QgsFilterLineEdit::busySpinnerRect() const
{
  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

  int offset = shouldShowClear() ? mClearIconSize.width() + frameWidth * 2 : frameWidth;

  return QRect( rect().right() - offset - mClearIconSize.width(),
                ( rect().bottom() + 1 - mClearIconSize.height() ) / 2,
                mClearIconSize.width(),
                mClearIconSize.height() );
}

QRect QgsFilterLineEdit::searchRect() const
{
  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  return QRect( rect().left() + frameWidth * 2,
                ( rect().bottom() + 1 - mSearchIconSize.height() ) / 2,
                mSearchIconSize.width(),
                mSearchIconSize.height() );
}
