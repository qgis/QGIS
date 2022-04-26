/***************************************************************************
                         qgsrasterlayerprofilegenerator.h
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
#ifndef QGSRASTERLAYERPROFILEGENERATOR_H
#define QGSRASTERLAYERPROFILEGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractprofilegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsrasterlayerelevationproperties.h"

#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsRasterLayer;
class QgsRasterDataProvider;
class QgsRasterBlockFeedback;
class QgsLineSymbol;
class QgsProfileSnapContext;

#define SIP_NO_FILE

/**
 * \brief Implementation of QgsAbstractProfileResults for raster layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRasterLayerProfileResults : public QgsAbstractProfileResults
{

  public:

    QgsPointSequence rawPoints;
    QMap< double, double > results;

    double minZ = std::numeric_limits< double >::max();
    double maxZ = std::numeric_limits< double >::lowest();

    Qgis::ProfileSurfaceSymbology symbology = Qgis::ProfileSurfaceSymbology::Line;
    std::unique_ptr< QgsLineSymbol > lineSymbol;
    std::unique_ptr< QgsFillSymbol > fillSymbol;

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
 * \brief Implementation of QgsAbstractProfileGenerator for raster layers.
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRasterLayerProfileGenerator : public QgsAbstractProfileGenerator
{

  public:

    /**
     * Constructor for QgsRasterLayerProfileGenerator.
     */
    QgsRasterLayerProfileGenerator( QgsRasterLayer *layer, const QgsProfileRequest &request );

    ~QgsRasterLayerProfileGenerator() override;

    QString sourceId() const override;
    bool generateProfile() override;
    QgsAbstractProfileResults *takeResults() override;
    QgsFeedback *feedback() const override;

  private:
    QString mId;
    std::unique_ptr<QgsRasterBlockFeedback> mFeedback = nullptr;

    std::unique_ptr< QgsCurve > mProfileCurve;

    Qgis::ProfileSurfaceSymbology mSymbology = Qgis::ProfileSurfaceSymbology::Line;
    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    std::unique_ptr< QgsFillSymbol > mFillSymbol;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    double mOffset = 0;
    double mScale = 1;

    std::unique_ptr< QgsRasterDataProvider > mRasterProvider;

    std::unique_ptr< QgsRasterLayerProfileResults > mResults;

    int mBand = 1;
    double mRasterUnitsPerPixelX = 1;
    double mRasterUnitsPerPixelY = 1;

    double mStepDistance = std::numeric_limits<double>::quiet_NaN();

    friend class QgsRasterLayerProfileResults;

};

#endif // QGSRASTERLAYERPROFILEGENERATOR_H
