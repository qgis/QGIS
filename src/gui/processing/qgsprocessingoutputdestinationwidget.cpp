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

#include "qgsapplication.h"
#include "qgsdatasourceselectdialog.h"
#include "qgsdatasourceuri.h"
#include "qgsencodingfiledialog.h"
#include "qgsfieldmappingwidget.h"
#include "qgsfileutils.h"
#include "qgsnewdatabasetablenamewidget.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingparameters.h"
#include "qgssettings.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLocale>
#include <QMenu>
#include <QTextCodec>
#include <QUrl>

#include "moc_qgsprocessingoutputdestinationwidget.cpp"

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
  mEncoding = QgsProcessingUtils::resolveDefaultEncoding( settings.value( u"/Processing/encoding"_s, u"System"_s ).toString() );
  settings.setValue( u"/Processing/encoding"_s, mEncoding );

  if ( !mParameter->defaultValueForGui().isValid() )
  {
    // no default value -- we default to either skipping the output or a temporary output, depending on the createByDefault value
    if ( mParameter->flags() & Qgis::ProcessingParameterFlag::Optional && !mParameter->createByDefault() )
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
  if ( !value.isValid() || ( value.userType() == QMetaType::Type::QString && value.toString().isEmpty() ) )
  {
    if ( mParameter->flags() & Qgis::ProcessingParameterFlag::Optional )
      skipOutput();
    else
      saveToTemporary();
  }
  else
  {
    if ( value.toString() == "memory:"_L1 || value.toString() == QgsProcessing::TEMPORARY_OUTPUT )
    {
      saveToTemporary();
    }
    else if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
    {
      const QgsProcessingOutputLayerDefinition def = value.value<QgsProcessingOutputLayerDefinition>();
      if ( def.sink.staticValue().toString() == "memory:"_L1 || def.sink.staticValue().toString() == QgsProcessing::TEMPORARY_OUTPUT || def.sink.staticValue().toString().isEmpty() )
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
      mEncoding = def.createOptions.value( u"fileEncoding"_s ).toString();
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
        if ( prev.userType() != qMetaTypeId<QgsProcessingOutputLayerDefinition>() || !( prev.value<QgsProcessingOutputLayerDefinition>() == QgsProcessingLayerOutputDestinationWidget::value().value<QgsProcessingOutputLayerDefinition>() ) )
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

  if ( key.isEmpty() && mParameter->flags() & Qgis::ProcessingParameterFlag::Optional )
    return QVariant();

  QString provider;
  QString uri;
  if ( !key.isEmpty() && key != QgsProcessing::TEMPORARY_OUTPUT
       && !key.startsWith( "memory:"_L1 )
       && !key.startsWith( "ogr:"_L1 )
       && !key.startsWith( "postgres:"_L1 )
       && !key.startsWith( "postgis:"_L1 )
       && !QgsProcessingUtils::decodeProviderKeyAndUri( key, provider, uri ) )
  {
    // output should be a file path
    QString folder = QFileInfo( key ).path();
    if ( folder == '.' )
    {
      // output name does not include a folder - use default
      QString defaultFolder = settings.value( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, u"%1/processing"_s.arg( QDir::homePath() ) ).toString();
      QDir destDir( defaultFolder );
      if ( !destDir.exists() && !QDir().mkpath( defaultFolder ) )
      {
        QgsDebugError( u"Can't create output folder '%1'"_s.arg( defaultFolder ) );
      }
      key = destDir.filePath( key );
    }
  }

  if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName() )
    return key;
  else if ( mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
    return key;

  QgsProcessingOutputLayerDefinition value( key );
  value.createOptions.insert( u"fileEncoding"_s, mEncoding );
  if ( mUseRemapping )
    value.setRemappingDefinition( mRemapDefinition );
  if ( !mFormat.isEmpty() )
    value.setFormat( mFormat );
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

  connect( this, &QgsProcessingLayerOutputDestinationWidget::skipOutputChanged, this, [this]( bool skipped ) {
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
    if ( mParameter->flags() & Qgis::ProcessingParameterFlag::Optional )
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

    if ( mParameter->algorithm() && qgis::down_cast<const QgsProcessingParameterFeatureSink *>( mParameter )->supportsAppend() )
    {
      mMenu->addSeparator();
      QAction *actionAppendToLayer = new QAction( tr( "Append to Layer…" ), this );
      connect( actionAppendToLayer, &QAction::triggered, this, &QgsProcessingLayerOutputDestinationWidget::appendToLayer );
      mMenu->addAction( actionAppendToLayer );
      if ( mUseRemapping )
      {
        QAction *editMappingAction = new QAction( tr( "Edit Field Mapping…" ), this );
        connect( editMappingAction, &QAction::triggered, this, [this] {
          setAppendDestination( value().value<QgsProcessingOutputLayerDefinition>().sink.staticValue().toString(), mRemapDefinition.destinationFields() );
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
    lastDir = settings.value( u"/Processing/LastOutputPath"_s, QDir::homePath() ).toString();

  const QString dirName = QFileDialog::getExistingDirectory( this, tr( "Select Directory" ), lastDir, QFileDialog::Options() );
  if ( !dirName.isEmpty() )
  {
    leText->setText( QDir::toNativeSeparators( dirName ) );
    settings.setValue( u"/Processing/LastOutputPath"_s, dirName );
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
  QString lastFormatPath;
  QString lastFormat;
  if ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() || mParameter->type() == QgsProcessingParameterVectorDestination::typeName() )
  {
    lastExtPath = u"/Processing/LastVectorOutputExt"_s;
    lastExt = settings.value( lastExtPath, u".%1"_s.arg( mParameter->defaultFileExtension() ) ).toString();
  }
  else if ( mParameter->type() == QgsProcessingParameterRasterDestination::typeName() )
  {
    const QgsProcessingParameterRasterDestination *dest = dynamic_cast<const QgsProcessingParameterRasterDestination *>( mParameter );
    Q_ASSERT( dest );
    lastFormatPath = u"/Processing/LastRasterOutputFormat"_s;
    lastFormat = settings.value( lastFormatPath, dest->defaultFileFormat() ).toString();
  }
  else if ( mParameter->type() == QgsProcessingParameterPointCloudDestination::typeName() )
  {
    lastExtPath = u"/Processing/LastPointCloudOutputExt"_s;
    lastExt = settings.value( lastExtPath, u".%1"_s.arg( mParameter->defaultFileExtension() ) ).toString();
  }
  else if ( mParameter->type() == QgsProcessingParameterVectorTileDestination::typeName() )
  {
    lastExtPath = u"/Processing/LastVectorTileOutputExt"_s;
    lastExt = settings.value( lastExtPath, u".%1"_s.arg( mParameter->defaultFileExtension() ) ).toString();
  }

  // get default filter
  const QStringList filters = fileFilter.split( u";;"_s );
  QString lastFilter;
  for ( const QString &f : filters )
  {
    if ( !lastFormat.isEmpty() && f.contains( lastFormat, Qt::CaseInsensitive ) )
    {
      lastFilter = f;
      break;
    }
    else if ( !lastExt.isEmpty() && f.contains( u"*.%1"_s.arg( lastExt ), Qt::CaseInsensitive ) )
    {
      lastFilter = f;
      break;
    }
  }

  QString path;
  if ( settings.contains( u"/Processing/LastOutputPath"_s ) )
    path = settings.value( u"/Processing/LastOutputPath"_s ).toString();
  else
    path = settings.value( u"/Processing/Configuration/OUTPUTS_FOLDER"_s ).toString();

  const bool dontConfirmOverwrite = mParameter->metadata().value( u"widget_wrapper"_s ).toMap().value( u"dontconfirmoverwrite"_s, false ).toBool();

  QString filename = QFileDialog::getSaveFileName( this, tr( "Save file" ), path, fileFilter, &lastFilter, dontConfirmOverwrite ? QFileDialog::Options( QFileDialog::DontConfirmOverwrite ) : QFileDialog::Options() );
  if ( !filename.isEmpty() )
  {
    mUseTemporary = false;
    mUseRemapping = false;
    if ( mParameter->type() == QgsProcessingParameterRasterDestination::typeName() )
    {
      int spacePos = static_cast<int>( lastFilter.indexOf( ' ' ) );
      if ( spacePos > 0 )
      {
        mFormat = lastFilter.left( spacePos );
      }
    }
    filename = QgsFileUtils::addExtensionFromFilter( filename, lastFilter );

    leText->setText( filename );
    settings.setValue( u"/Processing/LastOutputPath"_s, QFileInfo( filename ).path() );
    if ( !lastFormatPath.isEmpty() && !mFormat.isEmpty() )
      settings.setValue( lastFormatPath, mFormat );
    else if ( !lastExtPath.isEmpty() )
      settings.setValue( lastExtPath, QFileInfo( filename ).suffix().toLower() );

    emit skipOutputChanged( false );
    emit destinationChanged();
  }
  // return dialog focus on Mac
  activateWindow();
  raise();
}

void QgsProcessingLayerOutputDestinationWidget::saveToGeopackage()
{
  QgsSettings settings;
  QString lastPath = settings.value( u"/Processing/LastOutputPath"_s, QString() ).toString();
  if ( lastPath.isEmpty() )
    lastPath = settings.value( u"/Processing/Configuration/OUTPUTS_FOLDER"_s, QString() ).toString();

  QString filename = QFileDialog::getSaveFileName( this, tr( "Save to GeoPackage" ), lastPath, tr( "GeoPackage files (*.gpkg);;All files (*.*)" ), nullptr, QFileDialog::DontConfirmOverwrite );
  // return dialog focus on Mac
  activateWindow();
  raise();

  if ( filename.isEmpty() )
    return;

  const QString layerName = QInputDialog::getText( this, tr( "Save to GeoPackage" ), tr( "Layer name" ), QLineEdit::Normal, mParameter->name().toLower() );
  if ( layerName.isEmpty() )
    return;

  mUseTemporary = false;
  mUseRemapping = false;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << u"gpkg"_s );

  settings.setValue( u"/Processing/LastOutputPath"_s, QFileInfo( filename ).path() );

  QgsDataSourceUri uri;
  uri.setTable( layerName );
  uri.setDatabase( filename );

  QString geomColumn;
  if ( const QgsProcessingParameterFeatureSink *sink = dynamic_cast<const QgsProcessingParameterFeatureSink *>( mParameter ) )
  {
    if ( sink->hasGeometry() )
      geomColumn = u"geom"_s;
  }
  uri.setGeometryColumn( geomColumn );

  leText->setText( u"ogr:%1"_s.arg( uri.uri() ) );

  emit skipOutputChanged( false );
  emit destinationChanged();
}

void QgsProcessingLayerOutputDestinationWidget::saveToDatabase()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    QgsNewDatabaseTableNameWidget *widget = new QgsNewDatabaseTableNameWidget( mBrowserModel, QStringList() << u"postgres"_s << u"mssql"_s << u"ogr"_s << u"hana"_s << u"spatialite"_s << u"oracle"_s, this );
    widget->setPanelTitle( tr( "Save “%1” to Database Table" ).arg( mParameter->description() ) );
    widget->setAcceptButtonVisible( true );

    panel->openPanel( widget );

    auto changed = [this, widget] {
      mUseTemporary = false;
      mUseRemapping = false;

      QString geomColumn;
      if ( const QgsProcessingParameterFeatureSink *sink = dynamic_cast<const QgsProcessingParameterFeatureSink *>( mParameter ) )
      {
        if ( sink->hasGeometry() )
          geomColumn = widget->dataProviderKey() == "oracle"_L1 ? u"GEOM"_s : u"geom"_s;
      }

      if ( widget->dataProviderKey() == "ogr"_L1 )
      {
        QgsDataSourceUri uri;
        uri.setTable( widget->table() );
        uri.setDatabase( widget->schema() );
        uri.setGeometryColumn( geomColumn );
        leText->setText( u"ogr:%1"_s.arg( uri.uri() ) );
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

    connect( widget, &QgsNewDatabaseTableNameWidget::tableNameChanged, this, [changed] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::schemaNameChanged, this, [changed] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::validationChanged, this, [changed] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::providerKeyChanged, this, [changed] { changed(); } );
    connect( widget, &QgsNewDatabaseTableNameWidget::accepted, this, [changed, widget] {
      changed();
      widget->acceptPanel();
    } );
  }
}

void QgsProcessingLayerOutputDestinationWidget::appendToLayer()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    QgsDataSourceSelectWidget *widget = new QgsDataSourceSelectWidget( mBrowserModel, true, Qgis::LayerType::Vector );
    widget->setPanelTitle( tr( "Append \"%1\" to Layer" ).arg( mParameter->description() ) );

    panel->openPanel( widget );

    connect( widget, &QgsDataSourceSelectWidget::itemTriggered, this, [widget]( const QgsMimeDataUtils::Uri & ) {
      widget->acceptPanel();
    } );
    connect( widget, &QgsPanelWidget::panelAccepted, this, [this, widget]() {
      if ( widget->uri().uri.isEmpty() )
        setValue( QVariant() );
      else
      {
        // get fields for destination
        auto dest = std::make_unique<QgsVectorLayer>( widget->uri().uri, QString(), widget->uri().providerKey );
        if ( widget->uri().providerKey == "ogr"_L1 )
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

  const QgsProcessingAlgorithm::VectorProperties outputProps = alg->sinkProperties( mParameter->name(), props, *mContext, QMap<QString, QgsProcessingAlgorithm::VectorProperties>() );
  if ( outputProps.availability == Qgis::ProcessingPropertyAvailability::Available )
  {
    if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
    {
      // get mapping from fields output by algorithm to destination fields
      QgsFieldMappingWidget *widget = new QgsFieldMappingWidget( nullptr, outputProps.fields, destFields );
      widget->setPanelTitle( tr( "Append \"%1\" to Layer" ).arg( mParameter->description() ) );
      if ( !mRemapDefinition.fieldMap().isEmpty() )
        widget->setFieldPropertyMap( mRemapDefinition.fieldMap() );

      panel->openPanel( widget );

      connect( widget, &QgsPanelWidget::panelAccepted, this, [this, outputProps, widget, destFields, uri]() {
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
    mEncoding = QgsProcessingUtils::resolveDefaultEncoding( dialog.encoding() );

    QgsSettings settings;
    settings.setValue( u"/Processing/encoding"_s, mEncoding );

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
         && u.layerType == "vector"_L1 && u.providerKey == "ogr"_L1 )
    {
      return u.uri;
    }
    else if ( ( mParameter->type() == QgsProcessingParameterRasterDestination::typeName()
                || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
              && u.layerType == "raster"_L1 && u.providerKey == "gdal"_L1 )
    {
      return u.uri;
    }
    else if ( ( mParameter->type() == QgsProcessingParameterPointCloudDestination::typeName()
                || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
              && u.layerType == "pointcloud"_L1 && ( u.providerKey == "ept"_L1 || u.providerKey == "pdal"_L1 ) )
    {
      return u.uri;
    }
#if 0
    else if ( ( mParameter->type() == QgsProcessingParameterMeshDestination::typeName()
                || mParameter->type() == QgsProcessingParameterFileDestination::typeName() )
              && u.layerType == "mesh"_L1 && u.providerKey == "mdal"_L1 )
      return u.uri;

#endif
    else if ( mParameter->type() == QgsProcessingParameterFolderDestination::typeName()
              && u.layerType == "directory"_L1 )
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
    if ( file.isFile() && ( mParameter->type() == QgsProcessingParameterFeatureSink::typeName() || mParameter->type() == QgsProcessingParameterVectorDestination::typeName() || mParameter->type() == QgsProcessingParameterRasterDestination::typeName() || mParameter->type() == QgsProcessingParameterVectorDestination::typeName() || mParameter->type() == QgsProcessingParameterFileDestination::typeName() || mParameter->type() == QgsProcessingParameterPointCloudDestination::typeName() ) )
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
