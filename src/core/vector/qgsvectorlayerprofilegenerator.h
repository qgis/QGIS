/***************************************************************************
                         qgsvectorlayerprofilegenerator.h
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSVECTORLAYERPROFILEGENERATOR_H
#define QGSVECTORLAYERPROFILEGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractprofilegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransform.h"

#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;
class QgsAbstractTerrainProvider;

#define SIP_NO_FILE


/**
 * \brief Implementation of QgsAbstractProfileResults for vector layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVectorLayerProfileResults : public QgsAbstractProfileResults
{

  public:

    // Temporary class only!
    struct Result
    {
      double distance;
      double height;
    };

    QgsPointSequence rawPoints;
    QList< Result > results;
    QVector< QgsGeometry > geometries;

    QString type() const override;
    QHash< double, double > distanceToHeightMap() const override;
    QgsPointSequence sampledPoints() const override;
    QVector< QgsGeometry > asGeometries() const override;
};


/**
 * \brief Implementation of QgsAbstractProfileGenerator for vector layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVectorLayerProfileGenerator : public QgsAbstractProfileGenerator
{

  public:

    /**
     * Constructor for QgsVectorLayerProfileGenerator.
     */
    QgsVectorLayerProfileGenerator( QgsVectorLayer *layer, const QgsProfileRequest &request );

    ~QgsVectorLayerProfileGenerator() override;

    bool generateProfile() override;
    QgsAbstractProfileResults *takeResults() override;

  private:

    bool generateProfileForPoints();

    double featureZToHeight( double x, double y, double z );

    std::unique_ptr< QgsCurve > mProfileCurve;
    std::unique_ptr< QgsAbstractTerrainProvider > mTerrainProvider;

    std::unique_ptr< QgsCurve > mTransformedCurve;
    double mTolerance = 0;


    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;
    QgsRectangle mExtent;

    std::unique_ptr< QgsVectorLayerFeatureSource > mSource;

    double mOffset = 0;
    double mScale = 1;
    Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Terrain;
    Qgis::AltitudeBinding mBinding = Qgis::AltitudeBinding::Centroid;
    bool mExtrusionEnabled = false;
    double mExtrusionHeight = 0;

    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    QgsCoordinateTransform mLayerToTargetTransform;
    QgsCoordinateTransform mTargetToTerrainProviderTransform;

    std::unique_ptr< QgsVectorLayerProfileResults > mResults;


};

#endif // QGSVECTORLAYERPROFILEGENERATOR_H
