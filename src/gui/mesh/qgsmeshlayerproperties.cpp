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
#include "qgsfileutils.h"
#include "qgshelp.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerproperties.h"
#include "qgsmeshstaticdatasetwidget.h"
#include "qgsproject.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgssettings.h"
#include "qgsdatumtransformdialog.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsmetadatawidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

QgsMeshLayerProperties::QgsMeshLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( QStringLiteral( "MeshLayerProperties" ), parent, fl )
  , mMeshLayer( qobject_cast<QgsMeshLayer *>( lyr ) )
  , mCanvas( canvas )
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

  mTemporalProviderTimeUnitComboBox->addItem( tr( "Seconds" ), QgsUnitTypes::TemporalSeconds );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Minutes" ), QgsUnitTypes::TemporalMinutes );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Hours" ), QgsUnitTypes::TemporalHours );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Days" ), QgsUnitTypes::TemporalDays );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsMeshLayerProperties::changeCrs );
  connect( mDatasetGroupTreeWidget, &QgsMeshDatasetGroupTreeWidget::datasetGroupAdded, this, &QgsMeshLayerProperties::syncToLayer );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsMeshLayerProperties::syncAndRepaint );

  connect( this, &QDialog::accepted, this, &QgsMeshLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsMeshLayerProperties::onCancel );
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

  connect( mAlwaysTimeFromSourceCheckBox, &QCheckBox::stateChanged, this, [this]
  {
    mTemporalDateTimeReference->setEnabled( !mAlwaysTimeFromSourceCheckBox->isChecked() );
    if ( mAlwaysTimeFromSourceCheckBox->isChecked() )
      reloadTemporalProperties();
  } );

  mComboBoxTemporalDatasetMatchingMethod->addItem( tr( "Find Closest Dataset Before Requested Time" ),
      QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetBeforeStartRangeTime );
  mComboBoxTemporalDatasetMatchingMethod->addItem( tr( "Find Closest Dataset From Requested Time (After or Before)" ),
      QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetFromStartRangeTime );

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mMeshLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );
  mBackupCrs = mMeshLayer->crs();

  mTemporalDateTimeStart->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mTemporalDateTimeEnd->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mTemporalDateTimeReference->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/MeshLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/MeshLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
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
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsMeshLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsMeshLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsMeshLayerProperties::saveDefaultStyle );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsMeshLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsMeshLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsMeshLayerProperties::loadMetadata );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsMeshLayerProperties::saveMetadataAs );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  QString title = tr( "Layer Properties — %1" ).arg( lyr->name() );

  if ( !mMeshLayer->styleManager()->isDefault( mMeshLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mMeshLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );
}

void QgsMeshLayerProperties::addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory )
{
  if ( !factory->supportsLayer( mMeshLayer ) || !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QgsMapLayerConfigWidget *page = factory->createWidget( mMeshLayer, mCanvas, false, this );
  mConfigWidgets << page;

  const QString beforePage = factory->layerPropertiesPagePositionHint();
  if ( beforePage.isEmpty() )
    addPage( factory->title(), factory->title(), factory->icon(), page );
  else
    insertPage( factory->title(), factory->title(), factory->icon(), page, beforePage );

  page->syncToLayer( mMeshLayer );

}

void QgsMeshLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  bool isMetadataPanel = ( index == mOptStackedWidget->indexOf( mOptsPage_Metadata ) );
  mBtnStyle->setVisible( ! isMetadataPanel );
  mBtnMetadata->setVisible( isMetadataPanel );
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
  connect( mInformationTextBrowser, &QTextBrowser::anchorClicked, this, &QgsMeshLayerProperties::urlClicked );

  QgsDebugMsgLevel( QStringLiteral( "populate source tab" ), 4 );
  /*
   * Source Tab
   */
  mLayerOrigNameLineEd->setText( mMeshLayer->name() );
  whileBlocking( mCrsSelector )->setCrs( mMeshLayer->crs() );

  if ( mMeshLayer )
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
  const QgsMeshLayerTemporalProperties *temporalProperties = qobject_cast< const QgsMeshLayerTemporalProperties * >( mMeshLayer->temporalProperties() );
  whileBlocking( mTemporalDateTimeReference )->setDateTime( temporalProperties->referenceTime() );
  const QgsDateTimeRange timeRange = temporalProperties->timeExtent();
  mTemporalDateTimeStart->setDateTime( timeRange.begin() );
  mTemporalDateTimeEnd->setDateTime( timeRange.end() );
  if ( mMeshLayer->dataProvider() )
  {
    mTemporalProviderTimeUnitComboBox->setCurrentIndex(
      mTemporalProviderTimeUnitComboBox->findData( mMeshLayer->dataProvider()->temporalCapabilities()->temporalUnit() ) );
  }
  mAlwaysTimeFromSourceCheckBox->setChecked( temporalProperties->alwaysLoadReferenceTimeFromSource() );
  mComboBoxTemporalDatasetMatchingMethod->setCurrentIndex(
    mComboBoxTemporalDatasetMatchingMethod->findData( temporalProperties->matchingMethod() ) );

  mStaticDatasetWidget->syncToLayer();
  mStaticDatasetGroupBox->setChecked( !mMeshLayer->temporalProperties()->isActive() );
}

void QgsMeshLayerProperties::loadDefaultStyle()
{
  bool defaultLoadedFlag = false;
  QString myMessage = mMeshLayer->loadDefaultStyle( defaultLoadedFlag );
  // reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    // otherwise let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsMeshLayerProperties::saveDefaultStyle()
{
  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // TODO Once the deprecated `saveDefaultStyle()` method is gone, just
  // remove the NOWARN_DEPRECATED tags
  Q_NOWARN_DEPRECATED_PUSH
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  QString myMessage = mMeshLayer->saveDefaultStyle( defaultSavedFlag );
  Q_NOWARN_DEPRECATED_POP
  if ( !defaultSavedFlag )
  {
    // let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsMeshLayerProperties::loadStyle()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName(
                       this,
                       tr( "Load rendering setting from style file" ),
                       lastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( fileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".qml" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".qml" );

  mOldStyle = mMeshLayer->styleManager()->style( mMeshLayer->styleManager()->currentStyle() );

  bool defaultLoadedFlag = false;
  QString message = mMeshLayer->loadNamedStyle( fileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).absolutePath() );
    syncToLayer();
  }
  else
  {
    QMessageBox::information( this, tr( "Load Style" ), message );
  }
}

void QgsMeshLayerProperties::saveStyleAs()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName(
                             this,
                             tr( "Save layer properties as style file" ),
                             lastUsedDir,
                             tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( outputFileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, QStringList() << QStringLiteral( "qml" ) );

  apply(); // make sure the style to save is up-to-date

  // then export style
  bool defaultLoadedFlag = false;
  QString message;
  message = mMeshLayer->saveNamedStyle( outputFileName, defaultLoadedFlag );

  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( outputFileName ).absolutePath() );
  }
  else
    QMessageBox::information( this, tr( "Save Style" ), message );
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
  bool needMeshUpdating = ( ( simplifySettings.isEnabled() != mMeshLayer->meshSimplificationSettings().isEnabled() ) ||
                            ( simplifySettings.reductionFactor() != mMeshLayer->meshSimplificationSettings().reductionFactor() ) );

  mMeshLayer->setMeshSimplificationSettings( simplifySettings );

  mMeshLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mMeshLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mMeshLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  QgsDebugMsgLevel( QStringLiteral( "processing temporal tab" ), 4 );
  /*
   * Temporal Tab
   */

  mMeshLayer->setReferenceTime( mTemporalDateTimeReference->dateTime() );
  if ( mMeshLayer->dataProvider() )
    mMeshLayer->dataProvider()->setTemporalUnit(
      static_cast<QgsUnitTypes::TemporalUnit>( mTemporalProviderTimeUnitComboBox->currentData().toInt() ) );

  mStaticDatasetWidget->apply();
  bool needEmitRendererChanged = mMeshLayer->temporalProperties()->isActive() == mStaticDatasetGroupBox->isChecked();
  mMeshLayer->temporalProperties()->setIsActive( !mStaticDatasetGroupBox->isChecked() );
  mMeshLayer->setTemporalMatchingMethod( static_cast<QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod>(
      mComboBoxTemporalDatasetMatchingMethod->currentData().toInt() ) );
  static_cast<QgsMeshLayerTemporalProperties *>(
    mMeshLayer->temporalProperties() )->setAlwaysLoadReferenceTimeFromSource( mAlwaysTimeFromSourceCheckBox->isChecked() );

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

void QgsMeshLayerProperties::urlClicked( const QUrl &url )
{
  QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsMeshLayerProperties::loadMetadata()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer metadata from metadata file" ), myLastUsedDir,
                       tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;
  myMessage = mMeshLayer->loadNamedMetadata( myFileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mMeshLayer->metadata() );
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Metadata" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  activateWindow(); // set focus back to properties dialog
}

void QgsMeshLayerProperties::saveMetadataAs()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save Layer Metadata as QMD" ),
                             myLastUsedDir, tr( "QMD File" ) + " (*.qmd)" );
  if ( myOutputFileName.isNull() ) //dialog canceled
  {
    return;
  }

  mMetadataWidget->acceptMetadata();

  //ensure the user never omitted the extension from the file name
  if ( !myOutputFileName.endsWith( QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata ), Qt::CaseInsensitive ) )
  {
    myOutputFileName += QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata );
  }

  bool defaultLoadedFlag = false;
  QString message = mMeshLayer->saveNamedMetadata( myOutputFileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
    myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( myOutputFileName ).absolutePath() );
  else
    QMessageBox::information( this, tr( "Save Metadata" ), message );
}

void QgsMeshLayerProperties::onCancel()
{
  if ( mBackupCrs != mMeshLayer->crs() )
    mMeshLayer->setCrs( mBackupCrs );
}
