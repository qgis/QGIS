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
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native random points in polygons creation algorithm.
 */
class QgsRandomPointsInPolygonsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRandomPointsInPolygonsAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmRandomPointsInPolygons.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmRandomPointsInPolygons.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsRandomPointsInPolygonsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    // The algorithm parameter names:
    static inline const QString INPUT = u"INPUT"_s;
    static inline const QString POINTS_NUMBER = u"POINTS_NUMBER"_s;
    static inline const QString MIN_DISTANCE_GLOBAL = u"MIN_DISTANCE_GLOBAL"_s;
    static inline const QString MIN_DISTANCE = u"MIN_DISTANCE"_s;
    static inline const QString MAX_TRIES_PER_POINT = u"MAX_TRIES_PER_POINT"_s;
    static inline const QString SEED = u"SEED"_s;
    static inline const QString INCLUDE_POLYGON_ATTRIBUTES = u"INCLUDE_POLYGON_ATTRIBUTES"_s;
    static inline const QString OUTPUT = u"OUTPUT"_s;
    static inline const QString OUTPUT_POINTS = u"OUTPUT_POINTS"_s;
    static inline const QString POINTS_MISSED = u"POINTS_MISSED"_s;
    static inline const QString POLYGONS_WITH_MISSED_POINTS = u"POLYGONS_WITH_MISSED_POINTS"_s;
    static inline const QString FEATURES_WITH_EMPTY_OR_NO_GEOMETRY = u"FEATURES_WITH_EMPTY_OR_NO_GEOMETRY"_s;

    int mNumPoints = 0;
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
