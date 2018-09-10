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
