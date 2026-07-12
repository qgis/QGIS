/***************************************************************************
    qgsairunpythontool.h
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

#ifndef QGSAIRUNPYTHONTOOL_H
#define QGSAIRUNPYTHONTOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QWidget;

/**
 * run_python: lets the model execute a snippet of PyQGIS code in the running
 * QGIS session. The user MUST approve via QgsAiPythonApprovalDialog before any
 * high-risk code runs; cancellation returns an "user_rejected" tool result.
 *
 * Output (stdout/stderr) and any traceback are captured via a small Python
 * wrapper that redirects sys.stdout/sys.stderr and writes the captured streams
 * to a temporary JSON file, which the tool reads back.
 *
 * Hard caps:
 *
 * - Maximum 8000 characters of code per call (prevents pathological prompts).
 * - Stdout/stderr capture is truncated to 32 KB each before being returned to the model.
 */
class APP_EXPORT QgsAiRunPythonTool : public QgsAiTool
{
  public:
    static constexpr int MAX_CODE_CHARS = 8000;
    static constexpr int MAX_CAPTURE_BYTES = 32768;

    explicit QgsAiRunPythonTool( QWidget *dialogParent );

    QString name() const override { return u"run_python"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolApprovalMode approvalMode() const override { return QgsAiToolApprovalMode::SelfApproved; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::Critical; }
    bool isAvailable() const override;
    QString availabilityReason() const override;
    void setRememberApprovalsForSession( bool enabled );

  private:
    QWidget *mDialogParent = nullptr;
    bool mRememberApprovalsForSession = false;
    bool mLowRiskApprovalGrantedForSession = false;
};

#endif // QGSAIRUNPYTHONTOOL_H
