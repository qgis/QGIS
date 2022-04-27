/***************************************************************************
  qgsformlabelformatwidget.cpp - QgsFormLabelFormatWidget

 ---------------------
 begin                : 22.4.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsformlabelformatwidget.h"
#include "qgsguiutils.h"

/// @cond private

QgsFormLabelFormatWidget::QgsFormLabelFormatWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mFontBoldBtn->setChecked( false );
  mFontItalicBtn->setChecked( false );
  mFontUnderlineBtn->setChecked( false );
  mFontStrikethroughBtn->setChecked( false );

  const int buttonSize = QgsGuiUtils::scaleIconSize( 24 );
  mFontUnderlineBtn->setMinimumSize( buttonSize, buttonSize );
  mFontUnderlineBtn->setMaximumSize( buttonSize, buttonSize );
  mFontBoldBtn->setMinimumSize( buttonSize, buttonSize );
  mFontBoldBtn->setMaximumSize( buttonSize, buttonSize );
  mFontItalicBtn->setMinimumSize( buttonSize, buttonSize );
  mFontItalicBtn->setMaximumSize( buttonSize, buttonSize );

  btnTextColor->setAllowOpacity( true );
  btnTextColor->setShowNull( true, tr( "Default color" ) );

}

void QgsFormLabelFormatWidget::setColor( const QColor &color )
{
  btnTextColor->setColor( color );
}

void QgsFormLabelFormatWidget::setFont( const QFont &font )
{
  mFontFamilyCmbBx->setCurrentFont( font );
  mFontUnderlineBtn->setChecked( font.underline() );
  mFontItalicBtn->setChecked( font.italic() );
  mFontBoldBtn->setChecked( font.bold() );
  mFontStrikethroughBtn->setChecked( font.strikeOut() );
}

QColor QgsFormLabelFormatWidget::color() const
{
  return btnTextColor->color();
}

QFont QgsFormLabelFormatWidget::font() const
{
  QFont currentFont;
  currentFont.setFamily( mFontFamilyCmbBx->currentFont().family() );
  currentFont.setBold( mFontBoldBtn->isChecked() );
  currentFont.setItalic( mFontItalicBtn->isChecked() );
  currentFont.setUnderline( mFontUnderlineBtn->isChecked() );
  currentFont.setStrikeOut( mFontStrikethroughBtn->isChecked() );
  return currentFont;
}

/// @endcond private
