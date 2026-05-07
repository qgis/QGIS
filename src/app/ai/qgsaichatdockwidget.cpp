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

#include "qgisapp.h"
#include "qgsaiagentsessionmanager.h"
#include "qgsaiclaudeoauthclient.h"
#include "qgsaicodexoauthclient.h"
#include "qgsaimodelrouter.h"
#include "qgsaireviewpatchengine.h"
#include "qgsapplication.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgssettings.h"

#include <QAction>
#include <QActionGroup>
#include <QCheckBox>
#include <QColor>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QToolButton>
#include <QUrlQuery>
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
      { u"Codex GPT-5.5"_s, u"gpt-5.5"_s, QgsAiModelRouter::Provider::Codex },
      { u"Claude Sonnet 4"_s, u"claude-sonnet-4-20250514"_s, QgsAiModelRouter::Provider::Claude },
      { u"Claude Sonnet 3.7"_s, u"claude-3-7-sonnet-20250219"_s, QgsAiModelRouter::Provider::Claude },
      { u"Claude Opus 4.1"_s, u"claude-opus-4-1-20250805"_s, QgsAiModelRouter::Provider::Claude },
      { u"Plan backend"_s, u"managed-plan"_s, QgsAiModelRouter::Provider::Plan },
    };
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

  mTranscript = new QTextEdit( container );
  mTranscript->setReadOnly( true );
  mTranscript->setFrameShape( QFrame::NoFrame );
  const QColor codeBg = mTranscript->palette().alternateBase().color();
  mTranscript->document()->setDefaultStyleSheet( QStringLiteral(
    "pre { background-color: %1; padding: 6px; font-family: monospace; white-space: pre-wrap; }"
    "code { background-color: %1; font-family: monospace; padding: 1px 3px; }"
    "h1, h2, h3 { margin-top: 8px; margin-bottom: 4px; }"
  ).arg( codeBg.name() ) );
  layout->addWidget( mTranscript, 1 );

  mFileContextChipRow = new QWidget( container );
  mFileContextChipRow->setObjectName( u"aiAttachmentChipRow"_s );
  mFileContextChipLayout = new QHBoxLayout( mFileContextChipRow );
  mFileContextChipLayout->setContentsMargins( 0, 0, 0, 0 );
  mFileContextChipLayout->setSpacing( 4 );
  mFileContextChipLayout->addStretch( 1 );
  mFileContextChipRow->setVisible( false );
  layout->addWidget( mFileContextChipRow );

  mInputTextEdit = new QTextEdit( container );
  mInputTextEdit->setPlaceholderText( tr( "Ask a question, tag project files with @, or send /patch…  (Shift+Enter for newline)" ) );
  mInputTextEdit->setAcceptRichText( false );
  mInputTextEdit->setTabChangesFocus( true );
  mInputTextEdit->setFixedHeight( 72 );
  mInputTextEdit->installEventFilter( this );
  layout->addWidget( mInputTextEdit );

  mMentionPopup = new QFrame( this, Qt::Popup );
  mMentionPopup->setObjectName( u"aiMentionPopup"_s );
  QVBoxLayout *mentionLayout = new QVBoxLayout( mMentionPopup );
  mentionLayout->setContentsMargins( 0, 0, 0, 0 );
  mMentionList = new QListWidget( mMentionPopup );
  mMentionList->setObjectName( u"aiMentionList"_s );
  mMentionList->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mMentionList->setUniformItemSizes( true );
  mentionLayout->addWidget( mMentionList );
  mMentionPopup->hide();

  QHBoxLayout *bottomBar = new QHBoxLayout();
  bottomBar->setContentsMargins( 0, 0, 0, 0 );
  bottomBar->setSpacing( 6 );

  mModePill = new QToolButton( container );
  mModePill->setPopupMode( QToolButton::InstantPopup );
  mModePill->setToolButtonStyle( Qt::ToolButtonTextOnly );
  mModePill->setAutoRaise( true );
  bottomBar->addWidget( mModePill );

  mModelPill = new QToolButton( container );
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
  mSettingsButton->setIcon( QgsApplication::getThemeIcon( u"mActionOptions.svg"_s ) );
  mSettingsButton->setAutoRaise( true );
  mSettingsButton->setFixedSize( 28, 28 );
  mSettingsButton->setToolTip( tr( "Provider settings" ) );
  bottomBar->addWidget( mSettingsButton );

  mSendButton = new QToolButton( container );
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
        finalizeStreamingAssistantMessage( message.content );
        return;
      }
      closeStreamingAssistantMessage();
      appendTranscriptMessage( qgsAiChatRoleToString( message.role ), message.content );
    } );
    connect( mSessionManager, &QgsAiAgentSessionManager::proposalCreated, this, [this]( const QString & ) { refreshProposalList(); } );
    connect( mSessionManager, &QgsAiAgentSessionManager::responseChunkReceived, this, &QgsAiChatDockWidget::appendStreamChunk );
    connect( mSessionManager, &QgsAiAgentSessionManager::requestStateChanged, this, &QgsAiChatDockWidget::updateRuntimeState );
    connect( mSessionManager, &QgsAiAgentSessionManager::requestRunningChanged, this, &QgsAiChatDockWidget::setRequestRunning );
  }

  if ( mReviewEngine )
  {
    connect( mReviewEngine, &QgsAiReviewPatchEngine::proposalAdded, this, [this]( const QString & ) { refreshProposalList(); } );
    connect( mReviewEngine, &QgsAiReviewPatchEngine::proposalAccepted, this, [this]( const QString & ) { refreshProposalList(); } );
    connect( mReviewEngine, &QgsAiReviewPatchEngine::proposalRejected, this, [this]( const QString & ) { refreshProposalList(); } );
  }

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
  const QPalette pal = QgsApplication::palette();
  const QString border = pal.mid().color().name();
  const QString hover = pal.highlight().color().name();
  const QString pillStyle = QStringLiteral(
                              "QToolButton { border: 1px solid %1; border-radius: 10px; padding: 2px 10px; } "
                              "QToolButton::menu-indicator { image: none; width: 0; } "
                              "QToolButton:hover { background: %2; color: palette(highlighted-text); }"
  )
                              .arg( border, hover );
  mModePill->setStyleSheet( pillStyle );
  mModelPill->setStyleSheet( pillStyle );

  mSendButton->setStyleSheet( QStringLiteral(
    "QToolButton { border-radius: 14px; background: palette(highlight); color: palette(highlighted-text); font-weight: bold; } "
    "QToolButton:disabled { background: palette(mid); color: palette(button-text); }"
  ) );
}

QString QgsAiChatDockWidget::renderMarkdown( const QString &md )
{
  QTextDocument doc;
  doc.setMarkdown( md, QTextDocument::MarkdownDialectGitHub );
  return doc.toHtml();
}

void QgsAiChatDockWidget::appendTranscriptMessage( const QString &role, const QString &content )
{
  const QString html = QStringLiteral( "<p><b>[%1]</b></p>" ).arg( role.toHtmlEscaped() )
                       + renderMarkdown( content );
  mTranscript->append( html );
}

void QgsAiChatDockWidget::appendStreamChunk( const QString &chunk )
{
  if ( chunk.isEmpty() )
    return;

  if ( !mStreamingInProgress )
  {
    mTranscript->append( QStringLiteral( "<p><b>[assistant]</b></p>" ) );
    QTextCursor cursor = mTranscript->textCursor();
    cursor.movePosition( QTextCursor::End );
    mStreamingContentStartPosition = cursor.position();
    mStreamingInProgress = true;
  }
  QTextCursor cursor = mTranscript->textCursor();
  cursor.movePosition( QTextCursor::End );
  cursor.insertText( chunk );
  mTranscript->setTextCursor( cursor );
}

void QgsAiChatDockWidget::closeStreamingAssistantMessage()
{
  mStreamingInProgress = false;
  mStreamingContentStartPosition = -1;
}

void QgsAiChatDockWidget::finalizeStreamingAssistantMessage( const QString &finalText )
{
  if ( !mStreamingInProgress )
    return;

  if ( mStreamingContentStartPosition >= 0 && !finalText.isEmpty() )
  {
    QTextCursor cursor( mTranscript->document() );
    cursor.setPosition( mStreamingContentStartPosition );
    cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
    cursor.insertHtml( renderMarkdown( finalText ) );
    mTranscript->setTextCursor( cursor );
  }

  closeStreamingAssistantMessage();
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
  const QString label = action->text();
  mModePill->setText( label + u" ▾"_s );
  if ( mSessionManager )
    mSessionManager->setActiveAgent( modeLabelToAgent( label ) );
}

void QgsAiChatDockWidget::onModelSelected( QAction *action )
{
  if ( !action || !mModelRouter )
    return;
  const ModelEntry entry = action->data().value<ModelEntry>();
  if ( entry.displayName.isEmpty() )
    return;

  const QList<QgsAiModelRouter::Provider> providers = { QgsAiModelRouter::Provider::OpenAi, QgsAiModelRouter::Provider::Codex, QgsAiModelRouter::Provider::Claude, QgsAiModelRouter::Provider::Plan };
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

void QgsAiChatDockWidget::sendMessage()
{
  if ( !mSessionManager )
    return;
  const QString input = mInputTextEdit->toPlainText().trimmed();
  const QList<QgsAiChatContextFile> contextFiles = contextFilesForCurrentMessage( input );
  if ( input.isEmpty() && contextFiles.isEmpty() )
    return;

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
    chip->setStyleSheet( u"QWidget#aiAttachmentChip { border: 1px solid palette(mid); border-radius: 10px; padding: 1px 4px; }"_s );
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
  QVBoxLayout *layout = new QVBoxLayout( &dialog );
  QFormLayout *form = new QFormLayout();

  QLineEdit *openAiEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi ).endpoint, &dialog );
  QLineEdit *openAiModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi ).model, &dialog );
  QLineEdit *openAiKey = new QLineEdit( &dialog );
  openAiKey->setEchoMode( QLineEdit::Password );
  openAiKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi ) ? tr( "Saved locally — enter a new key only to replace it" ) : tr( "sk-..." ) );

  QgsAiCodexOAuthClient::DeviceCode codexDeviceCode;
  QLineEdit *codexEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex ).endpoint, &dialog );
  QLineEdit *codexModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex ).model, &dialog );
  codexModel->setPlaceholderText( u"gpt-5.5"_s );
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

  form->addRow( tr( "OpenAI endpoint" ), openAiEndpoint );
  form->addRow( tr( "OpenAI model" ), openAiModel );
  form->addRow( tr( "OpenAI API key" ), openAiKey );
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
  layout->addLayout( form );

  QLabel *helpLabel
    = new QLabel( tr( "OpenAI and Claude API keys are stored locally in QGIS settings. Codex and Claude OAuth refresh tokens are stored in the encrypted QGIS authentication store. Leave API key fields empty to keep the current saved value." ), &dialog );
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
    QString callbackCode;
    QTcpServer callbackServer;
    const bool hasLocalCallback = callbackServer.listen( QHostAddress::LocalHost, 0 );
    const QString redirectUri = hasLocalCallback ? u"http://127.0.0.1:%1/callback"_s.arg( callbackServer.serverPort() ) : QString();
    const QgsAiClaudeOAuthClient::AuthorizationRequest authRequest = hasLocalCallback ? QgsAiClaudeOAuthClient::buildAuthorizationRequest( redirectUri )
                                                                                      : QgsAiClaudeOAuthClient::buildAuthorizationRequest();

    if ( hasLocalCallback )
    {
      QEventLoop callbackLoop;
      QTimer timeout;
      timeout.setSingleShot( true );
      QObject::connect( &timeout, &QTimer::timeout, &callbackLoop, &QEventLoop::quit );
      QObject::connect( &callbackServer, &QTcpServer::newConnection, &dialog, [&callbackServer, &callbackLoop, &callbackCode]() {
        QTcpSocket *socket = callbackServer.nextPendingConnection();
        if ( !socket )
          return;
        if ( !socket->waitForReadyRead( 3000 ) )
        {
          socket->deleteLater();
          return;
        }

        const QByteArray requestLine = socket->readLine();
        const QList<QByteArray> parts = requestLine.split( ' ' );
        if ( parts.size() >= 2 )
        {
          const QUrl callbackUrl( QString::fromLatin1( parts.at( 1 ) ) );
          callbackCode = QUrlQuery( callbackUrl ).queryItemValue( u"code"_s ).trimmed();
        }

        const QByteArray body = "<html><body><h3>Claude login completed.</h3>You can return to QGIS_AI.</body></html>";
        socket->write( "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: " + QByteArray::number( body.size() ) + "\r\n\r\n" + body );
        socket->disconnectFromHost();
        socket->deleteLater();
        callbackLoop.quit();
      } );
      timeout.start( 120000 );
      QDesktopServices::openUrl( authRequest.authorizationUrl );
      callbackLoop.exec();
      callbackServer.close();
    }
    else
    {
      QDesktopServices::openUrl( authRequest.authorizationUrl );
    }

    QString code = callbackCode;
    if ( code.isEmpty() )
    {
      bool ok = false;
      code = QInputDialog::getText( &dialog, QObject::tr( "Claude OAuth" ), QObject::tr( "After approving Claude, paste the authorization code or callback URL:" ), QLineEdit::Normal, QString(), &ok ).trimmed();
      if ( !ok || code.isEmpty() )
        return;
    }

    QString error;
    if ( !QgsAiClaudeOAuthClient::exchangeAuthorizationCode( code, authRequest.codeVerifier, authRequest.redirectUri, &error ) )
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

  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog );
  connect( buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject );
  layout->addWidget( buttons );

  if ( dialog.exec() != QDialog::Accepted )
    return;

  QgsAiModelRouter::ProviderSettings openAiSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::OpenAi );
  openAiSettings.endpoint = openAiEndpoint->text().trimmed();
  openAiSettings.model = openAiModel->text().trimmed();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::OpenAi, openAiSettings );

  QgsAiModelRouter::ProviderSettings codexSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Codex );
  codexSettings.endpoint = codexEndpoint->text().trimmed();
  codexSettings.model = codexModel->text().trimmed();
  codexSettings.credentialMode = QgsAiModelRouter::CredentialMode::OAuth;
  codexSettings.enabled = QgsAiCodexOAuthClient::hasRefreshToken() || codexSettings.enabled;
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Codex, codexSettings );

  QgsAiModelRouter::ProviderSettings claudeSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude );
  claudeSettings.endpoint = claudeEndpoint->text().trimmed();
  claudeSettings.model = claudeModel->text().trimmed();
  claudeSettings.credentialMode = claudeUseOAuth->isChecked() ? QgsAiModelRouter::CredentialMode::OAuth : QgsAiModelRouter::CredentialMode::ApiKey;
  if ( claudeSettings.credentialMode == QgsAiModelRouter::CredentialMode::OAuth )
    claudeSettings.enabled = QgsAiClaudeOAuthClient::hasRefreshToken() || claudeSettings.enabled;
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Claude, claudeSettings );

  QgsAiModelRouter::ProviderSettings planSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan );
  planSettings.endpoint = planEndpoint->text().trimmed();
  planSettings.authConfigId = planAuthCfg->text().trimmed();
  mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Plan, planSettings );
  mModelRouter->setPlanAuthConfigId( planAuthCfg->text().trimmed() );

  QString errorMessages;
  QString error;
  if ( !openAiKey->text().trimmed().isEmpty() && !mModelRouter->storeApiKey( QgsAiModelRouter::Provider::OpenAi, openAiKey->text().trimmed(), &error ) )
    errorMessages += error + '\n';
  if ( !claudeKey->text().trimmed().isEmpty() && !mModelRouter->storeApiKey( QgsAiModelRouter::Provider::Claude, claudeKey->text().trimmed(), &error ) )
    errorMessages += error + '\n';
  if ( !planToken->text().trimmed().isEmpty() && !mModelRouter->setPlanSessionToken( planToken->text().trimmed(), &error ) )
    errorMessages += error + '\n';

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
  if ( settings.value( u"qgis_ai/welcome_seen"_s, false ).toBool() )
    return;

  if ( !mModelRouter )
    return;

  // If the user already has a key for any of the standard providers, don't
  // bother them — just remember we've seen it and move on.
  if ( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi )
       || mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Codex )
       || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude )
       || mModelRouter->hasStoredOAuthRefreshToken( QgsAiModelRouter::Provider::Claude )
       || mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Plan ) )
  {
    settings.setValue( u"qgis_ai/welcome_seen"_s, true );
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
  settings.setValue( u"qgis_ai/welcome_seen"_s, true );
}
