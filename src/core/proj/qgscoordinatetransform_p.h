/***************************************************************************
               qgscoordinatetransform_p.h
               --------------------------
    begin                : July 2016
    copyright            : (C) 2016 Nyall Dawson
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
#ifndef QGSCOORDINATETRANSFORMPRIVATE_H
#define QGSCOORDINATETRANSFORMPRIVATE_H

#define SIP_NO_FILE
#include "qgsconfig.h"

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QSharedData>

struct PJconsts;
typedef struct PJconsts PJ;
typedef PJ *ProjData;

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"

class QgsCoordinateTransformPrivate : public QSharedData
{

  public:

    explicit QgsCoordinateTransformPrivate();

    QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source,
                                   const QgsCoordinateReferenceSystem &destination,
                                   const QgsCoordinateTransformContext &context );

    QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source,
                                   const QgsCoordinateReferenceSystem &destination,
                                   int sourceDatumTransform,
                                   int destDatumTransform );

    QgsCoordinateTransformPrivate( const QgsCoordinateTransformPrivate &other );

    ~QgsCoordinateTransformPrivate();

    bool checkValidity();

    void invalidate();

    bool initialize();

    void calculateTransforms( const QgsCoordinateTransformContext &context );

    ProjData threadLocalProjData();

    int mAvailableOpCount = -1;
    ProjData threadLocalFallbackProjData();

    // Only meant to be called by QgsCoordinateTransform::removeFromCacheObjectsBelongingToCurrentThread()
    bool removeObjectsBelongingToCurrentThread( void *pj_context );

    /**
     * Flag to indicate whether the transform is valid (ie has a valid
     * source and destination crs)
     */
    bool mIsValid = false;

    /**
     * Flag to indicate that the source and destination coordinate systems are
     * equal and not transformation needs to be done
     */
    bool mShortCircuit = false;

    //! Flag to indicate EPSG:4326 to EPSG:3857 reprojection
    bool mGeographicToWebMercator = false;

    //! QgsCoordinateReferenceSystem of the source (layer) coordinate system
    QgsCoordinateReferenceSystem mSourceCRS;

    //! QgsCoordinateReferenceSystem of the destination (map canvas) coordinate system
    QgsCoordinateReferenceSystem mDestCRS;

    Q_DECL_DEPRECATED QString mSourceProjString;
    Q_DECL_DEPRECATED QString mDestProjString;

    Q_DECL_DEPRECATED int mSourceDatumTransform = -1;
    Q_DECL_DEPRECATED int mDestinationDatumTransform = -1;
    QString mProjCoordinateOperation;
    bool mShouldReverseCoordinateOperation = false;
    bool mAllowFallbackTransforms = true;

    bool mSourceIsDynamic = false;
    bool mDestIsDynamic = false;
    double mSourceCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();
    double mDestCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();
    double mDefaultTime = std::numeric_limits< double >::quiet_NaN();

    //! True if the proj transform corresponds to the reverse direction, and must be flipped when transforming...
    bool mIsReversed = false;

    QReadWriteLock mProjLock;
    QMap < uintptr_t, ProjData > mProjProjections;
    QMap < uintptr_t, ProjData > mProjFallbackProjections;

    /**
     * Sets a custom handler to use when a coordinate transform is created between \a sourceCrs and
     * \a destinationCrs, yet the coordinate operation requires a transform \a grid which is not present
     * on the system.
     *
     * \since QGIS 3.8
     */
    static void setCustomMissingRequiredGridHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QgsDatumTransform::GridDetails &grid )> &handler );

    /**
     * Sets a custom handler to use when a coordinate transform is created between \a sourceCrs and
     * \a destinationCrs, yet a preferred (more accurate?) operation is available which could not
     * be created on the system (e.g. due to missing transform grids).
     *
     * \a preferredOperation gives the details of the preferred coordinate operation, and
     * \a availableOperation gives the details of the actual operation to be used during the
     * transform.
     *
     * \since QGIS 3.8
     */
    static void setCustomMissingPreferredGridHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QgsDatumTransform::TransformDetails &preferredOperation,
        const QgsDatumTransform::TransformDetails &availableOperation )> &handler );

    /**
     * Sets a custom handler to use when a coordinate transform was required between \a sourceCrs and
     * \a destinationCrs, yet the coordinate operation could not be created. The \a error argument
     * specifies the error message obtained.
     *
     * \since QGIS 3.8
     */
    static void setCustomCoordinateOperationCreationErrorHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QString &error )> &handler );

    /**
     * Sets a custom handler to use when a coordinate operation was specified for use between \a sourceCrs and
     * \a destinationCrs by the transform context, yet the coordinate operation could not be created. The \a desiredOperation argument
     * specifies the desired transform details as specified by the context.
     *
     * \since QGIS 3.8
     */
    static void setCustomMissingGridUsedByContextHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QgsDatumTransform::TransformDetails &desiredOperation )> &handler );

    /**
     * Sets a custom \a handler to use when the desired coordinate operation for use between \a sourceCrs and
     * \a destinationCrs is a dynamic CRS to dynamic CRS transform, not currently supported by PROJ.
     *
     * \since QGIS 3.20
     */
    static void setDynamicCrsToDynamicCrsWarningHandler( const std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs )> &handler );

  private:

    void freeProj();

    static std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QgsDatumTransform::GridDetails &grid )> sMissingRequiredGridHandler;

    static std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QgsDatumTransform::TransformDetails &preferredOperation,
                                const QgsDatumTransform::TransformDetails &availableOperation )> sMissingPreferredGridHandler;

    static std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QString &error )> sCoordinateOperationCreationErrorHandler;

    static std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QgsDatumTransform::TransformDetails &desiredOperation )> sMissingGridUsedByContextHandler;

    static std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs )> sDynamicCrsToDynamicCrsWarningHandler;

    QgsCoordinateTransformPrivate &operator= ( const QgsCoordinateTransformPrivate & ) = delete;
};

/// @endcond

#endif // QGSCOORDINATETRANSFORMPRIVATE_H
