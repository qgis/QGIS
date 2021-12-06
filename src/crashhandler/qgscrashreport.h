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

#include "qgsstacktrace.h"

#include <QObject>
#include <QVector>


/**
 * Include information to generate user friendly crash report for QGIS.
 */
class QgsCrashReport
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

    enum class LikelyPythonFaultCause
    {
      NotPython,
      Unknown,
      ProcessingScript,
      Plugin,
      ConsoleCommand
    };

    /**
     * Sets the stack trace for the crash report.
     * \param value A string list for each line in the stack trace.
     */
    void setStackTrace( QgsStackTrace *value ) { mStackTrace = value; }

    /**
     * Returns the stack trace for this report.
     * \return A string list for each line in the stack trace.
     */
    QgsStackTrace *StackTrace() const { return mStackTrace; }

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
    const QString toHtml() const;

    /**
     * Generates a crash ID for the crash report.
     * \return
     */
    const QString crashID() const;

    void exportToCrashFolder();

    QString crashReportFolder();

    void setVersionInfo( const QStringList &versionInfo ) { mVersionInfo = versionInfo; }

    /**
     * Sets the \a path to the associated Python crash log.
     */
    void setPythonCrashLogFilePath( const QString &path );

    class PythonFault
    {
      public:

        LikelyPythonFaultCause cause = LikelyPythonFaultCause::NotPython;
        QString title;
        QString filePath;
    };

    PythonFault pythonFault() const { return mPythonFault; }

    /**
     * convert htmlToMarkdown (copied from QgsStringUtils::htmlToMarkdown)
     * \param html text in html
     * \return the reformatted text in markdown
     */
    static QString htmlToMarkdown( const QString &html );

  private:
    Flags mFlags;
    QgsStackTrace *mStackTrace = nullptr;
    QStringList mVersionInfo;
    QString mPythonCrashLogFilePath;

    PythonFault mPythonFault;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsCrashReport::Flags )

#endif // QGSCRASHREPORT_H
