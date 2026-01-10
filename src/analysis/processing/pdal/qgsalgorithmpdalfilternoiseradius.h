/***************************************************************************
                         qgsalgorithmpdalfilternoiseradius.h
                         ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMPDALFILTERNOISERADIUS_H
#define QGSALGORITHMPDALFILTERNOISERADIUS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgspdalalgorithmbase.h"

///@cond PRIVATE

/**
 * Native point cloud filter noise using radius algorithm.
 */
class QgsPdalFilterNoiseRadiusAlgorithm : public QgsPdalAlgorithmBase
{
  public:
    QgsPdalFilterNoiseRadiusAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsPdalFilterNoiseRadiusAlgorithm *createInstance() const override SIP_FACTORY;

    QStringList createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    friend class TestQgsProcessingPdalAlgs;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPDALFILTERNOISERADIUS_H
