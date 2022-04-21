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
#include "qgsfractionnumericformat.h"
#include "qgscoordinatenumericformat.h"
#include "qgsgui.h"
#include "qgis.h"
#include <QDialogButtonBox>

//
// QgsBasicNumericFormatWidget
//
QgsBasicNumericFormatWidget::QgsBasicNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );
  setFormat( format->clone() );

  mDecimalsSpinBox->setClearValue( 6 );
  mThousandsLineEdit->setShowClearButton( true );
  mDecimalLineEdit->setShowClearButton( true );

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

  connect( mDecimalsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mRadDecimalPlaces, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( !checked )
      return;

    mFormat->setRoundingType( QgsBasicNumericFormat::DecimalPlaces );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mRadSignificantFigures, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( !checked )
      return;

    mFormat->setRoundingType( QgsBasicNumericFormat::SignificantFigures );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mThousandsLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    mFormat->setThousandsSeparator( text.isEmpty() ? QChar() : text.at( 0 ) );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    mFormat->setDecimalSeparator( text.isEmpty() ? QChar() : text.at( 0 ) );
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
  mThousandsLineEdit->setText( mFormat->thousandsSeparator().isNull() ? QString() : mFormat->thousandsSeparator() );
  mDecimalLineEdit->setText( mFormat->decimalSeparator().isNull() ? QString() : mFormat->decimalSeparator() );
  switch ( mFormat->roundingType() )
  {
    case QgsBasicNumericFormat::DecimalPlaces:
      mRadDecimalPlaces->setChecked( true );
      break;

    case QgsBasicNumericFormat::SignificantFigures:
      mRadSignificantFigures->setChecked( true );
      break;
  }

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

  mDecimalsSpinBox->setClearValue( 6 );
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

  connect( mDecimalsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
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
// QgsBearingNumericFormatDialog
//

QgsBearingNumericFormatDialog::QgsBearingNumericFormatDialog( const QgsNumericFormat *format, QWidget *parent )
  : QDialog( parent )
{
  setLayout( new QVBoxLayout() );
  mWidget = new QgsBearingNumericFormatWidget( format );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  layout()->addWidget( mWidget );
  layout()->addWidget( buttonBox );

  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  setObjectName( QStringLiteral( "QgsBearingNumericFormatDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );
}

QgsBearingNumericFormat *QgsBearingNumericFormatDialog::format()
{
  return static_cast< QgsBearingNumericFormat * >( mWidget->format() );
}





//
// QgsGeographicCoordinateNumericFormatWidget
//

QgsGeographicCoordinateNumericFormatWidget::QgsGeographicCoordinateNumericFormatWidget( const QgsNumericFormat *format, bool hidePrecisionControl, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );

  mDecimalsSpinBox->setClearValue( 6 );
  mFormatComboBox->addItem( QObject::tr( "Decimal Degrees" ), static_cast< int >( QgsGeographicCoordinateNumericFormat::AngleFormat::DecimalDegrees ) );
  mFormatComboBox->addItem( QObject::tr( "Degrees, Minutes" ), static_cast< int >( QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutes ) );
  mFormatComboBox->addItem( QObject::tr( "Degrees, Minutes, Seconds" ), static_cast< int >( QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutesSeconds ) );

  if ( hidePrecisionControl )
  {
    mLabelDecimalPlaces->hide();
    mDecimalsSpinBox->hide();
  }
  setFormat( format->clone() );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowDirectionalSuffixCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowDirectionalSuffix( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowLeadingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowLeadingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowLeadingZerosForDegreesCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowDegreeLeadingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    mFormat->setAngleFormat( static_cast < QgsGeographicCoordinateNumericFormat::AngleFormat >( mFormatComboBox->currentData().toInt() ) );
    if ( !mBlockSignals )
      emit changed();
  } );
}

QgsGeographicCoordinateNumericFormatWidget::~QgsGeographicCoordinateNumericFormatWidget() = default;

void QgsGeographicCoordinateNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsGeographicCoordinateNumericFormat * >( format ) );

  mBlockSignals = true;
  mDecimalsSpinBox->setValue( mFormat->numberDecimalPlaces() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mShowDirectionalSuffixCheckBox->setChecked( mFormat->showDirectionalSuffix() );
  mShowLeadingZerosCheckBox->setChecked( mFormat->showLeadingZeros() );
  mShowLeadingZerosForDegreesCheckBox->setChecked( mFormat->showDegreeLeadingZeros() );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( mFormat->angleFormat() ) ) );
  mBlockSignals = false;
}

QgsNumericFormat *QgsGeographicCoordinateNumericFormatWidget::format()
{
  return mFormat->clone();
}

//
// QgsGeographicCoordinateNumericFormatDialog
//

QgsGeographicCoordinateNumericFormatDialog::QgsGeographicCoordinateNumericFormatDialog( const QgsNumericFormat *format, bool hidePrecisionControl, QWidget *parent )
  : QDialog( parent )
{
  setLayout( new QVBoxLayout() );
  mWidget = new QgsGeographicCoordinateNumericFormatWidget( format, hidePrecisionControl );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  layout()->addWidget( mWidget );
  layout()->addWidget( buttonBox );

  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  setObjectName( QStringLiteral( "QgsGeographicCoordinateNumericFormatDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );
}

QgsGeographicCoordinateNumericFormat *QgsGeographicCoordinateNumericFormatDialog::format()
{
  return static_cast< QgsGeographicCoordinateNumericFormat * >( mWidget->format() );
}




//
// QgsCurrencyNumericFormatWidget
//
QgsCurrencyNumericFormatWidget::QgsCurrencyNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );
  mDecimalsSpinBox->setClearValue( 2 );
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

  connect( mDecimalsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
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

  mDecimalsSpinBox->setClearValue( 6 );
  mScalingComboBox->addItem( QObject::tr( "Values are Percentages (e.g. 50)" ), QgsPercentageNumericFormat::ValuesArePercentage );
  mScalingComboBox->addItem( QObject::tr( "Values are Fractions (e.g. 0.5)" ), QgsPercentageNumericFormat::ValuesAreFractions );

  setFormat( format->clone() );

  connect( mShowTrailingZerosCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowTrailingZeros( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowPlusCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowPlusSign( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowThousandsCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowThousandsSeparator( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mDecimalsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
  {
    mFormat->setNumberDecimalPlaces( value );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mScalingComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
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
  mShowPlusCheckBox->setChecked( mFormat->showPlusSign() );
  mShowTrailingZerosCheckBox->setChecked( mFormat->showTrailingZeros() );
  mShowThousandsCheckBox->setChecked( mFormat->showThousandsSeparator() );
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
  mDecimalsSpinBox->setClearValue( 6 );
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

  connect( mDecimalsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int value )
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



//
// QgsFractionNumericFormatWidget
//
QgsFractionNumericFormatWidget::QgsFractionNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent )
  : QgsNumericFormatWidget( parent )
{
  setupUi( this );
  setFormat( format->clone() );

  mThousandsLineEdit->setShowClearButton( true );

  connect( mUseDedicatedUnicodeCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setUseDedicatedUnicodeCharacters( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mUseUnicodeSupersubscriptCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setUseUnicodeSuperSubscript( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowPlusCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowPlusSign( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mShowThousandsCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFormat->setShowThousandsSeparator( checked );
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mThousandsLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    mFormat->setThousandsSeparator( text.isEmpty() ? QChar() : text.at( 0 ) );
    if ( !mBlockSignals )
      emit changed();
  } );

}

QgsFractionNumericFormatWidget::~QgsFractionNumericFormatWidget() = default;

void QgsFractionNumericFormatWidget::setFormat( QgsNumericFormat *format )
{
  mFormat.reset( static_cast< QgsFractionNumericFormat * >( format ) );

  mBlockSignals = true;
  mUseDedicatedUnicodeCheckBox->setChecked( mFormat->useDedicatedUnicodeCharacters() );
  mUseUnicodeSupersubscriptCheckBox->setChecked( mFormat->useUnicodeSuperSubscript() );
  mShowPlusCheckBox->setChecked( mFormat->showPlusSign() );
  mShowThousandsCheckBox->setChecked( mFormat->showThousandsSeparator() );
  mThousandsLineEdit->setText( mFormat->thousandsSeparator().isNull() ? QString() : mFormat->thousandsSeparator() );
  mBlockSignals = false;
}

QgsNumericFormat *QgsFractionNumericFormatWidget::format()
{
  return mFormat->clone();
}
