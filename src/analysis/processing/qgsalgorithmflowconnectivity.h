/***************************************************************************
                         qgsalgorithmflowconnectivity.h
                         ---------------------
    begin                : September 2026
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

#ifndef QGSALGORITHMFLOWCONNECTIVITY_H
#define QGSALGORITHMFLOWCONNECTIVITY_H


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native flow connectivity calculation algorithm, based on SAGA's "Channel Network and Drainage Basins" tool.
 */
class QgsFlowConnectivityD8Algorithm : public QgsProcessingAlgorithm
{
  public:
    QgsFlowConnectivityD8Algorithm() = default;

    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QList<QgsAcademicReference> academicReferences() const override;
    QList<QgsProcessingAlgorithm::ExternalLink> externalLinks() const override;
    QString group() const override;
    QString groupId() const override;
    QStringList tags() const override;
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mInterface;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    double mCellSizeX = 0.0;
    double mCellSizeY = 0.0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
};
///@endcond PRIVATE

#endif // QGSALGORITHMFLOWCONNECTIVITY_H
