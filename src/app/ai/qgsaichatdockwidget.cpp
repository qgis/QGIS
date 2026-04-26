#include "qgsaichatdockwidget.h"

#include "qgsaiagentsessionmanager.h"
#include "qgsaimodelrouter.h"
#include "qgsaireviewpatchengine.h"
#include "qgsapplication.h"

#include <QAction>
#include <QActionGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QTextCursor>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

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
      { QStringLiteral( "GPT-4o" ), QStringLiteral( "gpt-4o" ), QgsAiModelRouter::Provider::OpenAi },
      { QStringLiteral( "GPT-4.1 mini" ), QStringLiteral( "gpt-4.1-mini" ), QgsAiModelRouter::Provider::OpenAi },
      { QStringLiteral( "Claude Sonnet 4" ), QStringLiteral( "claude-sonnet-4-20250514" ), QgsAiModelRouter::Provider::Claude },
      { QStringLiteral( "Claude Sonnet 3.7" ), QStringLiteral( "claude-3-7-sonnet-20250219" ), QgsAiModelRouter::Provider::Claude },
      { QStringLiteral( "Claude Opus 4.1" ), QStringLiteral( "claude-opus-4-1-20250805" ), QgsAiModelRouter::Provider::Claude },
      { QStringLiteral( "Plan backend" ), QStringLiteral( "managed-plan" ), QgsAiModelRouter::Provider::Plan },
    };
  }

  QString modeLabelToAgent( const QString &label )
  {
    if ( label == QLatin1String( "Plan" ) )
      return QStringLiteral( "planner" );
    if ( label == QLatin1String( "Agent" ) )
      return QStringLiteral( "editor" );
    if ( label == QLatin1String( "Ask" ) )
      return QStringLiteral( "reviewer" );
    return QStringLiteral( "planner" );
  }

  QString agentToModeLabel( const QString &agent )
  {
    if ( agent == QLatin1String( "planner" ) )
      return QStringLiteral( "Plan" );
    if ( agent == QLatin1String( "editor" ) )
      return QStringLiteral( "Agent" );
    if ( agent == QLatin1String( "reviewer" ) )
      return QStringLiteral( "Ask" );
    return QStringLiteral( "Plan" );
  }
}

Q_DECLARE_METATYPE( ModelEntry )

QgsAiChatDockWidget::QgsAiChatDockWidget( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiReviewPatchEngine *reviewEngine, QWidget *parent )
  : QgsDockWidget( tr( "AI Assistant" ), parent )
  , mSessionManager( sessionManager )
  , mModelRouter( modelRouter )
  , mReviewEngine( reviewEngine )
{
  setObjectName( QStringLiteral( "AiAssistant" ) );

  QWidget *container = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( container );
  layout->setContentsMargins( 8, 8, 8, 8 );
  layout->setSpacing( 6 );

  mTranscript = new QTextEdit( container );
  mTranscript->setReadOnly( true );
  mTranscript->setFrameShape( QFrame::NoFrame );
  layout->addWidget( mTranscript, 1 );

  mFileContextChipRow = new QWidget( container );
  QHBoxLayout *chipLayout = new QHBoxLayout( mFileContextChipRow );
  chipLayout->setContentsMargins( 0, 0, 0, 0 );
  chipLayout->setSpacing( 4 );
  mFileContextChip = new QLabel( mFileContextChipRow );
  mFileContextChip->setTextInteractionFlags( Qt::TextSelectableByMouse );
  mFileContextClearBtn = new QToolButton( mFileContextChipRow );
  mFileContextClearBtn->setText( QStringLiteral( "×" ) );
  mFileContextClearBtn->setAutoRaise( true );
  mFileContextClearBtn->setFixedSize( 18, 18 );
  mFileContextClearBtn->setToolTip( tr( "Remove file context" ) );
  chipLayout->addWidget( mFileContextChip, 1 );
  chipLayout->addWidget( mFileContextClearBtn );
  mFileContextChipRow->setVisible( false );
  layout->addWidget( mFileContextChipRow );

  mInputTextEdit = new QTextEdit( container );
  mInputTextEdit->setPlaceholderText( tr( "Ask a question, or send /patch…  (Shift+Enter for newline)" ) );
  mInputTextEdit->setAcceptRichText( false );
  mInputTextEdit->setTabChangesFocus( true );
  mInputTextEdit->setFixedHeight( 72 );
  mInputTextEdit->installEventFilter( this );
  layout->addWidget( mInputTextEdit );

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
  mAttachButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionAdd.svg" ) ) );
  mAttachButton->setAutoRaise( true );
  mAttachButton->setFixedSize( 28, 28 );
  mAttachButton->setToolTip( tr( "Attach file context" ) );
  bottomBar->addWidget( mAttachButton );

  mSettingsButton = new QToolButton( container );
  mSettingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ) );
  mSettingsButton->setAutoRaise( true );
  mSettingsButton->setFixedSize( 28, 28 );
  mSettingsButton->setToolTip( tr( "Provider settings" ) );
  bottomBar->addWidget( mSettingsButton );

  mSendButton = new QToolButton( container );
  mSendButton->setText( QStringLiteral( "↑" ) );
  mSendButton->setToolTip( tr( "Send (Enter)" ) );
  mSendButton->setFixedSize( 28, 28 );
  bottomBar->addWidget( mSendButton );

  layout->addLayout( bottomBar );

  mRuntimeStatusLabel = new QLabel( container );
  mRuntimeStatusLabel->setObjectName( QStringLiteral( "aiRuntimeStatusLabel" ) );
  mRuntimeStatusLabel->setText( tr( "Provider state: idle - Ready." ) );

  QHBoxLayout *statusRow = new QHBoxLayout();
  statusRow->setContentsMargins( 0, 0, 0, 0 );
  statusRow->setSpacing( 6 );
  statusRow->addWidget( mRuntimeStatusLabel, 1 );

  mCancelButton = new QPushButton( tr( "Cancel" ), container );
  mCancelButton->setObjectName( QStringLiteral( "aiCancelRequestButton" ) );
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
    mModePill->setText( initialLabel + QStringLiteral( " ▾" ) );
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
      appendTranscriptLine( QStringLiteral( "[%1] %2" ).arg( qgsAiChatRoleToString( message.role ), message.content ) );
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
  connect( mFileContextClearBtn, &QToolButton::clicked, this, &QgsAiChatDockWidget::clearFileContext );
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
    if ( ( keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter )
         && !( keyEvent->modifiers() & Qt::ShiftModifier ) )
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
  const QStringList labels = { QStringLiteral( "Plan" ), QStringLiteral( "Agent" ), QStringLiteral( "Ask" ) };
  for ( const QString &label : labels )
  {
    QAction *action = menu->addAction( label );
    action->setCheckable( true );
    group->addAction( action );
  }
  mModePill->setMenu( menu );
  mModePill->setText( QStringLiteral( "Plan ▾" ) );
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
  mModelPill->setText( initialDisplay + QStringLiteral( " ▾" ) );

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

void QgsAiChatDockWidget::appendTranscriptLine( const QString &line )
{
  mTranscript->append( line );
}

void QgsAiChatDockWidget::appendStreamChunk( const QString &chunk )
{
  if ( chunk.isEmpty() )
    return;

  if ( !mStreamingInProgress )
  {
    mTranscript->append( QStringLiteral( "[assistant] " ) );
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
    cursor.insertText( finalText );
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
    mSendButton->setText( running ? QStringLiteral( "◼" ) : QStringLiteral( "↑" ) );
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
  mModePill->setText( label + QStringLiteral( " ▾" ) );
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

  const QList<QgsAiModelRouter::Provider> providers = {
    QgsAiModelRouter::Provider::OpenAi,
    QgsAiModelRouter::Provider::Claude,
    QgsAiModelRouter::Provider::Plan
  };
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
  mModelPill->setText( entry.displayName + QStringLiteral( " ▾" ) );
}

void QgsAiChatDockWidget::sendMessage()
{
  if ( !mSessionManager )
    return;
  const QString input = mInputTextEdit->toPlainText().trimmed();
  if ( input.isEmpty() )
    return;

  mInputTextEdit->clear();
  mSessionManager->sendUserMessage( input, mFileContextPath );
}

void QgsAiChatDockWidget::cancelRunningRequest()
{
  if ( mSessionManager )
    mSessionManager->cancelActiveRequest();
}

void QgsAiChatDockWidget::attachFile()
{
  const QString path = QFileDialog::getOpenFileName( this, tr( "Attach file context" ) );
  if ( path.isEmpty() )
    return;
  mFileContextPath = path;
  updateFileContextChip();
}

void QgsAiChatDockWidget::clearFileContext()
{
  mFileContextPath.clear();
  updateFileContextChip();
}

void QgsAiChatDockWidget::updateFileContextChip()
{
  const bool hasContext = !mFileContextPath.isEmpty();
  if ( mFileContextChipRow )
    mFileContextChipRow->setVisible( hasContext );
  if ( hasContext && mFileContextChip )
  {
    const QString name = QFileInfo( mFileContextPath ).fileName();
    mFileContextChip->setText( QStringLiteral( "📎 %1" ).arg( name ) );
    mFileContextChip->setToolTip( mFileContextPath );
  }
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
  const QString input = QInputDialog::getText( this, tr( "Accept partial proposal" ), tr( "Hunk indices (comma separated, zero based):" ), QLineEdit::Normal, QStringLiteral( "0" ), &ok );
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
  openAiKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::OpenAi )
                                   ? tr( "Saved locally — enter a new key only to replace it" )
                                   : tr( "sk-..." ) );

  QLineEdit *claudeEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).endpoint, &dialog );
  QLineEdit *claudeModel = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude ).model, &dialog );
  QLineEdit *claudeKey = new QLineEdit( &dialog );
  claudeKey->setEchoMode( QLineEdit::Password );
  claudeKey->setPlaceholderText( mModelRouter->hasStoredApiKey( QgsAiModelRouter::Provider::Claude )
                                   ? tr( "Saved locally — enter a new key only to replace it" )
                                   : tr( "anthropic key..." ) );

  QLineEdit *planEndpoint = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint, &dialog );
  QLineEdit *planAuthCfg = new QLineEdit( mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).authConfigId, &dialog );
  QLineEdit *planToken = new QLineEdit( &dialog );
  planToken->setEchoMode( QLineEdit::Password );
  planToken->setPlaceholderText( tr( "Session token from your plan login..." ) );

  form->addRow( tr( "OpenAI endpoint" ), openAiEndpoint );
  form->addRow( tr( "OpenAI model" ), openAiModel );
  form->addRow( tr( "OpenAI API key" ), openAiKey );
  form->addRow( tr( "Claude endpoint" ), claudeEndpoint );
  form->addRow( tr( "Claude model" ), claudeModel );
  form->addRow( tr( "Claude API key" ), claudeKey );
  form->addRow( tr( "Plan backend endpoint" ), planEndpoint );
  form->addRow( tr( "Plan OAuth authcfg ID" ), planAuthCfg );
  form->addRow( tr( "Plan session token" ), planToken );
  layout->addLayout( form );

  QLabel *helpLabel = new QLabel( tr( "OpenAI and Claude API keys are stored locally in QGIS settings and do not require the QGIS master authentication password. Leave API key fields empty to keep the current saved value." ), &dialog );
  helpLabel->setWordWrap( true );
  layout->addWidget( helpLabel );

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

  QgsAiModelRouter::ProviderSettings claudeSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Claude );
  claudeSettings.endpoint = claudeEndpoint->text().trimmed();
  claudeSettings.model = claudeModel->text().trimmed();
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
