#include "qgsagentplugin.h"

#include "qgis.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgslayertreeview.h"
#include "qgsmaplayer.h"
#include "qgsprocessing.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingguiutils.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingregistry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QKeyEvent>
#include <QLabel>
#include <QStyle>
#include <QMessageBox>
#include <QMovie>
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>
#include <QLineEdit>

#include <algorithm>
#include <memory>

using namespace Qt::StringLiterals;

namespace
{
  class QgsAgentTaskInputEdit : public QTextEdit
  {
    public:
      explicit QgsAgentTaskInputEdit( QWidget *parent = nullptr )
        : QTextEdit( parent )
      {
      }

      void setRightOverlayMargin( int margin )
      {
        setViewportMargins( 0, 0, margin, 0 );
      }
  };

  const QString sName = QObject::tr( "QGIS Agent" );
  const QString sDescription = QObject::tr( "Codex-style autonomous agent for QGIS tasks" );
  const QString sCategory = QObject::tr( "Plugins" );
  const QString sPluginVersion = QObject::tr( "Version 0.1.0" );
  const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
  const QString sPluginIcon;

  QString layerTypeName( Qgis::LayerType type )
  {
    switch ( type )
    {
      case Qgis::LayerType::Vector:
        return u"vector"_s;
      case Qgis::LayerType::Raster:
        return u"raster"_s;
      case Qgis::LayerType::Plugin:
        return u"plugin"_s;
      case Qgis::LayerType::Mesh:
        return u"mesh"_s;
      case Qgis::LayerType::VectorTile:
        return u"vector_tile"_s;
      case Qgis::LayerType::Annotation:
        return u"annotation"_s;
      case Qgis::LayerType::PointCloud:
        return u"point_cloud"_s;
      case Qgis::LayerType::Group:
        return u"group"_s;
      case Qgis::LayerType::TiledScene:
        return u"tiled_scene"_s;
    }
    return u"unknown"_s;
  }

  QJsonValue jsonValue( const QVariant &value )
  {
    return QJsonValue::fromVariant( value );
  }

  QString compactJson( const QJsonObject &object )
  {
    return QString::fromUtf8( QJsonDocument( object ).toJson( QJsonDocument::Compact ) );
  }

  QString tomlString( const QString &value )
  {
    const QByteArray encoded = QJsonDocument( QJsonArray{ value } ).toJson( QJsonDocument::Compact );
    return QString::fromUtf8( encoded.mid( 1, encoded.size() - 2 ) );
  }

  QString htmlBlock( const QString &role, const QString &message, const QString &extra = QString() )
  {
    return u"<p><b>%1</b><br>%2%3</p>"_s.arg(
      role.toHtmlEscaped(),
      message.toHtmlEscaped().replace( QLatin1Char( '\n' ), u"<br>"_s ),
      extra
    );
  }
}

QgsAgentServer::QgsAgentServer( QgisInterface *interface, QObject *parent )
  : QObject( parent )
  , mInterface( interface )
  , mServer( new QTcpServer( this ) )
{
  const quint64 first = QRandomGenerator::global()->generate64();
  const quint64 second = QRandomGenerator::global()->generate64();
  mToken = u"%1%2"_s.arg( first, 16, 16, QLatin1Char( '0' ) ).arg( second, 16, 16, QLatin1Char( '0' ) );
  connect( mServer, &QTcpServer::newConnection, this, &QgsAgentServer::acceptConnection );
}

bool QgsAgentServer::start()
{
  return mServer->listen( QHostAddress::LocalHost, 0 );
}

quint16 QgsAgentServer::port() const
{
  return mServer->serverPort();
}

QString QgsAgentServer::token() const
{
  return mToken;
}

void QgsAgentServer::setConfirmActions( bool confirm )
{
  mConfirmActions = confirm;
}

void QgsAgentServer::acceptConnection()
{
  while ( QTcpSocket *socket = mServer->nextPendingConnection() )
  {
    mBuffers.insert( socket, QByteArray() );
    connect( socket, &QTcpSocket::readyRead, this, &QgsAgentServer::readRequest );
    connect( socket, &QTcpSocket::disconnected, this, [this, socket] {
      mBuffers.remove( socket );
      socket->deleteLater();
    } );
  }
}

void QgsAgentServer::readRequest()
{
  QTcpSocket *socket = qobject_cast<QTcpSocket *>( sender() );
  if ( !socket )
    return;

  QByteArray &buffer = mBuffers[socket];
  buffer.append( socket->readAll() );

  const qsizetype headerEnd = buffer.indexOf( "\r\n\r\n" );
  if ( headerEnd < 0 )
    return;

  const QByteArray header = buffer.left( headerEnd );
  const QRegularExpression contentLengthExpression( u"Content-Length:\\s*(\\d+)"_s, QRegularExpression::CaseInsensitiveOption );
  const QRegularExpressionMatch lengthMatch = contentLengthExpression.match( QString::fromLatin1( header ) );
  if ( !lengthMatch.hasMatch() )
  {
    sendResponse( socket, 400, QJsonObject{ { u"ok"_s, false }, { u"error"_s, u"Missing Content-Length"_s } } );
    return;
  }

  const qsizetype contentLength = lengthMatch.captured( 1 ).toLongLong();
  const qsizetype bodyStart = headerEnd + 4;
  if ( buffer.size() < bodyStart + contentLength )
    return;

  const QByteArray body = buffer.mid( bodyStart, contentLength );
  buffer.remove( 0, bodyStart + contentLength );

  const QRegularExpression authorizationExpression( u"Authorization:\\s*Bearer\\s+([^\\r\\n]+)"_s, QRegularExpression::CaseInsensitiveOption );
  const QRegularExpressionMatch authorizationMatch = authorizationExpression.match( QString::fromLatin1( header ) );
  if ( !authorizationMatch.hasMatch() || authorizationMatch.captured( 1 ).trimmed() != mToken )
  {
    sendResponse( socket, 401, QJsonObject{ { u"ok"_s, false }, { u"error"_s, u"Unauthorized"_s } } );
    return;
  }

  QJsonParseError parseError;
  const QJsonDocument requestDocument = QJsonDocument::fromJson( body, &parseError );
  if ( parseError.error != QJsonParseError::NoError || !requestDocument.isObject() )
  {
    sendResponse( socket, 400, QJsonObject{ { u"ok"_s, false }, { u"error"_s, parseError.errorString() } } );
    return;
  }

  const QJsonObject request = requestDocument.object();
  const QString tool = request.value( u"tool"_s ).toString();
  const QJsonObject arguments = request.value( u"arguments"_s ).toObject();
  emit activity( tr( "Tool: %1" ).arg( tool ) );
  sendResponse( socket, 200, executeTool( tool, arguments ) );
}

QJsonObject QgsAgentServer::executeTool( const QString &tool, const QJsonObject &arguments )
{
  if ( tool == u"project_summary"_s )
    return projectSummary();
  if ( tool == u"inspect_layer"_s )
    return inspectLayer( arguments );
  if ( tool == u"list_processing_algorithms"_s )
    return listProcessingAlgorithms( arguments );
  if ( tool == u"run_processing_algorithm"_s )
    return runProcessingAlgorithm( arguments );

  return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Unknown tool: %1" ).arg( tool ) } };
}

QJsonObject QgsAgentServer::projectSummary() const
{
  QgsProject *project = QgsProject::instance();
  QJsonArray layers;
  const QMap<QString, QgsMapLayer *> projectLayers = project->mapLayers();
  for ( QgsMapLayer *layer : projectLayers )
    layers.append( layerDetails( layer ) );

  QJsonObject result{
    { u"ok"_s, true },
    { u"qgis_version"_s, Qgis::version() },
    { u"project_title"_s, project->title() },
    { u"project_file"_s, project->fileName() },
    { u"project_crs"_s, project->crs().authid() },
    { u"project_dirty"_s, project->isDirty() },
    { u"layers"_s, layers },
  };
  if ( QgsMapLayer *activeLayer = mInterface->activeLayer() )
    result.insert( u"active_layer_id"_s, activeLayer->id() );

  return result;
}

QJsonObject QgsAgentServer::inspectLayer( const QJsonObject &arguments ) const
{
  const QString layerReference = arguments.value( u"layer"_s ).toString();
  QgsProject *project = QgsProject::instance();
  QgsMapLayer *layer = project->mapLayer( layerReference );
  if ( !layer )
  {
    const QList<QgsMapLayer *> matches = project->mapLayersByName( layerReference );
    if ( matches.size() == 1 )
      layer = matches.constFirst();
    else if ( matches.size() > 1 )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Multiple layers are named '%1'; use a layer ID." ).arg( layerReference ) } };
  }

  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Layer not found: %1" ).arg( layerReference ) } };

  QJsonObject result = layerDetails( layer );
  result.insert( u"ok"_s, true );
  result.insert( u"extent"_s, QJsonArray{ layer->extent().xMinimum(), layer->extent().yMinimum(), layer->extent().xMaximum(), layer->extent().yMaximum() } );

  if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer ) )
  {
    QJsonArray fields;
    const QgsFields layerFields = vectorLayer->fields();
    for ( const QgsField &field : layerFields )
    {
      fields.append( QJsonObject{
        { u"name"_s, field.name() },
        { u"alias"_s, field.alias() },
        { u"type"_s, field.typeName() },
        { u"display_type"_s, field.displayType() },
      } );
    }
    result.insert( u"fields"_s, fields );
  }

  return result;
}

QJsonObject QgsAgentServer::listProcessingAlgorithms( const QJsonObject &arguments ) const
{
  const QString query = arguments.value( u"query"_s ).toString().trimmed();
  const int limit = std::clamp( arguments.value( u"limit"_s ).toInt( 12 ), 1, 50 );
  QJsonArray algorithms;
  const QList<const QgsProcessingAlgorithm *> availableAlgorithms = QgsApplication::processingRegistry()->algorithms();

  for ( const QgsProcessingAlgorithm *algorithm : availableAlgorithms )
  {
    const QString searchable = u"%1 %2 %3 %4"_s.arg( algorithm->id(), algorithm->displayName(), algorithm->group(), algorithm->tags().join( QLatin1Char( ' ' ) ) );
    if ( !query.isEmpty() && !searchable.contains( query, Qt::CaseInsensitive ) )
      continue;

    algorithms.append( algorithmDetails( algorithm ) );
    if ( algorithms.size() >= limit )
      break;
  }

  return QJsonObject{
    { u"ok"_s, true },
    { u"query"_s, query },
    { u"algorithms"_s, algorithms },
  };
}

QJsonObject QgsAgentServer::runProcessingAlgorithm( const QJsonObject &arguments )
{
  const QString algorithmId = arguments.value( u"algorithm_id"_s ).toString();
  const QgsProcessingAlgorithm *algorithm = QgsApplication::processingRegistry()->algorithmById( algorithmId );
  if ( !algorithm )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Processing algorithm not found: %1" ).arg( algorithmId ) } };

  QVariantMap parameters = arguments.value( u"parameters"_s ).toObject().toVariantMap();
  QgsProject *project = QgsProject::instance();
  QgsProcessingContext context;
  context.setProject( project );
  for ( const QgsProcessingParameterDefinition *definition : algorithm->parameterDefinitions() )
  {
    if ( !definition->isDestination() )
      continue;

    const QString parameterName = definition->name();
    const bool hasExplicitOutput = parameters.contains( parameterName ) && !parameters.value( parameterName ).toString().isEmpty();
    const bool requestedTemporaryOutput = hasExplicitOutput && parameters.value( parameterName ).toString() == QgsProcessing::TEMPORARY_OUTPUT;
    const bool needsTemporaryOutput = !hasExplicitOutput || requestedTemporaryOutput;
    if ( needsTemporaryOutput )
    {
      if ( !hasExplicitOutput && definition->flags() & Qgis::ProcessingParameterFlag::Optional )
        continue;

      const auto *destination = dynamic_cast<const QgsProcessingDestinationParameter *>( definition );
      const bool isFileDestination = dynamic_cast<const QgsProcessingParameterFileDestination *>( definition )
                                     || dynamic_cast<const QgsProcessingParameterFolderDestination *>( definition );
      const bool isLayerDestination = dynamic_cast<const QgsProcessingParameterFeatureSink *>( definition )
                                      || dynamic_cast<const QgsProcessingParameterVectorDestination *>( definition )
                                      || dynamic_cast<const QgsProcessingParameterRasterDestination *>( definition )
                                      || dynamic_cast<const QgsProcessingParameterPointCloudDestination *>( definition )
                                      || dynamic_cast<const QgsProcessingParameterVectorTileDestination *>( definition );
      if ( destination && isLayerDestination )
        parameters.insert( parameterName, QVariant::fromValue( QgsProcessingOutputLayerDefinition( QgsProcessing::TEMPORARY_OUTPUT, project ) ) );
      else if ( destination && isFileDestination )
        parameters.insert( parameterName, destination->generateTemporaryDestination( &context ) );
      else
        parameters.insert( parameterName, QgsProcessing::TEMPORARY_OUTPUT );
    }
  }

  QgsProcessingFeedback feedback( true );
  QString validationMessage;
  if ( !algorithm->checkParameterValues( parameters, context, &validationMessage ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, validationMessage } };

  if ( mConfirmActions )
  {
    const QString reason = arguments.value( u"reason"_s ).toString();
    const QString details = tr( "Algorithm: %1\n\nReason: %2\n\nParameters:\n%3" )
                              .arg( algorithm->displayName(), reason.isEmpty() ? tr( "No reason provided" ) : reason, QString::fromUtf8( QJsonDocument( QJsonObject::fromVariantMap( parameters ) ).toJson( QJsonDocument::Indented ) ) );
    if ( QMessageBox::question( mInterface->mainWindow(), tr( "Run QGIS Agent action?" ), details, QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true }, { u"error"_s, tr( "The user declined this action." ) } };
  }

  bool ok = false;
  const QVariantMap rawResults = algorithm->run( parameters, context, &feedback, &ok );
  if ( !ok )
  {
    return QJsonObject{
      { u"ok"_s, false },
      { u"error"_s, tr( "Processing algorithm failed." ) },
      { u"log"_s, feedback.textLog() },
    };
  }

  QVector<QgsProcessingGuiUtils::ResultLayerDetails> layersToAdd;
  QStringList loadedLayerIds;
  const QMap<QString, QgsProcessingContext::LayerDetails> pendingLayers = context.layersToLoadOnCompletion();
  for ( auto it = pendingLayers.constBegin(); it != pendingLayers.constEnd(); ++it )
  {
    QgsMapLayer *layer = context.getMapLayer( it.key() );
    if ( !layer )
      continue;

    QgsMapLayer *ownedLayer = context.takeResultLayer( layer->id() );
    if ( !ownedLayer )
      continue;

    it.value().setOutputLayerName( ownedLayer );
    QgsProcessingGuiUtils::ResultLayerDetails resultDetails( ownedLayer );
    loadedLayerIds.append( ownedLayer->id() );
    resultDetails.destinationProject = it.value().project ? it.value().project : project;
    resultDetails.targetLayerTreeGroup = QgsProcessingGuiUtils::layerTreeResultsGroup( it.value(), context );
    resultDetails.sortKey = it.value().layerSortKey;
    layersToAdd.append( resultDetails );
  }
  QgsProcessingGuiUtils::addResultLayers( layersToAdd, context, mInterface->layerTreeView() );
  if ( !loadedLayerIds.isEmpty() )
    emit layersLoaded( loadedLayerIds, algorithm->displayName() );

  return QJsonObject{
    { u"ok"_s, true },
    { u"algorithm_id"_s, algorithmId },
    { u"results"_s, QJsonObject::fromVariantMap( rawResults ) },
    { u"loaded_layer_ids"_s, QJsonArray::fromStringList( loadedLayerIds ) },
    { u"loaded_layer_count"_s, layersToAdd.size() },
    { u"log"_s, feedback.textLog() },
  };
}

QJsonObject QgsAgentServer::layerDetails( QgsMapLayer *layer ) const
{
  QJsonObject details{
    { u"id"_s, layer->id() },
    { u"name"_s, layer->name() },
    { u"type"_s, layerTypeName( layer->type() ) },
    { u"crs"_s, layer->crs().authid() },
    { u"provider"_s, layer->providerType() },
    { u"source"_s, layer->source() },
    { u"valid"_s, layer->isValid() },
  };

  if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer ) )
  {
    details.insert( u"feature_count"_s, vectorLayer->featureCount() );
    details.insert( u"selected_feature_count"_s, vectorLayer->selectedFeatureCount() );
    details.insert( u"geometry_type"_s, QgsWkbTypes::displayString( vectorLayer->wkbType() ) );
  }
  return details;
}

QJsonObject QgsAgentServer::algorithmDetails( const QgsProcessingAlgorithm *algorithm ) const
{
  QJsonArray parameters;
  for ( const QgsProcessingParameterDefinition *parameter : algorithm->parameterDefinitions() )
  {
    if ( parameter->flags() & Qgis::ProcessingParameterFlag::Hidden )
      continue;

    QJsonObject parameterDetails{
      { u"name"_s, parameter->name() },
      { u"description"_s, parameter->description() },
      { u"type"_s, parameter->type() },
      { u"optional"_s, bool( parameter->flags() & Qgis::ProcessingParameterFlag::Optional ) },
      { u"advanced"_s, bool( parameter->flags() & Qgis::ProcessingParameterFlag::Advanced ) },
      { u"destination"_s, parameter->isDestination() },
      { u"default"_s, jsonValue( parameter->defaultValue() ) },
      { u"raw_definition"_s, QJsonObject::fromVariantMap( parameter->toVariantMap() ) },
    };
    if ( !parameter->help().isEmpty() )
      parameterDetails.insert( u"help"_s, parameter->help() );
    parameters.append( parameterDetails );
  }

  QJsonArray outputs;
  for ( const QgsProcessingOutputDefinition *output : algorithm->outputDefinitions() )
  {
    outputs.append( QJsonObject{
      { u"name"_s, output->name() },
      { u"description"_s, output->description() },
      { u"type"_s, output->type() },
    } );
  }

  return QJsonObject{
    { u"id"_s, algorithm->id() },
    { u"name"_s, algorithm->displayName() },
    { u"group"_s, algorithm->group() },
    { u"description"_s, algorithm->shortDescription() },
    { u"parameters"_s, parameters },
    { u"outputs"_s, outputs },
  };
}

void QgsAgentServer::sendResponse( QTcpSocket *socket, int statusCode, const QJsonObject &body )
{
  const QByteArray payload = QJsonDocument( body ).toJson( QJsonDocument::Compact );
  const QByteArray statusText = statusCode == 200 ? "OK" : "Error";
  QByteArray response = "HTTP/1.1 " + QByteArray::number( statusCode ) + ' ' + statusText + "\r\n";
  response += "Content-Type: application/json\r\n";
  response += "Content-Length: " + QByteArray::number( payload.size() ) + "\r\n";
  response += "Connection: close\r\n\r\n";
  response += payload;
  socket->write( response );
  socket->disconnectFromHost();
}

QgsAgentDockWidget::QgsAgentDockWidget( QgisInterface *interface, QWidget *parent )
  : QDockWidget( tr( "QGIS Agent" ), parent )
  , mInterface( interface )
  , mServer( new QgsAgentServer( interface, this ) )
  , mProcess( new QProcess( this ) )
  , mTimeoutTimer( new QTimer( this ) )
{
  setObjectName( u"QgsAgentDockWidget"_s );
  setMinimumWidth( 380 );

  QWidget *content = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( content );
  layout->setContentsMargins( 8, 8, 8, 8 );

  mTranscript = new QTextBrowser( content );
  mTranscript->setOpenExternalLinks( false );
  mTranscript->setOpenLinks( false );
  mTranscript->document()->addResource(
    QTextDocument::ImageResource,
    QUrl( u"qgis-agent-icon:withdraw"_s ),
    QgsApplication::getThemePixmap( u"/mActionUndo.svg"_s, QColor(), QColor(), 16 )
  );
  mLoadingMovie = new QMovie( QgsApplication::iconPath( u"/mIconLoading.gif"_s ), QByteArray(), this );
  mLoadingMovie->setScaledSize( QSize( 16, 16 ) );
  connect( mLoadingMovie, &QMovie::frameChanged, this, [this] {
    mTranscript->document()->addResource(
      QTextDocument::ImageResource,
      QUrl( u"qgis-agent-icon:loading"_s ),
      mLoadingMovie->currentPixmap()
    );
    mTranscript->document()->markContentsDirty( 0, mTranscript->document()->characterCount() );
    mTranscript->viewport()->update();
  } );
  mTranscript->setPlaceholderText( tr( "Describe a GIS goal. The agent can inspect the active project, plan a workflow, and run QGIS Processing tools." ) );
  layout->addWidget( mTranscript, 1 );

  QgsAgentTaskInputEdit *taskInput = new QgsAgentTaskInputEdit( content );
  mTaskInput = taskInput;
  mTaskInput->setPlaceholderText( tr( "Example: Buffer the active roads layer by 500 meters, dissolve the result, and add it to the project." ) );
  mTaskInput->setMaximumHeight( 110 );
  taskInput->setRightOverlayMargin( 40 );
  mTaskInput->installEventFilter( this );
  layout->addWidget( mTaskInput );

  mActionButton = new QToolButton( mTaskInput );
  mActionButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  mActionButton->setToolTip( tr( "Send" ) );
  mActionButton->setEnabled( false );
  mActionButton->setFixedSize( 32, 32 );
  positionActionButton();
  updateActionButton();

  QHBoxLayout *providerLayout = new QHBoxLayout();
  providerLayout->addWidget( new QLabel( tr( "Provider" ), content ) );
  mProviderCombo = new QComboBox( content );
  populateProviderOptions();
  providerLayout->addWidget( mProviderCombo, 1 );
  layout->addLayout( providerLayout );

  QHBoxLayout *modelLayout = new QHBoxLayout();
  modelLayout->addWidget( new QLabel( tr( "Model" ), content ) );
  mModelCombo = new QComboBox( content );
  mModelCombo->setEditable( true );
  mModelCombo->setInsertPolicy( QComboBox::NoInsert );
  modelLayout->addWidget( mModelCombo, 1 );
  layout->addLayout( modelLayout );
  populateModelOptions();

  mConfirmActions = new QCheckBox( tr( "Confirm write actions" ), content );
  mConfirmActions->setChecked( true );
  layout->addWidget( mConfirmActions );

  mStatusLabel = new QLabel( tr( "Starting local QGIS tools..." ), content );
  mStatusLabel->setWordWrap( true );
  layout->addWidget( mStatusLabel );

  setWidget( content );

  connect( mActionButton, &QToolButton::clicked, this, &QgsAgentDockWidget::primaryActionTriggered );
  connect( mTranscript, &QTextBrowser::anchorClicked, this, &QgsAgentDockWidget::handleTranscriptLink );
  connect( mProviderCombo, &QComboBox::currentIndexChanged, this, &QgsAgentDockWidget::switchProvider );
  connect( mModelCombo, &QComboBox::currentIndexChanged, this, &QgsAgentDockWidget::switchModel );
  connect( mModelCombo->lineEdit(), &QLineEdit::editingFinished, this, &QgsAgentDockWidget::switchModel );
  connect( mConfirmActions, &QCheckBox::toggled, mServer, &QgsAgentServer::setConfirmActions );
  connect( mServer, &QgsAgentServer::activity, this, &QgsAgentDockWidget::showServerActivity );
  connect( mServer, &QgsAgentServer::layersLoaded, this, &QgsAgentDockWidget::recordLoadedLayers );
  connect( mProcess, &QProcess::started, this, &QgsAgentDockWidget::agentStarted );
  connect( mProcess, &QProcess::readyReadStandardOutput, this, &QgsAgentDockWidget::readAgentOutput );
  connect( mProcess, &QProcess::readyReadStandardError, this, &QgsAgentDockWidget::readAgentError );
  connect( mProcess, qOverload<int, QProcess::ExitStatus>( &QProcess::finished ), this, &QgsAgentDockWidget::agentFinished );
  connect( mProcess, &QProcess::errorOccurred, this, [this]( QProcess::ProcessError ) {
    mTimeoutTimer->stop();
    mAgentReady = false;
    mTurnActive = false;
    updateActionButton();
    mStatusLabel->setText( tr( "Agent failed to start." ) );
    appendMessage( tr( "Runtime" ), mProcess->errorString() );
  } );
  mTimeoutTimer->setSingleShot( true );
  connect( mTimeoutTimer, &QTimer::timeout, this, [this] {
    if ( !mAgentReady )
    {
      appendMessage( tr( "Runtime" ), tr( "The Agent session could not start within one minute." ) );
      shutdownAgentSession();
    }
    else
    {
      appendMessage( tr( "Runtime" ), tr( "The task exceeded five minutes and was stopped." ) );
      stopTask();
    }
  } );

  if ( mServer->start() )
  {
    if ( mProviderCombo->count() > 0 )
    {
      mStatusLabel->setText( tr( "Starting Agent session..." ) );
      startAgentSession();
    }
    else
    {
      mStatusLabel->setText( tr( "No supported coding CLI was found. Install TraeX, Codex, Claude Code, OpenCode, or Gemini CLI." ) );
      updateActionButton();
    }
  }
  else
  {
    updateActionButton();
    mStatusLabel->setText( tr( "Could not start the local QGIS tool server." ) );
  }
}

QgsAgentDockWidget::~QgsAgentDockWidget()
{
  shutdownAgentSession();
}

bool QgsAgentDockWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mTaskInput && ( event->type() == QEvent::Resize || event->type() == QEvent::Show ) )
    positionActionButton();
  if ( watched == mTaskInput && event->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    if ( keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter )
    {
      if ( keyEvent->modifiers() & Qt::ShiftModifier )
        return false;
      if ( !mTurnActive )
        submitTask();
      return true;
    }
  }
  return QDockWidget::eventFilter( watched, event );
}

void QgsAgentDockWidget::startAgentSession()
{
  const QString provider = mProviderCombo->currentData().toString();
  const QString executable = agentExecutable();
  const QString python = pythonExecutable();
  if ( executable.isEmpty() )
  {
    mStatusLabel->setText( tr( "%1 CLI was not found." ).arg( agentName() ) );
    appendMessage( tr( "Runtime" ), tr( "Install %1 or add it to PATH." ).arg( agentName() ) );
    return;
  }
  if ( python.isEmpty() )
  {
    mStatusLabel->setText( tr( "Python 3 was not found." ) );
    appendMessage( tr( "Runtime" ), tr( "Python 3 is required for the MCP bridge." ) );
    return;
  }
  if ( !QFileInfo::exists( bridgePath() ) )
  {
    mStatusLabel->setText( tr( "The bundled MCP bridge was not found." ) );
    return;
  }
  if ( ( provider == u"claude"_s || provider == u"opencode"_s || provider == u"gemini"_s ) && !QFileInfo::exists( adapterPath() ) )
  {
    mStatusLabel->setText( tr( "The bundled CLI adapter was not found." ) );
    return;
  }

  QDir().mkpath( agentWorkspace() );
  QFile connectionFile( connectionFilePath() );
  if ( !connectionFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    mStatusLabel->setText( tr( "Could not create the local Agent connection file." ) );
    return;
  }
  connectionFile.setPermissions( QFileDevice::ReadOwner | QFileDevice::WriteOwner );
  connectionFile.write( QJsonDocument( QJsonObject{
    { u"port"_s, mServer->port() },
    { u"token"_s, mServer->token() },
  } ).toJson( QJsonDocument::Compact ) );
  connectionFile.close();

  const QString mcpLogPath = QDir::temp().filePath( u"qgis-agent-mcp-%1.log"_s.arg( QCoreApplication::applicationPid() ) );
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  if ( provider == u"traex"_s )
    environment.insert( u"TRAE_HOME"_s, agentHome() );
  else if ( provider == u"codex"_s )
    environment.insert( u"CODEX_HOME"_s, agentHome() );
  environment.insert( u"QGIS_AGENT_MCP_LOG"_s, mcpLogPath );
  mProcess->setProcessEnvironment( environment );
  mProcess->setWorkingDirectory( agentWorkspace() );
  const QString model = selectedModel();
  mSessionModel = model;

  QStringList arguments;
  if ( provider == u"traex"_s || provider == u"codex"_s )
  {
    mProcess->setProgram( executable );
    arguments = {
      u"-c"_s,
      u"model_reasoning_effort=\"low\""_s,
      u"-c"_s,
      u"approval_policy=\"on-request\""_s,
      u"-c"_s,
      u"approvals_reviewer=\"auto_review\""_s,
      u"-c"_s,
      u"mcp_servers.qgis_agent.command=%1"_s.arg( tomlString( python ) ),
      u"-c"_s,
      u"mcp_servers.qgis_agent.args=[%1]"_s.arg( tomlString( bridgePath() ) ),
      u"-c"_s,
      u"mcp_servers.qgis_agent.env.QGIS_AGENT_CONNECTION_FILE=%1"_s.arg( tomlString( connectionFilePath() ) ),
      u"-c"_s,
      u"mcp_servers.qgis_agent.env.QGIS_AGENT_MCP_LOG=%1"_s.arg( tomlString( mcpLogPath ) ),
    };
    if ( !model.isEmpty() )
      arguments << u"-c"_s << u"model=%1"_s.arg( tomlString( model ) );

    if ( provider == u"traex"_s )
    {
      arguments << u"-c"_s << u"mcp_servers.framelink-figma.enabled=false"_s
                << u"-c"_s << u"mcp_servers.bytedance-figma-mcp.enabled=false"_s
                << u"-c"_s << u"mcp_servers.bam-api-doc.enabled=false"_s
                << u"-c"_s << u"mcp_servers.Bits-DevOps.enabled=false"_s;
    }
    arguments << u"app-server"_s;
  }
  else
  {
    mProcess->setProgram( python );
    arguments = {
      adapterPath(),
      u"--provider"_s,
      provider,
      u"--executable"_s,
      executable,
      u"--bridge"_s,
      bridgePath(),
      u"--connection-file"_s,
      connectionFilePath(),
      u"--workspace"_s,
      agentWorkspace(),
    };
    if ( !model.isEmpty() )
      arguments << u"--model"_s << model;
  }
  mProcess->setArguments( arguments );
  mOutputBuffer.clear();
  mPendingRequests.clear();
  mNextRequestId = 1;
  mShuttingDown = false;
  mProcess->start();
  mTimeoutTimer->start( 60 * 1000 );
}

void QgsAgentDockWidget::shutdownAgentSession()
{
  mShuttingDown = true;
  mTimeoutTimer->stop();
  if ( mProcess->state() != QProcess::NotRunning )
  {
    mProcess->closeWriteChannel();
    if ( !mProcess->waitForFinished( 1500 ) )
    {
      mProcess->terminate();
      if ( !mProcess->waitForFinished( 1000 ) )
        mProcess->kill();
    }
  }
  QFile::remove( connectionFilePath() );
}

qint64 QgsAgentDockWidget::sendRequest( const QString &method, const QJsonObject &params )
{
  if ( mProcess->state() != QProcess::Running )
    return -1;

  const qint64 id = mNextRequestId++;
  mPendingRequests.insert( id, method );
  mProcess->write( QJsonDocument( QJsonObject{
    { u"id"_s, id },
    { u"method"_s, method },
    { u"params"_s, params },
  } ).toJson( QJsonDocument::Compact ) + '\n' );
  return id;
}

void QgsAgentDockWidget::sendNotification( const QString &method, const QJsonObject &params )
{
  if ( mProcess->state() != QProcess::Running )
    return;
  mProcess->write( QJsonDocument( QJsonObject{
    { u"method"_s, method },
    { u"params"_s, params },
  } ).toJson( QJsonDocument::Compact ) + '\n' );
}

void QgsAgentDockWidget::sendResponse( const QJsonValue &id, const QJsonObject &result )
{
  if ( mProcess->state() != QProcess::Running )
    return;
  QJsonObject response{ { u"result"_s, result } };
  response.insert( u"id"_s, id );
  mProcess->write( QJsonDocument( response ).toJson( QJsonDocument::Compact ) + '\n' );
}

void QgsAgentDockWidget::sendErrorResponse( const QJsonValue &id, int code, const QString &message )
{
  if ( mProcess->state() != QProcess::Running )
    return;
  QJsonObject response{ { u"error"_s, QJsonObject{ { u"code"_s, code }, { u"message"_s, message } } } };
  response.insert( u"id"_s, id );
  mProcess->write( QJsonDocument( response ).toJson( QJsonDocument::Compact ) + '\n' );
}

void QgsAgentDockWidget::handleAgentFrame( const QJsonObject &frame )
{
  if ( frame.contains( u"method"_s ) && frame.contains( u"id"_s ) )
    handleAgentRequest( frame );
  else if ( frame.contains( u"method"_s ) )
    handleAgentNotification( frame.value( u"method"_s ).toString(), frame.value( u"params"_s ).toObject() );
  else if ( frame.contains( u"id"_s ) )
    handleAgentResponse( frame );
}

void QgsAgentDockWidget::handleAgentResponse( const QJsonObject &frame )
{
  const qint64 id = frame.value( u"id"_s ).toInteger();
  const QString method = mPendingRequests.take( id );
  if ( method.isEmpty() )
    return;

  if ( frame.contains( u"error"_s ) )
  {
    const QJsonObject error = frame.value( u"error"_s ).toObject();
    appendMessage( tr( "Error" ), error.value( u"message"_s ).toString( compactJson( error ) ) );
    if ( method == u"initialize"_s || method == u"thread/start"_s )
      shutdownAgentSession();
    else if ( method == u"turn/start"_s )
      finishTurn( u"failed"_s );
    return;
  }

  const QJsonObject result = frame.value( u"result"_s ).toObject();
  if ( method == u"initialize"_s )
  {
    sendNotification( u"initialized"_s );
    sendRequest( u"thread/start"_s, QJsonObject{
      { u"cwd"_s, agentWorkspace() },
      { u"ephemeral"_s, true },
      { u"approvalPolicy"_s, u"on-request"_s },
      { u"approvalsReviewer"_s, u"auto_review"_s },
      { u"sandbox"_s, u"read-only"_s },
      { u"developerInstructions"_s, agentInstructions() },
      { u"threadSource"_s, u"user"_s },
    } );
  }
  else if ( method == u"thread/start"_s )
  {
    setAgentReady( result.value( u"thread"_s ).toObject().value( u"id"_s ).toString() );
  }
  else if ( method == u"turn/start"_s )
  {
    const QString turnId = result.value( u"turn"_s ).toObject().value( u"id"_s ).toString();
    if ( !turnId.isEmpty() )
      mTurnId = turnId;
  }
}

void QgsAgentDockWidget::handleAgentNotification( const QString &method, const QJsonObject &params )
{
  const QString notificationThreadId = params.value( u"threadId"_s ).toString();
  if ( !notificationThreadId.isEmpty() && notificationThreadId != mThreadId )
    return;

  if ( method == u"turn/started"_s )
  {
    mTurnId = params.value( u"turn"_s ).toObject().value( u"id"_s ).toString();
    mStatusLabel->setText( tr( "Thinking..." ) );
    if ( mWithdrawnTurnIndexes.contains( mCurrentTurnIndex ) )
      stopTask();
    return;
  }

  if ( method == u"queue/status"_s )
  {
    const QString state = params.value( u"state"_s ).toString();
    const int position = params.value( u"position"_s ).toInt( -1 );
    if ( state == u"ready"_s )
      mStatusLabel->setText( tr( "Thinking..." ) );
    else if ( position >= 0 )
      mStatusLabel->setText( tr( "TraeX is busy. Queue position: %1" ).arg( position ) );
    else
      mStatusLabel->setText( params.value( u"message"_s ).toString( tr( "Waiting for TraeX..." ) ) );
    return;
  }

  if ( method == u"item/agentMessage/delta"_s )
  {
    const QString itemId = params.value( u"itemId"_s ).toString();
    if ( !mStreamingAnswerItemId.isEmpty() && mStreamingAnswerItemId != itemId )
      commitStreamingAnswer();
    mStreamingAnswerItemId = itemId;
    mStreamingAnswer += params.value( u"delta"_s ).toString();
    renderTranscript();
    return;
  }

  if ( method == u"item/reasoning/summaryPartAdded"_s )
  {
    const QString itemId = params.value( u"itemId"_s ).toString();
    if ( !mStreamingReasoningItemId.isEmpty() && mStreamingReasoningItemId != itemId )
      commitStreamingReasoning();
    mStreamingReasoningItemId = itemId;
    if ( !mStreamingReasoning.isEmpty() && !mStreamingReasoning.endsWith( QLatin1Char( '\n' ) ) )
      mStreamingReasoning += QLatin1Char( '\n' );
    renderTranscript();
    return;
  }

  if ( method == u"item/reasoning/summaryTextDelta"_s )
  {
    const QString itemId = params.value( u"itemId"_s ).toString();
    if ( !mStreamingReasoningItemId.isEmpty() && mStreamingReasoningItemId != itemId )
      commitStreamingReasoning();
    mStreamingReasoningItemId = itemId;
    mStreamingReasoning += params.value( u"delta"_s ).toString();
    renderTranscript();
    return;
  }

  if ( method == u"turn/plan/updated"_s )
  {
    mPlanSteps.clear();
    const QJsonArray plan = params.value( u"plan"_s ).toArray();
    for ( const QJsonValue &stepValue : plan )
    {
      const QJsonObject step = stepValue.toObject();
      const QString status = step.value( u"status"_s ).toString();
      const QString marker = status == u"completed"_s ? u"[done]"_s : status == u"inProgress"_s ? u"[working]"_s
                                                                                                 : u"[ ]"_s;
      mPlanSteps.append( u"%1 %2"_s.arg( marker, step.value( u"step"_s ).toString() ) );
    }
    renderTranscript();
    return;
  }

  if ( method == u"item/mcpToolCall/progress"_s )
  {
    const QString itemId = params.value( u"itemId"_s ).toString();
    const QString message = params.value( u"message"_s ).toString();
    if ( !itemId.isEmpty() && !message.isEmpty() )
      mRunningToolActivities.insert( itemId, message );
    renderTranscript();
    return;
  }

  if ( method == u"item/started"_s || method == u"item/completed"_s )
  {
    const QJsonObject item = params.value( u"item"_s ).toObject();
    const QString itemType = item.value( u"type"_s ).toString();
    if ( itemType == u"mcpToolCall"_s )
    {
      const QString itemId = item.value( u"id"_s ).toString();
      const QString tool = item.value( u"tool"_s ).toString();
      const QString label = tr( "%1.%2" ).arg( item.value( u"server"_s ).toString(), tool );
      if ( method == u"item/started"_s )
      {
        mRunningToolActivities.insert( itemId, tr( "Running %1" ).arg( label ) );
        mStatusLabel->setText( tr( "Tool: %1" ).arg( tool ) );
      }
      else
      {
        mRunningToolActivities.remove( itemId );
        const QString status = item.value( u"status"_s ).toString();
        mCompletedToolActivities.append( tr( "%1: %2" ).arg( label, status ) );
        mStatusLabel->setText( tr( "Thinking..." ) );
      }
      renderTranscript();
    }
    else if ( method == u"item/completed"_s && itemType == u"agentMessage"_s )
    {
      const QString itemId = item.value( u"id"_s ).toString();
      const QString text = item.value( u"text"_s ).toString();
      if ( itemId == mStreamingAnswerItemId )
      {
        if ( !text.isEmpty() )
          mStreamingAnswer = text;
        commitStreamingAnswer();
      }
      else if ( !text.isEmpty() )
        appendMessage( tr( "Agent" ), text );
    }
    else if ( method == u"item/completed"_s && itemType == u"reasoning"_s
              && item.value( u"id"_s ).toString() == mStreamingReasoningItemId )
    {
      commitStreamingReasoning();
    }
    return;
  }

  if ( method == u"turn/completed"_s )
  {
    const QString threadId = params.value( u"threadId"_s ).toString();
    const QJsonObject turn = params.value( u"turn"_s ).toObject();
    const QString completedTurnId = turn.value( u"id"_s ).toString();
    const bool matchesThread = threadId.isEmpty() || threadId == mThreadId;
    const bool matchesTurn = mTurnId.isEmpty() || completedTurnId.isEmpty() || completedTurnId == mTurnId;
    if ( mTurnActive && matchesThread && matchesTurn )
    {
      const QJsonObject error = turn.value( u"error"_s ).toObject();
      const QString status = turn.value( u"status"_s ).toString();
      finishTurn( status.isEmpty() ? u"completed"_s : status );
      if ( !error.isEmpty() )
        appendMessage( tr( "Error" ), error.value( u"message"_s ).toString( compactJson( error ) ) );
    }
    return;
  }

  if ( method == u"error"_s )
  {
    const QJsonObject error = params.value( u"error"_s ).toObject();
    appendMessage( tr( "Error" ), error.value( u"message"_s ).toString( compactJson( params ) ) );
  }
}

void QgsAgentDockWidget::handleAgentRequest( const QJsonObject &frame )
{
  const QString method = frame.value( u"method"_s ).toString();
  const QJsonValue id = frame.value( u"id"_s );
  if ( method == u"item/permissions/requestApproval"_s )
  {
    sendResponse( id, QJsonObject{ { u"permissions"_s, QJsonObject() }, { u"scope"_s, u"turn"_s }, { u"strictAutoReview"_s, true } } );
  }
  else if ( method == u"mcpServer/elicitation/request"_s )
  {
    sendResponse( id, QJsonObject{ { u"action"_s, u"decline"_s }, { u"content"_s, QJsonValue::Null }, { u"_meta"_s, QJsonValue::Null } } );
  }
  else
  {
    sendErrorResponse( id, -32000, tr( "QGIS Agent rejected unsupported request %1" ).arg( method ) );
  }
}

void QgsAgentDockWidget::setAgentReady( const QString &threadId )
{
  if ( threadId.isEmpty() )
  {
    appendMessage( tr( "Runtime" ), tr( "Codex app-server did not return a thread ID." ) );
    shutdownAgentSession();
    return;
  }
  mTimeoutTimer->stop();
  mThreadId = threadId;
  mAgentReady = true;
  updateActionButton();
  mStatusLabel->setText( tr( "Ready. Agent session is active." ) );
  appendMessage( tr( "System" ), tr( "Agent session is ready. It will decide when QGIS tools are needed." ) );
}

void QgsAgentDockWidget::finishTurn( const QString &status )
{
  mTimeoutTimer->stop();
  commitStreamingReasoning();
  if ( !mPlanSteps.isEmpty() )
  {
    mTranscriptBlocks.append( TranscriptBlock{ tr( "Plan" ), mPlanSteps.join( QLatin1Char( '\n' ) ), QStringList(), false, mCurrentTurnIndex, false, false } );
  }
  if ( !mCompletedToolActivities.isEmpty() )
  {
    QString toolsMessage = mCompletedToolActivities.join( QLatin1Char( '\n' ) );
    if ( !mCurrentTurnUndoDescriptions.isEmpty() )
      toolsMessage += u"\n%1"_s.arg( tr( "Loaded layers: %1" ).arg( mCurrentTurnUndoDescriptions.join( u", "_s ) ) );
    mTranscriptBlocks.append( TranscriptBlock{ tr( "Tools" ), toolsMessage, QStringList(), false, mCurrentTurnIndex, false, false } );
  }
  commitStreamingAnswer();
  if ( !mCurrentTurnUndoLayerIds.isEmpty() && mCurrentTurnUserBlockIndex >= 0 && mCurrentTurnUserBlockIndex < mTranscriptBlocks.size() )
    mTranscriptBlocks[mCurrentTurnUserBlockIndex].undoLayerIds = mCurrentTurnUndoLayerIds;
  mPlanSteps.clear();
  mCompletedToolActivities.clear();
  mCurrentTurnUndoLayerIds.clear();
  mCurrentTurnUndoDescriptions.clear();
  mCurrentTurnUserBlockIndex = -1;
  mCurrentTurnIndex = -1;
  mRunningToolActivities.clear();
  mTurnActive = false;
  mTurnId.clear();
  updateActionButton();
  mStatusLabel->setText( status == u"completed"_s ? tr( "Task finished." ) : tr( "Task ended: %1" ).arg( status ) );
  renderTranscript();
}

void QgsAgentDockWidget::switchProvider()
{
  if ( mTurnActive )
  {
    QMessageBox::information( this, tr( "QGIS Agent" ), tr( "Stop the current task before changing providers." ) );
    return;
  }
  populateModelOptions();
  shutdownAgentSession();
  mShuttingDown = false;
  mStatusLabel->setText( tr( "Starting %1 session..." ).arg( agentName() ) );
  startAgentSession();
}

void QgsAgentDockWidget::switchModel()
{
  if ( selectedModel() == mSessionModel )
    return;
  if ( mTurnActive )
  {
    QMessageBox::information( this, tr( "QGIS Agent" ), tr( "Stop the current task before changing models." ) );
    return;
  }
  if ( !mAgentReady && mProcess->state() == QProcess::NotRunning )
    return;
  shutdownAgentSession();
  mShuttingDown = false;
  mStatusLabel->setText( selectedModel().isEmpty()
                           ? tr( "Starting %1 session with default model..." ).arg( agentName() )
                           : tr( "Starting %1 session with %2..." ).arg( agentName(), selectedModel() ) );
  startAgentSession();
}

void QgsAgentDockWidget::primaryActionTriggered()
{
  if ( mTurnActive )
    stopTask();
  else
    submitTask();
}

void QgsAgentDockWidget::submitTask()
{
  const QString task = mTaskInput->toPlainText().trimmed();
  if ( task.isEmpty() || !mAgentReady || mTurnActive )
    return;

  mCurrentTurnIndex = mNextTurnIndex++;
  appendMessage( tr( "You" ), task, QStringList(), mCurrentTurnIndex, true );
  mCurrentTurnUserBlockIndex = mTranscriptBlocks.size() - 1;
  mTaskInput->clear();
  resetTurnStreaming();
  mTurnActive = true;
  mTurnId.clear();
  updateActionButton();
  renderTranscript();
  mStatusLabel->setText( tr( "Agent is thinking..." ) );

  sendRequest( u"turn/start"_s, QJsonObject{
    { u"threadId"_s, mThreadId },
    { u"clientUserMessageId"_s, QUuid::createUuid().toString( QUuid::WithoutBraces ) },
    { u"input"_s, QJsonArray{ QJsonObject{ { u"type"_s, u"text"_s }, { u"text"_s, task } } } },
    { u"effort"_s, u"low"_s },
    { u"summary"_s, u"concise"_s },
    { u"approvalPolicy"_s, u"on-request"_s },
    { u"approvalsReviewer"_s, u"auto_review"_s },
    { u"sandboxPolicy"_s, QJsonObject{ { u"type"_s, u"readOnly"_s }, { u"networkAccess"_s, false } } },
  } );
  mTimeoutTimer->start( 5 * 60 * 1000 );
}

void QgsAgentDockWidget::stopTask()
{
  if ( !mTurnActive )
    return;
  if ( mTurnId.isEmpty() )
  {
    mStatusLabel->setText( tr( "Waiting for current turn..." ) );
    return;
  }
  sendRequest( u"turn/interrupt"_s, QJsonObject{ { u"threadId"_s, mThreadId }, { u"turnId"_s, mTurnId } } );
  mStatusLabel->setText( tr( "Stopping current task..." ) );
}

void QgsAgentDockWidget::agentStarted()
{
  sendRequest( u"initialize"_s, QJsonObject{
    { u"clientInfo"_s, QJsonObject{ { u"name"_s, u"qgis_agent"_s }, { u"title"_s, u"QGIS Agent"_s }, { u"version"_s, u"0.1.0"_s } } },
    { u"capabilities"_s, QJsonObject{
        { u"experimentalApi"_s, true },
        { u"requestAttestation"_s, false },
        { u"mcpServerOpenaiFormElicitation"_s, false },
        { u"extensions"_s, QJsonObject() },
      } },
  } );
}

void QgsAgentDockWidget::readAgentOutput()
{
  mOutputBuffer.append( mProcess->readAllStandardOutput() );
  while ( true )
  {
    const qsizetype newline = mOutputBuffer.indexOf( '\n' );
    if ( newline < 0 )
      break;

    const QByteArray line = mOutputBuffer.left( newline ).trimmed();
    mOutputBuffer.remove( 0, newline + 1 );
    if ( line.isEmpty() )
      continue;

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson( line, &error );
    if ( error.error == QJsonParseError::NoError && document.isObject() )
      handleAgentFrame( document.object() );
    else
      appendMessage( tr( "Runtime" ), QString::fromUtf8( line ) );
  }
}

void QgsAgentDockWidget::readAgentError()
{
  const QString output = QString::fromUtf8( mProcess->readAllStandardError() ).trimmed();
  if ( !output.isEmpty() )
    appendMessage( tr( "Runtime" ), output );
}

void QgsAgentDockWidget::agentFinished( int exitCode, QProcess::ExitStatus status )
{
  mTimeoutTimer->stop();
  mAgentReady = false;
  mTurnActive = false;
  mPendingRequests.clear();
  mThreadId.clear();
  mTurnId.clear();
  mSessionModel.clear();
  QFile::remove( connectionFilePath() );
  updateActionButton();
  if ( !mShuttingDown )
  {
    const QString message = status == QProcess::NormalExit && exitCode == 0
                              ? tr( "Agent session ended." )
                              : tr( "Agent session stopped with exit code %1." ).arg( exitCode );
    mStatusLabel->setText( message );
    appendMessage( tr( "Runtime" ), message );
  }
}

void QgsAgentDockWidget::showServerActivity( const QString &message )
{
  mStatusLabel->setText( message );
}

void QgsAgentDockWidget::recordLoadedLayers( const QStringList &layerIds, const QString &description )
{
  for ( const QString &layerId : layerIds )
  {
    if ( !mCurrentTurnUndoLayerIds.contains( layerId ) )
      mCurrentTurnUndoLayerIds.append( layerId );
  }
  if ( !description.isEmpty() )
    mCurrentTurnUndoDescriptions.append( description );
}

void QgsAgentDockWidget::handleTranscriptLink( const QUrl &url )
{
  if ( url.scheme() != u"qgis-agent"_s || ( url.host() != u"undo"_s && url.host() != u"withdraw"_s ) )
  {
    QDesktopServices::openUrl( url );
    return;
  }

  bool ok = false;
  const int blockIndex = url.path().mid( 1 ).toInt( &ok );
  if ( ok && url.host() == u"undo"_s )
    undoTranscriptBlock( blockIndex );
  else if ( ok )
    withdrawTurn( blockIndex );
}

void QgsAgentDockWidget::undoTranscriptBlock( int blockIndex )
{
  if ( blockIndex < 0 || blockIndex >= mTranscriptBlocks.size() )
    return;

  TranscriptBlock &block = mTranscriptBlocks[blockIndex];
  if ( block.undone || block.undoLayerIds.isEmpty() )
    return;

  QStringList removedLayerIds;
  QgsProject *project = QgsProject::instance();
  for ( const QString &layerId : std::as_const( block.undoLayerIds ) )
  {
    if ( project->mapLayer( layerId ) )
      removedLayerIds.append( layerId );
  }
  if ( !removedLayerIds.isEmpty() )
    project->removeMapLayers( removedLayerIds );

  block.undone = true;
  renderTranscript();
}

void QgsAgentDockWidget::withdrawTurn( int turnIndex )
{
  if ( turnIndex < 0 || mWithdrawnTurnIndexes.contains( turnIndex ) )
    return;

  if ( mTurnActive && turnIndex == mCurrentTurnIndex )
    stopTask();

  QStringList removedLayerIds;
  QgsProject *project = QgsProject::instance();
  for ( const TranscriptBlock &block : std::as_const( mTranscriptBlocks ) )
  {
    if ( block.turnIndex != turnIndex )
      continue;
    for ( const QString &layerId : block.undoLayerIds )
    {
      if ( !removedLayerIds.contains( layerId ) && project->mapLayer( layerId ) )
        removedLayerIds.append( layerId );
    }
  }
  if ( !removedLayerIds.isEmpty() )
    project->removeMapLayers( removedLayerIds );

  mWithdrawnTurnIndexes.insert( turnIndex );
  for ( TranscriptBlock &block : mTranscriptBlocks )
  {
    if ( block.turnIndex == turnIndex )
      block.withdrawn = true;
  }
  renderTranscript();
}

void QgsAgentDockWidget::updateActionButton()
{
  if ( !mActionButton )
    return;

  if ( mLoadingMovie )
  {
    if ( mTurnActive )
      mLoadingMovie->start();
    else
      mLoadingMovie->stop();
  }

  mActionButton->setEnabled( mAgentReady || mTurnActive );
  if ( mTurnActive )
  {
    mActionButton->setIcon( style()->standardIcon( QStyle::SP_MediaPause ) );
    mActionButton->setText( tr( "Pause" ) );
    mActionButton->setToolTip( tr( "Pause current task" ) );
  }
  else
  {
    mActionButton->setIcon( style()->standardIcon( QStyle::SP_ArrowForward ) );
    mActionButton->setText( tr( "Send" ) );
    mActionButton->setToolTip( tr( "Send" ) );
  }
}

void QgsAgentDockWidget::appendMessage( const QString &role, const QString &message )
{
  appendMessage( role, message, QStringList(), mCurrentTurnIndex, false );
}

void QgsAgentDockWidget::appendMessage( const QString &role, const QString &message, const QStringList &undoLayerIds )
{
  appendMessage( role, message, undoLayerIds, mCurrentTurnIndex, false );
}

void QgsAgentDockWidget::appendMessage( const QString &role, const QString &message, const QStringList &undoLayerIds, int turnIndex, bool userBlock )
{
  mTranscriptBlocks.append( TranscriptBlock{ role, message, undoLayerIds, false, turnIndex, userBlock, false } );
  renderTranscript();
}

void QgsAgentDockWidget::renderTranscript()
{
  QStringList blocks;
  const bool currentTurnWithdrawn = mCurrentTurnIndex >= 0 && mWithdrawnTurnIndexes.contains( mCurrentTurnIndex );
  for ( int i = 0; i < mTranscriptBlocks.size(); ++i )
  {
    const TranscriptBlock &block = mTranscriptBlocks.at( i );
    const bool turnWithdrawn = block.withdrawn || ( block.turnIndex >= 0 && mWithdrawnTurnIndexes.contains( block.turnIndex ) );
    if ( turnWithdrawn && !block.userBlock )
      continue;
    if ( turnWithdrawn && block.userBlock )
    {
      blocks.append( htmlBlock( block.role, tr( "Conversation withdrawn" ) ) );
      continue;
    }
    QString extra;
    if ( block.userBlock && block.turnIndex >= 0 )
    {
      extra = u"<br><a href=\"qgis-agent://withdraw/%1\" title=\"%2\"><img src=\"qgis-agent-icon:withdraw\" width=\"16\" height=\"16\" alt=\"%2\"></a>"_s.arg(
        QString::number( block.turnIndex ),
        tr( "Withdraw" ).toHtmlEscaped()
      );
    }
    else if ( !block.undoLayerIds.isEmpty() )
    {
      extra = block.undone
                ? u"<br><span style=\"color:#6b7280\">%1</span>"_s.arg( tr( "Changes undone" ).toHtmlEscaped() )
                : u"<br><a href=\"qgis-agent://undo/%1\">%2</a>"_s.arg( QString::number( i ), tr( "Undo changes" ).toHtmlEscaped() );
    }
    blocks.append( htmlBlock( block.role, block.message, extra ) );
  }
  if ( !currentTurnWithdrawn && !mStreamingReasoning.isEmpty() )
  {
    blocks.append( u"<div style=\"color:#6b7280;margin:6px 0\"><b>%1</b><br>%2</div>"_s.arg(
      tr( "Thinking summary" ).toHtmlEscaped(),
      mStreamingReasoning.toHtmlEscaped().replace( QLatin1Char( '\n' ), u"<br>"_s )
    ) );
  }
  if ( !currentTurnWithdrawn && !mPlanSteps.isEmpty() )
  {
    QStringList escapedSteps;
    for ( const QString &step : std::as_const( mPlanSteps ) )
      escapedSteps.append( step.toHtmlEscaped() );
    blocks.append( u"<div style=\"color:#4b5563;margin:6px 0\"><b>%1</b><br>%2</div>"_s.arg( tr( "Plan" ).toHtmlEscaped(), escapedSteps.join( u"<br>"_s ) ) );
  }
  if ( !currentTurnWithdrawn && ( !mCompletedToolActivities.isEmpty() || !mRunningToolActivities.isEmpty() ) )
  {
    QStringList activities;
    for ( const QString &activity : std::as_const( mCompletedToolActivities ) )
      activities.append( activity.toHtmlEscaped() );
    QStringList running = mRunningToolActivities.values();
    running.sort();
    for ( const QString &activity : std::as_const( running ) )
      activities.append( activity.toHtmlEscaped() );
    blocks.append( u"<div style=\"color:#4b5563;margin:6px 0\"><b>%1</b><br>%2</div>"_s.arg( tr( "Activity" ).toHtmlEscaped(), activities.join( u"<br>"_s ) ) );
  }
  if ( !currentTurnWithdrawn && !mStreamingAnswer.isEmpty() )
  {
    blocks.append( u"<p><b>%1</b><br>%2</p>"_s.arg(
      tr( "Agent" ).toHtmlEscaped(),
      mStreamingAnswer.toHtmlEscaped().replace( QLatin1Char( '\n' ), u"<br>"_s )
    ) );
  }
  if ( mTurnActive && !currentTurnWithdrawn )
    blocks.append( u"<div style=\"margin:6px 0\"><img src=\"qgis-agent-icon:loading\" width=\"16\" height=\"16\"></div>"_s );
  mTranscript->setHtml( blocks.join( QLatin1Char( '\n' ) ) );
  mTranscript->verticalScrollBar()->setValue( mTranscript->verticalScrollBar()->maximum() );
}

void QgsAgentDockWidget::resetTurnStreaming()
{
  mStreamingAnswer.clear();
  mStreamingAnswerItemId.clear();
  mStreamingReasoning.clear();
  mStreamingReasoningItemId.clear();
  mPlanSteps.clear();
  mCompletedToolActivities.clear();
  mRunningToolActivities.clear();
  mCurrentTurnUndoLayerIds.clear();
  mCurrentTurnUndoDescriptions.clear();
  renderTranscript();
}

void QgsAgentDockWidget::commitStreamingAnswer()
{
  if ( !mStreamingAnswer.isEmpty() && !mWithdrawnTurnIndexes.contains( mCurrentTurnIndex ) )
    mTranscriptBlocks.append( TranscriptBlock{ tr( "Agent" ), mStreamingAnswer, QStringList(), false, mCurrentTurnIndex, false, false } );
  mStreamingAnswer.clear();
  mStreamingAnswerItemId.clear();
}

void QgsAgentDockWidget::commitStreamingReasoning()
{
  if ( !mStreamingReasoning.isEmpty() && !mWithdrawnTurnIndexes.contains( mCurrentTurnIndex ) )
    mTranscriptBlocks.append( TranscriptBlock{ tr( "Thinking summary" ), mStreamingReasoning, QStringList(), false, mCurrentTurnIndex, false, false } );
  mStreamingReasoning.clear();
  mStreamingReasoningItemId.clear();
}

void QgsAgentDockWidget::populateProviderOptions()
{
  if ( !mProviderCombo )
    return;

  mProviderCombo->clear();
  if ( !agentExecutable( u"traex"_s ).isEmpty() )
    mProviderCombo->addItem( tr( "TraeX" ), u"traex"_s );
  if ( !agentExecutable( u"codex"_s ).isEmpty() )
    mProviderCombo->addItem( tr( "Codex" ), u"codex"_s );
  if ( !agentExecutable( u"claude"_s ).isEmpty() )
    mProviderCombo->addItem( tr( "Claude Code" ), u"claude"_s );
  if ( !agentExecutable( u"opencode"_s ).isEmpty() )
    mProviderCombo->addItem( tr( "OpenCode" ), u"opencode"_s );
  if ( !agentExecutable( u"gemini"_s ).isEmpty() )
    mProviderCombo->addItem( tr( "Gemini CLI" ), u"gemini"_s );
}

void QgsAgentDockWidget::populateModelOptions()
{
  if ( !mModelCombo || !mProviderCombo )
    return;

  const QString currentModel = mModelCombo->currentData().toString();
  const bool wasBlocked = mModelCombo->blockSignals( true );
  mModelCombo->clear();

  if ( mProviderCombo->currentData().toString() == u"traex"_s )
  {
    mModelCombo->addItem( u"GPT-5.5"_s, u"GPT-5.5"_s );
    mModelCombo->addItem( u"GPT-5.6-Sol"_s, u"GPT-5.6-Sol"_s );
    mModelCombo->addItem( u"GPT-5.4"_s, u"GPT-5.4"_s );
    mModelCombo->addItem( u"GPT-5.2"_s, u"GPT-5.2"_s );
    mModelCombo->addItem( u"GPT-6-Astra"_s, u"GPT-6-Astra"_s );
    mModelCombo->addItem( u"Seed-Code"_s, u"Seed-Code"_s );
    mModelCombo->addItem( u"DeepSeek-V4-Pro"_s, u"DeepSeek-V4-Pro"_s );
    mModelCombo->addItem( u"DeepSeek-V4-Flash"_s, u"DeepSeek-V4-Flash"_s );
    mModelCombo->addItem( tr( "TraeX default" ), QString() );
  }
  else if ( mProviderCombo->currentData().toString() == u"codex"_s )
  {
    mModelCombo->addItem( tr( "Codex default" ), QString() );
  }
  else if ( mProviderCombo->currentData().toString() == u"claude"_s )
  {
    mModelCombo->addItem( tr( "Claude default" ), QString() );
    mModelCombo->addItem( u"sonnet"_s, u"sonnet"_s );
    mModelCombo->addItem( u"opus"_s, u"opus"_s );
    mModelCombo->addItem( u"haiku"_s, u"haiku"_s );
  }
  else if ( mProviderCombo->currentData().toString() == u"opencode"_s )
  {
    mModelCombo->addItem( tr( "OpenCode default" ), QString() );
    mModelCombo->addItem( u"ai_agent_proxy_chat/gpt-5.5-2026-04-24"_s, u"ai_agent_proxy_chat/gpt-5.5-2026-04-24"_s );
    mModelCombo->addItem( u"openrouter/~anthropic/claude-sonnet-latest"_s, u"openrouter/~anthropic/claude-sonnet-latest"_s );
    mModelCombo->addItem( u"openrouter/~deepseek/deepseek-pro-latest"_s, u"openrouter/~deepseek/deepseek-pro-latest"_s );
  }
  else if ( mProviderCombo->currentData().toString() == u"gemini"_s )
  {
    mModelCombo->addItem( tr( "Gemini default" ), QString() );
    mModelCombo->addItem( u"gemini-2.5-pro"_s, u"gemini-2.5-pro"_s );
    mModelCombo->addItem( u"gemini-2.5-flash"_s, u"gemini-2.5-flash"_s );
  }

  const int currentIndex = currentModel.isEmpty() ? -1 : mModelCombo->findData( currentModel );
  if ( currentIndex >= 0 )
    mModelCombo->setCurrentIndex( currentIndex );
  mModelCombo->blockSignals( wasBlocked );
}

void QgsAgentDockWidget::positionActionButton()
{
  if ( !mTaskInput || !mActionButton )
    return;

  const int margin = 6;
  const QRect editorRect = mTaskInput->contentsRect();
  mActionButton->move( editorRect.right() - mActionButton->width() - margin, editorRect.bottom() - mActionButton->height() - margin );
}

QString QgsAgentDockWidget::agentExecutable( const QString &provider ) const
{
  QString command;
  QString environmentName;
  if ( provider == u"traex"_s )
  {
    command = u"traex"_s;
    environmentName = u"QGIS_AGENT_TRAEX"_s;
  }
  else if ( provider == u"codex"_s )
  {
    command = u"codex"_s;
    environmentName = u"QGIS_AGENT_CODEX"_s;
  }
  else if ( provider == u"claude"_s )
  {
    command = u"claude"_s;
    environmentName = u"QGIS_AGENT_CLAUDE"_s;
  }
  else if ( provider == u"opencode"_s )
  {
    command = u"opencode"_s;
    environmentName = u"QGIS_AGENT_OPENCODE"_s;
  }
  else if ( provider == u"gemini"_s )
  {
    command = u"gemini"_s;
    environmentName = u"QGIS_AGENT_GEMINI"_s;
  }
  else
  {
    return QString();
  }

  const QString configured = qEnvironmentVariable( environmentName.toUtf8().constData() );
  if ( !configured.isEmpty() && QFileInfo::exists( configured ) )
    return configured;
  const QString executable = QStandardPaths::findExecutable( command );
  if ( !executable.isEmpty() )
    return executable;

  const QStringList fallbackDirectories{
    QDir::home().filePath( u".local/bin"_s ),
    QDir::home().filePath( u".local/share/pnpm"_s ),
    QDir::home().filePath( u".volta/bin"_s ),
    QDir::home().filePath( u".bun/bin"_s ),
    u"/opt/homebrew/bin"_s,
    u"/usr/local/bin"_s,
    qEnvironmentVariable( "NVM_BIN" ),
  };
  for ( const QString &directory : fallbackDirectories )
  {
    if ( directory.isEmpty() )
      continue;
    const QString candidate = QDir( directory ).filePath( command );
    if ( QFileInfo::exists( candidate ) && QFileInfo( candidate ).isExecutable() )
      return candidate;
  }

  QDir nodeVersions( QDir::home().filePath( u".nvm/versions/node"_s ) );
  const QStringList versions = nodeVersions.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed );
  for ( const QString &version : versions )
  {
    const QString candidate = nodeVersions.filePath( u"%1/bin/%2"_s.arg( version, command ) );
    if ( QFileInfo::exists( candidate ) && QFileInfo( candidate ).isExecutable() )
      return candidate;
  }
  return QString();
}

QString QgsAgentDockWidget::agentExecutable() const
{
  return agentExecutable( mProviderCombo ? mProviderCombo->currentData().toString() : u"traex"_s );
}

QString QgsAgentDockWidget::agentHome() const
{
  const QString provider = mProviderCombo ? mProviderCombo->currentData().toString() : QString();
  if ( provider == u"traex"_s )
    return qEnvironmentVariable( "QGIS_AGENT_TRAE_HOME", QDir::home().filePath( u".trae"_s ) );
  if ( provider == u"codex"_s )
    return qEnvironmentVariable( "QGIS_AGENT_CODEX_HOME", QDir::home().filePath( u".codex"_s ) );
  return QString();
}

QString QgsAgentDockWidget::agentName() const
{
  const QString provider = mProviderCombo ? mProviderCombo->currentData().toString() : QString();
  if ( provider == u"traex"_s )
    return u"TraeX"_s;
  if ( provider == u"codex"_s )
    return u"Codex"_s;
  if ( provider == u"claude"_s )
    return u"Claude Code"_s;
  if ( provider == u"opencode"_s )
    return u"OpenCode"_s;
  if ( provider == u"gemini"_s )
    return u"Gemini CLI"_s;
  return tr( "Agent" );
}

QString QgsAgentDockWidget::selectedModel() const
{
  if ( !mModelCombo )
    return QString();

  const QString modelText = mModelCombo->currentText().trimmed();
  if ( mModelCombo->currentIndex() >= 0 && modelText == mModelCombo->itemText( mModelCombo->currentIndex() ).trimmed() )
    return mModelCombo->currentData().toString();
  return modelText;
}

QString QgsAgentDockWidget::pythonExecutable() const
{
  const QString configured = qEnvironmentVariable( "QGIS_AGENT_PYTHON" );
  if ( !configured.isEmpty() && QFileInfo::exists( configured ) )
    return configured;
  for ( const QString &candidate : { u"python3"_s, u"python"_s } )
  {
    const QString executable = QStandardPaths::findExecutable( candidate );
    if ( !executable.isEmpty() )
      return executable;
  }
  return QString();
}

QString QgsAgentDockWidget::bridgePath() const
{
  return QDir( QgsApplication::pluginPath() ).filePath( u"qgis_agent_mcp_bridge.py"_s );
}

QString QgsAgentDockWidget::adapterPath() const
{
  return QDir( QgsApplication::pluginPath() ).filePath( u"qgis_agent_cli_adapter.py"_s );
}

QString QgsAgentDockWidget::connectionFilePath() const
{
  return QDir( agentWorkspace() ).filePath( u"connection.json"_s );
}

QString QgsAgentDockWidget::agentWorkspace() const
{
  return QDir::temp().filePath( u"qgis-agent-%1"_s.arg( QCoreApplication::applicationPid() ) );
}

QString QgsAgentDockWidget::agentInstructions() const
{
  return tr(
    "You are the autonomous GIS agent embedded in QGIS. Interpret each user request yourself and decide whether QGIS tools are needed. "
    "Answer greetings, identity questions, and general explanations directly without calling tools. "
    "For tasks that depend on the live project, inspect the project and relevant layers before planning or acting. "
    "Search Processing algorithms before execution, use exact layer IDs, account for CRS and distance units, and validate results. "
    "Use only qgis_agent MCP tools for QGIS actions. Do not use shell or file-editing tools. "
    "Destructive QGIS actions are confirmed in the application. If essential information is missing, explain the exact blocker."
  );
}

QgsAgentPlugin::QgsAgentPlugin( QgisInterface *interface )
  : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
  , mInterface( interface )
{
}

void QgsAgentPlugin::initGui()
{
  mAction = new QAction( QgsApplication::getThemeIcon( u"/processingAlgorithm.svg"_s ), tr( "QGIS Agent" ), this );
  mAction->setCheckable( true );
  connect( mAction, &QAction::triggered, this, &QgsAgentPlugin::showPanel );
  mInterface->addPluginToMenu( tr( "&QGIS Agent" ), mAction );
  mInterface->addToolBarIcon( mAction );
  showPanel();
}

void QgsAgentPlugin::unload()
{
  if ( mDock )
  {
    mInterface->removeDockWidget( mDock );
    delete mDock;
  }
  if ( mAction )
  {
    mInterface->removePluginMenu( tr( "&QGIS Agent" ), mAction );
    mInterface->removeToolBarIcon( mAction );
    delete mAction;
    mAction = nullptr;
  }
}

void QgsAgentPlugin::showPanel()
{
  if ( !mDock )
  {
    mDock = new QgsAgentDockWidget( mInterface, mInterface->mainWindow() );
    mInterface->addDockWidget( Qt::RightDockWidgetArea, mDock );
    connect( mDock, &QDockWidget::visibilityChanged, mAction, &QAction::setChecked );
  }
  mDock->show();
  mDock->raise();
  mAction->setChecked( true );
}

QGISEXTERN QgisPlugin *classFactory( QgisInterface *interface )
{
  return new QgsAgentPlugin( interface );
}

QGISEXTERN const QString *name()
{
  return &sName;
}

QGISEXTERN const QString *description()
{
  return &sDescription;
}

QGISEXTERN int type()
{
  return sPluginType;
}

QGISEXTERN const QString *category()
{
  return &sCategory;
}

QGISEXTERN const QString *version()
{
  return &sPluginVersion;
}

QGISEXTERN const QString *icon()
{
  return &sPluginIcon;
}

QGISEXTERN void unload( QgisPlugin *plugin )
{
  delete plugin;
}

#include "moc_qgsagentplugin.cpp"
