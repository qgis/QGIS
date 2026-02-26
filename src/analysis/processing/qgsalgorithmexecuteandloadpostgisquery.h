/***************************************************************************
                         qgsalgorithmexecuteandloadpostgisquery.h
                         ------------------------------
    begin                : December 2025
    copyright            : (C) 2025 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXECUTEANDLOADPOSTGISQUERY_H
#define QGSALGORITHMEXECUTEANDLOADPOSTGISQUERY_H


#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native execute and load PostGIS query algorithm.
 */
class QgsExecuteAndLoadPostgisQueryAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsExecuteAndLoadPostgisQueryAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QgsExecuteAndLoadPostgisQueryAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE


#endif // QGSALGORITHMEXECUTEANDLOADPOSTGISQUERY_H
