/***************************************************************************
    qgsaiedittools.cpp
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

#include "qgsaiedittools.h"

#include "qgsaireviewdialog.h"
#include "qgsaireviewpatchengine.h"
#include "qgsaitoolschemautil.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QUuid>

using namespace Qt::StringLiterals;


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
    return QgsAiToolResult::error( u"Review engine not available."_s );
  if ( hunks.isEmpty() )
    return QgsAiToolResult::error( u"Cannot create an empty proposal."_s );

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
    output.insert( u"proposal_id"_s, proposalId );
    output.insert( u"status"_s, u"rejected"_s );
    output.insert( u"applied_hunks"_s, 0 );
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
  output.insert( u"proposal_id"_s, proposalId );
  if ( ok )
  {
    output.insert( u"status"_s, u"accepted"_s );
    output.insert( u"applied_hunks"_s, appliedCount );
    return QgsAiToolResult::ok( output );
  }

  // Engine refused to apply (e.g. stale original_text). Drop the proposal so it
  // doesn't linger in the pending list and surface the error to the model.
  if ( mEngine->hasPendingProposal( proposalId ) )
    mEngine->rejectProposal( proposalId );
  output.insert( u"status"_s, u"apply_failed"_s );
  output.insert( u"error"_s, errorMessage );
  return QgsAiToolResult::error( u"Patch could not be applied: %1"_s.arg( errorMessage ) );
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
  properties.insert( u"path"_s, prop( u"string"_s, u"Workspace path of the file to edit."_s ) );
  properties.insert( u"old_text"_s, prop( u"string"_s, u"Exact verbatim substring of the current file content."_s ) );
  properties.insert( u"new_text"_s, prop( u"string"_s, u"Replacement text. Empty string to delete the old_text region."_s ) );
  properties.insert( u"description"_s, prop( u"string"_s, u"Short human-readable summary of the change for the review dialog."_s ) );
  return schemaObject( properties, QJsonArray { u"path"_s, u"old_text"_s, u"new_text"_s } );
}

QgsAiToolResult QgsAiProposeEditTool::execute( const QJsonObject &args )
{
  const QString path = args.value( u"path"_s ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'path' is required."_s );
  if ( !args.contains( u"old_text"_s ) || !args.contains( u"new_text"_s ) )
    return QgsAiToolResult::error( u"Arguments 'old_text' and 'new_text' are required."_s );

  QgsAiPatchHunk hunk;
  hunk.filePath = path;
  hunk.originalText = args.value( u"old_text"_s ).toString();
  hunk.replacementText = args.value( u"new_text"_s ).toString();

  const QString description = args.value( u"description"_s ).toString();
  const QString title = description.isEmpty() ? u"Edit %1"_s.arg( path ) : description;

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
  properties.insert( u"path"_s, prop( u"string"_s, u"Workspace path for the new file."_s ) );
  properties.insert( u"content"_s, prop( u"string"_s, u"Full file content."_s ) );
  properties.insert( u"description"_s, prop( u"string"_s, u"Short human-readable summary."_s ) );
  return schemaObject( properties, QJsonArray { u"path"_s, u"content"_s } );
}

QgsAiToolResult QgsAiProposeCreateFileTool::execute( const QJsonObject &args )
{
  const QString path = args.value( u"path"_s ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'path' is required."_s );
  if ( !args.contains( u"content"_s ) )
    return QgsAiToolResult::error( u"Argument 'content' is required."_s );

  QgsAiPatchHunk hunk;
  hunk.filePath = path;
  hunk.replacementText = args.value( u"content"_s ).toString();
  hunk.isCreate = true;

  const QString description = args.value( u"description"_s ).toString();
  const QString title = description.isEmpty() ? u"Create %1"_s.arg( path ) : description;
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
  properties.insert( u"path"_s, prop( u"string"_s, u"Workspace path of the file to delete."_s ) );
  properties.insert( u"reason"_s, prop( u"string"_s, u"Why the file should be deleted (shown in the review dialog)."_s ) );
  return schemaObject( properties, QJsonArray { u"path"_s } );
}

QgsAiToolResult QgsAiProposeDeleteFileTool::execute( const QJsonObject &args )
{
  const QString path = args.value( u"path"_s ).toString().trimmed();
  if ( path.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'path' is required."_s );

  QgsAiPatchHunk hunk;
  hunk.filePath = path;
  hunk.isDelete = true;

  const QString reason = args.value( u"reason"_s ).toString();
  const QString title = reason.isEmpty() ? u"Delete %1"_s.arg( path ) : reason;
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
  editItem.insert( u"type"_s, u"object"_s );
  QJsonObject editItemProps;
  editItemProps.insert( u"path"_s, prop( u"string"_s, u"Workspace path of the file to edit."_s ) );
  editItemProps.insert( u"old_text"_s, prop( u"string"_s, u"Exact substring to replace."_s ) );
  editItemProps.insert( u"new_text"_s, prop( u"string"_s, u"Replacement text."_s ) );
  editItem.insert( u"properties"_s, editItemProps );
  editItem.insert( u"required"_s, QJsonArray { u"path"_s, u"old_text"_s, u"new_text"_s } );

  QJsonObject editsProp;
  editsProp.insert( u"type"_s, u"array"_s );
  editsProp.insert( u"description"_s, u"List of edits to bundle in one proposal."_s );
  editsProp.insert( u"items"_s, editItem );

  QJsonObject properties;
  properties.insert( u"edits"_s, editsProp );
  properties.insert( u"description"_s, prop( u"string"_s, u"Short human-readable summary of the batch."_s ) );
  return schemaObject( properties, QJsonArray { u"edits"_s } );
}

QgsAiToolResult QgsAiProposeMultiEditTool::execute( const QJsonObject &args )
{
  const QJsonArray edits = args.value( u"edits"_s ).toArray();
  if ( edits.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'edits' must be a non-empty array."_s );

  QList<QgsAiPatchHunk> hunks;
  hunks.reserve( edits.size() );
  for ( int i = 0; i < edits.size(); ++i )
  {
    const QJsonObject e = edits.at( i ).toObject();
    QgsAiPatchHunk hunk;
    hunk.filePath = e.value( u"path"_s ).toString().trimmed();
    hunk.originalText = e.value( u"old_text"_s ).toString();
    hunk.replacementText = e.value( u"new_text"_s ).toString();
    if ( hunk.filePath.isEmpty() )
      return QgsAiToolResult::error( u"edits[%1] is missing 'path'."_s.arg( i ) );
    hunks.append( hunk );
  }

  const QString description = args.value( u"description"_s ).toString();
  const QString title = description.isEmpty() ? u"Batch edit (%1 hunks)"_s.arg( hunks.size() ) : description;
  return reviewAndApply( title, hunks );
}
