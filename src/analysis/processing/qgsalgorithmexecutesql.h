/***************************************************************************
                         qgsalgorithmexecutesql.h
                         ---------------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXECUTESQL_H
#define QGSALGORITHMEXECUTESQL_H


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native execute SQL using virtual layers algorithm.
 */
class QgsExecuteSqlAlgorithm : public QgsProcessingAlgorithm
{
  public:
    /**
   * Constructor for QgsExecuteSqlAlgorithm.
   */
    QgsExecuteSqlAlgorithm();

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsExecuteSqlAlgorithm *createInstance() const override SIP_FACTORY;
    Qgis::ProcessingAlgorithmFlags flags() const override;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::vector<std::pair<Qgis::WkbType, QString>> mGeometryTypes;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXECUTESQL_H
