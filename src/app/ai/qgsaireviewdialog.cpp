/***************************************************************************
    qgsaireviewdialog.cpp
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

#include "qgsaireviewdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "moc_qgsaireviewdialog.cpp"

QgsAiReviewDialog::QgsAiReviewDialog( const QgsAiPatchProposal &proposal, QWidget *parent )
  : QDialog( parent )
  , mProposal( proposal )
{
  setWindowTitle( tr( "AI proposal review" ) );
  // Make the dialog comfortably wide for diff inspection.
  resize( 1100, 700 );
  setSizeGripEnabled( true );
  setModal( true );

  QVBoxLayout *layout = new QVBoxLayout( this );

  QLabel *header = new QLabel( this );
  header->setTextFormat( Qt::PlainText );
  header->setWordWrap( true );
  const QString title = mProposal.title.isEmpty() ? tr( "Proposed changes" ) : mProposal.title;
  header->setText( tr( "%1\n%n hunk(s) — review each one before accepting.", "", mProposal.hunks.size() ).arg( title ) );
  layout->addWidget( header );

  mTabs = new QTabWidget( this );
  layout->addWidget( mTabs, /*stretch=*/1 );

  const QFont monoFont = QFontDatabase::systemFont( QFontDatabase::FixedFont );

  for ( int i = 0; i < mProposal.hunks.size(); ++i )
  {
    const QgsAiPatchHunk &hunk = mProposal.hunks.at( i );
    QWidget *page = new QWidget( mTabs );
    QVBoxLayout *pageLayout = new QVBoxLayout( page );

    QCheckBox *include = new QCheckBox( tr( "Include this hunk in selective accept" ), page );
    include->setChecked( true );
    pageLayout->addWidget( include );
    mHunkChecks.append( include );

    QLabel *summary = new QLabel( hunkSummary( hunk ), page );
    summary->setTextFormat( Qt::PlainText );
    summary->setWordWrap( true );
    pageLayout->addWidget( summary );

    QSplitter *splitter = new QSplitter( Qt::Horizontal, page );
    QPlainTextEdit *beforeView = new QPlainTextEdit( splitter );
    beforeView->setReadOnly( true );
    beforeView->setLineWrapMode( QPlainTextEdit::NoWrap );
    beforeView->setFont( monoFont );
    if ( hunk.isCreate )
      beforeView->setPlainText( tr( "(file does not exist)" ) );
    else if ( hunk.isDelete )
      beforeView->setPlainText( tr( "(file will be deleted)" ) );
    else if ( hunk.originalText.isEmpty() )
      beforeView->setPlainText( tr( "(append-only — no original text to match)" ) );
    else
      beforeView->setPlainText( hunk.originalText );

    QPlainTextEdit *afterView = new QPlainTextEdit( splitter );
    afterView->setReadOnly( true );
    afterView->setLineWrapMode( QPlainTextEdit::NoWrap );
    afterView->setFont( monoFont );
    if ( hunk.isDelete )
      afterView->setPlainText( tr( "(removed)" ) );
    else
      afterView->setPlainText( hunk.replacementText );

    splitter->addWidget( beforeView );
    splitter->addWidget( afterView );
    splitter->setSizes( QList<int>() << 1 << 1 );
    pageLayout->addWidget( splitter, /*stretch=*/1 );

    QString tabTitle = hunk.filePath;
    if ( hunk.isCreate )
      tabTitle = tr( "+ %1" ).arg( hunk.filePath );
    else if ( hunk.isDelete )
      tabTitle = tr( "- %1" ).arg( hunk.filePath );
    mTabs->addTab( page, tabTitle );
  }

  QDialogButtonBox *buttons = new QDialogButtonBox( this );
  QPushButton *acceptAllBtn = buttons->addButton( tr( "Accept all" ), QDialogButtonBox::AcceptRole );
  QPushButton *acceptSelectedBtn = buttons->addButton( tr( "Accept selected" ), QDialogButtonBox::AcceptRole );
  QPushButton *rejectBtn = buttons->addButton( tr( "Reject" ), QDialogButtonBox::RejectRole );

  // Single hunk → "Accept selected" is redundant, hide it to keep UI clean.
  if ( mProposal.hunks.size() <= 1 )
    acceptSelectedBtn->setVisible( false );

  connect( acceptAllBtn, &QPushButton::clicked, this, &QgsAiReviewDialog::onAcceptAll );
  connect( acceptSelectedBtn, &QPushButton::clicked, this, &QgsAiReviewDialog::onAcceptSelected );
  connect( rejectBtn, &QPushButton::clicked, this, &QgsAiReviewDialog::onReject );

  layout->addWidget( buttons );
}

QString QgsAiReviewDialog::hunkSummary( const QgsAiPatchHunk &hunk ) const
{
  if ( hunk.isCreate )
    return tr( "Create new file: %1\n%2 bytes will be written." ).arg( hunk.filePath ).arg( hunk.replacementText.size() );
  if ( hunk.isDelete )
    return tr( "Delete file: %1\nA backup is kept until the chat session ends." ).arg( hunk.filePath );
  return tr( "Edit file: %1\nReplace %2 character(s) with %3 character(s)." ).arg( hunk.filePath ).arg( hunk.originalText.size() ).arg( hunk.replacementText.size() );
}

void QgsAiReviewDialog::onAcceptAll()
{
  mAcceptedHunkIndexes.clear();
  accept();
}

void QgsAiReviewDialog::onAcceptSelected()
{
  mAcceptedHunkIndexes.clear();
  for ( int i = 0; i < mHunkChecks.size(); ++i )
  {
    if ( mHunkChecks.at( i )->isChecked() )
      mAcceptedHunkIndexes.append( i );
  }
  if ( mAcceptedHunkIndexes.isEmpty() )
  {
    // Nothing selected → treat as reject so the caller doesn't apply an empty proposal.
    reject();
    return;
  }
  accept();
}

void QgsAiReviewDialog::onReject()
{
  mAcceptedHunkIndexes.clear();
  reject();
}
