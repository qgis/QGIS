/***************************************************************************
    qgscharacterselectordialog.cpp - single font character selector dialog

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
#include "qgscharacterselectordialog.h"


QgsCharacterSelectorDialog::QgsCharacterSelectorDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mChar( QChar::Null )
{
  setupUi( this );
  mCharWidget = new CharacterWidget( this );
  mCharSelectScrollArea->setWidget( mCharWidget );
  mCharSelectScrollArea->setVerticalOnly( true );
  connect( mCharWidget, &CharacterWidget::characterSelected, this, &QgsCharacterSelectorDialog::setCharacter );
}

QChar QgsCharacterSelectorDialog::selectCharacter( bool *gotChar, const QFont &font, const QString &style, QChar initialSelection )
{
  mCharSelectLabelFont->setText( QStringLiteral( "%1 %2" ).arg( font.family(), style ) );
  mCharWidget->setFont( font );
  mCharWidget->setFontStyle( style );
  mCharWidget->setFontSize( 22.0 );
  mCharSelectScrollArea->viewport()->update();

  mCharWidget->setCharacter( initialSelection );

  QApplication::setOverrideCursor( Qt::ArrowCursor );
  const int res = exec();
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
