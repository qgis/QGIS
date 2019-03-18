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

#include "qgis_core.h"
#include "qgsconfig.h"

#ifndef USE_THREAD_LOCAL
#include <QThreadStorage>
#endif

#define SIP_NO_FILE

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
#ifdef USE_THREAD_LOCAL
    static thread_local QgsProjContext sProjContext;
#else
    static QThreadStorage< QgsProjContext * > sProjContext;
#endif
};


#endif // QGSPROJUTILS_H
