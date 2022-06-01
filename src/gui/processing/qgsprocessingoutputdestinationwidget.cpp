/***************************************************************************
                             qgsprocessingmatrixparameterdialog.cpp
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

#include "qgsprocessingoutputdestinationwidget.h"
#include "qgsgui.h"
#include "qgsprocessingparameters.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsnewdatabasetablenamewidget.h"
#include "qgssettings.h"
#include "qgsfileutils.h"
#include "qgsdatasourceuri.h"
#include "qgsencodingfiledialog.h"
#include "qgsdatasourceselectdialog.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingalgorithm.h"
#include "qgsfieldmappingwidget.h"
#include <QMenu>
#include <QFileDialog>
#include <QInputDialog>
#include <QCheckBox>
#include <QUrl>

///@cond NOT_STABLE

QgsProcessingLayerOutputDestinationWidget::QgsProcessingLayerOutputDestinationWidget( const QgsProcessingDestinationParameter *param, bool defaultSelection, QWidget *parent )
  : QWidget( parent )
  , mParameter( param )
  , mDefaultSelection( defaultSelection )
{
  Q_ASSERT( mParameter );

  setupUi( this );

  leText->setClearButtonEnabled( false );

  connect( leText, &QLineEdit::textEdited, this, &QgsProcessingLayerOutputDestinationWidget::textChanged );

  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsProcessingLayerOutputDestinationWidget::menuAboutToShow );
  mSelectButton->setMenu( mMenu );
  mSelectButton->setPopupMode( QToolButton::InstantPopup );

  QgsSettings settings;
  mEncoding = settings.value( QStringLiteral( "/Processing/encoding" ), QStringLiteral( "System" ) ).toString();

  if ( !mParameter->defaultValueForGui().isValid() )
  {
    // no default value -- we default to either skipping the output or a temporary output, depending on the createByDefault value
    if ( mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional && !mParameter->createByDefault() )
      setValue( QVariant() );
    else
      setValue( QgsProcessing::TEMPORARY_OUTPUT );
  }
  else
  {
    setValue( mParameter->defaultValueForGui() );
  }

  setToolTip( mParameter->toolTip() );

  setAcceptDrops( true );
  leText->setAcceptDrops( false );
}

bool QgsProcessingLayerOutputDestinationWidget::outputIsSkipped() const
{
  return leText->text().isEmpty() && !mUseTemporary;
}

void QgsProcessingLayerOutputDestinationWidget::setValue( const QVariant &value )
{
  const bool prevSkip = outputIsSkipped();
  mUseRemapping = false;
  if ( !value.isValid() || ( value.type() == QVariant::String && value.toString().isEmpty() ) )
  {
    if ( mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
      skipOutput();
    else
      saveToTemporary();
  }
  else
  {
    if ( value.toString() == QLatin1String( "memory:" ) || value.toString() == QgsProcessing::TEMPORARY_OUTPUT )
    {
      saveToTemporary();
    }
    else if ( value.canConvert< QgsProcessingOutputLayerDefinition >() )
    {
      const QgsProcessingOutputLayerDefinition def = value.value< QgsProcessingOutputLayerDefinition >();
      if ( def.sink.staticValue().toString() == QLatin1String( "memory:" ) || def.sink.staticValue().toString() == QgsProcessing::TEMPORARY_OUTPUT || def.sink.staticValue().toString().isEmpty() )
      {
        saveToTemporary();
      }
      else
      {
        const QVariant prev = QgsProcessingLayerOutputDestinationWidget::value();
        leText->setText( def.sink.staticValue().toString() );
        mUseTemporary = false;
        if ( prevSkip )
          emit skipOutputChanged( false );
        if ( prev != QgsProcessingLayerOutputDestinationWidget::value() )
          emit destinationChanged();
      }
      mUseRemapping = def.useRemapping();
      mRemapDefinition = def.remappingDefinition();
      mEncoding = def.createOptions.value( QStringLiteral( "fileEncoding" ) ).toString();
    }
    else
    {
      const QVariant prev = QgsProcessingLayerOutputDestinationWidget::value();
      leText->setText( value.toString() );
      mUseTemporary = false;
      if ( prevSkip )
        emit skipOutputChanged( false );

      if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
      {
        if ( prev.toString() != QgsProcessingLayerOutputDestinationWidget::value().toString() )
          emit destinationChanged();
      }
      else
      {
        if ( !prev.canConvert<QgsProcessingOutputLayerDefinition>() ||
             !( prev.value< QgsProcessingOutputLayerDefinition >() == QgsProcessingLayerOutputDestinationWidget::value().value< QgsProcessingOutputLayerDefinition >() ) )
          emit destinationChanged();
      }
    }
  }
}

QVariant QgsProcessingLayerOutputDestinationWidget::value() const
{
  QgsSettings settings;
  QString key;
  if ( mUseTemporary && mParameter->type() == QgsProcessingParameterFeatureSink::typeName() )
  {
    key = QgsProcessing::TEMPORARY_OUTPUT;
  }
  else if ( mUseTemporary && !mDefaultSelection )
  {
    key = QgsProcessing::TEMPORARY_OUTPUT;
  }
  else
  {
    key = leText->text();
  }

  if ( key.isEmpty() && mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
    return QVariant();

  QString provider;
  QString uri;
  if ( !key.isEmpty() && key != QgsProcessing::TEMPORARY_OUTPUT
       && !key.startsWith( QLatin1String( "memory:" ) )
       && !key.startsWith( QLatin1String( "ogr:" ) )
       && !key.startsWith( QLatin1String( "postgres:" ) )
       && !key.startsWith( QLatin1String( "postgis:" ) )
       && !QgsProcessingUtils::decodeProviderKeyAndUri( key, provider, uri ) )
  {
    // output should be a file path
    QString folder = QFileInfo( key ).path();
    if ( folder == '.' )
    {
      // output name does not include a folder - use default
      QString defaultFolder = settings.value( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ) ).toString();
      key = QDir( defaultFolder ).filePath( key );
    }
  }

  if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() )
    return key;
  else if ( mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
    return key;

  QgsProcessingOutputLayerDefinition value( key );
  value.createOptions.insert( QStringLiteral( "fileEncoding" ), mEncoding );
  if ( mUseRemapping )
    value.setRemappingDefinition( mRemapDefinition );
  return value;
}

void QgsProcessingLayerOutputDestinationWidget::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mBrowserModel = context.browserModel();
}

void QgsProcessingLayerOutputDestinationWidget::setContext( QgsProcessingContext *context )
{
  mContext = context;
}

void QgsProcessingLayerOutputDestinationWidget::registerProcessingParametersGenerator( QgsProcessingParametersGenerator *generator )
{
  mParametersGenerator = generator;
}

void QgsProcessingLayerOutputDestinationWidget::addOpenAfterRunningOption()
{
  Q_ASSERT( mOpenAfterRunningCheck == nullptr );
  mOpenAfterRunningCheck = new QCheckBox( tr( "Open output file after running algorithm" ) );
  mOpenAfterRunningCheck->setChecked( !outputIsSkipped() );
  mOpenAfterRunningCheck->setEnabled( !outputIsSkipped() );
  gridLayout->addWidget( mOpenAfterRunningCheck, 1, 0, 1, 2 );

  connect( this, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged, this, [ = ]( bool skipped )
  {
    bool enabled = !skipped;
    mOpenAfterRunningCheck->setEnabled( enabled );
    mOpenAfterRunningCheck->setChecked( enabled );
  } );
}

bool QgsProcessingLayerOutputDestinationWidget::openAfterRunning() const
{
  return mOpenAfterRunningCheck && mOpenAfterRunningCheck->isChecked();
}

void QgsProcessingLayerOutputDestinationWidget::menuAboutToShow()
{
  mMenu->clear();

  if ( !mDefaultSelection )
  {
    if ( mParameter->flags() & QgsProcessingParameterDefinition::FlagOptional )
    {
      QAction *actionSkipOutput = new QAction( tr( "Skip Output" ), this );
      connect( actionSkipOutput, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::skipOutput );
      mMenu->addAction( actionSkipOutput );
    }

    QAction *actionSaveToTemp = nullptr;
    if ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() && mParameter->supportsNonFileBasedOutput() )
    {
      // use memory layers for temporary layers if supported
      actionSaveToTemp = new QAction( tr( "Create Temporary Layer" ), this );
    }
    else if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() )
    {
      actionSaveToTemp = new QAction( tr( "Save to a Temporary Directory" ), this );
    }
    else
    {
      actionSaveToTemp = new QAction( tr( "Save to a Temporary File" ), this );
    }

    connect( actionSaveToTemp, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::saveToTemporary );
    mMenu->addAction( actionSaveToTemp );
  }

  QAction *actionSaveToFile = nullptr;
  if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() )
  {
    actionSaveToFile = new QAction( tr( "Save to Directory…" ), this );
    connect( actionSaveToFile, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::selectDirectory );
  }
  else
  {
    actionSaveToFile = new QAction( tr( "Save to File…" ), this );
    connect( actionSaveToFile, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::selectFile );
  }
  mMenu->addAction( actionSaveToFile );

  if ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() && mParameter->supportsNonFileBasedOutput() )
  {
    QAction *actionSaveToGpkg = new QAction( tr( "Save to GeoPackage…" ), this );
    connect( actionSaveToGpkg, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::saveToGeopackage );
    mMenu->addAction( actionSaveToGpkg );

    QAction *actionSaveToDatabase = new QAction( tr( "Save to Database Table…" ), this );
    connect( actionSaveToDatabase, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::saveToDatabase );
    mMenu->addAction( actionSaveToDatabase );

    if ( mParameter->algorithm() && qgis::down_cast< const QgsProcessingParameterFeatureSink * >( mParameter )->supportsAppend() )
    {
      mMenu->addSeparator();
      QAction *actionAppendToLayer = new QAction( tr( "Append to Layer…" ), this );
      connect( actionAppendToLayer, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::appendToLayer );
      mMenu->addAction( actionAppendToLayer );
      if ( mUseRemapping )
      {
        QAction *editMappingAction = new QAction( tr( "Edit Field Mapping…" ), this );
        connect( editMappingAction, &QAction::triggered, this, [ = ]
        {
          setAppendDestination( value().value< QgsProcessingOutputLayerDefinition >().sink.staticValue().toString(), mRemapDefinition.destinationFields() );
        } );
        mMenu->addAction( editMappingAction );
      }
    }
  }

  if ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() )
  {
    mMenu->addSeparator();
    QAction *actionSetEncoding = new QAction( tr( "Change File Encoding (%1)…" ).arg( mEncoding ), this );
    connect( actionSetEncoding, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::selectEncoding );
    mMenu->addAction( actionSetEncoding );
  }
}

void QgsProcessingLayerOutputDestinationWidget::skipOutput()
{
  leText->setPlaceholderText( tr( "[Skip output]" ) );
  leText->clear();
  mUseTemporary = false;
  mUseRemapping = false;

  emit skipOutputChanged( true );
  emit destinationChanged();
}

void QgsProcessingLayerOutputDestinationWidget::saveToTemporary()
{
  const bool prevSkip = outputIsSkipped();

  if ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() && mParameter->supportsNonFileBasedOutput() )
  {
    leText->setPlaceholderText( tr( "[Create temporary layer]" ) );
  }
  else if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() )
  {
    leText->setPlaceholderText( tr( "[Save to temporary folder]" ) );
  }
  else
  {
    leText->setPlaceholderText( tr( "[Save to temporary file]" ) );
  }
  leText->clear();

  if ( mUseTemporary )
    return;

  mUseTemporary = true;
  mUseRemapping = false;
  if ( prevSkip )
    emit skipOutputChanged( false );
  emit destinationChanged();
}

void QgsProcessingLayerOutputDestinationWidget::selectDirectory()
{
  QString lastDir = leText->text();
  QgsSettings settings;
  if ( lastDir.isEmpty() )
    lastDir = settings.value( QStringLiteral( "/Processing/LastOutputPath" ), QDir::homePath() ).toString();

  const QString dirName = QFileDialog::getExistingDirectory( this, tr( "Select Directory" ), lastDir, QFileDialog::Options() );
  if ( !dirName.isEmpty() )
  {
    leText->setText( QDir::toNativeSeparators( dirName ) );
    settings.setValue( QStringLiteral( "/Processing/LastOutputPath" ), dirName );
    mUseTemporary = false;
    mUseRemapping = false;
    emit skipOutputChanged( false );
    emit destinationChanged();
  }
}

void QgsProcessingLayerOutputDestinationWidget::selectFile()
{
  const QString fileFilter = mParameter->createFileFilter();

  QgsSettings settings;

  QString lastExtPath;
  QString lastExt;
  if ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() || mParameter->type() == QgsProcessingParameterVectorDestination::typeName() )
  {
    lastExtPath = QStringLiteral( "/Processing/LastVectorOutputExt" );
    lastExt = settings.value( lastExtPath, QStringLiteral( ".%1" ).arg( mParameter->defaultFileExtension() ) ).toString() ;
  }
  else if ( mParameter->type() == QgsProcessingParameterRasterDestination::typeName() )
  {
    lastExtPath = QStringLiteral( "/Processing/LastRasterOutputExt" );
    lastExt = settings.value( lastExtPath, QStringLiteral( ".%1" ).arg( mParameter->defaultFileExtension() ) ).toString();
  }
  else if ( mParameter->type() == QgsProcessingParameterPointCloudDestination::typeName() )
  {
    lastExtPath = QStringLiteral( "/Processing/LastPointCloudOutputExt" );
    lastExt = settings.value( lastExtPath, QStringLiteral( ".%1" ).arg( mParameter->defaultFileExtension() ) ).toString();
  }

  // get default filter
  const QStringList filters = fileFilter.split( QStringLiteral( ";;" ) );
  QString lastFilter;
  for ( const QString &f : filters )
  {
    if ( f.contains( QStringLiteral( "*.%1" ).arg( lastExt ), Qt::CaseInsensitive ) )
    {
      lastFilter = f;
      break;
    }
  }

  QString path;
  if ( settings.contains( QStringLiteral( "/Processing/LastOutputPath" ) ) )
    path = settings.value( QStringLiteral( "/Processing/LastOutputPath" ) ).toString();
  else
    path = settings.value( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ) ).toString();

  const bool dontConfirmOverwrite = mParameter->metadata().value( QStringLiteral( "widget_wrapper" ) ).toMap().value( QStringLiteral( "dontconfirmoverwrite" ), false ).toBool();

  QString filename = QFileDialog::getSaveFileName( this, tr( "Save file" ), path, fileFilter, &lastFilter, dontConfirmOverwrite ? QFileDialog::Options( QFileDialog::DontConfirmOverwrite ) : QFileDialog::Options() );
  if ( !filename.isEmpty() )
  {
    mUseTemporary = false;
    mUseRemapping = false;
    filename = QgsFileUtils::addExtensionFromFilter( filename, lastFilter );

    leText->setText( filename );
    settings.setValue( QStringLiteral( "/Processing/LastOutputPath" ), QFileInfo( filename ).path() );
    if ( !lastExtPath.isEmpty() )
      settings.setValue( lastExtPath, QFileInfo( filename ).suffix().toLower() );

    emit skipOutputChanged( false );
    emit destinationChanged();
  }
}

void QgsProcessingLayerOutputDestinationWidget::saveToGeopackage()
{
  QgsSettings settings;
  QString lastPath = settings.value( QStringLiteral( "/Processing/LastOutputPath" ), QString() ).toString();
  if ( lastPath.isEmpty() )
    lastPath = settings.value( QStringLiteral( "/Processing/Configuration/OUTPUTS_FOLDER" ), QString() ).toString();

  QString filename =  QFileDialog::getSaveFileName( this, tr( "Save to GeoPackage" ), lastPath, tr( "GeoPackage files (*.gpkg);;All files (*.*)" ), nullptr, QFileDialog::DontConfirmOverwrite );

  if ( filename.isEmpty() )
    return;

  const QString layerName = QInputDialog::getText( this, tr( "Save to GeoPackage" ), tr( "Layer name" ), QLineEdit::Normal, mParameter->name().toLower() );
  if ( layerName.isEmpty() )
    return;

  mUseTemporary = false;
  mUseRemapping = false;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "gpkg" ) );

  settings.setValue( QStringLiteral( "/Processing/LastOutputPath" ), QFileInfo( filename ).path() );

  QgsDataSourceUri uri;
  uri.setTable( layerName );
  uri.setDatabase( filename );

  QString geomColumn;
  if ( const QgsProcessingParameterFeatureSink *sink = dynamic_cast< const QgsProcessingParameterFeatureSink * >( mParameter ) )
  {
    if ( sink->hasGeometry() )
      geomColumn = QStringLiteral( "geom" );
  }
  uri.setGeometryColumn( geomColumn );

  leText->setText( QStringLiteral( "ogr:%1" ).arg( uri.uri() ) );

  emit skipOutputChanged( false );
  emit destinationChanged();
}

void QgsProcessingLayerOutputDestinationWidget::saveToDatabase()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {

    QgsNewDatabaseTableNameWidget *widget = new QgsNewDatabaseTableNameWidget( mBrowserModel, QStringList() << QStringLiteral( "postgres" )
        << QStringLiteral( "mssql" )
        << QStringLiteral( "ogr" )
        << QStringLiteral( "hana" )
        << QStringLiteral( "spatialite" ), this );
    widget->setPanelTitle( tr( "Save “%1” to Database Table" ).arg( mParameter->description() ) );
    widget->setAcceptButtonVisible( true );

    panel->openPanel( widget );

    auto changed = [ = ]
    {
      mUseTemporary = false;
      mUseRemapping = false;

      QString geomColumn;
      if ( const QgsProcessingParameterFeatureSink *sink = dynamic_cast< const QgsProcessingParameterFeatureSink * >( mParameter ) )
      {
        if ( sink->hasGeometry() )
          geomColumn = QStringLiteral( "geom" );
      }

      if ( widget->dataProviderKey() == QLatin1String( "ogr" ) )
      {
        QgsDataSourceUri uri;
        uri.setTable( widget->table() );
        uri.setDatabase( widget->schema() );
        uri.setGeometryColumn( geomColumn );
        leText->setText( QStringLiteral( "ogr:%1" ).arg( uri.uri() ) );
      }
      else
      {
        QgsDataSourceUri uri( widget->uri() );
        uri.setGeometryColumn( geomColumn );
        leText->setText( QgsProcessingUtils::encodeProviderKeyAndUri( widget->dataProviderKey(), uri.uri() ) );
      }

      emit skipOutputChanged( false );
      emit destinationChanged();
    };

    connect( widget, &QgsNewDatabaseTableNameWidget::tableNameChanged, this, [ = ] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::schemaNameChanged, this, [ = ] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::validationChanged, this, [ = ] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::providerKeyChanged, this, [ = ] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::accepted, this, [ = ]
    {
      changed();
      widget->acceptPanel();
    } );
  }
}

void QgsProcessingLayerOutputDestinationWidget::appendToLayer()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    QgsDataSourceSelectWidget *widget = new QgsDataSourceSelectWidget( mBrowserModel, true, QgsMapLayerType::VectorLayer );
    widget->setPanelTitle( tr( "Append \"%1\" to Layer" ).arg( mParameter->description() ) );

    panel->openPanel( widget );

    connect( widget, &QgsDataSourceSelectWidget::itemTriggered, this, [ = ]( const QgsMimeDataUtils::Uri & )
    {
      widget->acceptPanel();
    } );
    connect( widget, &QgsPanelWidget::panelAccepted, this, [ = ]()
    {
      if ( widget->uri().uri.isEmpty() )
        setValue( QVariant() );
      else
      {
        // get fields for destination
        std::unique_ptr< QgsVectorLayer > dest = std::make_unique< QgsVectorLayer >( widget->uri().uri, QString(), widget->uri().providerKey );
        if ( widget->uri().providerKey == QLatin1String( "ogr" ) )
          setAppendDestination( widget->uri().uri, dest->fields() );
        else
          setAppendDestination( QgsProcessingUtils::encodeProviderKeyAndUri( widget->uri().providerKey, widget->uri().uri ), dest->fields() );
      }
    } );
  }
}


void QgsProcessingLayerOutputDestinationWidget::setAppendDestination( const QString &uri, const QgsFields &destFields )
{
  const QgsProcessingAlgorithm *alg = mParameter->algorithm();
  QVariantMap props;
  if ( mParametersGenerator )
    props = mParametersGenerator->createProcessingParameters();
  props.insert( mParameter->name(), uri );

  const QgsProcessingAlgorithm::VectorProperties outputProps = alg->sinkProperties( mParameter->name(), props, *mContext, QMap<QString, QgsProcessingAlgorithm::VectorProperties >() );
  if ( outputProps.availability == QgsProcessingAlgorithm::Available )
  {
    if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
    {
      // get mapping from fields output by algorithm to destination fields
      QgsFieldMappingWidget *widget = new QgsFieldMappingWidget( nullptr, outputProps.fields, destFields );
      widget->setPanelTitle( tr( "Append \"%1\" to Layer" ).arg( mParameter->description() ) );
      if ( !mRemapDefinition.fieldMap().isEmpty() )
        widget->setFieldPropertyMap( mRemapDefinition.fieldMap() );

      panel->openPanel( widget );

      connect( widget, &QgsPanelWidget::panelAccepted, this, [ = ]()
      {
        QgsProcessingOutputLayerDefinition def( uri );
        QgsRemappingSinkDefinition remap;
        remap.setSourceCrs( outputProps.crs );
        remap.setFieldMap( widget->fieldPropertyMap() );
        remap.setDestinationFields( destFields );
        def.setRemappingDefinition( remap );
        setValue( def );
      } );
    }
  }
}

void QgsProcessingLayerOutputDestinationWidget::selectEncoding()
{
  QgsEncodingSelectionDialog dialog( this, tr( "File encoding" ), mEncoding );
  if ( dialog.exec() )
  {
    mEncoding = dialog.encoding();
    QgsSettings settings;
    settings.setValue( QStringLiteral( "/Processing/encoding" ), mEncoding );
    emit destinationChanged();
  }
}

void QgsProcessingLayerOutputDestinationWidget::textChanged( const QString &text )
{
  mUseTemporary = text.isEmpty();
  mUseRemapping = false;
  emit destinationChanged();
}


QString QgsProcessingLayerOutputDestinationWidget::mimeDataToPath( const QMimeData *data )
{
  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &u : uriList )
  {
    if ( ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName()
           || mParameter->type() == QgsProcessingParameterVectorDestination::typeName()
           || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
         && u.layerType == QLatin1String( "vector" ) && u.providerKey == QLatin1String( "ogr" ) )
    {
      return u.uri;
    }
    else if ( ( mParameter->type() == QgsProcessingParameterRasterDestination::typeName()
                || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
              && u.layerType == QLatin1String( "raster" ) && u.providerKey == QLatin1String( "gdal" ) )
    {
      return u.uri;
    }
    else if ( ( mParameter->type() == QgsProcessingParameterPointCloudDestination::typeName()
                || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
              && u.layerType == QLatin1String( "pointcloud" ) && ( u.providerKey == QLatin1String( "ept" ) || u.providerKey == QLatin1String( "pdal" ) ) )
    {
      return u.uri;
    }
#if 0
    else if ( ( mParameter->type() == QgsProcessingParameterMeshDestination::typeName()
                || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
              && u.layerType == QLatin1String( "mesh" ) && u.providerKey == QLatin1String( "mdal" ) )
      return u.uri;

#endif
    else if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName()
              && u.layerType == QLatin1String( "directory" ) )
    {
      return u.uri;
    }
  }
  if ( !uriList.isEmpty() )
    return QString();

  // files dragged from file explorer, outside of QGIS
  QStringList rawPaths;
  if ( data->hasUrls() )
  {
    const QList< QUrl > urls = data->urls();
    rawPaths.reserve( urls.count() );
    for ( const QUrl &url : urls )
    {
      const QString local =  url.toLocalFile();
      if ( !rawPaths.contains( local ) )
        rawPaths.append( local );
    }
  }
  if ( !data->text().isEmpty() && !rawPaths.contains( data->text() ) )
    rawPaths.append( data->text() );

  for ( const QString &path : std::as_const( rawPaths ) )
  {
    QFileInfo file( path );
    if ( file.isFile() && ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName()
                            || mParameter->type() == QgsProcessingParameterVectorDestination::typeName()
                            || mParameter->type() == QgsProcessingParameterRasterDestination::typeName()
                            || mParameter->type() == QgsProcessingParameterVectorDestination::typeName()
                            || mParameter->type() == QgsProcessingParameterFileDestination::typeName()
                            || mParameter->type() == QgsProcessingParameterPointCloudDestination::typeName() ) )
    {
      // TODO - we should check to see if it's a valid extension for the parameter, but that's non-trivial
      return path;
    }
    else if ( file.isDir() && ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() ) )
      return path;
  }

  return QString();
}

void QgsProcessingLayerOutputDestinationWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  const QString path = mimeDataToPath( event->mimeData() );
  if ( !path.isEmpty() )
  {
    // dragged an acceptable path, phew
    event->setDropAction( Qt::CopyAction );
    event->accept();
    leText->setHighlighted( true );
  }
}

void QgsProcessingLayerOutputDestinationWidget::dragLeaveEvent( QDragLeaveEvent *event )
{
  QWidget::dragLeaveEvent( event );
  if ( leText->isHighlighted() )
  {
    event->accept();
    leText->setHighlighted( false );
  }
}

void QgsProcessingLayerOutputDestinationWidget::dropEvent( QDropEvent *event )
{
  if ( !( event->possibleActions() & Qt::CopyAction ) )
    return;

  const QString path = mimeDataToPath( event->mimeData() );
  if ( !path.isEmpty() )
  {
    // dropped an acceptable path, phew
    setFocus( Qt::MouseFocusReason );
    event->setDropAction( Qt::CopyAction );
    event->accept();
    setValue( path );
  }
  leText->setHighlighted( false );
}

///@endcond
