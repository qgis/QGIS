#include "qgsaireviewpatchengine.h"

#include "qgsaifilecontextprovider.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
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

bool QgsAiReviewPatchEngine::validateHunkPath( const QgsAiPatchHunk &hunk, QString *errorMessage ) const
{
  if ( hunk.filePath.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Encountered a patch hunk with an empty file path." );
    return false;
  }

  if ( !mContextProvider )
    return true;  // No sandbox configured (legacy callers); accept.

  // Reject path-traversal: the path must resolve inside the workspace.
  const QString resolved = mContextProvider->resolveWorkspaceFile( hunk.filePath );
  if ( resolved.isEmpty() )
  {
    // For create-file we accept paths that don't exist yet but lie inside the workspace.
    if ( hunk.isCreate )
    {
      const QString root = mContextProvider->workspaceRoot();
      const QFileInfo info( hunk.filePath );
      const QString absolute = info.isAbsolute() ? info.absoluteFilePath() : QDir( root ).absoluteFilePath( hunk.filePath );
      const QString relative = QDir( root ).relativeFilePath( absolute );
      const bool inside = !root.isEmpty()
                          && !relative.startsWith( QStringLiteral( "../" ) )
                          && relative != QLatin1String( ".." )
                          && !QDir::isAbsolutePath( relative );
      if ( inside )
        return true;
    }
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Path is outside workspace or unreadable: %1" ).arg( hunk.filePath );
    return false;
  }
  return true;
}

bool QgsAiReviewPatchEngine::applyProposalInternal( const QgsAiPatchProposal &proposal, const QList<int> *hunkIndexes, AppliedPatch &appliedPatch, QString *errorMessage ) const
{
  // Two passes:
  //   1. validate every hunk and compute the final content of each touched file in memory
  //   2. snapshot existing files for backup, then write/delete to disk
  // Splitting this way means a failure mid-way still leaves disk untouched.

  QMap<QString, QString> fileContents;       // for edit + create-file, the post-state to write
  QMap<QString, QString> originalContents;   // for backup
  QSet<QString> filesToDelete;               // paths to remove
  QSet<QString> filesBeingCreated;           // didn't exist before (used by undo)

  auto resolvedPath = [this]( const QgsAiPatchHunk &hunk ) -> QString {
    if ( !mContextProvider )
      return hunk.filePath;  // legacy callers
    const QString existing = mContextProvider->resolveWorkspaceFile( hunk.filePath );
    if ( !existing.isEmpty() )
      return existing;
    // Not found on disk: compute the absolute path under the workspace root.
    const QString root = mContextProvider->workspaceRoot();
    const QFileInfo info( hunk.filePath );
    return info.isAbsolute() ? info.absoluteFilePath() : QDir( root ).absoluteFilePath( hunk.filePath );
  };

  auto applyHunk = [&]( const QgsAiPatchHunk &hunk ) -> bool {
    if ( !validateHunkPath( hunk, errorMessage ) )
      return false;

    const QString path = resolvedPath( hunk );

    if ( hunk.isDelete )
    {
      if ( !QFileInfo::exists( path ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Cannot delete: file does not exist: %1" ).arg( path );
        return false;
      }
      // Snapshot for undo.
      QFile sourceFile( path );
      if ( !sourceFile.open( QIODevice::ReadOnly ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Cannot read file before delete (for backup): %1" ).arg( path );
        return false;
      }
      originalContents.insert( path, QString::fromUtf8( sourceFile.readAll() ) );
      filesToDelete.insert( path );
      return true;
    }

    if ( hunk.isCreate )
    {
      if ( QFileInfo::exists( path ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Cannot create: file already exists: %1" ).arg( path );
        return false;
      }
      filesBeingCreated.insert( path );
      fileContents.insert( path, hunk.replacementText );
      return true;
    }

    // Default: in-place edit.
    if ( !fileContents.contains( path ) )
    {
      QFile sourceFile( path );
      if ( !sourceFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Cannot open file for patching: %1" ).arg( path );
        return false;
      }
      const QString current = QString::fromUtf8( sourceFile.readAll() );
      fileContents.insert( path, current );
      originalContents.insert( path, current );
    }

    QString currentContent = fileContents.value( path );
    if ( !hunk.originalText.isEmpty() )
    {
      const int replaceIndex = currentContent.indexOf( hunk.originalText );
      if ( replaceIndex < 0 )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Original text for file '%1' could not be found. Proposal is stale." ).arg( path );
        return false;
      }

      currentContent.replace( replaceIndex, hunk.originalText.length(), hunk.replacementText );
    }
    else
    {
      currentContent.append( hunk.replacementText );
    }

    fileContents[path] = currentContent;
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

  // Build backups: existing edits + deleted files + created files (originalContent empty, wasMissing=true).
  for ( auto it = originalContents.constBegin(); it != originalContents.constEnd(); ++it )
  {
    FileBackup backup;
    backup.filePath = it.key();
    backup.originalContent = it.value();
    backup.wasMissing = false;
    appliedPatch.backups.push_back( backup );
  }
  for ( const QString &path : filesBeingCreated )
  {
    FileBackup backup;
    backup.filePath = path;
    backup.wasMissing = true;
    appliedPatch.backups.push_back( backup );
  }

  // Write phase: edits/creates first, then deletes.
  for ( auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it )
  {
    const QString &path = it.key();
    QFileInfo info( path );
    QDir parent = info.dir();
    if ( !parent.exists() && !parent.mkpath( QStringLiteral( "." ) ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Cannot create parent directory for: %1" ).arg( path );
      return false;
    }
    QFile targetFile( path );
    if ( !targetFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Cannot write patched file: %1" ).arg( path );
      return false;
    }

    QTextStream output( &targetFile );
    output << it.value();
  }

  for ( const QString &path : filesToDelete )
  {
    if ( !QFile::remove( path ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Cannot delete file: %1" ).arg( path );
      return false;
    }
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
    if ( backup.wasMissing )
    {
      // The file was created by this patch; undo by removing it.
      if ( QFile::exists( backup.filePath ) && !QFile::remove( backup.filePath ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Cannot remove file created by patch during undo: %1" ).arg( backup.filePath );
        return false;
      }
      continue;
    }

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
