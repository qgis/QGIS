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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
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
#include "qgsprojectionselectiondialog.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgssettings.h"
#include "qgsprojecttimesettings.h"
#include "qgsproviderregistry.h"
#ifdef HAVE_3D
#include "qgsmeshlayer3drendererwidget.h"
#endif

#include <QFileDialog>
#include <QMessageBox>

QgsMeshLayerProperties::QgsMeshLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( QStringLiteral( "MeshLayerProperties" ), parent, fl )
  , mMeshLayer( qobject_cast<QgsMeshLayer *>( lyr ) )
{
  Q_ASSERT( mMeshLayer );

  setupUi( this );
  mRendererMeshPropertiesWidget = new QgsRendererMeshPropertiesWidget( mMeshLayer, canvas, this );
  mOptsPage_StyleContent->layout()->addWidget( mRendererMeshPropertiesWidget );

  mStaticScalarWidget->setLayer( mMeshLayer );

  mTemporalProviderTimeUnitComboBox->addItem( tr( "Seconds" ), QgsUnitTypes::TemporalSeconds );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Minutes" ), QgsUnitTypes::TemporalMinutes );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Hours" ), QgsUnitTypes::TemporalHours );
  mTemporalProviderTimeUnitComboBox->addItem( tr( "Days" ), QgsUnitTypes::TemporalDays );

  connect( mLayerOrigNameLineEd, &QLineEdit::textEdited, this, &QgsMeshLayerProperties::updateLayerName );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsMeshLayerProperties::changeCrs );
  connect( mAddDatasetButton, &QPushButton::clicked, this, &QgsMeshLayerProperties::addDataset );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsMeshLayerProperties::syncAndRepaint );

  connect( this, &QDialog::accepted, this, &QgsMeshLayerProperties::apply );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsMeshLayerProperties::apply );

  connect( mMeshLayer, &QgsMeshLayer::dataChanged, this, &QgsMeshLayerProperties::syncAndRepaint );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMeshLayerProperties::showHelp );

  connect( mTemporalReloadButton, &QPushButton::clicked, this, &QgsMeshLayerProperties::reloadTemporalProperties );
  connect( mTemporalDateTimeReference, &QDateTimeEdit::dateTimeChanged, this, &QgsMeshLayerProperties::onTimeReferenceChange );
  connect( mTemporalStaticDatasetCheckBox, &QCheckBox::toggled, this, &QgsMeshLayerProperties::onStaticDatasetCheckBoxChanged );
  connect( mMeshLayer, &QgsMeshLayer::activeScalarDatasetGroupChanged, mStaticScalarWidget, &QgsMeshStaticDatasetWidget::setScalarDatasetGroup );
  connect( mMeshLayer, &QgsMeshLayer::activeVectorDatasetGroupChanged, mStaticScalarWidget, &QgsMeshStaticDatasetWidget::setVectorDatasetGroup );

#ifdef HAVE_3D
  mMesh3DWidget = new QgsMeshLayer3DRendererWidget( mMeshLayer, canvas, mOptsPage_3DView );

  mOptsPage_3DView->setLayout( new QVBoxLayout( mOptsPage_3DView ) );
  mOptsPage_3DView->layout()->addWidget( mMesh3DWidget );
  mOptsPage_3DView->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#d-view-properties" ) );
#else
  delete mOptsPage_3DView;  // removes both the "3d view" list item and its page
#endif

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

  QString title = QString( tr( "Layer Properties - %1" ) ).arg( lyr->name() );

  if ( !mMeshLayer->styleManager()->isDefault( mMeshLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mMeshLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );

  //Add help page references
  mOptsPage_Information->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#information-properties" ) );
  mOptsPage_Source->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#source-properties" ) );
  mOptsPage_Style->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#symbology-properties" ) );
  mOptsPage_Rendering->setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#rendering-properties" ) );


  QPushButton *btnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsMeshLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsMeshLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsMeshLayerProperties::saveDefaultStyle );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsMeshLayerProperties::loadDefaultStyle );
  btnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsMeshLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( btnStyle, QDialogButtonBox::ResetRole );
}

void QgsMeshLayerProperties::syncToLayer()
{
  Q_ASSERT( mRendererMeshPropertiesWidget );

  QgsDebugMsgLevel( QStringLiteral( "populate general information tab" ), 4 );
  /*
   * Information Tab
   */
  QString info;
  if ( mMeshLayer->dataProvider() )
  {
    info += QStringLiteral( "<table>" );
    info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Uri" ) ).arg( mMeshLayer->dataProvider()->dataSourceUri() );
    info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Vertex count" ) ).arg( mMeshLayer->dataProvider()->vertexCount() );
    info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Face count" ) ).arg( mMeshLayer->dataProvider()->faceCount() );
    info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Edge count" ) ).arg( mMeshLayer->dataProvider()->edgeCount() );
    info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Dataset groups count" ) ).arg( mMeshLayer->dataProvider()->datasetGroupCount() );
    info += QStringLiteral( "</table>" );
  }
  else
  {
    info += tr( "Invalid data provider" );
  }
  mInformationTextBrowser->setText( info );

  QgsDebugMsgLevel( QStringLiteral( "populate source tab" ), 4 );
  /*
   * Source Tab
   */
  mLayerOrigNameLineEd->setText( mMeshLayer->name() );
  leDisplayName->setText( mMeshLayer->name() );
  whileBlocking( mCrsSelector )->setCrs( mMeshLayer->crs() );

  if ( mMeshLayer && mMeshLayer->dataProvider() )
  {
    mUriLabel->setText( mMeshLayer->dataProvider()->dataSourceUri() );
  }
  else
  {
    mUriLabel->setText( tr( "Not assigned" ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "populate styling tab" ), 4 );
  /*
   * Styling Tab
   */
  mRendererMeshPropertiesWidget->syncToLayer();

  /*
   * 3D View Tab
   */
#ifdef HAVE_3D
  mMesh3DWidget->setLayer( mMeshLayer );
#endif

  QgsDebugMsgLevel( QStringLiteral( "populate rendering tab" ), 4 );
  QgsMeshSimplificationSettings simplifySettings = mMeshLayer->meshSimplificationSettings();

  mSimplifyMeshGroupBox->setChecked( simplifySettings.isEnabled() );
  mSimplifyReductionFactorSpinBox->setValue( simplifySettings.reductionFactor() );
  mSimplifyMeshResolutionSpinBox->setValue( simplifySettings.meshResolution() );

  QgsDebugMsgLevel( QStringLiteral( "populate temporal tab" ), 4 );
  whileBlocking( mTemporalDateTimeReference )->setDateTime( mMeshLayer->temporalProperties()->referenceTime() );
  const QgsDateTimeRange timeRange = mMeshLayer->temporalProperties()->timeExtent();
  mTemporalDateTimeStart->setDateTime( timeRange.begin() );
  mTemporalDateTimeEnd->setDateTime( timeRange.end() );
  if ( mMeshLayer->dataProvider() )
  {
    mTemporalProviderTimeUnitComboBox->setCurrentIndex(
      mTemporalProviderTimeUnitComboBox->findData( mMeshLayer->dataProvider()->temporalCapabilities()->temporalUnit() ) );
  }

  mStaticScalarWidget->syncToLayer();
  mStaticScalarWidget->setVisible( !mMeshLayer->temporalProperties()->isActive() );
  mTemporalStaticDatasetCheckBox->setChecked( !mMeshLayer->temporalProperties()->isActive() );
}

void QgsMeshLayerProperties::addDataset()
{
  if ( !mMeshLayer->dataProvider() )
    return;

  QgsSettings settings;
  QString openFileDir = settings.value( QStringLiteral( "lastMeshDatasetDir" ), QDir::homePath(), QgsSettings::App ).toString();
  QString openFileString = QFileDialog::getOpenFileName( nullptr,
                           tr( "Load mesh datasets" ),
                           openFileDir,
                           QgsProviderRegistry::instance()->fileMeshDatasetFilters() );

  if ( openFileString.isEmpty() )
  {
    return; // canceled by the user
  }

  QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );
  QFile datasetFile( openFileString );

  bool isTemporalBefore = mMeshLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities();
  bool ok = mMeshLayer->dataProvider()->addDataset( openFileString );
  if ( ok )
  {
    if ( !isTemporalBefore && mMeshLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
    {
      mMeshLayer->temporalProperties()->setDefaultsFromDataProviderTemporalCapabilities(
        mMeshLayer->dataProvider()->temporalCapabilities() );

      if ( ! mMeshLayer->temporalProperties()->referenceTime().isValid() )
      {
        QDateTime referenceTime = QgsProject::instance()->timeSettings()->temporalRange().begin();
        if ( !referenceTime.isValid() ) // If project reference time is invalid, use current date
          referenceTime = QDateTime( QDate::currentDate(), QTime( 0, 0, 0, Qt::UTC ) );
        mMeshLayer->temporalProperties()->setReferenceTime( referenceTime, mMeshLayer->dataProvider()->temporalCapabilities() );
      }
    }

    syncToLayer();
    QMessageBox::information( this, tr( "Load mesh datasets" ), tr( "Datasets successfully added to the mesh layer" ) );
    emit mMeshLayer->dataSourceChanged();
  }
  else
  {
    QMessageBox::warning( this, tr( "Load mesh datasets" ), tr( "Could not read mesh dataset." ) );
  }
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
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  QString myMessage = mMeshLayer->saveDefaultStyle( defaultSavedFlag );
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

  QgsDebugMsgLevel( QStringLiteral( "processing style tab" ), 4 );
  /*
   * Style Tab
   */
  mRendererMeshPropertiesWidget->apply();

  /*
   * 3D View Tab
   */
#ifdef HAVE_3D
  mMesh3DWidget->apply();
#endif

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

  QgsDebugMsgLevel( QStringLiteral( "processing temporal tab" ), 4 );
  /*
   * Temporal Tab
   */

  mMeshLayer->setReferenceTime( mTemporalDateTimeReference->dateTime() );
  if ( mMeshLayer->dataProvider() )
    mMeshLayer->dataProvider()->setTemporalUnit(
      static_cast<QgsUnitTypes::TemporalUnit>( mTemporalProviderTimeUnitComboBox->currentData().toInt() ) );

  mStaticScalarWidget->apply();
  bool needEmitRendererChanged = mMeshLayer->temporalProperties()->isActive() == mTemporalStaticDatasetCheckBox->isChecked();
  mMeshLayer->temporalProperties()->setIsActive( !mTemporalStaticDatasetCheckBox->isChecked() );

  if ( needMeshUpdating )
    mMeshLayer->reload();

  if ( needEmitRendererChanged )
    emit mMeshLayer->rendererChanged();

  //make sure the layer is redrawn
  mMeshLayer->triggerRepaint();

  // notify the project we've made a change
  QgsProject::instance()->setDirty( true );
}

void QgsMeshLayerProperties::changeCrs( const QgsCoordinateReferenceSystem &crs )
{
  QgisApp::instance()->askUserForDatumTransform( crs, QgsProject::instance()->crs() );
  mMeshLayer->setCrs( crs );
}

void QgsMeshLayerProperties::updateLayerName( const QString &text )
{
  leDisplayName->setText( mMeshLayer->formatLayerName( text ) );
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
  const QgsDateTimeRange &timeExtent = mMeshLayer->dataProvider()->temporalCapabilities()->timeExtent( mTemporalDateTimeReference->dateTime() );
  mTemporalDateTimeStart->setDateTime( timeExtent.begin() );
  mTemporalDateTimeEnd->setDateTime( timeExtent.end() );
}

void QgsMeshLayerProperties::onStaticDatasetCheckBoxChanged()
{
  mStaticScalarWidget->setVisible( mTemporalStaticDatasetCheckBox->isChecked() );
}
