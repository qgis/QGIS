/***************************************************************************
                         qgsmeshlayerprofilegenerator.h
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
#ifndef QGSMESHLAYERPROFILEGENERATOR_H
#define QGSMESHLAYERPROFILEGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractprofilegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransform.h"
#include "qgstriangularmesh.h"

#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsMeshLayer;
class QgsAbstractTerrainProvider;

#define SIP_NO_FILE


/**
 * \brief Implementation of QgsAbstractProfileResults for mesh layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMeshLayerProfileResults : public QgsAbstractProfileResults
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

    QString type() const override;
    QMap< double, double > distanceToHeightMap() const override;
    QgsPointSequence sampledPoints() const override;
    QVector< QgsGeometry > asGeometries() const override;
};


/**
 * \brief Implementation of QgsAbstractProfileGenerator for mesh layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMeshLayerProfileGenerator : public QgsAbstractProfileGenerator
{

  public:

    /**
     * Constructor for QgsMeshLayerProfileGenerator.
     */
    QgsMeshLayerProfileGenerator( QgsMeshLayer *layer, const QgsProfileRequest &request );

    ~QgsMeshLayerProfileGenerator() override;

    bool generateProfile() override;
    QgsAbstractProfileResults *takeResults() override;

  private:

    double heightAt( double x, double y );

    std::unique_ptr< QgsCurve > mProfileCurve;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    double mStepDistance = std::numeric_limits<double>::quiet_NaN();

    QgsTriangularMesh mTriangularMesh;

    QgsCoordinateTransform mLayerToTargetTransform;

    std::unique_ptr< QgsMeshLayerProfileResults > mResults;


};

#endif // QGSMESHLAYERPROFILEGENERATOR_H
