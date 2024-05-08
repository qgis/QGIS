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

#include <QVBoxLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QShortcut>

QgsCodeEditorWidget::QgsCodeEditorWidget( QgsCodeEditor *editor, QWidget *parent )
  : QgsPanelWidget( parent )
  , mEditor( editor )
{
  Q_ASSERT( mEditor );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->setSpacing( 0 );
  vl->addWidget( editor, 1 );

  mFindWidget = new QWidget();
  QHBoxLayout *layoutFind = new QHBoxLayout();
  layoutFind->setContentsMargins( 0, 2, 0, 0 );
  mLineEditFind = new QgsFilterLineEdit();
  mLineEditFind->setShowSearchIcon( true );
  mLineEditFind->setPlaceholderText( tr( "Enter text to find…" ) );
  layoutFind->addWidget( mLineEditFind, 1 );

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

  mCaseSensitiveCheck = new QCheckBox( tr( "Case Sensitive" ) );
  layoutFind->addWidget( mCaseSensitiveCheck );

  mWholeWordCheck = new QCheckBox( tr( "Whole Word" ) );
  layoutFind->addWidget( mWholeWordCheck );

  mWrapAroundCheck = new QCheckBox( tr( "Wrap Around" ) );
  layoutFind->addWidget( mWrapAroundCheck );

  connect( mLineEditFind, &QLineEdit::returnPressed, this, &QgsCodeEditorWidget::findNext );
  connect( mLineEditFind, &QLineEdit::textChanged, this, &QgsCodeEditorWidget::textSearchChanged );
  connect( mFindNextButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::findNext );
  connect( mFindPrevButton, &QToolButton::clicked, this, &QgsCodeEditorWidget::findPrevious );
  connect( mCaseSensitiveCheck, &QCheckBox::toggled, this, &QgsCodeEditorWidget::updateSearch );
  connect( mWholeWordCheck, &QCheckBox::toggled, this, &QgsCodeEditorWidget::updateSearch );
  connect( mWrapAroundCheck, &QCheckBox::toggled, this, &QgsCodeEditorWidget::updateSearch );

  QShortcut *findShortcut = new QShortcut( QKeySequence::StandardKey::Find, mEditor );
  findShortcut->setContext( Qt::ShortcutContext::WidgetWithChildrenShortcut );
  connect( findShortcut, &QShortcut::activated, this, [this]
  {
    showSearchBar();
    mLineEditFind->setFocus();
    if ( mEditor->hasSelectedText() )
    {
      mBlockSearching++;
      mLineEditFind->setText( mEditor->selectedText().trimmed() );
      mBlockSearching--;
    }
    mLineEditFind->selectAll();
  } );

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

  mFindWidget->setLayout( layoutFind );
  vl->addWidget( mFindWidget );
  mFindWidget->hide();

  setLayout( vl );
}

void QgsCodeEditorWidget::showSearchBar()
{
  mFindWidget->show();
}

void QgsCodeEditorWidget::hideSearchBar()
{
  mFindWidget->hide();
}

void QgsCodeEditorWidget::setSearchBarVisible( bool visible )
{
  if ( visible )
    showSearchBar();
  else
    hideSearchBar();
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
    mLineEditFind->setStyleSheet( QString() );
    mFindNextButton->setEnabled( false );
    mFindPrevButton->setEnabled( false );
  }
}

void QgsCodeEditorWidget::updateSearch()
{
  if ( mBlockSearching )
    return;

  const QString searchString = mLineEditFind->text();
  if ( searchString.isEmpty() )
    return;

  findText( true, true, true );
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
  const bool wrapAround = mWrapAroundCheck->isChecked();
  const bool isCaseSensitive = mCaseSensitiveCheck->isChecked();
  const bool isWholeWordOnly = mWholeWordCheck->isChecked();

  const bool found = mEditor->findFirst( searchString, isRegEx, isCaseSensitive, isWholeWordOnly, wrapAround, forward,
                                         line, index );

  if ( !found )
  {
    const QString styleError = QStringLiteral( "QLineEdit {background-color: #d65253;  color: #ffffff;}" );
    mLineEditFind->setStyleSheet( styleError );

    Q_UNUSED( showNotFoundWarning )
#if 0 // TODO -- port this bit when messagebar is available
    if ( showMessage )
    {
      mMessageBar->pushMessage( QString(), tr( "\"%1\" was not found" ).arg( searchString ),
                                Qgis::MessageLevel::Info );
    }
#endif
  }
  else
  {
    mLineEditFind->setStyleSheet( QString() );
  }
}

