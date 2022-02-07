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

#include "qgsgcplist.h"
#include "qgsgcplistmodel.h"
#include "qgis.h"
#include "qgsgeorefdatapoint.h"
#include "qgsgeoreftransform.h"
#include "qgssettings.h"

#include <cmath>

class QgsStandardItem : public QStandardItem
{
  public:
    explicit QgsStandardItem( const QString &text ) : QStandardItem( text )
    {
      // In addition to the DisplayRole, also set the user role, which is used for sorting.
      // This is needed for numerical sorting to work correctly (otherwise sorting is lexicographic).
      setData( QVariant( text ), Qt::UserRole );
      setTextAlignment( Qt::AlignRight );
    }

    explicit QgsStandardItem( int value ) : QStandardItem( QString::number( value ) )
    {
      setData( QVariant( value ), Qt::UserRole );
      setTextAlignment( Qt::AlignCenter );
    }

    explicit QgsStandardItem( double value ) : QStandardItem( QString::number( value, 'f', 4 ) )
    {
      setData( QVariant( value ), Qt::UserRole );
      //show the full precision when editing points
      setData( QVariant( value ), Qt::EditRole );
      setData( QVariant( value ), Qt::ToolTipRole );
      setTextAlignment( Qt::AlignRight );
    }
};

QgsGCPListModel::QgsGCPListModel( QObject *parent )
  : QStandardItemModel( parent )
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
void QgsGCPListModel::setGeorefTransform( QgsGeorefTransform *georefTransform )
{
  mGeorefTransform = georefTransform;
  updateModel();
}

void QgsGCPListModel::updateModel()
{

  //clear();
  if ( !mGCPList )
    return;

  bool bTransformUpdated = false;
  //  // Setup table header
  QStringList itemLabels;
  QString unitType;
  const QgsSettings s;
  bool mapUnitsPossible = false;
  QVector<QgsPointXY> sourceCoordinates;
  QVector<QgsPointXY> destinationCoordinates;

  mGCPList->createGCPVectors( sourceCoordinates, destinationCoordinates,
                              QgsCoordinateReferenceSystem( s.value( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ) ).toString() ) );

  if ( mGeorefTransform )
  {
    bTransformUpdated = mGeorefTransform->updateParametersFromGcps( sourceCoordinates, destinationCoordinates, true );
    mapUnitsPossible = mGeorefTransform->providesAccurateInverseTransformation();
  }

  if ( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ) ) == "mapUnits" && mapUnitsPossible )
  {
    unitType = tr( "map units" );
  }
  else
  {
    unitType = tr( "pixels" );
  }

  itemLabels << tr( "Visible" )
             << tr( "ID" )
             << tr( "Source X" )
             << tr( "Source Y" )
             << tr( "Dest. X" )
             << tr( "Dest. Y" )
             << tr( "dX (%1)" ).arg( unitType )
             << tr( "dY (%1)" ).arg( unitType )
             << tr( "Residual (%1)" ).arg( unitType );

  setHorizontalHeaderLabels( itemLabels );
  setRowCount( mGCPList->size() );

  for ( int i = 0; i < mGCPList->size(); ++i )
  {
    int j = 0;
    QgsGeorefDataPoint *p = mGCPList->at( i );

    if ( !p )
      continue;

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
    setItem( i, j++, new QgsStandardItem( p->sourceCoords().x() ) );
    setItem( i, j++, new QgsStandardItem( p->sourceCoords().y() ) );
    setItem( i, j++, new QgsStandardItem( p->transCoords().x() ) );
    setItem( i, j++, new QgsStandardItem( p->transCoords().y() ) );

    double residual;
    double dX = 0;
    double dY = 0;
    // Calculate residual if transform is available and up-to-date
    if ( mGeorefTransform && bTransformUpdated && mGeorefTransform->parametersInitialized() )
    {
      QgsPointXY dst;
      const QgsPointXY pixel = mGeorefTransform->toSourcePixel( p->sourceCoords() );
      if ( unitType == tr( "pixels" ) )
      {
        // Transform from world to raster coordinate:
        // This is the transform direction used by the warp operation.
        // As transforms of order >=2 are not invertible, we are only
        // interested in the residual in this direction
        if ( mGeorefTransform->transformWorldToRaster( p->transCoords(), dst ) )
        {
          dX = ( dst.x() - pixel.x() );
          dY = -( dst.y() - pixel.y() );
        }
      }
      else if ( unitType == tr( "map units" ) )
      {
        if ( mGeorefTransform->transformRasterToWorld( pixel, dst ) )
        {
          dX = ( dst.x() - p->transCoords().x() );
          dY = ( dst.y() - p->transCoords().y() );
        }
      }
    }
    residual = std::sqrt( dX * dX + dY * dY );

    p->setResidual( QPointF( dX, dY ) );

    if ( residual >= 0.f )
    {
      setItem( i, j++, new QgsStandardItem( dX ) );
      setItem( i, j++, new QgsStandardItem( dY ) );
      setItem( i, j++, new QgsStandardItem( residual ) );
    }
    else
    {
      setItem( i, j++, new QgsStandardItem( QStringLiteral( "n/a" ) ) );
      setItem( i, j++, new QgsStandardItem( QStringLiteral( "n/a" ) ) );
      setItem( i, j++, new QgsStandardItem( QStringLiteral( "n/a" ) ) );
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
