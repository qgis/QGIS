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

#include "qgsabstractmaterialsettings.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgsmaterialregistry.h"
#include "qgsmaterialsettingswidget.h"
#include "qgsphongmaterialsettings.h"
#include "qgsreadwritecontext.h"
#include "qgsvectorlayer.h"

#include <QDialogButtonBox>
#include <QString>

#include "moc_qgsmaterialwidget.cpp"

using namespace Qt::StringLiterals;

//
// QgsMaterialWidget
//

QgsMaterialWidget::QgsMaterialWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  const QStringList materialTypes = QgsApplication::materialRegistry()->materialSettingsTypes();
  for ( const QString &type : materialTypes )
  {
    if ( type == "null"_L1 )
      continue;

    mMaterialTypeComboBox->addItem( QgsApplication::materialRegistry()->materialSettingsMetadata( type )->icon(), QgsApplication::materialRegistry()->materialSettingsMetadata( type )->visibleName(), type );
  }

  mMaterialTypeComboBox->setCurrentIndex( -1 );
  connect( mMaterialTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMaterialWidget::materialTypeChanged );
  materialTypeChanged();

  setSettings( new QgsPhongMaterialSettings(), nullptr );
}

QgsMaterialWidget::~QgsMaterialWidget() = default;

void QgsMaterialWidget::setTechnique( Qgis::MaterialRenderingTechnique technique )
{
  mTechnique = technique;
  rebuildAvailableTypes();
}

void QgsMaterialWidget::rebuildAvailableTypes()
{
  const QString prevType = mMaterialTypeComboBox->currentData().toString();
  mMaterialTypeComboBox->blockSignals( true );
  mMaterialTypeComboBox->clear();

  const QStringList materialTypes = QgsApplication::materialRegistry()->materialSettingsTypes();
  for ( const QString &type : materialTypes )
  {
    if ( mFilterByTechnique && !QgsApplication::materialRegistry()->materialSettingsMetadata( type )->supportsTechnique( mTechnique ) )
      continue;

    else if ( type == "null"_L1 && !mFilterByTechnique )
      continue; // don't expose null as an option if we're showing in a generic mode

    mMaterialTypeComboBox->addItem( QgsApplication::materialRegistry()->materialSettingsMetadata( type )->icon(), QgsApplication::materialRegistry()->materialSettingsMetadata( type )->visibleName(), type );
  }

  const int prevIndex = mMaterialTypeComboBox->findData( prevType );
  if ( prevIndex == -1 )
  {
    // if phong material type is available, default to it (for now?)
    const int phongIndex = mMaterialTypeComboBox->findData( u"phong"_s );
    if ( phongIndex >= 0 )
      mMaterialTypeComboBox->setCurrentIndex( phongIndex );
    else
      mMaterialTypeComboBox->setCurrentIndex( 0 );
  }
  else
    mMaterialTypeComboBox->setCurrentIndex( prevIndex );

  if ( QgsMaterialSettingsWidget *w = qobject_cast<QgsMaterialSettingsWidget *>( mStackedWidget->currentWidget() ) )
  {
    w->setTechnique( mTechnique );
  }

  mMaterialTypeComboBox->blockSignals( false );
  materialTypeChanged();
}

void QgsMaterialWidget::setFilterByTechnique( bool enabled )
{
  mFilterByTechnique = enabled;
  rebuildAvailableTypes();
}

void QgsMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  mMaterialTypeComboBox->setCurrentIndex( mMaterialTypeComboBox->findData( settings->type() ) );
  mCurrentSettings.reset( settings->clone() );
  mLayer = layer;
  updateMaterialWidget();
}

std::unique_ptr< QgsAbstractMaterialSettings > QgsMaterialWidget::settings()
{
  return mCurrentSettings ? std::unique_ptr< QgsAbstractMaterialSettings >( mCurrentSettings->clone() ) : nullptr;
}

void QgsMaterialWidget::setType( const QString &type )
{
  mMaterialTypeComboBox->setCurrentIndex( mMaterialTypeComboBox->findData( type ) );
  materialTypeChanged();
}

void QgsMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewVisible = visible;
  if ( QgsMaterialSettingsWidget *w = qobject_cast<QgsMaterialSettingsWidget *>( mStackedWidget->currentWidget() ) )
  {
    w->setPreviewVisible( visible );
  }
}

void QgsMaterialWidget::materialTypeChanged()
{
  std::unique_ptr<QgsAbstractMaterialSettings> currentSettings( settings() );
  const QString existingType = currentSettings ? currentSettings->type() : QString();
  const QString newType = mMaterialTypeComboBox->currentData().toString();
  if ( existingType == newType )
    return;

  if ( QgsMaterialSettingsAbstractMetadata *am = QgsApplication::materialRegistry()->materialSettingsMetadata( newType ) )
  {
    // change material to a new (with different type)
    // base new layer on existing materials's properties
    std::unique_ptr<QgsAbstractMaterialSettings> newMaterial( am->create() );
    if ( newMaterial )
    {
      if ( currentSettings )
      {
        QDomDocument doc;
        QDomElement tempElem = doc.createElement( u"temp"_s );
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
    mCurrentSettings = w->settings();
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
  if ( QgsMaterialSettingsAbstractMetadata *am = QgsApplication::materialRegistry()->materialSettingsMetadata( settingsType ) )
  {
    if ( QgsMaterialSettingsWidget *w = am->createWidget() )
    {
      w->setSettings( mCurrentSettings.get(), mLayer );
      w->setTechnique( mTechnique );
      w->setPreviewVisible( mPreviewVisible );
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


//
// QgsMaterialWidgetDialog
//

QgsMaterialWidgetDialog::QgsMaterialWidgetDialog( const QgsAbstractMaterialSettings *settings, QWidget *parent )
  : QDialog( parent )
{
  QgsGui::enableAutoGeometryRestore( this );

  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsMaterialWidget();
  mWidget->setPreviewVisible( true );
  vLayout->addWidget( mWidget, 1 );

  if ( settings )
  {
    mWidget->setSettings( settings, nullptr );
  }

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "Material" ) );

  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsMaterialWidgetDialog::settings()
{
  return mWidget->settings();
}

QDialogButtonBox *QgsMaterialWidgetDialog::buttonBox()
{
  return mButtonBox;
}
