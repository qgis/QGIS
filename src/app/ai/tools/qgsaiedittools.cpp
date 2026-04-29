#include "qgsaiedittools.h"

#include "qgsaireviewdialog.h"
#include "qgsaireviewpatchengine.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

namespace
{
  QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
  {
    QJsonObject schema;
    schema.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
    schema.insert( QStringLiteral( "properties" ), properties );
    if ( !required.isEmpty() )
      schema.insert( QStringLiteral( "required" ), required );
    return schema;
  }

  QJsonObject prop( const QString &type, const QString &description )
  {
    QJsonObject p;
    p.insert( QStringLiteral( "type" ), type );
    p.insert( QStringLiteral( "description" ), description );
    return p;
  }
}

// ---------------------------------------------------------------------------
// QgsAiBasePatchTool
// ---------------------------------------------------------------------------

QgsAiBasePatchTool::QgsAiBasePatchTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent )
  : mEngine( engine )
  , mDialogParent( dialogParent )
{}

QgsAiToolResult QgsAiBasePatchTool::reviewAndApply( const QString &title, const QList<QgsAiPatchHunk> &hunks ) const
{
  if ( !mEngine )
    return QgsAiToolResult::error( QStringLiteral( "Review engine not available." ) );
  if ( hunks.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Cannot create an empty proposal." ) );

  QgsAiPatchProposal proposal;
  proposal.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  proposal.title = title;
  proposal.hunks = hunks;
  proposal.createdAt = QDateTime::currentDateTimeUtc();

  const QString proposalId = mEngine->registerProposal( proposal );

  QgsAiReviewDialog dialog( proposal, mDialogParent );
  const int dialogResult = dialog.exec();

  if ( dialogResult != QDialog::Accepted )
  {
    mEngine->rejectProposal( proposalId );
    QJsonObject output;
    output.insert( QStringLiteral( "proposal_id" ), proposalId );
    output.insert( QStringLiteral( "status" ), QStringLiteral( "rejected" ) );
    output.insert( QStringLiteral( "applied_hunks" ), 0 );
    return QgsAiToolResult::ok( output );
  }

  const QList<int> selected = dialog.acceptedHunkIndexes();
  QString errorMessage;
  bool ok = false;
  int appliedCount = 0;
  if ( selected.isEmpty() )
  {
    ok = mEngine->acceptProposal( proposalId, &errorMessage );
    appliedCount = ok ? hunks.size() : 0;
  }
  else
  {
    ok = mEngine->acceptHunks( proposalId, selected, &errorMessage );
    appliedCount = ok ? selected.size() : 0;
  }

  QJsonObject output;
  output.insert( QStringLiteral( "proposal_id" ), proposalId );
  if ( ok )
  {
    output.insert( QStringLiteral( "status" ), QStringLiteral( "accepted" ) );
    output.insert( QStringLiteral( "applied_hunks" ), appliedCount );
    return QgsAiToolResult::ok( output );
  }

  // Engine refused to apply (e.g. stale original_text). Drop the proposal so it
  // doesn't linger in the pending list and surface the error to the model.
  if ( mEngine->hasPendingProposal( proposalId ) )
    mEngine->rejectProposal( proposalId );
  output.insert( QStringLiteral( "status" ), QStringLiteral( "apply_failed" ) );
  output.insert( QStringLiteral( "error" ), errorMessage );
  return QgsAiToolResult::error(
    QStringLiteral( "Patch could not be applied: %1" ).arg( errorMessage ) );
}

// ---------------------------------------------------------------------------
// propose_edit
// ---------------------------------------------------------------------------

QgsAiProposeEditTool::QgsAiProposeEditTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent )
  : QgsAiBasePatchTool( engine, dialogParent )
{}

QString QgsAiProposeEditTool::description() const
{
  return QStringLiteral(
    "Proposes a single in-place edit on an existing workspace file. "
    "old_text MUST be an exact verbatim substring of the current file content "
    "(read the file with read_file first to capture it). new_text replaces it. "
    "The user reviews the diff in a modal dialog and accepts or rejects."
  );
}

QJsonObject QgsAiProposeEditTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "path" ), prop( QStringLiteral( "string" ), QStringLiteral( "Workspace path of the file to edit." ) ) );
  properties.insert( QStringLiteral( "old_text" ), prop( QStringLiteral( "string" ), QStringLiteral( "Exact verbatim substring of the current file content." ) ) );
  properties.insert( QStringLiteral( "new_text" ), prop( QStringLiteral( "string" ), QStringLiteral( "Replacement text. Empty string to delete the old_text region." ) ) );
  properties.insert( QStringLiteral( "description" ), prop( QStringLiteral( "string" ), QStringLiteral( "Short human-readable summary of the change for the review dialog." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "path" ), QStringLiteral( "old_text" ), QStringLiteral( "new_text" ) } );
}

QgsAiToolResult QgsAiProposeEditTool::execute( const QJsonObject &args )
{
  const QString path = args.value( QStringLiteral( "path" ) ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'path' is required." ) );
  if ( !args.contains( QStringLiteral( "old_text" ) ) || !args.contains( QStringLiteral( "new_text" ) ) )
    return QgsAiToolResult::error( QStringLiteral( "Arguments 'old_text' and 'new_text' are required." ) );

  QgsAiPatchHunk hunk;
  hunk.filePath = path;
  hunk.originalText = args.value( QStringLiteral( "old_text" ) ).toString();
  hunk.replacementText = args.value( QStringLiteral( "new_text" ) ).toString();

  const QString description = args.value( QStringLiteral( "description" ) ).toString();
  const QString title = description.isEmpty()
                          ? QStringLiteral( "Edit %1" ).arg( path )
                          : description;

  return reviewAndApply( title, QList<QgsAiPatchHunk> { hunk } );
}

// ---------------------------------------------------------------------------
// propose_create_file
// ---------------------------------------------------------------------------

QgsAiProposeCreateFileTool::QgsAiProposeCreateFileTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent )
  : QgsAiBasePatchTool( engine, dialogParent )
{}

QString QgsAiProposeCreateFileTool::description() const
{
  return QStringLiteral(
    "Proposes the creation of a new file inside the workspace. The path must not "
    "already exist. Parent directories are created automatically when accepted."
  );
}

QJsonObject QgsAiProposeCreateFileTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "path" ), prop( QStringLiteral( "string" ), QStringLiteral( "Workspace path for the new file." ) ) );
  properties.insert( QStringLiteral( "content" ), prop( QStringLiteral( "string" ), QStringLiteral( "Full file content." ) ) );
  properties.insert( QStringLiteral( "description" ), prop( QStringLiteral( "string" ), QStringLiteral( "Short human-readable summary." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "path" ), QStringLiteral( "content" ) } );
}

QgsAiToolResult QgsAiProposeCreateFileTool::execute( const QJsonObject &args )
{
  const QString path = args.value( QStringLiteral( "path" ) ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'path' is required." ) );
  if ( !args.contains( QStringLiteral( "content" ) ) )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'content' is required." ) );

  QgsAiPatchHunk hunk;
  hunk.filePath = path;
  hunk.replacementText = args.value( QStringLiteral( "content" ) ).toString();
  hunk.isCreate = true;

  const QString description = args.value( QStringLiteral( "description" ) ).toString();
  const QString title = description.isEmpty() ? QStringLiteral( "Create %1" ).arg( path ) : description;
  return reviewAndApply( title, QList<QgsAiPatchHunk> { hunk } );
}

// ---------------------------------------------------------------------------
// propose_delete_file
// ---------------------------------------------------------------------------

QgsAiProposeDeleteFileTool::QgsAiProposeDeleteFileTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent )
  : QgsAiBasePatchTool( engine, dialogParent )
{}

QString QgsAiProposeDeleteFileTool::description() const
{
  return QStringLiteral(
    "Proposes the deletion of an existing workspace file. A backup is taken so "
    "the user can undo the deletion via the engine's undo facility."
  );
}

QJsonObject QgsAiProposeDeleteFileTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "path" ), prop( QStringLiteral( "string" ), QStringLiteral( "Workspace path of the file to delete." ) ) );
  properties.insert( QStringLiteral( "reason" ), prop( QStringLiteral( "string" ), QStringLiteral( "Why the file should be deleted (shown in the review dialog)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "path" ) } );
}

QgsAiToolResult QgsAiProposeDeleteFileTool::execute( const QJsonObject &args )
{
  const QString path = args.value( QStringLiteral( "path" ) ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'path' is required." ) );

  QgsAiPatchHunk hunk;
  hunk.filePath = path;
  hunk.isDelete = true;

  const QString reason = args.value( QStringLiteral( "reason" ) ).toString();
  const QString title = reason.isEmpty() ? QStringLiteral( "Delete %1" ).arg( path ) : reason;
  return reviewAndApply( title, QList<QgsAiPatchHunk> { hunk } );
}

// ---------------------------------------------------------------------------
// propose_multi_edit
// ---------------------------------------------------------------------------

QgsAiProposeMultiEditTool::QgsAiProposeMultiEditTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent )
  : QgsAiBasePatchTool( engine, dialogParent )
{}

QString QgsAiProposeMultiEditTool::description() const
{
  return QStringLiteral(
    "Proposes a batch of edits across one or more existing files as a single "
    "review dialog. Each entry of 'edits' has {path, old_text, new_text}. The user "
    "can accept all, reject, or accept only the selected hunks."
  );
}

QJsonObject QgsAiProposeMultiEditTool::schema() const
{
  QJsonObject editItem;
  editItem.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
  QJsonObject editItemProps;
  editItemProps.insert( QStringLiteral( "path" ), prop( QStringLiteral( "string" ), QStringLiteral( "Workspace path of the file to edit." ) ) );
  editItemProps.insert( QStringLiteral( "old_text" ), prop( QStringLiteral( "string" ), QStringLiteral( "Exact substring to replace." ) ) );
  editItemProps.insert( QStringLiteral( "new_text" ), prop( QStringLiteral( "string" ), QStringLiteral( "Replacement text." ) ) );
  editItem.insert( QStringLiteral( "properties" ), editItemProps );
  editItem.insert( QStringLiteral( "required" ), QJsonArray { QStringLiteral( "path" ), QStringLiteral( "old_text" ), QStringLiteral( "new_text" ) } );

  QJsonObject editsProp;
  editsProp.insert( QStringLiteral( "type" ), QStringLiteral( "array" ) );
  editsProp.insert( QStringLiteral( "description" ), QStringLiteral( "List of edits to bundle in one proposal." ) );
  editsProp.insert( QStringLiteral( "items" ), editItem );

  QJsonObject properties;
  properties.insert( QStringLiteral( "edits" ), editsProp );
  properties.insert( QStringLiteral( "description" ), prop( QStringLiteral( "string" ), QStringLiteral( "Short human-readable summary of the batch." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "edits" ) } );
}

QgsAiToolResult QgsAiProposeMultiEditTool::execute( const QJsonObject &args )
{
  const QJsonArray edits = args.value( QStringLiteral( "edits" ) ).toArray();
  if ( edits.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'edits' must be a non-empty array." ) );

  QList<QgsAiPatchHunk> hunks;
  hunks.reserve( edits.size() );
  for ( int i = 0; i < edits.size(); ++i )
  {
    const QJsonObject e = edits.at( i ).toObject();
    QgsAiPatchHunk hunk;
    hunk.filePath = e.value( QStringLiteral( "path" ) ).toString().trimmed();
    hunk.originalText = e.value( QStringLiteral( "old_text" ) ).toString();
    hunk.replacementText = e.value( QStringLiteral( "new_text" ) ).toString();
    if ( hunk.filePath.isEmpty() )
      return QgsAiToolResult::error( QStringLiteral( "edits[%1] is missing 'path'." ).arg( i ) );
    hunks.append( hunk );
  }

  const QString description = args.value( QStringLiteral( "description" ) ).toString();
  const QString title = description.isEmpty()
                          ? QStringLiteral( "Batch edit (%1 hunks)" ).arg( hunks.size() )
                          : description;
  return reviewAndApply( title, hunks );
}
