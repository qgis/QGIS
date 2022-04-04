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

#include "qgsprocessingalgorithmdialogbase.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsmessagebar.h"
#include "qgsgui.h"
#include "processing/qgsprocessingalgorithm.h"
#include "processing/qgsprocessingprovider.h"
#include "qgstaskmanager.h"
#include "processing/qgsprocessingalgrunnertask.h"
#include "qgsstringutils.h"
#include "qgsapplication.h"
#include "qgspanelwidget.h"
#include "qgsjsonutils.h"
#include <QToolButton>
#include <QDesktopServices>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMimeData>
#include <QMenu>
#include <nlohmann/json.hpp>


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

//
// QgsProcessingAlgorithmDialogBase
//

QgsProcessingAlgorithmDialogBase::QgsProcessingAlgorithmDialogBase( QWidget *parent, Qt::WindowFlags flags, DialogMode mode )
  : QDialog( parent, flags )
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

  const QgsSettings settings;
  splitter->restoreState( settings.value( QStringLiteral( "/Processing/dialogBaseSplitter" ), QByteArray() ).toByteArray() );
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
    case DialogMode::Single:
    {
      mAdvancedButton = new QPushButton( tr( "Advanced" ) );
      mAdvancedMenu = new QMenu( this );
      mAdvancedButton->setMenu( mAdvancedMenu );

      QAction *copyAsPythonCommand = new QAction( tr( "Copy as Python Command" ), mAdvancedMenu );
      copyAsPythonCommand->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconPythonFile.svg" ) ) );

      mAdvancedMenu->addAction( copyAsPythonCommand );
      connect( copyAsPythonCommand, &QAction::triggered, this, [this]
      {
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
      mCopyAsQgisProcessCommand->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionTerminal.svg" ) ) );
      mAdvancedMenu->addAction( mCopyAsQgisProcessCommand );

      connect( mCopyAsQgisProcessCommand, &QAction::triggered, this, [this]
      {
        if ( const QgsProcessingAlgorithm *alg = algorithm() )
        {
          QgsProcessingContext *context = processingContext();
          if ( !context )
            return;

          bool ok = false;
          const QString command = alg->asQgisProcessCommand( createProcessingParameters(), *context, ok );
          if ( ! ok )
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
      copyAsJson->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionEditCopy.svg" ) ) );

      mAdvancedMenu->addAction( copyAsJson );
      connect( copyAsJson, &QAction::triggered, this, [this]
      {
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
      mPasteJsonAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionEditPaste.svg" ) ) );

      mAdvancedMenu->addAction( mPasteJsonAction );
      connect( mPasteJsonAction, &QAction::triggered, this, [this]
      {
        const QString text = QApplication::clipboard()->text();
        if ( text.isEmpty() )
          return;

        const QVariantMap parameterValues = QgsJsonUtils::parseJson( text ).toMap().value( QStringLiteral( "inputs" ) ).toMap();
        if ( parameterValues.isEmpty() )
          return;

        setParameters( parameterValues );
      } );

      mButtonBox->addButton( mAdvancedButton, QDialogButtonBox::ResetRole );
      break;
    }

    case DialogMode::Batch:
      break;
  }

  if ( mAdvancedMenu )
  {
    connect( mAdvancedMenu, &QMenu::aboutToShow, this, [ = ]
    {
      mCopyAsQgisProcessCommand->setEnabled( algorithm()
                                             && !( algorithm()->flags() & QgsProcessingAlgorithm::FlagNotAvailableInStandaloneTool ) );
      mPasteJsonAction->setEnabled( !QApplication::clipboard()->text().isEmpty() );
    } );
  }

  connect( mButtonRun, &QPushButton::clicked, this, &QgsProcessingAlgorithmDialogBase::runAlgorithm );
  connect( mButtonChangeParameters, &QPushButton::clicked, this, &QgsProcessingAlgorithmDialogBase::showParameters );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsProcessingAlgorithmDialogBase::closeClicked );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProcessingAlgorithmDialogBase::openHelp );
  connect( mButtonCollapse, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::toggleCollapsed );
  connect( splitter, &QSplitter::splitterMoved, this, &QgsProcessingAlgorithmDialogBase::splitterChanged );

  connect( mButtonSaveLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::saveLog );
  connect( mButtonCopyLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::copyLogToClipboard );
  connect( mButtonClearLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::clearLog );

  connect( mTabWidget, &QTabWidget::currentChanged, this, &QgsProcessingAlgorithmDialogBase::mTabWidget_currentChanged );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  verticalLayout->insertWidget( 0,  mMessageBar );

  connect( QgsApplication::taskManager(), &QgsTaskManager::taskTriggered, this, &QgsProcessingAlgorithmDialogBase::taskTriggered );
}

QgsProcessingAlgorithmDialogBase::~QgsProcessingAlgorithmDialogBase() = default;

void QgsProcessingAlgorithmDialogBase::setParameters( const QVariantMap & )
{}

void QgsProcessingAlgorithmDialogBase::setAlgorithm( QgsProcessingAlgorithm *algorithm )
{
  mAlgorithm.reset( algorithm );
  QString title;
  if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) && !( algorithm->flags() & QgsProcessingAlgorithm::FlagDisplayNameIsLiteral ) )
  {
    title = QgsStringUtils::capitalize( mAlgorithm->displayName(), Qgis::Capitalization::TitleCase );
  }
  else
  {
    title = mAlgorithm->displayName();
  }
  setWindowTitle( title );

  const QString algHelp = formatHelp( algorithm );
  if ( algHelp.isEmpty() )
    textShortHelp->hide();
  else
  {
    textShortHelp->document()->setDefaultStyleSheet( QStringLiteral( ".summary { margin-left: 10px; margin-right: 10px; }\n"
        "h2 { color: #555555; padding-bottom: 15px; }\n"
        "a { text - decoration: none; color: #3498db; font-weight: bold; }\n"
        "p { color: #666666; }\n"
        "b { color: #333333; }\n"
        "dl dd { margin - bottom: 5px; }" ) );
    textShortHelp->setHtml( algHelp );
    connect( textShortHelp, &QTextBrowser::anchorClicked, this, &QgsProcessingAlgorithmDialogBase::linkClicked );
    textShortHelp->show();
  }

  if ( algorithm->helpUrl().isEmpty() && algorithm->provider()->helpId().isEmpty() )
  {
    mButtonBox->removeButton( mButtonBox->button( QDialogButtonBox::Help ) );
  }

  const QString warning = algorithm->provider()->warningMessage();
  if ( !warning.isEmpty() )
  {
    mMessageBar->pushMessage( warning, Qgis::MessageLevel::Warning );
  }
}

QgsProcessingAlgorithm *QgsProcessingAlgorithmDialogBase::algorithm()
{
  return mAlgorithm.get();
}

void QgsProcessingAlgorithmDialogBase::setMainWidget( QgsPanelWidget *widget )
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

QgsPanelWidget *QgsProcessingAlgorithmDialogBase::mainWidget()
{
  return mMainWidget;
}

void QgsProcessingAlgorithmDialogBase::saveLogToFile( const QString &path, const LogFormat format )
{
  QFile logFile( path );
  if ( !logFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    return;
  }
  QTextStream fout( &logFile );

  switch ( format )
  {
    case FormatPlainText:
      fout << txtLog->toPlainText();
      break;

    case FormatHtml:
      fout << txtLog->toHtml();
      break;
  }
}

QgsProcessingFeedback *QgsProcessingAlgorithmDialogBase::createFeedback()
{
  auto feedback = std::make_unique< QgsProcessingAlgorithmDialogFeedback >();
  connect( feedback.get(), &QgsProcessingFeedback::progressChanged, this, &QgsProcessingAlgorithmDialogBase::setPercentage );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::commandInfoPushed, this, &QgsProcessingAlgorithmDialogBase::pushCommandInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::consoleInfoPushed, this, &QgsProcessingAlgorithmDialogBase::pushConsoleInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::debugInfoPushed, this, &QgsProcessingAlgorithmDialogBase::pushDebugInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::errorReported, this, &QgsProcessingAlgorithmDialogBase::reportError );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::warningPushed, this, &QgsProcessingAlgorithmDialogBase::pushWarning );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::infoPushed, this, &QgsProcessingAlgorithmDialogBase::pushInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::progressTextChanged, this, &QgsProcessingAlgorithmDialogBase::setProgressText );
  connect( buttonCancel, &QPushButton::clicked, feedback.get(), &QgsProcessingFeedback::cancel );
  return feedback.release();
}

QDialogButtonBox *QgsProcessingAlgorithmDialogBase::buttonBox()
{
  return mButtonBox;
}

QTabWidget *QgsProcessingAlgorithmDialogBase::tabWidget()
{
  return mTabWidget;
}

void QgsProcessingAlgorithmDialogBase::showLog()
{
  mTabWidget->setCurrentIndex( 1 );
}

void QgsProcessingAlgorithmDialogBase::showParameters()
{
  mTabWidget->setCurrentIndex( 0 );
}

QPushButton *QgsProcessingAlgorithmDialogBase::runButton()
{
  return mButtonRun;
}

QPushButton *QgsProcessingAlgorithmDialogBase::cancelButton()
{
  return buttonCancel;
}

QPushButton *QgsProcessingAlgorithmDialogBase::changeParametersButton()
{
  return mButtonChangeParameters;
}

void QgsProcessingAlgorithmDialogBase::clearProgress()
{
  progressBar->setMaximum( 0 );
}

void QgsProcessingAlgorithmDialogBase::setExecuted( bool executed )
{
  mExecuted = executed;
}

void QgsProcessingAlgorithmDialogBase::setExecutedAnyResult( bool executedAnyResult )
{
  mExecutedAnyResult = executedAnyResult;
}

void QgsProcessingAlgorithmDialogBase::setResults( const QVariantMap &results )
{
  mResults = results;
}

void QgsProcessingAlgorithmDialogBase::finished( bool, const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{

}

void QgsProcessingAlgorithmDialogBase::openHelp()
{
  QUrl algHelp = mAlgorithm->helpUrl();
  if ( algHelp.isEmpty() )
  {
    algHelp = QgsHelp::helpUrl( QStringLiteral( "processing_algs/%1/%2.html#%3" ).arg( mAlgorithm->provider()->helpId(), mAlgorithm->groupId(), QStringLiteral( "%1%2" ).arg( mAlgorithm->provider()->helpId() ).arg( mAlgorithm->name() ) ) );
  }

  if ( !algHelp.isEmpty() )
    QDesktopServices::openUrl( algHelp );
}

void QgsProcessingAlgorithmDialogBase::toggleCollapsed()
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

void QgsProcessingAlgorithmDialogBase::splitterChanged( int, int )
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

void QgsProcessingAlgorithmDialogBase::mTabWidget_currentChanged( int )
{
  updateRunButtonVisibility();
}

void QgsProcessingAlgorithmDialogBase::linkClicked( const QUrl &url )
{
  QDesktopServices::openUrl( url.toString() );
}

void QgsProcessingAlgorithmDialogBase::algExecuted( bool successful, const QVariantMap & )
{
  mAlgorithmTask = nullptr;

  if ( !successful )
  {
    // show dialog to display errors
    show();
    raise();
    setWindowState( ( windowState() & ~Qt::WindowMinimized ) | Qt::WindowActive );
    activateWindow();
    showLog();
  }
  else
  {
    // delete dialog if closed
    if ( isFinalized() && !isVisible() )
    {
      deleteLater();
    }
  }
}

void QgsProcessingAlgorithmDialogBase::taskTriggered( QgsTask *task )
{
  if ( task == mAlgorithmTask )
  {
    show();
    raise();
    setWindowState( ( windowState() & ~Qt::WindowMinimized ) | Qt::WindowActive );
    activateWindow();
    showLog();
  }
}

void QgsProcessingAlgorithmDialogBase::closeClicked()
{
  reject();
  close();
}

QgsProcessingContext::LogLevel QgsProcessingAlgorithmDialogBase::logLevel() const
{
  return mLogLevel;
}

void QgsProcessingAlgorithmDialogBase::setLogLevel( QgsProcessingContext::LogLevel level )
{
  mLogLevel = level;
}

void QgsProcessingAlgorithmDialogBase::reportError( const QString &error, bool fatalError )
{
  setInfo( error, true );
  if ( fatalError )
    resetGui();
  showLog();
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::pushWarning( const QString &warning )
{
  setInfo( warning, false, true, true );
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::pushInfo( const QString &info )
{
  setInfo( info );
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::pushCommandInfo( const QString &command )
{
  txtLog->append( QStringLiteral( "<code>%1<code>" ).arg( formatStringForLog( command.toHtmlEscaped() ) ) );
  scrollToBottomOfLog();
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::pushDebugInfo( const QString &message )
{
  txtLog->append( QStringLiteral( "<span style=\"color:#777\">%1</span>" ).arg( formatStringForLog( message.toHtmlEscaped() ) ) );
  scrollToBottomOfLog();
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::pushConsoleInfo( const QString &info )
{
  txtLog->append( QStringLiteral( "<code style=\"color:#777\">%1</code>" ).arg( formatStringForLog( info.toHtmlEscaped() ) ) );
  scrollToBottomOfLog();
  processEvents();
}

QDialog *QgsProcessingAlgorithmDialogBase::createProgressDialog()
{
  QgsProcessingAlgorithmProgressDialog *dialog = new QgsProcessingAlgorithmProgressDialog( this );
  dialog->setWindowModality( Qt::ApplicationModal );
  dialog->setWindowTitle( windowTitle() );
  dialog->setGeometry( geometry() ); // match size/position to this dialog
  connect( progressBar, &QProgressBar::valueChanged, dialog->progressBar(), &QProgressBar::setValue );
  connect( dialog->cancelButton(), &QPushButton::clicked, buttonCancel, &QPushButton::click );
  dialog->logTextEdit()->setHtml( txtLog->toHtml() );
  connect( txtLog, &QTextEdit::textChanged, dialog, [this, dialog]()
  {
    dialog->logTextEdit()->setHtml( txtLog->toHtml() );
    QScrollBar *sb = dialog->logTextEdit()->verticalScrollBar();
    sb->setValue( sb->maximum() );
  } );
  return dialog;
}

void QgsProcessingAlgorithmDialogBase::clearLog()
{
  txtLog->clear();
}

void QgsProcessingAlgorithmDialogBase::saveLog()
{
  QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "/Processing/lastUsedLogDirectory" ), QDir::homePath() ).toString();

  QString filter;
  const QString txtExt = tr( "Text files" ) + QStringLiteral( " (*.txt *.TXT)" );
  const QString htmlExt = tr( "HTML files" ) + QStringLiteral( " (*.html *.HTML)" );

  const QString path = QFileDialog::getSaveFileName( this, tr( "Save Log to File" ), lastUsedDir, txtExt + ";;" + htmlExt, &filter );
  if ( path.isEmpty() )
  {
    return;
  }

  settings.setValue( QStringLiteral( "/Processing/lastUsedLogDirectory" ), QFileInfo( path ).path() );

  LogFormat format = FormatPlainText;
  if ( filter == htmlExt )
  {
    format = FormatHtml;
  }
  saveLogToFile( path, format );
}

void QgsProcessingAlgorithmDialogBase::copyLogToClipboard()
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

void QgsProcessingAlgorithmDialogBase::closeEvent( QCloseEvent *e )
{
  if ( !mHelpCollapsed )
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "/Processing/dialogBaseSplitter" ), splitter->saveState() );
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

void QgsProcessingAlgorithmDialogBase::runAlgorithm()
{

}

void QgsProcessingAlgorithmDialogBase::setPercentage( double percent )
{
  // delay setting maximum progress value until we know algorithm reports progress
  if ( progressBar->maximum() == 0 )
    progressBar->setMaximum( 100 );
  progressBar->setValue( percent );
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::setProgressText( const QString &text )
{
  lblProgress->setText( text );
  setInfo( text, false );
  scrollToBottomOfLog();
  processEvents();
}

QString QgsProcessingAlgorithmDialogBase::formatHelp( QgsProcessingAlgorithm *algorithm )
{
  const QString text = algorithm->shortHelpString();
  if ( !text.isEmpty() )
  {
    const QStringList paragraphs = text.split( '\n' );
    QString help;
    for ( const QString &paragraph : paragraphs )
    {
      help += QStringLiteral( "<p>%1</p>" ).arg( paragraph );
    }
    return QStringLiteral( "<h2>%1</h2>%2" ).arg( algorithm->displayName(), help );
  }
  else if ( !algorithm->shortDescription().isEmpty() )
  {
    return QStringLiteral( "<h2>%1</h2><p>%2</p>" ).arg( algorithm->displayName(), algorithm->shortDescription() );
  }
  else
    return QString();
}

void QgsProcessingAlgorithmDialogBase::processEvents()
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

void QgsProcessingAlgorithmDialogBase::scrollToBottomOfLog()
{
  QScrollBar *sb = txtLog->verticalScrollBar();
  sb->setValue( sb->maximum() );
}

void QgsProcessingAlgorithmDialogBase::resetGui()
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

void QgsProcessingAlgorithmDialogBase::updateRunButtonVisibility()
{
  // Activate run button if current tab is Parameters
  const bool runButtonVisible = mTabWidget->currentIndex() == 0;
  mButtonRun->setVisible( runButtonVisible );
  mButtonChangeParameters->setVisible( !runButtonVisible && mExecutedAnyResult && mButtonChangeParameters->isEnabled() );
}

void QgsProcessingAlgorithmDialogBase::resetAdditionalGui()
{

}

void QgsProcessingAlgorithmDialogBase::blockControlsWhileRunning()
{
  mButtonRun->setEnabled( false );
  mButtonChangeParameters->setEnabled( false );
  if ( mMainWidget )
  {
    mMainWidget->setEnabled( false );
  }
  blockAdditionalControlsWhileRunning();
}

void QgsProcessingAlgorithmDialogBase::blockAdditionalControlsWhileRunning()
{

}

QgsMessageBar *QgsProcessingAlgorithmDialogBase::messageBar()
{
  return mMessageBar;
}

void QgsProcessingAlgorithmDialogBase::hideShortHelp()
{
  textShortHelp->setVisible( false );
}

void QgsProcessingAlgorithmDialogBase::setCurrentTask( QgsProcessingAlgRunnerTask *task )
{
  mAlgorithmTask = task;
  connect( mAlgorithmTask, &QgsProcessingAlgRunnerTask::executed, this, &QgsProcessingAlgorithmDialogBase::algExecuted );
  QgsApplication::taskManager()->addTask( mAlgorithmTask );
}

QString QgsProcessingAlgorithmDialogBase::formatStringForLog( const QString &string )
{
  QString s = string;
  s.replace( '\n', QLatin1String( "<br>" ) );
  return s;
}

bool QgsProcessingAlgorithmDialogBase::isFinalized()
{
  return true;
}

void QgsProcessingAlgorithmDialogBase::setInfo( const QString &message, bool isError, bool escapeHtml, bool isWarning )
{
  constexpr int MESSAGE_COUNT_LIMIT = 10000;
  // Avoid logging too many messages, which might blow memory.
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    return;
  ++mMessageLoggedCount;

  // note -- we have to wrap the message in a span block, or QTextEdit::append sometimes gets confused
  // and varies between treating it as a HTML string or a plain text string! (see https://github.com/qgis/QGIS/issues/37934)
  if ( mMessageLoggedCount == MESSAGE_COUNT_LIMIT )
    txtLog->append( QStringLiteral( "<span style=\"color:red\">%1</span>" ).arg( tr( "Message log truncated" ) ) );
  else if ( isError || isWarning )
    txtLog->append( QStringLiteral( "<span style=\"color:%1\">%2</span>" ).arg( isError ? QStringLiteral( "red" ) : QStringLiteral( "#b85a20" ), escapeHtml ? formatStringForLog( message.toHtmlEscaped() ) : formatStringForLog( message ) ) );
  else if ( escapeHtml )
    txtLog->append( QStringLiteral( "<span>%1</span" ).arg( formatStringForLog( message.toHtmlEscaped() ) ) );
  else
    txtLog->append( QStringLiteral( "<span>%1</span>" ).arg( formatStringForLog( message ) ) );
  scrollToBottomOfLog();
  processEvents();
}

void QgsProcessingAlgorithmDialogBase::reject()
{
  if ( !mAlgorithmTask && isFinalized() )
  {
    setAttribute( Qt::WA_DeleteOnClose );
  }
  QDialog::reject();
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
{

}



///@endcond
