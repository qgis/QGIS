/***************************************************************************
                         qgsreadwritelocker.cpp
                         -------------------------
    begin                : September 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREADWRITELOCKER_H
#define QGSREADWRITELOCKER_H

#include "qgis_core.h"

#include <QReadWriteLock>

/**
 * \ingroup core
 * The QgsReadWriteLocker class is a convenience class that simplifies locking and unlocking QReadWriteLocks.
 *
 * Locking and unlocking a QReadWriteLocks in complex functions and statements or in exception handling code
 * is error-prone and difficult to debug.
 * QgsReadWriteLocker can be used in such situations to ensure that the state of the lock is always well-defined.
 *
 * QgsReadWriteLocker should be created within a function where a QReadWriteLock needs to be locked.
 * The lock may be locked when QgsReadWriteLocker is created or when changeMode is called.
 * You can unlock and relock the lock with unlock() and changeMode().
 * If locked, the lock will be unlocked when the QgsReadWriteLocker is destroyed.
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsReadWriteLocker
{
  public:

    /**
     * A QReadWriteLock can be in 3 different modes, read, write or unlocked.
     */
    enum Mode
    {
      Read, //!< Lock for read
      Write, //!< Lock for write
      Unlocked //!< Unlocked
    };

    /**
     * Create a new QgsReadWriteLocker for \a lock and initialize in \a mode.
     */
    QgsReadWriteLocker( QReadWriteLock &lock, Mode mode );

    /**
     * Change the mode of the lock to \a mode.
     * The lock will be unlocked and relocked as required.
     */
    void changeMode( Mode mode );

    /**
     * Unlocks the lock.
     * Equivalent to doing ``changeMode( QgsReadWriteLocker::Unlock );``
     */
    void unlock();

    ~QgsReadWriteLocker();

  private:
    QReadWriteLock &mLock;
    Mode mMode = Unlocked;
};

#endif // QGSREADWRITELOCKER_H
