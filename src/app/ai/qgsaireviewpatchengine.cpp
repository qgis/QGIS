#include "qgsaireviewpatchengine.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUuid>

QgsAiReviewPatchEngine::QgsAiReviewPatchEngine( QObject *parent )
  : QObject( parent )
{
}

QString QgsAiReviewPatchEngine::registerProposal( const QgsAiPatchProposal &proposal )
{
  QgsAiPatchProposal mutableProposal = proposal;
  if ( mutableProposal.id.isEmpty() )
    mutableProposal.id = QUuid::createUuid().toString( QUuid::WithoutBraces );

  mPendingProposals.insert( mutableProposal.id, mutableProposal );
  mAuditTrail << QStringLiteral( "proposal-added:%1" ).arg( mutableProposal.id );
  emit proposalAdded( mutableProposal.id );
  return mutableProposal.id;
}

QList<QgsAiPatchProposal> QgsAiReviewPatchEngine::pendingProposals() const
{
  return mPendingProposals.values();
}

bool QgsAiReviewPatchEngine::hasPendingProposal( const QString &proposalId ) const
{
  return mPendingProposals.contains( proposalId );
}

QString QgsAiReviewPatchEngine::proposalDiff( const QgsAiPatchProposal &proposal )
{
  QString diff;
  diff += QStringLiteral( "# %1\n\n" ).arg( proposal.title.isEmpty() ? QStringLiteral( "Untitled proposal" ) : proposal.title );
  for ( int i = 0; i < proposal.hunks.size(); ++i )
  {
    const QgsAiPatchHunk &hunk = proposal.hunks.at( i );
    diff += QStringLiteral( "## Hunk %1 - %2\n" ).arg( i + 1 ).arg( hunk.filePath );
    diff += QStringLiteral( "--- original\n%1\n" ).arg( hunk.originalText.left( 1500 ) );
    diff += QStringLiteral( "+++ replacement\n%1\n\n" ).arg( hunk.replacementText.left( 1500 ) );
  }
  return diff;
}

QString QgsAiReviewPatchEngine::previewProposalDiff( const QString &proposalId ) const
{
  if ( !mPendingProposals.contains( proposalId ) )
    return QString();
  return proposalDiff( mPendingProposals.value( proposalId ) );
}

bool QgsAiReviewPatchEngine::applyProposalInternal( const QgsAiPatchProposal &proposal, const QList<int> *hunkIndexes, AppliedPatch &appliedPatch, QString *errorMessage ) const
{
  QMap<QString, QString> fileContents;

  auto applyHunk = [&fileContents, errorMessage]( const QgsAiPatchHunk &hunk ) -> bool
  {
    if ( hunk.filePath.isEmpty() )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Encountered a patch hunk with an empty file path." );
      return false;
    }

    if ( !fileContents.contains( hunk.filePath ) )
    {
      QFile sourceFile( hunk.filePath );
      if ( !sourceFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Cannot open file for patching: %1" ).arg( hunk.filePath );
        return false;
      }

      fileContents.insert( hunk.filePath, QString::fromUtf8( sourceFile.readAll() ) );
    }

    QString currentContent = fileContents.value( hunk.filePath );
    if ( !hunk.originalText.isEmpty() )
    {
      const int replaceIndex = currentContent.indexOf( hunk.originalText );
      if ( replaceIndex < 0 )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Original text for file '%1' could not be found. Proposal is stale." ).arg( hunk.filePath );
        return false;
      }

      currentContent.replace( replaceIndex, hunk.originalText.length(), hunk.replacementText );
    }
    else
    {
      currentContent.append( hunk.replacementText );
    }

    fileContents[hunk.filePath] = currentContent;
    return true;
  };

  if ( hunkIndexes )
  {
    for ( const int hunkIndex : *hunkIndexes )
    {
      if ( hunkIndex < 0 || hunkIndex >= proposal.hunks.size() )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Invalid hunk index: %1" ).arg( hunkIndex );
        return false;
      }

      if ( !applyHunk( proposal.hunks.at( hunkIndex ) ) )
        return false;
    }
  }
  else
  {
    for ( const QgsAiPatchHunk &hunk : proposal.hunks )
    {
      if ( !applyHunk( hunk ) )
        return false;
    }
  }

  for ( auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it )
  {
    const QString path = it.key();
    QFile sourceFile( path );
    if ( !sourceFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Cannot reopen file while preparing backup: %1" ).arg( path );
      return false;
    }

    FileBackup backup;
    backup.filePath = path;
    backup.originalContent = QString::fromUtf8( sourceFile.readAll() );
    appliedPatch.backups.push_back( backup );
  }

  for ( auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it )
  {
    QFile targetFile( it.key() );
    if ( !targetFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Cannot write patched file: %1" ).arg( it.key() );
      return false;
    }

    QTextStream output( &targetFile );
    output << it.value();
  }

  return true;
}

bool QgsAiReviewPatchEngine::acceptProposal( const QString &proposalId, QString *errorMessage )
{
  const QgsAiPatchProposal proposal = mPendingProposals.value( proposalId );
  if ( proposal.id.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Unknown proposal ID." );
    return false;
  }

  AppliedPatch appliedPatch;
  appliedPatch.proposalId = proposalId;
  if ( !applyProposalInternal( proposal, nullptr, appliedPatch, errorMessage ) )
    return false;

  mAppliedPatches.push_back( appliedPatch );
  mPendingProposals.remove( proposalId );
  mAuditTrail << QStringLiteral( "proposal-accepted:%1" ).arg( proposalId );
  emit proposalAccepted( proposalId );
  return true;
}

bool QgsAiReviewPatchEngine::acceptHunks( const QString &proposalId, const QList<int> &hunkIndexes, QString *errorMessage )
{
  const QgsAiPatchProposal proposal = mPendingProposals.value( proposalId );
  if ( proposal.id.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Unknown proposal ID." );
    return false;
  }

  AppliedPatch appliedPatch;
  appliedPatch.proposalId = proposalId;
  if ( !applyProposalInternal( proposal, &hunkIndexes, appliedPatch, errorMessage ) )
    return false;

  mAppliedPatches.push_back( appliedPatch );
  mPendingProposals.remove( proposalId );
  mAuditTrail << QStringLiteral( "proposal-partial-accepted:%1" ).arg( proposalId );
  emit proposalAccepted( proposalId );
  return true;
}

bool QgsAiReviewPatchEngine::rejectProposal( const QString &proposalId )
{
  if ( !mPendingProposals.contains( proposalId ) )
    return false;

  mPendingProposals.remove( proposalId );
  mAuditTrail << QStringLiteral( "proposal-rejected:%1" ).arg( proposalId );
  emit proposalRejected( proposalId );
  return true;
}

bool QgsAiReviewPatchEngine::undoLastApply( QString *errorMessage )
{
  if ( mAppliedPatches.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "There is no applied proposal to undo." );
    return false;
  }

  const AppliedPatch appliedPatch = mAppliedPatches.takeLast();
  for ( const FileBackup &backup : appliedPatch.backups )
  {
    QFile file( backup.filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Cannot restore file during undo: %1" ).arg( backup.filePath );
      return false;
    }

    QTextStream output( &file );
    output << backup.originalContent;
  }

  mAuditTrail << QStringLiteral( "proposal-undo:%1" ).arg( appliedPatch.proposalId );
  return true;
}
