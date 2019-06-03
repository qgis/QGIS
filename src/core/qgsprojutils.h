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
#include "qgsconfig.h"
#include "qgsdatumtransform.h"
#include <memory>
#include <QStringList>

#if !defined(USE_THREAD_LOCAL) || defined(Q_OS_WIN)
#include <QThreadStorage>
#endif

#if PROJ_VERSION_MAJOR>=6
#ifndef SIP_RUN
struct PJconsts;
typedef struct PJconsts PJ;
#endif
#endif

/**
 * \class QgsProjUtils
 * \ingroup core
 * Utility functions for working with the proj library.
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProjUtils
{
  public:

    /**
     * Returns the proj library major version number.
     */
    static int projVersionMajor()
    {
      return PROJ_VERSION_MAJOR;
    }

    /**
     * Returns the current list of Proj file search paths.
     *
     * \note Only available on builds based on Proj >= 6.0. Builds based on
     * earlier Proj versions will always return an empty list.
     */
    static QStringList searchPaths();

#ifndef SIP_RUN
#if PROJ_VERSION_MAJOR >= 6

    /**
     * Destroys Proj PJ objects.
     */
    struct ProjPJDeleter
    {

      /**
       * Destroys an PJ \a object, using the correct proj calls.
       */
      void CORE_EXPORT operator()( PJ *object );

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
     * Given a PROJ crs (which may be a compound or bound crs, or some other type), extract a single crs
     * from it.
     */
    static proj_pj_unique_ptr crsToSingleCrs( const PJ *crs );

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
#endif
};

#ifndef SIP_RUN

#if PROJ_VERSION_MAJOR>=6
struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;
#else
typedef void PJ_CONTEXT;
#endif

/**
 * \class QgsProjContext
 * \ingroup core
 * Used to create and store a proj context object, correctly freeing the context upon destruction.
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

#endif
#endif // QGSPROJUTILS_H
