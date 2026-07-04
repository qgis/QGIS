/***************************************************************************
    qgsaiprojecttools.h
    ---------------------
    begin                : July 2026
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

#ifndef QGSAIPROJECTTOOLS_H
#define QGSAIPROJECTTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsProject;

/**
 * manage_project: saves the project and manages project-level metadata.
 */
class APP_EXPORT QgsAiManageProjectTool : public QgsAiTool
{
  public:
    explicit QgsAiManageProjectTool( QgsProject *project );

    QString name() const override { return u"manage_project"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::High; }

  private:
    QgsProject *mProject = nullptr;
};

/**
 * configure_snapping: reads or updates project snapping settings.
 */
class APP_EXPORT QgsAiConfigureSnappingTool : public QgsAiTool
{
  public:
    explicit QgsAiConfigureSnappingTool( QgsProject *project );

    QString name() const override { return u"configure_snapping"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::Low; }

  private:
    QgsProject *mProject = nullptr;
};

#endif // QGSAIPROJECTTOOLS_H
