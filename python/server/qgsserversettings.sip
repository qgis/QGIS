/***************************************************************************
                    qgsserversettings.sip
                    ---------------------
  begin                : December 19, 2016
  copyright            : (C) 2016 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/** \ingroup server
 * QgsServerSettings provides a way to retrieve settings by prioritizing
 * according to environment variables, ini file and default values.
 * @note added in QGIS 3.0
 */
class QgsServerSettings
{
%TypeHeaderCode
#include "qgsserversettings.h"
%End

  public:
    /** Constructor.
      */
    QgsServerSettings();

    /** Load settings according to current environment variables.
      */
    void load();

    /** Log a summary of settings curently loaded.
      */
    void logSummary() const;

    /** Returns the ini file loaded by QSetting.
      * @return the path of the ini file or an empty string if none is loaded.
      */
    QString iniFile() const;

    /** Returns the maximum number of threads to use.
      * @return the number of threads.
      */
    int maxThreads() const;

    /** Returns parallel rendering setting.
      * @return true if parallel rendering is activated, false otherwise.
      */
    bool parallelRendering() const;

    /** Returns the log level.
      * @return the log level.
      */
    QgsMessageLog::MessageLevel logLevel() const;

    /** Returns the log file.
      * @return the path of the log file or an empty string if none is defined.
      */
    QString logFile() const;

    /** Returns the QGS project file to use.
      * @return the path of the QGS project or an empty string if none is defined.
      */
    QString projectFile() const;

    /**
      * Returns the maximum number of cached layers.
      * @return the number of cached layers.
      */
    int maxCacheLayers() const;

    /** Returns the cache size.
      * @return the cache size.
      */
    qint64 cacheSize() const;

    /** Returns the cache directory.
      * @return the directory.
      */
    QString cacheDirectory() const;
};
