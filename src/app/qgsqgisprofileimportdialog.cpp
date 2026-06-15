/***************************************************************************
  qgsqgisprofileimportdialog.cpp
  ------------------------------
  begin                : June 2026
  copyright            : (C) 2026 by Francesco Mazzi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsqgisprofileimportdialog.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

QgsQgisProfileImportDialog::QgsQgisProfileImportDialog( const QList<QgsQgisProfileImporter::Candidate> &candidates, const QString &targetRootProfileFolder, Mode mode, QWidget *parent )
  : QDialog( parent )
  , mTargetRootProfileFolder( targetRootProfileFolder )
  , mMode( mode )
{
  setWindowTitle( mode == Mode::FirstRun ? tr( "Import QGIS Environment" ) : tr( "Import QGIS Profile" ) );
  resize( 640, 420 );

  QVBoxLayout *layout = new QVBoxLayout( this );

  QLabel *title = new QLabel( mode == Mode::FirstRun ? tr( "Strata found existing QGIS profiles." ) : tr( "Choose a QGIS profile to import into Strata." ), this );
  QFont titleFont = title->font();
  titleFont.setPointSize( titleFont.pointSize() + 3 );
  titleFont.setBold( true );
  title->setFont( titleFont );
  layout->addWidget( title );

  QLabel *description = new QLabel( tr( "The import copies QGIS preferences, user databases, authentication data, Python plugins, templates, scripts, styles, fonts, palettes, SVGs, and plugin enabled state." ), this );
  description->setWordWrap( true );
  layout->addWidget( description );

  mProfileList = new QListWidget( this );
  mProfileList->setSelectionMode( mode == Mode::FirstRun ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection );
  layout->addWidget( mProfileList, 1 );

  for ( const QgsQgisProfileImporter::Candidate &candidate : candidates )
    addCandidate( candidate, true );

  QPushButton *browseButton = new QPushButton( tr( "Browse..." ), this );
  connect( browseButton, &QPushButton::clicked, this, [this] { addCustomProfile(); } );

  QHBoxLayout *browseLayout = new QHBoxLayout();
  browseLayout->addStretch();
  browseLayout->addWidget( browseButton );
  layout->addLayout( browseLayout );

  if ( mode == Mode::Manual )
  {
    QFormLayout *formLayout = new QFormLayout();
    mTargetProfileName = new QLineEdit( this );
    formLayout->addRow( tr( "New Strata profile name" ), mTargetProfileName );
    layout->addLayout( formLayout );
  }

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel, this );
  mImportButton = buttonBox->addButton( mode == Mode::FirstRun ? tr( "Import and Start" ) : tr( "Import Profile" ), QDialogButtonBox::AcceptRole );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  layout->addWidget( buttonBox );

  connect( mProfileList, &QListWidget::itemChanged, this, [this]( QListWidgetItem * ) { updateImportButton(); } );
  connect( mProfileList, &QListWidget::currentItemChanged, this, [this]( QListWidgetItem *, QListWidgetItem * ) {
    updateSuggestedTargetName();
    updateImportButton();
  } );
  if ( mTargetProfileName )
    connect( mTargetProfileName, &QLineEdit::textChanged, this, [this] { updateImportButton(); } );

  if ( mProfileList->count() > 0 )
    mProfileList->setCurrentRow( 0 );

  updateSuggestedTargetName();
  updateImportButton();
}

QList<QgsQgisProfileImporter::Candidate> QgsQgisProfileImportDialog::selectedCandidates() const
{
  QList<QgsQgisProfileImporter::Candidate> candidates;
  for ( int i = 0; i < mProfileList->count(); ++i )
  {
    QListWidgetItem *item = mProfileList->item( i );
    const int candidateIndex = item->data( Qt::UserRole ).toInt();
    if ( candidateIndex < 0 || candidateIndex >= mCandidates.size() )
      continue;

    if ( mMode == Mode::FirstRun )
    {
      if ( item->checkState() == Qt::Checked )
        candidates.append( mCandidates.at( candidateIndex ) );
    }
    else if ( item->isSelected() || item == mProfileList->currentItem() )
    {
      candidates.append( mCandidates.at( candidateIndex ) );
      break;
    }
  }

  return candidates;
}

QString QgsQgisProfileImportDialog::targetProfileName() const
{
  return mTargetProfileName ? mTargetProfileName->text().trimmed() : QString();
}

void QgsQgisProfileImportDialog::addCandidate( const QgsQgisProfileImporter::Candidate &candidate, bool checked )
{
  mCandidates.append( candidate );

  QString label = tr( "%1 (%2)" ).arg( candidate.profileName, candidate.sourceVersionLabel );
  if ( candidate.pluginCount == 1 )
    label += tr( " - 1 Python plugin" );
  else if ( candidate.pluginCount > 1 )
    label += tr( " - %1 Python plugins" ).arg( candidate.pluginCount );

  QListWidgetItem *item = new QListWidgetItem( label, mProfileList );
  item->setToolTip( candidate.profilePath );
  item->setData( Qt::UserRole, mCandidates.size() - 1 );
  if ( mMode == Mode::FirstRun )
  {
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
    item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
  }
}

void QgsQgisProfileImportDialog::addCustomProfile()
{
  const QString profilePath = QFileDialog::getExistingDirectory( this, tr( "Choose QGIS Profile Folder" ), QDir::homePath() );
  if ( profilePath.isEmpty() )
    return;

  const QFileInfo profileInfo( profilePath );
  QgsQgisProfileImporter::Candidate candidate;
  candidate.profileName = profileInfo.fileName().isEmpty() ? u"qgis-profile"_s : profileInfo.fileName();
  candidate.profilePath = QDir::cleanPath( profileInfo.absoluteFilePath() );
  candidate.sourceRoot = QDir::cleanPath( profileInfo.absoluteDir().absolutePath() );
  candidate.sourceVersionLabel = tr( "Custom" );
  candidate.pluginCount = QDir( QDir( candidate.profilePath ).filePath( u"python/plugins"_s ) ).entryList( QDir::Dirs | QDir::NoDotAndDotDot ).size();

  addCandidate( candidate, true );
  mProfileList->setCurrentRow( mProfileList->count() - 1 );
  updateSuggestedTargetName();
  updateImportButton();
}

void QgsQgisProfileImportDialog::updateImportButton()
{
  const bool hasSelection = !selectedCandidates().isEmpty();
  const bool hasTargetName = !mTargetProfileName || !mTargetProfileName->text().trimmed().isEmpty();
  if ( mImportButton )
    mImportButton->setEnabled( hasSelection && hasTargetName );
}

void QgsQgisProfileImportDialog::updateSuggestedTargetName()
{
  if ( !mTargetProfileName )
    return;

  const QList<QgsQgisProfileImporter::Candidate> candidates = selectedCandidates();
  if ( candidates.isEmpty() )
    return;

  const QString suggestedName = QgsQgisProfileImporter::uniqueProfileName( candidates.constFirst().profileName, mTargetRootProfileFolder );
  if ( mTargetProfileName->text().isEmpty() || !QFileInfo::exists( QDir( mTargetRootProfileFolder ).filePath( mTargetProfileName->text() ) ) )
    mTargetProfileName->setText( suggestedName );
}
