/***************************************************************************
  qgsannotationlayerproperties.cpp
  --------------------------------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayerproperties.h"
#include "moc_qgsannotationlayerproperties.cpp"
#include "qgshelp.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsdatumtransformdialog.h"
#include "qgspainteffect.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

QgsAnnotationLayerProperties::QgsAnnotationLayerProperties( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *, QWidget *parent, Qt::WindowFlags flags )
  : QgsLayerPropertiesDialog( layer, canvas, QStringLiteral( "AnnotationLayerProperties" ), parent, flags )
  , mLayer( layer )
{
  setupUi( this );

  mLayerComboBox->setAllowEmptyLayer( true );

  connect( this, &QDialog::accepted, this, &QgsAnnotationLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsAnnotationLayerProperties::rollback );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsAnnotationLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsAnnotationLayerProperties::showHelp );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsAnnotationLayerProperties::crsChanged );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  // update based on layer's current state
  syncToLayer();

  QgsSettings settings;
  if ( !settings.contains( QStringLiteral( "/Windows/AnnotationLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/AnnotationLayerProperties/tab" ), mOptStackedWidget->indexOf( mOptsPage_Information ) );
  }

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsAnnotationLayerProperties::loadStyleFromFile );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsAnnotationLayerProperties::saveStyleToFile );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsAnnotationLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsAnnotationLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsAnnotationLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBackupCrs = mLayer->crs();

  initialize();
}

QgsAnnotationLayerProperties::~QgsAnnotationLayerProperties() = default;

void QgsAnnotationLayerProperties::apply()
{
  mLayer->setName( mLayerOrigNameLineEdit->text() );

  // scale based layer visibility
  mLayer->setScaleBasedVisibility( mScaleVisibilityGroupBox->isChecked() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );

  mBackupCrs = mLayer->crs();

  // set the blend mode and opacity for the layer
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mLayer ) );
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );
  mLayer->setOpacity( mOpacityWidget->opacity() );

  if ( mPaintEffect )
    mLayer->setPaintEffect( mPaintEffect->clone() );

  mLayer->setLinkedVisibilityLayer( mLayerComboBox->currentLayer() );

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->apply();

  mLayer->triggerRepaint();
}

void QgsAnnotationLayerProperties::rollback()
{
  QgsLayerPropertiesDialog::rollback();

  if ( mBackupCrs != mLayer->crs() )
    mLayer->setCrs( mBackupCrs );
}

void QgsAnnotationLayerProperties::syncToLayer()
{
  // populate the general information
  mLayerOrigNameLineEdit->setText( mLayer->name() );

  /*
   * Information Tab
   */
  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  mInformationTextBrowser->clear();
  mInformationTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mInformationTextBrowser->setHtml( mLayer->htmlMetadata() );
  mInformationTextBrowser->setOpenLinks( false );
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsAnnotationLayerProperties::openUrl );

  mCrsSelector->setCrs( mLayer->crs() );

  // scale based layer visibility
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );
  mScaleVisibilityGroupBox->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setMapCanvas( mCanvas );

  // opacity and blend modes
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );
  mOpacityWidget->setOpacity( mLayer->opacity() );

  if ( mLayer->paintEffect() )
  {
    mPaintEffect.reset( mLayer->paintEffect()->clone() );
    mEffectWidget->setPaintEffect( mPaintEffect.get() );
  }

  mLayerComboBox->setLayer( mLayer->linkedVisibilityLayer() );

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->syncToLayer( mLayer );
}

void QgsAnnotationLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsAnnotationLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector_tiles/vector_tiles_properties.html" ) );
  }
}

void QgsAnnotationLayerProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select transformation for the layer" ) );
  mLayer->setCrs( crs );
}
