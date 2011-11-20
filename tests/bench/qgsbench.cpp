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
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>
#include <math.h>

#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QTime>

#include "qgsbench.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"

QgsBench::QgsBench( int theWidth, int theHeight, int theIterations )
    : QObject(), mWidth( theWidth ), mHeight( theHeight ), mIterations( theIterations ), mSetExtent( false )
{
  QgsDebugMsg( "entered" );

  QgsDebugMsg( QString( "mIterations = %1" ).arg( mIterations ) );

  mMapRenderer = new QgsMapRenderer;

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
    mMapRenderer->readXML( node );
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

  QgsDebugMsg( "extent: " +  mMapRenderer->extent().toString() );

  QMap<QString, QgsMapLayer*> layersMap = QgsMapLayerRegistry::instance()->mapLayers();

  QStringList layers( layersMap.keys() );

  mMapRenderer->setLayerSet( layers );

  if ( mSetExtent )
  {
    mMapRenderer->setExtent( mExtent );
  }

  // Maybe in future
  //outputCRS = QgsCRSCache::instance()->crsByAuthId( crsId );
  //mMapRenderer->setMapUnits( outputCRS.mapUnits() );
  //mMapRenderer->setDestinationCrs( outputCRS );

  // TODO: this should be probably set according to project
  mMapRenderer->setProjectionsEnabled( true );

  // Necessary?
  //mMapRenderer->setLabelingEngine( new QgsPalLabeling() );

  mImage = new QImage( mWidth, mHeight, QImage::Format_ARGB32_Premultiplied );
  mImage->fill( 0 );

  mMapRenderer->setOutputSize( QSize( mWidth, mHeight ), mImage->logicalDpiX() );

  QPainter painter( mImage );

  painter.setRenderHints( mRendererHints );

  for ( int i = 0; i < mIterations; i++ )
  {
    start();
    mMapRenderer->render( &painter );
    elapsed();
  }

  mLogMap.insert( "iterations", mTimes.size() );

  // Calc stats: user, sys, total
  double avg[3], min[3], max[3];
  double stdev[3] = {0.};
  double maxdev[3] = {0.};

  for ( int t = 0; t < 3; t++ )
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
  for ( int t = 0; t < 3; t++ )
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

    const char *pre[] = { "user", "sys", "total" } ;

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
  mImage->save( fileName );
}

void QgsBench::printLog()
{
  std::cout << "iterations: " << mLogMap["iterations"].toString().toAscii().constData() << std::endl;

  QMap<QString, QVariant> timesMap = mLogMap["times"].toMap();
  QMap<QString, QVariant> totalMap = timesMap["total"].toMap();
  QMap<QString, QVariant>::iterator i = totalMap.begin();
  while ( i != totalMap.end() )
  {
    QString s = "total_" + i.key() + ": " + i.value().toString();
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
        list.append( space2 + i.key() + ": " + QString( "%1" ).arg( i.value().toInt() ) );
        break;
      case QMetaType::Double:
        list.append( space2 + i.key() + ": " + QString( "%1" ).arg( i.value().toDouble(), 0, 'f', 3 ) );
        break;
      case QMetaType::QString:
        list.append( space2 + i.key() + ": " + i.value().toString() );
        break;
        //case QMetaType::QMap: QMap is not in QMetaType
      default:
        list.append( space2 + i.key() + ": " + serialize( i.value().toMap(), level + 1 ) );
        break;
    }
    ++i;
  }
  return space + "{\n" +  list.join( ",\n" ) + "\n" + space + "}";
}

void QgsBench::saveLog( const QString & fileName )
{
  QFile file( fileName );
  file.open( QIODevice::WriteOnly | QIODevice::Text );
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
}

void QgsBench::elapsed()
{
  struct rusage usage;
  getrusage( RUSAGE_SELF, &usage );

  double userEnd = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.;
  double sysEnd = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.;

  double *t = new double[3];
  t[0] = userEnd - mUserStart;
  t[1] = sysEnd - mSysStart;
  t[2] = t[0] + t[1];
  mTimes.append( t );
}

