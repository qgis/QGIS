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

#include "qgsvectorlayer.h"

QgsRangeConfigDlg::QgsRangeConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  QString text;

  switch ( vl->fields().at( fieldIdx ).type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
    {
      rangeStackedWidget->setCurrentIndex( vl->fields().at( fieldIdx ).type() == QVariant::Double ? 1 : 0 );

      rangeWidget->clear();
      rangeWidget->addItem( tr( "Editable" ), "SpinBox" );
      rangeWidget->addItem( tr( "Slider" ), "Slider" );
      rangeWidget->addItem( tr( "Dial" ), "Dial" );

      QVariant min = vl->minimumValue( fieldIdx );
      QVariant max = vl->maximumValue( fieldIdx );

      text = tr( "Current minimum for this value is %1 and current maximum is %2." ).arg( min.toString(), max.toString() );
      break;
    }

    default:
      text = tr( "Attribute has no integer or real type, therefore range is not usable." );
      break;
  }

  valuesLabel->setText( text );

  connect( rangeWidget, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rangeWidgetChanged( int ) ) );

  connect( minimumSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( changed() ) );
  connect( maximumSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( changed() ) );
  connect( stepSpinBox, SIGNAL( valueChanged( int ) ), this, SIGNAL( changed() ) );
  connect( minimumDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SIGNAL( changed() ) );
  connect( maximumDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SIGNAL( changed() ) );
  connect( stepDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SIGNAL( changed() ) );
  connect( rangeWidget, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( changed() ) );
  connect( allowNullCheckBox, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( suffixLineEdit, SIGNAL( textChanged( QString ) ), this, SIGNAL( changed() ) );
}

QgsEditorWidgetConfig QgsRangeConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  switch ( layer()->fields().at( field() ).type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
      cfg.insert( QStringLiteral( "Min" ), minimumSpinBox->value() );
      cfg.insert( QStringLiteral( "Max" ), maximumSpinBox->value() );
      cfg.insert( QStringLiteral( "Step" ), stepSpinBox->value() );
      break;

    case QVariant::Double:
      cfg.insert( QStringLiteral( "Min" ), minimumDoubleSpinBox->value() );
      cfg.insert( QStringLiteral( "Max" ), maximumDoubleSpinBox->value() );
      cfg.insert( QStringLiteral( "Step" ), stepDoubleSpinBox->value() );
      break;

    default:
      break;
  }

  cfg.insert( QStringLiteral( "Style" ), rangeWidget->currentData().toString() );
  cfg.insert( QStringLiteral( "AllowNull" ), allowNullCheckBox->isChecked() );

  if ( suffixLineEdit->text() != QLatin1String( "" ) )
  {
    cfg.insert( QStringLiteral( "Suffix" ), suffixLineEdit->text() );
  }

  return cfg;
}

void QgsRangeConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  minimumDoubleSpinBox->setValue( config.value( QStringLiteral( "Min" ), -std::numeric_limits<double>::max() ).toDouble() );
  maximumDoubleSpinBox->setValue( config.value( QStringLiteral( "Max" ), std::numeric_limits<double>::max() ).toDouble() );
  stepDoubleSpinBox->setValue( config.value( QStringLiteral( "Step" ), 1.0 ).toDouble() );

  minimumSpinBox->setValue( config.value( QStringLiteral( "Min" ), std::numeric_limits<int>::min() ).toInt() );
  maximumSpinBox->setValue( config.value( QStringLiteral( "Max" ), std::numeric_limits<int>::max() ).toInt() );
  stepSpinBox->setValue( config.value( QStringLiteral( "Step" ), 1 ).toInt() );

  rangeWidget->setCurrentIndex( rangeWidget->findData( config.value( QStringLiteral( "Style" ), "SpinBox" ) ) );

  suffixLineEdit->setText( config.value( QStringLiteral( "Suffix" ) ).toString() );

  allowNullCheckBox->setChecked( config.value( QStringLiteral( "AllowNull" ), true ).toBool() );
}

void QgsRangeConfigDlg::rangeWidgetChanged( int index )
{
  QString style = rangeWidget->itemData( index ).toString();
  allowNullCheckBox->setEnabled( style == QLatin1String( "SpinBox" ) );
}
