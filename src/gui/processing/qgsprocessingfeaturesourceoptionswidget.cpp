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
#include "qgis.h"

///@cond NOT_STABLE

QgsProcessingFeatureSourceOptionsWidget::QgsProcessingFeatureSourceOptionsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mFeatureLimitSpinBox->setClearValue( 0, tr( "Not set" ) );
  mFeatureLimitSpinBox->clear();

  mComboInvalidFeatureFiltering->addItem( tr( "Use Default" ) );
  mComboInvalidFeatureFiltering->addItem( tr( "Do not Filter (Better Performance)" ), QgsFeatureRequest::GeometryNoCheck );
  mComboInvalidFeatureFiltering->addItem( tr( "Skip (Ignore) Features with Invalid Geometries" ), QgsFeatureRequest::GeometrySkipInvalid );
  mComboInvalidFeatureFiltering->addItem( tr( "Stop Algorithm Execution When a Geometry is Invalid" ), QgsFeatureRequest::GeometryAbortOnInvalid );
  mComboInvalidFeatureFiltering->addItem( tr( "Try to fix Invalid Geometry, otherwise skip it. May discard m-value and other attr." ), QgsFeatureRequest::GeometryFixInvalidSkipOnFailure );
  mComboInvalidFeatureFiltering->addItem( tr( "Try to fix Invalid Geometry, otherwise abort. May discard m-value and other attr." ), QgsFeatureRequest::GeometryFixInvalidAbortOnFailure );

  connect( mFeatureLimitSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mComboInvalidFeatureFiltering, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
}

void QgsProcessingFeatureSourceOptionsWidget::setGeometryCheckMethod( bool isOverridden, QgsFeatureRequest::InvalidGeometryCheck check )
{
  if ( !isOverridden )
    mComboInvalidFeatureFiltering->setCurrentIndex( mComboInvalidFeatureFiltering->findData( QVariant() ) );
  else
    mComboInvalidFeatureFiltering->setCurrentIndex( mComboInvalidFeatureFiltering->findData( check ) );
}

void QgsProcessingFeatureSourceOptionsWidget::setFeatureLimit( int limit )
{
  mFeatureLimitSpinBox->setValue( limit );
}

QgsFeatureRequest::InvalidGeometryCheck QgsProcessingFeatureSourceOptionsWidget::geometryCheckMethod() const
{
  return mComboInvalidFeatureFiltering->currentData().isValid() ? static_cast< QgsFeatureRequest::InvalidGeometryCheck >( mComboInvalidFeatureFiltering->currentData().toInt() ) : QgsFeatureRequest::GeometryAbortOnInvalid;
}

bool QgsProcessingFeatureSourceOptionsWidget::isOverridingInvalidGeometryCheck() const
{
  return mComboInvalidFeatureFiltering->currentData().isValid();
}

int QgsProcessingFeatureSourceOptionsWidget::featureLimit() const
{
  return mFeatureLimitSpinBox->value() > 0 ? mFeatureLimitSpinBox->value() : -1;
}

///@endcond
