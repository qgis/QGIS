/***************************************************************************
                         qgsalgorithmminimumboundinggeometry.h
                         ---------------------
    begin                : May 2025
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

#ifndef QGSALGORITHMMINIMUMBOUNDINGGEOMETRY_H
#define QGSALGORITHMMINIMUMBOUNDINGGEOMETRY_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native minimum bounding geometry algorithm.
 */
class QgsMinimumBoundingGeometryAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsMinimumBoundingGeometryAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmConvexHull.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmConvexHull.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsMinimumBoundingGeometryAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsFeature createFeature( QgsProcessingFeedback *feedback, const int featureId, const int featureType, QVector<QgsGeometry> geometries, QVariant classField = QVariant() );
};

///@endcond PRIVATE

#endif // QGSALGORITHMMINIMUMBOUNDINGGEOMETRY_H
