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
#include "qgis_sip.h"
#include <QPair>
#include <QStack>

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
     */
    void beginGroup( const QString &name );

    /**
     * \brief End the current active group.
     */
    void endGroup();

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
     * Returns all the current profile times.
     * \returns A list of profile event names and times.
     * \note not available in Python bindings
     */
    const QList<QPair<QString, double > > profileTimes() const { return mProfileTimes; } SIP_SKIP

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
    QString mGroupPrefix;
    QStack<QString> mGroupStack;
    QTime mProfileTime;
    QString mCurrentName;
    QList<QPair<QString, double > > mProfileTimes;
};

#endif // QGSRUNTIMEPROFILER_H
