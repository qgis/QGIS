/***************************************************************************
    qgsmeshvariablestrokewidthtwidget.cpp
    -------------------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshvariablestrokewidthwidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>

#include "qgssettings.h"

QgsMeshVariableStrokeWidthWidget::QgsMeshVariableStrokeWidthWidget(
  const QgsInterpolatedLineWidth &variableStrokeWidth,
  double defaultMinimumvalue,
  double defaultMaximumValue,
  QWidget *parent ):
  QgsPanelWidget( parent ),
  mDefaultMinimumValue( defaultMinimumvalue ),
  mDefaultMaximumValue( defaultMaximumValue )
{
  setupUi( this );

  mValueMinimumSpinBox->setSpecialValueText( QString( ) );
  mValueMinimumSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue );
  mValueMaximumSpinBox->setSpecialValueText( QString( ) );
  mValueMaximumSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue );

  setPanelTitle( tr( "Variable Stroke Width" ) );

  setVariableStrokeWidth( variableStrokeWidth );

  connect( mDefaultMinMaxButton, &QPushButton::clicked, this, &QgsMeshVariableStrokeWidthWidget::defaultMinMax );

  connect( mValueMinimumSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mValueMaximumSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mWidthMinimumSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mWidthMaximumSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mIgnoreOutOfRangecheckBox, &QCheckBox::toggled, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mUseAbsoluteValueCheckBox, &QCheckBox::toggled, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
}

void QgsMeshVariableStrokeWidthWidget::setVariableStrokeWidth( const QgsInterpolatedLineWidth &variableStrokeWidth )
{
  whileBlocking( mValueMinimumSpinBox )->setValue( variableStrokeWidth.minimumValue() );
  whileBlocking( mValueMaximumSpinBox )->setValue( variableStrokeWidth.maximumValue() );
  whileBlocking( mWidthMinimumSpinBox )->setValue( variableStrokeWidth.minimumWidth() );
  whileBlocking( mWidthMaximumSpinBox )->setValue( variableStrokeWidth.maximumWidth() );
  whileBlocking( mIgnoreOutOfRangecheckBox )->setChecked( variableStrokeWidth.ignoreOutOfRange() );
  whileBlocking( mUseAbsoluteValueCheckBox )->setChecked( variableStrokeWidth.useAbsoluteValue() );
}

void QgsMeshVariableStrokeWidthButton::setDefaultMinMaxValue( double minimum, double maximum )
{
  mMinimumDefaultValue = minimum;
  mMaximumDefaultValue = maximum;
}

QgsInterpolatedLineWidth QgsMeshVariableStrokeWidthWidget::variableStrokeWidth() const
{
  QgsInterpolatedLineWidth strokeWidth;
  strokeWidth.setMinimumValue( lineEditValue( mValueMinimumSpinBox ) );
  strokeWidth.setMaximumValue( lineEditValue( mValueMaximumSpinBox ) );
  strokeWidth.setMinimumWidth( mWidthMinimumSpinBox->value() );
  strokeWidth.setMaximumWidth( mWidthMaximumSpinBox->value() );
  strokeWidth.setIgnoreOutOfRange( mIgnoreOutOfRangecheckBox->isChecked() );
  strokeWidth.setUseAbsoluteValue( mUseAbsoluteValueCheckBox->isChecked() );
  return strokeWidth;
}

void QgsMeshVariableStrokeWidthWidget::defaultMinMax()
{
  whileBlocking( mValueMinimumSpinBox )->setValue( mDefaultMinimumValue );
  whileBlocking( mValueMaximumSpinBox )->setValue( mDefaultMaximumValue );
  emit widgetChanged();
}

QgsMeshVariableStrokeWidthButton::QgsMeshVariableStrokeWidthButton( QWidget *parent ): QPushButton( parent )
{
  updateText();
  connect( this, &QPushButton::clicked, this, &QgsMeshVariableStrokeWidthButton::openWidget );
}

QgsInterpolatedLineWidth QgsMeshVariableStrokeWidthButton::variableStrokeWidth() const
{
  return mVariableStrokeWidth;
}

void QgsMeshVariableStrokeWidthButton::setVariableStrokeWidth( const QgsInterpolatedLineWidth &variableStrokeWidth )
{
  mVariableStrokeWidth = variableStrokeWidth;
  updateText();
}

void QgsMeshVariableStrokeWidthButton::openWidget()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  QgsMeshVariableStrokeWidthWidget *widget =
    new QgsMeshVariableStrokeWidthWidget( mVariableStrokeWidth,
                                          mMinimumDefaultValue,
                                          mMaximumDefaultValue,
                                          panel );

  if ( panel && panel->dockMode() )
  {
    connect( widget, &QgsMeshVariableStrokeWidthWidget::widgetChanged, this, [this, widget]
    {
      // Update strokeWidth toward button
      this->setVariableStrokeWidth( widget->variableStrokeWidth() );
      this->emit widgetChanged();
    } );

    panel->openPanel( widget );
    return;
  }
  else
  {
    // Show the dialog version if not in a panel
    QDialog *dlg = new QDialog( this );
    const QString key = QStringLiteral( "/UI/paneldialog/%1" ).arg( widget->panelTitle() );
    QgsSettings settings;
    dlg->restoreGeometry( settings.value( key ).toByteArray() );
    dlg->setWindowTitle( widget->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( widget );
    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
    connect( buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject );
    dlg->layout()->addWidget( buttonBox );

    if ( dlg->exec() == QDialog::Accepted )
    {
      this->setVariableStrokeWidth( widget->variableStrokeWidth() );
      emit widgetChanged();
    }
    settings.setValue( key, dlg->saveGeometry() );
  }
}

void QgsMeshVariableStrokeWidthButton::updateText()
{
  setText( QString( "%1 - %2" ).
           arg( QLocale().toString( mVariableStrokeWidth.minimumWidth(), 'g', 3 ),
                QLocale().toString( mVariableStrokeWidth.maximumWidth(), 'g', 3 ) ) );
}

double QgsMeshVariableStrokeWidthWidget::lineEditValue( const QgsDoubleSpinBox *lineEdit ) const
{
  if ( lineEdit->value() == lineEdit->clearValue() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->value();
}

