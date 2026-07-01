/***************************************************************************
    qgsaiinstallpackagetool.h
    ---------------------
    begin                : May 2026
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

#ifndef QGSAIINSTALLPACKAGETOOL_H
#define QGSAIINSTALLPACKAGETOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QWidget;

/**
 * install_python_package: lets the model install one or more Python packages into
 * the QGIS Python interpreter via `pip install --user`. The user MUST approve via
 * QgsAiPipInstallApprovalDialog before any install runs; cancellation returns
 * "user_rejected".
 *
 * Each spec is validated with a strict regex that allows only the form
 * `<name>[<op><version>]`, blocking URLs, `git+...`, `-r requirements.txt`,
 * and any shell-injection vector.
 *
 * The install runs via QgsPythonRunner, shelling out to a validated Python
 * interpreter with matching major/minor version using `python -m pip install
 * --user`. Output is captured into a temp JSON file and returned to the model
 * (truncated to 32 KB per stream).
 *
 * Hard caps:
 *
 * - Maximum 10 specs per call.
 * - Each spec ≤ 120 characters.
 * - Stdout/stderr capture is truncated to 32 KB each before being returned to the model.
 * - 5-minute timeout on the underlying pip call (subprocess.run timeout).
 */
class APP_EXPORT QgsAiInstallPythonPackageTool : public QgsAiTool
{
  public:
    static constexpr int MAX_PACKAGES = 10;
    static constexpr int MAX_SPEC_CHARS = 120;
    static constexpr int MAX_CAPTURE_BYTES = 32768;

    explicit QgsAiInstallPythonPackageTool( QWidget *dialogParent );

    QString name() const override { return u"install_python_package"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::High; }
    bool isAvailable() const override;
    QString availabilityReason() const override;

  private:
    QWidget *mDialogParent = nullptr;
};

#endif // QGSAIINSTALLPACKAGETOOL_H
