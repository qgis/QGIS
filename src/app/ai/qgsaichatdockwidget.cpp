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

#include "ai/index/qgsaicloudindexclient.h"
#include "ai/index/qgsaiembeddingprovider.h"
#include "ai/index/qgsailayerindexcoordinator.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "qgisapp.h"
#include "qgsaichatpromptedit.h"
#include "qgsaiclaudeoauthclient.h"
#include "qgsaicodexoauthclient.h"
#include "qgsaigissuggestionengine.h"
#include "qgsaimodelrouter.h"
#include "qgsaiopenroutermodelcatalog.h"
#include "qgsaiplanclient.h"
#include "qgsaireviewpatchengine.h"
#include "qgsaisecretstore.h"
#include "qgsaisettingsdialog.h"
#include "qgsaisettingsutils.h"
#include "qgsaivisualcontextutils.h"
#include "qgsaiworkspacetrust.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslayoutmanager.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsscrollarea.h"
#include "qgssettings.h"
#include "qgstaskmanager.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

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
#include <QDateTime>
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
#include <QSaveFile>
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

using QgsAiSettingsUtils::humanBytes;
using QgsAiSettingsUtils::settingValueWithLegacy;

namespace
{
  struct ModelEntry
  {
      QString displayName;
      QString model;
      QgsAiModelRouter::Provider provider;
      QString tooltip = QString();
  };

  QVector<ModelEntry> predefinedModels()
  {
    return {
      { u"GPT-4o"_s, u"gpt-4o"_s, QgsAiModelRouter::Provider::OpenAi },
      { u"GPT-4.1 mini"_s, u"gpt-4.1-mini"_s, QgsAiModelRouter::Provider::OpenAi },
      { u"Claude Sonnet 4.6"_s, u"anthropic/claude-sonnet-4.6"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"Qwen3 235B"_s, u"qwen/qwen3-235b-a22b"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"DeepSeek V4 Flash"_s, u"deepseek/deepseek-v4-flash"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"DeepSeek R1"_s, u"deepseek/deepseek-r1"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"Kimi K2"_s, u"moonshotai/kimi-k2"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"OpenRouter Auto"_s, u"openrouter/auto"_s, QgsAiModelRouter::Provider::OpenRouter },
      { u"Codex GPT-5.4"_s, u"gpt-5.4"_s, QgsAiModelRouter::Provider::Codex },
      { u"Codex GPT-5.3 (codex)"_s, u"gpt-5.3-codex"_s, QgsAiModelRouter::Provider::Codex },
      { u"Claude Opus 4.8"_s, u"claude-opus-4-8"_s, QgsAiModelRouter::Provider::Claude },
      { u"Claude Sonnet 5"_s, u"claude-sonnet-5"_s, QgsAiModelRouter::Provider::Claude },
      { u"Claude Haiku 4.5"_s, u"claude-haiku-4-5"_s, QgsAiModelRouter::Provider::Claude },
      { u"Strata Managed"_s, u"managed-plan"_s, QgsAiModelRouter::Provider::Plan },
    };
  }

  bool isChatPlanModel( const QgsAiPlanClient::ModelInfo &model )
  {
    if ( model.capabilities.isEmpty() )
      return true;

    bool hasChatCapability = false;
    bool hasEmbeddingCapability = false;
    for ( const QString &capability : model.capabilities )
    {
      const QString normalized = capability.trimmed().toLower();
      if ( normalized == "chat"_L1 || normalized == "tools"_L1 || normalized == "tool-calling"_L1 || normalized == "vision"_L1 || normalized == "reasoning"_L1 || normalized == "text-generation"_L1 )
        hasChatCapability = true;
      if ( normalized.contains( "embed"_L1 ) )
        hasEmbeddingCapability = true;
    }

    return hasChatCapability || !hasEmbeddingCapability;
  }

  QVector<ModelEntry> cachedPlanModelEntries()
  {
    QVector<ModelEntry> entries;
    const QList<QgsAiPlanClient::ModelInfo> models = QgsAiPlanClient::cachedModels();
    const QgsAiManagedAgentPolicy policy = QgsAiPlanClient::cachedAgentPolicy();
    entries.reserve( models.size() );
    for ( const QgsAiPlanClient::ModelInfo &model : models )
    {
      if ( model.id.isEmpty() )
        continue;
      if ( !isChatPlanModel( model ) )
        continue;
      // Infra aliases: the managed-plan legacy default (routes to Lite) and the Pro fallback router.
      // Both route/fall back fine but must not be user-selectable — the picker exposes only the three
      // tiers Lite/Standard/Pro.
      if ( model.id == "managed-plan"_L1 || model.id == "openrouter/auto"_L1 )
        continue;
      if ( !policy.allowedModels.isEmpty() && !policy.allowedModels.contains( model.id ) )
        continue;
      // User-level preference from the Account "Models" list — takes effect on top of the tier policy.
      if ( QgsAiPlanClient::isModelDisabled( model.id ) )
        continue;
      entries.append( ModelEntry { model.displayLabel(), model.id, QgsAiModelRouter::Provider::Plan, model.tooltip() } );
    }
    return entries;
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
    if ( label == "Ask before edits"_L1 )
      return u"ask_before_edits"_s;
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
    if ( agent == "ask_before_edits"_L1 )
      return u"Ask before edits"_s;
    if ( agent == "reviewer"_L1 )
      return u"Ask"_s;
    return u"Plan"_s;
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

  QString removeAgentPlanProtocolBlocks( const QString &text )
  {
    QString stripped = text;
    static const QRegularExpression planRe( u"```strata_agent_plan\\s*\\n[\\s\\S]*?\\n```"_s, QRegularExpression::CaseInsensitiveOption );
    stripped.remove( planRe );
    return stripped.trimmed();
  }

  QJsonObject planJsonFromMetadata( const QVariantMap &metadata )
  {
    const QString json = metadata.value( u"plan_json"_s ).toString();
    if ( json.isEmpty() )
      return QJsonObject();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson( json.toUtf8(), &parseError );
    if ( parseError.error != QJsonParseError::NoError || !doc.isObject() )
      return QJsonObject();
    return doc.object();
  }

  QJsonObject planJsonForExecution( const QString &planMarkdown, const QVariantMap &metadata )
  {
    QJsonObject plan = planJsonFromMetadata( metadata );
    if ( !plan.isEmpty() )
      return plan;
    return QgsAiAgentSessionManager::extractAgentPlanJson( planMarkdown );
  }

  QStringList planToolNames( const QJsonObject &plan )
  {
    QStringList tools;
    QSet<QString> seen;
    const QJsonArray steps = plan.value( u"steps"_s ).toArray();
    for ( const QJsonValue &value : steps )
    {
      if ( !value.isObject() )
        continue;
      const QString tool = value.toObject().value( u"tool"_s ).toString().trimmed();
      if ( tool.isEmpty() || seen.contains( tool ) )
        continue;
      seen.insert( tool );
      tools << tool;
    }
    return tools;
  }

  QStringList jsonStringArray( const QJsonArray &array )
  {
    QStringList strings;
    for ( const QJsonValue &value : array )
    {
      const QString item = value.toString().trimmed();
      if ( !item.isEmpty() )
        strings << item;
    }
    return strings;
  }

  QString agentPlanMarkdown( const QJsonObject &plan )
  {
    if ( plan.isEmpty() )
      return QString();

    QStringList lines;
    const QString objective = plan.value( u"objective"_s ).toString().trimmed();
    const QString mode = plan.value( u"mode"_s ).toString().trimmed();
    if ( !objective.isEmpty() )
      lines << u"**Objective:** %1"_s.arg( objective );
    if ( !mode.isEmpty() )
      lines << u"**Mode:** %1"_s.arg( mode );
    if ( !lines.isEmpty() )
      lines << QString();

    const QJsonArray steps = plan.value( u"steps"_s ).toArray();
    int index = 1;
    for ( const QJsonValue &value : steps )
    {
      if ( !value.isObject() )
        continue;
      const QJsonObject step = value.toObject();
      const QString title = step.value( u"title"_s ).toString().trimmed();
      if ( title.isEmpty() )
        continue;

      QStringList attributes;
      const QString id = step.value( u"id"_s ).toString().trimmed();
      const QString tool = step.value( u"tool"_s ).toString().trimmed();
      const QString risk = step.value( u"risk"_s ).toString().trimmed();
      const QStringList dependsOn = jsonStringArray( step.value( u"depends_on"_s ).toArray() );
      if ( !id.isEmpty() )
        attributes << u"id: %1"_s.arg( id );
      if ( !tool.isEmpty() )
        attributes << u"tool: %1"_s.arg( tool );
      if ( !risk.isEmpty() )
        attributes << u"risk: %1"_s.arg( risk );
      if ( step.contains( u"requires_approval"_s ) )
        attributes << u"approval: %1"_s.arg( step.value( u"requires_approval"_s ).toBool() ? u"yes"_s : u"no"_s );
      if ( !dependsOn.isEmpty() )
        attributes << u"depends on: %1"_s.arg( dependsOn.join( ", "_L1 ) );

      QString line = u"%1. %2"_s.arg( index++ ).arg( title );
      if ( !attributes.isEmpty() )
        line += u" (%1)"_s.arg( attributes.join( "; "_L1 ) );
      lines << line;
    }

    return lines.join( '\n' ).trimmed();
  }

  QString planMarkdownFromMetadata( const QVariantMap &metadata )
  {
    const QString markdown = metadata.value( u"plan_markdown"_s ).toString().trimmed();
    if ( !markdown.isEmpty() )
      return markdown;
    return agentPlanMarkdown( planJsonFromMetadata( metadata ) );
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

  QString workflowSlug( const QString &planMarkdown )
  {
    QString slug = planMarkdown.simplified().left( 48 ).toLower();
    slug.replace( QRegularExpression( u"[^a-z0-9]+"_s ), u"-"_s );
    slug = slug.trimmed();
    while ( slug.startsWith( '-' ) )
      slug.remove( 0, 1 );
    while ( slug.endsWith( '-' ) )
      slug.chop( 1 );
    if ( slug.isEmpty() )
      slug = u"workflow"_s;
    const QString hash = QString::fromLatin1( QCryptographicHash::hash( planMarkdown.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 8 ) );
    return u"%1-%2"_s.arg( slug.left( 40 ), hash );
  }

  QJsonArray workflowStepsFromMarkdown( const QString &planMarkdown )
  {
    QJsonArray steps;
    const QStringList lines = planMarkdown.split( '\n' );
    int index = 1;
    for ( QString line : lines )
    {
      line = line.trimmed();
      line.remove( QRegularExpression( u"^[-*]\\s+"_s ) );
      line.remove( QRegularExpression( u"^\\d+[.)]\\s+"_s ) );
      if ( line.isEmpty() )
        continue;
      if ( line.startsWith( '#'_L1 ) || line.startsWith( "**Objective:**"_L1 ) || line.startsWith( "**Mode:**"_L1 ) || line.startsWith( "Objective:"_L1 ) || line.startsWith( "Mode:"_L1 ) )
        continue;
      QJsonObject step;
      step.insert( u"id"_s, u"step_%1"_s.arg( index++ ) );
      step.insert( u"text"_s, line );
      step.insert( u"status"_s, u"planned"_s );
      steps << step;
    }
    return steps;
  }

  QJsonObject workflowProvenance( const QString &workspaceRoot )
  {
    QJsonObject provenance;
    provenance.insert( u"savedBy"_s, u"strata-desktop"_s );
    provenance.insert( u"metadataOnly"_s, true );
    provenance.insert( u"workspaceRootHash"_s, QString::fromLatin1( QCryptographicHash::hash( QDir::cleanPath( workspaceRoot ).toUtf8(), QCryptographicHash::Sha1 ).toHex() ) );
    return provenance;
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

  mGisCardContainer = new QFrame( container );
  mGisCardContainer->setObjectName( u"aiGisSuggestionCard"_s );
  mGisCardContainer->setStyleSheet( u"QFrame#aiGisSuggestionCard { color: palette(window-text); background: palette(button); border: 0; border-radius: 10px; }"_s );
  QVBoxLayout *gisCardLayout = new QVBoxLayout( mGisCardContainer );
  gisCardLayout->setContentsMargins( 6, 3, 6, 3 );
  gisCardLayout->setSpacing( 2 );

  mGisCardToggle = new QToolButton( mGisCardContainer );
  mGisCardToggle->setObjectName( u"aiGisCardToggle"_s );
  mGisCardToggle->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  mGisCardToggle->setArrowType( Qt::DownArrow );
  mGisCardToggle->setCheckable( true );
  mGisCardToggle->setChecked( true );
  mGisCardToggle->setAutoRaise( true );
  gisCardLayout->addWidget( mGisCardToggle );

  mGisCardBody = new QWidget( mGisCardContainer );
  mGisCardBodyLayout = new QVBoxLayout( mGisCardBody );
  mGisCardBodyLayout->setContentsMargins( 0, 0, 0, 0 );
  mGisCardBodyLayout->setSpacing( 2 );
  gisCardLayout->addWidget( mGisCardBody );

  mGisCardContainer->setVisible( false );
  layout->addWidget( mGisCardContainer );

  mFileContextChipRow = new QWidget( container );
  mFileContextChipRow->setObjectName( u"aiAttachmentChipRow"_s );
  mFileContextChipLayout = new QHBoxLayout( mFileContextChipRow );
  mFileContextChipLayout->setContentsMargins( 0, 0, 0, 0 );
  mFileContextChipLayout->setSpacing( 4 );
  mFileContextChipLayout->addStretch( 1 );
  mFileContextChipRow->setVisible( false );
  layout->addWidget( mFileContextChipRow );

  mInputTextEdit = new QgsAiChatPromptEdit( container );
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

  // Per-session token/cost accounting (runtime only); hidden until usage arrives.
  mUsageLabel = new QLabel( container );
  mUsageLabel->setObjectName( u"aiSessionUsageLabel"_s );
  mUsageLabel->setVisible( false );
  statusRow->addWidget( mUsageLabel );

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

  mPlanClient = new QgsAiPlanClient( this );
  connect( mPlanClient, &QgsAiPlanClient::modelsReady, this, [this]( const QList<QgsAiPlanClient::ModelInfo> &, bool ) { rebuildModelMenu(); } );
  connect( mPlanClient, &QgsAiPlanClient::agentPolicyReady, this, [this]( const QgsAiManagedAgentPolicy &policy, bool ) {
    if ( mSessionManager )
      mSessionManager->setManagedAgentPolicy( policy );
    rebuildModelMenu();
  } );
  connect( mPlanClient, &QgsAiPlanClient::agentsReady, this, []( const QList<QgsAiManagedAgentPreset> &, bool ) {} );

  initModeMenu();
  initModelMenu();
  refreshPlanModels();
  refreshPlanAgentPolicy();
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
    connect( mSessionManager, &QgsAiAgentSessionManager::sessionUsageChanged, this, &QgsAiChatDockWidget::updateSessionUsage );
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
  connect( mInputTextEdit, &QgsAiChatPromptEdit::filesDropped, this, [this]( const QStringList &paths ) {
    bool added = false;
    for ( const QString &path : paths )
      added = addAttachedFile( path ) || added;
    if ( added )
      rebuildAttachmentChips();
  } );
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
  connect( mGisCardToggle, &QToolButton::toggled, this, [this]( bool expanded ) {
    if ( mGisCardBody )
      mGisCardBody->setVisible( expanded );
    if ( mGisCardToggle )
      mGisCardToggle->setArrowType( expanded ? Qt::DownArrow : Qt::RightArrow );
  } );

  // Suggestions are recomputed when the project changes shape; the debounce
  // matters because the collector samples features (up to 200 per vector layer)
  // and layer batches would otherwise stall the UI.
  mGisCardRefreshTimer = new QTimer( this );
  mGisCardRefreshTimer->setSingleShot( true );
  mGisCardRefreshTimer->setInterval( 1500 );
  connect( mGisCardRefreshTimer, &QTimer::timeout, this, &QgsAiChatDockWidget::refreshGisSuggestionCard );
  connect( QgsProject::instance(), &QgsProject::layersAdded, mGisCardRefreshTimer, qOverload<>( &QTimer::start ) );
  connect( QgsProject::instance(), &QgsProject::layersRemoved, mGisCardRefreshTimer, qOverload<>( &QTimer::start ) );
  connect( QgsProject::instance(), &QgsProject::readProject, mGisCardRefreshTimer, qOverload<>( &QTimer::start ) );
  connect( QgsProject::instance(), &QgsProject::cleared, mGisCardRefreshTimer, qOverload<>( &QTimer::start ) );

  setRequestRunning( false );
  refreshProposalList();
  refreshGisSuggestionCard();
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
  const QStringList labels = { u"Plan"_s, u"Agent"_s, u"Ask before edits"_s, u"Ask"_s };
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
  // The menu content depends on which providers are synced, which can change at
  // runtime (Provider Settings, OAuth login/logout). rebuildModelMenu() is the
  // repeatable builder and is also called after those events.
  rebuildModelMenu();
}

void QgsAiChatDockWidget::refreshPlanModels()
{
  if ( !mPlanClient || !mModelRouter )
    return;

  const QString endpoint = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint;
  if ( !QgsAiModelRouter::isUsablePlanEndpoint( endpoint ) )
    return;

  mPlanClient->refreshModels( endpoint );
}

void QgsAiChatDockWidget::refreshPlanAgentPolicy()
{
  if ( !mPlanClient || !mModelRouter )
    return;

  const QString endpoint = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint;
  if ( !QgsAiModelRouter::isUsablePlanEndpoint( endpoint ) )
    return;

  const QString sessionToken = mModelRouter->planSessionToken();
  if ( sessionToken.trimmed().isEmpty() )
  {
    const QgsAiManagedAgentPolicy cached = QgsAiPlanClient::cachedAgentPolicy();
    if ( mSessionManager && !cached.isEmpty() )
      mSessionManager->setManagedAgentPolicy( cached );
    return;
  }

  mPlanClient->refreshAgents( endpoint, sessionToken );
  mPlanClient->refreshAgentPolicy( endpoint, sessionToken );
}

QString QgsAiChatDockWidget::modelPillLabel( QgsAiModelRouter::Provider provider, const QString &displayName ) const
{
  const QString providerName = mModelRouter ? mModelRouter->providerDisplayName( provider ) : QString();
  if ( providerName.isEmpty() )
    return displayName + u" ▾"_s;
  return providerName + u" · "_s + displayName + u" ▾"_s;
}

void QgsAiChatDockWidget::rebuildModelMenu()
{
  if ( !mModelPill )
    return;

  // Replace any previous menu (and its action group) wholesale to avoid stale
  // actions; the old menu is owned by mModelPill and scheduled for deletion.
  if ( QMenu *oldMenu = mModelPill->menu() )
  {
    mModelPill->setMenu( nullptr );
    oldMenu->deleteLater();
  }

  QMenu *menu = new QMenu( mModelPill );

  // BYO providers stay selectable alongside the managed Strata catalog until paid
  // plans are live in production; revisit a Strata-only picker after launch.
  // Never mutate the active provider here: this rebuild runs on every modelsReady
  // refresh and would persistently override the user's explicit choice.
  QVector<ModelEntry> models;
  if ( mModelRouter )
  {
    for ( const ModelEntry &entry : predefinedModels() )
    {
      if ( entry.provider != QgsAiModelRouter::Provider::Plan && mModelRouter->isProviderAvailable( entry.provider ) )
        models.append( entry );
    }
    if ( mModelRouter->isProviderAvailable( QgsAiModelRouter::Provider::Plan ) )
    {
      QVector<ModelEntry> planEntries = cachedPlanModelEntries();
      if ( planEntries.isEmpty() )
        planEntries.append( ModelEntry { tr( "Strata Managed" ), u"managed-plan"_s, QgsAiModelRouter::Provider::Plan, tr( "Managed Strata Cloud default model" ) } );
      models += planEntries;
    }
  }

  // Nothing synced yet: guide the user to Provider Settings instead of offering
  // models that cannot be used.
  if ( models.isEmpty() )
  {
    QAction *none = menu->addAction( tr( "No AI providers configured" ) );
    none->setEnabled( false );
    menu->addSeparator();
    QAction *openSettings = menu->addAction( tr( "Open provider settings…" ) );
    connect( openSettings, &QAction::triggered, this, &QgsAiChatDockWidget::openProviderSettings );
    mModelPill->setMenu( menu );
    mModelPill->setText( tr( "No model ▾" ) );
    return;
  }

  const QgsAiModelRouter::Provider currentProvider = mModelRouter->resolveProvider();
  const QString currentModel = mModelRouter->providerSettings( currentProvider ).model;

  // Make sure the active model is always represented, even when it was chosen
  // from the full Settings catalog rather than the curated list.
  bool activeRepresented = false;
  for ( const ModelEntry &entry : models )
  {
    if ( entry.provider == currentProvider && entry.model == currentModel )
    {
      activeRepresented = true;
      break;
    }
  }
  if ( !activeRepresented && !currentModel.isEmpty() && mModelRouter->isProviderAvailable( currentProvider ) )
  {
    // Insert after the last row of the same provider so it lands in the right section.
    int insertAt = models.size();
    for ( int i = 0; i < models.size(); ++i )
    {
      if ( models.at( i ).provider == currentProvider )
        insertAt = i + 1;
    }
    models.insert( insertAt, ModelEntry { currentModel, currentModel, currentProvider } );
  }

  QActionGroup *group = new QActionGroup( menu );
  group->setExclusive( true );

  QgsAiModelRouter::Provider currentSection = QgsAiModelRouter::Provider::OpenAi;
  bool first = true;
  const ModelEntry *pillEntry = &models.first();
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
          header = tr( "Strata" );
          break;
      }
      menu->addSection( header );
      currentSection = entry.provider;
      first = false;
    }
    QAction *action = menu->addAction( entry.displayName );
    action->setCheckable( true );
    if ( !entry.tooltip.isEmpty() )
      action->setToolTip( entry.tooltip );
    action->setData( QVariant::fromValue( entry ) );
    group->addAction( action );
    if ( entry.provider == currentProvider && entry.model == currentModel )
    {
      action->setChecked( true );
      pillEntry = &entry;
    }
  }

  mModelPill->setMenu( menu );
  mModelPill->setText( modelPillLabel( pillEntry->provider, pillEntry->displayName ) );

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
  mMentionPopup->setStyleSheet( u"QFrame#aiMentionPopup { background: palette(base); border: 0; border-radius: 8px; }"_s );
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
    "QLabel#aiPlanStatusLabel, QLabel#aiQuestionsStatusLabel, QLabel#aiToolLimitStatusLabel { color: palette(highlight); font-weight: 600; }"
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
  const bool isPlanUi = uiKind == "plan"_L1 || uiKind == "agent_plan"_L1;
  if ( isPlanUi )
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
  else if ( uiKind == "tool_limit"_L1 )
  {
    QLabel *status = new QLabel( metadata.value( u"tool_limit_status"_s, u"pending"_s ).toString(), card );
    status->setObjectName( u"aiToolLimitStatusLabel"_s );
    headerLayout->addWidget( status );
  }
  cardLayout->addLayout( headerLayout );

  QString renderContent = content;
  if ( isPlanUi )
  {
    const QString planMarkdown = planMarkdownFromMetadata( metadata );
    if ( !planMarkdown.isEmpty() )
      renderContent = planMarkdown;
    else if ( uiKind == "agent_plan"_L1 )
      renderContent = removeAgentPlanProtocolBlocks( content );
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

  if ( isPlanUi )
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
  else if ( uiKind == "tool_limit"_L1 )
  {
    cardLayout->addWidget( createToolLimitActionsWidget( messageId, metadata ) );
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

  QHBoxLayout *workflowButtons = new QHBoxLayout();
  workflowButtons->setContentsMargins( 0, 0, 0, 0 );
  workflowButtons->setSpacing( 4 );
  QPushButton *saveWorkflowButton = new QPushButton( tr( "Save .strataflow" ), planCard );
  saveWorkflowButton->setObjectName( u"aiSaveWorkflowButton"_s );
  QPushButton *dryRunWorkflowButton = new QPushButton( tr( "Dry run" ), planCard );
  dryRunWorkflowButton->setObjectName( u"aiDryRunWorkflowButton"_s );
  QPushButton *runWorkflowButton = new QPushButton( tr( "Run workflow" ), planCard );
  runWorkflowButton->setObjectName( u"aiRunWorkflowButton"_s );
  QPushButton *exportWorkflowReportButton = new QPushButton( tr( "Export report" ), planCard );
  exportWorkflowReportButton->setObjectName( u"aiExportWorkflowReportButton"_s );
  const bool hasPlan = !planMarkdown.trimmed().isEmpty();
  for ( QPushButton *button : { saveWorkflowButton, dryRunWorkflowButton, runWorkflowButton, exportWorkflowReportButton } )
  {
    button->setEnabled( hasPlan );
    button->setStyleSheet(
      u"QPushButton { background: palette(button); color: palette(window-text); border: 0; border-radius: 6px; padding: 4px 8px; } QPushButton:hover:enabled { background: palette(alternate-base); }"_s
    );
    workflowButtons->addWidget( button );
  }
  workflowButtons->addStretch( 1 );
  layout->addLayout( workflowButtons );

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
  connect( saveWorkflowButton, &QPushButton::clicked, this, [this, messageId, planMarkdown]() {
    QString error;
    const QString path = saveWorkflowPlan( planMarkdown, messageId, &error );
    updateRuntimeState( u"workflow"_s, path.isEmpty() ? error : tr( "Saved %1" ).arg( path ) );
  } );
  connect( dryRunWorkflowButton, &QPushButton::clicked, this, [this, messageId, planMarkdown]() { dryRunWorkflowPlan( messageId, planMarkdown ); } );
  connect( runWorkflowButton, &QPushButton::clicked, this, [this, messageId, planMarkdown, metadata]() { runWorkflowPlan( messageId, planMarkdown, metadata ); } );
  connect( exportWorkflowReportButton, &QPushButton::clicked, this, [this, messageId, planMarkdown]() {
    QString error;
    const QString path = exportWorkflowReport( planMarkdown, messageId, &error );
    updateRuntimeState( u"workflow"_s, path.isEmpty() ? error : tr( "Exported %1" ).arg( path ) );
  } );
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

QWidget *QgsAiChatDockWidget::createToolLimitActionsWidget( const QString &messageId, const QVariantMap &metadata )
{
  QFrame *limitCard = new QFrame( mTranscriptContainer );
  limitCard->setObjectName( u"aiToolLimitCard"_s );
  limitCard->setFrameShape( QFrame::NoFrame );
  applyTranscriptWidthPolicy( limitCard );
  limitCard->setStyleSheet( u"QFrame#aiToolLimitCard { background: palette(base); border: 0; border-radius: 0; }"_s );

  QHBoxLayout *layout = new QHBoxLayout( limitCard );
  layout->setContentsMargins( 8, 4, 8, 4 );
  layout->setSpacing( 6 );

  QPushButton *continueButton = new QPushButton( tr( "Continue" ), limitCard );
  continueButton->setObjectName( u"aiContinueToolLimitButton"_s );
  const QString status = metadata.value( u"tool_limit_status"_s, u"pending"_s ).toString();
  continueButton->setProperty( "tool_limit_status", status );
  const bool pending = status == "pending"_L1;
  continueButton->setEnabled( pending && !mRequestRunning && mSessionManager );
  continueButton->setStyleSheet(
    u"QPushButton#aiContinueToolLimitButton { background: palette(highlight); color: palette(highlighted-text); border: 0; border-radius: 6px; padding: 4px 10px; font-weight: 600; } QPushButton#aiContinueToolLimitButton:disabled { background: palette(button); color: palette(mid); }"_s
  );
  layout->addWidget( continueButton );
  layout->addStretch( 1 );

  connect( continueButton, &QPushButton::clicked, this, [this, messageId, continueButton]() {
    if ( !mSessionManager || mRequestRunning )
      return;
    continueButton->setEnabled( false );
    if ( mSessionManager->continueAfterToolLimit( messageId ) )
    {
      continueButton->setProperty( "tool_limit_status", u"continued"_s );
      continueButton->setEnabled( false );
    }
    else
      continueButton->setEnabled( continueButton->property( "tool_limit_status" ).toString() == "pending"_L1 && !mRequestRunning );
  } );

  return limitCard;
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

QStringList QgsAiChatDockWidget::disallowedWorkflowTools( const QString &planMarkdown, const QVariantMap &metadata ) const
{
  if ( !mSessionManager )
    return QStringList();

  const QJsonObject plan = planJsonForExecution( planMarkdown, metadata );
  if ( plan.isEmpty() )
    return QStringList();

  const QStringList requestedTools = planToolNames( plan );
  if ( requestedTools.isEmpty() )
    return QStringList();

  const QStringList allowedTools = mSessionManager->allowedToolNamesForActiveAgent();
  QStringList disallowed;
  for ( const QString &toolName : requestedTools )
  {
    if ( !allowedTools.contains( toolName ) )
      disallowed << toolName;
  }
  return disallowed;
}

bool QgsAiChatDockWidget::requestWorkflowRevisionForDisallowedTools( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata )
{
  const QStringList disallowedTools = disallowedWorkflowTools( planMarkdown, metadata );
  if ( disallowedTools.isEmpty() )
    return false;

  markMessageStatus( messageId, metadata, u"plan_status"_s, u"revision_requested"_s );
  setModeLabel( u"Plan"_s );
  mSessionManager->sendUserMessage(
    tr( "Revise this workflow plan before execution. The current Agent allowlist does not permit these tools: %1.\n\nOriginal plan:\n%2\n\nReturn a new fenced strata_agent_plan JSON block using only available Agent tools, or explain which setting or managed policy must change." )
      .arg( disallowedTools.join( ", "_L1 ), planMarkdown.trimmed() )
  );
  reloadTranscriptFromHistory();
  return true;
}

void QgsAiChatDockWidget::acceptPlan( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata )
{
  if ( !mSessionManager || planMarkdown.trimmed().isEmpty() || mRequestRunning )
    return;

  setModeLabel( u"Agent"_s );
  if ( requestWorkflowRevisionForDisallowedTools( messageId, planMarkdown, metadata ) )
    return;

  markMessageStatus( messageId, metadata, u"plan_status"_s, u"accepted"_s );
  QString workflowError;
  const QString workflowPath = saveWorkflowPlan( planMarkdown, messageId, &workflowError );
  QString prompt = tr( "Execute the accepted plan exactly in Agent mode. Use the tool safety checks and any per-tool approval/review dialogs that apply." );
  if ( !workflowPath.isEmpty() )
    prompt += u"\n\n"_s + tr( "Saved reusable workflow: %1" ).arg( workflowPath );
  else if ( !workflowError.isEmpty() )
    prompt += u"\n\n"_s + tr( "Workflow save failed: %1" ).arg( workflowError );
  prompt += u"\n\n"_s + tr( "Accepted plan:\n%1" ).arg( planMarkdown.trimmed() );
  mSessionManager->sendUserMessage( prompt );
  reloadTranscriptFromHistory();
}

QString QgsAiChatDockWidget::saveWorkflowPlan( const QString &planMarkdown, const QString &messageId, QString *errorMessage ) const
{
  const QString trimmedPlan = planMarkdown.trimmed();
  if ( trimmedPlan.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = tr( "Workflow plan is empty." );
    return QString();
  }

  const QString workspaceRoot = mSessionManager ? mSessionManager->workspaceRoot() : QString();
  const QString baseDir = workspaceRoot.trimmed().isEmpty() ? QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"strata_workflows"_s ) : QDir( workspaceRoot ).filePath( u".strata/workflows"_s );
  QDir dir( baseDir );
  if ( !dir.exists() && !dir.mkpath( u"."_s ) )
  {
    if ( errorMessage )
      *errorMessage = tr( "Cannot create workflow directory: %1" ).arg( baseDir );
    return QString();
  }

  QJsonObject manifest;
  manifest.insert( u"kind"_s, u"strataflow"_s );
  manifest.insert( u"version"_s, 1 );
  manifest.insert( u"createdAt"_s, QDateTime::currentDateTimeUtc().toString( Qt::ISODateWithMs ) );
  manifest.insert( u"sourceMessageId"_s, messageId );
  manifest.insert( u"mode"_s, u"auto_edit"_s );
  manifest.insert( u"planMarkdown"_s, trimmedPlan );
  manifest.insert( u"steps"_s, workflowStepsFromMarkdown( trimmedPlan ) );
  QJsonObject runner;
  runner.insert( u"type"_s, u"strata-agent"_s );
  runner.insert( u"dryRunSupported"_s, true );
  runner.insert( u"requiresApproval"_s, true );
  manifest.insert( u"runner"_s, runner );
  manifest.insert( u"provenance"_s, workflowProvenance( workspaceRoot ) );

  const QString path = dir.filePath( workflowSlug( trimmedPlan ) + u".strataflow"_s );
  QSaveFile file( path );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    if ( errorMessage )
      *errorMessage = tr( "Cannot write workflow file: %1" ).arg( path );
    return QString();
  }
  file.write( QJsonDocument( manifest ).toJson( QJsonDocument::Indented ) );
  if ( !file.commit() )
  {
    if ( errorMessage )
      *errorMessage = tr( "Cannot commit workflow file: %1" ).arg( path );
    return QString();
  }
  return path;
}

QString QgsAiChatDockWidget::exportWorkflowReport( const QString &planMarkdown, const QString &messageId, QString *errorMessage ) const
{
  QString workflowError;
  const QString workflowPath = saveWorkflowPlan( planMarkdown, messageId, &workflowError );
  if ( workflowPath.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = workflowError;
    return QString();
  }

  QJsonObject report;
  report.insert( u"kind"_s, u"strataflow_report"_s );
  report.insert( u"version"_s, 1 );
  report.insert( u"workflowPath"_s, workflowPath );
  report.insert( u"exportedAt"_s, QDateTime::currentDateTimeUtc().toString( Qt::ISODateWithMs ) );
  report.insert( u"status"_s, u"ready_for_dry_run"_s );
  report.insert( u"planMarkdown"_s, planMarkdown.trimmed() );
  report.insert( u"steps"_s, workflowStepsFromMarkdown( planMarkdown ) );
  report.insert( u"provenance"_s, workflowProvenance( mSessionManager ? mSessionManager->workspaceRoot() : QString() ) );

  const QString reportPath = workflowPath + u".report.json"_s;
  QSaveFile file( reportPath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    if ( errorMessage )
      *errorMessage = tr( "Cannot write workflow report: %1" ).arg( reportPath );
    return QString();
  }
  file.write( QJsonDocument( report ).toJson( QJsonDocument::Indented ) );
  if ( !file.commit() )
  {
    if ( errorMessage )
      *errorMessage = tr( "Cannot commit workflow report: %1" ).arg( reportPath );
    return QString();
  }
  return reportPath;
}

void QgsAiChatDockWidget::dryRunWorkflowPlan( const QString &messageId, const QString &planMarkdown )
{
  if ( !mSessionManager || planMarkdown.trimmed().isEmpty() || mRequestRunning )
    return;

  QString error;
  const QString workflowPath = saveWorkflowPlan( planMarkdown, messageId, &error );
  setModeLabel( u"Plan"_s );
  mSessionManager->sendUserMessage(
    tr( "Dry-run this .strataflow workflow. Do not call mutating tools. Validate required inputs, risks, approvals, expected GIS side effects and rollback points.\n\nWorkflow path: %1\n\nPlan:\n%2" )
      .arg( workflowPath.isEmpty() ? error : workflowPath, planMarkdown.trimmed() )
  );
  reloadTranscriptFromHistory();
}

void QgsAiChatDockWidget::runWorkflowPlan( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata )
{
  if ( !mSessionManager || planMarkdown.trimmed().isEmpty() || mRequestRunning )
    return;

  setModeLabel( u"Agent"_s );
  if ( requestWorkflowRevisionForDisallowedTools( messageId, planMarkdown, metadata ) )
    return;

  QString error;
  const QString workflowPath = saveWorkflowPlan( planMarkdown, messageId, &error );
  mSessionManager->sendUserMessage(
    tr( "Run this .strataflow workflow through the Strata agent. Use native GIS tools where available, keep provenance for each step, and respect all per-tool safety checks and approval dialogs.\n\nWorkflow path: %1\n\nPlan:\n%2" )
      .arg( workflowPath.isEmpty() ? error : workflowPath, planMarkdown.trimmed() )
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
  mSessionManager->sendUserMessage(
    tr( "Revise the previous plan before execution. Keep Plan mode and return a new fenced strata_agent_plan JSON block.\n\nOriginal plan:\n%1\n\nUser revision request:\n%2" )
      .arg( planMarkdown.trimmed(), feedback.isEmpty() ? tr( "The plan was rejected; propose a safer or clearer revision." ) : feedback )
  );
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

void QgsAiChatDockWidget::updateSessionUsage( const QgsAiUsage &total )
{
  if ( !mUsageLabel )
    return;

  if ( !total.isValid() )
  {
    // New/empty session: hide until the first response brings usage back.
    mUsageLabel->setVisible( false );
    mUsageLabel->clear();
    return;
  }

  const auto humanTokens = []( qint64 tokens ) -> QString {
    if ( tokens >= 1000000 )
      return u"%1M"_s.arg( tokens / 1000000.0, 0, 'f', 1 );
    if ( tokens >= 1000 )
      return u"%1k"_s.arg( tokens / 1000.0, 0, 'f', 1 );
    return QString::number( tokens );
  };

  QString text = tr( "≈%1 tok" ).arg( humanTokens( total.totalTokens ) );
  if ( total.costUsd > 0.0 )
    text += u" · $%1"_s.arg( total.costUsd, 0, 'f', 4 );
  mUsageLabel->setText( text );
  mUsageLabel->setToolTip( tr( "Session usage - prompt: %1, completion: %2, cached: %3, reasoning: %4%5" )
                             .arg(
                               humanTokens( total.promptTokens ),
                               humanTokens( total.completionTokens ),
                               humanTokens( total.cachedTokens ),
                               humanTokens( total.reasoningTokens ),
                               total.costUsd > 0.0 ? tr( ", cost: $%1" ).arg( total.costUsd, 0, 'f', 6 ) : QString()
                             ) );
  mUsageLabel->setVisible( true );
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
  const QList<QPushButton *> continueButtons = findChildren<QPushButton *>( u"aiContinueToolLimitButton"_s );
  for ( QPushButton *button : continueButtons )
    button->setEnabled( button->property( "tool_limit_status" ).toString() == "pending"_L1 && !running && mSessionManager );
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

  // Record the choice on its own provider and mark that provider active, WITHOUT
  // disabling the others — every synced provider stays synced so switching back
  // and forth (e.g. Codex <-> OpenRouter) is instant and reversible.
  QgsAiModelRouter::ProviderSettings settings = mModelRouter->providerSettings( entry.provider );
  settings.model = entry.model;
  settings.enabled = true;
  mModelRouter->setProviderSettings( entry.provider, settings );
  mModelRouter->setActiveProvider( entry.provider );

  mModelPill->setText( modelPillLabel( entry.provider, entry.displayName ) );
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

  QString outgoing = input.isEmpty() ? tr( "Analyze the attached files." ) : input;
  static const QRegularExpression gisMentionRe( u"(^|\\s)@gis(\\s|$)"_s, QRegularExpression::CaseInsensitiveOption );
  if ( gisMentionRe.match( input ).hasMatch() )
  {
    // Explicit user request: attach the full block regardless of toggles or dismissals.
    const QString gisBlock = QgsAiGisSuggestionEngine::formatHealthBlock( QgsAiGisSuggestionEngine::suggestionsForProject( QgsProject::instance() ), true );
    outgoing += u"\n\n"_s + ( gisBlock.isEmpty() ? tr( "(No GIS suggestions for the current project right now.)" ) : gisBlock );
  }
  mSessionManager->sendUserMessage( outgoing, contextFiles );
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

  if ( QgsAiVisualContextUtils::isSupportedImagePath( path ) )
  {
    if ( !QgsAiVisualContextUtils::ensureVisualContextConsent( this ) )
      return false;
  }

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

    const bool isImage = QgsAiVisualContextUtils::isSupportedImagePath( file.filePath );
    QLabel *label = new QLabel( u"%1 %2"_s.arg( isImage ? u"🖼"_s : u"📎"_s, QFileInfo( file.filePath ).fileName() ), chip );
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

  const bool gisMentionMatches = u"gis"_s.startsWith( query.toLower() );
  const QStringList candidates = mSessionManager->projectFileCandidates( query, 25 );
  if ( candidates.isEmpty() && !gisMentionMatches )
  {
    hideMentionPopup();
    return;
  }

  mMentionStartPosition = atPosition;
  mMentionList->clear();
  if ( gisMentionMatches )
  {
    QListWidgetItem *gisItem = new QListWidgetItem( tr( "gis — Project GIS health" ), mMentionList );
    gisItem->setData( Qt::UserRole, u"@gis"_s );
    gisItem->setToolTip( tr( "Attach the current GIS suggestions to the message" ) );
  }
  for ( const QString &candidate : candidates )
  {
    QListWidgetItem *item = new QListWidgetItem( candidate, mMentionList );
    item->setData( Qt::UserRole, candidate );
    item->setToolTip( candidate );
  }
  mMentionList->setCurrentRow( 0 );

  const QRect cursorRect = mInputTextEdit->cursorRect( cursor );
  const QPoint popupPosition = mInputTextEdit->viewport()->mapToGlobal( cursorRect.bottomLeft() );
  const int popupHeight = std::min( 260, std::max( 48, mMentionList->sizeHintForRow( 0 ) * mMentionList->count() + 8 ) );
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

  const bool isGisMention = relativePath == "@gis"_L1;
  const bool needsQuotes = !isGisMention && relativePath.contains( QRegularExpression( u"\\s"_s ) );
  const QString mention = isGisMention ? u"@gis"_s : ( needsQuotes ? u"@\"%1\""_s.arg( relativePath ) : u"@%1"_s.arg( relativePath ) );
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
    // @gis is the suggestions mention, not a file reference.
    if ( match.captured( 1 ).isEmpty() && referencedPath.compare( "gis"_L1, Qt::CaseInsensitive ) == 0 )
      continue;
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

void QgsAiChatDockWidget::sendGisSuggestionToChat( const QgsAiGisSuggestion &suggestion )
{
  if ( !mSessionManager || suggestion.actionPrompt.trimmed().isEmpty() )
    return;

  setModeLabel( u"Ask before edits"_s );
  mSessionManager->sendUserMessage( tr(
                                      "GIS suggestion selected:\n\n"
                                      "Suggestion: %1\n"
                                      "ID: %2\n"
                                      "Risk: %3\n\n"
                                      "%4\n\n"
                                      "Use ask-before-edits mode. Inspect first, then request approval before any mutating GIS action."
  )
                                      .arg( suggestion.title, suggestion.id, suggestion.risk.isEmpty() ? u"low"_s : suggestion.risk, suggestion.actionPrompt.trimmed() ) );

  reloadTranscriptFromHistory();
}

void QgsAiChatDockWidget::dismissGisSuggestion( const QString &suggestionId )
{
  if ( suggestionId.isEmpty() )
    return;

  const QgsProject *project = QgsProject::instance();
  const QString projectFile = project ? project->fileName() : QString();
  const QString key = QgsAiGisSuggestionEngine::dismissedSettingsKey( projectFile );
  QgsSettings settings;
  QStringList dismissed = settings.value( key ).toStringList();
  if ( !dismissed.contains( suggestionId ) )
  {
    dismissed << suggestionId;
    settings.setValue( key, dismissed );
  }
  refreshGisSuggestionCard();
}

void QgsAiChatDockWidget::refreshGisSuggestionCard()
{
  if ( !mGisCardContainer || !mGisCardBodyLayout )
    return;

  while ( QLayoutItem *item = mGisCardBodyLayout->takeAt( 0 ) )
  {
    if ( QWidget *widget = item->widget() )
      widget->deleteLater();
    delete item;
  }

  QgsProject *project = QgsProject::instance();
  if ( !QgsAiGisSuggestionEngine::suggestionsEnabledForProject( project ) )
  {
    mGisCardContainer->setVisible( false );
    return;
  }

  const QString projectFile = project ? project->fileName() : QString();
  const QStringList dismissed = QgsSettings().value( QgsAiGisSuggestionEngine::dismissedSettingsKey( projectFile ) ).toStringList();

  // Low-risk findings stay model-side context only; the card surfaces what deserves attention.
  QList<QgsAiGisSuggestion> visible;
  const QList<QgsAiGisSuggestion> suggestions = QgsAiGisSuggestionEngine::suggestionsForProject( project );
  for ( const QgsAiGisSuggestion &suggestion : suggestions )
  {
    if ( suggestion.risk == "low"_L1 || dismissed.contains( suggestion.id ) )
      continue;
    visible << suggestion;
  }

  if ( visible.isEmpty() )
  {
    mGisCardContainer->setVisible( false );
    return;
  }

  mGisCardToggle->setText( tr( "Project suggestions (%1)" ).arg( visible.size() ) );
  for ( const QgsAiGisSuggestion &suggestion : std::as_const( visible ) )
  {
    QWidget *row = new QWidget( mGisCardBody );
    row->setObjectName( u"aiGisCardRow"_s );
    QHBoxLayout *rowLayout = new QHBoxLayout( row );
    rowLayout->setContentsMargins( 2, 0, 0, 0 );
    rowLayout->setSpacing( 4 );

    QLabel *riskBadge = new QLabel( u"[%1]"_s.arg( suggestion.risk ), row );
    riskBadge->setObjectName( u"aiGisCardRiskBadge"_s );
    rowLayout->addWidget( riskBadge );

    QLabel *titleLabel = new QLabel( suggestion.title, row );
    titleLabel->setToolTip( suggestion.detail );
    rowLayout->addWidget( titleLabel, 1 );

    QPushButton *analyzeButton = new QPushButton( tr( "Analyze" ), row );
    analyzeButton->setObjectName( u"aiGisCardReviewButton"_s );
    connect( analyzeButton, &QPushButton::clicked, this, [this, suggestion]() { sendGisSuggestionToChat( suggestion ); } );
    rowLayout->addWidget( analyzeButton );

    QToolButton *dismissButton = new QToolButton( row );
    dismissButton->setObjectName( u"aiGisCardDismissButton"_s );
    dismissButton->setText( u"×"_s );
    dismissButton->setAutoRaise( true );
    dismissButton->setToolTip( tr( "Dismiss this suggestion for this project" ) );
    connect( dismissButton, &QToolButton::clicked, this, [this, suggestion]() { dismissGisSuggestion( suggestion.id ); } );
    rowLayout->addWidget( dismissButton );

    mGisCardBodyLayout->addWidget( row );
  }

  mGisCardBody->setVisible( mGisCardToggle->isChecked() );
  mGisCardContainer->setVisible( true );
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

  QgsAiSettingsDialog dialog( mSessionManager, mModelRouter, mLayerIndexCoordinator, this );
  connect( &dialog, &QgsAiSettingsDialog::embeddingProviderSettingsChanged, this, &QgsAiChatDockWidget::embeddingProviderSettingsChanged );
  connect( &dialog, &QgsAiSettingsDialog::planAuthStateChanged, this, &QgsAiChatDockWidget::rebuildModelMenu );
  connect( &dialog, &QgsAiSettingsDialog::demoProjectCreated, this, &QgsAiChatDockWidget::refreshGisSuggestionCard );

  if ( dialog.exec() != QDialog::Accepted )
    return;

  refreshGisSuggestionCard();

  // Credentials/sync state may have changed (API keys, Codex/Claude OAuth
  // login/logout); refresh the picker so it lists exactly the synced providers.
  refreshPlanModels();
  refreshPlanAgentPolicy();
  rebuildModelMenu();
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
       || mModelRouter->isProviderAvailable( QgsAiModelRouter::Provider::Plan ) )
  {
    settings.setValue( u"strata/welcome_seen"_s, true );
    settings.remove( u"geoai/welcome_seen"_s );
    settings.remove( u"qgis_ai/welcome_seen"_s );
    return;
  }

  QgsMessageBar *messageBar = QgisApp::instance() ? QgisApp::instance()->messageBar() : nullptr;
  if ( !messageBar )
    return;

  QPushButton *settingsButton = new QPushButton( tr( "Open AI onboarding" ) );
  QgsMessageBarItem *item
    = new QgsMessageBarItem( tr( "AI Assistant" ), tr( "Complete Plan login or BYOK, privacy, model, indexing and demo setup before using the cloud agent." ), settingsButton, Qgis::MessageLevel::Info, 0, messageBar );

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
