/***************************************************************************
    qgsbacckgroundcachedshareddata.h
    ---------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCACHEDIRECTORYMANAGER_H
#define QGSCACHEDIRECTORYMANAGER_H

#include <QString>
#include <QThread>
#include <QMutex>

#if not defined( Q_OS_ANDROID )
#include <QSharedMemory>
#endif

#include <map>
#include <memory>

/**
 * Utility class to deal with the management of the temporary directory
 * that holds the on-disk cache.
*/
class QgsCacheDirectoryManager
{
  public:
    //! Returns the name of temporary directory. Must be paired with a call to releaseCacheDirectory()
    QString acquireCacheDirectory();

    //! To be called when a temporary file is removed from the directory
    void releaseCacheDirectory();

    //! Return the singleton for the given provider.
    static QgsCacheDirectoryManager &singleton( const QString &providerName );

  private:
    QMutex mMutex;
    QThread *mThread = nullptr;
    bool mKeepAliveWorks = false;
    int mCounter = 0;
    QString mProviderName;

    //! Used by singleton()
    static std::map<QString, std::unique_ptr<QgsCacheDirectoryManager>> sMap;

    //! Constructor
    QgsCacheDirectoryManager( const QString &providerName );

    //! Called by constructor
    void init();

#if not defined( Q_OS_ANDROID )
    //! Create a shared memory segment for the keep-alive mechanism
    std::unique_ptr<QSharedMemory> createAndAttachSHM();
#endif

    //! Returns the name of temporary directory.
    QString getCacheDirectory( bool createIfNotExisting );

    QString getBaseCacheDirectory( bool createIfNotExisting );

    //! Remove (recursively) a directory.
    static bool removeDir( const QString &dirName );
};

#if not defined( Q_OS_ANDROID )
//! For internal use of QgsCacheDirectoryManager
class QgsCacheDirectoryManagerKeepAlive : public QThread
{
    Q_OBJECT
  public:
    QgsCacheDirectoryManagerKeepAlive( std::unique_ptr<QSharedMemory> &&sharedMemory );
    ~QgsCacheDirectoryManagerKeepAlive() override;

    void run() override;
  private slots:
    void updateTimestamp();

  private:
    std::unique_ptr<QSharedMemory> mSharedMemory;
};
#endif

#endif // QGSCACHEDIRECTORYMANAGER_H
