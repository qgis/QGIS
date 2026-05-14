/***************************************************************************
  qgsmetalroughmaterialwidget.cpp
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughmaterialwidget.h"

#include "qgis.h"
#include "qgsdoublespinbox.h"
#include "qgsmetalroughmaterialsettings.h"

#include <QString>

#include "moc_qgsmetalroughmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsMetalRoughMaterialWidget::QgsMetalRoughMaterialWidget( QWidget *parent, bool )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"metalrough"_s );

  QgsMetalRoughMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  // clear has no meaning here
  mMetalnessWidget->spinBox()->setShowClearButton( false );
  mRoughnessWidget->spinBox()->setShowClearButton( false );

  connect( mButtonBaseColor, &QgsColorButton::colorChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mMetalnessWidget, &QgsPercentageWidget::valueChanged, this, [this] {
    updateWidgetState();
    emit changed();
  } );
  connect( mRoughnessWidget, &QgsPercentageWidget::valueChanged, this, [this] {
    updateWidgetState();
    emit changed();
  } );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsMetalRoughMaterialWidget::changed );

  connect( this, &QgsMetalRoughMaterialWidget::changed, this, &QgsMetalRoughMaterialWidget::updatePreview );
}

QgsMaterialSettingsWidget *QgsMetalRoughMaterialWidget::create()
{
  return new QgsMetalRoughMaterialWidget();
}

void QgsMetalRoughMaterialWidget::setTechnique( Qgis::MaterialRenderingTechnique )
{}

void QgsMetalRoughMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsMetalRoughMaterialSettings *material = dynamic_cast<const QgsMetalRoughMaterialSettings *>( settings );
  if ( !material )
    return;
  mButtonBaseColor->setColor( material->baseColor() );
  mMetalnessWidget->setValue( material->metalness() );
  mRoughnessWidget->setValue( material->roughness() );
  mOpacityWidget->setOpacity( material->opacity() );

  mPropertyCollection = settings->dataDefinedProperties();

  updateWidgetState();
  updatePreview();
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsMetalRoughMaterialWidget::settings()
{
  auto m = std::make_unique<QgsMetalRoughMaterialSettings>();
  m->setBaseColor( mButtonBaseColor->color() );
  m->setMetalness( mMetalnessWidget->value() );
  m->setRoughness( mRoughnessWidget->value() );
  m->setOpacity( mOpacityWidget->opacity() );
  m->setDataDefinedProperties( mPropertyCollection );
  return m;
}

void QgsMetalRoughMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsMetalRoughMaterialWidget::updateWidgetState()
{}

void QgsMetalRoughMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
