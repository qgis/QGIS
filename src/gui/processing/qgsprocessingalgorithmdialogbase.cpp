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
#include <QToolButton>
#include <QDesktopServices>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>


///@cond NOT_STABLE

void QgsProcessingAlgorithmDialogFeedback::setProgressText( const QString &text )
{
  emit progressTextChanged( text );
}

void QgsProcessingAlgorithmDialogFeedback::reportError( const QString &error, bool fatalError )
{
  emit errorReported( error, fatalError );
}

void QgsProcessingAlgorithmDialogFeedback::pushInfo( const QString &info )
{
  emit infoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushCommandInfo( const QString &info )
{
  emit commandInfoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushDebugInfo( const QString &info )
{
  emit debugInfoPushed( info );
}

void QgsProcessingAlgorithmDialogFeedback::pushConsoleInfo( const QString &info )
{
  emit consoleInfoPushed( info );
}

//
// QgsProcessingAlgorithmDialogBase
//

QgsProcessingAlgorithmDialogBase::QgsProcessingAlgorithmDialogBase( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
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

  QgsSettings settings;
  splitter->restoreState( settings.value( QStringLiteral( "/Processing/dialogBaseSplitter" ), QByteArray() ).toByteArray() );
  mSplitterState = splitter->saveState();
  splitterChanged( 0, 0 );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsProcessingAlgorithmDialogBase::closeClicked );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProcessingAlgorithmDialogBase::runAlgorithm );

  // Rename OK button to Run
  mButtonRun = mButtonBox->button( QDialogButtonBox::Ok );
  mButtonRun->setText( tr( "Run" ) );

  buttonCancel->setEnabled( false );
  mButtonClose = mButtonBox->button( QDialogButtonBox::Close );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProcessingAlgorithmDialogBase::openHelp );
  connect( mButtonCollapse, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::toggleCollapsed );
  connect( splitter, &QSplitter::splitterMoved, this, &QgsProcessingAlgorithmDialogBase::splitterChanged );

  connect( mButtonSaveLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::saveLog );
  connect( mButtonCopyLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::copyLogToClipboard );
  connect( mButtonClearLog, &QToolButton::clicked, this, &QgsProcessingAlgorithmDialogBase::clearLog );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  verticalLayout->insertWidget( 0,  mMessageBar );

  connect( QgsApplication::taskManager(), &QgsTaskManager::taskTriggered, this, &QgsProcessingAlgorithmDialogBase::taskTriggered );
}

QgsProcessingAlgorithmDialogBase::~QgsProcessingAlgorithmDialogBase() = default;

void QgsProcessingAlgorithmDialogBase::setAlgorithm( QgsProcessingAlgorithm *algorithm )
{
  mAlgorithm.reset( algorithm );
  QString title;
  if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) && !( algorithm->flags() & QgsProcessingAlgorithm::FlagDisplayNameIsLiteral ) )
  {
    title = QgsStringUtils::capitalize( mAlgorithm->displayName(), QgsStringUtils::TitleCase );
  }
  else
  {
    title = mAlgorithm->displayName();
  }
  setWindowTitle( title );

  QString algHelp = formatHelp( algorithm );
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
  }

  if ( algorithm->helpUrl().isEmpty() && algorithm->provider()->helpId().isEmpty() )
  {
    mButtonBox->removeButton( mButtonBox->button( QDialogButtonBox::Help ) );
  }
}

QgsProcessingAlgorithm *QgsProcessingAlgorithmDialogBase::algorithm()
{
  return mAlgorithm.get();
}

void QgsProcessingAlgorithmDialogBase::setMainWidget( QWidget *widget )
{
  if ( mMainWidget )
  {
    mMainWidget->deleteLater();
  }

  mMainWidget = widget;
  mTabWidget->widget( 0 )->layout()->addWidget( mMainWidget );
}

QWidget *QgsProcessingAlgorithmDialogBase::mainWidget()
{
  return mMainWidget;
}

QVariantMap QgsProcessingAlgorithmDialogBase::getParameterValues() const
{
  return QVariantMap();
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
  auto feedback = qgis::make_unique< QgsProcessingAlgorithmDialogFeedback >();
  connect( feedback.get(), &QgsProcessingFeedback::progressChanged, this, &QgsProcessingAlgorithmDialogBase::setPercentage );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::commandInfoPushed, this, &QgsProcessingAlgorithmDialogBase::pushCommandInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::consoleInfoPushed, this, &QgsProcessingAlgorithmDialogBase::pushConsoleInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::debugInfoPushed, this, &QgsProcessingAlgorithmDialogBase::pushDebugInfo );
  connect( feedback.get(), &QgsProcessingAlgorithmDialogFeedback::errorReported, this, &QgsProcessingAlgorithmDialogBase::reportError );
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

QPushButton *QgsProcessingAlgorithmDialogBase::runButton()
{
  return mButtonRun;
}

QPushButton *QgsProcessingAlgorithmDialogBase::cancelButton()
{
  return buttonCancel;
}

void QgsProcessingAlgorithmDialogBase::clearProgress()
{
  progressBar->setMaximum( 0 );
}

void QgsProcessingAlgorithmDialogBase::setExecuted( bool executed )
{
  mExecuted = executed;
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
    if ( !isVisible() )
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

void QgsProcessingAlgorithmDialogBase::reportError( const QString &error, bool fatalError )
{
  setInfo( error, true );
  if ( fatalError )
    resetGui();
  showLog();
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
  txtLog->append( QStringLiteral( "<span style=\"color:blue\">%1</span>" ).arg( formatStringForLog( message.toHtmlEscaped() ) ) );
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
  QString lastUsedDir = settings.value( QStringLiteral( "/Processing/lastUsedLogDirectory" ), QDir::homePath() ).toString();

  QString filter;
  const QString txtExt = tr( "Text files" ) + QStringLiteral( " (*.txt *.TXT)" );
  const QString htmlExt = tr( "HTML files" ) + QStringLiteral( " (*.html *.HTML)" );

  QString path = QFileDialog::getSaveFileName( this, tr( "Save Log to File" ), lastUsedDir, txtExt + ";;" + htmlExt, &filter );
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
  QDialog::closeEvent( e );

  if ( !mAlgorithmTask )
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
  QString text = algorithm->shortHelpString();
  if ( !text.isEmpty() )
  {
    QStringList paragraphs = text.split( '\n' );
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
  // For some reason on Windows hasPendingEvents() always return true,
  // but one iteration is actually enough on Windows to get good interactivity
  // whereas on Linux we must allow for far more iterations.
  // For safety limit the number of iterations
  int nIters = 0;
  while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
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
  mButtonClose->setEnabled( true );
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
  s.replace( '\n', QStringLiteral( "<br>" ) );
  return s;
}

void QgsProcessingAlgorithmDialogBase::setInfo( const QString &message, bool isError, bool escapeHtml )
{
  if ( isError )
    txtLog->append( QStringLiteral( "<span style=\"color:red\">%1</span>" ).arg( escapeHtml ? formatStringForLog( message.toHtmlEscaped() ) : formatStringForLog( message ) ) );
  else if ( escapeHtml )
    txtLog->append( formatStringForLog( message.toHtmlEscaped() ) );
  else
    txtLog->append( formatStringForLog( message ) );
  scrollToBottomOfLog();
  processEvents();
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
