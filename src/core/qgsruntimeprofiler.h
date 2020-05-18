/***************************************************************************
    qgsruntimeprofiler.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRUNTIMEPROFILER_H
#define QGSRUNTIMEPROFILER_H

#include <QTime>
#include <QElapsedTimer>
#include "qgis_sip.h"
#include <QPair>
#include <QStack>
#include <QList>

#include "qgis_core.h"

/**
 * \ingroup core
 * \class QgsRuntimeProfiler
 */
class CORE_EXPORT QgsRuntimeProfiler
{
  public:

    /**
     * Constructor to create a new runtime profiler.
     */
    QgsRuntimeProfiler() = default;

    /**
     * \brief Begin the group for the profiler. Groups will append {GroupName}/ to the
     * front of the profile tag set using start.
     * \param name The name of the group.
     *
     * \deprecated use start() instead
     */
    Q_DECL_DEPRECATED void beginGroup( const QString &name ) SIP_DEPRECATED;

    /**
     * \brief End the current active group.
     *
     * \deprecated use end() instead
     */
    Q_DECL_DEPRECATED void endGroup() SIP_DEPRECATED;

    /**
     * Returns a list of all child groups with the specified \a parent.
     * \since QGIS 3.14
     */
    QStringList childGroups( const QString &parent = QString() ) const;

    /**
     * \brief Start a profile event with the given name.
     * \param name The name of the profile event. Will have the name of
     * the active group appended after ending.
     */
    void start( const QString &name );

    /**
     * \brief End the current profile event.
     */
    void end();

    /**
     * Returns the profile time for the specified \a name.
     * \since QGIS 3.14
     */
    double profileTime( const QString &name ) const;

    /**
     * \brief clear Clear all profile data.
     */
    void clear();

    /**
     * \brief The current total time collected in the profiler.
     * \returns The current total time collected in the profiler.
     */
    double totalTime();

  private:

    QStack< QElapsedTimer > mProfileTime;
    QStack< QString > mCurrentName;
    QList< QPair< QString, double > > mProfileTimes;
};


/**
 * \ingroup core
 *
 * Scoped object for logging of the runtime for a single operation or group of operations.
 *
 * This class automatically takes care of registering an operation in the QgsApplication::profiler()
 * registry upon construction, and recording of the elapsed runtime upon destruction.
 *
 * Python scripts should not use QgsScopedRuntimeProfile directly. Instead, use QgsRuntimeProfiler.profile()
 * \code{.py}
 *   with QgsRuntimeProfiler.profile('My operation'):
 *     # do something
 * \endcode
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsScopedRuntimeProfile
{
  public:

    /**
     * Constructor for QgsScopedRuntimeProfile.
     *
     * Automatically registers the operation in the QgsApplication::profiler() instance
     * and starts recording the run time of the operation.
     */
    QgsScopedRuntimeProfile( const QString &name );

    /**
     * Records the final runtime of the operation in the profiler instance.
     */
    ~QgsScopedRuntimeProfile();

};


#endif // QGSRUNTIMEPROFILER_H
