/***************************************************************************
                         qgsalgorithmrandompointsinpolygons.h
                         ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Håvard Tveite
    email                : havard dot tveite at nmbu dot no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSALGORITHMRANDOMPOINTSINPOLYGONS_H
#define QGSALGORITHMRANDOMPOINTSINPOLYGONS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native random points in polygons creation algorithm.
 */
class QgsRandomPointsInPolygonsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRandomPointsInPolygonsAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomPointsInPolygons.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomPointsInPolygons.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsRandomPointsInPolygonsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context,
                                  QgsProcessingFeedback *feedback ) override;

  private:
    int mNumPoints;
    bool mDynamicNumPoints = false;
    QgsProperty mNumPointsProperty;

    double mMinDistance = 0;
    bool mDynamicMinDistance = false;
    QgsProperty mMinDistanceProperty;

    double mMinDistanceGlobal = 0;

    int mMaxAttempts = 10;
    bool mDynamicMaxAttempts = false;
    QgsProperty mMaxAttemptsProperty;

    bool mUseRandomSeed = false;
    int mRandSeed = 0;
    bool mIncludePolygonAttr = false;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRANDOMPOINTSINPOLYGONS_H
