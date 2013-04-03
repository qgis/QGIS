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

QgsFilterLineEdit::QgsFilterLineEdit( QWidget* parent, QString nullValue )
    : QLineEdit( parent )
    , mNullValue( nullValue )
{
  btnClear = new QToolButton( this );
  btnClear->setIcon( QgsApplication::getThemeIcon( "/mIconClear.png" ) );
  btnClear->setCursor( Qt::ArrowCursor );
  btnClear->setFocusPolicy( Qt::NoFocus );
  btnClear->setStyleSheet( "QToolButton { border: none; padding: 0px; }" );
  btnClear->hide();

  connect( btnClear, SIGNAL( clicked() ), this, SLOT( clear() ) );
  connect( btnClear, SIGNAL( clicked() ), this, SIGNAL( cleared() ) );
  connect( this, SIGNAL( textChanged( const QString& ) ), this,
           SLOT( toggleClearButton( const QString& ) ) );

  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  setStyleSheet( QString( "QLineEdit { padding-right: %1px; } " )
                 .arg( btnClear->sizeHint().width() + frameWidth + 1 ) );

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax( msz.width(), btnClear->sizeHint().height() + frameWidth * 2 + 2 ),
                  qMax( msz.height(), btnClear->sizeHint().height() + frameWidth * 2 + 2 ) );
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
  if ( !isEnabled() )
    btnClear->setVisible( false );
  else
    btnClear->setVisible( text() != mNullValue );
}

void QgsFilterLineEdit::toggleClearButton( const QString &text )
{
  btnClear->setVisible( !isReadOnly() && text != mNullValue );
}
