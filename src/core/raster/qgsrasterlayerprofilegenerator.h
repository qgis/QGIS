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

#include <memory>

class QgsProfileRequest;
class QgsCurve;
class QgsRasterLayer;
class QgsRasterDataProvider;


#define SIP_NO_FILE

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

    bool generateProfile() override;

  private:

    std::unique_ptr< QgsCurve > mProfileCurve;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    double mOffset = 0;
    double mScale = 1;

    std::unique_ptr< QgsRasterDataProvider > mRasterProvider;

    int mBand = 1;
    double mRasterUnitsPerPixelX = 1;
    double mRasterUnitsPerPixelY = 1;


};

#endif // QGSRASTERLAYERPROFILEGENERATOR_H
