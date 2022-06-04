/***************************************************************************
                         qgspointcloudlayerprofilegenerator.h
                         ---------------
    begin                : April 2022
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
#ifndef QGSPOINTCLOUDLAYERPROFILEGENERATOR_H
#define QGSPOINTCLOUDLAYERPROFILEGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractprofilegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransform.h"
#include "qgspointcloudattribute.h"
#include "qgslinesymbol.h"
#include "qgsvector3d.h"
#include "qgsgeos.h"

#include <geos_c.h>
#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsPointCloudLayer;
class QgsAbstractTerrainProvider;
class QgsProfileSnapContext;
class QgsPointCloudRenderer;
class IndexedPointCloudNode;
class QgsPointCloudIndex;
class QgsPointCloudRequest;
class QgsPointCloudBlock;
class QgsGeos;
class QgsPreparedPointCloudRendererData;

#define SIP_NO_FILE


/**
 * \brief Implementation of QgsAbstractProfileResults for point cloud layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudLayerProfileResults : public QgsAbstractProfileResults
{

  public:

    QgsPointCloudLayerProfileResults();
    ~QgsPointCloudLayerProfileResults() override;

    struct PointResult
    {
      double x = 0;
      double y = 0;
      double z = 0;
      double distanceAlongCurve = 0;
      double distanceFromCurve = 0; // only used when the opacity by distance effect is enabled
      QRgb color;
    };

    /**
     * Finalizes results -- should be called after last point is added.
     */
    void finalize( QgsFeedback *feedback );

    std::vector< PointResult > results;
    double tolerance = 0;

    double minZ = std::numeric_limits< double >::max();
    double maxZ = std::numeric_limits< double >::lowest();

    double pointSize = 1;
    QgsUnitTypes::RenderUnit pointSizeUnit = QgsUnitTypes::RenderMillimeters;
    Qgis::PointCloudSymbol pointSymbol = Qgis::PointCloudSymbol::Square;
    bool respectLayerColors = true;
    QColor pointColor;
    bool opacityByDistanceEffect = false;

    QString type() const override;
    QMap< double, double > distanceToHeightMap() const override;
    QgsDoubleRange zRange() const override;
    QgsPointSequence sampledPoints() const override;
    QVector< QgsGeometry > asGeometries() const override;
    void renderResults( QgsProfileRenderContext &context ) override;
    QgsProfileSnapResult snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context ) override;
    QVector<QgsProfileIdentifyResults> identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context ) override;
    QVector<QgsProfileIdentifyResults> identify( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange, const QgsProfileIdentifyContext &context ) override;
    void copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator ) override;

  private:

    GEOSSTRtree *mPointIndex = nullptr;
    QPointer< QgsPointCloudLayer > mLayer;
    QgsCoordinateReferenceSystem mCurveCrs;
    std::unique_ptr< QgsCurve > mProfileCurve;
    double mTolerance = 0;
    double mZOffset = 0;
    double mZScale = 1.0;
    double mMaxErrorInLayerCoordinates = 0;

    // only required for GEOS < 3.9
#if GEOS_VERSION_MAJOR<4 && GEOS_VERSION_MINOR<9
    std::vector< geos::unique_ptr > mSTRTreeItems;
#endif

    friend class QgsPointCloudLayerProfileGenerator;
};


/**
 * \brief Implementation of QgsAbstractProfileGenerator for point cloud layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudLayerProfileGenerator : public QgsAbstractProfileGenerator
{

  public:

    /**
     * Constructor for QgsPointCloudLayerProfileGenerator.
     */
    QgsPointCloudLayerProfileGenerator( QgsPointCloudLayer *layer, const QgsProfileRequest &request );

    ~QgsPointCloudLayerProfileGenerator() override;

    QString sourceId() const override;
    Qgis::ProfileGeneratorFlags flags() const override;
    bool generateProfile( const QgsProfileGenerationContext &context = QgsProfileGenerationContext() ) override;
    QgsAbstractProfileResults *takeResults() override;
    QgsFeedback *feedback() const override;

  private:
    QVector<IndexedPointCloudNode> traverseTree( const QgsPointCloudIndex *pc, IndexedPointCloudNode n, double maxErrorPixels, double nodeErrorPixels, const QgsDoubleRange &zRange );
    int visitNodesSync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRequest &request, const QgsDoubleRange &zRange );
    int visitNodesAsync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc,  QgsPointCloudRequest &request, const QgsDoubleRange &zRange );
    void visitBlock( const QgsPointCloudBlock *block, const QgsDoubleRange &zRange );

    QPointer< QgsPointCloudLayer > mLayer;
    QgsPointCloudAttributeCollection mLayerAttributes;
    std::unique_ptr< QgsPointCloudRenderer > mRenderer;
    double mMaximumScreenError = 0.3;
    QgsUnitTypes::RenderUnit mMaximumScreenErrorUnit = QgsUnitTypes::RenderMillimeters;

    double mPointSize = 1;
    QgsUnitTypes::RenderUnit mPointSizeUnit = QgsUnitTypes::RenderMillimeters;
    Qgis::PointCloudSymbol mPointSymbol = Qgis::PointCloudSymbol::Square;
    QColor mPointColor;
    bool mOpacityByDistanceEffect = false;

    QString mId;
    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    std::unique_ptr< QgsCurve > mProfileCurve;

    double mTolerance = 0;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    QgsVector3D mScale;
    QgsVector3D mOffset;
    double mZOffset = 0;
    double mZScale = 1.0;

    double mStepDistance = std::numeric_limits<double>::quiet_NaN();

    QgsCoordinateTransform mLayerToTargetTransform;

    std::unique_ptr< QgsAbstractGeometry > mSearchGeometryInLayerCrs;
    std::unique_ptr< QgsGeos > mSearchGeometryInLayerCrsGeometryEngine;
    QgsRectangle mMaxSearchExtentInLayerCrs;

    std::unique_ptr< QgsPreparedPointCloudRendererData > mPreparedRendererData;

    std::unique_ptr< QgsPointCloudLayerProfileResults > mResults;
    QVector< QgsPointCloudLayerProfileResults::PointResult > mGatheredPoints;

    friend class QgsPointCloudLayerProfileResults;

};

#endif // QGSPOINTCLOUDLAYERPROFILEGENERATOR_H
