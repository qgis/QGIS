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
#include "qgsabstractprofilesurfacegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransform.h"
#include "qgstriangularmesh.h"

#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsMeshLayer;
class QgsAbstractTerrainProvider;
class QgsProfileSnapContext;

#define SIP_NO_FILE


/**
 * \brief Implementation of QgsAbstractProfileResults for mesh layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMeshLayerProfileResults : public QgsAbstractProfileSurfaceResults
{

  public:

    QString type() const override;
    QVector<QgsProfileIdentifyResults> identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context ) override;

  private:

    QPointer< QgsMeshLayer > mLayer;

    friend class QgsMeshLayerProfileGenerator;
};


/**
 * \brief Implementation of QgsAbstractProfileGenerator for mesh layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMeshLayerProfileGenerator : public QgsAbstractProfileSurfaceGenerator
{

  public:

    /**
     * Constructor for QgsMeshLayerProfileGenerator.
     */
    QgsMeshLayerProfileGenerator( QgsMeshLayer *layer, const QgsProfileRequest &request );

    ~QgsMeshLayerProfileGenerator() override;

    QString sourceId() const override;
    bool generateProfile( const QgsProfileGenerationContext &context = QgsProfileGenerationContext() ) override;
    QgsAbstractProfileResults *takeResults() override;
    QgsFeedback *feedback() const override;

  private:

    double heightAt( double x, double y );

    QString mId;
    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    std::unique_ptr< QgsCurve > mProfileCurve;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    double mOffset = 0;
    double mScale = 1;
    QPointer< QgsMeshLayer > mLayer;

    double mStepDistance = std::numeric_limits<double>::quiet_NaN();

    QgsTriangularMesh mTriangularMesh;

    QgsCoordinateTransform mLayerToTargetTransform;

    std::unique_ptr< QgsMeshLayerProfileResults > mResults;

    friend class QgsMeshLayerProfileResults;


};

#endif // QGSMESHLAYERPROFILEGENERATOR_H
