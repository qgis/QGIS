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
#include "qgscodeeditor.h"
#include "qgsfilterlineedit.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgsmessagebar.h"
#include "qgsdecoratedscrollbar.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QShortcut>

QgsCodeEditorWidget::QgsCodeEditorWidget(
  QgsCodeEditor *editor,
  QgsMessageBar *messageBar,
  QWidget *parent )
  : QgsPanelWidget( parent )
  , mEditor( editor )
  , mMessageBar( messageBar )
{
  Q_ASSERT( mEditor );

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
  QHBoxLayout *layoutFind = new QHBoxLayout();
  layoutFind->setContentsMargins( 0, 2, 0, 0 );
  layoutFind->setSpacing( 1 );
  mLineEditFind = new QgsFilterLineEdit();
  mLineEditFind->setShowSearchIcon( true );
  mLineEditFind->setPlaceholderText( tr( "Enter text to find…" ) );
  layoutFind->addWidget( mLineEditFind, 1 );

  mCaseSensitiveButton = new QToolButton();
  mCaseSensitiveButton->setToolTip( tr( "Case Sensitive" ) );
  mCaseSensitiveButton->setCheckable( true );
  mCaseSensitiveButton->setAutoRaise( true );
  mCaseSensitiveButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchCaseSensitive.svg" ) ) );
  layoutFind->addWidget( mCaseSensitiveButton );

  mWholeWordButton = new QToolButton( );
  mWholeWordButton->setToolTip( tr( "Whole Word" ) );
  mWholeWordButton->setCheckable( true );
  mWholeWordButton->setAutoRaise( true );
  mWholeWordButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchWholeWord.svg" ) ) );
  layoutFind->addWidget( mWholeWordButton );

  mWrapAroundButton = new QToolButton();
  mWrapAroundButton->setToolTip( tr( "Wrap Around" ) );
  mWrapAroundButton->setCheckable( true );
  mWrapAroundButton->setAutoRaise( true );
  mWrapAroundButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconSearchWrapAround.svg" ) ) );
  layoutFind->addWidget( mWrapAroundButton );

  mFindPrevButton = new QToolButton();
  mFindPrevButton->setEnabled( false );
  mFindPrevButton->setToolTip( tr( "Find Previous" ) );
  mFindPrevButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconSearchPrevEditorConsole.svg" ) ) );
  mFindPrevButton->setAutoRaise( true );
  layoutFind->addWidget( mFindPrevButton );

  mFindNextButton = new QToolButton();
  mFindNextButton->setEnabled( false );
  mFindNextButton->setToolTip( tr( "Find Next" ) );
  mFindNextButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconSearchNextEditorConsole.svg" ) ) );
  mFindNextButton->setAutoRaise( true );
  layoutFind->addWidget( mFindNextButton );

  connect( mLineEditFind, &QLineEdit::returnPressed, this, &QgsCodeEditorWidget::findNext );
  connect( mLineEditFind, &QLineEdit::textChanged, this, &QgsCodeEditorWidget::textSearchChanged );
  connect( mFindNextButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::findNext );
  connect( mFindPrevButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::findPrevious );
  connect( mCaseSensitiveButton, &QToolButton::toggled, this, &QgsCodeEditorWidget::updateSearch );
  connect( mCaseSensitiveButton, &QToolButton::toggled, this, &QgsCodeEditorWidget::updateSearch );
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

  // escape on editor hides the find bar
  QShortcut *closeFindShortcut = new QShortcut( Qt::Key::Key_Escape, this );
  closeFindShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
  connect( closeFindShortcut, &QShortcut::activated, this, [this]
  {
    hideSearchBar();
    mEditor->setFocus();
  } );

  QToolButton *closeFindButton = new QToolButton( this );
  closeFindButton->setToolTip( tr( "Close" ) );
  closeFindButton->setMinimumWidth( QgsGuiUtils::scaleIconSize( 44 ) );
  closeFindButton->setStyleSheet(
    "QToolButton { border:none; background-color: rgba(0, 0, 0, 0); }"
    "QToolButton::menu-button { border:none; background-color: rgba(0, 0, 0, 0); }" );
  closeFindButton->setCursor( Qt::PointingHandCursor );
  closeFindButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconClose.svg" ) ) );

  const int iconSize = std::max( 18.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.9 );
  closeFindButton->setIconSize( QSize( iconSize, iconSize ) );
  closeFindButton->setFixedSize( QSize( iconSize, iconSize ) );
  connect( closeFindButton, &QAbstractButton::clicked, this, [this]
  {
    hideSearchBar();
    mEditor->setFocus();
  } );
  layoutFind->addWidget( closeFindButton );

  mFindWidget->setLayout( layoutFind );
  vl->addWidget( mFindWidget );
  mFindWidget->hide();

  setLayout( vl );

  mHighlightController = std::make_unique< QgsScrollBarHighlightController >();
  mHighlightController->setScrollArea( mEditor );
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

void QgsCodeEditorWidget::showSearchBar()
{
  addSearchHighlights();
  mFindWidget->show();
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

void QgsCodeEditorWidget::findNext()
{
  findText( true, false );
}

void QgsCodeEditorWidget::findPrevious()
{
  findText( false, false );
}

void QgsCodeEditorWidget::textSearchChanged( const QString &text )
{
  if ( !text.isEmpty() )
  {
    mFindNextButton->setEnabled( true );
    mFindPrevButton->setEnabled( true );
    updateSearch();
  }
  else
  {
    clearSearchHighlights();
    mLineEditFind->setStyleSheet( QString() );
    mFindNextButton->setEnabled( false );
    mFindPrevButton->setEnabled( false );
  }
}

void QgsCodeEditorWidget::updateSearch()
{
  if ( mBlockSearching )
    return;

  clearSearchHighlights();
  addSearchHighlights();

  findText( true, true, true );
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

  mHighlightController->setLineHeight( QFontMetrics( mEditor->font() ).lineSpacing() );
  mHighlightController->setVisibleRange( mEditor->viewport()->rect().height() );

  while ( true )
  {
    mEditor->SendScintilla( QsciScintilla::SCI_SETTARGETRANGE, startPos, docEnd );
    const int fstart = mEditor->SendScintilla( QsciScintilla::SCI_SEARCHINTARGET, searchString.length(), searchString.toLocal8Bit().constData() );
    if ( fstart < 0 )
      break;

    startPos = fstart + searchString.length();

    mEditor->SendScintilla( QsciScintilla::SCI_SETINDICATORCURRENT, QgsCodeEditor::SEARCH_RESULT_INDICATOR );
    mEditor->SendScintilla( QsciScintilla::SCI_INDICATORFILLRANGE, fstart, searchString.length() );

    int thisLine = 0;
    int thisIndex = 0;
    mEditor->lineIndexFromPosition( fstart, &thisLine, &thisIndex );
    mHighlightController->addHighlight( QgsScrollBarHighlight( SearchMatch, thisLine, QColor( 0, 200, 0 ), QgsScrollBarHighlight::Priority::HighPriority ) );
  }

  mEditor->SendScintilla( QsciScintilla::SCI_SETTARGETRANGE, originalStartPos, originalEndPos );
}

void QgsCodeEditorWidget::clearSearchHighlights()
{
  long docStart = 0;
  long docEnd = mEditor->length();
  mEditor->SendScintilla( QsciScintilla::SCI_SETINDICATORCURRENT, QgsCodeEditor::SEARCH_RESULT_INDICATOR );
  mEditor->SendScintilla( QsciScintilla::SCI_INDICATORCLEARRANGE, docStart, docEnd - docStart );

  mHighlightController->removeHighlights( SearchMatch );
}

void QgsCodeEditorWidget::findText( bool forward, bool findFirst, bool showNotFoundWarning )
{
  const QString searchString = mLineEditFind->text();
  if ( searchString.isEmpty() )
    return;

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

  const bool isRegEx = false;
  const bool wrapAround = mWrapAroundButton->isChecked();
  const bool isCaseSensitive = mCaseSensitiveButton->isChecked();
  const bool isWholeWordOnly = mWholeWordButton->isChecked();

  const bool found = mEditor->findFirst( searchString, isRegEx, isCaseSensitive, isWholeWordOnly, wrapAround, forward,
                                         line, index );

  if ( !found )
  {
    const QString styleError = QStringLiteral( "QLineEdit {background-color: #d65253;  color: #ffffff;}" );
    mLineEditFind->setStyleSheet( styleError );

    if ( showNotFoundWarning )
    {
      mMessageBar->pushMessage( QString(), tr( "\"%1\" was not found" ).arg( searchString ),
                                Qgis::MessageLevel::Info );
    }
  }
  else
  {
    mLineEditFind->setStyleSheet( QString() );
  }
}

