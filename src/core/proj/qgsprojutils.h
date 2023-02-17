/***************************************************************************
                             qgsprojutils.h
                             -------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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
#ifndef QGSPROJUTILS_H
#define QGSPROJUTILS_H

#include <QtGlobal>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsconfig.h"
#include "qgsdatumtransform.h"
#include <memory>
#include <QStringList>

#if !defined(USE_THREAD_LOCAL) || defined(Q_OS_WIN)
#include <QThreadStorage>
#endif

#ifndef SIP_RUN
struct PJconsts;
typedef struct PJconsts PJ;
#endif

/**
 * \class QgsProjUtils
 * \ingroup core
 * \brief Utility functions for working with the proj library.
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProjUtils
{
  public:

    /**
     * Returns the proj library major version number.
     */
    static int projVersionMajor();

    /**
     * Returns the proj library minor version number.
     *
     * \since QGIS 3.20
     */
    static int projVersionMinor();

    /**
     * Returns the EPSG registry database version used by the proj library (e.g. "v9.8.6").
     *
     * \see epsgRegistryDate()
     * \since QGIS 3.20
     */
    static QString epsgRegistryVersion();

    /**
     * Returns the EPSG registry database release date used by the proj library.
     *
     * \see epsgRegistryVersion()
     * \since QGIS 3.20
     */
    static QDate epsgRegistryDate();

    /**
     * Returns the ESRI projection engine database version used by the proj library (e.g. "ArcMap 10.8.0").
     *
     * \see esriDatabaseDate()
     * \since QGIS 3.20
     */
    static QString esriDatabaseVersion();

    /**
     * Returns the ESRI projection engine database release date used by the proj library.
     *
     * \see esriDatabaseVersion()
     * \since QGIS 3.20
     */
    static QDate esriDatabaseDate();

    /**
     * Returns the IGNF database version used by the proj library (e.g. "3.1.0").
     *
     * \see ignfDatabaseDate()
     * \since QGIS 3.20
     */
    static QString ignfDatabaseVersion();

    /**
     * Returns the IGNF database release date used by the proj library.
     *
     * \see ignfDatabaseVersion()
     * \since QGIS 3.20
     */
    static QDate ignfDatabaseDate();

    /**
     * Returns the current list of Proj file search paths.
     *
     * \note Only available on builds based on Proj >= 6.0. Builds based on
     * earlier Proj versions will always return an empty list.
     */
    static QStringList searchPaths();

#ifndef SIP_RUN

    //! Flags controlling CRS identification behavior
    enum IdentifyFlag
    {
      FlagMatchBoundCrsToUnderlyingSourceCrs = 1 << 0, //!< Allow matching a BoundCRS object to its underlying SourceCRS
    };
    Q_DECLARE_FLAGS( IdentifyFlags, IdentifyFlag )

    /**
     * Destroys Proj PJ objects.
     */
    struct ProjPJDeleter
    {

      /**
       * Destroys an PJ \a object, using the correct proj calls.
       */
      void CORE_EXPORT operator()( PJ *object ) const;

    };

    /**
     * Scoped Proj PJ object.
     */
    using proj_pj_unique_ptr = std::unique_ptr< PJ, ProjPJDeleter >;

    /**
     * Returns TRUE if the given proj coordinate system uses angular units. \a projDef must be
     * a proj string defining a CRS object.
     */
    static bool usesAngularUnit( const QString &projDef );

    //TODO - remove when proj 6.1 is minimum supported version, and replace with proj_normalize_for_visualization

    /**
     * Returns TRUE if the given proj coordinate system uses requires y/x coordinate
     * order instead of x/y.
     */
    static bool axisOrderIsSwapped( const PJ *crs );

    /**
     * Returns TRUE if the given proj coordinate system is a dynamic CRS.
     *
     * A dynamic CRS relies on a dynamic datum, that is a datum that is not
     * plate-fixed.
     *
     * \since QGIS 3.20
     */
    static bool isDynamic( const PJ *crs );

    /**
     * Given a PROJ crs (which may be a compound or bound crs, or some other type), extract a single crs
     * from it.
     */
    static proj_pj_unique_ptr crsToSingleCrs( const PJ *crs );

    /**
     * Given a PROJ \a crs, attempt to retrieve the datum ensemble from it.
     *
     * \warning This method requires PROJ 8.0 or later
     *
     * \throws QgsNotSupportedException on QGIS builds based on PROJ 7 or earlier.
     *
     * \since QGIS 3.20
     */
    static proj_pj_unique_ptr crsToDatumEnsemble( const PJ *crs );

    /**
     * Attempts to identify a \a crs, matching it to a known authority and code within
     * an acceptable level of tolerance.
     *
     * Returns TRUE if a matching authority and code was found.
     */
    static bool identifyCrs( const PJ *crs, QString &authName, QString &authCode, IdentifyFlags flags = IdentifyFlags() );

    /**
     * Returns TRUE if a coordinate operation (specified via proj string) is available.
     */
    static bool coordinateOperationIsAvailable( const QString &projDef );

    /**
     * Returns a list of grids used by the given \a proj string.
     */
    static QList< QgsDatumTransform::GridDetails > gridsUsed( const QString &proj );

#if 0 // not possible in current Proj 6 API

    /**
     * Given a coordinate operation (specified via proj string), returns a list of
     * any required grids which are not currently available for use.
     */
    static QStringList nonAvailableGrids( const QString &projDef );
#endif
#endif
};

#ifndef SIP_RUN

#if PROJ_VERSION_MAJOR>=8
struct pj_ctx;
typedef struct pj_ctx PJ_CONTEXT;
#else
struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;
#endif

/**
 * \class QgsProjContext
 * \ingroup core
 * \brief Used to create and store a proj context object, correctly freeing the context upon destruction.
 * \note Not available in Python bindings
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProjContext
{
  public:

    QgsProjContext();
    ~QgsProjContext();

    /**
     * Returns a thread local instance of a proj context, safe for use in the current thread.
     */
    static PJ_CONTEXT *get();

  private:
    PJ_CONTEXT *mContext = nullptr;

    /**
     * Thread local proj context storage. A new proj context will be created
     * for every thread.
     */
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
    static thread_local QgsProjContext sProjContext;
#else
    static QThreadStorage< QgsProjContext * > sProjContext;
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProjUtils::IdentifyFlags )
#endif
#endif // QGSPROJUTILS_H
