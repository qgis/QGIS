/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SIMPLE_MUTEX_H_
#define _SIMPLE_MUTEX_H_


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Platform dependent mutex
#ifdef _HAVE_PTHREAD_
#include <pthread.h>
#define THREAD_TYPE pthread_mutex_t


#define CREATE_MUTEX(mutex) (pthread_mutex_init(&mutex, NULL))
#define LOCK(mutex)  (pthread_mutex_lock(&mutex))
#define UNLOCK(mutex)  (pthread_mutex_unlock(&mutex))
#define DESTROY_MUTEX(mutex) (pthread_mutex_destroy(&mutex))
#endif

#ifdef _HAVE_WINDOWS_H_
#include <windows.h>
#define THREAD_TYPE HANDLE
#define CREATE_MUTEX(mutex) (mutex = CreateMutex(0, false, 0))
#define LOCK(mutex)  (WaitForSingleObject(mutex, INFINITE))
#define UNLOCK(mutex)  (ReleaseMutex(mutex))
#define DESTROY_MUTEX(mutex) (CloseHandle(mutex))
#endif

namespace pal
{

  typedef THREAD_TYPE MUTEX_T;

  class SimpleMutex
  {
    private:
      MUTEX_T mutex;

    public:
      SimpleMutex()
      {
        CREATE_MUTEX( mutex );
      }

      ~SimpleMutex()
      {
        DESTROY_MUTEX( mutex );
      }

      void lock()
      {
        LOCK( mutex );
      }

      void unlock()
      {
        UNLOCK( mutex );
      }
  };

} // namespace

#endif
