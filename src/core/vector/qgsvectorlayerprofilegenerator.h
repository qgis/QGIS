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
#include "qgsabstractprofilesurfacegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransform.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsfeatureid.h"

#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;
class QgsAbstractTerrainProvider;
class QgsGeos;
class QgsLineString;
class QgsPolygon;
class QgsProfileSnapContext;

#define SIP_NO_FILE


/**
 * \brief Implementation of QgsAbstractProfileResults for vector layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVectorLayerProfileResults : public QgsAbstractProfileSurfaceResults
{
  public:

    struct Feature
    {
      //! Original feature ID
      QgsFeatureId featureId;
      //! Feature's geometry with any terrain height adjustment and extrusion applied
      QgsGeometry geometry;
      //! Cross section distance vs height geometry for feature
      QgsGeometry crossSectionGeometry;
    };

    QHash< QgsFeatureId, QVector< Feature > > features;
    QPointer< QgsVectorLayer > mLayer;

    Qgis::VectorProfileType profileType = Qgis::VectorProfileType::IndividualFeatures;
    bool respectLayerSymbology = true;
    std::unique_ptr< QgsMarkerSymbol > mMarkerSymbol;
    bool mShowMarkerSymbolInSurfacePlots = false;

    QString type() const override;
    QVector< QgsGeometry > asGeometries() const override;
    QVector< QgsAbstractProfileResults::Feature > asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback = nullptr ) const override;
    QgsProfileSnapResult snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context ) override;
    QVector<QgsProfileIdentifyResults> identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context ) override;
    QVector<QgsProfileIdentifyResults> identify( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange, const QgsProfileIdentifyContext &context ) override;
    void renderResults( QgsProfileRenderContext &context ) override;
    void copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator ) override;

  private:
    void renderResultsAsIndividualFeatures( QgsProfileRenderContext &context );
    void renderMarkersOverContinuousSurfacePlot( QgsProfileRenderContext &context );
    QVector< QgsAbstractProfileResults::Feature > asIndividualFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback = nullptr ) const;
    QgsProfileSnapResult snapPointToIndividualFeatures( const QgsProfilePoint &point, const QgsProfileSnapContext &context );

    void visitFeaturesAtPoint( const QgsProfilePoint &point, double maximumPointDistanceDelta, double maximumPointElevationDelta, double maximumSurfaceElevationDelta,
                               const std::function< void( QgsFeatureId, double delta, double distance, double elevation ) > &visitor, bool visitWithin );
    void visitFeaturesInRange( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange,
                               const std::function<void ( QgsFeatureId )> &visitor );
};


/**
 * \brief Implementation of QgsAbstractProfileGenerator for vector layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVectorLayerProfileGenerator : public QgsAbstractProfileSurfaceGenerator
{

  public:

    /**
     * Constructor for QgsVectorLayerProfileGenerator.
     */
    QgsVectorLayerProfileGenerator( QgsVectorLayer *layer, const QgsProfileRequest &request );

    ~QgsVectorLayerProfileGenerator() override;

    QString sourceId() const override;
    bool generateProfile( const QgsProfileGenerationContext &context = QgsProfileGenerationContext() ) override;
    QgsAbstractProfileResults *takeResults() override;
    QgsFeedback *feedback() const override;

  private:

    bool generateProfileForPoints();
    bool generateProfileForLines();
    bool generateProfileForPolygons();

    void processIntersectionPoint( const QgsPoint *intersectionPoint, const QgsFeature &feature );
    void processIntersectionCurve( const QgsLineString *intersectionCurve, const QgsFeature &feature );

    QgsPoint interpolatePointOnTriangle( const QgsPolygon *triangle, double x, double y ) const;
    void processTriangleIntersectForPoint( const QgsPolygon *triangle, const QgsPoint *intersect, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts );
    void processTriangleIntersectForLine( const QgsPolygon *triangle, const QgsLineString *intersect, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts );
    void processTriangleIntersectForPolygon( const QgsPolygon *triangle, const QgsPolygon *intersectionPolygon, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts );

    double terrainHeight( double x, double y );
    double featureZToHeight( double x, double y, double z, double offset );

    void clampAltitudes( QgsLineString *lineString, const QgsPoint &centroid, double offset );
    bool clampAltitudes( QgsPolygon *polygon, double offset );

    QString mId;
    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    std::unique_ptr< QgsCurve > mProfileCurve;
    std::unique_ptr< QgsGeos > mProfileCurveEngine;

    std::unique_ptr<QgsAbstractGeometry> mProfileBufferedCurve;
    std::unique_ptr< QgsGeos > mProfileBufferedCurveEngine;

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
    Qgis::VectorProfileType mType = Qgis::VectorProfileType::IndividualFeatures;
    Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Terrain;
    Qgis::AltitudeBinding mBinding = Qgis::AltitudeBinding::Centroid;
    bool mExtrusionEnabled = false;
    double mExtrusionHeight = 0;

    QgsExpressionContext mExpressionContext;
    QgsFields mFields;
    QgsPropertyCollection mDataDefinedProperties;

    Qgis::WkbType mWkbType = Qgis::WkbType::Unknown;
    QgsCoordinateTransform mLayerToTargetTransform;
    QgsCoordinateTransform mTargetToTerrainProviderTransform;

    std::unique_ptr< QgsVectorLayerProfileResults > mResults;

    bool mRespectLayerSymbology = true;
    std::unique_ptr< QgsMarkerSymbol > mProfileMarkerSymbol;
    bool mShowMarkerSymbolInSurfacePlots = false;

    // NOT for use in the background thread!
    QPointer< QgsVectorLayer > mLayer;

    friend class QgsVectorLayerProfileResults;

};

#endif // QGSVECTORLAYERPROFILEGENERATOR_H
