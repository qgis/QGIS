/***************************************************************************
    qgsaiaccountwidget.cpp
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

#include "qgsaiaccountwidget.h"

#include <utility>

#include "qgsaiagentsessionmanager.h"
#include "qgsaimodelrouter.h"
#include "qgsaiplanclient.h"
#include "qgsaisettingsutils.h"
#include "qgscollapsiblegroupbox.h"

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QString>
#include <QStyle>
#include <QVBoxLayout>

#include "moc_qgsaiaccountwidget.cpp"

using namespace Qt::StringLiterals;

QgsAiAccountWidget::QgsAiAccountWidget( QgsAiModelRouter *modelRouter, QgsAiAgentSessionManager *sessionManager, QWidget *parent )
  : QWidget( parent )
  , mModelRouter( modelRouter )
  , mSessionManager( sessionManager )
{
  mPlanClient = new QgsAiPlanClient( this );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 12 );

  mStateStack = new QStackedWidget( this );
  mStateStack->setObjectName( u"aiPlanAccountStack"_s );
  mStateStack->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );
  mStateStack->addWidget( buildLoggedOutPane() );
  mStateStack->addWidget( buildLoggedInPane() );
  layout->addWidget( mStateStack );

  mStatusLabel = new QLabel( this );
  mStatusLabel->setObjectName( u"aiPlanStatusLabel"_s );
  mStatusLabel->setWordWrap( true );
  mStatusLabel->setMaximumWidth( 420 );
  layout->addWidget( mStatusLabel );

  QLabel *caption = new QLabel( tr( "Account changes take effect immediately." ), this );
  caption->setProperty( "aiRole", u"rowDescription"_s );
  layout->addWidget( caption );

  // Advanced: the technical plumbing hidden from the default flow.
  mAdvancedGroup = new QgsCollapsibleGroupBox( tr( "Advanced" ), this );
  mAdvancedGroup->setObjectName( u"aiPlanAdvancedGroupBox"_s );
  QFormLayout *advancedForm = new QFormLayout( mAdvancedGroup );
  mEndpointEdit = new QLineEdit( mModelRouter ? mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint : QString(), mAdvancedGroup );
  mEndpointEdit->setObjectName( u"aiPlanEndpointLineEdit"_s );
  mAuthCfgEdit = new QLineEdit( mModelRouter ? mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).authConfigId : QString(), mAdvancedGroup );
  mAuthCfgEdit->setObjectName( u"aiPlanAuthCfgLineEdit"_s );
  mTokenEdit = new QLineEdit( mAdvancedGroup );
  mTokenEdit->setObjectName( u"aiPlanSessionTokenLineEdit"_s );
  mTokenEdit->setEchoMode( QLineEdit::Password );
  mTokenEdit->setPlaceholderText( tr( "Session token from your plan login..." ) );
  advancedForm->addRow( tr( "Backend endpoint" ), mEndpointEdit );
  advancedForm->addRow( tr( "OAuth authcfg ID" ), mAuthCfgEdit );
  advancedForm->addRow( tr( "Session token" ), mTokenEdit );
  mAdvancedGroup->setCollapsed( true );
  layout->addWidget( mAdvancedGroup );

  connect( mEndpointEdit, &QLineEdit::textChanged, this, [this]( const QString & ) { updateFormState(); } );

  setStyleSheet( uR"css(
QPushButton[aiSegment="true"] { border: 1px solid palette(mid); background: transparent; padding: 6px 14px; }
QPushButton[aiSegment="true"]:checked { background: palette(highlight); color: palette(highlighted-text); border-color: palette(highlight); font-weight: 600; }
QPushButton#aiPlanModeLoginButton { border-top-left-radius: 6px; border-bottom-left-radius: 6px; }
QPushButton#aiPlanModeSignupButton { border-top-right-radius: 6px; border-bottom-right-radius: 6px; }
QPushButton#aiPlanLoginButton, QPushButton#aiPlanRegisterButton { background: palette(highlight); color: palette(highlighted-text); border: none; border-radius: 6px; padding: 9px 12px; font-weight: 600; }
QPushButton#aiPlanLoginButton:disabled, QPushButton#aiPlanRegisterButton:disabled { background: palette(midlight); color: palette(dark); }
QLabel#aiPlanAvatarLabel { background: palette(highlight); color: palette(highlighted-text); border-radius: 20px; font-weight: 600; }
QFrame#aiPlanAccountCard { background: palette(button); border-radius: 10px; }
QLabel#aiPlanStatusLabel[status="error"] { color: #e5534b; }
QLabel[aiRole="rowDescription"] { color: palette(dark); }
)css"_s );

  // Plan client wiring: login/register produce a desktop token, then account,
  // models and agent policy are chained. Same flow as the legacy dialog.
  connect( mPlanClient, &QgsAiPlanClient::desktopTokenReady, this, &QgsAiAccountWidget::onDesktopTokenReady );
  connect( mPlanClient, &QgsAiPlanClient::accountReady, this, [this]( const QgsAiPlanClient::AccountInfo &account ) {
    mAccountEmail = account.email;
    mAccountTier = account.tier;
    updateAccountCard();
    setStatus( tr( "Signed in as %1 (%2)." ).arg( account.email, account.tier ) );
    setBusy( false );
    emit accountInfoChanged();
  } );
  connect( mPlanClient, &QgsAiPlanClient::modelsReady, this, [this]( const QList<QgsAiPlanClient::ModelInfo> &models, bool fromCache ) {
    setStatus( tr( "Managed model catalog: %n model(s)%1.", nullptr, models.size() ).arg( fromCache ? tr( " from cache" ) : QString() ) );
    setBusy( false );
    emit authStateChanged();
  } );
  connect( mPlanClient, &QgsAiPlanClient::agentPolicyReady, this, [this]( const QgsAiManagedAgentPolicy &policy, bool fromCache ) {
    if ( mSessionManager )
      mSessionManager->setManagedAgentPolicy( policy );
    setStatus( tr( "Agent policy loaded for %1%2." ).arg( policy.tier.isEmpty() ? tr( "current tier" ) : policy.tier, fromCache ? tr( " from cache" ) : QString() ) );
    setBusy( false );
    emit authStateChanged();
  } );
  connect( mPlanClient, &QgsAiPlanClient::balanceReady, this, &QgsAiAccountWidget::onBalanceReady );
  connect( mPlanClient, &QgsAiPlanClient::modelPreferencesReady, this, &QgsAiAccountWidget::onModelPreferencesReady );
  connect( mPlanClient, &QgsAiPlanClient::modelPreferenceUpdated, this, &QgsAiAccountWidget::onModelPreferenceUpdated );
  connect( mPlanClient, &QgsAiPlanClient::modelPreferenceUpdateFailed, this, &QgsAiAccountWidget::onModelPreferenceUpdateFailed );
  connect( mPlanClient, &QgsAiPlanClient::requestFailed, this, &QgsAiAccountWidget::onRequestFailed );

  const bool signedIn = isSignedIn();
  mStateStack->setCurrentIndex( signedIn ? 1 : 0 );
  if ( signedIn )
  {
    updateAccountCard();
    updateUsageCard();
    populateModelList();
    if ( endpointUsable() )
    {
      setStatus( tr( "Signed in — loading account…" ) );
      mPlanClient->fetchMe( currentEndpoint(), mModelRouter->planSessionToken() );
      mPlanClient->fetchBalance( currentEndpoint(), mModelRouter->planSessionToken() );
      mPlanClient->fetchModelPreferences( currentEndpoint(), mModelRouter->planSessionToken() );
    }
    else
    {
      setStatus( tr( "Signed in." ) );
    }
  }
  else
  {
    setStatus( tr( "Not signed in." ) );
  }
  setMode( Mode::Login );
}

QWidget *QgsAiAccountWidget::buildLoggedOutPane()
{
  QWidget *pane = new QWidget( this );
  pane->setMaximumWidth( 420 );
  QVBoxLayout *layout = new QVBoxLayout( pane );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 10 );

  // The Log in / Sign up choice is the very first element of the page.
  QWidget *toggleRow = new QWidget( pane );
  QHBoxLayout *toggleLayout = new QHBoxLayout( toggleRow );
  toggleLayout->setContentsMargins( 0, 0, 0, 0 );
  toggleLayout->setSpacing( 0 );
  mModeLoginButton = new QPushButton( tr( "Log in" ), toggleRow );
  mModeLoginButton->setObjectName( u"aiPlanModeLoginButton"_s );
  mModeSignupButton = new QPushButton( tr( "Sign up" ), toggleRow );
  mModeSignupButton->setObjectName( u"aiPlanModeSignupButton"_s );
  for ( QPushButton *button : { mModeLoginButton, mModeSignupButton } )
  {
    button->setCheckable( true );
    button->setProperty( "aiSegment", true );
    button->setCursor( Qt::PointingHandCursor );
    toggleLayout->addWidget( button, 1 );
  }
  QButtonGroup *modeGroup = new QButtonGroup( toggleRow );
  modeGroup->setExclusive( true );
  modeGroup->addButton( mModeLoginButton );
  modeGroup->addButton( mModeSignupButton );
  mModeLoginButton->setChecked( true );
  connect( mModeLoginButton, &QPushButton::toggled, this, [this]( bool checked ) {
    if ( checked )
      setMode( Mode::Login );
  } );
  connect( mModeSignupButton, &QPushButton::toggled, this, [this]( bool checked ) {
    if ( checked )
      setMode( Mode::Signup );
  } );
  layout->addWidget( toggleRow );

  mEndpointWarning = new QLabel( tr( "Backend endpoint is not configured — set it under Advanced below." ), pane );
  mEndpointWarning->setWordWrap( true );
  mEndpointWarning->setProperty( "aiRole", u"rowDescription"_s );
  mEndpointWarning->setVisible( false );
  layout->addWidget( mEndpointWarning );

  mEmail = new QLineEdit( pane );
  mEmail->setObjectName( u"aiPlanEmailLineEdit"_s );
  mEmail->setPlaceholderText( tr( "you@example.com" ) );
  layout->addWidget( mEmail );

  mPassword = new QLineEdit( pane );
  mPassword->setObjectName( u"aiPlanPasswordLineEdit"_s );
  mPassword->setEchoMode( QLineEdit::Password );
  mPassword->setPlaceholderText( tr( "Password" ) );
  layout->addWidget( mPassword );

  mPasswordHint = new QLabel( tr( "At least 8 characters." ), pane );
  mPasswordHint->setProperty( "aiRole", u"rowDescription"_s );
  mPasswordHint->setVisible( false );
  layout->addWidget( mPasswordHint );

  mLoginButton = new QPushButton( tr( "Log in" ), pane );
  mLoginButton->setObjectName( u"aiPlanLoginButton"_s );
  mLoginButton->setCursor( Qt::PointingHandCursor );
  layout->addWidget( mLoginButton );

  mRegisterButton = new QPushButton( tr( "Create account" ), pane );
  mRegisterButton->setObjectName( u"aiPlanRegisterButton"_s );
  mRegisterButton->setCursor( Qt::PointingHandCursor );
  mRegisterButton->setVisible( false );
  layout->addWidget( mRegisterButton );

  connect( mEmail, &QLineEdit::textChanged, this, [this]( const QString & ) { updateFormState(); } );
  connect( mPassword, &QLineEdit::textChanged, this, [this]( const QString & ) { updateFormState(); } );
  connect( mEmail, &QLineEdit::returnPressed, this, [this]() { mPassword->setFocus(); } );
  connect( mPassword, &QLineEdit::returnPressed, this, [this]() {
    if ( mMode == Mode::Login && mLoginButton->isEnabled() )
      mLoginButton->click();
    else if ( mMode == Mode::Signup && mRegisterButton->isEnabled() )
      mRegisterButton->click();
  } );
  connect( mLoginButton, &QPushButton::clicked, this, &QgsAiAccountWidget::startLogin );
  connect( mRegisterButton, &QPushButton::clicked, this, &QgsAiAccountWidget::startRegister );

  return pane;
}

QWidget *QgsAiAccountWidget::buildLoggedInPane()
{
  QWidget *pane = new QWidget( this );
  pane->setMaximumWidth( 420 );
  QVBoxLayout *layout = new QVBoxLayout( pane );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 10 );

  QFrame *card = new QFrame( pane );
  card->setObjectName( u"aiPlanAccountCard"_s );
  QHBoxLayout *cardLayout = new QHBoxLayout( card );
  cardLayout->setContentsMargins( 12, 12, 12, 12 );
  cardLayout->setSpacing( 12 );

  mAvatarLabel = new QLabel( card );
  mAvatarLabel->setObjectName( u"aiPlanAvatarLabel"_s );
  mAvatarLabel->setFixedSize( 40, 40 );
  mAvatarLabel->setAlignment( Qt::AlignCenter );
  QFont avatarFont = mAvatarLabel->font();
  avatarFont.setPointSize( avatarFont.pointSize() + 4 );
  avatarFont.setBold( true );
  mAvatarLabel->setFont( avatarFont );
  cardLayout->addWidget( mAvatarLabel );

  QVBoxLayout *identityLayout = new QVBoxLayout();
  identityLayout->setContentsMargins( 0, 0, 0, 0 );
  identityLayout->setSpacing( 2 );
  mEmailLabel = new QLabel( card );
  QFont emailFont = mEmailLabel->font();
  emailFont.setBold( true );
  mEmailLabel->setFont( emailFont );
  mTierLabel = new QLabel( card );
  mTierLabel->setProperty( "aiRole", u"rowDescription"_s );
  identityLayout->addWidget( mEmailLabel );
  identityLayout->addWidget( mTierLabel );
  cardLayout->addLayout( identityLayout, 1 );
  layout->addWidget( card );

  QWidget *buttonRow = new QWidget( pane );
  QHBoxLayout *buttonLayout = new QHBoxLayout( buttonRow );
  buttonLayout->setContentsMargins( 0, 0, 0, 0 );
  mLogoutButton = new QPushButton( tr( "Log out" ), buttonRow );
  mLogoutButton->setObjectName( u"aiPlanLogoutButton"_s );
  mRefreshModelsButton = new QPushButton( tr( "Refresh models" ), buttonRow );
  mRefreshModelsButton->setObjectName( u"aiPlanRefreshModelsButton"_s );
  buttonLayout->addWidget( mLogoutButton );
  buttonLayout->addWidget( mRefreshModelsButton );
  buttonLayout->addStretch( 1 );
  layout->addWidget( buttonRow );

  connect( mLogoutButton, &QPushButton::clicked, this, &QgsAiAccountWidget::logout );
  connect( mRefreshModelsButton, &QPushButton::clicked, this, &QgsAiAccountWidget::refreshManagedModels );

  // Usage card: monthly credit quota consumption, Cursor "Plan & Usage" style.
  QVBoxLayout *usageLayout = new QVBoxLayout();
  usageLayout->setContentsMargins( 0, 0, 0, 0 );
  usageLayout->setSpacing( 4 );
  usageLayout->addWidget( QgsAiSettingsUtils::sectionHeader( tr( "Usage" ), pane ) );
  mUsageLabel = new QLabel( pane );
  mUsageLabel->setProperty( "aiRole", u"rowDescription"_s );
  usageLayout->addWidget( mUsageLabel );
  mUsageBar = new QProgressBar( pane );
  mUsageBar->setObjectName( u"aiPlanUsageBar"_s );
  mUsageBar->setRange( 0, 100 );
  mUsageBar->setTextVisible( false );
  mUsageBar->setFixedHeight( 8 );
  usageLayout->addWidget( mUsageBar );
  layout->addLayout( usageLayout );

  // Models: the managed catalog with per-account enable/disable, mirrors Cursor's Models page.
  QVBoxLayout *modelsLayout = new QVBoxLayout();
  modelsLayout->setContentsMargins( 0, 0, 0, 0 );
  modelsLayout->setSpacing( 4 );
  modelsLayout->addWidget( QgsAiSettingsUtils::sectionHeader( tr( "Models" ), pane ) );
  QLabel *modelsDescription = new QLabel( tr( "Choose which managed models appear in the chat model picker." ), pane );
  modelsDescription->setProperty( "aiRole", u"rowDescription"_s );
  modelsDescription->setWordWrap( true );
  modelsLayout->addWidget( modelsDescription );
  mModelListWidget = new QListWidget( pane );
  mModelListWidget->setObjectName( u"aiPlanModelListWidget"_s );
  mModelListWidget->setSelectionMode( QAbstractItemView::NoSelection );
  mModelListWidget->setMaximumHeight( 220 );
  modelsLayout->addWidget( mModelListWidget );
  layout->addLayout( modelsLayout );

  connect( mModelListWidget, &QListWidget::itemChanged, this, &QgsAiAccountWidget::onModelListItemChanged );

  return pane;
}

QString QgsAiAccountWidget::planEndpoint() const
{
  return mEndpointEdit->text().trimmed();
}

QString QgsAiAccountWidget::planAuthConfigId() const
{
  return mAuthCfgEdit->text().trimmed();
}

QString QgsAiAccountWidget::manualSessionToken() const
{
  return mTokenEdit->text().trimmed();
}

QString QgsAiAccountWidget::accountEmail() const
{
  return mAccountEmail;
}

QString QgsAiAccountWidget::accountTier() const
{
  return mAccountTier;
}

bool QgsAiAccountWidget::isSignedIn() const
{
  return mModelRouter && !mModelRouter->planSessionToken().trimmed().isEmpty();
}

void QgsAiAccountWidget::setMode( Mode mode )
{
  // Clear stale status only on an actual mode switch, so the initial status
  // set by the constructor survives its trailing setMode( Mode::Login ) call.
  const bool changed = mMode != mode;
  mMode = mode;
  mLoginButton->setVisible( mode == Mode::Login );
  mRegisterButton->setVisible( mode == Mode::Signup );
  mPasswordHint->setVisible( mode == Mode::Signup );
  if ( changed && !mBusy )
    setStatus( QString() );
  updateFormState();
}

void QgsAiAccountWidget::setBusy( bool busy )
{
  mBusy = busy;
  mModeLoginButton->setEnabled( !busy );
  mModeSignupButton->setEnabled( !busy );
  mEmail->setEnabled( !busy );
  mPassword->setEnabled( !busy );
  mRefreshModelsButton->setEnabled( !busy );
  if ( !busy )
  {
    mLoginButton->setText( tr( "Log in" ) );
    mRegisterButton->setText( tr( "Create account" ) );
  }
  updateFormState();
}

void QgsAiAccountWidget::setStatus( const QString &text, bool error )
{
  mStatusLabel->setText( text );
  mStatusLabel->setProperty( "status", error ? u"error"_s : QString() );
  if ( QStyle *labelStyle = mStatusLabel->style() )
  {
    labelStyle->unpolish( mStatusLabel );
    labelStyle->polish( mStatusLabel );
  }
}

void QgsAiAccountWidget::updateFormState()
{
  const bool usableEndpoint = endpointUsable();
  mEndpointWarning->setVisible( !usableEndpoint );

  const bool emailOk = mEmail->text().contains( '@'_L1 );
  const QString password = mPassword->text();
  const bool passwordOk = mMode == Mode::Login ? !password.isEmpty() : password.length() >= 8;
  const bool canSubmit = !mBusy && usableEndpoint && emailOk && passwordOk;
  mLoginButton->setEnabled( canSubmit );
  mRegisterButton->setEnabled( canSubmit );
}

void QgsAiAccountWidget::updateAccountCard()
{
  const QString email = mAccountEmail.isEmpty() ? tr( "Signed in" ) : mAccountEmail;
  mEmailLabel->setText( email );
  QString tierText = tr( "Plan account" );
  if ( !mAccountTier.isEmpty() )
  {
    QString tier = mAccountTier.toLower();
    tier[0] = tier[0].toUpper();
    tierText = tr( "%1 plan" ).arg( tier );
  }
  mTierLabel->setText( tierText );
  mAvatarLabel->setText( mAccountEmail.isEmpty() ? u"•"_s : mAccountEmail.left( 1 ).toUpper() );
}

void QgsAiAccountWidget::updateUsageCard()
{
  if ( !mUsageLabel || !mUsageBar )
    return;

  if ( mBalance.isUnlimited() )
  {
    mUsageLabel->setText( tr( "%1 credits available · Unlimited plan" ).arg( mBalance.available ) );
    mUsageBar->setValue( 0 );
    return;
  }

  const int percent = mBalance.usedPercent();
  mUsageLabel->setText( tr( "%1 / %2 credits · %3% used" ).arg( mBalance.available ).arg( mBalance.monthlyCredits ).arg( percent ) );
  mUsageBar->setValue( percent );
}

void QgsAiAccountWidget::populateModelList()
{
  if ( !mModelListWidget )
    return;

  mUpdatingModelList = true;
  mModelListWidget->clear();

  const QList<QgsAiPlanClient::ModelInfo> catalog = QgsAiPlanClient::cachedModels();
  QHash<QString, bool> enabledById;
  for ( const QgsAiPlanClient::ModelPreferenceInfo &preference : std::as_const( mModelPreferences ) )
    enabledById.insert( preference.modelId, preference.enabled );

  for ( const QgsAiPlanClient::ModelInfo &model : catalog )
  {
    auto *item = new QListWidgetItem( model.displayLabel(), mModelListWidget );
    item->setData( Qt::UserRole, model.id );
    if ( !model.tooltip().isEmpty() )
      item->setToolTip( model.tooltip() );
    item->setCheckState( enabledById.value( model.id, true ) ? Qt::Checked : Qt::Unchecked );
  }

  if ( catalog.isEmpty() )
  {
    auto *placeholder = new QListWidgetItem( tr( "Refresh models to see the managed catalog." ), mModelListWidget );
    placeholder->setFlags( placeholder->flags() & ~Qt::ItemIsEnabled );
  }

  mUpdatingModelList = false;
}

QString QgsAiAccountWidget::currentEndpoint() const
{
  return mEndpointEdit->text().trimmed();
}

bool QgsAiAccountWidget::endpointUsable() const
{
  return QgsAiModelRouter::isUsablePlanEndpoint( currentEndpoint() );
}

QString QgsAiAccountWidget::friendlyErrorMessage( const QString &message )
{
  if ( message.contains( "email_taken"_L1, Qt::CaseInsensitive ) )
  {
    mModeLoginButton->setChecked( true );
    return tr( "An account with this email already exists. Try logging in instead." );
  }
  if ( message.contains( "invalid_credentials"_L1, Qt::CaseInsensitive ) )
    return tr( "Incorrect email or password." );
  if ( message.contains( "account_suspended"_L1, Qt::CaseInsensitive ) )
    return tr( "This account is suspended. Contact support." );
  return message;
}

void QgsAiAccountWidget::startLogin()
{
  mInteractiveRequest = true;
  setBusy( true );
  mLoginButton->setText( tr( "Logging in…" ) );
  setStatus( tr( "Logging in to Plan Account..." ) );
  mPlanClient->login( currentEndpoint(), mEmail->text(), mPassword->text() );
}

void QgsAiAccountWidget::startRegister()
{
  mInteractiveRequest = true;
  setBusy( true );
  mRegisterButton->setText( tr( "Creating account…" ) );
  setStatus( tr( "Creating Plan Account..." ) );
  mPlanClient->registerAccount( currentEndpoint(), mEmail->text(), mPassword->text() );
}

void QgsAiAccountWidget::logout()
{
  QString error;
  if ( !mModelRouter || !mModelRouter->clearPlanSessionToken( &error ) )
  {
    setStatus( error.isEmpty() ? tr( "Unable to logout Plan Account." ) : error, true );
    return;
  }
  if ( mSessionManager )
    mSessionManager->setManagedAgentPolicy( QgsAiManagedAgentPolicy() );
  mAccountEmail.clear();
  mAccountTier.clear();
  mPassword->clear();
  mStateStack->setCurrentIndex( 0 );
  setStatus( tr( "Plan Account logged out." ) );
  emit accountInfoChanged();
  emit authStateChanged();
}

void QgsAiAccountWidget::refreshManagedModels()
{
  mInteractiveRequest = true;
  setBusy( true );
  setStatus( tr( "Refreshing managed model catalog..." ) );
  mPlanClient->refreshModels( currentEndpoint() );
  if ( mModelRouter && !mModelRouter->planSessionToken().trimmed().isEmpty() )
  {
    mPlanClient->refreshAgents( currentEndpoint(), mModelRouter->planSessionToken() );
    mPlanClient->refreshAgentPolicy( currentEndpoint(), mModelRouter->planSessionToken() );
    mPlanClient->fetchBalance( currentEndpoint(), mModelRouter->planSessionToken() );
    mPlanClient->fetchModelPreferences( currentEndpoint(), mModelRouter->planSessionToken() );
  }
}

void QgsAiAccountWidget::onDesktopTokenReady( const QString &token )
{
  if ( mModelRouter )
  {
    // The token was minted against the live Advanced-field endpoint: persist
    // that endpoint together with the token, so cancelling the dialog cannot
    // leave a fresh token paired with a stale persisted endpoint.
    QgsAiModelRouter::ProviderSettings planSettings = mModelRouter->providerSettings( QgsAiModelRouter::Provider::Plan );
    if ( planSettings.endpoint != currentEndpoint() )
    {
      planSettings.endpoint = currentEndpoint();
      mModelRouter->setProviderSettings( QgsAiModelRouter::Provider::Plan, planSettings );
    }
  }
  QString error;
  if ( !mModelRouter || !mModelRouter->setPlanSessionToken( token, &error ) )
  {
    setStatus( error.isEmpty() ? tr( "Unable to store Plan token." ) : error, true );
    setBusy( false );
    return;
  }
  mTokenEdit->clear();
  mPassword->clear();
  mStateStack->setCurrentIndex( 1 );
  updateAccountCard();
  setStatus( tr( "Plan Account signed in. Loading account and model catalog..." ) );
  mPlanClient->fetchMe( currentEndpoint(), token );
  mPlanClient->fetchBalance( currentEndpoint(), token );
  mPlanClient->refreshModels( currentEndpoint() );
  mPlanClient->refreshAgents( currentEndpoint(), token );
  mPlanClient->refreshAgentPolicy( currentEndpoint(), token );
  mPlanClient->fetchModelPreferences( currentEndpoint(), token );
  emit accountInfoChanged();
  emit authStateChanged();
}

void QgsAiAccountWidget::onRequestFailed( const QString &message )
{
  if ( !mInteractiveRequest )
  {
    // Failure of the silent account refresh fired on open (expired token,
    // backend unreachable): keep a neutral status instead of an unprompted
    // red "Incorrect email or password." on a pane with no credential fields.
    setStatus( isSignedIn() ? tr( "Signed in — account details are unavailable right now." ) : tr( "Not signed in." ) );
    setBusy( false );
    return;
  }
  setStatus( friendlyErrorMessage( message ), true );
  setBusy( false );
}

void QgsAiAccountWidget::onBalanceReady( const QgsAiPlanClient::BalanceInfo &balance )
{
  mBalance = balance;
  updateUsageCard();
}

void QgsAiAccountWidget::onModelPreferencesReady( const QList<QgsAiPlanClient::ModelPreferenceInfo> &preferences, bool fromCache )
{
  Q_UNUSED( fromCache )
  mModelPreferences = preferences;
  populateModelList();
}

void QgsAiAccountWidget::onModelPreferenceUpdated( const QString &modelId, bool enabled )
{
  bool found = false;
  for ( QgsAiPlanClient::ModelPreferenceInfo &preference : mModelPreferences )
  {
    if ( preference.modelId == modelId )
    {
      preference.enabled = enabled;
      found = true;
      break;
    }
  }
  if ( !found )
    mModelPreferences << QgsAiPlanClient::ModelPreferenceInfo { modelId, enabled };
  emit modelPreferencesChanged();
}

void QgsAiAccountWidget::onModelListItemChanged( QListWidgetItem *item )
{
  if ( mUpdatingModelList || !item )
    return;

  const QString modelId = item->data( Qt::UserRole ).toString();
  if ( modelId.isEmpty() )
    return;
  const bool enabled = item->checkState() == Qt::Checked;

  if ( !mModelRouter || mModelRouter->planSessionToken().trimmed().isEmpty() || !endpointUsable() )
  {
    // Optimistic UI wouldn't be able to persist without a session — revert immediately.
    mUpdatingModelList = true;
    item->setCheckState( enabled ? Qt::Unchecked : Qt::Checked );
    mUpdatingModelList = false;
    setStatus( tr( "Sign in to change model preferences." ), true );
    return;
  }

  mPlanClient->setModelPreference( currentEndpoint(), mModelRouter->planSessionToken(), modelId, enabled );
}

void QgsAiAccountWidget::onModelPreferenceUpdateFailed( const QString &modelId, bool requestedEnabled, const QString &message )
{
  // Revert exactly the toggle that failed, found by model id — a shared
  // pending pointer would misfire when unrelated requests fail or when two
  // toggles are in flight at once.
  if ( mModelListWidget )
  {
    for ( int i = 0; i < mModelListWidget->count(); ++i )
    {
      QListWidgetItem *item = mModelListWidget->item( i );
      if ( item && item->data( Qt::UserRole ).toString() == modelId )
      {
        mUpdatingModelList = true;
        item->setCheckState( requestedEnabled ? Qt::Unchecked : Qt::Checked );
        mUpdatingModelList = false;
        break;
      }
    }
  }
  setStatus( friendlyErrorMessage( message ), true );
}
