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
  setPanelTitle( tr( "Variable Stroke Width" ) );

  setVariableStrokeWidth( variableStrokeWidth );

  connect( mDefaultMinMaxButton, &QPushButton::clicked, this, &QgsMeshVariableStrokeWidthWidget::defaultMinMax );

  connect( mValueMinimumLineEdit, &QLineEdit::textEdited, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mValueMaximumLineEdit, &QLineEdit::textEdited, this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mWidthMinimumSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mWidthMaximumSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
  connect( mIgnoreOutOfRangecheckBox, &QCheckBox::toggled,
           this, &QgsMeshVariableStrokeWidthWidget::widgetChanged );
}

void QgsMeshVariableStrokeWidthWidget::setVariableStrokeWidth( const QgsInterpolatedLineWidth &variableStrokeWidth )
{
  whileBlocking( mValueMinimumLineEdit )->setText( QString::number( variableStrokeWidth.minimumValue() ) );
  whileBlocking( mValueMaximumLineEdit )->setText( QString::number( variableStrokeWidth.maximumValue() ) );
  whileBlocking( mWidthMinimumSpinBox )->setValue( variableStrokeWidth.minimumWidth() );
  whileBlocking( mWidthMaximumSpinBox )->setValue( variableStrokeWidth.maximumWidth() );
  whileBlocking( mIgnoreOutOfRangecheckBox )->setChecked( variableStrokeWidth.ignoreOutOfRange() );
}

void QgsMeshVariableStrokeWidthButton::setDefaultMinMaxValue( double minimum, double maximum )
{
  mMinimumDefaultValue = minimum;
  mMaximumDefaultValue = maximum;
}

QgsInterpolatedLineWidth QgsMeshVariableStrokeWidthWidget::variableStrokeWidth() const
{
  QgsInterpolatedLineWidth strokeWidth;
  strokeWidth.setMinimumValue( lineEditValue( mValueMinimumLineEdit ) );
  strokeWidth.setMaximumValue( lineEditValue( mValueMaximumLineEdit ) );
  strokeWidth.setMinimumWidth( mWidthMinimumSpinBox->value() );
  strokeWidth.setMaximumWidth( mWidthMaximumSpinBox->value() );
  strokeWidth.setIgnoreOutOfRange( mIgnoreOutOfRangecheckBox->isChecked() );

  return strokeWidth;
}

void QgsMeshVariableStrokeWidthWidget::defaultMinMax()
{
  whileBlocking( mValueMinimumLineEdit )->setText( QString::number( mDefaultMinimumValue ) );
  whileBlocking( mValueMaximumLineEdit )->setText( QString::number( mDefaultMaximumValue ) );
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
    QString key = QStringLiteral( "/UI/paneldialog/%1" ).arg( widget->panelTitle() );
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
           arg( QString::number( mVariableStrokeWidth.minimumWidth(), 'g', 3 ) ).
           arg( QString::number( mVariableStrokeWidth.maximumWidth(), 'g', 3 ) ) );
}

double QgsMeshVariableStrokeWidthWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

