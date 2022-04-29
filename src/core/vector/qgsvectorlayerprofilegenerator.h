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
class CORE_EXPORT QgsVectorLayerProfileResults : public QgsAbstractProfileResults
{
  public:

    QgsPointSequence rawPoints;
    QMap< double, double > mDistanceToHeightMap;

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

    double minZ = std::numeric_limits< double >::max();
    double maxZ = std::numeric_limits< double >::lowest();

    bool respectLayerSymbology = true;
    std::unique_ptr< QgsLineSymbol > profileLineSymbol;
    std::unique_ptr< QgsFillSymbol > profileFillSymbol;
    std::unique_ptr< QgsMarkerSymbol > profileMarkerSymbol;

    QString type() const override;
    QMap< double, double > distanceToHeightMap() const override;
    QgsDoubleRange zRange() const override;
    QgsPointSequence sampledPoints() const override;
    QVector< QgsGeometry > asGeometries() const override;
    QgsProfileSnapResult snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context ) override;
    void renderResults( QgsProfileRenderContext &context ) override;
    void copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator ) override;
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

    QString sourceId() const override;
    bool generateProfile( const QgsProfileGenerationContext &context = QgsProfileGenerationContext() ) override;
    QgsAbstractProfileResults *takeResults() override;
    QgsFeedback *feedback() const override;

  private:

    bool generateProfileForPoints();
    bool generateProfileForLines();
    bool generateProfileForPolygons();

    double terrainHeight( double x, double y );
    double featureZToHeight( double x, double y, double z, double offset );

    void clampAltitudes( QgsLineString *lineString, const QgsPoint &centroid, double offset );
    bool clampAltitudes( QgsPolygon *polygon, double offset );

    QString mId;
    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    std::unique_ptr< QgsCurve > mProfileCurve;
    std::unique_ptr< QgsGeos > mProfileCurveEngine;

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

    QgsExpressionContext mExpressionContext;
    QgsFields mFields;
    QgsPropertyCollection mDataDefinedProperties;

    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    QgsCoordinateTransform mLayerToTargetTransform;
    QgsCoordinateTransform mTargetToTerrainProviderTransform;

    std::unique_ptr< QgsVectorLayerProfileResults > mResults;

    bool mRespectLayerSymbology = true;
    std::unique_ptr< QgsLineSymbol > mProfileLineSymbol;
    std::unique_ptr< QgsFillSymbol > mProfileFillSymbol;
    std::unique_ptr< QgsMarkerSymbol > mProfileMarkerSymbol;

    // NOT for use in the background thread!
    QPointer< QgsVectorLayer > mLayer;

    friend class QgsVectorLayerProfileResults;

};

#endif // QGSVECTORLAYERPROFILEGENERATOR_H
