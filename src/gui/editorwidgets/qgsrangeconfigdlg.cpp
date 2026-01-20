/***************************************************************************
    qgsrangeconfigdlgbase.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrangeconfigdlg.h"

#include "qgsrangewidgetwrapper.h"
#include "qgsvectorlayer.h"

#include "moc_qgsrangeconfigdlg.cpp"

QgsRangeConfigDlg::QgsRangeConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  precisionSpinBox->setClearValue( 4 );
  setPrecision( precisionSpinBox->value() );

  minimumSpinBox->setMinimum( std::numeric_limits<int>::lowest() );
  minimumSpinBox->setMaximum( std::numeric_limits<int>::max() );
  minimumSpinBox->setValue( std::numeric_limits<int>::lowest() );

  maximumSpinBox->setMinimum( std::numeric_limits<int>::lowest() );
  maximumSpinBox->setMaximum( std::numeric_limits<int>::max() );
  maximumSpinBox->setValue( std::numeric_limits<int>::max() );

  stepSpinBox->setMaximum( std::numeric_limits<int>::max() );
  stepSpinBox->setValue( 1 );
  stepSpinBox->setClearValue( 1 );

  minimumDoubleSpinBox->setMinimum( std::numeric_limits<double>::lowest() );
  minimumDoubleSpinBox->setMaximum( std::numeric_limits<double>::max() );
  minimumDoubleSpinBox->setValue( std::numeric_limits<double>::min() );

  maximumDoubleSpinBox->setMinimum( std::numeric_limits<double>::lowest() );
  maximumDoubleSpinBox->setMaximum( std::numeric_limits<double>::max() );
  maximumDoubleSpinBox->setValue( std::numeric_limits<double>::max() );

  // Use integer here:
  stepDoubleSpinBox->setMaximum( std::numeric_limits<int>::max() );
  stepDoubleSpinBox->setValue( 1 );
  stepDoubleSpinBox->setClearValue( 1 );


  QString text;

  const QMetaType::Type fieldType( vl->fields().at( fieldIdx ).type() );

  switch ( fieldType )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
    {
      // we use the double spin boxes for double OR long long field types, as QSpinBox does not have sufficient
      // available range for long long values
      rangeStackedWidget->setCurrentIndex( fieldType == QMetaType::Type::Int ? 0 : 1 );
      if ( fieldType == QMetaType::Type::LongLong )
      {
        minimumDoubleSpinBox->setDecimals( 0 );
        maximumDoubleSpinBox->setDecimals( 0 );
        stepDoubleSpinBox->setDecimals( 0 );
      }

      rangeWidget->clear();
      rangeWidget->addItem( tr( "Editable" ), u"SpinBox"_s );
      rangeWidget->addItem( tr( "Slider" ), u"Slider"_s );
      rangeWidget->addItem( tr( "Dial" ), u"Dial"_s );

      QVariant min;
      QVariant max;
      vl->minimumAndMaximumValue( fieldIdx, min, max );

      text = tr( "Current minimum for this value is %1 and current maximum is %2." ).arg( min.toString(), max.toString() );
      break;
    }

    default:
      text = tr( "Attribute has no integer or real type, therefore range is not usable." );
      break;
  }

  // Hide precision for integer types
  if ( fieldType != QMetaType::Type::Double )
  {
    precisionSpinBox->hide();
    precisionLabel->hide();
  }

  valuesLabel->setText( text );

  connect( rangeWidget, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRangeConfigDlg::rangeWidgetChanged );
  connect( minimumSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( maximumSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( stepSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( minimumDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( maximumDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( stepDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( rangeWidget, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( allowNullCheckBox, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( suffixLineEdit, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( precisionSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsRangeConfigDlg::setPrecision );
}

QVariantMap QgsRangeConfigDlg::config()
{
  QVariantMap cfg;

  switch ( layer()->fields().at( field() ).type() )
  {
    case QMetaType::Type::Int:
      cfg.insert( u"Min"_s, minimumSpinBox->value() );
      cfg.insert( u"Max"_s, maximumSpinBox->value() );
      cfg.insert( u"Step"_s, stepSpinBox->value() );
      break;

    // we use the double spin boxes for double OR long long field types, as QSpinBox does not have sufficient
    // available range for long long values
    case QMetaType::Type::Double:
    case QMetaType::Type::LongLong:
      cfg.insert( u"Min"_s, minimumDoubleSpinBox->value() );
      cfg.insert( u"Max"_s, maximumDoubleSpinBox->value() );
      cfg.insert( u"Step"_s, stepDoubleSpinBox->value() );
      break;

    default:
      break;
  }

  cfg.insert( u"Style"_s, rangeWidget->currentData().toString() );
  cfg.insert( u"AllowNull"_s, allowNullCheckBox->isChecked() );
  cfg.insert( u"Precision"_s, precisionSpinBox->value() );

  if ( !suffixLineEdit->text().isEmpty() )
  {
    cfg.insert( u"Suffix"_s, suffixLineEdit->text() );
  }

  return cfg;
}

void QgsRangeConfigDlg::setConfig( const QVariantMap &config )
{
  minimumDoubleSpinBox->setValue( config.value( u"Min"_s, std::numeric_limits<double>::lowest() ).toDouble() );
  maximumDoubleSpinBox->setValue( config.value( u"Max"_s, std::numeric_limits<double>::max() ).toDouble() );
  stepDoubleSpinBox->setValue( config.value( u"Step"_s, 1.0 ).toDouble() );
  minimumSpinBox->setValue( config.value( u"Min"_s, std::numeric_limits<int>::lowest() ).toInt() );
  maximumSpinBox->setValue( config.value( u"Max"_s, std::numeric_limits<int>::max() ).toInt() );
  stepSpinBox->setValue( config.value( u"Step"_s, 1 ).toInt() );
  rangeWidget->setCurrentIndex( rangeWidget->findData( config.value( u"Style"_s, "SpinBox" ) ) );
  suffixLineEdit->setText( config.value( u"Suffix"_s ).toString() );
  allowNullCheckBox->setChecked( config.value( u"AllowNull"_s, true ).toBool() );

  const QgsField layerField = layer()->fields().at( field() );
  const int fieldPrecision = QgsRangeWidgetWrapper::defaultFieldPrecision( layerField );
  precisionSpinBox->setValue( config.value( u"Precision"_s, fieldPrecision ).toInt() );
}

void QgsRangeConfigDlg::rangeWidgetChanged( int index )
{
  const QString style = rangeWidget->itemData( index ).toString();
  allowNullCheckBox->setEnabled( style == "SpinBox"_L1 );
}

void QgsRangeConfigDlg::setPrecision( int precision )
{
  minimumDoubleSpinBox->setDecimals( precision );
  maximumDoubleSpinBox->setDecimals( precision );
  stepDoubleSpinBox->setDecimals( precision );
}
