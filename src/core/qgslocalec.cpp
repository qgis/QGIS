/***************************************************************************
                          qgslocalec.h - temporary C numeric locale
                          -------------------
    begin                : Jun 15th 2015
    copyright            : (C) 2015 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgslocalec.h>

#include <locale.h>
#include <QByteArray>

QMutex QgsLocaleNumC::sLocaleLock;

QgsLocaleNumC::QgsLocaleNumC()
{
  sLocaleLock.lock();

  mOldlocale = setlocale( LC_NUMERIC, NULL );
  if ( mOldlocale )
    mOldlocale = qstrdup( mOldlocale );

  setlocale( LC_NUMERIC, "C" );
}

QgsLocaleNumC::~QgsLocaleNumC()
{
  setlocale( LC_NUMERIC, mOldlocale );
  if ( mOldlocale )
    delete [] mOldlocale;

  sLocaleLock.unlock();
}
