/***************************************************************************
  qgstiledmeshlayerproperties.cpp
  --------------------------------------
  Date                 : June 2023
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

#include "qgstiledmeshlayerproperties.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgstiledmeshlayer.h"
#include "qgsmetadatawidget.h"
#include "qgsapplication.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "qgsdatumtransformdialog.h"

#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

QgsTiledMeshLayerProperties::QgsTiledMeshLayerProperties( QgsTiledMeshLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *, QWidget *parent, Qt::WindowFlags flags )
  : QgsLayerPropertiesDialog( layer, canvas, QStringLiteral( "QgsTiledMeshLayerProperties" ), parent, flags )
  , mLayer( layer )
{
  setupUi( this );

  connect( this, &QDialog::accepted, this, &QgsTiledMeshLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsTiledMeshLayerProperties::rollback );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsTiledMeshLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsTiledMeshLayerProperties::showHelp );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsTiledMeshLayerProperties::crsChanged );

  mScaleRangeWidget->setMapCanvas( mCanvas );
  chkUseScaleDependentRendering->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  // update based on layer's current state
  syncToLayer();

  QgsSettings settings;
  if ( !settings.contains( QStringLiteral( "/Windows/TiledMeshLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/TiledMeshLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Information ) );
  }

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsTiledMeshLayerProperties::loadStyleFromFile );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsTiledMeshLayerProperties::saveStyleToFile );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsTiledMeshLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsTiledMeshLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsTiledMeshLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsTiledMeshLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsTiledMeshLayerProperties::saveMetadataToFile );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save as Default" ), this, &QgsTiledMeshLayerProperties::saveMetadataAsDefault );
  menuMetadata->addAction( tr( "Restore Default" ), this, &QgsTiledMeshLayerProperties::loadDefaultMetadata );

  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  //Add help page references
#if 0 // TODO
  mOptsPage_Information->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#information-properties" ) );
  mOptsPage_Source->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#source-properties" ) );
  mOptsPage_Rendering->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#rendering-properties" ) );
  mOptsPage_Metadata->setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#metadata-properties" ) );
#endif

  mBackupCrs = mLayer->crs();

  initialize();
}

void QgsTiledMeshLayerProperties::apply()
{
  mMetadataWidget->acceptMetadata();

  mLayer->setName( mLayerOrigNameLineEdit->text() );

  mLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  mBackupCrs = mLayer->crs();

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->apply();

  mLayer->triggerRepaint();
}

void QgsTiledMeshLayerProperties::rollback()
{
  if ( mBackupCrs != mLayer->crs() )
    mLayer->setCrs( mBackupCrs );

  QgsLayerPropertiesDialog::rollback();
}

void QgsTiledMeshLayerProperties::syncToLayer()
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->name() );

  // information Tab
  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  mInformationTextBrowser->clear();
  mInformationTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mInformationTextBrowser->setHtml( mLayer->htmlMetadata() );
  mInformationTextBrowser->setOpenLinks( false );
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsTiledMeshLayerProperties::openUrl );

  mCrsSelector->setCrs( mLayer->crs() );

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->syncToLayer( mLayer );
}

void QgsTiledMeshLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsTiledMeshLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
#if 0 // TODO
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_point_clouds/point_clouds.html" ) );
  }
#endif
}

void QgsTiledMeshLayerProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select transformation for the layer" ) );
  mLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
}

