/***************************************************************************
                         qgspointcloudsourceselect.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
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

#include <QMessageBox>

#include <algorithm>
#include <string>
#include <vector>
#include <pdal/StageFactory.hpp>
#include <nlohmann/json.hpp>

#include "qgspointcloudsourceselect.h"
#include "moc_qgspointcloudsourceselect.cpp"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgshelp.h"
#include "qgsspinbox.h"

///@cond PRIVATE

const int QgsPointCloudSourceSelect::SEPARATOR_OTHER = -2;

QgsPointCloudSourceSelect::QgsPointCloudSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mOpenOptionsGroupBox->setCollapsed( false );

  connect( mRadioSrcFile, &QRadioButton::toggled, this, &QgsPointCloudSourceSelect::radioSrcFile_toggled );
  connect( mRadioSrcProtocol, &QRadioButton::toggled, this, &QgsPointCloudSourceSelect::radioSrcProtocol_toggled );
  connect( cmbProtocolTypes, &QComboBox::currentTextChanged, this, &QgsPointCloudSourceSelect::cmbProtocolTypes_currentIndexChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudSourceSelect::showHelp );

  radioSrcFile_toggled( true );
  setProtocolWidgetsVisibility();

  mFileWidget->setDialogTitle( tr( "Open Point Cloud Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->filePointCloudFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [=]( const QString &path ) {
    mPath = path;
    emit enableButtons( !mPath.isEmpty() );
    fillOpenOptions();
  } );

  connect( protocolURI, &QLineEdit::textChanged, this, [=]( const QString &path ) {
    mPath = path;
    emit enableButtons( !mPath.isEmpty() );
    fillOpenOptions();
  } );


  const QStringList protocolTypes = QStringLiteral( "HTTP/HTTPS/FTP,vsicurl" ).split( ';' );
  for ( int i = 0; i < protocolTypes.count(); i++ )
  {
    const QString protocolType = protocolTypes.at( i );
    if ( ( !protocolType.isEmpty() ) && ( !protocolType.isNull() ) )
      cmbProtocolTypes->addItem( protocolType.split( ',' ).at( 0 ) );
  }
}

void QgsPointCloudSourceSelect::addButtonClicked()
{
  QStringList openOptions;
  for ( QWidget *control : mOpenOptionsWidgets )
  {
    QString value;
    if ( control->objectName() == QStringLiteral( "separator" ) )
    {
      const QComboBox *comboBox = control->findChild<QComboBox *>();
      if ( comboBox )
      {
        const int currentData = comboBox->itemData( comboBox->currentIndex() ).toInt();
        int valueAsChar = 32; // space is the default separator
        if ( currentData == QgsPointCloudSourceSelect::SEPARATOR_OTHER )
        {
          const QLineEdit *otherWidget = control->findChild<QLineEdit *>();
          if ( otherWidget )
          {
            valueAsChar = otherWidget->text().at( 0 ).unicode();
          }
        }
        else
        {
          valueAsChar = currentData;
        }

        if ( valueAsChar != 32 )
        {
          value = QString::number( valueAsChar );
        }
      }
    }
    else if ( QLineEdit *lineEdit = qobject_cast<QLineEdit *>( control ) )
    {
      value = lineEdit->text();
    }
    else if ( QgsSpinBox *intSpin = qobject_cast<QgsSpinBox *>( control ) )
    {
      if ( intSpin->value() != intSpin->clearValue() )
      {
        value = QString::number( intSpin->value() );
      }
    }

    if ( !value.isEmpty() )
    {
      openOptions << QStringLiteral( "%1=%2" ).arg( control->objectName() ).arg( value );
    }
  }

  if ( mDataSourceType == QLatin1String( "file" ) )
  {
    if ( mPath.isEmpty() )
    {
      QMessageBox::information( this, tr( "Add Point Cloud Layers" ), tr( "No layers selected." ) );
      return;
    }

    for ( const QString &path : QgsFileWidget::splitFilePaths( mPath ) )
    {
      QVariantMap parts;
      if ( !openOptions.isEmpty() )
      {
        parts.insert( QStringLiteral( "openOptions" ), openOptions );
      }
      parts.insert( QStringLiteral( "path" ), path );

      // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
      const QList<QgsProviderRegistry::ProviderCandidateDetails> preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( path );
      // if no preferred providers we can still give pdal a try
      const QString providerKey = preferredProviders.empty() ? QStringLiteral( "pdal" ) : preferredProviders.first().metadata()->key();
      const QString uri = QgsProviderRegistry::instance()->encodeUri( providerKey, parts );
      Q_NOWARN_DEPRECATED_PUSH
      emit addPointCloudLayer( uri, QFileInfo( path ).baseName(), providerKey );
      Q_NOWARN_DEPRECATED_POP
      emit addLayer( Qgis::LayerType::PointCloud, uri, QFileInfo( path ).baseName(), providerKey );
    }
  }
  else if ( mDataSourceType == QLatin1String( "remote" ) )
  {
    if ( mPath.isEmpty() )
    {
      QMessageBox::information( this, tr( "Add Point Cloud Layers" ), tr( "No layers selected." ) );
      return;
    }

    QUrl url = QUrl::fromUserInput( mPath );
    QString fileName = url.fileName();

    if ( fileName.compare( QLatin1String( "ept.json" ), Qt::CaseInsensitive ) != 0 && !fileName.endsWith( QLatin1String( ".copc.laz" ), Qt::CaseInsensitive ) )
    {
      QMessageBox::information( this, tr( "Add Point Cloud Layers" ), tr( "Invalid point cloud URL \"%1\", please make sure your URL ends with /ept.json or .copc.laz" ).arg( mPath ) );
      return;
    }

    // auto determine preferred provider for each path
    const QList<QgsProviderRegistry::ProviderCandidateDetails> preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( mPath );
    // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
    if ( !preferredProviders.empty() )
    {
      QVariantMap parts;
      if ( !openOptions.isEmpty() )
      {
        parts.insert( QStringLiteral( "openOptions" ), openOptions );
      }
      parts.insert( QStringLiteral( "path" ), mPath );

      QString baseName = QStringLiteral( "remote ept layer" );
      if ( mPath.endsWith( QLatin1String( "/ept.json" ), Qt::CaseInsensitive ) )
      {
        QStringList separatedPath = mPath.split( '/' );
        if ( separatedPath.size() >= 2 )
          baseName = separatedPath[separatedPath.size() - 2];
      }
      if ( mPath.endsWith( QLatin1String( ".copc.laz" ), Qt::CaseInsensitive ) )
      {
        baseName = QFileInfo( mPath ).baseName();
      }

      const QString providerKey = preferredProviders.at( 0 ).metadata()->key();
      const QString uri = QgsProviderRegistry::instance()->encodeUri( providerKey, parts );

      Q_NOWARN_DEPRECATED_PUSH
      emit addPointCloudLayer( uri, baseName, providerKey );
      Q_NOWARN_DEPRECATED_POP
      emit addLayer( Qgis::LayerType::PointCloud, uri, baseName, providerKey );
    }
  }
}

void QgsPointCloudSourceSelect::radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->show();
    protocolGroupBox->hide();
    clearOpenOptions();

    mFileWidget->setDialogTitle( tr( "Open Point Cloud Dataset" ) );
    mFileWidget->setFilter( QgsProviderRegistry::instance()->filePointCloudFilters() );
    mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );

    mDataSourceType = QStringLiteral( "file" );

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
    fillOpenOptions();
  }
}

void QgsPointCloudSourceSelect::radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    protocolGroupBox->show();
    clearOpenOptions();

    mDataSourceType = QStringLiteral( "remote" );

    setProtocolWidgetsVisibility();

    emit enableButtons( !protocolURI->text().isEmpty() );
    fillOpenOptions();
  }
}

void QgsPointCloudSourceSelect::cmbProtocolTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  setProtocolWidgetsVisibility();
  clearOpenOptions();
}

void QgsPointCloudSourceSelect::setProtocolWidgetsVisibility()
{
  labelProtocolURI->show();
  protocolURI->show();
  mAuthGroupBox->show();
  labelBucket->hide();
  mBucket->hide();
  labelKey->hide();
  mKey->hide();
  mAuthWarning->hide();
}

void QgsPointCloudSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-layer-from-a-file" ) );
}

void QgsPointCloudSourceSelect::clearOpenOptions()
{
  mOpenOptionsWidgets.clear();
  mOpenOptionsGroupBox->setVisible( false );
  mOpenOptionsLabel->clear();
  while ( mOpenOptionsLayout->count() )
  {
    QLayoutItem *item = mOpenOptionsLayout->takeAt( 0 );
    delete item->widget();
    delete item;
  }
}

void QgsPointCloudSourceSelect::fillOpenOptions()
{
  clearOpenOptions();
  if ( mDataSourceType.isEmpty() )
  {
    return;
  }

  const QUrl url = QUrl::fromUserInput( mPath );
  const std::string driverName = pdal::StageFactory::inferReaderDriver( url.fileName().toStdString() );
  // Only handle the readers.text at the moment.
  // This could be expanded for other drivers.
  if ( driverName != std::string( "readers.text" ) )
  {
    return;
  }

  pdal::StageFactory stageFactory( false );
  pdal::Stage *stage = stageFactory.createStage( driverName );
  if ( !stage )
  {
    QgsDebugMsgLevel( QStringLiteral( "Unable to create stage for driver %1" ).arg( QString::fromStdString( driverName ) ), 2 );
    return;
  }

  pdal::ProgramArgs args;
  stage->addAllArgs( args );
  std::stringstream args_json_stream;
  args.dump3( args_json_stream );

  json args_json;
  args_json_stream >> args_json;

  const QList<QString> ignoredArgs = {
    QStringLiteral( "count" ),
    QStringLiteral( "default_srs" ),
    QStringLiteral( "filename" ),
    QStringLiteral( "log" ),
    QStringLiteral( "option_file" ),
    QStringLiteral( "override_srs" ),
    QStringLiteral( "user_data" ),
  };

  for ( auto &elem : args_json.items() )
  {
    auto arg = elem.value();
    const std::string argName = arg["name"];
    if ( std::find( ignoredArgs.begin(), ignoredArgs.end(), QString::fromStdString( argName ) ) != ignoredArgs.end() )
    {
      continue;
    }

    QLabel *optionLabel = new QLabel( argName.c_str() );
    QWidget *optionControl;

    std::string argDefault = arg.value( "default", "" );
    // skip default value may be empty. Use "0" instead to create a spinbox widget
    if ( argDefault.empty() && argName == std::string( "skip" ) )
    {
      argDefault = "0";
    }
    bool parsedInt;
    const int defaultIntValue = QString::fromStdString( argDefault ).toInt( &parsedInt );

    if ( argName == "separator" )
    {
      QWidget *separatorContainer = createSeparatorOptionWidget();
      if ( parsedInt )
      {
        QComboBox *comboBox = separatorContainer->findChild<QComboBox *>();
        if ( comboBox )
        {
          const int index = comboBox->findData( QVariant( defaultIntValue ) );
          if ( index != -1 )
          {
            comboBox->setCurrentIndex( index );
          }
        }
      }
      optionControl = separatorContainer;
    }
    else if ( parsedInt )
    {
      QgsSpinBox *spinBox = new QgsSpinBox();
      spinBox->setMaximum( std::numeric_limits< int>::max() - 1 );
      spinBox->setMinimum( 0 );
      spinBox->setClearValue( defaultIntValue );
      spinBox->clear();
      optionControl = spinBox;
    }
    else
    {
      QLineEdit *lineEdit = new QLineEdit();
      if ( !argDefault.empty() )
      {
        lineEdit->setText( QString::fromStdString( argDefault ) );
      }
      optionControl = lineEdit;
    }

    optionControl->setObjectName( argName.c_str() );
    mOpenOptionsWidgets.push_back( optionControl );

    const std::string argDescription = arg["description"];
    if ( !argDescription.empty() )
    {
      optionLabel->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( argDescription.c_str() ) );
      optionControl->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( argDescription.c_str() ) );
    }

    mOpenOptionsLayout->addRow( optionLabel, optionControl );
  }

  // Set label to point to driver help page
  const std::string docLink = pdal::PluginManager<pdal::Stage>::link( driverName );
  if ( !docLink.empty() )
  {
    mOpenOptionsLabel->setText( tr( "Consult <a href=\"%1\">%2 driver help page</a> for detailed explanations on options" ).arg( QString::fromStdString( docLink ) ).arg( QString::fromStdString( driverName ) ) );
    mOpenOptionsLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
    mOpenOptionsLabel->setOpenExternalLinks( true );
    mOpenOptionsLabel->setVisible( true );
  }
  else
  {
    mOpenOptionsLabel->setVisible( false );
  }

  mOpenOptionsGroupBox->setVisible( !mOpenOptionsWidgets.empty() );
}

QWidget *QgsPointCloudSourceSelect::createSeparatorOptionWidget() const
{
  // separator and associated ASCII code
  QComboBox *separatorComboBox = new QComboBox();
  const QList<std::pair<QString, int>> items = {
    std::pair<QString, int>( tr( "Colon" ), 58 ),
    std::pair<QString, int>( tr( "Comma" ), 44 ),
    std::pair<QString, int>( tr( "Semicolon" ), 59 ),
    std::pair<QString, int>( tr( "Space" ), 32 ),
    std::pair<QString, int>( tr( "Tab" ), 9 ),
    std::pair<QString, int>( tr( "Other" ), QgsPointCloudSourceSelect::SEPARATOR_OTHER ),
  };

  for ( const std::pair<QString, int> &item : items )
  {
    separatorComboBox->addItem( item.first, QVariant( item.second ) );
  }

  QLineEdit *otherWidget = new QLineEdit();
  otherWidget->setVisible( false );

  QHBoxLayout *horizontalLayout = new QHBoxLayout();
  horizontalLayout->addWidget( separatorComboBox );
  horizontalLayout->addWidget( otherWidget );

  QWidget *container = new QWidget();
  container->setLayout( horizontalLayout );

  connect( separatorComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudSourceSelect::onSeparatorSelectionChanged );

  return container;
}

void QgsPointCloudSourceSelect::onSeparatorSelectionChanged()
{
  const QComboBox *comboBox = qobject_cast<QComboBox *>( QObject::sender() );
  QLineEdit *otherWidget = comboBox->parentWidget()->findChild<QLineEdit *>();
  if ( otherWidget )
  {
    const QVariant currentData = comboBox->itemData( comboBox->currentIndex() );
    otherWidget->setVisible( currentData.toInt() == QgsPointCloudSourceSelect::SEPARATOR_OTHER );
  }
}

///@endcond
