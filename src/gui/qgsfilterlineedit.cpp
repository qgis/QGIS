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

QgsFilterLineEdit::QgsFilterLineEdit( QWidget* parent, const QString& nullValue )
    : QLineEdit( parent )
    , mNullValue( nullValue )
    , mFocusInEvent( false )
{
  btnClear = new QToolButton( this );
  btnClear->setIcon( QgsApplication::getThemeIcon( "/mIconClear.svg" ) );
  btnClear->setCursor( Qt::ArrowCursor );
  btnClear->setFocusPolicy( Qt::NoFocus );
  btnClear->setStyleSheet( "QToolButton { border: none; padding: 0px; }" );
  btnClear->hide();

  connect( btnClear, SIGNAL( clicked() ), this, SLOT( clear() ) );
  connect( btnClear, SIGNAL( clicked() ), this, SIGNAL( cleared() ) );
  connect( this, SIGNAL( textChanged( const QString& ) ), this,
           SLOT( onTextChanged( const QString& ) ) );

  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  mStyleSheet = QString( "QLineEdit { padding-right: %1px; } " )
                .arg( btnClear->sizeHint().width() + frameWidth + 1 );

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax( msz.width(), btnClear->sizeHint().height() + frameWidth * 2 + 2 ),
                  qMax( msz.height(), btnClear->sizeHint().height() + frameWidth * 2 + 2 ) );
}

void QgsFilterLineEdit::mousePressEvent( QMouseEvent* e )
{
  if ( !mFocusInEvent )
    QLineEdit::mousePressEvent( e );
  else
    mFocusInEvent = false;
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

void QgsFilterLineEdit::resizeEvent( QResizeEvent * )
{
  QSize sz = btnClear->sizeHint();
  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  btnClear->move( rect().right() - frameWidth - sz.width(),
                  ( rect().bottom() + 1 - sz.height() ) / 2 );
}

void QgsFilterLineEdit::clear()
{
  setText( mNullValue );
  setModified( true );
}

void QgsFilterLineEdit::changeEvent( QEvent *e )
{
  QLineEdit::changeEvent( e );
  btnClear->setVisible( isEnabled() && !isReadOnly() && !isNull() );
}

void QgsFilterLineEdit::paintEvent( QPaintEvent* e )
{
  QLineEdit::paintEvent( e );
  btnClear->setVisible( isEnabled() && !isReadOnly() && !isNull() );
}


void QgsFilterLineEdit::onTextChanged( const QString &text )
{
  btnClear->setVisible( isEnabled() && !isReadOnly() && !isNull() );

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
}
