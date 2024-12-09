/***************************************************************************
                             qgsprocessingfeaturesourceoptionswidget.cpp
                             ------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingfeaturesourceoptionswidget.h"
#include "moc_qgsprocessingfeaturesourceoptionswidget.cpp"
#include "qgis.h"

///@cond NOT_STABLE

QgsProcessingFeatureSourceOptionsWidget::QgsProcessingFeatureSourceOptionsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mFeatureLimitSpinBox->setClearValue( 0, tr( "Not set" ) );
  mFeatureLimitSpinBox->clear();

  mComboInvalidFeatureFiltering->addItem( tr( "Use Default" ) );
  mComboInvalidFeatureFiltering->addItem( tr( "Do not Filter (Better Performance)" ), QVariant::fromValue( Qgis::InvalidGeometryCheck::NoCheck ) );
  mComboInvalidFeatureFiltering->addItem( tr( "Skip (Ignore) Features with Invalid Geometries" ), QVariant::fromValue( Qgis::InvalidGeometryCheck::SkipInvalid ) );
  mComboInvalidFeatureFiltering->addItem( tr( "Stop Algorithm Execution When a Geometry is Invalid" ), QVariant::fromValue( Qgis::InvalidGeometryCheck::AbortOnInvalid ) );

  connect( mFeatureLimitSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mComboInvalidFeatureFiltering, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mFilterExpressionWidget, &QgsExpressionLineEdit::expressionChanged, this, &QgsPanelWidget::widgetChanged );
}

void QgsProcessingFeatureSourceOptionsWidget::setLayer( QgsVectorLayer *layer )
{
  mFilterExpressionWidget->setLayer( layer );
}

void QgsProcessingFeatureSourceOptionsWidget::setGeometryCheckMethod( bool isOverridden, Qgis::InvalidGeometryCheck check )
{
  if ( !isOverridden )
    mComboInvalidFeatureFiltering->setCurrentIndex( mComboInvalidFeatureFiltering->findData( QVariant() ) );
  else
    mComboInvalidFeatureFiltering->setCurrentIndex( mComboInvalidFeatureFiltering->findData( QVariant::fromValue( check ) ) );
}

void QgsProcessingFeatureSourceOptionsWidget::setFeatureLimit( int limit )
{
  mFeatureLimitSpinBox->setValue( limit );
}

void QgsProcessingFeatureSourceOptionsWidget::setFilterExpression( const QString &expression )
{
  mFilterExpressionWidget->setExpression( expression );
}

Qgis::InvalidGeometryCheck QgsProcessingFeatureSourceOptionsWidget::geometryCheckMethod() const
{
  return mComboInvalidFeatureFiltering->currentData().isValid() ? mComboInvalidFeatureFiltering->currentData().value<Qgis::InvalidGeometryCheck>() : Qgis::InvalidGeometryCheck::AbortOnInvalid;
}

bool QgsProcessingFeatureSourceOptionsWidget::isOverridingInvalidGeometryCheck() const
{
  return mComboInvalidFeatureFiltering->currentData().isValid();
}

int QgsProcessingFeatureSourceOptionsWidget::featureLimit() const
{
  return mFeatureLimitSpinBox->value() > 0 ? mFeatureLimitSpinBox->value() : -1;
}

QString QgsProcessingFeatureSourceOptionsWidget::filterExpression() const
{
  return mFilterExpressionWidget->expression();
}

///@endcond
