/***************************************************************************
  qgscrashreport.h - QgsCrashReport

 ---------------------
 begin                : 16.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCRASHREPORT_H
#define QGSCRASHREPORT_H

#include "qgis_app.h"
#include "qgsstacktrace.h"

#include <QObject>
#include <QVector>


/**
 * Include information to generate user friendly crash report for QGIS.
 */
class APP_EXPORT QgsCrashReport
{
  public:

    /**
     * Include information to generate user friendly crash report for QGIS.
     */
    QgsCrashReport();

  public:
    enum Flag
    {
      Stack                = 1 << 0,
      Plugins              = 1 << 1,
      ProjectDetails       = 1 << 2,
      SystemInfo           = 1 << 3,
      QgisInfo             = 1 << 4,
      All      = Stack | Plugins | ProjectDetails | SystemInfo | QgisInfo
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Sets the stack trace for the crash report.
     * \param value A string list for each line in the stack trace.
     */
    void setStackTrace( const QgsStackLines &value ) { mStackTrace = value; }

    /**
     * Returns the stack trace for this report.
     * \return A string list for each line in the stack trace.
     */
    QgsStackLines StackTrace() const { return mStackTrace; }

    /**
     * Set the flags to mark which features are included in this crash report.
     * \param flags The flag for each feature.
     */
    void setFlags( QgsCrashReport::Flags flags );

    /**
     * Returns the include flags that have been set for this report.
     * \return The flags marking what details are included in this report.
     */
    Flags flags() const { return mFlags; }

    /**
     * Generate a string version of the report.
     * \return A formatted string including all the information from the report.
     */
    const QString toString() const;

    /**
     * Generates a crash ID for the crash report.
     * \return
     */
    const QString crashID() const;

  private:
    Flags mFlags;
    QgsStackLines mStackTrace;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsCrashReport::Flags )

#endif // QGSCRASHREPORT_H
