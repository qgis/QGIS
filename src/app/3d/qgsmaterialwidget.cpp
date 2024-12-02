/***************************************************************************
  qgsmaterialwidget.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterialwidget.h"
#include "moc_qgsmaterialwidget.cpp"
#include "qgs3d.h"
#include "qgsmaterialregistry.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsmaterialsettingswidget.h"
#include "qgsreadwritecontext.h"
#include "qgsphongmaterialsettings.h"

QgsMaterialWidget::QgsMaterialWidget( QWidget *parent )
  : QWidget( parent )
  , mCurrentSettings( std::make_unique<QgsPhongMaterialSettings>() )
  , mTechnique( QgsMaterialSettingsRenderingTechnique::Triangles )
{
  setupUi( this );

  const QStringList materialTypes = Qgs3D::materialRegistry()->materialSettingsTypes();
  for ( const QString &type : materialTypes )
  {
    mMaterialTypeComboBox->addItem( Qgs3D::materialRegistry()->materialSettingsMetadata( type )->icon(), Qgs3D::materialRegistry()->materialSettingsMetadata( type )->visibleName(), type );
  }

  connect( mMaterialTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMaterialWidget::materialTypeChanged );
}

void QgsMaterialWidget::setTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  mTechnique = technique;
  const QString prevType = mMaterialTypeComboBox->currentData().toString();
  mMaterialTypeComboBox->blockSignals( true );
  mMaterialTypeComboBox->clear();

  const QStringList materialTypes = Qgs3D::materialRegistry()->materialSettingsTypes();
  for ( const QString &type : materialTypes )
  {
    if ( !Qgs3D::materialRegistry()->materialSettingsMetadata( type )->supportsTechnique( technique ) )
      continue;

    mMaterialTypeComboBox->addItem( Qgs3D::materialRegistry()->materialSettingsMetadata( type )->icon(), Qgs3D::materialRegistry()->materialSettingsMetadata( type )->visibleName(), type );
  }

  const int prevIndex = mMaterialTypeComboBox->findData( prevType );
  if ( prevIndex == -1 )
  {
    // if phong material type is available, default to it (for now?)
    const int phongIndex = mMaterialTypeComboBox->findData( QStringLiteral( "phong" ) );
    if ( phongIndex >= 0 )
      mMaterialTypeComboBox->setCurrentIndex( phongIndex );
    else
      mMaterialTypeComboBox->setCurrentIndex( 0 );
  }
  else
    mMaterialTypeComboBox->setCurrentIndex( prevIndex );

  if ( QgsMaterialSettingsWidget *w = qobject_cast<QgsMaterialSettingsWidget *>( mStackedWidget->currentWidget() ) )
    w->setTechnique( technique );

  mMaterialTypeComboBox->blockSignals( false );
  materialTypeChanged();
}

void QgsMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  mMaterialTypeComboBox->setCurrentIndex( mMaterialTypeComboBox->findData( settings->type() ) );
  mCurrentSettings.reset( settings->clone() );
  mLayer = layer;
  updateMaterialWidget();
}

QgsAbstractMaterialSettings *QgsMaterialWidget::settings()
{
  return mCurrentSettings->clone();
}

void QgsMaterialWidget::setType( const QString &type )
{
  mMaterialTypeComboBox->setCurrentIndex( mMaterialTypeComboBox->findData( type ) );
  materialTypeChanged();
}

void QgsMaterialWidget::materialTypeChanged()
{
  std::unique_ptr<QgsAbstractMaterialSettings> currentSettings( settings() );
  const QString existingType = currentSettings ? currentSettings->type() : QString();
  const QString newType = mMaterialTypeComboBox->currentData().toString();
  if ( existingType == newType )
    return;

  if ( QgsMaterialSettingsAbstractMetadata *am = Qgs3D::materialRegistry()->materialSettingsMetadata( newType ) )
  {
    // change material to a new (with different type)
    // base new layer on existing materials's properties
    std::unique_ptr<QgsAbstractMaterialSettings> newMaterial( am->create() );
    if ( newMaterial )
    {
      if ( currentSettings )
      {
        QDomDocument doc;
        QDomElement tempElem = doc.createElement( QStringLiteral( "temp" ) );
        currentSettings->writeXml( tempElem, QgsReadWriteContext() );
        newMaterial->readXml( tempElem, QgsReadWriteContext() );
      }

      mCurrentSettings = std::move( newMaterial );
      updateMaterialWidget();
      emit changed();
    }
  }
}

void QgsMaterialWidget::materialWidgetChanged()
{
  if ( QgsMaterialSettingsWidget *w = qobject_cast<QgsMaterialSettingsWidget *>( mStackedWidget->currentWidget() ) )
  {
    mCurrentSettings.reset( w->settings() );
  }
  emit changed();
}

void QgsMaterialWidget::updateMaterialWidget()
{
  if ( mStackedWidget->currentWidget() != mPageDummy )
  {
    // stop updating from the original widget
    if ( QgsMaterialSettingsWidget *w = qobject_cast<QgsMaterialSettingsWidget *>( mStackedWidget->currentWidget() ) )
      disconnect( w, &QgsMaterialSettingsWidget::changed, this, &QgsMaterialWidget::materialWidgetChanged );
    mStackedWidget->removeWidget( mStackedWidget->currentWidget() );
  }

  const QString settingsType = mCurrentSettings->type();
  if ( QgsMaterialSettingsAbstractMetadata *am = Qgs3D::materialRegistry()->materialSettingsMetadata( settingsType ) )
  {
    if ( QgsMaterialSettingsWidget *w = am->createWidget() )
    {
      w->setSettings( mCurrentSettings.get(), mLayer );
      w->setTechnique( mTechnique );
      mStackedWidget->addWidget( w );
      mStackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, &QgsMaterialSettingsWidget::changed, this, &QgsMaterialWidget::materialWidgetChanged );
      return;
    }
  }
  // When anything is not right
  mStackedWidget->setCurrentWidget( mPageDummy );
}
