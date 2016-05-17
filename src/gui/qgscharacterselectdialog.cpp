/***************************************************************************
    qgscharacterselectdialog.cpp - single font character selector dialog

    ---------------------
    begin                : November 2012
    copyright            : (C) 2012 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "characterwidget.h"
#include "qgscharacterselectdialog.h"


QgsCharacterSelectorDialog::QgsCharacterSelectorDialog( QWidget *parent, const Qt::WindowFlags& fl )
    : QDialog( parent, fl )
    , mChar( QChar::Null )
{
  setupUi( this );
  mCharWidget = new CharacterWidget( this );
  mCharSelectScrollArea->setWidget( mCharWidget );
  connect( mCharWidget, SIGNAL( characterSelected( const QChar & ) ), this, SLOT( setCharacter( const QChar & ) ) );
}

QgsCharacterSelectorDialog::~QgsCharacterSelectorDialog()
{
}

const QChar& QgsCharacterSelectorDialog::selectCharacter( bool* gotChar, const QFont& font, const QString& style )
{
  mCharSelectLabelFont->setText( QString( "%1 %2" ).arg( font.family(), style ) );
  mCharWidget->updateFont( font );
  mCharWidget->updateStyle( style );
  mCharWidget->updateSize( 22.0 );
  mCharSelectScrollArea->viewport()->update();

  QApplication::setOverrideCursor( Qt::ArrowCursor );
  int res = exec();
  QApplication::restoreOverrideCursor();

  if ( res == QDialog::Accepted )
  {
    if ( !mChar.isNull() && gotChar )
    {
      *gotChar = true;
    }
  }
  return mChar;
}

void QgsCharacterSelectorDialog::setCharacter( QChar chr )
{
  mChar = chr;
}
