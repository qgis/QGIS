/***************************************************************************
                         qgsreadwritelocker.cpp
                         -------------------------
    begin                : September 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsreadwritelocker.h"

QgsReadWriteLocker::QgsReadWriteLocker( QReadWriteLock &lock, QgsReadWriteLocker::Mode mode )
  : mLock( lock )
  , mMode( mode )
{
  if ( mode == Read )
    mLock.lockForRead();
  else if ( mode == Write )
    mLock.lockForWrite();
}

void QgsReadWriteLocker::changeMode( QgsReadWriteLocker::Mode mode )
{
  if ( mode == mMode )
    return;

  unlock();

  mMode = mode;

  if ( mMode == Read )
    mLock.lockForRead();
  else if ( mMode == Write )
    mLock.lockForWrite();
}

void QgsReadWriteLocker::unlock()
{
  if ( mMode != Unlocked )
    mLock.unlock();

  mMode = Unlocked;
}

QgsReadWriteLocker::~QgsReadWriteLocker()
{
  unlock();
}
