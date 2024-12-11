/***************************************************************************
  qgsmeshlayerproperties.cpp
  --------------------------
    begin                : Jun 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <typeinfo>

#include "qgsapplication.h"
#include "qgshelp.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerproperties.h"
#include "moc_qgsmeshlayerproperties.cpp"
#include "qgsmeshstaticdatasetwidget.h"
#include "qgsproject.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgssettings.h"
#include "qgsdatumtransformdialog.h"
#include "qgsmetadatawidget.h"
#include "qgsmeshlabelingwidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

QgsMeshLayerProperties::QgsMeshLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags fl )
  : QgsLayerPropertiesDialog( lyr, canvas, QStringLiteral( "MeshLayerProperties" ), parent, fl )
  , mMeshLayer( qobject_cast<QgsMeshLayer *>( lyr ) )
{
  Q_ASSERT( mMeshLayer );

  setupUi( this );
  mRendererMeshPropertiesWidget = new QgsRendererMeshPropertiesWidget( mMeshLayer, canvas, this );
  mConfigWidgets << mRendererMeshPropertiesWidget;
  mOptsPage_StyleContent->layout()->addWidget( mRendererMeshPropertiesWidget );

  mSimplifyReductionFactorSpinBox->setClearValue( 10.0 );
  mSimplifyMeshResolutionSpinBox->setClearValue( 5 );

  mStaticDatasetWidget->setLayer( mMeshLayer );
  mIsMapSettingsTemporal = mMeshLayer && canvas && canvas->mapSettings().isTemporal();

  mTemporalProviderTimeUnitComboBox->addItem( tr( "Seconds" ), static_cast<int>( Qgis::TemporalUnit::Seconds ) );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Minutes" ), static_cast<int>( Qgis::TemporalUnit::Minutes ) );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Hours" ), static_cast<int>( Qgis::TemporalUnit::Hours ) );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Days" ), static_cast<int>( Qgis::TemporalUnit::Days ) );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsMeshLayerProperties::changeCrs );
  connect( mDatasetGroupTreeWidget, &QgsMeshDatasetGroupTreeWidget::datasetGroupsChanged, this, &QgsMeshLayerProperties::syncToLayer );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsMeshLayerProperties::syncAndRepaint );

  connect( this, &QDialog::accepted, this, &QgsMeshLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsMeshLayerProperties::rollback );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsMeshLayerProperties::apply );

  connect( mMeshLayer, &QgsMeshLayer::dataChanged, this, &QgsMeshLayerProperties::syncAndRepaint );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMeshLayerProperties::showHelp );

  connect( mTemporalReloadButton, &QPushButton::clicked, this, &QgsMeshLayerProperties::reloadTemporalProperties );
  connect( mTemporalDateTimeReference, &QDateTimeEdit::dateTimeChanged, this, &QgsMeshLayerProperties::onTimeReferenceChange );
  connect( mMeshLayer, &QgsMeshLayer::activeScalarDatasetGroupChanged, mStaticDatasetWidget, &QgsMeshStaticDatasetWidget::setScalarDatasetGroup );
  connect( mMeshLayer, &QgsMeshLayer::activeVectorDatasetGroupChanged, mStaticDatasetWidget, &QgsMeshStaticDatasetWidget::setVectorDatasetGroup );

  mScaleRangeWidget->setMapCanvas( mCanvas );
  chkUseScaleDependentRendering->setChecked( lyr->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( lyr->minimumScale(), lyr->maximumScale() );

  connect( mAlwaysTimeFromSourceCheckBox, &QCheckBox::stateChanged, this, [this] {
    mTemporalDateTimeReference->setEnabled( !mAlwaysTimeFromSourceCheckBox->isChecked() );
    if ( mAlwaysTimeFromSourceCheckBox->isChecked() )
      reloadTemporalProperties();
  } );

  mComboBoxTemporalDatasetMatchingMethod->addItem( tr( "Find Closest Dataset Before Requested Time" ), QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetBeforeStartRangeTime );
  mComboBoxTemporalDatasetMatchingMethod->addItem( tr( "Find Closest Dataset From Requested Time (After or Before)" ), QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetFromStartRangeTime );

  QVBoxLayout *labelingLayout = nullptr;

  if ( mMeshLayer->contains( QgsMesh::ElementType::Face ) )
  {
    // Create the Labeling dialog tab
    labelingLayout = new QVBoxLayout( labelingFrame );
    labelingLayout->setContentsMargins( 0, 0, 0, 0 );
    mLabelingDialog = new QgsMeshLabelingWidget( mMeshLayer, mCanvas, labelingFrame );
    mLabelingDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
    labelingLayout->addWidget( mLabelingDialog );
    labelingFrame->setLayout( labelingLayout );
  }
  else
  {
    mLabelingDialog = nullptr;
    mOptsPage_Labels->setEnabled( false ); // disable labeling item
  }

  QVBoxLayout *metadataLayout = new QVBoxLayout( metadataFrame );
  metadataLayout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mMeshLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  metadataLayout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( metadataLayout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );
  mBackupCrs = mMeshLayer->crs();

  mTemporalDateTimeStart->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mTemporalDateTimeEnd->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mTemporalDateTimeReference->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/MeshLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/MeshLayerProperties/tab" ), mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  //Add help page references
  mOptsPage_Information->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#information-properties" ) );
  mOptsPage_Source->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#source-properties" ) );
  mOptsPage_Style->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#symbology-properties" ) );
  mOptsPage_Rendering->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#rendering-properties" ) );
  mOptsPage_Temporal->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#temporal-properties" ) );
  mOptsPage_Metadata->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#metadata-properties" ) );

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsMeshLayerProperties::loadStyleFromFile );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsMeshLayerProperties::saveStyleToFile );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsMeshLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsMeshLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsMeshLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsMeshLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsMeshLayerProperties::saveMetadataToFile );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  initialize();
}

void QgsMeshLayerProperties::syncToLayer()
{
  Q_ASSERT( mRendererMeshPropertiesWidget );

  QgsDebugMsgLevel( QStringLiteral( "populate general information tab" ), 4 );
  /*
  * Information Tab
  */
  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  mInformationTextBrowser->clear();
  mInformationTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mInformationTextBrowser->setHtml( mMeshLayer->htmlMetadata() );
  mInformationTextBrowser->setOpenLinks( false );
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsMeshLayerProperties::openUrl );

  QgsDebugMsgLevel( QStringLiteral( "populate source tab" ), 4 );
  /*
   * Source Tab
   */
  mLayerOrigNameLineEd->setText( mMeshLayer->name() );
  whileBlocking( mCrsSelector )->setCrs( mMeshLayer->crs() );

  mDatasetGroupTreeWidget->syncToLayer( mMeshLayer );

  QgsDebugMsgLevel( QStringLiteral( "populate config tab" ), 4 );
  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->syncToLayer( mMeshLayer );

  QgsDebugMsgLevel( QStringLiteral( "populate rendering tab" ), 4 );
  if ( mMeshLayer->isEditable() )
    mSimplifyMeshGroupBox->setEnabled( false );

  QgsMeshSimplificationSettings simplifySettings = mMeshLayer->meshSimplificationSettings();
  mSimplifyMeshGroupBox->setChecked( simplifySettings.isEnabled() );
  mSimplifyReductionFactorSpinBox->setValue( simplifySettings.reductionFactor() );
  mSimplifyMeshResolutionSpinBox->setValue( simplifySettings.meshResolution() );

  QgsDebugMsgLevel( QStringLiteral( "populate temporal tab" ), 4 );
  const QgsMeshLayerTemporalProperties *temporalProperties = qobject_cast<const QgsMeshLayerTemporalProperties *>( mMeshLayer->temporalProperties() );
  whileBlocking( mTemporalDateTimeReference )->setDateTime( temporalProperties->referenceTime() );
  const QgsDateTimeRange timeRange = temporalProperties->timeExtent();
  mTemporalDateTimeStart->setDateTime( timeRange.begin() );
  mTemporalDateTimeEnd->setDateTime( timeRange.end() );
  if ( mMeshLayer->dataProvider() )
  {
    mTemporalProviderTimeUnitComboBox->setCurrentIndex(
      mTemporalProviderTimeUnitComboBox->findData( static_cast<int>( mMeshLayer->dataProvider()->temporalCapabilities()->temporalUnit() ) )
    );
  }
  mAlwaysTimeFromSourceCheckBox->setChecked( temporalProperties->alwaysLoadReferenceTimeFromSource() );
  mComboBoxTemporalDatasetMatchingMethod->setCurrentIndex(
    mComboBoxTemporalDatasetMatchingMethod->findData( temporalProperties->matchingMethod() )
  );

  mStaticDatasetWidget->syncToLayer();
  mStaticDatasetGroupBox->setChecked( !mMeshLayer->temporalProperties()->isActive() );
}

void QgsMeshLayerProperties::saveDefaultStyle()
{
  saveStyleAsDefault();
}

void QgsMeshLayerProperties::loadStyle()
{
  loadStyleFromFile();
}

void QgsMeshLayerProperties::saveStyleAs()
{
  saveStyleToFile();
}

void QgsMeshLayerProperties::apply()
{
  Q_ASSERT( mRendererMeshPropertiesWidget );

  QgsDebugMsgLevel( QStringLiteral( "processing general tab" ), 4 );
  /*
   * General Tab
   */
  mMeshLayer->setName( mLayerOrigNameLineEd->text() );

  QgsDebugMsgLevel( QStringLiteral( "processing source tab" ), 4 );
  /*
   * Source Tab
   */
  mDatasetGroupTreeWidget->apply();

  QgsDebugMsgLevel( QStringLiteral( "processing config tabs" ), 4 );

  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->apply();

  QgsDebugMsgLevel( QStringLiteral( "processing rendering tab" ), 4 );
  /*
   * Rendering Tab
   */
  QgsMeshSimplificationSettings simplifySettings;
  simplifySettings.setEnabled( mSimplifyMeshGroupBox->isChecked() );
  simplifySettings.setReductionFactor( mSimplifyReductionFactorSpinBox->value() );
  simplifySettings.setMeshResolution( mSimplifyMeshResolutionSpinBox->value() );
  bool needMeshUpdating = ( ( simplifySettings.isEnabled() != mMeshLayer->meshSimplificationSettings().isEnabled() ) || ( simplifySettings.reductionFactor() != mMeshLayer->meshSimplificationSettings().reductionFactor() ) );

  mMeshLayer->setMeshSimplificationSettings( simplifySettings );

  mMeshLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mMeshLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mMeshLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  QgsDebugMsgLevel( QStringLiteral( "processing labeling tab" ), 4 );
  /*
   * Labeling Tab
   */
  if ( mLabelingDialog )
  {
    mLabelingDialog->writeSettingsToLayer();
  }

  QgsDebugMsgLevel( QStringLiteral( "processing temporal tab" ), 4 );
  /*
   * Temporal Tab
   */

  mMeshLayer->setReferenceTime( mTemporalDateTimeReference->dateTime() );
  if ( mMeshLayer->dataProvider() )
    mMeshLayer->dataProvider()->setTemporalUnit(
      static_cast<Qgis::TemporalUnit>( mTemporalProviderTimeUnitComboBox->currentData().toInt() )
    );

  mStaticDatasetWidget->apply();
  bool needEmitRendererChanged = mMeshLayer->temporalProperties()->isActive() == mStaticDatasetGroupBox->isChecked();
  mMeshLayer->temporalProperties()->setIsActive( !mStaticDatasetGroupBox->isChecked() );
  mMeshLayer->setTemporalMatchingMethod( static_cast<QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod>(
    mComboBoxTemporalDatasetMatchingMethod->currentData().toInt()
  ) );
  static_cast<QgsMeshLayerTemporalProperties *>(
    mMeshLayer->temporalProperties()
  )
    ->setAlwaysLoadReferenceTimeFromSource( mAlwaysTimeFromSourceCheckBox->isChecked() );

  mMetadataWidget->acceptMetadata();

  mBackupCrs = mMeshLayer->crs();

  if ( needMeshUpdating )
    mMeshLayer->reload();

  if ( needEmitRendererChanged )
    emit mMeshLayer->rendererChanged();

  //make sure the layer is redrawn
  mMeshLayer->triggerRepaint();

  // notify the project we've made a change
  QgsProject::instance()->setDirty( true );

  // Resync what have to be resync (widget that can be changed by other properties part)
  mStaticDatasetWidget->syncToLayer();
  for ( QgsMapLayerConfigWidget *w : std::as_const( mConfigWidgets ) )
    w->syncToLayer( mMeshLayer );
}

void QgsMeshLayerProperties::changeCrs( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select Transformation" ) );
  mMeshLayer->setCrs( crs );
}

void QgsMeshLayerProperties::syncAndRepaint()
{
  syncToLayer();
  mMeshLayer->triggerRepaint();
}

void QgsMeshLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_mesh/mesh_properties.html" ) );
  }
}

void QgsMeshLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mMeshLayer );
}

void QgsMeshLayerProperties::reloadTemporalProperties()
{
  if ( !mMeshLayer->dataProvider() )
    return;
  QgsMeshDataProviderTemporalCapabilities *temporalCapabalities = mMeshLayer->dataProvider()->temporalCapabilities();
  QgsDateTimeRange timeExtent;
  QDateTime referenceTime = temporalCapabalities->referenceTime();
  if ( referenceTime.isValid() )
  {
    timeExtent = temporalCapabalities->timeExtent();
    whileBlocking( mTemporalDateTimeReference )->setDateTime( referenceTime );
  }
  else
    // The reference time already here is used again to define the time extent
    timeExtent = temporalCapabalities->timeExtent( mTemporalDateTimeReference->dateTime() );

  mTemporalDateTimeStart->setDateTime( timeExtent.begin() );
  mTemporalDateTimeEnd->setDateTime( timeExtent.end() );
}

void QgsMeshLayerProperties::onTimeReferenceChange()
{
  if ( !mMeshLayer->dataProvider() )
    return;
  const QgsDateTimeRange &timeExtent = mMeshLayer->dataProvider()->temporalCapabilities()->timeExtent( mTemporalDateTimeReference->dateTime() );
  mTemporalDateTimeStart->setDateTime( timeExtent.begin() );
  mTemporalDateTimeEnd->setDateTime( timeExtent.end() );
}

void QgsMeshLayerProperties::rollback()
{
  if ( mBackupCrs != mMeshLayer->crs() )
    mMeshLayer->setCrs( mBackupCrs );

  QgsLayerPropertiesDialog::rollback();
}
