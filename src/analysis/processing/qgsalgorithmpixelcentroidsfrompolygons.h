/***************************************************************************
                         qgsalgorithmpixelcentroidsfrompolygons.h
                         ------------------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#ifndef QGSALGORITHMPIXELCENTROIDSFROMPOLYGONS_H
#define QGSALGORITHMPIXELCENTROIDSFROMPOLYGONS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native pixel centroids inside polygons algorithm.
 */
class QgsPixelCentroidsFromPolygonsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsPixelCentroidsFromPolygonsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsPixelCentroidsFromPolygonsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPIXELCENTROIDSFROMPOLYGONS_H
