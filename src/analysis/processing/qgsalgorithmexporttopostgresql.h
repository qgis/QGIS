/***************************************************************************
                         qgsalgorithmexporttopostgresql.h
                         ---------------------
    begin                : November 2021
    copyright            : (C) 2021 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXPORTTOPOSTGRESQL_H
#define QGSALGORITHMEXPORTTOPOSTGRESQL_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayerexporter.h"

///@cond PRIVATE

/**
 * Native Export to PostgreSQL algorithm.
 */
class QgsExportToPostgresqlAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsExportToPostgresqlAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsExportToPostgresqlAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsProcessingFeatureSource> mSource;
    std::unique_ptr<QgsAbstractDatabaseProviderConnection> mConn;
    const QString mProviderName = u"postgres"_s;
    QString mSchema;
    QString mTable = QString();
    QString mPrimaryKeyField = u"id"_s;
    QString mGeomColumn;
    QString mEncoding = u"UTF-8"_s;
    bool mCreateIndex = true;
    bool mOverwrite = true;
    QMap<QString, QVariant> mOptions;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTTOPOSTGRESQL_H
