/***************************************************************************
    qgsaieditingtools.h
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

#ifndef QGSAIEDITINGTOOLS_H
#define QGSAIEDITINGTOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsProject;

/**
 * edit_feature_geometry: edits the geometry of an existing vector feature.
 */
class APP_EXPORT QgsAiEditFeatureGeometryTool : public QgsAiTool
{
  public:
    explicit QgsAiEditFeatureGeometryTool( QgsProject *project );

    QString name() const override { return u"edit_feature_geometry"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::High; }

  private:
    QgsProject *mProject = nullptr;
};

#endif // QGSAIEDITINGTOOLS_H
