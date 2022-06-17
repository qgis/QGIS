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
#include <QGroupBox>

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

  mOverrideLabelFormatGroupBox->setSaveCheckedState( false );
  mOverrideLabelFormatGroupBox->setSaveCollapsedState( false );

  btnTextColor->setAllowOpacity( true );
  btnTextColor->setShowNull( true, tr( "Default color" ) );

}

void QgsFormLabelFormatWidget::setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle )
{
  mFontFamilyCmbBx->setCurrentFont( labelStyle.font );
  mFontUnderlineBtn->setChecked( labelStyle.font.underline() );
  mFontItalicBtn->setChecked( labelStyle.font.italic() );
  mFontBoldBtn->setChecked( labelStyle.font.bold() );
  mFontStrikethroughBtn->setChecked( labelStyle.font.strikeOut() );
  if ( labelStyle.color.isValid() )
  {
    btnTextColor->setColor( labelStyle.color );
  }
  else
  {
    btnTextColor->setToNull();
  }
  mOverrideLabelFormatGroupBox->setChecked( labelStyle.overrideFont || labelStyle.overrideColor );
  mOverrideLabelFormatGroupBox->setCollapsed( !( labelStyle.overrideFont || labelStyle.overrideColor ) );
}

QgsAttributeEditorElement::LabelStyle QgsFormLabelFormatWidget::labelStyle() const
{
  QgsAttributeEditorElement::LabelStyle style;
  style.color = btnTextColor->color();
  QFont currentFont;
  currentFont.setFamily( mFontFamilyCmbBx->currentFont().family() );
  currentFont.setBold( mFontBoldBtn->isChecked() );
  currentFont.setItalic( mFontItalicBtn->isChecked() );
  currentFont.setUnderline( mFontUnderlineBtn->isChecked() );
  currentFont.setStrikeOut( mFontStrikethroughBtn->isChecked() );
  style.font = currentFont;
  style.overrideColor = mOverrideLabelFormatGroupBox->isChecked( );
  style.overrideFont = mOverrideLabelFormatGroupBox->isChecked( );
  return style;
}

/// @endcond private
