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
#include "qgshelp.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerproperties.h"
#include "qgsproject.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"

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
}

void QgsMeshLayerProperties::syncToLayer()
{
  Q_ASSERT( mRendererMeshPropertiesWidget );

  QgsDebugMsg( QStringLiteral( "populate general information tab" ) );
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
    info += QStringLiteral( "<tr><td>%1: </td><td>%2</td><tr>" ).arg( tr( "Dataset groups count" ) ).arg( mMeshLayer->dataProvider()->datasetGroupCount() );
    info += QStringLiteral( "</table>" );
  }
  else
  {
    info += tr( "Invalid data provider" );
  }
  mInformationTextBrowser->setText( info );

  QgsDebugMsg( QStringLiteral( "populate source tab" ) );
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

  QgsDebugMsg( QStringLiteral( "populate styling tab" ) );
  /*
   * Styling Tab
   */
  mRendererMeshPropertiesWidget->syncToLayer();
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
    return; //canceled by the user
  }

  QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );
  QFile datasetFile( openFileString );

  bool ok = mMeshLayer->dataProvider()->addDataset( openFileString );
  if ( ok )
  {
    syncToLayer();
    QMessageBox::information( this, tr( "Load mesh datasets" ), tr( "Datasets successfully added to the mesh layer" ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Load mesh datasets" ), tr( "Could not read mesh dataset." ) );
  }
}

void QgsMeshLayerProperties::apply()
{
  Q_ASSERT( mRendererMeshPropertiesWidget );

  QgsDebugMsg( QStringLiteral( "processing general tab" ) );
  /*
   * General Tab
   */
  mMeshLayer->setName( mLayerOrigNameLineEd->text() );

  QgsDebugMsg( QStringLiteral( "processing style tab" ) );
  /*
   * Style Tab
   */
  mRendererMeshPropertiesWidget->apply();

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
