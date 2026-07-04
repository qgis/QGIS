/***************************************************************************
    qgsaiedittools.h
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

#ifndef QGSAIEDITTOOLS_H
#define QGSAIEDITTOOLS_H

#include "qgis_app.h"
#include "qgsaimodels.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsAiReviewPatchEngine;
class QWidget;

/**
 * Common base for the propose_* tools. Each tool builds a QgsAiPatchProposal,
 * registers it in the review engine, and synchronously shows the review dialog.
 * The user's accept/reject choice is reported back to the LLM as the tool result.
 *
 * The dialog is shown modally (\a QDialog::exec) so the agent loop blocks until
 * the user has decided. When integrated, the host application must pass a parent
 * widget so the dialog adopts the right window flags / position.
 */
class APP_EXPORT QgsAiBasePatchTool : public QgsAiTool
{
  public:
    QgsAiBasePatchTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent );

    bool requiresApproval() const override { return true; }
    QgsAiToolApprovalMode approvalMode() const override { return QgsAiToolApprovalMode::SelfApproved; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::Medium; }

  protected:
    /**
     * Registers \a proposal in the engine, opens the review dialog, applies the
     * accepted hunks (or rejects), and returns the JSON-friendly result for the LLM.
     */
    QgsAiToolResult reviewAndApply( const QString &title, const QList<QgsAiPatchHunk> &hunks ) const;

    QgsAiReviewPatchEngine *mEngine = nullptr;
    QWidget *mDialogParent = nullptr;
};

/**
 * propose_edit: replaces a contiguous text range in an existing file. The model
 * provides the EXACT \a old_text snippet from the file (must match) and the
 * \a new_text to replace it with.
 */
class APP_EXPORT QgsAiProposeEditTool : public QgsAiBasePatchTool
{
  public:
    QgsAiProposeEditTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent );

    QString name() const override { return u"propose_edit"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

/**
 * propose_create_file: creates a new file with the given content. The path must
 * be inside the workspace and must not already exist.
 */
class APP_EXPORT QgsAiProposeCreateFileTool : public QgsAiBasePatchTool
{
  public:
    QgsAiProposeCreateFileTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent );

    QString name() const override { return u"propose_create_file"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

/**
 * propose_delete_file: removes a file from the workspace. A backup is kept so
 * the user can undo the deletion via QgsAiReviewPatchEngine::undoLastApply.
 */
class APP_EXPORT QgsAiProposeDeleteFileTool : public QgsAiBasePatchTool
{
  public:
    QgsAiProposeDeleteFileTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent );

    QString name() const override { return u"propose_delete_file"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

/**
 * propose_multi_edit: groups N edits across one or more files into a single
 * proposal. The user can accept all hunks, accept only selected ones, or reject.
 */
class APP_EXPORT QgsAiProposeMultiEditTool : public QgsAiBasePatchTool
{
  public:
    QgsAiProposeMultiEditTool( QgsAiReviewPatchEngine *engine, QWidget *dialogParent );

    QString name() const override { return u"propose_multi_edit"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

#endif // QGSAIEDITTOOLS_H
