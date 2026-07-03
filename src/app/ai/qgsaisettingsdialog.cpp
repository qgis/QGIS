/***************************************************************************
    qgsaisettingsdialog.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaisettingsdialog.h"

#include <algorithm>
#include <initializer_list>
#include <memory>

#include "ai/index/qgsaicloudindexclient.h"
#include "ai/index/qgsaiembeddingprovider.h"
#include "ai/index/qgsailayerindexcoordinator.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "qgsaiaccountwidget.h"
#include "qgsaiagentsessionmanager.h"
#include "qgsaichatdockwidget.h"
#include "qgsaiclaudeoauthclient.h"
#include "qgsaigissuggestionengine.h"
#include "qgsaimodelrouter.h"
#include "qgsaiopenroutermodelcatalog.h"
#include "qgsairulesskillscloudclient.h"
#include "qgsairulesskillsstore.h"
#include "qgsaisecretstore.h"
#include "qgsaisettingsutils.h"
#include "qgsaiworkspacetrust.h"
#include "qgsapplication.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsscrollarea.h"
#include "qgssettings.h"
#include "qgstaskmanager.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QPushButton>
#include <QScreen>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QString>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include "moc_qgsaisettingsdialog.cpp"

using namespace Qt::StringLiterals;

using QgsAiSettingsUtils::humanBytes;
using QgsAiSettingsUtils::sectionHeader;
using QgsAiSettingsUtils::settingRow;
using QgsAiSettingsUtils::settingRowFullWidth;
using QgsAiSettingsUtils::settingValueWithLegacy;

namespace
{
  class ManualWorkspaceIndexTask final : public QgsTask
  {
    public:
      ManualWorkspaceIndexTask( QgsAiWorkspaceIndex *index, const QString &workspaceRoot, const QList<QgsAiWorkspaceIndex::WorkspaceFileSnapshot> &snapshot )
        : QgsTask( QObject::tr( "Rebuild AI workspace index" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
        , mIndex( index )
        , mWorkspaceRoot( workspaceRoot )
        , mSnapshot( snapshot )
      {}

      QString errorMessage() const { return mErrorMessage; }

    protected:
      bool run() override
      {
        if ( !mIndex )
        {
          mErrorMessage = QObject::tr( "Workspace index is unavailable." );
          return false;
        }

        const QMetaObject::Connection conn = connect(
          mIndex.data(),
          &QgsAiWorkspaceIndex::progress,
          this,
          [this]( int current, int total, const QString & ) {
            if ( total > 0 )
              setProgress( 100.0 * static_cast<double>( current ) / static_cast<double>( total ) );
          },
          Qt::DirectConnection
        );

        QString error;
        const bool ok = mIndex->reindex( mSnapshot, mWorkspaceRoot, &error );
        disconnect( conn );
        mIndex->closeDatabaseConnectionForCurrentThread();
        if ( !ok )
          mErrorMessage = error;
        return ok && !isCanceled();
      }

    private:
      QPointer<QgsAiWorkspaceIndex> mIndex;
      QString mWorkspaceRoot;
      QList<QgsAiWorkspaceIndex::WorkspaceFileSnapshot> mSnapshot;
      QString mErrorMessage;
  };

  class ManualLayerIndexTask final : public QgsTask
  {
    public:
      ManualLayerIndexTask( QgsAiWorkspaceIndex *index, const QgsAiWorkspaceIndex::WorkspaceLayerSnapshot &snapshot )
        : QgsTask( QObject::tr( "Rebuild AI layer index" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
        , mIndex( index )
        , mSnapshot( snapshot )
      {}

      QString errorMessage() const { return mErrorMessage; }

    protected:
      bool run() override
      {
        if ( !mIndex )
        {
          mErrorMessage = QObject::tr( "Workspace index is unavailable." );
          return false;
        }

        QString error;
        const bool ok = mIndex->reindexLayerSnapshot( mSnapshot, &error );
        mIndex->closeDatabaseConnectionForCurrentThread();
        if ( !ok )
          mErrorMessage = error;
        return ok && !isCanceled();
      }

    private:
      QPointer<QgsAiWorkspaceIndex> mIndex;
      QgsAiWorkspaceIndex::WorkspaceLayerSnapshot mSnapshot;
      QString mErrorMessage;
  };

  bool downloadEmbeddingModelFile( const QgsAiEmbeddingModelDownloadFile &file, const QString &baseDir, QProgressDialog &progress, qint64 &completedBytes, QString *errorMessage )
  {
    const QString destPath = QDir( baseDir ).filePath( file.relativePath );
    if ( QFileInfo::exists( destPath ) )
    {
      QString hashError;
      if ( QgsAiE5EmbeddingProvider::fileMatchesSha256( destPath, file.sha256, &hashError ) )
      {
        completedBytes += file.size;
        progress.setValue( static_cast<int>( std::min<qint64>( completedBytes, progress.maximum() ) ) );
        return true;
      }
    }

    QFileInfo destInfo( destPath );
    if ( !destInfo.dir().exists() && !destInfo.dir().mkpath( u"."_s ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Cannot create model directory: %1" ).arg( destInfo.dir().absolutePath() );
      return false;
    }

    QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
    if ( !nam )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Network manager is not available." );
      return false;
    }

    const QString partPath = destPath + u".part"_s;
    QFile::remove( partPath );
    QFile outFile( partPath );
    if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Cannot write model file: %1" ).arg( partPath );
      return false;
    }

    QNetworkRequest request( QUrl( file.url ) );
    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );
    request.setTransferTimeout( 30000 );

    QNetworkReply *reply = nam->get( request );
    if ( !reply )
    {
      outFile.close();
      QFile::remove( partPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Unable to start model download." );
      return false;
    }

    QCryptographicHash hash( QCryptographicHash::Sha256 );
    qint64 fileBytes = 0;
    auto drainReply = [&]() {
      const QByteArray chunk = reply->readAll();
      if ( chunk.isEmpty() )
        return;
      outFile.write( chunk );
      hash.addData( chunk );
      fileBytes += chunk.size();
      progress.setValue( static_cast<int>( std::min<qint64>( completedBytes + fileBytes, progress.maximum() ) ) );
      QApplication::processEvents();
    };

    QObject::connect( reply, &QNetworkReply::readyRead, reply, drainReply );
    QObject::connect( &progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort );

    QEventLoop loop;
    QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
    loop.exec();
    drainReply();

    const bool wasCanceled = progress.wasCanceled();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorString = reply->errorString();
    reply->deleteLater();

    outFile.close();
    if ( wasCanceled )
    {
      QFile::remove( partPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Model download canceled." );
      return false;
    }

    if ( networkError != QNetworkReply::NoError )
    {
      QFile::remove( partPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Model download failed: %1" ).arg( networkErrorString );
      return false;
    }

    if ( fileBytes != file.size )
    {
      QFile::remove( partPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Model download size mismatch for %1: expected %2, got %3." ).arg( file.relativePath ).arg( file.size ).arg( fileBytes );
      return false;
    }

    const QString actualSha = QString::fromLatin1( hash.result().toHex() );
    if ( actualSha.compare( file.sha256, Qt::CaseInsensitive ) != 0 )
    {
      QFile::remove( partPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Model download hash mismatch for %1." ).arg( file.relativePath );
      return false;
    }

    QFile::remove( destPath );
    if ( !QFile::rename( partPath, destPath ) )
    {
      QFile::remove( partPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Cannot move verified model file into place: %1" ).arg( destPath );
      return false;
    }

    completedBytes += file.size;
    progress.setValue( static_cast<int>( std::min<qint64>( completedBytes, progress.maximum() ) ) );
    return true;
  }

  bool downloadEmbeddingModelWithConsent( QWidget *parent, QString *errorMessage )
  {
    const QString destination = QgsAiE5EmbeddingProvider::userModelDirectory();
    const QString question = QObject::tr(
                               "Strata will download the local multilingual E5 embedding model from Hugging Face.\n\n"
                               "Source: https://huggingface.co/intfloat/multilingual-e5-small\n"
                               "License: MIT\n"
                               "Revision: 614241f622f53c4eeff9890bdc4f31cfecc418b3\n"
                               "Size: %1\n"
                               "Destination: %2\n\n"
                               "The model is used locally for workspace indexing and is not bundled with this release.\n\n"
                               "Download now?"
    )
                               .arg( humanBytes( QgsAiE5EmbeddingProvider::downloadSize() ), destination );

    if ( QMessageBox::question( parent, QObject::tr( "Download local embedding model" ), question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Model download was not approved." );
      return false;
    }

    QProgressDialog progress( QObject::tr( "Downloading local embedding model..." ), QObject::tr( "Cancel" ), 0, static_cast<int>( QgsAiE5EmbeddingProvider::downloadSize() ), parent );
    progress.setWindowModality( Qt::WindowModal );
    progress.setMinimumDuration( 0 );
    progress.setValue( 0 );

    qint64 completedBytes = 0;
    for ( const QgsAiEmbeddingModelDownloadFile &file : QgsAiE5EmbeddingProvider::downloadFiles() )
    {
      progress.setLabelText( QObject::tr( "Downloading %1" ).arg( file.relativePath ) );
      QString fileError;
      if ( !downloadEmbeddingModelFile( file, destination, progress, completedBytes, &fileError ) )
      {
        if ( errorMessage )
          *errorMessage = fileError;
        return false;
      }
    }

    QJsonArray files;
    for ( const QgsAiEmbeddingModelDownloadFile &file : QgsAiE5EmbeddingProvider::downloadFiles() )
    {
      QJsonObject item;
      item.insert( u"relative_path"_s, file.relativePath );
      item.insert( u"sha256"_s, file.sha256 );
      item.insert( u"size"_s, static_cast<double>( file.size ) );
      files.append( item );
    }

    QJsonObject manifest;
    manifest.insert( u"model_id"_s, QgsAiE5EmbeddingProvider::modelName() );
    manifest.insert( u"model_revision"_s, QgsAiE5EmbeddingProvider::pinnedModelRevision() );
    manifest.insert( u"license"_s, u"MIT"_s );
    manifest.insert( u"source"_s, u"https://huggingface.co/intfloat/multilingual-e5-small"_s );
    manifest.insert( u"source_revision"_s, u"614241f622f53c4eeff9890bdc4f31cfecc418b3"_s );
    manifest.insert( u"files"_s, files );

    const QString manifestPath = QDir( destination ).filePath( u"manifest.json"_s );
    const QString manifestPartPath = manifestPath + u".part"_s;
    QFile::remove( manifestPartPath );
    QFile manifestFile( manifestPartPath );
    if ( !manifestFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Cannot write model manifest: %1" ).arg( manifestPartPath );
      return false;
    }
    const QByteArray manifestJson = QJsonDocument( manifest ).toJson( QJsonDocument::Indented );
    if ( manifestFile.write( manifestJson ) != manifestJson.size() )
    {
      manifestFile.close();
      QFile::remove( manifestPartPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Cannot write complete model manifest: %1" ).arg( manifestPartPath );
      return false;
    }
    manifestFile.close();
    QFile::remove( manifestPath );
    if ( !QFile::rename( manifestPartPath, manifestPath ) )
    {
      QFile::remove( manifestPartPath );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Cannot move verified model manifest into place: %1" ).arg( manifestPath );
      return false;
    }

    progress.setValue( progress.maximum() );
    return true;
  }

  QString remoteEmbeddingModelSettingKey( const QString &providerId )
  {
    if ( providerId.compare( u"strata-cloud"_s, Qt::CaseInsensitive ) == 0 )
      return u"ai/embeddings/strata-cloud/model"_s;
    return providerId.compare( u"openrouter"_s, Qt::CaseInsensitive ) == 0 ? u"ai/embeddings/openrouter/model"_s : u"ai/embeddings/openai/model"_s;
  }

  QString remoteEmbeddingModelDefault( const QString &providerId )
  {
    if ( providerId.compare( u"strata-cloud"_s, Qt::CaseInsensitive ) == 0 )
      return u"strata-embedding-384"_s;
    return providerId.compare( u"openrouter"_s, Qt::CaseInsensitive ) == 0 ? u"openai/text-embedding-3-small"_s : u"text-embedding-3-small"_s;
  }

  QString gisProjectSettingsKey()
  {
    const QgsProject *project = QgsProject::instance();
    const QString projectFile = project ? project->fileName() : QString();
    return QgsAiGisSuggestionEngine::projectEnabledSettingsKey( projectFile );
  }

  bool hasByokProvider( const QgsAiModelRouter *modelRouter )
  {
    if ( !modelRouter )
      return false;
    return modelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi )
           || modelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenRouter )
           || modelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude )
           || modelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Codex )
           || modelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Claude )
           || !qEnvironmentVariable( "OPENAI_API_KEY" ).trimmed().isEmpty()
           || !qEnvironmentVariable( "OPENROUTER_API_KEY" ).trimmed().isEmpty()
           || !qEnvironmentVariable( "ANTHROPIC_API_KEY" ).trimmed().isEmpty();
  }

  bool demoProjectReady()
  {
    QgsSettings settings;
    return settings.value( u"strata/onboarding/demo_project_seen"_s, false ).toBool() || ( QgsProject::instance() && !QgsProject::instance()->mapLayers().isEmpty() );
  }
} // namespace

QgsAiSettingsDialog::QgsAiSettingsDialog( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiLayerIndexCoordinator *layerIndexCoordinator, QWidget *parent )
  : QDialog( parent )
  , mSessionManager( sessionManager )
  , mModelRouter( modelRouter )
  , mLayerIndexCoordinator( layerIndexCoordinator )
{
  setWindowTitle( tr( "AI Settings" ) );
  setObjectName( u"aiSettingsDialog"_s );

  QVBoxLayout *rootLayout = new QVBoxLayout( this );
  rootLayout->setContentsMargins( 0, 0, 0, 0 );
  rootLayout->setSpacing( 0 );

  QWidget *body = new QWidget( this );
  QHBoxLayout *bodyLayout = new QHBoxLayout( body );
  bodyLayout->setContentsMargins( 0, 0, 0, 0 );
  bodyLayout->setSpacing( 0 );

  // Sidebar: mini account header on top, flat section list below.
  QWidget *sidebarPane = new QWidget( body );
  sidebarPane->setObjectName( u"aiSettingsSidebarPane"_s );
  sidebarPane->setFixedWidth( 200 );
  QVBoxLayout *sidebarLayout = new QVBoxLayout( sidebarPane );
  sidebarLayout->setContentsMargins( 0, 12, 0, 12 );
  sidebarLayout->setSpacing( 8 );

  mSidebarHeader = new QWidget( sidebarPane );
  mSidebarHeader->setCursor( Qt::PointingHandCursor );
  mSidebarHeader->setToolTip( tr( "Open the Account section" ) );
  QHBoxLayout *headerLayout = new QHBoxLayout( mSidebarHeader );
  headerLayout->setContentsMargins( 14, 0, 8, 0 );
  headerLayout->setSpacing( 8 );
  mSidebarAvatar = new QLabel( mSidebarHeader );
  mSidebarAvatar->setObjectName( u"aiSettingsSidebarAvatar"_s );
  mSidebarAvatar->setFixedSize( 28, 28 );
  mSidebarAvatar->setAlignment( Qt::AlignCenter );
  mSidebarEmailLabel = new QLabel( mSidebarHeader );
  headerLayout->addWidget( mSidebarAvatar );
  headerLayout->addWidget( mSidebarEmailLabel, 1 );
  mSidebarHeader->installEventFilter( this );
  sidebarLayout->addWidget( mSidebarHeader );

  mSidebarList = new QListWidget( sidebarPane );
  mSidebarList->setObjectName( u"aiSettingsSidebar"_s );
  mSidebarList->setFrameShape( QFrame::NoFrame );
  mSidebarList->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  sidebarLayout->addWidget( mSidebarList, 1 );

  // Content: stacked pages inside a single scroll area.
  QgsScrollArea *scrollArea = new QgsScrollArea( body );
  scrollArea->setWidgetResizable( true );
  scrollArea->setFrameShape( QFrame::NoFrame );
  mStack = new QStackedWidget( scrollArea );
  mStack->setObjectName( u"aiSettingsStack"_s );
  scrollArea->setWidget( mStack );

  bodyLayout->addWidget( sidebarPane );
  bodyLayout->addWidget( scrollArea, 1 );
  rootLayout->addWidget( body, 1 );

  QFrame *buttonSeparator = new QFrame( this );
  buttonSeparator->setFrameShape( QFrame::HLine );
  buttonSeparator->setFrameShadow( QFrame::Sunken );
  rootLayout->addWidget( buttonSeparator );

  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
  connect( buttons, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );
  // Enter inside the auth (and other) line edits must submit the form action,
  // not fire the dialog-default OK button and close the whole dialog.
  const QList<QAbstractButton *> boxButtons = buttons->buttons();
  for ( QAbstractButton *boxButton : boxButtons )
  {
    if ( QPushButton *pushButton = qobject_cast<QPushButton *>( boxButton ) )
    {
      pushButton->setAutoDefault( false );
      pushButton->setDefault( false );
    }
  }
  QWidget *buttonRow = new QWidget( this );
  QHBoxLayout *buttonRowLayout = new QHBoxLayout( buttonRow );
  buttonRowLayout->setContentsMargins( 12, 8, 12, 8 );
  buttonRowLayout->addWidget( buttons );
  rootLayout->addWidget( buttonRow );

  // Build order matters: later pages capture widgets created by earlier ones
  // (cloud sync reads the Account endpoint, onboarding reads indexing/privacy).
  addSection( u"account"_s, tr( "Account" ), buildAccountPage() );
  addSection( u"providers"_s, tr( "Models & Providers" ), buildProvidersPage() );
  addSection( u"agent"_s, tr( "Agent" ), buildAgentPage() );
  addSection( u"rules"_s, tr( "Rules & Skills" ), buildRulesSkillsPage() );
  addSection( u"indexing"_s, tr( "Indexing & Docs" ), buildIndexingPage() );
  addSection( u"workspace"_s, tr( "Workspace" ), buildWorkspacePage() );
  addSection( u"privacy"_s, tr( "Privacy & Telemetry" ), buildPrivacyPage() );
  addSection( u"onboarding"_s, tr( "Onboarding & Release" ), buildOnboardingPage() );

  connect( mSidebarList, &QListWidget::currentRowChanged, mStack, &QStackedWidget::setCurrentIndex );
  mSidebarList->setCurrentRow( 0 );

  connect( mAccountWidget, &QgsAiAccountWidget::accountInfoChanged, this, &QgsAiSettingsDialog::refreshSidebarAccountHeader );
  connect( mAccountWidget, &QgsAiAccountWidget::authStateChanged, this, [this]() {
    refreshSidebarAccountHeader();
    refreshOnboardingStatus();
    mSyncCloudContextButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() && mModelRouter && !mModelRouter->planSessionToken().trimmed().isEmpty() );
    emit planAuthStateChanged();
  } );
  // Model enable/disable toggles should rebuild the chat model menu the same way an auth change does.
  connect( mAccountWidget, &QgsAiAccountWidget::modelPreferencesChanged, this, &QgsAiSettingsDialog::planAuthStateChanged );
  refreshSidebarAccountHeader();

  setStyleSheet( uR"css(
QWidget#aiSettingsSidebarPane { background: palette(alternate-base); border-right: 1px solid palette(mid); }
QListWidget#aiSettingsSidebar { background: transparent; border: none; outline: none; }
QListWidget#aiSettingsSidebar::item { padding: 7px 12px; border-radius: 6px; margin: 1px 8px; color: palette(window-text); }
QListWidget#aiSettingsSidebar::item:hover { background: palette(midlight); }
QListWidget#aiSettingsSidebar::item:selected { background: palette(midlight); color: palette(window-text); }
QLabel#aiSettingsSidebarAvatar { background: palette(highlight); color: palette(highlighted-text); border-radius: 14px; font-weight: 600; }
QLabel[aiRole="pageTitle"] { font-weight: 700; }
QLabel[aiRole="sectionHeader"] { font-weight: 600; margin-top: 8px; }
QLabel[aiRole="rowDescription"] { color: palette(dark); }
)css"_s );

  setMinimumSize( 720, 480 );
  if ( const QScreen *screen = QApplication::primaryScreen() )
  {
    const QSize availableSize = screen->availableGeometry().size();
    const int dialogWidth = std::min( 920, std::max( 720, availableSize.width() - 100 ) );
    const int dialogHeight = std::min( 640, std::max( 480, availableSize.height() - 120 ) );
    resize( dialogWidth, dialogHeight );
  }
  else
  {
    resize( 880, 620 );
  }
}

void QgsAiSettingsDialog::accept()
{
  applySettings();
  QDialog::accept();
}

void QgsAiSettingsDialog::showSection( const QString &key )
{
  for ( int i = 0; i < mSidebarList->count(); ++i )
  {
    if ( mSidebarList->item( i )->data( Qt::UserRole ).toString() == key )
    {
      mSidebarList->setCurrentRow( i );
      return;
    }
  }
}

bool QgsAiSettingsDialog::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mSidebarHeader && event->type() == QEvent::MouseButtonRelease )
  {
    showSection( u"account"_s );
    return true;
  }
  return QDialog::eventFilter( watched, event );
}

QWidget *QgsAiSettingsDialog::createPage( const QString &title, const QString &subtitle, QVBoxLayout *&contentLayout )
{
  QWidget *page = new QWidget();
  QVBoxLayout *outer = new QVBoxLayout( page );
  outer->setContentsMargins( 24, 20, 24, 24 );
  outer->setSpacing( 4 );

  QLabel *titleLabel = new QLabel( title, page );
  titleLabel->setProperty( "aiRole", u"pageTitle"_s );
  QFont titleFont = titleLabel->font();
  titleFont.setBold( true );
  titleFont.setPointSize( titleFont.pointSize() + 3 );
  titleLabel->setFont( titleFont );
  outer->addWidget( titleLabel );

  if ( !subtitle.isEmpty() )
  {
    QLabel *subtitleLabel = new QLabel( subtitle, page );
    subtitleLabel->setProperty( "aiRole", u"rowDescription"_s );
    subtitleLabel->setWordWrap( true );
    outer->addWidget( subtitleLabel );
  }

  QWidget *content = new QWidget( page );
  content->setMaximumWidth( 680 );
  contentLayout = new QVBoxLayout( content );
  contentLayout->setContentsMargins( 0, 12, 0, 0 );
  contentLayout->setSpacing( 8 );
  outer->addWidget( content );
  outer->addStretch( 1 );
  return page;
}

void QgsAiSettingsDialog::addSection( const QString &key, const QString &label, QWidget *page )
{
  QListWidgetItem *item = new QListWidgetItem( label, mSidebarList );
  item->setData( Qt::UserRole, key );
  mStack->addWidget( page );
}

QWidget *QgsAiSettingsDialog::buildAccountPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Account" ), tr( "Sign in to Strata Cloud to use managed models and the cloud agent." ), contentLayout );
  mAccountWidget = new QgsAiAccountWidget( mModelRouter, mSessionManager, page );
  contentLayout->addWidget( mAccountWidget );
  return page;
}

QWidget *QgsAiSettingsDialog::buildProvidersPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Models & Providers" ), tr( "Bring-your-own-key providers used when the Plan account is not active." ), contentLayout );

  // ---- OpenAI ----
  contentLayout->addWidget( sectionHeader( tr( "OpenAI" ), page ) );
  mOpenAiEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi ).endpoint, page );
  mOpenAiModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi ).model, page );
  mOpenAiKey = new QLineEdit( page );
  mOpenAiKey->setEchoMode( QLineEdit::Password );
  mOpenAiKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "sk-..." ) );
  contentLayout->addWidget( settingRow( tr( "Model" ), QString(), mOpenAiModel, page ) );
  contentLayout->addWidget( settingRow( tr( "API key" ), tr( "Stored locally. Leave empty to keep the saved key." ), mOpenAiKey, page ) );

  // ---- OpenRouter ----
  contentLayout->addWidget( sectionHeader( tr( "OpenRouter" ), page ) );
  mOpenRouterEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter ).endpoint, page );

  // Editable, searchable model picker fed by the async OpenRouter catalog
  // (tool-capable models with context size and pricing). Free text stays valid:
  // the current text is what gets persisted.
  mOpenRouterModel = new QComboBox( page );
  mOpenRouterModel->setObjectName( u"aiOpenRouterModelComboBox"_s );
  mOpenRouterModel->setEditable( true );
  mOpenRouterModel->setInsertPolicy( QComboBox::NoInsert );
  mOpenRouterModel->lineEdit()->setPlaceholderText( u"anthropic/claude-sonnet-4.6"_s );
  const QString configuredOpenRouterModel = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter ).model;
  mOpenRouterModel->setEditText( configuredOpenRouterModel );

  QgsAiOpenRouterModelCatalog *openRouterCatalog = new QgsAiOpenRouterModelCatalog( this );
  connect( openRouterCatalog, &QgsAiOpenRouterModelCatalog::modelsReady, this, [this]( const QList<QgsAiOpenRouterModelCatalog::ModelInfo> &models, bool ) {
    const QString currentText = mOpenRouterModel->currentText();
    mOpenRouterModel->clear();
    for ( const QgsAiOpenRouterModelCatalog::ModelInfo &model : models )
      mOpenRouterModel->addItem( model.displayLabel(), model.id );
    QCompleter *completer = new QCompleter( mOpenRouterModel->model(), mOpenRouterModel );
    completer->setFilterMode( Qt::MatchContains );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mOpenRouterModel->setCompleter( completer );
    mOpenRouterModel->setEditText( currentText );
  } );
  // Selecting a catalog entry replaces the display label with the model id.
  connect( mOpenRouterModel, qOverload<int>( &QComboBox::activated ), this, [this]( int index ) {
    const QString modelId = mOpenRouterModel->itemData( index ).toString();
    if ( !modelId.isEmpty() )
      mOpenRouterModel->setEditText( modelId );
  } );
  openRouterCatalog->refresh();

  mOpenRouterKey = new QLineEdit( page );
  mOpenRouterKey->setEchoMode( QLineEdit::Password );
  mOpenRouterKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenRouter ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "sk-or-..." ) );

  // Connection test: validates the pending (or stored) key against GET /key and
  // shows the credits summary inline.
  QPushButton *openRouterTestButton = new QPushButton( tr( "Test connection" ), page );
  openRouterTestButton->setObjectName( u"aiOpenRouterTestConnectionButton"_s );
  QLabel *openRouterTestResult = new QLabel( page );
  openRouterTestResult->setObjectName( u"aiOpenRouterTestConnectionLabel"_s );
  openRouterTestResult->setWordWrap( true );
  connect( openRouterCatalog, &QgsAiOpenRouterModelCatalog::keyInfoReady, this, [openRouterTestButton, openRouterTestResult]( const QString &summary ) {
    openRouterTestButton->setEnabled( true );
    openRouterTestResult->setText( summary );
  } );
  connect( openRouterCatalog, &QgsAiOpenRouterModelCatalog::keyInfoFailed, this, [openRouterTestButton, openRouterTestResult]( const QString &errorMessage ) {
    openRouterTestButton->setEnabled( true );
    openRouterTestResult->setText( errorMessage );
  } );
  connect( openRouterTestButton, &QPushButton::clicked, this, [this, openRouterCatalog, openRouterTestButton, openRouterTestResult]() {
    QString key = mOpenRouterKey->text().trimmed();
    if ( key.isEmpty() )
      key = QgsAiSecretStore::readSecret( u"ai/provider/openrouter/apiKey"_s, { u"OPENROUTER_API_KEY"_s } );
    openRouterTestButton->setEnabled( false );
    openRouterTestResult->setText( tr( "Testing…" ) );
    openRouterCatalog->fetchKeyInfo( key );
  } );

  mOpenRouterAutoRouting = new QCheckBox( page );
  mOpenRouterAutoRouting->setChecked( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter ).autoRouting );

  contentLayout->addWidget( settingRow( tr( "Model" ), QString(), mOpenRouterModel, page ) );
  contentLayout->addWidget( settingRow( tr( "API key" ), tr( "Stored locally. Leave empty to keep the saved key." ), mOpenRouterKey, page ) );
  contentLayout->addWidget(
    settingRow( tr( "Automatic routing" ), tr( "Adds OpenRouter provider routing preferences (tool-capable providers, price/throughput sorting) and falls back to the Auto router if the pinned model is unavailable." ), mOpenRouterAutoRouting, page )
  );
  contentLayout->addWidget( settingRow( tr( "Connection" ), QString(), openRouterTestButton, page ) );
  contentLayout->addWidget( openRouterTestResult );

  // ---- Codex ----
  contentLayout->addWidget( sectionHeader( tr( "Codex" ), page ) );
  mCodexEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex ).endpoint, page );
  mCodexModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex ).model, page );
  mCodexModel->setPlaceholderText( u"gpt-5.4"_s );
  mCodexStatus = new QLabel( mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Codex ) ? tr( "Signed in" ) : tr( "Not signed in" ), page );
  QPushButton *codexRequestCodeButton = new QPushButton( tr( "Get device code" ), page );
  QPushButton *codexCompleteLoginButton = new QPushButton( tr( "Complete login" ), page );
  QPushButton *codexLogoutButton = new QPushButton( tr( "Log out" ), page );
  QWidget *codexButtons = new QWidget( page );
  QHBoxLayout *codexButtonsLayout = new QHBoxLayout( codexButtons );
  codexButtonsLayout->setContentsMargins( 0, 0, 0, 0 );
  codexButtonsLayout->addWidget( codexRequestCodeButton );
  codexButtonsLayout->addWidget( codexCompleteLoginButton );
  codexButtonsLayout->addWidget( codexLogoutButton );

  contentLayout->addWidget( settingRow( tr( "Model" ), QString(), mCodexModel, page ) );
  contentLayout->addWidget( settingRow( tr( "Status" ), tr( "Codex uses a device-code OAuth login." ), mCodexStatus, page ) );
  contentLayout->addWidget( settingRow( tr( "Account" ), QString(), codexButtons, page ) );

  connect( codexRequestCodeButton, &QPushButton::clicked, this, [this]() {
    QString error;
    if ( !QgsAiCodexOAuthClient::requestDeviceCode( mCodexDeviceCode, &error ) )
    {
      QMessageBox::warning( this, tr( "Codex login failed" ), error );
      return;
    }

    mCodexStatus->setText( tr( "Open %1 and enter code %2" ).arg( mCodexDeviceCode.verificationUrl, mCodexDeviceCode.userCode ) );
    QDesktopServices::openUrl( QUrl( mCodexDeviceCode.verificationUrl ) );
    QMessageBox::information( this, tr( "Codex device code" ), tr( "Open %1 and enter this code:\n\n%2\n\nThen click Complete login." ).arg( mCodexDeviceCode.verificationUrl, mCodexDeviceCode.userCode ) );
  } );

  connect( codexCompleteLoginButton, &QPushButton::clicked, this, [this]() {
    if ( mCodexDeviceCode.deviceAuthId.isEmpty() )
    {
      QMessageBox::information( this, tr( "Codex login" ), tr( "Request a Codex device code first." ) );
      return;
    }

    QString error;
    if ( !QgsAiCodexOAuthClient::completeDeviceCodeLogin( mCodexDeviceCode, &error ) )
    {
      QMessageBox::warning( this, tr( "Codex login failed" ), error );
      return;
    }
    mCodexStatus->setText( tr( "Signed in" ) );
  } );

  connect( codexLogoutButton, &QPushButton::clicked, this, [this]() {
    QString error;
    if ( !QgsAiCodexOAuthClient::clearRefreshToken( &error ) )
    {
      QMessageBox::warning( this, tr( "Codex logout failed" ), error );
      return;
    }
    mCodexStatus->setText( tr( "Not signed in" ) );
  } );

  // ---- Claude ----
  contentLayout->addWidget( sectionHeader( tr( "Claude" ), page ) );
  mClaudeEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).endpoint, page );
  mClaudeModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).model, page );
  mClaudeKey = new QLineEdit( page );
  mClaudeKey->setEchoMode( QLineEdit::Password );
  mClaudeKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "anthropic key..." ) );
  mClaudeUseOAuth = new QCheckBox( page );
  mClaudeUseOAuth->setChecked( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).credentialMode == QgsAiModelRouter::CredentialMode::OAuth );
  mClaudeOAuthStatus = new QLabel( mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Claude ) ? tr( "Signed in" ) : tr( "Not signed in" ), page );
  QPushButton *claudeLoginButton = new QPushButton( tr( "Login with Claude" ), page );
  QPushButton *claudeLogoutButton = new QPushButton( tr( "Log out" ), page );
  QWidget *claudeOAuthButtons = new QWidget( page );
  QHBoxLayout *claudeOAuthButtonsLayout = new QHBoxLayout( claudeOAuthButtons );
  claudeOAuthButtonsLayout->setContentsMargins( 0, 0, 0, 0 );
  claudeOAuthButtonsLayout->addWidget( claudeLoginButton );
  claudeOAuthButtonsLayout->addWidget( claudeLogoutButton );

  contentLayout->addWidget( settingRow( tr( "Model" ), QString(), mClaudeModel, page ) );
  contentLayout->addWidget( settingRow( tr( "API key" ), tr( "Stored locally. Leave empty to keep the saved key." ), mClaudeKey, page ) );
  contentLayout->addWidget( settingRow( tr( "Use OAuth login" ), tr( "Use your Claude subscription via OAuth instead of an API key." ), mClaudeUseOAuth, page ) );
  contentLayout->addWidget( settingRow( tr( "OAuth status" ), QString(), mClaudeOAuthStatus, page ) );
  contentLayout->addWidget( settingRow( tr( "Account" ), QString(), claudeOAuthButtons, page ) );

  connect( claudeLoginButton, &QPushButton::clicked, this, [this]() {
    const QgsAiClaudeOAuthClient::AuthorizationRequest authRequest = QgsAiClaudeOAuthClient::buildAuthorizationRequest();

    if ( !QDesktopServices::openUrl( authRequest.authorizationUrl ) )
    {
      QMessageBox::information( this, tr( "Claude OAuth" ), tr( "Open this URL in your browser:\n\n%1" ).arg( authRequest.authorizationUrl.toString() ) );
    }

    bool ok = false;
    const QString code
      = QInputDialog::getText( this, tr( "Claude OAuth" ), tr( "After approving Claude in the browser, paste the authorization code or callback URL:" ), QLineEdit::Normal, QString(), &ok ).trimmed();
    if ( !ok || code.isEmpty() )
      return;

    QString error;
    if ( !QgsAiClaudeOAuthClient::exchangeAuthorizationCode( code, authRequest.codeVerifier, authRequest.redirectUri, authRequest.state, &error ) )
    {
      QMessageBox::warning( this, tr( "Claude login failed" ), error );
      return;
    }
    mClaudeUseOAuth->setChecked( true );
    mClaudeOAuthStatus->setText( tr( "Signed in" ) );
  } );

  connect( claudeLogoutButton, &QPushButton::clicked, this, [this]() {
    QString error;
    if ( !QgsAiClaudeOAuthClient::clearRefreshToken( &error ) )
    {
      QMessageBox::warning( this, tr( "Claude logout failed" ), error );
      return;
    }
    mClaudeOAuthStatus->setText( tr( "Not signed in" ) );
  } );

  // ---- Advanced endpoints ----
  QgsCollapsibleGroupBox *advancedEndpoints = new QgsCollapsibleGroupBox( tr( "Advanced endpoints" ), page );
  QFormLayout *endpointsForm = new QFormLayout( advancedEndpoints );
  endpointsForm->addRow( tr( "OpenAI endpoint" ), mOpenAiEndpoint );
  endpointsForm->addRow( tr( "OpenRouter endpoint" ), mOpenRouterEndpoint );
  endpointsForm->addRow( tr( "Codex endpoint" ), mCodexEndpoint );
  endpointsForm->addRow( tr( "Claude endpoint" ), mClaudeEndpoint );
  advancedEndpoints->setCollapsed( true );
  contentLayout->addWidget( advancedEndpoints );

  return page;
}

QWidget *QgsAiSettingsDialog::buildAgentPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Agent" ), tr( "How the assistant is allowed to act on your project." ), contentLayout );

  const QgsAiAgentBehaviorSettings currentBehavior = mSessionManager ? mSessionManager->agentBehaviorSettings() : QgsAiAgentBehaviorSettings();

  mAllowCustomActions = new QCheckBox( page );
  mAllowCustomActions->setChecked( currentBehavior.allowCustomActions );
  contentLayout->addWidget(
    settingRow( tr( "Allow custom agent actions" ), tr( "When enabled, the agent can call tools like read_file, propose_edit, run_python. Destructive tools still require confirmation in their dedicated review dialogs." ), mAllowCustomActions, page )
  );

  QgsSettings gisToggleSettings;
  mGisSuggestionsEnabled = new QCheckBox( page );
  mGisSuggestionsEnabled->setObjectName( u"aiGisGlobalEnableCheckBox"_s );
  mGisSuggestionsEnabled->setChecked( gisToggleSettings.value( QgsAiGisSuggestionEngine::globalEnabledSettingsKey(), true ).toBool() );
  contentLayout->addWidget(
    settingRow( tr( "Enable GIS suggestions" ), tr( "Rule-based project health suggestions: shown as a card in the chat and injected into the model context." ), mGisSuggestionsEnabled, page )
  );

  mGisSuggestionsProjectEnabled = new QCheckBox( page );
  mGisSuggestionsProjectEnabled->setObjectName( u"aiGisProjectEnableCheckBox"_s );
  mGisSuggestionsProjectEnabled->setChecked( gisToggleSettings.value( gisProjectSettingsKey(), true ).toBool() );
  contentLayout->addWidget( settingRow( tr( "Enable GIS suggestions for this project" ), QString(), mGisSuggestionsProjectEnabled, page ) );

  return page;
}

QWidget *QgsAiSettingsDialog::buildRulesSkillsPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Rules & Skills" ), tr( "Standing instructions and reusable playbooks the agent can draw on, per workspace." ), contentLayout );

  const QgsAiAgentBehaviorSettings currentBehavior = mSessionManager ? mSessionManager->agentBehaviorSettings() : QgsAiAgentBehaviorSettings();
  mRulesRelativeDirForList = currentBehavior.rulesPath;
  mSkillsRelativeDirForList = currentBehavior.skillsPath;

  mRulesSkillsTrustBanner = new QLabel( page );
  mRulesSkillsTrustBanner->setWordWrap( true );
  mRulesSkillsTrustBanner->setProperty( "aiRole", u"rowDescription"_s );
  contentLayout->addWidget( mRulesSkillsTrustBanner );

  // ---- Rules ----
  contentLayout->addWidget( sectionHeader( tr( "Rules" ), page ) );
  QLabel *rulesHint
    = new QLabel( tr( "Rules marked \u201cAlways\u201d are injected in full on every turn. Others are listed by name/description only \u2014 the agent reads the full content when it decides it is relevant." ), page );
  rulesHint->setWordWrap( true );
  rulesHint->setProperty( "aiRole", u"rowDescription"_s );
  contentLayout->addWidget( rulesHint );

  mRulesListWidget = new QListWidget( page );
  mRulesListWidget->setFixedHeight( 110 );
  contentLayout->addWidget( mRulesListWidget );

  mRuleNewButton = new QPushButton( tr( "+ New rule" ), page );
  contentLayout->addWidget( mRuleNewButton );

  mRuleEditorWidget = new QWidget( page );
  QVBoxLayout *ruleEditorLayout = new QVBoxLayout( mRuleEditorWidget );
  ruleEditorLayout->setContentsMargins( 0, 4, 0, 4 );

  mRuleNameEdit = new QLineEdit( mRuleEditorWidget );
  ruleEditorLayout->addWidget( settingRow( tr( "Name" ), QString(), mRuleNameEdit, mRuleEditorWidget ) );

  mRuleDescriptionEdit = new QLineEdit( mRuleEditorWidget );
  ruleEditorLayout->addWidget( settingRow( tr( "Description" ), QString(), mRuleDescriptionEdit, mRuleEditorWidget ) );

  mRuleGlobsEdit = new QLineEdit( mRuleEditorWidget );
  mRuleGlobsEdit->setPlaceholderText( u"*.qgz, *.gpkg"_s );
  ruleEditorLayout->addWidget( settingRow( tr( "Globs (optional, comma-separated)" ), QString(), mRuleGlobsEdit, mRuleEditorWidget ) );

  mRuleAlwaysApply = new QCheckBox( mRuleEditorWidget );
  mRuleAlwaysApply->setChecked( true );
  ruleEditorLayout->addWidget( settingRow( tr( "Always apply" ), tr( "When off, only the name/description are sent; the agent loads the full rule on demand." ), mRuleAlwaysApply, mRuleEditorWidget ) );

  mRuleEnabled = new QCheckBox( mRuleEditorWidget );
  mRuleEnabled->setChecked( true );
  ruleEditorLayout->addWidget( settingRow( tr( "Enabled" ), QString(), mRuleEnabled, mRuleEditorWidget ) );

  mRuleBodyEdit = new QTextEdit( mRuleEditorWidget );
  mRuleBodyEdit->setAcceptRichText( false );
  mRuleBodyEdit->setFixedHeight( 130 );
  ruleEditorLayout->addWidget( settingRowFullWidth( tr( "Content" ), QString(), mRuleBodyEdit, mRuleEditorWidget ) );

  QHBoxLayout *ruleButtonsLayout = new QHBoxLayout();
  mRuleSaveButton = new QPushButton( tr( "Save" ), mRuleEditorWidget );
  mRuleDuplicateButton = new QPushButton( tr( "Duplicate" ), mRuleEditorWidget );
  mRuleDeleteButton = new QPushButton( tr( "Delete" ), mRuleEditorWidget );
  ruleButtonsLayout->addWidget( mRuleSaveButton );
  ruleButtonsLayout->addWidget( mRuleDuplicateButton );
  ruleButtonsLayout->addWidget( mRuleDeleteButton );
  ruleButtonsLayout->addStretch( 1 );
  ruleEditorLayout->addLayout( ruleButtonsLayout );

  contentLayout->addWidget( mRuleEditorWidget );

  connect( mRulesListWidget, &QListWidget::currentItemChanged, this, [this]( QListWidgetItem *current, QListWidgetItem * ) {
    if ( current )
      selectRuleInEditor( current->data( Qt::UserRole ).value<QgsAiRuleInfo>() );
  } );
  connect( mRuleNewButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::newRule );
  connect( mRuleDuplicateButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::duplicateSelectedRule );
  connect( mRuleDeleteButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::deleteSelectedRule );
  connect( mRuleSaveButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::saveCurrentRule );

  // ---- Skills ----
  contentLayout->addWidget( sectionHeader( tr( "Skills" ), page ) );
  QLabel *skillsHint = new QLabel( tr( "Skills are always listed by name/description only; the agent reads the full SKILL.md content once it decides a skill applies." ), page );
  skillsHint->setWordWrap( true );
  skillsHint->setProperty( "aiRole", u"rowDescription"_s );
  contentLayout->addWidget( skillsHint );

  mSkillsListWidget = new QListWidget( page );
  mSkillsListWidget->setFixedHeight( 110 );
  contentLayout->addWidget( mSkillsListWidget );

  mSkillNewButton = new QPushButton( tr( "+ New skill" ), page );
  contentLayout->addWidget( mSkillNewButton );

  mSkillEditorWidget = new QWidget( page );
  QVBoxLayout *skillEditorLayout = new QVBoxLayout( mSkillEditorWidget );
  skillEditorLayout->setContentsMargins( 0, 4, 0, 4 );

  mSkillNameEdit = new QLineEdit( mSkillEditorWidget );
  skillEditorLayout->addWidget( settingRow( tr( "Name" ), QString(), mSkillNameEdit, mSkillEditorWidget ) );

  mSkillDescriptionEdit = new QLineEdit( mSkillEditorWidget );
  mSkillDescriptionEdit->setPlaceholderText( tr( "When should the agent reach for this skill?" ) );
  skillEditorLayout->addWidget( settingRow( tr( "Description" ), QString(), mSkillDescriptionEdit, mSkillEditorWidget ) );

  mSkillEnabled = new QCheckBox( mSkillEditorWidget );
  mSkillEnabled->setChecked( true );
  skillEditorLayout->addWidget( settingRow( tr( "Enabled" ), QString(), mSkillEnabled, mSkillEditorWidget ) );

  mSkillBodyEdit = new QTextEdit( mSkillEditorWidget );
  mSkillBodyEdit->setAcceptRichText( false );
  mSkillBodyEdit->setFixedHeight( 130 );
  skillEditorLayout->addWidget( settingRowFullWidth( tr( "Content (SKILL.md)" ), QString(), mSkillBodyEdit, mSkillEditorWidget ) );

  QHBoxLayout *skillButtonsLayout = new QHBoxLayout();
  mSkillSaveButton = new QPushButton( tr( "Save" ), mSkillEditorWidget );
  mSkillDuplicateButton = new QPushButton( tr( "Duplicate" ), mSkillEditorWidget );
  mSkillDeleteButton = new QPushButton( tr( "Delete" ), mSkillEditorWidget );
  skillButtonsLayout->addWidget( mSkillSaveButton );
  skillButtonsLayout->addWidget( mSkillDuplicateButton );
  skillButtonsLayout->addWidget( mSkillDeleteButton );
  skillButtonsLayout->addStretch( 1 );
  skillEditorLayout->addLayout( skillButtonsLayout );

  contentLayout->addWidget( mSkillEditorWidget );

  connect( mSkillsListWidget, &QListWidget::currentItemChanged, this, [this]( QListWidgetItem *current, QListWidgetItem * ) {
    if ( current )
      selectSkillInEditor( current->data( Qt::UserRole ).value<QgsAiSkillInfo>() );
  } );
  connect( mSkillNewButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::newSkill );
  connect( mSkillDuplicateButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::duplicateSelectedSkill );
  connect( mSkillDeleteButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::deleteSelectedSkill );
  connect( mSkillSaveButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::saveCurrentSkill );

  refreshRulesList();
  refreshSkillsList();
  refreshRulesSkillsTrustState();

  mSyncRulesSkillsCloudButton = new QPushButton( tr( "Sync to Strata Cloud" ), page );
  mSyncRulesSkillsCloudButton->setObjectName( u"aiSyncRulesSkillsCloudButton"_s );
  mSyncRulesSkillsCloudButton->setEnabled( mModelRouter && !mModelRouter->planSessionToken().trimmed().isEmpty() );
  contentLayout->addWidget(
    settingRow( tr( "Cloud sync (opt-in)" ), tr( "Pushes your local rules and skills to your Strata Cloud account so other installs can pull the same set." ), mSyncRulesSkillsCloudButton, page )
  );
  mRulesSkillsCloudStatusLabel = new QLabel( page );
  mRulesSkillsCloudStatusLabel->setWordWrap( true );
  mRulesSkillsCloudStatusLabel->setProperty( "aiRole", u"rowDescription"_s );
  contentLayout->addWidget( mRulesSkillsCloudStatusLabel );
  connect( mSyncRulesSkillsCloudButton, &QPushButton::clicked, this, &QgsAiSettingsDialog::syncRulesSkillsToCloud );

  // ---- Legacy inline rules/skills (deprecated) ----
  QgsCollapsibleGroupBox *legacyGroup = new QgsCollapsibleGroupBox( tr( "Legacy inline rules/skills (deprecated)" ), page );
  QVBoxLayout *legacyLayout = new QVBoxLayout( legacyGroup );

  mRulesEdit = new QTextEdit( legacyGroup );
  mRulesEdit->setAcceptRichText( false );
  mRulesEdit->setPlaceholderText( tr( "Custom rules the agent must follow (e.g. 'Always answer in English', 'Prefer GeoPandas over osmnx', …)." ) );
  mRulesEdit->setPlainText( currentBehavior.rulesText );
  mRulesEdit->setFixedHeight( 90 );
  legacyLayout->addWidget( settingRowFullWidth( tr( "Agent rules (inline)" ), QString(), mRulesEdit, legacyGroup ) );

  mSkillsEdit = new QTextEdit( legacyGroup );
  mSkillsEdit->setAcceptRichText( false );
  mSkillsEdit->setPlaceholderText( tr( "Reusable skills/instructions the agent can leverage when relevant." ) );
  mSkillsEdit->setPlainText( currentBehavior.skillsText );
  mSkillsEdit->setFixedHeight( 90 );
  legacyLayout->addWidget( settingRowFullWidth( tr( "Agent skills (inline)" ), QString(), mSkillsEdit, legacyGroup ) );

  legacyLayout->addWidget( sectionHeader( tr( "Workspace folders" ), legacyGroup ) );

  mLoadWorkspaceRules = new QCheckBox( legacyGroup );
  mLoadWorkspaceRules->setChecked( currentBehavior.loadWorkspaceRules );
  legacyLayout->addWidget( settingRow( tr( "Load rule files from workspace" ), tr( "Feeds the Rules list above into the prompt (trusted workspaces only)." ), mLoadWorkspaceRules, legacyGroup ) );

  mRulesPathEdit = new QLineEdit( currentBehavior.rulesPath, legacyGroup );
  mRulesPathEdit->setPlaceholderText( u".strata/rules"_s );
  legacyLayout->addWidget( settingRow( tr( "Rules folder (relative)" ), QString(), mRulesPathEdit, legacyGroup ) );

  mLoadWorkspaceSkills = new QCheckBox( legacyGroup );
  mLoadWorkspaceSkills->setChecked( currentBehavior.loadWorkspaceSkills );
  legacyLayout->addWidget( settingRow( tr( "Load skill files from workspace" ), tr( "Feeds the Skills list above into the prompt (trusted workspaces only)." ), mLoadWorkspaceSkills, legacyGroup ) );

  mSkillsPathEdit = new QLineEdit( currentBehavior.skillsPath, legacyGroup );
  mSkillsPathEdit->setPlaceholderText( u".strata/skills"_s );
  legacyLayout->addWidget( settingRow( tr( "Skills folder (relative)" ), QString(), mSkillsPathEdit, legacyGroup ) );

  legacyGroup->setCollapsed( true );
  contentLayout->addWidget( legacyGroup );

  return page;
}

QgsAiRulesSkillsStore QgsAiSettingsDialog::rulesSkillsStore() const
{
  return QgsAiRulesSkillsStore( mSessionManager ? mSessionManager->fileContextProvider() : nullptr );
}

bool QgsAiSettingsDialog::rulesSkillsWritable() const
{
  if ( !mSessionManager )
    return false;
  const QString root = mSessionManager->workspaceRoot();
  return !root.isEmpty() && QgsAiWorkspaceTrust::isTrusted( root );
}

void QgsAiSettingsDialog::refreshRulesSkillsTrustState()
{
  if ( !mRulesSkillsTrustBanner )
    return;
  const bool writable = rulesSkillsWritable();
  mRulesSkillsTrustBanner->setVisible( !writable );
  if ( !writable )
  {
    mRulesSkillsTrustBanner->setText(
      mSessionManager && !mSessionManager->workspaceRoot().isEmpty() ? tr( "Trust this workspace (Workspace section) to create, edit or delete rules and skills." )
                                                                     : tr( "Configure a workspace (Workspace section) to create rules and skills." )
    );
  }
  if ( mRuleNewButton )
    mRuleNewButton->setEnabled( writable );
  if ( mSkillNewButton )
    mSkillNewButton->setEnabled( writable );
}

void QgsAiSettingsDialog::refreshRulesList()
{
  if ( !mRulesListWidget )
    return;

  mRulesListWidget->blockSignals( true );
  mRulesListWidget->clear();
  const QList<QgsAiRuleInfo> rules = rulesSkillsStore().listRules( mRulesRelativeDirForList );
  for ( const QgsAiRuleInfo &rule : rules )
  {
    QString label = rule.name;
    label += rule.alwaysApply ? tr( "  \u00b7  Always" ) : tr( "  \u00b7  Manual" );
    if ( !rule.enabled )
      label += tr( "  \u00b7  disabled" );
    QListWidgetItem *item = new QListWidgetItem( label, mRulesListWidget );
    item->setData( Qt::UserRole, QVariant::fromValue( rule ) );
  }
  mRulesListWidget->blockSignals( false );

  if ( mRulesListWidget->count() == 0 )
  {
    clearRuleEditor();
    return;
  }

  int rowToSelect = 0;
  for ( int i = 0; i < mRulesListWidget->count(); ++i )
  {
    if ( !mCurrentRuleSlug.isEmpty() && mRulesListWidget->item( i )->data( Qt::UserRole ).value<QgsAiRuleInfo>().slug == mCurrentRuleSlug )
    {
      rowToSelect = i;
      break;
    }
  }
  mRulesListWidget->setCurrentRow( rowToSelect );
  selectRuleInEditor( mRulesListWidget->item( rowToSelect )->data( Qt::UserRole ).value<QgsAiRuleInfo>() );
}

void QgsAiSettingsDialog::selectRuleInEditor( const QgsAiRuleInfo &rule )
{
  mCurrentRuleSlug = rule.slug;
  mCurrentRulePath = rule.path;
  mRuleNameEdit->setText( rule.name );
  mRuleDescriptionEdit->setText( rule.description );
  mRuleGlobsEdit->setText( rule.globs.join( ", "_L1 ) );
  mRuleAlwaysApply->setChecked( rule.alwaysApply );
  mRuleEnabled->setChecked( rule.enabled );
  mRuleBodyEdit->setPlainText( rulesSkillsStore().readRuleBody( rule ) );

  const bool writable = rulesSkillsWritable();
  for ( QWidget *widget : std::initializer_list<QWidget *> { mRuleNameEdit, mRuleDescriptionEdit, mRuleGlobsEdit, mRuleAlwaysApply, mRuleEnabled, mRuleBodyEdit } )
    widget->setEnabled( writable );
  mRuleSaveButton->setEnabled( writable );
  mRuleDuplicateButton->setEnabled( writable );
  mRuleDeleteButton->setEnabled( writable );
}

void QgsAiSettingsDialog::clearRuleEditor()
{
  mCurrentRuleSlug.clear();
  mCurrentRulePath.clear();
  mRuleNameEdit->clear();
  mRuleDescriptionEdit->clear();
  mRuleGlobsEdit->clear();
  mRuleAlwaysApply->setChecked( true );
  mRuleEnabled->setChecked( true );
  mRuleBodyEdit->clear();

  const bool writable = rulesSkillsWritable();
  for ( QWidget *widget : std::initializer_list<QWidget *> { mRuleNameEdit, mRuleDescriptionEdit, mRuleGlobsEdit, mRuleAlwaysApply, mRuleEnabled, mRuleBodyEdit } )
    widget->setEnabled( writable );
  mRuleSaveButton->setEnabled( writable );
  mRuleDuplicateButton->setEnabled( false );
  mRuleDeleteButton->setEnabled( false );
}

void QgsAiSettingsDialog::newRule()
{
  mRulesListWidget->setCurrentRow( -1 );
  clearRuleEditor();
  mRuleNameEdit->setFocus();
}

void QgsAiSettingsDialog::duplicateSelectedRule()
{
  if ( mCurrentRuleSlug.isEmpty() || !rulesSkillsWritable() )
    return;

  QStringList existingSlugs;
  for ( int i = 0; i < mRulesListWidget->count(); ++i )
    existingSlugs << mRulesListWidget->item( i )->data( Qt::UserRole ).value<QgsAiRuleInfo>().slug;

  const QString baseSlug = QgsAiRulesSkillsStore::slugify( mRuleNameEdit->text() + u"-copy"_s );
  QString candidate = baseSlug;
  int suffix = 2;
  while ( existingSlugs.contains( candidate ) )
    candidate = u"%1-%2"_s.arg( baseSlug ).arg( suffix++ );

  QgsAiRuleInfo info;
  info.slug = candidate;
  info.name = mRuleNameEdit->text().trimmed() + tr( " (copy)" );
  info.description = mRuleDescriptionEdit->text().trimmed();
  info.globs = mRuleGlobsEdit->text().split( ',', Qt::SkipEmptyParts );
  info.alwaysApply = mRuleAlwaysApply->isChecked();
  info.enabled = mRuleEnabled->isChecked();

  QString error;
  if ( !rulesSkillsStore().writeRule( mRulesRelativeDirForList, info, mRuleBodyEdit->toPlainText(), &error ) )
  {
    QMessageBox::warning( this, tr( "Could not duplicate rule" ), error );
    return;
  }
  mCurrentRuleSlug = candidate;
  refreshRulesList();
}

void QgsAiSettingsDialog::deleteSelectedRule()
{
  if ( mCurrentRuleSlug.isEmpty() )
    return;
  if ( QMessageBox::question( this, tr( "Delete rule" ), tr( "Delete rule \u201c%1\u201d? This cannot be undone." ).arg( mRuleNameEdit->text() ) ) != QMessageBox::Yes )
    return;

  QgsAiRuleInfo info;
  info.slug = mCurrentRuleSlug;
  info.path = mCurrentRulePath;
  QString error;
  if ( !rulesSkillsStore().deleteRule( info, &error ) )
  {
    QMessageBox::warning( this, tr( "Could not delete rule" ), error );
    return;
  }
  mCurrentRuleSlug.clear();
  mCurrentRulePath.clear();
  refreshRulesList();
}

void QgsAiSettingsDialog::saveCurrentRule()
{
  if ( !rulesSkillsWritable() )
  {
    QMessageBox::warning( this, tr( "Workspace not trusted" ), tr( "Trust this workspace (Workspace section) before creating or editing rules." ) );
    return;
  }
  const QString name = mRuleNameEdit->text().trimmed();
  if ( name.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Name required" ), tr( "Give the rule a name before saving." ) );
    return;
  }

  QgsAiRuleInfo info;
  info.slug = mCurrentRuleSlug.isEmpty() ? QgsAiRulesSkillsStore::slugify( name ) : mCurrentRuleSlug;
  info.name = name;
  info.description = mRuleDescriptionEdit->text().trimmed();
  QStringList globs;
  for ( const QString &part : mRuleGlobsEdit->text().split( ',', Qt::SkipEmptyParts ) )
  {
    const QString trimmedGlob = part.trimmed();
    if ( !trimmedGlob.isEmpty() )
      globs << trimmedGlob;
  }
  info.globs = globs;
  info.alwaysApply = mRuleAlwaysApply->isChecked();
  info.enabled = mRuleEnabled->isChecked();

  QString error;
  if ( !rulesSkillsStore().writeRule( mRulesRelativeDirForList, info, mRuleBodyEdit->toPlainText(), &error ) )
  {
    QMessageBox::warning( this, tr( "Could not save rule" ), error );
    return;
  }
  mCurrentRuleSlug = info.slug;
  refreshRulesList();
}

void QgsAiSettingsDialog::refreshSkillsList()
{
  if ( !mSkillsListWidget )
    return;

  mSkillsListWidget->blockSignals( true );
  mSkillsListWidget->clear();
  const QList<QgsAiSkillInfo> skills = rulesSkillsStore().listSkills( mSkillsRelativeDirForList );
  for ( const QgsAiSkillInfo &skill : skills )
  {
    QString label = skill.name;
    if ( !skill.enabled )
      label += tr( "  \u00b7  disabled" );
    QListWidgetItem *item = new QListWidgetItem( label, mSkillsListWidget );
    item->setData( Qt::UserRole, QVariant::fromValue( skill ) );
  }
  mSkillsListWidget->blockSignals( false );

  if ( mSkillsListWidget->count() == 0 )
  {
    clearSkillEditor();
    return;
  }

  int rowToSelect = 0;
  for ( int i = 0; i < mSkillsListWidget->count(); ++i )
  {
    if ( !mCurrentSkillSlug.isEmpty() && mSkillsListWidget->item( i )->data( Qt::UserRole ).value<QgsAiSkillInfo>().slug == mCurrentSkillSlug )
    {
      rowToSelect = i;
      break;
    }
  }
  mSkillsListWidget->setCurrentRow( rowToSelect );
  selectSkillInEditor( mSkillsListWidget->item( rowToSelect )->data( Qt::UserRole ).value<QgsAiSkillInfo>() );
}

void QgsAiSettingsDialog::selectSkillInEditor( const QgsAiSkillInfo &skill )
{
  mCurrentSkillSlug = skill.slug;
  mSkillNameEdit->setText( skill.name );
  mSkillDescriptionEdit->setText( skill.description );
  mSkillEnabled->setChecked( skill.enabled );
  mSkillBodyEdit->setPlainText( rulesSkillsStore().readSkillBody( skill ) );

  const bool writable = rulesSkillsWritable();
  for ( QWidget *widget : std::initializer_list<QWidget *> { mSkillNameEdit, mSkillDescriptionEdit, mSkillEnabled, mSkillBodyEdit } )
    widget->setEnabled( writable );
  mSkillSaveButton->setEnabled( writable );
  mSkillDuplicateButton->setEnabled( writable );
  mSkillDeleteButton->setEnabled( writable );
}

void QgsAiSettingsDialog::clearSkillEditor()
{
  mCurrentSkillSlug.clear();
  mSkillNameEdit->clear();
  mSkillDescriptionEdit->clear();
  mSkillEnabled->setChecked( true );
  mSkillBodyEdit->clear();

  const bool writable = rulesSkillsWritable();
  for ( QWidget *widget : std::initializer_list<QWidget *> { mSkillNameEdit, mSkillDescriptionEdit, mSkillEnabled, mSkillBodyEdit } )
    widget->setEnabled( writable );
  mSkillSaveButton->setEnabled( writable );
  mSkillDuplicateButton->setEnabled( false );
  mSkillDeleteButton->setEnabled( false );
}

void QgsAiSettingsDialog::newSkill()
{
  mSkillsListWidget->setCurrentRow( -1 );
  clearSkillEditor();
  mSkillNameEdit->setFocus();
}

void QgsAiSettingsDialog::duplicateSelectedSkill()
{
  if ( mCurrentSkillSlug.isEmpty() || !rulesSkillsWritable() )
    return;

  QStringList existingSlugs;
  for ( int i = 0; i < mSkillsListWidget->count(); ++i )
    existingSlugs << mSkillsListWidget->item( i )->data( Qt::UserRole ).value<QgsAiSkillInfo>().slug;

  const QString baseSlug = QgsAiRulesSkillsStore::slugify( mSkillNameEdit->text() + u"-copy"_s );
  QString candidate = baseSlug;
  int suffix = 2;
  while ( existingSlugs.contains( candidate ) )
    candidate = u"%1-%2"_s.arg( baseSlug ).arg( suffix++ );

  QgsAiSkillInfo info;
  info.slug = candidate;
  info.name = mSkillNameEdit->text().trimmed() + tr( " (copy)" );
  info.description = mSkillDescriptionEdit->text().trimmed();
  info.enabled = mSkillEnabled->isChecked();

  QString error;
  if ( !rulesSkillsStore().writeSkill( mSkillsRelativeDirForList, info, mSkillBodyEdit->toPlainText(), &error ) )
  {
    QMessageBox::warning( this, tr( "Could not duplicate skill" ), error );
    return;
  }
  mCurrentSkillSlug = candidate;
  refreshSkillsList();
}

void QgsAiSettingsDialog::deleteSelectedSkill()
{
  if ( mCurrentSkillSlug.isEmpty() )
    return;
  if ( QMessageBox::question( this, tr( "Delete skill" ), tr( "Delete skill \u201c%1\u201d? This cannot be undone." ).arg( mSkillNameEdit->text() ) ) != QMessageBox::Yes )
    return;

  QgsAiSkillInfo info;
  info.slug = mCurrentSkillSlug;
  const QList<QgsAiSkillInfo> skills = rulesSkillsStore().listSkills( mSkillsRelativeDirForList );
  for ( const QgsAiSkillInfo &skill : skills )
  {
    if ( skill.slug == mCurrentSkillSlug )
    {
      info = skill;
      break;
    }
  }
  QString error;
  if ( !rulesSkillsStore().deleteSkill( info, &error ) )
  {
    QMessageBox::warning( this, tr( "Could not delete skill" ), error );
    return;
  }
  mCurrentSkillSlug.clear();
  refreshSkillsList();
}

void QgsAiSettingsDialog::saveCurrentSkill()
{
  if ( !rulesSkillsWritable() )
  {
    QMessageBox::warning( this, tr( "Workspace not trusted" ), tr( "Trust this workspace (Workspace section) before creating or editing skills." ) );
    return;
  }
  const QString name = mSkillNameEdit->text().trimmed();
  if ( name.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Name required" ), tr( "Give the skill a name before saving." ) );
    return;
  }
  const QString description = mSkillDescriptionEdit->text().trimmed();
  if ( description.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Description required" ), tr( "Describe when the agent should use this skill before saving." ) );
    return;
  }

  QgsAiSkillInfo info;
  info.slug = mCurrentSkillSlug.isEmpty() ? QgsAiRulesSkillsStore::slugify( name ) : mCurrentSkillSlug;
  info.name = name;
  info.description = description;
  info.enabled = mSkillEnabled->isChecked();

  QString error;
  if ( !rulesSkillsStore().writeSkill( mSkillsRelativeDirForList, info, mSkillBodyEdit->toPlainText(), &error ) )
  {
    QMessageBox::warning( this, tr( "Could not save skill" ), error );
    return;
  }
  mCurrentSkillSlug = info.slug;
  refreshSkillsList();
}

void QgsAiSettingsDialog::syncRulesSkillsToCloud()
{
  if ( !mSessionManager || !mModelRouter || !mAccountWidget )
    return;

  const QString token = mModelRouter->planSessionToken().trimmed();
  if ( token.isEmpty() )
  {
    QMessageBox::information( this, tr( "Strata Cloud sync" ), tr( "Sign in to Plan Account before syncing rules and skills." ) );
    return;
  }
  const QString workspaceRoot = mSessionManager->workspaceRoot();
  if ( workspaceRoot.trimmed().isEmpty() )
  {
    QMessageBox::warning( this, tr( "Strata Cloud sync" ), tr( "Workspace root is unset." ) );
    return;
  }

  const QgsAiRulesSkillsStore store = rulesSkillsStore();
  const QList<QgsAiRuleInfo> rules = store.listRules( mRulesRelativeDirForList );
  const QList<QgsAiSkillInfo> skills = store.listSkills( mSkillsRelativeDirForList );
  if ( rules.isEmpty() && skills.isEmpty() )
  {
    QMessageBox::information( this, tr( "Strata Cloud sync" ), tr( "No local rules or skills to sync yet." ) );
    return;
  }

  QList<QgsAiRulesSkillsCloudClient::RemoteRule> remoteRules;
  remoteRules.reserve( rules.size() );
  for ( const QgsAiRuleInfo &rule : rules )
    remoteRules << QgsAiRulesSkillsCloudClient::toRemoteRule( rule, store.readRuleBody( rule ) );

  QList<QgsAiRulesSkillsCloudClient::RemoteSkill> remoteSkills;
  remoteSkills.reserve( skills.size() );
  for ( const QgsAiSkillInfo &skill : skills )
    remoteSkills << QgsAiRulesSkillsCloudClient::toRemoteSkill( skill, store.readSkillBody( skill ) );

  const int totalExpected = remoteRules.size() + remoteSkills.size();
  auto progress = std::make_shared<int>( 0 );
  auto failures = std::make_shared<int>( 0 );
  auto workspaceObtained = std::make_shared<bool>( false );

  mSyncRulesSkillsCloudButton->setEnabled( false );
  mRulesSkillsCloudStatusLabel->setText( tr( "Syncing %1 rule(s) and %2 skill(s) to Strata Cloud…" ).arg( remoteRules.size() ).arg( remoteSkills.size() ) );

  QgsAiRulesSkillsCloudClient *client = new QgsAiRulesSkillsCloudClient( this );

  auto maybeFinish = [this, client, progress, failures, totalExpected]() {
    if ( *progress < totalExpected )
      return;
    mSyncRulesSkillsCloudButton->setEnabled( true );
    mRulesSkillsCloudStatusLabel->setText( *failures == 0 ? tr( "Synced %1 item(s) to Strata Cloud." ).arg( totalExpected ) : tr( "Synced with %1 error(s); see the warning dialog." ).arg( *failures ) );
    client->deleteLater();
  };

  connect( client, &QgsAiRulesSkillsCloudClient::ruleSynced, this, [progress, maybeFinish]( const QgsAiRulesSkillsCloudClient::RemoteRule & ) {
    ++( *progress );
    maybeFinish();
  } );
  connect( client, &QgsAiRulesSkillsCloudClient::skillSynced, this, [progress, maybeFinish]( const QgsAiRulesSkillsCloudClient::RemoteSkill & ) {
    ++( *progress );
    maybeFinish();
  } );
  connect( client, &QgsAiRulesSkillsCloudClient::requestFailed, this, [this, client, progress, failures, workspaceObtained, totalExpected, maybeFinish]( const QString &message ) {
    if ( !*workspaceObtained )
    {
      // Fatal: could not even resolve/create the cloud workspace, so no per-item requests were sent.
      mSyncRulesSkillsCloudButton->setEnabled( true );
      mRulesSkillsCloudStatusLabel->setText( tr( "Strata Cloud sync failed." ) );
      QMessageBox::warning( this, tr( "Strata Cloud sync failed" ), message );
      client->deleteLater();
      return;
    }
    ++( *progress );
    ++( *failures );
    if ( *progress == totalExpected )
      QMessageBox::warning( this, tr( "Strata Cloud sync" ), tr( "Some items failed to sync. Last error: %1" ).arg( message ) );
    maybeFinish();
  } );
  connect( client, &QgsAiRulesSkillsCloudClient::workspaceReady, this, [client, remoteRules, remoteSkills, workspaceObtained, apiBase = mAccountWidget->planEndpoint(), token]( const QString &workspaceId ) {
    *workspaceObtained = true;
    for ( const QgsAiRulesSkillsCloudClient::RemoteRule &rule : remoteRules )
      client->pushRule( apiBase, token, workspaceId, rule );
    for ( const QgsAiRulesSkillsCloudClient::RemoteSkill &skill : remoteSkills )
      client->pushSkill( apiBase, token, workspaceId, skill );
  } );

  client->ensureWorkspace( mAccountWidget->planEndpoint(), token, workspaceRoot, QFileInfo( workspaceRoot ).fileName() );
}

QWidget *QgsAiSettingsDialog::buildIndexingPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Indexing & Docs" ), tr( "Local retrieval index (RAG) over workspace files and layers." ), contentLayout );

  QgsSettings indexSettings;
  const bool canUseEmbeddings = mSessionManager && mSessionManager->workspaceIndex() && mSessionManager->workspaceIndex()->embeddingProviderAvailable();

  mEmbeddingProvider = new QComboBox( page );
  mEmbeddingProvider->setObjectName( u"aiEmbeddingProviderComboBox"_s );
  for ( const QgsAiEmbeddingProviderUiEntry &entry : QgsAiEmbeddingProviderRegistry::providerUiEntries() )
  {
    mEmbeddingProvider->addItem( entry.displayName, entry.providerId );
    const int row = mEmbeddingProvider->count() - 1;
    if ( !entry.unavailableReason.isEmpty() )
      mEmbeddingProvider->setItemData( row, entry.unavailableReason, Qt::ToolTipRole );
    if ( !entry.selectable )
    {
      if ( QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>( mEmbeddingProvider->model() ) )
      {
        if ( QStandardItem *item = itemModel->item( row ) )
        {
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( entry.unavailableReason );
        }
      }
    }
  }
  const QString configuredEmbeddingProvider = QgsAiEmbeddingProviderRegistry::configuredProviderId();
  const int embeddingProviderIndex = mEmbeddingProvider->findData( configuredEmbeddingProvider );
  mEmbeddingProvider->setCurrentIndex( embeddingProviderIndex >= 0 ? embeddingProviderIndex : 0 );
  contentLayout->addWidget( settingRow( tr( "Embedding provider" ), QString(), mEmbeddingProvider, page ) );

  mEmbeddingStatusLabel = new QLabel( page );
  mEmbeddingStatusLabel->setObjectName( u"aiEmbeddingProviderStatusLabel"_s );
  mEmbeddingStatusLabel->setWordWrap( true );
  mDownloadEmbeddingModelButton = new QPushButton( tr( "Download local E5 model" ), page );
  mDownloadEmbeddingModelButton->setObjectName( u"aiDownloadEmbeddingModelButton"_s );

  // Remote embedding model id, shown only when a remote provider is selected.
  mRemoteEmbeddingModel = new QLineEdit( page );
  mRemoteEmbeddingModel->setObjectName( u"aiRemoteEmbeddingModelLineEdit"_s );
  mRemoteEmbeddingModelRow = settingRow( tr( "Remote embedding model" ), QString(), mRemoteEmbeddingModel, page );
  contentLayout->addWidget( mRemoteEmbeddingModelRow );
  contentLayout->addWidget( mEmbeddingStatusLabel );
  contentLayout->addWidget( settingRow( tr( "Local model" ), QString(), mDownloadEmbeddingModelButton, page ) );

  refreshEmbeddingStatusLabel();
  refreshRemoteEmbeddingModelField();
  connect( mEmbeddingProvider, &QComboBox::currentIndexChanged, this, [this]( int ) {
    refreshEmbeddingStatusLabel();
    refreshRemoteEmbeddingModelField();
  } );
  connect( mDownloadEmbeddingModelButton, &QPushButton::clicked, this, [this]() {
    QString error;
    if ( !downloadEmbeddingModelWithConsent( this, &error ) )
    {
      if ( !error.contains( tr( "not approved" ), Qt::CaseInsensitive ) )
        QMessageBox::warning( this, tr( "Embedding model download failed" ), error.isEmpty() ? tr( "Unknown error." ) : error );
      refreshEmbeddingStatusLabel();
      return;
    }

    emit embeddingProviderSettingsChanged();
    refreshEmbeddingStatusLabel();
    QMessageBox::information( this, tr( "Embedding model downloaded" ), tr( "The local multilingual E5 embedding model is ready. Existing MinHash indexes will rebuild automatically when E5 is selected." ) );
  } );

  mAutomaticIndexing = new QCheckBox( page );
  mAutomaticIndexing->setObjectName( u"aiAutomaticIndexingCheckBox"_s );
  // Default ON only when an embedding provider is actually available; if the user
  // already chose a value, keep it. The toggle is disabled when no provider is available.
  const bool hasAutomaticSetting = indexSettings.contains( u"strata/index/automatic"_s );
  mAutomaticIndexing->setChecked( hasAutomaticSetting ? indexSettings.value( u"strata/index/automatic"_s, true ).toBool() : canUseEmbeddings );
  mAutomaticIndexing->setEnabled( canUseEmbeddings );
  contentLayout->addWidget(
    settingRow( tr( "Index workspace automatically" ), tr( "Strata refreshes the local retrieval index in the background after opening or changing the project. The task is cancellable." ), mAutomaticIndexing, page )
  );

  const bool hasLayerIndexingSetting = indexSettings.contains( u"strata/index/enable_layer_indexing"_s )
                                       || indexSettings.contains( u"geoai/index/enable_layer_indexing"_s )
                                       || indexSettings.contains( u"qgis_ai/index/enable_layer_indexing"_s );
  const bool defaultLayerIndexingEnabled = mAutomaticIndexing->isChecked();
  const bool requestedLayerIndexing
    = hasLayerIndexingSetting
        ? settingValueWithLegacy( indexSettings, u"strata/index/enable_layer_indexing"_s, QStringList { u"geoai/index/enable_layer_indexing"_s, u"qgis_ai/index/enable_layer_indexing"_s }, false ).toBool()
        : defaultLayerIndexingEnabled;
  const bool layerIndexingEnabled = requestedLayerIndexing && canUseEmbeddings;

  mEnableLayerIndexing = new QCheckBox( page );
  mEnableLayerIndexing->setObjectName( u"aiEnableLayerIndexingCheckBox"_s );
  mEnableLayerIndexing->setChecked( layerIndexingEnabled );
  mEnableLayerIndexing->setEnabled( canUseEmbeddings );
  contentLayout->addWidget(
    settingRow( tr( "Enable layer indexing" ), tr( "Layer attributes and bounding boxes are embedded with the selected indexing provider and indexed so the assistant can ground its answers on actual layer data. Reindexes on layer add/remove/edit." ), mEnableLayerIndexing, page )
  );

  mIndexStatusLabel = new QLabel( page );
  mIndexStatusLabel->setObjectName( u"aiIndexStatusLabel"_s );
  refreshIndexStatusLabel();
  contentLayout->addWidget( mIndexStatusLabel );

  contentLayout->addWidget( sectionHeader( tr( "Strata Cloud sync" ), page ) );

  mCloudContextOptIn = new QCheckBox( page );
  mCloudContextOptIn->setObjectName( u"aiCloudContextOptInCheckBox"_s );
  mCloudContextOptIn->setChecked( indexSettings.value( u"strata/index/cloud_context_opt_in"_s, false ).toBool() );
  contentLayout->addWidget(
    settingRow( tr( "Sync safe RAG context to Strata Cloud" ), tr( "Only metadata-safe layer summaries and opted-in rules/skills/PDF/image text are sent. Geometry, coordinates, WKT and datasource URIs are blocked before upload." ), mCloudContextOptIn, page )
  );

  mCloudIndexStatusLabel = new QLabel( page );
  mCloudIndexStatusLabel->setObjectName( u"aiCloudIndexStatusLabel"_s );
  mCloudIndexStatusLabel->setWordWrap( true );
  refreshCloudIndexStatusLabel();
  contentLayout->addWidget( mCloudIndexStatusLabel );

  mSyncCloudContextButton = new QPushButton( tr( "Sync safe context now" ), page );
  mSyncCloudContextButton->setObjectName( u"aiSyncCloudContextButton"_s );
  mSyncCloudContextButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() && mModelRouter && !mModelRouter->planSessionToken().trimmed().isEmpty() );
  contentLayout->addWidget( settingRow( tr( "Cloud sync" ), QString(), mSyncCloudContextButton, page ) );

  connect( mCloudContextOptIn, &QCheckBox::toggled, this, [this]( bool ) { refreshCloudIndexStatusLabel(); } );

  connect( mSyncCloudContextButton, &QPushButton::clicked, this, [this]() {
    if ( !mSessionManager || !mSessionManager->workspaceIndex() || !mModelRouter )
      return;

    if ( !mCloudContextOptIn->isChecked() )
    {
      QMessageBox::information( this, tr( "Cloud context sync" ), tr( "Enable the Strata Cloud context sync opt-in before uploading context." ) );
      return;
    }

    const QString token = mModelRouter->planSessionToken().trimmed();
    if ( token.isEmpty() )
    {
      QMessageBox::information( this, tr( "Cloud context sync" ), tr( "Sign in to Plan Account before syncing cloud context." ) );
      return;
    }

    auto buildCloudContextItems = [this]() {
      QList<QgsAiCloudIndexClient::ContextItem> items;
      if ( mSessionManager && mSessionManager->workspaceIndex() )
      {
        mSessionManager->workspaceIndex()->ensureLoaded();
        items += QgsAiCloudIndexClient::contextItemsFromChunks( mSessionManager->workspaceIndex()->chunks() );
      }
      if ( mSessionManager )
      {
        const QgsAiAgentBehaviorSettings behavior = mSessionManager->agentBehaviorSettings();
        items += QgsAiCloudIndexClient::
          contextItemsFromWorkspaceFolders( mSessionManager->workspaceRoot(), behavior.loadWorkspaceRules ? behavior.rulesPath : QString(), behavior.loadWorkspaceSkills ? behavior.skillsPath : QString() );
      }
      return QgsAiCloudIndexClient::deduplicateContextItems( items );
    };

    const QList<QgsAiCloudIndexClient::ContextItem> items = buildCloudContextItems();
    QString validationError;
    if ( !QgsAiCloudIndexClient::validateContextItems( items, &validationError ) )
    {
      QMessageBox::warning( this, tr( "Cloud context sync blocked" ), validationError );
      refreshCloudIndexStatusLabel();
      return;
    }

    const QString workspaceRoot = mSessionManager->workspaceRoot();
    if ( workspaceRoot.trimmed().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Cloud context sync" ), tr( "Workspace root is unset." ) );
      return;
    }

    mSyncCloudContextButton->setEnabled( false );
    mCloudIndexStatusLabel->setText( tr( "Cloud sync running..." ) );
    QgsAiCloudIndexClient *client = new QgsAiCloudIndexClient( this );
    connect( client, &QgsAiCloudIndexClient::contextSynced, this, [this, client]( const QgsAiCloudIndexClient::SyncResult &result ) {
      mSyncCloudContextButton->setEnabled( true );
      mCloudIndexStatusLabel->setText( tr( "Cloud sync queued %1 context items for workspace %2." ).arg( result.queued ).arg( result.workspaceId ) );
      refreshCloudIndexStatusLabel();
      client->deleteLater();
    } );
    connect( client, &QgsAiCloudIndexClient::requestFailed, this, [this, client]( const QString &message ) {
      mSyncCloudContextButton->setEnabled( true );
      mCloudIndexStatusLabel->setText( tr( "Cloud sync failed." ) );
      QMessageBox::warning( this, tr( "Cloud context sync failed" ), message );
      client->deleteLater();
    } );
    client->syncWorkspaceContext( mAccountWidget->planEndpoint(), token, workspaceRoot, QFileInfo( workspaceRoot ).fileName(), items, true );
  } );

  contentLayout->addWidget( sectionHeader( tr( "Maintenance" ), page ) );

  // Rebuilding can be expensive: heavy local CPU usage, or remote API cost and data egress.
  // Always confirm before starting, with a message tailored to the selected provider.
  mRebuildWorkspaceIndexButton = new QPushButton( tr( "Rebuild now" ), page );
  mRebuildWorkspaceIndexButton->setObjectName( u"aiRebuildWorkspaceIndexButton"_s );
  mRebuildWorkspaceIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
  contentLayout->addWidget( settingRow( tr( "File/workspace index" ), QString(), mRebuildWorkspaceIndexButton, page ) );

  mRebuildLayerIndexButton = new QPushButton( tr( "Rebuild now" ), page );
  mRebuildLayerIndexButton->setObjectName( u"aiRebuildLayerIndexButton"_s );
  mRebuildLayerIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
  contentLayout->addWidget( settingRow( tr( "Layer index" ), QString(), mRebuildLayerIndexButton, page ) );

  connect( mRebuildWorkspaceIndexButton, &QPushButton::clicked, this, [this]() {
    if ( !mSessionManager || !mSessionManager->workspaceIndex() )
      return;

    if ( !ensureEmbeddingProvider() )
      return;

    if ( !confirmRebuild( tr( "file/workspace index" ) ) )
      return;

    QString err;
    QString workspaceRoot;
    QList<QgsAiWorkspaceIndex::WorkspaceFileSnapshot> snapshot;
    if ( !mSessionManager->workspaceIndex()->createWorkspaceFileSnapshot( QgsAiWorkspaceIndex::DEFAULT_MAX_FILES, workspaceRoot, snapshot, &err ) )
    {
      QMessageBox::warning( this, tr( "Workspace reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
      return;
    }

    QgsTaskManager *taskManager = QgsApplication::taskManager();
    if ( !taskManager )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      const bool ok = mSessionManager->workspaceIndex()->reindex( snapshot, workspaceRoot, &err );
      QApplication::restoreOverrideCursor();
      if ( !ok )
      {
        QMessageBox::warning( this, tr( "Workspace reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
        return;
      }
      refreshIndexStatusLabel();
      const auto status = mSessionManager->workspaceIndex()->status();
      QMessageBox::information( this, tr( "Workspace reindex" ), tr( "Done — %1 file chunks indexed." ).arg( status.fileChunkCount ) );
      return;
    }

    mRebuildWorkspaceIndexButton->setEnabled( false );
    ManualWorkspaceIndexTask *task = new ManualWorkspaceIndexTask( mSessionManager->workspaceIndex(), workspaceRoot, snapshot );
    connect( task, &QgsTask::taskCompleted, this, [this]() {
      mRebuildWorkspaceIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
      refreshIndexStatusLabel();
      if ( mSessionManager && mSessionManager->workspaceIndex() )
      {
        const auto status = mSessionManager->workspaceIndex()->status();
        QMessageBox::information( this, tr( "Workspace reindex" ), tr( "Done — %1 file chunks indexed." ).arg( status.fileChunkCount ) );
      }
    } );
    connect( task, &QgsTask::taskTerminated, this, [this, task]() {
      mRebuildWorkspaceIndexButton->setEnabled( true );
      const QString taskError = task->errorMessage();
      if ( !taskError.isEmpty() )
        QMessageBox::warning( this, tr( "Workspace reindex failed" ), taskError );
    } );
    taskManager->addTask( task, 1 );
  } );

  connect( mRebuildLayerIndexButton, &QPushButton::clicked, this, [this]() {
    if ( !mSessionManager || !mSessionManager->workspaceIndex() )
      return;

    if ( !ensureEmbeddingProvider() )
      return;

    if ( !confirmRebuild( tr( "layer index" ) ) )
      return;

    QString err;
    QgsAiWorkspaceIndex::WorkspaceLayerSnapshot snapshot;
    if ( !mSessionManager->workspaceIndex()->createWorkspaceLayerSnapshot( snapshot, &err ) )
    {
      QMessageBox::warning( this, tr( "Layer reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
      return;
    }

    QgsTaskManager *taskManager = QgsApplication::taskManager();
    if ( !taskManager )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      const bool ok = mSessionManager->workspaceIndex()->reindexLayerSnapshot( snapshot, &err );
      QApplication::restoreOverrideCursor();
      if ( !ok )
      {
        QMessageBox::warning( this, tr( "Layer reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
        return;
      }
      refreshIndexStatusLabel();
      const auto status = mSessionManager->workspaceIndex()->status();
      QMessageBox::information( this, tr( "Layer reindex" ), tr( "Done — %1 layer chunks indexed." ).arg( status.layerChunkCount ) );
      return;
    }

    mRebuildLayerIndexButton->setEnabled( false );
    ManualLayerIndexTask *task = new ManualLayerIndexTask( mSessionManager->workspaceIndex(), snapshot );
    connect( task, &QgsTask::taskCompleted, this, [this]() {
      mRebuildLayerIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
      refreshIndexStatusLabel();
      if ( mSessionManager && mSessionManager->workspaceIndex() )
      {
        const auto status = mSessionManager->workspaceIndex()->status();
        QMessageBox::information( this, tr( "Layer reindex" ), tr( "Done — %1 layer chunks indexed." ).arg( status.layerChunkCount ) );
      }
    } );
    connect( task, &QgsTask::taskTerminated, this, [this, task]() {
      mRebuildLayerIndexButton->setEnabled( true );
      const QString taskError = task->errorMessage();
      if ( !taskError.isEmpty() )
        QMessageBox::warning( this, tr( "Layer reindex failed" ), taskError );
    } );
    taskManager->addTask( task, 1 );
  } );

  return page;
}

QWidget *QgsAiSettingsDialog::buildWorkspacePage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Workspace" ), tr( "Where the assistant reads and writes files." ), contentLayout );

  QgsSettings workspaceSettings;
  mWorkspaceRoot
    = new QLineEdit( settingValueWithLegacy( workspaceSettings, u"strata/workspace/root"_s, QStringList { u"geoai/workspace/root"_s, u"qgis_ai/workspace/root"_s }, QString() ).toString(), page );
  mWorkspaceRoot->setObjectName( u"aiWorkspaceRootLineEdit"_s );
  mWorkspaceRoot->setPlaceholderText( tr( "Used when the QGIS project is unsaved" ) );
  QPushButton *browseWorkspaceRoot = new QPushButton( tr( "Browse..." ), page );
  QWidget *workspaceRootWidget = new QWidget( page );
  QHBoxLayout *workspaceRootLayout = new QHBoxLayout( workspaceRootWidget );
  workspaceRootLayout->setContentsMargins( 0, 0, 0, 0 );
  workspaceRootLayout->addWidget( mWorkspaceRoot, 1 );
  workspaceRootLayout->addWidget( browseWorkspaceRoot );
  contentLayout->addWidget( settingRowFullWidth( tr( "AI workspace root" ), tr( "Used only when the current QGIS project has no home path." ), workspaceRootWidget, page ) );

  // Workspace trust: gates rules/skills loading and the risky tools.
  mTrustWorkspace = new QCheckBox( page );
  mTrustWorkspace->setObjectName( u"aiTrustWorkspaceCheckBox"_s );
  contentLayout->addWidget( settingRow( tr( "Trust this workspace" ), tr( "Enables rules/skills files and the run_python, install_python_package and download_file tools." ), mTrustWorkspace, page ) );
  refreshTrustWorkspace();

  connect( browseWorkspaceRoot, &QPushButton::clicked, this, [this]() {
    const QString dir = QFileDialog::getExistingDirectory( this, tr( "Choose AI workspace root" ), mWorkspaceRoot->text().trimmed() );
    if ( !dir.isEmpty() )
      mWorkspaceRoot->setText( QDir::cleanPath( dir ) );
  } );
  connect( mWorkspaceRoot, &QLineEdit::textChanged, this, [this]( const QString & ) { refreshTrustWorkspace(); } );

  return page;
}

QWidget *QgsAiSettingsDialog::buildPrivacyPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Privacy & Telemetry" ), tr( "All sharing is opt-in and metadata-only, never payloads." ), contentLayout );

  // Credential/data encryption status (best-effort vault policy: never prompts).
  QLabel *encryptionStatus = new QLabel( page );
  encryptionStatus->setObjectName( u"aiCredentialEncryptionStatusLabel"_s );
  encryptionStatus->setWordWrap( true );
  encryptionStatus->setText(
    QgsAiSecretStore::vaultUsable() ? tr( "AI credentials and data are encrypted in the QGIS authentication vault." )
                                    : tr( "AI credentials and data are stored unencrypted — unlock or set a QGIS master password (Options ▸ Authentication) to enable encryption." )
  );
  contentLayout->addWidget( encryptionStatus );

  QgsSettings productSettings;
  mPrivacyMetadataOnly = new QCheckBox( page );
  mPrivacyMetadataOnly->setObjectName( u"aiPrivacyMetadataOnlyCheckBox"_s );
  mPrivacyMetadataOnly->setChecked( productSettings.value( u"strata/privacy/metadata_only_ack"_s, false ).toBool() );
  contentLayout->addWidget(
    settingRow( tr( "Acknowledge managed cloud privacy boundary" ), tr( "Managed cloud features only ever receive metadata, never your data payloads." ), mPrivacyMetadataOnly, page )
  );

  mTelemetryOptIn = new QCheckBox( page );
  mTelemetryOptIn->setObjectName( u"aiTelemetryOptInCheckBox"_s );
  mTelemetryOptIn->setChecked( productSettings.value( u"strata/telemetry/opt_in"_s, false ).toBool() );
  contentLayout->addWidget( settingRow( tr( "Share product telemetry" ), tr( "Metadata-only usage signals." ), mTelemetryOptIn, page ) );

  mCrashReportOptIn = new QCheckBox( page );
  mCrashReportOptIn->setObjectName( u"aiCrashReportOptInCheckBox"_s );
  mCrashReportOptIn->setChecked( productSettings.value( u"strata/crash_reporting/metadata_only_opt_in"_s, false ).toBool() );
  contentLayout->addWidget( settingRow( tr( "Share crash reports" ), tr( "Metadata-only crash signatures." ), mCrashReportOptIn, page ) );

  QLabel *storageNote = new QLabel(
    tr(
      "OpenAI, OpenRouter and Claude API keys and the Codex OAuth refresh token are stored locally in application settings; the Claude OAuth refresh token is stored in the encrypted QGIS "
      "authentication store. Leave API key fields empty to keep the current saved value. Agent rules and skills are stored locally in application settings."
    ),
    page
  );
  storageNote->setProperty( "aiRole", u"rowDescription"_s );
  storageNote->setWordWrap( true );
  contentLayout->addWidget( storageNote );

  return page;
}

QWidget *QgsAiSettingsDialog::buildOnboardingPage()
{
  QVBoxLayout *contentLayout = nullptr;
  QWidget *page = createPage( tr( "Onboarding & Release" ), tr( "First-run checklist and release readiness." ), contentLayout );

  mOnboardingStatusLabel = new QLabel( page );
  mOnboardingStatusLabel->setObjectName( u"aiOnboardingStatusLabel"_s );
  mOnboardingStatusLabel->setWordWrap( true );
  contentLayout->addWidget( settingRowFullWidth( tr( "First-run checklist" ), QString(), mOnboardingStatusLabel, page ) );

  mReleaseDryRunStatus = new QLabel( page );
  mReleaseDryRunStatus->setObjectName( u"aiReleaseDryRunStatusLabel"_s );
  mReleaseDryRunStatus->setWordWrap( true );
  QgsSettings productSettings;
  const QString savedReleaseChecksum = productSettings.value( u"strata/release/dry_run_checksum"_s ).toString();
  mReleaseDryRunStatus->setText( savedReleaseChecksum.isEmpty() ? tr( "Release dry-run: not run." ) : tr( "Release dry-run checksum: %1." ).arg( savedReleaseChecksum ) );

  QPushButton *releaseDryRunButton = new QPushButton( tr( "Run release dry-run" ), page );
  releaseDryRunButton->setObjectName( u"aiReleaseDryRunButton"_s );
  QPushButton *createDemoProjectButton = new QPushButton( tr( "Create demo project" ), page );
  createDemoProjectButton->setObjectName( u"aiCreateDemoProjectButton"_s );

  contentLayout->addWidget( settingRow( tr( "Demo project" ), tr( "Creates a small in-memory sample project to try the assistant." ), createDemoProjectButton, page ) );
  contentLayout->addWidget( settingRow( tr( "Release dry-run" ), tr( "Stores only a local readiness manifest checksum." ), releaseDryRunButton, page ) );
  contentLayout->addWidget( mReleaseDryRunStatus );

  refreshOnboardingStatus();

  connect( mPrivacyMetadataOnly, &QCheckBox::toggled, this, [this]( bool ) { refreshOnboardingStatus(); } );
  connect( mAutomaticIndexing, &QCheckBox::toggled, this, [this]( bool ) { refreshOnboardingStatus(); } );
  connect( mEnableLayerIndexing, &QCheckBox::toggled, this, [this]( bool ) { refreshOnboardingStatus(); } );
  connect( mCloudContextOptIn, &QCheckBox::toggled, this, [this]( bool ) { refreshOnboardingStatus(); } );

  connect( createDemoProjectButton, &QPushButton::clicked, this, [this]() {
    QgsProject *project = QgsProject::instance();
    if ( !project )
      return;

    project->clear();
    QgsVectorLayer *demoLayer = new QgsVectorLayer( u"Point?field=name:string&field=kind:string&crs=EPSG:4326"_s, tr( "Strata demo points" ), u"memory"_s );
    if ( demoLayer->isValid() && demoLayer->dataProvider() )
    {
      QList<QgsFeature> features;
      QgsFeature rome( demoLayer->fields() );
      rome.setAttributes( QVariantList { tr( "Rome sample" ), tr( "point of interest" ) } );
      rome.setGeometry( QgsGeometry::fromWkt( u"POINT(12.4924 41.8902)"_s ) );
      features << rome;
      QgsFeature milan( demoLayer->fields() );
      milan.setAttributes( QVariantList { tr( "Milan sample" ), tr( "point of interest" ) } );
      milan.setGeometry( QgsGeometry::fromWkt( u"POINT(9.1900 45.4642)"_s ) );
      features << milan;
      demoLayer->dataProvider()->addFeatures( features );
      demoLayer->updateExtents();
      project->addMapLayer( demoLayer );
    }
    else
    {
      delete demoLayer;
    }
    project->setTitle( tr( "Strata demo project" ) );

    QgsSettings settings;
    settings.setValue( u"strata/onboarding/demo_project_seen"_s, true );
    refreshOnboardingStatus();
    emit demoProjectCreated();
  } );

  connect( releaseDryRunButton, &QPushButton::clicked, this, [this]() {
    QJsonObject manifest;
    manifest.insert( u"app"_s, u"Strata"_s );
    manifest.insert( u"plan_ready"_s, mModelRouter && mModelRouter->isProviderAvailable( QgsAiModelRouter::Provider::Plan ) );
    manifest.insert( u"byok_ready"_s, hasByokProvider( mModelRouter ) );
    manifest.insert( u"privacy_metadata_only_ack"_s, mPrivacyMetadataOnly && mPrivacyMetadataOnly->isChecked() );
    manifest.insert( u"telemetry_opt_in"_s, mTelemetryOptIn && mTelemetryOptIn->isChecked() );
    manifest.insert( u"crash_metadata_only_opt_in"_s, mCrashReportOptIn && mCrashReportOptIn->isChecked() );
    manifest.insert( u"automatic_indexing"_s, mAutomaticIndexing && mAutomaticIndexing->isChecked() );
    manifest.insert( u"layer_indexing"_s, mEnableLayerIndexing && mEnableLayerIndexing->isChecked() );
    manifest.insert( u"cloud_context_opt_in"_s, mCloudContextOptIn && mCloudContextOptIn->isChecked() );
    manifest.insert( u"demo_project_ready"_s, demoProjectReady() );
    manifest.insert( u"active_provider"_s, mModelRouter ? mModelRouter->providerDisplayName( mModelRouter->resolveProvider() ) : QString() );
    manifest.insert( u"workspace_root_configured"_s, mSessionManager && !mSessionManager->workspaceRoot().trimmed().isEmpty() );

    const QByteArray payload = QJsonDocument( manifest ).toJson( QJsonDocument::Compact );
    const QString checksum = QString::fromLatin1( QCryptographicHash::hash( payload, QCryptographicHash::Sha256 ).toHex().left( 16 ) );
    mReleaseDryRunStatus->setProperty( "checksum", checksum );
    mReleaseDryRunStatus->setText( tr( "Release dry-run OK. Metadata checksum: %1." ).arg( checksum ) );

    QgsSettings settings;
    settings.setValue( u"strata/release/dry_run_checksum"_s, checksum );
    settings.setValue( u"strata/release/dry_run_manifest"_s, QString::fromUtf8( payload ) );
  } );

  return page;
}

void QgsAiSettingsDialog::refreshSidebarAccountHeader()
{
  const bool signedIn = mAccountWidget->isSignedIn();
  const QString email = mAccountWidget->accountEmail();
  mSidebarAvatar->setText( email.isEmpty() ? ( signedIn ? u"•"_s : u"?"_s ) : email.left( 1 ).toUpper() );
  mSidebarEmailLabel->setText( signedIn ? ( email.isEmpty() ? tr( "Signed in" ) : email ) : tr( "Not signed in" ) );
  mSidebarEmailLabel->setToolTip( email );
}

void QgsAiSettingsDialog::refreshTrustWorkspace()
{
  auto trustRoot = [this]() {
    if ( QgsProject::instance() )
    {
      const QString projectHome = QgsProject::instance()->homePath().trimmed();
      if ( !projectHome.isEmpty() )
        return QDir( projectHome ).absolutePath();
    }

    const QString requestedWorkspaceRoot = mWorkspaceRoot->text().trimmed();
    return requestedWorkspaceRoot.isEmpty() ? QString() : QDir( requestedWorkspaceRoot ).absolutePath();
  };

  mTrustRootForCheckbox = trustRoot();
  const bool hasRoot = !mTrustRootForCheckbox.isEmpty();
  mTrustWorkspace->setEnabled( hasRoot );
  mTrustWorkspace->setChecked( hasRoot && QgsAiWorkspaceTrust::isTrusted( mTrustRootForCheckbox ) );
  mTrustWorkspace->setToolTip( hasRoot ? tr( "Current workspace: %1" ).arg( mTrustRootForCheckbox ) : tr( "No workspace configured." ) );
}

void QgsAiSettingsDialog::refreshEmbeddingStatusLabel()
{
  const QString providerId = mEmbeddingProvider->currentData().toString();
  const bool e5Compiled = QgsAiEmbeddingProviderRegistry::providerIds().contains( QgsAiE5EmbeddingProvider::staticProviderId() );
  if ( QgsAiEmbeddingProviderRegistry::isRemoteProviderId( providerId ) )
  {
    mEmbeddingStatusLabel->setText( tr( "Remote workspace indexing will use %1 only because it is explicitly selected here. Saved provider keys do not switch indexing by themselves." )
                                      .arg( QgsAiEmbeddingProviderRegistry::displayNameForProviderId( providerId ) ) );
    mDownloadEmbeddingModelButton->setVisible( false );
  }
  else if ( providerId == QgsAiE5EmbeddingProvider::staticProviderId() )
  {
    if ( !e5Compiled )
    {
      mEmbeddingStatusLabel->setText(
        tr( "Local multilingual E5 is not available in this build because ONNX Runtime and SentencePiece support were not found at compile time. Use MinHash or rebuild Strata with those dependencies." )
      );
      mDownloadEmbeddingModelButton->setVisible( false );
      mDownloadEmbeddingModelButton->setEnabled( false );
      return;
    }
    QString filesError;
    const QString modelDir = QgsAiE5EmbeddingProvider::activeModelDirectory();
    const bool filesAvailable = QgsAiE5EmbeddingProvider::modelFilesAvailable( modelDir, &filesError );
    const QString developerDir = QgsAiE5EmbeddingProvider::developerModelDirectory();
    mDownloadEmbeddingModelButton->setVisible( true );
    mDownloadEmbeddingModelButton->setEnabled( developerDir.isEmpty() );
    mDownloadEmbeddingModelButton->setToolTip(
      developerDir.isEmpty() ? tr( "Download the pinned multilingual E5 ONNX model and SentencePiece tokenizer to the local Strata model cache." )
                             : tr( "STRATA_AI_EMBEDDING_MODEL_DIR is set; unset it to use the downloaded cache from this dialog." )
    );
    mEmbeddingStatusLabel->setText(
      filesAvailable ? tr( "Local multilingual E5 model files are installed in %1. Indexing runs on this computer without an API key." ).arg( modelDir )
                     : tr( "Local multilingual E5 model is not installed or not usable: %1\nDownload size: %2. Developers can set STRATA_AI_EMBEDDING_MODEL_DIR." )
                         .arg( filesError, humanBytes( QgsAiE5EmbeddingProvider::downloadSize() ) )
    );
  }
  else
  {
    mEmbeddingStatusLabel->setText(
      e5Compiled ? tr( "Local MinHash fallback is available and runs on this computer without an API key. Semantic quality is lower than multilingual E5." )
                 : tr( "Local MinHash fallback is available and runs on this computer without an API key. Multilingual E5 requires ONNX Runtime and SentencePiece support in the Strata build." )
    );
    mDownloadEmbeddingModelButton->setVisible( false );
  }
}

void QgsAiSettingsDialog::refreshRemoteEmbeddingModelField()
{
  const QString providerId = mEmbeddingProvider->currentData().toString();
  const bool remote = QgsAiEmbeddingProviderRegistry::isRemoteProviderId( providerId );
  mRemoteEmbeddingModelRow->setVisible( remote );
  if ( remote )
  {
    QgsSettings settings;
    mRemoteEmbeddingModel->setText( settings.value( remoteEmbeddingModelSettingKey( providerId ), remoteEmbeddingModelDefault( providerId ) ).toString() );
    mRemoteEmbeddingModel->setPlaceholderText( remoteEmbeddingModelDefault( providerId ) );
  }
}

void QgsAiSettingsDialog::refreshIndexStatusLabel()
{
  if ( mSessionManager && mSessionManager->workspaceIndex() )
  {
    mSessionManager->workspaceIndex()->ensureLoaded();
    const auto status = mSessionManager->workspaceIndex()->status();
    mIndexStatusLabel->setText( tr( "Indexed: %1 file chunks, %2 layer chunks (last sync: %3)" )
                                  .arg( status.fileChunkCount )
                                  .arg( status.layerChunkCount )
                                  .arg( status.lastSync.isValid() ? status.lastSync.toLocalTime().toString( Qt::ISODate ) : tr( "never" ) ) );
  }
  else
  {
    mIndexStatusLabel->setText( tr( "Indexed: (workspace index unavailable)" ) );
  }
}

void QgsAiSettingsDialog::refreshCloudIndexStatusLabel()
{
  QList<QgsAiCloudIndexClient::ContextItem> items;
  if ( mSessionManager && mSessionManager->workspaceIndex() )
  {
    mSessionManager->workspaceIndex()->ensureLoaded();
    items += QgsAiCloudIndexClient::contextItemsFromChunks( mSessionManager->workspaceIndex()->chunks() );
  }
  if ( mSessionManager )
  {
    const QgsAiAgentBehaviorSettings behavior = mSessionManager->agentBehaviorSettings();
    items += QgsAiCloudIndexClient::
      contextItemsFromWorkspaceFolders( mSessionManager->workspaceRoot(), behavior.loadWorkspaceRules ? behavior.rulesPath : QString(), behavior.loadWorkspaceSkills ? behavior.skillsPath : QString() );
  }
  items = QgsAiCloudIndexClient::deduplicateContextItems( items );

  if ( items.isEmpty() )
  {
    mCloudIndexStatusLabel->setText( tr( "Cloud sync preview: no safe context items yet." ) );
    return;
  }
  QString validationError;
  if ( !QgsAiCloudIndexClient::validateContextItems( items, &validationError ) )
  {
    mCloudIndexStatusLabel->setText( tr( "Cloud sync preview blocked: %1" ).arg( validationError ) );
    return;
  }
  int layerItems = 0;
  int ruleItems = 0;
  int skillItems = 0;
  int documentItems = 0;
  for ( const QgsAiCloudIndexClient::ContextItem &item : items )
  {
    if ( item.sourceType == "layer"_L1 )
      ++layerItems;
    else if ( item.sourceType == "rule"_L1 )
      ++ruleItems;
    else if ( item.sourceType == "skill"_L1 )
      ++skillItems;
    else if ( item.sourceType == "pdf"_L1 || item.sourceType == "image"_L1 )
      ++documentItems;
  }
  mCloudIndexStatusLabel->setText(
    tr( "Cloud sync preview: %1 items (%2 layer, %3 rule, %4 skill, %5 document)." ).arg( items.size() ).arg( layerItems ).arg( ruleItems ).arg( skillItems ).arg( documentItems )
  );
}

QString QgsAiSettingsDialog::onboardingStatusText() const
{
  const bool planReady = mModelRouter && mModelRouter->isProviderAvailable( QgsAiModelRouter::Provider::Plan );
  const bool byokReady = hasByokProvider( mModelRouter );
  const bool modelReady = mModelRouter && mModelRouter->isProviderUsable( mModelRouter->resolveProvider() );
  const bool indexingReady = ( mAutomaticIndexing && mAutomaticIndexing->isChecked() )
                             || ( mEnableLayerIndexing && mEnableLayerIndexing->isChecked() )
                             || ( mCloudContextOptIn && mCloudContextOptIn->isChecked() );
  QStringList lines;
  lines << tr( "Plan login: %1" ).arg( planReady ? tr( "ready" ) : tr( "not configured" ) );
  lines << tr( "BYOK fallback: %1" ).arg( byokReady ? tr( "ready" ) : tr( "optional" ) );
  lines << tr( "Privacy boundary: %1" ).arg( mPrivacyMetadataOnly && mPrivacyMetadataOnly->isChecked() ? tr( "acknowledged" ) : tr( "needs review" ) );
  lines << tr( "Model route: %1" ).arg( modelReady ? tr( "ready" ) : tr( "choose Plan or BYOK model" ) );
  lines << tr( "Indexing: %1" ).arg( indexingReady ? tr( "enabled" ) : tr( "off" ) );
  lines << tr( "Demo project: %1" ).arg( demoProjectReady() ? tr( "ready" ) : tr( "not created" ) );
  return lines.join( '\n' );
}

void QgsAiSettingsDialog::refreshOnboardingStatus()
{
  if ( mOnboardingStatusLabel )
    mOnboardingStatusLabel->setText( onboardingStatusText() );
}

bool QgsAiSettingsDialog::ensureEmbeddingProvider()
{
  if ( mSessionManager && mSessionManager->workspaceIndex() && mSessionManager->workspaceIndex()->embeddingProviderAvailable() )
    return true;

  QMessageBox::information( this, tr( "Workspace indexing unavailable" ), tr( "Workspace indexing requires the selected embedding provider to be available. Chat with Claude/Codex works without indexing." ) );
  return false;
}

bool QgsAiSettingsDialog::confirmRebuild( const QString &what )
{
  const QString providerId = mEmbeddingProvider->currentData().toString();
  const QString message = QgsAiEmbeddingProviderRegistry::isRemoteProviderId( providerId )
                            ? tr( "Rebuilding the %1 will re-embed content with the remote provider %2. This sends data to that service and may incur API costs.\n\nProceed?" )
                                .arg( what, QgsAiEmbeddingProviderRegistry::displayNameForProviderId( providerId ) )
                            : tr( "Rebuilding the %1 will re-embed content on this computer and may use significant CPU for a while.\n\nProceed?" ).arg( what );
  return QMessageBox::question( this, tr( "Rebuild index" ), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes;
}

void QgsAiSettingsDialog::applySettings()
{
  if ( !mModelRouter )
    return;

  const QString pendingOpenAiKey = mOpenAiKey->text().trimmed();
  const QString pendingOpenRouterKey = mOpenRouterKey->text().trimmed();
  const QString pendingClaudeKey = mClaudeKey->text().trimmed();
  const QString pendingPlanToken = mAccountWidget->manualSessionToken();

  QString errorMessages;
  QString error;

  QgsAiModelRouter::ProviderSettings openAiSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi );
  openAiSettings.endpoint = mOpenAiEndpoint->text().trimmed();
  openAiSettings.model = mOpenAiModel->text().trimmed();
  openAiSettings.enabled = !pendingOpenAiKey.isEmpty() || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi ) || !qEnvironmentVariable( "OPENAI_API_KEY" ).trimmed().isEmpty();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::OpenAi, openAiSettings );

  QgsAiModelRouter::ProviderSettings openRouterSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  openRouterSettings.endpoint = mOpenRouterEndpoint->text().trimmed();
  openRouterSettings.model = mOpenRouterModel->currentText().trimmed();
  openRouterSettings.autoRouting = mOpenRouterAutoRouting->isChecked();
  openRouterSettings.enabled = !pendingOpenRouterKey.isEmpty()
                               || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenRouter )
                               || !qEnvironmentVariable( "OPENROUTER_API_KEY" ).trimmed().isEmpty();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, openRouterSettings );

  QgsAiModelRouter::ProviderSettings codexSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex );
  codexSettings.endpoint = mCodexEndpoint->text().trimmed();
  codexSettings.model = mCodexModel->text().trimmed();
  codexSettings.credentialMode = QgsAiModelRouter::CredentialMode::OAuth;
  codexSettings.enabled = QgsAiCodexOAuthClient::hasRefreshToken();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Codex, codexSettings );

  QgsAiModelRouter::ProviderSettings planSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan );
  planSettings.endpoint = mAccountWidget->planEndpoint();
  planSettings.authConfigId = mAccountWidget->planAuthConfigId();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Plan, planSettings );
  mModelRouter->setPlanAuthConfigId( mAccountWidget->planAuthConfigId() );

  if ( !pendingOpenAiKey.isEmpty() && !mModelRouter->storeApiKey( QgsAiModelRouter::Provider::OpenAi, pendingOpenAiKey, &error ) )
    errorMessages += error + '\n';
  if ( !pendingOpenRouterKey.isEmpty() && !mModelRouter->storeApiKey( QgsAiModelRouter::Provider::OpenRouter, pendingOpenRouterKey, &error ) )
    errorMessages += error + '\n';
  if ( !pendingClaudeKey.isEmpty() && !mModelRouter->storeApiKey( QgsAiModelRouter::Provider::Claude, pendingClaudeKey, &error ) )
    errorMessages += error + '\n';
  if ( !pendingPlanToken.isEmpty() && !mModelRouter->setPlanSessionToken( pendingPlanToken, &error ) )
    errorMessages += error + '\n';

  {
    QgsSettings settings;
    const QString requestedWorkspaceRoot = mWorkspaceRoot->text().trimmed();
    const QString configuredWorkspaceRoot = requestedWorkspaceRoot.isEmpty() ? QString() : QDir::cleanPath( requestedWorkspaceRoot );
    if ( configuredWorkspaceRoot.isEmpty() )
    {
      settings.remove( u"strata/workspace/root"_s );
      settings.remove( u"geoai/workspace/root"_s );
      settings.remove( u"qgis_ai/workspace/root"_s );
    }
    else
    {
      settings.setValue( u"strata/workspace/root"_s, QDir::cleanPath( configuredWorkspaceRoot ) );
      settings.remove( u"geoai/workspace/root"_s );
      settings.remove( u"qgis_ai/workspace/root"_s );
    }

    if ( mSessionManager && QgsProject::instance() && QgsProject::instance()->homePath().isEmpty() )
      mSessionManager->setWorkspaceRoot( configuredWorkspaceRoot );

    // Persist the trust decision only for the workspace represented by the
    // checkbox. Changing the root in this dialog recalculates the checkbox state,
    // so trust is never inherited from a previous root.
    if ( !mTrustRootForCheckbox.isEmpty() )
      QgsAiWorkspaceTrust::setState( mTrustRootForCheckbox, mTrustWorkspace->isChecked() ? QgsAiWorkspaceTrust::State::Trusted : QgsAiWorkspaceTrust::State::Untrusted );

    QgsAiEmbeddingProviderRegistry::setConfiguredProviderId( mEmbeddingProvider->currentData().toString() );
    if ( QgsAiEmbeddingProviderRegistry::isRemoteProviderId( mEmbeddingProvider->currentData().toString() ) )
    {
      const QString embeddingModelKey = remoteEmbeddingModelSettingKey( mEmbeddingProvider->currentData().toString() );
      const QString embeddingModelValue = mRemoteEmbeddingModel->text().trimmed();
      if ( embeddingModelValue.isEmpty() )
        settings.remove( embeddingModelKey );
      else
        settings.setValue( embeddingModelKey, embeddingModelValue );
    }
    settings.setValue( u"strata/index/automatic"_s, mAutomaticIndexing->isChecked() );
    settings.setValue( u"strata/index/cloud_context_opt_in"_s, mCloudContextOptIn->isChecked() );
    settings.setValue( u"strata/privacy/metadata_only_ack"_s, mPrivacyMetadataOnly->isChecked() );
    settings.setValue( u"strata/telemetry/opt_in"_s, mTelemetryOptIn->isChecked() );
    settings.setValue( u"strata/crash_reporting/metadata_only_opt_in"_s, mCrashReportOptIn->isChecked() );
  }

  emit embeddingProviderSettingsChanged();

  QgsAiModelRouter::ProviderSettings claudeSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude );
  claudeSettings.endpoint = mClaudeEndpoint->text().trimmed();
  claudeSettings.model = mClaudeModel->text().trimmed();
  const bool claudeOAuthRequested = mClaudeUseOAuth->isChecked();
  const bool claudeOAuthAvailable = QgsAiClaudeOAuthClient::hasRefreshToken();
  const bool claudeApiKeyAvailable = !pendingClaudeKey.isEmpty() || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude );
  if ( claudeOAuthRequested && claudeOAuthAvailable )
  {
    claudeSettings.credentialMode = QgsAiModelRouter::CredentialMode::OAuth;
    claudeSettings.enabled = true;
  }
  else
  {
    if ( claudeOAuthRequested && !claudeOAuthAvailable )
    {
      errorMessages += ( claudeApiKeyAvailable ? tr( "Claude OAuth login was not completed; Claude will keep using API key mode." )
                                               : tr( "Claude OAuth login was not completed; Claude is disabled until you complete login or configure an API key." ) )
                       + '\n';
    }
    claudeSettings.credentialMode = QgsAiModelRouter::CredentialMode::ApiKey;
    claudeSettings.enabled = claudeApiKeyAvailable;
  }
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Claude, claudeSettings );

  QgsSettings gisSaveSettings;
  gisSaveSettings.setValue( QgsAiGisSuggestionEngine::globalEnabledSettingsKey(), mGisSuggestionsEnabled->isChecked() );
  gisSaveSettings.setValue( gisProjectSettingsKey(), mGisSuggestionsProjectEnabled->isChecked() );

  if ( mSessionManager )
  {
    QgsAiAgentBehaviorSettings behaviorSettings = mSessionManager->agentBehaviorSettings();
    behaviorSettings.allowCustomActions = mAllowCustomActions->isChecked();
    behaviorSettings.rulesText = mRulesEdit->toPlainText();
    behaviorSettings.skillsText = mSkillsEdit->toPlainText();
    behaviorSettings.loadWorkspaceRules = mLoadWorkspaceRules->isChecked();
    behaviorSettings.loadWorkspaceSkills = mLoadWorkspaceSkills->isChecked();
    behaviorSettings.rulesPath = mRulesPathEdit->text().trimmed();
    behaviorSettings.skillsPath = mSkillsPathEdit->text().trimmed();
    mSessionManager->setAgentBehaviorSettings( behaviorSettings );

    const bool canUseEmbeddings = mSessionManager->workspaceIndex() && mSessionManager->workspaceIndex()->embeddingProviderAvailable();
    bool layerIndexingChoice = mEnableLayerIndexing->isChecked() && canUseEmbeddings;

    if ( mEnableLayerIndexing->isChecked() && !canUseEmbeddings )
    {
      errorMessages += tr( "Layer indexing requires the selected embedding provider to be available." ) + '\n';
      mEnableLayerIndexing->setChecked( false );
    }

    // Gate first-time layer indexing so users explicitly acknowledge the local
    // background work and local cache before it starts.
    if ( layerIndexingChoice && QgsAiChatDockWidget::requiresLayerIndexingConsent() )
    {
      const auto choice = QMessageBox::question(
        this,
        tr( "Enable layer indexing" ),
        QgsAiEmbeddingProviderRegistry::isRemoteProviderId( mEmbeddingProvider->currentData().toString() )
          ? tr( "Enabling layer indexing means Strata will send sampled layer attributes and bounding boxes to the selected remote embedding provider and store the resulting index locally.\n\nProceed?" )
          : tr( "Enabling layer indexing means Strata will process sampled layer attributes and bounding boxes on this computer and store the resulting index locally.\n\nProceed?" ),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
      );
      if ( choice != QMessageBox::Yes )
      {
        layerIndexingChoice = false;
        mEnableLayerIndexing->setChecked( false );
      }
      else
      {
        QgsAiChatDockWidget::recordLayerIndexingConsent();
      }
    }

    QgsSettings layerSettings;
    layerSettings.setValue( u"strata/index/enable_layer_indexing"_s, layerIndexingChoice );
    layerSettings.remove( u"geoai/index/enable_layer_indexing"_s );
    layerSettings.remove( u"qgis_ai/index/enable_layer_indexing"_s );
    if ( mLayerIndexCoordinator )
    {
      mLayerIndexCoordinator->setEnabled( layerIndexingChoice && canUseEmbeddings );
      if ( layerIndexingChoice && canUseEmbeddings )
        mLayerIndexCoordinator->scheduleAllLayers();
    }
  }

  if ( !errorMessages.isEmpty() )
    QMessageBox::warning( this, tr( "Provider configuration warnings" ), errorMessages.trimmed() );
}
