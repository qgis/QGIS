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


#include <geos_c.h>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsabstractprofilesurfacegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsgeos.h"
#include "qgslinesymbol.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudsubindex.h"
#include "qgsvector3d.h"

class QgsProfileRequest;
class QgsCurve;
class QgsPointCloudLayer;
class QgsAbstractTerrainProvider;
class QgsProfileSnapContext;
class QgsPointCloudRenderer;
class QgsPointCloudNodeId;
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
    Qgis::RenderUnit pointSizeUnit = Qgis::RenderUnit::Millimeters;
    Qgis::PointCloudSymbol pointSymbol = Qgis::PointCloudSymbol::Square;
    bool respectLayerColors = true;
    QColor pointColor;
    bool opacityByDistanceEffect = false;

    QString type() const override;
    QMap< double, double > distanceToHeightMap() const override;
    QgsDoubleRange zRange() const override;
    QgsPointSequence sampledPoints() const override;
    QVector< QgsGeometry > asGeometries() const override;
    QVector<  QgsAbstractProfileResults::Feature > asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback = nullptr ) const override;
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
    QString mLayerId;

    friend class QgsPointCloudLayerProfileGenerator;
};


/**
 * \brief Base class for the point cloud elevation profile generators.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsPointCloudLayerProfileGeneratorBase : public QgsAbstractProfileGenerator
{
  public:
    /**
     * Constructor for QgsPointCloudLayerProfileGeneratorBase.
     */
    QgsPointCloudLayerProfileGeneratorBase( QgsPointCloudLayer *layer, const QgsProfileRequest &request );
    virtual ~QgsPointCloudLayerProfileGeneratorBase() override;

  protected:

    /**
     * Collects point cloud data along a curve within the specified range and tolerance.
     *
     * Prepares attribute requests and renderer data,
     * then gathers points from relevant point cloud indexes.
     * Updates the maximum allowable error in layer coordinates.
     *
     * \param curve Search geometry.
     * \param mapUnitsPerPixel Number of map units per pixel in the distance dimension.
     * \param maximumErrorPixels Maximum permissible projected error in pixels.
     * \param zRange Vertical range filter.
     * \param maxErrorInLayerCoordinates Output parameter for the derived maximum error in layer CRS.
     * \return True on success, false if canceled or an error occured.
     */
    bool collectData( QgsGeos &curve, const double mapUnitsPerPixel, const double maximumErrorPixels, const QgsDoubleRange &zRange, double &maxErrorInLayerCrs );

    /**
     * Collects points from the point cloud index within the given extent and Z range.
     *
     * Traverses the node tree to identify relevant nodes, then loads and processes them
     * either synchronously or asynchronously depending on the access type.
     *
     * \param pc Pint cloud index.
     * \param request Point cloud data request item.
     * \param maxErrorPixels Maximum allowed projected error in pixels.
     * \param nodeErrorPixels Node's projected error in pixels.
     * \param zRange Vertical range filter.
     * \param searchExtent Search extent.
     */
    void gatherPoints( QgsPointCloudIndex &pc, QgsPointCloudRequest &request, double maxErrorPixels, double nodeErrorPixels, const QgsDoubleRange &zRange, const QgsRectangle &searchExtent );

    /**
     * Recursively traverses the point cloud node tree to gather visible nodes.
     *
     * Filters nodes based on Z range, spatial intersection with the search extent,
     * and screen-space error threshold. Returns all nodes that meet the criteria
     * and contain points.
     *
     * \param pc Point cloud index.
     * \param n Node ID to evaluate.
     * \param maxErrorPixels Maximum allowed projected error in pixels.
     * \param nodeErrorPixels Node's projected error in pixels.
     * \param zRange Vertical range filter.
     * \param searchExtent Search extent.
     * \return List of nodes that satisfy the given constraints.
     */
    QVector<QgsPointCloudNodeId> traverseTree( QgsPointCloudIndex &pc, QgsPointCloudNodeId n, double maxErrorPixels, double nodeErrorPixels, const QgsDoubleRange &zRange, const QgsRectangle &searchExtent );

    /**
     * Synchronously visits point cloud nodes and processes their data.
     *
     * Loads each node’s block data sequentially, calls visitBlock() for valid blocks
     * within the given Z range.
     *
     * \param nodes Nodes to process.
     * \param pc Point cloud index.
     * \param request Point cloud data request item.
     * \param zRange Vertical range for filtering points.
     * \return Number of successfully processed nodes.
     */
    int visitNodesSync( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRequest &request, const QgsDoubleRange &zRange );

    /**
     * Asynchronously visits point cloud nodes, loading their data and processing valid blocks.
     *
     * Requests node data asynchronously, waits for completion and calls visitBlock() for each
     * successfully loaded block within the specified Z range.
     *
     * \param nodes List of nodes to process.
     * \param pc Point cloud index.
     * \param request Point cloud data request item.
     * \param zRange Vertical range for filtering points.
     * \return Number of successfully processed nodes.
     */
    int visitNodesAsync( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc,  QgsPointCloudRequest &request, const QgsDoubleRange &zRange );

    /**
     * Visits a point cloud block and collects points within the given Z range and search geometry.
     *
     * Extracts point coordinates from the block, optionally filters them using the renderer,
     * handled reprojection if needed and adds the valid points to results.
     *
     * \param block Point cloud block to process.
     * \param zRange Vertical range to include points within.
     */
    virtual void visitBlock( const QgsPointCloudBlock *block, const QgsDoubleRange &zRange ) = 0;

    QPointer< QgsPointCloudLayer > mLayer;
    QgsPointCloudIndex mIndex;
    const QVector< QgsPointCloudSubIndex > mSubIndexes;
    QgsPointCloudAttributeCollection mLayerAttributes;
    std::unique_ptr< QgsPointCloudRenderer > mRenderer;

    double mMaximumScreenError = 0.3;
    Qgis::RenderUnit mMaximumScreenErrorUnit = Qgis::RenderUnit::Millimeters;

    QString mId;

    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    double mTolerance = 0;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    double mStepDistance = std::numeric_limits<double>::quiet_NaN();

    double mZOffset = 0;
    double mZScale = 1.0;

    QgsCoordinateTransform mLayerToTargetTransform;
    std::unique_ptr< QgsCurve > mProfileCurve;

    std::unique_ptr< QgsPreparedPointCloudRendererData > mPreparedRendererData;
    std::unique_ptr< QgsGeos > mSearchGeometryInLayerCrsGeometryEngine;
};

/**
 * \brief Implementation of QgsAbstractProfileGenerator for point cloud layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudLayerProfileGenerator : public QgsPointCloudLayerProfileGeneratorBase
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
    QString type() const override;

  private:
    void visitBlock( const QgsPointCloudBlock *block, const QgsDoubleRange &zRange ) override;

    double mPointSize = 1;
    Qgis::RenderUnit mPointSizeUnit = Qgis::RenderUnit::Millimeters;
    Qgis::PointCloudSymbol mPointSymbol = Qgis::PointCloudSymbol::Square;
    QColor mPointColor;
    bool mOpacityByDistanceEffect = false;

    std::unique_ptr< QgsAbstractGeometry > mSearchGeometryInLayerCrs;
    std::unique_ptr< QgsPointCloudLayerProfileResults > mResults;
    QVector< QgsPointCloudLayerProfileResults::PointResult > mGatheredPoints;

    friend class QgsPointCloudLayerProfileResults;
};

/**
 * \brief Implementation of QgsAbstractProfileSurfaceResults for triangulated point cloud layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsTriangulatedPointCloudLayerProfileResults : public QgsAbstractProfileSurfaceResults
{

  public:

    QString type() const override;
    using QgsAbstractProfileSurfaceResults::identify;
    QVector<QgsProfileIdentifyResults> identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context ) override;
    void copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator ) override;
    void renderResults( QgsProfileRenderContext &context ) override;

  private:
    QPointer< QgsPointCloudLayer > mLayer;
    QString mLayerId;
    QgsCoordinateReferenceSystem mCurveCrs;
    std::unique_ptr< QgsCurve > mProfileCurve;
    double mTolerance = 0.0;
    double mZOffset = 0.0;
    double mZScale = 1.0;

    friend class QgsTriangulatedPointCloudLayerProfileGenerator;
};


/**
 * \brief Implementation of QgsAbstractProfileGenerator for triangulated point cloud layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsTriangulatedPointCloudLayerProfileGenerator : public QgsPointCloudLayerProfileGeneratorBase
{

  public:

    /**
     * Constructor for QgsTriangulatedPointCloudLayerProfileGenerator.
     */
    QgsTriangulatedPointCloudLayerProfileGenerator( QgsPointCloudLayer *layer, const QgsProfileRequest &request );

    ~QgsTriangulatedPointCloudLayerProfileGenerator() override;

    QString sourceId() const override;
    bool generateProfile( const QgsProfileGenerationContext &context ) override;
    QgsAbstractProfileResults *takeResults() override;
    QgsFeedback *feedback() const override;
    QString type() const override;

  private:
    void visitBlock( const QgsPointCloudBlock *block, const QgsDoubleRange &zRange ) override;

    std::unique_ptr< QgsTriangulatedPointCloudLayerProfileResults > mResults;
    std::unique_ptr< QgsAbstractGeometry > mSearchGeometryInLayerCrs;
    QVector< QgsPoint > mGatheredPoints;

    Qgis::ProfileSurfaceSymbology mSymbology = Qgis::ProfileSurfaceSymbology::Line;
    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    std::unique_ptr< QgsFillSymbol > mFillSymbol;
    double mElevationLimit = std::numeric_limits< double >::quiet_NaN();

    friend class QgsTriangulatedPointCloudLayerProfileResults;
};


#endif // QGSPOINTCLOUDLAYERPROFILEGENERATOR_H
