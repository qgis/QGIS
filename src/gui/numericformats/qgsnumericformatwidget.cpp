/***************************************************************************
    qgsnumericformatwidget.cpp
    --------------------------
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

#include "qgsnumericformatwidget.h"
#include "qgsbasicnumericformat.h"
#include "qgscurrencynumericformat.h"
#include "qgspercentagenumericformat.h"
#include "qgsbearingnumericformat.h"
#include "qgsscientificnumericformat.h"
#include "qgis.h"

//
// QgsBasicNumericFormatWidget
//
QgsBasicNumericFormatWidget::QgsBasicNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );
  setFormat( format->clone() );

  connect( mShowPlusCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowPlusSign( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowThousandsCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowThousandsSeparator( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );
}

QgsBasicNumericFormatWidget::~QgsBasicNumericFormatWidget() = default;

void QgsBasicNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsBasicNumericFormat * >( format ) );

  mBlockSignals = true;
  mDecimalsSpinBox->setValue( mFormat->numberDecimalPlaces() );
  mShowPlusCheckBox->setChecked( mFormat->showPlusSign() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mShowThousandsCheckBox->setChecked( mFormat->showThousandsSeparator() );
  mBlockSignals = false;
}

QgsNumericFormat *QgsBasicNumericFormatWidget::format()
{
  return mFormat->clone();
}

//
// QgsBearingNumericFormatWidget
//

QgsBearingNumericFormatWidget::QgsBearingNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );

  mFormatComboBox->addItem( QObject::tr( "0 to 180°, with E/W suffix" ), QgsBearingNumericFormat::UseRange0To180WithEWDirectionalSuffix );
  mFormatComboBox->addItem( QObject::tr( "-180 to +180°" ), QgsBearingNumericFormat::UseRangeNegative180ToPositive180 );
  mFormatComboBox->addItem( QObject::tr( "0 to 360°" ), QgsBearingNumericFormat::UseRange0To360 );

  setFormat( format->clone() );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mFormatComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    mFormat->setDirectionFormat( static_cast < QgsBearingNumericFormat::FormatDirectionOption >( mFormatComboBox->currentData().toInt() ) );
    if ( !mBlockSignals )
      emit changed();
  } );
}

QgsBearingNumericFormatWidget::~QgsBearingNumericFormatWidget() = default;

void QgsBearingNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsBearingNumericFormat * >( format ) );

  mBlockSignals = true;
  mDecimalsSpinBox->setValue( mFormat->numberDecimalPlaces() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( mFormat->directionFormat() ) ) );
  mBlockSignals = false;
}

QgsNumericFormat *QgsBearingNumericFormatWidget::format()
{
  return mFormat->clone();
}


//
// QgsCurrencyNumericFormatWidget
//
QgsCurrencyNumericFormatWidget::QgsCurrencyNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );
  setFormat( format->clone() );

  connect( mShowPlusCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowPlusSign( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowThousandsCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowThousandsSeparator( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mPrefixLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    mFormat->setPrefix( text );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mSuffixLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    mFormat->setSuffix( text );
    if ( !mBlockSignals )
      emit changed();
  } );
}

QgsCurrencyNumericFormatWidget::~QgsCurrencyNumericFormatWidget() = default;

void QgsCurrencyNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsCurrencyNumericFormat * >( format ) );

  mBlockSignals = true;
  mDecimalsSpinBox->setValue( mFormat->numberDecimalPlaces() );
  mShowPlusCheckBox->setChecked( mFormat->showPlusSign() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mShowThousandsCheckBox->setChecked( mFormat->showThousandsSeparator() );
  mPrefixLineEdit->setText( mFormat->prefix() );
  mSuffixLineEdit->setText( mFormat->suffix() );

  mBlockSignals = false;
}

QgsNumericFormat *QgsCurrencyNumericFormatWidget::format()
{
  return mFormat->clone();
}


//
// QgsPercentageNumericFormatWidget
//

QgsPercentageNumericFormatWidget::QgsPercentageNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );

  mScalingComboBox->addItem( QObject::tr( "Values are Percentages (e.g. 50)" ), QgsPercentageNumericFormat::ValuesArePercentage );
  mScalingComboBox->addItem( QObject::tr( "Values are Fractions (e.g. 0.5)" ), QgsPercentageNumericFormat::ValuesAreFractions );

  setFormat( format->clone() );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mScalingComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    mFormat->setInputValues( static_cast < QgsPercentageNumericFormat::InputValues >( mScalingComboBox->currentData().toInt() ) );
    if ( !mBlockSignals )
      emit changed();
  } );
}

QgsPercentageNumericFormatWidget::~QgsPercentageNumericFormatWidget() = default;

void QgsPercentageNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsPercentageNumericFormat * >( format ) );

  mBlockSignals = true;
  mDecimalsSpinBox->setValue( mFormat->numberDecimalPlaces() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mScalingComboBox->setCurrentIndex( mScalingComboBox->findData( static_cast< int >( mFormat->inputValues() ) ) );
  mBlockSignals = false;
}

QgsNumericFormat *QgsPercentageNumericFormatWidget::format()
{
  return mFormat->clone();
}

//
// QgsScientificNumericFormatWidget
//
QgsScientificNumericFormatWidget::QgsScientificNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );
  setFormat( format->clone() );

  connect( mShowPlusCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowPlusSign( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );
}

QgsScientificNumericFormatWidget::~QgsScientificNumericFormatWidget() = default;

void QgsScientificNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsScientificNumericFormat * >( format ) );

  mBlockSignals = true;
  mDecimalsSpinBox->setValue( mFormat->numberDecimalPlaces() );
  mShowPlusCheckBox->setChecked( mFormat->showPlusSign() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mBlockSignals = false;
}

QgsNumericFormat *QgsScientificNumericFormatWidget::format()
{
  return mFormat->clone();
}

