/***************************************************************************
                         qgsalgorithmpdalexportraster.h
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#ifndef QGSALGORITHMPDALEXPORTRASTER_H
#define QGSALGORITHMPDALEXPORTRASTER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgspdalalgorithmbase.h"

///@cond PRIVATE

/**
 * Native point cloud export to raster algorithm.
 */
class QgsPdalExportRasterAlgorithm : public QgsPdalAlgorithmBase
{
  public:
    QgsPdalExportRasterAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsPdalExportRasterAlgorithm *createInstance() const override SIP_FACTORY;

    QStringList createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    friend class TestQgsProcessingPdalAlgs;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPDALEXPORTRASTER_H
