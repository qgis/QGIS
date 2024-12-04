/***************************************************************************
                         qgsalgorithmexportlayersinformation.h
                         ---------------------------------
    begin                : December 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXPORTLAYERSINFORMATION_H
#define QGSALGORITHMEXPORTLAYERSINFORMATION_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native export layers information algorithm.
 */
class QgsExportLayersInformationAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsExportLayersInformationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsExportLayersInformationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::vector<std::unique_ptr<QgsMapLayer>> mLayers;
    QgsCoordinateReferenceSystem mCrs;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTLAYERSINFORMATION_H
