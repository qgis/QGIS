/***************************************************************************
                             qgsprocessingalgorithmdialogbase.cpp
                             ------------------------------------
    Date                 : November 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingalgorithmwidgetbase.h"

#include <nlohmann/json.hpp>

#include "processing/qgsprocessingalgorithm.h"
#include "processing/qgsprocessingalgrunnertask.h"
#include "processing/qgsprocessingprovider.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsjsonutils.h"
#include "qgsmessagebar.h"
#include "qgsnative.h"
#include "qgspanelwidget.h"
#include "qgssettings.h"
#include "qgsstringutils.h"
#include "qgstaskmanager.h"
#include "qgsunittypes.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QScrollBar>
#include <QString>
#include <QToolButton>

#include "moc_qgsprocessingalgorithmwidgetbase.cpp"

using namespace Qt::StringLiterals;

///@cond NOT_STABLE

QgsProcessingAlgorithmDialogFeedback::QgsProcessingAlgorithmDialogFeedback()
  : QgsProcessingFeedback( false )
{}

void QgsProcessingAlgorithmDialogFeedback::setProgressText( const QString &text )
{
  QgsProcessingFeedback::setProgressText( text );
  emit progressTextChanged( text );
}

void QgsProcessingAlgorithmDialogFeedback::reportError( const QString &error, bool fatalError )
{
  QgsProcessingFeedback::reportError( error, fatalError );
  emit errorReported( error, fatalError );
}

void QgsProcessingAlgorithmDialogFeedback::pushWarning( const QString &warning )
{
  QgsProcessingFeedback::pushWarning( warning );
  emit warningPushed( warning );
}

void QgsProcessingAlgorithmDialogFeedback::pushInfo( const QString &info )
{
  QgsProcessingFeedback::pushInfo( info );
  emit infoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushCommandInfo( const QString &info )
{
  QgsProcessingFeedback::pushCommandInfo( info );
  emit commandInfoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushDebugInfo( const QString &info )
{
  QgsProcessingFeedback::pushDebugInfo( info );
  emit debugInfoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushConsoleInfo( const QString &info )
{
  QgsProcessingFeedback::pushConsoleInfo( info );
  emit consoleInfoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushFormattedMessage( const QString &html, const QString &text )
{
  QgsProcessingFeedback::pushFormattedMessage( html, text );
  emit formattedMessagePushed( html );
}

//
// QgsProcessingAlgorithmWidgetBase
//

QgsProcessingAlgorithmWidgetBase::QgsProcessingAlgorithmWidgetBase( QWidget *parent, WidgetMode mode )
  : QDialog( parent )
  , mMode( mode )
{
  setupUi( this );

  //don't collapse parameters panel
  splitter->setCollapsible( 0, false );

  // add collapse button to splitter
  QSplitterHandle *splitterHandle = splitter->handle( 1 );
  QVBoxLayout *handleLayout = new QVBoxLayout();
  handleLayout->setContentsMargins( 0, 0, 0, 0 );
  mButtonCollapse = new QToolButton( splitterHandle );
  mButtonCollapse->setAutoRaise( true );
  mButtonCollapse->setFixedSize( 12, 12 );
  mButtonCollapse->setCursor( Qt::ArrowCursor );
  handleLayout->addWidget( mButtonCollapse );
  handleLayout->addStretch();
  splitterHandle->setLayout( handleLayout );

  QgsGui::enableAutoGeometryRestore( this );

  txtLog->setOpenLinks( false );
  connect( txtLog, &QTextBrowser::anchorClicked, this, &QgsProcessingAlgorithmWidgetBase::urlClicked );

  const QgsSettings settings;
  splitter->restoreState( settings.value( u"/Processing/dialogBaseSplitter"_s, QByteArray() ).toByteArray() );
  mSplitterState = splitter->saveState();
  splitterChanged( 0, 0 );

  // Rename OK button to Run
  mButtonRun = mButtonBox->button( QDialogButtonBox::Ok );
  mButtonRun->setText( tr( "Run" ) );

  // Rename Yes button. Yes is used to ensure same position of Run and Change Parameters with respect to Close button.
  mButtonChangeParameters = mButtonBox->button( QDialogButtonBox::Yes );
  mButtonChangeParameters->setText( tr( "Change Parameters" ) );

  buttonCancel->setEnabled( false );
  mButtonClose = mButtonBox->button( QDialogButtonBox::Close );

  switch ( mMode )
  {
    case QgsProcessingAlgorithmWidgetBase::WidgetMode::Single:
    {
      mAdvancedButton = new QPushButton( tr( "Advanced" ) );
      mAdvancedMenu = new QMenu( this );
      mAdvancedButton->setMenu( mAdvancedMenu );

      mContextSettingsAction = new QAction( tr( "Algorithm Settings…" ), mAdvancedMenu );
      mContextSettingsAction->setIcon( QgsApplication::getThemeIcon( u"/propertyicons/settings.svg"_s ) );
      mAdvancedMenu->addAction( mContextSettingsAction );

      connect( mContextSettingsAction, &QAction::triggered, this, [this] {
        if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( mMainWidget ) )
        {
          mTabWidget->setCurrentIndex( 0 );

          if ( !mContextOptionsWidget )
          {
            mContextOptionsWidget = new QgsProcessingContextOptionsWidget();
            mContextOptionsWidget->setFromContext( processingContext() );
            mContextOptionsWidget->setLogLevel( mLogLevel );
            panel->openPanel( mContextOptionsWidget );

            connect( mContextOptionsWidget, &QgsPanelWidget::widgetChanged, this, [this] {
              mOverrideDefaultContextSettings = true;
              mGeometryCheck = mContextOptionsWidget->invalidGeometryCheck();
              mDistanceUnits = mContextOptionsWidget->distanceUnit();
              mAreaUnits = mContextOptionsWidget->areaUnit();
              mTemporaryFolderOverride = mContextOptionsWidget->temporaryFolder();
              mMaximumThreads = mContextOptionsWidget->maximumThreads();
              mLogLevel = mContextOptionsWidget->logLevel();
            } );
          }
        }
      } );
      mAdvancedMenu->addSeparator();

      QAction *copyAsPythonCommand = new QAction( tr( "Copy as Python Command" ), mAdvancedMenu );
      copyAsPythonCommand->setIcon( QgsApplication::getThemeIcon( u"mIconPythonFile.svg"_s ) );

      mAdvancedMenu->addAction( copyAsPythonCommand );
      connect( copyAsPythonCommand, &QAction::triggered, this, [this] {
        if ( const QgsProcessingAlgorithm *alg = algorithm() )
        {
          QgsProcessingContext *context = processingContext();
          if ( !context )
            return;

          const QString command = alg->asPythonCommand( createProcessingParameters(), *context );
          QMimeData *m = new QMimeData();
          m->setText( command );
          QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
          cb->setMimeData( m, QClipboard::Selection );
#endif
          cb->setMimeData( m, QClipboard::Clipboard );
        }
      } );

      mCopyAsQgisProcessCommand = new QAction( tr( "Copy as qgis_process Command" ), mAdvancedMenu );
      mCopyAsQgisProcessCommand->setIcon( QgsApplication::getThemeIcon( u"mActionTerminal.svg"_s ) );
      mAdvancedMenu->addAction( mCopyAsQgisProcessCommand );

      connect( mCopyAsQgisProcessCommand, &QAction::triggered, this, [this] {
        if ( const QgsProcessingAlgorithm *alg = algorithm() )
        {
          QgsProcessingContext *context = processingContext();
          if ( !context )
            return;

          bool ok = false;
          const QString command = alg->asQgisProcessCommand( createProcessingParameters(), *context, ok );
          if ( !ok )
          {
            mMessageBar->pushMessage( tr( "Current settings cannot be specified as arguments to qgis_process (Pipe parameters as JSON to qgis_process instead)" ), Qgis::MessageLevel::Warning );
          }
          else
          {
            QMimeData *m = new QMimeData();
            m->setText( command );
            QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
            cb->setMimeData( m, QClipboard::Selection );
#endif
            cb->setMimeData( m, QClipboard::Clipboard );
          }
        }
      } );

      mAdvancedMenu->addSeparator();

      QAction *copyAsJson = new QAction( tr( "Copy as JSON" ), mAdvancedMenu );
      copyAsJson->setIcon( QgsApplication::getThemeIcon( u"mActionEditCopy.svg"_s ) );

      mAdvancedMenu->addAction( copyAsJson );
      connect( copyAsJson, &QAction::triggered, this, [this] {
        if ( const QgsProcessingAlgorithm *alg = algorithm() )
        {
          QgsProcessingContext *context = processingContext();
          if ( !context )
            return;

          const QVariantMap properties = alg->asMap( createProcessingParameters(), *context );
          const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( properties ).dump( 2 ) );

          QMimeData *m = new QMimeData();
          m->setText( json );
          QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
          cb->setMimeData( m, QClipboard::Selection );
#endif
          cb->setMimeData( m, QClipboard::Clipboard );
        }
      } );

      mPasteJsonAction = new QAction( tr( "Paste Settings" ), mAdvancedMenu );
      mPasteJsonAction->setIcon( QgsApplication::getThemeIcon( u"mActionEditPaste.svg"_s ) );

      mAdvancedMenu->addAction( mPasteJsonAction );
      connect( mPasteJsonAction, &QAction::triggered, this, [this] {
        const QString text = QApplication::clipboard()->text();
        if ( text.isEmpty() )
          return;

        const QVariantMap parameterValues = QgsJsonUtils::parseJson( text ).toMap().value( u"inputs"_s ).toMap();
        if ( parameterValues.isEmpty() )
          return;

        bool ok = false;
        QString error;
        const QVariantMap preparedValues = QgsProcessingUtils::preprocessQgisProcessParameters( parameterValues, ok, error );

        setParameters( preparedValues );
      } );

      mButtonBox->addButton( mAdvancedButton, QDialogButtonBox::ResetRole );
      break;
    }

    case QgsProcessingAlgorithmWidgetBase::WidgetMode::Batch:
      break;
  }

  if ( mAdvancedMenu )
  {
    connect( mAdvancedMenu, &QMenu::aboutToShow, this, [this] {
      mCopyAsQgisProcessCommand->setEnabled( algorithm() && !( algorithm()->flags() & Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool ) );
      mPasteJsonAction->setEnabled( !QApplication::clipboard()->text().isEmpty() );
    } );
  }

  connect( mButtonRun, &QPushButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::runAlgorithm );
  connect( mButtonChangeParameters, &QPushButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::showParameters );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsProcessingAlgorithmWidgetBase::closeClicked );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProcessingAlgorithmWidgetBase::openHelp );
  connect( mButtonCollapse, &QToolButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::toggleCollapsed );
  connect( splitter, &QSplitter::splitterMoved, this, &QgsProcessingAlgorithmWidgetBase::splitterChanged );

  connect( mButtonSaveLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::saveLog );
  connect( mButtonCopyLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::copyLogToClipboard );
  connect( mButtonClearLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::clearLog );

  connect( buttonCancel, &QPushButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::cancelRequested );

  connect( mTabWidget, &QTabWidget::currentChanged, this, &QgsProcessingAlgorithmWidgetBase::mTabWidget_currentChanged );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  verticalLayout->insertWidget( 0, mMessageBar );

  connect( QgsApplication::taskManager(), &QgsTaskManager::taskTriggered, this, &QgsProcessingAlgorithmWidgetBase::taskTriggered );
}

QgsProcessingAlgorithmWidgetBase::~QgsProcessingAlgorithmWidgetBase() = default;

void QgsProcessingAlgorithmWidgetBase::setParameters( const QVariantMap & )
{}

void QgsProcessingAlgorithmWidgetBase::setAlgorithm( QgsProcessingAlgorithm *algorithm )
{
  mAlgorithm.reset( algorithm );
  QString title;
  if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) && !( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::DisplayNameIsLiteral ) )
  {
    title = mAlgorithm->group().isEmpty()
              ? QgsStringUtils::capitalize( mAlgorithm->displayName(), Qgis::Capitalization::TitleCase )
              : u"%1 - %2"_s.arg( QgsStringUtils::capitalize( mAlgorithm->group(), Qgis::Capitalization::TitleCase ), QgsStringUtils::capitalize( mAlgorithm->displayName(), Qgis::Capitalization::TitleCase ) );
  }
  else
  {
    title = mAlgorithm->group().isEmpty() ? mAlgorithm->displayName() : u"%1 - %2"_s.arg( mAlgorithm->group(), mAlgorithm->displayName() );
  }

  setWindowTitle( title );

  const QString algHelp = formatHelp( algorithm );
  if ( algHelp.isEmpty() )
    textShortHelp->hide();
  else
  {
    textShortHelp->document()->setDefaultStyleSheet( QStringLiteral(
      ".summary { margin-left: 10px; margin-right: 10px; }\n"
      "h2 { color: #555555; padding-bottom: 15px; }\n"
      "a { text - decoration: none; color: #3498db; font-weight: bold; }\n"
      "p, ul, li { color: #666666; }\n"
      "b { color: #333333; }\n"
      "dl dd { margin - bottom: 5px; }"
    ) );
    textShortHelp->setHtml( algHelp );
    connect( textShortHelp, &QTextBrowser::anchorClicked, this, &QgsProcessingAlgorithmWidgetBase::linkClicked );
    textShortHelp->show();
  }

  if ( algorithm->helpUrl().isEmpty() && ( !algorithm->provider() || algorithm->provider()->helpId().isEmpty() ) )
  {
    mButtonBox->removeButton( mButtonBox->button( QDialogButtonBox::Help ) );
  }

  const QString warning = algorithm->provider() ? algorithm->provider()->warningMessage() : QString();
  if ( !warning.isEmpty() )
  {
    mMessageBar->pushMessage( warning, Qgis::MessageLevel::Warning );
  }
}

QgsProcessingAlgorithm *QgsProcessingAlgorithmWidgetBase::algorithm()
{
  return mAlgorithm.get();
}

void QgsProcessingAlgorithmWidgetBase::setMainWidget( QgsPanelWidget *widget )
{
  if ( mMainWidget )
  {
    mMainWidget->deleteLater();
  }

  mPanelStack->setMainPanel( widget );
  widget->setDockMode( true );

  mMainWidget = widget;
  connect( mMainWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );
}

QgsPanelWidget *QgsProcessingAlgorithmWidgetBase::mainWidget()
{
  return mMainWidget;
}

void QgsProcessingAlgorithmWidgetBase::saveLogToFile( const QString &path, const LogFormat format )
{
  QFile logFile( path );
  if ( !logFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    return;
  }
  QTextStream fout( &logFile );

  switch ( format )
  {
    case QgsProcessingAlgorithmWidgetBase::LogFormat::FormatPlainText:
      fout << txtLog->toPlainText();
      break;

    case QgsProcessingAlgorithmWidgetBase::LogFormat::FormatHtml:
      fout << txtLog->toHtml();
      break;
  }
}

QgsProcessingFeedback *QgsProcessingAlgorithmWidgetBase::createFeedback()
{
  auto feedback = std::make_unique<QgsProcessingAlgorithmDialogFeedback>();
  connect( feedback.get(), &QgsProcessingFeedback::progressChanged, this, &QgsProcessingAlgorithmWidgetBase::setPercentage );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::commandInfoPushed, this, &QgsProcessingAlgorithmWidgetBase::pushCommandInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::consoleInfoPushed, this, &QgsProcessingAlgorithmWidgetBase::pushConsoleInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::debugInfoPushed, this, &QgsProcessingAlgorithmWidgetBase::pushDebugInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::errorReported, this, &QgsProcessingAlgorithmWidgetBase::reportError );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::warningPushed, this, &QgsProcessingAlgorithmWidgetBase::pushWarning );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::infoPushed, this, &QgsProcessingAlgorithmWidgetBase::pushInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::formattedMessagePushed, this, &QgsProcessingAlgorithmWidgetBase::pushFormattedMessage );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::progressTextChanged, this, &QgsProcessingAlgorithmWidgetBase::setProgressText );
  connect( this, &QgsProcessingAlgorithmWidgetBase::cancelRequested, feedback.get(), &QgsProcessingFeedback::cancel );
  return feedback.release();
}

QDialogButtonBox *QgsProcessingAlgorithmWidgetBase::buttonBox()
{
  return mButtonBox;
}

QTabWidget *QgsProcessingAlgorithmWidgetBase::tabWidget()
{
  return mTabWidget;
}

void QgsProcessingAlgorithmWidgetBase::showLog()
{
  mTabWidget->setCurrentIndex( 1 );
}

void QgsProcessingAlgorithmWidgetBase::showParameters()
{
  mTabWidget->setCurrentIndex( 0 );
}

QPushButton *QgsProcessingAlgorithmWidgetBase::runButton()
{
  return mButtonRun;
}

QPushButton *QgsProcessingAlgorithmWidgetBase::cancelButton()
{
  return buttonCancel;
}

QPushButton *QgsProcessingAlgorithmWidgetBase::changeParametersButton()
{
  return mButtonChangeParameters;
}

void QgsProcessingAlgorithmWidgetBase::clearProgress()
{
  progressBar->setMaximum( 0 );
}

void QgsProcessingAlgorithmWidgetBase::setExecuted( bool executed )
{
  mExecuted = executed;
}

void QgsProcessingAlgorithmWidgetBase::setExecutedAnyResult( bool executedAnyResult )
{
  mExecutedAnyResult = executedAnyResult;
}

void QgsProcessingAlgorithmWidgetBase::setResults( const QVariantMap &results )
{
  mResults = results;
}

void QgsProcessingAlgorithmWidgetBase::finished( bool, const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{}

void QgsProcessingAlgorithmWidgetBase::openHelp()
{
  QUrl algHelp = mAlgorithm->helpUrl();
  if ( algHelp.isEmpty() && mAlgorithm->provider() )
  {
    algHelp = QgsHelp::helpUrl(
      u"processing_algs/%1/%2.html#%3"_s.arg( mAlgorithm->provider()->helpId(), mAlgorithm->groupId(), u"%1%2"_s.arg( mAlgorithm->provider()->helpId() ).arg( mAlgorithm->name().replace( "_", "-" ) ) )
    );
  }

  if ( !algHelp.isEmpty() )
    QDesktopServices::openUrl( algHelp );
}

void QgsProcessingAlgorithmWidgetBase::toggleCollapsed()
{
  if ( mHelpCollapsed )
  {
    splitter->restoreState( mSplitterState );
    mButtonCollapse->setArrowType( Qt::RightArrow );
  }
  else
  {
    mSplitterState = splitter->saveState();
    splitter->setSizes( QList<int>() << 1 << 0 );
    mButtonCollapse->setArrowType( Qt::LeftArrow );
  }
  mHelpCollapsed = !mHelpCollapsed;
}

void QgsProcessingAlgorithmWidgetBase::splitterChanged( int, int )
{
  if ( splitter->sizes().at( 1 ) == 0 )
  {
    mHelpCollapsed = true;
    mButtonCollapse->setArrowType( Qt::LeftArrow );
  }
  else
  {
    mHelpCollapsed = false;
    mButtonCollapse->setArrowType( Qt::RightArrow );
  }
}

void QgsProcessingAlgorithmWidgetBase::mTabWidget_currentChanged( int )
{
  updateRunButtonVisibility();
}

void QgsProcessingAlgorithmWidgetBase::linkClicked( const QUrl &url )
{
  QDesktopServices::openUrl( url.toString() );
}

void QgsProcessingAlgorithmWidgetBase::algExecuted( bool successful, const QVariantMap & )
{
  mAlgorithmTask = nullptr;

  if ( !successful )
  {
    // show widget to display errors
    showWidget();
    showLog();
  }
  else
  {
    if ( isFinalized() && successful )
    {
      progressBar->setFormat( tr( "Complete" ) );
    }

    // delete widget if closed
    if ( isFinalized() && !isVisible() )
    {
      deleteLater();
    }
  }
}

void QgsProcessingAlgorithmWidgetBase::taskTriggered( QgsTask *task )
{
  if ( task == mAlgorithmTask )
  {
    showWidget();
    showLog();
  }
}

void QgsProcessingAlgorithmWidgetBase::showWidget()
{
  show();
  raise();
  setWindowState( ( windowState() & ~Qt::WindowMinimized ) | Qt::WindowActive );
  activateWindow();
}

void QgsProcessingAlgorithmWidgetBase::closeClicked()
{
  reject();
  close();
}

void QgsProcessingAlgorithmWidgetBase::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}


Qgis::ProcessingLogLevel QgsProcessingAlgorithmWidgetBase::logLevel() const
{
  return mLogLevel;
}

void QgsProcessingAlgorithmWidgetBase::setLogLevel( Qgis::ProcessingLogLevel level )
{
  mLogLevel = level;
}

void QgsProcessingAlgorithmWidgetBase::reportError( const QString &error, bool fatalError )
{
  setInfo( error, true );
  if ( fatalError )
    resetGui();
  showLog();
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::pushWarning( const QString &warning )
{
  setInfo( warning, false, true, true );
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::pushInfo( const QString &info )
{
  setInfo( info );
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::pushFormattedMessage( const QString &html )
{
  setInfo( html, false, false );
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::pushCommandInfo( const QString &command )
{
  txtLog->append( u"<code>%1<code>"_s.arg( formatStringForLog( command.toHtmlEscaped() ) ) );
  scrollToBottomOfLog();
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::pushDebugInfo( const QString &message )
{
  txtLog->append( u"<span style=\"color:#777\">%1</span>"_s.arg( formatStringForLog( message.toHtmlEscaped() ) ) );
  scrollToBottomOfLog();
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::pushConsoleInfo( const QString &info )
{
  txtLog->append( u"<code style=\"color:#777\">%1</code>"_s.arg( formatStringForLog( info.toHtmlEscaped() ) ) );
  scrollToBottomOfLog();
  processEvents();
}

QDialog *QgsProcessingAlgorithmWidgetBase::createProgressDialog()
{
  QgsProcessingAlgorithmProgressDialog *dialog = new QgsProcessingAlgorithmProgressDialog( this );
  dialog->setWindowModality( Qt::ApplicationModal );
  dialog->setWindowTitle( windowTitle() );
  dialog->setGeometry( geometry() ); // match size/position to this dialog
  connect( progressBar, &QProgressBar::valueChanged, dialog->progressBar(), &QProgressBar::setValue );
  connect( dialog->cancelButton(), &QPushButton::clicked, this, &QgsProcessingAlgorithmWidgetBase::cancelRequested );
  dialog->logTextEdit()->setHtml( txtLog->toHtml() );
  connect( txtLog, &QTextEdit::textChanged, dialog, [this, dialog]() {
    dialog->logTextEdit()->setHtml( txtLog->toHtml() );
    QScrollBar *sb = dialog->logTextEdit()->verticalScrollBar();
    sb->setValue( sb->maximum() );
  } );
  return dialog;
}

void QgsProcessingAlgorithmWidgetBase::clearLog()
{
  txtLog->clear();
}

void QgsProcessingAlgorithmWidgetBase::saveLog()
{
  QgsSettings settings;
  const QString lastUsedDir = settings.value( u"/Processing/lastUsedLogDirectory"_s, QDir::homePath() ).toString();

  QString filter;
  const QString txtExt = tr( "Text files" ) + u" (*.txt *.TXT)"_s;
  const QString htmlExt = tr( "HTML files" ) + u" (*.html *.HTML)"_s;

  const QString path = QFileDialog::getSaveFileName( this, tr( "Save Log to File" ), lastUsedDir, txtExt + ";;" + htmlExt, &filter );
  // return dialog focus on Mac
  activateWindow();
  raise();
  if ( path.isEmpty() )
  {
    return;
  }

  settings.setValue( u"/Processing/lastUsedLogDirectory"_s, QFileInfo( path ).path() );

  LogFormat format = QgsProcessingAlgorithmWidgetBase::LogFormat::FormatPlainText;
  if ( filter == htmlExt )
  {
    format = QgsProcessingAlgorithmWidgetBase::LogFormat::FormatHtml;
  }
  saveLogToFile( path, format );
}

void QgsProcessingAlgorithmWidgetBase::copyLogToClipboard()
{
  QMimeData *m = new QMimeData();
  m->setText( txtLog->toPlainText() );
  m->setHtml( txtLog->toHtml() );
  QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
  cb->setMimeData( m, QClipboard::Selection );
#endif
  cb->setMimeData( m, QClipboard::Clipboard );
}

void QgsProcessingAlgorithmWidgetBase::closeEvent( QCloseEvent *e )
{
  if ( !mHelpCollapsed )
  {
    QgsSettings settings;
    settings.setValue( u"/Processing/dialogBaseSplitter"_s, splitter->saveState() );
  }

  QDialog::closeEvent( e );

  if ( !mAlgorithmTask && isFinalized() )
  {
    // when running a background task, the dialog is kept around and deleted only when the task
    // completes. But if not running a task, we auto cleanup (later - gotta give callers a chance
    // to retrieve results and execution status).
    deleteLater();
  }
}

void QgsProcessingAlgorithmWidgetBase::runAlgorithm()
{}

void QgsProcessingAlgorithmWidgetBase::setPercentage( double percent )
{
  // delay setting maximum progress value until we know algorithm reports progress
  if ( progressBar->maximum() == 0 )
    progressBar->setMaximum( 100 );
  progressBar->setValue( percent );
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::setProgressText( const QString &text )
{
  lblProgress->setText( text );
  setInfo( text, false );
  scrollToBottomOfLog();
  processEvents();
}

QString QgsProcessingAlgorithmWidgetBase::formatHelp( QgsProcessingAlgorithm *algorithm )
{
  QString result;
  const QString text = algorithm->shortHelpString();
  if ( !text.isEmpty() )
  {
    const QStringList paragraphs = text.split( '\n' );
    QString help;
    for ( const QString &paragraph : paragraphs )
    {
      help += u"<p>%1</p>"_s.arg( paragraph );
    }
    result = u"<h2>%1</h2>%2"_s.arg( algorithm->displayName(), help );
  }
  else if ( !algorithm->shortDescription().isEmpty() )
  {
    result = u"<h2>%1</h2><p>%2</p>"_s.arg( algorithm->displayName(), algorithm->shortDescription() );
  }

  if ( algorithm->documentationFlags() != Qgis::ProcessingAlgorithmDocumentationFlags() )
  {
    QStringList flags;
    for ( Qgis::ProcessingAlgorithmDocumentationFlag flag : qgsEnumList<Qgis::ProcessingAlgorithmDocumentationFlag>() )
    {
      if ( algorithm->documentationFlags() & flag )
      {
        flags << QgsProcessing::documentationFlagToString( flag );
      }
    }
    result += u"<ul><li><i>%1</i></li></ul>"_s.arg( flags.join( "</i></li><li><i>"_L1 ) );
  }
  if ( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::SecurityRisk )
  {
    result += u"<p><b>%1</b></p>"_s.arg( tr( "Warning: This algorithm is a potential security risk if executed with unchecked inputs, and may result in system damage or data leaks." ) );
  }
  if ( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::KnownIssues )
  {
    result += u"<p><b>%1</b></p>"_s.arg( tr( "Warning: This algorithm has known issues. The results must be carefully validated by the user." ) );
  }

  return result;
}

void QgsProcessingAlgorithmWidgetBase::processEvents()
{
  if ( mAlgorithmTask )
  {
    // no need to call this - the algorithm is running in a thread.
    // in fact, calling it causes a crash on Windows when the algorithm
    // is running in a background thread... unfortunately we need something
    // like this for non-threadable algorithms, otherwise there's no chance
    // for users to hit cancel or see progress updates...
    return;
  }

  // So that we get a chance of hitting the Abort button
#ifdef Q_OS_LINUX
  // One iteration is actually enough on Windows to get good interactivity
  // whereas on Linux we must allow for far more iterations.
  // For safety limit the number of iterations
  int nIters = 0;
  while ( ++nIters < 100 )
#endif
  {
    QCoreApplication::processEvents();
  }
}

void QgsProcessingAlgorithmWidgetBase::scrollToBottomOfLog()
{
  QScrollBar *sb = txtLog->verticalScrollBar();
  sb->setValue( sb->maximum() );
}

void QgsProcessingAlgorithmWidgetBase::resetGui()
{
  lblProgress->clear();
  progressBar->setMaximum( 100 );
  progressBar->setValue( 0 );
  mButtonRun->setEnabled( true );
  mButtonChangeParameters->setEnabled( true );
  mButtonClose->setEnabled( true );
  if ( mMainWidget )
  {
    mMainWidget->setEnabled( true );
  }
  updateRunButtonVisibility();
  resetAdditionalGui();
}

void QgsProcessingAlgorithmWidgetBase::updateRunButtonVisibility()
{
  // Activate run button if current tab is Parameters
  const bool runButtonVisible = mTabWidget->currentIndex() == 0;
  mButtonRun->setVisible( runButtonVisible );
  if ( runButtonVisible )
    progressBar->resetFormat();
  mButtonChangeParameters->setVisible( !runButtonVisible && mExecutedAnyResult && mButtonChangeParameters->isEnabled() );
}

void QgsProcessingAlgorithmWidgetBase::resetAdditionalGui()
{}

void QgsProcessingAlgorithmWidgetBase::blockControlsWhileRunning()
{
  mButtonRun->setEnabled( false );
  mButtonChangeParameters->setEnabled( false );
  if ( mMainWidget )
  {
    mMainWidget->setEnabled( false );
  }
  blockAdditionalControlsWhileRunning();
}

void QgsProcessingAlgorithmWidgetBase::blockAdditionalControlsWhileRunning()
{}

QgsMessageBar *QgsProcessingAlgorithmWidgetBase::messageBar()
{
  return mMessageBar;
}

void QgsProcessingAlgorithmWidgetBase::hideShortHelp()
{
  textShortHelp->setVisible( false );
}

void QgsProcessingAlgorithmWidgetBase::setCurrentTask( QgsProcessingAlgRunnerTask *task )
{
  mAlgorithmTask = task;
  connect( mAlgorithmTask, &QgsProcessingAlgRunnerTask::executed, this, &QgsProcessingAlgorithmWidgetBase::algExecuted );
  QgsApplication::taskManager()->addTask( mAlgorithmTask );
}

void QgsProcessingAlgorithmWidgetBase::disconnectCurrentTask()
{
  if ( mAlgorithmTask )
  {
    disconnect( mAlgorithmTask, &QgsProcessingAlgRunnerTask::executed, this, &QgsProcessingAlgorithmWidgetBase::algExecuted );
    mAlgorithmTask = nullptr;
  }
}

QString QgsProcessingAlgorithmWidgetBase::formatStringForLog( const QString &string )
{
  QString s = string;
  s.replace( '\n', "<br>"_L1 );
  return s;
}

bool QgsProcessingAlgorithmWidgetBase::isFinalized()
{
  return true;
}

bool QgsProcessingAlgorithmWidgetBase::isRunning()
{
  return false;
}

void QgsProcessingAlgorithmWidgetBase::cancel()
{
  emit cancelRequested();
}

void QgsProcessingAlgorithmWidgetBase::applyContextOverrides( QgsProcessingContext *context )
{
  if ( !context )
    return;

  context->setLogLevel( logLevel() );

  if ( mOverrideDefaultContextSettings )
  {
    context->setInvalidGeometryCheck( mGeometryCheck );
    context->setDistanceUnit( mDistanceUnits );
    context->setAreaUnit( mAreaUnits );
    context->setTemporaryFolder( mTemporaryFolderOverride );
    context->setMaximumThreads( mMaximumThreads );
  }
}

void QgsProcessingAlgorithmWidgetBase::setInfo( const QString &message, bool isError, bool escapeHtml, bool isWarning )
{
  constexpr int MESSAGE_COUNT_LIMIT = 10000;
  // Avoid logging too many messages, which might blow memory.
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    return;
  ++mMessageLoggedCount;

  // note -- we have to wrap the message in a span block, or QTextEdit::append sometimes gets confused
  // and varies between treating it as a HTML string or a plain text string! (see https://github.com/qgis/QGIS/issues/37934)
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    txtLog->append( u"<span style=\"color:red\">%1</span>"_s.arg( tr( "Message log truncated" ) ) );
  else if ( isError || isWarning )
    txtLog->append( u"<span style=\"color:%1\">%2</span>"_s.arg( isError ? u"red"_s : u"#b85a20"_s, escapeHtml ? formatStringForLog( message.toHtmlEscaped() ) : formatStringForLog( message ) ) );
  else if ( escapeHtml )
    txtLog->append( u"<span>%1</span"_s.arg( formatStringForLog( message.toHtmlEscaped() ) ) );
  else
    txtLog->append( u"<span>%1</span>"_s.arg( formatStringForLog( message ) ) );
  scrollToBottomOfLog();
  processEvents();
}

void QgsProcessingAlgorithmWidgetBase::reject()
{
  if ( !mAlgorithmTask && isFinalized() )
  {
    setAttribute( Qt::WA_DeleteOnClose );
  }
  QDialog::reject();
}

void QgsProcessingAlgorithmWidgetBase::forceClose()
{
  disconnectCurrentTask();
  close();
}

//
// QgsProcessingAlgorithmProgressDialog
//

QgsProcessingAlgorithmProgressDialog::QgsProcessingAlgorithmProgressDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
}

QProgressBar *QgsProcessingAlgorithmProgressDialog::progressBar()
{
  return mProgressBar;
}

QPushButton *QgsProcessingAlgorithmProgressDialog::cancelButton()
{
  return mButtonBox->button( QDialogButtonBox::Cancel );
}

QTextEdit *QgsProcessingAlgorithmProgressDialog::logTextEdit()
{
  return mTxtLog;
}

void QgsProcessingAlgorithmProgressDialog::reject()
{}


//
// QgsProcessingContextOptionsWidget
//

QgsProcessingContextOptionsWidget::QgsProcessingContextOptionsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Algorithm Settings" ) );

  mComboInvalidFeatureFiltering->addItem( tr( "Do not Filter (Better Performance)" ), QVariant::fromValue( Qgis::InvalidGeometryCheck::NoCheck ) );
  mComboInvalidFeatureFiltering->addItem( tr( "Skip (Ignore) Features with Invalid Geometries" ), QVariant::fromValue( Qgis::InvalidGeometryCheck::SkipInvalid ) );
  mComboInvalidFeatureFiltering->addItem( tr( "Stop Algorithm Execution When a Geometry is Invalid" ), QVariant::fromValue( Qgis::InvalidGeometryCheck::AbortOnInvalid ) );

  mTemporaryFolderWidget->setDialogTitle( tr( "Select Temporary Directory" ) );
  mTemporaryFolderWidget->setStorageMode( QgsFileWidget::GetDirectory );
  mTemporaryFolderWidget->lineEdit()->setPlaceholderText( tr( "Default" ) );

  mLogLevelComboBox->addItem( tr( "Default" ), static_cast<int>( Qgis::ProcessingLogLevel::DefaultLevel ) );
  mLogLevelComboBox->addItem( tr( "Verbose" ), static_cast<int>( Qgis::ProcessingLogLevel::Verbose ) );
  mLogLevelComboBox->addItem( tr( "Verbose (Model Debugging)" ), static_cast<int>( Qgis::ProcessingLogLevel::ModelDebug ) );

  mDistanceUnitsCombo->addItem( tr( "Default" ), QVariant::fromValue( Qgis::DistanceUnit::Unknown ) );
  for ( Qgis::DistanceUnit unit : {
          Qgis::DistanceUnit::Meters,
          Qgis::DistanceUnit::Kilometers,
          Qgis::DistanceUnit::Centimeters,
          Qgis::DistanceUnit::Millimeters,
          Qgis::DistanceUnit::Feet,
          Qgis::DistanceUnit::Miles,
          Qgis::DistanceUnit::NauticalMiles,
          Qgis::DistanceUnit::Yards,
          Qgis::DistanceUnit::Inches,
          Qgis::DistanceUnit::Degrees,
        } )
  {
    QString title;
    if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) )
    {
      title = QgsStringUtils::capitalize( QgsUnitTypes::toString( unit ), Qgis::Capitalization::TitleCase );
    }
    else
    {
      title = QgsUnitTypes::toString( unit );
    }

    mDistanceUnitsCombo->addItem( title, QVariant::fromValue( unit ) );
  }

  mAreaUnitsCombo->addItem( tr( "Default" ), QVariant::fromValue( Qgis::AreaUnit::Unknown ) );
  for ( Qgis::AreaUnit unit : {
          Qgis::AreaUnit::SquareMeters,
          Qgis::AreaUnit::Hectares,
          Qgis::AreaUnit::SquareKilometers,
          Qgis::AreaUnit::SquareCentimeters,
          Qgis::AreaUnit::SquareMillimeters,
          Qgis::AreaUnit::SquareFeet,
          Qgis::AreaUnit::SquareMiles,
          Qgis::AreaUnit::SquareNauticalMiles,
          Qgis::AreaUnit::SquareYards,
          Qgis::AreaUnit::SquareInches,
          Qgis::AreaUnit::Acres,
          Qgis::AreaUnit::SquareDegrees,
        } )
  {
    QString title;
    if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) )
    {
      title = QgsStringUtils::capitalize( QgsUnitTypes::toString( unit ), Qgis::Capitalization::TitleCase );
    }
    else
    {
      title = QgsUnitTypes::toString( unit );
    }

    mAreaUnitsCombo->addItem( title, QVariant::fromValue( unit ) );
  }

  mThreadsSpinBox->setRange( 1, QThread::idealThreadCount() );

  connect( mLogLevelComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mComboInvalidFeatureFiltering, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mDistanceUnitsCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mAreaUnitsCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mTemporaryFolderWidget, &QgsFileWidget::fileChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mThreadsSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
}

void QgsProcessingContextOptionsWidget::setFromContext( const QgsProcessingContext *context )
{
  whileBlocking( mComboInvalidFeatureFiltering )->setCurrentIndex( mComboInvalidFeatureFiltering->findData( QVariant::fromValue( context->invalidGeometryCheck() ) ) );
  whileBlocking( mDistanceUnitsCombo )->setCurrentIndex( mDistanceUnitsCombo->findData( QVariant::fromValue( context->distanceUnit() ) ) );
  whileBlocking( mAreaUnitsCombo )->setCurrentIndex( mAreaUnitsCombo->findData( QVariant::fromValue( context->areaUnit() ) ) );
  whileBlocking( mTemporaryFolderWidget )->setFilePath( context->temporaryFolder() );
  whileBlocking( mThreadsSpinBox )->setValue( context->maximumThreads() );
  whileBlocking( mLogLevelComboBox )->setCurrentIndex( mLogLevelComboBox->findData( static_cast<int>( context->logLevel() ) ) );
}

Qgis::InvalidGeometryCheck QgsProcessingContextOptionsWidget::invalidGeometryCheck() const
{
  return mComboInvalidFeatureFiltering->currentData().value<Qgis::InvalidGeometryCheck>();
}

Qgis::DistanceUnit QgsProcessingContextOptionsWidget::distanceUnit() const
{
  return mDistanceUnitsCombo->currentData().value<Qgis::DistanceUnit>();
}

Qgis::AreaUnit QgsProcessingContextOptionsWidget::areaUnit() const
{
  return mAreaUnitsCombo->currentData().value<Qgis::AreaUnit>();
}

QString QgsProcessingContextOptionsWidget::temporaryFolder()
{
  return mTemporaryFolderWidget->filePath();
}

int QgsProcessingContextOptionsWidget::maximumThreads() const
{
  return mThreadsSpinBox->value();
}

void QgsProcessingContextOptionsWidget::setLogLevel( Qgis::ProcessingLogLevel level )
{
  whileBlocking( mLogLevelComboBox )->setCurrentIndex( mLogLevelComboBox->findData( static_cast<int>( level ) ) );
}

Qgis::ProcessingLogLevel QgsProcessingContextOptionsWidget::logLevel() const
{
  return static_cast<Qgis::ProcessingLogLevel>( mLogLevelComboBox->currentData().toInt() );
}

///@endcond
