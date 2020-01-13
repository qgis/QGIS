/***************************************************************************
    qgstableeditorformattingwidget.cpp
    ------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstableeditorformattingwidget.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgsnumericformat.h"

QgsTableEditorFormattingWidget::QgsTableEditorFormattingWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Formatting" ) );

  mFormatNumbersCheckBox->setTristate( false );

  mTextColorButton->setAllowOpacity( true );
  mTextColorButton->setColorDialogTitle( tr( "Text Color" ) );
  mTextColorButton->setDefaultColor( QColor( 0, 0, 0 ) );
  mTextColorButton->setShowNull( true );
  mBackgroundColorButton->setAllowOpacity( true );
  mBackgroundColorButton->setColorDialogTitle( tr( "Text Color" ) );
  mBackgroundColorButton->setDefaultColor( QColor( 255, 255, 255 ) );
  mBackgroundColorButton->setShowNull( true );
  connect( mTextColorButton, &QgsColorButton::colorChanged, this, [ = ]
  {
    if ( !mBlockSignals )
      emit foregroundColorChanged( mTextColorButton->color() );
  } );
  connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this,  [ = ]
  {
    if ( !mBlockSignals )
      emit backgroundColorChanged( mBackgroundColorButton->color() );
  } );

  connect( mFormatNumbersCheckBox, &QCheckBox::stateChanged, this, [ = ]( int state )
  {
    mCustomizeFormatButton->setEnabled( state == Qt::Checked );
    if ( state != Qt::PartiallyChecked )
      mFormatNumbersCheckBox->setTristate( false );
    if ( !mBlockSignals )
      emit numberFormatChanged();
  } );

  mCustomizeFormatButton->setEnabled( false );
  connect( mCustomizeFormatButton, &QPushButton::clicked, this, [ = ]
  {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setFormat( mNumericFormat.get() );
    widget->setPanelTitle( tr( "Number Format" ) );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [ = ]
    {
      mNumericFormat.reset( widget->format() );
      emit numberFormatChanged();
    } );
    openPanel( widget );
  } );
}

QgsNumericFormat *QgsTableEditorFormattingWidget::numericFormat()
{
  if ( !mNumericFormat || mFormatNumbersCheckBox->checkState() != Qt::Checked )
    return nullptr;

  return mNumericFormat->clone();
}

void QgsTableEditorFormattingWidget::setForegroundColor( const QColor &color )
{
  mBlockSignals = true;
  mTextColorButton->setColor( color );
  mBlockSignals = false;
}

void QgsTableEditorFormattingWidget::setBackgroundColor( const QColor &color )
{
  mBlockSignals = true;
  mBackgroundColorButton->setColor( color );
  mBlockSignals = false;
}

void QgsTableEditorFormattingWidget::setNumericFormat( QgsNumericFormat *format, bool isMixedFormat )
{
  mNumericFormat.reset( format ? format->clone() : nullptr );
  mBlockSignals = true;
  mFormatNumbersCheckBox->setTristate( isMixedFormat );
  mFormatNumbersCheckBox->setCheckState( isMixedFormat ? Qt::PartiallyChecked : ( mNumericFormat.get() ? Qt::Checked : Qt::Unchecked ) );
  mBlockSignals = false;
}

QgsTableEditorFormattingWidget::~QgsTableEditorFormattingWidget() = default;
