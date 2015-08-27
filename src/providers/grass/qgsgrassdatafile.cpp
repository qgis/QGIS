/***************************************************************************
                            qgsgrassdatafile.cpp
                             -------------------
    begin                : June, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgrassdatafile.h"

#ifdef Q_OS_UNIX
#include <sys/select.h>
#endif

QgsGrassDataFile::QgsGrassDataFile( QObject *parent )
    : QFile( parent )
    , mFh( 0 )
{
}

bool QgsGrassDataFile::open( FILE * fh )
{
  bool ret = QFile::open( fh, QIODevice::ReadOnly | QIODevice::Unbuffered );
  if ( ret )
  {
    mFh = fh;
  }
  return ret;
}

qint64 QgsGrassDataFile::readData( char * data, qint64 len )
{
  qint64 readSoFar = 0;
  forever
  {
    // QFile::readData should return -1 when pipe is closed, but it does not
    // and error is not set (at least with QProcess::closeWriteChannel).
    // In fact, qfsfileengine_unix.cpp QFSFileEnginePrivate::nativeRead returns -1
    // if (readBytes == 0 && !feof(fh)).
    //
    // feof(stdin) works (tested on Linux) but it is impossible to get FILE* from QFile
    // ( fdopen(handle(),"rb") returns FILE*, but it doesn't have eof set until read() is used on it)
    // => store FILE* in open()

    qint64 read = QFile::readData( data + readSoFar, len - readSoFar );
    if ( read == -1 )
    {
      return -1;
    }
    readSoFar += read;

    //fprintf(stderr, "len = %d readSoFar = %d feof = %d", (int)len, (int)readSoFar, (int)feof(mFh)  );
    if ( readSoFar == len )
    {
      break;
    }
    if ( feof( mFh ) )
    {
      return -1;
    }
    // Should we select()? QFile has no waitForReadyRead() implementation.
    // QFile::readData() seems to be blocking until there are data on Linux if pipe was not closed.
    // If pipe was closed, QFile::readData() does not block and returns 0 (instead of -1) but
    // we catch closed pipe above (feof) so select probably is not necessary, normally (on Linux) it is not reached.
    // TODO: verify what happens on Windows and possibly port select().
#ifdef Q_OS_UNIX
    if ( read == 0 )
    {
      fd_set readFds;
      FD_ZERO( &readFds );
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10000; // we could also wait for ever
      int sel = select( 0, &readFds, NULL, NULL, &tv );
      Q_UNUSED( sel );
      //fprintf(stderr, "sel = %d", sel);
    }
#endif
  }
  return readSoFar;
}
