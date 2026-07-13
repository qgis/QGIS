/***************************************************************************
                         qgsalgorithmcoverageclean.h
                         ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Juho Ervasti
    email                : juho dot ervasti at gispo dot fi
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCOVERAGECLEAN_H
#define QGSALGORITHMCOVERAGECLEAN_H


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native coverage cleaning algorithm.
 */
class QgsCoverageCleanAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsCoverageCleanAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsCoverageCleanAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};


///@endcond PRIVATE

#endif // QGSALGORITHMCOVERAGECLEAN_H
