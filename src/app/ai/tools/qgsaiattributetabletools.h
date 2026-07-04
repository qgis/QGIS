/***************************************************************************
    qgsaiattributetabletools.h
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

#ifndef QGSAIATTRIBUTETABLETOOLS_H
#define QGSAIATTRIBUTETABLETOOLS_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsProject;

class APP_EXPORT QgsAiQueryFeaturesTool : public QgsAiTool
{
  public:
    explicit QgsAiQueryFeaturesTool( QgsProject *project );

    QString name() const override { return u"query_features"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsProject *mProject = nullptr;
};

class APP_EXPORT QgsAiBatchUpdateAttributesTool : public QgsAiTool
{
  public:
    explicit QgsAiBatchUpdateAttributesTool( QgsProject *project );

    QString name() const override { return u"batch_update_attributes"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::High; }

  private:
    QgsProject *mProject = nullptr;
};

class APP_EXPORT QgsAiSelectFeaturesTool : public QgsAiTool
{
  public:
    explicit QgsAiSelectFeaturesTool( QgsProject *project );

    QString name() const override { return u"select_features"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool requiresApproval() const override { return true; }
    QgsAiToolRiskLevel riskLevel() const override { return QgsAiToolRiskLevel::Low; }

  private:
    QgsProject *mProject = nullptr;
};

class APP_EXPORT QgsAiIdentifyFeaturesAtTool : public QgsAiTool
{
  public:
    explicit QgsAiIdentifyFeaturesAtTool( QgsProject *project );

    QString name() const override { return u"identify_features_at"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;

  private:
    QgsProject *mProject = nullptr;
};

#endif // QGSAIATTRIBUTETABLETOOLS_H
