/***************************************************************************
    qgsmeshrenderervectorsettingswidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderervectorsettingswidget.h"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"

#include <QIntValidator>

QgsMeshRendererVectorSettingsWidget::QgsMeshRendererVectorSettingsWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  mShaftLengthComboBox->setCurrentIndex( -1 );

  connect( mColorWidget, &QgsColorButton::colorChanged, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mLineWidthSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           mShaftOptionsStackedWidget, &QStackedWidget::setCurrentIndex );

  connect( mDisplayVectorsOnGridGroupBox, &QGroupBox::toggled, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  QVector<QLineEdit *> widgets;
  widgets << mMinMagLineEdit << mMaxMagLineEdit
          << mHeadWidthLineEdit << mHeadLengthLineEdit
          << mMinimumShaftLineEdit << mMaximumShaftLineEdit
          << mScaleShaftByFactorOfLineEdit << mShaftLengthLineEdit;

  for ( auto widget : widgets )
  {
    connect( widget, &QLineEdit::textChanged, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  }

  connect( mXSpacingSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mYSpacingSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
}

void QgsMeshRendererVectorSettingsWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

QgsMeshRendererVectorSettings QgsMeshRendererVectorSettingsWidget::settings() const
{
  QgsMeshRendererVectorSettings settings;

  // basic
  settings.setColor( mColorWidget->color() );
  settings.setLineWidth( mLineWidthSpinBox->value() );

  // filter by magnitude
  double val = filterValue( mMinMagLineEdit->text(), -1 );
  settings.setFilterMin( val );

  val = filterValue( mMaxMagLineEdit->text(), -1 );
  settings.setFilterMax( val );

  // arrow head
  val = filterValue( mHeadWidthLineEdit->text(), settings.arrowHeadWidthRatio() * 100.0 );
  settings.setArrowHeadWidthRatio( val / 100.0 );

  val = filterValue( mHeadLengthLineEdit->text(), settings.arrowHeadLengthRatio() * 100.0 );
  settings.setArrowHeadLengthRatio( val / 100.0 );

  // user grid
  bool enabled = mDisplayVectorsOnGridGroupBox->isChecked();
  settings.setOnUserDefinedGrid( enabled );
  settings.setUserGridCellWidth( mXSpacingSpinBox->value() );
  settings.setUserGridCellHeight( mYSpacingSpinBox->value() );

  // shaft length
  auto method = static_cast<QgsMeshRendererVectorSettings::ArrowScalingMethod>( mShaftLengthComboBox->currentIndex() );
  settings.setShaftLengthMethod( method );

  val = filterValue( mMinimumShaftLineEdit->text(), settings.minShaftLength() );
  settings.setMinShaftLength( val );

  val = filterValue( mMaximumShaftLineEdit->text(), settings.maxShaftLength() );
  settings.setMaxShaftLength( val );

  val = filterValue( mScaleShaftByFactorOfLineEdit->text(), settings.scaleFactor() );
  settings.setScaleFactor( val );

  val = filterValue( mShaftLengthLineEdit->text(), settings.fixedShaftLength() );
  settings.setFixedShaftLength( val );

  return settings;
}

void QgsMeshRendererVectorSettingsWidget::syncToLayer( )
{
  if ( !mMeshLayer )
    return;

  if ( mActiveDatasetGroup < 0 )
    return;

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( mActiveDatasetGroup );

  // basic
  mColorWidget->setColor( settings.color() );
  mLineWidthSpinBox->setValue( settings.lineWidth() );

  // filter by magnitude
  if ( settings.filterMin() > 0 )
  {
    mMinMagLineEdit->setText( QString::number( settings.filterMin() ) );
  }
  if ( settings.filterMax() > 0 )
  {
    mMaxMagLineEdit->setText( QString::number( settings.filterMax() ) );
  }

  // arrow head
  mHeadWidthLineEdit->setText( QString::number( settings.arrowHeadWidthRatio() * 100.0 ) );
  mHeadLengthLineEdit->setText( QString::number( settings.arrowHeadLengthRatio() * 100.0 ) );

  // shaft length
  mShaftLengthComboBox->setCurrentIndex( settings.shaftLengthMethod() );

  mMinimumShaftLineEdit->setText( QString::number( settings.minShaftLength() ) );
  mMaximumShaftLineEdit->setText( QString::number( settings.maxShaftLength() ) );
  mScaleShaftByFactorOfLineEdit->setText( QString::number( settings.scaleFactor() ) );
  mShaftLengthLineEdit->setText( QString::number( settings.fixedShaftLength() ) );

}

double QgsMeshRendererVectorSettingsWidget::filterValue( const QString &text, double errVal ) const
{
  if ( text.isEmpty() )
    return errVal;

  bool ok;
  double val = text.toDouble( &ok );
  if ( !ok )
    return errVal;

  if ( val < 0 )
    return errVal;

  return val;
}
