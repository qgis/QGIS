/***************************************************************************
                     qgsbench.cpp  - Benchmark
                             -------------------
    begin                : 2011-11-15
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtGlobal>

#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#ifndef Q_OS_WIN
#include <sys/resource.h>
#endif
#include <time.h>
#include <math.h>

#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QTime>

#ifndef QGSVERSION
#include "qgsversion.h"
#endif
#include "qgsbench.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsproject.h"

const char *pre[] = { "user", "sys", "total", "wall" };

#ifdef Q_OS_WIN
// slightly adapted from http://anoncvs.postgresql.org/cvsweb.cgi/pgsql/src/port/getrusage.c?rev=1.18;content-type=text%2Fplain

#include <winsock2.h>
#include <errno.h>

#define RUSAGE_SELF     0

struct rusage
{
  struct timeval ru_utime;    /* user time used */
  struct timeval ru_stime;    /* system time used */
};

/*-------------------------------------------------------------------------
 *
 * getrusage.c
 *   get information about resource utilisation
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *   $PostgreSQL: pgsql/src/port/getrusage.c,v 1.18 2010-01-02 16:58:13 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */


int getrusage( int who, struct rusage * rusage )
{
  FILETIME starttime;
  FILETIME exittime;
  FILETIME kerneltime;
  FILETIME usertime;
  ULARGE_INTEGER li;

  if ( who != RUSAGE_SELF )
  {
    /* Only RUSAGE_SELF is supported in this implementation for now */
    errno = EINVAL;
    return -1;
  }

  if ( rusage == ( struct rusage * ) NULL )
  {
    errno = EFAULT;
    return -1;
  }
  memset( rusage, 0, sizeof( struct rusage ) );
  if ( GetProcessTimes( GetCurrentProcess(),
                        &starttime, &exittime, &kerneltime, &usertime ) == 0 )
  {
    // _dosmaperr(GetLastError());
    return -1;
  }

  /* Convert FILETIMEs (0.1 us) to struct timeval */
  memcpy( &li, &kerneltime, sizeof( FILETIME ) );
  li.QuadPart /= 10L;   /* Convert to microseconds */
  rusage->ru_stime.tv_sec = li.QuadPart / 1000000L;
  rusage->ru_stime.tv_usec = li.QuadPart % 1000000L;

  memcpy( &li, &usertime, sizeof( FILETIME ) );
  li.QuadPart /= 10L;   /* Convert to microseconds */
  rusage->ru_utime.tv_sec = li.QuadPart / 1000000L;
  rusage->ru_utime.tv_usec = li.QuadPart % 1000000L;

  return 0;
}
#endif

QgsBench::QgsBench( int theWidth, int theHeight, int theIterations )
    : QObject()
    , mWidth( theWidth )
    , mHeight( theHeight )
    , mIterations( theIterations )
    , mSetExtent( false )
    , mUserStart( 0.0 )
    , mSysStart( 0.0 )
    , mParallel( false )
{
  QgsDebugMsg( "entered" );

  QgsDebugMsg( QString( "mIterations = %1" ).arg( mIterations ) );

  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
}

QgsBench::~QgsBench()
{
}

bool QgsBench::openProject( const QString & theFileName )
{
  QgsDebugMsg( "entered" );
  // QgsProject loads layers to QgsMapLayerRegistry singleton
  QFileInfo file( theFileName );
  if ( ! QgsProject::instance()->read( file ) )
  {
    return false;
  }
  mLogMap.insert( "project", theFileName );
  return true;
}

void QgsBench::readProject( const QDomDocument &doc )
{
  QgsDebugMsg( "entered" );
  QDomNodeList nodes = doc.elementsByTagName( "mapcanvas" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    mMapSettings.readXML( node );
  }
  else
  {
    fprintf( stderr, "Cannot read mapcanvas from project\n" );
  }
}

void QgsBench::setExtent( const QgsRectangle & extent )
{
  mExtent = extent;
  mSetExtent = true;
}

void QgsBench::render()
{
  QgsDebugMsg( "entered" );

  QgsDebugMsg( "extent: " +  mMapSettings.extent().toString() );

  QMap<QString, QgsMapLayer*> layersMap = QgsMapLayerRegistry::instance()->mapLayers();

  QStringList layers( layersMap.keys() );

  mMapSettings.setLayers( layers );

  if ( mSetExtent )
  {
    mMapSettings.setExtent( mExtent );
  }

  // Maybe in future
  //outputCRS = QgsCRSCache::instance()->crsByAuthId( crsId );
  //mMapRenderer->setMapUnits( outputCRS.mapUnits() );
  //mMapRenderer->setDestinationCrs( outputCRS );

  // TODO: this should be probably set according to project
  mMapSettings.setCrsTransformEnabled( true );

  // Enable labeling
  mMapSettings.setFlag( QgsMapSettings::DrawLabeling );

  mMapSettings.setOutputSize( QSize( mWidth, mHeight ) );

  // TODO: do we need the other QPainter flags?
  mMapSettings.setFlag( QgsMapSettings::Antialiasing, mRendererHints.testFlag( QPainter::Antialiasing ) );

  for ( int i = 0; i < mIterations; i++ )
  {
    QgsMapRendererQImageJob* job;
    if ( mParallel )
      job = new QgsMapRendererParallelJob( mMapSettings );
    else
      job = new QgsMapRendererSequentialJob( mMapSettings );

    start();
    job->start();
    job->waitForFinished();
    elapsed();

    mImage = job->renderedImage();
    delete job;
  }


  mLogMap.insert( "iterations", mTimes.size() );
  mLogMap.insert( "revision", QGSVERSION );

  // Calc stats: user, sys, total
  double min[4], max[4];
  double stdev[4] = {0.};
  double maxdev[4] = {0.};
  double avg[4] = {0.};

  for ( int t = 0; t < 4; t++ )
  {
    for ( int i = 0; i < mTimes.size(); i++ )
    {
      avg[t] += mTimes[i][t];

      if ( i == 0 || mTimes[i][t] < min[t] ) min[t] = mTimes[i][t];
      if ( i == 0 || mTimes[i][t] > max[t] ) max[t] = mTimes[i][t];
    }
    avg[t] /= mTimes.size();
  }

  QMap<QString, QVariant> timesMap;
  for ( int t = 0; t < 4; t++ )
  {
    if ( mIterations > 1 )
    {
      for ( int i = 0; i < mTimes.size(); i++ )
      {
        double d = fabs( avg[t] - mTimes[i][t] );
        stdev[t] += pow( d, 2 );
        if ( i == 0 || d > maxdev[t] ) maxdev[t] = d;
      }

      stdev[t] = sqrt( stdev[t] / mTimes.size() );
    }

    QMap<QString, QVariant> map;

    map.insert( "min", min[t] );
    map.insert( "max", max[t] );
    map.insert( "avg", avg[t] );
    map.insert( "stdev", stdev[t] );
    map.insert( "maxdev", maxdev[t] );

    timesMap.insert( pre[t], map );
  }
  mLogMap.insert( "times", timesMap );
}

void QgsBench::saveSnapsot( const QString & fileName )
{
  // If format is 0, QImage will attempt to guess the format by looking at fileName's suffix.
  mImage.save( fileName );
}

void QgsBench::printLog( const QString& printTime )
{
  std::cout << "iterations: " << mLogMap["iterations"].toString().toAscii().constData() << std::endl;

  bool validPrintTime = false;
  for ( int x = 0; x < 4; ++x )
    if ( printTime == pre[x] )
      validPrintTime = true;

  if ( !validPrintTime )
  {
    std::cout << "invalid --print option: " << printTime.toAscii().data() << std::endl;
    return;
  }

  QMap<QString, QVariant> timesMap = mLogMap["times"].toMap();
  QMap<QString, QVariant> totalMap = timesMap[printTime].toMap();
  QMap<QString, QVariant>::iterator i = totalMap.begin();
  while ( i != totalMap.end() )
  {
    QString s = printTime + "_" + i.key() + ": " + i.value().toString();
    std::cout << s.toAscii().constData() << std::endl;
    ++i;
  }
}

QString QgsBench::serialize( QMap<QString, QVariant> theMap, int level )
{
  QStringList list;
  QString space = QString( " " ).repeated( level * 2 );
  QString space2 = QString( " " ).repeated( level * 2 + 2 );
  QMap<QString, QVariant>::const_iterator i = theMap.constBegin();
  while ( i != theMap.constEnd() )
  {
    switch ( i.value().type() )
    {
      case QMetaType::Int:
        list.append( space2 + "\"" + i.key() + "\": " + QString( "%1" ).arg( i.value().toInt() ) );
        break;
      case QMetaType::Double:
        list.append( space2 + "\"" + i.key() + "\": " + QString( "%1" ).arg( i.value().toDouble(), 0, 'f', 3 ) );
        break;
      case QMetaType::QString:
        list.append( space2 + "\"" + i.key() + "\": \"" + i.value().toString().replace( "\\", "\\\\" ).replace( "\"", "\\\"" ) + "\"" );
        break;
        //case QMetaType::QMap: QMap is not in QMetaType
      default:
        list.append( space2 + "\"" + i.key() + "\": " + serialize( i.value().toMap(), level + 1 ) );
        break;
    }
    ++i;
  }
  return space + "{\n" +  list.join( ",\n" ) + "\n" + space + "}";
}

void QgsBench::saveLog( const QString & fileName )
{
  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    return;

  QTextStream out( &file );
  out << serialize( mLogMap ).toAscii().constData() << "\n";
  file.close();
}

void QgsBench::start()
{
  struct rusage usage;
  getrusage( RUSAGE_SELF, &usage );
  mUserStart = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.;
  mSysStart = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.;
  mWallTime.start();
}

void QgsBench::elapsed()
{
  struct rusage usage;
  getrusage( RUSAGE_SELF, &usage );

  double userEnd = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.;
  double sysEnd = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.;

  double *t = new double[4];
  t[0] = userEnd - mUserStart;
  t[1] = sysEnd - mSysStart;
  t[2] = t[0] + t[1];
  t[3] = mWallTime.elapsed() / 1000.;
  mTimes.append( t );
}
