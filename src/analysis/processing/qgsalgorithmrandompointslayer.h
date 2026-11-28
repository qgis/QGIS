/***************************************************************************
                         qgsalgorithmrandompointslayer.h
                         ---------------------
    begin                : November 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#ifndef QGSALGORITHMRANDOMPOINTSLAYER_H
#define QGSALGORITHMRANDOMPOINTSLAYER_H

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

class QgsSpatialIndex;

using namespace Qt::StringLiterals;

///@cond PRIVATE

/**
 * Native random points in layer bounds algorithm.
 */
class QgsRandomPointsLayerAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRandomPointsLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmRandomPointsWithinExtent.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmRandomPointsWithinExtent.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsRandomPointsLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    bool isWithinLayerBounds( const QgsGeometry &geom, const QgsSpatialIndex &sourceIndex, QgsProcessingFeedback *feedback ) const;

    std::unique_ptr<QgsFeatureSource> mSource;
    QgsRectangle mExtent;
    int mNumPoints = 0;
    double mDistance = 0;
    int mMaxAttempts = 0;
};


///@endcond PRIVATE

#endif // QGSALGORITHMRANDOMPOINTSLAYER_H
