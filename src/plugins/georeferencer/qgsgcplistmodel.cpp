/***************************************************************************
    qgsgcplistmodel.cpp - Model implementation of GCPList Model/View
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsgcplist.h"
#include "qgsgcplistmodel.h"
#include "qgis.h"
#include "qgsgeorefdatapoint.h"
#include "qgsgeoreftransform.h"
#include <QSettings>

#include <cmath>
using namespace std;

class QgsStandardItem : public QStandardItem
{
  public:
    QgsStandardItem( QString text ) : QStandardItem( text )
    {
      // In addition to the DisplayRole, also set the user role, which is used for sorting.
      setData( QVariant( text ), Qt::UserRole );
      setTextAlignment( Qt::AlignRight );
    }

    QgsStandardItem( int value ) : QStandardItem( QString::number( value ) )
    {
      // In addition to the DisplayRole, also set the user role, which is used for sorting.
      // This is needed for numerical sorting to work corretly (otherwise sorting is lexicographic).
      setData( QVariant( value ), Qt::UserRole );
      setTextAlignment( Qt::AlignCenter );
    }

    QgsStandardItem( double value ) : QStandardItem( QString::number( value, 'f', 2 ) )
    {
      // In addition to the DisplayRole, also set the user role, which is used for sorting.
      // This is needed for numerical sorting to work corretly (otherwise sorting is lexicographic).
      setData( QVariant( value ), Qt::UserRole );
      setTextAlignment( Qt::AlignRight );
    }
};

QgsGCPListModel::QgsGCPListModel( QObject *parent )
    : QStandardItemModel( parent )
    , mGCPList( 0 )
    , mGeorefTransform( 0 )
{
  // Use data provided by Qt::UserRole as sorting key (needed for numerical sorting).
  setSortRole( Qt::UserRole );
}

void QgsGCPListModel::setGCPList( QgsGCPList *theGCPList )
{
  mGCPList = theGCPList;
  updateModel();
}

// ------------------------------- public ---------------------------------- //
void QgsGCPListModel::setGeorefTransform( QgsGeorefTransform *theGeorefTransform )
{
  mGeorefTransform = theGeorefTransform;
  updateModel();
}

void QgsGCPListModel::updateModel()
{
  //clear();
  if ( !mGCPList )
    return;

  bool bTransformUpdated = false;
  QgsPoint origin;

  vector<QgsPoint> mapCoords, pixelCoords;
  mGCPList->createGCPVectors( mapCoords, pixelCoords );

  //  // Setup table header
  QStringList itemLabels;
  QString unitType;
  QSettings s;
  bool mapUnitsPossible = false;

  if ( mGeorefTransform )
  {
    bTransformUpdated = mGeorefTransform->updateParametersFromGCPs( mapCoords, pixelCoords );
    mapUnitsPossible = mGeorefTransform->providesAccurateInverseTransformation();
  }


  if ( s.value( "/Plugin-GeoReferencer/Config/ResidualUnits" ) == "mapUnits" && mapUnitsPossible )
  {
    unitType = tr( "map units" );
  }
  else
  {
    unitType = tr( "pixels" );
  }

  itemLabels << "on/off" << "id" << "srcX" << "srcY" << "dstX" << "dstY" << QString( "dX[" ) + unitType + "]" << QString( "dY[" ) + unitType + "]" << "residual[" + unitType + "]";

  setHorizontalHeaderLabels( itemLabels );
  setRowCount( mGCPList->size() );

  for ( int i = 0; i < mGCPList->sizeAll(); ++i )
  {
    int j = 0;
    QgsGeorefDataPoint *p = mGCPList->at( i );
    p->setId( i );

    QStandardItem *si = new QStandardItem();
    si->setTextAlignment( Qt::AlignCenter );
    si->setCheckable( true );
    if ( p->isEnabled() )
      si->setCheckState( Qt::Checked );
    else
      si->setCheckState( Qt::Unchecked );

    setItem( i, j++, si );
    setItem( i, j++, new QgsStandardItem( i ) );
    setItem( i, j++, new QgsStandardItem( p->pixelCoords().x() ) );
    setItem( i, j++, new QgsStandardItem( -p->pixelCoords().y() ) );
    setItem( i, j++, new QgsStandardItem( p->mapCoords().x() ) );
    setItem( i, j++, new QgsStandardItem( p->mapCoords().y() ) );

    double residual;
    double dX = 0;
    double dY = 0;
    // Calculate residual if transform is available and up-to-date
    if ( mGeorefTransform && bTransformUpdated && mGeorefTransform->parametersInitialized() )
    {
      QgsPoint dst;
      if ( unitType == tr( "pixels" ) )
      {
        // Transform from world to raster coordinate:
        // This is the transform direction used by the warp operation.
        // As transforms of order >=2 are not invertible, we are only
        // interested in the residual in this direction
        if ( mGeorefTransform->transformWorldToRaster( p->mapCoords(), dst ) )
        {
          dX = ( dst.x() - p->pixelCoords().x() );
          dY = -( dst.y() - p->pixelCoords().y() );
        }
      }
      else if ( unitType == tr( "map units" ) )
      {
        if ( mGeorefTransform->transformRasterToWorld( p->pixelCoords(), dst ) )
        {
          dX = ( dst.x() - p->mapCoords().x() );
          dY = ( dst.y() - p->mapCoords().y() );
        }
      }
    }
    residual = sqrt( dX * dX + dY * dY );

    if ( p )
    {
      p->setResidual( QPointF( dX, dY ) );
    }

    if ( residual >= 0.f )
    {
      setItem( i, j++, new QgsStandardItem( dX ) );
      setItem( i, j++, new QgsStandardItem( dY ) );
      setItem( i, j++, new QgsStandardItem( residual ) );
    }
    else
    {
      setItem( i, j++, new QgsStandardItem( "n/a" ) );
      setItem( i, j++, new QgsStandardItem( "n/a" ) );
      setItem( i, j++, new QgsStandardItem( "n/a" ) );
    }
  }
}

// --------------------------- public slots -------------------------------- //
void QgsGCPListModel::replaceDataPoint( QgsGeorefDataPoint *newDataPoint, int i )
{
  mGCPList->replace( i, newDataPoint );
}

void QgsGCPListModel::onGCPListModified()
{
}

void QgsGCPListModel::onTransformationModified()
{
}
