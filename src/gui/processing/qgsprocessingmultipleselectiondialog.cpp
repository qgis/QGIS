/***************************************************************************
                             qgsprocessingmultipleselectiondialog.cpp
                             ------------------------------------
    Date                 : February 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmultipleselectiondialog.h"
#include "moc_qgsprocessingmultipleselectiondialog.cpp"
#include "qgsgui.h"
#include "qgssettings.h"
#include "qgsfileutils.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"
#include "qgsrasterlayer.h"
#include "qgspluginlayer.h"
#include "qgspointcloudlayer.h"
#include "qgsannotationlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsproject.h"
#include "processing/models/qgsprocessingmodelchildparametersource.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPushButton>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QDirIterator>
#include "qgsmimedatautils.h"
#include <QDragEnterEvent>

///@cond NOT_STABLE

QgsProcessingMultipleSelectionPanelWidget::QgsProcessingMultipleSelectionPanelWidget( const QVariantList &availableOptions, const QVariantList &selectedOptions, QWidget *parent )
  : QgsPanelWidget( parent )
  , mValueFormatter( []( const QVariant &v ) -> QString {
    if ( v.userType() == qMetaTypeId<QgsProcessingModelChildParameterSource>() )
      return v.value<QgsProcessingModelChildParameterSource>().staticValue().toString();
    else
      return v.toString();
  } )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  mSelectionList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSelectionList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mSelectionList->setDragDropMode( QAbstractItemView::InternalMove );

  mButtonSelectAll = new QPushButton( tr( "Select All" ) );
  mButtonBox->addButton( mButtonSelectAll, QDialogButtonBox::ActionRole );

  mButtonClearSelection = new QPushButton( tr( "Clear Selection" ) );
  mButtonBox->addButton( mButtonClearSelection, QDialogButtonBox::ActionRole );

  mButtonToggleSelection = new QPushButton( tr( "Toggle Selection" ) );
  mButtonBox->addButton( mButtonToggleSelection, QDialogButtonBox::ActionRole );

  connect( mButtonSelectAll, &QPushButton::clicked, this, [=] { selectAll( true ); } );
  connect( mButtonClearSelection, &QPushButton::clicked, this, [=] { selectAll( false ); } );
  connect( mButtonToggleSelection, &QPushButton::clicked, this, &QgsProcessingMultipleSelectionPanelWidget::toggleSelection );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked );
  populateList( availableOptions, selectedOptions );

  connect( mModel, &QStandardItemModel::itemChanged, this, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged );

  // When user moves an item, a new item is created and another one is removed, so we need to fire selectionChanged
  // see https://github.com/qgis/QGIS/issues/44270
  connect( mModel, &QStandardItemModel::rowsRemoved, this, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged );
}

void QgsProcessingMultipleSelectionPanelWidget::setValueFormatter( const std::function<QString( const QVariant & )> &formatter )
{
  mValueFormatter = formatter;
  // update item text using new formatter
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    mModel->item( i )->setText( mValueFormatter( mModel->item( i )->data( Qt::UserRole ) ) );
  }
}

QVariantList QgsProcessingMultipleSelectionPanelWidget::selectedOptions() const
{
  QVariantList options;
  options.reserve( mModel->rowCount() );
  bool hasModelSources = false;
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    QStandardItem *item = mModel->item( i );
    if ( !item )
    {
      continue;
    }

    if ( item->checkState() == Qt::Checked )
    {
      const QVariant option = item->data( Qt::UserRole );

      if ( option.userType() == qMetaTypeId<QgsProcessingModelChildParameterSource>() )
        hasModelSources = true;

      options << option;
    }
  }

  if ( hasModelSources )
  {
    // if any selected value is a QgsProcessingModelChildParameterSource, then we need to upgrade them all
    QVariantList originalOptions = options;
    options.clear();
    for ( const QVariant &option : originalOptions )
    {
      if ( option.userType() == qMetaTypeId<QgsProcessingModelChildParameterSource>() )
        options << option;
      else
        options << QVariant::fromValue( QgsProcessingModelChildParameterSource::fromStaticValue( option ) );
    }
  }

  return options;
}


void QgsProcessingMultipleSelectionPanelWidget::selectAll( const bool checked )
{
  const QList<QStandardItem *> items = currentItems();
  for ( QStandardItem *item : items )
  {
    item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
  }
}

void QgsProcessingMultipleSelectionPanelWidget::toggleSelection()
{
  const QList<QStandardItem *> items = currentItems();
  for ( QStandardItem *item : items )
  {
    item->setCheckState( item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked );
  }
}

QList<QStandardItem *> QgsProcessingMultipleSelectionPanelWidget::currentItems()
{
  QList<QStandardItem *> items;
  const QModelIndexList selection = mSelectionList->selectionModel()->selectedIndexes();
  if ( selection.size() > 1 )
  {
    items.reserve( selection.size() );
    for ( const QModelIndex &index : selection )
    {
      items << mModel->itemFromIndex( index );
    }
  }
  else
  {
    items.reserve( mModel->rowCount() );
    for ( int i = 0; i < mModel->rowCount(); ++i )
    {
      items << mModel->item( i );
    }
  }
  return items;
}

void QgsProcessingMultipleSelectionPanelWidget::populateList( const QVariantList &availableOptions, const QVariantList &selectedOptions )
{
  mModel = new QStandardItemModel( this );

  QVariantList remainingOptions = availableOptions;

  // we add selected options first, keeping the existing order of options
  for ( const QVariant &option : selectedOptions )
  {
    //    if isinstance(t, QgsProcessingModelChildParameterSource):
    //       item = QStandardItem(t.staticValue())
    // else:

    addOption( option, mValueFormatter( option ), true );
    remainingOptions.removeAll( option );
  }

  for ( const QVariant &option : std::as_const( remainingOptions ) )
  {
    addOption( option, mValueFormatter( option ), false );
  }

  mSelectionList->setModel( mModel );
}

QList<int> QgsProcessingMultipleSelectionPanelWidget::existingMapLayerFromMimeData( const QMimeData *data ) const
{
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  QList<int> indexes;
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    // is this uri from the current project?
    if ( QgsMapLayer *layer = u.mapLayer() )
    {
      for ( int i = 0; i < mModel->rowCount(); ++i )
      {
        // try to match project layers to current layers
        QString userRole = mModel->item( i )->data( Qt::UserRole ).toString();
        if ( userRole == layer->id() || userRole == layer->source() )
        {
          indexes.append( i );
        }
      }
    }
  }
  return indexes;
}

void QgsProcessingMultipleSelectionPanelWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  const QList<int> indexes = existingMapLayerFromMimeData( event->mimeData() );
  if ( !indexes.isEmpty() )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
  }
}

void QgsProcessingMultipleSelectionPanelWidget::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  const QList<int> indexes = existingMapLayerFromMimeData( event->mimeData() );
  if ( !indexes.isEmpty() )
  {
    // dropped an acceptable layer, phew
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();

    for ( const int i : indexes )
    {
      mModel->item( i )->setCheckState( Qt::Checked );
    }
    emit selectionChanged();
  }
}

void QgsProcessingMultipleSelectionPanelWidget::addOption( const QVariant &value, const QString &title, bool selected, bool updateExistingTitle )
{
  // don't add duplicate options
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    if ( mModel->item( i )->data( Qt::UserRole ) == value || ( mModel->item( i )->data( Qt::UserRole ).userType() == qMetaTypeId<QgsProcessingModelChildParameterSource>() && value.userType() == qMetaTypeId<QgsProcessingModelChildParameterSource>() && mModel->item( i )->data( Qt::UserRole ).value<QgsProcessingModelChildParameterSource>() == value.value<QgsProcessingModelChildParameterSource>() ) )
    {
      if ( updateExistingTitle )
        mModel->item( i )->setText( title );
      return;
    }
  }

  std::unique_ptr<QStandardItem> item = std::make_unique<QStandardItem>( title );
  item->setData( value, Qt::UserRole );
  item->setCheckState( selected ? Qt::Checked : Qt::Unchecked );
  item->setCheckable( true );
  item->setDropEnabled( false );
  mModel->appendRow( item.release() );
}

//
// QgsProcessingMultipleSelectionDialog
//


QgsProcessingMultipleSelectionDialog::QgsProcessingMultipleSelectionDialog( const QVariantList &availableOptions, const QVariantList &selectedOptions, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setWindowTitle( tr( "Multiple Selection" ) );
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, selectedOptions );
  vLayout->addWidget( mWidget );
  mWidget->buttonBox()->addButton( QDialogButtonBox::Cancel );
  connect( mWidget->buttonBox(), &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mWidget->buttonBox(), &QDialogButtonBox::rejected, this, &QDialog::reject );
  setLayout( vLayout );
}

void QgsProcessingMultipleSelectionDialog::setValueFormatter( const std::function<QString( const QVariant & )> &formatter )
{
  mWidget->setValueFormatter( formatter );
}

QVariantList QgsProcessingMultipleSelectionDialog::selectedOptions() const
{
  return mWidget->selectedOptions();
}


//
// QgsProcessingMultipleInputPanelWidget
//

QgsProcessingMultipleInputPanelWidget::QgsProcessingMultipleInputPanelWidget( const QgsProcessingParameterMultipleLayers *parameter, const QVariantList &selectedOptions, const QList<QgsProcessingModelChildParameterSource> &modelSources, QgsProcessingModelAlgorithm *model, QWidget *parent )
  : QgsProcessingMultipleSelectionPanelWidget( QVariantList(), selectedOptions, parent )
  , mParameter( parameter )
{
  QPushButton *addFileButton = new QPushButton( tr( "Add File(s)…" ) );
  connect( addFileButton, &QPushButton::clicked, this, &QgsProcessingMultipleInputPanelWidget::addFiles );
  buttonBox()->addButton( addFileButton, QDialogButtonBox::ActionRole );

  QPushButton *addDirButton = new QPushButton( tr( "Add Directory…" ) );
  connect( addDirButton, &QPushButton::clicked, this, &QgsProcessingMultipleInputPanelWidget::addDirectory );
  buttonBox()->addButton( addDirButton, QDialogButtonBox::ActionRole );
  setAcceptDrops( true );
  for ( const QgsProcessingModelChildParameterSource &source : modelSources )
  {
    addOption( QVariant::fromValue( source ), source.friendlyIdentifier( model ), false, true );
  }
}

void QgsProcessingMultipleInputPanelWidget::setProject( QgsProject *project )
{
  if ( mParameter->layerType() != Qgis::ProcessingSourceType::File )
    populateFromProject( project );
}

void QgsProcessingMultipleInputPanelWidget::addFiles()
{
  QgsSettings settings;
  QString path = settings.value( QStringLiteral( "/Processing/LastInputPath" ), QDir::homePath() ).toString();

  QString filter;
  if ( const QgsFileFilterGenerator *generator = dynamic_cast<const QgsFileFilterGenerator *>( mParameter ) )
    filter = generator->createFileFilter();
  else
    filter = QObject::tr( "All files (*.*)" );

  const QStringList filenames = QFileDialog::getOpenFileNames( this, tr( "Select File(s)" ), path, filter );
  if ( filenames.empty() )
    return;

  settings.setValue( QStringLiteral( "/Processing/LastInputPath" ), QFileInfo( filenames.at( 0 ) ).path() );

  for ( const QString &file : filenames )
  {
    addOption( file, file, true );
  }

  emit selectionChanged();
}

void QgsProcessingMultipleInputPanelWidget::addDirectory()
{
  QgsSettings settings;
  const QString path = settings.value( QStringLiteral( "/Processing/LastInputPath" ), QDir::homePath() ).toString();

  const QString dir = QFileDialog::getExistingDirectory( this, tr( "Select Directory" ), path );
  if ( dir.isEmpty() )
    return;

  settings.setValue( QStringLiteral( "/Processing/LastInputPath" ), dir );

  QStringList nameFilters;
  if ( const QgsFileFilterGenerator *generator = dynamic_cast<const QgsFileFilterGenerator *>( mParameter ) )
  {
    const QStringList extensions = QgsFileUtils::extensionsFromFilter( generator->createFileFilter() );
    for ( const QString &extension : extensions )
    {
      nameFilters << QStringLiteral( "*.%1" ).arg( extension );
      nameFilters << QStringLiteral( "*.%1" ).arg( extension.toUpper() );
      nameFilters << QStringLiteral( "*.%1" ).arg( extension.toLower() );
    }
  }

  QDirIterator it( dir, nameFilters, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories );
  while ( it.hasNext() )
  {
    const QString fullPath = it.next();
    if ( fullPath.endsWith( QLatin1String( ".dbf" ), Qt::CaseInsensitive ) )
    {
      if ( QFileInfo::exists( QStringLiteral( "%1.shp" ).arg( fullPath.chopped( 4 ) ) ) || QFileInfo::exists( QStringLiteral( "%1.SHP" ).arg( fullPath.chopped( 4 ) ) ) )
      {
        // Skip DBFs that are sidecar files to a Shapefile
        continue;
      }
    }
    else if ( fullPath.endsWith( QLatin1String( ".aux.xml" ), Qt::CaseInsensitive ) || fullPath.endsWith( QLatin1String( ".shp.xml" ), Qt::CaseInsensitive ) )
    {
      // Skip XMLs that are sidecar files  to datasets
      continue;
    }
    addOption( fullPath, fullPath, true );
  }
  emit selectionChanged();
}

QList<int> QgsProcessingMultipleInputPanelWidget::existingMapLayerFromMimeData( const QMimeData *data, QgsMimeDataUtils::UriList &handledUrls ) const
{
  handledUrls.clear();
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  QList<int> indexes;
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    // is this uri from the current project?
    bool matched = false;
    if ( QgsMapLayer *layer = u.mapLayer() )
    {
      for ( int i = 0; i < mModel->rowCount(); ++i )
      {
        // try to match project layers to current layers
        const QString userRole = mModel->item( i )->data( Qt::UserRole ).toString();
        if ( userRole == layer->id() || userRole == layer->source() )
        {
          indexes.append( i );
          matched = true;
        }
      }
    }

    if ( matched )
    {
      handledUrls.append( u );
    }
  }
  return indexes;
}


QStringList QgsProcessingMultipleInputPanelWidget::compatibleUrisFromMimeData( const QgsProcessingParameterMultipleLayers *parameter, const QMimeData *data, const QgsMimeDataUtils::UriList &skipUrls )
{
  QStringList skipUrlData;
  skipUrlData.reserve( skipUrls.size() );
  for ( const QgsMimeDataUtils::Uri &u : skipUrls )
  {
    skipUrlData.append( u.data() );
  }

  QStringList res;

  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    if ( skipUrlData.contains( u.data() ) )
      continue;

    // clang analyzer is not happy because of the multiple duplicate return branches, but it makes the code more readable
    // NOLINTBEGIN(bugprone-branch-clone)
    if ( ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer
           || parameter->layerType() == Qgis::ProcessingSourceType::Vector
           || parameter->layerType() == Qgis::ProcessingSourceType::VectorAnyGeometry
           || parameter->layerType() == Qgis::ProcessingSourceType::VectorLine
           || parameter->layerType() == Qgis::ProcessingSourceType::VectorPoint
           || parameter->layerType() == Qgis::ProcessingSourceType::VectorPolygon )
         && u.layerType == QLatin1String( "vector" ) )
    {
      bool acceptable = false;
      switch ( QgsWkbTypes::geometryType( u.wkbType ) )
      {
        case Qgis::GeometryType::Unknown:
          acceptable = true;
          break;

        case Qgis::GeometryType::Point:
          if ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::Vector || parameter->layerType() == Qgis::ProcessingSourceType::VectorAnyGeometry || parameter->layerType() == Qgis::ProcessingSourceType::VectorPoint )
            acceptable = true;
          break;

        case Qgis::GeometryType::Line:
          if ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::Vector || parameter->layerType() == Qgis::ProcessingSourceType::VectorAnyGeometry || parameter->layerType() == Qgis::ProcessingSourceType::VectorLine )
            acceptable = true;
          break;

        case Qgis::GeometryType::Polygon:
          if ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::Vector || parameter->layerType() == Qgis::ProcessingSourceType::VectorAnyGeometry || parameter->layerType() == Qgis::ProcessingSourceType::VectorPolygon )
            acceptable = true;
          break;

        case Qgis::GeometryType::Null:
          if ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::Vector )
            acceptable = true;
          break;
      }
      if ( acceptable )
        res.append( u.providerKey != QLatin1String( "ogr" ) ? QgsProcessingUtils::encodeProviderKeyAndUri( u.providerKey, u.uri ) : u.uri );
    }
    else if ( ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::Raster )
              && u.layerType == QLatin1String( "raster" ) && u.providerKey == QLatin1String( "gdal" ) )
      res.append( u.uri );
    else if ( ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::Mesh )
              && u.layerType == QLatin1String( "mesh" ) && u.providerKey == QLatin1String( "mdal" ) )
      res.append( u.uri );
    else if ( ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::PointCloud )
              && u.layerType == QLatin1String( "pointcloud" ) )
      res.append( u.uri );
    else if ( ( parameter->layerType() == Qgis::ProcessingSourceType::MapLayer || parameter->layerType() == Qgis::ProcessingSourceType::VectorTile )
              && u.layerType == QLatin1String( "vector-tile" ) )
      res.append( u.uri );
    // NOLINTEND(bugprone-branch-clone)
  }
  if ( !uriList.isEmpty() )
    return res;

  // second chance -- files dragged from file explorer, outside of QGIS
  QStringList rawPaths;
  if ( data->hasUrls() )
  {
    const QList<QUrl> urls = data->urls();
    rawPaths.reserve( urls.count() );
    for ( const QUrl &url : urls )
    {
      const QString local = url.toLocalFile();
      if ( !rawPaths.contains( local ) )
        rawPaths.append( local );
    }
  }
  if ( !data->text().isEmpty() && !rawPaths.contains( data->text() ) )
    rawPaths.append( data->text() );

  for ( const QString &path : std::as_const( rawPaths ) )
  {
    QFileInfo file( path );
    if ( file.isFile() )
    {
      // TODO - we should check to see if it's a valid extension for the parameter, but that's non-trivial
      res.append( path );
    }
  }

  return res;
}

void QgsProcessingMultipleInputPanelWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  // maybe dragging layers from the project
  QgsMimeDataUtils::UriList handledUris;
  const QList<int> indexes = existingMapLayerFromMimeData( event->mimeData(), handledUris );
  if ( !indexes.isEmpty() )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
    return;
  }

  // maybe dragging layers from browser or file explorer
  const QStringList uris = compatibleUrisFromMimeData( mParameter, event->mimeData(), handledUris );
  if ( !uris.isEmpty() )
  {
    // dragged an acceptable layer, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
  }
}

void QgsProcessingMultipleInputPanelWidget::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  QgsMimeDataUtils::UriList handledUris;
  const QList<int> indexes = existingMapLayerFromMimeData( event->mimeData(), handledUris );
  if ( !indexes.isEmpty() )
  {
    // dropped an acceptable layer, phew
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();

    for ( const int i : indexes )
    {
      mModel->item( i )->setCheckState( Qt::Checked );
    }
    emit selectionChanged();
  }

  // maybe dragging layers from browser or file explorer
  const QStringList uris = compatibleUrisFromMimeData( mParameter, event->mimeData(), handledUris );
  if ( !uris.isEmpty() )
  {
    for ( const QString &uri : uris )
    {
      addOption( uri, uri, true );
    }
    emit selectionChanged();
  }
}

void QgsProcessingMultipleInputPanelWidget::populateFromProject( QgsProject *project )
{
  connect( project, &QgsProject::layerRemoved, this, [&]( const QString &layerId ) {
    for ( int i = 0; i < mModel->rowCount(); ++i )
    {
      const QStandardItem *item = mModel->item( i );
      if ( item->data( Qt::UserRole ) == layerId )
      {
        bool isChecked = ( item->checkState() == Qt::Checked );
        mModel->removeRow( i );

        if ( isChecked )
          emit selectionChanged();

        break;
      }
    }
  } );

  QgsSettings settings;
  auto addLayer = [&]( const QgsMapLayer *layer ) {
    const QString authid = layer->crs().authid();
    QString title;
    if ( settings.value( QStringLiteral( "Processing/Configuration/SHOW_CRS_DEF" ), true ).toBool() && !authid.isEmpty() )
      title = QStringLiteral( "%1 [%2]" ).arg( layer->name(), authid );
    else
      title = layer->name();


    QString id = layer->id();
    if ( layer == project->mainAnnotationLayer() )
      id = QStringLiteral( "main" );

    for ( int i = 0; i < mModel->rowCount(); ++i )
    {
      // try to match project layers to current layers
      if ( mModel->item( i )->data( Qt::UserRole ) == layer->id() )
      {
        id = layer->id();
        break;
      }
      else if ( mModel->item( i )->data( Qt::UserRole ) == layer->source() )
      {
        id = layer->source();
        break;
      }
    }

    addOption( id, title, false, true );
  };

  switch ( mParameter->layerType() )
  {
    case Qgis::ProcessingSourceType::File:
      break;

    case Qgis::ProcessingSourceType::Raster:
    {
      const QList<QgsRasterLayer *> options = QgsProcessingUtils::compatibleRasterLayers( project, false );
      for ( const QgsRasterLayer *layer : options )
      {
        addLayer( layer );
      }
      break;
    }

    case Qgis::ProcessingSourceType::Mesh:
    {
      const QList<QgsMeshLayer *> options = QgsProcessingUtils::compatibleMeshLayers( project, false );
      for ( const QgsMeshLayer *layer : options )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::Plugin:
    {
      const QList<QgsPluginLayer *> options = QgsProcessingUtils::compatiblePluginLayers( project, false );
      for ( const QgsPluginLayer *layer : options )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::Annotation:
    {
      const QList<QgsAnnotationLayer *> options = QgsProcessingUtils::compatibleAnnotationLayers( project, false );
      for ( const QgsAnnotationLayer *layer : options )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::PointCloud:
    {
      const QList<QgsPointCloudLayer *> options = QgsProcessingUtils::compatiblePointCloudLayers( project, false );
      for ( const QgsPointCloudLayer *layer : options )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::VectorTile:
    {
      const QList<QgsVectorTileLayer *> options = QgsProcessingUtils::compatibleVectorTileLayers( project, false );
      for ( const QgsVectorTileLayer *layer : options )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::Vector:
    case Qgis::ProcessingSourceType::VectorAnyGeometry:
    {
      const QList<QgsVectorLayer *> options = QgsProcessingUtils::compatibleVectorLayers( project, QList<int>() << static_cast<int>( mParameter->layerType() ) );
      for ( const QgsVectorLayer *layer : options )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::MapLayer:
    {
      const QList<QgsVectorLayer *> vectors = QgsProcessingUtils::compatibleVectorLayers( project, QList<int>() );
      for ( const QgsVectorLayer *layer : vectors )
      {
        addLayer( layer );
      }
      const QList<QgsRasterLayer *> rasters = QgsProcessingUtils::compatibleRasterLayers( project );
      for ( const QgsRasterLayer *layer : rasters )
      {
        addLayer( layer );
      }
      const QList<QgsMeshLayer *> meshes = QgsProcessingUtils::compatibleMeshLayers( project );
      for ( const QgsMeshLayer *layer : meshes )
      {
        addLayer( layer );
      }
      const QList<QgsPluginLayer *> plugins = QgsProcessingUtils::compatiblePluginLayers( project );
      for ( const QgsPluginLayer *layer : plugins )
      {
        addLayer( layer );
      }
      const QList<QgsPointCloudLayer *> pointClouds = QgsProcessingUtils::compatiblePointCloudLayers( project );
      for ( const QgsPointCloudLayer *layer : pointClouds )
      {
        addLayer( layer );
      }
      const QList<QgsAnnotationLayer *> annotations = QgsProcessingUtils::compatibleAnnotationLayers( project );
      for ( const QgsAnnotationLayer *layer : annotations )
      {
        addLayer( layer );
      }

      break;
    }

    case Qgis::ProcessingSourceType::VectorPoint:
    case Qgis::ProcessingSourceType::VectorLine:
    case Qgis::ProcessingSourceType::VectorPolygon:
    {
      const QList<QgsVectorLayer *> vectors = QgsProcessingUtils::compatibleVectorLayers( project, QList<int>() << static_cast<int>( mParameter->layerType() ) );
      for ( const QgsVectorLayer *layer : vectors )
      {
        addLayer( layer );
      }
      break;
    }
  }
}

//
// QgsProcessingMultipleInputDialog
//

QgsProcessingMultipleInputDialog::QgsProcessingMultipleInputDialog( const QgsProcessingParameterMultipleLayers *parameter, const QVariantList &selectedOptions, const QList<QgsProcessingModelChildParameterSource> &modelSources, QgsProcessingModelAlgorithm *model, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setWindowTitle( tr( "Multiple Selection" ) );
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsProcessingMultipleInputPanelWidget( parameter, selectedOptions, modelSources, model );
  vLayout->addWidget( mWidget );
  mWidget->buttonBox()->addButton( QDialogButtonBox::Cancel );
  connect( mWidget->buttonBox(), &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mWidget->buttonBox(), &QDialogButtonBox::rejected, this, &QDialog::reject );
  setLayout( vLayout );
  setAcceptDrops( true );
}

QVariantList QgsProcessingMultipleInputDialog::selectedOptions() const
{
  return mWidget->selectedOptions();
}

void QgsProcessingMultipleInputDialog::setProject( QgsProject *project )
{
  mWidget->setProject( project );
}


///@endcond
