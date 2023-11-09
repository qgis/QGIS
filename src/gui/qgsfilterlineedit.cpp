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
#include "qgis.h"

#include <QAction>
#include <QToolButton>
#include <QStyle>
#include <QFocusEvent>
#include <QPainter>

QgsFilterLineEdit::QgsFilterLineEdit( QWidget *parent, const QString &nullValue )
  : QLineEdit( parent )
  , mNullValue( nullValue )
{
  // icon size is about 2/3 height of text, but minimum size of 16
  const int iconSize = std::floor( std::max( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.75, 16.0 ) );

  mClearIcon.addPixmap( QgsApplication::getThemeIcon( "/mIconClearText.svg" ).pixmap( QSize( iconSize, iconSize ) ), QIcon::Normal, QIcon::On );
  mClearIcon.addPixmap( QgsApplication::getThemeIcon( "/mIconClearTextHover.svg" ).pixmap( QSize( iconSize, iconSize ) ), QIcon::Selected, QIcon::On );

  connect( this, &QLineEdit::textChanged, this,
           &QgsFilterLineEdit::onTextChanged );
}

void QgsFilterLineEdit::setShowClearButton( bool visible )
{
  mClearButtonVisible = visible;
  updateClearIcon();
}

void QgsFilterLineEdit::setShowSearchIcon( bool visible )
{
  if ( visible && !mSearchAction )
  {
    const QIcon searchIcon = QgsApplication::getThemeIcon( "/search.svg" );
    mSearchAction = new QAction( searchIcon, QString(), this );
    mSearchAction->setCheckable( false );
    addAction( mSearchAction, QLineEdit::LeadingPosition );
  }
  else if ( !visible && mSearchAction )
  {
    mSearchAction->deleteLater();
    mSearchAction = nullptr;
  }
}

void QgsFilterLineEdit::setDefaultValue( const QString &defaultValue )
{
  if ( defaultValue == mDefaultValue )
    return;

  mDefaultValue = defaultValue;
  updateClearIcon();
}

void QgsFilterLineEdit::updateClearIcon()
{
  const bool showClear = shouldShowClear();
  if ( showClear && !mClearAction )
  {
    mClearAction = new QAction( mClearIcon, QString(), this );
    mClearAction->setCheckable( false );
    addAction( mClearAction, QLineEdit::TrailingPosition );
    connect( mClearAction, &QAction::triggered, this, &QgsFilterLineEdit::clearValue );
  }
  else if ( !showClear && mClearAction )
  {
    // pretty freakin weird... seems the deleteLater call on the mClearAction
    // isn't sufficient to actually remove the action from the line edit, and
    // a kind of "ghost" action gets left behind... resulting in duplicate
    // clear actions appearing if later we re-create the action.
    // in summary: don't remove this "removeAction" call!
    removeAction( mClearAction );
    mClearAction->deleteLater();
    mClearAction = nullptr;
  }
}

void QgsFilterLineEdit::focusInEvent( QFocusEvent *e )
{
  QLineEdit::focusInEvent( e );
  if ( e->reason() == Qt::MouseFocusReason && ( isNull() || mSelectOnFocus ) )
  {
    mWaitingForMouseRelease = true;
  }
}

void QgsFilterLineEdit::mouseReleaseEvent( QMouseEvent *e )
{
  QLineEdit::mouseReleaseEvent( e );
  if ( mWaitingForMouseRelease )
  {
    mWaitingForMouseRelease = false;
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

  setModified( true );
  emit cleared();
}

void QgsFilterLineEdit::onTextChanged( const QString &text )
{
  updateClearIcon();

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
}

void QgsFilterLineEdit::updateBusySpinner()
{
  if ( !mBusySpinnerAction )
  {
    mBusySpinnerAction = addAction( mBusySpinnerAnimatedIcon->icon(), QLineEdit::TrailingPosition );
  }
  mBusySpinnerAction->setIcon( mBusySpinnerAnimatedIcon->icon() );
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
    if ( !mBusySpinnerAnimatedIcon )
      mBusySpinnerAnimatedIcon = new QgsAnimatedIcon( QgsApplication::iconPath( QStringLiteral( "/mIconLoading.gif" ) ), this );

    mBusySpinnerAnimatedIcon->connectFrameChanged( this, &QgsFilterLineEdit::updateBusySpinner );
  }
  else
  {
    mBusySpinnerAnimatedIcon->disconnectFrameChanged( this, &QgsFilterLineEdit::updateBusySpinner );
    removeAction( mBusySpinnerAction );
    mBusySpinnerAction = nullptr;
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

bool QgsFilterLineEdit::event( QEvent *event )
{
  if ( event->type() == QEvent::ReadOnlyChange || event->type() == QEvent::EnabledChange )
    updateClearIcon();

  return QLineEdit::event( event );
}

void QgsFilterLineEdit::storeState()
{
  mLineEditState.text = text();
  mLineEditState.selectionStart = selectionStart();
  mLineEditState.selectionLength = selectedText().length();
  mLineEditState.cursorPosition = cursorPosition();
  mLineEditState.hasStateStored = true;
}

void QgsFilterLineEdit::restoreState()
{
  setText( mLineEditState.text );
  setCursorPosition( mLineEditState.cursorPosition );
  if ( mLineEditState.selectionStart > -1 )
    setSelection( mLineEditState.selectionStart, mLineEditState.selectionLength );
  mLineEditState.hasStateStored = false;
}

/// @cond PRIVATE
void QgsSpinBoxLineEdit::focusInEvent( QFocusEvent *e )
{
  QgsFilterLineEdit::focusInEvent( e );
  if ( isNull() )
  {
    clear();
  }
}
/// @endcond
