/***************************************************************************
    qgscodeeditorwidget.cpp
     --------------------------------------
    Date                 : May 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditorwidget.h"
#include "moc_qgscodeeditorwidget.cpp"
#include "qgscodeeditor.h"
#include "qgsfilterlineedit.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgsmessagebar.h"
#include "qgsdecoratedscrollbar.h"
#include "qgscodeeditorpython.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsjsonutils.h"
#include "nlohmann/json.hpp"
#include "qgssettings.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QShortcut>
#include <QGridLayout>
#include <QDesktopServices>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QNetworkRequest>

QgsCodeEditorWidget::QgsCodeEditorWidget(
  QgsCodeEditor *editor,
  QgsMessageBar *messageBar,
  QWidget *parent
)
  : QgsPanelWidget( parent )
  , mEditor( editor )
  , mMessageBar( messageBar )
{
  Q_ASSERT( mEditor );

  mEditor->installEventFilter( this );
  installEventFilter( this );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->setSpacing( 0 );
  vl->addWidget( editor, 1 );

  if ( !mMessageBar )
  {
    QGridLayout *layout = new QGridLayout( mEditor );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addItem( new QSpacerItem( 20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding ), 1, 0, 1, 1 );

    mMessageBar = new QgsMessageBar();
    QSizePolicy sizePolicy = QSizePolicy( QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed );
    mMessageBar->setSizePolicy( sizePolicy );
    layout->addWidget( mMessageBar, 0, 0, 1, 1 );
  }

  mFindWidget = new QWidget();
  QGridLayout *layoutFind = new QGridLayout();
  layoutFind->setContentsMargins( 0, 2, 0, 0 );
  layoutFind->setSpacing( 1 );

  if ( !mEditor->isReadOnly() )
  {
    mShowReplaceBarButton = new QToolButton();
    mShowReplaceBarButton->setToolTip( tr( "Replace" ) );
    mShowReplaceBarButton->setCheckable( true );
    mShowReplaceBarButton->setAutoRaise( true );
    mShowReplaceBarButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionReplace.svg" ) ) );
    layoutFind->addWidget( mShowReplaceBarButton, 0, 0 );

    connect( mShowReplaceBarButton, &QCheckBox::toggled, this, &QgsCodeEditorWidget::setReplaceBarVisible );
  }

  mLineEditFind = new QgsFilterLineEdit();
  mLineEditFind->setShowSearchIcon( true );
  mLineEditFind->setPlaceholderText( tr( "Enter text to find…" ) );
  layoutFind->addWidget( mLineEditFind, 0, mShowReplaceBarButton ? 1 : 0 );

  mLineEditReplace = new QgsFilterLineEdit();
  mLineEditReplace->setShowSearchIcon( true );
  mLineEditReplace->setPlaceholderText( tr( "Replace…" ) );
  layoutFind->addWidget( mLineEditReplace, 1, mShowReplaceBarButton ? 1 : 0 );

  QHBoxLayout *findButtonLayout = new QHBoxLayout();
  findButtonLayout->setContentsMargins( 0, 0, 0, 0 );
  findButtonLayout->setSpacing( 1 );
  mCaseSensitiveButton = new QToolButton();
  mCaseSensitiveButton->setToolTip( tr( "Case Sensitive" ) );
  mCaseSensitiveButton->setCheckable( true );
  mCaseSensitiveButton->setAutoRaise( true );
  mCaseSensitiveButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchCaseSensitive.svg" ) ) );
  findButtonLayout->addWidget( mCaseSensitiveButton );

  mWholeWordButton = new QToolButton();
  mWholeWordButton->setToolTip( tr( "Whole Word" ) );
  mWholeWordButton->setCheckable( true );
  mWholeWordButton->setAutoRaise( true );
  mWholeWordButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchWholeWord.svg" ) ) );
  findButtonLayout->addWidget( mWholeWordButton );

  mRegexButton = new QToolButton();
  mRegexButton->setToolTip( tr( "Use Regular Expressions" ) );
  mRegexButton->setCheckable( true );
  mRegexButton->setAutoRaise( true );
  mRegexButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchRegex.svg" ) ) );
  findButtonLayout->addWidget( mRegexButton );

  mWrapAroundButton = new QToolButton();
  mWrapAroundButton->setToolTip( tr( "Wrap Around" ) );
  mWrapAroundButton->setCheckable( true );
  mWrapAroundButton->setAutoRaise( true );
  mWrapAroundButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchWrapAround.svg" ) ) );
  findButtonLayout->addWidget( mWrapAroundButton );

  mFindPrevButton = new QToolButton();
  mFindPrevButton->setEnabled( false );
  mFindPrevButton->setToolTip( tr( "Find Previous" ) );
  mFindPrevButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconSearchPrevEditorConsole.svg" ) ) );
  mFindPrevButton->setAutoRaise( true );
  findButtonLayout->addWidget( mFindPrevButton );

  mFindNextButton = new QToolButton();
  mFindNextButton->setEnabled( false );
  mFindNextButton->setToolTip( tr( "Find Next" ) );
  mFindNextButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconSearchNextEditorConsole.svg" ) ) );
  mFindNextButton->setAutoRaise( true );
  findButtonLayout->addWidget( mFindNextButton );

  connect( mLineEditFind, &QLineEdit::returnPressed, this, &QgsCodeEditorWidget::findNext );
  connect( mLineEditFind, &QLineEdit::textChanged, this, &QgsCodeEditorWidget::textSearchChanged );
  connect( mFindNextButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::findNext );
  connect( mFindPrevButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::findPrevious );
  connect( mCaseSensitiveButton, &QToolButton::toggled, this, &QgsCodeEditorWidget::updateSearch );
  connect( mWholeWordButton, &QToolButton::toggled, this, &QgsCodeEditorWidget::updateSearch );
  connect( mRegexButton, &QToolButton::toggled, this, &QgsCodeEditorWidget::updateSearch );
  connect( mWrapAroundButton, &QCheckBox::toggled, this, &QgsCodeEditorWidget::updateSearch );

  QShortcut *findShortcut = new QShortcut( QKeySequence::StandardKey::Find, mEditor );
  findShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
  connect( findShortcut, &QShortcut::activated, this, &QgsCodeEditorWidget::triggerFind );

  QShortcut *findNextShortcut = new QShortcut( QKeySequence::StandardKey::FindNext, this );
  findNextShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
  connect( findNextShortcut, &QShortcut::activated, this, &QgsCodeEditorWidget::findNext );

  QShortcut *findPreviousShortcut = new QShortcut( QKeySequence::StandardKey::FindPrevious, this );
  findPreviousShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
  connect( findPreviousShortcut, &QShortcut::activated, this, &QgsCodeEditorWidget::findPrevious );

  if ( !mEditor->isReadOnly() )
  {
    QShortcut *replaceShortcut = new QShortcut( QKeySequence::StandardKey::Replace, this );
    replaceShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
    connect( replaceShortcut, &QShortcut::activated, this, [=] {
      // shortcut toggles bar visibility
      const bool show = mLineEditReplace->isHidden();
      setReplaceBarVisible( show );

      // ensure search bar is also visible
      if ( show )
        showSearchBar();
    } );
  }

  // escape on editor hides the find bar
  QShortcut *closeFindShortcut = new QShortcut( Qt::Key::Key_Escape, this );
  closeFindShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
  connect( closeFindShortcut, &QShortcut::activated, this, [this] {
    hideSearchBar();
    mEditor->setFocus();
  } );

  layoutFind->addLayout( findButtonLayout, 0, mShowReplaceBarButton ? 2 : 1 );

  QHBoxLayout *replaceButtonLayout = new QHBoxLayout();
  replaceButtonLayout->setContentsMargins( 0, 0, 0, 0 );
  replaceButtonLayout->setSpacing( 1 );

  mReplaceButton = new QToolButton();
  mReplaceButton->setText( tr( "Replace" ) );
  mReplaceButton->setEnabled( false );
  connect( mReplaceButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::replace );
  replaceButtonLayout->addWidget( mReplaceButton );

  mReplaceAllButton = new QToolButton();
  mReplaceAllButton->setText( tr( "Replace All" ) );
  mReplaceAllButton->setEnabled( false );
  connect( mReplaceAllButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::replaceAll );
  replaceButtonLayout->addWidget( mReplaceAllButton );

  layoutFind->addLayout( replaceButtonLayout, 1, mShowReplaceBarButton ? 2 : 1 );

  QToolButton *closeFindButton = new QToolButton( this );
  closeFindButton->setToolTip( tr( "Close" ) );
  closeFindButton->setMinimumWidth( QgsGuiUtils::scaleIconSize( 44 ) );
  closeFindButton->setStyleSheet(
    "QToolButton { border:none; background-color: rgba(0, 0, 0, 0); }"
    "QToolButton::menu-button { border:none; background-color: rgba(0, 0, 0, 0); }"
  );
  closeFindButton->setCursor( Qt::PointingHandCursor );
  closeFindButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconClose.svg" ) ) );

  const int iconSize = std::max( 18.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.9 );
  closeFindButton->setIconSize( QSize( iconSize, iconSize ) );
  closeFindButton->setFixedSize( QSize( iconSize, iconSize ) );
  connect( closeFindButton, &QAbstractButton::clicked, this, [this] {
    hideSearchBar();
    mEditor->setFocus();
  } );
  layoutFind->addWidget( closeFindButton, 0, mShowReplaceBarButton ? 3 : 2 );

  layoutFind->setColumnStretch( mShowReplaceBarButton ? 1 : 0, 1 );

  mFindWidget->setLayout( layoutFind );
  vl->addWidget( mFindWidget );
  mFindWidget->hide();

  setReplaceBarVisible( false );

  setLayout( vl );

  mHighlightController = std::make_unique<QgsScrollBarHighlightController>();
  mHighlightController->setScrollArea( mEditor );
}

void QgsCodeEditorWidget::resizeEvent( QResizeEvent *event )
{
  QgsPanelWidget::resizeEvent( event );
  updateHighlightController();
}

void QgsCodeEditorWidget::showEvent( QShowEvent *event )
{
  QgsPanelWidget::showEvent( event );
  updateHighlightController();
}

bool QgsCodeEditorWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::FocusIn )
  {
    if ( !mFilePath.isEmpty() )
    {
      if ( !QFile::exists( mFilePath ) )
      {
        // file deleted externally
        if ( mMessageBar )
        {
          mMessageBar->pushCritical( QString(), tr( "The file <b>\"%1\"</b> has been deleted or is not accessible" ).arg( QDir::toNativeSeparators( mFilePath ) ) );
        }
      }
      else
      {
        const QFileInfo fi( mFilePath );
        if ( mLastModified != fi.lastModified() )
        {
          // TODO - we should give users a choice of how to react to this, eg "ignore changes"
          // note -- we intentionally don't call loadFile here -- we want this action to be undo-able
          QFile file( mFilePath );
          if ( file.open( QFile::ReadOnly ) )
          {
            int currentLine = -1, currentColumn = -1;
            if ( !mLastModified.isNull() )
            {
              mEditor->getCursorPosition( &currentLine, &currentColumn );
            }

            const QString content = file.readAll();

            // don't clear, instead perform undoable actions:
            mEditor->beginUndoAction();
            mEditor->selectAll();
            mEditor->removeSelectedText();
            mEditor->insert( content );
            mEditor->setModified( false );
            mEditor->recolor();
            mEditor->endUndoAction();

            mLastModified = fi.lastModified();
            if ( currentLine >= 0 && currentLine < mEditor->lines() )
            {
              mEditor->setCursorPosition( currentLine, currentColumn );
            }

            emit loadedExternalChanges();
          }
        }
      }
    }
  }
  return QgsPanelWidget::eventFilter( obj, event );
}

QgsCodeEditorWidget::~QgsCodeEditorWidget() = default;

bool QgsCodeEditorWidget::isSearchBarVisible() const
{
  return !mFindWidget->isHidden();
}

QgsMessageBar *QgsCodeEditorWidget::messageBar()
{
  return mMessageBar;
}

QgsScrollBarHighlightController *QgsCodeEditorWidget::scrollbarHighlightController()
{
  return mHighlightController.get();
}

void QgsCodeEditorWidget::addWarning( int lineNumber, const QString &warning )
{
  mEditor->addWarning( lineNumber, warning );

  mHighlightController->addHighlight(
    QgsScrollBarHighlight(
      HighlightCategory::Warning,
      lineNumber,
      QColor( 255, 0, 0 ),
      QgsScrollBarHighlight::Priority::HighestPriority
    )
  );
}

void QgsCodeEditorWidget::clearWarnings()
{
  mEditor->clearWarnings();

  mHighlightController->removeHighlights(
    HighlightCategory::Warning
  );
}

void QgsCodeEditorWidget::showSearchBar()
{
  addSearchHighlights();
  mFindWidget->show();

  if ( mEditor->isReadOnly() )
  {
    setReplaceBarVisible( false );
  }

  emit searchBarToggled( true );
}

void QgsCodeEditorWidget::hideSearchBar()
{
  clearSearchHighlights();
  mFindWidget->hide();
  emit searchBarToggled( false );
}

void QgsCodeEditorWidget::setSearchBarVisible( bool visible )
{
  if ( visible )
    showSearchBar();
  else
    hideSearchBar();
}

void QgsCodeEditorWidget::setReplaceBarVisible( bool visible )
{
  if ( visible )
  {
    mReplaceAllButton->show();
    mReplaceButton->show();
    mLineEditReplace->show();
  }
  else
  {
    mReplaceAllButton->hide();
    mReplaceButton->hide();
    mLineEditReplace->hide();
  }
  if ( mShowReplaceBarButton )
    mShowReplaceBarButton->setChecked( visible );
}

void QgsCodeEditorWidget::triggerFind()
{
  clearSearchHighlights();
  mLineEditFind->setFocus();
  if ( mEditor->hasSelectedText() )
  {
    mBlockSearching++;
    mLineEditFind->setText( mEditor->selectedText().trimmed() );
    mBlockSearching--;
  }
  mLineEditFind->selectAll();
  showSearchBar();
}

bool QgsCodeEditorWidget::loadFile( const QString &path )
{
  if ( !QFile::exists( path ) )
    return false;

  QFile file( path );
  if ( file.open( QFile::ReadOnly ) )
  {
    const QString content = file.readAll();
    mEditor->setText( content );
    setFilePath( path );
    mEditor->recolor();
    mEditor->setModified( false );
    mLastModified = QFileInfo( path ).lastModified();
    return true;
  }
  return false;
}

void QgsCodeEditorWidget::setFilePath( const QString &path )
{
  if ( mFilePath == path )
    return;

  mFilePath = path;
  mLastModified = QDateTime();

  emit filePathChanged( mFilePath );
}

bool QgsCodeEditorWidget::save( const QString &path )
{
  const QString filePath = !path.isEmpty() ? path : mFilePath;
  if ( !filePath.isEmpty() )
  {
    QFile file( filePath );
    if ( file.open( QFile::WriteOnly ) )
    {
      file.write( mEditor->text().toUtf8() );
      file.close();

      setFilePath( filePath );
      mEditor->setModified( false );
      mLastModified = QFileInfo( filePath ).lastModified();

      return true;
    }
  }
  return false;
}

bool QgsCodeEditorWidget::openInExternalEditor( int line, int column )
{
  if ( mFilePath.isEmpty() )
    return false;

  const QDir dir = QFileInfo( mFilePath ).dir();

  bool useFallback = true;

  QString externalEditorCommand;
  switch ( mEditor->language() )
  {
    case Qgis::ScriptLanguage::Python:
      externalEditorCommand = QgsCodeEditorPython::settingExternalPythonEditorCommand->value();
      break;

    case Qgis::ScriptLanguage::Css:
    case Qgis::ScriptLanguage::QgisExpression:
    case Qgis::ScriptLanguage::Html:
    case Qgis::ScriptLanguage::JavaScript:
    case Qgis::ScriptLanguage::Json:
    case Qgis::ScriptLanguage::R:
    case Qgis::ScriptLanguage::Sql:
    case Qgis::ScriptLanguage::Batch:
    case Qgis::ScriptLanguage::Bash:
    case Qgis::ScriptLanguage::Unknown:
      break;
  }

  int currentLine, currentColumn;
  mEditor->getCursorPosition( &currentLine, &currentColumn );
  if ( line < 0 )
    line = currentLine;
  if ( column < 0 )
    column = currentColumn;

  if ( !externalEditorCommand.isEmpty() )
  {
    externalEditorCommand = externalEditorCommand.replace( QLatin1String( "<file>" ), mFilePath );
    externalEditorCommand = externalEditorCommand.replace( QLatin1String( "<line>" ), QString::number( line + 1 ) );
    externalEditorCommand = externalEditorCommand.replace( QLatin1String( "<col>" ), QString::number( column + 1 ) );

    const QStringList commandParts = QProcess::splitCommand( externalEditorCommand );
    if ( QProcess::startDetached( commandParts.at( 0 ), commandParts.mid( 1 ), dir.absolutePath() ) )
    {
      return true;
    }
  }

  const QString editorCommand = qgetenv( "EDITOR" );
  if ( !editorCommand.isEmpty() )
  {
    const QFileInfo fi( editorCommand );
    if ( fi.exists() )
    {
      const QString command = fi.fileName();
      const bool isTerminalEditor = command.compare( QLatin1String( "nano" ), Qt::CaseInsensitive ) == 0
                                    || command.contains( QLatin1String( "vim" ), Qt::CaseInsensitive );

      if ( !isTerminalEditor && QProcess::startDetached( editorCommand, { mFilePath }, dir.absolutePath() ) )
      {
        useFallback = false;
      }
    }
  }

  if ( useFallback )
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( mFilePath ) );
  }
  return true;
}

bool QgsCodeEditorWidget::shareOnGist( bool isPublic )
{
  const QString accessToken = QgsSettings().value( "pythonConsole/accessTokenGithub", QString() ).toString();
  if ( accessToken.isEmpty() )
  {
    if ( mMessageBar )
      mMessageBar->pushWarning( QString(), tr( "GitHub personal access token must be generated (see IDE Options)" ) );
    return false;
  }

  QString defaultFileName;
  switch ( mEditor->language() )
  {
    case Qgis::ScriptLanguage::Python:
      defaultFileName = QStringLiteral( "pyqgis_snippet.py" );
      break;

    case Qgis::ScriptLanguage::Css:
      defaultFileName = QStringLiteral( "qgis_snippet.css" );
      break;

    case Qgis::ScriptLanguage::QgisExpression:
      defaultFileName = QStringLiteral( "qgis_snippet" );
      break;

    case Qgis::ScriptLanguage::Html:
      defaultFileName = QStringLiteral( "qgis_snippet.html" );
      break;

    case Qgis::ScriptLanguage::JavaScript:
      defaultFileName = QStringLiteral( "qgis_snippet.js" );
      break;

    case Qgis::ScriptLanguage::Json:
      defaultFileName = QStringLiteral( "qgis_snippet.json" );
      break;

    case Qgis::ScriptLanguage::R:
      defaultFileName = QStringLiteral( "qgis_snippet.r" );
      break;

    case Qgis::ScriptLanguage::Sql:
      defaultFileName = QStringLiteral( "qgis_snippet.sql" );
      break;

    case Qgis::ScriptLanguage::Batch:
      defaultFileName = QStringLiteral( "qgis_snippet.bat" );
      break;

    case Qgis::ScriptLanguage::Bash:
      defaultFileName = QStringLiteral( "qgis_snippet.sh" );
      break;

    case Qgis::ScriptLanguage::Unknown:
      defaultFileName = QStringLiteral( "qgis_snippet.txt" );
      break;
  }
  const QString filename = mFilePath.isEmpty() ? defaultFileName : QFileInfo( mFilePath ).fileName();

  const QString contents = mEditor->hasSelectedText() ? mEditor->selectedText() : mEditor->text();
  const QVariantMap data {
    { QStringLiteral( "description" ), "Gist created by PyQGIS Console" },
    { QStringLiteral( "public" ), isPublic },
    { QStringLiteral( "files" ), QVariantMap { { filename, QVariantMap { { QStringLiteral( "content" ), contents } } } } }
  };

  QNetworkRequest request;
  request.setUrl( QUrl( QStringLiteral( "https://api.github.com/gists" ) ) );
  request.setRawHeader( "Authorization", QStringLiteral( "token %1" ).arg( accessToken ).toLocal8Bit() );
  request.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/json" ) );
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsCodeEditorWidget" ) );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->post( request, QgsJsonUtils::jsonFromVariant( data ).dump().c_str() );
  connect( reply, &QNetworkReply::finished, this, [this, reply] {
    if ( reply->error() == QNetworkReply::NoError )
    {
      const QVariantMap replyJson = QgsJsonUtils::parseJson( reply->readAll() ).toMap();
      const QString link = replyJson.value( QStringLiteral( "html_url" ) ).toString();
      QDesktopServices::openUrl( QUrl( link ) );
    }
    else
    {
      if ( mMessageBar )
        mMessageBar->pushCritical( QString(), tr( "Connection error: %1" ).arg( reply->errorString() ) );
    }
    reply->deleteLater();
  } );
  return true;
}

bool QgsCodeEditorWidget::findNext()
{
  return findText( true, false );
}

void QgsCodeEditorWidget::findPrevious()
{
  findText( false, false );
}

void QgsCodeEditorWidget::textSearchChanged( const QString &text )
{
  if ( !text.isEmpty() )
  {
    updateSearch();
  }
  else
  {
    clearSearchHighlights();
    mLineEditFind->setStyleSheet( QString() );
  }
}

void QgsCodeEditorWidget::updateSearch()
{
  if ( mBlockSearching )
    return;

  clearSearchHighlights();
  addSearchHighlights();

  findText( true, true );
}

void QgsCodeEditorWidget::replace()
{
  if ( mEditor->isReadOnly() )
    return;

  replaceSelection();

  clearSearchHighlights();
  addSearchHighlights();
  findNext();
}

void QgsCodeEditorWidget::replaceSelection()
{
  const long selectionStart = mEditor->SendScintilla( QsciScintilla::SCI_GETSELECTIONSTART );
  const long selectionEnd = mEditor->SendScintilla( QsciScintilla::SCI_GETSELECTIONEND );
  if ( selectionEnd - selectionStart <= 0 )
    return;

  const QString replacement = mLineEditReplace->text();

  mEditor->SendScintilla( QsciScintilla::SCI_SETTARGETRANGE, selectionStart, selectionEnd );

  if ( mRegexButton->isChecked() )
    mEditor->SendScintilla( QsciScintilla::SCI_REPLACETARGETRE, replacement.size(), replacement.toLocal8Bit().constData() );
  else
    mEditor->SendScintilla( QsciScintilla::SCI_REPLACETARGET, replacement.size(), replacement.toLocal8Bit().constData() );

  // set the cursor to the end of the replaced text
  const long postReplacementEnd = mEditor->SendScintilla( QsciScintilla::SCI_GETTARGETEND );
  mEditor->SendScintilla( QsciScintilla::SCI_SETCURRENTPOS, postReplacementEnd );
}

void QgsCodeEditorWidget::replaceAll()
{
  if ( mEditor->isReadOnly() )
    return;

  if ( !findText( true, true ) )
  {
    return;
  }

  mEditor->SendScintilla( QsciScintilla::SCI_BEGINUNDOACTION );
  replaceSelection();

  while ( findText( true, false ) )
  {
    replaceSelection();
  }

  mEditor->SendScintilla( QsciScintilla::SCI_ENDUNDOACTION );
  clearSearchHighlights();
}

void QgsCodeEditorWidget::addSearchHighlights()
{
  const QString searchString = mLineEditFind->text();
  if ( searchString.isEmpty() )
    return;

  const long originalStartPos = mEditor->SendScintilla( QsciScintilla::SCI_GETTARGETSTART );
  const long originalEndPos = mEditor->SendScintilla( QsciScintilla::SCI_GETTARGETEND );
  long startPos = 0;
  long docEnd = mEditor->length();

  updateHighlightController();

  int searchFlags = 0;
  const bool isRegEx = mRegexButton->isChecked();
  const bool isCaseSensitive = mCaseSensitiveButton->isChecked();
  const bool isWholeWordOnly = mWholeWordButton->isChecked();
  if ( isRegEx )
    searchFlags |= QsciScintilla::SCFIND_REGEXP | QsciScintilla::SCFIND_CXX11REGEX;
  if ( isCaseSensitive )
    searchFlags |= QsciScintilla::SCFIND_MATCHCASE;
  if ( isWholeWordOnly )
    searchFlags |= QsciScintilla::SCFIND_WHOLEWORD;
  mEditor->SendScintilla( QsciScintilla::SCI_SETSEARCHFLAGS, searchFlags );
  int matchCount = 0;
  while ( true )
  {
    mEditor->SendScintilla( QsciScintilla::SCI_SETTARGETRANGE, startPos, docEnd );
    const int fstart = mEditor->SendScintilla( QsciScintilla::SCI_SEARCHINTARGET, searchString.length(), searchString.toLocal8Bit().constData() );
    if ( fstart < 0 )
      break;

    const int matchLength = mEditor->SendScintilla( QsciScintilla::SCI_GETTARGETTEXT, 0, static_cast<void *>( nullptr ) );

    if ( matchLength == 0 )
    {
      startPos += 1;
      continue;
    }

    matchCount++;
    startPos = fstart + matchLength;

    mEditor->SendScintilla( QsciScintilla::SCI_SETINDICATORCURRENT, QgsCodeEditor::SEARCH_RESULT_INDICATOR );
    mEditor->SendScintilla( QsciScintilla::SCI_INDICATORFILLRANGE, fstart, matchLength );

    int thisLine = 0;
    int thisIndex = 0;
    mEditor->lineIndexFromPosition( fstart, &thisLine, &thisIndex );
    mHighlightController->addHighlight( QgsScrollBarHighlight( SearchMatch, thisLine, QColor( 0, 200, 0 ), QgsScrollBarHighlight::Priority::HighPriority ) );
  }

  mEditor->SendScintilla( QsciScintilla::SCI_SETTARGETRANGE, originalStartPos, originalEndPos );

  searchMatchCountChanged( matchCount );
}

void QgsCodeEditorWidget::clearSearchHighlights()
{
  long docStart = 0;
  long docEnd = mEditor->length();
  mEditor->SendScintilla( QsciScintilla::SCI_SETINDICATORCURRENT, QgsCodeEditor::SEARCH_RESULT_INDICATOR );
  mEditor->SendScintilla( QsciScintilla::SCI_INDICATORCLEARRANGE, docStart, docEnd - docStart );

  mHighlightController->removeHighlights( SearchMatch );

  searchMatchCountChanged( 0 );
}

bool QgsCodeEditorWidget::findText( bool forward, bool findFirst )
{
  const QString searchString = mLineEditFind->text();
  if ( searchString.isEmpty() )
    return false;

  int lineFrom = 0;
  int indexFrom = 0;
  int lineTo = 0;
  int indexTo = 0;
  mEditor->getSelection( &lineFrom, &indexFrom, &lineTo, &indexTo );

  int line = 0;
  int index = 0;
  if ( !findFirst )
  {
    mEditor->getCursorPosition( &line, &index );
  }
  if ( !forward )
  {
    line = lineFrom;
    index = indexFrom;
  }

  const bool isRegEx = mRegexButton->isChecked();
  const bool wrapAround = mWrapAroundButton->isChecked();
  const bool isCaseSensitive = mCaseSensitiveButton->isChecked();
  const bool isWholeWordOnly = mWholeWordButton->isChecked();

  const bool found = mEditor->findFirst( searchString, isRegEx, isCaseSensitive, isWholeWordOnly, wrapAround, forward, line, index, true, true, isRegEx );

  if ( !found )
  {
    const QString styleError = QStringLiteral( "QLineEdit {background-color: #d65253;  color: #ffffff;}" );
    mLineEditFind->setStyleSheet( styleError );
  }
  else
  {
    mLineEditFind->setStyleSheet( QString() );
  }
  return found;
}

void QgsCodeEditorWidget::searchMatchCountChanged( int matchCount )
{
  mReplaceButton->setEnabled( matchCount > 0 );
  mReplaceAllButton->setEnabled( matchCount > 0 );
  mFindNextButton->setEnabled( matchCount > 0 );
  mFindPrevButton->setEnabled( matchCount > 0 );
}

void QgsCodeEditorWidget::updateHighlightController()
{
  mHighlightController->setLineHeight( QFontMetrics( mEditor->font() ).lineSpacing() );
  mHighlightController->setVisibleRange( mEditor->viewport()->rect().height() );
}
