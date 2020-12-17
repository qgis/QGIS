/***************************************************************************
                          qgshandlebadlayers.cpp  -  description
                             -------------------
    begin                : Sat 5 Mar 2011
    copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshandlebadlayers.h"
#include "qgisapp.h"
#include "qgsauthconfigselect.h"
#include "qgsdataprovider.h"
#include "qgsguiutils.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsproviderregistry.h"
#include "qgsmessagebar.h"
#include "qgssettings.h"
#include "qgslayertreeregistrybridge.h"
#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgsprovidermetadata.h"
#include "qgsmaplayerfactory.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QUrl>
#include <QDir>
#include <QProgressDialog>
#include <QUrlQuery>

void QgsHandleBadLayersHandler::handleBadLayers( const QList<QDomNode> &layers )
{
  QgsTemporaryCursorRestoreOverride cursorOverride;

  QgsHandleBadLayers *dialog = new QgsHandleBadLayers( layers );

  dialog->buttonBox->button( QDialogButtonBox::Ignore )->setToolTip( tr( "Import all unavailable layers unmodified (you can fix them later)." ) );
  dialog->buttonBox->button( QDialogButtonBox::Ignore )->setText( tr( "Keep Unavailable Layers" ) );
  dialog->buttonBox->button( QDialogButtonBox::Discard )->setToolTip( tr( "Remove all unavailable layers from the project" ) );
  dialog->buttonBox->button( QDialogButtonBox::Discard )->setText( tr( "Remove Unavailable Layers" ) );
  dialog->buttonBox->button( QDialogButtonBox::Discard )->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );

  if ( dialog->layerCount() < layers.size() )
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Handle unavailable layers" ),
      tr( "%1 of %2 unavailable layers were not fixable." )
      .arg( layers.size() - dialog->layerCount() )
      .arg( layers.size() ),
      Qgis::MessageLevel::Warning );

  if ( dialog->layerCount() > 0 )
  {
    if ( dialog->exec() == dialog->Accepted )
    {
      emit layersChanged();
    }
  }

  delete dialog;
}

QgsHandleBadLayers::QgsHandleBadLayers( const QList<QDomNode> &layers )
  : QDialog( QgisApp::instance() )
  , mLayers( layers )
{
  setupUi( this );

  mBrowseButton = new QPushButton( tr( "Browse" ) );
  buttonBox->addButton( mBrowseButton, QDialogButtonBox::ActionRole );
  mBrowseButton->setDisabled( true );
  mAutoFindButton = new QPushButton( tr( "Auto-Find" ) );
  mAutoFindButton->setToolTip( tr( "Attempts to automatically find the layers based on the file name (can be slow)." ) );
  buttonBox->addButton( mAutoFindButton, QDialogButtonBox::ActionRole );
  mApplyButton = new QPushButton( tr( "Apply Changes" ) );
  mApplyButton->setToolTip( tr( "Apply fixes to unavailable layers and load them in the project if the new path is correct." ) );
  buttonBox->addButton( mApplyButton, QDialogButtonBox::ActionRole );

  connect( mLayerList, &QTableWidget::itemSelectionChanged, this, &QgsHandleBadLayers::selectionChanged );
  connect( mBrowseButton, &QAbstractButton::clicked, this, &QgsHandleBadLayers::browseClicked );
  connect( mApplyButton, &QAbstractButton::clicked, this, &QgsHandleBadLayers::apply );
  connect( mAutoFindButton, &QAbstractButton::clicked, this, &QgsHandleBadLayers::autoFind );
  connect( buttonBox->button( QDialogButtonBox::Ignore ), &QPushButton::clicked, this, &QgsHandleBadLayers::reject );
  connect( buttonBox->button( QDialogButtonBox::Discard ), &QPushButton::clicked, this, &QgsHandleBadLayers::accept );

  mLayerList->clear();
  mLayerList->setSortingEnabled( true );
  mLayerList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mLayerList->setColumnCount( 5 );
  mLayerList->setColumnWidth( 3, 75 );

  mLayerList->setHorizontalHeaderLabels( QStringList()
                                         << tr( "Layer name" )
                                         << tr( "Type" )
                                         << tr( "Provider" )
                                         << tr( "Auth config" )
                                         << tr( "Datasource" )
                                       );

  mLayerList->horizontalHeader()->setSectionsMovable( true );
  mLayerList->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );

  int j = 0;
  for ( int i = 0; i < mLayers.size(); i++ )
  {
    const QDomNode &node = mLayers[i];

    const QString name = node.namedItem( QStringLiteral( "layername" ) ).toElement().text();
    const QString type = node.toElement().attribute( QStringLiteral( "type" ) );
    const QString layerId = node.namedItem( QStringLiteral( "id" ) ).toElement().text();
    const QString datasource = node.namedItem( QStringLiteral( "datasource" ) ).toElement().text();
    const QString provider = node.namedItem( QStringLiteral( "provider" ) ).toElement().text();

    bool providerFileBased = false;
    if ( const QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider ) )
      providerFileBased = metadata->providerCapabilities() & QgsProviderMetadata::FileBasedUris;

    const QString basepath = QFileInfo( datasource ).absolutePath();
    mOriginalFileBase[ layerId ].append( basepath );

    QgsDebugMsgLevel( QStringLiteral( "name=%1 type=%2 provider=%3 datasource='%4'" )
                      .arg( name,
                            type,
                            provider,
                            datasource ), 2 );

    mLayerList->setRowCount( j + 1 );

    QTableWidgetItem *item = nullptr;

    bool ok = false;
    item = new QTableWidgetItem( name );
    item->setData( static_cast< int >( CustomRoles::Index ), i );
    item->setData( static_cast< int >( CustomRoles::Provider ), provider );
    item->setData( static_cast< int >( CustomRoles::ProviderIsFileBased ), providerFileBased );
    item->setData( static_cast< int >( CustomRoles::LayerId ), layerId );
    item->setData( static_cast< int >( CustomRoles::LayerType ), static_cast< int >( QgsMapLayerFactory::typeFromString( type, ok ) ) );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 0, item );

    item = new QTableWidgetItem( type );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 1, item );

    item = new QTableWidgetItem( provider );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 2, item );

    if ( QgsAuthConfigUriEdit::hasConfigId( datasource ) )
    {
      QToolButton *btn = new QToolButton( this );
      btn->setMaximumWidth( 75 );
      btn->setMinimumHeight( 24 );
      btn->setText( tr( "Edit" ) );
      btn->setProperty( "row", j );
      connect( btn, &QAbstractButton::clicked, this, &QgsHandleBadLayers::editAuthCfg );
      mLayerList->setCellWidget( j, 3, btn );
    }
    else
    {
      item = new QTableWidgetItem( QString() );
      mLayerList->setItem( j, 3, item );
    }

    item = new QTableWidgetItem( datasource );
    mLayerList->setItem( j, 4, item );

    j++;
  }

  // mLayerList->resizeColumnsToContents();
}

void QgsHandleBadLayers::selectionChanged()
{
  mBrowseButton->setEnabled( !fileBasedRows( true ).isEmpty() );
}

QString QgsHandleBadLayers::filename( int row )
{
  const bool providerFileBased = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::ProviderIsFileBased ) ).toBool();
  if ( !providerFileBased )
    return QString();

  const QString provider = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::Provider ) ).toString();
  const QString datasource = mLayerList->item( row, 4 )->text();

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( provider, datasource );
  return parts.value( QStringLiteral( "path" ) ).toString();
}

void QgsHandleBadLayers::setFilename( int row, const QString &filename )
{
  if ( !QFileInfo::exists( filename ) )
    return;

  const QString provider = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::Provider ) ).toString();
  QTableWidgetItem *item = mLayerList->item( row, 4 );

  const QString datasource = item->text();

  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( provider, datasource );
  parts.insert( QStringLiteral( "path" ), filename );

  item->setText( QgsProviderRegistry::instance()->encodeUri( provider, parts ) );
}

QList< int > QgsHandleBadLayers::fileBasedRows( bool selectedOnly )
{
  QList< int > res;
  if ( selectedOnly )
  {
    const QList<QTableWidgetItem *> selectedItems = mLayerList->selectedItems();

    for ( QTableWidgetItem *item : selectedItems )
    {
      if ( item->column() != 0 )
        continue;

      const bool providerFileBased = mLayerList->item( item->row(), 0 )->data( static_cast< int >( CustomRoles::ProviderIsFileBased ) ).toBool();
      if ( !providerFileBased )
        continue;

      res << item->row();
    }

  }
  else
  {
    for ( int row = 0; row < mLayerList->rowCount(); row++ )
    {
      const bool providerFileBased = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::ProviderIsFileBased ) ).toBool();
      if ( !providerFileBased )
        continue;

      res << row;
    }
  }
  return res;
}

void QgsHandleBadLayers::browseClicked()
{
  const QList< int > selectedRows = fileBasedRows( true );

  if ( selectedRows.empty() )
    return;

  if ( selectedRows.size() == 1 )
  {
    int row = selectedRows.at( 0 );

    QString memoryQualifier;

    const QgsMapLayerType layerType = static_cast< QgsMapLayerType >( mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::LayerType ) ).toInt() );
    const QString provider = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::Provider ) ).toString();

    QString fileFilter;
    switch ( layerType )
    {
      case QgsMapLayerType::VectorLayer:
        memoryQualifier = QStringLiteral( "lastVectorFileFilter" );
        fileFilter = QgsProviderRegistry::instance()->providerMetadata( provider )->filters( QgsProviderMetadata::FilterType::FilterVector );
        break;
      case QgsMapLayerType::RasterLayer:
        memoryQualifier = QStringLiteral( "lastRasterFileFilter" );
        fileFilter = QgsProviderRegistry::instance()->providerMetadata( provider )->filters( QgsProviderMetadata::FilterType::FilterRaster );
        break;
      case QgsMapLayerType::MeshLayer:
        memoryQualifier = QStringLiteral( "lastMeshFileFilter" );
        fileFilter = QgsProviderRegistry::instance()->fileMeshFilters();
        break;
      case QgsMapLayerType::VectorTileLayer:
        memoryQualifier = QStringLiteral( "lastVectorTileFileFilter" );
        // not quite right -- but currently there's no generic method to get vector tile filters...
        fileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
        break;
      case QgsMapLayerType::PointCloudLayer:
        memoryQualifier = QStringLiteral( "lastPointCloudFileFilter" );
        fileFilter = QgsProviderRegistry::instance()->providerMetadata( provider )->filters( QgsProviderMetadata::FilterType::FilterPointCloud );
        break;

      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }

    QString fn = filename( row );
    if ( fn.isNull() )
      return;

    QStringList selectedFiles;
    QString enc;
    QString title = tr( "Select File to Replace '%1'" ).arg( fn );

    QgsGuiUtils::openFilesRememberingFilter( memoryQualifier, fileFilter, selectedFiles, enc, title );
    if ( selectedFiles.size() != 1 )
    {
      QMessageBox::information( this, title, tr( "Please select exactly one file." ) );
      return;
    }

    setFilename( row, selectedFiles[0] );
  }
  else
  {
    QString title = tr( "Select New Directory of Selected Files" );

    QgsSettings settings;
    QString lastDir = settings.value( QStringLiteral( "UI/missingDirectory" ), QDir::homePath() ).toString();
    QString selectedFolder = QFileDialog::getExistingDirectory( this, title, lastDir );
    if ( selectedFolder.isEmpty() )
    {
      return;
    }

    QDir dir( selectedFolder );
    if ( !dir.exists() )
    {
      return;
    }

    for ( int row : selectedRows )
    {
      const bool providerFileBased = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::ProviderIsFileBased ) ).toBool();
      if ( !providerFileBased )
        continue;

      QString fn = filename( row );
      if ( fn.isEmpty() )
        continue;

      QFileInfo fi( fn );
      fi.setFile( dir, fi.fileName() );
      if ( !fi.exists() )
        continue;

      setFilename( row, fi.absoluteFilePath() );
    }
  }
}

void QgsHandleBadLayers::editAuthCfg()
{
  QToolButton *btn = qobject_cast<QToolButton *>( sender() );
  int row = -1;
  for ( int i = 0; i < mLayerList->rowCount(); i++ )
  {
    if ( mLayerList->cellWidget( i, 3 ) == btn )
    {
      row = i;
      break;
    }
  }

  if ( row == -1 )
    return;

  const QString provider = mLayerList->item( row, 0 )->data( static_cast< int >( CustomRoles::Provider ) ).toString();

  QString prevuri = mLayerList->item( row, 4 )->text();

  QgsAuthConfigUriEdit *dlg = new QgsAuthConfigUriEdit( this, prevuri, provider );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 500, 500 );
  if ( dlg->exec() )
  {
    QString newuri( dlg->dataSourceUri() );
    if ( newuri != prevuri )
    {
      mLayerList->item( row, 4 )->setText( newuri );
    }
  }
  dlg->deleteLater();
}

void QgsHandleBadLayers::apply()
{
  QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( true );
  QDir::setCurrent( QgsProject::instance()->absolutePath() );
  for ( int i = 0; i < mLayerList->rowCount(); i++ )
  {
    const int idx = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::Index ) ).toInt();
    QDomNode &node = const_cast<QDomNode &>( mLayers[ idx ] );

    QTableWidgetItem *item = mLayerList->item( i, 4 );
    QString datasource = item->text();
    const QString layerId = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::LayerId ) ).toString();
    const QString name { mLayerList->item( i, 0 )->text() };
    const QString provider = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::Provider ) ).toString();

    const bool dataSourceWasAutoRepaired = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::DataSourceWasAutoRepaired ) ).toBool();
    const bool providerFileBased = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::ProviderIsFileBased ) ).toBool();
    if ( providerFileBased && !dataSourceWasAutoRepaired )
    {
      QVariantMap providerMap = QgsProviderRegistry::instance()->decodeUri( provider, datasource );
      const QString filePath = providerMap[ QStringLiteral( "path" ) ].toString();
      const QFileInfo dataInfo = QFileInfo( filePath );

      bool fixedPath = false;
      const QString correctedPath = checkBasepath( layerId, dataInfo.absoluteDir().path(), dataInfo.fileName(), fixedPath );
      if ( fixedPath && correctedPath != filePath )
      {
        // re-encode uri for provider
        providerMap.insert( QStringLiteral( "path" ), correctedPath );
        datasource = QgsProviderRegistry::instance()->encodeUri( provider, providerMap );
      }
    }

    bool dataSourceChanged { false };


    // Try first to change the datasource of the existing layers, this will
    // maintain the current status (checked/unchecked) and group
    if ( QgsProject::instance()->mapLayer( layerId ) )
    {
      QgsDataProvider::ProviderOptions options;
      QgsMapLayer *mapLayer = QgsProject::instance()->mapLayer( layerId );
      if ( mapLayer )
      {
        QString subsetString;
        QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer *>( mapLayer );
        if ( vlayer )
        {
          // store the previous layer subset string, so we can restore after fixing the data source
          subsetString = vlayer->subsetString();
        }

        mapLayer->setDataSource( datasource, name, provider, options );
        dataSourceChanged = mapLayer->isValid();

        if ( dataSourceChanged && vlayer && !subsetString.isEmpty() )
        {
          vlayer->setSubsetString( subsetString );
        }
      }
    }

    // If the data source was changed successfully, remove the bad layer from the dialog
    // otherwise, try to set the new datasource in the XML node and reload the layer,
    // finally marks with red all remaining bad layers.
    if ( dataSourceChanged )
    {
      mLayerList->removeRow( i-- );
    }
    else
    {
      node.namedItem( QStringLiteral( "datasource" ) ).toElement().firstChild().toText().setData( datasource );
      if ( QgsProject::instance()->readLayer( node ) )
      {
        mLayerList->removeRow( i-- );
      }
      else
      {
        item->setForeground( QBrush( Qt::red ) );
      }
    }
  }

  // Final cleanup: remove any bad layer (none should remain by now)
  if ( mLayerList->rowCount() == 0 )
  {
    QList<QgsMapLayer *> toRemove;
    const auto mapLayers = QgsProject::instance()->mapLayers();
    for ( const auto &l : mapLayers )
    {
      if ( ! l->isValid() )
        toRemove << l;
    }
    QgsProject::instance()->removeMapLayers( toRemove );
    accept();
  }

  QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( false );

}

void QgsHandleBadLayers::accept()
{

  if ( mLayerList->rowCount() > 0  &&
       QMessageBox::warning( this,
                             tr( "Unhandled layer will be lost." ),
                             tr( "There are still %n unhandled layer(s). If they are not fixed, they will be disabled/deactivated until the project is opened again.",
                                 "unhandled layers",
                                 mLayerList->rowCount() ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }
  QList<QgsMapLayer *> toRemove;
  for ( const auto &l : QgsProject::instance()->mapLayers( ) )
  {
    if ( ! l->isValid() )
      toRemove << l;
  }
  QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( true );
  QgsProject::instance()->removeMapLayers( toRemove );
  QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( false );
  mLayerList->clear();

  QDialog::accept();
}

int QgsHandleBadLayers::layerCount()
{
  return mLayerList->rowCount();
}

QString QgsHandleBadLayers::checkBasepath( const QString &layerId, const QString &newPath, const QString &fileName, bool &foundPath )
{
  foundPath = false;
  const QString originalBase = mOriginalFileBase.value( layerId );
  const QDir newpathDir = QDir( newPath );
  bool exists = newpathDir.exists( fileName );
  if ( exists )
  {
    foundPath = true;
    const QString newBasepath = newpathDir.absolutePath();
    if ( !mAlternativeBasepaths.value( originalBase ).contains( newBasepath ) )
      mAlternativeBasepaths[ originalBase ].append( newBasepath );
    return newpathDir.filePath( fileName );
  }
  else if ( mAlternativeBasepaths.contains( originalBase ) )
  {
    const QStringList altPaths = mAlternativeBasepaths.value( originalBase );
    for ( const QString &altPath : altPaths )
    {
      QDir altDir( altPath );
      if ( altDir.exists( fileName ) && QFileInfo( altDir.filePath( fileName ) ).isFile() )
      {
        foundPath = true;
        return altDir.filePath( fileName );
      }
    }
  }
  return mOriginalFileBase.value( layerId );
}

void QgsHandleBadLayers::autoFind()
{
  QDir::setCurrent( QgsProject::instance()->absolutePath() );
  QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( true );

  const QList<int> layersToFind = fileBasedRows( !mLayerList->selectedItems().isEmpty() );

  QProgressDialog progressDialog( QObject::tr( "Searching files" ), 0, 1, layersToFind.size(), this, Qt::Dialog );
  QgsTaskManager *manager = QgsApplication::taskManager();

  for ( int i : std::as_const( layersToFind ) )
  {
    const int idx = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::Index ) ).toInt();
    QDomNode &node = const_cast<QDomNode &>( mLayers[ idx ] );

    QTableWidgetItem *item = mLayerList->item( i, 4 );
    QString datasource = item->text();
    QString fileName;
    const QString layerId = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::LayerId ) ).toString();
    const QString name { mLayerList->item( i, 0 )->text() };
    const QFileInfo dataInfo = QFileInfo( datasource );
    const QString basepath = dataInfo.absoluteDir().path();
    const QString longName = dataInfo.fileName();
    const QString provider = mLayerList->item( i, 0 )->data( static_cast< int >( CustomRoles::Provider ) ).toString();

    progressDialog.setValue( i );
    QChar sentenceEnd = ( name.length() > 15 ) ? QChar( 0x2026 ) : '.';
    progressDialog.setLabelText( QObject::tr( "Searching for file: %1 \n [ %2 of %3 ] " ).arg( name.left( 15 ) + sentenceEnd,
                                 QLocale().toString( i + 1 ), QLocale().toString( layersToFind.size() ) ) );
    progressDialog.open();

    QVariantMap providerMap = QgsProviderRegistry::instance()->decodeUri( provider, dataInfo.absoluteFilePath() );
    if ( providerMap.contains( QStringLiteral( "path" ) ) )
      fileName = QFileInfo( providerMap[ QStringLiteral( "path" ) ].toString() ).fileName();
    else
    {
      item->setForeground( QBrush( Qt::red ) );
      continue;
    }

    bool fixedPath = false;
    datasource = checkBasepath( layerId, basepath, fileName, fixedPath );

    bool dataSourceChanged { false };

    // Try first to change the datasource of the existing layers, this will
    // maintain the current status (checked/unchecked) and group
    if ( !datasource.isEmpty() && QgsProject::instance()->mapLayer( layerId ) )
    {
      QgsDataProvider::ProviderOptions options;
      QgsMapLayer *mapLayer = QgsProject::instance()->mapLayer( layerId );
      if ( mapLayer )
      {
        mapLayer->setDataSource( datasource.replace( fileName, longName ), name, provider, options );
        dataSourceChanged = mapLayer->isValid();
      }
    }

    if ( !dataSourceChanged )
    {
      QStringList filesFound;
      QgsFileSearchTask *fileutil = new QgsFileSearchTask( fileName,  basepath, 4, 4, QgsProject::instance()->absolutePath() );
      fileutil->setDescription( "Searching for " + fileName );
      manager->addTask( fileutil );
      while ( !( ( fileutil->status() == QgsTask::Complete ) || ( fileutil->status() == QgsTask::Terminated ) ) )
      {
        QCoreApplication::processEvents();
        if ( progressDialog.wasCanceled() )
          fileutil->cancel();
      }
      // fileutil->waitForFinished();
      if ( !( fileutil->isActive() ) )
        filesFound = fileutil->results();

      if ( filesFound.length() > 1 )
      {
        bool ok;
        datasource = QInputDialog::getItem( nullptr, QObject::tr( "Select layer source" ), QObject::tr( "Many files were found, please select the source for %1 " ).arg( fileName ), filesFound, 0, false, &ok, Qt::Popup );
        if ( !ok )
          datasource = filesFound.at( 0 );
      }
      else
      {
        QString tdatasource = filesFound.length() == 1 ? filesFound.at( 0 ) : QString();
        if ( !tdatasource.isEmpty() )
          datasource = tdatasource;
      }

      if ( QgsProject::instance()->mapLayer( layerId ) && !( datasource.isEmpty() ) )
      {
        QgsDataProvider::ProviderOptions options;
        QgsMapLayer *mapLayer = QgsProject::instance()->mapLayer( layerId );
        if ( mapLayer )
        {
          mapLayer->setDataSource( datasource.replace( fileName, longName ), name, provider, options );
          dataSourceChanged = mapLayer->isValid();
        }
      }
      if ( dataSourceChanged )
      {
        QString cleanSrc = QFileInfo( datasource ).absoluteDir().absolutePath();
        checkBasepath( layerId, cleanSrc, fileName, fixedPath );
      }
    }

    // If the data source was changed successfully, remove the bad layer from the dialog
    // otherwise, try to set the new datasource in the XML node and reload the layer,
    // finally marks with red all remaining bad layers.
    if ( dataSourceChanged )
    {
      setFilename( i, datasource );
      item->setText( datasource );
      item->setForeground( QBrush( Qt::green ) );
      mLayerList->item( i, 0 )->setData( static_cast< int >( CustomRoles::DataSourceWasAutoRepaired ), QVariant( true ) );
    }
    else
    {
      node.namedItem( QStringLiteral( "datasource" ) ).toElement().firstChild().toText().setData( datasource );
      if ( QgsProject::instance()->readLayer( node ) )
      {
        mLayerList->removeRow( i-- );
      }
      else
      {
        item->setForeground( QBrush( Qt::red ) );
      }
    }
    if ( progressDialog.wasCanceled() )
      break;
  }

  QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( false );

}



