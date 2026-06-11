/***************************************************************************
    qgsaichatdockwidget.cpp
    ---------------------
    begin                : April 2026
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

#include "qgsaichatdockwidget.h"

#include <algorithm>
#include <utility>

#include "ai/index/qgsaiembeddingprovider.h"
#include "ai/index/qgsailayerindexcoordinator.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "qgisapp.h"
#include "qgsaiagentsessionmanager.h"
#include "qgsaiclaudeoauthclient.h"
#include "qgsaicodexoauthclient.h"
#include "qgsaimodelrouter.h"
#include "qgsaiopenroutermodelcatalog.h"
#include "qgsaireviewpatchengine.h"
#include "qgsaisecretstore.h"
#include "qgsaiworkspacetrust.h"
#include "qgsapplication.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsscrollarea.h"
#include "qgssettings.h"
#include "qgstaskmanager.h"

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QCompleter>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QKeyEvent>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPalette>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollBar>
#include <QSet>
#include <QSize>
#include <QSizePolicy>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextOption>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>

#include "moc_qgsaichatdockwidget.cpp"

using namespace Qt::StringLiterals;

namespace
{
  struct ModelEntry
  {
      QString displayName;
      QString model;
      QgsAiModelRouter::Provider provider;
  };

  QVector<ModelEntry> predefinedModels()
  {
    return {
      { u"GPT-4o"_s, u"gpt-4o"_s, QgsAiModelRouter::Provider::OpenAi },
      { u"GPT-4.1 mini"_s, u"gpt-4.1-mini"_s, QgsAiModelRouter::Provider::OpenAi },
      { u"Claude Sonnet 4.6 (OpenRouter)"_s, u"anthropic/claude-sonnet-4.6"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"OpenRouter Auto"_s, u"openrouter/auto"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"Codex GPT-5.4"_s, u"gpt-5.4"_s, QgsAiModelRouter::Provider::Codex },
      { u"Codex GPT-5.3 (codex)"_s, u"gpt-5.3-codex"_s, QgsAiModelRouter::Provider::Codex },
      { u"Claude Sonnet 4"_s, u"claude-sonnet-4-20250514"_s, QgsAiModelRouter::Provider::Claude },
      { u"Claude Sonnet 3.7"_s, u"claude-3-7-sonnet-20250219"_s, QgsAiModelRouter::Provider::Claude },
      { u"Claude Opus 4.1"_s, u"claude-opus-4-1-20250805"_s, QgsAiModelRouter::Provider::Claude },
      { u"Plan backend"_s, u"managed-plan"_s, QgsAiModelRouter::Provider::Plan },
    };
  }

  QString humanBytes( qint64 bytes )
  {
    if ( bytes < 1024 )
      return u"%1 B"_s.arg( bytes );
    const double kb = bytes / 1024.0;
    if ( kb < 1024 )
      return u"%1 KiB"_s.arg( kb, 0, 'f', 1 );
    const double mb = kb / 1024.0;
    if ( mb < 1024 )
      return u"%1 MiB"_s.arg( mb, 0, 'f', 1 );
    return u"%1 GiB"_s.arg( mb / 1024.0, 0, 'f', 2 );
  }

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

  void applyTranscriptWidthPolicy( QWidget *widget )
  {
    if ( !widget )
      return;

    QSizePolicy policy = widget->sizePolicy();
    policy.setHorizontalPolicy( QSizePolicy::Ignored );
    policy.setHorizontalStretch( 1 );
    widget->setSizePolicy( policy );
    widget->setMinimumWidth( 0 );
  }

  void applyTranscriptTextEditWrapping( QTextEdit *edit )
  {
    if ( !edit )
      return;

    applyTranscriptWidthPolicy( edit );
    edit->setLineWrapMode( QTextEdit::WidgetWidth );
    edit->setWordWrapMode( QTextOption::WrapAnywhere );
    edit->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  }

  QString modeLabelToAgent( const QString &label )
  {
    if ( label == "Plan"_L1 )
      return u"planner"_s;
    if ( label == "Agent"_L1 )
      return u"editor"_s;
    if ( label == "Ask"_L1 )
      return u"reviewer"_s;
    return u"planner"_s;
  }

  QString agentToModeLabel( const QString &agent )
  {
    if ( agent == "planner"_L1 )
      return u"Plan"_s;
    if ( agent == "editor"_L1 )
      return u"Agent"_s;
    if ( agent == "reviewer"_L1 )
      return u"Ask"_s;
    return u"Plan"_s;
  }

  template<typename Settings> QVariant settingValueWithLegacy( Settings &settings, const QString &key, const QStringList &legacyKeys, const QVariant &defaultValue )
  {
    if ( settings.contains( key ) )
      return settings.value( key, defaultValue );
    for ( const QString &legacyKey : legacyKeys )
    {
      if ( settings.contains( legacyKey ) )
        return settings.value( legacyKey, defaultValue );
    }
    return defaultValue;
  }

  QString truncateForTranscript( const QString &text, int maxChars = 4000 )
  {
    if ( text.size() <= maxChars )
      return text;
    return text.left( maxChars ) + u"\n...[truncated]"_s;
  }

  QString markdownCodeBlock( const QString &label, const QString &text, const QString &language = QString() )
  {
    if ( text.isEmpty() )
      return QString();
    return u"\n%1\n```%2\n%3\n```\n"_s.arg( label, language, truncateForTranscript( text ) );
  }

  QJsonObject jsonObjectFromMessageContent( const QString &content )
  {
    const QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8() );
    return doc.isObject() ? doc.object() : QJsonObject();
  }

  QString scalarForTranscript( const QJsonValue &value )
  {
    if ( value.isString() )
      return value.toString();
    if ( value.isDouble() )
      return QString::number( value.toDouble(), 'g', 12 );
    if ( value.isBool() )
      return value.toBool() ? u"true"_s : u"false"_s;
    if ( value.isNull() || value.isUndefined() )
      return u"null"_s;
    return QString::fromUtf8( QJsonDocument( value.toObject() ).toJson( QJsonDocument::Compact ) );
  }

  QString urlHostForTranscript( const QString &urlText )
  {
    const QUrl url( urlText );
    if ( url.isValid() && !url.host().isEmpty() )
      return url.host();
    return urlText;
  }

  QString relativePathForTranscript( const QString &workspaceRoot, const QString &path )
  {
    if ( workspaceRoot.isEmpty() || path.isEmpty() )
      return path;
    const QString relative = QDir( workspaceRoot ).relativeFilePath( path );
    if ( relative == "."_L1 || relative.startsWith( "../"_L1 ) || relative == ".."_L1 || QDir::isAbsolutePath( relative ) )
      return path;
    return relative;
  }

  struct TechnicalSection
  {
      QString title;
      QString content;
      QString language;
  };

  struct RenderedMessageContent
  {
      QString markdown;
      QList<TechnicalSection> technicalSections;
  };

  QString removeQuestionsProtocolBlocks( const QString &text )
  {
    QString stripped = text;
    static const QRegularExpression questionsRe( u"```qgis_ai_questions\\s*\\n[\\s\\S]*?\\n```"_s, QRegularExpression::CaseInsensitiveOption );
    stripped.remove( questionsRe );
    return stripped.trimmed();
  }

  QString planMarkdownFromMetadata( const QVariantMap &metadata )
  {
    return metadata.value( u"plan_markdown"_s ).toString().trimmed();
  }

  QJsonObject questionsPayloadFromMetadata( const QVariantMap &metadata )
  {
    const QString json = metadata.value( u"questions_json"_s ).toString();
    if ( json.isEmpty() )
      return QJsonObject();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson( json.toUtf8(), &parseError );
    if ( parseError.error != QJsonParseError::NoError || !doc.isObject() )
      return QJsonObject();
    return doc.object();
  }

  int balancedJsonEnd( const QString &text, int start )
  {
    QList<QChar> stack;
    bool inString = false;
    bool escape = false;

    for ( int i = start; i < text.size(); ++i )
    {
      const QChar ch = text.at( i );
      if ( inString )
      {
        if ( escape )
        {
          escape = false;
        }
        else if ( ch == '\\'_L1 )
        {
          escape = true;
        }
        else if ( ch == '"'_L1 )
        {
          inString = false;
        }
        continue;
      }

      if ( ch == '"'_L1 )
      {
        inString = true;
        continue;
      }

      if ( ch == '{'_L1 )
      {
        stack << '}'_L1;
        continue;
      }
      if ( ch == '['_L1 )
      {
        stack << ']'_L1;
        continue;
      }

      if ( ch == '}'_L1 || ch == ']'_L1 )
      {
        if ( stack.isEmpty() || stack.takeLast() != ch )
          return -1;
        if ( stack.isEmpty() )
          return i;
      }
    }

    return -1;
  }

  QString extractLargeJsonBlocks( const QString &text, QList<TechnicalSection> &sections )
  {
    QString visible;
    int last = 0;
    int i = 0;
    while ( i < text.size() )
    {
      const QChar ch = text.at( i );
      if ( ch != '{'_L1 && ch != '['_L1 )
      {
        ++i;
        continue;
      }

      const int end = balancedJsonEnd( text, i );
      if ( end <= i )
      {
        ++i;
        continue;
      }

      const QString candidate = text.mid( i, end - i + 1 );
      QJsonParseError parseError;
      const QJsonDocument doc = QJsonDocument::fromJson( candidate.toUtf8(), &parseError );
      if ( parseError.error == QJsonParseError::NoError && candidate.size() > 160 )
      {
        visible += text.mid( last, i - last );
        visible += "\n\n_Technical JSON hidden below._\n\n"_L1;
        TechnicalSection section;
        section.title = QObject::tr( "JSON" );
        section.language = u"json"_s;
        section.content = QString::fromUtf8( doc.toJson( QJsonDocument::Indented ) );
        sections << section;
        i = end + 1;
        last = i;
        continue;
      }

      ++i;
    }

    visible += text.mid( last );
    return visible;
  }

  RenderedMessageContent splitTechnicalContent( const QString &content )
  {
    RenderedMessageContent result;
    QString visible;

    static const QRegularExpression fenceRe( u"```([A-Za-z0-9_+.#-]*)\\s*\\n([\\s\\S]*?)\\n```"_s );
    QRegularExpressionMatchIterator it = fenceRe.globalMatch( content );
    int lastEnd = 0;
    int codeIndex = 1;
    while ( it.hasNext() )
    {
      const QRegularExpressionMatch match = it.next();
      visible += content.mid( lastEnd, match.capturedStart() - lastEnd );

      const QString language = match.captured( 1 ).trimmed();
      if ( language != "qgis_ai_questions"_L1 )
      {
        TechnicalSection section;
        section.language = language;
        section.content = match.captured( 2 );
        section.title = language.isEmpty() ? QObject::tr( "Code %1" ).arg( codeIndex ) : QObject::tr( "Code %1 (%2)" ).arg( codeIndex ).arg( language );
        result.technicalSections << section;
        ++codeIndex;
      }
      lastEnd = match.capturedEnd();
    }
    visible += content.mid( lastEnd );
    visible = extractLargeJsonBlocks( visible, result.technicalSections );

    result.markdown = visible.trimmed();
    return result;
  }

  QString roleDisplayName( QgsAiChatRole role, const QString &fallback )
  {
    switch ( role )
    {
      case QgsAiChatRole::User:
        return QObject::tr( "User" );
      case QgsAiChatRole::Assistant:
        return QObject::tr( "Assistant" );
      case QgsAiChatRole::Tool:
        return QObject::tr( "Tool" );
      case QgsAiChatRole::System:
        return QObject::tr( "System" );
    }
    return fallback;
  }
} //namespace

Q_DECLARE_METATYPE( ModelEntry )

QgsAiChatDockWidget::QgsAiChatDockWidget( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiReviewPatchEngine *reviewEngine, QWidget *parent )
  : QgsDockWidget( tr( "AI Assistant" ), parent )
  , mSessionManager( sessionManager )
  , mModelRouter( modelRouter )
  , mReviewEngine( reviewEngine )
{
  setObjectName( u"AiAssistant"_s );

  QWidget *container = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( container );
  layout->setContentsMargins( 8, 8, 8, 8 );
  layout->setSpacing( 6 );

  QHBoxLayout *topBar = new QHBoxLayout();
  topBar->setContentsMargins( 0, 0, 0, 0 );
  topBar->setSpacing( 4 );

  mNewChatButton = new QToolButton( container );
  mNewChatButton->setObjectName( u"aiNewChatButton"_s );
  mNewChatButton->setText( tr( "+ New" ) );
  mNewChatButton->setToolTip( tr( "Start a new chat" ) );
  mNewChatButton->setAutoRaise( true );
  topBar->addWidget( mNewChatButton );

  mHistoryButton = new QToolButton( container );
  mHistoryButton->setObjectName( u"aiHistoryButton"_s );
  mHistoryButton->setText( tr( "History" ) + u" ▾"_s );
  mHistoryButton->setToolTip( tr( "Past chats in this project" ) );
  mHistoryButton->setAutoRaise( true );
  mHistoryButton->setPopupMode( QToolButton::InstantPopup );
  mHistoryButton->setMenu( new QMenu( mHistoryButton ) );
  topBar->addWidget( mHistoryButton );

  topBar->addStretch( 1 );
  layout->addLayout( topBar );

  mTranscriptScrollArea = new QgsScrollArea( container );
  mTranscriptScrollArea->setObjectName( u"aiTranscriptScrollArea"_s );
  mTranscriptScrollArea->setWidgetResizable( true );
  mTranscriptScrollArea->setFrameShape( QFrame::NoFrame );
  mTranscriptScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mTranscriptScrollArea->setSizeAdjustPolicy( QAbstractScrollArea::AdjustIgnored );
  mTranscriptScrollArea->setMinimumWidth( 0 );

  mTranscriptContainer = new QWidget( mTranscriptScrollArea );
  mTranscriptContainer->setObjectName( u"aiTranscriptContainer"_s );
  applyTranscriptWidthPolicy( mTranscriptContainer );
  mTranscriptLayout = new QVBoxLayout( mTranscriptContainer );
  mTranscriptLayout->setContentsMargins( 0, 0, 0, 0 );
  mTranscriptLayout->setSpacing( 8 );
  mTranscriptLayout->addStretch( 1 );
  mTranscriptScrollArea->setWidget( mTranscriptContainer );
  layout->addWidget( mTranscriptScrollArea, 1 );

  mFileContextChipRow = new QWidget( container );
  mFileContextChipRow->setObjectName( u"aiAttachmentChipRow"_s );
  mFileContextChipLayout = new QHBoxLayout( mFileContextChipRow );
  mFileContextChipLayout->setContentsMargins( 0, 0, 0, 0 );
  mFileContextChipLayout->setSpacing( 4 );
  mFileContextChipLayout->addStretch( 1 );
  mFileContextChipRow->setVisible( false );
  layout->addWidget( mFileContextChipRow );

  mInputTextEdit = new QTextEdit( container );
  mInputTextEdit->setObjectName( u"aiPromptInput"_s );
  mInputTextEdit->setPlaceholderText( tr( "Ask a question, tag project files with @, or send /patch…  (Shift+Enter for newline)" ) );
  mInputTextEdit->setAcceptRichText( false );
  mInputTextEdit->setTabChangesFocus( true );
  mInputTextEdit->setFixedHeight( 72 );
  mInputTextEdit->setFrameShape( QFrame::NoFrame );
  mInputTextEdit->installEventFilter( this );
  layout->addWidget( mInputTextEdit );

  mMentionPopup = new QFrame( this, Qt::Popup );
  mMentionPopup->setObjectName( u"aiMentionPopup"_s );
  mMentionPopup->setFrameShape( QFrame::NoFrame );
  QVBoxLayout *mentionLayout = new QVBoxLayout( mMentionPopup );
  mentionLayout->setContentsMargins( 0, 0, 0, 0 );
  mMentionList = new QListWidget( mMentionPopup );
  mMentionList->setObjectName( u"aiMentionList"_s );
  mMentionList->setFrameShape( QFrame::NoFrame );
  mMentionList->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mMentionList->setUniformItemSizes( true );
  mentionLayout->addWidget( mMentionList );
  mMentionPopup->hide();

  QHBoxLayout *bottomBar = new QHBoxLayout();
  bottomBar->setContentsMargins( 0, 0, 0, 0 );
  bottomBar->setSpacing( 6 );

  mModePill = new QToolButton( container );
  mModePill->setObjectName( u"aiModePill"_s );
  mModePill->setPopupMode( QToolButton::InstantPopup );
  mModePill->setToolButtonStyle( Qt::ToolButtonTextOnly );
  mModePill->setAutoRaise( true );
  bottomBar->addWidget( mModePill );

  mModelPill = new QToolButton( container );
  mModelPill->setObjectName( u"aiModelPill"_s );
  mModelPill->setPopupMode( QToolButton::InstantPopup );
  mModelPill->setToolButtonStyle( Qt::ToolButtonTextOnly );
  mModelPill->setAutoRaise( true );
  bottomBar->addWidget( mModelPill );

  bottomBar->addStretch( 1 );

  mAttachButton = new QToolButton( container );
  mAttachButton->setObjectName( u"aiAttachFileButton"_s );
  mAttachButton->setIcon( QgsApplication::getThemeIcon( u"mEditorWidgetAttachment.svg"_s ) );
  mAttachButton->setAutoRaise( true );
  mAttachButton->setFixedSize( 28, 28 );
  mAttachButton->setToolTip( tr( "Attach external files" ) );
  bottomBar->addWidget( mAttachButton );

  mSettingsButton = new QToolButton( container );
  mSettingsButton->setObjectName( u"aiProviderSettingsButton"_s );
  mSettingsButton->setIcon( QgsApplication::getThemeIcon( u"mActionOptions.svg"_s ) );
  mSettingsButton->setAutoRaise( true );
  mSettingsButton->setFixedSize( 28, 28 );
  mSettingsButton->setToolTip( tr( "Provider settings" ) );
  bottomBar->addWidget( mSettingsButton );

  mSendButton = new QToolButton( container );
  mSendButton->setObjectName( u"aiSendButton"_s );
  mSendButton->setText( u"↑"_s );
  mSendButton->setToolTip( tr( "Send (Enter)" ) );
  mSendButton->setFixedSize( 28, 28 );
  bottomBar->addWidget( mSendButton );

  layout->addLayout( bottomBar );

  mRuntimeStatusLabel = new QLabel( container );
  mRuntimeStatusLabel->setObjectName( u"aiRuntimeStatusLabel"_s );
  mRuntimeStatusLabel->setText( tr( "Provider state: idle - Ready." ) );

  QHBoxLayout *statusRow = new QHBoxLayout();
  statusRow->setContentsMargins( 0, 0, 0, 0 );
  statusRow->setSpacing( 6 );
  statusRow->addWidget( mRuntimeStatusLabel, 1 );

  mCancelButton = new QPushButton( tr( "Cancel" ), container );
  mCancelButton->setObjectName( u"aiCancelRequestButton"_s );
  mCancelButton->setEnabled( false );
  statusRow->addWidget( mCancelButton );
  layout->addLayout( statusRow );

  mReviewContainer = new QWidget( container );
  QVBoxLayout *reviewLayout = new QVBoxLayout( mReviewContainer );
  reviewLayout->setContentsMargins( 0, 0, 0, 0 );
  reviewLayout->addWidget( new QLabel( tr( "Review Proposals" ), mReviewContainer ) );
  mProposalList = new QListWidget( mReviewContainer );
  mProposalList->setFrameShape( QFrame::NoFrame );
  reviewLayout->addWidget( mProposalList, 2 );

  QHBoxLayout *reviewButtons = new QHBoxLayout();
  QPushButton *previewButton = new QPushButton( tr( "Preview" ), mReviewContainer );
  QPushButton *acceptButton = new QPushButton( tr( "Accept" ), mReviewContainer );
  QPushButton *acceptPartialButton = new QPushButton( tr( "Accept Partial" ), mReviewContainer );
  QPushButton *rejectButton = new QPushButton( tr( "Reject" ), mReviewContainer );
  reviewButtons->addWidget( previewButton );
  reviewButtons->addWidget( acceptButton );
  reviewButtons->addWidget( acceptPartialButton );
  reviewButtons->addWidget( rejectButton );
  reviewLayout->addLayout( reviewButtons );

  mReviewStatusLabel = new QLabel( mReviewContainer );
  reviewLayout->addWidget( mReviewStatusLabel );

  layout->addWidget( mReviewContainer );

  setWidget( container );

  initModeMenu();
  initModelMenu();
  applyPillStyling();

  if ( mSessionManager )
  {
    const QString initialLabel = agentToModeLabel( mSessionManager->activeAgent() );
    mModePill->setText( initialLabel + u" ▾"_s );
    const QList<QAction *> modeActions = mModePill->menu()->actions();
    for ( QAction *a : modeActions )
    {
      if ( a->text() == initialLabel )
        a->setChecked( true );
    }

    connect( mSessionManager, &QgsAiAgentSessionManager::messageAdded, this, [this]( const QgsAiChatMessage &message ) {
      if ( mStreamingInProgress && message.role == QgsAiChatRole::Assistant )
      {
        if ( mStreamingTextEdit )
        {
          QWidget *streamingWidget = mStreamingTextEdit->parentWidget();
          mStreamingTextEdit = nullptr;
          if ( streamingWidget )
            streamingWidget->deleteLater();
        }
        closeStreamingAssistantMessage();
        appendTranscriptMessage( message );
        return;
      }
      closeStreamingAssistantMessage();
      appendTranscriptMessage( message );
    } );
    connect( mSessionManager, &QgsAiAgentSessionManager::proposalCreated, this, [this]( const QString & ) { refreshProposalList(); } );
    connect( mSessionManager, &QgsAiAgentSessionManager::responseChunkReceived, this, &QgsAiChatDockWidget::appendStreamChunk );
    connect( mSessionManager, &QgsAiAgentSessionManager::requestStateChanged, this, &QgsAiChatDockWidget::updateRuntimeState );
    connect( mSessionManager, &QgsAiAgentSessionManager::requestRunningChanged, this, &QgsAiChatDockWidget::setRequestRunning );
    connect( mSessionManager, &QgsAiAgentSessionManager::historyReplaced, this, &QgsAiChatDockWidget::reloadTranscriptFromHistory );
    connect( mSessionManager, &QgsAiAgentSessionManager::sessionListChanged, this, &QgsAiChatDockWidget::rebuildHistoryMenu );
  }

  if ( mReviewEngine )
  {
    connect( mReviewEngine, &QgsAiReviewPatchEngine::proposalAdded, this, [this]( const QString & ) { refreshProposalList(); } );
    connect( mReviewEngine, &QgsAiReviewPatchEngine::proposalAccepted, this, [this]( const QString & ) { refreshProposalList(); } );
    connect( mReviewEngine, &QgsAiReviewPatchEngine::proposalRejected, this, [this]( const QString & ) { refreshProposalList(); } );
  }

  connect( mNewChatButton, &QToolButton::clicked, this, &QgsAiChatDockWidget::onNewChatClicked );
  connect( mHistoryButton, &QToolButton::clicked, this, &QgsAiChatDockWidget::rebuildHistoryMenu );
  connect( mHistoryButton->menu(), &QMenu::triggered, this, &QgsAiChatDockWidget::onHistoryEntryTriggered );
  connect( mAttachButton, &QToolButton::clicked, this, &QgsAiChatDockWidget::attachFile );
  connect( mInputTextEdit, &QTextEdit::textChanged, this, &QgsAiChatDockWidget::updateMentionPopup );
  connect( mMentionList, &QListWidget::itemActivated, this, [this]( QListWidgetItem *item ) {
    if ( item )
      insertMentionFile( item->data( Qt::UserRole ).toString() );
  } );
  connect( mSendButton, &QToolButton::clicked, this, &QgsAiChatDockWidget::onSendOrStopClicked );
  connect( mCancelButton, &QPushButton::clicked, this, &QgsAiChatDockWidget::cancelRunningRequest );
  connect( mSettingsButton, &QToolButton::clicked, this, &QgsAiChatDockWidget::openProviderSettings );
  connect( previewButton, &QPushButton::clicked, this, &QgsAiChatDockWidget::previewProposal );
  connect( acceptButton, &QPushButton::clicked, this, &QgsAiChatDockWidget::acceptProposal );
  connect( acceptPartialButton, &QPushButton::clicked, this, &QgsAiChatDockWidget::acceptPartialProposal );
  connect( rejectButton, &QPushButton::clicked, this, &QgsAiChatDockWidget::rejectProposal );

  setRequestRunning( false );
  refreshProposalList();
}

bool QgsAiChatDockWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mInputTextEdit && event->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    if ( mMentionPopup && mMentionPopup->isVisible() )
    {
      if ( keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up )
      {
        const int row = mMentionList->currentRow();
        const int nextRow = keyEvent->key() == Qt::Key_Down ? std::min( row + 1, mMentionList->count() - 1 ) : std::max( row - 1, 0 );
        mMentionList->setCurrentRow( nextRow );
        return true;
      }
      if ( keyEvent->key() == Qt::Key_Escape )
      {
        hideMentionPopup();
        return true;
      }
      if ( keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter )
      {
        insertSelectedMention();
        return true;
      }
    }

    if ( ( keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter ) && !( keyEvent->modifiers() & Qt::ShiftModifier ) )
    {
      sendMessage();
      return true;
    }
  }
  return QgsDockWidget::eventFilter( watched, event );
}

void QgsAiChatDockWidget::initModeMenu()
{
  QMenu *menu = new QMenu( mModePill );
  QActionGroup *group = new QActionGroup( menu );
  group->setExclusive( true );
  const QStringList labels = { u"Plan"_s, u"Agent"_s, u"Ask"_s };
  for ( const QString &label : labels )
  {
    QAction *action = menu->addAction( label );
    action->setCheckable( true );
    group->addAction( action );
  }
  mModePill->setMenu( menu );
  mModePill->setText( u"Plan ▾"_s );
  connect( group, &QActionGroup::triggered, this, &QgsAiChatDockWidget::onModeSelected );
}

void QgsAiChatDockWidget::initModelMenu()
{
  QMenu *menu = new QMenu( mModelPill );
  QActionGroup *group = new QActionGroup( menu );
  group->setExclusive( true );

  const QVector<ModelEntry> models = predefinedModels();
  QgsAiModelRouter::Provider currentSection = QgsAiModelRouter::Provider::OpenAi;
  bool first = true;
  for ( const ModelEntry &entry : models )
  {
    if ( first || entry.provider != currentSection )
    {
      QString header;
      switch ( entry.provider )
      {
        case QgsAiModelRouter::Provider::OpenAi:
          header = tr( "OpenAI" );
          break;
        case QgsAiModelRouter::Provider::OpenRouter:
          header = tr( "OpenRouter" );
          break;
        case QgsAiModelRouter::Provider::Codex:
          header = tr( "Codex / ChatGPT" );
          break;
        case QgsAiModelRouter::Provider::Claude:
          header = tr( "Anthropic" );
          break;
        case QgsAiModelRouter::Provider::Plan:
          header = tr( "Plan backend" );
          break;
      }
      menu->addSection( header );
      currentSection = entry.provider;
      first = false;
    }
    QAction *action = menu->addAction( entry.displayName );
    action->setCheckable( true );
    action->setData( QVariant::fromValue( entry ) );
    group->addAction( action );
  }

  mModelPill->setMenu( menu );

  QString initialDisplay = models.isEmpty() ? QString() : models.first().displayName;
  if ( mModelRouter )
  {
    const QgsAiModelRouter::Provider currentProvider = mModelRouter->resolveProvider();
    const QString currentModel = mModelRouter->providerSettings( currentProvider ).model;
    for ( QAction *action : menu->actions() )
    {
      if ( !action->isCheckable() )
        continue;
      const ModelEntry entry = action->data().value<ModelEntry>();
      if ( entry.provider == currentProvider && entry.model == currentModel )
      {
        action->setChecked( true );
        initialDisplay = entry.displayName;
        break;
      }
    }
  }
  mModelPill->setText( initialDisplay + u" ▾"_s );

  connect( group, &QActionGroup::triggered, this, &QgsAiChatDockWidget::onModelSelected );
}

void QgsAiChatDockWidget::applyPillStyling()
{
  const QString pillStyle = QStringLiteral(
    "QToolButton { color: palette(window-text); background: palette(button); border: 0; border-radius: 10px; padding: 3px 10px; } "
    "QToolButton::menu-indicator { image: none; width: 0; } "
    "QToolButton:hover { background: palette(alternate-base); } "
    "QToolButton:pressed, QToolButton:checked { background: palette(highlight); color: palette(highlighted-text); }"
  );
  mNewChatButton->setStyleSheet( pillStyle );
  mHistoryButton->setStyleSheet( pillStyle );
  mModePill->setStyleSheet( pillStyle );
  mModelPill->setStyleSheet( pillStyle );

  const QString iconButtonStyle = QStringLiteral(
    "QToolButton { color: palette(window-text); background: transparent; border: 0; border-radius: 14px; padding: 2px; } "
    "QToolButton:hover { background: palette(alternate-base); } "
    "QToolButton:pressed { background: palette(highlight); color: palette(highlighted-text); }"
  );
  mAttachButton->setStyleSheet( iconButtonStyle );
  mSettingsButton->setStyleSheet( iconButtonStyle );

  mSendButton->setStyleSheet( QStringLiteral(
    "QToolButton { border: 0; border-radius: 14px; background: palette(highlight); color: palette(highlighted-text); font-weight: 600; } "
    "QToolButton:hover { background: palette(highlight); } "
    "QToolButton:disabled { background: palette(button); color: palette(mid); }"
  ) );

  mInputTextEdit->setStyleSheet( QStringLiteral(
    "QTextEdit#aiPromptInput { color: palette(text); background: palette(base); border: 0; border-radius: 8px; padding: 7px; selection-background-color: palette(highlight); selection-color: "
    "palette(highlighted-text); } "
    "QTextEdit#aiPromptInput:focus { background: palette(base); } "
    "QTextEdit#aiPromptInput:disabled { color: palette(mid); background: palette(alternate-base); }"
  ) );
  mTranscriptScrollArea->setStyleSheet( QStringLiteral(
    "QgsScrollArea#aiTranscriptScrollArea { background: palette(window); border: 0; } "
    "QWidget#aiTranscriptContainer { background: palette(window); }"
  ) );
  mMentionPopup->setStyleSheet( QStringLiteral( "QFrame#aiMentionPopup { background: palette(base); border: 0; border-radius: 8px; }" ) );
  mMentionList->setStyleSheet( QStringLiteral(
    "QListWidget#aiMentionList { color: palette(text); background: palette(base); border: 0; } "
    "QListWidget#aiMentionList::item { padding: 4px 8px; border-radius: 4px; } "
    "QListWidget#aiMentionList::item:selected { color: palette(highlighted-text); background: palette(highlight); } "
    "QListWidget#aiMentionList::item:hover { background: palette(alternate-base); }"
  ) );
  mRuntimeStatusLabel->setStyleSheet( u"QLabel#aiRuntimeStatusLabel { color: palette(mid); }"_s );
  mCancelButton->setStyleSheet( QStringLiteral(
    "QPushButton#aiCancelRequestButton { color: palette(window-text); background: palette(button); border: 0; border-radius: 6px; padding: 3px 9px; } "
    "QPushButton#aiCancelRequestButton:hover:enabled { background: palette(alternate-base); } "
    "QPushButton#aiCancelRequestButton:disabled { color: palette(mid); background: palette(window); }"
  ) );
  mReviewContainer->setStyleSheet( QStringLiteral(
    "QWidget { background: palette(window); } "
    "QListWidget { color: palette(text); background: palette(base); border: 0; } "
    "QListWidget::item { padding: 3px 4px; border: 0; } "
    "QListWidget::item:selected { color: palette(highlighted-text); background: palette(highlight); } "
    "QPushButton { color: palette(window-text); background: palette(button); border: 0; border-radius: 6px; padding: 4px 8px; } "
    "QPushButton:hover:enabled { background: palette(alternate-base); } "
    "QPushButton:pressed { background: palette(highlight); color: palette(highlighted-text); }"
  ) );
}

QString QgsAiChatDockWidget::renderMarkdown( const QString &md )
{
  QTextDocument doc;
  QTextOption textOption = doc.defaultTextOption();
  textOption.setWrapMode( QTextOption::WrapAnywhere );
  doc.setDefaultTextOption( textOption );
  doc.setDefaultStyleSheet( QStringLiteral(
    "body { white-space: normal; } "
    "p, li { white-space: normal; } "
    "table { width: 100%; table-layout: fixed; border-collapse: collapse; } "
    "th, td { white-space: normal; word-wrap: break-word; overflow-wrap: anywhere; word-break: break-word; } "
    "pre, code { white-space: pre-wrap; word-wrap: break-word; overflow-wrap: anywhere; word-break: break-word; }"
  ) );
  doc.setMarkdown( md, QTextDocument::MarkdownDialectGitHub );
  QString html = doc.toHtml();
  static const QRegularExpression tableWithoutWidthRe( u"<table(?![^>]*\\bwidth=)"_s, QRegularExpression::CaseInsensitiveOption );
  html.replace( tableWithoutWidthRe, u"<table width=\"100%\" style=\"width:100%; table-layout:fixed; border-collapse:collapse;\""_s );
  return html;
}

void QgsAiChatDockWidget::appendTranscriptMessage( const QString &role, const QString &content )
{
  QWidget *messageWidget = createMessageWidget( role, content );
  if ( !messageWidget || !mTranscriptLayout )
    return;

  const int insertIndex = std::max( 0, mTranscriptLayout->count() - 1 );
  mTranscriptLayout->insertWidget( insertIndex, messageWidget );
  scrollTranscriptToBottom();
}

void QgsAiChatDockWidget::appendTranscriptMessage( const QgsAiChatMessage &message )
{
  QString content = message.content;
  if ( message.role == QgsAiChatRole::Tool )
  {
    content = renderToolMessageMarkdown( message );
  }

  QWidget *messageWidget = createMessageWidget( roleDisplayName( message.role, qgsAiChatRoleToString( message.role ) ), content, message.metadata, message.id, message.role );
  if ( !messageWidget || !mTranscriptLayout )
    return;

  const int insertIndex = std::max( 0, mTranscriptLayout->count() - 1 );
  mTranscriptLayout->insertWidget( insertIndex, messageWidget );
  scrollTranscriptToBottom();
}

QWidget *QgsAiChatDockWidget::createMessageWidget( const QString &role, const QString &content, const QVariantMap &metadata, const QString &messageId, QgsAiChatRole messageRole )
{
  QFrame *card = new QFrame( mTranscriptContainer );
  card->setObjectName( u"aiMessage"_s );
  card->setFrameShape( QFrame::NoFrame );
  applyTranscriptWidthPolicy( card );
  card->setStyleSheet( QStringLiteral(
    "QFrame#aiMessage { border: 0; border-radius: 0; background: palette(base); } "
    "QLabel#aiMessageRole { color: palette(mid); font-weight: 600; } "
    "QLabel#aiMessageBody { color: palette(text); } "
    "QLabel#aiPlanStatusLabel, QLabel#aiQuestionsStatusLabel { color: palette(highlight); font-weight: 600; }"
  ) );

  QVBoxLayout *cardLayout = new QVBoxLayout( card );
  cardLayout->setContentsMargins( 8, 6, 8, 8 );
  cardLayout->setSpacing( 6 );

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->setContentsMargins( 0, 0, 0, 0 );
  QLabel *roleLabel = new QLabel( role, card );
  roleLabel->setObjectName( u"aiMessageRole"_s );
  headerLayout->addWidget( roleLabel );
  headerLayout->addStretch( 1 );
  const QString uiKind = metadata.value( u"ui_kind"_s ).toString();
  if ( uiKind == "plan"_L1 )
  {
    QLabel *status = new QLabel( metadata.value( u"plan_status"_s, u"pending"_s ).toString(), card );
    status->setObjectName( u"aiPlanStatusLabel"_s );
    headerLayout->addWidget( status );
  }
  else if ( uiKind == "questions"_L1 )
  {
    QLabel *status = new QLabel( metadata.value( u"questions_status"_s, u"pending"_s ).toString(), card );
    status->setObjectName( u"aiQuestionsStatusLabel"_s );
    headerLayout->addWidget( status );
  }
  cardLayout->addLayout( headerLayout );

  QString renderContent = content;
  if ( uiKind == "plan"_L1 )
  {
    const QString planMarkdown = planMarkdownFromMetadata( metadata );
    if ( !planMarkdown.isEmpty() )
      renderContent = planMarkdown;
  }
  else if ( uiKind == "questions"_L1 )
  {
    renderContent = removeQuestionsProtocolBlocks( content );
  }

  RenderedMessageContent rendered = splitTechnicalContent( renderContent );

  if ( !rendered.markdown.isEmpty() )
  {
    QLabel *body = new QLabel( card );
    body->setObjectName( u"aiMessageBody"_s );
    body->setWordWrap( true );
    body->setTextFormat( Qt::RichText );
    body->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse );
    body->setOpenExternalLinks( true );
    applyTranscriptWidthPolicy( body );
    body->setText( renderMarkdown( rendered.markdown ) );
    cardLayout->addWidget( body );
  }

  if ( metadata.contains( u"tool_calls"_s ) )
  {
    const QJsonArray calls = QJsonArray::fromVariantList( metadata.value( u"tool_calls"_s ).toList() );
    rendered.technicalSections.prepend( { tr( "Tool calls" ), QString::fromUtf8( QJsonDocument( calls ).toJson( QJsonDocument::Indented ) ), u"json"_s } );
  }

  for ( const TechnicalSection &section : std::as_const( rendered.technicalSections ) )
    cardLayout->addWidget( createCollapsibleSection( section.title, section.content, section.language, true ) );

  if ( uiKind == "plan"_L1 )
  {
    const QString planMarkdown = planMarkdownFromMetadata( metadata );
    if ( !planMarkdown.isEmpty() )
      cardLayout->addWidget( createPlanActionsWidget( messageId, planMarkdown, metadata ) );
  }
  else if ( uiKind == "questions"_L1 )
  {
    const QJsonObject payload = questionsPayloadFromMetadata( metadata );
    if ( !payload.isEmpty() )
      cardLayout->addWidget( createQuestionsWidget( messageId, payload, metadata ) );
  }

  if ( messageRole == QgsAiChatRole::User )
    card->setStyleSheet( card->styleSheet() + u"QFrame#aiMessage { background: palette(base); }"_s );

  return card;
}

QWidget *QgsAiChatDockWidget::createCollapsibleSection( const QString &title, const QString &content, const QString &language, bool collapsed )
{
  QWidget *section = new QWidget( mTranscriptContainer );
  section->setObjectName( u"aiTechnicalSection"_s );
  applyTranscriptWidthPolicy( section );
  QVBoxLayout *layout = new QVBoxLayout( section );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 3 );

  QToolButton *toggle = new QToolButton( section );
  toggle->setObjectName( u"aiTechnicalToggle"_s );
  toggle->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  toggle->setCheckable( true );
  toggle->setChecked( !collapsed );
  toggle->setAutoRaise( true );
  toggle->setText( title );
  toggle->setArrowType( collapsed ? Qt::RightArrow : Qt::DownArrow );
  toggle->setStyleSheet( QStringLiteral(
    "QToolButton#aiTechnicalToggle { color: palette(window-text); background: transparent; border: 0; border-radius: 6px; padding: 3px 6px; text-align: left; } "
    "QToolButton#aiTechnicalToggle:hover { background: palette(alternate-base); }"
  ) );
  layout->addWidget( toggle );

  QTextEdit *details = new QTextEdit( section );
  details->setObjectName( u"aiTechnicalContent"_s );
  details->setReadOnly( true );
  details->setAcceptRichText( false );
  details->setPlainText( content );
  details->setFrameShape( QFrame::NoFrame );
  applyTranscriptTextEditWrapping( details );
  details->setStyleSheet( QStringLiteral(
    "QTextEdit#aiTechnicalContent { color: palette(text); background: palette(alternate-base); border: 0; border-radius: 6px; padding: 6px; selection-background-color: palette(highlight); "
    "selection-color: palette(highlighted-text); }"
  ) );
  QFont mono = QFontDatabase::systemFont( QFontDatabase::FixedFont );
  details->setFont( mono );
  details->setMinimumHeight( 80 );
  details->setMaximumHeight( 260 );
  details->setVisible( !collapsed );
  details->setProperty( "language", language );
  layout->addWidget( details );

  connect( toggle, &QToolButton::toggled, section, [toggle, details]( bool checked ) {
    toggle->setArrowType( checked ? Qt::DownArrow : Qt::RightArrow );
    details->setVisible( checked );
  } );

  return section;
}

QWidget *QgsAiChatDockWidget::createPlanActionsWidget( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata )
{
  QFrame *planCard = new QFrame( mTranscriptContainer );
  planCard->setObjectName( u"aiPlanCard"_s );
  planCard->setFrameShape( QFrame::NoFrame );
  applyTranscriptWidthPolicy( planCard );
  planCard->setStyleSheet( u"QFrame#aiPlanCard { background: palette(base); border: 0; border-radius: 0; }"_s );

  QVBoxLayout *layout = new QVBoxLayout( planCard );
  layout->setContentsMargins( 8, 8, 8, 8 );
  layout->setSpacing( 6 );

  const QString status = metadata.value( u"plan_status"_s, u"pending"_s ).toString();
  const bool pending = status == "pending"_L1;

  QHBoxLayout *buttons = new QHBoxLayout();
  QPushButton *acceptButton = new QPushButton( tr( "Accept plan" ), planCard );
  acceptButton->setObjectName( u"aiAcceptPlanButton"_s );
  acceptButton->setEnabled( pending );
  acceptButton->setStyleSheet(
    u"QPushButton#aiAcceptPlanButton { background: palette(highlight); color: palette(highlighted-text); border: 0; border-radius: 6px; padding: 4px 10px; font-weight: 600; } QPushButton#aiAcceptPlanButton:disabled { background: palette(button); color: palette(mid); }"_s
  );
  QPushButton *reviseButton = new QPushButton( tr( "Reject / revise" ), planCard );
  reviseButton->setObjectName( u"aiRejectPlanButton"_s );
  reviseButton->setEnabled( pending );
  reviseButton->setStyleSheet(
    u"QPushButton#aiRejectPlanButton { background: palette(button); color: palette(window-text); border: 0; border-radius: 6px; padding: 4px 10px; } QPushButton#aiRejectPlanButton:hover:enabled { background: palette(alternate-base); }"_s
  );
  buttons->addWidget( acceptButton );
  buttons->addWidget( reviseButton );
  buttons->addStretch( 1 );
  layout->addLayout( buttons );

  QTextEdit *revisionEdit = new QTextEdit( planCard );
  revisionEdit->setObjectName( u"aiPlanRevisionEdit"_s );
  revisionEdit->setAcceptRichText( false );
  revisionEdit->setPlaceholderText( tr( "Describe what should change in the plan..." ) );
  revisionEdit->setFixedHeight( 72 );
  revisionEdit->setVisible( false );
  revisionEdit->setFrameShape( QFrame::NoFrame );
  applyTranscriptTextEditWrapping( revisionEdit );
  revisionEdit->setStyleSheet(
    u"QTextEdit#aiPlanRevisionEdit { color: palette(text); background: palette(alternate-base); border: 0; border-radius: 8px; padding: 6px; } QTextEdit#aiPlanRevisionEdit:focus { background: palette(base); }"_s
  );
  layout->addWidget( revisionEdit );

  QPushButton *sendRevisionButton = new QPushButton( tr( "Send revision" ), planCard );
  sendRevisionButton->setObjectName( u"aiSendPlanRevisionButton"_s );
  sendRevisionButton->setVisible( false );
  sendRevisionButton->setEnabled( pending );
  sendRevisionButton->setStyleSheet(
    u"QPushButton#aiSendPlanRevisionButton { background: palette(highlight); color: palette(highlighted-text); border: 0; border-radius: 6px; padding: 4px 10px; } QPushButton#aiSendPlanRevisionButton:disabled { background: palette(button); color: palette(mid); }"_s
  );
  layout->addWidget( sendRevisionButton );

  connect( acceptButton, &QPushButton::clicked, this, [this, messageId, planMarkdown, metadata]() { acceptPlan( messageId, planMarkdown, metadata ); } );
  connect( reviseButton, &QPushButton::clicked, planCard, [revisionEdit, sendRevisionButton]() {
    revisionEdit->setVisible( true );
    sendRevisionButton->setVisible( true );
    revisionEdit->setFocus();
  } );
  connect( sendRevisionButton, &QPushButton::clicked, this, [this, messageId, planMarkdown, metadata, revisionEdit]() { sendPlanRevision( messageId, planMarkdown, metadata, revisionEdit ); } );

  return planCard;
}

QWidget *QgsAiChatDockWidget::createQuestionsWidget( const QString &messageId, const QJsonObject &payload, const QVariantMap &metadata )
{
  QFrame *questionsCard = new QFrame( mTranscriptContainer );
  questionsCard->setObjectName( u"aiQuestionsCard"_s );
  questionsCard->setFrameShape( QFrame::NoFrame );
  applyTranscriptWidthPolicy( questionsCard );
  questionsCard->setStyleSheet( u"QFrame#aiQuestionsCard { background: palette(base); border: 0; border-radius: 0; }"_s );

  QVBoxLayout *layout = new QVBoxLayout( questionsCard );
  layout->setContentsMargins( 8, 8, 8, 8 );
  layout->setSpacing( 8 );

  const bool pending = metadata.value( u"questions_status"_s, u"pending"_s ).toString() == "pending"_L1;
  const QJsonArray questions = payload.value( u"questions"_s ).toArray();
  for ( const QJsonValue &questionValue : questions )
  {
    const QJsonObject question = questionValue.toObject();
    const QString questionId = question.value( u"id"_s ).toString();
    const QString type = question.value( u"type"_s ).toString( u"single"_s );

    QLabel *questionLabel = new QLabel( question.value( u"question"_s ).toString(), questionsCard );
    questionLabel->setWordWrap( true );
    questionLabel->setProperty( "question_id", questionId );
    applyTranscriptWidthPolicy( questionLabel );
    QFont f = questionLabel->font();
    f.setBold( true );
    questionLabel->setFont( f );
    layout->addWidget( questionLabel );

    QButtonGroup *singleGroup = nullptr;
    if ( type != "multi"_L1 )
    {
      singleGroup = new QButtonGroup( questionsCard );
      singleGroup->setExclusive( true );
    }

    const QJsonArray options = question.value( u"options"_s ).toArray();
    for ( const QJsonValue &optionValue : options )
    {
      const QJsonObject option = optionValue.toObject();
      const QString optionId = option.value( u"id"_s ).toString();
      const QString label = option.value( u"label"_s ).toString();
      const QString description = option.value( u"description"_s ).toString();

      QAbstractButton *button = type == "multi"_L1 ? static_cast<QAbstractButton *>( new QCheckBox( label, questionsCard ) ) : static_cast<QAbstractButton *>( new QRadioButton( label, questionsCard ) );
      button->setObjectName( u"aiQuestionOption"_s );
      button->setProperty( "question_id", questionId );
      button->setProperty( "option_id", optionId );
      button->setProperty( "question_type", type );
      button->setEnabled( pending );
      button->setToolTip( description );
      applyTranscriptWidthPolicy( button );
      button->setStyleSheet( QStringLiteral(
        "QAbstractButton#aiQuestionOption { color: palette(window-text); background: transparent; border: 0; padding: 2px 4px; } "
        "QAbstractButton#aiQuestionOption:hover { background: palette(base); border-radius: 4px; } "
        "QAbstractButton#aiQuestionOption:disabled { color: palette(mid); }"
      ) );
      if ( singleGroup )
        singleGroup->addButton( button );
      layout->addWidget( button );
    }

    if ( question.value( u"allow_other"_s ).toBool( true ) )
    {
      QLineEdit *other = new QLineEdit( questionsCard );
      other->setObjectName( u"aiQuestionOtherLineEdit"_s );
      other->setProperty( "question_id", questionId );
      other->setPlaceholderText( tr( "Other..." ) );
      other->setEnabled( pending );
      other->setFrame( false );
      other->setStyleSheet(
        u"QLineEdit#aiQuestionOtherLineEdit { color: palette(text); background: palette(alternate-base); border: 0; border-radius: 6px; padding: 4px 6px; } QLineEdit#aiQuestionOtherLineEdit:focus { background: palette(base); }"_s
      );
      layout->addWidget( other );
    }
  }

  QPushButton *submit = new QPushButton( tr( "Send answers" ), questionsCard );
  submit->setObjectName( u"aiSubmitQuestionAnswersButton"_s );
  submit->setEnabled( pending );
  submit->setStyleSheet(
    u"QPushButton#aiSubmitQuestionAnswersButton { background: palette(highlight); color: palette(highlighted-text); border: 0; border-radius: 6px; padding: 4px 10px; font-weight: 600; } QPushButton#aiSubmitQuestionAnswersButton:disabled { background: palette(button); color: palette(mid); }"_s
  );
  layout->addWidget( submit );

  connect( submit, &QPushButton::clicked, this, [this, messageId, metadata, questionsCard]() { sendQuestionAnswers( messageId, metadata, questionsCard ); } );

  return questionsCard;
}

void QgsAiChatDockWidget::clearTranscriptWidgets()
{
  if ( !mTranscriptLayout )
    return;

  while ( mTranscriptLayout->count() > 1 )
  {
    QLayoutItem *item = mTranscriptLayout->takeAt( 0 );
    if ( QWidget *widget = item->widget() )
      widget->deleteLater();
    delete item;
  }
}

void QgsAiChatDockWidget::scrollTranscriptToBottom()
{
  if ( !mTranscriptScrollArea )
    return;
  QTimer::singleShot( 0, this, [this]() {
    if ( mTranscriptScrollArea && mTranscriptScrollArea->verticalScrollBar() )
      mTranscriptScrollArea->verticalScrollBar()->setValue( mTranscriptScrollArea->verticalScrollBar()->maximum() );
  } );
}

void QgsAiChatDockWidget::setModeLabel( const QString &label )
{
  if ( !mModePill )
    return;

  mModePill->setText( label + u" ▾"_s );
  if ( mModePill->menu() )
  {
    for ( QAction *action : mModePill->menu()->actions() )
    {
      if ( action->isCheckable() )
        action->setChecked( action->text() == label );
    }
  }
  if ( mSessionManager )
    mSessionManager->setActiveAgent( modeLabelToAgent( label ) );
}

void QgsAiChatDockWidget::markMessageStatus( const QString &messageId, const QVariantMap &metadata, const QString &key, const QString &value )
{
  if ( !mSessionManager || messageId.isEmpty() )
    return;

  QVariantMap updated = metadata;
  updated.insert( key, value );
  mSessionManager->updateMessageMetadata( messageId, updated );
}

void QgsAiChatDockWidget::acceptPlan( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata )
{
  if ( !mSessionManager || planMarkdown.trimmed().isEmpty() || mRequestRunning )
    return;

  markMessageStatus( messageId, metadata, u"plan_status"_s, u"accepted"_s );
  setModeLabel( u"Agent"_s );
  mSessionManager->sendUserMessage(
    tr( "Execute the accepted plan exactly. Use the existing approval/review dialogs for any operation that requires confirmation.\n\nAccepted plan:\n%1" ).arg( planMarkdown.trimmed() )
  );
  reloadTranscriptFromHistory();
}

void QgsAiChatDockWidget::sendPlanRevision( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata, QTextEdit *revisionEdit )
{
  if ( !mSessionManager || mRequestRunning )
    return;

  const QString feedback = revisionEdit ? revisionEdit->toPlainText().trimmed() : QString();
  markMessageStatus( messageId, metadata, u"plan_status"_s, u"rejected"_s );
  setModeLabel( u"Plan"_s );
  mSessionManager->sendUserMessage( tr( "Revise the previous plan before execution. Keep Plan mode and return a new proposed_plan block.\n\nOriginal plan:\n%1\n\nUser revision request:\n%2" )
                                      .arg( planMarkdown.trimmed(), feedback.isEmpty() ? tr( "The plan was rejected; propose a safer or clearer revision." ) : feedback ) );
  reloadTranscriptFromHistory();
}

void QgsAiChatDockWidget::sendQuestionAnswers( const QString &messageId, const QVariantMap &metadata, QWidget *questionsCard )
{
  if ( !mSessionManager || !questionsCard || mRequestRunning )
    return;

  QMap<QString, QJsonObject> answersById;
  const QList<QAbstractButton *> options = questionsCard->findChildren<QAbstractButton *>( u"aiQuestionOption"_s );
  for ( QAbstractButton *option : options )
  {
    if ( !option->isChecked() )
      continue;

    const QString questionId = option->property( "question_id" ).toString();
    if ( questionId.isEmpty() )
      continue;

    QJsonObject answer = answersById.value( questionId );
    QJsonArray selected = answer.value( u"selected"_s ).toArray();
    selected.append( option->property( "option_id" ).toString() );
    answer.insert( u"id"_s, questionId );
    answer.insert( u"selected"_s, selected );
    answersById.insert( questionId, answer );
  }

  const QList<QLineEdit *> otherEdits = questionsCard->findChildren<QLineEdit *>( u"aiQuestionOtherLineEdit"_s );
  for ( QLineEdit *other : otherEdits )
  {
    const QString text = other->text().trimmed();
    if ( text.isEmpty() )
      continue;

    const QString questionId = other->property( "question_id" ).toString();
    if ( questionId.isEmpty() )
      continue;

    QJsonObject answer = answersById.value( questionId );
    answer.insert( u"id"_s, questionId );
    answer.insert( u"other"_s, text );
    answersById.insert( questionId, answer );
  }

  QJsonArray answers;
  for ( auto it = answersById.constBegin(); it != answersById.constEnd(); ++it )
    answers.append( it.value() );

  if ( answers.isEmpty() )
    return;

  QJsonObject payload;
  payload.insert( u"answers"_s, answers );
  markMessageStatus( messageId, metadata, u"questions_status"_s, u"answered"_s );
  setModeLabel( u"Plan"_s );
  mSessionManager->sendUserMessage(
    tr( "Answers to your planning questions:\n```qgis_ai_answers\n%1\n```" ).arg( QString::fromUtf8( QJsonDocument( payload ).toJson( QJsonDocument::Indented ) ).trimmed() )
  );
  reloadTranscriptFromHistory();
}

QString QgsAiChatDockWidget::renderToolMessageMarkdown( const QgsAiChatMessage &message ) const
{
  const QString toolName = message.metadata.value( u"tool_name"_s ).toString();
  const bool isError = message.metadata.value( u"is_error"_s ).toBool();
  const QJsonObject output = jsonObjectFromMessageContent( message.content );
  const QVariantMap args = message.metadata.value( u"tool_args"_s ).toMap();
  const QString status = output.value( u"status"_s ).toString( isError ? u"error"_s : u"ok"_s );

  QString md = u"**%1** - `%2`\n"_s.arg( toolName.isEmpty() ? u"tool"_s : toolName, status );

  if ( toolName == "run_python"_L1 )
  {
    const QString description = message.metadata.value( u"tool_description"_s ).toString().trimmed();
    if ( !description.isEmpty() )
      md += u"\n%1\n"_s.arg( description );

    md += markdownCodeBlock( tr( "Approved code:" ), message.metadata.value( u"tool_code"_s ).toString(), u"python"_s );
    md += markdownCodeBlock( tr( "stdout:" ), output.value( u"stdout"_s ).toString() );
    md += markdownCodeBlock( tr( "stderr:" ), output.value( u"stderr"_s ).toString() );
    md += markdownCodeBlock( tr( "traceback:" ), output.value( u"traceback"_s ).toString(), u"text"_s );
    if ( output.contains( u"error"_s ) )
      md += u"\nError: `%1`\n"_s.arg( output.value( u"error"_s ).toString() );
    return md.trimmed();
  }

  if ( toolName == "download_file"_L1 )
  {
    const QString host = urlHostForTranscript( args.value( u"url"_s ).toString() );
    const QString dest = relativePathForTranscript( mSessionManager ? mSessionManager->workspaceRoot() : QString(), output.value( u"dest_path"_s ).toString() );
    md += u"\nHost: `%1`\n"_s.arg( host.isEmpty() ? u"(unknown)"_s : host );
    if ( !dest.isEmpty() )
      md += u"Destination: `%1`\n"_s.arg( dest );
    if ( output.contains( u"bytes_written"_s ) )
      md += u"Bytes: `%1`\n"_s.arg( scalarForTranscript( output.value( u"bytes_written"_s ) ) );
    if ( output.contains( u"http_status"_s ) )
      md += u"HTTP: `%1`\n"_s.arg( scalarForTranscript( output.value( u"http_status"_s ) ) );
    if ( output.contains( u"error"_s ) )
      md += u"Error: `%1`\n"_s.arg( output.value( u"error"_s ).toString() );
    return md.trimmed();
  }

  if ( toolName == "install_python_package"_L1 )
  {
    md += u"\nPackages requested: `%1`\n"_s.arg( args.value( u"packages"_s ).toList().size() );
    if ( output.contains( u"exit_code"_s ) )
      md += u"Exit code: `%1`\n"_s.arg( scalarForTranscript( output.value( u"exit_code"_s ) ) );
    md += markdownCodeBlock( tr( "stdout:" ), output.value( u"stdout"_s ).toString() );
    md += markdownCodeBlock( tr( "stderr:" ), output.value( u"stderr"_s ).toString() );
    if ( output.contains( u"error"_s ) )
      md += u"\nError: `%1`\n"_s.arg( output.value( u"error"_s ).toString() );
    return md.trimmed();
  }

  if ( output.isEmpty() )
  {
    md += u"\n%1"_s.arg( truncateForTranscript( message.content, 1000 ) );
    return md.trimmed();
  }

  int shown = 0;
  for ( auto it = output.constBegin(); it != output.constEnd() && shown < 6; ++it )
  {
    if ( it.value().isObject() || it.value().isArray() )
      continue;
    md += u"\n- %1: `%2`"_s.arg( it.key(), truncateForTranscript( scalarForTranscript( it.value() ), 160 ) );
    ++shown;
  }
  if ( shown == 0 )
    md += "\nResult received."_L1;
  return md.trimmed();
}

void QgsAiChatDockWidget::onNewChatClicked()
{
  if ( mSessionManager )
    mSessionManager->startNewSession();
}

void QgsAiChatDockWidget::reloadTranscriptFromHistory()
{
  if ( !mTranscriptLayout )
    return;
  closeStreamingAssistantMessage();
  clearTranscriptWidgets();
  if ( !mSessionManager )
    return;
  const QList<QgsAiChatMessage> history = mSessionManager->history();
  for ( const QgsAiChatMessage &m : history )
  {
    if ( m.role == QgsAiChatRole::System )
      continue;
    appendTranscriptMessage( m );
  }
}

void QgsAiChatDockWidget::rebuildHistoryMenu()
{
  if ( !mHistoryButton || !mHistoryButton->menu() )
    return;

  QMenu *menu = mHistoryButton->menu();
  menu->clear();

  if ( !mSessionManager )
  {
    QAction *empty = menu->addAction( tr( "No session manager" ) );
    empty->setEnabled( false );
    return;
  }

  if ( !mSessionManager->hasPersistentChatHistoryScope() )
  {
    QAction *empty = menu->addAction( tr( "Save or open a QGIS project to save chat history" ) );
    empty->setEnabled( false );
    return;
  }

  const QList<QgsAiChatHistoryStore::SessionInfo> sessions = mSessionManager->listSessions();
  if ( sessions.isEmpty() )
  {
    QAction *empty = menu->addAction( tr( "No past chats yet" ) );
    empty->setEnabled( false );
    return;
  }

  const QString activeId = mSessionManager->activeSessionId();
  for ( const QgsAiChatHistoryStore::SessionInfo &s : sessions )
  {
    const QString stamp = s.updatedAt.toLocalTime().toString( u"yyyy-MM-dd HH:mm"_s );
    const QString label = u"%1 — %2"_s.arg( s.title, stamp );
    QAction *act = menu->addAction( label );
    act->setData( s.id );
    if ( s.id == activeId )
    {
      act->setCheckable( true );
      act->setChecked( true );
    }
  }

  menu->addSeparator();

  if ( !activeId.isEmpty() )
  {
    QAction *rename = menu->addAction( tr( "Rename current chat…" ) );
    rename->setData( u"__rename__"_s );
    QAction *del = menu->addAction( tr( "Delete current chat" ) );
    del->setData( u"__delete__"_s );
  }
}

void QgsAiChatDockWidget::onHistoryEntryTriggered( QAction *action )
{
  if ( !action || !mSessionManager )
    return;

  const QString data = action->data().toString();
  if ( data.isEmpty() )
    return;

  if ( data == "__rename__"_L1 )
  {
    QString current;
    const QString activeId = mSessionManager->activeSessionId();
    const QList<QgsAiChatHistoryStore::SessionInfo> sessions = mSessionManager->listSessions();
    for ( const QgsAiChatHistoryStore::SessionInfo &s : sessions )
    {
      if ( s.id == activeId )
      {
        current = s.title;
        break;
      }
    }
    bool ok = false;
    const QString newTitle = QInputDialog::getText( this, tr( "Rename chat" ), tr( "Title:" ), QLineEdit::Normal, current, &ok );
    if ( ok && !newTitle.trimmed().isEmpty() )
      mSessionManager->renameActiveSession( newTitle.trimmed() );
    return;
  }
  if ( data == "__delete__"_L1 )
  {
    const auto answer = QMessageBox::question( this, tr( "Delete chat" ), tr( "Delete the current chat permanently?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
    if ( answer == QMessageBox::Yes )
      mSessionManager->deleteSession( mSessionManager->activeSessionId() );
    return;
  }

  mSessionManager->loadSession( data );
}

void QgsAiChatDockWidget::appendStreamChunk( const QString &chunk )
{
  if ( chunk.isEmpty() )
    return;

  if ( !mStreamingInProgress )
  {
    QFrame *card = new QFrame( mTranscriptContainer );
    card->setObjectName( u"aiStreamingMessage"_s );
    card->setFrameShape( QFrame::NoFrame );
    applyTranscriptWidthPolicy( card );
    card->setStyleSheet(
      u"QFrame#aiStreamingMessage { border: 0; border-radius: 0; background: palette(base); } QLabel#aiMessageRole { color: palette(mid); font-weight: 600; } QTextEdit#aiStreamingTextEdit { color: palette(text); background: transparent; border: 0; }"_s
    );
    QVBoxLayout *layout = new QVBoxLayout( card );
    layout->setContentsMargins( 8, 6, 8, 8 );
    QLabel *roleLabel = new QLabel( tr( "Assistant" ), card );
    roleLabel->setObjectName( u"aiMessageRole"_s );
    layout->addWidget( roleLabel );

    mStreamingTextEdit = new QTextEdit( card );
    mStreamingTextEdit->setObjectName( u"aiStreamingTextEdit"_s );
    mStreamingTextEdit->setReadOnly( true );
    mStreamingTextEdit->setAcceptRichText( false );
    mStreamingTextEdit->setFrameShape( QFrame::NoFrame );
    mStreamingTextEdit->setMinimumHeight( 48 );
    applyTranscriptTextEditWrapping( mStreamingTextEdit );
    layout->addWidget( mStreamingTextEdit );

    const int insertIndex = std::max( 0, mTranscriptLayout->count() - 1 );
    mTranscriptLayout->insertWidget( insertIndex, card );
    mStreamingInProgress = true;
  }
  if ( mStreamingTextEdit )
  {
    QTextCursor cursor = mStreamingTextEdit->textCursor();
    cursor.movePosition( QTextCursor::End );
    cursor.insertText( chunk );
    mStreamingTextEdit->setTextCursor( cursor );
  }
  scrollTranscriptToBottom();
}

void QgsAiChatDockWidget::closeStreamingAssistantMessage()
{
  mStreamingInProgress = false;
  mStreamingTextEdit = nullptr;
}

void QgsAiChatDockWidget::updateRuntimeState( const QString &state, const QString &detail )
{
  const QString text = tr( "Provider state: %1 - %2" ).arg( state, detail );
  mRuntimeStatusLabel->setText( text );
  if ( mSendButton )
    mSendButton->setToolTip( mRequestRunning ? tr( "Stop (%1)" ).arg( text ) : tr( "Send (Enter)" ) );
}

void QgsAiChatDockWidget::setRequestRunning( bool running )
{
  mRequestRunning = running;
  if ( mSendButton )
  {
    mSendButton->setText( running ? u"◼"_s : u"↑"_s );
    mSendButton->setToolTip( running ? tr( "Stop" ) : tr( "Send (Enter)" ) );
  }
  if ( mInputTextEdit )
    mInputTextEdit->setEnabled( !running );
  if ( mCancelButton )
    mCancelButton->setEnabled( running );
  if ( !running && mStreamingInProgress )
    closeStreamingAssistantMessage();
}

void QgsAiChatDockWidget::onSendOrStopClicked()
{
  if ( mRequestRunning )
    cancelRunningRequest();
  else
    sendMessage();
}

void QgsAiChatDockWidget::onModeSelected( QAction *action )
{
  if ( !action )
    return;
  setModeLabel( action->text() );
}

void QgsAiChatDockWidget::onModelSelected( QAction *action )
{
  if ( !action || !mModelRouter )
    return;
  const ModelEntry entry = action->data().value<ModelEntry>();
  if ( entry.displayName.isEmpty() )
    return;

  const QList<QgsAiModelRouter::Provider> providers
    = { QgsAiModelRouter::Provider::OpenAi, QgsAiModelRouter::Provider::OpenRouter, QgsAiModelRouter::Provider::Codex, QgsAiModelRouter::Provider::Claude, QgsAiModelRouter::Provider::Plan };
  for ( QgsAiModelRouter::Provider provider : providers )
  {
    QgsAiModelRouter::ProviderSettings settings = mModelRouter->providerSettings( provider );
    if ( provider == entry.provider )
    {
      settings.model = entry.model;
      settings.enabled = true;
    }
    else
    {
      settings.enabled = false;
    }
    mModelRouter->setProviderSettings( provider, settings );
  }
  mModelPill->setText( entry.displayName + u" ▾"_s );
}

void QgsAiChatDockWidget::ensureWorkspaceTrustDecision()
{
  const QString root = QgsAiWorkspaceTrust::currentWorkspaceRoot();
  if ( root.isEmpty() || QgsAiWorkspaceTrust::state( root ) != QgsAiWorkspaceTrust::State::Unknown )
    return;

  QMessageBox box( this );
  box.setIcon( QMessageBox::Question );
  box.setWindowTitle( tr( "Trust this workspace?" ) );
  box.setText( tr(
                 "The AI assistant is about to work in:\n%1\n\n"
                 "Trusted workspaces can load AI rules/skills files (.strata/rules, .strata/skills) "
                 "into the assistant's instructions and enable the risky tools "
                 "(run_python, install_python_package, download_file).\n\n"
                 "Only trust workspaces whose content you control. You can change this "
                 "any time from the AI provider settings."
  )
                 .arg( root ) );
  QPushButton *trustButton = box.addButton( tr( "Trust workspace" ), QMessageBox::AcceptRole );
  QPushButton *dontTrustButton = box.addButton( tr( "Don't trust" ), QMessageBox::RejectRole );
  box.setDefaultButton( dontTrustButton );
  box.exec();

  QgsAiWorkspaceTrust::setState( root, box.clickedButton() == trustButton ? QgsAiWorkspaceTrust::State::Trusted : QgsAiWorkspaceTrust::State::Untrusted );
}

void QgsAiChatDockWidget::sendMessage()
{
  if ( !mSessionManager )
    return;
  const QString input = mInputTextEdit->toPlainText().trimmed();
  const QList<QgsAiChatContextFile> contextFiles = contextFilesForCurrentMessage( input );
  if ( input.isEmpty() && contextFiles.isEmpty() )
    return;

  // First AI interaction with an undecided workspace: ask for the trust decision.
  // Never blocks sending — untrusted just restricts rules/skills and risky tools.
  ensureWorkspaceTrustDecision();

  mInputTextEdit->clear();
  hideMentionPopup();
  mAttachedFiles.clear();
  rebuildAttachmentChips();
  mSessionManager->sendUserMessage( input.isEmpty() ? tr( "Analyze the attached files." ) : input, contextFiles );
}

void QgsAiChatDockWidget::cancelRunningRequest()
{
  if ( mSessionManager )
    mSessionManager->cancelActiveRequest();
}

void QgsAiChatDockWidget::attachFile()
{
  const QStringList paths = QFileDialog::getOpenFileNames( this, tr( "Attach files to chat" ) );
  if ( paths.isEmpty() )
    return;

  bool added = false;
  for ( const QString &path : paths )
    added = addAttachedFile( path ) || added;

  if ( added )
    rebuildAttachmentChips();
}

void QgsAiChatDockWidget::clearFileContext()
{
  mAttachedFiles.clear();
  rebuildAttachmentChips();
}

void QgsAiChatDockWidget::updateFileContextChip()
{
  rebuildAttachmentChips();
}

bool QgsAiChatDockWidget::addAttachedFile( const QString &path )
{
  const QFileInfo info( path );
  if ( !info.exists() || !info.isFile() )
    return false;

  const QString absolutePath = QDir::cleanPath( info.absoluteFilePath() );
  for ( const AttachedFile &file : std::as_const( mAttachedFiles ) )
  {
    if ( file.filePath == absolutePath )
      return false;
  }

  AttachedFile file;
  file.filePath = absolutePath;
  file.allowExternal = true;
  mAttachedFiles << file;
  return true;
}

void QgsAiChatDockWidget::rebuildAttachmentChips()
{
  if ( !mFileContextChipLayout )
    return;

  while ( QLayoutItem *item = mFileContextChipLayout->takeAt( 0 ) )
  {
    if ( QWidget *widget = item->widget() )
      widget->deleteLater();
    delete item;
  }

  for ( const AttachedFile &file : std::as_const( mAttachedFiles ) )
  {
    QWidget *chip = new QWidget( mFileContextChipRow );
    chip->setObjectName( u"aiAttachmentChip"_s );
    chip->setStyleSheet(
      u"QWidget#aiAttachmentChip { color: palette(window-text); background: palette(button); border: 0; border-radius: 10px; padding: 1px 4px; } QWidget#aiAttachmentChip:hover { background: palette(alternate-base); }"_s
    );
    QHBoxLayout *chipLayout = new QHBoxLayout( chip );
    chipLayout->setContentsMargins( 6, 1, 2, 1 );
    chipLayout->setSpacing( 3 );

    QLabel *label = new QLabel( u"📎 %1"_s.arg( QFileInfo( file.filePath ).fileName() ), chip );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse );
    label->setToolTip( file.filePath );
    chipLayout->addWidget( label );

    QToolButton *removeButton = new QToolButton( chip );
    removeButton->setText( u"×"_s );
    removeButton->setAutoRaise( true );
    removeButton->setFixedSize( 18, 18 );
    removeButton->setToolTip( tr( "Remove attachment" ) );
    chipLayout->addWidget( removeButton );

    const QString path = file.filePath;
    connect( removeButton, &QToolButton::clicked, this, [this, path]() {
      for ( int i = 0; i < mAttachedFiles.size(); ++i )
      {
        if ( mAttachedFiles.at( i ).filePath == path )
        {
          mAttachedFiles.removeAt( i );
          break;
        }
      }
      rebuildAttachmentChips();
    } );

    mFileContextChipLayout->addWidget( chip );
  }

  mFileContextChipLayout->addStretch( 1 );
  if ( mFileContextChipRow )
    mFileContextChipRow->setVisible( !mAttachedFiles.isEmpty() );
}

void QgsAiChatDockWidget::hideMentionPopup()
{
  mMentionStartPosition = -1;
  if ( mMentionPopup )
    mMentionPopup->hide();
}

void QgsAiChatDockWidget::updateMentionPopup()
{
  if ( !mInputTextEdit || !mMentionPopup || !mMentionList || !mSessionManager )
    return;

  const QTextCursor cursor = mInputTextEdit->textCursor();
  const QString textBeforeCursor = mInputTextEdit->toPlainText().left( cursor.position() );
  const int atPosition = textBeforeCursor.lastIndexOf( '@'_L1 );
  if ( atPosition < 0 )
  {
    hideMentionPopup();
    return;
  }

  if ( atPosition > 0 && !textBeforeCursor.at( atPosition - 1 ).isSpace() )
  {
    hideMentionPopup();
    return;
  }

  QString query = textBeforeCursor.mid( atPosition + 1 );
  if ( query.contains( QRegularExpression( u"\\s"_s ) ) || query.contains( '"'_L1 ) )
  {
    hideMentionPopup();
    return;
  }

  const QStringList candidates = mSessionManager->projectFileCandidates( query, 25 );
  if ( candidates.isEmpty() )
  {
    hideMentionPopup();
    return;
  }

  mMentionStartPosition = atPosition;
  mMentionList->clear();
  for ( const QString &candidate : candidates )
  {
    QListWidgetItem *item = new QListWidgetItem( candidate, mMentionList );
    item->setData( Qt::UserRole, candidate );
    item->setToolTip( candidate );
  }
  mMentionList->setCurrentRow( 0 );

  const QRect cursorRect = mInputTextEdit->cursorRect( cursor );
  const QPoint popupPosition = mInputTextEdit->viewport()->mapToGlobal( cursorRect.bottomLeft() );
  const int popupHeight = std::min( 260, std::max( 48, mMentionList->sizeHintForRow( 0 ) * static_cast<int>( candidates.size() ) + 8 ) );
  mMentionPopup->setFixedSize( 420, popupHeight );
  mMentionPopup->move( popupPosition );
  mMentionPopup->show();
}

void QgsAiChatDockWidget::insertSelectedMention()
{
  if ( !mMentionList || !mMentionList->currentItem() )
    return;

  insertMentionFile( mMentionList->currentItem()->data( Qt::UserRole ).toString() );
}

void QgsAiChatDockWidget::insertMentionFile( const QString &relativePath )
{
  if ( relativePath.isEmpty() || !mInputTextEdit || mMentionStartPosition < 0 )
    return;

  QTextCursor cursor = mInputTextEdit->textCursor();
  const int endPosition = cursor.position();
  cursor.setPosition( mMentionStartPosition );
  cursor.setPosition( endPosition, QTextCursor::KeepAnchor );

  const bool needsQuotes = relativePath.contains( QRegularExpression( u"\\s"_s ) );
  const QString mention = needsQuotes ? u"@\"%1\""_s.arg( relativePath ) : u"@%1"_s.arg( relativePath );
  cursor.insertText( mention + ' '_L1 );
  mInputTextEdit->setTextCursor( cursor );
  hideMentionPopup();
}

QList<QgsAiChatContextFile> QgsAiChatDockWidget::contextFilesForCurrentMessage( const QString &text ) const
{
  QList<QgsAiChatContextFile> contextFiles;
  QSet<QString> seenPaths;

  for ( const AttachedFile &attachedFile : mAttachedFiles )
  {
    QgsAiChatContextFile contextFile;
    contextFile.filePath = attachedFile.filePath;
    contextFile.allowExternal = attachedFile.allowExternal;
    contextFiles << contextFile;
    seenPaths.insert( attachedFile.filePath );
  }

  if ( !mSessionManager )
    return contextFiles;

  static const QRegularExpression mentionRe( u"@\"([^\"]+)\"|@([^\\s]+)"_s );
  QRegularExpressionMatchIterator it = mentionRe.globalMatch( text );
  while ( it.hasNext() )
  {
    const QRegularExpressionMatch match = it.next();
    const QString referencedPath = !match.captured( 1 ).isEmpty() ? match.captured( 1 ) : match.captured( 2 );
    const QString resolvedPath = mSessionManager->resolveProjectFile( referencedPath );
    if ( resolvedPath.isEmpty() || seenPaths.contains( resolvedPath ) )
      continue;

    QgsAiChatContextFile contextFile;
    contextFile.filePath = resolvedPath;
    contextFile.allowExternal = false;
    contextFiles << contextFile;
    seenPaths.insert( resolvedPath );
  }

  return contextFiles;
}

QString QgsAiChatDockWidget::selectedProposalId() const
{
  const QListWidgetItem *item = mProposalList->currentItem();
  return item ? item->data( Qt::UserRole ).toString() : QString();
}

void QgsAiChatDockWidget::refreshProposalList()
{
  mProposalList->clear();
  if ( !mReviewEngine )
  {
    if ( mReviewContainer )
      mReviewContainer->setVisible( false );
    return;
  }

  const QList<QgsAiPatchProposal> proposals = mReviewEngine->pendingProposals();
  for ( const QgsAiPatchProposal &proposal : proposals )
  {
    QListWidgetItem *item = new QListWidgetItem( proposal.title.isEmpty() ? proposal.id : proposal.title, mProposalList );
    item->setData( Qt::UserRole, proposal.id );
    item->setToolTip( proposal.id );
  }
  mReviewStatusLabel->setText( tr( "Pending reviews: %1" ).arg( proposals.size() ) );
  mReviewContainer->setVisible( !proposals.isEmpty() );
}

void QgsAiChatDockWidget::previewProposal()
{
  if ( !mReviewEngine )
    return;
  const QString proposalId = selectedProposalId();
  if ( proposalId.isEmpty() )
    return;

  const QString diff = mReviewEngine->previewProposalDiff( proposalId );
  if ( diff.isEmpty() )
    return;
  QMessageBox::information( this, tr( "Proposal preview" ), diff );
}

void QgsAiChatDockWidget::acceptProposal()
{
  if ( !mReviewEngine )
    return;
  const QString proposalId = selectedProposalId();
  if ( proposalId.isEmpty() )
    return;

  QString error;
  if ( !mReviewEngine->acceptProposal( proposalId, &error ) )
    QMessageBox::warning( this, tr( "Cannot apply proposal" ), error );
  refreshProposalList();
}

void QgsAiChatDockWidget::acceptPartialProposal()
{
  if ( !mReviewEngine )
    return;
  const QString proposalId = selectedProposalId();
  if ( proposalId.isEmpty() )
    return;

  bool ok = false;
  const QString input = QInputDialog::getText( this, tr( "Accept partial proposal" ), tr( "Hunk indices (comma separated, zero based):" ), QLineEdit::Normal, u"0"_s, &ok );
  if ( !ok || input.trimmed().isEmpty() )
    return;

  QList<int> hunkIndexes;
  const QStringList chunks = input.split( ',', Qt::SkipEmptyParts );
  for ( const QString &chunk : chunks )
    hunkIndexes << chunk.trimmed().toInt();

  QString error;
  if ( !mReviewEngine->acceptHunks( proposalId, hunkIndexes, &error ) )
    QMessageBox::warning( this, tr( "Cannot apply partial proposal" ), error );
  refreshProposalList();
}

void QgsAiChatDockWidget::rejectProposal()
{
  if ( !mReviewEngine )
    return;
  const QString proposalId = selectedProposalId();
  if ( proposalId.isEmpty() )
    return;
  mReviewEngine->rejectProposal( proposalId );
  refreshProposalList();
}

void QgsAiChatDockWidget::openProviderSettings()
{
  if ( !mModelRouter )
    return;

  QDialog dialog( this );
  dialog.setWindowTitle( tr( "AI Provider Settings" ) );
  QVBoxLayout *dialogLayout = new QVBoxLayout( &dialog );
  QgsScrollArea *scrollArea = new QgsScrollArea( &dialog );
  scrollArea->setWidgetResizable( true );
  scrollArea->setFrameShape( QFrame::NoFrame );
  QWidget *settingsContent = new QWidget( scrollArea );
  QVBoxLayout *layout = new QVBoxLayout( settingsContent );
  QFormLayout *form = new QFormLayout();

  QLineEdit *openAiEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi ).endpoint, &dialog );
  QLineEdit *openAiModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi ).model, &dialog );
  QLineEdit *openAiKey = new QLineEdit( &dialog );
  openAiKey->setEchoMode( QLineEdit::Password );
  openAiKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "sk-..." ) );

  QLineEdit *openRouterEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter ).endpoint, &dialog );

  // Editable, searchable model picker fed by the async OpenRouter catalog
  // (tool-capable models with context size and pricing). Free text stays valid:
  // the current text is what gets persisted.
  QComboBox *openRouterModel = new QComboBox( &dialog );
  openRouterModel->setObjectName( u"aiOpenRouterModelComboBox"_s );
  openRouterModel->setEditable( true );
  openRouterModel->setInsertPolicy( QComboBox::NoInsert );
  openRouterModel->lineEdit()->setPlaceholderText( u"anthropic/claude-sonnet-4.6"_s );
  const QString configuredOpenRouterModel = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter ).model;
  openRouterModel->setEditText( configuredOpenRouterModel );

  QgsAiOpenRouterModelCatalog *openRouterCatalog = new QgsAiOpenRouterModelCatalog( &dialog );
  connect( openRouterCatalog, &QgsAiOpenRouterModelCatalog::modelsReady, &dialog, [openRouterModel]( const QList<QgsAiOpenRouterModelCatalog::ModelInfo> &models, bool ) {
    const QString currentText = openRouterModel->currentText();
    openRouterModel->clear();
    for ( const QgsAiOpenRouterModelCatalog::ModelInfo &model : models )
      openRouterModel->addItem( model.displayLabel(), model.id );
    QCompleter *completer = new QCompleter( openRouterModel->model(), openRouterModel );
    completer->setFilterMode( Qt::MatchContains );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    openRouterModel->setCompleter( completer );
    openRouterModel->setEditText( currentText );
  } );
  // Selecting a catalog entry replaces the display label with the model id.
  connect( openRouterModel, QOverload<int>::of( &QComboBox::activated ), &dialog, [openRouterModel]( int index ) {
    const QString modelId = openRouterModel->itemData( index ).toString();
    if ( !modelId.isEmpty() )
      openRouterModel->setEditText( modelId );
  } );
  openRouterCatalog->refresh();

  QLineEdit *openRouterKey = new QLineEdit( &dialog );
  openRouterKey->setEchoMode( QLineEdit::Password );
  openRouterKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenRouter ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "sk-or-..." ) );

  // Connection test: validates the pending (or stored) key against GET /key and
  // shows the credits summary inline.
  QPushButton *openRouterTestButton = new QPushButton( tr( "Test connection" ), &dialog );
  openRouterTestButton->setObjectName( u"aiOpenRouterTestConnectionButton"_s );
  QLabel *openRouterTestResult = new QLabel( &dialog );
  openRouterTestResult->setObjectName( u"aiOpenRouterTestConnectionLabel"_s );
  openRouterTestResult->setWordWrap( true );
  connect( openRouterCatalog, &QgsAiOpenRouterModelCatalog::keyInfoReady, &dialog, [openRouterTestButton, openRouterTestResult]( const QString &summary ) {
    openRouterTestButton->setEnabled( true );
    openRouterTestResult->setText( summary );
  } );
  connect( openRouterCatalog, &QgsAiOpenRouterModelCatalog::keyInfoFailed, &dialog, [openRouterTestButton, openRouterTestResult]( const QString &errorMessage ) {
    openRouterTestButton->setEnabled( true );
    openRouterTestResult->setText( errorMessage );
  } );
  connect( openRouterTestButton, &QPushButton::clicked, &dialog, [openRouterCatalog, openRouterKey, openRouterTestButton, openRouterTestResult]() {
    QString key = openRouterKey->text().trimmed();
    if ( key.isEmpty() )
      key = QgsAiSecretStore::readSecret( u"ai/provider/openrouter/apiKey"_s, { u"OPENROUTER_API_KEY"_s } );
    openRouterTestButton->setEnabled( false );
    openRouterTestResult->setText( tr( "Testing…" ) );
    openRouterCatalog->fetchKeyInfo( key );
  } );

  QCheckBox *openRouterAutoRouting = new QCheckBox( tr( "Use OpenRouter automatic routing" ), &dialog );
  openRouterAutoRouting->setChecked( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter ).autoRouting );
  openRouterAutoRouting->setToolTip(
    tr( "Adds OpenRouter provider routing preferences (tool-capable providers, price/throughput sorting) and falls back to the Auto router if the pinned model is unavailable." )
  );

  QgsAiCodexOAuthClient::DeviceCode codexDeviceCode;
  QLineEdit *codexEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex ).endpoint, &dialog );
  QLineEdit *codexModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex ).model, &dialog );
  codexModel->setPlaceholderText( u"gpt-5.4"_s );
  QLabel *codexStatus = new QLabel( mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Codex ) ? tr( "Signed in" ) : tr( "Not signed in" ), &dialog );
  QPushButton *codexRequestCodeButton = new QPushButton( tr( "Get Codex device code" ), &dialog );
  QPushButton *codexCompleteLoginButton = new QPushButton( tr( "Complete Codex login" ), &dialog );
  QPushButton *codexLogoutButton = new QPushButton( tr( "Logout Codex" ), &dialog );
  QWidget *codexButtons = new QWidget( &dialog );
  QHBoxLayout *codexButtonsLayout = new QHBoxLayout( codexButtons );
  codexButtonsLayout->setContentsMargins( 0, 0, 0, 0 );
  codexButtonsLayout->addWidget( codexRequestCodeButton );
  codexButtonsLayout->addWidget( codexCompleteLoginButton );
  codexButtonsLayout->addWidget( codexLogoutButton );

  QLineEdit *claudeEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).endpoint, &dialog );
  QLineEdit *claudeModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).model, &dialog );
  QLineEdit *claudeKey = new QLineEdit( &dialog );
  claudeKey->setEchoMode( QLineEdit::Password );
  claudeKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "anthropic key..." ) );
  QCheckBox *claudeUseOAuth = new QCheckBox( tr( "Use Claude OAuth login instead of API key" ), &dialog );
  claudeUseOAuth->setChecked( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).credentialMode == QgsAiModelRouter::CredentialMode::OAuth );
  QLabel *claudeOAuthStatus = new QLabel( mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Claude ) ? tr( "Signed in" ) : tr( "Not signed in" ), &dialog );
  QPushButton *claudeLoginButton = new QPushButton( tr( "Login with Claude" ), &dialog );
  QPushButton *claudeLogoutButton = new QPushButton( tr( "Logout Claude" ), &dialog );
  QWidget *claudeOAuthButtons = new QWidget( &dialog );
  QHBoxLayout *claudeOAuthButtonsLayout = new QHBoxLayout( claudeOAuthButtons );
  claudeOAuthButtonsLayout->setContentsMargins( 0, 0, 0, 0 );
  claudeOAuthButtonsLayout->addWidget( claudeLoginButton );
  claudeOAuthButtonsLayout->addWidget( claudeLogoutButton );

  QLineEdit *planEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint, &dialog );
  QLineEdit *planAuthCfg = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).authConfigId, &dialog );
  QLineEdit *planToken = new QLineEdit( &dialog );
  planToken->setEchoMode( QLineEdit::Password );
  planToken->setPlaceholderText( tr( "Session token from your plan login..." ) );

  QgsSettings workspaceSettings;
  QLineEdit *aiWorkspaceRoot
    = new QLineEdit( settingValueWithLegacy( workspaceSettings, u"strata/workspace/root"_s, QStringList { u"geoai/workspace/root"_s, u"qgis_ai/workspace/root"_s }, QString() ).toString(), &dialog );
  aiWorkspaceRoot->setObjectName( u"aiWorkspaceRootLineEdit"_s );
  aiWorkspaceRoot->setPlaceholderText( tr( "Used when the QGIS project is unsaved" ) );
  QPushButton *browseWorkspaceRoot = new QPushButton( tr( "Browse..." ), &dialog );
  QWidget *workspaceRootWidget = new QWidget( &dialog );
  QHBoxLayout *workspaceRootLayout = new QHBoxLayout( workspaceRootWidget );
  workspaceRootLayout->setContentsMargins( 0, 0, 0, 0 );
  workspaceRootLayout->addWidget( aiWorkspaceRoot, 1 );
  workspaceRootLayout->addWidget( browseWorkspaceRoot );

  // Credential/data encryption status (best-effort vault policy: never prompts).
  QLabel *encryptionStatus = new QLabel( &dialog );
  encryptionStatus->setObjectName( u"aiCredentialEncryptionStatusLabel"_s );
  encryptionStatus->setWordWrap( true );
  encryptionStatus->setText(
    QgsAiSecretStore::vaultUsable() ? tr( "AI credentials and data are encrypted in the QGIS authentication vault." )
                                    : tr( "AI credentials and data are stored unencrypted — unlock or set a QGIS master password (Options ▸ Authentication) to enable encryption." )
  );

  // Workspace trust: gates rules/skills loading and the risky tools.
  QCheckBox *trustWorkspace = new QCheckBox( tr( "Trust this workspace (enables rules/skills files and run_python, install_python_package, download_file)" ), &dialog );
  trustWorkspace->setObjectName( u"aiTrustWorkspaceCheckBox"_s );
  QString trustRootForCheckbox;
  auto dialogTrustRoot = [aiWorkspaceRoot]() {
    if ( QgsProject::instance() )
    {
      const QString projectHome = QgsProject::instance()->homePath().trimmed();
      if ( !projectHome.isEmpty() )
        return QDir( projectHome ).absolutePath();
    }

    const QString requestedWorkspaceRoot = aiWorkspaceRoot->text().trimmed();
    return requestedWorkspaceRoot.isEmpty() ? QString() : QDir( requestedWorkspaceRoot ).absolutePath();
  };
  auto refreshTrustWorkspace = [trustWorkspace, dialogTrustRoot, &trustRootForCheckbox]() {
    trustRootForCheckbox = dialogTrustRoot();
    const bool hasRoot = !trustRootForCheckbox.isEmpty();
    trustWorkspace->setEnabled( hasRoot );
    trustWorkspace->setChecked( hasRoot && QgsAiWorkspaceTrust::isTrusted( trustRootForCheckbox ) );
    trustWorkspace->setToolTip( hasRoot ? QObject::tr( "Current workspace: %1" ).arg( trustRootForCheckbox ) : QObject::tr( "No workspace configured." ) );
  };
  refreshTrustWorkspace();

  form->addRow( tr( "OpenAI endpoint" ), openAiEndpoint );
  form->addRow( tr( "OpenAI model" ), openAiModel );
  form->addRow( tr( "OpenAI API key" ), openAiKey );
  form->addRow( tr( "OpenRouter endpoint" ), openRouterEndpoint );
  form->addRow( tr( "OpenRouter model" ), openRouterModel );
  form->addRow( tr( "OpenRouter API key" ), openRouterKey );
  form->addRow( QString(), openRouterTestButton );
  form->addRow( QString(), openRouterTestResult );
  form->addRow( QString(), openRouterAutoRouting );
  form->addRow( tr( "Codex endpoint" ), codexEndpoint );
  form->addRow( tr( "Codex model" ), codexModel );
  form->addRow( tr( "Codex OAuth status" ), codexStatus );
  form->addRow( tr( "Codex login" ), codexButtons );
  form->addRow( tr( "Claude endpoint" ), claudeEndpoint );
  form->addRow( tr( "Claude model" ), claudeModel );
  form->addRow( tr( "Claude API key" ), claudeKey );
  form->addRow( QString(), claudeUseOAuth );
  form->addRow( tr( "Claude OAuth status" ), claudeOAuthStatus );
  form->addRow( tr( "Claude OAuth" ), claudeOAuthButtons );
  form->addRow( tr( "Plan backend endpoint" ), planEndpoint );
  form->addRow( tr( "Plan OAuth authcfg ID" ), planAuthCfg );
  form->addRow( tr( "Plan session token" ), planToken );
  form->addRow( tr( "AI workspace root" ), workspaceRootWidget );
  form->addRow( QString(), trustWorkspace );
  form->addRow( QString(), encryptionStatus );
  layout->addLayout( form );

  // ----------------------------------------------------------------------
  // Agent behavior: rules, skills, and the master switch for custom actions.
  // Mirrors the Cursor "Rules / Skills / Allow tool use" surface and is wired
  // to QgsAiAgentSessionManager::setAgentBehaviorSettings on accept.
  // ----------------------------------------------------------------------
  const QgsAiAgentBehaviorSettings currentBehavior = mSessionManager ? mSessionManager->agentBehaviorSettings() : QgsAiAgentBehaviorSettings();

  QFrame *behaviorSeparator = new QFrame( &dialog );
  behaviorSeparator->setFrameShape( QFrame::HLine );
  behaviorSeparator->setFrameShadow( QFrame::Sunken );
  layout->addWidget( behaviorSeparator );

  QLabel *behaviorTitle = new QLabel( tr( "Agent behavior" ), &dialog );
  QFont behaviorTitleFont = behaviorTitle->font();
  behaviorTitleFont.setBold( true );
  behaviorTitle->setFont( behaviorTitleFont );
  layout->addWidget( behaviorTitle );

  QFormLayout *behaviorForm = new QFormLayout();

  QCheckBox *allowCustomActions = new QCheckBox( tr( "Allow custom agent actions (tool use)" ), &dialog );
  allowCustomActions->setChecked( currentBehavior.allowCustomActions );
  allowCustomActions->setToolTip( tr( "When enabled, the agent can call tools like read_file, propose_edit, run_python. Destructive tools still require confirmation in their dedicated review dialogs." ) );
  behaviorForm->addRow( QString(), allowCustomActions );

  QTextEdit *rulesEdit = new QTextEdit( &dialog );
  rulesEdit->setAcceptRichText( false );
  rulesEdit->setPlaceholderText( tr( "Custom rules the agent must follow (e.g. 'Always answer in English', 'Prefer GeoPandas over osmnx', …)." ) );
  rulesEdit->setPlainText( currentBehavior.rulesText );
  rulesEdit->setFixedHeight( 90 );
  behaviorForm->addRow( tr( "Agent rules" ), rulesEdit );

  QTextEdit *skillsEdit = new QTextEdit( &dialog );
  skillsEdit->setAcceptRichText( false );
  skillsEdit->setPlaceholderText( tr( "Reusable skills/instructions the agent can leverage when relevant." ) );
  skillsEdit->setPlainText( currentBehavior.skillsText );
  skillsEdit->setFixedHeight( 90 );
  behaviorForm->addRow( tr( "Agent skills" ), skillsEdit );

  QCheckBox *loadWorkspaceRules = new QCheckBox( tr( "Also load rule files from workspace folder" ), &dialog );
  loadWorkspaceRules->setChecked( currentBehavior.loadWorkspaceRules );
  behaviorForm->addRow( QString(), loadWorkspaceRules );

  QLineEdit *rulesPathEdit = new QLineEdit( currentBehavior.rulesPath, &dialog );
  rulesPathEdit->setPlaceholderText( u".strata/rules"_s );
  behaviorForm->addRow( tr( "Rules folder (relative)" ), rulesPathEdit );

  QCheckBox *loadWorkspaceSkills = new QCheckBox( tr( "Also load skill files from workspace folder" ), &dialog );
  loadWorkspaceSkills->setChecked( currentBehavior.loadWorkspaceSkills );
  behaviorForm->addRow( QString(), loadWorkspaceSkills );

  QLineEdit *skillsPathEdit = new QLineEdit( currentBehavior.skillsPath, &dialog );
  skillsPathEdit->setPlaceholderText( u".strata/skills"_s );
  behaviorForm->addRow( tr( "Skills folder (relative)" ), skillsPathEdit );

  layout->addLayout( behaviorForm );

  // ----------------------------------------------------------------------
  // Workspace indexing (RAG): file + layer chunks → local embeddings provider.
  // ----------------------------------------------------------------------
  QFrame *indexingSeparator = new QFrame( &dialog );
  indexingSeparator->setFrameShape( QFrame::HLine );
  indexingSeparator->setFrameShadow( QFrame::Sunken );
  layout->addWidget( indexingSeparator );

  QLabel *indexingTitle = new QLabel( tr( "Workspace indexing (RAG)" ), &dialog );
  QFont indexingTitleFont = indexingTitle->font();
  indexingTitleFont.setBold( true );
  indexingTitle->setFont( indexingTitleFont );
  layout->addWidget( indexingTitle );

  QFormLayout *indexingForm = new QFormLayout();
  QgsSettings indexSettings;

  const bool canUseEmbeddings = mSessionManager && mSessionManager->workspaceIndex() && mSessionManager->workspaceIndex()->embeddingProviderAvailable();

  QComboBox *embeddingProvider = new QComboBox( &dialog );
  embeddingProvider->setObjectName( u"aiEmbeddingProviderComboBox"_s );
  for ( const QgsAiEmbeddingProviderUiEntry &entry : QgsAiEmbeddingProviderRegistry::providerUiEntries() )
  {
    embeddingProvider->addItem( entry.displayName, entry.providerId );
    const int row = embeddingProvider->count() - 1;
    if ( !entry.unavailableReason.isEmpty() )
      embeddingProvider->setItemData( row, entry.unavailableReason, Qt::ToolTipRole );
    if ( !entry.selectable )
    {
      if ( QStandardItemModel *itemModel = qobject_cast<QStandardItemModel *>( embeddingProvider->model() ) )
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
  const int embeddingProviderIndex = embeddingProvider->findData( configuredEmbeddingProvider );
  embeddingProvider->setCurrentIndex( embeddingProviderIndex >= 0 ? embeddingProviderIndex : 0 );
  indexingForm->addRow( tr( "Embedding provider" ), embeddingProvider );

  QLabel *embeddingStatusLabel = new QLabel( &dialog );
  embeddingStatusLabel->setObjectName( u"aiEmbeddingProviderStatusLabel"_s );
  embeddingStatusLabel->setWordWrap( true );
  QPushButton *downloadEmbeddingModelButton = new QPushButton( tr( "Download local E5 model" ), &dialog );
  downloadEmbeddingModelButton->setObjectName( u"aiDownloadEmbeddingModelButton"_s );

  // Remote embedding model id, shown only when a remote provider is selected.
  QLineEdit *remoteEmbeddingModel = new QLineEdit( &dialog );
  remoteEmbeddingModel->setObjectName( u"aiRemoteEmbeddingModelLineEdit"_s );
  QLabel *remoteEmbeddingModelLabel = new QLabel( tr( "Remote embedding model" ), &dialog );
  auto remoteEmbeddingModelSettingKey = []( const QString &providerId ) {
    return providerId.compare( u"openrouter"_s, Qt::CaseInsensitive ) == 0 ? u"ai/embeddings/openrouter/model"_s : u"ai/embeddings/openai/model"_s;
  };
  auto remoteEmbeddingModelDefault = []( const QString &providerId ) {
    return providerId.compare( u"openrouter"_s, Qt::CaseInsensitive ) == 0 ? u"openai/text-embedding-3-small"_s : u"text-embedding-3-small"_s;
  };
  auto refreshRemoteEmbeddingModelField = [embeddingProvider, remoteEmbeddingModel, remoteEmbeddingModelLabel, remoteEmbeddingModelSettingKey, remoteEmbeddingModelDefault]() {
    const QString providerId = embeddingProvider->currentData().toString();
    const bool remote = QgsAiEmbeddingProviderRegistry::isRemoteProviderId( providerId );
    remoteEmbeddingModel->setVisible( remote );
    remoteEmbeddingModelLabel->setVisible( remote );
    if ( remote )
    {
      QgsSettings settings;
      remoteEmbeddingModel->setText( settings.value( remoteEmbeddingModelSettingKey( providerId ), remoteEmbeddingModelDefault( providerId ) ).toString() );
      remoteEmbeddingModel->setPlaceholderText( remoteEmbeddingModelDefault( providerId ) );
    }
  };

  auto refreshEmbeddingStatusLabel = [embeddingProvider, embeddingStatusLabel, downloadEmbeddingModelButton]() {
    const QString providerId = embeddingProvider->currentData().toString();
    const bool e5Compiled = QgsAiEmbeddingProviderRegistry::providerIds().contains( QgsAiE5EmbeddingProvider::staticProviderId() );
    if ( QgsAiEmbeddingProviderRegistry::isRemoteProviderId( providerId ) )
    {
      embeddingStatusLabel->setText( tr( "Remote workspace indexing will use %1 only because it is explicitly selected here. Saved OpenAI/OpenRouter keys do not switch indexing by themselves." )
                                       .arg( QgsAiEmbeddingProviderRegistry::displayNameForProviderId( providerId ) ) );
      downloadEmbeddingModelButton->setVisible( false );
    }
    else if ( providerId == QgsAiE5EmbeddingProvider::staticProviderId() )
    {
      if ( !e5Compiled )
      {
        embeddingStatusLabel->setText( tr(
          "Local multilingual E5 is not available in this build because ONNX Runtime and SentencePiece support were not found at compile time. Use MinHash or rebuild Strata with those dependencies."
        ) );
        downloadEmbeddingModelButton->setVisible( false );
        downloadEmbeddingModelButton->setEnabled( false );
        return;
      }
      QString filesError;
      const QString modelDir = QgsAiE5EmbeddingProvider::activeModelDirectory();
      const bool filesAvailable = QgsAiE5EmbeddingProvider::modelFilesAvailable( modelDir, &filesError );
      const QString developerDir = QgsAiE5EmbeddingProvider::developerModelDirectory();
      downloadEmbeddingModelButton->setVisible( true );
      downloadEmbeddingModelButton->setEnabled( developerDir.isEmpty() );
      downloadEmbeddingModelButton->setToolTip(
        developerDir.isEmpty() ? tr( "Download the pinned multilingual E5 ONNX model and SentencePiece tokenizer to the local Strata model cache." )
                               : tr( "STRATA_AI_EMBEDDING_MODEL_DIR is set; unset it to use the downloaded cache from this dialog." )
      );
      embeddingStatusLabel->setText(
        filesAvailable ? tr( "Local multilingual E5 model files are installed in %1. Indexing runs on this computer without an API key." ).arg( modelDir )
                       : tr( "Local multilingual E5 model is not installed or not usable: %1\nDownload size: %2. Developers can set STRATA_AI_EMBEDDING_MODEL_DIR." )
                           .arg( filesError, humanBytes( QgsAiE5EmbeddingProvider::downloadSize() ) )
      );
    }
    else
    {
      embeddingStatusLabel->setText(
        e5Compiled ? tr( "Local MinHash fallback is available and runs on this computer without an API key. Semantic quality is lower than multilingual E5." )
                   : tr( "Local MinHash fallback is available and runs on this computer without an API key. Multilingual E5 requires ONNX Runtime and SentencePiece support in the Strata build." )
      );
      downloadEmbeddingModelButton->setVisible( false );
    }
  };
  refreshEmbeddingStatusLabel();
  refreshRemoteEmbeddingModelField();
  connect( embeddingProvider, &QComboBox::currentIndexChanged, &dialog, [refreshEmbeddingStatusLabel, refreshRemoteEmbeddingModelField]( int ) {
    refreshEmbeddingStatusLabel();
    refreshRemoteEmbeddingModelField();
  } );
  indexingForm->addRow( remoteEmbeddingModelLabel, remoteEmbeddingModel );
  indexingForm->addRow( QString(), embeddingStatusLabel );
  indexingForm->addRow( QString(), downloadEmbeddingModelButton );
  connect( downloadEmbeddingModelButton, &QPushButton::clicked, &dialog, [this, &dialog, refreshEmbeddingStatusLabel]() {
    QString error;
    if ( !downloadEmbeddingModelWithConsent( &dialog, &error ) )
    {
      if ( !error.contains( tr( "not approved" ), Qt::CaseInsensitive ) )
        QMessageBox::warning( &dialog, tr( "Embedding model download failed" ), error.isEmpty() ? tr( "Unknown error." ) : error );
      refreshEmbeddingStatusLabel();
      return;
    }

    emit embeddingProviderSettingsChanged();
    refreshEmbeddingStatusLabel();
    QMessageBox::information( &dialog, tr( "Embedding model downloaded" ), tr( "The local multilingual E5 embedding model is ready. Existing MinHash indexes will rebuild automatically when E5 is selected." ) );
  } );

  QCheckBox *automaticIndexing = new QCheckBox( tr( "Index workspace automatically in the background" ), &dialog );
  automaticIndexing->setObjectName( u"aiAutomaticIndexingCheckBox"_s );
  // Default ON only when an embedding provider is actually available; if the user
  // already chose a value, keep it. The toggle is disabled when no provider is available.
  const bool hasAutomaticSetting = indexSettings.contains( u"strata/index/automatic"_s );
  automaticIndexing->setChecked( hasAutomaticSetting ? indexSettings.value( u"strata/index/automatic"_s, true ).toBool() : canUseEmbeddings );
  automaticIndexing->setEnabled( canUseEmbeddings );
  automaticIndexing->setToolTip( tr( "When enabled, Strata refreshes the local retrieval index after opening or changing the project. The task is cancellable." ) );
  indexingForm->addRow( QString(), automaticIndexing );

  const bool hasLayerIndexingSetting = indexSettings.contains( u"strata/index/enable_layer_indexing"_s )
                                       || indexSettings.contains( u"geoai/index/enable_layer_indexing"_s )
                                       || indexSettings.contains( u"qgis_ai/index/enable_layer_indexing"_s );
  const bool defaultLayerIndexingEnabled = automaticIndexing->isChecked();
  const bool requestedLayerIndexing
    = hasLayerIndexingSetting
        ? settingValueWithLegacy( indexSettings, u"strata/index/enable_layer_indexing"_s, QStringList { u"geoai/index/enable_layer_indexing"_s, u"qgis_ai/index/enable_layer_indexing"_s }, false ).toBool()
        : defaultLayerIndexingEnabled;
  const bool layerIndexingEnabled = requestedLayerIndexing && canUseEmbeddings;

  QCheckBox *enableLayerIndexing = new QCheckBox( tr( "Enable layer indexing (auto reindex on layer add/remove/edit)" ), &dialog );
  enableLayerIndexing->setObjectName( u"aiEnableLayerIndexingCheckBox"_s );
  enableLayerIndexing->setChecked( layerIndexingEnabled );
  enableLayerIndexing->setEnabled( canUseEmbeddings );
  enableLayerIndexing->setToolTip(
    tr( "When enabled, layer attributes and bounding boxes are embedded with the selected indexing provider and indexed so the assistant can ground its answers on actual layer data." )
  );
  indexingForm->addRow( QString(), enableLayerIndexing );

  QLabel *indexStatusLabel = new QLabel( &dialog );
  indexStatusLabel->setObjectName( u"aiIndexStatusLabel"_s );
  if ( mSessionManager && mSessionManager->workspaceIndex() )
  {
    mSessionManager->workspaceIndex()->ensureLoaded();
    const auto status = mSessionManager->workspaceIndex()->status();
    indexStatusLabel->setText( tr( "Indexed: %1 file chunks, %2 layer chunks (last sync: %3)" )
                                 .arg( status.fileChunkCount )
                                 .arg( status.layerChunkCount )
                                 .arg( status.lastSync.isValid() ? status.lastSync.toLocalTime().toString( Qt::ISODate ) : tr( "never" ) ) );
  }
  else
  {
    indexStatusLabel->setText( tr( "Indexed: (workspace index unavailable)" ) );
  }
  indexingForm->addRow( QString(), indexStatusLabel );

  auto refreshIndexStatusLabel = [this, indexStatusLabel]() {
    if ( mSessionManager && mSessionManager->workspaceIndex() )
    {
      mSessionManager->workspaceIndex()->ensureLoaded();
      const auto status = mSessionManager->workspaceIndex()->status();
      indexStatusLabel->setText( tr( "Indexed: %1 file chunks, %2 layer chunks (last sync: %3)" )
                                   .arg( status.fileChunkCount )
                                   .arg( status.layerChunkCount )
                                   .arg( status.lastSync.isValid() ? status.lastSync.toLocalTime().toString( Qt::ISODate ) : tr( "never" ) ) );
    }
    else
    {
      indexStatusLabel->setText( tr( "Indexed: (workspace index unavailable)" ) );
    }
  };

  auto ensureEmbeddingProvider = [this, &dialog]() {
    if ( mSessionManager && mSessionManager->workspaceIndex() && mSessionManager->workspaceIndex()->embeddingProviderAvailable() )
      return true;

    QMessageBox::information( &dialog, tr( "Workspace indexing unavailable" ), tr( "Workspace indexing requires the selected embedding provider to be available. Chat with Claude/Codex works without indexing." ) );
    return false;
  };

  // Rebuilding can be expensive: heavy local CPU usage, or remote API cost and data egress.
  // Always confirm before starting, with a message tailored to the selected provider.
  auto confirmRebuild = [&dialog, embeddingProvider]( const QString &what ) {
    const QString providerId = embeddingProvider->currentData().toString();
    const QString message = QgsAiEmbeddingProviderRegistry::isRemoteProviderId( providerId )
                              ? tr( "Rebuilding the %1 will re-embed content with the remote provider %2. This sends data to that service and may incur API costs.\n\nProceed?" )
                                  .arg( what, QgsAiEmbeddingProviderRegistry::displayNameForProviderId( providerId ) )
                              : tr( "Rebuilding the %1 will re-embed content on this computer and may use significant CPU for a while.\n\nProceed?" ).arg( what );
    return QMessageBox::question( &dialog, tr( "Rebuild index" ), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes;
  };

  QPushButton *rebuildWorkspaceIndexButton = new QPushButton( tr( "Rebuild file/workspace index now" ), &dialog );
  rebuildWorkspaceIndexButton->setObjectName( u"aiRebuildWorkspaceIndexButton"_s );
  rebuildWorkspaceIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
  indexingForm->addRow( QString(), rebuildWorkspaceIndexButton );

  QPushButton *rebuildLayerIndexButton = new QPushButton( tr( "Rebuild layer index now" ), &dialog );
  rebuildLayerIndexButton->setObjectName( u"aiRebuildLayerIndexButton"_s );
  rebuildLayerIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
  indexingForm->addRow( QString(), rebuildLayerIndexButton );

  connect( rebuildWorkspaceIndexButton, &QPushButton::clicked, &dialog, [this, &dialog, rebuildWorkspaceIndexButton, ensureEmbeddingProvider, confirmRebuild, refreshIndexStatusLabel]() {
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
      QMessageBox::warning( &dialog, tr( "Workspace reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
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
        QMessageBox::warning( &dialog, tr( "Workspace reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
        return;
      }
      refreshIndexStatusLabel();
      const auto status = mSessionManager->workspaceIndex()->status();
      QMessageBox::information( &dialog, tr( "Workspace reindex" ), tr( "Done — %1 file chunks indexed." ).arg( status.fileChunkCount ) );
      return;
    }

    rebuildWorkspaceIndexButton->setEnabled( false );
    ManualWorkspaceIndexTask *task = new ManualWorkspaceIndexTask( mSessionManager->workspaceIndex(), workspaceRoot, snapshot );
    connect( task, &QgsTask::taskCompleted, &dialog, [this, &dialog, rebuildWorkspaceIndexButton, refreshIndexStatusLabel]() {
      rebuildWorkspaceIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
      refreshIndexStatusLabel();
      if ( mSessionManager && mSessionManager->workspaceIndex() )
      {
        const auto status = mSessionManager->workspaceIndex()->status();
        QMessageBox::information( &dialog, tr( "Workspace reindex" ), tr( "Done — %1 file chunks indexed." ).arg( status.fileChunkCount ) );
      }
    } );
    connect( task, &QgsTask::taskTerminated, &dialog, [&dialog, rebuildWorkspaceIndexButton, task]() {
      rebuildWorkspaceIndexButton->setEnabled( true );
      const QString taskError = task->errorMessage();
      if ( !taskError.isEmpty() )
        QMessageBox::warning( &dialog, QObject::tr( "Workspace reindex failed" ), taskError );
    } );
    taskManager->addTask( task, 1 );
  } );

  connect( rebuildLayerIndexButton, &QPushButton::clicked, &dialog, [this, &dialog, rebuildLayerIndexButton, ensureEmbeddingProvider, confirmRebuild, refreshIndexStatusLabel]() {
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
      QMessageBox::warning( &dialog, tr( "Layer reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
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
        QMessageBox::warning( &dialog, tr( "Layer reindex failed" ), err.isEmpty() ? tr( "Unknown error." ) : err );
        return;
      }
      refreshIndexStatusLabel();
      const auto status = mSessionManager->workspaceIndex()->status();
      QMessageBox::information( &dialog, tr( "Layer reindex" ), tr( "Done — %1 layer chunks indexed." ).arg( status.layerChunkCount ) );
      return;
    }

    rebuildLayerIndexButton->setEnabled( false );
    ManualLayerIndexTask *task = new ManualLayerIndexTask( mSessionManager->workspaceIndex(), snapshot );
    connect( task, &QgsTask::taskCompleted, &dialog, [this, &dialog, rebuildLayerIndexButton, refreshIndexStatusLabel]() {
      rebuildLayerIndexButton->setEnabled( mSessionManager && mSessionManager->workspaceIndex() );
      refreshIndexStatusLabel();
      if ( mSessionManager && mSessionManager->workspaceIndex() )
      {
        const auto status = mSessionManager->workspaceIndex()->status();
        QMessageBox::information( &dialog, tr( "Layer reindex" ), tr( "Done — %1 layer chunks indexed." ).arg( status.layerChunkCount ) );
      }
    } );
    connect( task, &QgsTask::taskTerminated, &dialog, [&dialog, rebuildLayerIndexButton, task]() {
      rebuildLayerIndexButton->setEnabled( true );
      const QString taskError = task->errorMessage();
      if ( !taskError.isEmpty() )
        QMessageBox::warning( &dialog, QObject::tr( "Layer reindex failed" ), taskError );
    } );
    taskManager->addTask( task, 1 );
  } );

  layout->addLayout( indexingForm );

  QLabel *helpLabel = new QLabel(
    tr(
      "OpenAI, OpenRouter and Claude API keys are stored locally in application settings. The Codex OAuth refresh token is stored locally in application settings; the Claude OAuth refresh token is "
      "stored in the "
      "encrypted "
      "QGIS authentication store. Leave API key fields empty to keep "
      "the current saved value.\n\nAgent rules and skills are stored locally in application settings. When the workspace toggle is enabled, .md/.txt files inside the configured folder are appended "
      "to the "
      "prompt. The AI workspace root is used only when the current QGIS project has no home path. Custom actions remain subject to the existing review/approval dialogs."
    ),
    &dialog
  );
  helpLabel->setWordWrap( true );
  layout->addWidget( helpLabel );

  connect( codexRequestCodeButton, &QPushButton::clicked, &dialog, [&dialog, &codexDeviceCode, codexStatus]() {
    QString error;
    if ( !QgsAiCodexOAuthClient::requestDeviceCode( codexDeviceCode, &error ) )
    {
      QMessageBox::warning( &dialog, QObject::tr( "Codex login failed" ), error );
      return;
    }

    codexStatus->setText( QObject::tr( "Open %1 and enter code %2" ).arg( codexDeviceCode.verificationUrl, codexDeviceCode.userCode ) );
    QDesktopServices::openUrl( QUrl( codexDeviceCode.verificationUrl ) );
    QMessageBox::information(
      &dialog, QObject::tr( "Codex device code" ), QObject::tr( "Open %1 and enter this code:\n\n%2\n\nThen click Complete Codex login." ).arg( codexDeviceCode.verificationUrl, codexDeviceCode.userCode )
    );
  } );

  connect( codexCompleteLoginButton, &QPushButton::clicked, &dialog, [&dialog, &codexDeviceCode, codexStatus]() {
    if ( codexDeviceCode.deviceAuthId.isEmpty() )
    {
      QMessageBox::information( &dialog, QObject::tr( "Codex login" ), QObject::tr( "Request a Codex device code first." ) );
      return;
    }

    QString error;
    if ( !QgsAiCodexOAuthClient::completeDeviceCodeLogin( codexDeviceCode, &error ) )
    {
      QMessageBox::warning( &dialog, QObject::tr( "Codex login failed" ), error );
      return;
    }
    codexStatus->setText( QObject::tr( "Signed in" ) );
  } );

  connect( codexLogoutButton, &QPushButton::clicked, &dialog, [&dialog, codexStatus]() {
    QString error;
    if ( !QgsAiCodexOAuthClient::clearRefreshToken( &error ) )
    {
      QMessageBox::warning( &dialog, QObject::tr( "Codex logout failed" ), error );
      return;
    }
    codexStatus->setText( QObject::tr( "Not signed in" ) );
  } );

  connect( claudeLoginButton, &QPushButton::clicked, &dialog, [&dialog, claudeOAuthStatus, claudeUseOAuth]() {
    const QgsAiClaudeOAuthClient::AuthorizationRequest authRequest = QgsAiClaudeOAuthClient::buildAuthorizationRequest();

    if ( !QDesktopServices::openUrl( authRequest.authorizationUrl ) )
    {
      QMessageBox::information( &dialog, QObject::tr( "Claude OAuth" ), QObject::tr( "Open this URL in your browser:\n\n%1" ).arg( authRequest.authorizationUrl.toString() ) );
    }

    bool ok = false;
    const QString code
      = QInputDialog::getText( &dialog, QObject::tr( "Claude OAuth" ), QObject::tr( "After approving Claude in the browser, paste the authorization code or callback URL:" ), QLineEdit::Normal, QString(), &ok )
          .trimmed();
    if ( !ok || code.isEmpty() )
      return;

    QString error;
    if ( !QgsAiClaudeOAuthClient::exchangeAuthorizationCode( code, authRequest.codeVerifier, authRequest.redirectUri, authRequest.state, &error ) )
    {
      QMessageBox::warning( &dialog, QObject::tr( "Claude login failed" ), error );
      return;
    }
    claudeUseOAuth->setChecked( true );
    claudeOAuthStatus->setText( QObject::tr( "Signed in" ) );
  } );

  connect( claudeLogoutButton, &QPushButton::clicked, &dialog, [&dialog, claudeOAuthStatus]() {
    QString error;
    if ( !QgsAiClaudeOAuthClient::clearRefreshToken( &error ) )
    {
      QMessageBox::warning( &dialog, QObject::tr( "Claude logout failed" ), error );
      return;
    }
    claudeOAuthStatus->setText( QObject::tr( "Not signed in" ) );
  } );

  connect( browseWorkspaceRoot, &QPushButton::clicked, &dialog, [&dialog, aiWorkspaceRoot]() {
    const QString dir = QFileDialog::getExistingDirectory( &dialog, QObject::tr( "Choose AI workspace root" ), aiWorkspaceRoot->text().trimmed() );
    if ( !dir.isEmpty() )
      aiWorkspaceRoot->setText( QDir::cleanPath( dir ) );
  } );
  connect( aiWorkspaceRoot, &QLineEdit::textChanged, &dialog, [refreshTrustWorkspace]( const QString & ) { refreshTrustWorkspace(); } );

  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog );
  connect( buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject );
  scrollArea->setWidget( settingsContent );
  dialogLayout->addWidget( scrollArea );
  dialogLayout->addWidget( buttons );

  dialog.setMinimumSize( 480, 360 );
  if ( const QScreen *screen = QApplication::primaryScreen() )
  {
    const QSize availableSize = screen->availableGeometry().size();
    const int dialogWidth = std::min( 720, std::max( 480, availableSize.width() - 100 ) );
    const int dialogHeight = std::min( 640, std::max( 360, availableSize.height() - 120 ) );
    dialog.resize( dialogWidth, dialogHeight );
  }
  else
  {
    dialog.resize( 680, 620 );
  }

  if ( dialog.exec() != QDialog::Accepted )
    return;

  const QString pendingOpenAiKey = openAiKey->text().trimmed();
  const QString pendingOpenRouterKey = openRouterKey->text().trimmed();
  const QString pendingClaudeKey = claudeKey->text().trimmed();
  const QString pendingPlanToken = planToken->text().trimmed();

  QString errorMessages;
  QString error;

  QgsAiModelRouter::ProviderSettings openAiSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi );
  openAiSettings.endpoint = openAiEndpoint->text().trimmed();
  openAiSettings.model = openAiModel->text().trimmed();
  openAiSettings.enabled = !pendingOpenAiKey.isEmpty() || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi ) || !qEnvironmentVariable( "OPENAI_API_KEY" ).trimmed().isEmpty();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::OpenAi, openAiSettings );

  QgsAiModelRouter::ProviderSettings openRouterSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenRouter );
  openRouterSettings.endpoint = openRouterEndpoint->text().trimmed();
  openRouterSettings.model = openRouterModel->currentText().trimmed();
  openRouterSettings.autoRouting = openRouterAutoRouting->isChecked();
  openRouterSettings.enabled = !pendingOpenRouterKey.isEmpty()
                               || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenRouter )
                               || !qEnvironmentVariable( "OPENROUTER_API_KEY" ).trimmed().isEmpty();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::OpenRouter, openRouterSettings );

  QgsAiModelRouter::ProviderSettings codexSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex );
  codexSettings.endpoint = codexEndpoint->text().trimmed();
  codexSettings.model = codexModel->text().trimmed();
  codexSettings.credentialMode = QgsAiModelRouter::CredentialMode::OAuth;
  codexSettings.enabled = QgsAiCodexOAuthClient::hasRefreshToken();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Codex, codexSettings );

  QgsAiModelRouter::ProviderSettings planSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan );
  planSettings.endpoint = planEndpoint->text().trimmed();
  planSettings.authConfigId = planAuthCfg->text().trimmed();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Plan, planSettings );
  mModelRouter->setPlanAuthConfigId( planAuthCfg->text().trimmed() );

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
    const QString requestedWorkspaceRoot = aiWorkspaceRoot->text().trimmed();
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
    if ( !trustRootForCheckbox.isEmpty() )
      QgsAiWorkspaceTrust::setState( trustRootForCheckbox, trustWorkspace->isChecked() ? QgsAiWorkspaceTrust::State::Trusted : QgsAiWorkspaceTrust::State::Untrusted );

    QgsAiEmbeddingProviderRegistry::setConfiguredProviderId( embeddingProvider->currentData().toString() );
    if ( QgsAiEmbeddingProviderRegistry::isRemoteProviderId( embeddingProvider->currentData().toString() ) )
    {
      const QString embeddingModelKey = remoteEmbeddingModelSettingKey( embeddingProvider->currentData().toString() );
      const QString embeddingModelValue = remoteEmbeddingModel->text().trimmed();
      if ( embeddingModelValue.isEmpty() )
        settings.remove( embeddingModelKey );
      else
        settings.setValue( embeddingModelKey, embeddingModelValue );
    }
    settings.setValue( u"strata/index/automatic"_s, automaticIndexing->isChecked() );
  }

  emit embeddingProviderSettingsChanged();

  QgsAiModelRouter::ProviderSettings claudeSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude );
  claudeSettings.endpoint = claudeEndpoint->text().trimmed();
  claudeSettings.model = claudeModel->text().trimmed();
  const bool claudeOAuthRequested = claudeUseOAuth->isChecked();
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

  if ( mSessionManager )
  {
    QgsAiAgentBehaviorSettings behaviorSettings = mSessionManager->agentBehaviorSettings();
    behaviorSettings.allowCustomActions = allowCustomActions->isChecked();
    behaviorSettings.rulesText = rulesEdit->toPlainText();
    behaviorSettings.skillsText = skillsEdit->toPlainText();
    behaviorSettings.loadWorkspaceRules = loadWorkspaceRules->isChecked();
    behaviorSettings.loadWorkspaceSkills = loadWorkspaceSkills->isChecked();
    behaviorSettings.rulesPath = rulesPathEdit->text().trimmed();
    behaviorSettings.skillsPath = skillsPathEdit->text().trimmed();
    mSessionManager->setAgentBehaviorSettings( behaviorSettings );

    const bool canUseEmbeddings = mSessionManager->workspaceIndex() && mSessionManager->workspaceIndex()->embeddingProviderAvailable();
    bool layerIndexingChoice = enableLayerIndexing->isChecked() && canUseEmbeddings;

    if ( enableLayerIndexing->isChecked() && !canUseEmbeddings )
    {
      errorMessages += tr( "Layer indexing requires the selected embedding provider to be available." ) + '\n';
      enableLayerIndexing->setChecked( false );
    }

    // Gate first-time layer indexing so users explicitly acknowledge the local
    // background work and local cache before it starts.
    if ( layerIndexingChoice && requiresLayerIndexingConsent() )
    {
      const auto choice = QMessageBox::question(
        &dialog,
        tr( "Enable layer indexing" ),
        QgsAiEmbeddingProviderRegistry::isRemoteProviderId( embeddingProvider->currentData().toString() )
          ? tr( "Enabling layer indexing means Strata will send sampled layer attributes and bounding boxes to the selected remote embedding provider and store the resulting index locally.\n\nProceed?" )
          : tr( "Enabling layer indexing means Strata will process sampled layer attributes and bounding boxes on this computer and store the resulting index locally.\n\nProceed?" ),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
      );
      if ( choice != QMessageBox::Yes )
      {
        layerIndexingChoice = false;
        enableLayerIndexing->setChecked( false );
      }
      else
      {
        recordLayerIndexingConsent();
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

void QgsAiChatDockWidget::showEvent( QShowEvent *event )
{
  QgsDockWidget::showEvent( event );
  maybeShowWelcomeBanner();
}

void QgsAiChatDockWidget::maybeShowWelcomeBanner()
{
  QgsSettings settings;
  if ( settingValueWithLegacy( settings, u"strata/welcome_seen"_s, QStringList { u"geoai/welcome_seen"_s, u"qgis_ai/welcome_seen"_s }, false ).toBool() )
    return;

  if ( !mModelRouter )
    return;

  // If the user already has a key for any of the standard providers, don't
  // bother them — just remember we've seen it and move on.
  if ( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi )
       || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenRouter )
       || mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Codex )
       || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude )
       || mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Claude )
       || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Plan ) )
  {
    settings.setValue( u"strata/welcome_seen"_s, true );
    settings.remove( u"geoai/welcome_seen"_s );
    settings.remove( u"qgis_ai/welcome_seen"_s );
    return;
  }

  QgsMessageBar *messageBar = QgisApp::instance() ? QgisApp::instance()->messageBar() : nullptr;
  if ( !messageBar )
    return;

  QPushButton *settingsButton = new QPushButton( tr( "Open AI settings" ) );
  QgsMessageBarItem *item
    = new QgsMessageBarItem( tr( "AI Assistant" ), tr( "Configure an API key or login with Codex/Claude to start using the AI assistant." ), settingsButton, Qgis::MessageLevel::Info, 0, messageBar );

  connect( settingsButton, &QPushButton::clicked, this, [this, messageBar, item]() {
    openProviderSettings();
    messageBar->popWidget( item );
  } );

  messageBar->pushItem( item );
  settings.setValue( u"strata/welcome_seen"_s, true );
  settings.remove( u"geoai/welcome_seen"_s );
  settings.remove( u"qgis_ai/welcome_seen"_s );
}

void QgsAiChatDockWidget::setLayerIndexCoordinator( QgsAiLayerIndexCoordinator *coordinator )
{
  mLayerIndexCoordinator = coordinator;
}

bool QgsAiChatDockWidget::requiresLayerIndexingConsent()
{
  QgsSettings settings;
  return !settingValueWithLegacy( settings, u"strata/index/layer_indexing_consented"_s, QStringList { u"geoai/index/layer_indexing_consented"_s, u"qgis_ai/index/layer_indexing_consented"_s }, false ).toBool();
}

void QgsAiChatDockWidget::recordLayerIndexingConsent()
{
  QgsSettings settings;
  settings.setValue( u"strata/index/layer_indexing_consented"_s, true );
  settings.remove( u"geoai/index/layer_indexing_consented"_s );
  settings.remove( u"qgis_ai/index/layer_indexing_consented"_s );
}
