/***************************************************************************
  qgsgraphbuilderinterface.cpp
  --------------------------------------
  Date                 : 2018-10-08
  Copyright            : (C) 2018 Denis Rouzaud
  Email                : denis@opengis.ch
****************************************************************************
*                                                                          *
*    *
*   it under the terms of the GNU General Public License as published by   *
*         *
*                                      *
*                                                                          *
***************************************************************************/

#include "qgsgraphbuilderinterface.h"

#include "qgsproject.h"

QgsGraphBuilderInterface::QgsGraphBuilderInterface( const QgsCoordinateReferenceSystem &crs, bool ctfEnabled,
    double topologyTolerance, const QString &ellipsoidID )
  : mCrs( crs )
  , mCtfEnabled( ctfEnabled )
  , mTopologyTolerance( topologyTolerance )
{
  mDa.setSourceCrs( mCrs, QgsProject::instance()->transformContext() );
  mDa.setEllipsoid( ellipsoidID );
}

void QgsGraphBuilderInterface::addVertex( int id, const QgsPointXY &pt )
{
  Q_UNUSED( id )
  Q_UNUSED( pt )
}

void QgsGraphBuilderInterface::addEdge( int pt1id, const QgsPointXY &pt1,
                                        int pt2id, const QgsPointXY &pt2,
                                        const QVector<QVariant> &strategies )
{
  Q_UNUSED( pt1id )
  Q_UNUSED( pt1 )
  Q_UNUSED( pt2id )
  Q_UNUSED( pt2 )
  Q_UNUSED( strategies )
}
