/***************************************************************************
    qgswfsutils.h
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSUTILS_H
#define QGSWFSUTILS_H

#include "qgsfeature.h"

#include <QString>
#include <QThread>
#include <QMutex>
#include <QSharedMemory>

/** Utility class to deal mostly with the management of the temporary directory
    that holds the on-disk cache. */
class QgsWFSUtils
{
  public:
    /** Return the name of temporary directory. */
    static QString acquireCacheDirectory();

    /** To be called when a temporary file is removed from the directory */
    static void releaseCacheDirectory();

    /** Initial cleanup. */
    static void init();

    /** Removes a possible namespace prefix from a typename*/
    static QString removeNamespacePrefix( const QString& tname );
    /** Returns namespace prefix (or an empty string if there is no prefix)*/
    static QString nameSpacePrefix( const QString& tname );

    /** Return a unique identifier made from feature content */
    static QString getMD5( const QgsFeature& f );

  protected:
    friend class QgsWFSUtilsKeepAlive;
    static QSharedMemory* createAndAttachSHM();

  private:
    static QMutex gmMutex;
    static QThread* gmThread;
    static bool gmKeepAliveWorks;
    static int gmCounter;

    /** Return the name of temporary directory. */
    static QString getCacheDirectory( bool createIfNotExisting );

    static QString getBaseCacheDirectory( bool createIfNotExisting );

    /** Remove (recursively) a directory. */
    static bool removeDir( const QString &dirName );
};

/** For internal use of QgsWFSUtils */
class QgsWFSUtilsKeepAlive: public QThread
{
    Q_OBJECT
  public:
    QgsWFSUtilsKeepAlive();
    ~QgsWFSUtilsKeepAlive();

    void run();
  private slots:
    void updateTimestamp();
  private:
    QSharedMemory* mSharedMemory;
};

#endif // QGSWFSUTILS_H
